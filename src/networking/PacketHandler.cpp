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

#include <cmath>

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
    sendSetExperience(conn, 0.0f, 0, 0);

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
        case ServerboundPacket::ClickWindow:
        case ServerboundPacket::ConfirmTransaction:
        case ServerboundPacket::UpdateSign:
            // Silently consume unimplemented inventory/sign packets
            break;
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
        // NBT tag — for now, no NBT (Short -1 = no tag)
        // Java: writeNBTTagCompoundToBuffer(null) → writeShort(-1)
        writeShort(pkt, -1);
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
    // Java reference: S19PacketEntityHeadLook.writePacketData()
    // Format: Int entityId, Byte yaw
    std::vector<uint8_t> pkt;
    writeVarInt(pkt, ClientboundPacket::EntityHeadLook);
    writeInt(pkt, entityId);
    writeByte(pkt, static_cast<uint8_t>(static_cast<int8_t>(yaw * 256.0f / 360.0f)));
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
    double oldX = playerX_, oldZ = playerZ_;
    playerX_ = readDouble(data);
    playerY_ = readDouble(data + 8);
    // stance = readDouble(data + 16) — head Y, not stored separately
    playerZ_ = readDouble(data + 24);
    playerOnGround_ = data[32] != 0;

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
    double oldX = playerX_, oldZ = playerZ_;
    playerX_ = readDouble(data);
    playerY_ = readDouble(data + 8);
    // stance = readDouble(data + 16)
    playerZ_ = readDouble(data + 24);
    playerYaw_ = readFloat(data + 32);
    playerPitch_ = readFloat(data + 36);
    playerOnGround_ = data[40] != 0;

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
    // 0 = Started digging (in creative: instant break)
    // 1 = Cancelled digging
    // 2 = Finished digging
    // 3 = Drop item stack
    // 4 = Drop item
    // 5 = Shoot arrow / finish eating

    if (status == 3 || status == 4 || status == 5) {
        // Drop item / shoot arrow — silently consume for now
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

    if (status == 0 || status == 2) {
        // Java: status 0 = start digging. In creative mode, instant break.
        // Status 2 = finished digging (survival). We treat both as instant break for now.
        Block* existingBlock = world->getBlock(blockX, blockY, blockZ);
        if (existingBlock && Block::getIdFromBlock(existingBlock) != 0) {
            int32_t brokenBlockId = Block::getIdFromBlock(existingBlock);
            int32_t brokenMeta = world->getBlockMetadata(blockX, blockY, blockZ);

            // Set to air (block ID 0)
            world->setBlock(blockX, blockY, blockZ, Block::getBlockById(0));
            world->setBlockMetadata(blockX, blockY, blockZ, 0);

            std::cout << "[World] " << playerName_ << " broke block at "
                      << blockX << "," << (int)blockY << "," << blockZ << "\n";

            // Broadcast block change to all players
            server_.broadcastBlockChange(blockX, blockY, blockZ, 0, 0);

            // Play break sound — Java reference: worldObj.playAuxSFXAtEntity
            // Using generic "dig.stone" sound; volume=1.0, pitch=0.8 (matches vanilla)
            server_.broadcastSound("dig.stone",
                static_cast<double>(blockX) + 0.5,
                static_cast<double>(blockY) + 0.5,
                static_cast<double>(blockZ) + 0.5,
                1.0f, 0.8f);

            // Spawn item drop — Java reference: Block.dropBlockAsItem
            server_.spawnItemDrop(
                static_cast<double>(blockX),
                static_cast<double>(blockY),
                static_cast<double>(blockZ),
                brokenBlockId, brokenMeta, 1);
        }
    }
    // status 1 = cancel — nothing to do in creative mode
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
    // For now, skip the held item slot ItemStack (complex NBT parsing) and place stone (ID 1)
    // TODO: Read held item from packet and use Item→Block mapping
    // After offset 10: Short heldItemId
    int32_t placeBlockId = 1; // stone fallback
    if (length >= 12) {
        int16_t heldItemId = static_cast<int16_t>((data[10] << 8) | data[11]);
        if (heldItemId >= 0 && heldItemId < 256) {
            // Direct block IDs (items 0-255 correspond to blocks)
            Block* heldBlock = Block::getBlockById(heldItemId);
            if (heldBlock && Block::getIdFromBlock(heldBlock) != 0) {
                placeBlockId = heldItemId;
            }
        }
    }

    // Place the block
    Block* newBlock = Block::getBlockById(placeBlockId);
    if (!newBlock) return;

    world->setBlock(placeX, placeY, placeZ, newBlock);

    std::cout << "[World] " << playerName_ << " placed block " << placeBlockId
              << " at " << placeX << "," << placeY << "," << placeZ << "\n";

    // Broadcast block change to all players
    server_.broadcastBlockChange(placeX, placeY, placeZ, placeBlockId, 0);

    // Play place sound — Java reference: Block.onBlockPlacedBy / stepSound
    // Using generic "dig.stone" sound; volume=1.0, pitch=0.8 (matches vanilla)
    server_.broadcastSound("dig.stone",
        static_cast<double>(placeX) + 0.5,
        static_cast<double>(placeY) + 0.5,
        static_cast<double>(placeZ) + 0.5,
        1.0f, 0.8f);
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
        server_.handlePlayerAttack(*this, targetEntityId);
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

} // namespace mccpp

