/**
 * PacketHandler.cpp — Packet handler implementations for Handshake, Status, Login.
 *
 * Reference:
 *   - net.minecraft.server.network.NetHandlerHandshakeTCP
 *   - net.minecraft.server.network.NetHandlerStatusServer
 *   - net.minecraft.server.network.NetHandlerLoginServer
 */

#include "networking/PacketHandler.h"
#include "networking/Connection.h"
#include "networking/PlayPackets.h"
#include "server/MinecraftServer.h"
#include "types/VarInt.h"

#include <array>
#include <cstring>
#include <functional>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace mccpp {

// ─── MD5-based UUID generation (for offline mode) ───────────────────────────
// Java's UUID.nameUUIDFromBytes() uses MD5 with version=3, variant=2.
// We implement a minimal MD5 here to avoid an OpenSSL dependency just for this.
// This will be replaced when OpenSSL is added for encryption.

namespace {

// Minimal MD5 — RFC 1321 implementation for offline UUID generation
struct MD5 {
    uint32_t state[4];
    uint64_t count;
    uint8_t  buffer[64];

    MD5() : count(0) {
        state[0] = 0x67452301;
        state[1] = 0xefcdab89;
        state[2] = 0x98badcfe;
        state[3] = 0x10325476;
        std::memset(buffer, 0, sizeof(buffer));
    }

    static uint32_t F(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (~x & z); }
    static uint32_t G(uint32_t x, uint32_t y, uint32_t z) { return (x & z) | (y & ~z); }
    static uint32_t H(uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; }
    static uint32_t I(uint32_t x, uint32_t y, uint32_t z) { return y ^ (x | ~z); }
    static uint32_t rotl(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }

    void transform(const uint8_t block[64]) {
        static const uint32_t T[64] = {
            0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
            0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,0xa679438e,0x49b40821,
            0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
            0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
            0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
            0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
            0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
            0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391,
        };
        static const int s[64] = {
            7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
            5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
            4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
            6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21,
        };

        uint32_t M[16];
        for (int i = 0; i < 16; ++i) {
            M[i] = static_cast<uint32_t>(block[i*4]) |
                    (static_cast<uint32_t>(block[i*4+1]) << 8) |
                    (static_cast<uint32_t>(block[i*4+2]) << 16) |
                    (static_cast<uint32_t>(block[i*4+3]) << 24);
        }

        uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
        for (int i = 0; i < 64; ++i) {
            uint32_t f, g;
            if (i < 16)      { f = F(b,c,d); g = i; }
            else if (i < 32) { f = G(b,c,d); g = (5*i+1) % 16; }
            else if (i < 48) { f = H(b,c,d); g = (3*i+5) % 16; }
            else              { f = I(b,c,d); g = (7*i) % 16; }
            uint32_t tmp = d;
            d = c; c = b;
            b = b + rotl(a + f + T[i] + M[g], s[i]);
            a = tmp;
        }
        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    }

    void update(const uint8_t* data, size_t len) {
        size_t index = static_cast<size_t>(count % 64);
        count += len;
        size_t i = 0;
        if (index) {
            size_t part = 64 - index;
            if (len >= part) {
                std::memcpy(buffer + index, data, part);
                transform(buffer);
                i = part;
            } else {
                std::memcpy(buffer + index, data, len);
                return;
            }
        }
        for (; i + 64 <= len; i += 64) {
            transform(data + i);
        }
        if (i < len) {
            std::memcpy(buffer, data + i, len - i);
        }
    }

    std::array<uint8_t, 16> digest() {
        uint8_t padding[64] = {0x80};
        size_t index = static_cast<size_t>(count % 64);
        size_t padLen = (index < 56) ? (56 - index) : (120 - index);
        uint64_t bits = count * 8;
        update(padding, padLen);
        uint8_t bitsLE[8];
        for (int i = 0; i < 8; ++i) bitsLE[i] = static_cast<uint8_t>(bits >> (8*i));
        update(bitsLE, 8);

        std::array<uint8_t, 16> result;
        for (int i = 0; i < 4; ++i) {
            result[i*4]   = static_cast<uint8_t>(state[i]);
            result[i*4+1] = static_cast<uint8_t>(state[i] >> 8);
            result[i*4+2] = static_cast<uint8_t>(state[i] >> 16);
            result[i*4+3] = static_cast<uint8_t>(state[i] >> 24);
        }
        return result;
    }
};

/**
 * Java's UUID.nameUUIDFromBytes() — MD5 hash with version=3, variant=IETF.
 * Returns a formatted UUID string: "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
 */
std::string nameUUIDFromBytes(const std::string& input) {
    MD5 md5;
    md5.update(reinterpret_cast<const uint8_t*>(input.data()), input.size());
    auto hash = md5.digest();

    // Set version to 3 (name-based MD5)
    hash[6] = (hash[6] & 0x0F) | 0x30;
    // Set variant to IETF (10xx)
    hash[8] = (hash[8] & 0x3F) | 0x80;

    // Format as UUID string
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) ss << '-';
        ss << std::setw(2) << static_cast<int>(hash[i]);
    }
    return ss.str();
}

} // anonymous namespace

