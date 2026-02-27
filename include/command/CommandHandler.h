#pragma once
// CommandHandler — server-side slash command processing.
// Implements basic vanilla commands: /gamemode, /tp, /give, /kill, /time, /help, /say.
// In vanilla this is split across many classes (e.g., CommandGameMode = vz.java,
// CommandTeleport = wh.java, etc.), but we consolidate here for simplicity.

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>

#include "networking/Connection.h"
#include "networking/PlayPackets.h"
#include "entity/Player.h"
#include "world/World.h"

namespace mc {

class CommandHandler {
public:
    // Returns true if the message was a command (started with /)
    bool handleCommand(const std::string& message, Player& sender,
                       Connection& conn,
                       std::unordered_map<int, Player>& players,
                       std::unordered_map<int, Connection>& connections,
                       World& world) {
        if (message.empty() || message[0] != '/') return false;

        auto args = split(message.substr(1));
        if (args.empty()) return false;

        std::string cmd = toLower(args[0]);

        if (cmd == "gamemode" || cmd == "gm") {
            cmdGamemode(args, sender, conn, players, connections);
        } else if (cmd == "tp" || cmd == "teleport") {
            cmdTeleport(args, sender, conn, players);
        } else if (cmd == "give") {
            cmdGive(args, sender, conn);
        } else if (cmd == "kill") {
            cmdKill(args, sender, conn, players, connections);
        } else if (cmd == "time") {
            cmdTime(args, sender, conn, world, connections);
        } else if (cmd == "say") {
            cmdSay(args, sender, connections);
        } else if (cmd == "help" || cmd == "?") {
            cmdHelp(sender, conn);
        } else if (cmd == "list") {
            cmdList(sender, conn, players);
        } else {
            sendError(conn, "Unknown command: /" + cmd + ". Type /help for help.");
        }
        return true;
    }

    // Get tab completions for a partial command
    std::vector<std::string> getCompletions(const std::string& partial,
                                             const std::unordered_map<int, Player>& players) {
        std::vector<std::string> result;
        auto args = split(partial.substr(1)); // skip /

        if (args.size() <= 1) {
            // Complete command name
            std::string prefix = args.empty() ? "" : toLower(args[0]);
            for (auto& cmd : commands_) {
                if (cmd.find(prefix) == 0) {
                    result.push_back("/" + cmd);
                }
            }
        } else {
            // Complete player names for commands that take them
            std::string cmd = toLower(args[0]);
            if (cmd == "tp" || cmd == "kill" || cmd == "gamemode" || cmd == "give") {
                std::string prefix = args.back();
                for (auto& [fd, p] : players) {
                    if (p.name.find(prefix) == 0) {
                        result.push_back(p.name);
                    }
                }
            }
        }
        return result;
    }

private:
    std::vector<std::string> commands_ = {
        "gamemode", "tp", "give", "kill", "time", "say", "help", "list"
    };

    // /gamemode <mode> [player]
    void cmdGamemode(const std::vector<std::string>& args, Player& sender,
                     Connection& conn,
                     std::unordered_map<int, Player>& players,
                     std::unordered_map<int, Connection>& connections) {
        if (args.size() < 2) {
            sendError(conn, "Usage: /gamemode <0|1|2|s|c|a> [player]");
            return;
        }

        GameMode mode;
        std::string modeStr = toLower(args[1]);
        if (modeStr == "0" || modeStr == "s" || modeStr == "survival") {
            mode = GameMode::Survival;
        } else if (modeStr == "1" || modeStr == "c" || modeStr == "creative") {
            mode = GameMode::Creative;
        } else if (modeStr == "2" || modeStr == "a" || modeStr == "adventure") {
            mode = GameMode::Adventure;
        } else {
            sendError(conn, "Invalid game mode: " + args[1]);
            return;
        }

        Player* target = &sender;
        Connection* targetConn = &conn;
        if (args.size() >= 3) {
            auto [p, c] = findPlayer(args[2], players, connections);
            if (!p) {
                sendError(conn, "Player not found: " + args[2]);
                return;
            }
            target = p;
            targetConn = c;
        }

        target->gameMode = mode;

        // Send Change Game State packet (reason=3 = change game mode)
        ChangeGameStatePacket gs;
        gs.reason = 3;
        gs.value = static_cast<float>(static_cast<uint8_t>(mode));
        targetConn->sendPacket(gs.serialize());

        // Update abilities
        PlayerAbilitiesPacket abilities;
        abilities.invulnerable = (mode == GameMode::Creative);
        abilities.flying = target->flying;
        abilities.allowFlying = (mode == GameMode::Creative);
        abilities.creativeMode = (mode == GameMode::Creative);
        abilities.flySpeed = target->flySpeed;
        abilities.walkSpeed = target->walkSpeed;
        targetConn->sendPacket(abilities.serialize());

        std::string modeName;
        switch (mode) {
            case GameMode::Survival: modeName = "Survival"; break;
            case GameMode::Creative: modeName = "Creative"; break;
            case GameMode::Adventure: modeName = "Adventure"; break;
        }
        sendSuccess(conn, "Set " + target->name + "'s game mode to " + modeName);
    }

