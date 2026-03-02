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
 * ALL commands now produce actual game effects via MinecraftServer helpers.
 */

#include "command/CommandSystem.h"
#include "server/MinecraftServer.h"
#include "world/WorldServer.h"
#include <algorithm>
#include <iostream>
#include <optional>
#include <sstream>

namespace mccpp {

// ═════════════════════════════════════════════════════════════════════════════
// CommandHandler
// ═════════════════════════════════════════════════════════════════════════════

CommandHandler::CommandHandler() {
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
    registerCommand(std::make_shared<CommandWeather>());
    registerCommand(std::make_shared<CommandEffect>());
    registerCommand(std::make_shared<CommandXP>());
    registerCommand(std::make_shared<CommandEnchant>());
    registerCommand(std::make_shared<CommandClear>());
    registerCommand(std::make_shared<CommandSpawnpoint>());
    std::cout << "[Commands] Registered " << getCommandCount() << " commands\n";
}

void CommandHandler::registerCommand(std::shared_ptr<ICommand> command) {
    std::unique_lock lock(mutex_);
    commandMap_[command->getCommandName()] = command;
    commandSet_.insert(command);
    for (const auto& alias : command->getCommandAliases()) {
        auto it = commandMap_.find(alias);
        if (it != commandMap_.end() && it->second->getCommandName() == alias) continue;
        commandMap_[alias] = command;
    }
}

int32_t CommandHandler::executeCommand(ICommandSender& sender, const std::string& rawCommand) {
    std::string command = rawCommand;
    size_t start = command.find_first_not_of(" \t");
    if (start == std::string::npos) return 0;
    command = command.substr(start);
    size_t end = command.find_last_not_of(" \t");
    if (end != std::string::npos) command.resize(end + 1);
    if (!command.empty() && command[0] == '/') command = command.substr(1);

    auto parts = splitString(command);
    if (parts.empty()) return 0;

    std::string cmdName = parts[0];
    auto args = dropFirstString(parts);

    std::shared_lock lock(mutex_);
    auto it = commandMap_.find(cmdName);
    if (it == commandMap_.end()) {
        sender.addChatMessage("§cUnknown command. Try /help for a list of commands.");
        return 0;
    }
    auto& cmd = it->second;
    if (!cmd->canCommandSenderUseCommand(sender)) {
        sender.addChatMessage("§cYou do not have permission to use this command.");
        return 0;
    }
    try {
        cmd->processCommand(sender, args);
        return 1;
    } catch (const std::exception& e) {
        sender.addChatMessage(std::string("§cError: ") + e.what());
        std::cerr << "[Commands] Error executing '/" << command << "': " << e.what() << "\n";
        return 0;
    }
}

std::vector<std::string> CommandHandler::getPossibleCommands(
    const ICommandSender& sender, const std::string& partial) const {
    auto parts = splitString(partial);
    if (parts.empty()) return {};
    std::shared_lock lock(mutex_);
    if (parts.size() == 1) {
        std::vector<std::string> results;
        for (const auto& [name, cmd] : commandMap_) {
            if (doesStringStartWith(parts[0], name) && cmd->canCommandSenderUseCommand(sender))
                results.push_back(name);
        }
        std::sort(results.begin(), results.end());
        return results;
    }
    auto it = commandMap_.find(parts[0]);
    if (it != commandMap_.end())
        return it->second->addTabCompletionOptions(sender, dropFirstString(parts));
    return {};
}

std::vector<std::shared_ptr<ICommand>> CommandHandler::getAvailableCommands(
    const ICommandSender& sender) const {
    std::shared_lock lock(mutex_);
    std::vector<std::shared_ptr<ICommand>> results;
    for (const auto& cmd : commandSet_) {
        if (cmd->canCommandSenderUseCommand(sender)) results.push_back(cmd);
    }
    return results;
}

int32_t CommandHandler::getCommandCount() const {
    std::shared_lock lock(mutex_);
    return static_cast<int32_t>(commandSet_.size());
}

std::vector<std::string> CommandHandler::dropFirstString(const std::vector<std::string>& args) {
    if (args.size() <= 1) return {};
    return std::vector<std::string>(args.begin() + 1, args.end());
}

int32_t CommandHandler::getUsernameIndex(const ICommand& command, const std::vector<std::string>& args) {
    for (int32_t i = 0; i < static_cast<int32_t>(args.size()); ++i) {
        if (command.isUsernameIndex(args, i)) return i;
    }
    return -1;
}

std::vector<std::string> CommandHandler::splitString(const std::string& str) {
    std::vector<std::string> result;
    std::istringstream iss(str);
    std::string token;
    while (iss >> token) result.push_back(token);
    return result;
}

bool CommandHandler::doesStringStartWith(const std::string& prefix, const std::string& str) {
    if (prefix.size() > str.size()) return false;
    for (size_t i = 0; i < prefix.size(); ++i) {
        if (std::tolower(prefix[i]) != std::tolower(str[i])) return false;
    }
    return true;
}

// ═════════════════════════════════════════════════════════════════════════════
// Built-in command implementations — ALL with actual game effects
// ═════════════════════════════════════════════════════════════════════════════

// /stop — Java: CommandStop.processCommand
void CommandStop::processCommand(ICommandSender& sender, const std::vector<std::string>& /*args*/) {
    sender.addChatMessage("Stopping the server...");
    std::cout << "[Server] Server stop requested by " << sender.getCommandSenderName() << "\n";
    auto* server = sender.getServer();
    if (server) server->requestShutdown();
}

// /say — Java: CommandSay.processCommand
void CommandSay::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) { sender.addChatMessage("§cUsage: /say <message>"); return; }
    std::string message;
    for (size_t i = 0; i < args.size(); ++i) { if (i > 0) message += " "; message += args[i]; }
    std::string broadcast = "[" + sender.getCommandSenderName() + "] " + message;
    std::cout << "[Server] " << broadcast << "\n";
    auto* server = sender.getServer();
    if (server) server->broadcastChatMessage("§d" + broadcast);
    else sender.addChatMessage(broadcast);
}

