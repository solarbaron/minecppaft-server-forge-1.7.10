#pragma once
// PacketHandler — dispatches incoming packets by connection state.
// Implements Handshake (0x00), Status Request/Ping, Login Start,
// and the Play state join sequence + Keep Alive for protocol 5 (1.7.10).

#include <cstdint>
#include <string>
#include <iostream>
#include <chrono>
#include <atomic>
#include <unordered_map>

#include "networking/Connection.h"
#include "networking/PacketBuffer.h"
#include "networking/ConnectionState.h"
#include "networking/PlayPackets.h"

namespace mc {

class PacketHandler {
public:
    // Server description shown in the server list
    std::string motd = "A MineCPPaft Server";
    int maxPlayers = 20;
    int onlinePlayers = 0;

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

    // Called each tick to send Keep Alive to all play-state connections
    void tick(Connection& conn) {
        if (conn.state() != ConnectionState::Play) return;

        auto now = std::chrono::steady_clock::now();
        auto& info = playerInfo_[conn.fd()];

        // Send Keep Alive every 20 seconds (400 ticks)
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - info.lastKeepAlive);
        if (elapsed.count() >= 20) {
            KeepAlivePacket ka;
            ka.keepAliveId = static_cast<int32_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()).count() & 0x7FFFFFFF
            );
            conn.sendPacket(ka.serialize());
            info.lastKeepAlive = now;
        }
    }

    void onDisconnect(int fd) {
        playerInfo_.erase(fd);
    }

