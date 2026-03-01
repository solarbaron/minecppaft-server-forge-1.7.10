/**
 * MinecraftServer.h — Core server class.
 *
 * Reference: net.minecraft.server.MinecraftServer (MinecraftServer.java, obfuscated)
 * This class manages the server lifecycle: initialization, the main tick loop,
 * player management, and shutdown.
 *
 * Multi-threaded adaptation: the tick loop runs on a dedicated thread, networking
 * is handled asynchronously, and world operations will be parallelized.
 */
#pragma once

#include "entity/EntityItem.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "command/CommandSystem.h"

namespace mccpp {

class TcpListener;   // forward decl
class Connection;    // forward decl
class WorldServer;   // forward decl
class PlayHandler;   // forward decl

/**
 * MinecraftServer — the central server object.
 *
 * Owns the TCP listener, manages connections, and runs the 20 TPS tick loop.
 *
 * Java reference: MinecraftServer.java fields:
 *   - s (hostname), t (port), v (running flag)
 *   - D (motd), x (tick counter)
 *   - 50ms tick interval (20 TPS)
 */
class MinecraftServer {
public:
    static constexpr int PROTOCOL_VERSION = 5;
    static constexpr const char* GAME_VERSION = "1.7.10";
    static constexpr int TICKS_PER_SECOND = 20;
    static constexpr int MS_PER_TICK = 1000 / TICKS_PER_SECOND; // 50ms

    MinecraftServer();
    ~MinecraftServer();

    // Non-copyable
    MinecraftServer(const MinecraftServer&) = delete;
    MinecraftServer& operator=(const MinecraftServer&) = delete;

    /**
     * Initialize the server: load properties, bind port, prepare worlds.
     * @return true if initialization succeeded
     */
    bool init();

    /**
     * Start the main tick loop (blocking — runs until stop() is called).
     * Java reference: MinecraftServer.run() — the main server loop.
     */
    void run();

    /**
     * Signal the server to stop. Thread-safe.
     * Java reference: MinecraftServer.r() — sets running = false.
     */
    void stop();

    /**
     * Check if the server is currently running.
     */
    bool isRunning() const { return running_.load(std::memory_order_relaxed); }

    // ─── Accessors ──────────────────────────────────────────────────────

    const std::string& getMotd() const { return motd_; }
    void setMotd(const std::string& motd) { motd_ = motd; }

    const std::string& getBindAddress() const { return bindAddress_; }
    void setBindAddress(const std::string& addr) { bindAddress_ = addr; }

    uint16_t getPort() const { return port_; }
    void setPort(uint16_t port) { port_ = port; }

    int getMaxPlayers() const { return maxPlayers_; }
    void setMaxPlayers(int max) { maxPlayers_ = max; }

    int getOnlinePlayerCount() const;

    int getTickCount() const { return tickCount_.load(std::memory_order_relaxed); }

    /**
     * Access worlds (e.g. for sending chunk data to players).
     */
    const std::vector<std::unique_ptr<WorldServer>>& getWorlds() const { return worlds_; }
    std::vector<std::unique_ptr<WorldServer>>& getWorlds() { return worlds_; }

    /**
     * Get the command handler for dispatching commands.
     * Java reference: MinecraftServer.getCommandManager()
     */
    CommandHandler& getCommandHandler() { return commandHandler_; }

    /**
     * Broadcast a chat message to all connected players.
     * Java reference: MinecraftServer.addChatMessage / PlayerList.sendChatMsg
     */
    void broadcastChatMessage(const std::string& message);

    /**
     * Broadcast a block change to all connected players.
     * Java reference: WorldServer.markBlockForUpdate()
     */
    void broadcastBlockChange(int32_t x, int32_t y, int32_t z, int32_t blockId, int32_t metadata);

    /**
     * Called when a player finishes login — broadcasts SpawnPlayer + PlayerListItem.
     * Java reference: ServerConfigurationManager.playerLoggedIn()
     */
    void onPlayerJoined(Connection& joinedConn, PlayHandler& joinedHandler);

    /**
     * Called when a player disconnects — broadcasts DestroyEntities + PlayerListItem(offline).
     * Java reference: ServerConfigurationManager.playerLoggedOut()
     */
    void onPlayerLeft(PlayHandler& leftHandler);

    /**
     * Broadcast a player's position/rotation to all other connected players.
     * Java reference: EntityTrackerEntry.sendLocationToAllClients()
     */
    void broadcastPlayerPosition(PlayHandler& movedHandler);

    /**
     * Broadcast a sound effect to all connected players.
     * Java reference: WorldServer.playSoundEffect()
     */
    void broadcastSound(const std::string& soundName, double x, double y, double z,
                        float volume, float pitch);

    /**
     * Save all world chunks to disk.\n     * Java reference: MinecraftServer.saveAllWorlds()\n     */
    void saveAllWorlds();

    /**
     * Spawn a dropped item entity at the given position.
     * Broadcasts S0E + S1C to all connected players.
     * Returns the entity ID of the spawned item.
     */
    int32_t spawnItemDrop(double x, double y, double z,
                          int32_t blockId, int32_t metadata, int32_t count = 1);

    /**
     * Tick all item entities (physics, despawn, pickup).
     * Called from the main server tick.
     */
    void tickItemEntities();

    /**
     * Register a new client connection (called from TcpListener callback).
     * Thread-safe.
     */
    void addConnection(std::shared_ptr<Connection> conn);

    /**
     * Remove a disconnected connection. Thread-safe.
     */
    void removeConnection(Connection* conn);

    /**
     * Broadcast S1A EntityStatus to all connected players.
     * Java reference: WorldServer.setEntityState()
     */
    void broadcastEntityEvent(int32_t entityId, int8_t status);

    /**
     * Handle a player attacking another entity (player-to-player combat).
     * Java reference: EntityPlayerMP.attackTargetEntityWithCurrentItem()
     */
    void handlePlayerAttack(PlayHandler& attacker, int32_t targetEntityId);

private:
    /**
     * Execute a single server tick.
     * Java reference: MinecraftServer.u() — tick() method.
     */
    void tick();

    /**
     * Called when a new client is accepted by the TCP listener.
     */
    void onClientAccepted(int fd, const std::string& address, uint16_t port);

    // ─── Server properties ──────────────────────────────────────────────
    std::string bindAddress_ = "0.0.0.0";
    uint16_t    port_        = 25565;
    std::string motd_        = "A MineCPPaft Server";
    int         maxPlayers_  = 20;
    bool        onlineMode_  = true;

    // ─── Runtime state ──────────────────────────────────────────────────
    std::atomic<bool> running_{false};
    std::atomic<int>  tickCount_{0};

    // ─── Networking ─────────────────────────────────────────────────────
    std::unique_ptr<TcpListener> listener_;

    mutable std::mutex connectionsMutex_;
    std::vector<std::shared_ptr<Connection>> connections_;

    // ─── Worlds ──────────────────────────────────────────────────────────
    std::vector<std::unique_ptr<WorldServer>> worlds_;

    // ─── Commands ────────────────────────────────────────────────────────
    CommandHandler commandHandler_;

    // ─── Timing (Java reference: MinecraftServer.run() tick timing) ─────
    using Clock = std::chrono::steady_clock;

    // ─── Item entities ──────────────────────────────────────────────────
    struct DroppedItem {
        EntityItem entity;
        int64_t spawnTick = 0;
    };
    mutable std::mutex itemEntitiesMutex_;
    std::vector<DroppedItem> itemEntities_;
    std::atomic<int32_t> nextItemEntityId_{100000};
};

} // namespace mccpp
