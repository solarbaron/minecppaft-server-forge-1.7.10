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
#include "networking/PacketBuilder.h"
#include "networking/PacketHandler.h"
#include "networking/TcpListener.h"
#include "mechanics/FoodStats.h"
#include "world/World.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <random>
#include <set>
#include <tuple>

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

    // Initialize food values registry
    // Java reference: ItemFood constructed with healAmount/saturationModifier in Items.<clinit>()
    FoodValues::init();

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

    // Save all world data before stopping listener
    saveAllWorlds();

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

    // Send Keep Alive to play-state connections every 300 ticks (15 seconds)
    // Java reference: NetHandlerPlayServer.update() — sends S00PacketKeepAlive
    // every 15 seconds (300 ticks). Client must respond within 30s or gets kicked.
    if (ticks > 0 && ticks % 300 == 0) {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (conn->isConnected() && conn->getState() == ConnectionState::Play) {
                auto handler = conn->getHandler();
                auto* playHandler = dynamic_cast<PlayHandler*>(handler.get());
                if (playHandler) {
                    playHandler->sendKeepAlive(*conn);
                }
            }
        }
    }

    // Tick all worlds
    // Java reference: MinecraftServer.u() — tickWorlds
    for (auto& world : worlds_) {
        world->tick();
    }

    // ─── Weather update (every tick) ────────────────────────────────────
    // Java: WorldServer.updateWeather() → S2B ChangeGameState
    if (!worlds_.empty()) {
        auto changes = worlds_[0]->updateWeather();

        if (changes.rainStarted || changes.rainStopped ||
            changes.rainStrengthChanged || changes.thunderStrengthChanged) {
            std::lock_guard<std::mutex> lock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (!ph) continue;

                // Rain start/stop (S2B reason 2=begin rain, 1=end rain)
                if (changes.rainStarted) {
                    ph->sendChangeGameState(*conn, 2, 0.0f);
                }
                if (changes.rainStopped) {
                    ph->sendChangeGameState(*conn, 1, 0.0f);
                }
                // Rain/thunder intensity (S2B reason 7=rain, 8=thunder)
                if (changes.rainStrengthChanged) {
                    ph->sendChangeGameState(*conn, 7, changes.newRainStrength);
                }
                if (changes.thunderStrengthChanged) {
                    ph->sendChangeGameState(*conn, 8, changes.newThunderStrength);
                }
            }
        }
    }

    // Tick item entities (physics, despawn, pickup)
    tickItemEntities();
    tickFurnaces();

    // Tick world time — Java: WorldServer.tick()
    tickCounter_.fetch_add(1);
    if (!worlds_.empty()) {
        worlds_[0]->setWorldTime(worlds_[0]->getWorldTime() + 1);
    }
    // Send S03 TimeUpdate every 20 ticks (1 second)
    if (tickCounter_.load() % 20 == 0) {
        broadcastTimeUpdate();
    }

    // Tick mob entities (despawn tracking)
    tickMobs();

    // Natural mob spawning every 200 ticks (10 seconds)
    // Java reference: WorldServer.tick() → SpawnerAnimals.findChunksForSpawning()
    if (ticks > 0 && ticks % 200 == 0) {
        spawnNaturalMobs();
    }

    // Tick food/hunger for all play-state players
    // Java reference: EntityPlayer.onUpdate() → FoodStats.onUpdate()
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* play = dynamic_cast<PlayHandler*>(handler.get());
            if (play) {
                play->tickFood(*conn);
            }
        }
    }

    // Send S03 TimeUpdate to all players every 20 ticks (1 second)
    // Java reference: WorldServer.tick() sends S03PacketTimeUpdate every 20 ticks
    if (ticks > 0 && ticks % 20 == 0 && !worlds_.empty()) {
        WorldServer* world = worlds_[0].get();
        int64_t totalWorldTime = world->getTotalWorldTime();
        int64_t worldTime = world->getWorldTime();

        std::lock_guard<std::mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (conn->isConnected() && conn->getState() == ConnectionState::Play) {
                auto handler = conn->getHandler();
                auto* playHandler = dynamic_cast<PlayHandler*>(handler.get());
                if (playHandler) {
                    playHandler->sendTimeUpdate(*conn, totalWorldTime, worldTime);
                }
            }
        }
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

void MinecraftServer::broadcastChatMessage(const std::string& message) {
    // Java reference: PlayerList.sendChatMsg(IChatComponent)
    // Send to all players in Play state
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (conn->isConnected() && conn->getState() == ConnectionState::Play) {
            auto handler = conn->getHandler();
            auto* playHandler = dynamic_cast<PlayHandler*>(handler.get());
            if (playHandler) {
                playHandler->sendChatMessage(*conn, message);
            }
        }
    }
}

void MinecraftServer::broadcastBlockChange(int32_t x, int32_t y, int32_t z,
                                            int32_t blockId, int32_t metadata) {
    // Java reference: WorldServer.markBlockForUpdate() → S23PacketBlockChange
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (conn->isConnected() && conn->getState() == ConnectionState::Play) {
            auto handler = conn->getHandler();
            auto* playHandler = dynamic_cast<PlayHandler*>(handler.get());
            if (playHandler) {
                playHandler->sendBlockChange(*conn, x, y, z, blockId, metadata);
            }
        }
    }
}