// ─── HandshakeHandler ───────────────────────────────────────────────────────
// Java reference: net.minecraft.server.network.NetHandlerHandshakeTCP

HandshakeHandler::HandshakeHandler(MinecraftServer& server)
    : server_(server) {}

void HandshakeHandler::handlePacket(int32_t packetId,
                                     const uint8_t* data,
                                     size_t length,
                                     Connection& conn) {
    // Java reference: NetHandlerHandshakeTCP.processHandshake(C00Handshake)
    if (packetId != HandshakePacket::Handshake) {
        conn.disconnect("Unexpected packet in Handshake state");
        return;
    }

    // Decode C00Handshake:
    //   VarInt protocolVersion
    //   String serverAddress
    //   UShort serverPort
    //   VarInt nextState (1 = Status, 2 = Login)
    size_t offset = 0;

    auto protocolResult = readVarInt(data + offset, length - offset);
    int32_t protocolVersion = protocolResult.value;
    offset += protocolResult.bytesRead;

    auto addressResult = readString(data + offset, length - offset);
    offset += addressResult.bytesRead;

    if (offset + 2 > length) {
        conn.disconnect("Handshake packet too short");
        return;
    }
    offset += 2; // server port (unsigned short, big-endian)

    auto nextStateResult = readVarInt(data + offset, length - offset);
    int32_t nextState = nextStateResult.value;

    std::cout << "[Handshake] Protocol=" << protocolVersion
              << " NextState=" << nextState
              << " from " << conn.getRemoteAddress() << "\n";

    if (nextState == 2) {
        // LOGIN requested
        conn.setState(ConnectionState::Login);

        // Java reference: NetHandlerHandshakeTCP.processHandshake()
        // Protocol version validation — exact match required (protocol 5 = 1.7.10)
        if (protocolVersion > MinecraftServer::PROTOCOL_VERSION) {
            // "Outdated server! I'm still on 1.7.10"
            std::string msg = R"({"text":"Outdated server! I'm still on 1.7.10"})";
            std::vector<uint8_t> pkt;
            writeVarInt(pkt, LoginPacket::Disconnect); // S00PacketDisconnect
            writeString(pkt, msg);
            conn.sendPacket(std::move(pkt));
            conn.disconnect("Outdated server");
            return;
        }
        if (protocolVersion < MinecraftServer::PROTOCOL_VERSION) {
            // "Outdated client! Please use 1.7.10"
            std::string msg = R"({"text":"Outdated client! Please use 1.7.10"})";
            std::vector<uint8_t> pkt;
            writeVarInt(pkt, LoginPacket::Disconnect);
            writeString(pkt, msg);
            conn.sendPacket(std::move(pkt));
            conn.disconnect("Outdated client");
            return;
        }

        // Swap handler to LoginHandler
        conn.setHandler(std::make_shared<LoginHandler>(server_));

    } else if (nextState == 1) {
        // STATUS requested
        conn.setState(ConnectionState::Status);
        conn.setHandler(std::make_shared<StatusHandler>(server_));

    } else {
        // Java: throw new UnsupportedOperationException("Invalid intention " + nextState)
        conn.disconnect("Invalid intention " + std::to_string(nextState));
    }
}

void HandshakeHandler::onDisconnect(const std::string& reason) {
    // Java: NetHandlerHandshakeTCP.onDisconnect() — empty
    (void)reason;
}

// ─── StatusHandler ──────────────────────────────────────────────────────────
// Java reference: net.minecraft.server.network.NetHandlerStatusServer