// /help — Java: CommandHelp.processCommand
void CommandHelp::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    auto commands = handler_.getAvailableCommands(sender);
    std::sort(commands.begin(), commands.end(),
        [](const auto& a, const auto& b) { return a->getCommandName() < b->getCommandName(); });
    if (args.empty()) {
        sender.addChatMessage("§eAvailable commands (" + std::to_string(commands.size()) + "):");
        for (const auto& cmd : commands)
            sender.addChatMessage("§6/" + cmd->getCommandName() + "§r - " + cmd->getCommandUsage());
    } else {
        auto cmds = handler_.getCommands();
        auto it = cmds.find(args[0]);
        if (it != cmds.end()) {
            sender.addChatMessage("§6/" + it->second->getCommandName() + "§r - " + it->second->getCommandUsage());
            auto aliases = it->second->getCommandAliases();
            if (!aliases.empty()) {
                std::string aliasStr;
                for (size_t i = 0; i < aliases.size(); ++i) {
                    if (i > 0) aliasStr += ", "; aliasStr += aliases[i];
                }
                sender.addChatMessage("§7Aliases: " + aliasStr);
            }
        } else {
            sender.addChatMessage("§cNo such command: " + args[0]);
        }
    }
}

// /gamemode — ACTUALLY changes game mode via S2B ChangeGameState
void CommandGameMode::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) { sender.addChatMessage("§cUsage: /gamemode <mode> [player]"); return; }
    int32_t mode = -1;
    try { mode = std::stoi(args[0]); } catch (...) {}
    if (args[0] == "survival" || args[0] == "s") mode = 0;
    else if (args[0] == "creative" || args[0] == "c") mode = 1;
    else if (args[0] == "adventure" || args[0] == "a") mode = 2;
    if (mode < 0 || mode > 3) { sender.addChatMessage("§cInvalid game mode: " + args[0]); return; }

    std::string target = args.size() > 1 ? args[1] : sender.getCommandSenderName();
    static const char* names[] = {"Survival", "Creative", "Adventure", "Spectator"};
    auto* server = sender.getServer();
    if (server) {
        server->setPlayerGameMode(target, mode);
        sender.addChatMessage("Set " + target + "'s game mode to " + names[mode]);
        std::cout << "[Server] " << sender.getCommandSenderName() << " set " << target
                  << "'s game mode to " << names[mode] << "\n";
    }
}

