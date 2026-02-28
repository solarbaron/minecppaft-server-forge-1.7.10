/**
 * MinecraftServer.cpp — Core server lifecycle implementation.
 *
 * Reference: net.minecraft.server.MinecraftServer (MinecraftServer.java)
 * Implements initialization, the 20 TPS tick loop, and shutdown.
 *
 * The tick loop mirrors the Java version's timing:
 *   - 50ms per tick target
 *   - "Can't keep up!" warning when >2000ms behind
 *   - Tick skipping when significantly behind
 */

#include "server/MinecraftServer.h"
#include "block/Block.h"
#include "item/Item.h"
#include "crafting/Crafting.h"
#include "networking/Connection.h"
#include "networking/PacketHandler.h"
#include "networking/TcpListener.h"
#include "world/World.h"

#include <algorithm>
#include <iostream>

namespace mccpp {

MinecraftServer::MinecraftServer() = default;

MinecraftServer::~MinecraftServer() {
    stop();
}

bool MinecraftServer::init() {
    std::cout << "[Server] Initializing MineCPPaft server...\n";
    std::cout << "[Server] Game version: " << GAME_VERSION
              << " (Protocol " << PROTOCOL_VERSION << ")\n";

    // Create and configure the TCP listener
    listener_ = std::make_unique<TcpListener>(bindAddress_, port_);
    listener_->setAcceptCallback(
        [this](int fd, const std::string& addr, uint16_t port) {
            onClientAccepted(fd, addr, port);
        }
    );

    // Start listening
    if (!listener_->start()) {
        std::cerr << "[Server] Failed to start TCP listener on "
                  << bindAddress_ << ":" << port_ << "\n";
        return false;
    }

    std::cout << "[Server] Listening on " << bindAddress_ << ":" << port_ << "\n";
    std::cout << "[Server] MOTD: " << motd_ << "\n";
    std::cout << "[Server] Max players: " << maxPlayers_ << "\n";

    // Initialize registries (must happen before anything accesses blocks/items)
    // Java reference: Block.registerBlocks() called during Bootstrap.register()
    Block::registerBlocks();

    Item::registerItems();

    // Initialize crafting and smelting recipe registries
    // Java reference: CraftingManager.<init>(), FurnaceRecipes.<init>()
    CraftingManager::getInstance();
    FurnaceRecipes::instance();

    // Initialize worlds
    // Java reference: MinecraftServer.h() — creates WorldServer for each dimension
    auto overworld = std::make_unique<WorldServer>(0, "world");
    overworld->initialize();
    worlds_.push_back(std::move(overworld));

    return true;
}

void MinecraftServer::run() {
    running_.store(true, std::memory_order_release);

    std::cout << "[Server] Starting main tick loop (" << TICKS_PER_SECOND << " TPS)\n";
    std::cout << "[Server] Done! Ready for connections.\n";

    // Java reference: MinecraftServer.run() — main loop
    auto lastTick = Clock::now();
    int64_t behindMs = 0;

    while (running_.load(std::memory_order_relaxed)) {
        auto now = Clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick).count();

        // "Can't keep up!" detection — same logic as Java MinecraftServer.run()
        if (elapsed > 2000 && lastTick.time_since_epoch().count() > 0) {
            std::cerr << "[Server] Can't keep up! Running " << elapsed
                      << "ms behind, skipping " << (elapsed / MS_PER_TICK) << " tick(s)\n";
            elapsed = 2000;
        }

        if (elapsed < 0) {
            std::cerr << "[Server] Time ran backwards! Did the system time change?\n";
            elapsed = 0;
        }

        behindMs += elapsed;
        lastTick = now;

        // Process ticks
        while (behindMs >= MS_PER_TICK) {
            behindMs -= MS_PER_TICK;
            tick();
        }

        // Sleep until next tick
        auto sleepMs = std::max(static_cast<int64_t>(1), static_cast<int64_t>(MS_PER_TICK) - behindMs);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
    }

    // Shutdown
    std::cout << "[Server] Shutting down...\n";

    if (listener_) {
        listener_->stop();
    }

    // Close all connections
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            conn->disconnect("Server shutting down");
        }
        connections_.clear();
    }

    std::cout << "[Server] Server stopped.\n";
}

void MinecraftServer::stop() {
    running_.store(false, std::memory_order_release);
}

int MinecraftServer::getOnlinePlayerCount() const {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    return static_cast<int>(std::count_if(connections_.begin(), connections_.end(),
        [](const auto& c) { return c->getState() == ConnectionState::Play; }));
}

void MinecraftServer::addConnection(std::shared_ptr<Connection> conn) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    connections_.push_back(std::move(conn));
}

void MinecraftServer::removeConnection(Connection* conn) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    connections_.erase(
        std::remove_if(connections_.begin(), connections_.end(),
            [conn](const auto& c) { return c.get() == conn; }),
        connections_.end()
    );
}

void MinecraftServer::tick() {
    int ticks = tickCount_.fetch_add(1, std::memory_order_relaxed);

    // Java reference: MinecraftServer.u() — per-tick processing

    // Clean up dead connections
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        connections_.erase(
            std::remove_if(connections_.begin(), connections_.end(),
                [](const auto& c) { return !c->isConnected(); }),
            connections_.end()
        );
    }

    // Tick all worlds
    // Java reference: MinecraftServer.u() — tickWorlds
    for (auto& world : worlds_) {
        world->tick();
    }

    // Periodic status logging (every 6000 ticks = 5 minutes)
    if (ticks > 0 && ticks % 6000 == 0) {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        std::cout << "[Server] Tick " << ticks
                  << " | Connections: " << connections_.size() << "\n";
    }
}

void MinecraftServer::onClientAccepted(int fd, const std::string& address, uint16_t port) {
    auto conn = std::make_shared<Connection>(fd, address, port);
    auto handler = std::make_shared<HandshakeHandler>(*this);
    conn->start(handler);
    addConnection(conn);
}

} // namespace mccpp