StatusHandler::StatusHandler(MinecraftServer& server)
    : server_(server) {}

void StatusHandler::handlePacket(int32_t packetId,
                                  const uint8_t* data,
                                  size_t length,
                                  Connection& conn) {
    if (packetId == StatusPacket::Request) {
        // Java reference: NetHandlerStatusServer.processServerQuery()
        // Responds with S00PacketServerInfo containing server status JSON
        std::string json = R"({"version":{"name":")" +
            std::string(MinecraftServer::GAME_VERSION) +
            R"(","protocol":)" + std::to_string(MinecraftServer::PROTOCOL_VERSION) +
            R"(},"players":{"max":)" + std::to_string(server_.getMaxPlayers()) +
            R"(,"online":)" + std::to_string(server_.getOnlinePlayerCount()) +
            R"(},"description":{"text":")" + server_.getMotd() + R"("}})";

        std::vector<uint8_t> response;
        writeVarInt(response, StatusPacket::Response);
        writeString(response, json);
        conn.sendPacket(std::move(response));

    } else if (packetId == StatusPacket::Ping) {
        // Java reference: NetHandlerStatusServer.processPing()
        // Echo back the 8-byte long (client time)
        if (length < 8) {
            conn.disconnect("Ping packet too short");
            return;
        }

        std::vector<uint8_t> pong;
        writeVarInt(pong, StatusPacket::Pong);
        pong.insert(pong.end(), data, data + 8);
        conn.sendPacket(std::move(pong));

    } else {
        conn.disconnect("Unexpected packet in Status state");
    }
}

void StatusHandler::onDisconnect(const std::string& reason) {
    // Java: NetHandlerStatusServer.onDisconnect() — empty
    (void)reason;
}

// ─── LoginHandler ───────────────────────────────────────────────────────────
// Java reference: net.minecraft.server.network.NetHandlerLoginServer

LoginHandler::LoginHandler(MinecraftServer& server)
    : server_(server) {}

void LoginHandler::handlePacket(int32_t packetId,
                                 const uint8_t* data,
                                 size_t length,
                                 Connection& conn) {
    if (packetId == LoginPacket::LoginStart) {
        // Java reference: NetHandlerLoginServer.processLoginStart()
        auto nameResult = readString(data, length);
        playerName_ = nameResult.value;

        std::cout << "[Login] Player '" << playerName_ << "' attempting to join from "
                  << conn.getRemoteAddress() << "\n";

        // Java reference: if (server.isServerInOnlineMode() && !networkManager.isLocalChannel())
        //   → send S01PacketEncryptionRequest, state = KEY
        // else → state = READY_TO_ACCEPT
        // Currently: offline mode only (no encryption)

        // Generate offline UUID
        // Java reference: NetHandlerLoginServer.getOfflineProfile()
        // UUID.nameUUIDFromBytes(("OfflinePlayer:" + name).getBytes(UTF_8))
        std::string uuid = generateOfflineUUID(playerName_);

        // Send S02PacketLoginSuccess
        // Java reference: NetHandlerLoginServer.func_147326_c()
        std::vector<uint8_t> success;
        writeVarInt(success, LoginPacket::LoginSuccess);
        writeString(success, uuid);
        writeString(success, playerName_);
        conn.sendPacket(std::move(success));

        conn.setState(ConnectionState::Play);
        std::cout << "[Login] Player '" << playerName_ << "' logged in (offline mode)"
                  << " UUID=" << uuid << "\n";

        // Transition to PlayHandler
        // Java: server.getConfigurationManager().initializeConnectionToPlayer(networkManager, player)
        auto playHandler = std::make_shared<PlayHandler>(server_, playerName_, uuid, conn);
        conn.setHandler(playHandler);

        // Send initial login sequence (Join Game, Spawn Position, Abilities, Position)
        playHandler->sendLoginSequence(conn);

    } else if (packetId == LoginPacket::EncryptionResponse) {
        // Java reference: NetHandlerLoginServer.processEncryptionResponse()
        // TODO: Implement when OpenSSL encryption is added
        std::cout << "[Login] Encryption response received (not yet implemented)\n";
        conn.disconnect("Encryption not yet supported");

    } else {
        conn.disconnect("Unexpected packet in Login state");
    }
}

void LoginHandler::onDisconnect(const std::string& reason) {
    // Java reference: NetHandlerLoginServer.onDisconnect()
    if (!playerName_.empty()) {
        std::cout << "[Login] " << playerName_ << " lost connection: " << reason << "\n";
    }
}

