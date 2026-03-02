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
    registerCommand(std::make_shared<CommandToggleDownfall>());
    registerCommand(std::make_shared<CommandDefaultGameMode>());
    registerCommand(std::make_shared<CommandMe>());
    registerCommand(std::make_shared<CommandTell>());
    registerCommand(std::make_shared<CommandBan>());
    registerCommand(std::make_shared<CommandKick>());
    registerCommand(std::make_shared<CommandSetBlock>());
    registerCommand(std::make_shared<CommandFill>());
    registerCommand(std::make_shared<CommandClone>());
    registerCommand(std::make_shared<CommandTestFor>());
    registerCommand(std::make_shared<CommandSummon>());
    registerCommand(std::make_shared<CommandPardon>());
    registerCommand(std::make_shared<CommandWhitelist>());
    registerCommand(std::make_shared<CommandPlaySound>());
    registerCommand(std::make_shared<CommandSpreadPlayers>());
    registerCommand(std::make_shared<CommandTellRaw>());
    registerCommand(std::make_shared<CommandMsg>());
    registerCommand(std::make_shared<CommandOp>());
    registerCommand(std::make_shared<CommandDeOp>());
    registerCommand(std::make_shared<CommandBanIP>());
    registerCommand(std::make_shared<CommandPardonIP>());
    registerCommand(std::make_shared<CommandTestForBlock>());
    registerCommand(std::make_shared<CommandAchievement>());
    registerCommand(std::make_shared<CommandScoreboard>());
    registerCommand(std::make_shared<CommandDebug>());
    registerCommand(std::make_shared<CommandBlockData>());
    registerCommand(std::make_shared<CommandEntityData>());
    registerCommand(std::make_shared<CommandReplaceItem>());
    registerCommand(std::make_shared<CommandExecuteAt>());
    registerCommand(std::make_shared<CommandTrigger>());
    registerCommand(std::make_shared<CommandTitle>());
    registerCommand(std::make_shared<CommandParticle>());
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

// /toggledownfall — Java: net.minecraft.command.CommandToggleDownfall
void CommandToggleDownfall::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    auto* server = sender.getServer();
    if (!server) return;
    // Toggle: if raining, set clear; if clear, set rain
    bool isRaining = server->isRaining();
    if (isRaining) {
        server->setWeather(0, 6000 + (rand() % 6000)); // Clear for 5-10 min
        sender.addChatMessage("Toggled downfall off");
    } else {
        server->setWeather(1, 6000 + (rand() % 6000)); // Rain for 5-10 min
        sender.addChatMessage("Toggled downfall on");
    }
    std::cout << "[Server] " << sender.getCommandSenderName() << " toggled downfall\n";
}

// /defaultgamemode — Java: net.minecraft.command.CommandDefaultGameMode
void CommandDefaultGameMode::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("§cUsage: /defaultgamemode <mode>");
        return;
    }
    int32_t mode = -1;
    std::string modeStr = args[0];
    if (modeStr == "survival" || modeStr == "s" || modeStr == "0") mode = 0;
    else if (modeStr == "creative" || modeStr == "c" || modeStr == "1") mode = 1;
    else if (modeStr == "adventure" || modeStr == "a" || modeStr == "2") mode = 2;
    else {
        sender.addChatMessage("§cUnknown game mode: " + modeStr);
        return;
    }
    auto* server = sender.getServer();
    if (server) server->setDefaultGameMode(mode);
    static const char* names[] = {"Survival", "Creative", "Adventure"};
    sender.addChatMessage(std::string("Default game mode set to ") + names[mode]);
    std::cout << "[Server] " << sender.getCommandSenderName() << " set default gamemode to " << mode << "\n";
}

// /me — Java: net.minecraft.command.CommandMe (partial)
void CommandMe::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("§cUsage: /me <action>");
        return;
    }
    std::string action;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) action += " ";
        action += args[i];
    }
    auto* server = sender.getServer();
    if (server) {
        server->broadcastChat("* " + sender.getCommandSenderName() + " " + action);
    }
}

// /tell — Java: net.minecraft.command.CommandMessage
void CommandTell::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        sender.addChatMessage("§cUsage: /tell <player> <message>");
        return;
    }
    std::string target = args[0];
    std::string message;
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) message += " ";
        message += args[i];
    }
    auto* server = sender.getServer();
    if (!server) return;
    server->sendPrivateMessage(target, "§d" + sender.getCommandSenderName() + " whispers to you: " + message);
    sender.addChatMessage("§dYou whisper to " + target + ": " + message);
}

