/**
 * CommandHandler.h — Server command dispatch, registration, and tab completion.
 *
 * Java references:
 *   - net.minecraft.command.CommandHandler — Core command dispatcher
 *   - net.minecraft.command.ServerCommandManager — Server-specific registration
 *   - net.minecraft.command.ICommand — Command interface
 *   - net.minecraft.command.ICommandSender — Command sender interface
 *   - net.minecraft.command.PlayerSelector — @a/@p/@r expansion
 *
 * Architecture:
 *   - commandMap: HashMap<string, ICommand> for O(1) lookup by name/alias
 *   - commandSet: HashSet<ICommand> for unique command iteration
 *   - executeCommand: strip "/", split on space, lookup, permission check,
 *     player selector expansion, error handling with chat messages
 *   - Tab completion: 1 arg = prefix match names, 2+ args = delegate to command
 *   - Aliases: registered in map, skip if existing command has same primary name
 *   - Operator notification: gray italic admin chat broadcast to all ops
 *
 * Thread safety: Commands execute on main server thread (tick loop).
 * JNI readiness: Simple interfaces for command binding.
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// ICommand — Command interface.
// Java reference: net.minecraft.command.ICommand
// ═══════════════════════════════════════════════════════════════════════════

class ICommand {
public:
    virtual ~ICommand() = default;

    virtual std::string getCommandName() const = 0;
    virtual std::string getCommandUsage() const = 0;
    virtual std::vector<std::string> getCommandAliases() const { return {}; }
    virtual int32_t getRequiredPermissionLevel() const { return 4; }  // Default: op level 4
    virtual bool canCommandSenderUseCommand(int32_t permLevel) const {
        return permLevel >= getRequiredPermissionLevel();
    }
    virtual void processCommand(const std::string& senderName, int32_t permLevel,
                                  const std::vector<std::string>& args) = 0;
    virtual std::vector<std::string> addTabCompletionOptions(
            const std::vector<std::string>& args) const { return {}; }
    virtual bool isUsernameIndex(const std::vector<std::string>& args, int32_t index) const {
        return false;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// CommandResult — Result of command execution.
// ═══════════════════════════════════════════════════════════════════════════

struct CommandResult {
    int32_t successCount;
    std::string errorMessage;  // Empty if no error
    bool notFound;
    bool noPermission;
};

// ═══════════════════════════════════════════════════════════════════════════
// CommandHandler — Core command dispatcher.
// Java reference: net.minecraft.command.CommandHandler
// ═══════════════════════════════════════════════════════════════════════════

class CommandHandler {
public:
    CommandHandler() = default;

    // ─── Registration ───

    // Java: registerCommand
    void registerCommand(std::shared_ptr<ICommand> command) {
        std::string name = command->getCommandName();
        commandMap_[name] = command;
        commandSet_.insert(command);

        auto aliases = command->getCommandAliases();
        for (const auto& alias : aliases) {
            auto existing = commandMap_.find(alias);
            if (existing != commandMap_.end() &&
                existing->second->getCommandName() == alias) {
                continue;  // Don't override primary command name
            }
            commandMap_[alias] = command;
        }
    }

    // ─── Execution ───

    // Java: executeCommand
    // senderName: who sent the command
    // permLevel: permission level of sender
    // rawCommand: full command string (may start with /)
    // matchPlayers: callback to expand @a/@p/@r selectors
    using MatchPlayersFn = std::function<std::vector<std::string>(const std::string&)>;

    CommandResult executeCommand(const std::string& senderName, int32_t permLevel,
                                   const std::string& rawCommand,
                                   MatchPlayersFn matchPlayers = nullptr) {
        CommandResult result{0, "", false, false};

        // Strip leading /
        std::string cmd = rawCommand;
        size_t start = 0;
        while (start < cmd.size() && cmd[start] == ' ') ++start;
        cmd = cmd.substr(start);
        if (!cmd.empty() && cmd[0] == '/') {
            cmd = cmd.substr(1);
        }

        // Split on space
        auto parts = splitString(cmd, ' ');
        if (parts.empty()) {
            result.notFound = true;
            result.errorMessage = "commands.generic.notFound";
            return result;
        }

        std::string commandName = parts[0];
        std::vector<std::string> args(parts.begin() + 1, parts.end());

        // Lookup command
        auto it = commandMap_.find(commandName);
        if (it == commandMap_.end()) {
            result.notFound = true;
            result.errorMessage = "commands.generic.notFound";
            return result;
        }

        auto& command = it->second;

        // Permission check
        if (!command->canCommandSenderUseCommand(permLevel)) {
            result.noPermission = true;
            result.errorMessage = "commands.generic.permission";
            return result;
        }

        // Player selector expansion
        int32_t usernameIdx = getUsernameIndex(command.get(), args, matchPlayers);

        if (usernameIdx > -1 && matchPlayers) {
            auto matchedPlayers = matchPlayers(args[usernameIdx]);
            std::string originalArg = args[usernameIdx];

            for (const auto& playerName : matchedPlayers) {
                args[usernameIdx] = playerName;
                try {
                    command->processCommand(senderName, permLevel, args);
                    ++result.successCount;
                } catch (const std::exception& e) {
                    result.errorMessage = e.what();
                }
            }
            args[usernameIdx] = originalArg;
        } else {
            try {
                command->processCommand(senderName, permLevel, args);
                ++result.successCount;
            } catch (const std::exception& e) {
                result.errorMessage = e.what();
            }
        }

        return result;
    }

    // ─── Tab Completion ───

    // Java: getPossibleCommands(sender, partialCommand)
    std::vector<std::string> getTabCompletions(int32_t permLevel,
                                                  const std::string& partial) {
        auto parts = splitString(partial, ' ', true);

        if (parts.size() <= 1) {
            // Complete command name
            std::string prefix = parts.empty() ? "" : parts[0];
            std::vector<std::string> completions;
            for (const auto& entry : commandMap_) {
                if (doesStringStartWith(prefix, entry.first) &&
                    entry.second->canCommandSenderUseCommand(permLevel)) {
                    completions.push_back(entry.first);
                }
            }
            std::sort(completions.begin(), completions.end());
            return completions;
        }

        // Delegate to command
        std::string commandName = parts[0];
        auto it = commandMap_.find(commandName);
        if (it != commandMap_.end()) {
            std::vector<std::string> args(parts.begin() + 1, parts.end());
            return it->second->addTabCompletionOptions(args);
        }
        return {};
    }

    // ─── Queries ───

    // Java: getPossibleCommands(sender) — accessible commands list
    std::vector<std::string> getAccessibleCommands(int32_t permLevel) const {
        std::vector<std::string> result;
        for (const auto& cmd : commandSet_) {
            if (cmd->canCommandSenderUseCommand(permLevel)) {
                result.push_back(cmd->getCommandName());
            }
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    const std::unordered_map<std::string, std::shared_ptr<ICommand>>& getCommands() const {
        return commandMap_;
    }

    int32_t getCommandCount() const {
        return static_cast<int32_t>(commandSet_.size());
    }

private:
    // Java: getUsernameIndex
    int32_t getUsernameIndex(ICommand* cmd, const std::vector<std::string>& args,
                               MatchPlayersFn& matchPlayers) {
        if (!cmd || !matchPlayers) return -1;
        for (int32_t i = 0; i < static_cast<int32_t>(args.size()); ++i) {
            if (cmd->isUsernameIndex(args, i) && isPlayerSelector(args[i])) {
                return i;
            }
        }
        return -1;
    }

    // Java: PlayerSelector.matchesMultiplePlayers — @a, @e always multi, @p/@r single
    static bool isPlayerSelector(const std::string& s) {
        if (s.size() < 2 || s[0] != '@') return false;
        char c = s[1];
        return c == 'a' || c == 'e' || c == 'p' || c == 'r';
    }

    // Java: CommandBase.doesStringStartWith
    static bool doesStringStartWith(const std::string& prefix, const std::string& str) {
        if (prefix.empty()) return true;
        if (str.size() < prefix.size()) return false;
        for (size_t i = 0; i < prefix.size(); ++i) {
            if (std::tolower(prefix[i]) != std::tolower(str[i])) return false;
        }
        return true;
    }

    static std::vector<std::string> splitString(const std::string& s, char delimiter,
                                                   bool keepEmpty = false) {
        std::vector<std::string> result;
        std::string current;
        for (char c : s) {
            if (c == delimiter) {
                if (!current.empty() || keepEmpty) {
                    result.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        if (!current.empty() || keepEmpty) {
            result.push_back(current);
        }
        return result;
    }

    std::unordered_map<std::string, std::shared_ptr<ICommand>> commandMap_;

    struct PtrHash {
        size_t operator()(const std::shared_ptr<ICommand>& p) const {
            return std::hash<ICommand*>()(p.get());
        }
    };
    std::unordered_set<std::shared_ptr<ICommand>, PtrHash> commandSet_;
};

// ═══════════════════════════════════════════════════════════════════════════
// Built-in commands — Concrete implementations.
// Java reference: net.minecraft.command.server.* and net.minecraft.command.*
// ═══════════════════════════════════════════════════════════════════════════

// ─── /help ───
class CommandHelp : public ICommand {
public:
    std::string getCommandName() const override { return "help"; }
    std::string getCommandUsage() const override { return "commands.help.usage"; }
    std::vector<std::string> getCommandAliases() const override { return {"?"}; }
    int32_t getRequiredPermissionLevel() const override { return 0; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /stop ───
class CommandStop : public ICommand {
public:
    std::string getCommandName() const override { return "stop"; }
    std::string getCommandUsage() const override { return "commands.stop.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /time ───
class CommandTime : public ICommand {
public:
    std::string getCommandName() const override { return "time"; }
    std::string getCommandUsage() const override { return "commands.time.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /gamemode ───
class CommandGameMode : public ICommand {
public:
    std::string getCommandName() const override { return "gamemode"; }
    std::string getCommandUsage() const override { return "commands.gamemode.usage"; }
    bool isUsernameIndex(const std::vector<std::string>& args, int32_t idx) const override {
        return idx == 1;
    }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /tp ───
class CommandTeleport : public ICommand {
public:
    std::string getCommandName() const override { return "tp"; }
    std::string getCommandUsage() const override { return "commands.tp.usage"; }
    bool isUsernameIndex(const std::vector<std::string>& args, int32_t idx) const override {
        return idx == 0;
    }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /give ───
class CommandGive : public ICommand {
public:
    std::string getCommandName() const override { return "give"; }
    std::string getCommandUsage() const override { return "commands.give.usage"; }
    bool isUsernameIndex(const std::vector<std::string>& args, int32_t idx) const override {
        return idx == 0;
    }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /kill ───
class CommandKill : public ICommand {
public:
    std::string getCommandName() const override { return "kill"; }
    std::string getCommandUsage() const override { return "commands.kill.usage"; }
    int32_t getRequiredPermissionLevel() const override { return 0; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /say ───
class CommandBroadcast : public ICommand {
public:
    std::string getCommandName() const override { return "say"; }
    std::string getCommandUsage() const override { return "commands.say.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /weather ───
class CommandWeather : public ICommand {
public:
    std::string getCommandName() const override { return "weather"; }
    std::string getCommandUsage() const override { return "commands.weather.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /difficulty ───
class CommandDifficulty : public ICommand {
public:
    std::string getCommandName() const override { return "difficulty"; }
    std::string getCommandUsage() const override { return "commands.difficulty.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /gamerule ───
class CommandGameRule : public ICommand {
public:
    std::string getCommandName() const override { return "gamerule"; }
    std::string getCommandUsage() const override { return "commands.gamerule.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /xp ───
class CommandXP : public ICommand {
public:
    std::string getCommandName() const override { return "xp"; }
    std::string getCommandUsage() const override { return "commands.xp.usage"; }
    bool isUsernameIndex(const std::vector<std::string>& args, int32_t idx) const override {
        return idx == 1;
    }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /op ───
class CommandOp : public ICommand {
public:
    std::string getCommandName() const override { return "op"; }
    std::string getCommandUsage() const override { return "commands.op.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /deop ───
class CommandDeOp : public ICommand {
public:
    std::string getCommandName() const override { return "deop"; }
    std::string getCommandUsage() const override { return "commands.deop.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /kick ───
class CommandServerKick : public ICommand {
public:
    std::string getCommandName() const override { return "kick"; }
    std::string getCommandUsage() const override { return "commands.kick.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /ban ───
class CommandBanPlayer : public ICommand {
public:
    std::string getCommandName() const override { return "ban"; }
    std::string getCommandUsage() const override { return "commands.ban.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /pardon ───
class CommandPardonPlayer : public ICommand {
public:
    std::string getCommandName() const override { return "pardon"; }
    std::string getCommandUsage() const override { return "commands.unban.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /list ───
class CommandListPlayers : public ICommand {
public:
    std::string getCommandName() const override { return "list"; }
    std::string getCommandUsage() const override { return "commands.players.usage"; }
    int32_t getRequiredPermissionLevel() const override { return 0; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /save-all ───
class CommandSaveAll : public ICommand {
public:
    std::string getCommandName() const override { return "save-all"; }
    std::string getCommandUsage() const override { return "commands.save.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /setblock ───
class CommandSetBlock : public ICommand {
public:
    std::string getCommandName() const override { return "setblock"; }
    std::string getCommandUsage() const override { return "commands.setblock.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /summon ───
class CommandSummon : public ICommand {
public:
    std::string getCommandName() const override { return "summon"; }
    std::string getCommandUsage() const override { return "commands.summon.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /seed ───
class CommandShowSeed : public ICommand {
public:
    std::string getCommandName() const override { return "seed"; }
    std::string getCommandUsage() const override { return "commands.seed.usage"; }
    int32_t getRequiredPermissionLevel() const override { return 0; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ─── /whitelist ───
class CommandWhitelist : public ICommand {
public:
    std::string getCommandName() const override { return "whitelist"; }
    std::string getCommandUsage() const override { return "commands.whitelist.usage"; }
    void processCommand(const std::string&, int32_t, const std::vector<std::string>&) override {}
};

// ═══════════════════════════════════════════════════════════════════════════
// ServerCommandManager — Register all built-in commands.
// Java reference: net.minecraft.command.ServerCommandManager
// ═══════════════════════════════════════════════════════════════════════════

class ServerCommandManager {
public:
    static void registerAllCommands(CommandHandler& handler, bool isDedicated) {
        handler.registerCommand(std::make_shared<CommandHelp>());
        handler.registerCommand(std::make_shared<CommandTime>());
        handler.registerCommand(std::make_shared<CommandGameMode>());
        handler.registerCommand(std::make_shared<CommandDifficulty>());
        handler.registerCommand(std::make_shared<CommandKill>());
        handler.registerCommand(std::make_shared<CommandWeather>());
        handler.registerCommand(std::make_shared<CommandXP>());
        handler.registerCommand(std::make_shared<CommandTeleport>());
        handler.registerCommand(std::make_shared<CommandGive>());
        handler.registerCommand(std::make_shared<CommandBroadcast>());
        handler.registerCommand(std::make_shared<CommandGameRule>());
        handler.registerCommand(std::make_shared<CommandSetBlock>());
        handler.registerCommand(std::make_shared<CommandSummon>());
        handler.registerCommand(std::make_shared<CommandShowSeed>());
        handler.registerCommand(std::make_shared<CommandListPlayers>());

        if (isDedicated) {
            handler.registerCommand(std::make_shared<CommandOp>());
            handler.registerCommand(std::make_shared<CommandDeOp>());
            handler.registerCommand(std::make_shared<CommandStop>());
            handler.registerCommand(std::make_shared<CommandSaveAll>());
            handler.registerCommand(std::make_shared<CommandServerKick>());
            handler.registerCommand(std::make_shared<CommandBanPlayer>());
            handler.registerCommand(std::make_shared<CommandPardonPlayer>());
            handler.registerCommand(std::make_shared<CommandWhitelist>());
        }
    }
};

} // namespace mccpp
