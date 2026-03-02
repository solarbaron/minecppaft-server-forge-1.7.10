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
#include "networking/PacketBuilder.h"
#include "networking/PacketReader.h"
#include "networking/PlayPackets.h"
#include "server/MinecraftServer.h"
#include "command/CommandSystem.h"
#include "types/VarInt.h"
#include "world/World.h"
#include "crafting/Crafting.h"

#include <cmath>
#include <random>

#include <algorithm>
#include <array>
#include <climits>
#include <cstring>
#include <functional>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <zlib.h>

#include "nbt/NBT.h"

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
        auto playHandler = std::make_shared<PlayHandler>(server_, playerName_, uuid);
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

// Static entity ID counter — Java reference: Entity.nextEntityID
std::atomic<int32_t> PlayHandler::nextEntityId_{1}; // Start at 1 (0 is reserved)

PlayHandler::PlayHandler(MinecraftServer& server, const std::string& name,
                         const std::string& uuid)
    : server_(server)
    , playerName_(name)
    , uuid_(uuid)
{
    entityId_ = nextEntityId_.fetch_add(1, std::memory_order_relaxed);
    // Default spawn position for superflat world
    playerX_ = 0.5;
    playerY_ = 4.0;  // Above superflat surface (bedrock=0, dirt=1-2, grass=3)
    playerZ_ = 0.5;
    playerYaw_ = 0.0f;
    playerPitch_ = 0.0f;

    // Initialize player container (45 slots: crafting out + grid + armor + main + hotbar)
    // Java reference: EntityPlayerMP constructor → ContainerPlayer
    container_ = std::make_unique<ContainerPlayer>(inventory_);
}

void PlayHandler::sendLoginSequence(Connection& conn) {
    // Java reference: ServerConfigurationManager.initializeConnectionToPlayer()
    // CRITICAL ORDER: chunks must arrive BEFORE PlayerPosAndLook
    // so the client has terrain loaded before the player starts falling.

    // Load saved player data (position, rotation) — modifies playerX_/Y_/Z_ etc.
    loadPlayerData();

    // 1. S01PacketJoinGame (0x01)
    {
        std::vector<uint8_t> pkt;
        writeVarInt(pkt, ClientboundPacket::JoinGame);
        writeInt(pkt, entityId_);  // Entity ID (unique per player)
        writeUByte(pkt, static_cast<uint8_t>(gameMode_)); // Gamemode
        writeByte(pkt, 0);         // Dimension: 0 = Overworld
        writeUByte(pkt, 1);        // Difficulty: 1 = Easy
        writeUByte(pkt, static_cast<uint8_t>(server_.getMaxPlayers()));
        writeString(pkt, "flat");  // Level type
        conn.sendPacket(std::move(pkt));
    }

    // 2. S05PacketSpawnPosition (0x05)
    {
        std::vector<uint8_t> pkt;
        writeVarInt(pkt, ClientboundPacket::SpawnPosition);
        writeInt(pkt, 0);   // Spawn X
        writeInt(pkt, 4);   // Spawn Y
        writeInt(pkt, 0);   // Spawn Z
        conn.sendPacket(std::move(pkt));
    }

    // 3. S39PacketPlayerAbilities (0x39)
    {
        std::vector<uint8_t> pkt;
        writeVarInt(pkt, ClientboundPacket::PlayerAbilities);
        writeByte(pkt, 0x00);          // Flags: survival, not flying
        writeFloat(pkt, 0.05f);        // Fly speed
        writeFloat(pkt, 0.1f);         // Walk speed (FOV modifier)
        conn.sendPacket(std::move(pkt));
    }

    // 4. Send chunk data around spawn BEFORE position
    // Java reference: ServerConfigurationManager sends MapChunkBulk before S08
    int playerChunkX = static_cast<int>(playerX_) >> 4;
    int playerChunkZ = static_cast<int>(playerZ_) >> 4;
    constexpr int VIEW_RADIUS = 2; // 5x5 = 25 chunks (fast initial load)

    auto* overworld = server_.getWorlds().empty() ? nullptr : server_.getWorlds()[0].get();
    int chunksSent = 0;
    if (overworld) {
        for (int cx = playerChunkX - VIEW_RADIUS; cx <= playerChunkX + VIEW_RADIUS; ++cx) {
            for (int cz = playerChunkZ - VIEW_RADIUS; cz <= playerChunkZ + VIEW_RADIUS; ++cz) {
                Chunk* chunk = overworld->getChunkFromChunkCoords(cx, cz);
                if (chunk) {
                    sendChunkData(conn, chunk);
                    loadedChunks_.insert({cx, cz});
                    ++chunksSent;
                }
            }
        }
    }
    lastChunkX_ = playerChunkX;
    lastChunkZ_ = playerChunkZ;

    // 5. S08PacketPlayerPosLook (0x08) — AFTER chunks so client has terrain
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
              << playerX_ << ", " << playerY_ << ", " << playerZ_ << ") — "
              << chunksSent << " chunks sent (entityId=" << entityId_ << ")\n";

    // 6. S06PacketUpdateHealth — initial health/food/saturation
    // Java reference: EntityPlayer defaults: health=20, foodLevel=20, saturation=5.0f
    sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());

    // 7. S1FPacketSetExperience — initial XP (bar=0, level=0, total=0)
    // Java reference: EntityPlayerMP.onNewPotionEffect sends this on join
    sendSetExperience(conn, experienceBar_, experienceLevel_, experienceTotal_);

    // 8. S30PacketWindowItems — send initial inventory contents
    // Java reference: EntityPlayerMP.onNewPotionEffect / initializeConnectionToPlayer
    sendWindowItems(conn);

    // ─── Player visibility broadcasts ─────────────────────────────────
    // Java reference: ServerConfigurationManager.playerLoggedIn()
    // Send this player's info to all existing players, and send existing
    // players' info to this player.
    server_.onPlayerJoined(conn, *this);
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
            handleHeldItemChange(data, length, conn);
            break;
        case ServerboundPacket::Animation:
            handleAnimation(data, length, conn);
            break;
        case ServerboundPacket::EntityAction:
            handleEntityAction(data, length, conn);
            break;
        case ServerboundPacket::PlayerDigging:
            handlePlayerDigging(data, length, conn);
            break;
        case ServerboundPacket::PlayerBlockPlace:
            handlePlayerBlockPlace(data, length, conn);
            break;
        case ServerboundPacket::CloseWindow:
            // Close any open container window (workbench, chest, etc.)
            if (openWindowId_ > 0) {
                closeOpenWindow(conn);
                break;
            }
            // Drop cursor item if any — Java: Container.onContainerClosed
            if (cursorItem_) {
                server_.spawnItemDrop(playerX_, playerY_ + 1.5, playerZ_,
                    cursorItem_->getItemId(), cursorItem_->getDamage(), cursorItem_->getStackSize());
                cursorItem_ = std::nullopt;
            }
            // Drop crafting grid items (slots 1-4) — Java: ContainerPlayer.onContainerClosed
            if (container_) {
                for (int32_t i = 1; i <= 4; ++i) {
                    Slot* gridSlot = container_->getSlot(i);
                    if (!gridSlot) continue;
                    auto gridStack = gridSlot->getStack();
                    if (gridStack) {
                        server_.spawnItemDrop(playerX_, playerY_ + 1.5, playerZ_,
                            gridStack->getItemId(), gridStack->getDamage(), gridStack->getStackSize());
                        gridSlot->putStack(std::nullopt);
                    }
                }
                // Clear crafting output
                Slot* outputSlot = container_->getSlot(0);
                if (outputSlot) outputSlot->putStack(std::nullopt);
            }
            break;
        case ServerboundPacket::ClickWindow:
            handleClickWindow(data, length, conn);
            break;
        case ServerboundPacket::ConfirmTransaction:
            break;
        case ServerboundPacket::UpdateSign: {
            // Java: NetHandlerPlayServer.processUpdateSign()
            // Parse C12 UpdateSign: x(int), y(short), z(int), line1-4(string)
            if (length >= 14) {
                PacketReader reader(data, length);
                int32_t sx = reader.readInt();
                int16_t sy = reader.readShort();
                int32_t sz = reader.readInt();
                std::string l1 = reader.readString(15);
                std::string l2 = reader.readString(15);
                std::string l3 = reader.readString(15);
                std::string l4 = reader.readString(15);
                server_.setSignText(sx, sy, sz, l1, l2, l3, l4);
            }
            break;
        }
        case ServerboundPacket::CreativeInventory:
            handleCreativeInventory(data, length, conn);
            break;
        case ServerboundPacket::UseEntity:
            handleUseEntity(data, length, conn);
            break;
        case ServerboundPacket::ClientStatus:
            handleClientStatus(data, length, conn);
            break;
        case ServerboundPacket::TabComplete:
            handleTabComplete(data, length, conn);
            break;
        case ServerboundPacket::SteerVehicle:
        case ServerboundPacket::EnchantItem:
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
    // Save player data before removing from server
    savePlayerData();
    // Notify all other players this player left
    server_.onPlayerLeft(*this);
}

void PlayHandler::sendKeepAlive(Connection& conn) {
    // Java reference: NetHandlerPlayServer.update() → S00PacketKeepAlive
    // S00PacketKeepAlive.writePacketData: packetBuffer.writeInt(keepAliveId)
    // Format: Int keepAliveId (NOT VarInt!)
    ++lastKeepAliveId_;
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::KeepAlive);
    writeInt(pkt, lastKeepAliveId_);  // Must be Int, not VarInt!
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

void PlayHandler::sendChunkData(Connection& conn, Chunk* chunk) {
    // Java reference: S21PacketChunkData
    // Protocol v5 format:
    //   Int chunkX, Int chunkZ
    //   Bool groundUpContinuous (true = full column)
    //   Short primaryBitMap (which sections have data)
    //   Short addBitMap (which sections have add data - block IDs > 255)
    //   Int compressedSize
    //   Byte[] compressedData
    //
    // Per-section data order (for each bit set in primaryBitMap):
    //   Block LSB array: 4096 bytes (16*16*16)
    // Then per-section:
    //   Metadata nibble: 2048 bytes
    // Then per-section:
    //   Block light nibble: 2048 bytes
    // Then per-section:
    //   Sky light nibble: 2048 bytes
    // Then if addBitMap bit:
    //   Add block ID nibble: 2048 bytes
    // Finally:
    //   Biome data: 256 bytes (only if groundUpContinuous)

    if (!chunk) return;

    // Calculate section bitmask
    uint16_t primaryBitMap = 0;
    uint16_t addBitMap = 0;
    for (int i = 0; i < 16; ++i) {
        if (chunk->sections[i] && !chunk->sections[i]->isEmpty()) {
            primaryBitMap |= (1 << i);
            if (chunk->sections[i]->getBlockMSBArray()) {
                addBitMap |= (1 << i);
            }
        }
    }

    // Build uncompressed data buffer
    // Size calculation: for each section in primaryBitMap:
    //   4096 (LSB) + 2048 (meta) + 2048 (blocklight) + 2048 (skylight) = 10240
    //   + 2048 (add) if in addBitMap
    // + 256 (biomes)
    int sectionCount = 0;
    int addCount = 0;
    for (int i = 0; i < 16; ++i) {
        if (primaryBitMap & (1 << i)) ++sectionCount;
        if (addBitMap & (1 << i)) ++addCount;
    }
    size_t uncompressedSize = static_cast<size_t>(sectionCount) * 10240
                           + static_cast<size_t>(addCount) * 2048
                           + 256; // biomes

    std::vector<uint8_t> uncompressed;
    uncompressed.reserve(uncompressedSize);

    // 1. Block LSB arrays (4096 bytes each)
    for (int i = 0; i < 16; ++i) {
        if (!(primaryBitMap & (1 << i))) continue;
        const auto& lsb = chunk->sections[i]->getBlockLSBArray();
        uncompressed.insert(uncompressed.end(), lsb.begin(), lsb.end());
    }

    // 2. Metadata nibble arrays (2048 bytes each)
    for (int i = 0; i < 16; ++i) {
        if (!(primaryBitMap & (1 << i))) continue;
        const auto& meta = chunk->sections[i]->getMetadataArray().data;
        uncompressed.insert(uncompressed.end(), meta.begin(), meta.end());
    }

    // 3. Block light nibble arrays (2048 bytes each)
    for (int i = 0; i < 16; ++i) {
        if (!(primaryBitMap & (1 << i))) continue;
        const auto& bl = chunk->sections[i]->getBlocklightArray().data;
        uncompressed.insert(uncompressed.end(), bl.begin(), bl.end());
    }

    // 4. Sky light nibble arrays (2048 bytes each)
    for (int i = 0; i < 16; ++i) {
        if (!(primaryBitMap & (1 << i))) continue;
        auto* sl = chunk->sections[i]->getSkylightArray();
        if (sl) {
            uncompressed.insert(uncompressed.end(), sl->data.begin(), sl->data.end());
        } else {
            // No skylight — send all 0xFF (full brightness, like overworld default)
            uncompressed.insert(uncompressed.end(), 2048, 0xFF);
        }
    }

    // 5. Add block ID nibble arrays (2048 bytes each)
    for (int i = 0; i < 16; ++i) {
        if (!(addBitMap & (1 << i))) continue;
        auto* msb = chunk->sections[i]->getBlockMSBArray();
        if (msb) {
            uncompressed.insert(uncompressed.end(), msb->data.begin(), msb->data.end());
        }
    }

    // 6. Biome data (256 bytes)
    uncompressed.insert(uncompressed.end(), chunk->biomes.begin(), chunk->biomes.end());

    // Compress with zlib (deflate)
    // Java uses Deflater with default compression
    std::vector<uint8_t> compressed(uncompressed.size() + 256); // extra space for zlib header
    uLongf compressedLen = static_cast<uLongf>(compressed.size());
    int zret = compress2(compressed.data(), &compressedLen,
                         uncompressed.data(), static_cast<uLong>(uncompressed.size()),
                         Z_BEST_SPEED);  // Use fast compression to avoid blocking
    if (zret != Z_OK) {
        std::cerr << "[Play] Failed to compress chunk data for ("
                  << chunk->xPosition << ", " << chunk->zPosition << ")\n";
        return;
    }
    compressed.resize(static_cast<size_t>(compressedLen));

    // Build packet: 0x21 ChunkData
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::ChunkData);
    writeInt(pkt, chunk->xPosition);             // Chunk X
    writeInt(pkt, chunk->zPosition);             // Chunk Z
    writeBool(pkt, true);                         // Ground-up continuous
    writeShort(pkt, static_cast<int16_t>(primaryBitMap)); // Primary bit mask
    writeShort(pkt, static_cast<int16_t>(addBitMap));     // Add bit mask
    writeInt(pkt, static_cast<int32_t>(compressed.size())); // Compressed data length
    pkt.insert(pkt.end(), compressed.begin(), compressed.end()); // Compressed data

    conn.sendPacket(std::move(pkt));
}

void PlayHandler::handleKeepAlive(const uint8_t* data, size_t length, Connection& /*conn*/) {
    // Java reference: NetHandlerPlayServer.processKeepAlive()
    // C00PacketKeepAlive.readPacketData: packetBuffer.readInt()
    // Client echoes back the keepAlive ID we sent as Int (NOT VarInt!)
    if (length < 4) return;
    int32_t id = readInt(data);
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

    if (!message.empty() && message[0] == '/') {
        // Java: NetHandlerPlayServer.processChatMessage() -> handleSlashCommand()
        // Dispatch command via CommandHandler
        std::cout << "[Command] " << playerName_ << " issued: " << message << "\n";

        // Create a PlayerCommandSender that routes addChatMessage() back to this client
        // Java: EntityPlayerMP implements ICommandSender
        class PlayerCommandSender : public ICommandSender {
        public:
            PlayerCommandSender(PlayHandler& handler, Connection& conn, const std::string& name,
                                MinecraftServer& server)
                : handler_(handler), conn_(conn), name_(name), server_(server) {}
            std::string getCommandSenderName() const override { return name_; }
            void addChatMessage(const std::string& msg) override {
                handler_.sendChatMessage(conn_, msg);
            }
            bool canCommandSenderUseCommand(int32_t /*permLevel*/, const std::string& /*cmd*/) const override {
                return true; // All players are ops in offline mode for now
            }
            MinecraftServer* getServer() const override { return &server_; }
        private:
            PlayHandler& handler_;
            Connection& conn_;
            std::string name_;
            MinecraftServer& server_;
        };

        PlayerCommandSender sender(*this, conn, playerName_, server_);
        server_.getCommandHandler().executeCommand(sender, message);
    } else {
        // Regular chat message — broadcast to all players
        std::cout << "[Chat] <" << playerName_ << "> " << message << "\n";
        std::string formatted = "<" + playerName_ + "> " + message;
        server_.broadcastChatMessage(formatted);
    }
}

void PlayHandler::sendTimeUpdate(Connection& conn, int64_t worldAge, int64_t timeOfDay) {
    // Java reference: S03PacketTimeUpdate
    // Format: Long worldAge, Long timeOfDay
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::TimeUpdate);
    writeLong(pkt, worldAge);
    writeLong(pkt, timeOfDay);
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendUpdateHealth(Connection& conn, float health, int32_t food, float saturation) {
    // Java reference: S06PacketUpdateHealth.writePacketData()
    // Format: Float health, Short food, Float foodSaturation
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::UpdateHealth);
    writeFloat(pkt, health);
    writeShort(pkt, static_cast<int16_t>(food));
    writeFloat(pkt, saturation);
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendBlockChange(Connection& conn, int32_t x, int32_t y, int32_t z,
                                    int32_t blockId, int32_t metadata) {
    // Java reference: S23PacketBlockChange
    // Format: Int x, UByte y, Int z, VarInt blockType, UByte metadata
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::BlockChange);
    writeInt(pkt, x);
    writeByte(pkt, static_cast<uint8_t>(y));
    writeInt(pkt, z);
    writeVarInt(pkt, blockId);
    writeByte(pkt, static_cast<uint8_t>(metadata));
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendPlayerListItem(Connection& conn, const std::string& playerName,
                                       bool online, int16_t ping) {
    // Java reference: S38PacketPlayerListItem
    // Format: String playerName, Boolean online, Short ping  
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::PlayerListItem);
    writeString(pkt, playerName);
    writeBool(pkt, online);
    writeShort(pkt, ping);
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendSpawnPlayer(Connection& conn, int32_t entityId,
                                    const std::string& uuid, const std::string& name,
                                    double x, double y, double z,
                                    float yaw, float pitch, int16_t heldItem) {
    // Java reference: S0CPacketSpawnPlayer.writePacketData()
    // Format:
    //   VarInt entityId
    //   String uuid (36 chars, e.g. "550e8400-e29b-41d4-a716-446655440000")
    //   String playerName (16 chars max)
    //   VarInt propertyCount (0 = no auth properties in offline mode)
    //   Int x (fixed-point: x * 32)
    //   Int y (fixed-point: y * 32)
    //   Int z (fixed-point: z * 32)
    //   Byte yaw (angle: yaw * 256 / 360)
    //   Byte pitch (angle: pitch * 256 / 360)
    //   Short currentItem (held item ID, 0 = none)
    //   DataWatcher metadata (0x7F = end marker, minimal)
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::SpawnPlayer);
    writeVarInt(pkt, entityId);
    writeString(pkt, uuid);     // UUID as string
    writeString(pkt, name);     // player name
    writeVarInt(pkt, 0);        // property count (offline mode = 0)

    // Fixed-point position: Java MathHelper.floor_double(pos * 32.0)
    writeInt(pkt, static_cast<int32_t>(std::floor(x * 32.0)));
    writeInt(pkt, static_cast<int32_t>(std::floor(y * 32.0)));
    writeInt(pkt, static_cast<int32_t>(std::floor(z * 32.0)));

    // Angle conversion: Java (byte)(rotation * 256.0f / 360.0f)
    writeByte(pkt, static_cast<uint8_t>(static_cast<int>(yaw * 256.0f / 360.0f) & 0xFF));
    writeByte(pkt, static_cast<uint8_t>(static_cast<int>(pitch * 256.0f / 360.0f) & 0xFF));

    writeShort(pkt, heldItem);  // current item

    // DataWatcher metadata — minimal: just the end marker
    // Java: DataWatcher.func_151509_a(buffer) writes watched objects then 0x7F
    writeByte(pkt, 0x7F);  // End of metadata

    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendDestroyEntities(Connection& conn, const std::vector<int32_t>& entityIds) {
    // Java reference: S13PacketDestroyEntities.writePacketData()
    // Format: Byte count, Int[] entityIds
    if (entityIds.empty()) return;
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::DestroyEntities);
    writeByte(pkt, static_cast<uint8_t>(entityIds.size()));
    for (int32_t id : entityIds) {
        writeInt(pkt, id);
    }
    conn.sendPacket(std::move(pkt));
}

// ─── ItemStack serialization helper ──────────────────────────────────────
// Java reference: PacketBuffer.writeItemStackToBuffer(ItemStack)
namespace {
void writeItemStack(std::vector<uint8_t>& pkt, const std::optional<ItemStack>& stack) {
    if (!stack || stack->isEmpty()) {
        writeShort(pkt, -1);  // null item
    } else {
        writeShort(pkt, static_cast<int16_t>(stack->getItemId()));
        writeByte(pkt, static_cast<uint8_t>(stack->getStackSize()));
        writeShort(pkt, static_cast<int16_t>(stack->getDamage()));

        if (stack->hasEnchantments()) {
            // Write NBT compound with enchantment list
            // NBT format: TAG_Compound (unnamed root) → TAG_List "ench" → TAG_Compound entries
            std::vector<uint8_t> nbt;
            // Root compound tag (type 10, unnamed)
            nbt.push_back(10); // TAG_Compound
            nbt.push_back(0); nbt.push_back(0); // empty name length

            // TAG_List named "ench"
            nbt.push_back(9); // TAG_List type
            nbt.push_back(0); nbt.push_back(4); // name length = 4
            nbt.push_back('e'); nbt.push_back('n'); nbt.push_back('c'); nbt.push_back('h');
            nbt.push_back(10); // list element type = TAG_Compound
            auto& enchants = stack->getEnchantments();
            int32_t count = static_cast<int32_t>(enchants.size());
            nbt.push_back((count >> 24) & 0xFF);
            nbt.push_back((count >> 16) & 0xFF);
            nbt.push_back((count >> 8) & 0xFF);
            nbt.push_back(count & 0xFF);
            for (auto& e : enchants) {
                // TAG_Short "id"
                nbt.push_back(2); // TAG_Short
                nbt.push_back(0); nbt.push_back(2); // name length = 2
                nbt.push_back('i'); nbt.push_back('d');
                nbt.push_back((e.id >> 8) & 0xFF);
                nbt.push_back(e.id & 0xFF);
                // TAG_Short "lvl"
                nbt.push_back(2); // TAG_Short
                nbt.push_back(0); nbt.push_back(3); // name length = 3
                nbt.push_back('l'); nbt.push_back('v'); nbt.push_back('l');
                nbt.push_back((e.level >> 8) & 0xFF);
                nbt.push_back(e.level & 0xFF);
                // End compound
                nbt.push_back(0); // TAG_End
            }
            // End root compound
            nbt.push_back(0); // TAG_End

            // Write NBT length as short, then compressed data
            // Java uses gzip compressed NBT in slot data
            // Protocol 1.7.10: Short length + gzip data, or Short -1 for no tag
            // Actually, protocol 1.7.10 uses: Short nbtLength + gzip(nbt) or Short -1
            std::vector<uint8_t> compressed;
            compressed.resize(nbt.size() + 64);
            z_stream zs{};
            deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
            zs.avail_in = static_cast<uInt>(nbt.size());
            zs.next_in = nbt.data();
            zs.avail_out = static_cast<uInt>(compressed.size());
            zs.next_out = compressed.data();
            deflate(&zs, Z_FINISH);
            compressed.resize(zs.total_out);
            deflateEnd(&zs);

            writeShort(pkt, static_cast<int16_t>(compressed.size()));
            pkt.insert(pkt.end(), compressed.begin(), compressed.end());
        } else {
            // No NBT tag
            writeShort(pkt, -1);
        }
    }
}
} // anonymous namespace

void PlayHandler::sendWindowItems(Connection& conn) {
    // Java reference: S30PacketWindowItems.writePacketData()
    // Format: Byte windowId, Short slotCount, ItemStack[slotCount]
    if (!container_) return;

    int32_t slotCount = container_->getSlotCount();
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::WindowItems);
    writeByte(pkt, static_cast<uint8_t>(container_->windowId)); // window ID (0 = player inv)
    writeShort(pkt, static_cast<int16_t>(slotCount));

    for (int32_t i = 0; i < slotCount; ++i) {
        Slot* slot = container_->getSlot(i);
        if (slot) {
            writeItemStack(pkt, slot->getStack());
        } else {
            writeShort(pkt, -1); // empty slot
        }
    }

    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendSetSlot(Connection& conn, int8_t windowId, int16_t slot,
                               const std::optional<ItemStack>& stack) {
    // Java reference: S2FPacketSetSlot.writePacketData()
    // Format: Byte windowId, Short slot, ItemStack
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::SetSlot);
    writeByte(pkt, static_cast<uint8_t>(windowId));
    writeShort(pkt, slot);
    writeItemStack(pkt, stack);
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendOpenWindow(Connection& conn, int8_t windowId, int8_t windowType,
                                  const std::string& windowTitle, int8_t slotCount) {
    // Java reference: S2DPacketOpenWindow.writePacketData()
    // Format: UByte windowId, UByte invType, String windowTitle, UByte numSlots, Bool useProvidedTitle
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::OpenWindow);
    writeByte(pkt, static_cast<uint8_t>(windowId));
    writeByte(pkt, static_cast<uint8_t>(windowType));
    writeString(pkt, windowTitle);
    writeByte(pkt, static_cast<uint8_t>(slotCount));
    writeByte(pkt, 1); // useProvidedTitle = true
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::openWorkbench(Connection& conn, int32_t blockX, int32_t blockY, int32_t blockZ) {
    // Java: EntityPlayerMP.displayGUIWorkbench → S2D OpenWindow type 1 "minecraft:crafting_table"
    // Close any existing open window first
    if (openWindowId_ > 0) {
        closeOpenWindow(conn);
    }

    // Assign new window ID
    openWindowId_ = nextWindowId_++;
    if (nextWindowId_ > 100) nextWindowId_ = 1; // Wrap around
    openWindowType_ = 1; // crafting_table

    // Clear workbench grid
    for (int i = 0; i < 9; ++i) workbenchGrid_[i] = std::nullopt;
    workbenchResult_ = std::nullopt;

    // Send S2D OpenWindow (type 1 = workbench)
    sendOpenWindow(conn, openWindowId_, 1, "Crafting", 0);

    // Send empty window contents — 10 slots (result + 9 grid) + 36 player inv
    // Java: ContainerWorkbench has 46 slots total (0=result, 1-9=grid, 10-36=main, 37-45=hotbar)
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::WindowItems);
    writeByte(pkt, static_cast<uint8_t>(openWindowId_));
    writeShort(pkt, 46); // 46 slots total

    // Slot 0: crafting output (empty)
    writeShort(pkt, -1);
    // Slots 1-9: crafting grid (empty)
    for (int i = 0; i < 9; ++i) writeShort(pkt, -1);
    // Slots 10-36: main inventory (rows 1-3 of player inventory, slots 9-35)
    for (int i = 9; i < 36; ++i) {
        writeItemStack(pkt, inventory_.getStackInSlot(i));
    }
    // Slots 37-45: hotbar (player inventory slots 0-8)
    for (int i = 0; i < 9; ++i) {
        writeItemStack(pkt, inventory_.getStackInSlot(i));
    }

    conn.sendPacket(std::move(pkt));
}