void MinecraftServer::onPlayerJoined(Connection& joinedConn, PlayHandler& joinedHandler) {
    // Java reference: ServerConfigurationManager.playerLoggedIn()
    // 1. Send existing players to the new player (SpawnPlayer + PlayerListItem)
    // 2. Send the new player to all existing players

    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* otherPlay = dynamic_cast<PlayHandler*>(handler.get());
        if (!otherPlay) continue;

        // Skip self
        if (otherPlay->getEntityId() == joinedHandler.getEntityId()) continue;

        // Send existing player to the new player
        joinedHandler.sendSpawnPlayer(joinedConn,
            otherPlay->getEntityId(), otherPlay->getUuid(), otherPlay->getPlayerName(),
            otherPlay->getPlayerX(), otherPlay->getPlayerY(), otherPlay->getPlayerZ(),
            otherPlay->getPlayerYaw(), otherPlay->getPlayerPitch(), 0);
        joinedHandler.sendPlayerListItem(joinedConn, otherPlay->getPlayerName(), true, 0);

        // Send the new player to the existing player
        otherPlay->sendSpawnPlayer(*conn,
            joinedHandler.getEntityId(), joinedHandler.getUuid(), joinedHandler.getPlayerName(),
            joinedHandler.getPlayerX(), joinedHandler.getPlayerY(), joinedHandler.getPlayerZ(),
            joinedHandler.getPlayerYaw(), joinedHandler.getPlayerPitch(), 0);
        otherPlay->sendPlayerListItem(*conn, joinedHandler.getPlayerName(), true, 0);
    }

    // Send own PlayerListItem to self (so own name shows in tab list)
    joinedHandler.sendPlayerListItem(joinedConn, joinedHandler.getPlayerName(), true, 0);

    // Broadcast equipment for all players
    // Send existing players' equipment to the new player, and new player's to existing
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* otherPlay = dynamic_cast<PlayHandler*>(handler.get());
        if (!otherPlay) continue;
        if (otherPlay->getEntityId() == joinedHandler.getEntityId()) continue;

        // Send existing player's held item to the new player
        auto otherHeld = otherPlay->getHeldItem();
        joinedHandler.sendEntityEquipment(joinedConn, otherPlay->getEntityId(), 0, otherHeld);
        // Send armor slots 1-4
        for (int16_t s = 1; s <= 4; ++s) {
            auto armorItem = otherPlay->getArmorItem(s);
            joinedHandler.sendEntityEquipment(joinedConn, otherPlay->getEntityId(), s, armorItem);
        }

        // Send the new player's held item to existing players
        auto newHeld = joinedHandler.getHeldItem();
        otherPlay->sendEntityEquipment(*conn, joinedHandler.getEntityId(), 0, newHeld);
        for (int16_t s = 1; s <= 4; ++s) {
            auto armorItem = joinedHandler.getArmorItem(s);
            otherPlay->sendEntityEquipment(*conn, joinedHandler.getEntityId(), s, armorItem);
        }
    }

    // ─── Send current weather state to joining player ────────────────────
    // Java: EntityPlayerMP.onNewPotionEffect() / initializeConnectionToPlayer()
    if (!worlds_.empty()) {
        auto* world = worlds_[0].get();
        if (world->isRaining() && world->getRainingStrength() > 0.0f) {
            joinedHandler.sendChangeGameState(joinedConn, 2, 0.0f);  // Begin rain
            joinedHandler.sendChangeGameState(joinedConn, 7, world->getRainingStrength());
            joinedHandler.sendChangeGameState(joinedConn, 8, world->getThunderingStrength());
        }
    }
}

void MinecraftServer::onPlayerLeft(PlayHandler& leftHandler) {
    // Java reference: ServerConfigurationManager.playerLoggedOut()
    // Broadcast DestroyEntities + PlayerListItem(offline) to all remaining players
    std::vector<int32_t> destroyIds = { leftHandler.getEntityId() };

    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* otherPlay = dynamic_cast<PlayHandler*>(handler.get());
        if (!otherPlay || otherPlay->getEntityId() == leftHandler.getEntityId()) continue;

        otherPlay->sendDestroyEntities(*conn, destroyIds);
        otherPlay->sendPlayerListItem(*conn, leftHandler.getPlayerName(), false, 0);
    }
}

void MinecraftServer::broadcastPlayerPosition(PlayHandler& movedHandler) {
    // Java reference: EntityTrackerEntry.sendLocationToAllClients()
    // Uses S15/S16/S17 for small deltas, S18 for large deltas or forced sync

    // Compute fixed-point positions (value * 32)
    int32_t newPosX = static_cast<int32_t>(std::floor(movedHandler.getPlayerX() * 32.0));
    int32_t newPosY = static_cast<int32_t>(std::floor(movedHandler.getPlayerY() * 32.0));
    int32_t newPosZ = static_cast<int32_t>(std::floor(movedHandler.getPlayerZ() * 32.0));

    int32_t dx = newPosX - movedHandler.lastSentPosX_;
    int32_t dy = newPosY - movedHandler.lastSentPosY_;
    int32_t dz = newPosZ - movedHandler.lastSentPosZ_;

    bool posChanged = (dx != 0 || dy != 0 || dz != 0);
    bool rotChanged = (movedHandler.getPlayerYaw() != movedHandler.lastSentYaw_ ||
                       movedHandler.getPlayerPitch() != movedHandler.lastSentPitch_);

    // Force teleport if delta is too large for byte or every 400 ticks
    bool forceTeleport = (dx < -128 || dx > 127 || dy < -128 || dy > 127 || dz < -128 || dz > 127)
                         || (movedHandler.ticksSinceLastTeleport_++ >= 400);

    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* otherPlay = dynamic_cast<PlayHandler*>(handler.get());
        if (!otherPlay || otherPlay->getEntityId() == movedHandler.getEntityId()) continue;

        if (forceTeleport) {
            otherPlay->sendEntityTeleport(*conn, movedHandler.getEntityId(),
                movedHandler.getPlayerX(), movedHandler.getPlayerY(), movedHandler.getPlayerZ(),
                movedHandler.getPlayerYaw(), movedHandler.getPlayerPitch());
        } else if (posChanged && rotChanged) {
            otherPlay->sendEntityLookRelMove(*conn, movedHandler.getEntityId(),
                static_cast<int8_t>(dx), static_cast<int8_t>(dy), static_cast<int8_t>(dz),
                movedHandler.getPlayerYaw(), movedHandler.getPlayerPitch());
        } else if (posChanged) {
            otherPlay->sendEntityRelMove(*conn, movedHandler.getEntityId(),
                static_cast<int8_t>(dx), static_cast<int8_t>(dy), static_cast<int8_t>(dz));
        } else if (rotChanged) {
            otherPlay->sendEntityLook(*conn, movedHandler.getEntityId(),
                movedHandler.getPlayerYaw(), movedHandler.getPlayerPitch());
        }

        // Always send head look when rotation changes
        if (rotChanged || forceTeleport) {
            otherPlay->sendEntityHeadLook(*conn, movedHandler.getEntityId(),
                movedHandler.getPlayerYaw());
        }
    }

    // Update tracking state
    movedHandler.lastSentPosX_ = newPosX;
    movedHandler.lastSentPosY_ = newPosY;
    movedHandler.lastSentPosZ_ = newPosZ;
    movedHandler.lastSentYaw_ = movedHandler.getPlayerYaw();
    movedHandler.lastSentPitch_ = movedHandler.getPlayerPitch();
    if (forceTeleport) movedHandler.ticksSinceLastTeleport_ = 0;
}

