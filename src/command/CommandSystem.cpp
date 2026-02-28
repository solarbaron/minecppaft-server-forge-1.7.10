/**
 * CommandSystem.cpp — Command system implementation.
 *
 * Java references:
 *   net.minecraft.command.CommandHandler — command dispatch and registration
 *   net.minecraft.command.server.CommandStop — /stop
 *   net.minecraft.command.server.CommandSay — /say
 *   net.minecraft.command.CommandHelp — /help
 *   net.minecraft.command.CommandGameMode — /gamemode
 *   net.minecraft.command.CommandTime — /time
 *   net.minecraft.command.CommandGive — /give
 *   net.minecraft.command.CommandTeleport — /tp
 *   net.minecraft.command.CommandGameRule — /gamerule
 *
 * Key behaviors preserved from Java:
 *   - executeCommand: strips leading '/', splits by space, looks up command
 *   - registerCommand: registers by name AND all aliases
 *   - Aliases don't override existing primary commands
 *   - Tab completion: partial match on first arg, delegate on subsequent
 */

#include "command/CommandSystem.h"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace mccpp {

// ═════════════════════════════════════════════════════════════════════════════
// CommandHandler
// ═════════════════════════════════════════════════════════════════════════════

CommandHandler::CommandHandler() {
    // Register all built-in commands
    // Java: ServerCommandManager constructor
    registerCommand(std::make_shared<CommandStop>());
    registerCommand(std::make_shared<CommandSay>());
    registerCommand(std::make_shared<CommandHelp>(*this));
    registerCommand(std::make_shared<CommandGameMode>());
    registerCommand(std::make_shared<CommandTime>());
    registerCommand(std::make_shared<CommandGive>());
    registerCommand(std::make_shared<CommandTeleport>());
    registerCommand(std::make_shared<CommandGameRule>());
    registerCommand(std::make_shared<CommandDifficulty>());
    registerCommand(std::make_shared<CommandSeed>());
    registerCommand(std::make_shared<CommandList>());
    registerCommand(std::make_shared<CommandKill>());

    std::cout << "[Commands] Registered " << getCommandCount() << " commands\n";
}

void CommandHandler::registerCommand(std::shared_ptr<ICommand> command) {
    // Java: CommandHandler.registerCommand(ICommand)
    std::unique_lock lock(mutex_);

    commandMap_[command->getCommandName()] = command;
    commandSet_.insert(command);

    // Register aliases
    for (const auto& alias : command->getCommandAliases()) {
        auto it = commandMap_.find(alias);
        // Java: don't override if alias matches a primary command name
        if (it != commandMap_.end() && it->second->getCommandName() == alias) {
            continue;
        }
        commandMap_[alias] = command;
    }
}

int32_t CommandHandler::executeCommand(ICommandSender& sender, const std::string& rawCommand) {
    // Java: CommandHandler.executeCommand(ICommandSender, String)
    std::string command = rawCommand;

    // Trim whitespace
    size_t start = command.find_first_not_of(" \t");
    if (start == std::string::npos) return 0;
    command = command.substr(start);
    size_t end = command.find_last_not_of(" \t");
    if (end != std::string::npos) command.resize(end + 1);

    // Strip leading '/'
    if (!command.empty() && command[0] == '/') {
        command = command.substr(1);
    }

    auto parts = splitString(command);
    if (parts.empty()) return 0;

    std::string cmdName = parts[0];
    auto args = dropFirstString(parts);

    int32_t successCount = 0;

    std::shared_lock lock(mutex_);
    auto it = commandMap_.find(cmdName);

    if (it == commandMap_.end()) {
        // Java: throw new CommandNotFoundException()
        sender.addChatMessage("§cUnknown command. Try /help for a list of commands.");
        return 0;
    }

    auto& cmd = it->second;

    if (!cmd->canCommandSenderUseCommand(sender)) {
        // Java: "commands.generic.permission"
        sender.addChatMessage("§cYou do not have permission to use this command.");
        return 0;
    }

    try {
        cmd->processCommand(sender, args);
        ++successCount;
    } catch (const std::exception& e) {
        sender.addChatMessage(std::string("§cError: ") + e.what());
        std::cerr << "[Commands] Error executing '/" << command << "': " << e.what() << "\n";
    }

    return successCount;
}