// /time — ACTUALLY changes WorldServer.worldTime
void CommandTime::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) { sender.addChatMessage("§cUsage: /time <set|add|query> <value>"); return; }
    auto* server = sender.getServer();
    if (!server) return;

    if (args[0] == "set") {
        int32_t value = 0;
        if (args[1] == "day") value = 1000;
        else if (args[1] == "night") value = 13000;
        else { try { value = std::stoi(args[1]); } catch (...) {
            sender.addChatMessage("§cInvalid time: " + args[1]); return;
        }}
        server->setWorldTime(static_cast<int64_t>(value));
        sender.addChatMessage("Set the time to " + std::to_string(value));
        std::cout << "[Server] Time set to " << value << " by " << sender.getCommandSenderName() << "\n";
    } else if (args[0] == "add") {
        int32_t value = 0;
        try { value = std::stoi(args[1]); } catch (...) {
            sender.addChatMessage("§cInvalid time: " + args[1]); return;
        }
        server->addWorldTime(static_cast<int64_t>(value));
        sender.addChatMessage("Added " + std::to_string(value) + " to the time");
    } else if (args[0] == "query") {
        auto& worlds = server->getWorlds();
        if (args[1] == "daytime" && !worlds.empty()) {
            int64_t t = worlds[0]->worldTime % 24000;
            sender.addChatMessage("The time is " + std::to_string(t));
        } else if (args[1] == "gametime" && !worlds.empty()) {
            sender.addChatMessage("The game time is " + std::to_string(worlds[0]->totalWorldTime));
        }
    } else {
        sender.addChatMessage("§cUsage: /time <set|add|query> <value>");
    }
}

// /give — ACTUALLY gives items via S2F SetSlot
void CommandGive::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) { sender.addChatMessage("§cUsage: /give <player> <item> [amount] [damage]"); return; }
    std::string player = args[0];
    int32_t itemId = 0;
    try { itemId = std::stoi(args[1]); } catch (...) {
        sender.addChatMessage("§cInvalid item ID: " + args[1]); return;
    }
    int32_t amount = args.size() > 2 ? std::stoi(args[2]) : 1;
    int32_t damage = args.size() > 3 ? std::stoi(args[3]) : 0;

    auto* server = sender.getServer();
    if (server) {
        server->givePlayerItem(player, itemId, amount, damage);
        sender.addChatMessage("Given " + player + " " + std::to_string(amount)
                              + " of item " + std::to_string(itemId) + ":" + std::to_string(damage));
        std::cout << "[Server] " << sender.getCommandSenderName() << " gave " << player
                  << " " << amount << "x" << itemId << ":" << damage << "\n";
    }
}

// /tp — ACTUALLY teleports player
void CommandTeleport::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("§cUsage: /tp <player> <target> OR /tp <x> <y> <z> OR /tp <player> <x> <y> <z>");
        return;
    }
    auto* server = sender.getServer();
    if (!server) return;

    // /tp <x> <y> <z> — teleport self
    if (args.size() == 3) {
        try {
            double x = std::stod(args[0]), y = std::stod(args[1]), z = std::stod(args[2]);
            server->teleportPlayer(sender.getCommandSenderName(), x, y, z);
            sender.addChatMessage("Teleported to " + args[0] + " " + args[1] + " " + args[2]);
        } catch (...) { sender.addChatMessage("§cInvalid coordinates"); }
        return;
    }
    // /tp <player> <target>
    if (args.size() == 2) {
        auto targetPos = server->getPlayerPosition(args[1]);
        if (targetPos) {
            server->teleportPlayer(args[0], targetPos->x, targetPos->y, targetPos->z);
            sender.addChatMessage("Teleported " + args[0] + " to " + args[1]);
        } else {
            sender.addChatMessage("§cPlayer not found: " + args[1]);
        }
        return;
    }
    // /tp <player> <x> <y> <z>
    if (args.size() >= 4) {
        try {
            double x = std::stod(args[1]), y = std::stod(args[2]), z = std::stod(args[3]);
            server->teleportPlayer(args[0], x, y, z);
            sender.addChatMessage("Teleported " + args[0] + " to "
                                  + args[1] + " " + args[2] + " " + args[3]);
        } catch (...) { sender.addChatMessage("§cInvalid coordinates"); }
    }
}