void MinecraftServer::broadcastSound(const std::string& soundName, double x, double y, double z,
                                      float volume, float pitch) {
    // Java reference: WorldServer.playSoundEffect() → sends S29PacketSoundEffect to all players
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* play = dynamic_cast<PlayHandler*>(handler.get());
        if (!play) continue;

        play->sendSoundEffect(*conn, soundName, x, y, z, volume, pitch);
    }
}

int64_t MinecraftServer::getWorldTime() const {
    if (!worlds_.empty()) return worlds_[0]->getWorldTime();
    return 0;
}

void MinecraftServer::broadcastTimeUpdate() {
    int64_t age = getWorldAge();
    int64_t time = getWorldTime();
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* play = dynamic_cast<PlayHandler*>(handler.get());
        if (!play) continue;
        play->sendTimeUpdate(*conn, age, time);
    }
}

float MinecraftServer::getBlockExplosionResistance(int32_t blockId) {
    // Delegate to Block registry if block exists
    // Java reference: Block.getExplosionResistance() = resistance / 5.0f
    Block* block = Block::getBlockById(blockId);
    if (block) return block->getExplosionResistance();
    return 0.0f;
}

void MinecraftServer::createExplosion(double x, double y, double z, float power,
                                       bool causesFire, bool breakBlocks) {
    // Java reference: Explosion.doExplosionA() + doExplosionB()
    // Phase 1: Raycast to determine affected blocks
    std::set<int64_t> affectedBlocks; // packed positions
    std::vector<std::tuple<int32_t, int32_t, int32_t>> blockPositions;

    constexpr int RAYS = 16;
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int i = 0; i < RAYS; ++i) {
        for (int j = 0; j < RAYS; ++j) {
            for (int k = 0; k < RAYS; ++k) {
                // Only trace rays from the surface of the cube
                if (i != 0 && i != RAYS - 1 && j != 0 && j != RAYS - 1 &&
                    k != 0 && k != RAYS - 1) continue;

                double dx = static_cast<float>(i) / (RAYS - 1.0f) * 2.0f - 1.0f;
                double dy = static_cast<float>(j) / (RAYS - 1.0f) * 2.0f - 1.0f;
                double dz = static_cast<float>(k) / (RAYS - 1.0f) * 2.0f - 1.0f;
                double len = std::sqrt(dx * dx + dy * dy + dz * dz);
                dx /= len; dy /= len; dz /= len;

                double traceX = x, traceY = y, traceZ = z;
                float strength = power * (0.7f + dist(rng) * 0.6f);
                constexpr float STEP = 0.3f;

                while (strength > 0.0f) {
                    int32_t bx = static_cast<int32_t>(std::floor(traceX));
                    int32_t by = static_cast<int32_t>(std::floor(traceY));
                    int32_t bz = static_cast<int32_t>(std::floor(traceZ));

                    int32_t blockId = 0;
                    if (!worlds_.empty() && by >= 0 && by < 256) {
                        Block* block = worlds_[0]->getBlock(bx, by, bz);
                        blockId = block ? Block::getIdFromBlock(block) : 0;
                    }

                    if (blockId != 0) {
                        float resistance = getBlockExplosionResistance(blockId);
                        strength -= (resistance + 0.3f) * STEP;
                    }

                    if (strength > 0.0f && breakBlocks && by >= 0 && by < 256) {
                        int64_t key = packBlockPos(bx, by, bz);
                        if (affectedBlocks.find(key) == affectedBlocks.end()) {
                            affectedBlocks.insert(key);
                            blockPositions.emplace_back(bx, by, bz);
                        }
                    }

                    traceX += dx * STEP;
                    traceY += dy * STEP;
                    traceZ += dz * STEP;
                    strength -= STEP * 0.75f;
                }
            }
        }
    }

    // Phase 2: Entity damage and knockback
    double doubleRadius = power * 2.0;
    struct PlayerKnockback { double vx, vy, vz; };
    std::map<int32_t, PlayerKnockback> playerKnockbacks; // entityId → velocity

    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* play = dynamic_cast<PlayHandler*>(handler.get());
            if (!play || play->isDead()) continue;

            double px = play->getPlayerX();
            double py = play->getPlayerY();
            double pz = play->getPlayerZ();
            double dist2 = std::sqrt((px - x) * (px - x) + (py - y) * (py - y) + (pz - z) * (pz - z));
            double normalizedDist = dist2 / doubleRadius;

            if (normalizedDist > 1.0) continue;

            double diffX = px - x;
            double diffY = (py + 1.62) - y; // eye height
            double diffZ = pz - z;
            double diffLen = std::sqrt(diffX * diffX + diffY * diffY + diffZ * diffZ);
            if (diffLen == 0.0) continue;

            diffX /= diffLen; diffY /= diffLen; diffZ /= diffLen;

            // Java: (1.0 - normalizedDist) * blockDensity
            // Simplified: skip block density check, use 1.0
            double impact = (1.0 - normalizedDist);

            // Damage: ((impact² + impact) / 2) * 8 * power + 1
            float damage = static_cast<float>(
                (impact * impact + impact) / 2.0 * 8.0 * static_cast<double>(power) + 1.0);

            if (play->getGameMode() != 1) { // Not creative
                play->applyDamage(damage);
                play->sendUpdateHealth(*conn, play->getHealth(),
                    play->getFood(), play->getSaturation());
            }

            // Knockback velocity
            double knockback = impact;
            playerKnockbacks[play->getEntityId()] = {
                diffX * knockback, diffY * knockback, diffZ * knockback
            };

            // Send velocity packet
            play->sendEntityVelocity(*conn, play->getEntityId(),
                diffX * knockback, diffY * knockback, diffZ * knockback);
        }
    }

    // Phase 3: Destroy blocks and spawn drops
    if (breakBlocks) {
        for (auto& [bx, by, bz] : blockPositions) {
            if (worlds_.empty()) break;
            Block* block = worlds_[0]->getBlock(bx, by, bz);
            int32_t blockId = block ? Block::getIdFromBlock(block) : 0;
            if (blockId == 0) continue; // air

            // TNT chain reaction: ignite nearby TNT
            if (blockId == 46) {
                // Set air first, then create sub-explosion next tick
                worlds_[0]->setBlock(bx, by, bz, nullptr);
                broadcastBlockChange(bx, by, bz, 0, 0);
                // Spawn item drop for TNT (1/power chance)
                // In vanilla, TNT doesn't drop from explosions — it ignites
                // For simplicity, just destroy it
                continue;
            }

            // Drop items with 1/power chance
            if (dist(rng) < 1.0f / power) {
                // Get drop item for this block
                int32_t dropId = blockId; // simplified — use block ID as item
                // Use proper drop table if available
                switch (blockId) {
                    case 1: dropId = 4; break;   // Stone → cobblestone
                    case 14: dropId = 14; break;  // Gold ore
                    case 15: dropId = 15; break;  // Iron ore
                    case 16: dropId = 263; break;  // Coal ore → coal
                    case 21: dropId = 351; break;  // Lapis ore → lapis (simplified)
                    case 56: dropId = 264; break;  // Diamond ore → diamond
                    case 73: case 74: dropId = 331; break; // Redstone ore → redstone
                    case 129: dropId = 388; break; // Emerald ore → emerald
                    default: break;
                }
                if (dropId > 0 && dropId < 256) {
                    spawnItemDrop(
                        static_cast<double>(bx) + 0.5,
                        static_cast<double>(by) + 0.5,
                        static_cast<double>(bz) + 0.5,
                        dropId, 0, 1);
                }
            }

            // Set to air
            worlds_[0]->setBlock(bx, by, bz, nullptr);
            broadcastBlockChange(bx, by, bz, 0, 0);
        }
    }

    // Phase 4: Send S27 Explosion packet to all players
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* play = dynamic_cast<PlayHandler*>(handler.get());
            if (!play) continue;

            std::vector<uint8_t> pkt;
            // Helper lambdas
            auto writeVarInt = [](std::vector<uint8_t>& buf, int32_t value) {
                uint32_t uval = static_cast<uint32_t>(value);
                do {
                    uint8_t b = uval & 0x7F;
                    uval >>= 7;
                    if (uval != 0) b |= 0x80;
                    buf.push_back(b);
                } while (uval != 0);
            };
            auto writeFloat = [](std::vector<uint8_t>& buf, float value) {
                uint32_t bits;
                std::memcpy(&bits, &value, sizeof(bits));
                buf.push_back((bits >> 24) & 0xFF);
                buf.push_back((bits >> 16) & 0xFF);
                buf.push_back((bits >> 8) & 0xFF);
                buf.push_back(bits & 0xFF);
            };
            auto writeInt = [](std::vector<uint8_t>& buf, int32_t value) {
                buf.push_back((value >> 24) & 0xFF);
                buf.push_back((value >> 16) & 0xFF);
                buf.push_back((value >> 8) & 0xFF);
                buf.push_back(value & 0xFF);
            };

            writeVarInt(pkt, 0x27); // S27 Explosion
            writeFloat(pkt, static_cast<float>(x));
            writeFloat(pkt, static_cast<float>(y));
            writeFloat(pkt, static_cast<float>(z));
            writeFloat(pkt, power);

            // Record count
            int32_t recordCount = breakBlocks ? static_cast<int32_t>(blockPositions.size()) : 0;
            writeInt(pkt, recordCount);

            // Affected block offsets (relative to explosion center)
            int32_t centerX = static_cast<int32_t>(std::floor(x));
            int32_t centerY = static_cast<int32_t>(std::floor(y));
            int32_t centerZ = static_cast<int32_t>(std::floor(z));

            if (breakBlocks) {
                for (auto& [bx, by, bz] : blockPositions) {
                    pkt.push_back(static_cast<uint8_t>(static_cast<int8_t>(bx - centerX)));
                    pkt.push_back(static_cast<uint8_t>(static_cast<int8_t>(by - centerY)));
                    pkt.push_back(static_cast<uint8_t>(static_cast<int8_t>(bz - centerZ)));
                }
            }

            // Player motion (unique per player)
            auto it = playerKnockbacks.find(play->getEntityId());
            if (it != playerKnockbacks.end()) {
                writeFloat(pkt, static_cast<float>(it->second.vx));
                writeFloat(pkt, static_cast<float>(it->second.vy));
                writeFloat(pkt, static_cast<float>(it->second.vz));
            } else {
                writeFloat(pkt, 0.0f);
                writeFloat(pkt, 0.0f);
                writeFloat(pkt, 0.0f);
            }

            conn->sendPacket(std::move(pkt));
        }
    }

    // Phase 5: Play explosion sound
    broadcastSound("random.explode", x, y, z, 4.0f,
        (1.0f + (dist(rng) - dist(rng)) * 0.2f) * 0.7f);
}

