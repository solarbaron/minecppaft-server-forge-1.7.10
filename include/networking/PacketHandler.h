#pragma once
// PacketHandler — dispatches incoming packets by connection state.
// Implements Handshake, Status, Login, and Play states for protocol 5 (1.7.10).
// Now with Player entity tracking, World management, chat broadcast,
// and entity spawn/despawn for multiplayer.

#include <cstdint>
#include <string>
#include <iostream>
#include <chrono>
#include <atomic>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "networking/Connection.h"
#include "networking/PacketBuffer.h"
#include "networking/ConnectionState.h"
#include "networking/PlayPackets.h"
#include "entity/Player.h"
#include "world/World.h"

namespace mc {

class PacketHandler {
public:
    // Server description shown in the server list
    std::string motd = "A MineCPPaft Server";
    int maxPlayers = 20;

    // World
    World world;

    void handle(Connection& conn, PacketBuffer& buf) {
        int32_t packetId = buf.readVarInt();

        switch (conn.state()) {
            case ConnectionState::Handshaking:
                handleHandshake(conn, packetId, buf);
                break;
            case ConnectionState::Status:
                handleStatus(conn, packetId, buf);
                break;
            case ConnectionState::Login:
                handleLogin(conn, packetId, buf);
                break;
            case ConnectionState::Play:
                handlePlay(conn, packetId, buf);
                break;
        }
    }

    // Called each tick for play-state connections
    void tick(Connection& conn) {
        if (conn.state() != ConnectionState::Play) return;

        auto now = std::chrono::steady_clock::now();
        auto it = players_.find(conn.fd());
        if (it == players_.end()) return;
        auto& player = it->second;

        // Send Keep Alive every 20 seconds
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - player.lastKeepAlive);
        if (elapsed.count() >= 20) {
            KeepAlivePacket ka;
            ka.keepAliveId = static_cast<int32_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()).count() & 0x7FFFFFFF
            );
            conn.sendPacket(ka.serialize());
            player.lastKeepAlive = now;
        }
    }

    // Called each server tick for world updates
    void worldTick(std::unordered_map<int, Connection>& connections) {
        world.tick();

        // Send time update every 20 ticks (1 second)
        if (world.worldTime % 20 == 0) {
            TimeUpdatePacket time;
            time.worldAge = world.worldTime;
            time.timeOfDay = world.dayTime;

            for (auto& [fd, conn] : connections) {
                if (conn.state() == ConnectionState::Play) {
                    conn.sendPacket(time.serialize());
                }
            }
        }
    }

    void onDisconnect(int fd, std::unordered_map<int, Connection>& connections) {
        auto it = players_.find(fd);
        if (it == players_.end()) return;

        auto& player = it->second;
        std::cout << "[PLAY] " << player.name << " left the game\n";

        // Notify other players to destroy this entity
        DestroyEntitiesPacket destroy;
        destroy.entityIds.push_back(player.entityId);

        // Send yellow leave message
        auto msg = ChatMessagePacket::makeText(
            "\u00a7e" + player.name + " left the game");

        for (auto& [otherFd, otherConn] : connections) {
            if (otherFd != fd && otherConn.state() == ConnectionState::Play) {
                otherConn.sendPacket(destroy.serialize());
                otherConn.sendPacket(msg.serialize());
            }
        }

        players_.erase(it);
    }

    int onlineCount() const { return static_cast<int>(players_.size()); }

    const std::unordered_map<int, Player>& players() const { return players_; }