void PlayHandler::closeOpenWindow(Connection& conn) {
    if (openWindowId_ <= 0) return;

    // Java: ContainerWorkbench.onContainerClosed — drop items from crafting grid
    if (openWindowType_ == 1) { // crafting_table
        for (int i = 0; i < 9; ++i) {
            if (workbenchGrid_[i]) {
                server_.spawnItemDrop(playerX_, playerY_ + 1.5, playerZ_,
                    workbenchGrid_[i]->getItemId(), workbenchGrid_[i]->getDamage(),
                    workbenchGrid_[i]->getStackSize());
                workbenchGrid_[i] = std::nullopt;
            }
        }
        workbenchResult_ = std::nullopt;
    }

    // Chest/Ender Chest: clear pointer + play close sound
    if (openWindowType_ == 0) {
        // Play chest close sound at the chest/ender chest position
        if (chestInventory_) {
            server_.broadcastSound("random.chestclose",
                static_cast<double>(openChestX_) + 0.5,
                static_cast<double>(openChestY_) + 0.5,
                static_cast<double>(openChestZ_) + 0.5,
                0.5f, 1.0f);
        }
        chestInventory_ = nullptr;
        isEnderChest_ = false;
    }

    // Furnace: clear pointer (items stay in furnace)
    if (openWindowType_ == 2) {
        furnaceData_ = nullptr;
        openFurnaceKey_ = 0;
    }

    // Drop cursor item
    if (cursorItem_) {
        server_.spawnItemDrop(playerX_, playerY_ + 1.5, playerZ_,
            cursorItem_->getItemId(), cursorItem_->getDamage(), cursorItem_->getStackSize());
        cursorItem_ = std::nullopt;
    }

    openWindowId_ = 0;
    openWindowType_ = -1;
}

void PlayHandler::openChest(Connection& conn, int32_t blockX, int32_t blockY, int32_t blockZ) {
    // Java: EntityPlayerMP.displayGUIChest → S2D OpenWindow type 0 "minecraft:container"
    if (openWindowId_ > 0) {
        closeOpenWindow(conn);
    }

    openWindowId_ = nextWindowId_++;
    if (nextWindowId_ > 100) nextWindowId_ = 1;
    openWindowType_ = 0; // generic container (chest)

    // Get or create chest storage
    chestInventory_ = &server_.getOrCreateChest(blockX, blockY, blockZ);
    openChestX_ = blockX; openChestY_ = blockY; openChestZ_ = blockZ;

    // Send S2D OpenWindow (type 0 = generic, 27 slots = 3 rows)
    sendOpenWindow(conn, openWindowId_, 0, "Chest", 27);

    // Send window contents — 63 slots (27 chest + 27 main inv + 9 hotbar)
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::WindowItems);
    writeByte(pkt, static_cast<uint8_t>(openWindowId_));
    writeShort(pkt, 63); // 63 slots total

    // Slots 0-26: chest contents
    for (int i = 0; i < 27; ++i) {
        writeItemStack(pkt, (*chestInventory_)[i]);
    }
    // Slots 27-53: main inventory (player slots 9-35)
    for (int i = 9; i < 36; ++i) {
        writeItemStack(pkt, inventory_.getStackInSlot(i));
    }
    // Slots 54-62: hotbar (player slots 0-8)
    for (int i = 0; i < 9; ++i) {
        writeItemStack(pkt, inventory_.getStackInSlot(i));
    }

    conn.sendPacket(std::move(pkt));

    // Play chest open sound
    server_.broadcastSound("random.chestopen",
        static_cast<double>(blockX) + 0.5,
        static_cast<double>(blockY) + 0.5,
        static_cast<double>(blockZ) + 0.5,
        0.5f, 1.0f);
}

void PlayHandler::openEnderChest(Connection& conn, int32_t blockX, int32_t blockY, int32_t blockZ) {
    // Java: BlockEnderChest.onBlockActivated() → InventoryEnderChest
    // Per-player storage, reuses the chest window handler
    if (openWindowId_ > 0) {
        closeOpenWindow(conn);
    }

    openWindowId_ = nextWindowId_++;
    if (nextWindowId_ > 100) nextWindowId_ = 1;
    openWindowType_ = 0; // generic container (same as regular chest)
    isEnderChest_ = true;

    // Point the chest handler at per-player ender chest storage
    chestInventory_ = &enderChestInventory_;

    // Send S2D OpenWindow (type 0 = generic, 27 slots = 3 rows)
    sendOpenWindow(conn, openWindowId_, 0, "Ender Chest", 27);

    // Send window contents — 63 slots (27 ender chest + 27 main inv + 9 hotbar)
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::WindowItems);
    writeByte(pkt, static_cast<uint8_t>(openWindowId_));
    writeShort(pkt, 63);

    for (int i = 0; i < 27; ++i) {
        writeItemStack(pkt, enderChestInventory_[i]);
    }
    for (int i = 9; i < 36; ++i) {
        writeItemStack(pkt, inventory_.getStackInSlot(i));
    }
    for (int i = 0; i < 9; ++i) {
        writeItemStack(pkt, inventory_.getStackInSlot(i));
    }

    conn.sendPacket(std::move(pkt));

    // Ender chest open sound — in Java: "random.chestopen" (same sound)
    server_.broadcastSound("random.chestopen",
        static_cast<double>(blockX) + 0.5,
        static_cast<double>(blockY) + 0.5,
        static_cast<double>(blockZ) + 0.5,
        0.5f, 1.0f);
}

void PlayHandler::openFurnace(Connection& conn, int32_t blockX, int32_t blockY, int32_t blockZ) {
    // Java: ContainerFurnace — S2D OpenWindow type 2 "minecraft:furnace"
    if (openWindowId_ > 0) {
        closeOpenWindow(conn);
    }

    openWindowId_ = nextWindowId_++;
    if (nextWindowId_ > 100) nextWindowId_ = 1;
    openWindowType_ = 2; // furnace

    furnaceData_ = static_cast<void*>(&server_.getOrCreateFurnace(blockX, blockY, blockZ));
    auto* furnace = static_cast<MinecraftServer::FurnaceData*>(furnaceData_);
    openFurnaceKey_ = MinecraftServer::packBlockPos(blockX, blockY, blockZ);

    // Send S2D OpenWindow (type 2 = furnace, 3 slots)
    sendOpenWindow(conn, openWindowId_, 2, "Furnace", 3);

    // Send window contents — 39 slots (3 furnace + 27 main inv + 9 hotbar)
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::WindowItems);
    writeByte(pkt, static_cast<uint8_t>(openWindowId_));
    writeShort(pkt, 39); // 39 slots total

    for (int i = 0; i < 3; ++i) {
        writeItemStack(pkt, furnace->slots[i]);
    }
    // Slots 3-29: main inventory (player slots 9-35)
    for (int i = 9; i < 36; ++i) {
        writeItemStack(pkt, inventory_.getStackInSlot(i));
    }
    // Slots 30-38: hotbar (player slots 0-8)
    for (int i = 0; i < 9; ++i) {
        writeItemStack(pkt, inventory_.getStackInSlot(i));
    }

    conn.sendPacket(std::move(pkt));

    // Send initial progress bar values (S31)
    sendWindowProperty(conn, openWindowId_, 0, furnace->furnaceCookTime);
    sendWindowProperty(conn, openWindowId_, 1, furnace->furnaceBurnTime);
    sendWindowProperty(conn, openWindowId_, 2, furnace->currentItemBurnTime);
}

void PlayHandler::sendWindowProperty(Connection& conn, int8_t windowId,
                                      int16_t property, int16_t value) {
    // S31 WindowProperty — furnace progress bars
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::WindowProperty);
    writeByte(pkt, static_cast<uint8_t>(windowId));
    writeShort(pkt, property);
    writeShort(pkt, value);
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendEntityEquipment(Connection& conn, int32_t entityId, int16_t equipSlot,
                                       const std::optional<ItemStack>& stack) {
    // Java reference: S04PacketEntityEquipment.writePacketData()
    // Format: Int entityId, Short slot, ItemStack
    // slot: 0=held, 1=boots, 2=leggings, 3=chestplate, 4=helmet
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::EntityEquipment);
    writeInt(pkt, entityId);
    writeShort(pkt, equipSlot);
    writeItemStack(pkt, stack);
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendEntityTeleport(Connection& conn, int32_t entityId,
                                      double x, double y, double z,
                                      float yaw, float pitch) {
    // Java reference: S18PacketEntityTeleport.writePacketData()
    // Format: Int entityId, Int x*32, Int y*32, Int z*32, Byte yaw, Byte pitch
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::EntityTeleport);
    writeInt(pkt, entityId);
    writeInt(pkt, static_cast<int32_t>(std::floor(x * 32.0)));
    writeInt(pkt, static_cast<int32_t>(std::floor(y * 32.0)));
    writeInt(pkt, static_cast<int32_t>(std::floor(z * 32.0)));
    writeByte(pkt, static_cast<uint8_t>(static_cast<int8_t>(yaw * 256.0f / 360.0f)));
    writeByte(pkt, static_cast<uint8_t>(static_cast<int8_t>(pitch * 256.0f / 360.0f)));
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendEntityHeadLook(Connection& conn, int32_t entityId, float yaw) {
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::EntityHeadLook);
    writeInt(pkt, entityId);
    writeByte(pkt, static_cast<uint8_t>(static_cast<int8_t>(yaw * 256.0f / 360.0f)));
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendEntityRelMove(Connection& conn, int32_t entityId,
                                     int8_t dx, int8_t dy, int8_t dz) {
    // Java reference: S15PacketEntityRelMove.writePacketData()
    // Format: Int entityId, Byte dx, Byte dy, Byte dz
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::EntityRelMove);
    writeInt(pkt, entityId);
    writeByte(pkt, static_cast<uint8_t>(dx));
    writeByte(pkt, static_cast<uint8_t>(dy));
    writeByte(pkt, static_cast<uint8_t>(dz));
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendEntityLook(Connection& conn, int32_t entityId,
                                  float yaw, float pitch) {
    // Java reference: S16PacketEntityLook.writePacketData()
    // Format: Int entityId, Byte yaw, Byte pitch
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::EntityLook);
    writeInt(pkt, entityId);
    writeByte(pkt, static_cast<uint8_t>(static_cast<int8_t>(yaw * 256.0f / 360.0f)));
    writeByte(pkt, static_cast<uint8_t>(static_cast<int8_t>(pitch * 256.0f / 360.0f)));
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendEntityLookRelMove(Connection& conn, int32_t entityId,
                                         int8_t dx, int8_t dy, int8_t dz,
                                         float yaw, float pitch) {
    // Java reference: S17PacketEntityLookMove.writePacketData()
    // Format: Int entityId, Byte dx, Byte dy, Byte dz, Byte yaw, Byte pitch
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::EntityLookAndRelMove);
    writeInt(pkt, entityId);
    writeByte(pkt, static_cast<uint8_t>(dx));
    writeByte(pkt, static_cast<uint8_t>(dy));
    writeByte(pkt, static_cast<uint8_t>(dz));
    writeByte(pkt, static_cast<uint8_t>(static_cast<int8_t>(yaw * 256.0f / 360.0f)));
    writeByte(pkt, static_cast<uint8_t>(static_cast<int8_t>(pitch * 256.0f / 360.0f)));
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendSoundEffect(Connection& conn, const std::string& soundName,
                                   double x, double y, double z,
                                   float volume, float pitch) {
    // Java reference: S29PacketSoundEffect.writePacketData()
    // Format: String name, Int x*8, Int y*8, Int z*8, Float volume, Byte pitch*63
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::SoundEffect);
    writeString(pkt, soundName);
    writeInt(pkt, static_cast<int32_t>(x * 8.0));
    writeInt(pkt, static_cast<int32_t>(y * 8.0));
    writeInt(pkt, static_cast<int32_t>(z * 8.0));
    writeFloat(pkt, volume);
    // pitch encoding: (int)(pitch * 63.0f), clamped to [0, 255]
    int pitchInt = static_cast<int>(pitch * 63.0f);
    if (pitchInt < 0) pitchInt = 0;
    if (pitchInt > 255) pitchInt = 255;
    writeByte(pkt, static_cast<uint8_t>(pitchInt));
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendSpawnObject(Connection& conn, int32_t entityId, int8_t type,
                                   double x, double y, double z,
                                   float yaw, float pitch, int32_t data,
                                   double motionX, double motionY, double motionZ) {
    // Java reference: S0EPacketSpawnObject.writePacketData()
    // Format: VarInt entityId, Byte type, Int x*32, Int y*32, Int z*32,
    //   Byte pitch, Byte yaw, Int data, [Short velX, Short velY, Short velZ if data > 0]
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::SpawnObject);
    writeVarInt(pkt, entityId);
    writeByte(pkt, static_cast<uint8_t>(type));
    writeInt(pkt, static_cast<int32_t>(x * 32.0));
    writeInt(pkt, static_cast<int32_t>(y * 32.0));
    writeInt(pkt, static_cast<int32_t>(z * 32.0));
    writeByte(pkt, static_cast<uint8_t>(static_cast<int>(pitch * 256.0f / 360.0f) & 0xFF));
    writeByte(pkt, static_cast<uint8_t>(static_cast<int>(yaw * 256.0f / 360.0f) & 0xFF));
    writeInt(pkt, data);
    if (data > 0) {
        // Velocity encoded: vel * 8000, clamped ±3.9
        auto encVel = [](double v) -> int16_t {
            if (v < -3.9) v = -3.9;
            if (v > 3.9) v = 3.9;
            return static_cast<int16_t>(v * 8000.0);
        };
        writeShort(pkt, encVel(motionX));
        writeShort(pkt, encVel(motionY));
        writeShort(pkt, encVel(motionZ));
    }
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendEntityMetadataItem(Connection& conn, int32_t entityId,
                                          int16_t itemId, int8_t stackSize, int16_t damage) {
    // Java reference: S1CPacketEntityMetadata + DataWatcher serialization
    // DataWatcher slot 10, type 5 (ItemStack)
    // Format: VarInt packetId, Int entityId, DataWatcher entries
    // DataWatcher entry: Byte (type<<5 | index), then type-specific data
    //   Type 5 = ItemStack: Short itemId, Byte count, Short damage, Byte 0 (no NBT)
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::EntityMetadata);
    writeInt(pkt, entityId);
    // DataWatcher entry: type=5 (ItemStack), index=10
    // Header byte = (type << 5) | (index & 0x1F) = (5 << 5) | 10 = 0xAA
    writeByte(pkt, 0xAA);
    writeShort(pkt, itemId);
    writeByte(pkt, static_cast<uint8_t>(stackSize));
    writeShort(pkt, damage);
    // NBT tag: Java readItemStackFromBuffer calls readNBTTagCompoundFromBuffer
    // which does readShort() — Short(-1) = no NBT tag
    writeShort(pkt, -1);
    writeByte(pkt, 0x7F); // DataWatcher terminator
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendCollectItem(Connection& conn, int32_t collectedEntityId, int32_t collectorEntityId) {
    // Java reference: S0DPacketCollectItem.writePacketData()
    // Format: Int collectedEntityId, Int collectorEntityId
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::CollectItem);
    writeInt(pkt, collectedEntityId);
    writeInt(pkt, collectorEntityId);
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::handlePlayerPosition(const uint8_t* data, size_t length, Connection& conn) {
    // Java reference: NetHandlerPlayServer.processPlayer()
    // C04PacketPlayerPosition: Double x, Double y, Double stance, Double z, Bool onGround
    // Note: 1.7.10 sends BOTH y (feet) AND stance (head) — 33 bytes total
    if (length < 33) return;
    double oldX = playerX_, oldY = playerY_, oldZ = playerZ_;
    playerX_ = readDouble(data);
    playerY_ = readDouble(data + 8);
    // stance = readDouble(data + 16) — head Y, not stored separately
    playerZ_ = readDouble(data + 24);
    playerOnGround_ = data[32] != 0;

    // ─── Fall damage tracking ──────────────────────────────────────────
    // Java: Entity.moveEntity → updateFallState → EntityLivingBase.fall()
    double deltaY = playerY_ - oldY;
    if (deltaY < 0.0) {
        // Player is falling — accumulate distance
        fallDistance_ += static_cast<float>(-deltaY);
    }
    if (playerOnGround_ && fallDistance_ > 0.0f) {
        // Landed! Apply fall damage — Java: damage = ceil(fallDistance - 3.0)
        if (gameMode_ != 1 && !dead_) { // No fall damage in creative
            int damage = static_cast<int>(std::ceil(fallDistance_ - 3.0f));
            if (damage > 0) {
                health_ -= static_cast<float>(damage);
                if (health_ < 0.0f) health_ = 0.0f;
                sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
                // Play fall sound — Java: EntityLivingBase.func_146067_o
                const char* fallSound = (damage > 4) ? "game.player.hurt.fall.big" : "game.player.hurt.fall.small";
                server_.broadcastSound(fallSound, playerX_, playerY_, playerZ_, 1.0f, 1.0f);
                std::cout << "[Fall] " << playerName_ << " took " << damage
                          << " fall damage (fell " << fallDistance_ << " blocks, hp=" << health_ << ")\n";
                if (health_ <= 0.0f) {
                    // Player died from fall damage
                    dead_ = true;
                    server_.broadcastEntityEvent(entityId_, 3);
                    server_.broadcastChatMessage(playerName_ + " fell from a high place");
                }
            }
        }
        fallDistance_ = 0.0f;
    }

    // Movement exhaustion — Java: EntityPlayer.addExhaustion per meter
    double dx = playerX_ - oldX;
    double dz = playerZ_ - oldZ;
    double distSq = dx * dx + dz * dz;
    if (distSq > 0.0001) {
        float dist = static_cast<float>(std::sqrt(distSq));
        if (isSprinting_) {
            foodStats_.addExhaustion(dist * Exhaustion::SPRINT); // 0.1 per meter
        } else {
            foodStats_.addExhaustion(dist * Exhaustion::WALK);   // 0.01 per meter
        }
    }

    server_.broadcastPlayerPosition(*this);
    updateChunks(conn);
}

void PlayHandler::handlePlayerLook(const uint8_t* data, size_t length, Connection& conn) {
    // Java reference: NetHandlerPlayServer.processPlayer()
    // C05PacketPlayerLook: Float yaw, Float pitch, Bool onGround
    if (length < 9) return;
    playerYaw_ = readFloat(data);
    playerPitch_ = readFloat(data + 4);
    playerOnGround_ = data[8] != 0;
    server_.broadcastPlayerPosition(*this);
}