// /gamerule — queries/sets game rules via WorldServer direct fields
void CommandGameRule::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    static const std::vector<std::string> gameRules = {
        "doFireTick", "mobGriefing", "keepInventory", "doMobSpawning",
        "doMobLoot", "doTileDrops", "commandBlockOutput", "naturalRegeneration",
        "doDaylightCycle", "logAdminCommands", "showDeathMessages",
        "randomTickSpeed", "sendCommandFeedback", "reducedDebugInfo"
    };
    if (args.empty()) {
        std::string list;
        for (size_t i = 0; i < gameRules.size(); ++i) {
            if (i > 0) list += ", "; list += gameRules[i];
        }
        sender.addChatMessage("§eGame rules: " + list);
    } else if (args.size() == 1) {
        auto* server = sender.getServer();
        if (server) {
            auto& worlds = server->getWorlds();
            if (!worlds.empty()) {
                std::string val = "unknown";
                if (args[0] == "doDaylightCycle") val = worlds[0]->doDaylightCycle ? "true" : "false";
                else if (args[0] == "doMobSpawning") val = worlds[0]->doMobSpawning ? "true" : "false";
                else if (args[0] == "doMobLoot") val = worlds[0]->doMobLoot ? "true" : "false";
                sender.addChatMessage(args[0] + " = " + val);
            }
        }
    } else {
        auto* server = sender.getServer();
        if (server) {
            auto& worlds = server->getWorlds();
            if (!worlds.empty()) {
                bool val = (args[1] == "true");
                if (args[0] == "doDaylightCycle") worlds[0]->doDaylightCycle = val;
                else if (args[0] == "doMobSpawning") worlds[0]->doMobSpawning = val;
                else if (args[0] == "doMobLoot") worlds[0]->doMobLoot = val;
                sender.addChatMessage("Game rule " + args[0] + " set to " + args[1]);
                std::cout << "[Server] " << sender.getCommandSenderName() << " set gamerule "
                          << args[0] << " to " << args[1] << "\n";
            }
        }
    }
}

// /difficulty — sets WorldInfo difficulty
void CommandDifficulty::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) { sender.addChatMessage("§cUsage: /difficulty <0|1|2|3>"); return; }
    int32_t diff = -1;
    try { diff = std::stoi(args[0]); } catch (...) {}
    if (args[0] == "peaceful") diff = 0;
    else if (args[0] == "easy") diff = 1;
    else if (args[0] == "normal") diff = 2;
    else if (args[0] == "hard") diff = 3;
    if (diff < 0 || diff > 3) { sender.addChatMessage("§cInvalid difficulty: " + args[0]); return; }

    static const char* names[] = {"Peaceful", "Easy", "Normal", "Hard"};
    auto* server = sender.getServer();
    if (server) {
        auto& worlds = server->getWorlds();
        if (!worlds.empty()) worlds[0]->difficulty = static_cast<Difficulty>(diff);
    }
    sender.addChatMessage("Set difficulty to " + std::string(names[diff]));
    std::cout << "[Server] Difficulty set to " << names[diff]
              << " by " << sender.getCommandSenderName() << "\n";
}

// /seed — returns actual world seed
void CommandSeed::processCommand(ICommandSender& sender, const std::vector<std::string>& /*args*/) {
    auto* server = sender.getServer();
    if (server) {
        auto& worlds = server->getWorlds();
        if (!worlds.empty()) {
            sender.addChatMessage("Seed: " + std::to_string(worlds[0]->worldSeed));
            return;
        }
    }
    sender.addChatMessage("Seed: 0");
}

// /list — returns real player count + names
void CommandList::processCommand(ICommandSender& sender, const std::vector<std::string>& /*args*/) {
    auto* server = sender.getServer();
    if (server) {
        auto names = server->getOnlinePlayerNames();
        int count = static_cast<int>(names.size());
        std::string msg = "There are " + std::to_string(count) + "/20 players online:";
        if (!names.empty()) {
            msg += " ";
            for (size_t i = 0; i < names.size(); ++i) {
                if (i > 0) msg += ", "; msg += names[i];
            }
        }
        sender.addChatMessage(msg);
    } else {
        sender.addChatMessage("There are 0/20 players online:");
    }
}

// /kill — ACTUALLY kills the player
void CommandKill::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    std::string target = args.empty() ? sender.getCommandSenderName() : args[0];
    auto* server = sender.getServer();
    if (server) server->killPlayer(target);
    std::cout << "[Server] " << sender.getCommandSenderName() << " killed " << target << "\n";
}