private:
    std::unordered_map<int, Player> players_;
    std::atomic<int32_t> nextEntityId_{1};

    // === Handshake (state -1) ===
    void handleHandshake(Connection& conn, int32_t packetId, PacketBuffer& buf) {
        if (packetId != 0x00) {
            conn.close();
            return;
        }

        int32_t protocolVersion = buf.readVarInt();
        std::string serverAddress = buf.readString(255);
        uint16_t serverPort = buf.readUnsignedShort();
        int32_t nextState = buf.readVarInt();

        (void)protocolVersion;
        (void)serverAddress;
        (void)serverPort;

        if (nextState == 1) {
            conn.setState(ConnectionState::Status);
        } else if (nextState == 2) {
            conn.setState(ConnectionState::Login);
        } else {
            conn.close();
        }
    }

    // === Status (state 1) ===
    void handleStatus(Connection& conn, int32_t packetId, PacketBuffer& buf) {
        if (packetId == 0x00) {
            // S→C Status Response
            // kf.java: protocol version 5, name "1.7.10"
            std::string json = R"({)"
                R"("version":{"name":"1.7.10","protocol":5},)"
                R"("players":{"max":)" + std::to_string(maxPlayers) +
                R"(,"online":)" + std::to_string(onlineCount()) +
                R"(,"sample":[]},)"
                R"("description":{"text":")" + motd + R"("})"
                R"(})";

            PacketBuffer resp;
            resp.writeVarInt(0x00);
            resp.writeString(json);
            conn.sendPacket(resp);
        }
        else if (packetId == 0x01) {
            int64_t payload = buf.readLong();
            PacketBuffer resp;
            resp.writeVarInt(0x01);
            resp.writeLong(payload);
            conn.sendPacket(resp);
        }
    }

    // === Login (state 2) ===
    void handleLogin(Connection& conn, int32_t packetId, PacketBuffer& buf) {
        if (packetId == 0x00) {
            std::string playerName = buf.readString(16);
            std::cout << "[PKT] Login Start: " << playerName << "\n";

            // Offline mode: Login Success
            std::string uuid = "00000000-0000-0000-0000-000000000000";

            PacketBuffer resp;
            resp.writeVarInt(0x02);
            resp.writeString(uuid);
            resp.writeString(playerName);
            conn.sendPacket(resp);

            conn.setState(ConnectionState::Play);

            sendJoinSequence(conn, playerName, uuid);
        }
    }

    // === Play join sequence ===
    void sendJoinSequence(Connection& conn, const std::string& playerName,
                          const std::string& uuid) {
        int32_t eid = nextEntityId_++;

        Player player;
        player.entityId = eid;
        player.name = playerName;
        player.uuid = uuid;
        player.connectionFd = conn.fd();
        player.posX = 0.5;
        player.posY = 4.0;  // Spawn on top of flat world (bedrock+2dirt+grass = y=4)
        player.posZ = 0.5;
        player.lastKeepAlive = std::chrono::steady_clock::now();

        // 1. Join Game — hd.java
        JoinGamePacket joinGame;
        joinGame.entityId = eid;
        joinGame.gameMode = static_cast<uint8_t>(player.gameMode);
        joinGame.hardcore = false;
        joinGame.dimension = player.dimension;
        joinGame.difficulty = 1; // Easy
        joinGame.maxPlayers = static_cast<uint8_t>(maxPlayers);
        joinGame.levelType = "flat";
        conn.sendPacket(joinGame.serialize());

        // 2. MC|Brand
        auto brand = PluginMessagePacket::makeBrand("MineCPPaft");
        conn.sendPacket(brand.serialize());

        // 3. Spawn Position
        SpawnPositionPacket spawnPos;
        spawnPos.x = 0;
        spawnPos.y = 4;
        spawnPos.z = 0;
        conn.sendPacket(spawnPos.serialize());

        // 4. Player Abilities
        PlayerAbilitiesPacket abilities;
        abilities.invulnerable = player.invulnerable;
        abilities.flying = player.flying;
        abilities.allowFlying = player.allowFlying;
        abilities.creativeMode = (player.gameMode == GameMode::Creative);
        abilities.flySpeed = player.flySpeed;
        abilities.walkSpeed = player.walkSpeed;
        conn.sendPacket(abilities.serialize());

        // 5. Time
        TimeUpdatePacket time;
        time.worldAge = world.worldTime;
        time.timeOfDay = world.dayTime;
        conn.sendPacket(time.serialize());

        // 6. Chunks around spawn (7x7)
        for (int cx = -3; cx <= 3; ++cx) {
            for (int cz = -3; cz <= 3; ++cz) {
                auto& chunk = world.getChunk(cx, cz);
                auto pkt = ChunkDataPacket::fromChunkColumn(chunk, true);
                conn.sendPacket(pkt.serialize());
            }
        }

        // 7. Player Position And Look
        PlayerPositionAndLookPacket posLook;
        posLook.x = player.posX;
        posLook.y = player.posY;
        posLook.z = player.posZ;
        posLook.yaw = player.yaw;
        posLook.pitch = player.pitch;
        posLook.onGround = false;
        conn.sendPacket(posLook.serialize());

        // Store player BEFORE broadcasting (so we have the entity ID)
        players_[conn.fd()] = player;

        std::cout << "[PLAY] " << playerName << " (eid=" << eid
                  << ") joined the game\n";

        // 8. Broadcast join message to all players
        auto joinMsg = ChatMessagePacket::makeText(
            "\u00a7e" + playerName + " joined the game");

        // 9. Spawn existing players for the new player, and new player for existing
        for (auto& [otherFd, otherPlayer] : players_) {
            if (otherFd == conn.fd()) continue;

            // Send SpawnPlayer of the new player to existing players
            // (need access to connection — done via fd lookup in caller)
        }
    }

    // === Play (state 0) ===
    void handlePlay(Connection& conn, int32_t packetId, PacketBuffer& buf) {
        auto it = players_.find(conn.fd());
        Player* player = (it != players_.end()) ? &it->second : nullptr;

        switch (packetId) {
            case 0x00: {
                // C→S Keep Alive
                buf.readInt();
                break;
            }
            case 0x01: {
                // C→S Chat Message
                std::string message = buf.readString(100);
                if (player) {
                    std::cout << "[CHAT] <" << player->name << "> " << message << "\n";
                    // Broadcast handled by caller via broadcastChat()
                }
                break;
            }
            case 0x03: {
                // C→S Player (ground only)
                if (player) player->onGround = buf.readBoolean();
                break;
            }
            case 0x04: {
                // C→S Player Position
                if (player) {
                    player->posX = buf.readDouble();
                    player->posY = buf.readDouble();
                    buf.readDouble(); // headY
                    player->posZ = buf.readDouble();
                    player->onGround = buf.readBoolean();
                }
                break;
            }
            case 0x05: {
                // C→S Player Look
                if (player) {
                    player->yaw = buf.readFloat();
                    player->pitch = buf.readFloat();
                    player->onGround = buf.readBoolean();
                }
                break;
            }
            case 0x06: {
                // C→S Player Position And Look
                if (player) {
                    player->posX = buf.readDouble();
                    player->posY = buf.readDouble();
                    buf.readDouble(); // headY
                    player->posZ = buf.readDouble();
                    player->yaw = buf.readFloat();
                    player->pitch = buf.readFloat();
                    player->onGround = buf.readBoolean();
                }
                break;
            }
            case 0x0A: {
                // C→S Animation
                buf.readVarInt();
                buf.readByte();
                break;
            }
            case 0x15: {
                // C→S Client Settings
                buf.readString(16); // locale
                buf.readByte();     // view distance
                buf.readByte();     // chat mode
                buf.readBoolean();  // chat colors
                buf.readByte();     // difficulty
                buf.readBoolean();  // show cape
                break;
            }
            case 0x16: {
                // C→S Client Status
                buf.readVarInt();
                break;
            }
            case 0x17: {
                // C→S Plugin Message
                std::string channel = buf.readString(20);
                int16_t len = buf.readShort();
                if (len > 0) buf.readBytes(static_cast<size_t>(len));
                break;
            }
            default:
                break;
        }
    }