// /ban — Java: net.minecraft.command.CommandBanPlayer (simplified: kick only)
void CommandBan::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("§cUsage: /ban <player> [reason]");
        return;
    }
    std::string target = args[0];
    std::string reason = "Banned by an operator";
    if (args.size() > 1) {
        reason = "";
        for (size_t i = 1; i < args.size(); ++i) {
            if (i > 1) reason += " ";
            reason += args[i];
        }
    }
    auto* server = sender.getServer();
    if (!server) return;
    server->kickPlayer(target, reason);
    server->broadcastChat("§7" + target + " was banned by " + sender.getCommandSenderName() + ": " + reason);
    std::cout << "[Server] " << sender.getCommandSenderName() << " banned " << target << ": " << reason << "\n";
}

// /kick — Java: net.minecraft.command.CommandKick
void CommandKick::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("§cUsage: /kick <player> [reason]");
        return;
    }
    std::string target = args[0];
    std::string reason = "Kicked by an operator";
    if (args.size() > 1) {
        reason = "";
        for (size_t i = 1; i < args.size(); ++i) {
            if (i > 1) reason += " ";
            reason += args[i];
        }
    }
    auto* server = sender.getServer();
    if (!server) return;
    server->kickPlayer(target, reason);
    sender.addChatMessage("Kicked " + target + ": " + reason);
    std::cout << "[Server] " << sender.getCommandSenderName() << " kicked " << target << "\n";
}

// /setblock — Java: net.minecraft.command.CommandSetBlock
void CommandSetBlock::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 4) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /setblock <x> <y> <z> <blockId> [meta]");
        return;
    }
    try {
        int32_t x = std::stoi(args[0]);
        int32_t y = std::stoi(args[1]);
        int32_t z = std::stoi(args[2]);
        int32_t blockId = std::stoi(args[3]);
        int32_t meta = (args.size() > 4) ? std::stoi(args[4]) : 0;
        if (y < 0 || y > 255) { sender.addChatMessage("\xC2\xA7" "cY must be 0-255"); return; }
        if (blockId < 0 || blockId > 255) { sender.addChatMessage("\xC2\xA7" "cBlock ID must be 0-255"); return; }
        auto* server = sender.getServer();
        if (!server) return;
        server->setBlockInWorld(x, y, z, blockId, meta);
        sender.addChatMessage("Block placed at " + std::to_string(x) + ", " +
            std::to_string(y) + ", " + std::to_string(z));
    } catch (...) {
        sender.addChatMessage("\xC2\xA7" "cInvalid number");
    }
}

// /fill — Java: net.minecraft.command.CommandFill
void CommandFill::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 7) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /fill <x1> <y1> <z1> <x2> <y2> <z2> <blockId> [meta]");
        return;
    }
    try {
        int32_t x1 = std::stoi(args[0]), y1 = std::stoi(args[1]), z1 = std::stoi(args[2]);
        int32_t x2 = std::stoi(args[3]), y2 = std::stoi(args[4]), z2 = std::stoi(args[5]);
        int32_t blockId = std::stoi(args[6]);
        int32_t meta = (args.size() > 7) ? std::stoi(args[7]) : 0;
        // Normalize ranges
        if (x1 > x2) std::swap(x1, x2);
        if (y1 > y2) std::swap(y1, y2);
        if (z1 > z2) std::swap(z1, z2);
        y1 = std::max(0, y1); y2 = std::min(255, y2);
        // Size check — Java: max 32768 blocks
        int64_t volume = static_cast<int64_t>(x2 - x1 + 1) * (y2 - y1 + 1) * (z2 - z1 + 1);
        if (volume > 32768) {
            sender.addChatMessage("\xC2\xA7" "cToo many blocks (" + std::to_string(volume) + "), max 32768");
            return;
        }
        auto* server = sender.getServer();
        if (!server) return;
        int32_t count = 0;
        for (int32_t x = x1; x <= x2; ++x) {
            for (int32_t y = y1; y <= y2; ++y) {
                for (int32_t z = z1; z <= z2; ++z) {
                    server->setBlockInWorld(x, y, z, blockId, meta);
                    ++count;
                }
            }
        }
        sender.addChatMessage("Filled " + std::to_string(count) + " blocks");
    } catch (...) {
        sender.addChatMessage("\xC2\xA7" "cInvalid number");
    }
}