std::vector<std::string> CommandHandler::getPossibleCommands(
    const ICommandSender& sender, const std::string& partial) const {
    // Java: CommandHandler.getPossibleCommands(ICommandSender, String)
    auto parts = splitString(partial);
    if (parts.empty()) return {};

    std::shared_lock lock(mutex_);

    if (parts.size() == 1) {
        // Complete command name
        std::vector<std::string> results;
        for (const auto& [name, cmd] : commandMap_) {
            if (doesStringStartWith(parts[0], name) && cmd->canCommandSenderUseCommand(sender)) {
                results.push_back(name);
            }
        }
        std::sort(results.begin(), results.end());
        return results;
    }

    // Delegate to command's tab completion
    auto it = commandMap_.find(parts[0]);
    if (it != commandMap_.end()) {
        return it->second->addTabCompletionOptions(sender, dropFirstString(parts));
    }

    return {};
}

std::vector<std::shared_ptr<ICommand>> CommandHandler::getAvailableCommands(
    const ICommandSender& sender) const {
    // Java: CommandHandler.getPossibleCommands(ICommandSender)
    std::shared_lock lock(mutex_);
    std::vector<std::shared_ptr<ICommand>> results;
    for (const auto& cmd : commandSet_) {
        if (cmd->canCommandSenderUseCommand(sender)) {
            results.push_back(cmd);
        }
    }
    return results;
}

int32_t CommandHandler::getCommandCount() const {
    std::shared_lock lock(mutex_);
    return static_cast<int32_t>(commandSet_.size());
}

std::vector<std::string> CommandHandler::dropFirstString(const std::vector<std::string>& args) {
    // Java: CommandHandler.dropFirstString(String[])
    if (args.size() <= 1) return {};
    return std::vector<std::string>(args.begin() + 1, args.end());
}

int32_t CommandHandler::getUsernameIndex(const ICommand& command, const std::vector<std::string>& args) {
    // Java: CommandHandler.getUsernameIndex(ICommand, String[])
    for (int32_t i = 0; i < static_cast<int32_t>(args.size()); ++i) {
        if (command.isUsernameIndex(args, i)) {
            return i;
        }
    }
    return -1;
}

std::vector<std::string> CommandHandler::splitString(const std::string& str) {
    std::vector<std::string> result;
    std::istringstream iss(str);
    std::string token;
    while (iss >> token) {
        result.push_back(token);
    }
    return result;
}

bool CommandHandler::doesStringStartWith(const std::string& prefix, const std::string& str) {
    // Java: CommandBase.doesStringStartWith(String, String)
    if (prefix.size() > str.size()) return false;
    for (size_t i = 0; i < prefix.size(); ++i) {
        if (std::tolower(prefix[i]) != std::tolower(str[i])) return false;
    }
    return true;
}

// ═════════════════════════════════════════════════════════════════════════════
// Built-in command implementations
// ═════════════════════════════════════════════════════════════════════════════

// /stop — Java: CommandStop.processCommand
void CommandStop::processCommand(ICommandSender& sender, const std::vector<std::string>& /*args*/) {
    sender.addChatMessage("Stopping the server...");
    std::cout << "[Server] Server stop requested by " << sender.getCommandSenderName() << "\n";
    // In full implementation, would set MinecraftServer.serverRunning = false
}

// /say — Java: CommandSay.processCommand
void CommandSay::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("§cUsage: /say <message>");
        return;
    }
    std::string message;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) message += " ";
        message += args[i];
    }
    // Java: broadcasts [sender] message
    std::string broadcast = "[" + sender.getCommandSenderName() + "] " + message;
    std::cout << "[Server] " << broadcast << "\n";
    sender.addChatMessage(broadcast);
}