std::string LoginHandler::generateOfflineUUID(const std::string& playerName) {
    // Java reference: NetHandlerLoginServer.getOfflineProfile()
    // UUID = UUID.nameUUIDFromBytes(("OfflinePlayer:" + name).getBytes(Charsets.UTF_8))
    return nameUUIDFromBytes("OfflinePlayer:" + playerName);
}

// ─── PlayHandler ────────────────────────────────────────────────────────────
// Java reference: net.minecraft.network.play.server.NetHandlerPlayServer
// Handles all Play-state packets after login success.

namespace {

// Big-endian write helpers for protocol data types
inline void writeInt(std::vector<uint8_t>& out, int32_t v) {
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >>  8) & 0xFF));
    out.push_back(static_cast<uint8_t>( v        & 0xFF));
}

inline void writeLong(std::vector<uint8_t>& out, int64_t v) {
    for (int i = 56; i >= 0; i -= 8) {
        out.push_back(static_cast<uint8_t>((v >> i) & 0xFF));
    }
}

inline void writeFloat(std::vector<uint8_t>& out, float v) {
    uint32_t bits;
    std::memcpy(&bits, &v, 4);
    writeInt(out, static_cast<int32_t>(bits));
}

inline void writeDouble(std::vector<uint8_t>& out, double v) {
    uint64_t bits;
    std::memcpy(&bits, &v, 8);
    writeLong(out, static_cast<int64_t>(bits));
}

inline void writeByte(std::vector<uint8_t>& out, uint8_t v) {
    out.push_back(v);
}

inline void writeUByte(std::vector<uint8_t>& out, uint8_t v) {
    out.push_back(v);
}

inline void writeBool(std::vector<uint8_t>& out, bool v) {
    out.push_back(v ? 1 : 0);
}

inline void writeShort(std::vector<uint8_t>& out, int16_t v) {
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>( v       & 0xFF));
}

// Big-endian read helpers
inline int32_t readInt(const uint8_t* data) {
    return (static_cast<int32_t>(data[0]) << 24) |
           (static_cast<int32_t>(data[1]) << 16) |
           (static_cast<int32_t>(data[2]) <<  8) |
            static_cast<int32_t>(data[3]);
}

inline double readDouble(const uint8_t* data) {
    uint64_t bits = 0;
    for (int i = 0; i < 8; ++i) {
        bits = (bits << 8) | data[i];
    }
    double v;
    std::memcpy(&v, &bits, 8);
    return v;
}

inline float readFloat(const uint8_t* data) {
    uint32_t bits = (static_cast<uint32_t>(data[0]) << 24) |
                     (static_cast<uint32_t>(data[1]) << 16) |
                     (static_cast<uint32_t>(data[2]) <<  8) |
                      static_cast<uint32_t>(data[3]);
    float v;
    std::memcpy(&v, &bits, 4);
    return v;
}

} // anonymous namespace

PlayHandler::PlayHandler(MinecraftServer& server, const std::string& playerName,
                         const std::string& uuid, Connection& /*conn*/)
    : server_(server)
    , playerName_(playerName)
    , uuid_(uuid)
{
    // Default spawn position for superflat world
    playerX_ = 0.5;
    playerY_ = 4.0;  // Above superflat surface (bedrock=0, dirt=1-2, grass=3)
    playerZ_ = 0.5;
    playerYaw_ = 0.0f;
    playerPitch_ = 0.0f;
}