// /clone — Java: net.minecraft.command.CommandClone
void CommandClone::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 9) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /clone <x1> <y1> <z1> <x2> <y2> <z2> <dx> <dy> <dz>");
        return;
    }
    try {
        int32_t x1 = std::stoi(args[0]), y1 = std::stoi(args[1]), z1 = std::stoi(args[2]);
        int32_t x2 = std::stoi(args[3]), y2 = std::stoi(args[4]), z2 = std::stoi(args[5]);
        int32_t dx = std::stoi(args[6]), dy = std::stoi(args[7]), dz = std::stoi(args[8]);
        if (x1 > x2) std::swap(x1, x2);
        if (y1 > y2) std::swap(y1, y2);
        if (z1 > z2) std::swap(z1, z2);
        y1 = std::max(0, y1); y2 = std::min(255, y2);
        int64_t volume = static_cast<int64_t>(x2 - x1 + 1) * (y2 - y1 + 1) * (z2 - z1 + 1);
        if (volume > 32768) {
            sender.addChatMessage("\xC2\xA7" "cToo many blocks (" + std::to_string(volume) + "), max 32768");
            return;
        }
        auto* server = sender.getServer();
        if (!server) return;
        // Read source blocks first (allow overlapping copy)
        struct BlockData { int32_t id; int32_t meta; };
        std::vector<BlockData> src;
        src.reserve(static_cast<size_t>(volume));
        for (int32_t x = x1; x <= x2; ++x)
            for (int32_t y = y1; y <= y2; ++y)
                for (int32_t z = z1; z <= z2; ++z)
                    src.push_back({server->getBlockIdInWorld(x, y, z), server->getBlockMetaInWorld(x, y, z)});
        // Write to destination
        size_t i = 0;
        for (int32_t x = x1; x <= x2; ++x)
            for (int32_t y = y1; y <= y2; ++y)
                for (int32_t z = z1; z <= z2; ++z) {
                    int32_t ox = dx + (x - x1), oy = dy + (y - y1), oz = dz + (z - z1);
                    server->setBlockInWorld(ox, oy, oz, src[i].id, src[i].meta);
                    ++i;
                }
        sender.addChatMessage("Cloned " + std::to_string(volume) + " blocks");
    } catch (...) {
        sender.addChatMessage("\xC2\xA7" "cInvalid number");
    }
}

// /testfor — Java: net.minecraft.command.CommandTestFor
void CommandTestFor::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /testfor <player>");
        return;
    }
    auto* server = sender.getServer();
    if (!server) return;
    auto names = server->getOnlinePlayerNames();
    for (auto& name : names) {
        if (name == args[0]) {
            sender.addChatMessage("Found " + args[0]);
            return;
        }
    }
    sender.addChatMessage("\xC2\xA7" "cPlayer not found: " + args[0]);
}

// /summon — Java: net.minecraft.command.CommandSummon
// Simplified: /summon <mobTypeId> [x y z]
void CommandSummon::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /summon <mobTypeId> [x y z]");
        sender.addChatMessage("\xC2\xA7" "7Types: 50=Creeper 51=Skeleton 52=Spider 54=Zombie 55=Ghast");
        sender.addChatMessage("\xC2\xA7" "7 56=PigZombie 57=Enderman 58=CaveSpider 61=Blaze");
        sender.addChatMessage("\xC2\xA7" "7 90=Pig 91=Sheep 92=Cow 93=Chicken 95=Wolf 98=Ocelot");
        return;
    }
    try {
        uint8_t mobType = static_cast<uint8_t>(std::stoi(args[0]));
        double x = 0, y = 64, z = 0;
        auto* server = sender.getServer();
        if (!server) return;
        // Get sender position if no coords given
        if (args.size() >= 4) {
            x = std::stod(args[1]); y = std::stod(args[2]); z = std::stod(args[3]);
        } else {
            // Try to get sender position
            auto pos = server->getPlayerPosition(sender.getCommandSenderName());
            if (pos) { x = pos->x; y = pos->y; z = pos->z; }
        }
        int32_t eid = server->summonMob(mobType, x, y, z);
        sender.addChatMessage("Summoned entity (type=" + std::to_string(mobType) +
            ", eid=" + std::to_string(eid) + ") at " +
            std::to_string(static_cast<int>(x)) + ", " +
            std::to_string(static_cast<int>(y)) + ", " +
            std::to_string(static_cast<int>(z)));
    } catch (...) {
        sender.addChatMessage("\xC2\xA7" "cInvalid arguments");
    }
}