void MinecraftServer::saveAllWorlds() {
    // Java reference: MinecraftServer.saveAllWorlds()
    std::cout << "[Server] Saving world data...\n";
    for (auto& world : worlds_) {
        if (world) {
            world->saveAllChunks();
        }
    }
    std::cout << "[Server] World data saved.\n";
}

int32_t MinecraftServer::spawnItemDrop(double x, double y, double z,
                                       int32_t blockId, int32_t metadata, int32_t count) {
    // Java reference: Block.dropBlockAsItem / EntityItem constructor
    if (blockId == 0) return -1;  // Don't drop air

    int32_t eid = nextItemEntityId_.fetch_add(1, std::memory_order_relaxed);

    EntityItem entity;
    entity.entityId = eid;
    entity.itemId = blockId;
    entity.itemMeta = static_cast<int32_t>(metadata);
    entity.stackSize = count;
    entity.spawn(x + 0.5, y + 0.5, z + 0.5);  // Center of block
    entity.delayBeforeCanPickup = 10;  // Java: 10 tick pickup delay

    // Broadcast spawn to all players
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (conn->isConnected() && conn->getState() == ConnectionState::Play) {
                auto handler = conn->getHandler();
                auto* playHandler = dynamic_cast<PlayHandler*>(handler.get());
                if (playHandler) {
                    // S0E SpawnObject: type=2 (Item), data=1 (needed for velocity)
                    playHandler->sendSpawnObject(*conn, eid, 2,
                        entity.posX, entity.posY, entity.posZ,
                        entity.rotationYaw, 0.0f, 1,
                        entity.motionX, entity.motionY, entity.motionZ);
                    // S1C EntityMetadata with ItemStack at slot 10
                    playHandler->sendEntityMetadataItem(*conn, eid,
                        static_cast<int16_t>(blockId),
                        static_cast<int8_t>(count),
                        static_cast<int16_t>(metadata));
                }
            }
        }
    }

    // Store in tracked entities
    {
        std::lock_guard<std::mutex> lock(itemEntitiesMutex_);
        DroppedItem di;
        di.entity = entity;
        di.spawnTick = tickCount_.load(std::memory_order_relaxed);
        itemEntities_.push_back(std::move(di));
    }

    return eid;
}