void PlayHandler::sendLoginSequence(Connection& conn) {
    // Java reference: ServerConfigurationManager.initializeConnectionToPlayer()
    // + NetHandlerPlayServer constructor

    // 1. S01PacketJoinGame (0x01)
    // Format: Int entityID, UByte gamemode, Byte dimension, UByte difficulty,
    //         UByte maxPlayers, String levelType
    {
        std::vector<uint8_t> pkt;
        writeVarInt(pkt, ClientboundPacket::JoinGame);
        writeInt(pkt, 0);          // Entity ID (0 for first player)
        writeUByte(pkt, 0);        // Gamemode: 0 = Survival
        writeByte(pkt, 0);         // Dimension: 0 = Overworld
        writeUByte(pkt, 1);        // Difficulty: 1 = Easy
        writeUByte(pkt, static_cast<uint8_t>(server_.getMaxPlayers())); // Max players
        writeString(pkt, "flat");  // Level type: flat (superflat)
        conn.sendPacket(std::move(pkt));
    }

    // 2. S05PacketSpawnPosition (0x05)
    // Format: Int x, Int y, Int z
    {
        std::vector<uint8_t> pkt;
        writeVarInt(pkt, ClientboundPacket::SpawnPosition);
        writeInt(pkt, 0);   // Spawn X
        writeInt(pkt, 4);   // Spawn Y (above surface)
        writeInt(pkt, 0);   // Spawn Z
        conn.sendPacket(std::move(pkt));
    }

    // 3. S39PacketPlayerAbilities (0x39)
    // Format: Byte flags, Float flySpeed, Float walkSpeed
    // Flags: 0x01=invulnerable, 0x02=flying, 0x04=allowFlying, 0x08=creativeMode
    {
        std::vector<uint8_t> pkt;
        writeVarInt(pkt, ClientboundPacket::PlayerAbilities);
        writeByte(pkt, 0x00);          // Flags: survival, not flying
        writeFloat(pkt, 0.05f);        // Fly speed
        writeFloat(pkt, 0.1f);         // Walk speed (FOV modifier)
        conn.sendPacket(std::move(pkt));
    }

    // 4. S08PacketPlayerPosLook (0x08)
    // Format: Double x, Double y, Double z, Float yaw, Float pitch, Bool onGround
    {
        std::vector<uint8_t> pkt;
        writeVarInt(pkt, ClientboundPacket::PlayerPosAndLook);
        writeDouble(pkt, playerX_);     // X
        writeDouble(pkt, playerY_);     // Y
        writeDouble(pkt, playerZ_);     // Z
        writeFloat(pkt, playerYaw_);    // Yaw
        writeFloat(pkt, playerPitch_);  // Pitch
        writeBool(pkt, false);          // On ground
        conn.sendPacket(std::move(pkt));
    }

    std::cout << "[Play] " << playerName_ << " joined the game at ("
              << playerX_ << ", " << playerY_ << ", " << playerZ_ << ")\n";
}

void PlayHandler::handlePacket(int32_t packetId,
                                const uint8_t* data,
                                size_t length,
                                Connection& conn) {
    switch (packetId) {
        case ServerboundPacket::KeepAlive:
            handleKeepAlive(data, length, conn);
            break;
        case ServerboundPacket::ChatMessage:
            handleChatMessage(data, length, conn);
            break;
        case ServerboundPacket::Player:
            handlePlayerGround(data, length, conn);
            break;
        case ServerboundPacket::PlayerPosition:
            handlePlayerPosition(data, length, conn);
            break;
        case ServerboundPacket::PlayerLook:
            handlePlayerLook(data, length, conn);
            break;
        case ServerboundPacket::PlayerPosAndLook:
            handlePlayerPosAndLook(data, length, conn);
            break;
        case ServerboundPacket::ClientSettings:
            handleClientSettings(data, length, conn);
            break;
        case ServerboundPacket::PluginMessage:
            // Java: NetHandlerPlayServer.processVanilla250Packet()
            // Silently consume for now (Forge sends MC|Brand)
            break;
        case ServerboundPacket::PlayerAbilities:
            // Java: NetHandlerPlayServer.processPlayerAbilities()
            // Client sends flying state — accept silently for now
            break;
        case ServerboundPacket::HeldItemChange:
        case ServerboundPacket::Animation:
        case ServerboundPacket::EntityAction:
        case ServerboundPacket::PlayerDigging:
        case ServerboundPacket::PlayerBlockPlace:
        case ServerboundPacket::CloseWindow:
        case ServerboundPacket::ClickWindow:
        case ServerboundPacket::ConfirmTransaction:
        case ServerboundPacket::CreativeInventory:
        case ServerboundPacket::UpdateSign:
        case ServerboundPacket::UseEntity:
        case ServerboundPacket::SteerVehicle:
        case ServerboundPacket::TabComplete:
        case ServerboundPacket::EnchantItem:
        case ServerboundPacket::ClientStatus:
            // Silently consume unimplemented packets
            break;
        default:
            std::cerr << "[Play] Unknown packet 0x" << std::hex << packetId
                      << std::dec << " from " << playerName_ << "\n";
            break;
    }
}

