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

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace mccpp {

class TcpListener;   // forward decl
class Connection;    // forward decl
class WorldServer;   // forward decl

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
     * Register a new client connection (called from TcpListener callback).
     * Thread-safe.
     */
    void addConnection(std::shared_ptr<Connection> conn);

    /**
     * Remove a disconnected connection. Thread-safe.
     */
    void removeConnection(Connection* conn);

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

    // ─── Timing (Java reference: MinecraftServer.run() tick timing) ─────
    using Clock = std::chrono::steady_clock;
};

} // namespace mccpp