private:
    struct PlayerInfo {
        std::string name;
        int32_t entityId = 0;
        std::chrono::steady_clock::time_point lastKeepAlive = std::chrono::steady_clock::now();
    };

    std::unordered_map<int, PlayerInfo> playerInfo_;
    std::atomic<int32_t> nextEntityId_{1};

    // === Handshake (state -1) ===
    // C→S 0x00 Handshake: VarInt protocolVersion, String serverAddress,
    //                       UShort serverPort, VarInt nextState
    void handleHandshake(Connection& conn, int32_t packetId, PacketBuffer& buf) {
        if (packetId != 0x00) {
            std::cerr << "[PKT] Unknown handshake packet: 0x"
                      << std::hex << packetId << std::dec << "\n";
            conn.close();
            return;
        }

        int32_t protocolVersion = buf.readVarInt();
        std::string serverAddress = buf.readString(255);
        uint16_t serverPort = buf.readUnsignedShort();
        int32_t nextState = buf.readVarInt();

        std::cout << "[PKT] Handshake: protocol=" << protocolVersion
                  << " addr=" << serverAddress << ":" << serverPort
                  << " nextState=" << nextState << "\n";

        if (nextState == 1) {
            conn.setState(ConnectionState::Status);
        } else if (nextState == 2) {
            conn.setState(ConnectionState::Login);
        } else {
            std::cerr << "[PKT] Invalid nextState: " << nextState << "\n";
            conn.close();
        }
    }

    // === Status (state 1) ===
    void handleStatus(Connection& conn, int32_t packetId, PacketBuffer& buf) {
        if (packetId == 0x00) {
            // C→S Status Request (no payload)
            // S→C Status Response: JSON string
            // kf.java: protocol version 5, name "1.7.10"
            std::string json = R"({)"
                R"("version":{"name":"1.7.10","protocol":5},)"
                R"("players":{"max":)" + std::to_string(maxPlayers) +
                R"(,"online":)" + std::to_string(onlinePlayers) +
                R"(,"sample":[]},)"
                R"("description":{"text":")" + motd + R"("})"
                R"(})";

            PacketBuffer resp;
            resp.writeVarInt(0x00);
            resp.writeString(json);
            conn.sendPacket(resp);
        }
        else if (packetId == 0x01) {
            // C→S Ping: long payload
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
            // C→S Login Start: String playerName
            std::string playerName = buf.readString(16);
            std::cout << "[PKT] Login Start: " << playerName << "\n";

            // Offline mode: respond with Login Success immediately
            // S→C 0x02 Login Success: String UUID, String Username
            std::string uuid = "00000000-0000-0000-0000-000000000000";

            PacketBuffer resp;
            resp.writeVarInt(0x02);
            resp.writeString(uuid);
            resp.writeString(playerName);
            conn.sendPacket(resp);

            conn.setState(ConnectionState::Play);

            // Now send the join sequence
            sendJoinSequence(conn, playerName);
        }
    }

    // === Play join sequence ===
    // Mirrors the packet sequence in oi.java (PlayerList/NetHandlerPlayServer)
    // which is called after login success.
    void sendJoinSequence(Connection& conn, const std::string& playerName) {
        int32_t eid = nextEntityId_++;

        PlayerInfo info;
        info.name = playerName;
        info.entityId = eid;
        info.lastKeepAlive = std::chrono::steady_clock::now();
        playerInfo_[conn.fd()] = info;

        // 1. Join Game — hd.java
        JoinGamePacket joinGame;
        joinGame.entityId = eid;
        joinGame.gameMode = 0;       // Survival
        joinGame.hardcore = false;
        joinGame.dimension = 0;      // Overworld
        joinGame.difficulty = 1;     // Easy
        joinGame.maxPlayers = static_cast<uint8_t>(maxPlayers);
        joinGame.levelType = "default";
        conn.sendPacket(joinGame.serialize());

        // 2. Plugin Message: MC|Brand
        auto brand = PluginMessagePacket::makeBrand("MineCPPaft");
        conn.sendPacket(brand.serialize());

        // 3. Spawn Position
        SpawnPositionPacket spawnPos;
        spawnPos.x = 0;
        spawnPos.y = 64;
        spawnPos.z = 0;
        conn.sendPacket(spawnPos.serialize());

        // 4. Player Abilities
        PlayerAbilitiesPacket abilities;
        abilities.invulnerable = false;
        abilities.flying = false;
        abilities.allowFlying = false;
        abilities.creativeMode = false;
        abilities.flySpeed = 0.05f;
        abilities.walkSpeed = 0.1f;
        conn.sendPacket(abilities.serialize());

        // 5. Send empty chunks around spawn (7x7 grid = 49 chunks)
        // The client needs chunk data before it shows the world
        for (int cx = -3; cx <= 3; ++cx) {
            for (int cz = -3; cz <= 3; ++cz) {
                auto chunk = ChunkDataPacket::makeUnload(cx, cz);
                conn.sendPacket(chunk.serialize());
            }
        }

        // 6. Player Position And Look — teleport to spawn
        PlayerPositionAndLookPacket posLook;
        posLook.x = 0.5;
        posLook.y = 64.0;
        posLook.z = 0.5;
        posLook.yaw = 0.0f;
        posLook.pitch = 0.0f;
        posLook.onGround = false;
        conn.sendPacket(posLook.serialize());

        std::cout << "[PLAY] " << playerName << " (eid=" << eid
                  << ") joined the game\n";
        ++onlinePlayers;
    }

    // === Play (state 0) ===
    void handlePlay(Connection& conn, int32_t packetId, PacketBuffer& buf) {
        switch (packetId) {
            case 0x00: {
                // C→S Keep Alive: int keepAliveId
                int32_t id = buf.readInt();
                (void)id; // We just acknowledge it was received
                break;
            }
            case 0x01: {
                // C→S Chat Message: String message
                std::string message = buf.readString(100);
                std::cout << "[CHAT] <" << getPlayerName(conn.fd()) << "> "
                          << message << "\n";
                // TODO: broadcast to all players
                break;
            }
            case 0x03: {
                // C→S Player (ground status only): bool onGround
                buf.readBoolean();
                break;
            }
            case 0x04: {
                // C→S Player Position: double x,y,headY,z, bool onGround
                buf.readDouble(); // x
                buf.readDouble(); // y (feet)
                buf.readDouble(); // headY
                buf.readDouble(); // z
                buf.readBoolean();
                break;
            }
            case 0x05: {
                // C→S Player Look: float yaw, float pitch, bool onGround
                buf.readFloat();
                buf.readFloat();
                buf.readBoolean();
                break;
            }
            case 0x06: {
                // C→S Player Position And Look: double x,y,headY,z, float yaw,pitch, bool onGround
                buf.readDouble();
                buf.readDouble();
                buf.readDouble();
                buf.readDouble();
                buf.readFloat();
                buf.readFloat();
                buf.readBoolean();
                break;
            }
            case 0x0A: {
                // C→S Animation: int entityId, byte animation
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
                buf.readByte();     // difficulty (unused)
                buf.readBoolean();  // show cape
                break;
            }
            case 0x16: {
                // C→S Client Status: VarInt action
                int32_t action = buf.readVarInt();
                (void)action;
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
                // Unknown packet — skip silently
                break;
        }
    }

    std::string getPlayerName(int fd) const {
        auto it = playerInfo_.find(fd);
        if (it != playerInfo_.end()) return it->second.name;
        return "Unknown";
    }
};

} // namespace mc