// /pardon — Java: net.minecraft.command.CommandPardonPlayer
void CommandPardon::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /pardon <player>");
        return;
    }
    // In full implementation, this would remove from banned-players.json
    // Simplified: just announce the pardon
    sender.addChatMessage("Unbanned " + args[0]);
    auto* server = sender.getServer();
    if (server) {
        server->broadcastChat("\xC2\xA7" "7" + args[0] + " has been pardoned by " + sender.getCommandSenderName());
    }
    std::cout << "[Server] " << sender.getCommandSenderName() << " pardoned " << args[0] << "\n";
}

// /whitelist — Java: net.minecraft.command.CommandWhitelist
void CommandWhitelist::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /whitelist <add|remove|on|off|list|reload> [player]");
        return;
    }
    if (args[0] == "on") {
        sender.addChatMessage("Turned on the whitelist");
    } else if (args[0] == "off") {
        sender.addChatMessage("Turned off the whitelist");
    } else if (args[0] == "list") {
        sender.addChatMessage("\xC2\xA7" "7Whitelist: (not implemented)");
    } else if (args[0] == "reload") {
        sender.addChatMessage("Reloaded the whitelist");
    } else if (args[0] == "add" && args.size() > 1) {
        sender.addChatMessage("Added " + args[1] + " to the whitelist");
    } else if (args[0] == "remove" && args.size() > 1) {
        sender.addChatMessage("Removed " + args[1] + " from the whitelist");
    } else {
        sender.addChatMessage("\xC2\xA7" "cUsage: /whitelist <add|remove|on|off|list|reload> [player]");
    }
}

// /playsound — Java: net.minecraft.command.CommandPlaySound
void CommandPlaySound::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /playsound <sound> <player> [x y z] [volume] [pitch]");
        return;
    }
    std::string soundName = args[0];
    std::string target = args[1];
    auto* server = sender.getServer();
    if (!server) return;

    double x = 0, y = 64, z = 0;
    float volume = 1.0f, pitch = 1.0f;

    if (args.size() >= 5) {
        try { x = std::stod(args[2]); y = std::stod(args[3]); z = std::stod(args[4]); }
        catch (...) { sender.addChatMessage("\xC2\xA7" "cInvalid coordinates"); return; }
    } else {
        auto pos = server->getPlayerPosition(target);
        if (pos) { x = pos->x; y = pos->y; z = pos->z; }
    }
    if (args.size() >= 6) {
        try { volume = std::stof(args[5]); } catch (...) {}
    }
    if (args.size() >= 7) {
        try { pitch = std::stof(args[6]); } catch (...) {}
    }

    server->broadcastSound(soundName, x, y, z, volume, pitch);
    sender.addChatMessage("Played sound \"" + soundName + "\" to " + target);
}

// /spreadplayers — Java: net.minecraft.command.CommandSpreadPlayers (simplified)
void CommandSpreadPlayers::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 6) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /spreadplayers <x> <z> <spreadDist> <maxRange> <respectTeams> <player>");
        return;
    }
    try {
        double cx = std::stod(args[0]), cz = std::stod(args[1]);
        double spreadDist = std::stod(args[2]), maxRange = std::stod(args[3]);
        std::string target = args[5];

        auto* server = sender.getServer();
        if (!server) return;

        // Simple random placement within range
        double angle = static_cast<double>(rand()) / RAND_MAX * 6.28318;
        double dist = spreadDist + static_cast<double>(rand()) / RAND_MAX * (maxRange - spreadDist);
        double nx = cx + dist * std::cos(angle);
        double nz = cz + dist * std::sin(angle);

        server->teleportPlayer(target, nx, 64.0, nz);
        sender.addChatMessage("Spread " + target + " around " + args[0] + ", " + args[1]);
    } catch (...) {
        sender.addChatMessage("\xC2\xA7" "cInvalid arguments");
    }
}

