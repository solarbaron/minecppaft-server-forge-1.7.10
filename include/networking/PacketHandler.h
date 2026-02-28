/**
 * PacketHandler.h — Abstract packet handler with state-specific dispatching.
 *
 * Java reference: net.minecraft.network.INetHandler
 * Each connection state has its own handler subclass:
 *   - HandshakeHandler  → net.minecraft.server.network.NetHandlerHandshakeTCP
 *   - StatusHandler     → net.minecraft.server.network.NetHandlerStatusServer
 *   - LoginHandler      → net.minecraft.server.network.NetHandlerLoginServer
 *
 * The Connection class calls handlePacket() with raw packet data;
 * implementations decode the packet ID and dispatch accordingly.
 */
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace mccpp {

class Connection;      // forward decl
class MinecraftServer; // forward decl

/**
 * PacketHandler — base class for protocol state handlers.
 *
 * Java reference: net.minecraft.network.INetHandler
 */
class PacketHandler : public std::enable_shared_from_this<PacketHandler> {
public:
    virtual ~PacketHandler() = default;

    /**
     * Called by the Connection's read loop when a complete packet is received.
     * @param packetId  The VarInt packet ID.
     * @param data      The raw packet payload (after the ID).
     * @param length    The length of the payload.
     * @param conn      The connection that received this packet.
     */
    virtual void handlePacket(int32_t packetId,
                              const uint8_t* data,
                              size_t length,
                              Connection& conn) = 0;

    /**
     * Called when the connection is closed or lost.
     * Java reference: INetHandler.onDisconnect()
     */
    virtual void onDisconnect(const std::string& reason) = 0;

    /**
     * Human-readable name for logging.
     */
    virtual std::string handlerName() const = 0;
};

/**
 * HandshakeHandler — handles the initial Handshake packet (0x00).
 *
 * Validates protocol version (must be exactly 5 for 1.7.10).
 * Transitions connection to Status or Login state, swapping the handler.
 *
 * Java reference: net.minecraft.server.network.NetHandlerHandshakeTCP
 */
class HandshakeHandler : public PacketHandler {
public:
    explicit HandshakeHandler(MinecraftServer& server);

    void handlePacket(int32_t packetId,
                      const uint8_t* data,
                      size_t length,
                      Connection& conn) override;

    void onDisconnect(const std::string& reason) override;
    std::string handlerName() const override { return "HandshakeHandler"; }

private:
    MinecraftServer& server_;
};

/**
 * StatusHandler — handles Status Request (0x00) and Ping (0x01).
 *
 * Java reference: net.minecraft.server.network.NetHandlerStatusServer
 */
class StatusHandler : public PacketHandler {
public:
    explicit StatusHandler(MinecraftServer& server);

    void handlePacket(int32_t packetId,
                      const uint8_t* data,
                      size_t length,
                      Connection& conn) override;

    void onDisconnect(const std::string& reason) override;
    std::string handlerName() const override { return "StatusHandler"; }

private:
    MinecraftServer& server_;
};

/**
 * LoginHandler — handles Login Start (0x00), Encryption Response (0x01).
 *
 * Java reference: net.minecraft.server.network.NetHandlerLoginServer
 *
 * Login state machine (from NetHandlerLoginServer$LoginState):
 *   HELLO → KEY → AUTHENTICATING → READY_TO_ACCEPT → ACCEPTED
 *
 * Currently implements offline-mode login only (HELLO → READY_TO_ACCEPT → ACCEPTED).
 * 600-tick (30 second) timeout matches Java: connectionTimer++ == 600.
 */
class LoginHandler : public PacketHandler {
public:
    explicit LoginHandler(MinecraftServer& server);

    void handlePacket(int32_t packetId,
                      const uint8_t* data,
                      size_t length,
                      Connection& conn) override;

    void onDisconnect(const std::string& reason) override;
    std::string handlerName() const override { return "LoginHandler"; }

private:
    /**
     * Generate an offline-mode UUID from a player name.
     * Java reference: NetHandlerLoginServer.getOfflineProfile()
     * UUID = nameUUIDFromBytes("OfflinePlayer:" + name)
     */
    static std::string generateOfflineUUID(const std::string& playerName);

    MinecraftServer& server_;
    std::string playerName_;
};

/**
 * PlayHandler — handles all Play-state packets.
 *
 * Java reference: net.minecraft.network.play.server.NetHandlerPlayServer
 *
 * Packet flow after login success:
 *   1. Send S01PacketJoinGame
 *   2. Send S05PacketSpawnPosition
 *   3. Send S39PacketPlayerAbilities
 *   4. Send S08PacketPlayerPosLook
 *   5. Send chunk data around spawn
 *   6. Handle client Keep Alive, Position, Chat, etc.
 */
class PlayHandler : public PacketHandler {
public:
    PlayHandler(MinecraftServer& server, const std::string& playerName,
                const std::string& uuid, Connection& conn);

    void handlePacket(int32_t packetId,
                      const uint8_t* data,
                      size_t length,
                      Connection& conn) override;

    void onDisconnect(const std::string& reason) override;
    std::string handlerName() const override { return "PlayHandler"; }

    /**
     * Send the initial login sequence: Join Game, Spawn Position, 
     * Player Abilities, Player Position And Look.
     * Java reference: NetHandlerPlayServer constructor + initializeConnectionToPlayer()
     */
    void sendLoginSequence(Connection& conn);

    /**
     * Send a Keep Alive packet to the client.
     * Java reference: NetHandlerPlayServer.update() — sends every 15 seconds
     */
    void sendKeepAlive(Connection& conn);

    /**
     * Send a chat message to the client.
     * Java reference: S02PacketChat
     */
    void sendChatMessage(Connection& conn, const std::string& message);

    const std::string& getPlayerName() const { return playerName_; }
    int getKeepAliveId() const { return lastKeepAliveId_; }

private:
    void handleKeepAlive(const uint8_t* data, size_t length, Connection& conn);
    void handleChatMessage(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerPosition(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerLook(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerPosAndLook(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerGround(const uint8_t* data, size_t length, Connection& conn);
    void handleClientSettings(const uint8_t* data, size_t length, Connection& conn);

    MinecraftServer& server_;
    std::string playerName_;
    std::string uuid_;

    // Keep Alive tracking
    // Java: NetHandlerPlayServer.field_147378_h (keepAlive ID)
    int lastKeepAliveId_ = 0;
    int ticksSinceLastKeepAlive_ = 0;

    // Player position (serverside)
    double playerX_ = 0.0, playerY_ = 0.0, playerZ_ = 0.0;
    float playerYaw_ = 0.0f, playerPitch_ = 0.0f;
    bool playerOnGround_ = false;
};

} // namespace mccpp
