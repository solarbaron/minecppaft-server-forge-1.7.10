#pragma once
// PacketHandler — dispatches incoming packets by connection state.
// Implements Handshake (0x00), Status Request/Ping, and Login Start
// for Minecraft protocol 47 (1.7.10).

#include <cstdint>
#include <string>
#include <iostream>
#include <chrono>

#include "networking/Connection.h"
#include "networking/PacketBuffer.h"
#include "networking/ConnectionState.h"

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

private:
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
            std::string json = R"({)"
                R"("version":{"name":"1.7.10","protocol":5},)"
                R"("players":{"max":)" + std::to_string(maxPlayers) +
                R"(,"online":)" + std::to_string(onlinePlayers) +
                R"(,"sample":[]},)"
                R"("description":{"text":")" + motd + R"("})"
                R"(})";

            PacketBuffer resp;
            resp.writeVarInt(0x00); // Packet ID
            resp.writeString(json);
            conn.sendPacket(resp);
        }
        else if (packetId == 0x01) {
            // C→S Ping: long payload
            int64_t payload = buf.readLong();

            // S→C Pong: echo the long back
            PacketBuffer resp;
            resp.writeVarInt(0x01); // Packet ID
            resp.writeLong(payload);
            conn.sendPacket(resp);
        }
        else {
            std::cerr << "[PKT] Unknown status packet: 0x"
                      << std::hex << packetId << std::dec << "\n";
        }
    }

    // === Login (state 2) ===
    void handleLogin(Connection& conn, int32_t packetId, PacketBuffer& buf) {
        if (packetId == 0x00) {
            // C→S Login Start: String playerName
            std::string playerName = buf.readString(16);
            std::cout << "[PKT] Login Start: " << playerName << "\n";

            // For now (offline mode), respond with Login Success
            // S→C 0x02 Login Success: String UUID, String Username
            // Use offline UUID: OfflinePlayer:<name>
            std::string uuid = "00000000-0000-0000-0000-000000000000";

            PacketBuffer resp;
            resp.writeVarInt(0x02); // Packet ID: Login Success
            resp.writeString(uuid);
            resp.writeString(playerName);
            conn.sendPacket(resp);

            conn.setState(ConnectionState::Play);
            std::cout << "[PKT] Player " << playerName
                      << " logged in (offline mode)\n";

            // TODO: Send Join Game, Spawn Position, etc.
        }
        else {
            std::cerr << "[PKT] Unknown login packet: 0x"
                      << std::hex << packetId << std::dec << "\n";
        }
    }

    // === Play (state 0) ===
    void handlePlay(Connection& conn, int32_t packetId, PacketBuffer& buf) {
        // TODO: Implement play packets
        // For now, silently ignore
        (void)buf;
        (void)packetId;
    }
};

} // namespace mc