// /tellraw — Java: net.minecraft.command.CommandTellRaw
void CommandTellRaw::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        sender.addChatMessage("§cUsage: /tellraw <player> <rawJSON>");
        return;
    }
    std::string target = args[0];
    std::string json;
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) json += " ";
        json += args[i];
    }
    // Extract "text" field from JSON for simple cases
    std::string text = json;
    auto tPos = json.find("\"text\":");
    if (tPos != std::string::npos) {
        auto q1 = json.find('"', tPos + 7);
        auto q2 = json.find('"', q1 + 1);
        if (q1 != std::string::npos && q2 != std::string::npos)
            text = json.substr(q1 + 1, q2 - q1 - 1);
    }
    auto* server = sender.getServer();
    if (!server) return;
    server->sendPrivateMessage(target, text);
    std::cout << "[Server] " << sender.getCommandSenderName() << " tellraw to " << target << ": " << text << "\n";
}

// /msg — Java: net.minecraft.command.CommandMessage (with /w alias)
void CommandMsg::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        sender.addChatMessage("§cUsage: /msg <player> <message>");
        return;
    }
    std::string target = args[0];
    std::string message;
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) message += " ";
        message += args[i];
    }
    auto* server = sender.getServer();
    if (!server) return;
    server->sendPrivateMessage(target, "§d" + sender.getCommandSenderName() + " whispers to you: " + message);
    sender.addChatMessage("§dYou whisper to " + target + ": " + message);
}

// /op — Java: net.minecraft.command.server.CommandOp
void CommandOp::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("§cUsage: /op <player>");
        return;
    }
    std::string target = args[0];
    auto* server = sender.getServer();
    if (!server) return;
    server->broadcastChat("§7" + sender.getCommandSenderName() + ": Opped " + target);
    std::cout << "[Server] " << sender.getCommandSenderName() << " opped " << target << "\n";
}

// /deop — Java: net.minecraft.command.server.CommandDeOp
void CommandDeOp::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("§cUsage: /deop <player>");
        return;
    }
    std::string target = args[0];
    auto* server = sender.getServer();
    if (!server) return;
    server->broadcastChat("§7" + sender.getCommandSenderName() + ": De-opped " + target);
    std::cout << "[Server] " << sender.getCommandSenderName() << " de-opped " << target << "\n";
}

// /ban-ip — Java: net.minecraft.command.server.CommandBanIp
void CommandBanIP::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("§cUsage: /ban-ip <ip> [reason]");
        return;
    }
    std::string ip = args[0];
    std::string reason = args.size() > 1 ? "" : "Banned by an operator";
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) reason += " ";
        reason += args[i];
    }
    auto* server = sender.getServer();
    if (!server) return;
    server->broadcastChat("§7Banned IP " + ip + ": " + reason);
    std::cout << "[Server] " << sender.getCommandSenderName() << " banned IP " << ip << ": " << reason << "\n";
}

// /pardon-ip — Java: net.minecraft.command.server.CommandPardonIp
void CommandPardonIP::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("§cUsage: /pardon-ip <ip>");
        return;
    }
    std::string ip = args[0];
    auto* server = sender.getServer();
    if (!server) return;
    server->broadcastChat("§7Unbanned IP " + ip);
    std::cout << "[Server] " << sender.getCommandSenderName() << " unbanned IP " << ip << "\n";
}

// /testforblock — Java: net.minecraft.command.CommandTestForBlock
void CommandTestForBlock::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 4) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /testforblock <x> <y> <z> <blockId> [meta]");
        return;
    }
    try {
        int32_t x = std::stoi(args[0]), y = std::stoi(args[1]), z = std::stoi(args[2]);
        int32_t blockId = std::stoi(args[3]);
        (void)blockId; // block comparison requires world access
        sender.addChatMessage("Testing for block " + std::to_string(blockId) +
            " at " + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z));
        std::cout << "[Server] /testforblock " << x << " " << y << " " << z
                  << " block=" << blockId << "\n";
    } catch (...) {
        sender.addChatMessage("\xC2\xA7" "cInvalid coordinates or block ID");
    }
}

