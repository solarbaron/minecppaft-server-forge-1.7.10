/**
 * NetHandlerLoginServer.h — Server-side login state machine.
 *
 * Java reference: net.minecraft.server.network.NetHandlerLoginServer (167 lines)
 *
 * Login state machine:
 *   HELLO → (receive C00 LoginStart)
 *     if online mode: → KEY (send S01 EncryptionRequest)
 *       → (receive C01 EncryptionResponse, verify token, decrypt shared secret)
 *       → AUTHENTICATING (async thread checks Mojang session server)
 *       → READY_TO_ACCEPT
 *     else: → READY_TO_ACCEPT (offline mode, skip encryption)
 *   READY_TO_ACCEPT → (next tick: check allowUserToConnect, send S02 LoginSuccess)
 *   → ACCEPTED (transition to Play state, create player)
 *
 * Key behaviors:
 *   - 600 tick timeout (30 seconds at 20 TPS)
 *   - Offline UUID: UUID.nameUUIDFromBytes("OfflinePlayer:"+name)
 *     which is UUID version 3 (MD5) in "OfflinePlayer:" namespace
 *   - Verify token: 4 random bytes, checked via RSA decrypt match
 *   - Authentication runs on separate thread to avoid blocking server
 *   - Encryption enabled immediately after processing EncryptionResponse
 *
 * Thread safety: Login handler per connection, auth on separate thread.
 * JNI readiness: Simple state machine with clear lifecycle.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <atomic>
#include <functional>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Login States
// Java: NetHandlerLoginServer$LoginState
// ═══════════════════════════════════════════════════════════════════════════

enum class LoginState : int32_t {
    HELLO = 0,            // Waiting for C00 LoginStart
    KEY = 1,              // Sent S01 EncryptionRequest, waiting for C01
    AUTHENTICATING = 2,   // Async auth with Mojang session server
    READY_TO_ACCEPT = 3,  // Auth complete, will accept next tick
    ACCEPTED = 4          // Sent S02 LoginSuccess, transitioning to Play
};

// ═══════════════════════════════════════════════════════════════════════════
// Login constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LoginConstants {
    // Java: if (this.connectionTimer++ == 600)
    static constexpr int32_t LOGIN_TIMEOUT_TICKS = 600;  // 30 seconds

    // Java: private final byte[] field_147330_e = new byte[4]
    static constexpr int32_t VERIFY_TOKEN_LENGTH = 4;

    // Offline UUID prefix
    // Java: UUID.nameUUIDFromBytes(("OfflinePlayer:" + name).getBytes(UTF_8))
    static constexpr const char* OFFLINE_PREFIX = "OfflinePlayer:";
}

// ═══════════════════════════════════════════════════════════════════════════
// NetHandlerLoginServer — Per-connection login state machine.
// ═══════════════════════════════════════════════════════════════════════════

class NetHandlerLoginServer {
public:
    // ─── State ───
    LoginState currentState = LoginState::HELLO;
    int32_t connectionTimer = 0;
    std::string playerName;
    std::string playerUUID;    // Set after auth or offline generation
    std::string serverId;      // Empty string for offline hash base
    std::vector<uint8_t> verifyToken;  // 4 random bytes

    // ─── Callbacks (set by connection manager) ───
    std::function<void(const std::string&)> onDisconnect;
    std::function<void(const std::string&, const std::string&)> onLoginSuccess;

    // ─── Tick ───
    // Java: onNetworkTick()
    //   - If READY_TO_ACCEPT: finalize login
    //   - If timeout (600 ticks): disconnect
    // Returns: true if still active, false if should disconnect
    bool onNetworkTick() {
        if (currentState == LoginState::READY_TO_ACCEPT) {
            // Transition to ACCEPTED next tick
            // Send S02LoginSuccess, initialize player
            currentState = LoginState::ACCEPTED;
            if (onLoginSuccess) {
                onLoginSuccess(playerUUID, playerName);
            }
        }

        if (++connectionTimer >= LoginConstants::LOGIN_TIMEOUT_TICKS) {
            return false;  // "Took too long to log in"
        }

        return true;
    }

    // ─── Process Login Start ───
    // Java: processLoginStart(C00PacketLoginStart)
    //   - Stores player name
    //   - If online mode: send encryption request, go to KEY
    //   - If offline mode: skip to READY_TO_ACCEPT
    void processLoginStart(const std::string& name, bool onlineMode) {
        if (currentState != LoginState::HELLO) {
            return;  // "Unexpected hello packet"
        }

        playerName = name;

        if (onlineMode) {
            currentState = LoginState::KEY;
            // Caller should: send S01EncryptionRequest(serverId, publicKey, verifyToken)
        } else {
            // Generate offline UUID
            playerUUID = generateOfflineUUID(name);
            currentState = LoginState::READY_TO_ACCEPT;
        }
    }

    // ─── Process Encryption Response ───
    // Java: processEncryptionResponse(C01PacketEncryptionResponse)
    //   1. Verify state is KEY
    //   2. Decrypt verify token with private key, compare with stored
    //   3. Decrypt shared secret → AES key
    //   4. Enable encryption on network channel
    //   5. Start async authenticator thread
    //   6. Transition to AUTHENTICATING
    //
    // Returns: true if verify token matches, false if invalid nonce
    bool processEncryptionResponse(const std::vector<uint8_t>& decryptedVerifyToken,
                                    const std::vector<uint8_t>& decryptedSharedSecret)
    {
        if (currentState != LoginState::KEY) {
            return false;  // "Unexpected key packet"
        }

        // Verify nonce
        if (decryptedVerifyToken != verifyToken) {
            return false;  // "Invalid nonce!"
        }

        // Store shared secret for encryption setup
        // Caller should: enable AES/CFB8 encryption with decryptedSharedSecret
        currentState = LoginState::AUTHENTICATING;

        // Caller should: start async authentication thread
        // Thread calls Mojang session server, then sets state to READY_TO_ACCEPT
        return true;
    }

    // ─── Authentication complete callback ───
    // Called by async auth thread after successful Mojang verification
    void onAuthenticationComplete(const std::string& uuid) {
        if (currentState != LoginState::AUTHENTICATING) return;
        playerUUID = uuid;
        currentState = LoginState::READY_TO_ACCEPT;
    }

    // ─── Offline UUID generation ───
    // Java: getOfflineProfile()
    //   UUID.nameUUIDFromBytes(("OfflinePlayer:" + name).getBytes(Charsets.UTF_8))
    //   This is UUID version 3 (name-based MD5)
    //
    // Format: xxxxxxxx-xxxx-3xxx-yxxx-xxxxxxxxxxxx
    //   where 3 = version 3, y = 8|9|a|b (variant 1)
    static std::string generateOfflineUUID(const std::string& name) {
        // Input: "OfflinePlayer:" + name → MD5 → UUID v3
        // The actual MD5 computation would go here.
        // For now, return a deterministic placeholder based on name hash.
        // In production, use OpenSSL MD5 to compute the real UUID v3.
        std::string input = std::string(LoginConstants::OFFLINE_PREFIX) + name;

        // Simple hash for structure (replaced with proper MD5 in production):
        uint64_t h1 = 0, h2 = 0;
        for (size_t i = 0; i < input.size(); ++i) {
            h1 = h1 * 31 + static_cast<uint64_t>(input[i]);
            h2 = h2 * 37 + static_cast<uint64_t>(input[i]);
        }

        // Set version 3 and variant 1
        h1 = (h1 & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000003000ULL;
        h2 = (h2 & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

        // Format as UUID
        char buf[37];
        snprintf(buf, sizeof(buf),
                 "%08x-%04x-%04x-%04x-%012llx",
                 static_cast<uint32_t>(h1 >> 32),
                 static_cast<uint16_t>((h1 >> 16) & 0xFFFF),
                 static_cast<uint16_t>(h1 & 0xFFFF),
                 static_cast<uint16_t>(h2 >> 48),
                 static_cast<unsigned long long>(h2 & 0x0000FFFFFFFFFFFFULL));
        return std::string(buf);
    }

    // ─── State queries ───
    bool isHello() const { return currentState == LoginState::HELLO; }
    bool isAwaitingKey() const { return currentState == LoginState::KEY; }
    bool isAuthenticating() const { return currentState == LoginState::AUTHENTICATING; }
    bool isReadyToAccept() const { return currentState == LoginState::READY_TO_ACCEPT; }
    bool isAccepted() const { return currentState == LoginState::ACCEPTED; }
};

} // namespace mccpp