void PlayHandler::handlePlayerPosAndLook(const uint8_t* data, size_t length, Connection& conn) {
    // Java reference: NetHandlerPlayServer.processPlayer()
    // C06PacketPlayerPosLook: Double x, Double y, Double stance, Double z, Float yaw, Float pitch, Bool onGround
    if (length < 41) return;
    double oldX = playerX_, oldY = playerY_, oldZ = playerZ_;
    playerX_ = readDouble(data);
    playerY_ = readDouble(data + 8);
    // stance = readDouble(data + 16)
    playerZ_ = readDouble(data + 24);
    playerYaw_ = readFloat(data + 32);
    playerPitch_ = readFloat(data + 36);
    playerOnGround_ = data[40] != 0;

    // ─── Fall damage tracking (same as handlePlayerPosition) ───────────
    double deltaY = playerY_ - oldY;
    if (deltaY < 0.0) {
        fallDistance_ += static_cast<float>(-deltaY);
    }
    if (playerOnGround_ && fallDistance_ > 0.0f) {
        if (gameMode_ != 1 && !dead_) {
            int damage = static_cast<int>(std::ceil(fallDistance_ - 3.0f));
            if (damage > 0) {
                health_ -= static_cast<float>(damage);
                if (health_ < 0.0f) health_ = 0.0f;
                sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
                const char* fallSound = (damage > 4) ? "game.player.hurt.fall.big" : "game.player.hurt.fall.small";
                server_.broadcastSound(fallSound, playerX_, playerY_, playerZ_, 1.0f, 1.0f);
                std::cout << "[Fall] " << playerName_ << " took " << damage
                          << " fall damage (fell " << fallDistance_ << " blocks, hp=" << health_ << ")\n";
                if (health_ <= 0.0f) {
                    dead_ = true;
                    server_.broadcastEntityEvent(entityId_, 3);
                    server_.broadcastChatMessage(playerName_ + " fell from a high place");
                }
            }
        }
        fallDistance_ = 0.0f;
    }

    // Movement exhaustion — same as handlePlayerPosition
    double dx = playerX_ - oldX;
    double dz = playerZ_ - oldZ;
    double distSq = dx * dx + dz * dz;
    if (distSq > 0.0001) {
        float dist = static_cast<float>(std::sqrt(distSq));
        if (isSprinting_) {
            foodStats_.addExhaustion(dist * Exhaustion::SPRINT);
        } else {
            foodStats_.addExhaustion(dist * Exhaustion::WALK);
        }
    }

    server_.broadcastPlayerPosition(*this);
    updateChunks(conn);
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

void PlayHandler::handlePlayerDigging(const uint8_t* data, size_t length, Connection& conn) {
    // Java reference: NetHandlerPlayServer.processPlayerDigging()
    // → ItemInWorldManager.onBlockClicked() (status 0)
    // → ItemInWorldManager.blockRemoving() (status 2)
    // C07PacketPlayerDigging format:
    //   UByte status, Int blockX, UByte blockY, Int blockZ, UByte face
    if (length < 11) return;

    uint8_t status = data[0];
    int32_t blockX = readInt(data + 1);
    uint8_t blockY = data[5];
    int32_t blockZ = readInt(data + 6);
    uint8_t face   = data[10];
    (void)face;

    // Status values:
    // 0 = Started digging (creative: instant break; survival: start)
    // 1 = Cancelled digging
    // 2 = Finished digging (survival: break the block)
    // 3 = Drop item stack
    // 4 = Drop item
    // 5 = Shoot arrow / finish eating

    if (status == 3 || status == 4) {
        // Java: NetHandlerPlayServer.processPlayerDigging() → drop item
        // Status 3 = drop entire stack, Status 4 = drop single item
        // The item comes from the player's currently held hotbar slot
        auto currentItem = inventory_.getCurrentItem();
        if (!currentItem || currentItem->isEmpty()) return;

        int32_t dropCount = (status == 3) ? currentItem->getStackSize() : 1;
        int32_t itemId = currentItem->getItemId();
        int32_t damage = currentItem->getDamage();

        // Decrement the stack in inventory
        int32_t remaining = currentItem->getStackSize() - dropCount;
        if (remaining <= 0) {
            inventory_.setInventorySlotContents(currentSlot_, std::nullopt);
        } else {
            ItemStack updated = *currentItem;
            updated.setStackSize(remaining);
            inventory_.setInventorySlotContents(currentSlot_, updated);
        }

        // Sync the slot to client — ContainerPlayer hotbar slot = 36 + currentSlot_
        int16_t containerSlot = static_cast<int16_t>(36 + currentSlot_);
        sendSetSlot(conn, 0, containerSlot, inventory_.getStackInSlot(currentSlot_));

        // Spawn the dropped item entity in the world
        server_.spawnItemDrop(playerX_, playerY_ + 1.5, playerZ_, itemId, damage, dropCount);

        std::cout << "[Inv] " << playerName_ << " dropped " << dropCount << "x item " << itemId << "\n";
        return;
    }

    if (status == 5) {
        // Shoot arrow / finish eating — silently consume for now
        return;
    }

    // Range check — Java: d4 > 36.0 means distance > 6 blocks
    if (status == 0 || status == 1 || status == 2) {
        double dx = playerX_ - (static_cast<double>(blockX) + 0.5);
        double dy = playerY_ - (static_cast<double>(blockY) + 0.5) + 1.5;
        double dz = playerZ_ - (static_cast<double>(blockZ) + 0.5);
        double distSq = dx * dx + dy * dy + dz * dz;
        if (distSq > 36.0) return;

        // Build height check
        if (blockY >= 255) return;
    }

    auto& worlds = server_.getWorlds();
    if (worlds.empty()) return;
    WorldServer* world = worlds[0].get();

    // ─── Helper: get the break sound for a block's material ────────────
    // Java reference: Block$SoundType — each block has a stepSound
    // dig.stone, dig.wood, dig.gravel, dig.grass, dig.cloth, dig.sand, dig.snow, dig.glass
    auto getBreakSound = [](int32_t blockId) -> const char* {
        // Check material type based on block ID
        // Java reference: Block.registerBlocks() sets stepSound per block
        switch (blockId) {
            // Wood family — dig.wood
            case 5:   // planks
            case 17:  // log
            case 25:  // noteblock
            case 47:  // bookshelf
            case 50:  // torch
            case 53:  // oak_stairs
            case 54:  // chest
            case 58:  // crafting_table
            case 63:  // sign
            case 64:  // wooden_door
            case 84:  // jukebox
            case 85:  // fence
            case 96:  // trapdoor
            case 107: // fence_gate
            case 125: // double_wooden_slab
            case 126: // wooden_slab
            case 134: case 135: case 136: // spruce/birch/jungle stairs
            case 143: // wooden button
            case 154: // hopper
            case 162: // log2
            case 163: case 164: // acacia/dark_oak stairs
                return "dig.wood";

            // Grass family — dig.grass
            case 2:   // grass
            case 6:   // sapling
            case 18:  // leaves
            case 31:  // tallgrass
            case 32:  // deadbush
            case 37:  case 38: // flowers
            case 39:  case 40: // mushrooms
            case 86:  case 91: // pumpkin
            case 83:  // reeds
            case 104: case 105: // stems
            case 106: // vine
            case 111: // waterlily
            case 161: // leaves2
            case 170: // hay_block
            case 175: // double_plant
                return "dig.grass";

            // Gravel/dirt — dig.gravel
            case 3:   // dirt
            case 13:  // gravel
            case 60:  // farmland
            case 110: // mycelium
                return "dig.gravel";

            // Sand — dig.sand
            case 12:  // sand
            case 88:  // soul_sand
                return "dig.sand";

            // Cloth — dig.cloth
            case 35:  // wool
            case 81:  // cactus
            case 92:  // cake
            case 171: // carpet
                return "dig.cloth";

            // Snow — dig.snow
            case 78:  // snow_layer
            case 80:  // snow
                return "dig.snow";

            // Glass — dig.glass (actually uses random.glass usually)
            case 20:  // glass
            case 79:  // ice
            case 89:  // glowstone
            case 95:  // stained_glass
            case 102: // glass_pane
            case 160: // stained_glass_pane
            case 174: // packed_ice
                return "dig.glass";

            // Metal — dig.stone with higher pitch
            case 27: case 28: case 66: case 157: // rails
            case 41: case 42: case 57: case 133: // metal blocks
            case 52:  // mob_spawner
            case 71:  // iron_door
            case 101: // iron_bars
            case 145: // anvil
                return "dig.stone";

            // Default: stone for most blocks
            default:
                return "dig.stone";
        }
    };

    // ─── Helper: get the item drop for a block ─────────────────────────
    // Java reference: Block.getItemDropped() — overridden per subclass
    // Returns: {itemId, quantity, metadata}. itemId=-1 means no drop.
    struct BlockDrop { int32_t itemId; int32_t quantity; int32_t metadata; };
    auto getBlockDrop = [](int32_t blockId, int32_t blockMeta) -> BlockDrop {
        switch (blockId) {
            // Blocks that drop nothing
            case 0:   return {-1, 0, 0}; // air
            case 7:   return {-1, 0, 0}; // bedrock (unbreakable)
            case 8: case 9: return {-1, 0, 0};   // water
            case 10: case 11: return {-1, 0, 0};  // lava
            // Leaves — Java: BlockLeaves.getItemDropped (sapling 1/20, apple 1/200 for oak)
            case 18: {
                int leafType = blockMeta & 0x03; // 0=oak, 1=spruce, 2=birch, 3=jungle
                // 1/20 chance to drop sapling (matching type)
                if (rand() % 20 == 0) return {6, 1, leafType}; // Sapling with matching meta
                // 1/200 chance for apple from oak leaves
                if (leafType == 0 && rand() % 200 == 0) return {260, 1, 0}; // Apple
                return {-1, 0, 0};
            }
            case 161: {
                int leafType2 = blockMeta & 0x01; // 0=acacia, 1=dark_oak
                if (rand() % 20 == 0) return {6, 1, leafType2 + 4}; // Sapling (acacia=4, dark_oak=5)
                if (leafType2 == 1 && rand() % 200 == 0) return {260, 1, 0}; // Apple from dark oak
                return {-1, 0, 0};
            }
            case 20: return {-1, 0, 0};  // glass
            case 30: return {287, 1, 0}; // web → string (item 287)
            case 34: return {-1, 0, 0};  // piston_head
            case 36: return {-1, 0, 0};  // piston_extension
            case 51: return {-1, 0, 0};  // fire
            case 52: return {-1, 0, 0};  // mob_spawner
            case 59: {
                // Wheat crop — Java: BlockCrops.getItemDropped
                // Mature (meta 7): drop wheat (296) + 0-3 seeds (295)
                // Immature: drop 0-1 seeds
                if (blockMeta >= 7) return {296, 1, 0}; // Wheat item
                return {295, 1, 0}; // Seeds
            }
            // Carrot crop — Java: BlockCarrot.getItemDropped
            case 141: return {391, (blockMeta >= 7) ? 2 : 1, 0}; // Carrot
            // Potato crop — Java: BlockPotato.getItemDropped  
            case 142: return {392, (blockMeta >= 7) ? 2 : 1, 0}; // Potato
            // Melon stem → melon seeds
            case 104: return {362, 1, 0};
            // Pumpkin stem → pumpkin seeds
            case 105: return {361, 1, 0};
            case 78: return {332, 1, 0}; // snow_layer → snowball
            case 79: return {-1, 0, 0};  // ice
            case 90: return {-1, 0, 0};  // portal
            case 95: return {-1, 0, 0};  // stained glass
            case 97: return {-1, 0, 0};  // silverfish block
            case 102: return {-1, 0, 0}; // glass pane
            case 119: return {-1, 0, 0}; // end portal
            case 120: return {-1, 0, 0}; // end portal frame
            case 132: return {-1, 0, 0}; // tripwire
            case 160: return {-1, 0, 0}; // stained glass pane

            // ─── Blocks that drop a DIFFERENT item ────────────────
            // Java: BlockStone.getItemDropped → cobblestone (ID 4)
            case 1:   return {4, 1, 0};   // stone → cobblestone
            // Java: BlockGrass.getItemDropped → dirt (ID 3)
            case 2:   return {3, 1, 0};   // grass → dirt
            // Java: BlockOre coal → coal item (ID 263)
            case 16:  return {263, 1, 0};  // coal_ore → coal
            // Java: BlockOre diamond → diamond (ID 264)
            case 56:  return {264, 1, 0};  // diamond_ore → diamond
            // Java: BlockOre lapis → dye:4 (ID 351, meta 4)
            case 21:  return {351, 4, 4};  // lapis_ore → 4-8 lapis lazuli (simplified to 4)
            // Java: BlockOre redstone → redstone dust (ID 331), quantity 4-5
            case 73: case 74: return {331, 4, 0}; // redstone_ore → 4 redstone dust
            // Java: BlockOre emerald → emerald (ID 388)
            case 129: return {388, 1, 0};  // emerald_ore → emerald
            // Java: BlockOre quartz → quartz item (ID 406)
            case 153: return {406, 1, 0};  // quartz_ore → quartz
            // Java: BlockGlowstone → 2-4 glowstone dust (ID 348)
            case 89:  return {348, 3, 0};  // glowstone → 3 glowstone dust (avg)
            // Java: BlockClay → 4 clay balls (ID 337)
            case 82:  return {337, 4, 0};  // clay → 4 clay balls
            // Java: BlockMelon → 3-7 melon slices (ID 360)
            case 103: return {360, 4, 0};  // melon → 4 melon slices (avg)
            // Java: BlockSnow → 4 snowballs (ID 332)
            case 80:  return {332, 4, 0};  // snow block → 4 snowballs
            // Java: BlockTallGrass → seeds 1/8 chance (fortuna increases)
            case 31:  return (rand() % 8 == 0) ? BlockDrop{295, 1, 0} : BlockDrop{-1, 0, 0};
            case 32:  return {-1, 0, 0};   // dead bush → nothing (sticks with shears)

            // Door drops the item
            case 64:  return {324, 1, 0};  // wooden door → door item
            case 71:  return {330, 1, 0};  // iron door → iron door item
            // Bed drops bed item
            case 26:  return {355, 1, 0};  // bed → bed item
            // Redstone wire → redstone dust
            case 55:  return {331, 1, 0};  // redstone wire → redstone
            // Repeater → repeater item
            case 93: case 94: return {356, 1, 0}; // repeater
            // Comparator → comparator item
            case 149: case 150: return {404, 1, 0};
            // Sugar cane → sugar cane item
            case 83:  return {338, 1, 0};  // reeds → item
            // Cake drops nothing (already partially eaten)
            case 92:  return {-1, 0, 0};
            // Lit furnace → furnace
            case 62:  return {61, 1, 0};
            // Lit redstone lamp → redstone lamp
            case 124: return {123, 1, 0};

            // ─── Nether / End blocks ──────────────────────────────
            case 87:  return {87, 1, 0};  // netherrack → netherrack
            case 88:  return {88, 1, 0};  // soul sand → soul sand
            case 121: return {121, 1, 0}; // end stone → end stone
            case 112: return {112, 1, 0}; // nether brick → nether brick
            case 113: return {113, 1, 0}; // nether brick fence → fence
            case 114: return {114, 1, 0}; // nether brick stairs → stairs

            // ─── More special drops ───────────────────────────────
            // Bookshelf → 3 books (Java: BlockBookshelf.quantityDropped)
            case 47:  return {340, 3, 0};
            // Gravel → flint (10% chance, Java: BlockGravel.getItemDropped)
            case 13:  return (rand() % 10 == 0) ? BlockDrop{318, 1, 0} : BlockDrop{13, 1, 0};
            // Nether wart (block 115) → nether wart item (372)
            case 115: return {372, (blockMeta >= 3) ? 3 : 1, 0};
            // Cocoa bean (block 127) → cocoa beans (dye:3, item 351, dmg 3)
            case 127: return {351, (blockMeta >= 8) ? 3 : 1, 3};
            // Lily pad → self
            case 111: return {111, 1, 0};
            // Vines → nothing (shears needed)
            case 106: return {-1, 0, 0};
            // Mycelium → dirt
            case 110: return {3, 1, 0};
            // Dragon egg → self
            case 122: return {122, 1, 0};
            // Command block → nothing
            case 137: return {-1, 0, 0};
            // Beacon → self
            case 138: return {138, 1, 0};
            // Anvil → self (with damage preserved in meta)
            case 145: return {145, 1, blockMeta};
            // Huge brown mushroom (99) → brown mushroom (39), 0-2
            case 99:  return {39, 1, 0};
            // Huge red mushroom (100) → red mushroom (40), 0-2  
            case 100: return {40, 1, 0};
            // Double stone slab → 2 slabs
            case 43:  return {44, 2, blockMeta};
            case 125: return {126, 2, blockMeta}; // Double wood slab → 2 wood slabs
            // Brewing stand block → item 379
            case 117: return {379, 1, 0};
            // Cauldron block → item 380
            case 118: return {380, 1, 0};
            // Enchanting table → self
            case 116: return {116, 1, 0};
            // Ender chest → 8 obsidian (Java: BlockEnderChest.quantityDropped)
            case 130: return {49, 8, 0};
            // Banner → self (1.8+, skip)
            // Flower pot → item 390
            case 140: return {390, 1, 0};
            // Skull/head → item 397
            case 144: return {397, 1, blockMeta};
            // Daylight sensor → self
            case 151: return {151, 1, 0};
            // Hopper → self  
            case 154: return {154, 1, 0};
            // Dropper → self
            case 158: return {158, 1, 0};
            // Stained clay → self
            case 159: return {159, 1, blockMeta};
            // Hay bale → self
            case 170: return {170, 1, 0};
            // Hardened clay → self
            case 172: return {172, 1, 0};
            // Coal block → self
            case 173: return {173, 1, 0};
            // Packed ice → nothing (no silk touch support)
            case 174: return {-1, 0, 0};

            // ─── Default: block drops itself as item ──────────────
            // Java: Block.getItemDropped → Item.getItemFromBlock(this)
            // For most blocks, block ID == item ID
            default:
                return {blockId, 1, 0};
        }
    };

    // ═══════════════════════════════════════════════════════════════════
    // Status 0: Start digging
    // ═══════════════════════════════════════════════════════════════════
    if (status == 0) {
        Block* existingBlock = world->getBlock(blockX, blockY, blockZ);
        if (!existingBlock || Block::getIdFromBlock(existingBlock) == 0) return;

        int32_t brokenBlockId = Block::getIdFromBlock(existingBlock);
        int32_t brokenMeta = world->getBlockMetadata(blockX, blockY, blockZ);

        if (gameMode_ == 1) {
            // ─── CREATIVE: instant break ──────────────────────────
            // Java: ItemInWorldManager.onBlockClicked → isCreative → tryHarvestBlock
            world->setBlock(blockX, blockY, blockZ, Block::getBlockById(0));
            world->setBlockMetadata(blockX, blockY, blockZ, 0);
            server_.broadcastBlockChange(blockX, blockY, blockZ, 0, 0);
            server_.broadcastEffect(2001, blockX, blockY, blockZ, brokenBlockId);
            // Multi-block propagation (doors/beds/cactus/sugar cane)
            if (brokenBlockId == 64 || brokenBlockId == 71) {
                int otherY = (brokenMeta & 0x08) ? blockY - 1 : blockY + 1;
                Block* ob = world->getBlock(blockX, otherY, blockZ);
                if (ob && Block::getIdFromBlock(ob) == brokenBlockId) {
                    world->setBlock(blockX, otherY, blockZ, Block::getBlockById(0));
                    server_.broadcastBlockChange(blockX, otherY, blockZ, 0, 0);
                }
            }
            if (brokenBlockId == 26) {
                int bf = brokenMeta & 3; bool ih = (brokenMeta & 8) != 0;
                int ox = blockX, oz = blockZ;
                if (ih) { switch(bf){case 0:--oz;break;case 1:++ox;break;case 2:++oz;break;case 3:--ox;break;} }
                else    { switch(bf){case 0:++oz;break;case 1:--ox;break;case 2:--oz;break;case 3:++ox;break;} }
                Block* ob = world->getBlock(ox, blockY, oz);
                if (ob && Block::getIdFromBlock(ob) == 26) {
                    world->setBlock(ox, blockY, oz, Block::getBlockById(0));
                    server_.broadcastBlockChange(ox, blockY, oz, 0, 0);
                }
            }
            if (brokenBlockId == 81 || brokenBlockId == 83) {
                for (int cy2 = blockY + 1; cy2 < 256; ++cy2) {
                    Block* ab = world->getBlock(blockX, cy2, blockZ);
                    if (!ab || Block::getIdFromBlock(ab) != brokenBlockId) break;
                    world->setBlock(blockX, cy2, blockZ, Block::getBlockById(0));
                    server_.broadcastBlockChange(blockX, cy2, blockZ, 0, 0);
                }
            }
            server_.broadcastSound(getBreakSound(brokenBlockId),
                static_cast<double>(blockX) + 0.5,
                static_cast<double>(blockY) + 0.5,
                static_cast<double>(blockZ) + 0.5,
                1.0f, 0.8f);
            std::cout << "[World] " << playerName_ << " broke block at "
                      << blockX << "," << (int)blockY << "," << blockZ << " (creative)\n";
            // Creative: no item drops (Java: tryHarvestBlock skips harvestBlock in creative)
        } else {
            // ─── SURVIVAL: just acknowledge start ─────────────────
            // Java: ItemInWorldManager.onBlockClicked → check instant-break
            // If hardness == 0 (e.g., tall grass, torch), break instantly
            float hardness = existingBlock->getHardness();
            if (hardness == 0.0f) {
                // Instant-break blocks (hardness 0) — same as creative path
                world->setBlock(blockX, blockY, blockZ, Block::getBlockById(0));
                world->setBlockMetadata(blockX, blockY, blockZ, 0);
                server_.broadcastBlockChange(blockX, blockY, blockZ, 0, 0);
                server_.broadcastEffect(2001, blockX, blockY, blockZ, brokenBlockId);
                // Multi-block propagation (doors/beds/cactus/sugar cane)
                if (brokenBlockId == 64 || brokenBlockId == 71) {
                    int otherY2 = (brokenMeta & 0x08) ? blockY - 1 : blockY + 1;
                    Block* ob2 = world->getBlock(blockX, otherY2, blockZ);
                    if (ob2 && Block::getIdFromBlock(ob2) == brokenBlockId) {
                        world->setBlock(blockX, otherY2, blockZ, Block::getBlockById(0));
                        server_.broadcastBlockChange(blockX, otherY2, blockZ, 0, 0);
                    }
                }
                if (brokenBlockId == 26) {
                    int bf2 = brokenMeta & 3; bool ih2 = (brokenMeta & 8) != 0;
                    int ox2 = blockX, oz2 = blockZ;
                    if (ih2) { switch(bf2){case 0:--oz2;break;case 1:++ox2;break;case 2:++oz2;break;case 3:--ox2;break;} }
                    else     { switch(bf2){case 0:++oz2;break;case 1:--ox2;break;case 2:--oz2;break;case 3:++ox2;break;} }
                    Block* ob2b = world->getBlock(ox2, blockY, oz2);
                    if (ob2b && Block::getIdFromBlock(ob2b) == 26) {
                        world->setBlock(ox2, blockY, oz2, Block::getBlockById(0));
                        server_.broadcastBlockChange(ox2, blockY, oz2, 0, 0);
                    }
                }
                if (brokenBlockId == 81 || brokenBlockId == 83) {
                    for (int cy3 = blockY + 1; cy3 < 256; ++cy3) {
                        Block* ab3 = world->getBlock(blockX, cy3, blockZ);
                        if (!ab3 || Block::getIdFromBlock(ab3) != brokenBlockId) break;
                        world->setBlock(blockX, cy3, blockZ, Block::getBlockById(0));
                        server_.broadcastBlockChange(blockX, cy3, blockZ, 0, 0);
                        server_.spawnItemDrop(static_cast<double>(blockX), static_cast<double>(cy3), static_cast<double>(blockZ), brokenBlockId, 0, 1);
                    }
                }
                server_.broadcastSound(getBreakSound(brokenBlockId),
                    static_cast<double>(blockX) + 0.5,
                    static_cast<double>(blockY) + 0.5,
                    static_cast<double>(blockZ) + 0.5,
                    1.0f, 0.8f);
                // Spawn drop
                auto drop = getBlockDrop(brokenBlockId, brokenMeta);
                if (drop.itemId >= 0 && drop.quantity > 0) {
                    server_.spawnItemDrop(
                        static_cast<double>(blockX),
                        static_cast<double>(blockY),
                        static_cast<double>(blockZ),
                        drop.itemId, drop.metadata, drop.quantity);
                }
                // Exhaustion — Java: EntityPlayer.addExhaustion(0.025f) via harvestBlock
                foodStats_.addExhaustion(0.025f);
                std::cout << "[World] " << playerName_ << " broke block at "
                          << blockX << "," << (int)blockY << "," << blockZ << " (instant)\n";
            }
            // Else: client is starting to mine — we just wait for status 2
        }
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // Status 1: Cancel digging — nothing to do
    // ═══════════════════════════════════════════════════════════════════
    if (status == 1) {
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // Status 2: Finished digging (survival mode)
    // Java: ItemInWorldManager.blockRemoving → tryHarvestBlock → block.harvestBlock
    // ═══════════════════════════════════════════════════════════════════
    if (status == 2) {
        // In creative, status 2 should not happen (client sends 0 only)
        // But handle it gracefully
        Block* existingBlock = world->getBlock(blockX, blockY, blockZ);
        if (!existingBlock || Block::getIdFromBlock(existingBlock) == 0) return;

        int32_t brokenBlockId = Block::getIdFromBlock(existingBlock);
        int32_t brokenMeta = world->getBlockMetadata(blockX, blockY, blockZ);

        // Don't allow breaking unbreakable blocks (hardness < 0)
        float hardness = existingBlock->getHardness();
        if (hardness < 0.0f) {
            // Send block back to client — Java: sendPacket(new S23PacketBlockChange)
            sendBlockChange(conn, blockX, blockY, blockZ, brokenBlockId, brokenMeta);
            return;
        }

        // Break the block
        world->setBlock(blockX, blockY, blockZ, Block::getBlockById(0));
        world->setBlockMetadata(blockX, blockY, blockZ, 0);
        server_.broadcastBlockChange(blockX, blockY, blockZ, 0, 0);
        server_.broadcastEffect(2001, blockX, blockY, blockZ, brokenBlockId);

        // ─── Multi-block break propagation ───────────────────────────
        // Door (64/71): remove other half
        if (brokenBlockId == 64 || brokenBlockId == 71) {
            if (brokenMeta & 0x08) {
                // Broke upper half → remove lower
                Block* lower = world->getBlock(blockX, blockY - 1, blockZ);
                if (lower && Block::getIdFromBlock(lower) == brokenBlockId) {
                    world->setBlock(blockX, blockY - 1, blockZ, Block::getBlockById(0));
                    server_.broadcastBlockChange(blockX, blockY - 1, blockZ, 0, 0);
                }
            } else {
                // Broke lower half → remove upper
                Block* upper = world->getBlock(blockX, blockY + 1, blockZ);
                if (upper && Block::getIdFromBlock(upper) == brokenBlockId) {
                    world->setBlock(blockX, blockY + 1, blockZ, Block::getBlockById(0));
                    server_.broadcastBlockChange(blockX, blockY + 1, blockZ, 0, 0);
                }
            }
        }
        // Bed (26): remove other half
        if (brokenBlockId == 26) {
            int bedFacing = brokenMeta & 0x03;
            bool isHead = (brokenMeta & 0x08) != 0;
            int otherX = blockX, otherZ = blockZ;
            if (isHead) {
                // Remove foot
                switch (bedFacing) {
                    case 0: --otherZ; break; case 1: ++otherX; break;
                    case 2: ++otherZ; break; case 3: --otherX; break;
                }
            } else {
                // Remove head
                switch (bedFacing) {
                    case 0: ++otherZ; break; case 1: --otherX; break;
                    case 2: --otherZ; break; case 3: ++otherX; break;
                }
            }
            Block* other = world->getBlock(otherX, blockY, otherZ);
            if (other && Block::getIdFromBlock(other) == 26) {
                world->setBlock(otherX, blockY, otherZ, Block::getBlockById(0));
                server_.broadcastBlockChange(otherX, blockY, otherZ, 0, 0);
            }
        }
        // Cactus (81) / Sugar cane (83): break all blocks above (chain break)
        if (brokenBlockId == 81 || brokenBlockId == 83) {
            for (int cy = blockY + 1; cy < 256; ++cy) {
                Block* above = world->getBlock(blockX, cy, blockZ);
                if (!above || Block::getIdFromBlock(above) != brokenBlockId) break;
                world->setBlock(blockX, cy, blockZ, Block::getBlockById(0));
                server_.broadcastBlockChange(blockX, cy, blockZ, 0, 0);
                server_.broadcastEffect(2001, blockX, cy, blockZ, brokenBlockId);
                // Drop the block
                server_.spawnItemDrop(
                    static_cast<double>(blockX), static_cast<double>(cy),
                    static_cast<double>(blockZ),
                    brokenBlockId, 0, 1);
            }
        }

        // Play break sound — material-based
        server_.broadcastSound(getBreakSound(brokenBlockId),
            static_cast<double>(blockX) + 0.5,
            static_cast<double>(blockY) + 0.5,
            static_cast<double>(blockZ) + 0.5,
            1.0f, 0.8f);

        // Spawn item drop — Java: block.harvestBlock → dropBlockAsItem → getItemDropped
        auto drop = getBlockDrop(brokenBlockId, brokenMeta);
        if (drop.itemId >= 0 && drop.quantity > 0) {
            server_.spawnItemDrop(
                static_cast<double>(blockX),
                static_cast<double>(blockY),
                static_cast<double>(blockZ),
                drop.itemId, drop.metadata, drop.quantity);
        }

        // Exhaustion — Java: EntityPlayer.addExhaustion(0.025f) via harvestBlock
        foodStats_.addExhaustion(0.025f);

        // Tool durability — Java: ItemStack.damageItem via onBlockDestroyed
        // Swords take 2 damage, other tools take 1, only for blocks with hardness > 0
        if (hardness > 0.0f) {
            auto held = inventory_.getCurrentItem();
            if (held) {
                int32_t heldId = held->getItemId();
                // Swords: 2 durability per block (Java: ItemSword.onBlockDestroyed)
                bool isSword = (heldId == 268 || heldId == 272 || heldId == 267 ||
                                heldId == 276 || heldId == 283);
                damageHeldItem(isSword ? 2 : 1);
            }
        }

        // ─── Mining XP ──────────────────────────────────────────────────
        // Java: Block.getExpDrop() — ores drop XP when mined
        {
            int32_t xp = 0;
            switch (brokenBlockId) {
                case 16:  xp = (rand() % 3);     break; // Coal ore: 0-2
                case 56:  xp = 3 + (rand() % 5); break; // Diamond ore: 3-7
                case 129: xp = 3 + (rand() % 5); break; // Emerald ore: 3-7
                case 73: case 74: xp = 1 + (rand() % 5); break; // Redstone ore: 1-5
                case 21:  xp = 2 + (rand() % 4); break; // Lapis ore: 2-5
                case 153: xp = 2 + (rand() % 4); break; // Quartz ore: 2-5
                default: break;
            }
            if (xp > 0) {
                addExperience(xp);
                sendSetExperience(conn, experienceBar_, experienceLevel_, experienceTotal_);
            }
        }

        std::cout << "[World] " << playerName_ << " broke block at "
                  << blockX << "," << (int)blockY << "," << blockZ << "\n";
    }
}

void PlayHandler::handlePlayerBlockPlace(const uint8_t* data, size_t length, Connection& conn) {
    // Java reference: NetHandlerPlayServer.processPlayerBlockPlacement()
    // C08PacketPlayerBlockPlacement format:
    //   Int blockX, UByte blockY, Int blockZ, UByte direction,
    //   Short heldItemId, [if id >= 0: Byte count, Short damage, ...NBT],
    //   UByte cursorX, UByte cursorY, UByte cursorZ
    //
    // Minimum size: 4+1+4+1 = 10 bytes for coords + direction
    if (length < 10) return;

    int32_t blockX = readInt(data);
    uint8_t blockY = data[4];
    int32_t blockZ = readInt(data + 5);
    uint8_t direction = data[9];

    // direction = 255 means "use item" (right-click in air)
    if (direction == 255) {
        // Check if held item is food — Java: ItemFood.onItemRightClick
        if (length >= 12) {
            int16_t heldItemId = static_cast<int16_t>((data[10] << 8) | data[11]);
            const FoodValue* foodVal = FoodValues::getByItemId(heldItemId);
            if (foodVal && foodStats_.needFood()) {
                // Eat the food — Java: ItemFood.onItemUseFinish
                foodStats_.addStats(foodVal->healAmount, foodVal->saturationModifier);
                sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());

                // Consume 1 item from held slot — Java: --itemStack.stackSize
                auto heldStack = inventory_.getCurrentItem();
                if (heldStack) {
                    int32_t remaining = heldStack->getStackSize() - 1;
                    if (remaining <= 0) {
                        inventory_.setInventorySlotContents(currentSlot_, std::nullopt);
                    } else {
                        ItemStack updated = *heldStack;
                        updated.setStackSize(remaining);
                        inventory_.setInventorySlotContents(currentSlot_, updated);
                    }
                    // Sync slot to client — ContainerPlayer hotbar = 36 + currentSlot_
                    int16_t containerSlot = static_cast<int16_t>(36 + currentSlot_);
                    sendSetSlot(conn, 0, containerSlot, inventory_.getStackInSlot(currentSlot_));
                }

                // Play burp sound
                server_.broadcastSound("random.burp", playerX_, playerY_, playerZ_, 0.5f, 0.9f);
                std::cout << "[Food] " << playerName_ << " ate item " << heldItemId
                          << " (food=" << foodStats_.getFoodLevel()
                          << ", sat=" << foodStats_.getSaturationLevel() << ")\n";
            }
        }
        return;
    }

    // Range check
    double dx = playerX_ - (static_cast<double>(blockX) + 0.5);
    double dy = playerY_ - (static_cast<double>(blockY) + 0.5);
    double dz = playerZ_ - (static_cast<double>(blockZ) + 0.5);
    if (dx * dx + dy * dy + dz * dz > 64.0) return;

    auto& worlds = server_.getWorlds();
    if (worlds.empty()) return;
    WorldServer* world = worlds[0].get();

    // ─── Block activation check ──────────────────────────────────────────
    // Java: onBlockActivated() — crafting table, chests, furnaces, etc.
    Block* clickedBlock = world->getBlock(blockX, static_cast<int32_t>(blockY), blockZ);
    int32_t clickedBlockId = clickedBlock ? Block::getIdFromBlock(clickedBlock) : 0;

    // Crafting table (block ID 58) — Java: BlockWorkbench.onBlockActivated()
    // Sneaking players bypass activation to place blocks
    if (clickedBlockId == 58 && !isSneaking_) {
        openWorkbench(conn, blockX, static_cast<int32_t>(blockY), blockZ);
        return;
    }

    // Chest (block ID 54) — Java: BlockChest.onBlockActivated()
    if (clickedBlockId == 54 && !isSneaking_) {
        openChest(conn, blockX, static_cast<int32_t>(blockY), blockZ);
        return;
    }

    // Furnace (block ID 61=furnace, 62=lit_furnace) — Java: BlockFurnace.onBlockActivated()
    if ((clickedBlockId == 61 || clickedBlockId == 62) && !isSneaking_) {
        openFurnace(conn, blockX, static_cast<int32_t>(blockY), blockZ);
        return;
    }

    // Ender Chest (block ID 130) — Java: BlockEnderChest.onBlockActivated()
    if (clickedBlockId == 130 && !isSneaking_) {
        openEnderChest(conn, blockX, static_cast<int32_t>(blockY), blockZ);
        return;
    }

    // Bed (block ID 26) — Java: BlockBed.onBlockActivated()
    if (clickedBlockId == 26 && !isSneaking_) {
        int32_t by = static_cast<int32_t>(blockY);
        int32_t bedMeta = world->getBlockMetadata(blockX, by, blockZ);

        // Check if it's nighttime — Java: worldTime >= 12541 && worldTime <= 23458
        int64_t worldTime = world->getWorldTime() % 24000;
        if (worldTime < 12541 || worldTime > 23458) {
            // "You can only sleep at night"
            sendChatMessage(conn, std::string("\xc2\xa7") + "cYou can only sleep at night");
            return;
        }

        // Check if bed is occupied (bit 4)
        if (bedMeta & 0x04) {
            sendChatMessage(conn, std::string("\xc2\xa7") + "cThis bed is occupied");
            return;
        }

        // Spawn point setting — would go here when player entity has spawn fields
        // Java: EntityPlayer.setSpawnChunk(bedPos, false)

        // Skip the night immediately (simplified — no actual sleep animation)
        // Set world time to 0 (sunrise)
        world->setWorldTime(0);

        // Broadcast time to all players
        server_.broadcastTimeUpdate();

        // Send chat message
        server_.broadcastChatMessage(std::string("\xc2\xa7") + "e" + playerName_ + " slept through the night");

        // Clear weather if raining — Java: WorldServer.wakeAllPlayers → resetRainAndThunder
        // Simplified: clear rain on sleep
        // (weather clearing handled by world tick)

        sendChatMessage(conn, std::string("\xc2\xa7") + "aSpawn point set to bed location");
        return;
    }

    if (clickedBlockId == 116 && !isSneaking_) {
        openWindowId_ = 10; // Use unique window ID for enchanting
        enchantTableX_ = blockX;
        enchantTableY_ = static_cast<int32_t>(blockY);
        enchantTableZ_ = blockZ;

        // Count bookshelves in a ring 2 blocks out (Java: ContainerEnchantment.onCraftMatrixChanged)
        int bookshelfCount = 0;
        auto& worlds = server_.getWorlds();
        if (!worlds.empty()) {
            auto& w = worlds[0];
            for (int dz = -1; dz <= 1; ++dz) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dz == 0) continue;
                    // Check air gap between table and bookshelf
                    Block* gap1 = w->getBlock(blockX + dx, enchantTableY_, blockZ + dz);
                    Block* gap2 = w->getBlock(blockX + dx, enchantTableY_ + 1, blockZ + dz);
                    int gapId1 = gap1 ? Block::getIdFromBlock(gap1) : 0;
                    int gapId2 = gap2 ? Block::getIdFromBlock(gap2) : 0;
                    if (gapId1 != 0 || gapId2 != 0) continue;

                    // Check for bookshelves at distance 2
                    auto checkShelf = [&](int cx, int cy, int cz) {
                        Block* b = w->getBlock(cx, cy, cz);
                        if (b && Block::getIdFromBlock(b) == 47) ++bookshelfCount; // 47 = bookshelf
                    };
                    checkShelf(blockX + dx * 2, enchantTableY_, blockZ + dz * 2);
                    checkShelf(blockX + dx * 2, enchantTableY_ + 1, blockZ + dz * 2);
                    if (dx != 0 && dz != 0) {
                        checkShelf(blockX + dx * 2, enchantTableY_, blockZ + dz);
                        checkShelf(blockX + dx * 2, enchantTableY_ + 1, blockZ + dz);
                        checkShelf(blockX + dx, enchantTableY_, blockZ + dz * 2);
                        checkShelf(blockX + dx, enchantTableY_ + 1, blockZ + dz * 2);
                    }
                }
            }
        }
        if (bookshelfCount > 15) bookshelfCount = 15;

        // Generate 3 enchantment levels (simplified Java: EnchantmentHelper.calcItemStackEnchantability)
        std::mt19937 rng(std::random_device{}());
        for (int i = 0; i < 3; ++i) {
            int base = 1 + (bookshelfCount > 0 ? std::uniform_int_distribution<>(0, bookshelfCount)(rng) : 0)
                         + (bookshelfCount > 0 ? std::uniform_int_distribution<>(0, bookshelfCount)(rng) : 0);
            int level = static_cast<int>(base * (1.0f + static_cast<float>(i) / 3.0f));
            if (level < i + 1) level = i + 1;
            enchantLevels_[i] = level;
        }

        // Send S2D OpenWindow (type 4 = enchanting table)
        {
            std::vector<uint8_t> pkt;
            writeVarInt(pkt, ClientboundPacket::OpenWindow);
            writeByte(pkt, static_cast<uint8_t>(openWindowId_));
            writeByte(pkt, 4); // Type 4 = enchanting table
            writeString(pkt, "Enchant");
            writeByte(pkt, 0); // 0 slots (enchanting uses a single slot, handled differently)
            writeByte(pkt, 1); // Use provided title
            conn.sendPacket(std::move(pkt));
        }

        // Send S31 WindowProperty for 3 enchantment levels
        for (int i = 0; i < 3; ++i) {
            std::vector<uint8_t> propPkt;
            writeVarInt(propPkt, ClientboundPacket::WindowProperty);
            writeByte(propPkt, static_cast<uint8_t>(openWindowId_));
            writeShort(propPkt, static_cast<int16_t>(i));
            writeShort(propPkt, static_cast<int16_t>(enchantLevels_[i]));
            conn.sendPacket(std::move(propPkt));
        }

        // Send S30 WindowItems (37 slots: 1 enchant + 36 player inv)
        sendWindowItems(conn);
        return;
    }

    // Brewing Stand (block ID 117) — Java: BlockBrewingStand.onBlockActivated()
    if (clickedBlockId == 117 && !isSneaking_) {
        openWindowId_ = 11;
        openWindowType_ = 5; // Brewing stand

        int64_t key = (static_cast<int64_t>(blockX) & 0x3FFFFFFLL) << 38 |
                      (static_cast<int64_t>(blockY) & 0xFFFLL) << 26 |
                      (static_cast<int64_t>(blockZ) & 0x3FFFFFFLL);

        auto& brewData = server_.getOrCreateBrewingStand(key);

        // S2D OpenWindow (type 5 = brewing stand)
        {
            std::vector<uint8_t> pkt;
            writeVarInt(pkt, ClientboundPacket::OpenWindow);
            writeByte(pkt, static_cast<uint8_t>(openWindowId_));
            writeByte(pkt, 5); // Type 5 = brewing stand
            writeString(pkt, "Brewing Stand");
            writeByte(pkt, 4); // 4 slots: 3 potion output + 1 ingredient
            writeByte(pkt, 1); // Use provided title
            conn.sendPacket(std::move(pkt));
        }

        // S31 WindowProperty — brew time
        {
            std::vector<uint8_t> propPkt;
            writeVarInt(propPkt, ClientboundPacket::WindowProperty);
            writeByte(propPkt, static_cast<uint8_t>(openWindowId_));
            writeShort(propPkt, 0); // Property 0 = brew time
            writeShort(propPkt, static_cast<int16_t>(brewData.brewTime));
            conn.sendPacket(std::move(propPkt));
        }

        // S30 WindowItems (40 slots: 4 brewing + 36 player inv)
        sendWindowItems(conn);
        return;
    }

    // Dispenser (block ID 23) / Dropper (block ID 158)
    // Java: BlockDispenser.onBlockActivated() / BlockDropper
    if ((clickedBlockId == 23 || clickedBlockId == 158) && !isSneaking_) {
        openWindowId_ = 12;
        openWindowType_ = (clickedBlockId == 23) ? 3 : 6; // 3=dispenser, 6=dropper

        int64_t key = (static_cast<int64_t>(blockX) & 0x3FFFFFFLL) << 38 |
                      (static_cast<int64_t>(blockY) & 0xFFFLL) << 26 |
                      (static_cast<int64_t>(blockZ) & 0x3FFFFFFLL);

        auto& dispData = server_.getOrCreateDispenser(key);

        // S2D OpenWindow
        {
            std::vector<uint8_t> pkt;
            writeVarInt(pkt, ClientboundPacket::OpenWindow);
            writeByte(pkt, static_cast<uint8_t>(openWindowId_));
            writeByte(pkt, static_cast<uint8_t>(openWindowType_));
            writeString(pkt, (clickedBlockId == 23) ? "Dispenser" : "Dropper");
            writeByte(pkt, 9); // 9 slots
            writeByte(pkt, 1); // Use provided title
            conn.sendPacket(std::move(pkt));
        }

        // S30 WindowItems (45 slots: 9 container + 36 player inv)
        sendWindowItems(conn);
        return;
    }

    // Hopper (block ID 154) — Java: BlockHopper.onBlockActivated()
    if (clickedBlockId == 154 && !isSneaking_) {
        openWindowId_ = 13;
        openWindowType_ = 5; // Hopper uses same type as brewing stand? No, type 9

        int64_t key = (static_cast<int64_t>(blockX) & 0x3FFFFFFLL) << 38 |
                      (static_cast<int64_t>(blockY) & 0xFFFLL) << 26 |
                      (static_cast<int64_t>(blockZ) & 0x3FFFFFFLL);

        // Use dedicated hopper storage (5 slots + cooldown)
        auto& hopData = server_.getOrCreateHopper(key);

        // S2D OpenWindow (type 9 = hopper)
        {
            std::vector<uint8_t> pkt;
            writeVarInt(pkt, ClientboundPacket::OpenWindow);
            writeByte(pkt, static_cast<uint8_t>(openWindowId_));
            writeByte(pkt, 9); // Type 9 = hopper
            writeString(pkt, "Hopper");
            writeByte(pkt, 5); // 5 slots
            writeByte(pkt, 1);
            conn.sendPacket(std::move(pkt));
        }

        sendWindowItems(conn);
        return;
    }

    // TNT (block ID 46) — right-click with flint & steel (item 259) ignites
    // Java: BlockTNT.onBlockActivated → ItemFlintAndSteel
    if (clickedBlockId == 46) {
        auto held = inventory_.getCurrentItem();
        if (held && held->getItemId() == 259) { // Flint and steel
            // Set TNT to air
            if (!server_.getWorlds().empty()) {
                server_.getWorlds()[0]->setBlock(blockX, static_cast<int32_t>(blockY), blockZ, nullptr);
            }
            server_.broadcastBlockChange(blockX, static_cast<int32_t>(blockY), blockZ, 0, 0);

            // Play TNT fuse sound
            server_.broadcastSound("game.tnt.primed",
                static_cast<double>(blockX) + 0.5,
                static_cast<double>(blockY) + 0.5,
                static_cast<double>(blockZ) + 0.5,
                1.0f, 1.0f);

            // Damage flint & steel
            damageHeldItem(1);
            sendWindowItems(conn);

            // Create explosion (power 4.0, no fire, break blocks)
            // In vanilla, TNT has a 4-second fuse — we do instant for simplicity
            server_.createExplosion(
                static_cast<double>(blockX) + 0.5,
                static_cast<double>(blockY) + 0.5,
                static_cast<double>(blockZ) + 0.5,
                4.0f, false, true);
            return;
        }
    }

    // ─── Hoe tilling ──────────────────────────────────────────────────
    // Java: ItemHoe.onItemUse() — right-click grass(2)/dirt(3) → farmland(60)
    auto heldForUse = inventory_.getCurrentItem();
    if (heldForUse && (heldForUse->getItemId() >= 290 && heldForUse->getItemId() <= 294)) {
        // Hoe IDs: wood=290, stone=291, iron=292, diamond=293, gold=294
        if (clickedBlockId == 2 || clickedBlockId == 3) { // Grass or dirt
            if (!server_.getWorlds().empty()) {
                auto& w = server_.getWorlds()[0];
                int32_t by = static_cast<int32_t>(blockY);
                w->setBlock(blockX, by, blockZ, Block::getBlockById(60)); // Farmland
                w->setBlockMetadata(blockX, by, blockZ, 0);
                server_.broadcastBlockChange(blockX, by, blockZ, 60, 0);
                server_.broadcastSound("dig.gravel",
                    static_cast<double>(blockX) + 0.5, static_cast<double>(by) + 0.5,
                    static_cast<double>(blockZ) + 0.5, 1.0f, 0.8f);
                if (gameMode_ != 1) damageHeldItem(1);
            }
            return;
        }
    }

    // ─── Flint and Steel (259) — place fire on clicked face ──────────
    // Java: ItemFlintAndSteel.onItemUse()
    if (heldForUse && heldForUse->getItemId() == 259 && clickedBlockId != 46) {
        int32_t px = blockX, py = static_cast<int32_t>(blockY), pz = blockZ;
        switch (direction) {
            case 0: --py; break; case 1: ++py; break;
            case 2: --pz; break; case 3: ++pz; break;
            case 4: --px; break; case 5: ++px; break;
            default: break;
        }
        if (py >= 0 && py < 256 && !server_.getWorlds().empty()) {
            auto& w = server_.getWorlds()[0];
            Block* target = w->getBlock(px, py, pz);
            int targetId = target ? Block::getIdFromBlock(target) : 0;
            if (targetId == 0) {
                w->setBlock(px, py, pz, Block::getBlockById(51)); // Fire
                w->setBlockMetadata(px, py, pz, 0);
                server_.broadcastBlockChange(px, py, pz, 51, 0);
                server_.broadcastSound("fire.ignite",
                    static_cast<double>(px) + 0.5, static_cast<double>(py) + 0.5,
                    static_cast<double>(pz) + 0.5, 1.0f, 1.0f);
                if (gameMode_ != 1) damageHeldItem(1);
            }
        }
        return;
    }

    // ─── Shears (359) — instant-break leaves for leaf block drops ────
    // Java: ItemShears.onBlockDestroyed() — shears on leaves give the leaf block
    // (This is handled differently in vanilla but we intercept here for right-click shearing)

    // ─── Bucket interactions ──────────────────────────────────────────
    // Java: ItemBucket.onItemRightClick → tryPlaceContainedLiquid / ItemBucket → tryPickup

    // Water bucket (326) / Lava bucket (327) — place liquid source
    auto heldItem = inventory_.getCurrentItem();
    if (heldItem && (heldItem->getItemId() == 326 || heldItem->getItemId() == 327)) {
        int32_t liquidBlockId = (heldItem->getItemId() == 326) ? 9 : 11; // Still water / still lava
        // Place at face offset
        int32_t px = blockX, py = static_cast<int32_t>(blockY), pz = blockZ;
        switch (direction) {
            case 0: --py; break; case 1: ++py; break;
            case 2: --pz; break; case 3: ++pz; break;
            case 4: --px; break; case 5: ++px; break;
            default: break;
        }
        if (py >= 0 && py < 256 && !server_.getWorlds().empty()) {
            auto& w = server_.getWorlds()[0];
            Block* target = w->getBlock(px, py, pz);
            int targetId = target ? Block::getIdFromBlock(target) : 0;
            // Only place into air or replaceable blocks (tall grass, etc.)
            if (targetId == 0 || targetId == 31 || targetId == 32 || targetId == 37 || targetId == 38) {
                w->setBlock(px, py, pz, Block::getBlockById(liquidBlockId));
                w->setBlockMetadata(px, py, pz, 0);
                server_.broadcastBlockChange(px, py, pz, liquidBlockId, 0);
                // Play liquid place sound
                server_.broadcastSound(liquidBlockId == 9 ? "random.splash" : "random.fizz",
                    static_cast<double>(px) + 0.5, static_cast<double>(py) + 0.5,
                    static_cast<double>(pz) + 0.5, 1.0f, 1.0f);
                // Replace bucket with empty bucket (consume in survival)
                if (gameMode_ != 1) {
                    ItemStack emptyBucket(325, 1, 0);
                    inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), emptyBucket);
                    sendWindowItems(conn);
                }
            }
        }
        return;
    }

    // Empty bucket (325) — pick up water/lava source
    if (heldItem && heldItem->getItemId() == 325) {
        Block* target = world->getBlock(blockX, static_cast<int32_t>(blockY), blockZ);
        int targetId = target ? Block::getIdFromBlock(target) : 0;
        int targetMeta = world->getBlockMetadata(blockX, static_cast<int32_t>(blockY), blockZ);
        // Can only pick up source blocks (meta == 0):
        // Still water (9), flowing water (8), still lava (11), flowing lava (10)
        if ((targetId == 8 || targetId == 9) && targetMeta == 0) {
            // Pick up water
            world->setBlock(blockX, static_cast<int32_t>(blockY), blockZ, Block::getBlockById(0));
            server_.broadcastBlockChange(blockX, static_cast<int32_t>(blockY), blockZ, 0, 0);
            if (gameMode_ != 1) {
                ItemStack waterBucket(326, 1, 0);
                inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), waterBucket);
                sendWindowItems(conn);
            }
            return;
        }
        if ((targetId == 10 || targetId == 11) && targetMeta == 0) {
            // Pick up lava
            world->setBlock(blockX, static_cast<int32_t>(blockY), blockZ, Block::getBlockById(0));
            server_.broadcastBlockChange(blockX, static_cast<int32_t>(blockY), blockZ, 0, 0);
            if (gameMode_ != 1) {
                ItemStack lavaBucket(327, 1, 0);
                inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), lavaBucket);
                sendWindowItems(conn);
            }
            return;
        }
    }

    // Bonemeal (351 with damage 15) — Java: ItemDye.onItemUse()
    // Triggers growth particles (S28 effect 2005) and potentially grows plants
    if (heldItem && heldItem->getItemId() == 351 && heldItem->getDamage() == 15) {
        int32_t bx = blockX, by = static_cast<int32_t>(blockY), bz = blockZ;
        // Spawn bonemeal particles
        server_.broadcastEffect(2005, bx, by, bz, 0);
        // Consume in survival
        if (gameMode_ != 1) {
            if (heldItem->getStackSize() > 1) {
                ItemStack newStack(351, heldItem->getStackSize() - 1, 15);
                inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), newStack);
            } else {
                inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), std::nullopt);
            }
            sendWindowItems(conn);
        }
        // Growth mechanics: grow grass → flowers, saplings → trees, crops advance
        if (!server_.getWorlds().empty()) {
            auto& w = server_.getWorlds()[0];
            // Grass block (2) → spawn flowers in 7×7 area
            if (clickedBlockId == 2) {
                std::mt19937 rng(std::random_device{}());
                for (int i = 0; i < 5; ++i) {
                    int fx = bx + std::uniform_int_distribution<>(-3, 3)(rng);
                    int fz = bz + std::uniform_int_distribution<>(-3, 3)(rng);
                    int fy = by + 1;
                    Block* above = w->getBlock(fx, fy, fz);
                    int aboveId = above ? Block::getIdFromBlock(above) : 0;
                    if (aboveId == 0) {
                        // Place tall grass (31, meta 1)
                        w->setBlock(fx, fy, fz, Block::getBlockById(31));
                        w->setBlockMetadata(fx, fy, fz, 1);
                        server_.broadcastBlockChange(fx, fy, fz, 31, 1);
                    }
                }
            }
            // Wheat (59), Carrots (141), Potatoes (142) — advance to max growth (meta 7)
            if (clickedBlockId == 59 || clickedBlockId == 141 || clickedBlockId == 142) {
                int meta = w->getBlockMetadata(bx, by, bz);
                std::mt19937 rngCrop(std::random_device{}());
                int growth = std::uniform_int_distribution<>(2, 5)(rngCrop);
                int newMeta = std::min(meta + growth, 7);
                w->setBlockMetadata(bx, by, bz, newMeta);
                server_.broadcastBlockChange(bx, by, bz, clickedBlockId, newMeta);
            }
        }
        return;
    }

    // Bed (block ID 26) — Java: BlockBed.onBlockActivated()
    if (clickedBlockId == 26 && !isSneaking_) {
        int64_t worldTime = server_.getWorldTime() % 24000;
        if (worldTime >= 12541 || worldTime < 0) {
            // Night time — set spawn and skip to morning
            // Java: EntityPlayer.trySleep() → S0A UseBed
            std::vector<uint8_t> bedPkt;
            writeVarInt(bedPkt, ClientboundPacket::UseBed);
            writeInt(bedPkt, entityId_);
            writeInt(bedPkt, blockX);
            writeByte(bedPkt, static_cast<uint8_t>(blockY));
            writeInt(bedPkt, blockZ);
            conn.sendPacket(std::move(bedPkt));

            // Skip to morning
            server_.setWorldTime(0);

            // Broadcast time update to all players
            server_.broadcastTimeUpdate();

            sendChatMessage(conn, "\xC2\xA7" "7Good morning!");
        } else {
            sendChatMessage(conn, "\xC2\xA7" "cYou can only sleep at night");
        }
        return;
    }

    // ─── Interactive blocks (metadata toggle + sound + block change) ──────
    // Java reference: Block.onBlockActivated() for each block type

    // Wooden door (64) — toggle open/close via metadata bit 0x04
    // Java: BlockDoor.onBlockActivated()
    if (clickedBlockId == 64 && !isSneaking_) {
        if (!server_.getWorlds().empty()) {
            auto& world = server_.getWorlds()[0];
            int32_t by = static_cast<int32_t>(blockY);
            int meta = world->getBlockMetadata(blockX, by, blockZ);
            // Check if upper/lower half
            if (meta & 0x08) {
                // Upper half — toggle the lower half
                by -= 1;
                meta = world->getBlockMetadata(blockX, by, blockZ);
            }
            int newMeta = meta ^ 0x04; // Toggle open bit
            world->setBlockMetadata(blockX, by, blockZ, newMeta);
            server_.broadcastBlockChange(blockX, by, blockZ, 64, newMeta);
            // Also update the other half
            int32_t otherY = (meta & 0x08) ? by - 1 : by + 1;
            server_.broadcastBlockChange(blockX, otherY, blockZ, 64,
                world->getBlockMetadata(blockX, otherY, blockZ));
            server_.broadcastSound((newMeta & 0x04) ? "random.door_open" : "random.door_close",
                static_cast<double>(blockX) + 0.5, static_cast<double>(by) + 0.5,
                static_cast<double>(blockZ) + 0.5, 1.0f, 1.0f);
        }
        return;
    }

    // Trapdoor (96) — toggle open/close via metadata bit 0x04
    // Java: BlockTrapDoor.onBlockActivated()
    if (clickedBlockId == 96 && !isSneaking_) {
        if (!server_.getWorlds().empty()) {
            auto& world = server_.getWorlds()[0];
            int32_t by = static_cast<int32_t>(blockY);
            int meta = world->getBlockMetadata(blockX, by, blockZ);
            int newMeta = meta ^ 0x04;
            world->setBlockMetadata(blockX, by, blockZ, newMeta);
            server_.broadcastBlockChange(blockX, by, blockZ, 96, newMeta);
            server_.broadcastSound((newMeta & 0x04) ? "random.door_open" : "random.door_close",
                static_cast<double>(blockX) + 0.5, static_cast<double>(by) + 0.5,
                static_cast<double>(blockZ) + 0.5, 1.0f, 1.0f);
        }
        return;
    }

    // Fence gate (107) — toggle open/close via metadata bit 0x04
    // Java: BlockFenceGate.onBlockActivated()
    if (clickedBlockId == 107 && !isSneaking_) {
        if (!server_.getWorlds().empty()) {
            auto& world = server_.getWorlds()[0];
            int32_t by = static_cast<int32_t>(blockY);
            int meta = world->getBlockMetadata(blockX, by, blockZ);
            int newMeta = meta ^ 0x04;
            world->setBlockMetadata(blockX, by, blockZ, newMeta);
            server_.broadcastBlockChange(blockX, by, blockZ, 107, newMeta);
            server_.broadcastSound((newMeta & 0x04) ? "random.door_open" : "random.door_close",
                static_cast<double>(blockX) + 0.5, static_cast<double>(by) + 0.5,
                static_cast<double>(blockZ) + 0.5, 1.0f, 1.0f);
        }
        return;
    }

    // Lever (69) — toggle on/off via metadata bit 0x08
    // Java: BlockLever.onBlockActivated()
    if (clickedBlockId == 69 && !isSneaking_) {
        if (!server_.getWorlds().empty()) {
            auto& world = server_.getWorlds()[0];
            int32_t by = static_cast<int32_t>(blockY);
            int meta = world->getBlockMetadata(blockX, by, blockZ);
            int newMeta = meta ^ 0x08;
            world->setBlockMetadata(blockX, by, blockZ, newMeta);
            server_.broadcastBlockChange(blockX, by, blockZ, 69, newMeta);
            server_.broadcastSound("random.click",
                static_cast<double>(blockX) + 0.5, static_cast<double>(by) + 0.5,
                static_cast<double>(blockZ) + 0.5, 0.3f,
                (newMeta & 0x08) ? 0.6f : 0.5f);
        }
        return;
    }

    // Stone button (77), wooden button (143) — momentary press via metadata bit 0x08
    // Java: BlockButton.onBlockActivated()
    if ((clickedBlockId == 77 || clickedBlockId == 143) && !isSneaking_) {
        if (!server_.getWorlds().empty()) {
            auto& world = server_.getWorlds()[0];
            int32_t by = static_cast<int32_t>(blockY);
            int meta = world->getBlockMetadata(blockX, by, blockZ);
            if (!(meta & 0x08)) { // Not already pressed
                int newMeta = meta | 0x08;
                world->setBlockMetadata(blockX, by, blockZ, newMeta);
                server_.broadcastBlockChange(blockX, by, blockZ, clickedBlockId, newMeta);
                server_.broadcastSound("random.click",
                    static_cast<double>(blockX) + 0.5, static_cast<double>(by) + 0.5,
                    static_cast<double>(blockZ) + 0.5, 0.3f, 0.6f);
                // Note: auto-reset after 20/30 ticks not implemented (needs scheduled tick)
            }
        }
        return;
    }

    // Trapped Chest (block ID 146) — same as normal chest but sends redstone signal
    // Java: BlockChest.onBlockActivated() — same behavior
    if (clickedBlockId == 146 && !isSneaking_) {
        openChest(conn, blockX, static_cast<int32_t>(blockY), blockZ);
        return;
    }

    // Cake (block ID 92) — Java: BlockCake.func_150033_b()
    // Each right-click eats a slice (metadata 0-6), restores 2 food, plays eating sound
    if (clickedBlockId == 92 && !isSneaking_) {
        if (foodStats_.getFoodLevel() < 20) { // Can only eat if not full
            if (!server_.getWorlds().empty()) {
                auto& w = server_.getWorlds()[0];
                int32_t by = static_cast<int32_t>(blockY);
                int meta = w->getBlockMetadata(blockX, by, blockZ);
                if (meta < 6) {
                    // Eat a slice
                    w->setBlockMetadata(blockX, by, blockZ, meta + 1);
                    server_.broadcastBlockChange(blockX, by, blockZ, 92, meta + 1);
                    // Restore 2 food, 0.4 saturation per slice
                    foodStats_.addStats(2, 0.4f);
                    sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
                    // Eating sound
                    server_.broadcastSound("random.eat",
                        static_cast<double>(blockX) + 0.5, static_cast<double>(by) + 0.5,
                        static_cast<double>(blockZ) + 0.5, 1.0f, 1.0f);
                } else {
                    // Last slice eaten — remove cake
                    w->setBlock(blockX, by, blockZ, Block::getBlockById(0));
                    w->setBlockMetadata(blockX, by, blockZ, 0);
                    server_.broadcastBlockChange(blockX, by, blockZ, 0, 0);
                    server_.broadcastEffect(2001, blockX, by, blockZ, 92);
                    foodStats_.addStats(2, 0.4f);
                    sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
                    server_.broadcastSound("random.eat",
                        static_cast<double>(blockX) + 0.5, static_cast<double>(by) + 0.5,
                        static_cast<double>(blockZ) + 0.5, 1.0f, 1.0f);
                }
            }
        }
        return;
    }

    // Beacon (block ID 138) — Java: BlockBeacon.onBlockActivated()
    // Opens beacon GUI (S2D window type 7)
    if (clickedBlockId == 138 && !isSneaking_) {
        openWindowId_ = 15;
        openWindowType_ = 7; // Beacon

        {
            std::vector<uint8_t> pkt;
            writeVarInt(pkt, ClientboundPacket::OpenWindow);
            writeByte(pkt, static_cast<uint8_t>(openWindowId_));
            writeByte(pkt, 7); // Type 7 = beacon
            writeString(pkt, "Beacon");
            writeByte(pkt, 1); // 1 slot (payment slot)
            writeByte(pkt, 1);
            conn.sendPacket(std::move(pkt));
        }

        // S31 WindowProperty — send beacon properties (power level, effect IDs)
        // Property 0 = power level (0-4)
        // Property 1 = primary effect
        // Property 2 = secondary effect
        for (int i = 0; i < 3; ++i) {
            std::vector<uint8_t> propPkt;
            writeVarInt(propPkt, ClientboundPacket::WindowProperty);
            writeByte(propPkt, static_cast<uint8_t>(openWindowId_));
            writeShort(propPkt, static_cast<int16_t>(i));
            writeShort(propPkt, 0); // Default: no power/effects
            conn.sendPacket(std::move(propPkt));
        }

        sendWindowItems(conn);
        return;
    }

    // Note block (25) — play note and increment pitch
    // Java: BlockNote.onBlockActivated()

    // Bed (block ID 26) — Java: BlockBed.onBlockActivated()
    // Right-click at night: skip to dawn. Day: "You can only sleep at night"
    if (clickedBlockId == 26 && !isSneaking_) {
        if (!server_.getWorlds().empty()) {
            auto& w = server_.getWorlds()[0];
            int64_t timeOfDay = w->getWorldTime() % 24000;
            if (timeOfDay >= 12542) {
                // Night — skip to dawn
                int64_t currentCycle = (w->getWorldTime() / 24000) * 24000;
                w->setWorldTime(currentCycle + 24000); // Next dawn (0)
                server_.broadcastTimeUpdate();
                std::vector<uint8_t> chatPkt;
                writeVarInt(chatPkt, ClientboundPacket::ChatMessage);
                writeString(chatPkt, "{\"text\":\"\\u00a77Good morning!\"}");
                conn.sendPacket(std::move(chatPkt));
            } else {
                std::vector<uint8_t> chatPkt;
                writeVarInt(chatPkt, ClientboundPacket::ChatMessage);
                writeString(chatPkt, "{\"text\":\"You can only sleep at night\"}");
                conn.sendPacket(std::move(chatPkt));
            }
        }
        return;
    }

    // Repeater (block IDs 93=unpowered, 94=powered) — Java: BlockRedstoneRepeater.onBlockActivated()
    // Right-click cycles delay: bits 2-3 of metadata → 0,1,2,3 (1-4 tick delay)
    if ((clickedBlockId == 93 || clickedBlockId == 94) && !isSneaking_) {
        if (!server_.getWorlds().empty()) {
            auto& w = server_.getWorlds()[0];
            int32_t by = static_cast<int32_t>(blockY);
            int meta = w->getBlockMetadata(blockX, by, blockZ);
            int delay = (meta & 0x0C) >> 2;
            delay = (delay + 1) & 3; // Cycle 0→1→2→3→0
            int newMeta = (delay << 2) | (meta & 0x03); // Preserve direction bits
            w->setBlockMetadata(blockX, by, blockZ, newMeta);
            server_.broadcastBlockChange(blockX, by, blockZ, clickedBlockId, newMeta);
            // Click sound
            server_.broadcastSound("random.click",
                static_cast<double>(blockX) + 0.5, static_cast<double>(by) + 0.5,
                static_cast<double>(blockZ) + 0.5, 0.3f, 0.6f);
        }
        return;
    }

    // Comparator (block IDs 149=unpowered, 150=powered) — Java: BlockRedstoneComparator.onBlockActivated()
    // Right-click toggles subtract mode (bit 0x04 of metadata)
    if ((clickedBlockId == 149 || clickedBlockId == 150) && !isSneaking_) {
        if (!server_.getWorlds().empty()) {
            auto& w = server_.getWorlds()[0];
            int32_t by = static_cast<int32_t>(blockY);
            int meta = w->getBlockMetadata(blockX, by, blockZ);
            int newMeta = meta ^ 0x04; // Toggle subtract mode
            w->setBlockMetadata(blockX, by, blockZ, newMeta);
            server_.broadcastBlockChange(blockX, by, blockZ, clickedBlockId, newMeta);
            // Torch sound for mode change
            server_.broadcastSound("random.click",
                static_cast<double>(blockX) + 0.5, static_cast<double>(by) + 0.5,
                static_cast<double>(blockZ) + 0.5, 0.3f, (newMeta & 0x04) ? 0.55f : 0.5f);
        }
        return;
    }

    // Daylight sensor (151=normal, 178=inverted) — toggle mode
    // Java: BlockDaylightDetector.onBlockActivated()
    // Right-click swaps between daylight_detector (151) and daylight_detector_inverted (178)
    if ((clickedBlockId == 151 || clickedBlockId == 178) && !isSneaking_) {
        if (!server_.getWorlds().empty()) {
            auto& w = server_.getWorlds()[0];
            int32_t by = static_cast<int32_t>(blockY);
            int meta = w->getBlockMetadata(blockX, by, blockZ);
            int32_t newBlockId = (clickedBlockId == 151) ? 178 : 151;
            w->setBlock(blockX, by, blockZ, Block::getBlockById(newBlockId));
            w->setBlockMetadata(blockX, by, blockZ, meta);
            server_.broadcastBlockChange(blockX, by, blockZ, newBlockId, meta);
            server_.broadcastSound("random.click",
                static_cast<double>(blockX) + 0.5, static_cast<double>(by) + 0.5,
                static_cast<double>(blockZ) + 0.5, 0.3f, 0.6f);
        }
        return;
    }

    // Anvil (block ID 145) — Java: BlockAnvil.onBlockActivated()
    // Opens repair/rename GUI (S2D window type 8)
    if (clickedBlockId == 145 && !isSneaking_) {
        openWindowId_ = 14;
        openWindowType_ = 8; // Anvil

        // S2D OpenWindow (type 8 = anvil)
        {
            std::vector<uint8_t> pkt;
            writeVarInt(pkt, ClientboundPacket::OpenWindow);
            writeByte(pkt, static_cast<uint8_t>(openWindowId_));
            writeByte(pkt, 8); // Type 8 = anvil
            writeString(pkt, "Repair");
            writeByte(pkt, 0); // 0 container slots (handled like enchanting)
            writeByte(pkt, 1); // Use provided title
            conn.sendPacket(std::move(pkt));
        }

        sendWindowItems(conn);
        return;
    }

    // Note block (block ID 25) — Java: BlockNote.onBlockActivated()
    if (clickedBlockId == 25 && !isSneaking_) {
        if (!server_.getWorlds().empty()) {
            auto& world = server_.getWorlds()[0];
            int32_t by = static_cast<int32_t>(blockY);
            int meta = world->getBlockMetadata(blockX, by, blockZ);
            int newMeta = (meta + 1) % 25; // 0-24 pitch range
            world->setBlockMetadata(blockX, by, blockZ, newMeta);
            // Play note — S28 Block Action (noteblock event)
            float pitch = std::pow(2.0f, static_cast<float>(newMeta - 12) / 12.0f);
            server_.broadcastSound("note.harp",
                static_cast<double>(blockX) + 0.5, static_cast<double>(by) + 0.5,
                static_cast<double>(blockZ) + 0.5, 3.0f, pitch);
            // Spawn note particle
            server_.broadcastParticle("note",
                static_cast<float>(blockX) + 0.5f, static_cast<float>(by) + 1.2f,
                static_cast<float>(blockZ) + 0.5f,
                static_cast<float>(newMeta) / 24.0f, 0.0f, 0.0f, 0.0f, 1);
        }
        return;
    }

    // Calculate the position of the new block based on the face clicked
    // Java reference: same offset logic as processPlayerBlockPlacement
    int32_t placeX = blockX;
    int32_t placeY = static_cast<int32_t>(blockY);
    int32_t placeZ = blockZ;

    switch (direction) {
        case 0: --placeY; break; // Bottom face — place below
        case 1: ++placeY; break; // Top face — place above
        case 2: --placeZ; break; // North face
        case 3: ++placeZ; break; // South face
        case 4: --placeX; break; // West face
        case 5: ++placeX; break; // East face
        default: return;
    }

    // Build height check
    if (placeY < 0 || placeY >= 256) return;

    // Check that target is air
    Block* targetBlock = world->getBlock(placeX, placeY, placeZ);
    if (targetBlock && Block::getIdFromBlock(targetBlock) != 0) {
        // Can't place into a non-air block — send correction
        sendBlockChange(conn, placeX, placeY, placeZ,
                       Block::getIdFromBlock(targetBlock), world->getBlockMetadata(placeX, placeY, placeZ));
        return;
    }

    // Determine which block to place
    // C08 slot format: Short itemId, Byte count, Short damage, [NBT...]
    // Java reference: Packet.readItemStackFromBuffer
    int32_t placeBlockId = 1; // stone fallback
    int16_t itemDamage = 0;
    if (length >= 12) {
        int16_t heldItemId = static_cast<int16_t>((data[10] << 8) | data[11]);
        // Read count (byte 12) and damage (bytes 13-14) if available
        if (heldItemId >= 0 && length >= 15) {
            // byte 12 = count, bytes 13-14 = damage
            itemDamage = static_cast<int16_t>((data[13] << 8) | data[14]);
        }
        if (heldItemId >= 0 && heldItemId < 256) {
            // Direct block IDs (items 0-255 correspond to blocks)
            Block* heldBlock = Block::getBlockById(heldItemId);
            if (heldBlock && Block::getIdFromBlock(heldBlock) != 0) {
                placeBlockId = heldItemId;
            }
        } else {
            // Item→block mapping for items > 255
            // Java: ItemDoor, ItemSeeds, ItemRedstone, ItemSign, ItemBed, etc.
            switch (heldItemId) {
                case 324: placeBlockId = 64; break;  // Wooden door item → door block
                case 330: placeBlockId = 71; break;  // Iron door item → iron door block
                case 331: placeBlockId = 55; break;  // Redstone dust → redstone wire
                case 323: placeBlockId = 63; break;  // Sign → standing sign
                case 338: placeBlockId = 83; break;  // Sugar cane → sugar cane block
                case 356: placeBlockId = 93; break;  // Repeater item → repeater block
                case 404: placeBlockId = 149; break; // Comparator item → comparator block
                case 355: placeBlockId = 26; break;  // Bed item → bed block
                case 295: placeBlockId = 59; break;  // Wheat seeds → wheat crop
                case 391: placeBlockId = 141; break; // Carrot → carrot crop
                case 392: placeBlockId = 142; break; // Potato → potato crop
                case 360: placeBlockId = 105; break; // Melon seeds → melon stem
                case 361: placeBlockId = 104; break; // Pumpkin seeds → pumpkin stem
                case 321: break; // Paintings — entity, not block
                case 389: placeBlockId = 140; break; // Flower pot item → flower pot block
                case 397: placeBlockId = 144; break; // Skull item → skull block
                default: break;
            }
        }
    }

    // ─── Door item placement (324/330) — 2-block tall structure ───────
    // Java: ItemDoor.onItemUse() → place lower + upper halves
    if (placeBlockId == 64 || placeBlockId == 71) {
        // Doors can only be placed on top face
        if (direction != 1) return;
        // Need air at placeY and placeY+1
        if (placeY + 1 >= 256) return;
        Block* above = world->getBlock(placeX, placeY + 1, placeZ);
        if (above && Block::getIdFromBlock(above) != 0) return;

        // Calculate facing from player yaw
        // Java: MathHelper.floor_double((yaw + 180) * 4 / 360 - 0.5) & 3
        int doorFacing = (static_cast<int>(std::floor((playerYaw_ + 180.0f) * 4.0f / 360.0f - 0.5f)) & 3);
        // doorFacing: 0=west, 1=north, 2=east, 3=south

        // Lower half: meta = doorFacing
        world->setBlock(placeX, placeY, placeZ, Block::getBlockById(placeBlockId));
        world->setBlockMetadata(placeX, placeY, placeZ, doorFacing);
        server_.broadcastBlockChange(placeX, placeY, placeZ, placeBlockId, doorFacing);

        // Upper half: meta = 8 (upper bit)
        world->setBlock(placeX, placeY + 1, placeZ, Block::getBlockById(placeBlockId));
        world->setBlockMetadata(placeX, placeY + 1, placeZ, 8);
        server_.broadcastBlockChange(placeX, placeY + 1, placeZ, placeBlockId, 8);

        // Play place sound
        server_.broadcastSound("dig.wood",
            static_cast<double>(placeX) + 0.5, static_cast<double>(placeY) + 0.5,
            static_cast<double>(placeZ) + 0.5, 1.0f, 0.8f);

        // Consume item in survival
        if (gameMode_ != 1) {
            auto held = inventory_.getCurrentItem();
            if (held && held->getStackSize() > 1) {
                ItemStack newStack(held->getItemId(), held->getStackSize() - 1, held->getDamage());
                inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), newStack);
            } else {
                inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), std::nullopt);
            }
            sendWindowItems(conn);
        }
        return;
    }

    // ─── Seed placement (295/391/392/360/361) — only on farmland ─────
    // Java: ItemSeeds.onItemUse() — requires farmland (60) below
    if (placeBlockId == 59 || placeBlockId == 141 || placeBlockId == 142 ||
        placeBlockId == 104 || placeBlockId == 105) {
        // Seeds can only be placed on farmland (60), on top face
        if (direction != 1) return;
        // The clicked block must be farmland
        Block* clickedBlock = world->getBlock(blockX, static_cast<int32_t>(blockY), blockZ);
        int clickedId = clickedBlock ? Block::getIdFromBlock(clickedBlock) : 0;
        if (clickedId != 60) return; // Must be farmland

        // Place crop at meta 0 (initial growth stage)
        world->setBlock(placeX, placeY, placeZ, Block::getBlockById(placeBlockId));
        world->setBlockMetadata(placeX, placeY, placeZ, 0);
        server_.broadcastBlockChange(placeX, placeY, placeZ, placeBlockId, 0);

        // Play place sound
        server_.broadcastSound("dig.grass",
            static_cast<double>(placeX) + 0.5, static_cast<double>(placeY) + 0.5,
            static_cast<double>(placeZ) + 0.5, 1.0f, 0.8f);

        // Consume seed in survival
        if (gameMode_ != 1) {
            auto held = inventory_.getCurrentItem();
            if (held && held->getStackSize() > 1) {
                ItemStack newStack(held->getItemId(), held->getStackSize() - 1, held->getDamage());
                inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), newStack);
            } else {
                inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), std::nullopt);
            }
            sendWindowItems(conn);
        }
        return;
    }

    // ─── Slab double-slab placement (44→43, 126→125) ────────────────
    // Java: ItemSlab.func_150936_a — placing slab on matching slab creates double
    if (placeBlockId == 44 || placeBlockId == 126) {
        // Get slab sub-type from held item damage
        auto heldSlab = inventory_.getCurrentItem();
        int slabDamage = heldSlab ? heldSlab->getDamage() : 0;

        // Check if clicking on an existing matching slab
        Block* clickedBlock = world->getBlock(blockX, blockY, blockZ);
        int clickedId = clickedBlock ? Block::getIdFromBlock(clickedBlock) : 0;
        int clickedMeta = world->getBlockMetadata(blockX, blockY, blockZ);

        if (clickedId == placeBlockId && (clickedMeta & 0x07) == (slabDamage & 0x07)) {
            // Merge into double slab
            int doubleSlabId = (placeBlockId == 44) ? 43 : 125;
            world->setBlock(blockX, blockY, blockZ, Block::getBlockById(doubleSlabId));
            world->setBlockMetadata(blockX, blockY, blockZ, slabDamage & 0x07);
            server_.broadcastBlockChange(blockX, blockY, blockZ, doubleSlabId, slabDamage & 0x07);
            server_.broadcastSound("dig.stone",
                static_cast<double>(blockX) + 0.5, static_cast<double>(blockY) + 0.5,
                static_cast<double>(blockZ) + 0.5, 1.0f, 0.8f);
            if (gameMode_ != 1) {
                if (heldSlab && heldSlab->getStackSize() > 1) {
                    ItemStack ns(heldSlab->getItemId(), heldSlab->getStackSize() - 1, heldSlab->getDamage());
                    inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), ns);
                } else {
                    inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), std::nullopt);
                }
                sendWindowItems(conn);
            }
            return;
        }
    }

    // ─── Bed item placement (355) — 2-block wide structure ───────────
    // Java: ItemBed.onItemUse() → foot + head blocks
    if (placeBlockId == 26) {
        if (direction != 1) return; // Must click top face
        // facingDir: 0=south, 1=west, 2=north, 3=east
        int32_t facingDir2 = (static_cast<int32_t>(std::floor(playerYaw_ * 4.0f / 360.0f + 0.5f)) & 3);
        // Bed meta: 0=south, 1=west, 2=north, 3=east (matching player facing)
        int headX = placeX, headZ = placeZ;
        switch (facingDir2) {
            case 0: ++headZ; break; // South → head at +Z
            case 1: --headX; break; // West → head at -X
            case 2: --headZ; break; // North → head at -Z
            case 3: ++headX; break; // East → head at +X
        }
        // Check head position is air
        Block* headBlock = world->getBlock(headX, placeY, headZ);
        if (headBlock && Block::getIdFromBlock(headBlock) != 0) return;

        // Foot: meta = facing dir
        world->setBlock(placeX, placeY, placeZ, Block::getBlockById(26));
        world->setBlockMetadata(placeX, placeY, placeZ, facingDir2);
        server_.broadcastBlockChange(placeX, placeY, placeZ, 26, facingDir2);

        // Head: meta = facing dir | 8 (head bit)
        world->setBlock(headX, placeY, headZ, Block::getBlockById(26));
        world->setBlockMetadata(headX, placeY, headZ, facingDir2 | 8);
        server_.broadcastBlockChange(headX, placeY, headZ, 26, facingDir2 | 8);

        server_.broadcastSound("dig.wood",
            static_cast<double>(placeX) + 0.5, static_cast<double>(placeY) + 0.5,
            static_cast<double>(placeZ) + 0.5, 1.0f, 0.8f);

        if (gameMode_ != 1) {
            auto held = inventory_.getCurrentItem();
            if (held && held->getStackSize() > 1) {
                ItemStack ns(held->getItemId(), held->getStackSize() - 1, held->getDamage());
                inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), ns);
            } else {
                inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), std::nullopt);
            }
            sendWindowItems(conn);
        }
        return;
    }

    // Place the block
    Block* newBlock = Block::getBlockById(placeBlockId);
    if (!newBlock) return;

    // ─── Compute placement metadata ──────────────────────────────────────
    // Java reference: Block.onBlockPlaced / onBlockPlacedBy
    int32_t meta = 0;

    // Read cursor position for sub-block precision
    // Cursor bytes are at the end of the packet after the slot data
    float cursorY = 0.5f; // default center
    // We'll approximate from direction for now since cursor parsing is complex

    // Player facing direction from yaw — Java: MathHelper.floor_double(yaw * 4 / 360 + 0.5) & 3
    int32_t facingDir = (static_cast<int32_t>(std::floor(playerYaw_ * 4.0f / 360.0f + 0.5f)) & 3);
    // facingDir: 0=south, 1=west, 2=north, 3=east

    switch (placeBlockId) {
        // ─── Stairs — Java: BlockStairs.onBlockPlaced ─────────────────
        // Meta 0-3 = facing (0=east,1=west,2=south,3=north), bit 2 = upside-down
        case 53: case 67: case 108: case 109: case 114: case 128:
        case 134: case 135: case 136: case 156: case 163: case 164: {
            // Convert player facing to stair facing (player faces towards stair)
            // Java: 0=east,1=west,2=south,3=north — opposite of player facing
            int32_t stairFacing;
            switch (facingDir) {
                case 0: stairFacing = 3; break; // Player south → stairs face north
                case 1: stairFacing = 0; break; // Player west → stairs face east
                case 2: stairFacing = 2; break; // Player north → stairs face south
                case 3: stairFacing = 1; break; // Player east → stairs face west
                default: stairFacing = 0;
            }
            meta = stairFacing;
            // Upside-down if placing on bottom face of a block
            if (direction == 0) {
                meta |= 4; // upside-down bit
            }
            break;
        }

        // ─── Slabs — Java: BlockSlab.onBlockPlaced ───────────────────
        // Meta 8 = upper half slab
        case 44: case 126: {
            if (direction == 0) {
                meta = 8; // Clicking bottom face → upper slab
            }
            // Otherwise meta 0 = lower slab
            break;
        }

        // ─── Logs — Java: BlockLog.onBlockPlaced ─────────────────────
        // Meta bits 2-3: 0=Y axis, 1=X axis(east/west), 2=Z axis(north/south), 3=bark
        case 17: case 162: {
            switch (direction) {
                case 0: case 1: meta = 0; break; // Up/Down → Y axis
                case 2: case 3: meta = 8; break; // North/South → Z axis
                case 4: case 5: meta = 4; break; // East/West → X axis
            }
            break;
        }

        // ─── Torches — Java: BlockTorch.onBlockPlaced ────────────────
        // Meta: 1=east, 2=west, 3=south, 4=north, 5=floor
        case 50: case 75: case 76: {
            switch (direction) {
                case 1: meta = 5; break; // Top face → standing
                case 2: meta = 4; break; // North face → attach north
                case 3: meta = 3; break; // South face → attach south
                case 4: meta = 2; break; // West face → attach west
                case 5: meta = 1; break; // East face → attach east
                default: meta = 5;
            }
            break;
        }

        // ─── Ladders — Java: BlockLadder.onBlockPlaced ───────────────
        // Meta: 2=north, 3=south, 4=west, 5=east
        case 65: {
            switch (direction) {
                case 2: meta = 2; break; // North face
                case 3: meta = 3; break; // South face
                case 4: meta = 4; break; // West face
                case 5: meta = 5; break; // East face
                default: meta = 2;
            }
            break;
        }

        // ─── Furnace/Chest/Pumpkin/Dispenser — facing from player yaw ─
        // Meta: 2=north, 3=south, 4=west, 5=east
        case 23: case 54: case 61: case 62: case 130: case 146: case 154: case 158: {
            switch (facingDir) {
                case 0: meta = 3; break; // South
                case 1: meta = 4; break; // West
                case 2: meta = 2; break; // North
                case 3: meta = 5; break; // East
            }
            break;
        }

        // ─── Pumpkin/Jack-o-lantern — Java: BlockPumpkin.onBlockPlaced ─
        // Meta: 0=south, 1=west, 2=north, 3=east
        case 86: case 91: {
            meta = facingDir;
            break;
        }

        // ─── Pistons — direction-based ───────────────────────────────
        // Meta: 0=down, 1=up, 2=north, 3=south, 4=west, 5=east
        case 29: case 33: {
            // Use player look to determine piston facing
            if (playerPitch_ > 45.0f) {
                meta = 0; // Looking down → piston faces down
            } else if (playerPitch_ < -45.0f) {
                meta = 1; // Looking up → piston faces up
            } else {
                switch (facingDir) {
                    case 0: meta = 3; break; // South
                    case 1: meta = 4; break; // West
                    case 2: meta = 2; break; // North
                    case 3: meta = 5; break; // East
                }
            }
            break;
        }

        // ─── Levers — Java: BlockLever.onBlockPlaced ─────────────────
        case 69: {
            switch (direction) {
                case 0: meta = 0; break; // Floor (south when off)
                case 1: meta = 5; break; // Ceiling
                case 2: meta = 4; break; // North wall
                case 3: meta = 3; break; // South wall
                case 4: meta = 2; break; // West wall
                case 5: meta = 1; break; // East wall
                default: meta = 5;
            }
            break;
        }

        // ─── Buttons — Java: BlockButton.onBlockPlaced ───────────────
        case 77: case 143: {
            switch (direction) {
                case 2: meta = 4; break; // North
                case 3: meta = 3; break; // South
                case 4: meta = 2; break; // West
                case 5: meta = 1; break; // East
                default: meta = 1;
            }
            break;
        }

        // ─── Repeaters/Comparators — facing from player yaw ──────────
        case 93: case 94: case 149: case 150: {
            meta = facingDir;
            break;
        }

        // ─── Anvil — facing from player yaw ──────────────────────────
        case 145: {
            meta = facingDir;
            break;
        }

        // ─── Colored blocks — metadata from item damage ──────────────
        // Java: ItemCloth, ItemBlock subclasses use item damage as block metadata
        case 35:  // Wool
        case 95:  // Stained glass
        case 159: // Stained clay
        case 160: // Stained glass pane
        case 171: // Carpet
        case 172: // Hardened clay (no color, but damage=0)
        {
            meta = itemDamage & 0x0F;
            break;
        }

        // ─── Sandstone variants — metadata from item damage ──────────
        case 24: case 155: { // Sandstone, quartz block
            meta = itemDamage & 0x03;
            break;
        }

        // ─── Stone brick variants ────────────────────────────────────
        case 98: { // Stone brick: 0=normal, 1=mossy, 2=cracked, 3=chiseled
            meta = itemDamage & 0x03;
            break;
        }

        // ─── Planks — wood type from damage ──────────────────────────
        case 5: {
            meta = itemDamage & 0x07; // 0=oak, 1=spruce, 2=birch, 3=jungle, 4=acacia, 5=dark_oak
            break;
        }
    }

    world->setBlock(placeX, placeY, placeZ, newBlock);
    if (meta != 0) {
        world->setBlockMetadata(placeX, placeY, placeZ, meta);
    }

    // Survival mode: consume 1 item from held slot
    // Java: ItemStack.tryPlaceItemIntoWorld → --stackSize
    // Creative mode: don't consume (Java: ItemInWorldManager.activateBlockOrUseItem restores stackSize)
    if (gameMode_ != 1) {
        auto heldStack = inventory_.getCurrentItem();
        if (heldStack) {
            int32_t remaining = heldStack->getStackSize() - 1;
            if (remaining <= 0) {
                inventory_.setInventorySlotContents(currentSlot_, std::nullopt);
            } else {
                ItemStack updated = *heldStack;
                updated.setStackSize(remaining);
                inventory_.setInventorySlotContents(currentSlot_, updated);
            }
            // Sync slot to client — ContainerPlayer hotbar = 36 + currentSlot_
            int16_t containerSlot = static_cast<int16_t>(36 + currentSlot_);
            sendSetSlot(conn, 0, containerSlot, inventory_.getStackInSlot(currentSlot_));
        }
    }

    std::cout << "[World] " << playerName_ << " placed block " << placeBlockId
              << " at " << placeX << "," << placeY << "," << placeZ << "\n";

    // Broadcast block change to all players
    server_.broadcastBlockChange(placeX, placeY, placeZ, placeBlockId, meta);

    // Play place sound — Java: block.stepSound.getPlaceSound()
    // Reuse material→sound mapping based on the PLACED block
    const char* placeSound = "dig.stone";
    switch (placeBlockId) {
        case 5: case 17: case 25: case 47: case 50: case 53: case 54: case 58:
        case 63: case 64: case 84: case 85: case 96: case 107: case 125: case 126:
        case 134: case 135: case 136: case 143: case 154: case 162: case 163: case 164:
            placeSound = "dig.wood"; break;
        case 2: case 6: case 18: case 31: case 32: case 37: case 38: case 39: case 40:
        case 86: case 91: case 83: case 104: case 105: case 106: case 111: case 161:
        case 170: case 175:
            placeSound = "dig.grass"; break;
        case 3: case 13: case 60: case 110:
            placeSound = "dig.gravel"; break;
        case 12: case 88:
            placeSound = "dig.sand"; break;
        case 35: case 81: case 92: case 171:
            placeSound = "dig.cloth"; break;
        case 78: case 80:
            placeSound = "dig.snow"; break;
        case 20: case 79: case 89: case 95: case 102: case 160: case 174:
            placeSound = "dig.glass"; break;
    }
    server_.broadcastSound(placeSound,
        static_cast<double>(placeX) + 0.5,
        static_cast<double>(placeY) + 0.5,
        static_cast<double>(placeZ) + 0.5,
        1.0f, 0.8f);

    // ─── Sand/gravel/anvil gravity — Java: BlockFalling.onBlockAdded ─
    // Simplified: instant fall instead of EntityFallingBlock
    if (placeBlockId == 12 || placeBlockId == 13 || placeBlockId == 145) {
        int fallY = placeY - 1;
        while (fallY > 0) {
            Block* below = world->getBlock(placeX, fallY, placeZ);
            int belowId = below ? Block::getIdFromBlock(below) : 0;
            if (belowId != 0 && belowId != 8 && belowId != 9 &&
                belowId != 10 && belowId != 11 && belowId != 31 &&
                belowId != 51) {
                break;
            }
            --fallY;
        }
        ++fallY; // Land on top of the solid block

        if (fallY < placeY) {
            // Remove from original position
            world->setBlock(placeX, placeY, placeZ, Block::getBlockById(0));
            server_.broadcastBlockChange(placeX, placeY, placeZ, 0, 0);
            // Place at landing position
            world->setBlock(placeX, fallY, placeZ, Block::getBlockById(placeBlockId));
            world->setBlockMetadata(placeX, fallY, placeZ, meta);
            server_.broadcastBlockChange(placeX, fallY, placeZ, placeBlockId, meta);
            // Fall sound
            server_.broadcastSound("dig.sand",
                static_cast<double>(placeX) + 0.5, static_cast<double>(fallY) + 0.5,
                static_cast<double>(placeZ) + 0.5, 0.5f, 0.6f);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Click Window — C0E inventory interactions
// Java reference: Container.slotClick(slotId, button, mode, player)
//
// C0E format: Byte windowId, Short slot, Byte button, Short actionNumber,
//             Byte mode, Slot clickedItem
// ═══════════════════════════════════════════════════════════════════════════

void PlayHandler::handleClickWindow(const uint8_t* data, size_t length, Connection& conn) {
    // Minimum: 1(windowId) + 2(slot) + 1(button) + 2(actionNumber) + 1(mode) + 2(itemId) = 9
    if (length < 9) return;

    int8_t windowId = static_cast<int8_t>(data[0]);
    int16_t slotId = static_cast<int16_t>((data[1] << 8) | data[2]);
    int8_t button = static_cast<int8_t>(data[3]);
    int16_t actionNumber = static_cast<int16_t>((data[4] << 8) | data[5]);
    int8_t mode = static_cast<int8_t>(data[6]);
    // Skip reading the clicked item — server is authoritative

    int32_t slotCount = container_ ? container_->getSlotCount() : 0;

    // Helper: send S32 ConfirmTransaction (accepted)
    auto sendConfirm = [&](bool accepted) {
        std::vector<uint8_t> pkt;
        writeVarInt(pkt, ClientboundPacket::ConfirmTransaction);
        writeByte(pkt, static_cast<uint8_t>(windowId));
        writeShort(pkt, actionNumber);
        writeByte(pkt, accepted ? 1 : 0);
        conn.sendPacket(std::move(pkt));
    };

    // Helper: sync entire window to client after state change
    auto syncWindow = [&]() {
        if (!container_) return;
        // Update crafting output whenever anything changes
        container_->updateCraftingResult();
        for (int32_t i = 0; i < slotCount; ++i) {
            Slot* s = container_->getSlot(i);
            if (s) {
                sendSetSlot(conn, 0, static_cast<int16_t>(i), s->getStack());
            }
        }
        // Sync cursor item — slot -1
        sendSetSlot(conn, -1, -1, cursorItem_);
    };

    // ═══════════════════════════════════════════════════════════════════
    // Furnace window handler
    // ═══════════════════════════════════════════════════════════════════
    if (windowId > 0 && windowId == openWindowId_ && openWindowType_ == 2 && furnaceData_) {
        auto* furnace = static_cast<MinecraftServer::FurnaceData*>(furnaceData_);
        // Furnace slot layout (Java: ContainerFurnace):
        //   0       = input
        //   1       = fuel
        //   2       = output (can only pick up)
        //   3-29    = main inventory (player slots 9-35)
        //   30-38   = hotbar (player slots 0-8)

        auto getFurnaceSlotRef = [&](int16_t s) -> std::optional<ItemStack>* {
            if (s >= 0 && s < 3) return &furnace->slots[s];
            return nullptr;
        };
        auto getInvSlotForFurnace = [&](int16_t s) -> int32_t {
            if (s >= 3 && s <= 29) return s - 3 + 9;   // main inv: 9-35
            if (s >= 30 && s <= 38) return s - 30;      // hotbar: 0-8
            return -1;
        };
        auto syncFurnaceWindow = [&]() {
            for (int i = 0; i < 3; ++i)
                sendSetSlot(conn, openWindowId_, static_cast<int16_t>(i), furnace->slots[i]);
            for (int i = 9; i < 36; ++i)
                sendSetSlot(conn, openWindowId_, static_cast<int16_t>(i - 6), inventory_.getStackInSlot(i));
            for (int i = 0; i < 9; ++i)
                sendSetSlot(conn, openWindowId_, static_cast<int16_t>(30 + i), inventory_.getStackInSlot(i));
            sendSetSlot(conn, -1, -1, cursorItem_);
            // Send progress bars
            sendWindowProperty(conn, openWindowId_, 0, furnace->furnaceCookTime);
            sendWindowProperty(conn, openWindowId_, 1, furnace->furnaceBurnTime);
            sendWindowProperty(conn, openWindowId_, 2, furnace->currentItemBurnTime);
        };

        if (mode == 0 && (button == 0 || button == 1)) {
            if (slotId == -999) {
                if (cursorItem_) {
                    if (button == 0) {
                        server_.spawnItemDrop(playerX_, playerY_ + 1.5, playerZ_,
                            cursorItem_->getItemId(), cursorItem_->getDamage(), cursorItem_->getStackSize());
                        cursorItem_ = std::nullopt;
                    } else {
                        server_.spawnItemDrop(playerX_, playerY_ + 1.5, playerZ_,
                            cursorItem_->getItemId(), cursorItem_->getDamage(), 1);
                        int32_t rem = cursorItem_->getStackSize() - 1;
                        if (rem <= 0) cursorItem_ = std::nullopt;
                        else cursorItem_->setStackSize(rem);
                    }
                }
                sendConfirm(true);
                syncFurnaceWindow();
                return;
            }

            if (slotId < 0 || slotId > 38) { sendConfirm(false); return; }

            // Output slot (2): can only pick up, not place
            if (slotId == 2) {
                if (furnace->slots[2] && !cursorItem_) {
                    cursorItem_ = furnace->slots[2];
                    furnace->slots[2] = std::nullopt;
                } else if (furnace->slots[2] && cursorItem_ &&
                           cursorItem_->getItemId() == furnace->slots[2]->getItemId() &&
                           cursorItem_->getDamage() == furnace->slots[2]->getDamage()) {
                    int32_t newSize = cursorItem_->getStackSize() + furnace->slots[2]->getStackSize();
                    if (newSize <= 64) {
                        cursorItem_->setStackSize(newSize);
                        furnace->slots[2] = std::nullopt;
                    }
                }
                sendConfirm(true);
                syncFurnaceWindow();
                return;
            }

            std::optional<ItemStack>* furnRef = getFurnaceSlotRef(slotId);
            int32_t invIdx = (furnRef == nullptr) ? getInvSlotForFurnace(slotId) : -1;

            std::optional<ItemStack> slotStack;
            if (furnRef) slotStack = *furnRef;
            else if (invIdx >= 0) slotStack = inventory_.getStackInSlot(invIdx);
            else { sendConfirm(false); return; }

            if (button == 0) {
                if (furnRef) *furnRef = cursorItem_;
                else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, cursorItem_);
                cursorItem_ = slotStack;
            } else {
                if (cursorItem_ && !slotStack) {
                    ItemStack placed(cursorItem_->getItemId(), 1, cursorItem_->getDamage());
                    if (furnRef) *furnRef = placed;
                    else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, placed);
                    int32_t rem = cursorItem_->getStackSize() - 1;
                    if (rem <= 0) cursorItem_ = std::nullopt;
                    else cursorItem_->setStackSize(rem);
                } else if (cursorItem_ && slotStack &&
                           cursorItem_->getItemId() == slotStack->getItemId() &&
                           cursorItem_->getDamage() == slotStack->getDamage()) {
                    int32_t newSize = slotStack->getStackSize() + 1;
                    if (newSize <= 64) {
                        slotStack->setStackSize(newSize);
                        if (furnRef) *furnRef = slotStack;
                        else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, slotStack);
                        int32_t rem = cursorItem_->getStackSize() - 1;
                        if (rem <= 0) cursorItem_ = std::nullopt;
                        else cursorItem_->setStackSize(rem);
                    }
                } else if (!cursorItem_ && slotStack) {
                    int32_t half = (slotStack->getStackSize() + 1) / 2;
                    int32_t remaining = slotStack->getStackSize() - half;
                    cursorItem_ = ItemStack(slotStack->getItemId(), half, slotStack->getDamage());
                    if (remaining <= 0) {
                        if (furnRef) *furnRef = std::nullopt;
                        else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, std::nullopt);
                    } else {
                        slotStack->setStackSize(remaining);
                        if (furnRef) *furnRef = slotStack;
                        else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, slotStack);
                    }
                } else if (!cursorItem_ && !slotStack) {
                    // nothing
                } else {
                    if (furnRef) *furnRef = cursorItem_;
                    else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, cursorItem_);
                    cursorItem_ = slotStack;
                }
            }
            sendConfirm(true);
            syncFurnaceWindow();
            return;
        }

        // Other modes — confirm and sync
        sendConfirm(true);
        syncFurnaceWindow();
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // Chest (generic container) window handler
    // ═══════════════════════════════════════════════════════════════════
    if (windowId > 0 && windowId == openWindowId_ && openWindowType_ == 0 && chestInventory_) {
        // Chest slot layout (Java: ContainerChest, 3 rows):
        //   0-26    = chest slots
        //   27-53   = main inventory (player slots 9-35)
        //   54-62   = hotbar (player slots 0-8)

        auto getChestSlotRef = [&](int16_t s) -> std::optional<ItemStack>* {
            if (s >= 0 && s < 27) return &(*chestInventory_)[s];
            return nullptr;
        };
        auto getInvSlotForChest = [&](int16_t s) -> int32_t {
            if (s >= 27 && s <= 53) return s - 27 + 9;  // main inv: 9-35
            if (s >= 54 && s <= 62) return s - 54;       // hotbar: 0-8
            return -1;
        };
        auto syncChestWindow = [&]() {
            for (int i = 0; i < 27; ++i)
                sendSetSlot(conn, openWindowId_, static_cast<int16_t>(i), (*chestInventory_)[i]);
            for (int i = 9; i < 36; ++i)
                sendSetSlot(conn, openWindowId_, static_cast<int16_t>(i + 18), inventory_.getStackInSlot(i));
            for (int i = 0; i < 9; ++i)
                sendSetSlot(conn, openWindowId_, static_cast<int16_t>(54 + i), inventory_.getStackInSlot(i));
            sendSetSlot(conn, -1, -1, cursorItem_);
        };

        if (mode == 0 && (button == 0 || button == 1)) {
            if (slotId == -999) {
                if (cursorItem_) {
                    if (button == 0) {
                        server_.spawnItemDrop(playerX_, playerY_ + 1.5, playerZ_,
                            cursorItem_->getItemId(), cursorItem_->getDamage(), cursorItem_->getStackSize());
                        cursorItem_ = std::nullopt;
                    } else {
                        server_.spawnItemDrop(playerX_, playerY_ + 1.5, playerZ_,
                            cursorItem_->getItemId(), cursorItem_->getDamage(), 1);
                        int32_t rem = cursorItem_->getStackSize() - 1;
                        if (rem <= 0) cursorItem_ = std::nullopt;
                        else cursorItem_->setStackSize(rem);
                    }
                }
                sendConfirm(true);
                syncChestWindow();
                return;
            }

            if (slotId < 0 || slotId > 62) { sendConfirm(false); return; }

            std::optional<ItemStack>* chestRef = getChestSlotRef(slotId);
            int32_t invIdx = (chestRef == nullptr) ? getInvSlotForChest(slotId) : -1;

            std::optional<ItemStack> slotStack;
            if (chestRef) slotStack = *chestRef;
            else if (invIdx >= 0) slotStack = inventory_.getStackInSlot(invIdx);
            else { sendConfirm(false); return; }

            if (button == 0) {
                // Left click: swap
                if (chestRef) *chestRef = cursorItem_;
                else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, cursorItem_);
                cursorItem_ = slotStack;
            } else {
                // Right click: place 1 or pick up half
                if (cursorItem_ && !slotStack) {
                    ItemStack placed(cursorItem_->getItemId(), 1, cursorItem_->getDamage());
                    if (chestRef) *chestRef = placed;
                    else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, placed);
                    int32_t rem = cursorItem_->getStackSize() - 1;
                    if (rem <= 0) cursorItem_ = std::nullopt;
                    else cursorItem_->setStackSize(rem);
                } else if (cursorItem_ && slotStack &&
                           cursorItem_->getItemId() == slotStack->getItemId() &&
                           cursorItem_->getDamage() == slotStack->getDamage()) {
                    int32_t newSize = slotStack->getStackSize() + 1;
                    if (newSize <= 64) {
                        slotStack->setStackSize(newSize);
                        if (chestRef) *chestRef = slotStack;
                        else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, slotStack);
                        int32_t rem = cursorItem_->getStackSize() - 1;
                        if (rem <= 0) cursorItem_ = std::nullopt;
                        else cursorItem_->setStackSize(rem);
                    }
                } else if (!cursorItem_ && slotStack) {
                    int32_t half = (slotStack->getStackSize() + 1) / 2;
                    int32_t remaining = slotStack->getStackSize() - half;
                    cursorItem_ = ItemStack(slotStack->getItemId(), half, slotStack->getDamage());
                    if (remaining <= 0) {
                        if (chestRef) *chestRef = std::nullopt;
                        else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, std::nullopt);
                    } else {
                        slotStack->setStackSize(remaining);
                        if (chestRef) *chestRef = slotStack;
                        else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, slotStack);
                    }
                } else if (!cursorItem_ && !slotStack) {
                    // Nothing
                } else {
                    if (chestRef) *chestRef = cursorItem_;
                    else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, cursorItem_);
                    cursorItem_ = slotStack;
                }
            }
            sendConfirm(true);
            syncChestWindow();
            return;
        }

        // Other modes — confirm and sync
        sendConfirm(true);
        syncChestWindow();
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // Workbench (3×3 crafting table) window handler
    // ═══════════════════════════════════════════════════════════════════
    if (windowId > 0 && windowId == openWindowId_ && openWindowType_ == 1) {
        // Workbench slot layout (Java: ContainerWorkbench):
        //   0       = crafting result
        //   1-9     = crafting grid (3×3)
        //   10-36   = main inventory (player slots 9-35)
        //   37-45   = hotbar (player slots 0-8)

        // Helper: map workbench window slot to get/set reference
        auto getWbSlot = [&](int16_t s) -> std::optional<ItemStack>* {
            if (s == 0) return &workbenchResult_;
            if (s >= 1 && s <= 9) return &workbenchGrid_[s - 1];
            return nullptr;
        };

        // Helper: map workbench player-inventory slots to InventoryPlayer
        auto getInvSlotForWb = [&](int16_t s) -> int32_t {
            if (s >= 10 && s <= 36) return s - 10 + 9;  // main inv: 9-35
            if (s >= 37 && s <= 45) return s - 37;       // hotbar: 0-8
            return -1;
        };

        // Helper: update workbench crafting result
        auto updateWbCrafting = [&]() {
            CraftingGrid grid(3, 3);
            for (int i = 0; i < 9; ++i) {
                grid.setStack(i % 3, i / 3, workbenchGrid_[i]);
            }
            workbenchResult_ = CraftingManager::getInstance().findMatchingRecipe(grid);
        };

        // Helper: sync entire workbench window to client
        auto syncWbWindow = [&]() {
            updateWbCrafting();
            // Send result slot
            sendSetSlot(conn, openWindowId_, 0, workbenchResult_);
            // Send grid slots 1-9
            for (int i = 0; i < 9; ++i) {
                sendSetSlot(conn, openWindowId_, static_cast<int16_t>(i + 1), workbenchGrid_[i]);
            }
            // Send player inventory slots 10-36 (main inv)
            for (int i = 9; i < 36; ++i) {
                sendSetSlot(conn, openWindowId_, static_cast<int16_t>(i + 1), inventory_.getStackInSlot(i));
            }
            // Send hotbar 37-45
            for (int i = 0; i < 9; ++i) {
                sendSetSlot(conn, openWindowId_, static_cast<int16_t>(37 + i), inventory_.getStackInSlot(i));
            }
            // Sync cursor
            sendSetSlot(conn, -1, -1, cursorItem_);
        };

        if (mode == 0 && (button == 0 || button == 1)) {
            // Mode 0: Normal click
            if (slotId == -999) {
                // Click outside — drop cursor
                if (cursorItem_) {
                    if (button == 0) {
                        server_.spawnItemDrop(playerX_, playerY_ + 1.5, playerZ_,
                            cursorItem_->getItemId(), cursorItem_->getDamage(), cursorItem_->getStackSize());
                        cursorItem_ = std::nullopt;
                    } else {
                        server_.spawnItemDrop(playerX_, playerY_ + 1.5, playerZ_,
                            cursorItem_->getItemId(), cursorItem_->getDamage(), 1);
                        int32_t rem = cursorItem_->getStackSize() - 1;
                        if (rem <= 0) cursorItem_ = std::nullopt;
                        else cursorItem_->setStackSize(rem);
                    }
                }
                sendConfirm(true);
                syncWbWindow();
                return;
            }

            if (slotId < 0 || slotId > 45) { sendConfirm(false); return; }

            if (slotId == 0) {
                // Click result slot — extract crafted item
                if (workbenchResult_ && !cursorItem_) {
                    cursorItem_ = workbenchResult_;
                    workbenchResult_ = std::nullopt;
                    // Consume 1 from each grid slot that has items
                    for (int i = 0; i < 9; ++i) {
                        if (workbenchGrid_[i]) {
                            int32_t sz = workbenchGrid_[i]->getStackSize() - 1;
                            if (sz <= 0) workbenchGrid_[i] = std::nullopt;
                            else workbenchGrid_[i]->setStackSize(sz);
                        }
                    }
                } else if (workbenchResult_ && cursorItem_) {
                    // Cursor has item — can only stack if same type
                    if (cursorItem_->getItemId() == workbenchResult_->getItemId() &&
                        cursorItem_->getDamage() == workbenchResult_->getDamage()) {
                        int32_t newSize = cursorItem_->getStackSize() + workbenchResult_->getStackSize();
                        if (newSize <= 64) {
                            cursorItem_->setStackSize(newSize);
                            workbenchResult_ = std::nullopt;
                            for (int i = 0; i < 9; ++i) {
                                if (workbenchGrid_[i]) {
                                    int32_t sz = workbenchGrid_[i]->getStackSize() - 1;
                                    if (sz <= 0) workbenchGrid_[i] = std::nullopt;
                                    else workbenchGrid_[i]->setStackSize(sz);
                                }
                            }
                        }
                    }
                }
                sendConfirm(true);
                syncWbWindow();
                return;
            }

            // Grid slots (1-9) or player inventory slots (10-45)
            std::optional<ItemStack>* slotRef = getWbSlot(slotId);
            int32_t invIdx = (slotRef == nullptr) ? getInvSlotForWb(slotId) : -1;

            std::optional<ItemStack> slotStack;
            if (slotRef) slotStack = *slotRef;
            else if (invIdx >= 0) slotStack = inventory_.getStackInSlot(invIdx);
            else { sendConfirm(false); return; }

            if (button == 0) {
                // Left click: swap cursor ↔ slot
                if (slotRef) *slotRef = cursorItem_;
                else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, cursorItem_);
                cursorItem_ = slotStack;
            } else {
                // Right click: place 1 from cursor, or pick up half
                if (cursorItem_ && !slotStack) {
                    // Place 1 item
                    ItemStack placed(cursorItem_->getItemId(), 1, cursorItem_->getDamage());
                    if (slotRef) *slotRef = placed;
                    else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, placed);
                    int32_t rem = cursorItem_->getStackSize() - 1;
                    if (rem <= 0) cursorItem_ = std::nullopt;
                    else cursorItem_->setStackSize(rem);
                } else if (cursorItem_ && slotStack &&
                           cursorItem_->getItemId() == slotStack->getItemId() &&
                           cursorItem_->getDamage() == slotStack->getDamage()) {
                    // Place 1 onto matching stack
                    int32_t newSize = slotStack->getStackSize() + 1;
                    if (newSize <= 64) {
                        slotStack->setStackSize(newSize);
                        if (slotRef) *slotRef = slotStack;
                        else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, slotStack);
                        int32_t rem = cursorItem_->getStackSize() - 1;
                        if (rem <= 0) cursorItem_ = std::nullopt;
                        else cursorItem_->setStackSize(rem);
                    }
                } else if (!cursorItem_ && slotStack) {
                    // Pick up half
                    int32_t half = (slotStack->getStackSize() + 1) / 2;
                    int32_t remaining = slotStack->getStackSize() - half;
                    cursorItem_ = ItemStack(slotStack->getItemId(), half, slotStack->getDamage());
                    if (remaining <= 0) {
                        if (slotRef) *slotRef = std::nullopt;
                        else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, std::nullopt);
                    } else {
                        slotStack->setStackSize(remaining);
                        if (slotRef) *slotRef = slotStack;
                        else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, slotStack);
                    }
                } else if (!cursorItem_ && !slotStack) {
                    // Nothing to do
                } else {
                    // Swap (different items)
                    if (slotRef) *slotRef = cursorItem_;
                    else if (invIdx >= 0) inventory_.setInventorySlotContents(invIdx, cursorItem_);
                    cursorItem_ = slotStack;
                }
            }

            sendConfirm(true);
            syncWbWindow();
            return;
        }

        // Shift-click in workbench
        if (mode == 1 && (button == 0 || button == 1)) {
            if (slotId < 0 || slotId > 45) { sendConfirm(false); return; }

            // Simplified shift-click: move items between grid↔inventory
            // For now just confirm and resync
            sendConfirm(true);
            syncWbWindow();
            return;
        }

        // Other modes — confirm and sync
        sendConfirm(true);
        syncWbWindow();
        return;
    }

    if (!container_ || windowId != 0) {
        sendConfirm(false);
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // Mode 0: Normal click (button 0=left, 1=right)
    // Java: Container.slotClick mode 0
    // ═══════════════════════════════════════════════════════════════════
    if (mode == 0 && (button == 0 || button == 1)) {
        if (slotId == -999) {
            // Click outside window — drop cursor item
            if (cursorItem_) {
                if (button == 0) {
                    // Left: drop entire cursor stack
                    server_.spawnItemDrop(playerX_, playerY_ + 1.5, playerZ_,
                        cursorItem_->getItemId(), cursorItem_->getDamage(), cursorItem_->getStackSize());
                    cursorItem_ = std::nullopt;
                } else {
                    // Right: drop 1 from cursor
                    server_.spawnItemDrop(playerX_, playerY_ + 1.5, playerZ_,
                        cursorItem_->getItemId(), cursorItem_->getDamage(), 1);
                    int32_t remaining = cursorItem_->getStackSize() - 1;
                    if (remaining <= 0) {
                        cursorItem_ = std::nullopt;
                    } else {
                        cursorItem_->setStackSize(remaining);
                    }
                }
            }
            sendConfirm(true);
            syncWindow();
            return;
        }

        if (slotId < 0 || slotId >= slotCount) {
            sendConfirm(false);
            return;
        }

        Slot* slot = container_->getSlot(slotId);
        if (!slot) { sendConfirm(false); return; }

        auto slotStack = slot->getStack();

        // ─── Special handling for crafting output (slot 0) ──────────
        // Java: SlotCrafting.onPickupFromSlot() — decrement ingredients
        if (slotId == 0 && slotStack && !cursorItem_) {
            // Pick up the crafting result
            cursorItem_ = slotStack;
            slot->putStack(std::nullopt);
            // Decrement each ingredient in the 2×2 crafting grid (slots 1-4)
            for (int32_t i = 1; i <= 4; ++i) {
                Slot* gridSlot = container_->getSlot(i);
                if (!gridSlot) continue;
                auto gridStack = gridSlot->getStack();
                if (!gridStack) continue;
                int32_t newSize = gridStack->getStackSize() - 1;
                if (newSize <= 0) {
                    gridSlot->putStack(std::nullopt);
                } else {
                    gridStack->setStackSize(newSize);
                    gridSlot->putStack(gridStack);
                }
            }
            sendConfirm(true);
            syncWindow();
            return;
        }

        if (!slotStack) {
            // Empty slot — place cursor item into it
            if (cursorItem_) {
                if (button == 0) {
                    // Left: place all
                    slot->putStack(cursorItem_);
                    cursorItem_ = std::nullopt;
                } else {
                    // Right: place 1
                    ItemStack single(cursorItem_->getItemId(), 1, cursorItem_->getDamage());
                    slot->putStack(single);
                    int32_t remaining = cursorItem_->getStackSize() - 1;
                    if (remaining <= 0) {
                        cursorItem_ = std::nullopt;
                    } else {
                        cursorItem_->setStackSize(remaining);
                    }
                }
            }
        } else {
            // Slot has item
            if (!cursorItem_) {
                // Pick up from slot
                if (button == 0) {
                    // Left: pick up all
                    cursorItem_ = slotStack;
                    slot->putStack(std::nullopt);
                } else {
                    // Right: pick up half (round up)
                    int32_t halfCount = (slotStack->getStackSize() + 1) / 2;
                    ItemStack picked = *slotStack;
                    picked.setStackSize(halfCount);
                    cursorItem_ = picked;
                    int32_t remaining = slotStack->getStackSize() - halfCount;
                    if (remaining <= 0) {
                        slot->putStack(std::nullopt);
                    } else {
                        ItemStack left = *slotStack;
                        left.setStackSize(remaining);
                        slot->putStack(left);
                    }
                }
            } else {
                // Both slot and cursor have items
                if (cursorItem_->getItemId() == slotStack->getItemId() &&
                    cursorItem_->getDamage() == slotStack->getDamage()) {
                    // Same item type — merge
                    if (button == 0) {
                        // Left: add as many cursor items as possible
                        int32_t maxStack = cursorItem_->getMaxStackSize();
                        int32_t canAdd = maxStack - slotStack->getStackSize();
                        if (canAdd > 0) {
                            int32_t toAdd = std::min(canAdd, cursorItem_->getStackSize());
                            ItemStack updated = *slotStack;
                            updated.setStackSize(slotStack->getStackSize() + toAdd);
                            slot->putStack(updated);
                            int32_t cursorLeft = cursorItem_->getStackSize() - toAdd;
                            if (cursorLeft <= 0) {
                                cursorItem_ = std::nullopt;
                            } else {
                                cursorItem_->setStackSize(cursorLeft);
                            }
                        }
                    } else {
                        // Right: add 1 from cursor
                        if (slotStack->getStackSize() < slotStack->getMaxStackSize()) {
                            ItemStack updated = *slotStack;
                            updated.setStackSize(slotStack->getStackSize() + 1);
                            slot->putStack(updated);
                            int32_t cursorLeft = cursorItem_->getStackSize() - 1;
                            if (cursorLeft <= 0) {
                                cursorItem_ = std::nullopt;
                            } else {
                                cursorItem_->setStackSize(cursorLeft);
                            }
                        }
                    }
                } else {
                    // Different items — swap
                    auto temp = cursorItem_;
                    cursorItem_ = slotStack;
                    slot->putStack(temp);
                }
            }
        }
        sendConfirm(true);
        syncWindow();
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // Mode 1: Shift-click (quick move)
    // Java: Container.transferStackInSlot → mergeItemStack
    // ContainerPlayer layout: 0=output, 1-4=craft grid, 5-8=armor,
    //                        9-35=main inventory, 36-44=hotbar
    // ═══════════════════════════════════════════════════════════════════
    if (mode == 1 && (button == 0 || button == 1)) {
        if (slotId < 0 || slotId >= slotCount) {
            sendConfirm(false);
            return;
        }
        Slot* slot = container_->getSlot(slotId);
        if (!slot) { sendConfirm(false); return; }
        auto slotStack = slot->getStack();
        if (!slotStack) { sendConfirm(true); syncWindow(); return; }

        // Simple shift-click: hotbar (36-44) ↔ main inventory (9-35)
        int32_t destStart, destEnd;
        if (slotId >= 36 && slotId <= 44) {
            // Hotbar → main inventory
            destStart = 9; destEnd = 36;
        } else if (slotId >= 9 && slotId <= 35) {
            // Main → hotbar
            destStart = 36; destEnd = 45;
        } else if (slotId >= 5 && slotId <= 8) {
            // Armor → main+hotbar
            destStart = 9; destEnd = 45;
        } else if (slotId >= 1 && slotId <= 4) {
            // Crafting grid → main+hotbar
            destStart = 9; destEnd = 45;
        } else if (slotId == 0) {
            // Crafting output → main+hotbar
            destStart = 9; destEnd = 45;
        } else {
            sendConfirm(true);
            syncWindow();
            return;
        }

        // Try to merge into destination range
        ItemStack toMove = *slotStack;
        bool moved = false;

        // First pass: merge with existing stacks of same type
        for (int32_t i = destStart; i < destEnd && toMove.getStackSize() > 0; ++i) {
            Slot* dest = container_->getSlot(i);
            if (!dest) continue;
            auto destStack = dest->getStack();
            if (destStack && destStack->getItemId() == toMove.getItemId() &&
                destStack->getDamage() == toMove.getDamage()) {
                int32_t canAdd = destStack->getMaxStackSize() - destStack->getStackSize();
                if (canAdd > 0) {
                    int32_t add = std::min(canAdd, toMove.getStackSize());
                    ItemStack updated = *destStack;
                    updated.setStackSize(destStack->getStackSize() + add);
                    dest->putStack(updated);
                    toMove.setStackSize(toMove.getStackSize() - add);
                    moved = true;
                }
            }
        }

        // Second pass: place in empty slots
        for (int32_t i = destStart; i < destEnd && toMove.getStackSize() > 0; ++i) {
            Slot* dest = container_->getSlot(i);
            if (!dest) continue;
            auto destStack = dest->getStack();
            if (!destStack) {
                dest->putStack(toMove);
                toMove.setStackSize(0);
                moved = true;
            }
        }

        // Update source slot
        if (toMove.getStackSize() <= 0) {
            slot->putStack(std::nullopt);
        } else {
            slot->putStack(toMove);
        }

        // If we moved items from the crafting output (slot 0), decrement ingredients
        if (slotId == 0 && moved) {
            for (int32_t i = 1; i <= 4; ++i) {
                Slot* gridSlot = container_->getSlot(i);
                if (!gridSlot) continue;
                auto gridStack = gridSlot->getStack();
                if (!gridStack) continue;
                int32_t newSize = gridStack->getStackSize() - 1;
                if (newSize <= 0) {
                    gridSlot->putStack(std::nullopt);
                } else {
                    gridStack->setStackSize(newSize);
                    gridSlot->putStack(gridStack);
                }
            }
        }

        sendConfirm(true);
        syncWindow();
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // Mode 2: Number key swap (button = hotbar slot 0-8)
    // Java: Container.slotClick mode 2
    // ═══════════════════════════════════════════════════════════════════
    if (mode == 2 && button >= 0 && button < 9) {
        if (slotId < 0 || slotId >= slotCount) {
            sendConfirm(false);
            return;
        }
        Slot* clickedSlot = container_->getSlot(slotId);
        if (!clickedSlot) { sendConfirm(false); return; }

        // Hotbar slot in ContainerPlayer = 36 + button
        int32_t hotbarContainerSlot = 36 + button;
        Slot* hotbarSlot = container_->getSlot(hotbarContainerSlot);
        if (!hotbarSlot) { sendConfirm(false); return; }

        auto clickedStack = clickedSlot->getStack();
        auto hotbarStack = hotbarSlot->getStack();

        // Swap the two slots
        clickedSlot->putStack(hotbarStack);
        hotbarSlot->putStack(clickedStack);

        sendConfirm(true);
        syncWindow();
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // Mode 3: Creative middle click (clone item)
    // Java: Container.slotClick mode 3
    // ═══════════════════════════════════════════════════════════════════
    if (mode == 3 && gameMode_ == 1) {
        if (slotId >= 0 && slotId < slotCount && !cursorItem_) {
            Slot* slot = container_->getSlot(slotId);
            if (slot) {
                auto slotStack = slot->getStack();
                if (slotStack) {
                    ItemStack cloned = *slotStack;
                    cloned.setStackSize(cloned.getMaxStackSize());
                    cursorItem_ = cloned;
                }
            }
        }
        sendConfirm(true);
        syncWindow();
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // Mode 4: Drop item from slot (not cursor)
    // Java: Container.slotClick mode 4
    // button 0 = drop 1, button 1 = drop stack
    // ═══════════════════════════════════════════════════════════════════
    if (mode == 4 && !cursorItem_) {
        if (slotId >= 0 && slotId < slotCount) {
            Slot* slot = container_->getSlot(slotId);
            if (slot) {
                auto slotStack = slot->getStack();
                if (slotStack) {
                    int32_t dropCount = (button == 0) ? 1 : slotStack->getStackSize();
                    dropCount = std::min(dropCount, slotStack->getStackSize());

                    // Drop the items
                    server_.spawnItemDrop(playerX_, playerY_ + 1.5, playerZ_,
                        slotStack->getItemId(), slotStack->getDamage(), dropCount);

                    int32_t remaining = slotStack->getStackSize() - dropCount;
                    if (remaining <= 0) {
                        slot->putStack(std::nullopt);
                    } else {
                        ItemStack updated = *slotStack;
                        updated.setStackSize(remaining);
                        slot->putStack(updated);
                    }
                }
            }
        }
        sendConfirm(true);
        syncWindow();
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // Mode 5: Drag (paint mode) — complex, accept and resync
    // Mode 6: Double-click (collect all) — complex, accept and resync
    // For now: accept the action and resync the full window
    // ═══════════════════════════════════════════════════════════════════
    sendConfirm(true);
    syncWindow();
}

// ═══════════════════════════════════════════════════════════════════════════
// Player data persistence
// Java reference: SaveHandler.writePlayerData / readPlayerData
// ═══════════════════════════════════════════════════════════════════════════

void PlayHandler::savePlayerData() {
    namespace fs = std::filesystem;

    // Create playerdata directory
    fs::path dir = "world/playerdata";
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
    }

    // Build NBT compound
    // Java format: Pos(double list), Rotation(float list), OnGround(byte), Dimension(int)
    nbt::NBTTagCompound root;

    // Position — TAG_List of 3 doubles
    auto posList = std::make_unique<nbt::NBTTagList>();
    posList->appendTag(std::make_unique<nbt::NBTTagDouble>(playerX_));
    posList->appendTag(std::make_unique<nbt::NBTTagDouble>(playerY_));
    posList->appendTag(std::make_unique<nbt::NBTTagDouble>(playerZ_));
    root.setTag("Pos", std::move(posList));

    // Rotation — TAG_List of 2 floats
    auto rotList = std::make_unique<nbt::NBTTagList>();
    rotList->appendTag(std::make_unique<nbt::NBTTagFloat>(playerYaw_));
    rotList->appendTag(std::make_unique<nbt::NBTTagFloat>(playerPitch_));
    root.setTag("Rotation", std::move(rotList));

    // OnGround
    root.setByte("OnGround", playerOnGround_ ? 1 : 0);

    // Dimension
    root.setInteger("Dimension", 0);

    // Health
    root.setFloat("Health", health_);

    // Food stats — Java: FoodStats.writeNBT()
    root.setInteger("foodLevel", foodStats_.getFoodLevel());
    root.setFloat("foodSaturationLevel", foodStats_.getSaturationLevel());
    root.setFloat("foodExhaustionLevel", foodStats_.getExhaustionLevel());

    // XP — Java: EntityPlayer.writeEntityToNBT()
    root.setFloat("XpP", experienceBar_);
    root.setInteger("XpLevel", experienceLevel_);
    root.setInteger("XpTotal", experienceTotal_);

    // Serialize to binary
    auto data = nbt::serializeNBT(root);

    // Write to file
    fs::path filePath = dir / (uuid_ + ".dat");
    std::ofstream file(filePath, std::ios::binary);
    if (file) {
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        std::cout << "[Save] Saved player data for " << playerName_
                  << " (" << data.size() << " bytes)\n";
    }
}

bool PlayHandler::loadPlayerData() {
    namespace fs = std::filesystem;

    fs::path filePath = "world/playerdata" / fs::path(uuid_ + ".dat");
    if (!fs::exists(filePath)) return false;

    // Read file
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) return false;

    auto size = file.tellg();
    if (size <= 0) return false;
    file.seekg(0);

    std::vector<uint8_t> data(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(data.data()), size);

    // Parse NBT
    auto root = nbt::deserializeNBT(data.data(), data.size());
    if (!root) return false;

    // Read position
    if (root->hasKey("Pos")) {
        auto* posList = root->getTagList("Pos", 6);  // 6 = TAG_Double
        if (posList && posList->tagCount() >= 3) {
            playerX_ = posList->getDoubleAt(0);
            playerY_ = posList->getDoubleAt(1);
            playerZ_ = posList->getDoubleAt(2);
        }
    }

    // Read rotation
    if (root->hasKey("Rotation")) {
        auto* rotList = root->getTagList("Rotation", 5);  // 5 = TAG_Float
        if (rotList && rotList->tagCount() >= 2) {
            playerYaw_ = rotList->getFloatAt(0);
            playerPitch_ = rotList->getFloatAt(1);
        }
    }

    // Read onGround
    if (root->hasKey("OnGround")) {
        playerOnGround_ = root->getByte("OnGround") != 0;
    }

    // Read health
    if (root->hasKey("Health")) {
        health_ = root->getFloat("Health");
        if (health_ <= 0.0f) health_ = 20.0f; // Don't load dead
    }

    // Read food stats — Java: FoodStats.readNBT()
    if (root->hasKey("foodLevel")) {
        foodStats_.foodLevel = root->getInteger("foodLevel");
    }
    if (root->hasKey("foodSaturationLevel")) {
        foodStats_.foodSaturationLevel = root->getFloat("foodSaturationLevel");
    }
    if (root->hasKey("foodExhaustionLevel")) {
        foodStats_.foodExhaustionLevel = root->getFloat("foodExhaustionLevel");
    }

    // XP — Java: EntityPlayer.readEntityFromNBT()
    if (root->hasKey("XpP")) experienceBar_ = root->getFloat("XpP");
    if (root->hasKey("XpLevel")) experienceLevel_ = root->getInteger("XpLevel");
    if (root->hasKey("XpTotal")) experienceTotal_ = root->getInteger("XpTotal");

    std::cout << "[Load] Loaded player data for " << playerName_
              << " at (" << playerX_ << ", " << playerY_ << ", " << playerZ_ << ")\n";
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// Combat packet handlers
// ═══════════════════════════════════════════════════════════════════════════

void PlayHandler::handleUseEntity(const uint8_t* data, size_t length, Connection& conn) {
    // Java reference: C02PacketUseEntity
    // Format: Int entityId, VarInt action (0=interact, 1=attack, 2=interact_at)
    if (length < 5) return;

    int32_t targetEntityId = readInt(data);
    auto actionResult = readVarInt(data + 4, length - 4);
    int32_t action = actionResult.value;

    if (action == 1) {
        // ATTACK — Java: EntityPlayer.attackTargetEntityWithCurrentItem()
        server_.handlePlayerAttack(*this, conn, targetEntityId);
    }
    // action 0 (interact) and 2 (interact_at) not implemented yet
}

void PlayHandler::handleClientStatus(const uint8_t* data, size_t length, Connection& conn) {
    // Java reference: C16PacketClientStatus
    // Format: VarInt actionId (0=PERFORM_RESPAWN, 1=REQUEST_STATS, 2=OPEN_INVENTORY_ACHIEVEMENT)
    if (length < 1) return;

    auto actionResult = readVarInt(data, length);
    int32_t actionId = actionResult.value;

    if (actionId == 0 && dead_) {
        // PERFORM_RESPAWN — Java: EntityPlayerMP.onDeath → respawn
        // Reset health and state
        health_ = 20.0f;
        foodStats_ = FoodStats();  // Reset to defaults (20 food, 5.0 sat)
        dead_ = false;

        // Get spawn coordinates
        auto* overworld = server_.getWorlds().empty() ? nullptr : server_.getWorlds()[0].get();
        double spawnX = 0.0, spawnY = 80.0, spawnZ = 0.0;
        if (overworld) {
            spawnY = static_cast<double>(overworld->getSpawnY());
        }

        playerX_ = spawnX;
        playerY_ = spawnY;
        playerZ_ = spawnZ;
        playerYaw_ = 0.0f;
        playerPitch_ = 0.0f;

        // Send S07 Respawn packet
        // Java reference: S07PacketRespawn
        {
            std::vector<uint8_t> pkt;
            writeVarInt(pkt, ClientboundPacket::Respawn);
            writeInt(pkt, 0);          // Dimension: 0 = Overworld
            writeUByte(pkt, 1);        // Difficulty: 1 = Easy
            writeUByte(pkt, 0);        // Gamemode: 0 = Survival
            writeString(pkt, "flat");  // Level type
            conn.sendPacket(std::move(pkt));
        }

        // Send chunks around spawn
        int playerChunkX = static_cast<int>(playerX_) >> 4;
        int playerChunkZ = static_cast<int>(playerZ_) >> 4;
        if (overworld) {
            for (int cx = playerChunkX - 2; cx <= playerChunkX + 2; ++cx) {
                for (int cz = playerChunkZ - 2; cz <= playerChunkZ + 2; ++cz) {
                    Chunk* chunk = overworld->getChunkFromChunkCoords(cx, cz);
                    if (chunk) sendChunkData(conn, chunk);
                }
            }
        }

        // Send position
        {
            std::vector<uint8_t> pkt;
            writeVarInt(pkt, ClientboundPacket::PlayerPosAndLook);
            writeDouble(pkt, playerX_);
            writeDouble(pkt, playerY_);
            writeDouble(pkt, playerZ_);
            writeFloat(pkt, playerYaw_);
            writeFloat(pkt, playerPitch_);
            writeBool(pkt, false);
            conn.sendPacket(std::move(pkt));
        }

        // Send full health
        sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());

        // Re-broadcast spawn to other players
        server_.onPlayerJoined(conn, *this);

        std::cout << "[Combat] " << playerName_ << " respawned at spawn\n";
    }
}

void PlayHandler::sendEntityStatus(Connection& conn, int32_t entityId, int8_t status) {
    // Java reference: S1APacketEntityStatus
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::EntityStatus);
    writeInt(pkt, entityId);
    writeByte(pkt, status);
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendEntityVelocity(Connection& conn, int32_t entityId, double vx, double vy, double vz) {
    // Java reference: S12PacketEntityVelocity
    // velocity = clamped to [-3.9, 3.9], sent as short = (int)(v * 8000)
    auto clampVel = [](double v) -> int16_t {
        double c = std::max(-3.9, std::min(3.9, v));
        return static_cast<int16_t>(c * 8000.0);
    };
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::EntityVelocity);
    writeInt(pkt, entityId);
    writeShort(pkt, clampVel(vx));
    writeShort(pkt, clampVel(vy));
    writeShort(pkt, clampVel(vz));
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendSpawnMob(Connection& conn, int32_t entityId, uint8_t mobType,
                                double x, double y, double z,
                                float yaw, float pitch, float headYaw) {
    // Java reference: S0FPacketSpawnMob
    // Fixed-point: pos * 32
    // Angle: angle * 256 / 360
    auto encodePos = [](double pos) -> int32_t {
        return static_cast<int32_t>(pos * 32.0);
    };
    auto encodeAngle = [](float angle) -> int8_t {
        return static_cast<int8_t>(static_cast<int32_t>(angle * 256.0f / 360.0f) & 0xFF);
    };

    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::SpawnMob);
    writeVarInt(pkt, entityId);
    writeUByte(pkt, mobType);
    writeInt(pkt, encodePos(x));
    writeInt(pkt, encodePos(y));
    writeInt(pkt, encodePos(z));
    writeByte(pkt, encodeAngle(yaw));
    writeByte(pkt, encodeAngle(pitch));
    writeByte(pkt, encodeAngle(headYaw));
    writeShort(pkt, 0);  // velX
    writeShort(pkt, 0);  // velY
    writeShort(pkt, 0);  // velZ

    // DataWatcher metadata — minimal entity metadata
    // Index 0: byte — entity flags (0 = normal)
    pkt.push_back((0 << 5) | 0);  // type=byte(0), index=0
    pkt.push_back(0);              // value: no flags

    // Index 6: float — health (DataWatcher index 6 for LivingBase)
    pkt.push_back((3 << 5) | 6);  // type=float(3), index=6
    float health = 20.0f;
    uint32_t healthBits;
    std::memcpy(&healthBits, &health, sizeof(healthBits));
    pkt.push_back((healthBits >> 24) & 0xFF);
    pkt.push_back((healthBits >> 16) & 0xFF);
    pkt.push_back((healthBits >> 8) & 0xFF);
    pkt.push_back(healthBits & 0xFF);

    // Terminator
    pkt.push_back(0x7F);

    conn.sendPacket(std::move(pkt));
}

void PlayHandler::applyDamage(float amount) {
    // Java reference: EntityLivingBase.damageEntity()
    health_ = std::max(0.0f, health_ - amount);
    // Java: EntityPlayer takes 0.3 exhaustion on any damage
    foodStats_.addExhaustion(Exhaustion::DAMAGE);
    if (health_ <= 0.0f) {
        dead_ = true;
    }
}

void PlayHandler::sendPlayerPosAndLook(Connection& conn, double x, double y, double z,
                                        float yaw, float pitch) {
    // Java reference: S08PacketPlayerPosLook
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::PlayerPosAndLook);
    writeDouble(pkt, x);
    writeDouble(pkt, y);
    writeDouble(pkt, z);
    writeFloat(pkt, yaw);
    writeFloat(pkt, pitch);
    pkt.push_back(0); // flags: 0 = absolute
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendChangeGameState(Connection& conn, uint8_t reason, float value) {
    // Java reference: S2BPacketChangeGameState
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::ChangeGameState);
    writeUByte(pkt, reason);
    writeFloat(pkt, value);
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendSetExperience(Connection& conn, float bar, int32_t level, int32_t totalXp) {
    // Java reference: S1FPacketSetExperience.writePacketData()
    // Format: Float bar, Short level, Short totalXP
    // NOTE: Java uses writeShort, NOT writeVarInt!
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::SetExperience);
    writeFloat(pkt, bar);
    writeShort(pkt, static_cast<int16_t>(level));
    writeShort(pkt, static_cast<int16_t>(totalXp));
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::sendChunkUnload(Connection& conn, int32_t chunkX, int32_t chunkZ) {
    // Java reference: S21PacketChunkData with groundUpContinuous=true, bitmask=0
    // This tells the client to unload the chunk at (chunkX, chunkZ).
    // Format: Int chunkX, Int chunkZ, Bool groundUp, UShort primaryBitmask,
    //         UShort addBitmask, Int compressedSize, Byte[] compressedData
    // For unload: groundUp=true, bitmask=0, data is empty zlib output.
    
    // Minimal zlib-compressed empty payload
    // zlib compress of zero bytes → small header
    std::vector<uint8_t> emptyData;
    uLong compBound = compressBound(0);
    std::vector<uint8_t> compressed(compBound);
    uLong compSize = compBound;
    compress2(compressed.data(), &compSize, emptyData.data(), 0, Z_DEFAULT_COMPRESSION);

    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::ChunkData);
    writeInt(pkt, chunkX);
    writeInt(pkt, chunkZ);
    writeBool(pkt, true);         // ground-up continuous
    writeShort(pkt, 0);           // primary bitmask = 0 (no sections → unload)
    writeShort(pkt, 0);           // add bitmask = 0
    writeInt(pkt, static_cast<int32_t>(compSize));
    pkt.insert(pkt.end(), compressed.data(), compressed.data() + compSize);
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::updateChunks(Connection& conn) {
    // Java reference: PlayerManager.updatePlayerPertinentChunks()
    // Called when player position changes — sends new chunks, unloads old ones.
    
    int currentChunkX = static_cast<int>(std::floor(playerX_)) >> 4;
    int currentChunkZ = static_cast<int>(std::floor(playerZ_)) >> 4;
    
    // Only update if player crossed a chunk boundary
    if (currentChunkX == lastChunkX_ && currentChunkZ == lastChunkZ_) {
        return;
    }
    
    lastChunkX_ = currentChunkX;
    lastChunkZ_ = currentChunkZ;
    
    auto* overworld = server_.getWorlds().empty() ? nullptr : server_.getWorlds()[0].get();
    if (!overworld) return;
    
    // Compute the set of chunks that SHOULD be loaded
    std::set<std::pair<int,int>> desired;
    for (int cx = currentChunkX - VIEW_DISTANCE; cx <= currentChunkX + VIEW_DISTANCE; ++cx) {
        for (int cz = currentChunkZ - VIEW_DISTANCE; cz <= currentChunkZ + VIEW_DISTANCE; ++cz) {
            desired.insert({cx, cz});
        }
    }
    
    // Unload chunks that are no longer in view
    std::vector<std::pair<int,int>> toUnload;
    for (const auto& coord : loadedChunks_) {
        if (desired.find(coord) == desired.end()) {
            toUnload.push_back(coord);
        }
    }
    for (const auto& [cx, cz] : toUnload) {
        sendChunkUnload(conn, cx, cz);
        loadedChunks_.erase({cx, cz});
    }
    
    // Send chunks that are newly in view (sorted by distance for better UX)
    std::vector<std::pair<int,int>> toLoad;
    for (const auto& coord : desired) {
        if (loadedChunks_.find(coord) == loadedChunks_.end()) {
            toLoad.push_back(coord);
        }
    }
    
    // Sort by distance from player (closest first) for better loading feel
    std::sort(toLoad.begin(), toLoad.end(), [&](const auto& a, const auto& b) {
        int da = (a.first - currentChunkX) * (a.first - currentChunkX)
               + (a.second - currentChunkZ) * (a.second - currentChunkZ);
        int db = (b.first - currentChunkX) * (b.first - currentChunkX)
               + (b.second - currentChunkZ) * (b.second - currentChunkZ);
        return da < db;
    });
    
    for (const auto& [cx, cz] : toLoad) {
        Chunk* chunk = overworld->getChunkFromChunkCoords(cx, cz);
        if (chunk) {
            sendChunkData(conn, chunk);
            loadedChunks_.insert({cx, cz});
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// handleHeldItemChange — Track which hotbar slot the player has selected.
// Java: NetHandlerPlayServer.processHeldItemChange()
// ═══════════════════════════════════════════════════════════════════════════
void PlayHandler::handleHeldItemChange(const uint8_t* data, size_t length, Connection& conn) {
    PacketReader reader(data, length);
    auto pkt = SB_HeldItemChange::read(reader);

    // Java: if (slotId < 0 || slotId >= InventoryPlayer.getHotbarSize()) return;
    if (pkt.slotId < 0 || pkt.slotId >= 9) {
        std::cerr << "[Play] " << playerName_ << " tried to set invalid held item: "
                  << pkt.slotId << "\n";
        return;
    }

    currentSlot_ = pkt.slotId;
    // Java: this.playerEntity.inventory.currentItem = slotId;
    // Java: this.playerEntity.markPlayerActive();

    // Broadcast held item to other players — Java: EntityTrackerEntry equipment sync
    server_.broadcastEquipment(*this, 0);
}

// ═══════════════════════════════════════════════════════════════════════════
// handleAnimation — Broadcast arm swing to other players.
// Java: NetHandlerPlayServer.processAnimation()
// ═══════════════════════════════════════════════════════════════════════════
void PlayHandler::handleAnimation(const uint8_t* data, size_t length, Connection& conn) {
    PacketReader reader(data, length);
    auto pkt = SB_Animation::read(reader);

    // Java: if (animation == 1) { this.playerEntity.swingItem(); }
    // swingItem() broadcasts S0B animation packet to nearby players
    if (pkt.animation == 1) {
        // Broadcast S0B Animation (swing arm = type 0 in clientbound) to all other players
        server_.broadcastAnimation(entityId_, 0); // 0 = swing arm
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// handleEntityAction — Handle sneak, sprint, bed leave.
// Java: NetHandlerPlayServer.processEntityAction()
// ═══════════════════════════════════════════════════════════════════════════
void PlayHandler::handleEntityAction(const uint8_t* data, size_t length, Connection& conn) {
    PacketReader reader(data, length);
    auto pkt = SB_EntityAction::read(reader);

    // Java: func_149513_d() returns actionId
    // 1=start sneaking, 2=stop sneaking, 3=leave bed
    // 4=start sprinting, 5=stop sprinting
    // 6=horse jump, 7=open horse inventory
    switch (pkt.actionId) {
        case 1: // Start sneaking
            isSneaking_ = true;
            break;
        case 2: // Stop sneaking
            isSneaking_ = false;
            break;
        case 4: // Start sprinting
            isSprinting_ = true;
            break;
        case 5: // Stop sprinting
            isSprinting_ = false;
            break;
        default:
            // Other actions (bed, horse) not yet implemented
            return;
    }

    // Broadcast entity metadata flags to all other players
    // Java: DataWatcher index 0, byte type:
    //   bit 0 = on fire, bit 1 = sneaking, bit 3 = sprinting
    //   bit 4 = eating/drinking, bit 5 = invisible
    uint8_t flags = 0;
    if (isSneaking_) flags |= 0x02;  // bit 1
    if (isSprinting_) flags |= 0x08; // bit 3

    server_.broadcastEntityMetadataFlags(entityId_, flags);
}

// ═══════════════════════════════════════════════════════════════════════════
// handleTabComplete — Send command/player name completions.
// Java: NetHandlerPlayServer.processTabComplete()
// Java: MinecraftServer.getPossibleCompletions(ICommandSender, String)
// ═══════════════════════════════════════════════════════════════════════════
void PlayHandler::handleTabComplete(const uint8_t* data, size_t length, Connection& conn) {
    PacketReader reader(data, length);
    auto pkt = SB_TabComplete::read(reader);

    std::vector<std::string> completions;

    if (pkt.text.empty()) {
        // No text to complete
    } else if (pkt.text[0] == '/') {
        // Command completion
        // Java: this.serverController.getPossibleCompletions(this.playerEntity, message)
        // The Java implementation strips the leading '/' internally in CommandHandler
        std::string withoutSlash = pkt.text.substr(1);

        // Create a simple command sender for permission checks
        // The PlayerCommandSender is defined in the same compilation unit
        // For simplicity, we create a temporary sender
        class TempSender : public ICommandSender {
        public:
            TempSender(const std::string& name, MinecraftServer& server)
                : name_(name), server_(server) {}
            std::string getCommandSenderName() const override { return name_; }
            bool canCommandSenderUseCommand(int, const std::string&) const override { return true; }
            void addChatMessage(const std::string&) override {} // Tab complete doesn't need chat
            MinecraftServer* getServer() const override { return &server_; }
        private:
            std::string name_;
            MinecraftServer& server_;
        };

        TempSender sender(playerName_, server_);
        auto cmdCompletions = server_.getCommandHandler().getPossibleCommands(sender, withoutSlash);

        // Prepend '/' to each completion
        for (const auto& c : cmdCompletions) {
            completions.push_back("/" + c);
        }
    } else {
        // Player name completion
        // Java: MinecraftServer.getPossibleCompletions checks for player names
        // if the message doesn't start with '/'
        auto lastSpace = pkt.text.rfind(' ');
        std::string partial = (lastSpace != std::string::npos) ?
            pkt.text.substr(lastSpace + 1) : pkt.text;

        if (!partial.empty()) {
            auto playerNames = server_.getOnlinePlayerNames();
            for (const auto& name : playerNames) {
                // Case-insensitive prefix match
                if (name.size() >= partial.size()) {
                    bool match = true;
                    for (size_t i = 0; i < partial.size(); ++i) {
                        if (std::tolower(name[i]) != std::tolower(partial[i])) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        completions.push_back(name);
                    }
                }
            }
            std::sort(completions.begin(), completions.end());
        }
    }

    // Send S3A TabComplete response
    auto tabPkt = PacketBuilder::tabComplete(completions);
    conn.sendPacket(tabPkt);
}

// ═══════════════════════════════════════════════════════════════════════════
// handleCreativeInventory — C10PacketCreativeInventoryAction handler.
// Java reference: NetHandlerPlayServer.processCreativeInventoryAction()
//
// C10 format: Short slotId, ItemStack stack
//   slotId < 0 → drop item in world
//   slotId 1-44 → set container slot
// ═══════════════════════════════════════════════════════════════════════════

// readItemStack — Parse ItemStack from protocol buffer.
// Java reference: PacketBuffer.readItemStackFromBuffer()
// Format: Short itemId, [if >= 0: Byte count, Short damage, Short nbtLen, [nbt bytes]]
namespace {
std::optional<ItemStack> readItemStackFromBuffer(const uint8_t* data, size_t length, size_t& offset) {
    if (offset + 2 > length) return std::nullopt;
    int16_t itemId = static_cast<int16_t>((data[offset] << 8) | data[offset + 1]);
    offset += 2;

    if (itemId < 0) {
        return std::nullopt; // empty slot
    }

    if (offset + 3 > length) return std::nullopt;
    uint8_t count = data[offset++];
    int16_t damage = static_cast<int16_t>((data[offset] << 8) | data[offset + 1]);
    offset += 2;

    // Read NBT tag length (Short)
    // Java: readNBTTagCompoundFromBuffer() → readShort() for length
    if (offset + 2 > length) return std::nullopt;
    int16_t nbtLen = static_cast<int16_t>((data[offset] << 8) | data[offset + 1]);
    offset += 2;

    if (nbtLen > 0) {
        // Skip NBT data (we don't parse NBT from creative packets yet)
        if (offset + static_cast<size_t>(nbtLen) > length) return std::nullopt;
        offset += static_cast<size_t>(nbtLen);
    }

    return ItemStack(static_cast<int32_t>(itemId), static_cast<int32_t>(count),
                     static_cast<int32_t>(damage));
}
} // anonymous namespace

void PlayHandler::handleCreativeInventory(const uint8_t* data, size_t length, Connection& conn) {
    // Java reference: NetHandlerPlayServer.processCreativeInventoryAction()
    // C10PacketCreativeInventoryAction: Short slotId, ItemStack stack
    //
    // Java source lines 656-679:
    //   if (this.playerEntity.theItemInWorldManager.isCreative())
    //   {
    //       boolean flag = packetIn.func_149627_c() < 0;
    //       ItemStack itemstack = packetIn.func_149625_d();
    //       boolean flag1 = packetIn.func_149627_c() >= 1
    //           && packetIn.func_149627_c() < 36 + InventoryPlayer.getHotbarSize();
    //       boolean flag2 = itemstack == null || itemstack.getItem() != null;
    //       boolean flag3 = itemstack == null ||
    //           itemstack.getItemDamage() >= 0 &&
    //           itemstack.stackSize <= 64 &&
    //           itemstack.stackSize > 0;
    //
    //       if (flag1 && flag2 && flag3) {
    //           if (itemstack == null) {
    //               this.playerEntity.inventoryContainer.putStackInSlot(
    //                   packetIn.func_149627_c(), (ItemStack)null);
    //           } else {
    //               this.playerEntity.inventoryContainer.putStackInSlot(
    //                   packetIn.func_149627_c(), itemstack);
    //           }
    //           this.playerEntity.inventoryContainer.setPlayerIsPresent(
    //               this.playerEntity, true);
    //       }
    //       else if (flag && flag2 && flag3 && this.field_147366_g < 200) {
    //           this.field_147366_g += 20;
    //           EntityItem entityitem = this.playerEntity.dropPlayerItemWithRandomChoice(
    //               itemstack, true);
    //           if (entityitem != null) {
    //               entityitem.setAgeToCreativeDespawnTime();
    //           }
    //       }
    //   }

    // Must be in creative mode
    if (gameMode_ != 1) return;

    // Parse C10 packet: Short slotId + ItemStack
    if (length < 2) return;
    int16_t slotId = static_cast<int16_t>((data[0] << 8) | data[1]);
    size_t offset = 2;
    auto itemstack = readItemStackFromBuffer(data, length, offset);

    bool isDropAction = slotId < 0;
    // Java: slotId >= 1 && slotId < 36 + InventoryPlayer.getHotbarSize() (= 45)
    bool isValidSlot = slotId >= 1 && slotId < 36 + 9; // slots 1-44

    // Java: flag2 = itemstack == null || itemstack.getItem() != null
    // We check that the item ID is valid (exists in Item registry)
    bool isValidItem = !itemstack.has_value() || itemstack->getItemId() > 0;

    // Java: flag3 = itemstack == null || (damage >= 0 && stackSize <= 64 && stackSize > 0)
    bool isValidStack = !itemstack.has_value() ||
        (itemstack->getDamage() >= 0 &&
         itemstack->getStackSize() <= 64 &&
         itemstack->getStackSize() > 0);

    if (isValidSlot && isValidItem && isValidStack) {
        // Place item in container slot
        if (!container_) return;
        container_->putStackInSlot(slotId, itemstack);

        // Send S2F SetSlot to confirm
        sendSetSlot(conn, 0, slotId, itemstack);

        std::cout << "[Creative] " << playerName_ << " set slot " << slotId
                  << " to " << (itemstack ? std::to_string(itemstack->getItemId()) +
                     "x" + std::to_string(itemstack->getStackSize()) : "empty")
                  << "\n";
    }
    else if (isDropAction && isValidItem && isValidStack && itemDropThreshold_ < 200) {
        // Drop item into world
        // Java: field_147366_g += 20 (throttle)
        itemDropThreshold_ += 20;

        if (itemstack) {
            // Spawn the item in the world near the player
            server_.spawnItemDrop(playerX_, playerY_ + 1.62, playerZ_,
                                 itemstack->getItemId(),
                                 static_cast<int32_t>(itemstack->getDamage()),
                                 itemstack->getStackSize());

            std::cout << "[Creative] " << playerName_ << " dropped "
                      << itemstack->getItemId() << "x" << itemstack->getStackSize()
                      << "\n";
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// tickFood — Per-tick food/hunger processing.
// Java reference: EntityPlayer.onUpdate() → FoodStats.onUpdate()
//
// Called once per server tick for each Play-state connection.
// Uses the FoodStats struct (from FoodStats.h) with TickResult return value.
// ═══════════════════════════════════════════════════════════════════════════
void PlayHandler::tickFood(Connection& conn) {
    if (dead_) return;

    // ─── Environmental damage — Java: Entity.onUpdate + EntityLivingBase.onEntityUpdate() ───
    auto& worlds = server_.getWorlds();
    if (!worlds.empty()) {
        WorldServer* world = worlds[0].get();

        // ─── Void damage — Java: Entity.kill() when Y < -64 ──────────────
        // This applies even in creative mode (Java parity)
        if (playerY_ < -64.0) {
            health_ -= 4.0f;
            if (health_ < 0.0f) health_ = 0.0f;
            sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
            if (health_ <= 0.0f) {
                dead_ = true;
                server_.broadcastEntityEvent(entityId_, 3);
                server_.broadcastChatMessage(playerName_ + " fell out of the world");
            }
        }

        // Remaining env damage only in survival/adventure
        if (gameMode_ != 1) {
            int32_t headBlockX = static_cast<int32_t>(std::floor(playerX_));
            int32_t headBlockY = static_cast<int32_t>(std::floor(playerY_ + 1.62)); // eye height
            int32_t headBlockZ = static_cast<int32_t>(std::floor(playerZ_));
            int32_t feetBlockX = static_cast<int32_t>(std::floor(playerX_));
            int32_t feetBlockY = static_cast<int32_t>(std::floor(playerY_));
            int32_t feetBlockZ = static_cast<int32_t>(std::floor(playerZ_));

            Block* headBlock = world->getBlock(headBlockX, headBlockY, headBlockZ);
            int32_t headBlockId = headBlock ? Block::getIdFromBlock(headBlock) : 0;
            Block* feetBlock = world->getBlock(feetBlockX, feetBlockY, feetBlockZ);
            int32_t feetBlockId = feetBlock ? Block::getIdFromBlock(feetBlock) : 0;

            bool isInWater = (headBlockId == 8 || headBlockId == 9);
            bool isInLava = (feetBlockId == 10 || feetBlockId == 11 ||
                             headBlockId == 10 || headBlockId == 11);
            bool isInFire = (feetBlockId == 51); // fire block

            // ─── Drowning ─────────────────────────────────────────────
            if (isInWater) {
                --airSupply_;
                if (airSupply_ <= -20) {
                    airSupply_ = 0;
                    health_ -= 2.0f;
                    if (health_ < 0.0f) health_ = 0.0f;
                    sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
                    server_.broadcastSound("game.player.hurt", playerX_, playerY_, playerZ_, 1.0f, 1.0f);
                    if (health_ <= 0.0f) {
                        dead_ = true;
                        server_.broadcastEntityEvent(entityId_, 3);
                        server_.broadcastChatMessage(playerName_ + " drowned");
                    }
                }
                // Water extinguishes fire
                if (fireTicks_ > 0) fireTicks_ = 0;
            } else {
                airSupply_ = 300;
            }

            // ─── Lava damage — Java: Entity.onEntityUpdate → setFire(15) + 4 dmg ───
            if (isInLava) {
                health_ -= 4.0f;
                if (health_ < 0.0f) health_ = 0.0f;
                sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
                server_.broadcastSound("game.player.hurt", playerX_, playerY_, playerZ_, 1.0f, 1.0f);
                fireTicks_ = 300; // 15 seconds on fire (Java: setFire(15))
                if (health_ <= 0.0f) {
                    dead_ = true;
                    server_.broadcastEntityEvent(entityId_, 3);
                    server_.broadcastChatMessage(playerName_ + " tried to swim in lava");
                }
            }

            // ─── Fire block damage — Java: Entity.dealFireDamage(1) ───
            if (isInFire && !isInLava) {
                health_ -= 1.0f;
                if (health_ < 0.0f) health_ = 0.0f;
                sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
                fireTicks_ = std::max(fireTicks_, 160); // 8 seconds on fire
                if (health_ <= 0.0f) {
                    dead_ = true;
                    server_.broadcastEntityEvent(entityId_, 3);
                    server_.broadcastChatMessage(playerName_ + " went up in flames");
                }
            }

            // ─── Burning damage — Java: Entity.onEntityUpdate → fire tick + 1 dmg/sec ───
            if (fireTicks_ > 0) {
                --fireTicks_;
                // Deal 1 damage per second (every 20 ticks) while on fire
                if (fireTicks_ % 20 == 0 && fireTicks_ > 0) {
                    health_ -= 1.0f;
                    if (health_ < 0.0f) health_ = 0.0f;
                    sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
                    if (health_ <= 0.0f) {
                        dead_ = true;
                        server_.broadcastEntityEvent(entityId_, 3);
                        server_.broadcastChatMessage(playerName_ + " burned to death");
                    }
                }
            }

            // ─── Suffocation — Java: isEntityInsideOpaqueBlock() → DamageSource.inWall ───
            // Only full opaque solid blocks cause suffocation (not torches, flowers, etc.)
            // Check block at HEAD position: must be a full-cube solid block
            if (!isInWater && !isInLava && headBlockId != 0) {
                // List of block IDs that are full opaque cubes — anything that isn't
                // transparent, partial, or non-solid
                bool isOpaqueFullCube = false;
                switch (headBlockId) {
                    // Stone, granite, diorite, andesite
                    case 1: case 2: case 3: case 4: case 7: // stone, grass, dirt, cobble, bedrock
                    case 12: case 13: case 14: case 15: case 16: // sand, gravel, gold_ore, iron_ore, coal_ore
                    case 17: case 21: case 22: case 24: case 35: // log, lapis_ore, lapis_block, sandstone, wool
                    case 41: case 42: case 43: case 45: case 46: // gold/iron/doubleslab/brick/tnt
                    case 48: case 49: case 56: case 57: case 58: // mossy/obsidian/diamond_ore/diamond/crafting
                    case 60: case 61: case 62: case 73: case 74: // farmland/furnace/lit_furnace/redstone_ore
                    case 79: case 80: case 82: case 86: case 87: // ice, snow_block, clay, pumpkin, netherrack
                    case 88: case 89: case 91: case 97: case 98: // soulsand/glowstone/jack_o/silverfish/stonebrick
                    case 103: case 110: case 112: case 121: case 123: case 124: // melon/mycelium/nether_brick/end_stone/redstone_lamp
                    case 125: case 129: case 133: case 152: case 153: // double_wooden_slab/emerald_ore/emerald_block/redstone_block/nether_quartz_ore
                    case 155: case 159: case 162: case 170: case 172: // quartz/stained_clay/log2/hay/hardened_clay
                    case 173:  // coal_block
                        isOpaqueFullCube = true;
                        break;
                }
                if (isOpaqueFullCube) {
                    health_ -= 1.0f;
                    if (health_ < 0.0f) health_ = 0.0f;
                    sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
                    if (health_ <= 0.0f) {
                        dead_ = true;
                        server_.broadcastEntityEvent(entityId_, 3);
                        server_.broadcastChatMessage(playerName_ + " suffocated in a wall");
                    }
                }
            }
        }
    }

    // Get difficulty — WorldServer doesn't expose difficulty yet, default to Normal
    // Java: entityPlayer.worldObj.difficultySetting
    int32_t difficulty = 2; // Normal (TODO: expose difficulty on WorldServer)

    // Check naturalRegeneration game rule
    // GameRules not fully wired to WorldServer yet, default to true
    bool naturalRegen = true;

    float prevHealth = health_;
    int32_t prevFood = foodStats_.getFoodLevel();

    // Java: EntityPlayer.shouldHeal() → health > 0 && health < maxHealth
    bool canHeal = (health_ > 0.0f && health_ < 20.0f);

    // Tick the food system
    auto result = foodStats_.onUpdate(difficulty, naturalRegen, health_, canHeal);

    // Apply heal
    if (result.shouldHeal) {
        health_ = std::min(health_ + 1.0f, 20.0f);
    }

    // Apply starvation damage
    if (result.shouldStarve) {
        health_ = std::max(0.0f, health_ - result.starveDamage);
        if (health_ <= 0.0f) {
            dead_ = true;
        }
    }

    // Send S06 UpdateHealth if anything changed
    if (health_ != prevHealth || foodStats_.getFoodLevel() != prevFood) {
        sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
    }

    // Tick hurt resistant time
    if (hurtResistantTime_ > 0) {
        --hurtResistantTime_;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// S1D EntityEffect — send a potion effect to the client
// Java reference: S1DPacketEntityEffect
// Protocol: VarInt entityId, Byte effectId, Byte amplifier, Short duration
// ═══════════════════════════════════════════════════════════════════════════
void PlayHandler::sendEntityEffect(Connection& conn, int32_t entityId, int8_t effectId,
                                    int8_t amplifier, int16_t duration) {
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, 0x1D); // S1D
    writeVarInt(pkt, entityId);
    pkt.push_back(static_cast<uint8_t>(effectId));
    pkt.push_back(static_cast<uint8_t>(amplifier));
    writeShort(pkt, duration);
    conn.sendPacket(std::move(pkt));
}

// ═══════════════════════════════════════════════════════════════════════════
// S1E RemoveEntityEffect — remove a potion effect from the client
// Java reference: S1EPacketRemoveEntityEffect
// Protocol: VarInt entityId, Byte effectId
// ═══════════════════════════════════════════════════════════════════════════
void PlayHandler::sendRemoveEntityEffect(Connection& conn, int32_t entityId, int8_t effectId) {
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, 0x1E); // S1E
    writeVarInt(pkt, entityId);
    pkt.push_back(static_cast<uint8_t>(effectId));
    conn.sendPacket(std::move(pkt));
}

void PlayHandler::addPotionEffect(Connection& conn, int32_t effectId, int32_t duration, int32_t amplifier) {
    // Java reference: EntityLivingBase.addPotionEffect(PotionEffect)
    // If effect already active, update if new is stronger or same-strength-longer
    auto it = activePotionEffects_.find(effectId);
    if (it != activePotionEffects_.end()) {
        if (amplifier > it->second.amplifier ||
            (amplifier == it->second.amplifier && duration > it->second.duration)) {
            it->second.amplifier = amplifier;
            it->second.duration = duration;
        } else {
            return; // Current effect is stronger
        }
    } else {
        activePotionEffects_[effectId] = {effectId, amplifier, duration};
    }

    // Send S1D to client
    int16_t clampedDuration = static_cast<int16_t>(std::min(duration, 32767));
    sendEntityEffect(conn, entityId_, static_cast<int8_t>(effectId),
                     static_cast<int8_t>(amplifier), clampedDuration);

    std::cout << "[Effect] " << playerName_ << " got effect " << effectId
              << " amplifier " << amplifier << " for " << duration << " ticks\n";
}

void PlayHandler::removePotionEffect(Connection& conn, int32_t effectId) {
    auto it = activePotionEffects_.find(effectId);
    if (it != activePotionEffects_.end()) {
        activePotionEffects_.erase(it);
        sendRemoveEntityEffect(conn, entityId_, static_cast<int8_t>(effectId));
    }
}

void PlayHandler::clearPotionEffects(Connection& conn) {
    for (auto& [id, effect] : activePotionEffects_) {
        sendRemoveEntityEffect(conn, entityId_, static_cast<int8_t>(id));
    }
    activePotionEffects_.clear();
}

void PlayHandler::tickPotionEffects(Connection& conn) {
    // Java reference: EntityLivingBase.onUpdate() → potionsUpdateTick
    std::vector<int32_t> expired;

    for (auto& [id, effect] : activePotionEffects_) {
        --effect.duration;

        // Apply per-tick effects
        // Java reference: Potion.performEffect()
        switch (id) {
            case 10: // Regeneration — heal every (50 >> amplifier) ticks
            {
                int interval = std::max(1, 50 >> effect.amplifier);
                if (effect.duration % interval == 0 && health_ < 20.0f) {
                    health_ = std::min(20.0f, health_ + 1.0f);
                    sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
                }
                break;
            }
            case 19: // Poison — damage every (25 >> amplifier) ticks (won't kill)
            {
                int interval = std::max(1, 25 >> effect.amplifier);
                if (effect.duration % interval == 0 && health_ > 1.0f) {
                    health_ = std::max(1.0f, health_ - 1.0f);
                    sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
                }
                break;
            }
            case 20: // Wither — damage every (40 >> amplifier) ticks (CAN kill)
            {
                int interval = std::max(1, 40 >> effect.amplifier);
                if (effect.duration % interval == 0) {
                    health_ -= 1.0f;
                    if (health_ <= 0.0f) {
                        health_ = 0.0f;
                        dead_ = true;
                    }
                    sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
                }
                break;
            }
            case 23: // Saturation — restore food every tick
            {
                if (foodStats_.getFoodLevel() < 20) {
                    foodStats_.addStats(1 + effect.amplifier, 0.0f);
                    sendUpdateHealth(conn, health_, foodStats_.getFoodLevel(), foodStats_.getSaturationLevel());
                }
                break;
            }
            // Speed (1), Slowness (2), Haste (3), Mining Fatigue (4), Strength (5),
            // Instant Health (6), Instant Damage (7), Jump Boost (8), Nausea (9),
            // Night Vision (16), Hunger (17), Weakness (18), Absorption (22), etc.
            // These are client-side visual/movement effects — no server tick needed
            default:
                break;
        }

        if (effect.duration <= 0) {
            expired.push_back(id);
        }
    }

    for (int32_t id : expired) {
        activePotionEffects_.erase(id);
        sendRemoveEntityEffect(conn, entityId_, static_cast<int8_t>(id));
    }
}

} // namespace mccpp