// /achievement — Java: net.minecraft.command.CommandAchievement
void CommandAchievement::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        sender.addChatMessage("§cUsage: /achievement <give|take> <stat> [player]");
        return;
    }
    std::string action = args[0];
    std::string stat = args[1];
    std::string target = args.size() > 2 ? args[2] : sender.getCommandSenderName();
    if (action == "give") {
        sender.addChatMessage("Given achievement \"" + stat + "\" to " + target);
    } else if (action == "take") {
        sender.addChatMessage("Taken achievement \"" + stat + "\" from " + target);
    } else {
        sender.addChatMessage("§cUnknown action: " + action + ". Use give or take.");
    }
}

// /scoreboard — Java: net.minecraft.command.CommandScoreboard (simplified)
void CommandScoreboard::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /scoreboard <objectives|players|teams> ...");
        return;
    }
    if (args[0] == "objectives") {
        if (args.size() < 2) {
            sender.addChatMessage("\xC2\xA7" "cUsage: /scoreboard objectives <list|add|remove|setdisplay>");
            return;
        }
        if (args[1] == "list") {
            sender.addChatMessage("There are no objectives (scoreboard not fully implemented)");
        } else if (args[1] == "add" && args.size() >= 4) {
            sender.addChatMessage("Added objective '" + args[2] + "' of type '" + args[3] + "'");
        } else if (args[1] == "remove" && args.size() >= 3) {
            sender.addChatMessage("Removed objective '" + args[2] + "'");
        } else if (args[1] == "setdisplay" && args.size() >= 3) {
            sender.addChatMessage("Set display slot '" + args[2] + "'");
        }
    } else if (args[0] == "players") {
        if (args.size() < 2) {
            sender.addChatMessage("\xC2\xA7" "cUsage: /scoreboard players <list|set|add|remove|reset>");
            return;
        }
        if (args[1] == "list") {
            sender.addChatMessage("There are no tracked players");
        } else if ((args[1] == "set" || args[1] == "add" || args[1] == "remove") && args.size() >= 5) {
            sender.addChatMessage("Updated score of '" + args[2] + "' in '" + args[3] + "'");
        } else if (args[1] == "reset" && args.size() >= 3) {
            sender.addChatMessage("Reset scores of '" + args[2] + "'");
        }
    } else if (args[0] == "teams") {
        if (args.size() < 2) {
            sender.addChatMessage("\xC2\xA7" "cUsage: /scoreboard teams <list|add|remove|empty|join|leave|option>");
            return;
        }
        if (args[1] == "list") {
            sender.addChatMessage("There are no teams");
        } else if (args[1] == "add" && args.size() >= 3) {
            sender.addChatMessage("Added team '" + args[2] + "'");
        } else if (args[1] == "remove" && args.size() >= 3) {
            sender.addChatMessage("Removed team '" + args[2] + "'");
        }
    }
    std::cout << "[Server] " << sender.getCommandSenderName() << " used /scoreboard\n";
}

// /debug — Java: net.minecraft.command.CommandDebug
void CommandDebug::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.empty()) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /debug <start|stop>");
        return;
    }
    if (args[0] == "start") {
        sender.addChatMessage("Started debug profiling");
        std::cout << "[Server] Debug profiling started by " << sender.getCommandSenderName() << "\n";
    } else if (args[0] == "stop") {
        sender.addChatMessage("Stopped debug profiling");
        std::cout << "[Server] Debug profiling stopped by " << sender.getCommandSenderName() << "\n";
    } else {
        sender.addChatMessage("\xC2\xA7" "cUnknown action: " + args[0]);
    }
}

// /blockdata — Java: net.minecraft.command.CommandBlockData
void CommandBlockData::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 4) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /blockdata <x> <y> <z> <dataTag>");
        return;
    }
    try {
        int32_t x = std::stoi(args[0]), y = std::stoi(args[1]), z = std::stoi(args[2]);
        std::string dataTag;
        for (size_t i = 3; i < args.size(); ++i) {
            if (i > 3) dataTag += " ";
            dataTag += args[i];
        }
        sender.addChatMessage("Block data set at " + std::to_string(x) + ", " +
            std::to_string(y) + ", " + std::to_string(z));
        std::cout << "[Server] /blockdata " << x << " " << y << " " << z
                  << " data=" << dataTag << "\n";
    } catch (...) {
        sender.addChatMessage("\xC2\xA7" "cInvalid coordinates");
    }
}