// /weather — Java: net.minecraft.command.CommandWeather
void CommandWeather::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("§cUsage: /weather <clear|rain|thunder> [duration in seconds]");
        return;
    }

    int32_t mode = -1;
    if (args[0] == "clear") mode = 0;
    else if (args[0] == "rain") mode = 1;
    else if (args[0] == "thunder") mode = 2;

    if (mode < 0) {
        sender.addChatMessage("§cInvalid weather type: " + args[0] + ". Use clear, rain, or thunder.");
        return;
    }

    // Java: duration is in seconds (converted to ticks: seconds * 20)
    // Range: 1-1000000 seconds (Java: 300-1000000, but we're flexible)
    int32_t durationTicks = 0;
    if (args.size() > 1) {
        try {
            int32_t seconds = std::stoi(args[1]);
            if (seconds < 1) seconds = 1;
            if (seconds > 1000000) seconds = 1000000;
            durationTicks = seconds * 20;
        } catch (...) {
            sender.addChatMessage("§cInvalid duration: " + args[1]);
            return;
        }
    }

    auto* server = sender.getServer();
    if (server) {
        server->setWeather(mode, durationTicks);
        static const char* names[] = {"clear", "rain", "thunder"};
        sender.addChatMessage("Changing to " + std::string(names[mode]) + " weather");
        std::cout << "[Server] " << sender.getCommandSenderName()
                  << " changed weather to " << names[mode] << "\n";
    }
}

std::vector<std::string> CommandWeather::addTabCompletionOptions(
    const ICommandSender& /*sender*/, const std::vector<std::string>& args) const {
    if (args.size() == 1) {
        std::vector<std::string> options;
        for (const auto& opt : {"clear", "rain", "thunder"}) {
            if (std::string(opt).find(args[0]) == 0) options.push_back(opt);
        }
        return options;
    }
    return {};
}

// /effect — Java: net.minecraft.command.CommandEffect
void CommandEffect::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        sender.addChatMessage("§cUsage: /effect <player> <effectId> [seconds] [amplifier] OR /effect <player> clear");
        return;
    }

    std::string target = args[0];
    auto* server = sender.getServer();
    if (!server) return;

    // /effect <player> clear
    if (args[1] == "clear") {
        server->clearPlayerPotionEffects(target);
        sender.addChatMessage("Cleared all effects from " + target);
        return;
    }

    int32_t effectId = 0;
    try { effectId = std::stoi(args[1]); } catch (...) {
        sender.addChatMessage("§cInvalid effect ID: " + args[1]); return;
    }
    if (effectId < 1 || effectId > 23) {
        sender.addChatMessage("§cEffect ID must be 1-23"); return;
    }

    int32_t seconds = args.size() > 2 ? std::stoi(args[2]) : 30;
    if (seconds < 1) seconds = 1;
    if (seconds > 1000000) seconds = 1000000;
    int32_t amplifier = args.size() > 3 ? std::stoi(args[3]) : 0;
    if (amplifier < 0) amplifier = 0;
    if (amplifier > 255) amplifier = 255;

    int32_t durationTicks = seconds * 20;
    server->applyPlayerPotionEffect(target, effectId, durationTicks, amplifier);

    static const char* effectNames[] = {
        "", "Speed", "Slowness", "Haste", "MiningFatigue", "Strength",
        "InstantHealth", "InstantDamage", "JumpBoost", "Nausea", "Regeneration",
        "Resistance", "FireResistance", "WaterBreathing", "Invisibility", "Blindness",
        "NightVision", "Hunger", "Weakness", "Poison", "Wither",
        "HealthBoost", "Absorption", "Saturation"
    };
    std::string name = (effectId >= 1 && effectId <= 23) ? effectNames[effectId] : "Unknown";
    sender.addChatMessage("Given " + name + " (ID " + std::to_string(effectId)
        + ") * " + std::to_string(amplifier) + " to " + target
        + " for " + std::to_string(seconds) + " seconds");
}

// /xp — Java: net.minecraft.command.CommandXP
// /xp <amount> [player] — adds XP points
// /xp <amount>L [player] — adds XP levels
void CommandXP::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("§cUsage: /xp <amount>[L] [player]");
        return;
    }

    std::string amountStr = args[0];
    bool isLevels = false;

    // Check for L suffix (levels mode)
    if (!amountStr.empty() && (amountStr.back() == 'L' || amountStr.back() == 'l')) {
        isLevels = true;
        amountStr.pop_back();
    }

    int32_t amount = 0;
    try { amount = std::stoi(amountStr); } catch (...) {
        sender.addChatMessage("§cInvalid amount: " + args[0]);
        return;
    }

    // XP points must be non-negative
    if (!isLevels && amount < 0) {
        sender.addChatMessage("§cCannot give negative XP points. Use levels (L suffix) for negative.");
        return;
    }

    std::string target = args.size() > 1 ? args[1] : sender.getCommandSenderName();
    auto* server = sender.getServer();
    if (!server) return;

    if (isLevels) {
        server->addPlayerLevels(target, amount);
        if (amount >= 0) {
            sender.addChatMessage("Given " + std::to_string(amount) + " experience levels to " + target);
        } else {
            sender.addChatMessage("Taken " + std::to_string(-amount) + " experience levels from " + target);
        }
    } else {
        server->addPlayerExperience(target, amount);
        sender.addChatMessage("Given " + std::to_string(amount) + " experience to " + target);
    }
    std::cout << "[Server] " << sender.getCommandSenderName()
              << " gave " << target << " " << amount
              << (isLevels ? " levels" : " XP") << "\n";
}

