/**
 * ConnectionSequencePackets.h — Handshake, Status, and Login packet structures.
 *
 * Java references:
 *   Handshake:
 *     - C00Handshake (52 lines)
 *   Status:
 *     - C00PacketServerQuery (20 lines) — empty packet
 *     - C01PacketPing (28 lines)
 *     - S00PacketServerInfo (56 lines)
 *     - S01PacketPong (28 lines)
 *   Login:
 *     - C00PacketLoginStart (41 lines)
 *     - C01PacketEncryptionResponse (46 lines)
 *     - S00PacketDisconnect (32 lines)
 *     - S01PacketEncryptionRequest (46 lines)
 *     - S02PacketLoginSuccess (48 lines)
 *
 * Connection state flow:
 *   1. Client → Handshake(C00): protocol=5, ip, port, nextState(1=Status|2=Login)
 *   2a. Status path: Client → ServerQuery(C00) → Server → ServerInfo(S00, JSON)
 *                    Client → Ping(C01, long time) → Server → Pong(S01, echo time)
 *   2b. Login path:  Client → LoginStart(C00, name)
 *                    [Online mode]:
 *                      Server → EncryptionRequest(S01, serverId, publicKey, verifyToken)
 *                      Client → EncryptionResponse(C01, encryptedSharedSecret, encryptedVerifyToken)
 *                      [Server verifies with Mojang session server]
 *                      [Enable AES/CFB8 encryption on both sides]
 *                    Server → LoginSuccess(S02, uuid, name)
 *                    [State transitions to Play]
 *
 * Wire format notes:
 *   - All packets are VarInt length-prefixed: [VarInt length][VarInt packetId][payload]
 *   - Blob format: [Short length][byte[length]]
 *   - String format: [VarInt length][UTF-8 bytes]
 *
 * Thread safety: Packet structs are value types, one per connection.
 * JNI readiness: Simple POD structs.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Connection States
// Java: net.minecraft.network.EnumConnectionState
//   HANDSHAKING(-1), PLAY(0), STATUS(1), LOGIN(2)
// ═══════════════════════════════════════════════════════════════════════════

enum class ConnectionState : int32_t {
    HANDSHAKING = -1,
    PLAY = 0,
    STATUS = 1,
    LOGIN = 2
};

// ═══════════════════════════════════════════════════════════════════════════
// Protocol constants
// ═══════════════════════════════════════════════════════════════════════════

namespace ProtocolConstants {
    static constexpr int32_t PROTOCOL_VERSION = 5;     // Minecraft 1.7.10
    static constexpr int32_t MAX_PACKET_SIZE = 2097152; // 2 MiB
    static constexpr int32_t VARINT_MAX_BYTES = 5;

    // Handshake string limits
    static constexpr int32_t MAX_SERVER_ADDRESS = 255;

    // Login string limits
    static constexpr int32_t MAX_USERNAME_LENGTH = 16;
    static constexpr int32_t MAX_SERVER_ID_LENGTH = 20;
    static constexpr int32_t UUID_STRING_LENGTH = 36;

    // Status
    static constexpr int32_t MAX_SERVER_INFO_LENGTH = 32767; // Short.MAX_VALUE

    // Encryption
    static constexpr int32_t VERIFY_TOKEN_LENGTH = 4;
    static constexpr int32_t AES_KEY_LENGTH = 16;  // AES-128/CFB8
}

// ═══════════════════════════════════════════════════════════════════════════
//  ██╗  ██╗ █████╗ ███╗   ██╗██████╗ ███████╗██╗  ██╗ █████╗ ██╗  ██╗███████╗
//  ██║  ██║██╔══██╗████╗  ██║██╔══██╗██╔════╝██║  ██║██╔══██╗██║ ██╔╝██╔════╝
//  ███████║███████║██╔██╗ ██║██║  ██║███████╗███████║███████║█████╔╝ █████╗
//  ██╔══██║██╔══██║██║╚██╗██║██║  ██║╚════██║██╔══██║██╔══██║██╔═██╗ ██╔══╝
//  ██║  ██║██║  ██║██║ ╚████║██████╔╝███████║██║  ██║██║  ██║██║  ██╗███████╗
//  ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝
// ═══════════════════════════════════════════════════════════════════════════

// C00 — Handshake
// State: HANDSHAKING → STATUS or LOGIN
// Packet ID: 0x00
// Fields: VarInt protocolVersion, String serverAddress(max 255),
//         UShort serverPort, VarInt nextState(1=status, 2=login)
struct C00Handshake {
    static constexpr uint8_t PACKET_ID = 0x00;
    int32_t protocolVersion = 0;
    std::string serverAddress;  // Max 255 chars
    uint16_t serverPort = 0;
    int32_t nextState = 0;      // 1=Status, 2=Login

    bool requestsStatus() const { return nextState == 1; }
    bool requestsLogin() const { return nextState == 2; }
    bool isProtocolValid() const {
        return protocolVersion == ProtocolConstants::PROTOCOL_VERSION;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  ███████╗████████╗ █████╗ ████████╗██╗   ██╗███████╗
//  ██╔════╝╚══██╔══╝██╔══██╗╚══██╔══╝██║   ██║██╔════╝
//  ███████╗   ██║   ███████║   ██║   ██║   ██║███████╗
//  ╚════██║   ██║   ██╔══██║   ██║   ██║   ██║╚════██║
//  ███████║   ██║   ██║  ██║   ██║   ╚██████╔╝███████║
//  ╚══════╝   ╚═╝   ╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚══════╝
// ═══════════════════════════════════════════════════════════════════════════

// C00 — Server Query (status request)
// State: STATUS
// Packet ID: 0x00
// Fields: (empty — no payload)
struct StatusC00ServerQuery {
    static constexpr uint8_t PACKET_ID = 0x00;
    // No fields
};

// C01 — Ping
// State: STATUS
// Packet ID: 0x01
// Fields: Long time (echoed back in pong)
struct StatusC01Ping {
    static constexpr uint8_t PACKET_ID = 0x01;
    int64_t time = 0;
};

// S00 — Server Info (status response)
// State: STATUS
// Packet ID: 0x00
// Fields: String jsonResponse (ServerStatusResponse as JSON)
//
// JSON format:
// {
//   "version": { "name": "1.7.10", "protocol": 5 },
//   "players": {
//     "max": 20,
//     "online": 5,
//     "sample": [{ "name": "...", "id": "uuid" }, ...]
//   },
//   "description": { "text": "A Minecraft Server" },
//   "favicon": "data:image/png;base64,<base64 64x64 PNG>"
// }
struct StatusS00ServerInfo {
    static constexpr uint8_t PACKET_ID = 0x00;
    std::string jsonResponse;  // Max Short.MAX_VALUE (32767) chars
};

// S01 — Pong
// State: STATUS
// Packet ID: 0x01
// Fields: Long time (echoed from ping)
struct StatusS01Pong {
    static constexpr uint8_t PACKET_ID = 0x01;
    int64_t time = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// ServerStatusResponse — JSON structure for server list ping.
// Java: net.minecraft.network.ServerStatusResponse
// ═══════════════════════════════════════════════════════════════════════════

struct ServerStatusVersion {
    std::string name = "1.7.10";
    int32_t protocol = ProtocolConstants::PROTOCOL_VERSION;
};

struct ServerStatusPlayerSample {
    std::string name;
    std::string id;  // UUID string
};

struct ServerStatusPlayers {
    int32_t max = 20;
    int32_t online = 0;
    std::vector<ServerStatusPlayerSample> sample;
};

struct ServerStatusDescription {
    std::string text = "A MineCPPaft Server";
};

struct ServerStatusResponse {
    ServerStatusVersion version;
    ServerStatusPlayers players;
    ServerStatusDescription description;
    std::string favicon;  // "data:image/png;base64,..." (64x64 PNG)

    // Build JSON response string
    // Java: Gson serialization
    std::string toJson() const {
        std::string json = "{";

        // version
        json += "\"version\":{\"name\":\"" + version.name +
                "\",\"protocol\":" + std::to_string(version.protocol) + "},";

        // players
        json += "\"players\":{\"max\":" + std::to_string(players.max) +
                ",\"online\":" + std::to_string(players.online);
        if (!players.sample.empty()) {
            json += ",\"sample\":[";
            for (size_t i = 0; i < players.sample.size(); ++i) {
                if (i > 0) json += ",";
                json += "{\"name\":\"" + players.sample[i].name +
                        "\",\"id\":\"" + players.sample[i].id + "\"}";
            }
            json += "]";
        }
        json += "},";

        // description
        json += "\"description\":{\"text\":\"" + description.text + "\"}";

        // favicon
        if (!favicon.empty()) {
            json += ",\"favicon\":\"" + favicon + "\"";
        }

        json += "}";
        return json;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  ██╗      ██████╗  ██████╗ ██╗███╗   ██╗
//  ██║     ██╔═══██╗██╔════╝ ██║████╗  ██║
//  ██║     ██║   ██║██║  ███╗██║██╔██╗ ██║
//  ██║     ██║   ██║██║   ██║██║██║╚██╗██║
//  ███████╗╚██████╔╝╚██████╔╝██║██║ ╚████║
//  ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝╚═╝  ╚═══╝
// ═══════════════════════════════════════════════════════════════════════════

// C00 — Login Start
// State: LOGIN
// Packet ID: 0x00
// Fields: String name (max 16)
struct LoginC00LoginStart {
    static constexpr uint8_t PACKET_ID = 0x00;
    std::string playerName;  // Max 16 chars
};

// S00 — Login Disconnect
// State: LOGIN
// Packet ID: 0x00
// Fields: String reason (JSON chat component)
struct LoginS00Disconnect {
    static constexpr uint8_t PACKET_ID = 0x00;
    std::string reason;  // JSON chat component
};

// S01 — Encryption Request
// State: LOGIN
// Packet ID: 0x01
// Fields: String serverId (max 20, empty for offline),
//         Short keyLength, Byte[] publicKey (DER-encoded),
//         Short tokenLength, Byte[] verifyToken (4 random bytes)
struct LoginS01EncryptionRequest {
    static constexpr uint8_t PACKET_ID = 0x01;
    std::string serverId;                // Max 20 chars, "" for offline hash
    std::vector<uint8_t> publicKey;      // Server's RSA public key (DER X.509)
    std::vector<uint8_t> verifyToken;    // 4 random bytes
};

// C01 — Encryption Response
// State: LOGIN
// Packet ID: 0x01
// Fields: Short secretLength, Byte[] encryptedSharedSecret,
//         Short tokenLength, Byte[] encryptedVerifyToken
//
// Encrypted with server's RSA public key:
//   sharedSecret: 16 random bytes (AES-128 key for CFB8 stream)
//   verifyToken: echo of server's 4 verify token bytes
struct LoginC01EncryptionResponse {
    static constexpr uint8_t PACKET_ID = 0x01;
    std::vector<uint8_t> encryptedSharedSecret;  // RSA-encrypted AES key
    std::vector<uint8_t> encryptedVerifyToken;   // RSA-encrypted verify token echo
};

// S02 — Login Success
// State: LOGIN → PLAY
// Packet ID: 0x02
// Fields: String uuid (36 chars, with dashes), String username (max 16)
// NOTE: hasPriority() = true (sent before state change to PLAY)
struct LoginS02LoginSuccess {
    static constexpr uint8_t PACKET_ID = 0x02;
    std::string uuid;        // 36-char UUID with dashes
    std::string username;    // Max 16 chars
    bool hasPriority = true; // Sent before state transition
};

// ═══════════════════════════════════════════════════════════════════════════
// Encryption constants
// Java: CryptManager, MinecraftServer.getKeyPair()
// ═══════════════════════════════════════════════════════════════════════════

namespace EncryptionConstants {
    // RSA key size for server keypair
    static constexpr int32_t RSA_KEY_SIZE = 1024;

    // AES cipher: AES/CFB8/NoPadding
    // Used for stream encryption after login
    static constexpr int32_t AES_KEY_SIZE = 128;  // bits
    static constexpr int32_t AES_BLOCK_SIZE = 16;  // bytes

    // Mojang session server URL for online-mode verification
    // Java: "https://sessionserver.mojang.com/session/minecraft/hasJoined?username=...&serverId=..."
    static constexpr const char* SESSION_SERVER_URL =
        "https://sessionserver.mojang.com/session/minecraft/hasJoined";

    // Server ID hash: SHA-1 of (serverId + sharedSecret + publicKey)
    // Java: CryptManager.getServerIdHash(serverId, publicKey, sharedSecret)
    // Result: Notchian "special" hex digest (no leading zeros, negative prefix "-")
}

// ═══════════════════════════════════════════════════════════════════════════
// Connection state machine with packet ID→type mapping
// Java: EnumConnectionState — static initializer maps packet IDs per state
//
// HANDSHAKING:
//   C→S: 0x00 = C00Handshake
//
// STATUS:
//   C→S: 0x00 = C00PacketServerQuery, 0x01 = C01PacketPing
//   S→C: 0x00 = S00PacketServerInfo,  0x01 = S01PacketPong
//
// LOGIN:
//   C→S: 0x00 = C00PacketLoginStart,  0x01 = C01PacketEncryptionResponse
//   S→C: 0x00 = S00PacketDisconnect,  0x01 = S01PacketEncryptionRequest,
//         0x02 = S02PacketLoginSuccess
//
// PLAY:
//   C→S: 0x00-0x17 (26 packets, see ClientPlayPackets.h)
//   S→C: 0x00-0x40 (67 packets, see EntityMovementPackets.h, WorldEventPackets.h)
// ═══════════════════════════════════════════════════════════════════════════

} // namespace mccpp
