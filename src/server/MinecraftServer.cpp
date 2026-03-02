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
    tickHoppers();

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
                play->tickPotionEffects(*conn);
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

    // Random block ticks — crop growth, sapling growth, farmland hydration, grass spread
    // Java reference: WorldServer.func_147456_g() — applies random ticks per chunk section
    // We run this every tick but the method internally controls tick rate
    tickRandomBlocks();

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
    Block* block = Block::getBlockById(blockId);
    if (block) return block->getExplosionResistance();
    return 0.0f;
}

void MinecraftServer::setSignText(int32_t x, int32_t y, int32_t z,
                                   const std::string& l1, const std::string& l2,
                                   const std::string& l3, const std::string& l4) {
    // Store sign text
    int64_t key = packBlockPos(x, y, z);
    {
        std::lock_guard<std::mutex> lock(signsMutex_);
        signs_[key] = {{l1, l2, l3, l4}};
    }

    // Broadcast S33 UpdateSign to all connected players
    // Helper lambdas for packet building
    auto writeVarInt = [](std::vector<uint8_t>& buf, int32_t value) {
        uint32_t uval = static_cast<uint32_t>(value);
        do {
            uint8_t b = uval & 0x7F;
            uval >>= 7;
            if (uval != 0) b |= 0x80;
            buf.push_back(b);
        } while (uval != 0);
    };
    auto writeString = [&writeVarInt](std::vector<uint8_t>& buf, const std::string& s) {
        writeVarInt(buf, static_cast<int32_t>(s.size()));
        buf.insert(buf.end(), s.begin(), s.end());
    };
    auto writeInt = [](std::vector<uint8_t>& buf, int32_t value) {
        buf.push_back((value >> 24) & 0xFF);
        buf.push_back((value >> 16) & 0xFF);
        buf.push_back((value >> 8) & 0xFF);
        buf.push_back(value & 0xFF);
    };
    auto writeShort = [](std::vector<uint8_t>& buf, int16_t value) {
        buf.push_back((value >> 8) & 0xFF);
        buf.push_back(value & 0xFF);
    };

    std::vector<uint8_t> pkt;
    writeVarInt(pkt, 0x33); // S33 UpdateSign
    writeInt(pkt, x);
    writeShort(pkt, static_cast<int16_t>(y));
    writeInt(pkt, z);
    writeString(pkt, l1);
    writeString(pkt, l2);
    writeString(pkt, l3);
    writeString(pkt, l4);

    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto copy = pkt; // copy for each connection
        conn->sendPacket(std::move(copy));
    }
}

void MinecraftServer::sendSignToPlayer(PlayHandler& /*handler*/, Connection& conn,
                                        int32_t x, int32_t y, int32_t z) {
    int64_t key = packBlockPos(x, y, z);
    SignData data;
    {
        std::lock_guard<std::mutex> lock(signsMutex_);
        auto it = signs_.find(key);
        if (it == signs_.end()) return;
        data = it->second;
    }

    auto writeVarInt = [](std::vector<uint8_t>& buf, int32_t value) {
        uint32_t uval = static_cast<uint32_t>(value);
        do {
            uint8_t b = uval & 0x7F;
            uval >>= 7;
            if (uval != 0) b |= 0x80;
            buf.push_back(b);
        } while (uval != 0);
    };
    auto writeString = [&writeVarInt](std::vector<uint8_t>& buf, const std::string& s) {
        writeVarInt(buf, static_cast<int32_t>(s.size()));
        buf.insert(buf.end(), s.begin(), s.end());
    };
    auto writeInt = [](std::vector<uint8_t>& buf, int32_t value) {
        buf.push_back((value >> 24) & 0xFF);
        buf.push_back((value >> 16) & 0xFF);
        buf.push_back((value >> 8) & 0xFF);
        buf.push_back(value & 0xFF);
    };
    auto writeShort = [](std::vector<uint8_t>& buf, int16_t value) {
        buf.push_back((value >> 8) & 0xFF);
        buf.push_back(value & 0xFF);
    };

    std::vector<uint8_t> pkt;
    writeVarInt(pkt, 0x33);
    writeInt(pkt, x);
    writeShort(pkt, static_cast<int16_t>(y));
    writeInt(pkt, z);
    writeString(pkt, data.lines[0]);
    writeString(pkt, data.lines[1]);
    writeString(pkt, data.lines[2]);
    writeString(pkt, data.lines[3]);
    conn.sendPacket(std::move(pkt));
}

void MinecraftServer::broadcastParticle(const std::string& particleName,
                                         float x, float y, float z,
                                         float offsetX, float offsetY, float offsetZ,
                                         float speed, int32_t count) {
    // Java reference: WorldServer.spawnParticle() → S2A Particle
    auto writeVarInt = [](std::vector<uint8_t>& buf, int32_t value) {
        uint32_t uval = static_cast<uint32_t>(value);
        do {
            uint8_t b = uval & 0x7F;
            uval >>= 7;
            if (uval != 0) b |= 0x80;
            buf.push_back(b);
        } while (uval != 0);
    };
    auto writeString = [&writeVarInt](std::vector<uint8_t>& buf, const std::string& s) {
        writeVarInt(buf, static_cast<int32_t>(s.size()));
        buf.insert(buf.end(), s.begin(), s.end());
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

    std::vector<uint8_t> pkt;
    writeVarInt(pkt, 0x2A); // S2A Particle
    writeString(pkt, particleName);
    writeFloat(pkt, x);
    writeFloat(pkt, y);
    writeFloat(pkt, z);
    writeFloat(pkt, offsetX);
    writeFloat(pkt, offsetY);
    writeFloat(pkt, offsetZ);
    writeFloat(pkt, speed);
    writeInt(pkt, count);

    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto copy = pkt;
        conn->sendPacket(std::move(copy));
    }
}

