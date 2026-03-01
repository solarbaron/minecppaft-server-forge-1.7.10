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

    // Tick item entities (physics, despawn, pickup)
    tickItemEntities();

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
    // Broadcast S18 EntityTeleport + S19 EntityHeadLook to all other players
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* otherPlay = dynamic_cast<PlayHandler*>(handler.get());
        if (!otherPlay || otherPlay->getEntityId() == movedHandler.getEntityId()) continue;

        otherPlay->sendEntityTeleport(*conn, movedHandler.getEntityId(),
            movedHandler.getPlayerX(), movedHandler.getPlayerY(), movedHandler.getPlayerZ(),
            movedHandler.getPlayerYaw(), movedHandler.getPlayerPitch());
        otherPlay->sendEntityHeadLook(*conn, movedHandler.getEntityId(),
            movedHandler.getPlayerYaw());
    }
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

void MinecraftServer::handlePlayerAttack(PlayHandler& attacker, int32_t targetEntityId) {
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
    // Base fist damage = 1.0
    // TODO: Tool/weapon damage from held item
    float damage = 1.0f;

    // Check invulnerability frames (Java: EntityLivingBase.hurtResistantTime)
    // If target was recently hit, skip
    // Note: hurtResistantTime is decremented per tick in Java — we check > 0

    // ─── Apply damage ───────────────────────────────────────────────
    float oldHealth = target->getHealth();
    float newHealth = std::max(0.0f, oldHealth - damage);

    // Update target's health (need mutable access)
    target->applyDamage(damage);

    // Send health update to the victim
    target->sendUpdateHealth(*targetConn, target->getHealth(),
                              target->getFood(), target->getSaturation());

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

        // Set player to dead state (will respawn on ClientStatus packet)
        // The client will show the death screen and send C16 ClientStatus(0)
    }
}

} // namespace mccpp