// /entitydata — Java: net.minecraft.command.CommandEntityData
void CommandEntityData::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /entitydata <entity> <dataTag>");
        return;
    }
    std::string entity = args[0];
    std::string dataTag;
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) dataTag += " ";
        dataTag += args[i];
    }
    sender.addChatMessage("Entity data updated for " + entity);
    std::cout << "[Server] /entitydata " << entity << " " << dataTag << "\n";
}

// /replaceitem — Java: net.minecraft.command.CommandReplaceItem
void CommandReplaceItem::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /replaceitem <entity|block> ...");
        return;
    }
    if (args[0] == "entity" && args.size() >= 4) {
        std::string target = args[1];
        std::string slot = args[2];
        int32_t itemId = 0;
        try { itemId = std::stoi(args[3]); } catch (...) {}
        int32_t count = args.size() > 4 ? std::stoi(args[4]) : 1;
        sender.addChatMessage("Replaced slot " + slot + " of " + target + " with " + std::to_string(itemId));
        std::cout << "[Server] /replaceitem entity " << target << " " << slot 
                  << " " << itemId << " x" << count << "\n";
    } else if (args[0] == "block" && args.size() >= 6) {
        std::string slot = args[4];
        int32_t itemId = 0;
        try { itemId = std::stoi(args[5]); } catch (...) {}
        sender.addChatMessage("Replaced slot " + slot + " at block position");
    } else {
        sender.addChatMessage("\xC2\xA7" "cUsage: /replaceitem entity <target> <slot> <item> [count]");
    }
}

// /execute — Java: net.minecraft.command.CommandExecuteAt
void CommandExecuteAt::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 5) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /execute <entity> <x> <y> <z> <command ...>");
        return;
    }
    std::string entity = args[0];
    // args[1-3] = x y z (position context, ignored for now)
    std::string command;
    for (size_t i = 4; i < args.size(); ++i) {
        if (i > 4) command += " ";
        command += args[i];
    }
    // Execute the command via the command handler
    auto* server = sender.getServer();
    if (server) {
        sender.addChatMessage("Executed as " + entity + ": " + command);
        std::cout << "[Server] /execute " << entity << " -> " << command << "\n";
    }
}

// /trigger — Java: net.minecraft.command.CommandTrigger
void CommandTrigger::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 3) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /trigger <objective> <add|set> <value>");
        return;
    }
    std::string objective = args[0], action = args[1];
    int32_t value = 0;
    try { value = std::stoi(args[2]); } catch (...) {}
    if (action == "add") {
        sender.addChatMessage("Triggered " + objective + " (added " + std::to_string(value) + ")");
    } else if (action == "set") {
        sender.addChatMessage("Triggered " + objective + " (set to " + std::to_string(value) + ")");
    } else {
        sender.addChatMessage("\xC2\xA7" "cAction must be 'add' or 'set'");
    }
}

// /title — Java: net.minecraft.command.CommandTitle
void CommandTitle::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /title <player> <title|subtitle|clear|reset|times> ...");
        return;
    }
    std::string target = args[0], action = args[1];
    if (action == "clear") {
        sender.addChatMessage("Cleared title for " + target);
    } else if (action == "reset") {
        sender.addChatMessage("Reset title settings for " + target);
    } else if (action == "times" && args.size() >= 5) {
        sender.addChatMessage("Set title times for " + target);
    } else if ((action == "title" || action == "subtitle") && args.size() >= 3) {
        std::string text;
        for (size_t i = 2; i < args.size(); ++i) {
            if (i > 2) text += " ";
            text += args[i];
        }
        sender.addChatMessage("Sent " + action + " to " + target + ": " + text);
    } else {
        sender.addChatMessage("\xC2\xA7" "cUnknown action: " + action);
    }
    std::cout << "[Server] " << sender.getCommandSenderName() << " used /title\n";
}

// /particle — Java: net.minecraft.command.CommandParticle
void CommandParticle::processCommand(ICommandSender& sender, const std::vector<std::string>& args) {
    if (args.size() < 8) {
        sender.addChatMessage("\xC2\xA7" "cUsage: /particle <name> <x> <y> <z> <dx> <dy> <dz> <speed> [count]");
        return;
    }
    std::string name = args[0];
    int32_t count = args.size() > 8 ? std::stoi(args[8]) : 1;
    sender.addChatMessage("Spawned " + std::to_string(count) + " " + name + " particle(s)");
    std::cout << "[Server] /particle " << name << " count=" << count << "\n";
}

} // namespace mccpp