public:
    // Broadcast a chat message to all play-state connections
    void broadcastChat(const std::string& playerName, const std::string& message,
                       std::unordered_map<int, Connection>& connections) {
        auto pkt = ChatMessagePacket::makeChat(playerName, message);
        for (auto& [fd, conn] : connections) {
            if (conn.state() == ConnectionState::Play) {
                conn.sendPacket(pkt.serialize());
            }
        }
    }

    // Broadcast a spawn packet for a new player to all other players
    void broadcastSpawn(const Player& player,
                        std::unordered_map<int, Connection>& connections) {
        SpawnPlayerPacket spawn;
        spawn.entityId = player.entityId;
        spawn.uuid = player.uuid;
        spawn.name = player.name;
        spawn.x = player.posX;
        spawn.y = player.posY;
        spawn.z = player.posZ;
        spawn.yaw = player.yaw;
        spawn.pitch = player.pitch;
        spawn.currentItem = 0;

        auto joinMsg = ChatMessagePacket::makeText(
            "\u00a7e" + player.name + " joined the game");

        for (auto& [fd, conn] : connections) {
            if (fd != player.connectionFd && conn.state() == ConnectionState::Play) {
                conn.sendPacket(spawn.serialize());
                conn.sendPacket(joinMsg.serialize());
            }
        }

        // Send existing players to the new player
        auto it = connections.find(player.connectionFd);
        if (it != connections.end()) {
            for (auto& [otherFd, otherPlayer] : players_) {
                if (otherFd == player.connectionFd) continue;
                SpawnPlayerPacket otherSpawn;
                otherSpawn.entityId = otherPlayer.entityId;
                otherSpawn.uuid = otherPlayer.uuid;
                otherSpawn.name = otherPlayer.name;
                otherSpawn.x = otherPlayer.posX;
                otherSpawn.y = otherPlayer.posY;
                otherSpawn.z = otherPlayer.posZ;
                otherSpawn.yaw = otherPlayer.yaw;
                otherSpawn.pitch = otherPlayer.pitch;
                otherSpawn.currentItem = 0;
                it->second.sendPacket(otherSpawn.serialize());
            }
        }
    }
};

} // namespace mc