void MinecraftServer::broadcastEffect(int32_t effectId, int32_t x, int32_t y, int32_t z,
                                       int32_t data, bool disableRelativeVolume) {
    // Java reference: WorldManager.playAuxSFX() → S28 Effect
    auto writeVarInt = [](std::vector<uint8_t>& buf, int32_t value) {
        uint32_t uval = static_cast<uint32_t>(value);
        do {
            uint8_t b = uval & 0x7F;
            uval >>= 7;
            if (uval != 0) b |= 0x80;
            buf.push_back(b);
        } while (uval != 0);
    };
    auto writeInt = [](std::vector<uint8_t>& buf, int32_t value) {
        buf.push_back((value >> 24) & 0xFF);
        buf.push_back((value >> 16) & 0xFF);
        buf.push_back((value >> 8) & 0xFF);
        buf.push_back(value & 0xFF);
    };

    std::vector<uint8_t> pkt;
    writeVarInt(pkt, 0x28); // S28 Effect
    writeInt(pkt, effectId);
    writeInt(pkt, x);
    pkt.push_back(static_cast<uint8_t>(y));
    writeInt(pkt, z);
    writeInt(pkt, data);
    pkt.push_back(disableRelativeVolume ? 1 : 0);

    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto copy = pkt;
        conn->sendPacket(std::move(copy));
    }
}

MinecraftServer::BrewingStandData& MinecraftServer::getOrCreateBrewingStand(int64_t posKey) {
    std::lock_guard<std::mutex> lock(brewingStandMutex_);
    return brewingStandStorage_[posKey];
}

MinecraftServer::DispenserData& MinecraftServer::getOrCreateDispenser(int64_t posKey) {
    std::lock_guard<std::mutex> lock(dispenserMutex_);
    return dispenserStorage_[posKey];
}

MinecraftServer::HopperData& MinecraftServer::getOrCreateHopper(int64_t posKey) {
    std::lock_guard<std::mutex> lock(hopperMutex_);
    return hopperStorage_[posKey];
}