    // /tp <player> OR /tp <x> <y> <z> OR /tp <player1> <player2>
    void cmdTeleport(const std::vector<std::string>& args, Player& sender,
                     Connection& conn,
                     std::unordered_map<int, Player>& players) {
        if (args.size() < 2) {
            sendError(conn, "Usage: /tp <player> or /tp <x> <y> <z>");
            return;
        }

        if (args.size() == 2) {
            // /tp <player> — teleport sender to target
            auto [target, _] = findPlayer(args[1], players, *static_cast<std::unordered_map<int, Connection>*>(nullptr));
            // We need connections to find player, but we only have players map
            // Search by name
            Player* target2 = nullptr;
            for (auto& [fd, p] : players) {
                if (p.name == args[1]) { target2 = &p; break; }
            }
            if (!target2) {
                sendError(conn, "Player not found: " + args[1]);
                return;
            }
            sender.posX = target2->posX;
            sender.posY = target2->posY;
            sender.posZ = target2->posZ;
        } else if (args.size() >= 4) {
            // /tp <x> <y> <z>
            try {
                sender.posX = std::stod(args[1]);
                sender.posY = std::stod(args[2]);
                sender.posZ = std::stod(args[3]);
            } catch (...) {
                sendError(conn, "Invalid coordinates");
                return;
            }
        }

        // Send new position to client
        PlayerPositionAndLookPacket posLook;
        posLook.x = sender.posX;
        posLook.y = sender.posY;
        posLook.z = sender.posZ;
        posLook.yaw = sender.yaw;
        posLook.pitch = sender.pitch;
        posLook.onGround = false;
        conn.sendPacket(posLook.serialize());

        sendSuccess(conn, "Teleported to " +
            std::to_string(static_cast<int>(sender.posX)) + " " +
            std::to_string(static_cast<int>(sender.posY)) + " " +
            std::to_string(static_cast<int>(sender.posZ)));
    }

    // /give <item_id> [count] [damage]
    void cmdGive(const std::vector<std::string>& args, Player& sender,
                 Connection& conn) {
        if (args.size() < 2) {
            sendError(conn, "Usage: /give <item_id> [count] [damage]");
            return;
        }

        int16_t itemId;
        int8_t count = 1;
        int16_t damage = 0;

        try {
            itemId = static_cast<int16_t>(std::stoi(args[1]));
            if (args.size() >= 3) count = static_cast<int8_t>(std::stoi(args[2]));
            if (args.size() >= 4) damage = static_cast<int16_t>(std::stoi(args[3]));
        } catch (...) {
            sendError(conn, "Invalid item ID or count");
            return;
        }

        // Find first empty hotbar slot, or slot 0
        int targetSlot = 0;
        for (int i = 0; i < 9; ++i) {
            if (!sender.inventory.mainSlots[i] || sender.inventory.mainSlots[i]->isEmpty()) {
                targetSlot = i;
                break;
            }
        }

        ItemStack stack(itemId, count, damage);
        sender.inventory.mainSlots[targetSlot] = stack;

        // Send Set Slot (window slot = 36 + hotbar index)
        SetSlotPacket slot;
        slot.windowId = 0;
        slot.slotIndex = static_cast<int16_t>(36 + targetSlot);
        slot.item = stack;
        conn.sendPacket(slot.serialize());

        sendSuccess(conn, "Given [" + std::to_string(itemId) + "] x" +
            std::to_string(static_cast<int>(count)) + " to " + sender.name);
    }

    // /kill [player]
    void cmdKill(const std::vector<std::string>& args, Player& sender,
                 Connection& conn,
                 std::unordered_map<int, Player>& players,
                 std::unordered_map<int, Connection>& connections) {
        Player* target = &sender;
        Connection* targetConn = &conn;

        if (args.size() >= 2) {
            auto [p, c] = findPlayer(args[1], players, connections);
            if (!p) {
                sendError(conn, "Player not found: " + args[1]);
                return;
            }
            target = p;
            targetConn = c;
        }

        target->health = 0.0f;

        UpdateHealthPacket hp;
        hp.health = 0.0f;
        hp.food = target->foodLevel;
        hp.saturation = target->saturation;
        targetConn->sendPacket(hp.serialize());

        // Broadcast death to all
        auto deathMsg = ChatMessagePacket::makeText(
            "\u00a77" + target->name + " was killed");
        for (auto& [fd, c] : connections) {
            if (c.state() == ConnectionState::Play) {
                c.sendPacket(deathMsg.serialize());
            }
        }

        sendSuccess(conn, "Killed " + target->name);
    }

