/**
 * CommandSystem.h — Server command system.
 *
 * Java references:
 *   - net.minecraft.command.ICommandSender
 *   - net.minecraft.command.ICommand
 *   - net.minecraft.command.CommandBase
 *   - net.minecraft.command.CommandHandler
 *   - net.minecraft.command.server.CommandStop
 *   - net.minecraft.command.server.CommandSay
 *   - net.minecraft.command.CommandHelp
 *   - net.minecraft.command.CommandGameMode
 *   - net.minecraft.command.CommandTime
 *   - net.minecraft.command.CommandGive
 *   - net.minecraft.command.CommandTeleport
 *   - net.minecraft.command.CommandGameRule
 *
 * Thread safety:
 *   - CommandHandler is accessed from multiple threads (console + network).
 *   - Command registration happens at startup (single-threaded).
 *   - Command dispatch uses a shared_mutex for read-heavy access pattern.
 *
 * JNI readiness: String-based command names and aliases for easy JVM mapping.
 */
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// ICommandSender — Entity capable of receiving command output.
// Java reference: net.minecraft.command.ICommandSender
// ═══════════════════════════════════════════════════════════════════════════

class ICommandSender {
public:
    virtual ~ICommandSender() = default;

    // Java: ICommandSender.getCommandSenderName()
    virtual std::string getCommandSenderName() const = 0;

    // Java: ICommandSender.addChatMessage(IChatComponent)
    virtual void addChatMessage(const std::string& message) = 0;

    // Java: ICommandSender.canCommandSenderUseCommand(int, String)
    // Permission level: 0=all, 1=moderator, 2=gamemaster, 3=admin, 4=owner
    virtual bool canCommandSenderUseCommand(int32_t permLevel, const std::string& command) const = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// ICommand — Interface for all server commands.
// Java reference: net.minecraft.command.ICommand
// ═══════════════════════════════════════════════════════════════════════════

class ICommand {
public:
    virtual ~ICommand() = default;

    // Java: ICommand.getCommandName()
    virtual std::string getCommandName() const = 0;

    // Java: ICommand.getCommandUsage(ICommandSender)
    virtual std::string getCommandUsage() const = 0;

    // Java: ICommand.getCommandAliases()
    virtual std::vector<std::string> getCommandAliases() const { return {}; }

    // Java: ICommand.processCommand(ICommandSender, String[])
    virtual void processCommand(ICommandSender& sender, const std::vector<std::string>& args) = 0;

    // Java: ICommand.canCommandSenderUseCommand(ICommandSender)
    virtual bool canCommandSenderUseCommand(const ICommandSender& sender) const {
        return sender.canCommandSenderUseCommand(getRequiredPermissionLevel(), getCommandName());
    }

    // Java: ICommand.addTabCompletionOptions(ICommandSender, String[])
    virtual std::vector<std::string> addTabCompletionOptions(
        const ICommandSender& sender, const std::vector<std::string>& args) const { return {}; }

    // Java: ICommand.isUsernameIndex(String[], int)
    virtual bool isUsernameIndex(const std::vector<std::string>& args, int32_t index) const { return false; }

    // Java: CommandBase.getRequiredPermissionLevel() — default 4 (op)
    virtual int32_t getRequiredPermissionLevel() const { return 4; }
};

// ═══════════════════════════════════════════════════════════════════════════
// CommandHandler — Command registry and dispatcher.
// Java reference: net.minecraft.command.CommandHandler
//
// Thread safety: shared_mutex for concurrent read access during dispatch,
// exclusive access during registration.
// ═══════════════════════════════════════════════════════════════════════════

class CommandHandler {
public:
    CommandHandler();

    // Java: CommandHandler.registerCommand(ICommand)
    void registerCommand(std::shared_ptr<ICommand> command);

    // Java: CommandHandler.executeCommand(ICommandSender, String)
    // Returns number of successful executions
    int32_t executeCommand(ICommandSender& sender, const std::string& rawCommand);

    // Java: CommandHandler.getPossibleCommands(ICommandSender, String)
    std::vector<std::string> getPossibleCommands(const ICommandSender& sender,
                                                  const std::string& partial) const;

    // Java: CommandHandler.getPossibleCommands(ICommandSender)
    std::vector<std::shared_ptr<ICommand>> getAvailableCommands(const ICommandSender& sender) const;

    // Java: CommandHandler.getCommands()
    const std::unordered_map<std::string, std::shared_ptr<ICommand>>& getCommands() const {
        return commandMap_;
    }

    int32_t getCommandCount() const;

private:
    // Java: CommandHandler.dropFirstString(String[])
    static std::vector<std::string> dropFirstString(const std::vector<std::string>& args);

    // Java: CommandHandler.getUsernameIndex(ICommand, String[])
    static int32_t getUsernameIndex(const ICommand& command, const std::vector<std::string>& args);

    // Utility: split string by spaces
    static std::vector<std::string> splitString(const std::string& str);