void PlayHandler::onDisconnect(const std::string& reason) {
    std::cout << "[Play] " << playerName_ << " disconnected: " << reason << "\n";
}

void PlayHandler::sendKeepAlive(Connection& conn) {
    // Java reference: NetHandlerPlayServer.update() — sends S00PacketKeepAlive
    // Format: VarInt keepAliveId
    ++lastKeepAliveId_;
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::KeepAlive);
    writeVarInt(pkt, lastKeepAliveId_);
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendChatMessage(Connection& conn, const std::string& message) {
    // Java reference: S02PacketChat
    // Format: String jsonData
    std::string json = R"({"text":")" + message + R"("})";
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::ChatMessage);
    writeString(pkt, json);
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::handleKeepAlive(const uint8_t* data, size_t length, Connection& /*conn*/) {
    // Java reference: NetHandlerPlayServer.processKeepAlive()
    // Client echoes back the keepAlive ID we sent
    if (length < 1) return;
    auto result = readVarInt(data, length);
    int id = result.value;
    if (id == lastKeepAliveId_) {
        ticksSinceLastKeepAlive_ = 0;
    }
}

void PlayHandler::handleChatMessage(const uint8_t* data, size_t length, Connection& conn) {
    // Java reference: NetHandlerPlayServer.processChatMessage()
    if (length < 1) return;
    auto result = readString(data, length);
    std::string message = result.value;

    // Max chat length = 100 characters in 1.7.10
    if (message.size() > 100) {
        conn.disconnect("Chat message too long");
        return;
    }

    std::cout << "[Chat] <" << playerName_ << "> " << message << "\n";

    // Broadcast to all connections (echo format: <PlayerName> message)
    // For now, just echo back to the sender
    std::string formatted = "<" + playerName_ + "> " + message;
    sendChatMessage(conn, formatted);
}

void PlayHandler::handlePlayerPosition(const uint8_t* data, size_t length, Connection& /*conn*/) {
    // Java reference: NetHandlerPlayServer.processPlayer()
    // C04PacketPlayerPosition: Double x, Double y, Double stance, Double z, Bool onGround
    // Note: 1.7.10 sends BOTH y (feet) AND stance (head) — 33 bytes total
    if (length < 33) return;
    playerX_ = readDouble(data);
    playerY_ = readDouble(data + 8);
    // stance = readDouble(data + 16) — head Y, not stored separately
    playerZ_ = readDouble(data + 24);
    playerOnGround_ = data[32] != 0;
}

void PlayHandler::handlePlayerLook(const uint8_t* data, size_t length, Connection& /*conn*/) {
    // Java reference: NetHandlerPlayServer.processPlayer()
    // C05PacketPlayerLook: Float yaw, Float pitch, Bool onGround
    if (length < 9) return;
    playerYaw_ = readFloat(data);
    playerPitch_ = readFloat(data + 4);
    playerOnGround_ = data[8] != 0;
}

void PlayHandler::handlePlayerPosAndLook(const uint8_t* data, size_t length, Connection& /*conn*/) {
    // Java reference: NetHandlerPlayServer.processPlayer()
    // C06PacketPlayerPosLook: Double x, Double y, Double stance, Double z, Float yaw, Float pitch, Bool onGround
    if (length < 41) return;
    playerX_ = readDouble(data);
    playerY_ = readDouble(data + 8);
    // stance = readDouble(data + 16)
    playerZ_ = readDouble(data + 24);
    playerYaw_ = readFloat(data + 32);
    playerPitch_ = readFloat(data + 36);
    playerOnGround_ = data[40] != 0;
}

void PlayHandler::handlePlayerGround(const uint8_t* data, size_t length, Connection& /*conn*/) {
    // Java reference: NetHandlerPlayServer.processPlayer()
    // C03PacketPlayer: Bool onGround
    if (length < 1) return;
    playerOnGround_ = data[0] != 0;
}

void PlayHandler::handleClientSettings(const uint8_t* data, size_t length, Connection& /*conn*/) {
    // Java reference: NetHandlerPlayServer.processClientSettings()
    // C15PacketClientSettings: String locale, Byte viewDistance, Byte chatFlags,
    //                          Bool chatColours, Byte difficulty(?), Bool showCape
    if (length < 1) return;
    auto locale = readString(data, length);
    // Just log for now
    (void)locale;
}

} // namespace mccpp