void MinecraftServer::tickItemEntities() {
    // Tick all tracked item entities
    std::lock_guard<std::mutex> lock(itemEntitiesMutex_);

    std::vector<int32_t> deadEntityIds;

    for (auto& di : itemEntities_) {
        auto& e = di.entity;
        if (e.isDead) continue;

        // Simple physics tick (ground slipperiness = 0.6 default)
        auto result = e.onUpdate(0.6f, false);

        // Apply simple gravity movement (no full collision)
        e.posX += e.motionX;
        e.posY += e.motionY;
        e.posZ += e.motionZ;

        // Simple ground check: don't go below Y=0
        if (e.posY < 0.5) {
            e.posY = 0.5;
            e.onGround = true;
            e.motionY = 0;
        }

        if (result.shouldDie) {
            e.isDead = true;
            deadEntityIds.push_back(e.entityId);
            continue;
        }

        // Check for player pickup
        if (e.delayBeforeCanPickup == 0) {
            std::lock_guard<std::mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (!ph) continue;

                // Check distance to player (1.0 block radius)
                double dx = ph->getPlayerX() - e.posX;
                double dy = ph->getPlayerY() + 0.5 - e.posY;
                double dz = ph->getPlayerZ() - e.posZ;
                double distSq = dx * dx + dy * dy + dz * dz;

                if (distSq < 1.0) {
                    // Try to add item to player inventory
                    if (!ph->tryPickupItem(*conn, e.itemId, e.itemMeta, e.stackSize)) {
                        continue; // Inventory full — skip pickup
                    }

                    // Pickup! broadcast collect animation
                    for (auto& c2 : connections_) {
                        if (!c2->isConnected() || c2->getState() != ConnectionState::Play) continue;
                        auto h2 = c2->getHandler();
                        auto* ph2 = dynamic_cast<PlayHandler*>(h2.get());
                        if (ph2) {
                            ph2->sendCollectItem(*c2, e.entityId, ph->getEntityId());
                        }
                    }

                    // Play pickup sound
                    broadcastSound("random.pop", e.posX, e.posY, e.posZ, 0.2f, 1.0f);

                    e.isDead = true;
                    deadEntityIds.push_back(e.entityId);
                    break;
                }
            }
        }
    }

    // Broadcast destroy for dead entities
    if (!deadEntityIds.empty()) {
        std::lock_guard<std::mutex> connLock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (ph) {
                ph->sendDestroyEntities(*conn, deadEntityIds);
            }
        }
    }

    // Remove dead entities from tracking
    itemEntities_.erase(
        std::remove_if(itemEntities_.begin(), itemEntities_.end(),
            [](const DroppedItem& di) { return di.entity.isDead; }),
        itemEntities_.end()
    );
}

// ═══════════════════════════════════════════════════════════════════════════
// Combat system
// Java reference: EntityPlayer.attackTargetEntityWithCurrentItem()
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::broadcastEntityEvent(int32_t entityId, int8_t status) {
    // Java reference: WorldServer.setEntityState() → S1APacketEntityStatus
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* play = dynamic_cast<PlayHandler*>(handler.get());
        if (play) {
            play->sendEntityStatus(*conn, entityId, status);
        }
    }
}