// /help — Java: CommandHelp.processCommand
void CommandHelp::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    auto commands = handler_.getAvailableCommands(sender);
    std::sort(commands.begin(), commands.end(),
        [](const auto& a, const auto& b) { return a->getCommandName() < b->getCommandName(); });

    if (args.empty()) {
        // List all available commands
        // Java: shows page-based help (7 per page)
        sender.addChatMessage("§eAvailable commands (" + std::to_string(commands.size()) + "):");
        for (const auto& cmd : commands) {
            sender.addChatMessage("§6/" + cmd->getCommandName() + "§r - " + cmd->getCommandUsage());
        }
    } else {
        // Show help for specific command
        auto cmds = handler_.getCommands();
        auto it = cmds.find(args[0]);
        if (it != cmds.end()) {
            sender.addChatMessage("§6/" + it->second->getCommandName() + "§r - " + it->second->getCommandUsage());
            auto aliases = it->second->getCommandAliases();
            if (!aliases.empty()) {
                std::string aliasStr;
                for (size_t i = 0; i < aliases.size(); ++i) {
                    if (i > 0) aliasStr += ", ";
                    aliasStr += aliases[i];
                }
                sender.addChatMessage("§7Aliases: " + aliasStr);
            }
        } else {
            sender.addChatMessage("§cNo such command: " + args[0]);
        }
    }
}

// /gamemode — Java: CommandGameMode.processCommand
void CommandGameMode::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("§cUsage: /gamemode <mode> [player]");
        return;
    }
    // Java game modes: 0=survival, 1=creative, 2=adventure, 3=spectator
    int32_t mode = -1;
    try { mode = std::stoi(args[0]); } catch (...) {}

    if (args[0] == "survival" || args[0] == "s") mode = 0;
    else if (args[0] == "creative" || args[0] == "c") mode = 1;
    else if (args[0] == "adventure" || args[0] == "a") mode = 2;

    if (mode < 0 || mode > 3) {
        sender.addChatMessage("§cInvalid game mode: " + args[0]);
        return;
    }

    std::string target = args.size() > 1 ? args[1] : sender.getCommandSenderName();
    std::string modeName;
    switch (mode) {
        case 0: modeName = "Survival"; break;
        case 1: modeName = "Creative"; break;
        case 2: modeName = "Adventure"; break;
        case 3: modeName = "Spectator"; break;
    }
    sender.addChatMessage("Set " + target + "'s game mode to " + modeName);
    std::cout << "[Server] " << sender.getCommandSenderName() << " set " << target
              << "'s game mode to " << modeName << "\n";
}

// /time — Java: CommandTime.processCommand
void CommandTime::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        sender.addChatMessage("§cUsage: /time <set|add|query> <value>");
        return;
    }
    if (args[0] == "set") {
        int32_t value = 0;
        if (args[1] == "day") value = 1000;
        else if (args[1] == "night") value = 13000;
        else { try { value = std::stoi(args[1]); } catch (...) {
            sender.addChatMessage("§cInvalid time: " + args[1]);
            return;
        }}
        sender.addChatMessage("Set the time to " + std::to_string(value));
        std::cout << "[Server] Time set to " << value << " by " << sender.getCommandSenderName() << "\n";
    } else if (args[0] == "add") {
        int32_t value = 0;
        try { value = std::stoi(args[1]); } catch (...) {
            sender.addChatMessage("§cInvalid time: " + args[1]);
            return;
        }
        sender.addChatMessage("Added " + std::to_string(value) + " to the time");
    } else if (args[0] == "query") {
        if (args[1] == "daytime") {
            sender.addChatMessage("The time is 0"); // Would query actual world time
        } else if (args[1] == "gametime") {
            sender.addChatMessage("The game time is 0");
        }
    } else {
        sender.addChatMessage("§cUsage: /time <set|add|query> <value>");
    }
}