    // /time set <value> OR /time query
    void cmdTime(const std::vector<std::string>& args, Player& sender,
                 Connection& conn, World& world,
                 std::unordered_map<int, Connection>& connections) {
        if (args.size() < 2) {
            sendError(conn, "Usage: /time set <value> or /time query day|daytime");
            return;
        }

        std::string sub = toLower(args[1]);
        if (sub == "set" && args.size() >= 3) {
            int64_t value;
            std::string timeStr = toLower(args[2]);
            if (timeStr == "day") value = 1000;
            else if (timeStr == "night") value = 13000;
            else if (timeStr == "noon") value = 6000;
            else if (timeStr == "midnight") value = 18000;
            else {
                try { value = std::stoll(args[2]); }
                catch (...) { sendError(conn, "Invalid time value"); return; }
            }
            world.dayTime = value;

            // Broadcast time to all
            TimeUpdatePacket time;
            time.worldAge = world.worldTime;
            time.timeOfDay = world.dayTime;
            for (auto& [fd, c] : connections) {
                if (c.state() == ConnectionState::Play) {
                    c.sendPacket(time.serialize());
                }
            }
            sendSuccess(conn, "Set time to " + std::to_string(value));
        } else if (sub == "query") {
            sendSuccess(conn, "Day time: " + std::to_string(world.dayTime) +
                ", World age: " + std::to_string(world.worldTime));
        } else {
            sendError(conn, "Usage: /time set <value> or /time query");
        }
    }

    // /say <message>
    void cmdSay(const std::vector<std::string>& args, Player& sender,
                std::unordered_map<int, Connection>& connections) {
        if (args.size() < 2) return;
        std::string msg;
        for (size_t i = 1; i < args.size(); ++i) {
            if (i > 1) msg += " ";
            msg += args[i];
        }
        auto pkt = ChatMessagePacket::makeText(
            "\u00a7d[" + sender.name + "] " + msg);
        for (auto& [fd, c] : connections) {
            if (c.state() == ConnectionState::Play) {
                c.sendPacket(pkt.serialize());
            }
        }
    }

    // /help
    void cmdHelp(Player& sender, Connection& conn) {
        sendInfo(conn, "\u00a76--- Help ---");
        sendInfo(conn, "\u00a7e/gamemode <0|1|2> [player]");
        sendInfo(conn, "\u00a7e/tp <player> | /tp <x> <y> <z>");
        sendInfo(conn, "\u00a7e/give <item_id> [count] [damage]");
        sendInfo(conn, "\u00a7e/kill [player]");
        sendInfo(conn, "\u00a7e/time set <value> | /time query");
        sendInfo(conn, "\u00a7e/say <message>");
        sendInfo(conn, "\u00a7e/list");
    }

    // /list
    void cmdList(Player& sender, Connection& conn,
                 const std::unordered_map<int, Player>& players) {
        std::string list = "\u00a76Online (" + std::to_string(players.size()) + "): \u00a7f";
        bool first = true;
        for (auto& [fd, p] : players) {
            if (!first) list += ", ";
            list += p.name;
            first = false;
        }
        sendInfo(conn, list);
    }

    // Helpers
    void sendError(Connection& conn, const std::string& msg) {
        auto pkt = ChatMessagePacket::makeText("\u00a7c" + msg);
        conn.sendPacket(pkt.serialize());
    }

    void sendSuccess(Connection& conn, const std::string& msg) {
        auto pkt = ChatMessagePacket::makeText("\u00a7a" + msg);
        conn.sendPacket(pkt.serialize());
    }

    void sendInfo(Connection& conn, const std::string& msg) {
        auto pkt = ChatMessagePacket::makeText(msg);
        conn.sendPacket(pkt.serialize());
    }

    std::pair<Player*, Connection*> findPlayer(const std::string& name,
                                                std::unordered_map<int, Player>& players,
                                                std::unordered_map<int, Connection>& connections) {
        for (auto& [fd, p] : players) {
            if (p.name == name) {
                auto it = connections.find(fd);
                if (it != connections.end()) {
                    return {&p, &it->second};
                }
                return {&p, nullptr};
            }
        }
        return {nullptr, nullptr};
    }

    std::vector<std::string> split(const std::string& s) {
        std::vector<std::string> result;
        std::istringstream iss(s);
        std::string token;
        while (iss >> token) {
            result.push_back(token);
        }
        return result;
    }

    std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }
};

} // namespace mc