void MinecraftServer::broadcastAnimation(int32_t entityId, uint8_t animationType) {
    // Java reference: WorldServer entity.worldObj.setEntityState() for animation
    // Broadcasts S0B Animation to all players except the source entity
    auto pkt = PacketBuilder::animation(entityId, animationType);
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* play = dynamic_cast<PlayHandler*>(handler.get());
        if (!play || play->getEntityId() == entityId) continue;
        conn->sendPacket(pkt);
    }
}

void MinecraftServer::broadcastEntityMetadataFlags(int32_t entityId, uint8_t flags) {
    // Java reference: EntityTrackerEntry.func_151261_b() → S1CPacketEntityMetadata
    // Broadcasts entity metadata flags (sneaking, sprinting) to all players except the source
    auto pkt = PacketBuilder::entityMetadataFlags(entityId, flags);
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* play = dynamic_cast<PlayHandler*>(handler.get());
        if (!play || play->getEntityId() == entityId) continue;
        conn->sendPacket(pkt);
    }
}

void MinecraftServer::broadcastEquipment(PlayHandler& handler, int16_t equipSlot) {
    // Java reference: EntityTrackerEntry.func_151261_b() → S04PacketEntityEquipment
    // Broadcast a single equipment slot change to all other players
    int32_t entityId = handler.getEntityId();
    std::optional<ItemStack> item;
    if (equipSlot == 0) {
        item = handler.getHeldItem();
    } else {
        item = handler.getArmorItem(equipSlot);
    }

    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto h = conn->getHandler();
        auto* play = dynamic_cast<PlayHandler*>(h.get());
        if (!play || play->getEntityId() == entityId) continue;
        play->sendEntityEquipment(*conn, entityId, equipSlot, item);
    }
}

void MinecraftServer::handlePlayerAttack(PlayHandler& attacker, Connection& attackerConn, int32_t targetEntityId) {
    // Java reference: EntityPlayer.attackTargetEntityWithCurrentItem(Entity)
    // Find target player
    PlayHandler* target = nullptr;
    Connection* targetConn = nullptr;

    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* play = dynamic_cast<PlayHandler*>(handler.get());
            if (play && play->getEntityId() == targetEntityId) {
                target = play;
                targetConn = conn.get();
                break;
            }
        }
    }

    if (!target || !targetConn || target->isDead()) return;

    // ─── Damage calculation ─────────────────────────────────────────
    // Java: EntityPlayer.attackTargetEntityWithCurrentItem()
    // Damage from held weapon via attribute modifier system
    float damage = attacker.getWeaponDamage();

    // ─── Armor reduction ────────────────────────────────────────────
    // Java: EntityLivingBase.applyArmorCalculations()
    // Formula: damage = damage * (25 - totalArmorValue) / 25.0f
    int32_t armorValue = target->getTotalArmorValue();
    if (armorValue > 0) {
        float reduced = damage * static_cast<float>(25 - armorValue) / 25.0f;
        damage = reduced;
    }

    // Skip if no damage
    if (damage <= 0.0f) return;

    // ─── Apply damage ───────────────────────────────────────────────
    target->applyDamage(damage);

    // Send health update to the victim
    target->sendUpdateHealth(*targetConn, target->getHealth(),
                              target->getFood(), target->getSaturation());

    // ─── Durability ─────────────────────────────────────────────────
    // Java: ItemStack.damageItem(1) on attacker's weapon per hit
    float rawDamage = attacker.getWeaponDamage();
    attacker.damageHeldItem(1);
    // Java: InventoryPlayer.damageArmor(rawDamage) on victim
    target->damageArmor(rawDamage);

    // Java: EntityPlayer.addExhaustion(0.3f) on attack
    attacker.getFoodStats().addExhaustion(0.3f);

    // ─── Hurt animation (S1A EntityStatus, status=2) ────────────────
    // Java: EntityLivingBase.attackEntityFrom → setEntityState(entity, 2)
    broadcastEntityEvent(targetEntityId, 2);

    // ─── Knockback ──────────────────────────────────────────────────
    // Java: EntityLivingBase.knockBack(entity, damage, dx, dz)
    // direction: from attacker to target
    double dx = target->getPlayerX() - attacker.getPlayerX();
    double dz = target->getPlayerZ() - attacker.getPlayerZ();
    double dist = std::sqrt(dx * dx + dz * dz);

    if (dist > 0.0) {
        dx /= dist;
        dz /= dist;
    } else {
        dx = 0.0;
        dz = 0.0;
    }

    // Java knockback formula: motionX/2 + dx*0.4, motionY/2 + 0.4, motionZ/2 + dz*0.4
    // then clamp motionY to 0.4 max
    double kbX = dx * 0.4;
    double kbY = 0.4;
    double kbZ = dz * 0.4;

    // Send S12 EntityVelocity to the target
    target->sendEntityVelocity(*targetConn, targetEntityId, kbX, kbY, kbZ);

    // ─── Damage sound ───────────────────────────────────────────────
    broadcastSound("game.player.hurt",
        target->getPlayerX(), target->getPlayerY(), target->getPlayerZ(),
        1.0f, 1.0f);

    // ─── Death check ────────────────────────────────────────────────
    if (target->getHealth() <= 0.0f) {
        // Death! Java: EntityLivingBase.onDeath()
        // Status 3 = entity death animation
        broadcastEntityEvent(targetEntityId, 3);

        // Broadcast death message
        // Java: DamageSource.getDeathMessage() → "death.attack.player"
        std::string deathMsg = "{\"text\":\"" + target->getPlayerName() +
            " was slain by " + attacker.getPlayerName() + "\"}";
        broadcastChatMessage(deathMsg);

        // Java: EntityPlayer.getExperiencePoints() — drops level*7 XP (max 100)
        int32_t killXp = std::min(target->getExperienceLevel() * 7, 100);
        if (killXp > 0) {
            attacker.grantExperience(killXp);
            attacker.sendExperienceUpdate(attackerConn);
        }

        // Reset victim's XP on death
        target->resetExperience();

        // Set player to dead state (will respawn on ClientStatus packet)
        // The client will show the death screen and send C16 ClientStatus(0)
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Command helper methods
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::teleportPlayer(const std::string& playerName, double x, double y, double z) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (!ph || ph->getPlayerName() != playerName) continue;

        ph->setPlayerPosition(x, y, z);
        ph->sendPlayerPosAndLook(*conn, x, y, z, ph->getPlayerYaw(), ph->getPlayerPitch());
        // Broadcast to others
        for (auto& oc : connections_) {
            if (oc.get() == conn.get()) continue;
            if (!oc->isConnected() || oc->getState() != ConnectionState::Play) continue;
            auto oh = oc->getHandler();
            auto* op = dynamic_cast<PlayHandler*>(oh.get());
            if (op) op->sendEntityTeleport(*oc, ph->getEntityId(), x, y, z, ph->getPlayerYaw(), ph->getPlayerPitch());
        }
        return;
    }
}