    // Utility: case-insensitive starts-with
    static bool doesStringStartWith(const std::string& prefix, const std::string& str);

    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<ICommand>> commandMap_;
    std::unordered_set<std::shared_ptr<ICommand>> commandSet_;
};

// ═══════════════════════════════════════════════════════════════════════════
// Built-in Server Commands
// ═══════════════════════════════════════════════════════════════════════════

// /stop — Stops the server
// Java: net.minecraft.command.server.CommandStop
class CommandStop : public ICommand {
public:
    std::string getCommandName() const override { return "stop"; }
    std::string getCommandUsage() const override { return "commands.stop.usage"; }
    void processCommand(ICommandSender& sender, const std::vector<std::string>& args) override;
};

// /say <message> — Broadcasts a message
// Java: net.minecraft.command.server.CommandSay
class CommandSay : public ICommand {
public:
    std::string getCommandName() const override { return "say"; }
    std::string getCommandUsage() const override { return "commands.say.usage"; }
    int32_t getRequiredPermissionLevel() const override { return 1; }
    void processCommand(ICommandSender& sender, const std::vector<std::string>& args) override;
};

// /help [command] — Shows help
// Java: net.minecraft.command.CommandHelp
class CommandHelp : public ICommand {
public:
    explicit CommandHelp(CommandHandler& handler) : handler_(handler) {}
    std::string getCommandName() const override { return "help"; }
    std::string getCommandUsage() const override { return "commands.help.usage"; }
    std::vector<std::string> getCommandAliases() const override { return {"?"}; }
    int32_t getRequiredPermissionLevel() const override { return 0; }
    void processCommand(ICommandSender& sender, const std::vector<std::string>& args) override;
private:
    CommandHandler& handler_;
};

// /gamemode <mode> [player] — Changes game mode
// Java: net.minecraft.command.CommandGameMode
class CommandGameMode : public ICommand {
public:
    std::string getCommandName() const override { return "gamemode"; }
    std::string getCommandUsage() const override { return "commands.gamemode.usage"; }
    int32_t getRequiredPermissionLevel() const override { return 2; }
    void processCommand(ICommandSender& sender, const std::vector<std::string>& args) override;
    bool isUsernameIndex(const std::vector<std::string>& args, int32_t index) const override {
        return index == 1;
    }
};

// /time <set|add> <value> — Manages world time
// Java: net.minecraft.command.CommandTime
class CommandTime : public ICommand {
public:
    std::string getCommandName() const override { return "time"; }
    std::string getCommandUsage() const override { return "commands.time.usage"; }
    int32_t getRequiredPermissionLevel() const override { return 2; }
    void processCommand(ICommandSender& sender, const std::vector<std::string>& args) override;
};

// /give <player> <item> [amount] [damage] — Gives items
// Java: net.minecraft.command.CommandGive
class CommandGive : public ICommand {
public:
    std::string getCommandName() const override { return "give"; }
    std::string getCommandUsage() const override { return "commands.give.usage"; }
    int32_t getRequiredPermissionLevel() const override { return 2; }
    void processCommand(ICommandSender& sender, const std::vector<std::string>& args) override;
    bool isUsernameIndex(const std::vector<std::string>& args, int32_t index) const override {
        return index == 0;
    }
};

// /tp <target> <destination> OR /tp <target> <x> <y> <z> — Teleport
// Java: net.minecraft.command.CommandTeleport
class CommandTeleport : public ICommand {
public:
    std::string getCommandName() const override { return "tp"; }
    std::string getCommandUsage() const override { return "commands.tp.usage"; }
    int32_t getRequiredPermissionLevel() const override { return 2; }
    void processCommand(ICommandSender& sender, const std::vector<std::string>& args) override;
    bool isUsernameIndex(const std::vector<std::string>& args, int32_t index) const override {
        return index == 0;
    }
};

// /gamerule <rule> [value] — Manages game rules
// Java: net.minecraft.command.CommandGameRule
class CommandGameRule : public ICommand {
public:
    std::string getCommandName() const override { return "gamerule"; }
    std::string getCommandUsage() const override { return "commands.gamerule.usage"; }
    int32_t getRequiredPermissionLevel() const override { return 2; }
    void processCommand(ICommandSender& sender, const std::vector<std::string>& args) override;
};

// /difficulty <level> — Sets difficulty
// Java: net.minecraft.command.CommandDifficulty
class CommandDifficulty : public ICommand {
public:
    std::string getCommandName() const override { return "difficulty"; }
    std::string getCommandUsage() const override { return "commands.difficulty.usage"; }
    int32_t getRequiredPermissionLevel() const override { return 2; }
    void processCommand(ICommandSender& sender, const std::vector<std::string>& args) override;
};

// /seed — Shows world seed
// Java: net.minecraft.command.CommandShowSeed
class CommandSeed : public ICommand {
public:
    std::string getCommandName() const override { return "seed"; }
    std::string getCommandUsage() const override { return "commands.seed.usage"; }
    int32_t getRequiredPermissionLevel() const override { return 2; }
    void processCommand(ICommandSender& sender, const std::vector<std::string>& args) override;
};

// /list — Lists online players
// Java: net.minecraft.command.CommandListPlayers
class CommandList : public ICommand {
public:
    std::string getCommandName() const override { return "list"; }
    std::string getCommandUsage() const override { return "commands.players.usage"; }
    int32_t getRequiredPermissionLevel() const override { return 0; }
    void processCommand(ICommandSender& sender, const std::vector<std::string>& args) override;
};

// /kill [player] — Kills a player
// Java: net.minecraft.command.CommandKill
class CommandKill : public ICommand {
public:
    std::string getCommandName() const override { return "kill"; }
    std::string getCommandUsage() const override { return "commands.kill.usage"; }
    int32_t getRequiredPermissionLevel() const override { return 2; }
    void processCommand(ICommandSender& sender, const std::vector<std::string>& args) override;
    bool isUsernameIndex(const std::vector<std::string>& args, int32_t index) const override {
        return index == 0;
    }
};

} // namespace mccpp