// ═══════════════════════════════════════════════════════════════════════════
// Hopper tile entity ticking — Java: TileEntityHopper.updateEntity()
// Every 8 ticks: pull 1 item from inventory above, push 1 item to output direction
// ═══════════════════════════════════════════════════════════════════════════
void MinecraftServer::tickHoppers() {
    std::lock_guard<std::mutex> lock(hopperMutex_);
    if (hopperStorage_.empty()) return;

    auto world = worlds_.empty() ? nullptr : worlds_[0].get();
    if (!world) return;

    // Helper: make position key from xyz
    auto makePosKey = [](int32_t x, int32_t y, int32_t z) -> int64_t {
        return (static_cast<int64_t>(x) & 0x3FFFFFFLL) << 38 |
               (static_cast<int64_t>(y) & 0xFFFLL) << 26 |
               (static_cast<int64_t>(z) & 0x3FFFFFFLL);
    };

    // Helper: unpack position key
    auto unpackPos = [](int64_t key, int32_t& x, int32_t& y, int32_t& z) {
        x = static_cast<int32_t>((key >> 38) & 0x3FFFFFF);
        if (x & 0x2000000) x |= ~0x3FFFFFF; // sign extend
        y = static_cast<int32_t>((key >> 26) & 0xFFF);
        z = static_cast<int32_t>(key & 0x3FFFFFF);
        if (z & 0x2000000) z |= ~0x3FFFFFF; // sign extend
    };

    // Helper: check if items can stack
    auto canStack = [](const std::optional<ItemStack>& a, const ItemStack& b) -> bool {
        if (!a.has_value()) return false;
        return a->getItemId() == b.getItemId() &&
               a->getDamage() == b.getDamage() &&
               a->getStackSize() < a->getMaxStackSize();
    };

    // Hopper output direction from metadata
    // Java: BlockHopper.getDirectionFromMetadata(meta)
    // 0=down, 2=north, 3=south, 4=west, 5=east
    static const int offX[] = {0, 0, 0, 0, -1, 1};
    static const int offY[] = {-1, 0, 0, 0, 0, 0};
    static const int offZ[] = {0, 0, -1, 1, 0, 0};

    for (auto& [posKey, hopper] : hopperStorage_) {
        // Decrement cooldown
        if (hopper.transferCooldown > 0) {
            --hopper.transferCooldown;
            continue;
        }

        int32_t hx, hy, hz;
        unpackPos(posKey, hx, hy, hz);

        // Get hopper block and meta
        Block* hBlock = world->getBlock(hx, hy, hz);
        int hId = hBlock ? Block::getIdFromBlock(hBlock) : 0;
        if (hId != 154) continue; // Not a hopper anymore

        int hMeta = world->getBlockMetadata(hx, hy, hz);
        int dir = hMeta & 7;
        if (dir > 5) dir = 0;

        bool transferred = false;

        // ─── Step 1: Pull from inventory above ───────────────────────
        int32_t aboveX = hx, aboveY = hy + 1, aboveZ = hz;
        Block* aboveBlock = world->getBlock(aboveX, aboveY, aboveZ);
        int aboveId = aboveBlock ? Block::getIdFromBlock(aboveBlock) : 0;
        int64_t aboveKey = makePosKey(aboveX, aboveY, aboveZ);

        // Pull from chest above (54, 146)
        if ((aboveId == 54 || aboveId == 146) && !transferred) {
            std::lock_guard<std::mutex> cLock(chestMutex_);
            auto cIt = chestStorage_.find(aboveKey);
            if (cIt != chestStorage_.end()) {
                for (int s = 0; s < 27 && !transferred; ++s) {
                    if (cIt->second[s].has_value() && cIt->second[s]->getStackSize() > 0) {
                        // Try to insert 1 item into hopper
                        auto& srcItem = cIt->second[s];
                        for (int h = 0; h < 5; ++h) {
                            if (!hopper.slots[h].has_value()) {
                                hopper.slots[h] = ItemStack(srcItem->getItemId(), 1, srcItem->getDamage());
                                srcItem->setStackSize(srcItem->getStackSize() - 1);
                                if (srcItem->getStackSize() <= 0) srcItem.reset();
                                transferred = true;
                                break;
                            } else if (canStack(hopper.slots[h], *srcItem) &&
                                       hopper.slots[h]->getStackSize() < hopper.slots[h]->getMaxStackSize()) {
                                hopper.slots[h]->setStackSize(hopper.slots[h]->getStackSize() + 1);
                                srcItem->setStackSize(srcItem->getStackSize() - 1);
                                if (srcItem->getStackSize() <= 0) srcItem.reset();
                                transferred = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        // Pull from hopper above
        if (aboveId == 154 && !transferred) {
            auto hIt = hopperStorage_.find(aboveKey);
            if (hIt != hopperStorage_.end() && &hIt->second != &hopper) {
                for (int s = 0; s < 5 && !transferred; ++s) {
                    if (hIt->second.slots[s].has_value() && hIt->second.slots[s]->getStackSize() > 0) {
                        auto& srcItem = hIt->second.slots[s];
                        for (int h = 0; h < 5; ++h) {
                            if (!hopper.slots[h].has_value()) {
                                hopper.slots[h] = ItemStack(srcItem->getItemId(), 1, srcItem->getDamage());
                                srcItem->setStackSize(srcItem->getStackSize() - 1);
                                if (srcItem->getStackSize() <= 0) srcItem.reset();
                                transferred = true;
                                break;
                            } else if (canStack(hopper.slots[h], *srcItem) &&
                                       hopper.slots[h]->getStackSize() < hopper.slots[h]->getMaxStackSize()) {
                                hopper.slots[h]->setStackSize(hopper.slots[h]->getStackSize() + 1);
                                srcItem->setStackSize(srcItem->getStackSize() - 1);
                                if (srcItem->getStackSize() <= 0) srcItem.reset();
                                transferred = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        // ─── Step 2: Push to output inventory ────────────────────────
        int32_t outX = hx + offX[dir], outY = hy + offY[dir], outZ = hz + offZ[dir];
        Block* outBlock = world->getBlock(outX, outY, outZ);
        int outId = outBlock ? Block::getIdFromBlock(outBlock) : 0;
        int64_t outKey = makePosKey(outX, outY, outZ);

        if (!transferred) {
            // Push to chest (54, 146)
            if (outId == 54 || outId == 146) {
                std::lock_guard<std::mutex> cLock(chestMutex_);
                auto cIt = chestStorage_.find(outKey);
                if (cIt != chestStorage_.end()) {
                    for (int h = 0; h < 5 && !transferred; ++h) {
                        if (hopper.slots[h].has_value() && hopper.slots[h]->getStackSize() > 0) {
                            auto& srcItem = hopper.slots[h];
                            for (int s = 0; s < 27; ++s) {
                                if (!cIt->second[s].has_value()) {
                                    cIt->second[s] = ItemStack(srcItem->getItemId(), 1, srcItem->getDamage());
                                    srcItem->setStackSize(srcItem->getStackSize() - 1);
                                    if (srcItem->getStackSize() <= 0) srcItem.reset();
                                    transferred = true;
                                    break;
                                } else if (canStack(cIt->second[s], *srcItem) &&
                                           cIt->second[s]->getStackSize() < cIt->second[s]->getMaxStackSize()) {
                                    cIt->second[s]->setStackSize(cIt->second[s]->getStackSize() + 1);
                                    srcItem->setStackSize(srcItem->getStackSize() - 1);
                                    if (srcItem->getStackSize() <= 0) srcItem.reset();
                                    transferred = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            // Push to hopper output
            if (outId == 154 && !transferred) {
                auto hIt = hopperStorage_.find(outKey);
                if (hIt != hopperStorage_.end() && &hIt->second != &hopper) {
                    for (int h = 0; h < 5 && !transferred; ++h) {
                        if (hopper.slots[h].has_value() && hopper.slots[h]->getStackSize() > 0) {
                            auto& srcItem = hopper.slots[h];
                            for (int s = 0; s < 5; ++s) {
                                if (!hIt->second.slots[s].has_value()) {
                                    hIt->second.slots[s] = ItemStack(srcItem->getItemId(), 1, srcItem->getDamage());
                                    srcItem->setStackSize(srcItem->getStackSize() - 1);
                                    if (srcItem->getStackSize() <= 0) srcItem.reset();
                                    transferred = true;
                                    break;
                                } else if (canStack(hIt->second.slots[s], *srcItem) &&
                                           hIt->second.slots[s]->getStackSize() < hIt->second.slots[s]->getMaxStackSize()) {
                                    hIt->second.slots[s]->setStackSize(hIt->second.slots[s]->getStackSize() + 1);
                                    srcItem->setStackSize(srcItem->getStackSize() - 1);
                                    if (srcItem->getStackSize() <= 0) srcItem.reset();
                                    transferred = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Set cooldown
        if (transferred) {
            hopper.transferCooldown = 8; // Java: TileEntityHopper cooldown = 8 ticks
        }
    }
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
// Potion effect helpers — used by /effect command
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::applyPlayerPotionEffect(const std::string& playerName, int32_t effectId,
                                               int32_t durationTicks, int32_t amplifier) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (ph && ph->getPlayerName() == playerName) {
            ph->addPotionEffect(*conn, effectId, durationTicks, amplifier);
            return;
        }
    }
}

void MinecraftServer::clearPlayerPotionEffects(const std::string& playerName) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (ph && ph->getPlayerName() == playerName) {
            ph->clearPotionEffects(*conn);
            return;
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

void MinecraftServer::tickRandomBlocks() {
    // Java reference: WorldServer.func_147456_g() — random tick speed 3 per section
    // Simplified: for each loaded chunk near a player, pick random blocks and apply growth
    if (worlds_.empty()) return;
    auto& world = worlds_[0];

    static thread_local std::mt19937 rng(std::random_device{}());

    // Get player positions
    std::vector<std::pair<int, int>> playerChunks;
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* play = dynamic_cast<PlayHandler*>(handler.get());
            if (!play) continue;
            int cx = static_cast<int>(std::floor(play->getPlayerX())) >> 4;
            int cz = static_cast<int>(std::floor(play->getPlayerZ())) >> 4;
            playerChunks.emplace_back(cx, cz);
        }
    }

    // For each player's nearby chunks, apply random ticks
    // Java: iterates all loaded chunks in range, 3 random ticks per section
    // Simplified: 3 chunks × 3 ticks = 9 random block ticks per player per tick
    for (auto& [pcx, pcz] : playerChunks) {
        for (int attempt = 0; attempt < 3; ++attempt) {
            // Pick a random chunk within 7-chunk radius
            int cx = pcx + std::uniform_int_distribution<>(-7, 7)(rng);
            int cz = pcz + std::uniform_int_distribution<>(-7, 7)(rng);

            // For each chunk, pick 3 random positions
            for (int t = 0; t < 3; ++t) {
                int lx = std::uniform_int_distribution<>(0, 15)(rng);
                int ly = std::uniform_int_distribution<>(1, 128)(rng); // Focus on surface
                int lz = std::uniform_int_distribution<>(0, 15)(rng);

                int bx = cx * 16 + lx;
                int by = ly;
                int bz = cz * 16 + lz;

                Block* block = world->getBlock(bx, by, bz);
                if (!block) continue;
                int blockId = Block::getIdFromBlock(block);
                if (blockId == 0) continue;

                int meta = world->getBlockMetadata(bx, by, bz);

                // ─── Wheat (59), Carrots (141), Potatoes (142) ───
                // Java: BlockCrops.updateTick — advance growth with 1/4 chance
                if (blockId == 59 || blockId == 141 || blockId == 142) {
                    if (meta < 7 && std::uniform_int_distribution<>(0, 3)(rng) == 0) {
                        world->setBlockMetadata(bx, by, bz, meta + 1);
                        broadcastBlockChange(bx, by, bz, blockId, meta + 1);
                    }
                }

                // ─── Sapling (6) — grow into tree ───
                // Java: BlockSapling.updateTick — stage 0→8 then tree
                if (blockId == 6) {
                    if (meta < 8) {
                        // Increment stage (uses bits 3 for stage flag)
                        if (std::uniform_int_distribution<>(0, 6)(rng) == 0) {
                            int newMeta = meta | 0x08; // Set stage bit
                            world->setBlockMetadata(bx, by, bz, newMeta);
                            broadcastBlockChange(bx, by, bz, 6, newMeta);
                        }
                    }
                    if (meta & 0x08) {
                        // Stage 1 — attempt tree growth
                        if (std::uniform_int_distribution<>(0, 6)(rng) == 0) {
                            // Remove sapling
                            world->setBlock(bx, by, bz, Block::getBlockById(0));
                            broadcastBlockChange(bx, by, bz, 0, 0);
                            // Generate simple oak tree
                            // Java: WorldGenTrees.generate()
                            int treeHeight = 4 + std::uniform_int_distribution<>(0, 2)(rng);
                            bool canGrow = true;
                            for (int y2 = by + 1; y2 <= by + treeHeight + 1 && y2 < 256; ++y2) {
                                Block* b = world->getBlock(bx, y2, bz);
                                if (b && Block::getIdFromBlock(b) != 0 && Block::getIdFromBlock(b) != 18) {
                                    canGrow = false;
                                    break;
                                }
                            }
                            if (canGrow && by + treeHeight + 2 < 256) {
                                // Trunk
                                for (int y2 = by; y2 < by + treeHeight; ++y2) {
                                    world->setBlock(bx, y2, bz, Block::getBlockById(17)); // Log
                                    world->setBlockMetadata(bx, y2, bz, 0);
                                    broadcastBlockChange(bx, y2, bz, 17, 0);
                                }
                                // Leaves — 3×3 for top 2 layers, 5×5 for middle 2
                                for (int ly2 = treeHeight - 3; ly2 <= treeHeight; ++ly2) {
                                    int radius = (ly2 >= treeHeight - 1) ? 1 : 2;
                                    for (int dx = -radius; dx <= radius; ++dx) {
                                        for (int dz = -radius; dz <= radius; ++dz) {
                                            if (dx == 0 && dz == 0 && ly2 < treeHeight) continue; // Trunk
                                            int lbx = bx + dx, lby = by + ly2, lbz = bz + dz;
                                            if (lby >= 256) continue;
                                            Block* lb = world->getBlock(lbx, lby, lbz);
                                            if (!lb || Block::getIdFromBlock(lb) == 0) {
                                                world->setBlock(lbx, lby, lbz, Block::getBlockById(18));
                                                world->setBlockMetadata(lbx, lby, lbz, 0);
                                                broadcastBlockChange(lbx, lby, lbz, 18, 0);
                                            }
                                        }
                                    }
                                }
                            } else {
                                // Can't grow — put sapling back
                                world->setBlock(bx, by, bz, Block::getBlockById(6));
                                world->setBlockMetadata(bx, by, bz, meta & 0x07);
                                broadcastBlockChange(bx, by, bz, 6, meta & 0x07);
                            }
                        }
                    }
                }

                // ─── Grass block (2) spread to dirt ───
                // Java: BlockGrass.updateTick — spreads to adjacent dirt blocks
                if (blockId == 2) {
                    if (std::uniform_int_distribution<>(0, 3)(rng) == 0) {
                        int dx = std::uniform_int_distribution<>(-1, 1)(rng);
                        int dy = std::uniform_int_distribution<>(-1, 1)(rng);
                        int dz2 = std::uniform_int_distribution<>(-1, 1)(rng);
                        int nx = bx + dx, ny = by + dy, nz = bz + dz2;
                        Block* nb = world->getBlock(nx, ny, nz);
                        if (nb && Block::getIdFromBlock(nb) == 3) { // Dirt
                            // Check above is air or transparent
                            Block* above = world->getBlock(nx, ny + 1, nz);
                            int aboveId = above ? Block::getIdFromBlock(above) : 0;
                            if (aboveId == 0 || aboveId == 31 || aboveId == 37 || aboveId == 38) {
                                world->setBlock(nx, ny, nz, Block::getBlockById(2)); // Spread grass
                                broadcastBlockChange(nx, ny, nz, 2, 0);
                            }
                        }
                    }
                }

                // ─── Farmland (60) — hydration/decay ───
                // Java: BlockFarmland.updateTick — check for water nearby
                if (blockId == 60) {
                    bool nearWater = false;
                    for (int dx = -4; dx <= 4 && !nearWater; ++dx) {
                        for (int dz2 = -4; dz2 <= 4 && !nearWater; ++dz2) {
                            Block* wb = world->getBlock(bx + dx, by, bz + dz2);
                            if (wb) {
                                int wid = Block::getIdFromBlock(wb);
                                if (wid == 8 || wid == 9) nearWater = true;
                            }
                            wb = world->getBlock(bx + dx, by + 1, bz + dz2);
                            if (wb) {
                                int wid = Block::getIdFromBlock(wb);
                                if (wid == 8 || wid == 9) nearWater = true;
                            }
                        }
                    }
                    if (nearWater) {
                        if (meta < 7) {
                            world->setBlockMetadata(bx, by, bz, 7); // Fully hydrated
                            broadcastBlockChange(bx, by, bz, 60, 7);
                        }
                    } else {
                        if (meta > 0) {
                            world->setBlockMetadata(bx, by, bz, meta - 1); // Dry out
                            broadcastBlockChange(bx, by, bz, 60, meta - 1);
                        } else {
                            // Check if there's a crop above — if no crop, revert to dirt
                            Block* above = world->getBlock(bx, by + 1, bz);
                            int aboveId = above ? Block::getIdFromBlock(above) : 0;
                            if (aboveId != 59 && aboveId != 141 && aboveId != 142 &&
                                aboveId != 104 && aboveId != 105) {
                                world->setBlock(bx, by, bz, Block::getBlockById(3)); // Dirt
                                world->setBlockMetadata(bx, by, bz, 0);
                                broadcastBlockChange(bx, by, bz, 3, 0);
                            }
                        }
                    }
                }

                // ─── Melon/Pumpkin stems (104/105) — grow fruit ───
                if (blockId == 104 || blockId == 105) {
                    if (meta < 7 && std::uniform_int_distribution<>(0, 5)(rng) == 0) {
                        world->setBlockMetadata(bx, by, bz, meta + 1);
                        broadcastBlockChange(bx, by, bz, blockId, meta + 1);
                    } else if (meta >= 7 && std::uniform_int_distribution<>(0, 7)(rng) == 0) {
                        // Try to place fruit adjacent
                        int fruitId = (blockId == 104) ? 86 : 103; // Pumpkin : Melon block
                        int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
                        int d = std::uniform_int_distribution<>(0, 3)(rng);
                        int fx = bx + dirs[d][0], fz = bz + dirs[d][1];
                        Block* fb = world->getBlock(fx, by, fz);
                        Block* below = world->getBlock(fx, by - 1, fz);
                        int belowId = below ? Block::getIdFromBlock(below) : 0;
                        if (fb && Block::getIdFromBlock(fb) == 0 &&
                            (belowId == 2 || belowId == 3 || belowId == 60)) {
                            world->setBlock(fx, by, fz, Block::getBlockById(fruitId));
                            broadcastBlockChange(fx, by, fz, fruitId, 0);
                        }
                    }
                }

                // ─── Leaf decay (18/161) ───
                // Java: BlockLeaves.updateTick — natural leaves check for nearby logs
                // Meta bit 2 = player-placed (never decays)
                if (blockId == 18 || blockId == 161) {
                    if (!(meta & 0x04)) { // Not player-placed
                        if (std::uniform_int_distribution<>(0, 3)(rng) == 0) {
                            bool hasLog = false;
                            for (int dx = -4; dx <= 4 && !hasLog; ++dx) {
                                for (int dy2 = -4; dy2 <= 4 && !hasLog; ++dy2) {
                                    for (int dz2 = -4; dz2 <= 4 && !hasLog; ++dz2) {
                                        if (std::abs(dx) + std::abs(dy2) + std::abs(dz2) > 4) continue;
                                        Block* lb = world->getBlock(bx + dx, by + dy2, bz + dz2);
                                        if (lb) {
                                            int lid = Block::getIdFromBlock(lb);
                                            if (lid == 17 || lid == 162) hasLog = true;
                                        }
                                    }
                                }
                            }
                            if (!hasLog) {
                                world->setBlock(bx, by, bz, Block::getBlockById(0));
                                broadcastBlockChange(bx, by, bz, 0, 0);
                                broadcastEffect(2001, bx, by, bz, blockId);
                            }
                        }
                    }
                }

                // ─── Sand/gravel gravity (12/13) — Java: BlockFalling.updateTick ─
                if (blockId == 12 || blockId == 13) {
                    Block* below = world->getBlock(bx, by - 1, bz);
                    int belowId = below ? Block::getIdFromBlock(below) : 0;
                    if (by > 1 && (belowId == 0 || belowId == 8 || belowId == 9 ||
                        belowId == 10 || belowId == 11)) {
                        // Find landing position
                        int fallY = by - 1;
                        while (fallY > 0) {
                            Block* fb = world->getBlock(bx, fallY, bz);
                            int fid = fb ? Block::getIdFromBlock(fb) : 0;
                            if (fid != 0 && fid != 8 && fid != 9 && fid != 10 && fid != 11) break;
                            --fallY;
                        }
                        ++fallY;
                        if (fallY < by) {
                            world->setBlock(bx, by, bz, Block::getBlockById(0));
                            broadcastBlockChange(bx, by, bz, 0, 0);
                            world->setBlock(bx, fallY, bz, Block::getBlockById(blockId));
                            broadcastBlockChange(bx, fallY, bz, blockId, 0);
                        }
                    }
                }

                // ─── Water/Lava flow (8/9/10/11) — Java: BlockDynamicLiquid.updateTick ─
                // Water: block 8 (flowing) / 9 (still source) — decay 1, tick rate 5
                // Lava:  block 10 (flowing) / 11 (still source) — decay 2, tick rate 30
                if (blockId == 8 || blockId == 9 || blockId == 10 || blockId == 11) {
                    bool isWater = (blockId == 8 || blockId == 9);
                    int liquidMeta = world->getBlockMetadata(bx, by, bz);
                    int flowId = isWater ? 8 : 10;   // Flowing block ID
                    int stillId = isWater ? 9 : 11;   // Still source ID
                    int decayFactor = isWater ? 1 : 2; // Java: lava overworld decay=2
                    int maxLevel = 7; // Meta 0=source, 1-7=flow distance, 8+=falling

                    // [canDisplace lambda removed — blocksFlow covers the needed checks]

                    // Helper: is position blocksMovement (solid or liquid-blocking)?
                    auto blocksFlow = [&](int x, int y, int z) -> bool {
                        Block* b = world->getBlock(x, y, z);
                        int bid = b ? Block::getIdFromBlock(b) : 0;
                        if (bid == 0) return false;
                        if (bid == 64 || bid == 71 || bid == 63 || bid == 68 || bid == 65 || bid == 83) return true; // doors/signs/ladders/reeds
                        if (bid == 90) return true; // Portal
                        if (isWater && (bid == 8 || bid == 9)) return false;
                        if (!isWater && (bid == 10 || bid == 11)) return false;
                        // Non-solid: air, flowers, grass, torches, etc.
                        if (bid == 31 || bid == 32 || bid == 37 || bid == 38 || bid == 39 || bid == 40 ||
                            bid == 6 || bid == 78 || bid == 106 || bid == 50 || bid == 51 || bid == 55 ||
                            bid == 75 || bid == 76 || bid == 175) return false;
                        return true; // Most blocks are solid
                    };

                    // Helper: place flowing liquid at position
                    auto placeFlow = [&](int x, int y, int z, int meta) {
                        Block* existing = world->getBlock(x, y, z);
                        int eid = existing ? Block::getIdFromBlock(existing) : 0;

                        // Lava + water interaction
                        if (!isWater) {
                            // Lava flowing into water
                            if (eid == 8 || eid == 9) {
                                int waterMeta = world->getBlockMetadata(x, y, z);
                                if (waterMeta == 0 || eid == 9) {
                                    // Lava source touching water source → obsidian
                                    world->setBlock(x, y, z, Block::getBlockById(49)); // Obsidian
                                    world->setBlockMetadata(x, y, z, 0);
                                    broadcastBlockChange(x, y, z, 49, 0);
                                } else {
                                    // Lava flowing touching flowing water → cobblestone
                                    world->setBlock(x, y, z, Block::getBlockById(4)); // Cobblestone
                                    world->setBlockMetadata(x, y, z, 0);
                                    broadcastBlockChange(x, y, z, 4, 0);
                                }
                                broadcastSound("random.fizz",
                                    static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5,
                                    static_cast<double>(z) + 0.5, 0.5f, 2.6f);
                                return;
                            }
                        } else {
                            // Water flowing into lava
                            if (eid == 10 || eid == 11) {
                                int lavaMeta = world->getBlockMetadata(x, y, z);
                                if (lavaMeta == 0 || eid == 11) {
                                    // Water touching lava source → obsidian
                                    world->setBlock(x, y, z, Block::getBlockById(49));
                                    world->setBlockMetadata(x, y, z, 0);
                                    broadcastBlockChange(x, y, z, 49, 0);
                                } else {
                                    // Water touching flowing lava → cobblestone
                                    world->setBlock(x, y, z, Block::getBlockById(4));
                                    world->setBlockMetadata(x, y, z, 0);
                                    broadcastBlockChange(x, y, z, 4, 0);
                                }
                                broadcastSound("random.fizz",
                                    static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5,
                                    static_cast<double>(z) + 0.5, 0.5f, 2.6f);
                                return;
                            }
                        }

                        // Normal displacement
                        world->setBlock(x, y, z, Block::getBlockById(flowId));
                        world->setBlockMetadata(x, y, z, meta);
                        broadcastBlockChange(x, y, z, flowId, meta);
                    };

                    // Source blocks (meta 0 or still source IDs 9/11) always try to spread
                    bool isSource = (liquidMeta == 0 || blockId == stillId);
                    int currentLevel = isSource ? 0 : (liquidMeta & 0x07);

                    // ─── Infinite water source (Java: field_149815_a >= 2) ───
                    // Water only: if 2+ adjacent source blocks AND solid below → become source
                    if (isWater && !isSource && currentLevel > 0 && currentLevel < 8) {
                        int adjacentSources = 0;
                        static const int sdx[] = {-1, 1, 0, 0};
                        static const int sdz[] = {0, 0, -1, 1};
                        for (int d = 0; d < 4; ++d) {
                            Block* ab = world->getBlock(bx + sdx[d], by, bz + sdz[d]);
                            int aid = ab ? Block::getIdFromBlock(ab) : 0;
                            if (aid == 9) { ++adjacentSources; continue; } // Still water source
                            if (aid == 8 && world->getBlockMetadata(bx + sdx[d], by, bz + sdz[d]) == 0) ++adjacentSources;
                        }
                        if (adjacentSources >= 2) {
                            // Check below is solid or water source
                            Block* belowB = world->getBlock(bx, by - 1, bz);
                            int belowBid = belowB ? Block::getIdFromBlock(belowB) : 0;
                            bool solidBelow = blocksFlow(bx, by - 1, bz) ||
                                              belowBid == 9 || (belowBid == 8 && world->getBlockMetadata(bx, by - 1, bz) == 0);
                            if (solidBelow) {
                                // Become a source!
                                world->setBlockMetadata(bx, by, bz, 0);
                                broadcastBlockChange(bx, by, bz, 8, 0);
                                isSource = true;
                                currentLevel = 0;
                            }
                        }
                    }

                    // Step 1: Try to flow downward
                    if (by > 0) {
                        Block* below = world->getBlock(bx, by - 1, bz);
                        int belowId = below ? Block::getIdFromBlock(below) : 0;

                        if (belowId == 0 || (!blocksFlow(bx, by - 1, bz) &&
                            !(isWater ? (belowId == 8 || belowId == 9) : (belowId == 10 || belowId == 11)))) {
                            // Flow down — falling has meta 8+ (8 = falling from source)
                            int fallMeta = (currentLevel >= 8) ? currentLevel : (currentLevel + 8);
                            if (fallMeta > 15) fallMeta = 8;
                            placeFlow(bx, by - 1, bz, fallMeta);
                        }
                    }

                    // Step 2: Spread horizontally if we haven't exceeded range
                    int nextLevel = currentLevel + decayFactor;
                    if (currentLevel >= 8) nextLevel = 1; // Falling → restart at level 1
                    if (nextLevel <= maxLevel) {
                        // Java: func_149808_o — find shortest path to downward edge
                        // Simplified: spread to all 4 cardinal directions that aren't blocked
                        static const int dx[] = {-1, 1, 0, 0};
                        static const int dz[] = {0, 0, -1, 1};

                        for (int dir = 0; dir < 4; ++dir) {
                            int nx = bx + dx[dir];
                            int nz = bz + dz[dir];

                            // Can't flow into solid blocks
                            if (blocksFlow(nx, by, nz)) continue;

                            Block* nb = world->getBlock(nx, by, nz);
                            int nid = nb ? Block::getIdFromBlock(nb) : 0;

                            // Skip if same liquid already at equal or lower level
                            if ((isWater && (nid == 8 || nid == 9)) ||
                                (!isWater && (nid == 10 || nid == 11))) {
                                int existingMeta = world->getBlockMetadata(nx, by, nz);
                                int existingLevel = (existingMeta >= 8) ? 0 : existingMeta;
                                if (nid == stillId) existingLevel = 0;
                                if (existingLevel <= nextLevel) continue;
                            }

                            placeFlow(nx, by, nz, nextLevel);
                        }
                    }

                    // Step 3: Source blocks with no feed should have been handled.
                    // Non-source flowing blocks should shrink if no adjacent source feeds them
                    if (!isSource && currentLevel < 8) {
                        // Check if any adjacent block feeds this flow
                        bool fed = false;
                        // Check above for falling
                        Block* above = world->getBlock(bx, by + 1, bz);
                        int aboveId = above ? Block::getIdFromBlock(above) : 0;
                        if ((isWater && (aboveId == 8 || aboveId == 9)) ||
                            (!isWater && (aboveId == 10 || aboveId == 11))) {
                            fed = true;
                        }
                        // Check 4 cardinal directions for lower-level source
                        if (!fed) {
                            static const int dx2[] = {-1, 1, 0, 0};
                            static const int dz2[] = {0, 0, -1, 1};
                            for (int dir = 0; dir < 4; ++dir) {
                                int ax = bx + dx2[dir];
                                int az = bz + dz2[dir];
                                Block* ab = world->getBlock(ax, by, az);
                                int aid = ab ? Block::getIdFromBlock(ab) : 0;
                                if ((isWater && (aid == 8 || aid == 9)) ||
                                    (!isWater && (aid == 10 || aid == 11))) {
                                    int adjMeta = world->getBlockMetadata(ax, by, az);
                                    int adjLevel = (adjMeta >= 8) ? 0 : adjMeta;
                                    if (aid == stillId) adjLevel = 0;
                                    if (adjLevel < currentLevel) {
                                        fed = true;
                                        break;
                                    }
                                }
                            }
                        }
                        // If not fed, shrink (increase level) or remove
                        if (!fed) {
                            int newLevel = currentLevel + decayFactor;
                            if (newLevel > maxLevel) {
                                // Remove this flow
                                world->setBlock(bx, by, bz, Block::getBlockById(0));
                                broadcastBlockChange(bx, by, bz, 0, 0);
                            } else {
                                world->setBlockMetadata(bx, by, bz, newLevel);
                                broadcastBlockChange(bx, by, bz, flowId, newLevel);
                            }
                        }
                    }
                }

                // ─── Fire spread (51) — Java: BlockFire.updateTick ──────────────
                // Fire burns adjacent flammable blocks and spreads
                if (blockId == 51) {
                    int fireMeta = world->getBlockMetadata(bx, by, bz);
                    // Fire burns out: meta increments, at 15 it goes out (if not on netherrack)
                    if (fireMeta < 15) {
                        int newMeta = fireMeta + (rng() % 3 == 0 ? 1 : 0);
                        if (newMeta > 15) newMeta = 15;
                        world->setBlockMetadata(bx, by, bz, newMeta);
                        broadcastBlockChange(bx, by, bz, 51, newMeta);
                    }
                    // Check if fire should go out (no fuel below and not netherrack)
                    Block* below = world->getBlock(bx, by - 1, bz);
                    int belowId = below ? Block::getIdFromBlock(below) : 0;
                    bool hasNetherrack = (belowId == 87); // Netherrack: eternal fire
                    bool hasFuel = false;
                    // Check adjacent blocks for flammability
                    static const int fdx[] = {-1, 1, 0, 0, 0, 0};
                    static const int fdy[] = {0, 0, -1, 1, 0, 0};
                    static const int fdz[] = {0, 0, 0, 0, -1, 1};
                    // Flammable block IDs: wood(5,17,125,126,162), planks, wool(35), bookshelf(47),
                    // leaves(18,161), fence(85,113), stairs(53,134-136,163-164), carpet(171), hay(170)
                    auto isFlammable = [](int id) -> bool {
                        return id == 5 || id == 17 || id == 18 || id == 35 || id == 47 ||
                               id == 53 || id == 85 || id == 125 || id == 126 || id == 134 ||
                               id == 135 || id == 136 || id == 161 || id == 162 || id == 163 ||
                               id == 164 || id == 170 || id == 171 || id == 113 || id == 107 ||
                               id == 31 || id == 32 || id == 37 || id == 38 || id == 175 || id == 106;
                    };
                    for (int d = 0; d < 6; ++d) {
                        int fx = bx + fdx[d];
                        int fy = by + fdy[d];
                        int fz = bz + fdz[d];
                        Block* fb = world->getBlock(fx, fy, fz);
                        int fid = fb ? Block::getIdFromBlock(fb) : 0;
                        if (isFlammable(fid)) {
                            hasFuel = true;
                            // Random chance to ignite/consume adjacent block
                            if (rng() % 3 == 0) {
                                // Consume the block, possibly replace with fire
                                world->setBlock(fx, fy, fz, Block::getBlockById(0));
                                broadcastBlockChange(fx, fy, fz, 0, 0);
                                broadcastEffect(2001, fx, fy, fz, fid); // Break particles
                                // Spread fire to the position
                                if (rng() % 2 == 0) {
                                    world->setBlock(fx, fy, fz, Block::getBlockById(51));
                                    world->setBlockMetadata(fx, fy, fz, 0);
                                    broadcastBlockChange(fx, fy, fz, 51, 0);
                                }
                            }
                        }
                    }
                    // Fire goes out if no fuel and not netherrack
                    if (!hasFuel && !hasNetherrack && fireMeta >= 3) {
                        world->setBlock(bx, by, bz, Block::getBlockById(0));
                        broadcastBlockChange(bx, by, bz, 0, 0);
                    }
                }

                // ─── Ice melting (79) — Java: BlockIce.updateTick ────────────────
                // Ice melts into water when in bright conditions (simplified: ~1/8 chance per tick)
                if (blockId == 79) {
                    // Simplified: ice melts with low probability (no light engine)
                    if (rng() % 8 == 0) {
                        world->setBlock(bx, by, bz, Block::getBlockById(8)); // Flowing water
                        world->setBlockMetadata(bx, by, bz, 0);
                        broadcastBlockChange(bx, by, bz, 8, 0);
                    }
                }

                // ─── Mushroom spread (39/40) — Java: BlockMushroom.updateTick ────
                if (blockId == 39 || blockId == 40) {
                    if (rng() % 25 == 0) {
                        // Spread to random adjacent position
                        int mx = bx + (rng() % 3) - 1;
                        int my = by + (rng() % 3) - 1;
                        int mz = bz + (rng() % 3) - 1;
                        Block* mb = world->getBlock(mx, my, mz);
                        int mid = mb ? Block::getIdFromBlock(mb) : 0;
                        if (mid == 0) { // Air
                            Block* mbelow = world->getBlock(mx, my - 1, mz);
                            int mbelowId = mbelow ? Block::getIdFromBlock(mbelow) : 0;
                            // Mushrooms grow on opaque blocks
                            if (mbelowId >= 1 && mbelowId != 8 && mbelowId != 9 &&
                                mbelowId != 10 && mbelowId != 11 && mbelowId != 20) {
                                world->setBlock(mx, my, mz, Block::getBlockById(blockId));
                                broadcastBlockChange(mx, my, mz, blockId, 0);
                            }
                        }
                    }
                }

                // ─── Vine growth (106) — Java: BlockVine.updateTick ─────────────
                // Vines grow downward through air
                if (blockId == 106) {
                    if (rng() % 4 == 0) {
                        // Grow downward
                        if (by > 1) {
                            Block* below = world->getBlock(bx, by - 1, bz);
                            int belowId = below ? Block::getIdFromBlock(below) : 0;
                            if (belowId == 0) {
                                // Place vine below with same metadata
                                int vineMeta = world->getBlockMetadata(bx, by, bz);
                                world->setBlock(bx, by - 1, bz, Block::getBlockById(106));
                                world->setBlockMetadata(bx, by - 1, bz, vineMeta);
                                broadcastBlockChange(bx, by - 1, bz, 106, vineMeta);
                            }
                        }
                    }
                }

                // ─── Nether wart growth (115) — Java: BlockNetherWart.updateTick ─
                // Nether wart grows: meta 0→1→2→3 (3 = mature)
                if (blockId == 115) {
                    int wartMeta = world->getBlockMetadata(bx, by, bz);
                    if (wartMeta < 3 && rng() % 10 == 0) {
                        world->setBlockMetadata(bx, by, bz, wartMeta + 1);
                        broadcastBlockChange(bx, by, bz, 115, wartMeta + 1);
                    }
                }

                // ─── Cocoa bean growth (127) — Java: BlockCocoa.updateTick ───────
                // Cocoa beans grow: meta bits 2-3 store age (0→1→2, or 0→4→8)
                if (blockId == 127) {
                    int cocoaMeta = world->getBlockMetadata(bx, by, bz);
                    int age = (cocoaMeta >> 2) & 0x03;
                    if (age < 2 && rng() % 5 == 0) {
                        int newMeta = (cocoaMeta & 0x03) | ((age + 1) << 2);
                        world->setBlockMetadata(bx, by, bz, newMeta);
                        broadcastBlockChange(bx, by, bz, 127, newMeta);
                    }
                }
            }
        }
    }
}

} // namespace mccpp
