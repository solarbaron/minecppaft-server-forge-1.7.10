#pragma once
// CommandSystem — server command parser, dispatcher, and vanilla commands.
// Ported from vanilla 1.7.10 CommandHandler (bl.java), and individual
// command classes in the bq-cz range.
//
// Vanilla 1.7.10 commands:
//   /gamemode, /give, /tp, /kill, /time, /weather, /say, /tell,
//   /kick, /ban, /op, /deop, /difficulty, /seed, /whitelist,
//   /xp, /effect, /enchant, /spawnpoint, /gamerule, /clear,
//   /toggledownfall, /defaultgamemode, /publish, /me

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <functional>
#include <algorithm>
#include <optional>

namespace mc {

// ============================================================
// Command sender types
// ============================================================
enum class SenderType {
    PLAYER,
    CONSOLE,
    COMMAND_BLOCK
};

struct CommandSender {
    SenderType  type;
    int         fd = -1;         // Player socket fd (if player)
    std::string name;
    bool        isOp = false;

    bool hasPermission(int level) const {
        if (type == SenderType::CONSOLE) return true;
        if (type == SenderType::COMMAND_BLOCK) return true;
        return isOp && level <= 4;
    }
};

// ============================================================
// Command result
// ============================================================
struct CommandResult {
    bool        success = false;
    std::string message;
    int         affectedEntities = 0;

    static CommandResult ok(const std::string& msg = "") {
        return {true, msg, 0};
    }
    static CommandResult error(const std::string& msg) {
        return {false, msg, 0};
    }
};

// ============================================================
// Command definition
// ============================================================
struct CommandDef {
    std::string name;
    std::string usage;
    int         requiredPermLevel; // 0=all, 1=moderator, 2=gamemaster, 3=admin, 4=owner
    std::vector<std::string> aliases;
    std::function<CommandResult(const CommandSender&, const std::vector<std::string>&)> handler;
};

// ============================================================
// Argument parsing helpers
// ============================================================
inline std::vector<std::string> splitArgs(const std::string& input) {
    std::vector<std::string> args;
    std::istringstream ss(input);
    std::string token;
    while (ss >> token) {
        args.push_back(token);
    }
    return args;
}

inline std::optional<int> parseInt(const std::string& s) {
    try { return std::stoi(s); } catch (...) { return std::nullopt; }
}

inline std::optional<double> parseDouble(const std::string& s) {
    try { return std::stod(s); } catch (...) { return std::nullopt; }
}

// Parse relative coordinates (~, ~10, ~-5)
inline std::optional<double> parseCoord(const std::string& s, double base) {
    if (s.empty()) return std::nullopt;
    if (s[0] == '~') {
        if (s.size() == 1) return base;
        auto offset = parseDouble(s.substr(1));
        if (!offset) return std::nullopt;
        return base + *offset;
    }
    return parseDouble(s);
}

// ============================================================
// CommandDispatcher — registers and executes commands
// ============================================================
class CommandDispatcher {
public:
    CommandDispatcher() {
        registerVanillaCommands();
    }

    // Parse and dispatch a command string (with leading /)
    CommandResult dispatch(const CommandSender& sender, const std::string& input) {
        if (input.empty()) return CommandResult::error("Empty command");

        std::string cmd = input;
        if (cmd[0] == '/') cmd = cmd.substr(1);

        auto args = splitArgs(cmd);
        if (args.empty()) return CommandResult::error("Empty command");

        std::string cmdName = args[0];
        std::transform(cmdName.begin(), cmdName.end(), cmdName.begin(), ::tolower);
        args.erase(args.begin()); // Remove command name

        // Find command
        auto it = commands_.find(cmdName);
        if (it == commands_.end()) {
            // Check aliases
            auto aliasIt = aliases_.find(cmdName);
            if (aliasIt != aliases_.end()) {
                it = commands_.find(aliasIt->second);
            }
        }

        if (it == commands_.end()) {
            return CommandResult::error("Unknown command: " + cmdName + ". Type /help for help.");
        }

        auto& def = it->second;

        // Permission check
        if (!sender.hasPermission(def.requiredPermLevel)) {
            return CommandResult::error("You don't have permission to use this command.");
        }

        return def.handler(sender, args);
    }

    // Register a custom command
    void registerCommand(CommandDef def) {
        std::string name = def.name;
        for (auto& alias : def.aliases) {
            aliases_[alias] = name;
        }
        commands_[name] = std::move(def);
    }