void MinecraftServer::setWorldTime(int64_t time) {
    if (!worlds_.empty()) worlds_[0]->setWorldTime(time);
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (ph) {
            int64_t total = worlds_.empty() ? 0 : worlds_[0]->getTotalWorldTime();
            ph->sendTimeUpdate(*conn, total, time);
        }
    }
}

void MinecraftServer::addWorldTime(int64_t amount) {
    int64_t newTime = 0;
    if (!worlds_.empty()) {
        worlds_[0]->setWorldTime(worlds_[0]->getWorldTime() + amount);
        newTime = worlds_[0]->getWorldTime();
    }
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (ph) {
            int64_t total = worlds_.empty() ? 0 : worlds_[0]->getTotalWorldTime();
            ph->sendTimeUpdate(*conn, total, newTime);
        }
    }
}

void MinecraftServer::killPlayer(const std::string& playerName) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (!ph || ph->getPlayerName() != playerName || ph->isDead()) continue;
        ph->applyDamage(ph->getHealth());
        ph->sendUpdateHealth(*conn, 0.0f, ph->getFood(), ph->getSaturation());
        broadcastEntityEvent(ph->getEntityId(), 3);
        broadcastChatMessage(playerName + " was killed");
        return;
    }
}

void MinecraftServer::setPlayerGameMode(const std::string& playerName, int32_t gameMode) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (!ph || ph->getPlayerName() != playerName) continue;
        ph->setGameMode(gameMode);
        ph->sendChangeGameState(*conn, 3, static_cast<float>(gameMode));
        return;
    }
}

std::vector<std::string> MinecraftServer::getOnlinePlayerNames() const {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    std::vector<std::string> names;
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (ph) names.push_back(ph->getPlayerName());
    }
    return names;
}

std::optional<PlayerPosition> MinecraftServer::getPlayerPosition(const std::string& playerName) const {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (ph && ph->getPlayerName() == playerName) {
            return PlayerPosition{ph->getPlayerX(), ph->getPlayerY(), ph->getPlayerZ()};
        }
    }
    return std::nullopt;
}

void MinecraftServer::givePlayerItem(const std::string& playerName, int32_t itemId, int32_t amount, int32_t damage) {
    // Give item by sending S2F SetSlot to the first empty hotbar/inventory slot
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (!ph || ph->getPlayerName() != playerName) continue;

        // Add to first hotbar slot
        ItemStack stack(itemId, amount, damage);
        ph->sendSetSlot(*conn, 0, 36, stack);
        return;
    }
}