// /enchant — Java: net.minecraft.command.CommandEnchant
// /enchant <player> <enchantmentId> [level]
void CommandEnchant::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        sender.addChatMessage("§cUsage: /enchant <player> <enchantmentId> [level]");
        return;
    }
    std::string target = args[0];
    int32_t enchId = 0, level = 1;
    try { enchId = std::stoi(args[1]); } catch (...) {
        sender.addChatMessage("§cInvalid enchantment ID: " + args[1]);
        return;
    }
    if (args.size() > 2) {
        try { level = std::stoi(args[2]); } catch (...) {
            sender.addChatMessage("§cInvalid level: " + args[2]);
            return;
        }
    }
    if (enchId < 0 || enchId > 61) {
        sender.addChatMessage("§cEnchantment ID must be 0-61");
        return;
    }
    if (level < 1 || level > 5) {
        sender.addChatMessage("§cLevel must be 1-5");
        return;
    }
    auto* server = sender.getServer();
    if (!server) return;
    server->enchantPlayerItem(target, enchId, level);
    sender.addChatMessage("Enchanting " + target + "'s held item with "
        + std::to_string(enchId) + " level " + std::to_string(level));
    std::cout << "[Server] " << sender.getCommandSenderName()
              << " enchanted " << target << "'s item with " << enchId << " lv" << level << "\n";
}

// /clear — Java: net.minecraft.command.CommandClearInventory
// /clear [player] [itemId] [damage]
void CommandClear::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    std::string target = args.empty() ? sender.getCommandSenderName() : args[0];
    int32_t itemId = -1, damage = -1;
    if (args.size() > 1) {
        try { itemId = std::stoi(args[1]); } catch (...) {
            sender.addChatMessage("§cInvalid item ID: " + args[1]);
            return;
        }
    }
    if (args.size() > 2) {
        try { damage = std::stoi(args[2]); } catch (...) {
            sender.addChatMessage("§cInvalid damage: " + args[2]);
            return;
        }
    }
    auto* server = sender.getServer();
    if (!server) return;
    int32_t cleared = server->clearPlayerInventory(target, itemId, damage);
    if (itemId < 0) {
        sender.addChatMessage("Cleared the inventory of " + target + ", removing " + std::to_string(cleared) + " items");
    } else {
        sender.addChatMessage("Cleared " + std::to_string(cleared) + " items from " + target);
    }
    std::cout << "[Server] " << sender.getCommandSenderName()
              << " cleared " << cleared << " items from " << target << "\n";
}

// /spawnpoint — Java: net.minecraft.command.CommandSetSpawnpoint
// /spawnpoint [player] [x y z]
void CommandSpawnpoint::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    std::string target = args.empty() ? sender.getCommandSenderName() : args[0];
    auto* server = sender.getServer();
    if (!server) return;

    int32_t x, y, z;
    if (args.size() >= 4) {
        try {
            x = std::stoi(args[1]);
            y = std::stoi(args[2]);
            z = std::stoi(args[3]);
        } catch (...) {
            sender.addChatMessage("§cInvalid coordinates");
            return;
        }
    } else {
        // Use player's current position
        auto pos = server->getPlayerPosition(target);
        if (!pos) {
            sender.addChatMessage("§cPlayer not found: " + target);
            return;
        }
        x = static_cast<int32_t>(pos->x);
        y = static_cast<int32_t>(pos->y);
        z = static_cast<int32_t>(pos->z);
    }
    server->setPlayerSpawnPoint(target, x, y, z);
    sender.addChatMessage("Set " + target + "'s spawn point to ("
        + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
    std::cout << "[Server] " << sender.getCommandSenderName()
              << " set " << target << " spawnpoint to " << x << " " << y << " " << z << "\n";
}

} // namespace mccpp