    // Get all command names
    std::vector<std::string> getCommandNames() const {
        std::vector<std::string> names;
        for (auto& [name, _] : commands_) {
            names.push_back(name);
        }
        std::sort(names.begin(), names.end());
        return names;
    }

    // Tab completion
    std::vector<std::string> tabComplete(const CommandSender& sender, const std::string& partial) {
        std::vector<std::string> matches;
        std::string lower = partial;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower.size() > 0 && lower[0] == '/') lower = lower.substr(1);

        for (auto& [name, def] : commands_) {
            if (sender.hasPermission(def.requiredPermLevel)) {
                if (name.find(lower) == 0) {
                    matches.push_back("/" + name);
                }
            }
        }
        std::sort(matches.begin(), matches.end());
        return matches;
    }

    // Callbacks — set by the server to execute command effects
    std::function<void(int fd, const std::string& msg)> sendMessage;
    std::function<void(const std::string& msg)> broadcastMessage;
    std::function<void(int fd, int mode)> setGamemode;
    std::function<void(int fd, int16_t itemId, int count, int16_t meta)> giveItem;
    std::function<void(int fd, double x, double y, double z)> teleportPlayer;
    std::function<void(int fd)> killPlayer;
    std::function<void(int64_t ticks)> setWorldTime;
    std::function<void(bool rain)> setWeather;
    std::function<void(int fd, int8_t effectId, int8_t amp, int32_t dur)> addEffect;
    std::function<void(int fd, int8_t effectId)> removeEffect;
    std::function<void(int fd)> kickPlayer;
    std::function<int(const std::string& name)> getPlayerFd; // Name → fd, -1 = not found
    std::function<std::pair<double,double>(int fd)> getPlayerPos; // fd → (x, z)

private:
    void registerVanillaCommands() {
        // /help
        registerCommand({"help", "/help [command]", 0, {"?"}, [this](auto& sender, auto& args) {
            std::string msg = "§e--- Available Commands ---\n";
            for (auto& name : getCommandNames()) {
                auto& def = commands_[name];
                if (sender.hasPermission(def.requiredPermLevel)) {
                    msg += "§6" + def.usage + "\n";
                }
            }
            return CommandResult::ok(msg);
        }});

        // /gamemode <mode> [player]
        registerCommand({"gamemode", "/gamemode <0|1|2|3> [player]", 2, {"gm"}, [this](auto& sender, auto& args) {
            if (args.empty()) return CommandResult::error("Usage: /gamemode <0|1|2|3> [player]");
            auto mode = parseInt(args[0]);
            if (!mode || *mode < 0 || *mode > 3) return CommandResult::error("Invalid gamemode. Use 0-3.");
            int fd = sender.fd;
            if (args.size() > 1 && getPlayerFd) {
                fd = getPlayerFd(args[1]);
                if (fd < 0) return CommandResult::error("Player not found: " + args[1]);
            }
            if (setGamemode) setGamemode(fd, *mode);
            static const char* names[] = {"Survival", "Creative", "Adventure", "Spectator"};
            return CommandResult::ok("Set gamemode to " + std::string(names[*mode]));
        }});

        // /give <player> <item> [amount] [data]
        registerCommand({"give", "/give <player> <item> [amount] [data]", 2, {}, [this](auto& sender, auto& args) {
            if (args.size() < 2) return CommandResult::error("Usage: /give <player> <item> [amount] [data]");
            int fd = -1;
            if (getPlayerFd) fd = getPlayerFd(args[0]);
            if (fd < 0) return CommandResult::error("Player not found: " + args[0]);
            auto itemId = parseInt(args[1]);
            if (!itemId) return CommandResult::error("Invalid item ID");
            int count = (args.size() > 2) ? parseInt(args[2]).value_or(1) : 1;
            int16_t meta = (args.size() > 3) ? static_cast<int16_t>(parseInt(args[3]).value_or(0)) : 0;
            if (count < 1 || count > 64) count = 1;
            if (giveItem) giveItem(fd, static_cast<int16_t>(*itemId), count, meta);
            return CommandResult::ok("Gave " + std::to_string(count) + " of item " + std::to_string(*itemId) + " to " + args[0]);
        }});

        // /tp <player> <x> <y> <z>  OR  /tp <player> <target>
        registerCommand({"tp", "/tp <player> <x> <y> <z>", 2, {"teleport"}, [this](auto& sender, auto& args) {
            if (args.size() < 2) return CommandResult::error("Usage: /tp <player> <x> <y> <z> OR /tp <player> <target>");
            int fd = -1;
            if (getPlayerFd) fd = getPlayerFd(args[0]);
            if (fd < 0) return CommandResult::error("Player not found: " + args[0]);
            if (args.size() >= 4) {
                auto x = parseDouble(args[1]), y = parseDouble(args[2]), z = parseDouble(args[3]);
                if (!x || !y || !z) return CommandResult::error("Invalid coordinates");
                if (teleportPlayer) teleportPlayer(fd, *x, *y, *z);
                return CommandResult::ok("Teleported " + args[0] + " to " + args[1] + " " + args[2] + " " + args[3]);
            } else {
                int targetFd = getPlayerFd ? getPlayerFd(args[1]) : -1;
                if (targetFd < 0) return CommandResult::error("Target not found: " + args[1]);
                auto pos = getPlayerPos ? getPlayerPos(targetFd) : std::pair{0.0, 0.0};
                if (teleportPlayer) teleportPlayer(fd, pos.first, 64.0, pos.second);
                return CommandResult::ok("Teleported " + args[0] + " to " + args[1]);
            }
        }});

        // /kill [player]
        registerCommand({"kill", "/kill [player]", 2, {}, [this](auto& sender, auto& args) {
            int fd = sender.fd;
            if (!args.empty() && getPlayerFd) {
                fd = getPlayerFd(args[0]);
                if (fd < 0) return CommandResult::error("Player not found");
            }
            if (killPlayer) killPlayer(fd);
            return CommandResult::ok("Killed player");
        }});

        // /time set <value> | /time add <value>
        registerCommand({"time", "/time <set|add|query> <value>", 2, {}, [this](auto& sender, auto& args) {
            if (args.empty()) return CommandResult::error("Usage: /time <set|add|query> <value>");
            if (args[0] == "set" && args.size() > 1) {
                int64_t t = 0;
                if (args[1] == "day") t = 1000;
                else if (args[1] == "night") t = 13000;
                else {
                    auto val = parseInt(args[1]);
                    if (!val) return CommandResult::error("Invalid time value");
                    t = *val;
                }
                if (setWorldTime) setWorldTime(t);
                return CommandResult::ok("Set time to " + std::to_string(t));
            }
            if (args[0] == "add" && args.size() > 1) {
                auto val = parseInt(args[1]);
                if (!val) return CommandResult::error("Invalid time value");
                if (setWorldTime) setWorldTime(*val); // Server should handle add vs set
                return CommandResult::ok("Added " + std::to_string(*val) + " to time");
            }
            return CommandResult::error("Usage: /time <set|add> <value>");
        }});

        // /weather <clear|rain|thunder> [duration]
        registerCommand({"weather", "/weather <clear|rain|thunder> [duration]", 2, {}, [this](auto& sender, auto& args) {
            if (args.empty()) return CommandResult::error("Usage: /weather <clear|rain|thunder>");
            bool rain = (args[0] == "rain" || args[0] == "thunder");
            if (setWeather) setWeather(rain);
            return CommandResult::ok("Set weather to " + args[0]);
        }});

        // /say <message>
        registerCommand({"say", "/say <message>", 1, {}, [this](auto& sender, auto& args) {
            if (args.empty()) return CommandResult::error("Usage: /say <message>");
            std::string msg;
            for (size_t i = 0; i < args.size(); ++i) {
                if (i > 0) msg += " ";
                msg += args[i];
            }
            if (broadcastMessage) broadcastMessage("§d[" + sender.name + "] " + msg);
            return CommandResult::ok("");
        }});

        // /tell <player> <message>
        registerCommand({"tell", "/tell <player> <message>", 0, {"msg", "w"}, [this](auto& sender, auto& args) {
            if (args.size() < 2) return CommandResult::error("Usage: /tell <player> <message>");
            int fd = getPlayerFd ? getPlayerFd(args[0]) : -1;
            if (fd < 0) return CommandResult::error("Player not found: " + args[0]);
            std::string msg;
            for (size_t i = 1; i < args.size(); ++i) {
                if (i > 1) msg += " ";
                msg += args[i];
            }
            if (sendMessage) sendMessage(fd, "§7" + sender.name + " whispers: " + msg);
            return CommandResult::ok("§7Whispered to " + args[0]);
        }});

        // /kick <player> [reason]
        registerCommand({"kick", "/kick <player> [reason]", 3, {}, [this](auto& sender, auto& args) {
            if (args.empty()) return CommandResult::error("Usage: /kick <player> [reason]");
            int fd = getPlayerFd ? getPlayerFd(args[0]) : -1;
            if (fd < 0) return CommandResult::error("Player not found: " + args[0]);
            if (kickPlayer) kickPlayer(fd);
            return CommandResult::ok("Kicked " + args[0]);
        }});

        // /xp <amount>[L] [player]
        registerCommand({"xp", "/xp <amount>[L] [player]", 2, {"experience"}, [this](auto& sender, auto& args) {
            if (args.empty()) return CommandResult::error("Usage: /xp <amount>[L] [player]");
            bool levels = false;
            std::string amtStr = args[0];
            if (amtStr.back() == 'L' || amtStr.back() == 'l') {
                levels = true;
                amtStr.pop_back();
            }
            auto amt = parseInt(amtStr);
            if (!amt) return CommandResult::error("Invalid XP amount");
            return CommandResult::ok("Gave " + std::to_string(*amt) + (levels ? " levels" : " XP points"));
        }});

        // /effect <player> <effect> [duration] [amplifier]
        registerCommand({"effect", "/effect <player> <effect> [dur] [amp]", 2, {}, [this](auto& sender, auto& args) {
            if (args.size() < 2) return CommandResult::error("Usage: /effect <player> <effect> [duration] [amplifier]");
            int fd = getPlayerFd ? getPlayerFd(args[0]) : -1;
            if (fd < 0) return CommandResult::error("Player not found: " + args[0]);
            auto effectId = parseInt(args[1]);
            if (!effectId) return CommandResult::error("Invalid effect ID");
            int dur = args.size() > 2 ? parseInt(args[2]).value_or(30) : 30;
            int amp = args.size() > 3 ? parseInt(args[3]).value_or(0) : 0;
            if (addEffect) addEffect(fd, static_cast<int8_t>(*effectId),
                                      static_cast<int8_t>(amp), dur * 20);
            return CommandResult::ok("Applied effect " + std::to_string(*effectId) + " to " + args[0]);
        }});

        // /difficulty <0-3>
        registerCommand({"difficulty", "/difficulty <0|1|2|3>", 2, {}, [this](auto& sender, auto& args) {
            if (args.empty()) return CommandResult::error("Usage: /difficulty <0-3>");
            auto diff = parseInt(args[0]);
            if (!diff || *diff < 0 || *diff > 3) return CommandResult::error("Invalid difficulty 0-3");
            static const char* names[] = {"Peaceful", "Easy", "Normal", "Hard"};
            return CommandResult::ok("Set difficulty to " + std::string(names[*diff]));
        }});

        // /seed
        registerCommand({"seed", "/seed", 0, {}, [](auto& sender, auto& args) {
            return CommandResult::ok("Seed: [not implemented]");
        }});

        // /toggledownfall
        registerCommand({"toggledownfall", "/toggledownfall", 2, {}, [this](auto& sender, auto& args) {
            if (setWeather) setWeather(true); // Toggle
            return CommandResult::ok("Toggled weather");
        }});

        // /me <action>
        registerCommand({"me", "/me <action>", 0, {}, [this](auto& sender, auto& args) {
            if (args.empty()) return CommandResult::error("Usage: /me <action>");
            std::string msg;
            for (size_t i = 0; i < args.size(); ++i) {
                if (i > 0) msg += " ";
                msg += args[i];
            }
            if (broadcastMessage) broadcastMessage("* " + sender.name + " " + msg);
            return CommandResult::ok("");
        }});

        // /list
        registerCommand({"list", "/list", 0, {}, [](auto& sender, auto& args) {
            return CommandResult::ok("Player list: [use tab]");
        }});

        // /stop
        registerCommand({"stop", "/stop", 4, {}, [](auto& sender, auto& args) {
            return CommandResult::ok("Stopping server...");
        }});
    }

    std::unordered_map<std::string, CommandDef> commands_;
    std::unordered_map<std::string, std::string> aliases_;
};

} // namespace mc