// /give — Java: CommandGive.processCommand
void CommandGive::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        sender.addChatMessage("§cUsage: /give <player> <item> [amount] [damage]");
        return;
    }
    std::string player = args[0];
    int32_t itemId = 0;
    try { itemId = std::stoi(args[1]); } catch (...) {
        sender.addChatMessage("§cInvalid item ID: " + args[1]);
        return;
    }
    int32_t amount = args.size() > 2 ? std::stoi(args[2]) : 1;
    int32_t damage = args.size() > 3 ? std::stoi(args[3]) : 0;

    sender.addChatMessage("Given " + player + " " + std::to_string(amount)
                          + " of item " + std::to_string(itemId) + ":" + std::to_string(damage));
    std::cout << "[Server] " << sender.getCommandSenderName() << " gave " << player
              << " " << amount << "x" << itemId << ":" << damage << "\n";
}

// /tp — Java: CommandTeleport.processCommand
void CommandTeleport::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        sender.addChatMessage("§cUsage: /tp <player> <target> OR /tp <player> <x> <y> <z>");
        return;
    }
    std::string player = args[0];

    if (args.size() == 2) {
        // Teleport player to target
        sender.addChatMessage("Teleported " + player + " to " + args[1]);
    } else if (args.size() >= 4) {
        // Teleport player to coordinates
        sender.addChatMessage("Teleported " + player + " to "
                              + args[1] + " " + args[2] + " " + args[3]);
    }
    std::cout << "[Server] " << sender.getCommandSenderName() << " teleported " << player << "\n";
}

// /gamerule — Java: CommandGameRule.processCommand
void CommandGameRule::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    // Java game rules in 1.7.10
    static const std::vector<std::string> gameRules = {
        "doFireTick", "mobGriefing", "keepInventory", "doMobSpawning",
        "doMobLoot", "doTileDrops", "commandBlockOutput", "naturalRegeneration",
        "doDaylightCycle", "logAdminCommands", "showDeathMessages",
        "randomTickSpeed", "sendCommandFeedback", "reducedDebugInfo"
    };

    if (args.empty()) {
        // List all game rules
        std::string list;
        for (size_t i = 0; i < gameRules.size(); ++i) {
            if (i > 0) list += ", ";
            list += gameRules[i];
        }
        sender.addChatMessage("§eGame rules: " + list);
    } else if (args.size() == 1) {
        // Query game rule
        sender.addChatMessage(args[0] + " = true"); // Simplified
    } else {
        // Set game rule
        sender.addChatMessage("Game rule " + args[0] + " set to " + args[1]);
        std::cout << "[Server] " << sender.getCommandSenderName() << " set gamerule "
                  << args[0] << " to " << args[1] << "\n";
    }
}

// /difficulty — Java: CommandDifficulty.processCommand
void CommandDifficulty::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("§cUsage: /difficulty <0|1|2|3>");
        return;
    }
    int32_t diff = -1;
    try { diff = std::stoi(args[0]); } catch (...) {}
    if (args[0] == "peaceful") diff = 0;
    else if (args[0] == "easy") diff = 1;
    else if (args[0] == "normal") diff = 2;
    else if (args[0] == "hard") diff = 3;

    if (diff < 0 || diff > 3) {
        sender.addChatMessage("§cInvalid difficulty: " + args[0]);
        return;
    }
    static const char* names[] = {"Peaceful", "Easy", "Normal", "Hard"};
    sender.addChatMessage("Set difficulty to " + std::string(names[diff]));
    std::cout << "[Server] Difficulty set to " << names[diff]
              << " by " << sender.getCommandSenderName() << "\n";
}

// /seed — Java: CommandShowSeed.processCommand
void CommandSeed::processCommand(ICommandSender& sender, const std::vector<std::string>& /*args*/) {
    // Would return actual world seed
    sender.addChatMessage("Seed: 0");
}

// /list — Java: CommandListPlayers.processCommand
void CommandList::processCommand(ICommandSender& sender, const std::vector<std::string>& /*args*/) {
    // Would list all connected players
    sender.addChatMessage("There are 0/20 players online:");
}

// /kill — Java: CommandKill.processCommand
void CommandKill::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    std::string target = args.empty() ? sender.getCommandSenderName() : args[0];
    sender.addChatMessage("Killed " + target);
    std::cout << "[Server] " << sender.getCommandSenderName() << " killed " << target << "\n";
}

} // namespace mccpp