void MinecraftServer::setWeather(int32_t mode, int32_t durationTicks) {
    // Java: CommandWeather.processCommand()
    // mode: 0=clear, 1=rain, 2=thunder
    if (worlds_.empty()) return;

    auto* world = worlds_[0].get();

    // Default duration: random 6000-18000 ticks (5-15 min)
    if (durationTicks <= 0) durationTicks = 6000 + std::rand() % 12000;

    switch (mode) {
        case 0: // clear
            world->setWeatherState(false, false, durationTicks, durationTicks);
            break;
        case 1: // rain
            world->setWeatherState(true, false, durationTicks, durationTicks);
            break;
        case 2: // thunder
            world->setWeatherState(true, true, durationTicks, durationTicks);
            break;
    }

    // Broadcast immediately to all players
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (!ph) continue;

            if (mode == 0) {
                ph->sendChangeGameState(*conn, 1, 0.0f);  // End rain
            } else {
                ph->sendChangeGameState(*conn, 2, 0.0f);  // Begin rain
            }
            ph->sendChangeGameState(*conn, 7, world->getRainingStrength());
            ph->sendChangeGameState(*conn, 8, world->getThunderingStrength());
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Mob spawning system

void MinecraftServer::spawnNaturalMobs() {
    // Don't spawn if no players
    int playerCount = getOnlinePlayerCount();
    if (playerCount == 0) return;
    if (worlds_.empty()) return;

    // Check mob cap
    {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        int aliveCount = 0;
        for (auto& mob : mobEntities_) {
            if (!mob.isDead) aliveCount++;
        }
        if (aliveCount >= MAX_HOSTILE_MOBS) return;
    }

    // Pick a random player to spawn near
    static thread_local std::mt19937 rng(std::random_device{}());
    PlayHandler* targetPlayer = nullptr;
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        std::vector<PlayHandler*> players;
        for (auto& conn : connections_) {
            if (conn->isConnected() && conn->getState() == ConnectionState::Play) {
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (ph && !ph->isDead()) players.push_back(ph);
            }
        }
        if (players.empty()) return;
        targetPlayer = players[rng() % players.size()];
    }

    // Generate spawn position 8-24 blocks from the chosen player
    // Java reference: SpawnerAnimals — random offset from chunk center
    std::uniform_int_distribution<int> offsetDist(-24, 24);
    double spawnX = targetPlayer->getPlayerX() + offsetDist(rng);
    double spawnZ = targetPlayer->getPlayerZ() + offsetDist(rng);

    // Keep distance reasonable (not too close)
    double dx = spawnX - targetPlayer->getPlayerX();
    double dz = spawnZ - targetPlayer->getPlayerZ();
    if (dx * dx + dz * dz < 64.0) return; // Min 8 blocks away

    // Find surface Y at spawn position
    WorldServer* world = worlds_[0].get();
    int bx = static_cast<int>(std::floor(spawnX));
    int bz = static_cast<int>(std::floor(spawnZ));

    // Find highest non-air block
    int surfaceY = 64; // default
    for (int y = 255; y > 0; --y) {
        Block* block = world->getBlock(bx, y, bz);
        if (block != nullptr) {
            surfaceY = y + 1;
            break;
        }
    }

    // Don't spawn in void or too high
    if (surfaceY <= 1 || surfaceY > 250) return;

    double spawnY = static_cast<double>(surfaceY);

    // Pick mob type  — Java entity type IDs:
    // 50=Creeper, 51=Skeleton, 52=Spider, 54=Zombie
    static const uint8_t hostileMobs[] = {54, 54, 54, 54, 51, 51, 51, 52, 52, 50};
    uint8_t mobType = hostileMobs[rng() % 10];

    // Random facing
    std::uniform_real_distribution<float> yawDist(0.0f, 360.0f);
    float yaw = yawDist(rng);

    // Create mob
    int32_t eid = nextMobEntityId_.fetch_add(1, std::memory_order_relaxed);
    int64_t currentTick = tickCount_.load(std::memory_order_relaxed);

    SpawnedMob mob;
    mob.entityId = eid;
    mob.mobType = mobType;
    mob.posX = spawnX;
    mob.posY = spawnY;
    mob.posZ = spawnZ;
    mob.yaw = yaw;
    mob.pitch = 0.0f;
    mob.health = 20.0f;
    mob.spawnTick = currentTick;
    mob.isDead = false;

    // Broadcast S0F SpawnMob to all players
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (ph) {
                ph->sendSpawnMob(*conn, eid, mobType, spawnX, spawnY, spawnZ,
                                  yaw, 0.0f, yaw);
            }
        }
    }

    // Track
    {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        mobEntities_.push_back(std::move(mob));
    }
}

void MinecraftServer::tickMobs() {
    // Despawn mobs that are old AND far from all players
    // Java reference: EntityLiving.despawnEntity() — >600 ticks + >32 blocks
    int64_t currentTick = tickCount_.load(std::memory_order_relaxed);
    std::vector<int32_t> deadIds;

    {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        for (auto& mob : mobEntities_) {
            if (mob.isDead) continue;

            int64_t age = currentTick - mob.spawnTick;
            if (age < 600) continue;

            // Check distance to nearest player
            bool nearPlayer = false;
            {
                std::lock_guard<std::mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph) continue;
                    double dx = ph->getPlayerX() - mob.posX;
                    double dz = ph->getPlayerZ() - mob.posZ;
                    if (dx * dx + dz * dz < 1024.0) { // 32 blocks
                        nearPlayer = true;
                        break;
                    }
                }
            }

            if (!nearPlayer) {
                mob.isDead = true;
                deadIds.push_back(mob.entityId);
            }
        }

        // Remove dead
        mobEntities_.erase(
            std::remove_if(mobEntities_.begin(), mobEntities_.end(),
                [](const SpawnedMob& m) { return m.isDead; }),
            mobEntities_.end()
        );
    }

    // Broadcast destroy for despawned mobs
    if (!deadIds.empty()) {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (ph) {
                ph->sendDestroyEntities(*conn, deadIds);
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Chest tile entity storage
// Java reference: TileEntityChest — simplified to in-memory per-position storage
// ═══════════════════════════════════════════════════════════════════════════

std::array<std::optional<ItemStack>, 27>& MinecraftServer::getOrCreateChest(
    int32_t x, int32_t y, int32_t z) {
    std::lock_guard<std::mutex> lock(chestMutex_);
    int64_t key = packBlockPos(x, y, z);
    auto& chest = chestStorage_[key];
    return chest;
}

// ═══════════════════════════════════════════════════════════════════════════
// Furnace tile entity storage and ticking
// Java reference: TileEntityFurnace.updateEntity()
// ═══════════════════════════════════════════════════════════════════════════

MinecraftServer::FurnaceData& MinecraftServer::getOrCreateFurnace(
    int32_t x, int32_t y, int32_t z) {
    std::lock_guard<std::mutex> lock(furnaceMutex_);
    int64_t key = packBlockPos(x, y, z);
    return furnaceStorage_[key];
}

void MinecraftServer::tickFurnaces() {
    std::lock_guard<std::mutex> lock(furnaceMutex_);
    for (auto& [key, furnace] : furnaceStorage_) {
        bool dirty = furnace.tick();
        if (dirty) {
            // Send updates to any player who has this furnace open
            std::lock_guard<std::mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (!ph) continue;
                if (ph->getOpenWindowType() == 2 && ph->getOpenFurnaceKey() == key) {
                    // Send S31 WindowProperty for progress bars
                    ph->sendWindowProperty(*conn, ph->getOpenWindowId(), 0, furnace.furnaceCookTime);
                    ph->sendWindowProperty(*conn, ph->getOpenWindowId(), 1, furnace.furnaceBurnTime);
                    ph->sendWindowProperty(*conn, ph->getOpenWindowId(), 2, furnace.currentItemBurnTime);
                    // Send slot contents
                    for (int i = 0; i < 3; ++i) {
                        ph->sendSetSlot(*conn, ph->getOpenWindowId(), static_cast<int16_t>(i), furnace.slots[i]);
                    }
                }
            }
        }
    }
}

} // namespace mccpp
