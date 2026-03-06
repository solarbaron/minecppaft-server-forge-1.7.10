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
#include "redstone/Redstone.h"
#include "entity/EntityList.h"
#include "worldgen/LootTables.h"
#include "worldgen/NoiseGeneratorOctaves.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
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

    // Set the tile entity loader callback so that chunks loaded from disk
    // populate the server's tile entity storage maps.
    overworld->setTileEntityLoader([this](const nbt::NBTTagCompound& levelTag) {
        loadTileEntitiesFromChunk(levelTag);
    });

    // Set spawner registrar callback so freshly-generated dungeon spawners
    // create matching SpawnerData entries in the server's spawnerStorage_.
    overworld->setSpawnerRegistrar([this](Chunk& chunk) {
        for (auto& info : chunk.pendingSpawners) {
            int64_t pk = packBlockPos(info.x, info.y, info.z);
            std::lock_guard<std::mutex> lock(spawnerMutex_);
            auto& sd = spawnerStorage_[pk];
            sd.entityId = info.entityId;
        }
        chunk.pendingSpawners.clear();
    });

    // Set chest registrar callback so freshly-generated structure chests
    // are populated with randomized loot in the server's chestStorage_.
    // Java reference: WeightedRandomChestContent.generateChestContents()
    overworld->setChestRegistrar([this](Chunk& chunk) {
        for (auto& info : chunk.pendingChests) {
            // Seed RNG from chest position for deterministic loot
            NoiseGeneratorImproved::RNG rng;
            int64_t posSeed = static_cast<int64_t>(info.x) * 341873128712LL +
                              static_cast<int64_t>(info.y) * 28457361 +
                              static_cast<int64_t>(info.z) * 132897987541LL;
            rng.setSeed(posSeed);

            auto contents = generateChestContents(info.lootTable, rng);

            int64_t pk = packBlockPos(info.x, info.y, info.z);
            std::lock_guard<std::mutex> lock(chestMutex_);
            chestStorage_[pk] = std::move(contents);
        }
        chunk.pendingChests.clear();
    });

    // Set villager registrar callback so freshly-generated village buildings
    // spawn villager entities with the correct profession.
    // Java reference: StructureVillagePieces$Village.spawnVillagers()
    overworld->setVillagerRegistrar([this](Chunk& chunk) {
        for (auto& info : chunk.pendingVillagers) {
            int32_t eid = summonMob(120, info.x + 0.5, info.y, info.z + 0.5);
            if (eid >= 0) {
                // Set villager profession in SpawnedMob storage
                {
                    std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
                    for (auto& mob : mobEntities_) {
                        if (mob.entityId == eid) {
                            mob.villagerProfession = info.profession;
                            break;
                        }
                    }
                }
                // Broadcast updated profession via DataWatcher (DW 16 int)
                if (info.profession != 0) {
                    auto dw16Pkt = PacketBuilder::entityMetadataInt(eid, 16, info.profession);
                    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
                    for (auto& conn : connections_) {
                        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                        conn->sendPacket(dw16Pkt);
                    }
                }
            }
        }
        chunk.pendingVillagers.clear();
    });

    worlds_.push_back(std::move(overworld));

    // Create Nether world (dimension -1)
    // Java reference: MinecraftServer creates WorldServer for each dimension
    auto nether = std::make_unique<WorldServer>(-1, "world_nether");
    nether->initialize();
    worlds_.push_back(std::move(nether));

    // Create End world (dimension 1)
    // Java reference: MinecraftServer creates WorldServer for The End
    auto end = std::make_unique<WorldServer>(1, "world_the_end");
    end->initialize();
    worlds_.push_back(std::move(end));

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
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    return static_cast<int>(std::count_if(connections_.begin(), connections_.end(),
        [](const auto& c) { return c->getState() == ConnectionState::Play; }));
}

WorldServer* MinecraftServer::getWorldForDimension(int dim) {
    for (auto& w : worlds_) {
        if (w->getDimensionId() == dim) return w.get();
    }
    return nullptr;
}

void MinecraftServer::addConnection(std::shared_ptr<Connection> conn) {
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    connections_.push_back(std::move(conn));
}

void MinecraftServer::removeConnection(Connection* conn) {
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
            std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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

        // ─── Thunderstorm lightning strikes — Java: WorldServer.func_147456_g() line 225 ───
        // For each active chunk: 1/100000 chance per tick during thunderstorm to spawn lightning
        // Java: if (rand.nextInt(100000) == 0 && isRaining() && isThundering())
        auto* world = worlds_[0].get();
        if (world->isRaining() && world->isThundering() &&
            world->getRainingStrength() > 0.0f && world->getThunderingStrength() > 0.0f) {
            // Approximate Java's per-active-chunk iteration:
            //   Java iterates ~441 chunks (21×21 view), each with 1/100000 chance.
            //   We sample around each player's loaded chunk area.
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (!ph) continue;

                // ~441 active chunks per player (21×21 view distance)
                // Each chunk has 1/100000 chance → ~441/100000 ≈ 0.44% per tick per player
                // Simplify: roll once with adjusted probability
                int32_t activeChunks = 441; // 21×21 default view
                // For each virtual chunk, roll 1/100000
                for (int32_t c = 0; c < activeChunks; ++c) {
                    if (rand() % 100000 != 0) continue;

                    // Pick random position near player — Java uses LCG for XZ within chunk
                    double px = ph->getPlayerX();
                    double pz = ph->getPlayerZ();
                    // Random chunk offset within view distance (±10 chunks)
                    int32_t chunkOffX = (rand() % 21) - 10;
                    int32_t chunkOffZ = (rand() % 21) - 10;
                    int32_t chunkX = static_cast<int32_t>(std::floor(px / 16.0)) + chunkOffX;
                    int32_t chunkZ = static_cast<int32_t>(std::floor(pz / 16.0)) + chunkOffZ;
                    // Random XZ within chunk (0-15)
                    int32_t strikeX = chunkX * 16 + (rand() % 16);
                    int32_t strikeZ = chunkZ * 16 + (rand() % 16);

                    // getPrecipitationHeight — find highest non-air block
                    int32_t strikeY = 0;
                    for (int32_t y = 255; y >= 0; --y) {
                        Block* blk = world->getBlock(strikeX, y, strikeZ);
                        int32_t bid = blk ? Block::getIdFromBlock(blk) : 0;
                        if (bid != 0) {
                            strikeY = y + 1;
                            break;
                        }
                    }

                    spawnLightning(static_cast<double>(strikeX),
                                   static_cast<double>(strikeY),
                                   static_cast<double>(strikeZ));
                    break; // One strike per player per tick max
                }
            }
        }
    }

    // Tick item entities (physics, despawn, pickup)
    tickItemEntities();
    tickFurnaces();
    tickHoppers();
    tickBrewingStands();
    tickMobSpawners();
    tickScheduledBlocks();

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
    tickArrows();
    tickThrowables();
    tickFishHooks();
    tickMinecarts();
    tickLightning();
    tickBoats();
    tickXPOrbs();

    // Natural mob spawning every 200 ticks (10 seconds)
    // Java reference: WorldServer.tick() → SpawnerAnimals.findChunksForSpawning()
    if (ticks > 0 && ticks % 200 == 0) {
        spawnNaturalMobs();
    }

    // Passive mob spawning every 400 ticks (20 seconds)
    // Java: SpawnerAnimals.findChunksForSpawning() for EnumCreatureType.creature
    if (ticks > 0 && ticks % 400 == 0) {
        spawnPassiveMobs();
    }

    // Tick food/hunger for all play-state players
    // Java reference: EntityPlayer.onUpdate() → FoodStats.onUpdate()
    {
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* play = dynamic_cast<PlayHandler*>(handler.get());
            if (play) {
                play->tickFood(*conn);
                play->tickPotionEffects(*conn);
                play->tickPortal(*conn);
            }
        }
    }

    // Send S03 TimeUpdate to all players every 20 ticks (1 second)
    // Java reference: WorldServer.tick() sends S03PacketTimeUpdate every 20 ticks
    if (ticks > 0 && ticks % 20 == 0 && !worlds_.empty()) {
        WorldServer* world = worlds_[0].get();
        int64_t totalWorldTime = world->getTotalWorldTime();
        int64_t worldTime = world->getWorldTime();

        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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

    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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

    // ─── Send current scoreboard state to joining player ─────────────────
    // Java: ServerConfigurationManager.playerLoggedIn() → sends scoreboard packets
    sendScoreboardState(joinedConn);
}

void MinecraftServer::onPlayerLeft(PlayHandler& leftHandler) {
    // Java reference: ServerConfigurationManager.playerLoggedOut()
    // Broadcast DestroyEntities + PlayerListItem(offline) to all remaining players
    std::vector<int32_t> destroyIds = { leftHandler.getEntityId() };

    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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

    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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

    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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

    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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

    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto copy = pkt;
        conn->sendPacket(std::move(copy));
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Scoreboard packet broadcasting — Java: ServerScoreboard
// ═══════════════════════════════════════════════════════════════════════════

// Helper lambdas (reused across all scoreboard packet builders)
namespace {
    void sbWriteVarInt(std::vector<uint8_t>& buf, int32_t value) {
        uint32_t uval = static_cast<uint32_t>(value);
        do {
            uint8_t b = uval & 0x7F;
            uval >>= 7;
            if (uval != 0) b |= 0x80;
            buf.push_back(b);
        } while (uval != 0);
    }
    void sbWriteString(std::vector<uint8_t>& buf, const std::string& s) {
        sbWriteVarInt(buf, static_cast<int32_t>(s.size()));
        buf.insert(buf.end(), s.begin(), s.end());
    }
    void sbWriteInt(std::vector<uint8_t>& buf, int32_t value) {
        buf.push_back((value >> 24) & 0xFF);
        buf.push_back((value >> 16) & 0xFF);
        buf.push_back((value >> 8) & 0xFF);
        buf.push_back(value & 0xFF);
    }
    void sbWriteShort(std::vector<uint8_t>& buf, int16_t value) {
        buf.push_back((value >> 8) & 0xFF);
        buf.push_back(value & 0xFF);
    }
    void sbWriteByte(std::vector<uint8_t>& buf, int8_t value) {
        buf.push_back(static_cast<uint8_t>(value));
    }

    // Build S3B Scoreboard Objective packet
    std::vector<uint8_t> buildS3B(const std::string& objName, const std::string& displayName, int8_t mode) {
        std::vector<uint8_t> pkt;
        sbWriteVarInt(pkt, 0x3B);
        sbWriteString(pkt, objName);
        sbWriteString(pkt, displayName);
        sbWriteByte(pkt, mode);
        return pkt;
    }

    // Build S3C Update Score packet (create/update)
    std::vector<uint8_t> buildS3CUpdate(const std::string& itemName, const std::string& objName, int32_t value) {
        std::vector<uint8_t> pkt;
        sbWriteVarInt(pkt, 0x3C);
        sbWriteString(pkt, itemName);
        sbWriteByte(pkt, 0); // action = update
        sbWriteString(pkt, objName);
        sbWriteInt(pkt, value); // Java: writeInt, not VarInt
        return pkt;
    }

    // Build S3C Update Score packet (remove all scores for a player)
    // Java: S3CPacketUpdateScore(String) — action=1, no objective/score fields written
    std::vector<uint8_t> buildS3CRemove(const std::string& itemName) {
        std::vector<uint8_t> pkt;
        sbWriteVarInt(pkt, 0x3C);
        sbWriteString(pkt, itemName);
        sbWriteByte(pkt, 1); // action = remove — nothing else written
        return pkt;
    }

    // Build S3D Display Scoreboard packet
    std::vector<uint8_t> buildS3D(int8_t position, const std::string& objName) {
        std::vector<uint8_t> pkt;
        sbWriteVarInt(pkt, 0x3D);
        sbWriteByte(pkt, position);
        sbWriteString(pkt, objName);
        return pkt;
    }

    // Build S3E Teams packet
    std::vector<uint8_t> buildS3E(const std::string& teamName, int8_t mode,
                                   const std::string& displayName = "",
                                   const std::string& prefix = "",
                                   const std::string& suffix = "",
                                   int8_t friendlyFire = 3,
                                   const std::vector<std::string>& players = {}) {
        std::vector<uint8_t> pkt;
        sbWriteVarInt(pkt, 0x3E);
        sbWriteString(pkt, teamName);
        sbWriteByte(pkt, mode);

        // Mode 0 (create) and 2 (update) include team info
        if (mode == 0 || mode == 2) {
            sbWriteString(pkt, displayName);
            sbWriteString(pkt, prefix);
            sbWriteString(pkt, suffix);
            sbWriteByte(pkt, friendlyFire);
        }

        // Mode 0 (create), 3 (add players), 4 (remove players) include player list
        if (mode == 0 || mode == 3 || mode == 4) {
            sbWriteShort(pkt, static_cast<int16_t>(players.size()));
            for (const auto& p : players) {
                sbWriteString(pkt, p);
            }
        }

        return pkt;
    }
} // anonymous namespace

void MinecraftServer::broadcastScoreboardObjective(const std::string& objName,
                                                    const std::string& displayName,
                                                    int8_t mode) {
    auto pkt = buildS3B(objName, displayName, mode);
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto copy = pkt;
        conn->sendPacket(std::move(copy));
    }
}

void MinecraftServer::broadcastUpdateScore(const std::string& playerName,
                                            const std::string& objName,
                                            int32_t value, int8_t action) {
    std::vector<uint8_t> pkt;
    if (action == 0) {
        pkt = buildS3CUpdate(playerName, objName, value);
    } else {
        pkt = buildS3CRemove(playerName);
    }
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto copy = pkt;
        conn->sendPacket(std::move(copy));
    }
}

void MinecraftServer::broadcastRemoveScore(const std::string& playerName) {
    auto pkt = buildS3CRemove(playerName);
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto copy = pkt;
        conn->sendPacket(std::move(copy));
    }
}

void MinecraftServer::broadcastDisplayScoreboard(int8_t position, const std::string& objName) {
    auto pkt = buildS3D(position, objName);
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto copy = pkt;
        conn->sendPacket(std::move(copy));
    }
}

void MinecraftServer::broadcastTeams(const ScorePlayerTeam& team, int8_t mode,
                                      const std::vector<std::string>& players) {
    // For mode 0 (create) and 2 (update), include team info
    // For mode 0, 3, 4, include player list
    std::vector<std::string> playerList;
    if (mode == 0) {
        // Create: send all current members
        playerList.assign(team.members.begin(), team.members.end());
    } else if (mode == 3 || mode == 4) {
        // Add/Remove: send the specified players
        playerList = players;
    }

    auto pkt = buildS3E(team.registeredName, mode,
                         team.displayName, team.prefix, team.suffix,
                         static_cast<int8_t>(team.getFriendlyFlags()),
                         playerList);

    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto copy = pkt;
        conn->sendPacket(std::move(copy));
    }
}

void MinecraftServer::sendScoreboardState(Connection& conn) {
    // Java: ServerScoreboard.func_96549_e — send all objectives, scores, display slots, teams
    // to a single joining player.

    // 1. Send all objectives (S3B mode=0 create) + their scores (S3C)
    auto objNames = scoreboard_.getObjectiveNames();
    for (const auto& name : objNames) {
        const ScoreObjective* obj = scoreboard_.getObjective(name);
        if (!obj) continue;

        // S3B create
        auto pktObj = buildS3B(obj->name, obj->displayName, 0);
        conn.sendPacket(std::move(pktObj));

        // S3C for all scores in this objective
        auto scores = scoreboard_.getSortedScores(name);
        for (const auto& score : scores) {
            auto pktScore = buildS3CUpdate(score.playerName, score.objectiveName, score.points);
            conn.sendPacket(std::move(pktScore));
        }
    }

    // 2. Send display slots (S3D)
    for (int8_t slot = 0; slot < 3; ++slot) {
        std::string dispObj = scoreboard_.getDisplaySlot(slot);
        if (!dispObj.empty()) {
            auto pktDisp = buildS3D(slot, dispObj);
            conn.sendPacket(std::move(pktDisp));
        }
    }

    // 3. Send all teams (S3E mode=0 create)
    auto teamNames = scoreboard_.getTeamNames();
    for (const auto& name : teamNames) {
        const ScorePlayerTeam* team = scoreboard_.getTeam(name);
        if (!team) continue;

        std::vector<std::string> members(team->members.begin(), team->members.end());
        auto pktTeam = buildS3E(team->registeredName, 0,
                                 team->displayName, team->prefix, team->suffix,
                                 static_cast<int8_t>(team->getFriendlyFlags()),
                                 members);
        conn.sendPacket(std::move(pktTeam));
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

MinecraftServer::SpawnerData& MinecraftServer::getOrCreateSpawner(int64_t posKey) {
    std::lock_guard<std::mutex> lock(spawnerMutex_);
    return spawnerStorage_[posKey];
}

// ═══════════════════════════════════════════════════════════════════════════
// Mob Spawner tile entity ticking — Java: MobSpawnerBaseLogic.updateSpawner()
// Spawns mobs when a player is within activatingRange and spawnDelay reaches 0.
// ═══════════════════════════════════════════════════════════════════════════
void MinecraftServer::tickMobSpawners() {
    static thread_local std::mt19937 rng(std::random_device{}());

    // Snapshot spawner entries under lock
    std::vector<std::pair<int64_t, SpawnerData*>> snapshot;
    {
        std::lock_guard<std::mutex> lock(spawnerMutex_);
        snapshot.reserve(spawnerStorage_.size());
        for (auto& [pk, sd] : spawnerStorage_) {
            snapshot.emplace_back(pk, &sd);
        }
    }

    if (snapshot.empty()) return;

    // Unpack a block position key
    auto unpackBlockPos = [](int64_t packed, int32_t& x, int32_t& y, int32_t& z) {
        x = static_cast<int32_t>(packed >> 40);
        z = static_cast<int32_t>((packed >> 20) & 0xFFFFF);
        y = static_cast<int32_t>(packed & 0xFFFFF);
        if (z & 0x80000) z |= static_cast<int32_t>(0xFFF00000);
        if (y & 0x80000) y |= static_cast<int32_t>(0xFFF00000);
    };

    // Collect player positions for range check
    std::vector<std::tuple<double, double, double>> playerPositions;
    {
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (!ph) continue;
            playerPositions.emplace_back(ph->getPlayerX(), ph->getPlayerY(), ph->getPlayerZ());
        }
    }

    for (auto& [pk, sd] : snapshot) {
        int32_t bx, by, bz;
        unpackBlockPos(pk, bx, by, bz);
        double spawnerX = static_cast<double>(bx) + 0.5;
        double spawnerY = static_cast<double>(by);
        double spawnerZ = static_cast<double>(bz) + 0.5;

        // Java: MobSpawnerBaseLogic.updateSpawner() — skip if no player within range
        bool playerInRange = false;
        double range = static_cast<double>(sd->activatingRange);
        for (auto& [px, py, pz] : playerPositions) {
            double dx = px - spawnerX;
            double dy = py - spawnerY;
            double dz = pz - spawnerZ;
            if (dx * dx + dy * dy + dz * dz <= range * range) {
                playerInRange = true;
                break;
            }
        }
        if (!playerInRange) continue;

        // Decrement spawn delay
        if (sd->spawnDelay > 0) {
            --sd->spawnDelay;
            continue;
        }

        // Resolve entity type ID from name
        int32_t entityTypeId = EntityList::getIdByName(sd->entityId);
        if (entityTypeId == 0) {
            // Unknown entity — reset delay and skip
            sd->spawnDelay = sd->maxDelay;
            continue;
        }

        // Count nearby entities of same type for maxNearbyEntities cap
        // Java: MobSpawnerBaseLogic checks AABB around spawner ±spawnRange
        int nearbyCount = 0;
        {
            std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
            for (auto& mob : mobEntities_) {
                if (mob.isDead) continue;
                if (mob.mobType != static_cast<uint8_t>(entityTypeId)) continue;
                double dx = mob.posX - spawnerX;
                double dy = mob.posY - spawnerY;
                double dz = mob.posZ - spawnerZ;
                double dist2 = dx * dx + dy * dy + dz * dz;
                double cap = static_cast<double>(sd->spawnRange) * 2.0;
                if (dist2 <= cap * cap) {
                    ++nearbyCount;
                }
            }
        }

        if (nearbyCount >= sd->maxNearby) {
            // Too many nearby — reset delay
            std::uniform_int_distribution<int16_t> delayDist(sd->minDelay, sd->maxDelay);
            sd->spawnDelay = delayDist(rng);
            continue;
        }

        // Spawn spawnCount entities — Java: MobSpawnerBaseLogic loop
        bool spawned = false;
        for (int16_t s = 0; s < sd->spawnCount; ++s) {
            // Random position in spawnRange — Java: nextDouble() * spawnRange * 2 - spawnRange
            std::uniform_real_distribution<double> posDist(
                -static_cast<double>(sd->spawnRange),
                 static_cast<double>(sd->spawnRange));
            double sx = spawnerX + posDist(rng);
            double sy = spawnerY + std::uniform_real_distribution<double>(-1.0, 1.0)(rng);
            double sz = spawnerZ + posDist(rng);

            // Basic ground check: don't spawn in solid blocks
            int32_t checkX = static_cast<int32_t>(std::floor(sx));
            int32_t checkY = static_cast<int32_t>(std::floor(sy));
            int32_t checkZ = static_cast<int32_t>(std::floor(sz));
            if (!worlds_.empty()) {
                auto* block = worlds_[0]->getBlock(checkX, checkY, checkZ);
                if (block && Block::getIdFromBlock(block) != 0) continue; // Solid
                auto* above = worlds_[0]->getBlock(checkX, checkY + 1, checkZ);
                if (above && Block::getIdFromBlock(above) != 0) continue; // Head blocked
            }

            summonMob(static_cast<uint8_t>(entityTypeId), sx, sy, sz);
            spawned = true;
        }

        // Reset delay — Java: spawnDelay = randBetween(minDelay, maxDelay)
        std::uniform_int_distribution<int16_t> delayDist(sd->minDelay, sd->maxDelay);
        sd->spawnDelay = delayDist(rng);

        // Spawn event particles if anything spawned
        if (spawned) {
            // Java: S2A particle packet around spawner (flame + smoke)
            // TODO: send S2A particle packets to nearby players
        }
    }
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

// ═══════════════════════════════════════════════════════════════════════════
// Brewing stand automation — Java: TileEntityBrewingStand.updateEntity()
//
// Simplified potion brewing using lookup table for common recipes.
// Brew time = 400 ticks (20 seconds). Ingredient slot 3, potion slots 0-2.
// ═══════════════════════════════════════════════════════════════════════════

// Java: PotionHelper.applyIngredient — simplified lookup
static int applyPotionIngredient(int potionMeta, int ingredientItemId) {
    // Water bottle (meta 0) + ingredient → base potion
    if (potionMeta == 0) {
        switch (ingredientItemId) {
            case 372: return 16;   // Nether wart → Awkward
            case 331: return 64;   // Redstone → Mundane (extended)
            case 348: return 32;   // Glowstone → Thick
            case 376: return 8200; // Fermented spider eye → Weakness
            default: return potionMeta;
        }
    }
    // Awkward potion (meta 16) + ingredient → effect potion
    if (potionMeta == 16) {
        switch (ingredientItemId) {
            case 370: return 8193; // Ghast tear → Regeneration
            case 377: return 8201; // Blaze powder → Strength
            case 375: return 8196; // Spider eye → Poison
            case 382: return 8197; // Glistering melon → Healing
            case 378: return 8195; // Magma cream → Fire Resistance
            case 353: return 8194; // Sugar → Swiftness
            case 396: return 8198; // Golden carrot → Night Vision
            default: return potionMeta;
        }
    }
    // Extend with redstone (add bit 6 = 0x40)
    if (ingredientItemId == 331 && !(potionMeta & 0x40)) return potionMeta | 0x40;
    // Amplify with glowstone (add bit 5 = 0x20)
    if (ingredientItemId == 348 && !(potionMeta & 0x20)) return potionMeta | 0x20;
    // Splash with gunpowder (add bit 14 = 0x4000)
    if (ingredientItemId == 289 && !(potionMeta & 0x4000)) return potionMeta | 0x4000;
    // Fermented spider eye corrupts
    if (ingredientItemId == 376) {
        int base = potionMeta & 0x000F;
        if (base == 5) return (potionMeta & ~0x000F) | 12; // Healing → Harming
        if (base == 4) return (potionMeta & ~0x000F) | 12; // Poison → Harming
        if (base == 2) return (potionMeta & ~0x000F) | 10; // Swiftness → Slowness
        if (base == 3) return (potionMeta & ~0x000F) | 10; // Fire Res → Slowness
        if (base == 6) return (potionMeta & ~0x000F) | 14; // Night Vision → Invisibility
    }
    return potionMeta;
}

static bool isPotionIngredient(int itemId) {
    switch (itemId) {
        case 372: case 348: case 331: case 376: case 375: case 382:
        case 370: case 377: case 378: case 353: case 396: case 289:
            return true;
        default: return false;
    }
}

void MinecraftServer::tickBrewingStands() {
    std::lock_guard<std::mutex> lock(brewingStandMutex_);
    for (auto& [posKey, brew] : brewingStandStorage_) {
        auto& ingredient = brew.slots[3];
        auto canBrew = [&]() -> bool {
            if (!ingredient || ingredient->isEmpty()) return false;
            if (!isPotionIngredient(ingredient->getItemId())) return false;
            for (int i = 0; i < 3; ++i) {
                if (!brew.slots[i] || brew.slots[i]->getItemId() != 373) continue;
                int meta = brew.slots[i]->getDamage();
                if (applyPotionIngredient(meta, ingredient->getItemId()) != meta) return true;
            }
            return false;
        };
        if (brew.brewTime > 0) {
            --brew.brewTime;
            if (brew.brewTime == 0) {
                if (canBrew()) {
                    for (int i = 0; i < 3; ++i) {
                        if (!brew.slots[i] || brew.slots[i]->getItemId() != 373) continue;
                        int meta = brew.slots[i]->getDamage();
                        int newMeta = applyPotionIngredient(meta, ingredient->getItemId());
                        if (newMeta != meta) brew.slots[i]->setDamage(newMeta);
                    }
                    ingredient->setStackSize(ingredient->getStackSize() - 1);
                    if (ingredient->getStackSize() <= 0) ingredient.reset();
                }
            } else if (!canBrew()) {
                brew.brewTime = 0;
            } else if (ingredient && ingredient->getItemId() != brew.ingredientId) {
                brew.brewTime = 0;
            }
        } else if (canBrew()) {
            brew.brewTime = 400;
            brew.ingredientId = ingredient ? ingredient->getItemId() : 0;
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
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
                // Java: applyArmorCalculations + applyPotionDamageCalculations
                // Armor absorption
                int32_t armorValue = play->getTotalArmorValue();
                float afterArmor = damage;
                if (armorValue > 0) {
                    float reduction = damage * (1.0f - std::max(armorValue / 5.0f, armorValue - damage / 2.0f) / 25.0f);
                    afterArmor = std::max(damage - reduction, damage * 0.2f);
                }
                // Blast Protection (damageType=3)
                int32_t blastProt = play->getEnchantmentProtectionModifier(3);
                if (blastProt > 0) {
                    afterArmor *= (1.0f - std::min(blastProt, 20) * 0.04f);
                }

                play->applyDamage(afterArmor);
                play->sendUpdateHealth(*conn, play->getHealth(),
                    play->getFood(), play->getSaturation());
                broadcastEntityEvent(play->getEntityId(), 2); // Hurt animation
                broadcastSound("game.player.hurt", px, py, pz, 1.0f, 1.0f);

                if (play->getHealth() <= 0.0f) {
                    broadcastEntityEvent(play->getEntityId(), 3);
                    broadcastChatMessage(play->getPlayerName() + " was blown up");
                }
            }

            // Knockback velocity — Java: EnchantmentProtection.func_92092_a
            // Blast Protection reduces explosion knockback by level * 0.15
            double knockback = impact;
            int32_t bpLevel = 0;
            for (int16_t slot = 1; slot <= 4; ++slot) {
                auto arm = play->getArmorItem(slot);
                if (arm && arm->hasEnchantments()) {
                    int16_t bp = arm->getEnchantmentLevel(3);
                    bpLevel += bp;
                }
            }
            if (bpLevel > 0) {
                knockback *= std::max(0.0, 1.0 - bpLevel * 0.15);
            }
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

            // TNT chain reaction — Java: BlockTNT.onBlockDestroyedByExplosion
            // Spawn EntityTNTPrimed with shortened fuse; simplified: instant chain explosion
            if (blockId == 46) {
                worlds_[0]->setBlock(bx, by, bz, nullptr);
                broadcastBlockChange(bx, by, bz, 0, 0);
                // Java: fuse = rand.nextInt(fuse/4) + fuse/8 ≈ 10-30 ticks
                // Simplified: immediate chain detonation with primed sound
                broadcastSound("game.tnt.primed",
                    static_cast<double>(bx) + 0.5, static_cast<double>(by) + 0.5,
                    static_cast<double>(bz) + 0.5, 1.0f, 1.0f);
                createExplosion(
                    static_cast<double>(bx) + 0.5,
                    static_cast<double>(by) + 0.5,
                    static_cast<double>(bz) + 0.5,
                    4.0f, true, true);
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
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    // We inline the chunk-save loop here (instead of delegating to WorldServer::saveAllChunks)
    // so that we can inject tile entity NBT into each chunk's Level compound before serialization.
    std::cout << "[Server] Saving world data...\n";

    for (auto& world : worlds_) {
        if (!world) continue;

        auto* provider = world->getChunkProvider();
        if (!provider) continue;

        auto chunks = provider->getLoadedChunks();
        if (chunks.empty()) continue;

        std::string worldDir = world->getWorldName();
        std::string regionDir = worldDir + "/region";
        std::filesystem::create_directories(regionDir);

        // Cache open region files by region key
        std::unordered_map<int64_t, std::unique_ptr<RegionFile>> regionCache;
        auto getRegion = [&](int32_t chunkX, int32_t chunkZ) -> RegionFile* {
            int32_t regionX = chunkX >> 5;
            int32_t regionZ = chunkZ >> 5;
            int64_t key = (static_cast<int64_t>(regionX) << 32) | (static_cast<uint32_t>(regionZ));
            auto it = regionCache.find(key);
            if (it != regionCache.end()) return it->second.get();
            std::string path = regionDir + "/r." + std::to_string(regionX) + "." + std::to_string(regionZ) + ".mca";
            auto region = std::make_unique<RegionFile>(path);
            RegionFile* ptr = region.get();
            regionCache[key] = std::move(region);
            return ptr;
        };

        int saved = 0;
        for (Chunk* chunk : chunks) {
            if (!chunk || !chunk->needsSaving()) continue;

            // Serialize chunk to NBT
            auto nbtTag = chunk->writeToNBT();
            if (!nbtTag) continue;

            // ── Inject tile entity data into the Level compound ──
            saveTileEntitiesForChunk(*nbtTag, chunk->xPosition, chunk->zPosition);

            // Wrap in root compound with "Level" key (Anvil format)
            nbt::NBTTagCompound root;
            auto levelPtr = std::unique_ptr<nbt::NBTBase>(
                new nbt::NBTTagCompound(std::move(*nbtTag)));
            root.setTag("Level", std::move(levelPtr));

            // Serialize NBT to bytes
            auto bytes = nbt::serializeNBT(root);
            if (bytes.empty()) continue;

            // Write to region file (handles compression)
            RegionFile* region = getRegion(chunk->xPosition, chunk->zPosition);
            int32_t localX = chunk->xPosition & 31;
            int32_t localZ = chunk->zPosition & 31;
            if (region && region->writeChunkData(localX, localZ, bytes)) {
                chunk->isModified = false;
                ++saved;
            }
        }

        if (saved > 0) {
            std::cout << "[World] Saved " << saved << " chunk(s) for dimension "
                      << world->getDimensionId() << "\n";
        }
    }

    std::cout << "[Server] World data saved.\n";
}

// ─── Tile Entity Save ────────────────────────────────────────────────────
// Java reference: AnvilChunkLoader.writeChunkToNBT() — TileEntities section

void MinecraftServer::saveTileEntitiesForChunk(nbt::NBTTagCompound& levelTag,
                                                int chunkX, int chunkZ) {
    // Compute block coordinate range for this chunk
    int32_t minX = chunkX * 16;
    int32_t maxX = minX + 15;
    int32_t minZ = chunkZ * 16;
    int32_t maxZ = minZ + 15;

    auto tileEntities = std::make_unique<nbt::NBTTagList>();

    // Helper: unpack a block position key created by packBlockPos()
    auto unpackBlockPos = [](int64_t packed, int32_t& x, int32_t& y, int32_t& z) {
        x = static_cast<int32_t>(packed >> 40);
        z = static_cast<int32_t>((packed >> 20) & 0xFFFFF);
        y = static_cast<int32_t>(packed & 0xFFFFF);
        // Sign-extend the 20-bit fields
        if (z & 0x80000) z |= static_cast<int32_t>(0xFFF00000);
        if (y & 0x80000) y |= static_cast<int32_t>(0xFFF00000);
    };

    auto inChunk = [&](int32_t x, int32_t z) {
        return x >= minX && x <= maxX && z >= minZ && z <= maxZ;
    };

    // ── Chests ──
    {
        std::lock_guard<std::mutex> lock(chestMutex_);
        for (auto& [posKey, slots] : chestStorage_) {
            int32_t x, y, z;
            unpackBlockPos(posKey, x, y, z);
            if (!inChunk(x, z)) continue;

            auto te = std::make_unique<nbt::NBTTagCompound>();
            te->setString("id", "Chest");
            te->setInteger("x", x);
            te->setInteger("y", y);
            te->setInteger("z", z);

            auto items = std::make_unique<nbt::NBTTagList>();
            for (int8_t i = 0; i < 27; ++i) {
                if (slots[i]) {
                    items->appendTag(writeItemStackToNBT(*slots[i], i));
                }
            }
            te->setTag("Items", std::move(items));
            tileEntities->appendTag(std::move(te));
        }
    }

    // ── Furnaces ──
    {
        std::lock_guard<std::mutex> lock(furnaceMutex_);
        for (auto& [posKey, furnace] : furnaceStorage_) {
            int32_t x, y, z;
            unpackBlockPos(posKey, x, y, z);
            if (!inChunk(x, z)) continue;

            auto te = std::make_unique<nbt::NBTTagCompound>();
            te->setString("id", "Furnace");
            te->setInteger("x", x);
            te->setInteger("y", y);
            te->setInteger("z", z);
            te->setShort("BurnTime", static_cast<int16_t>(furnace.furnaceBurnTime));
            te->setShort("CookTime", static_cast<int16_t>(furnace.furnaceCookTime));

            auto items = std::make_unique<nbt::NBTTagList>();
            for (int8_t i = 0; i < 3; ++i) {
                if (furnace.slots[i]) {
                    items->appendTag(writeItemStackToNBT(*furnace.slots[i], i));
                }
            }
            te->setTag("Items", std::move(items));
            tileEntities->appendTag(std::move(te));
        }
    }

    // ── Dispensers / Droppers (Java tile entity ID = "Trap") ──
    {
        std::lock_guard<std::mutex> lock(dispenserMutex_);
        for (auto& [posKey, disp] : dispenserStorage_) {
            int32_t x, y, z;
            unpackBlockPos(posKey, x, y, z);
            if (!inChunk(x, z)) continue;

            auto te = std::make_unique<nbt::NBTTagCompound>();
            te->setString("id", "Trap");
            te->setInteger("x", x);
            te->setInteger("y", y);
            te->setInteger("z", z);

            auto items = std::make_unique<nbt::NBTTagList>();
            for (int8_t i = 0; i < 9; ++i) {
                if (disp.slots[i]) {
                    items->appendTag(writeItemStackToNBT(*disp.slots[i], i));
                }
            }
            te->setTag("Items", std::move(items));
            tileEntities->appendTag(std::move(te));
        }
    }

    // ── Hoppers ──
    {
        std::lock_guard<std::mutex> lock(hopperMutex_);
        for (auto& [posKey, hopper] : hopperStorage_) {
            int32_t x, y, z;
            unpackBlockPos(posKey, x, y, z);
            if (!inChunk(x, z)) continue;

            auto te = std::make_unique<nbt::NBTTagCompound>();
            te->setString("id", "Hopper");
            te->setInteger("x", x);
            te->setInteger("y", y);
            te->setInteger("z", z);
            te->setInteger("TransferCooldown", hopper.transferCooldown);

            auto items = std::make_unique<nbt::NBTTagList>();
            for (int8_t i = 0; i < 5; ++i) {
                if (hopper.slots[i]) {
                    items->appendTag(writeItemStackToNBT(*hopper.slots[i], i));
                }
            }
            te->setTag("Items", std::move(items));
            tileEntities->appendTag(std::move(te));
        }
    }

    // ── Brewing Stands (Java tile entity ID = "Cauldron") ──
    {
        std::lock_guard<std::mutex> lock(brewingStandMutex_);
        for (auto& [posKey, brew] : brewingStandStorage_) {
            int32_t x, y, z;
            unpackBlockPos(posKey, x, y, z);
            if (!inChunk(x, z)) continue;

            auto te = std::make_unique<nbt::NBTTagCompound>();
            te->setString("id", "Cauldron");
            te->setInteger("x", x);
            te->setInteger("y", y);
            te->setInteger("z", z);
            te->setInteger("BrewTime", brew.brewTime);

            auto items = std::make_unique<nbt::NBTTagList>();
            for (int8_t i = 0; i < 4; ++i) {
                if (brew.slots[i]) {
                    items->appendTag(writeItemStackToNBT(*brew.slots[i], i));
                }
            }
            te->setTag("Items", std::move(items));
            tileEntities->appendTag(std::move(te));
        }
    }

    // ── Signs ──
    {
        std::lock_guard<std::mutex> lock(signsMutex_);
        for (auto& [posKey, sign] : signs_) {
            int32_t x, y, z;
            unpackBlockPos(posKey, x, y, z);
            if (!inChunk(x, z)) continue;

            auto te = std::make_unique<nbt::NBTTagCompound>();
            te->setString("id", "Sign");
            te->setInteger("x", x);
            te->setInteger("y", y);
            te->setInteger("z", z);
            te->setString("Text1", sign.lines[0]);
            te->setString("Text2", sign.lines[1]);
            te->setString("Text3", sign.lines[2]);
            te->setString("Text4", sign.lines[3]);
            tileEntities->appendTag(std::move(te));
        }
    }

    // ── Mob Spawners ──
    {
        std::lock_guard<std::mutex> lock(spawnerMutex_);
        for (auto& [posKey, sd] : spawnerStorage_) {
            int32_t x, y, z;
            unpackBlockPos(posKey, x, y, z);
            if (!inChunk(x, z)) continue;

            auto te = std::make_unique<nbt::NBTTagCompound>();
            te->setString("id", "MobSpawner");
            te->setInteger("x", x);
            te->setInteger("y", y);
            te->setInteger("z", z);
            te->setString("EntityId", sd.entityId);
            te->setShort("Delay", sd.spawnDelay);
            te->setShort("MinSpawnDelay", sd.minDelay);
            te->setShort("MaxSpawnDelay", sd.maxDelay);
            te->setShort("SpawnCount", sd.spawnCount);
            te->setShort("MaxNearbyEntities", sd.maxNearby);
            te->setShort("RequiredPlayerRange", sd.activatingRange);
            te->setShort("SpawnRange", sd.spawnRange);
            tileEntities->appendTag(std::move(te));
        }
    }

    levelTag.setTag("TileEntities", std::move(tileEntities));
}

// ─── Tile Entity Load ────────────────────────────────────────────────────
// Java reference: AnvilChunkLoader.readChunkFromNBT() — TileEntities section

void MinecraftServer::loadTileEntitiesFromChunk(const nbt::NBTTagCompound& levelTag) {
    auto* teList = levelTag.getTagList("TileEntities",
                                        static_cast<int>(nbt::TagType::Compound));
    if (!teList || teList->tagCount() == 0) return;

    for (int32_t i = 0; i < teList->tagCount(); ++i) {
        auto* te = teList->getCompoundTagAt(i);
        if (!te) continue;

        std::string id = te->getString("id");
        int32_t x = te->getInteger("x");
        int32_t y = te->getInteger("y");
        int32_t z = te->getInteger("z");
        int64_t posKey = packBlockPos(x, y, z);

        if (id == "Chest" || id == "TrappedChest") {
            auto* items = te->getTagList("Items",
                                          static_cast<int>(nbt::TagType::Compound));
            if (!items) continue;

            std::lock_guard<std::mutex> lock(chestMutex_);
            auto& slots = chestStorage_[posKey];
            slots.fill(std::nullopt);
            for (int32_t j = 0; j < items->tagCount(); ++j) {
                auto* itemTag = items->getCompoundTagAt(j);
                if (!itemTag) continue;
                int8_t slot = itemTag->getByte("Slot");
                if (slot >= 0 && slot < 27) {
                    slots[slot] = readItemStackFromNBT(*itemTag);
                }
            }
        } else if (id == "Furnace") {
            auto* items = te->getTagList("Items",
                                          static_cast<int>(nbt::TagType::Compound));
            std::lock_guard<std::mutex> lock(furnaceMutex_);
            auto& furnace = furnaceStorage_[posKey];
            furnace.slots[0] = std::nullopt;
            furnace.slots[1] = std::nullopt;
            furnace.slots[2] = std::nullopt;
            furnace.furnaceBurnTime = static_cast<int32_t>(te->getShort("BurnTime"));
            furnace.furnaceCookTime = static_cast<int32_t>(te->getShort("CookTime"));
            if (items) {
                for (int32_t j = 0; j < items->tagCount(); ++j) {
                    auto* itemTag = items->getCompoundTagAt(j);
                    if (!itemTag) continue;
                    int8_t slot = itemTag->getByte("Slot");
                    if (slot >= 0 && slot < 3) {
                        furnace.slots[slot] = readItemStackFromNBT(*itemTag);
                    }
                }
            }
        } else if (id == "Trap") {
            // Dispenser / Dropper
            auto* items = te->getTagList("Items",
                                          static_cast<int>(nbt::TagType::Compound));
            if (!items) continue;

            std::lock_guard<std::mutex> lock(dispenserMutex_);
            auto& disp = dispenserStorage_[posKey];
            disp.slots.fill(std::nullopt);
            for (int32_t j = 0; j < items->tagCount(); ++j) {
                auto* itemTag = items->getCompoundTagAt(j);
                if (!itemTag) continue;
                int8_t slot = itemTag->getByte("Slot");
                if (slot >= 0 && slot < 9) {
                    disp.slots[slot] = readItemStackFromNBT(*itemTag);
                }
            }
        } else if (id == "Hopper") {
            auto* items = te->getTagList("Items",
                                          static_cast<int>(nbt::TagType::Compound));
            if (!items) continue;

            std::lock_guard<std::mutex> lock(hopperMutex_);
            auto& hopper = hopperStorage_[posKey];
            hopper.slots.fill(std::nullopt);
            hopper.transferCooldown = te->getInteger("TransferCooldown");
            for (int32_t j = 0; j < items->tagCount(); ++j) {
                auto* itemTag = items->getCompoundTagAt(j);
                if (!itemTag) continue;
                int8_t slot = itemTag->getByte("Slot");
                if (slot >= 0 && slot < 5) {
                    hopper.slots[slot] = readItemStackFromNBT(*itemTag);
                }
            }
        } else if (id == "Cauldron") {
            // Brewing Stand
            auto* items = te->getTagList("Items",
                                          static_cast<int>(nbt::TagType::Compound));
            std::lock_guard<std::mutex> lock(brewingStandMutex_);
            auto& brew = brewingStandStorage_[posKey];
            brew.slots.fill(std::nullopt);
            brew.brewTime = te->getInteger("BrewTime");
            if (items) {
                for (int32_t j = 0; j < items->tagCount(); ++j) {
                    auto* itemTag = items->getCompoundTagAt(j);
                    if (!itemTag) continue;
                    int8_t slot = itemTag->getByte("Slot");
                    if (slot >= 0 && slot < 4) {
                        brew.slots[slot] = readItemStackFromNBT(*itemTag);
                    }
                }
            }
        } else if (id == "Sign") {
            std::lock_guard<std::mutex> lock(signsMutex_);
            auto& sign = signs_[posKey];
            sign.lines[0] = te->getString("Text1");
            sign.lines[1] = te->getString("Text2");
            sign.lines[2] = te->getString("Text3");
            sign.lines[3] = te->getString("Text4");
        } else if (id == "MobSpawner") {
            std::lock_guard<std::mutex> lock(spawnerMutex_);
            auto& sd = spawnerStorage_[posKey];
            sd.entityId = te->getString("EntityId");
            if (sd.entityId.empty()) sd.entityId = "Pig"; // Java default
            sd.spawnDelay = te->getShort("Delay");
            sd.minDelay = te->getShort("MinSpawnDelay");
            sd.maxDelay = te->getShort("MaxSpawnDelay");
            sd.spawnCount = te->getShort("SpawnCount");
            sd.maxNearby = te->getShort("MaxNearbyEntities");
            sd.activatingRange = te->getShort("RequiredPlayerRange");
            sd.spawnRange = te->getShort("SpawnRange");
            // Apply Java defaults if values are zero (new/incomplete NBT)
            if (sd.minDelay == 0) sd.minDelay = 200;
            if (sd.maxDelay == 0) sd.maxDelay = 800;
            if (sd.spawnCount == 0) sd.spawnCount = 4;
            if (sd.maxNearby == 0) sd.maxNearby = 6;
            if (sd.activatingRange == 0) sd.activatingRange = 16;
            if (sd.spawnRange == 0) sd.spawnRange = 4;
        }
    }
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
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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

int32_t MinecraftServer::spawnItemDropStack(double x, double y, double z, const ItemStack& stack,
                                             double motionX, double motionY, double motionZ) {
    // Java reference: EntityPlayer.func_146097_a → entityDropItem → new EntityItem
    // Spawns a dropped item with full NBT (enchantments, custom name, etc.)
    if (stack.isEmpty() || stack.getItemId() == 0) return -1;

    int32_t eid = nextItemEntityId_.fetch_add(1, std::memory_order_relaxed);

    EntityItem entity;
    entity.entityId = eid;
    entity.itemId = stack.getItemId();
    entity.itemMeta = stack.getDamage();
    entity.stackSize = stack.getStackSize();
    // Java: EntityPlayer.func_146097_a — spawn at player pos, Y + 0.5 (eye height offset)
    entity.posX = x;
    entity.posY = y;
    entity.posZ = z;
    entity.motionX = motionX;
    entity.motionY = motionY;
    entity.motionZ = motionZ;
    entity.rotationYaw = static_cast<float>(std::rand() % 360);
    entity.onGround = false;
    entity.isDead = false;
    entity.age = 0;
    entity.delayBeforeCanPickup = 40;  // Java: EntityItem pickup delay = 40 ticks (2 seconds) for death drops

    // Broadcast spawn to all players
    {
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (conn->isConnected() && conn->getState() == ConnectionState::Play) {
                auto handler = conn->getHandler();
                auto* playHandler = dynamic_cast<PlayHandler*>(handler.get());
                if (playHandler) {
                    // S0E SpawnObject: type=2 (Item), data=1 (velocity present)
                    playHandler->sendSpawnObject(*conn, eid, 2,
                        entity.posX, entity.posY, entity.posZ,
                        entity.rotationYaw, 0.0f, 1,
                        entity.motionX, entity.motionY, entity.motionZ);
                    // S1C EntityMetadata with full ItemStack NBT at slot 10
                    playHandler->sendEntityMetadataItemStack(*conn, eid, stack);
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

        // Block collision: check if item has moved into a solid block
        // Java: EntityItem uses full AABB collision, we simplify to column check
        {
            int32_t bx = static_cast<int32_t>(std::floor(e.posX));
            int32_t by = static_cast<int32_t>(std::floor(e.posY));
            int32_t bz = static_cast<int32_t>(std::floor(e.posZ));

            // Check if the block at the item's feet is solid
            int32_t blockBelow = getBlockIdInWorld(bx, by, bz);
            if (blockBelow != 0 && blockBelow != 8 && blockBelow != 9 &&   // not air/water
                blockBelow != 10 && blockBelow != 11 &&                      // not lava
                blockBelow != 31 && blockBelow != 37 && blockBelow != 38 &&  // not tallgrass/flowers
                blockBelow != 6 && blockBelow != 32 && blockBelow != 106) {  // not saplings/deadbush/vines
                // Item is inside a solid block — push up to top
                e.posY = static_cast<double>(by + 1) + 0.01;
                e.onGround = true;
                e.motionY = 0;
            } else {
                // Check if block below feet is solid (for landing on top of blocks)
                int32_t blockUnder = getBlockIdInWorld(bx, by - 1, bz);
                if (e.motionY <= 0 && by >= 0 &&
                    blockUnder != 0 && blockUnder != 8 && blockUnder != 9 &&
                    blockUnder != 10 && blockUnder != 11 &&
                    blockUnder != 31 && blockUnder != 37 && blockUnder != 38 &&
                    blockUnder != 6 && blockUnder != 32 && blockUnder != 106) {
                    // Landing on top of a solid block
                    double blockTop = static_cast<double>(by);
                    if (e.posY <= blockTop + 0.01) {
                        e.posY = blockTop + 0.01;
                        e.onGround = true;
                        e.motionY = 0;
                    } else {
                        e.onGround = false;
                    }
                } else {
                    e.onGround = false;
                }
            }

            // Final safety: don't fall below Y=0
            if (e.posY < 0.5) {
                e.posY = 0.5;
                e.onGround = true;
                e.motionY = 0;
            }
        }

        if (result.shouldDie) {
            e.isDead = true;
            deadEntityIds.push_back(e.entityId);
            continue;
        }

        // Check for player pickup
        if (e.delayBeforeCanPickup == 0) {
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
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
        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* play = dynamic_cast<PlayHandler*>(handler.get());
        if (play) {
            play->sendEntityStatus(*conn, entityId, status);
        }
    }
}

void MinecraftServer::broadcastAttachEntity(int32_t leashId, int32_t riderId, int32_t vehicleId) {
    // S1B AttachEntity — broadcast to all players
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* play = dynamic_cast<PlayHandler*>(handler.get());
        if (play) {
            play->sendAttachEntity(*conn, leashId, riderId, vehicleId);
        }
    }
}

void MinecraftServer::broadcastAnimation(int32_t entityId, uint8_t animationType) {
    // Java reference: WorldServer entity.worldObj.setEntityState() for animation
    // Broadcasts S0B Animation to all players except the source entity
    auto pkt = PacketBuilder::animation(entityId, animationType);
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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

    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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

    if (!target || !targetConn || target->isDead()) {
        // ─── Player-vs-Minecart attack ───────────────────────────────
        // Check if the target is a minecart entity
        {
            std::lock_guard<std::mutex> cartLock(minecartEntitiesMutex_);
            for (auto& cart : minecartEntities_) {
                if (cart.entityId != targetEntityId || cart.isDead) continue;

                bool isCreative = (attacker.getGameMode() == 1);
                auto result = cart.logic.attackEntityFrom(attacker.getWeaponDamage(), isCreative, false);

                if (result.broken) {
                    cart.isDead = true;
                    // Drop minecart item in survival
                    if (result.dropItems) {
                        // Java: EntityMinecart.killMinecart → drop Items.minecart (328)
                        // Minecart type → item: 0→328, 1→342 (chest), 2→343 (furnace),
                        //   3→407 (TNT), 5→408 (hopper)
                        int32_t dropItemId = 328; // default: minecart
                        switch (cart.minecartType) {
                            case 1: dropItemId = 342; break; // chest minecart
                            case 2: dropItemId = 343; break; // furnace minecart
                            case 3: dropItemId = 407; break; // TNT minecart
                            case 5: dropItemId = 408; break; // hopper minecart
                            default: break;
                        }
                        spawnItemDrop(cart.logic.posX, cart.logic.posY, cart.logic.posZ,
                            dropItemId, 0, 1);
                    }

                    // Broadcast S13 DestroyEntities
                    std::vector<int32_t> destroyIds = { cart.entityId };
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    for (auto& conn : connections_) {
                        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                        auto handler = conn->getHandler();
                        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                        if (ph) ph->sendDestroyEntities(*conn, destroyIds);
                    }
                    std::cout << "[Minecart] Entity " << cart.entityId << " destroyed by " << attacker.getPlayerName() << "\n";
                }
                return; // Handled
            }
        }

        // ─── Player-vs-Boat attack ───────────────────────────────────
        // Java: EntityBoat.attackEntityFrom()
        {
            std::lock_guard<std::mutex> boatLock(boatEntitiesMutex_);
            for (auto& boat : boatEntities_) {
                if (boat.entityId != targetEntityId || boat.isDead) continue;

                bool isCreative = (attacker.getGameMode() == 1);

                if (isCreative) {
                    // Creative: instant kill
                    boat.isDead = true;
                } else {
                    // Java: EntityBoat.attackEntityFrom → setForwardDirection(-getForwardDirection())
                    boat.forwardDirection = -boat.forwardDirection;
                    // Java: setTimeSinceHit(10)
                    boat.timeSinceHit = 10;
                    // Java: setDamageTaken(getDamageTaken() + damage * 10)
                    float damage = attacker.getWeaponDamage();
                    boat.damageTaken += damage * 10.0f;

                    // Java: if getDamageTaken() > 40 → setDead + drop items
                    if (boat.damageTaken > 40.0f) {
                        boat.isDead = true;
                    }
                }

                if (boat.isDead) {
                    // Dismount rider + broadcast destroy — single lock
                    std::vector<int32_t> destroyIds = { boat.entityId };
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    for (auto& conn : connections_) {
                        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                        auto handler = conn->getHandler();
                        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                        if (!ph) continue;
                        // Dismount rider
                        if (boat.riderEntityId >= 0) {
                            if (ph->getEntityId() == boat.riderEntityId) {
                                ph->setRidingEntityId(-1);
                            }
                            ph->sendAttachEntity(*conn, 0, boat.riderEntityId, -1);
                        }
                        ph->sendDestroyEntities(*conn, destroyIds);
                    }

                    // Drop boat item in survival
                    if (!isCreative) {
                        spawnItemDrop(boat.posX, boat.posY, boat.posZ, 333, 0, 1);
                    }
                    std::cout << "[Boat] Entity " << boat.entityId << " destroyed by " << attacker.getPlayerName() << "\n";
                } else {
                    // Hurt animation — S19 EntityStatus byte 2
                    broadcastEntityEvent(boat.entityId, 2);
                }
                return;
            }
        }

        // ─── Player-vs-Mob attack ────────────────────────────────────
        // If not a player target, check if it's a mob entity
        std::lock_guard<std::mutex> mobLock(mobEntitiesMutex_);
        for (auto& mob : mobEntities_) {
            if (mob.entityId != targetEntityId || mob.isDead) continue;

            // Apply weapon damage (includes Sharpness from getWeaponDamage)
            float damage = attacker.getWeaponDamage();

            // ─── Smite (ID 17) — bonus vs undead ──────────────────
            // Java: EnchantmentDamage type 1 → level * 2.5f vs UNDEAD
            auto attackerWeapon = attacker.getHeldItem();
            if (attackerWeapon && attackerWeapon->hasEnchantments()) {
                int16_t smiteLevel = attackerWeapon->getEnchantmentLevel(17);
                if (smiteLevel > 0) {
                    // Undead: Zombie(54), Skeleton(51), Zombie Pigman(57), Wither(64)
                    if (mob.mobType == 54 || mob.mobType == 51 ||
                        mob.mobType == 57 || mob.mobType == 64) {
                        damage += smiteLevel * 2.5f;
                    }
                }
                // ─── Bane of Arthropods (ID 18) — bonus vs arthropods ──
                // Java: EnchantmentDamage type 2 → level * 2.5f vs ARTHROPOD
                int16_t baneLevel = attackerWeapon->getEnchantmentLevel(18);
                if (baneLevel > 0) {
                    // Arthropods: Spider(52), Cave Spider(59), Silverfish(60)
                    if (mob.mobType == 52 || mob.mobType == 59 || mob.mobType == 60) {
                        damage += baneLevel * 2.5f;
                    }
                }
            }

            // ─── Ender Dragon damage — Java: EntityDragon.attackEntityFromPart() ───
            // Dragon ignores generic attackEntityFrom; only takes player/explosion damage
            // Body hits: damage = f/4 + 1; Head hits: full damage
            // Since we use a single hitbox, treat all hits as body hits (reduced damage)
            if (mob.mobType == 63) {
                damage = damage / 4.0f + 1.0f;  // Java: EntityDragonPart != head → f/4+1
                // Force retarget after being hit — Java: EntityDragon.attackEntityFromPart()
                float yawRad = mob.yaw * static_cast<float>(M_PI) / 180.0f;
                mob.dragonTargetX = mob.posX + static_cast<double>(std::sin(yawRad) * 5.0f) + ((double)(rand() % 100) / 100.0 - 0.5) * 2.0;
                mob.dragonTargetY = mob.posY + ((double)(rand() % 100) / 100.0) * 3.0 + 1.0;
                mob.dragonTargetZ = mob.posZ - static_cast<double>(std::cos(yawRad) * 5.0f) + ((double)(rand() % 100) / 100.0 - 0.5) * 2.0;
                mob.dragonTargetEntityId = -1;  // Clear player target
            }

            // ─── Wither damage — Java: EntityWither.attackEntityFrom() ───
            if (mob.mobType == 64) {
                // Invulnerable during spawn sequence
                if (mob.witherInvulTime > 0) {
                    return;
                }
                // Java: isArmored() && source is arrow → return false
                // (arrows blocked when health <= 50%)
                // We treat all player melee as non-arrow; arrow hits handled separately
                // Java: field_82222_j = 20 — block break timer on damage
                if (mob.witherBlockBreakTimer <= 0) {
                    mob.witherBlockBreakTimer = 20;
                }
                // Java: field_82224_i[n] += 3 — accelerate charged shot counters
                for (int i = 0; i < 2; ++i) {
                    mob.witherSideChargeCounter[i] += 3;
                }
            }

            mob.health -= damage;

            // Hurt animation
            broadcastEntityEvent(targetEntityId, 2);

            // ─── Mob-specific hurt sounds ─────────────────────────────
            // Java: EntityLiving.attackEntityFrom() → getHurtSound()
            {
                const char* hurtSound = "game.hostile.hurt";
                float hurtVolume = 1.0f;
                switch (mob.mobType) {
                    case 50: hurtSound = "mob.creeper.say"; break;
                    case 51: hurtSound = "mob.skeleton.hurt"; break;
                    case 52: hurtSound = "mob.spider.say"; break;
                    case 54: hurtSound = "mob.zombie.hurt"; break;
                    case 55: hurtSound = "mob.slime.small"; break;
                    case 56: hurtSound = "mob.ghast.scream"; break;
                    case 57: hurtSound = "mob.zombiepig.zpighurt"; break;
                    case 58: hurtSound = "mob.endermen.hit"; break;
                    case 59: hurtSound = "mob.spider.say"; break;
                    case 60: hurtSound = "mob.silverfish.hit"; break;
                    case 61: hurtSound = "mob.blaze.hit"; break;
                    case 62: hurtSound = "mob.magmacube.small"; break;
                    case 63: hurtSound = "mob.enderdragon.hit"; hurtVolume = 5.0f; break;
                    case 64: hurtSound = "mob.wither.hurt"; break;
                    case 66: hurtSound = "mob.witch.hurt"; break;
                    case 92: hurtSound = "mob.cow.hurt"; break;
                    case 90: hurtSound = "mob.pig.say"; break;
                    case 91: hurtSound = "mob.sheep.say"; break;
                    case 93: hurtSound = "mob.chicken.hurt"; break;
                    case 95: hurtSound = "mob.wolf.hurt"; break;
                    case 97: break; // Snow Golem — no hurt sound in Java
                    case 98: hurtSound = "mob.cat.hitt"; break;  // Ocelot/Cat
                    case 99: hurtSound = "mob.irongolem.hit"; break;
                    case 100: { // Horse — Java: EntityHorse.getHurtSound()
                        int ht = mob.horseType;
                        if (ht == 3) hurtSound = "mob.horse.zombie.hit";
                        else if (ht == 4) hurtSound = "mob.horse.skeleton.hit";
                        else if (ht == 1 || ht == 2) hurtSound = "mob.horse.donkey.hit";
                        else hurtSound = "mob.horse.hit";
                        break;
                    }
                    default: break;
                }
                broadcastSound(hurtSound, mob.posX, mob.posY, mob.posZ, hurtVolume, 1.0f);
            }

            // ─── Enderman teleport on hit ────────────────────────────
            // Java: EntityEnderman.attackEntityFrom() → teleportRandomly()
            // Enderman tries up to 64 random teleport attempts when hit
            if (mob.mobType == 58) {
                for (int tp = 0; tp < 64; ++tp) {
                    double newX = mob.posX + ((double)rand() / RAND_MAX - 0.5) * 64.0;
                    double newY = mob.posY + (double)(rand() % 64 - 32);
                    double newZ = mob.posZ + ((double)rand() / RAND_MAX - 0.5) * 64.0;
                    int bx = static_cast<int>(std::floor(newX));
                    int by = static_cast<int>(std::floor(newY));
                    int bz = static_cast<int>(std::floor(newZ));
                    if (by < 1 || by > 250) continue;
                    // Find ground: descend until solid
                    while (by > 1 && getBlockIdInWorld(bx, by - 1, bz) == 0) --by;
                    // Check destination is clear (2 blocks of air)
                    if (getBlockIdInWorld(bx, by, bz) != 0 || getBlockIdInWorld(bx, by + 1, bz) != 0) continue;
                    // Teleport successful
                    broadcastSound("mob.endermen.portal", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
                    mob.posX = newX;
                    mob.posY = static_cast<double>(by);
                    mob.posZ = newZ;
                    broadcastSound("mob.endermen.portal", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
                    {
                        std::lock_guard<std::recursive_mutex> cl(connectionsMutex_);
                        for (auto& c : connections_) {
                            if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                            auto h = c->getHandler();
                            auto* p = dynamic_cast<PlayHandler*>(h.get());
                            if (p) p->sendEntityTeleport(*c, mob.entityId, mob.posX, mob.posY, mob.posZ, mob.yaw, 0.0f);
                        }
                    }
                    break;
                }
            }

            // ─── Zombie Pigman pack aggro ────────────────────────────
            // Java: EntityPigZombie.attackEntityFrom() → becomeAngryAt()
            // All pigmen within 32 blocks become angry at the attacker
            if (mob.mobType == 57) {
                mob.angerLevel = 400 + (rand() % 400);
                mob.angerTarget = attacker.getEntityId();
                broadcastSound("mob.zombiepig.zpigangry", mob.posX, mob.posY, mob.posZ, 2.0f, 1.8f);
                // Aggro nearby pigmen — Java: expand(32, 32, 32)
                for (auto& other : mobEntities_) {
                    if (&other == &mob || other.isDead || other.mobType != 57) continue;
                    double dx = other.posX - mob.posX;
                    double dy = other.posY - mob.posY;
                    double dz = other.posZ - mob.posZ;
                    if (dx * dx + dy * dy + dz * dz < 1024.0) { // 32 blocks squared
                        other.angerLevel = 400 + (rand() % 400);
                        other.angerTarget = attacker.getEntityId();
                    }
                }
            }

            // ─── Wolf owner-defense — Java: EntityAIOwnerHurtTarget ──
            // When a player attacks a mob, their standing tamed wolves target it
            {
                std::string attackerUuid = attacker.getUuid();
                for (auto& wolf : mobEntities_) {
                    if (wolf.isDead || wolf.mobType != 95) continue;
                    if (!wolf.isTamed || wolf.isSitting) continue;
                    if (wolf.ownerUuid != attackerUuid) continue;
                    double wdx = wolf.posX - mob.posX;
                    double wdz = wolf.posZ - mob.posZ;
                    if (wdx * wdx + wdz * wdz < 256.0) { // Within 16 blocks
                        wolf.wolfAttackTarget = targetEntityId;
                    }
                }
            }

            // Fire Aspect — set mob on fire for cooked drops
            auto attackerHeld = attacker.getHeldItem();
            if (attackerHeld && attackerHeld->hasEnchantments()) {
                int16_t faLevel = attackerHeld->getEnchantmentLevel(20);
                if (faLevel > 0) {
                    mob.isOnFire = true;
                }
            }

            // Durability + exhaustion
            attacker.damageHeldItem(1);
            attacker.getFoodStats().addExhaustion(0.3f);

            // Death check
            if (mob.health <= 0.0f) {
                // ─── Ender Dragon death — Java: EntityDragon.onDeathUpdate() ───
                // Dragon doesn't die immediately; it enters a 200-tick death sequence
                if (mob.mobType == 63 && mob.dragonDeathTicks == 0) {
                    mob.health = 0.0f;
                    mob.dragonDeathTicks = 1;  // Start death sequence
                    // Broadcast entity status 3 (death) — triggers client death animation
                    broadcastEntityEvent(targetEntityId, 3);
                    // Java: playBroadcastSound(1018, ...) — ender dragon death roar
                    broadcastSound("mob.enderdragon.end", mob.posX, mob.posY, mob.posZ, 5.0f, 1.0f);
                    std::cout << "[Dragon] Ender Dragon death sequence started at ("
                              << mob.posX << ", " << mob.posY << ", " << mob.posZ << ")\n";
                    // Don't mark isDead yet — death sequence handles that over 200 ticks
                    return;
                }

                mob.isDead = true;

                // Death animation
                broadcastEntityEvent(targetEntityId, 3);

                // ─── Mob-specific death sounds ─────────────────────────
                // Java: EntityLiving.onDeath() → getDeathSound()
                {
                    const char* deathSound = "game.hostile.die";
                    float deathVolume = 1.0f;
                    switch (mob.mobType) {
                        case 50: deathSound = "mob.creeper.death"; break;
                        case 51: deathSound = "mob.skeleton.death"; break;
                        case 52: deathSound = "mob.spider.death"; break;
                        case 54: deathSound = "mob.zombie.death"; break;
                        case 55: deathSound = "mob.slime.big"; break;
                        case 56: deathSound = "mob.ghast.death"; break;
                        case 57: deathSound = "mob.zombiepig.zpigdeath"; break;
                        case 58: deathSound = "mob.endermen.death"; break;
                        case 59: deathSound = "mob.spider.death"; break;
                        case 60: deathSound = "mob.silverfish.kill"; break;
                        case 61: deathSound = "mob.blaze.death"; break;
                        case 62: deathSound = "mob.magmacube.big"; break;
                        case 63: deathSound = "mob.enderdragon.end"; deathVolume = 5.0f; break;
                        case 64: deathSound = "mob.wither.death"; break;
                        case 66: deathSound = "mob.witch.death"; break;
                        case 92: deathSound = "mob.cow.hurt"; break;
                        case 90: deathSound = "mob.pig.death"; break;
                        case 91: deathSound = "mob.sheep.say"; break;
                        case 93: deathSound = "mob.chicken.hurt"; break;
                        case 95: deathSound = "mob.wolf.death"; break;
                        case 97: break; // Snow Golem — no death sound in Java
                        case 98: deathSound = "mob.cat.hitt"; break;  // Ocelot/Cat
                        case 99: deathSound = "mob.irongolem.death"; break;
                        case 100: { // Horse — Java: EntityHorse.getDeathSound()
                            int ht = mob.horseType;
                            if (ht == 3) deathSound = "mob.horse.zombie.death";
                            else if (ht == 4) deathSound = "mob.horse.skeleton.death";
                            else if (ht == 1 || ht == 2) deathSound = "mob.horse.donkey.death";
                            else deathSound = "mob.horse.death";
                            break;
                        }
                        default: break;
                    }
                    broadcastSound(deathSound, mob.posX, mob.posY, mob.posZ, deathVolume, 1.0f);
                }

                // Destroy entity
                {
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    std::vector<int32_t> dead = {targetEntityId};
                    for (auto& c : connections_) {
                        if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                        auto h = c->getHandler();
                        auto* ph = dynamic_cast<PlayHandler*>(h.get());
                        if (ph) ph->sendDestroyEntities(*c, dead);
                    }
                }

                // ─── Mob drops — Java: EntityLiving.dropFewItems() ─────
                // Looting enchantment (ID 21) adds extra drops
                int32_t lootingLevel = 0;
                if (attackerHeld && attackerHeld->hasEnchantments()) {
                    lootingLevel = attackerHeld->getEnchantmentLevel(21);
                }

                // Java: EntityLiving.getExperiencePoints() — per mob type
                auto getMobXp = [](uint8_t mobType) -> int32_t {
                    switch (mobType) {
                        case 54: return 5;   // Zombie
                        case 51: return 5;   // Skeleton
                        case 50: return 5;   // Creeper
                        case 52: return 5;   // Spider
                        case 58: return 5;   // Enderman
                        case 66: return 5;   // Witch
                        case 61: return 10;  // Blaze
                        case 56: return 5;   // Ghast
                        case 62: return 1;   // Magma Cube (size-based, simplified to 1)
                        case 60: return 5;   // Silverfish
                        case 59: return 5;   // Cave Spider
                        case 57: return 5;   // Zombie Pigman
                        case 55: return 1;   // Slime (size-based, simplified to 1)
                        case 63: return 0;   // Ender Dragon — XP handled by death sequence
                        case 64: return 50;  // Wither — Java: experienceValue = 50
                        // Passive mobs
                        case 92: return 1 + (rand() % 3); // Cow (1-3)
                        case 90: return 1 + (rand() % 3); // Pig
                        case 91: return 1 + (rand() % 3); // Sheep
                        case 93: return 1 + (rand() % 3); // Chicken
                        case 94: return 1 + (rand() % 3); // Squid
                        case 97: return 0;   // Snow Golem — no XP in Java (golem)
                        case 99: return 0;   // Iron Golem (gives no XP in Java)
                        default: return 5;   // Default
                    }
                };
                int32_t killXp = getMobXp(mob.mobType);
                // Java: EntityLiving.onDeathUpdate() → world.spawnEntityInWorld(new EntityXPOrb(...))
                // Spawn XP orbs at mob death position instead of granting directly
                if (killXp > 0) {
                    spawnXPOrbs(mob.posX, mob.posY + 0.5, mob.posZ, killXp);
                }

                // Drop items based on mob type
                // Java: EntityZombie/EntitySkeleton/etc.dropFewItems()
                auto dropMobItems = [&](uint8_t mobType, int32_t looting) {
                    int32_t baseCount = 0;
                    int32_t dropId = 0;
                    int32_t dropMeta = 0;
                    switch (mobType) {
                        case 54: // Zombie → rotten flesh (367)
                            dropId = 367; baseCount = 1 + (rand() % 2); break;
                        case 51: // Skeleton → bones (352) + arrows (262)
                            dropId = 352; baseCount = 1 + (rand() % 2);
                            if (rand() % 2 == 0) {
                                int32_t arrowCount = 1 + (rand() % 2) + looting;
                                spawnItemDrop(mob.posX, mob.posY, mob.posZ, 262, 0, arrowCount);
                            }
                            break;
                        case 50: // Creeper → gunpowder (289)
                            dropId = 289; baseCount = rand() % 2; break;
                        case 52: // Spider → string (287) + spider eye (375)
                            dropId = 287; baseCount = 1 + (rand() % 2);
                            if (rand() % 3 == 0) {
                                spawnItemDrop(mob.posX, mob.posY, mob.posZ, 375, 0, 1);
                            }
                            break;
                        case 58: // Enderman → ender pearl (368)
                            dropId = 368; baseCount = rand() % 2; break;
                        case 66: // Witch → various potions/glowstone/sticks
                            dropId = 331; baseCount = 1 + (rand() % 3); break; // redstone
                        case 61: // Blaze → blaze rod (369)
                            dropId = 369; baseCount = rand() % 2; break;
                        case 56: // Ghast → ghast tear (370) + gunpowder (289)
                            dropId = 370; baseCount = rand() % 2;
                            spawnItemDrop(mob.posX, mob.posY, mob.posZ, 289, 0, 1 + (rand() % 2));
                            break;
                        case 55: // Slime → slimeball (341)
                            dropId = 341; baseCount = 1; break;
                        case 57: // Zombie Pigman → rotten flesh (367) + gold nugget (371)
                            dropId = 367; baseCount = 1;
                            if (rand() % 2 == 0) {
                                spawnItemDrop(mob.posX, mob.posY, mob.posZ, 371, 0, 1 + (rand() % 2));
                            }
                            break;
                        case 59: // Cave Spider → string (287) + spider eye (375)
                            dropId = 287; baseCount = 1;
                            if (rand() % 3 == 0) spawnItemDrop(mob.posX, mob.posY, mob.posZ, 375, 0, 1);
                            break;
                        case 60: // Silverfish → nothing
                            break;
                        case 64: // Wither → Nether Star (399)
                            dropId = 399; baseCount = 1; break;
                        case 62: // Magma Cube → magma cream (378)
                            dropId = 378; baseCount = rand() % 2; break;
                        // ─── Passive mob drops ──────────────────────────
                        // Java: EntityAnimal.dropFewItems() — drops cooked meat if isBurning()
                        case 92: // Cow → leather (334) + raw beef (363) / steak (364) if on fire
                            dropId = 334; baseCount = 1 + (rand() % 2);
                            spawnItemDrop(mob.posX, mob.posY, mob.posZ,
                                mob.isOnFire ? 364 : 363, 0, 1 + (rand() % 3));
                            break;
                        case 90: // Pig → raw porkchop (319) / cooked (320) if on fire
                            dropId = mob.isOnFire ? 320 : 319;
                            baseCount = 1 + (rand() % 3);
                            // Java: EntityPig.dropFewItems — if saddled, drop saddle
                            if (mob.isSaddled) {
                                spawnItemDrop(mob.posX, mob.posY, mob.posZ, 329, 0, 1);
                            }
                            // Dismount rider if pig was being ridden
                            if (mob.riderEntityId >= 0) {
                                broadcastAttachEntity(0, mob.riderEntityId, -1);
                                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                                for (auto& c : connections_) {
                                    if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                                    auto h = c->getHandler();
                                    auto* ph = dynamic_cast<PlayHandler*>(h.get());
                                    if (ph && ph->getEntityId() == mob.riderEntityId) {
                                        ph->setRidingEntityId(-1);
                                        break;
                                    }
                                }
                                mob.riderEntityId = -1;
                            }
                            break;
                        case 91: // Sheep → wool (35) only if not sheared
                            // Java: EntitySheep.dropFewItems — only drops wool if !getSheared()
                            if (!mob.isSheared) {
                                dropId = 35; baseCount = 1;
                                dropMeta = mob.fleeceColor;  // Wool color
                            }
                            break;
                        case 93: // Chicken → feather (288) + raw chicken (365) / cooked (366) if on fire
                            dropId = 288; baseCount = 1 + (rand() % 2);
                            spawnItemDrop(mob.posX, mob.posY, mob.posZ,
                                mob.isOnFire ? 366 : 365, 0, 1);
                            break;
                        case 94: // Squid → ink sac (351 damage 0)
                            dropId = 351; baseCount = 1 + (rand() % 3); break;
                        case 97: // Snow Golem → snowball (332) × 0-15
                            dropId = 332; baseCount = rand() % 16;
                            break;
                        case 99: // Iron Golem → iron ingots (265) + rose (38)
                            dropId = 265; baseCount = 3 + (rand() % 3);
                            spawnItemDrop(mob.posX, mob.posY, mob.posZ, 38, 0, 1 + (rand() % 2));
                            break;
                        case 100: { // Horse — Java: EntityHorse.getDropItem()
                            int ht = mob.horseType;
                            if (ht == 4) { dropId = 352; baseCount = 1 + (rand() % 2); } // Skeleton horse → bone
                            else if (ht == 3) { // Zombie horse → rotten flesh (75% chance)
                                if (rand() % 4 != 0) { dropId = 367; baseCount = 1 + (rand() % 2); }
                            } else { dropId = 334; baseCount = 1 + (rand() % 2); } // Normal → leather
                            // Drop saddle if saddled
                            if (mob.isHorseSaddled) {
                                spawnItemDrop(mob.posX, mob.posY, mob.posZ, 329, 0, 1);
                            }
                            // Drop armor — Java: EntityHorse.dropChestItems()
                            if (mob.horseArmorIndex == 1) spawnItemDrop(mob.posX, mob.posY, mob.posZ, 417, 0, 1); // iron
                            else if (mob.horseArmorIndex == 2) spawnItemDrop(mob.posX, mob.posY, mob.posZ, 418, 0, 1); // gold
                            else if (mob.horseArmorIndex == 3) spawnItemDrop(mob.posX, mob.posY, mob.posZ, 419, 0, 1); // diamond
                            // Drop chest if chested donkey/mule
                            if (mob.isHorseChested) {
                                spawnItemDrop(mob.posX, mob.posY, mob.posZ, 54, 0, 1); // chest block
                            }
                            // Dismount rider if horse was being ridden
                            if (mob.riderEntityId >= 0) {
                                broadcastAttachEntity(0, mob.riderEntityId, -1);
                                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                                for (auto& c : connections_) {
                                    if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                                    auto h = c->getHandler();
                                    auto* ph = dynamic_cast<PlayHandler*>(h.get());
                                    if (ph && ph->getEntityId() == mob.riderEntityId) {
                                        ph->setRidingEntityId(-1);
                                        break;
                                    }
                                }
                                mob.riderEntityId = -1;
                            }
                            break;
                        }
                        default: break;
                    }
                    if (dropId > 0) {
                        int32_t totalCount = baseCount + looting;
                        if (totalCount > 0) {
                            spawnItemDrop(mob.posX, mob.posY, mob.posZ, dropId, dropMeta, totalCount);
                        }
                    }
                };

                dropMobItems(mob.mobType, lootingLevel);

                // ─── Slime/Magma Cube split on death ────────────────────
                // Java: EntitySlime.setDead() — size>1 spawns 2-4 half-size slimes
                // Slime=55, Magma Cube=62
                if (mob.mobType == 55 || mob.mobType == 62) {
                    // For now, assume large slimes (size 4 → spawn size 2 → spawn size 1)
                    // Simplified: spawn 2-4 smaller slimes at death position
                    int splitCount = 2 + (rand() % 3); // Java: 2 + rand.nextInt(3)
                    for (int sc = 0; sc < splitCount; ++sc) {
                        double offsetX = ((rand() % 100) / 100.0 - 0.5) * 1.0;
                        double offsetZ = ((rand() % 100) / 100.0 - 0.5) * 1.0;
                        summonMob(mob.mobType, mob.posX + offsetX, mob.posY + 0.5, mob.posZ + offsetZ);
                    }
                    broadcastSound(mob.mobType == 55 ? "mob.slime.big" : "mob.magmacube.big",
                        mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
                }
            }
            return; // Found target mob, done
        }
        return; // No target found at all
    }

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

    // ─── Enchantment protection reduction ────────────────────────────
    // Java: EntityLivingBase.applyPotionDamageCalculations()
    // → EnchantmentHelper.getEnchantmentModifierDamage()
    // Protection enchants add extra damage reduction: damage * (25 - modifier) / 25
    int32_t enchProtMod = target->getEnchantmentProtectionModifier();
    if (enchProtMod > 0) {
        damage = damage * static_cast<float>(25 - enchProtMod) / 25.0f;
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

    // ─── Knockback enchantment (ID 19) ──────────────────────────────
    // Java: EnchantmentHelper.getKnockbackModifier → extra knockback
    // Each level adds 0.5 knockback strength multiplier
    auto attackerHeld = attacker.getHeldItem();
    if (attackerHeld && attackerHeld->hasEnchantments()) {
        int16_t kbLevel = attackerHeld->getEnchantmentLevel(19);
        if (kbLevel > 0) {
            double kbMult = 1.0 + static_cast<double>(kbLevel) * 0.5;
            kbX *= kbMult;
            kbZ *= kbMult;
        }
    }

    // Send S12 EntityVelocity to the target
    target->sendEntityVelocity(*targetConn, targetEntityId, kbX, kbY, kbZ);

    // ─── Fire Aspect enchantment (ID 20) ────────────────────────────
    // Java: EnchantmentHelper.getFireAspectModifier → set target on fire
    // level * 4 seconds = level * 80 ticks
    if (attackerHeld && attackerHeld->hasEnchantments()) {
        int16_t faLevel = attackerHeld->getEnchantmentLevel(20);
        if (faLevel > 0) {
            int32_t fireTicks = faLevel * 80;
            target->setOnFire(fireTicks);
        }
    }

    // ─── Thorns enchantment (ID 7) ──────────────────────────────────
    // Java: EnchantmentThorns.func_92094_a → level*15% chance, 1-4 reflected damage
    // Check all armor pieces of the TARGET for Thorns
    for (int16_t slot = 1; slot <= 4; ++slot) {
        auto armor = target->getArmorItem(slot);
        if (!armor || !armor->hasEnchantments()) continue;
        int16_t thornsLevel = armor->getEnchantmentLevel(7);
        if (thornsLevel > 0) {
            // Java: random.nextInt(100) < level * 15
            if ((rand() % 100) < thornsLevel * 15) {
                int32_t thornsDmg = 1 + (rand() % 4); // 1-4 damage
                attacker.applyDamage(static_cast<float>(thornsDmg));
                attacker.sendUpdateHealth(attackerConn, attacker.getHealth(),
                    attacker.getFood(), attacker.getSaturation());
                broadcastSound("damage.thorns",
                    attacker.getPlayerX(), attacker.getPlayerY(), attacker.getPlayerZ(),
                    1.0f, 1.0f);
            }
        }
    }

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
        // Java: EntityPlayer.getExperiencePoints() — drops level*7 XP (max 100) as orbs
        int32_t killXp = std::min(target->getExperienceLevel() * 7, 100);
        if (killXp > 0) {
            spawnXPOrbs(target->getPlayerX(), target->getPlayerY() + 0.5, target->getPlayerZ(), killXp);
        }

        // Reset victim's XP on death
        target->resetExperience();

        // Set player to dead state (will respawn on ClientStatus packet)
        // The client will show the death screen and send C16 ClientStatus(0)
    }
}

void MinecraftServer::handleEntityInteract(PlayHandler& player, Connection& conn, int32_t targetEntityId) {
    // Java: EntityPlayer.interactWith(targetEntity)
    // Handles: sheep shearing/dyeing, cow/mooshroom milking, mooshroom stew/shearing, pig saddling
    auto heldItem = player.getHeldItem();
    int32_t heldId = heldItem ? heldItem->getItemId() : 0;
    int32_t gameMode = player.getGameMode();

    // ─── Shears interactions (item 359) ──────────────────────────────
    // Java: EntitySheep.interact() + EntityMooshroom.interact()
    if (heldId == 359) {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        for (auto& mob : mobEntities_) {
            if (mob.entityId != targetEntityId) continue;
            if (mob.isDead) break;

            // ─── Sheep shearing — Java: EntitySheep.interact() ──────
            if (mob.mobType == 91 && !mob.isSheared) {
                mob.isSheared = true;
                int32_t woolCount = 1 + (rand() % 3);  // 1-3 wool blocks

                for (int i = 0; i < woolCount; ++i) {
                    double dropX = mob.posX + ((rand() % 100 - 50) / 500.0);
                    double dropZ = mob.posZ + ((rand() % 100 - 50) / 500.0);
                    spawnItemDrop(dropX, mob.posY + 1.0, dropZ, 35, mob.fleeceColor, 1);
                }

                player.damageHeldItem(1);
                player.sendWindowItems(conn);
                broadcastSound("mob.sheep.shear", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);

                // S1C EntityMetadata — DataWatcher byte 16 (sheared bit 0x10)
                {
                    uint8_t dw16 = static_cast<uint8_t>((mob.fleeceColor & 0x0F) | 0x10);
                    auto metaPkt = PacketBuilder::entityMetadataByte(mob.entityId, 16, dw16);
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    for (auto& c : connections_) {
                        if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                        c->sendPacket(metaPkt);
                    }
                }
            }

            // ─── Mooshroom shearing — Java: EntityMooshroom.interact() ──
            // Shears on mooshroom → kill mooshroom, spawn cow, drop 5 red mushrooms
            if (mob.mobType == 96) {
                double mX = mob.posX, mY = mob.posY, mZ = mob.posZ;
                int32_t oldEntityId = mob.entityId;

                // Kill the mooshroom — Java: this.setDead()
                mob.isDead = true;
                mob.health = 0;

                // Broadcast destroy mooshroom
                {
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    for (auto& c : connections_) {
                        if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                        auto handler = c->getHandler();
                        auto* h = dynamic_cast<PlayHandler*>(handler.get());
                        if (h) h->sendDestroyEntities(
                            *c, std::vector<int32_t>{oldEntityId});
                    }
                }

                // Spawn a cow (92) at the same position — Java: new EntityCow
                summonMob(92, mX, mY, mZ);

                // Drop 5 red mushrooms (block 40) — Java: Blocks.red_mushroom
                for (int i = 0; i < 5; ++i) {
                    spawnItemDrop(mX, mY + 1.3, mZ, 40, 0, 1);
                }

                player.damageHeldItem(1);
                player.sendWindowItems(conn);
                broadcastSound("mob.sheep.shear", mX, mY, mZ, 1.0f, 1.0f);

                // S2A largeexplode particle — Java: this.worldObj.spawnParticle
                broadcastParticle("largeexplode",
                    static_cast<float>(mX), static_cast<float>(mY + 0.65f),
                    static_cast<float>(mZ), 0.0f, 0.0f, 0.0f, 0.0f, 1);
            }
            break;
        }
        return;  // Shears handled, skip other checks
    }

    // ─── Cow/Mooshroom milking — Java: EntityCow.interact() ─────────
    // Bucket (325) on cow (92) or mooshroom (96) → milk bucket (335)
    // Java: !entityPlayer.capabilities.isCreativeMode — skip in creative
    if (heldId == 325 && gameMode != 1) {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        for (auto& mob : mobEntities_) {
            if (mob.entityId != targetEntityId) continue;
            if (mob.isDead) break;
            if (mob.mobType != 92 && mob.mobType != 96) break;

            if (heldItem->getStackSize() == 1) {
                player.replaceHeldItem(ItemStack(335, 1, 0));
            } else {
                player.decrHeldItem();
                ItemStack milkBucket(335, 1, 0);
                if (!player.addItemToInventory(milkBucket)) {
                    spawnItemDrop(player.getPlayerX(), player.getPlayerY() + 1.0,
                                  player.getPlayerZ(), 335, 0, 1);
                }
            }
            player.sendWindowItems(conn);
            break;
        }
        return;
    }

    // ─── Mooshroom bowl stew — Java: EntityMooshroom.interact() ─────
    // Bowl (281) on mooshroom (96) → mushroom stew (282)
    if (heldId == 281) {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        for (auto& mob : mobEntities_) {
            if (mob.entityId != targetEntityId) continue;
            if (mob.isDead || mob.mobType != 96) break;

            // Java: if (itemStack.stackSize == 1) replace, else add + decrement
            if (heldItem->getStackSize() == 1) {
                player.replaceHeldItem(ItemStack(282, 1, 0));  // mushroom_stew
            } else {
                ItemStack stew(282, 1, 0);
                if (player.addItemToInventory(stew) && gameMode != 1) {
                    player.decrHeldItem();
                }
            }
            player.sendWindowItems(conn);
            break;
        }
        return;
    }

    // ─── Pig saddling — Java: EntityPig → EntityAnimal.interact() ───
    // Saddle (329) on pig (90) → set saddled, consume saddle
    if (heldId == 329) {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        for (auto& mob : mobEntities_) {
            if (mob.entityId != targetEntityId) continue;
            if (mob.isDead || mob.mobType != 90) break;
            if (mob.isSaddled) break;  // Already saddled

            // Java: EntityAnimal.interact() via isBreedingItem check —
            // but saddle is handled in net.minecraft.entity.passive.EntityPig parent EntityAnimal.interact()
            // Actually: saddle check is in EntityLiving.interact() not EntityPig.interact()
            mob.isSaddled = true;

            // Consume saddle in survival — Java: --itemStack.stackSize
            if (gameMode != 1) {
                player.decrHeldItem();
            }
            player.sendWindowItems(conn);

            // S1C EntityMetadata — DataWatcher byte 16 = 1 (saddled)
            {
                auto metaPkt = PacketBuilder::entityMetadataByte(mob.entityId, 16, 1);
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& c : connections_) {
                    if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                    c->sendPacket(metaPkt);
                }
            }
            // Java: this.worldObj.playSoundAtEntity mob.horse.leather
            broadcastSound("mob.horse.leather", mob.posX, mob.posY, mob.posZ, 0.5f, 1.0f);
            break;
        }
        return;
    }

    // ─── Sheep dyeing — Java: EntitySheep.interact() via ItemDye ────
    // Dye item (351) on sheep → change fleece color to 15-damage
    // Java: Items.dye (id 351), meta = dye color, fleece color = 15 - dye color
    if (heldId == 351) {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        for (auto& mob : mobEntities_) {
            if (mob.entityId != targetEntityId) continue;
            if (mob.isDead || mob.mobType != 91) break;

            int32_t dyeColor = heldItem->getDamage() & 0x0F;
            int32_t newFleeceColor = 15 - dyeColor;  // Java: BlockColored.func_150032_b(damage)

            if (mob.fleeceColor == newFleeceColor) break;  // Already this color

            mob.fleeceColor = newFleeceColor;

            // Consume dye in survival
            if (gameMode != 1) {
                player.decrHeldItem();
            }
            player.sendWindowItems(conn);

            // S1C EntityMetadata — DataWatcher byte 16 (color in lower 4 bits, sheared in bit 4)
            {
                uint8_t dw16 = static_cast<uint8_t>(
                    (mob.fleeceColor & 0x0F) | (mob.isSheared ? 0x10 : 0x00));
                auto metaPkt = PacketBuilder::entityMetadataByte(mob.entityId, 16, dw16);
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& c : connections_) {
                    if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                    c->sendPacket(metaPkt);
                }
            }
            break;
        }
        return;
    }

    // ─── Wolf interactions — Java: EntityWolf.interact() ──────────────
    // Handles: bone taming, sit/stand toggle, meat healing, dye collar, breeding
    {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        for (auto& mob : mobEntities_) {
            if (mob.entityId != targetEntityId) continue;
            if (mob.isDead || mob.mobType != 95) break;

            // Helper: broadcast wolf DataWatcher 16 byte (sitting|angry|tamed flags)
            auto broadcastWolfDW16 = [&](SpawnedMob& w) {
                uint8_t dw16 = 0;
                if (w.isSitting) dw16 |= 0x01;
                if (w.isAngry)   dw16 |= 0x02;
                if (w.isTamed)   dw16 |= 0x04;
                auto metaPkt = PacketBuilder::entityMetadataByte(w.entityId, 16, dw16);
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& c : connections_) {
                    if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                    c->sendPacket(metaPkt);
                }
            };

            // Helper: is this a wolf-favorite meat? Java: ItemFood.isWolfsFavoriteMeat()
            auto isWolfMeat = [](int32_t id) -> bool {
                // Raw porkchop(319), cooked porkchop(320), raw beef(363), steak(364),
                // raw chicken(365), cooked chicken(366), rotten flesh(367)
                return id == 319 || id == 320 || id == 363 || id == 364 ||
                       id == 365 || id == 366 || id == 367;
            };

            // Helper: get heal amount for meat items — Java: ItemFood.getHealAmount()
            auto getMeatHealAmount = [](int32_t id) -> float {
                switch (id) {
                    case 319: return 3.0f;  // raw_porkchop
                    case 320: return 8.0f;  // cooked_porkchop
                    case 363: return 3.0f;  // raw_beef
                    case 364: return 8.0f;  // steak
                    case 365: return 2.0f;  // raw_chicken
                    case 366: return 6.0f;  // cooked_chicken
                    case 367: return 4.0f;  // rotten_flesh
                    default: return 0.0f;
                }
            };

            if (mob.isTamed) {
                // ─── Tamed wolf: heal with meat ───
                if (heldItem && isWolfMeat(heldId) && mob.health < 20.0f) {
                    float healAmount = getMeatHealAmount(heldId);
                    mob.health = std::min(20.0f, mob.health + healAmount);
                    if (gameMode != 1) {
                        player.decrHeldItem();
                    }
                    player.sendWindowItems(conn);

                    // Broadcast updated health — DW 18 float
                    {
                        auto metaPkt = PacketBuilder::entityMetadataFloat(mob.entityId, 18, mob.health);
                        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                        for (auto& c : connections_) {
                            if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                            c->sendPacket(metaPkt);
                        }
                    }
                    return;
                }

                // ─── Tamed wolf: dye collar — Java: EntityWolf.interact() with Items.dye ───
                if (heldId == 351) {
                    int32_t dyeColor = heldItem->getDamage() & 0x0F;
                    int32_t newCollarColor = 15 - dyeColor;  // Java: BlockColored.func_150032_b()
                    if (newCollarColor != mob.collarColor) {
                        mob.collarColor = newCollarColor;
                        if (gameMode != 1) {
                            player.decrHeldItem();
                        }
                        player.sendWindowItems(conn);
                        // Broadcast DW 20 byte — collar color
                        {
                            auto metaPkt = PacketBuilder::entityMetadataByte(
                                mob.entityId, 20, static_cast<uint8_t>(mob.collarColor & 0x0F));
                            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                            for (auto& c : connections_) {
                                if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                                c->sendPacket(metaPkt);
                            }
                        }
                    }
                    return;
                }

                // ─── Tamed wolf: sit/stand toggle — Java: EntityAISit ───
                // Only owner can toggle, and only if not holding breeding item
                if (mob.ownerUuid == player.getUuid() && !(heldItem && isWolfMeat(heldId))) {
                    mob.isSitting = !mob.isSitting;
                    mob.wolfAttackTarget = -1;
                    broadcastWolfDW16(mob);
                    std::cout << "[Wolf] " << player.getPlayerName()
                              << (mob.isSitting ? " made wolf sit" : " made wolf stand")
                              << " (entity " << mob.entityId << ")\n";
                    return;
                }
            } else {
                // ─── Wild wolf: tame with bone (352) — Java: EntityWolf.interact() ───
                if (heldId == 352 && !mob.isAngry) {
                    if (gameMode != 1) {
                        player.decrHeldItem();
                    }
                    player.sendWindowItems(conn);

                    if (rand() % 3 == 0) {
                        // Taming success! — Java: 1/3 chance
                        mob.isTamed = true;
                        mob.isSitting = true;
                        mob.isAngry = false;
                        mob.ownerUuid = player.getUuid();
                        mob.health = 20.0f;  // Java: setHealth(20.0f) on tame
                        mob.wolfAttackTarget = -1;

                        broadcastWolfDW16(mob);

                        // Broadcast updated health — DW 18
                        {
                            auto metaPkt = PacketBuilder::entityMetadataFloat(mob.entityId, 18, mob.health);
                            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                            for (auto& c : connections_) {
                                if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                                c->sendPacket(metaPkt);
                            }
                        }

                        // S1A EntityStatus byte 7 — heart particles (tame success)
                        broadcastEntityEvent(mob.entityId, 7);

                        std::cout << "[Wolf] " << player.getPlayerName()
                                  << " tamed wolf " << mob.entityId << "\n";
                    } else {
                        // Taming failed — S1A EntityStatus byte 6 — smoke particles
                        broadcastEntityEvent(mob.entityId, 6);
                    }
                    return;
                }
            }
            break;
        }
    }

    // ─── Ocelot interactions — Java: EntityOcelot.interact() ────────────
    // Handles: fish taming, sit/stand toggle
    {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        for (auto& mob : mobEntities_) {
            if (mob.entityId != targetEntityId) continue;
            if (mob.isDead || mob.mobType != 98) break;

            // Helper: broadcast ocelot DataWatcher 16 byte (sitting|tamed flags)
            auto broadcastCatDW16 = [&](SpawnedMob& o) {
                uint8_t dw16 = 0;
                if (o.isSitting) dw16 |= 0x01;
                if (o.isTamed)   dw16 |= 0x04;
                auto metaPkt = PacketBuilder::entityMetadataByte(o.entityId, 16, dw16);
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& c : connections_) {
                    if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                    c->sendPacket(metaPkt);
                }
            };

            if (mob.isTamed) {
                // ─── Tamed ocelot: sit/stand toggle ───
                // Java: only owner can toggle, and only if not holding breeding item
                if (mob.ownerUuid == player.getUuid() && !(heldId == 349)) {
                    mob.isSitting = !mob.isSitting;
                    broadcastCatDW16(mob);
                    std::cout << "[Cat] " << player.getPlayerName()
                              << (mob.isSitting ? " made cat sit" : " made cat stand")
                              << " (entity " << mob.entityId << ")\n";
                    return;
                }
            } else {
                // ─── Wild ocelot: tame with fish (349) — Java: EntityOcelot.interact() ───
                // Java: aiTempt.isRunning() && Items.fish && dist < 9.0 && random(3)==0
                if (heldId == 349) {
                    double dx = player.getPlayerX() - mob.posX;
                    double dz = player.getPlayerZ() - mob.posZ;
                    if (dx * dx + dz * dz < 9.0) {
                        if (gameMode != 1) {
                            player.decrHeldItem();
                        }
                        player.sendWindowItems(conn);

                        if (rand() % 3 == 0) {
                            // Taming success! — Java: 1/3 chance
                            mob.isTamed = true;
                            mob.isSitting = true;
                            mob.ownerUuid = player.getUuid();
                            // Java: setTameSkin(1 + world.rand.nextInt(3))
                            mob.catSkinType = 1 + (rand() % 3);

                            broadcastCatDW16(mob);

                            // Broadcast DW 18 (catSkinType)
                            {
                                auto metaPkt = PacketBuilder::entityMetadataByte(
                                    mob.entityId, 18, static_cast<uint8_t>(mob.catSkinType));
                                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                                for (auto& c : connections_) {
                                    if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                                    c->sendPacket(metaPkt);
                                }
                            }

                            // S1A EntityStatus byte 7 — heart particles (tame success)
                            broadcastEntityEvent(mob.entityId, 7);

                            std::cout << "[Cat] " << player.getPlayerName()
                                      << " tamed ocelot " << mob.entityId
                                      << " (skin=" << mob.catSkinType << ")\n";
                        } else {
                            // Taming failed — S1A EntityStatus byte 6 — smoke particles
                            broadcastEntityEvent(mob.entityId, 6);
                        }
                        return;
                    }
                }
            }
            break;
        }
    }

    // ─── Horse interactions — Java: EntityHorse.interact() ────────────
    // Handles: food healing/temper, saddling, armor, chest, taming (mount → buck), mounting tamed
    {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        for (auto& mob : mobEntities_) {
            if (mob.entityId != targetEntityId) continue;
            if (mob.isDead || mob.mobType != 100) break;

            // Helper: broadcast horse DW 16 int (bit flags)
            auto broadcastHorseDW16 = [&](SpawnedMob& h) {
                int32_t dw16val = 0;
                if (h.isTamed) dw16val |= 2;
                if (h.isHorseSaddled) dw16val |= 4;
                if (h.isHorseChested) dw16val |= 8;
                auto metaPkt = PacketBuilder::entityMetadataInt(h.entityId, 16, dw16val);
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& c : connections_) {
                    if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                    c->sendPacket(metaPkt);
                }
            };

            // Helper: broadcast horse DW 22 (armor index)
            auto broadcastHorseDW22 = [&](SpawnedMob& h) {
                auto metaPkt = PacketBuilder::entityMetadataInt(h.entityId, 22, h.horseArmorIndex);
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& c : connections_) {
                    if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                    c->sendPacket(metaPkt);
                }
            };

            bool consumed = false;

            // ─── Horse armor equipping — Java: EntityHorse.interact() armor check ───
            if (heldItem && mob.horseType == 0) { // Only normal horses wear armor
                int armorIdx = 0;
                if (heldId == 417) armorIdx = 1; // iron_horse_armor
                else if (heldId == 418) armorIdx = 2; // golden_horse_armor
                else if (heldId == 419) armorIdx = 3; // diamond_horse_armor
                if (armorIdx > 0 && mob.isTamed) {
                    mob.horseArmorIndex = armorIdx;
                    broadcastHorseDW22(mob);
                    if (gameMode != 1) player.decrHeldItem();
                    player.sendWindowItems(conn);
                    broadcastSound("mob.horse.armor", mob.posX, mob.posY, mob.posZ, 0.5f, 1.0f);
                    std::cout << "[Horse] " << player.getPlayerName()
                              << " equipped armor " << armorIdx << " on horse " << mob.entityId << "\n";
                    return;
                }
            }

            // ─── Food items — Java: EntityHorse.interact() food table ───
            // wheat=2hp/60age/3temper, sugar=1hp/30age/3temper, bread=7hp/180age/3temper
            // apple=3hp/60age/3temper, golden_carrot=4hp/60age/5temper+breed, golden_apple=10hp/240age/10temper+breed
            if (heldItem) {
                float heal = 0; int ageGrowth = 0; int temperBoost = 0;
                bool breedItem = false;
                if (heldId == 296) { heal = 2; ageGrowth = 60; temperBoost = 3; }      // wheat
                else if (heldId == 353) { heal = 1; ageGrowth = 30; temperBoost = 3; }  // sugar
                else if (heldId == 297) { heal = 7; ageGrowth = 180; temperBoost = 3; } // bread
                else if (heldId == 260) { heal = 3; ageGrowth = 60; temperBoost = 3; }  // apple
                else if (heldId == 396) { heal = 4; ageGrowth = 60; temperBoost = 5; breedItem = true; } // golden_carrot
                else if (heldId == 322) { heal = 10; ageGrowth = 240; temperBoost = 10; breedItem = true; } // golden_apple

                if (heal > 0 || temperBoost > 0) {
                    // Heal if needed — horse base max health is 26.0f
                    float maxHp = 26.0f;
                    if (mob.health < maxHp && heal > 0) {
                        mob.health = std::min(mob.health + heal, maxHp);
                        consumed = true;
                    }
                    // Increase temper if not tamed
                    if (!mob.isTamed && temperBoost > 0 && mob.horseTemper < 100) {
                        mob.horseTemper = std::min(mob.horseTemper + temperBoost, 100);
                        consumed = true;
                    }
                    // Breeding — golden_carrot or golden_apple on tamed adult full-health horse
                    if (breedItem && mob.isTamed && mob.inLoveTicks <= 0 && mob.breedCooldown <= 0) {
                        mob.inLoveTicks = 600;
                        broadcastEntityEvent(mob.entityId, 18); // Love hearts
                        consumed = true;
                    }
                    if (consumed) {
                        // Eating sound + consume item
                        broadcastSound("eating", mob.posX, mob.posY, mob.posZ, 1.0f,
                            1.0f + ((float)(rand() % 40) / 100.0f - 0.2f));
                        if (gameMode != 1) player.decrHeldItem();
                        player.sendWindowItems(conn);
                        return;
                    }
                }
            }

            // ─── Saddle equipping — Java: EntityHorse.interact() saddle check ───
            if (heldItem && heldId == 329 && mob.isTamed && !mob.isHorseSaddled) {
                mob.isHorseSaddled = true;
                broadcastHorseDW16(mob);
                if (gameMode != 1) player.decrHeldItem();
                player.sendWindowItems(conn);
                broadcastSound("mob.horse.leather", mob.posX, mob.posY, mob.posZ, 0.5f, 1.0f);
                std::cout << "[Horse] " << player.getPlayerName()
                          << " saddled horse " << mob.entityId << "\n";
                return;
            }

            // ─── Chest equipping (donkey/mule only) — Java: EntityHorse.canCarryChest() ───
            if (heldItem && heldId == 54 && mob.isTamed && !mob.isHorseChested &&
                (mob.horseType == 1 || mob.horseType == 2)) {
                mob.isHorseChested = true;
                broadcastHorseDW16(mob);
                if (gameMode != 1) player.decrHeldItem();
                player.sendWindowItems(conn);
                broadcastSound("mob.chickenplop", mob.posX, mob.posY, mob.posZ, 1.0f,
                    ((float)(rand() % 40) / 100.0f - 0.2f) + 1.0f);
                std::cout << "[Horse] " << player.getPlayerName()
                          << " added chest to " << (mob.horseType == 1 ? "donkey" : "mule")
                          << " " << mob.entityId << "\n";
                return;
            }

            // ─── Tamed horse: mount if saddled ───
            if (mob.isTamed && mob.isHorseSaddled && mob.riderEntityId < 0) {
                // Mount the player on the horse
                mob.riderEntityId = player.getEntityId();
                player.setRidingEntityId(mob.entityId);
                broadcastAttachEntity(0, player.getEntityId(), mob.entityId);
                std::cout << "[Horse] " << player.getPlayerName()
                          << " mounted horse " << mob.entityId << "\n";
                return;
            }

            // ─── Untamed horse: attempt taming (mount → may buck) ───
            if (!mob.isTamed && mob.riderEntityId < 0) {
                // Java: EntityAIRunAroundLikeCrazy — mount player, then check temper
                mob.riderEntityId = player.getEntityId();
                player.setRidingEntityId(mob.entityId);
                broadcastAttachEntity(0, player.getEntityId(), mob.entityId);

                // Java: temper check — rand(maxTemper=100) < temper → tame
                if (rand() % 100 < mob.horseTemper) {
                    // Taming success!
                    mob.isTamed = true;
                    mob.ownerUuid = player.getUuid();
                    broadcastHorseDW16(mob);
                    broadcastEntityEvent(mob.entityId, 7); // Heart particles
                    // Play angry sound, then settle
                    std::string angrySound = "mob.horse.angry";
                    if (mob.horseType == 1 || mob.horseType == 2) angrySound = "mob.horse.donkey.angry";
                    broadcastSound(angrySound, mob.posX, mob.posY, mob.posZ, 0.8f, 1.0f);
                    std::cout << "[Horse] " << player.getPlayerName()
                              << " tamed horse " << mob.entityId << " (temper=" << mob.horseTemper << ")\n";
                } else {
                    // Taming failed — buck player off after a short delay
                    // For now, immediate dismount + smoke particles
                    broadcastEntityEvent(mob.entityId, 6); // Smoke particles
                    std::string angrySound = "mob.horse.angry";
                    if (mob.horseType == 1 || mob.horseType == 2) angrySound = "mob.horse.donkey.angry";
                    broadcastSound(angrySound, mob.posX, mob.posY, mob.posZ, 0.8f, 1.0f);
                    // Dismount immediately
                    broadcastAttachEntity(0, player.getEntityId(), -1);
                    player.setRidingEntityId(-1);
                    mob.riderEntityId = -1;
                    // Increase temper by 5 per attempt — Java: increaseTemper(5)
                    mob.horseTemper = std::min(mob.horseTemper + 5, 100);
                    std::cout << "[Horse] " << player.getPlayerName()
                              << " failed to tame horse " << mob.entityId
                              << " (temper now " << mob.horseTemper << ")\n";
                }
                return;
            }
            break;
        }
    }

    // ─── Villager trading — Java: EntityVillager.interact() ──────────────
    // Right-click on a villager → open trading GUI
    {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        for (auto& mob : mobEntities_) {
            if (mob.entityId != targetEntityId) continue;
            if (mob.isDead || mob.mobType != 120) break;

            // Generate trades on first interaction — Java: EntityVillager.getRecipes()
            if (!mob.villagerTradesGenerated) {
                generateVillagerTrades(mob.villagerProfession, mob.villagerTrades);
                mob.villagerTradesGenerated = true;
            }

            // Open trading GUI — Java: EntityVillager.interact() → displayGUIMerchant
            player.openVillagerTrading(conn, mob.entityId, mob.villagerTrades);
            broadcastSound("mob.villager.haggle", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
            std::cout << "[Villager] " << player.getPlayerName()
                      << " opened trading GUI for villager " << mob.entityId
                      << " (profession=" << mob.villagerProfession
                      << ", trades=" << mob.villagerTrades.size() << ")\n";
            return;
        }
    }

    // ─── Animal feeding (breeding) — Java: EntityAnimal.interact() ───
    // Feeding the breeding item to a passive mob puts it in love mode (600 ticks)
    // Per-mob breeding items: cow/mooshroom/sheep → wheat (296), pig → carrot (391), chicken → seeds
    // Wolf → any meat (tamed only, when at full health)
    if (heldItem && heldId != 0) {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        for (auto& mob : mobEntities_) {
            if (mob.entityId != targetEntityId) continue;
            if (mob.isDead || !mob.isPassive) break;

            // Check if this is the correct breeding item for this mob type
            bool isBreedingItem = false;
            switch (mob.mobType) {
                case 92:  // Cow → wheat (296)
                case 96:  // Mooshroom → wheat (296)
                case 91:  // Sheep → wheat (296)
                    isBreedingItem = (heldId == 296);
                    break;
                case 90:  // Pig → carrot (391)
                    isBreedingItem = (heldId == 391);
                    break;
                case 93:  // Chicken → any seeds (ItemSeeds: wheat 295, melon 362, pumpkin 361)
                    isBreedingItem = (heldId == 295 || heldId == 361 || heldId == 362);
                    break;
                case 95:  // Wolf → meat (tamed only) — Java: EntityWolf.isBreedingItem()
                    if (mob.isTamed) {
                        isBreedingItem = (heldId == 319 || heldId == 320 || heldId == 363 ||
                                         heldId == 364 || heldId == 365 || heldId == 366 || heldId == 367);
                    }
                    break;
                case 98:  // Ocelot → fish (tamed only) — Java: EntityOcelot.isBreedingItem()
                    if (mob.isTamed) {
                        isBreedingItem = (heldId == 349);  // Raw fish
                    }
                    break;
                default:
                    break;
            }

            if (!isBreedingItem) break;
            // Java: this.getGrowingAge() == 0 && this.inLove <= 0
            if (mob.inLoveTicks > 0 || mob.breedCooldown > 0) break;

            // Set in love — Java: EntityAnimal.setInLove()
            mob.inLoveTicks = 600;  // 30 seconds
            mob.breedingCounter = 0;
            mob.mateEntityId = -1;

            // Consume item in survival — Java: --itemStack.stackSize
            if (gameMode != 1) {
                player.decrHeldItem();
            }
            player.sendWindowItems(conn);

            // S1A EntityStatus byte 18 — love hearts particle
            broadcastEntityEvent(mob.entityId, 18);
            break;
        }
    }

    // ─── Saddled pig mounting — Java: EntityPig.interact() ─────────────
    // If not a breeding item, check if target is a saddled pig to mount
    // Java: if (!super.interact(player)) { if (getSaddled() && !isRemote && (riddenByEntity == null || riddenByEntity == player)) { player.mountEntity(this); } }
    {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        for (auto& mob : mobEntities_) {
            if (mob.entityId != targetEntityId) continue;
            if (mob.isDead || mob.mobType != 90) break;
            if (!mob.isSaddled) break;

            if (mob.riderEntityId == player.getEntityId()) {
                // Dismount — Java: player.mountEntity(null)
                mob.riderEntityId = -1;
                mob.pigCurrentSpeed = 0.0f;
                mob.pigSpeedBoosted = false;
                player.setRidingEntityId(-1);

                broadcastAttachEntity(0, player.getEntityId(), -1);
                std::cout << "[Pig] " << player.getPlayerName() << " dismounted pig " << mob.entityId << "\n";
            } else if (mob.riderEntityId < 0) {
                // Mount — Java: player.mountEntity(pig)
                mob.riderEntityId = player.getEntityId();
                mob.pigCurrentSpeed = 0.0f;
                player.setRidingEntityId(mob.entityId);

                broadcastAttachEntity(0, player.getEntityId(), mob.entityId);
                std::cout << "[Pig] " << player.getPlayerName() << " mounted saddled pig " << mob.entityId << "\n";
            }
            // Java: if riddenByEntity != null && riddenByEntity != player → return true (already occupied)
            return;
        }
    }

    // ─── Minecart mounting — Java: EntityMinecart.interactFirst() ────
    // Right-click on an empty minecart (type 0) to mount it
    // Right-click while riding to dismount
    {
        std::lock_guard<std::mutex> cartLock(minecartEntitiesMutex_);
        for (auto& cart : minecartEntities_) {
            if (cart.entityId != targetEntityId || cart.isDead) continue;
            if (cart.minecartType != 0) break; // Only empty minecarts are rideable

            if (cart.riderEntityId == player.getEntityId()) {
                // Dismount — Java: EntityMinecart.interactFirst → mountEntity(null)
                cart.riderEntityId = -1;
                cart.logic.riderEntityId = -1;
                player.setRidingEntityId(-1);

                // S1B AttachEntity — detach (vehicleId = -1)
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& c : connections_) {
                    if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                    auto h = c->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(h.get());
                    if (ph) ph->sendAttachEntity(*c, 0, player.getEntityId(), -1);
                }
                std::cout << "[Minecart] " << player.getPlayerName() << " dismounted minecart " << cart.entityId << "\n";
            } else if (cart.riderEntityId < 0) {
                // Mount — Java: player.mountEntity(minecart)
                cart.riderEntityId = player.getEntityId();
                cart.logic.riderEntityId = player.getEntityId();
                player.setRidingEntityId(cart.entityId);

                // S1B AttachEntity — attach (vehicleId = cart entityId)
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& c : connections_) {
                    if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                    auto h = c->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(h.get());
                    if (ph) ph->sendAttachEntity(*c, 0, player.getEntityId(), cart.entityId);
                }
                std::cout << "[Minecart] " << player.getPlayerName() << " mounted minecart " << cart.entityId << "\n";
            }
            return;
        }
    }

    // ─── Boat mounting — Java: EntityBoat.interactFirst() ────────────
    // Right-click on a boat to mount. Right-click while riding to dismount.
    {
        std::lock_guard<std::mutex> boatLock(boatEntitiesMutex_);
        for (auto& boat : boatEntities_) {
            if (boat.entityId != targetEntityId || boat.isDead) continue;

            if (boat.riderEntityId == player.getEntityId()) {
                // Dismount
                boat.riderEntityId = -1;
                player.setRidingEntityId(-1);

                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& c : connections_) {
                    if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                    auto h = c->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(h.get());
                    if (ph) ph->sendAttachEntity(*c, 0, player.getEntityId(), -1);
                }
                std::cout << "[Boat] " << player.getPlayerName() << " dismounted boat " << boat.entityId << "\n";
            } else if (boat.riderEntityId < 0) {
                // Mount — Java: player.mountEntity(boat)
                boat.riderEntityId = player.getEntityId();
                player.setRidingEntityId(boat.entityId);

                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& c : connections_) {
                    if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                    auto h = c->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(h.get());
                    if (ph) ph->sendAttachEntity(*c, 0, player.getEntityId(), boat.entityId);
                }
                std::cout << "[Boat] " << player.getPlayerName() << " mounted boat " << boat.entityId << "\n";
            }
            return;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Pig speed boost — Java: EntityAIControlledByPlayer.boostSpeed()
// Called by PlayHandler when player right-clicks carrot-on-a-stick while riding
// ═══════════════════════════════════════════════════════════════════════════

bool MinecraftServer::boostRiddenPig(int32_t riderEntityId) {
    std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
    for (auto& mob : mobEntities_) {
        if (mob.isDead || mob.mobType != 90 || mob.riderEntityId != riderEntityId) continue;
        if (!mob.isSaddled) return false;

        // Java: isControlledByPlayer() → rider has carrot_on_stick && !speedBoosted
        //                                && currentSpeed > maxSpeed * 0.3f
        constexpr float PIG_MAX_SPEED = 0.3f;
        if (mob.pigSpeedBoosted) return false;  // Already boosting
        if (mob.pigCurrentSpeed < PIG_MAX_SPEED * 0.3f) return false;  // Too slow

        // Java: boostSpeed() → speedBoosted = true, speedBoostTime = 0, maxSpeedBoostTime = rand(841)+140
        mob.pigSpeedBoosted = true;
        mob.pigSpeedBoostTime = 0;
        mob.pigMaxSpeedBoostTime = 140 + (rand() % 841);  // 140-980 ticks = 7-49 seconds

        std::cout << "[Pig] Speed boost activated on pig " << mob.entityId
                  << " (duration=" << mob.pigMaxSpeedBoostTime << " ticks)\n";
        return true;
    }
    return false;
}

// ═══════════════════════════════════════════════════════════════════════════
// Command helper methods
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::teleportPlayer(const std::string& playerName, double x, double y, double z) {
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (!ph || ph->getPlayerName() != playerName) continue;

        ph->setPlayerPosition(x, y, z);
        ph->resetFallDistance(); // Prevent phantom fall damage after teleport
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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

void MinecraftServer::addPlayerLevels(const std::string& playerName, int32_t levels) {
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (!ph || ph->getPlayerName() != playerName) continue;
        ph->addLevels(levels);
        ph->sendExperienceUpdate(*conn);
        return;
    }
}

void MinecraftServer::addPlayerExperience(const std::string& playerName, int32_t amount) {
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (!ph || ph->getPlayerName() != playerName) continue;
        ph->grantExperience(amount);
        ph->sendExperienceUpdate(*conn);
        return;
    }
}

void MinecraftServer::enchantPlayerItem(const std::string& playerName, int32_t enchId, int32_t level) {
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (!ph || ph->getPlayerName() != playerName) continue;
        ph->enchantHeldItem(enchId, level);
        ph->sendWindowItems(*conn); // Refresh inventory display
        return;
    }
}

int32_t MinecraftServer::clearPlayerInventory(const std::string& playerName, int32_t itemId, int32_t damage) {
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (!ph || ph->getPlayerName() != playerName) continue;
        int32_t count = ph->clearInventory(itemId, damage);
        ph->sendWindowItems(*conn);
        return count;
    }
    return 0;
}

void MinecraftServer::setPlayerSpawnPoint(const std::string& playerName, int32_t x, int32_t y, int32_t z) {
    // Send S05 SpawnPosition update to the target player
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (!ph || ph->getPlayerName() != playerName) continue;
        // S05 SpawnPosition: Int x, Int y, Int z
        std::vector<uint8_t> pkt;
        pkt.reserve(16);
        // VarInt packet ID
        pkt.push_back(0x05);
        // Int x
        pkt.push_back((x >> 24) & 0xFF);
        pkt.push_back((x >> 16) & 0xFF);
        pkt.push_back((x >> 8) & 0xFF);
        pkt.push_back(x & 0xFF);
        // Int y
        pkt.push_back((y >> 24) & 0xFF);
        pkt.push_back((y >> 16) & 0xFF);
        pkt.push_back((y >> 8) & 0xFF);
        pkt.push_back(y & 0xFF);
        // Int z
        pkt.push_back((z >> 24) & 0xFF);
        pkt.push_back((z >> 16) & 0xFF);
        pkt.push_back((z >> 8) & 0xFF);
        pkt.push_back(z & 0xFF);
        conn->sendPacket(std::move(pkt));
        return;
    }
}

std::vector<std::string> MinecraftServer::getOnlinePlayerNames() const {
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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

void MinecraftServer::sendPrivateMessage(const std::string& playerName, const std::string& message) {
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (!ph || ph->getPlayerName() != playerName) continue;
        ph->sendChatMessage(*conn, message);
        return;
    }
}

void MinecraftServer::kickPlayer(const std::string& playerName, const std::string& reason) {
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (!ph || ph->getPlayerName() != playerName) continue;
        // Send S40 Disconnect packet with reason (build manually)
        std::string json = "{\"text\":\"" + reason + "\"}";
        std::vector<uint8_t> pkt;
        pkt.push_back(0x40); // Disconnect packet ID (VarInt, single byte)
        // Write string length as VarInt
        uint32_t len = static_cast<uint32_t>(json.size());
        while (len > 0x7F) { pkt.push_back(static_cast<uint8_t>(len & 0x7F) | 0x80); len >>= 7; }
        pkt.push_back(static_cast<uint8_t>(len));
        // Write string bytes
        pkt.insert(pkt.end(), json.begin(), json.end());
        conn->sendPacket(std::move(pkt));
        conn->disconnect();
        std::cout << "[Server] Kicked " << playerName << ": " << reason << "\n";
        return;
    }
}

void MinecraftServer::setBlockInWorld(int32_t x, int32_t y, int32_t z, int32_t blockId, int32_t meta) {
    if (worlds_.empty()) return;
    auto& world = worlds_[0];
    world->setBlock(x, y, z, Block::getBlockById(blockId));
    if (meta != 0) {
        world->setBlockMetadata(x, y, z, meta);
    }
    broadcastBlockChange(x, y, z, blockId, meta);
}

int32_t MinecraftServer::getBlockIdInWorld(int32_t x, int32_t y, int32_t z) const {
    if (worlds_.empty()) return 0;
    Block* b = worlds_[0]->getBlock(x, y, z);
    return b ? Block::getIdFromBlock(b) : 0;
}

int32_t MinecraftServer::getBlockMetaInWorld(int32_t x, int32_t y, int32_t z) const {
    if (worlds_.empty()) return 0;
    return worlds_[0]->getBlockMetadata(x, y, z);
}

// Java: Block.getComparatorInputOverride() + Container.calcRedstoneFromInventory()
// Returns 0-15 for containers, or -1 if block has no comparator input override
int32_t MinecraftServer::getComparatorContainerSignal(int32_t x, int32_t y, int32_t z) const {
    int32_t blockId = getBlockIdInWorld(x, y, z);
    int64_t posKey = packBlockPos(x, y, z);

    // Helper: calculate redstone signal from container slots
    // Java: Container.calcRedstoneFromInventory(IInventory)
    auto calcSignal = [](int32_t totalItems, int32_t maxCapacity, int32_t slotCount) -> int32_t {
        if (slotCount == 0) return 0;
        if (totalItems == 0) return 0;
        float fillRatio = static_cast<float>(totalItems) / static_cast<float>(maxCapacity);
        return static_cast<int32_t>(fillRatio * 14.0f) + 1;
    };

    // Chest / Trapped Chest — 27 slots × 64 max
    if (blockId == 54 || blockId == 146) {
        auto it = chestStorage_.find(posKey);
        if (it == chestStorage_.end()) return 0;
        int32_t totalItems = 0;
        for (const auto& slot : it->second) {
            if (slot.has_value()) totalItems += slot->getStackSize();
        }
        return calcSignal(totalItems, 27 * 64, 27);
    }

    // Furnace — 3 slots × 64 max
    if (blockId == 61 || blockId == 62) {
        auto it = furnaceStorage_.find(posKey);
        if (it == furnaceStorage_.end()) return 0;
        int32_t totalItems = 0;
        for (int i = 0; i < 3; ++i) {
            if (it->second.slots[i].has_value()) totalItems += it->second.slots[i]->getStackSize();
        }
        return calcSignal(totalItems, 3 * 64, 3);
    }

    // Hopper — 5 slots × 64 max
    if (blockId == 154) {
        auto it = hopperStorage_.find(posKey);
        if (it == hopperStorage_.end()) return 0;
        int32_t totalItems = 0;
        for (const auto& slot : it->second.slots) {
            if (slot.has_value()) totalItems += slot->getStackSize();
        }
        return calcSignal(totalItems, 5 * 64, 5);
    }

    // Dispenser / Dropper — 9 slots × 64 max
    if (blockId == 23 || blockId == 158) {
        auto it = dispenserStorage_.find(posKey);
        if (it == dispenserStorage_.end()) return 0;
        int32_t totalItems = 0;
        for (const auto& slot : it->second.slots) {
            if (slot.has_value()) totalItems += slot->getStackSize();
        }
        return calcSignal(totalItems, 9 * 64, 9);
    }

    // Brewing Stand — 4 slots (3 potions × 1, 1 ingredient × 64)
    if (blockId == 117) {
        auto it = brewingStandStorage_.find(posKey);
        if (it == brewingStandStorage_.end()) return 0;
        int32_t totalItems = 0;
        for (const auto& slot : it->second.slots) {
            if (slot.has_value()) totalItems += slot->getStackSize();
        }
        // Java: 3 potion slots (max 1 each) + 1 ingredient slot (max 64)
        return calcSignal(totalItems, 3 * 1 + 64, 4);
    }

    // Cauldron — signal = water level meta (0-3)
    if (blockId == 118) {
        return getBlockMetaInWorld(x, y, z); // 0=empty, 1-3=water level
    }

    return -1; // No comparator override for this block
}

// Java: BlockDaylightDetector.func_149957_e() + TileEntityDaylightDetector.updateEntity()
// Computes power level (0-15) from world time + celestial angle
// Normal sensor (151) emits during daytime, Inverted (178) emits at night
int32_t MinecraftServer::getDaylightSensorPower(bool inverted) const {
    if (worlds_.empty()) return 0;

    int64_t worldTime = worlds_[0]->getWorldTime();

    // Java: World.getCelestialAngle(1.0f) — angle based on time of day
    // worldTime % 24000: 0 = sunrise, 6000 = noon, 12000 = sunset, 18000 = midnight
    float timeOfDay = static_cast<float>(worldTime % 24000) / 24000.0f;

    // Java: World.getCelestialAngle computes angle 0.0-1.0
    float angle = timeOfDay;
    if (angle > 1.0f) angle -= 1.0f;
    // Adjust: shift so noon is at 0 radians
    angle = angle + (angle * (1.0f - angle) * 0.2f - 0.25f);
    if (angle < 0.0f) angle += 1.0f;
    if (angle > 1.0f) angle -= 1.0f;

    // Convert to radians
    float radians = angle * 2.0f * 3.14159265f;

    // Java: skylightSubtracted (0=clear, up to 11 during storms)
    // Simplified: clear weather = 0, rain = 3, thunder = 5
    int32_t skylightSubtracted = 0;
    if (!worlds_.empty()) {
        auto* world = worlds_[0].get();
        if (world->getThunderingStrength() > 0.0f) skylightSubtracted = 5;
        else if (world->isRaining() && world->getRainingStrength() > 0.0f) skylightSubtracted = 3;
    }

    // Assume sky light at sensor = 15 (exposed to sky — typical for daylight sensors)
    int32_t skyLight = 15 - skylightSubtracted;

    // Java: n5 = Math.round((float)n5 * MathHelper.cos(f))
    int32_t signal = static_cast<int32_t>(std::round(static_cast<float>(skyLight) * std::cos(radians)));

    // Clamp 0-15
    if (signal < 0) signal = 0;
    if (signal > 15) signal = 15;

    if (inverted) {
        signal = 15 - signal;
        if (signal < 0) signal = 0;
    }

    return signal;
}

// ═══════════════════════════════════════════════════════════════════════════
// Redstone signal propagation — Java: World.notifyBlocksOfNeighborChange()
// Called when a redstone-relevant block changes (lever toggle, wire place/break,
// torch state change, repeater state change, etc.)
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::redstoneNotifyNeighbors(int32_t x, int32_t y, int32_t z) {
    // Recursion guard — prevent infinite loops from torch↔wire cascades
    // Java doesn't have this exact guard but torch burnout (8 toggles in 60 ticks) limits it
    static thread_local int32_t recursionDepth = 0;
    if (recursionDepth >= 64) return; // Max propagation depth
    ++recursionDepth;

    if (worlds_.empty()) { --recursionDepth; return; }
    auto& world = worlds_[0];

    // Build callbacks for the RedstoneSignal engine
    auto getBlockFn = [this](int32_t bx, int32_t by, int32_t bz) -> int32_t {
        return getBlockIdInWorld(bx, by, bz);
    };
    auto getMetaFn = [this](int32_t bx, int32_t by, int32_t bz) -> int32_t {
        return getBlockMetaInWorld(bx, by, bz);
    };
    auto setMetaFn = [this, &world](int32_t bx, int32_t by, int32_t bz, int32_t meta) {
        world->setBlockMetadata(bx, by, bz, meta);
        // Wire block ID stays 55; broadcast new power level (meta = signal 0-15)
        broadcastBlockChange(bx, by, bz, mccpp::RedstoneBlocks::REDSTONE_WIRE, meta);
    };
    auto notifyFn = [this](int32_t bx, int32_t by, int32_t bz) {
        // Check if a redstone lamp (123/124) needs to toggle
        int32_t nId = getBlockIdInWorld(bx, by, bz);
        if (nId == mccpp::RedstoneBlocks::REDSTONE_LAMP_OFF ||
            nId == mccpp::RedstoneBlocks::REDSTONE_LAMP_ON) {
            // Check if any adjacent block provides power to this lamp
            bool powered = false;
            for (int f = 0; f < 6 && !powered; ++f) {
                int32_t ax = bx + mccpp::FACING_OFFSETS[f].dx;
                int32_t ay = by + mccpp::FACING_OFFSETS[f].dy;
                int32_t az = bz + mccpp::FACING_OFFSETS[f].dz;
                int32_t adjId = getBlockIdInWorld(ax, ay, az);
                int32_t adjMeta = getBlockMetaInWorld(ax, ay, az);
                // Direct power sources
                if (adjId == mccpp::RedstoneBlocks::REDSTONE_BLOCK) powered = true;
                else if (adjId == mccpp::RedstoneBlocks::REDSTONE_TORCH_ON) powered = true;
                else if (adjId == mccpp::RedstoneBlocks::REDSTONE_WIRE && adjMeta > 0) powered = true;
                else if (adjId == mccpp::RedstoneBlocks::LEVER && mccpp::PowerSource::isLeverPowered(adjMeta)) powered = true;
                else if (adjId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON) {
                    // Check if repeater faces toward this lamp
                    int32_t repFacing = adjMeta & 0x3;
                    int32_t repOutFace;
                    switch (repFacing) {
                        case 0: repOutFace = 3; break; // S
                        case 1: repOutFace = 4; break; // W
                        case 2: repOutFace = 2; break; // N
                        default: repOutFace = 5; break; // E
                    }
                    if (repOutFace == mccpp::OPPOSITE_FACE[f]) powered = true;
                }
                else if ((adjId == mccpp::RedstoneBlocks::STONE_BUTTON ||
                          adjId == mccpp::RedstoneBlocks::WOODEN_BUTTON) &&
                         (adjMeta & 0x08)) powered = true;
                // Comparator ON — directional output like repeater
                else if (adjId == mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_ON) {
                    int32_t compFacing = adjMeta & 0x3;
                    int32_t compOutFace;
                    switch (compFacing) {
                        case 0: compOutFace = 3; break;
                        case 1: compOutFace = 4; break;
                        case 2: compOutFace = 2; break;
                        default: compOutFace = 5; break;
                    }
                    if (compOutFace == mccpp::OPPOSITE_FACE[f]) powered = true;
                }
                // Pressure plates — provides power when active
                else if ((adjId == 70 || adjId == 72 || adjId == 147 || adjId == 148) &&
                         (adjMeta & 0x01)) powered = true;
                // Daylight sensor — provides power based on world time
                else if ((adjId == 151 || adjId == 178) && getDaylightSensorPower(adjId == 178) > 0) powered = true;
                // Tripwire hook — provides power when powered (entity on wire)
                else if (adjId == 131 && (adjMeta & 0x08)) powered = true;
                // Detector rail — provides power when entity on rail (meta bit 0x08)
                // Java: BlockRailDetector.isProvidingWeakPower
                else if (adjId == 28 && (adjMeta & 0x08)) powered = true;
            }
            int32_t newLampId = powered ? mccpp::RedstoneBlocks::REDSTONE_LAMP_ON
                                        : mccpp::RedstoneBlocks::REDSTONE_LAMP_OFF;
            if (newLampId != nId) {
                setBlockInWorld(bx, by, bz, newLampId, 0);
            }
        }
    };
    auto isSolidFn = [this](int32_t bx, int32_t by, int32_t bz) -> bool {
        int32_t id = getBlockIdInWorld(bx, by, bz);
        // Java: Block.isNormalCube() — simplified for common solid blocks
        if (id == 0) return false; // air
        // Non-solid: wire, torch, repeater, comparator, rail, flower, glass pane,
        // slab (lower), fence, ladder, sign, chest, snow layer, button, pressure plate, etc.
        switch (id) {
            case 6: case 18: case 20: case 26: case 27: case 28: case 30: case 31:
            case 32: case 34: case 36: case 37: case 38: case 39: case 40: case 44:
            case 50: case 51: case 55: case 59: case 63: case 64: case 65: case 66:
            case 68: case 69: case 70: case 71: case 72: case 75: case 76: case 77:
            case 78: case 83: case 85: case 90: case 92: case 93: case 94: case 96:
            case 101: case 102: case 104: case 105: case 106: case 107: case 111:
            case 113: case 115: case 117: case 119: case 126: case 127: case 131:
            case 132: case 139: case 140: case 141: case 142: case 143: case 144:
            case 145: case 147: case 148: case 149: case 150: case 151: case 154:
            case 157: case 160: case 161: case 167: case 171: case 175:
                return false;
            default:
                return true; // Most blocks are solid
        }
    };

    // ─── Update adjacent redstone wire ────────────────────────────────
    // Check all 6 neighbors + diagonals for wire and update it
    constexpr int32_t offsets[6][3] = {{-1,0,0},{1,0,0},{0,-1,0},{0,1,0},{0,0,-1},{0,0,1}};
    for (const auto& off : offsets) {
        int32_t nx = x + off[0], ny = y + off[1], nz = z + off[2];
        int32_t nId = getBlockIdInWorld(nx, ny, nz);
        if (nId == mccpp::RedstoneBlocks::REDSTONE_WIRE) {
            mccpp::RedstoneSignal::updateSurroundingRedstone(
                nx, ny, nz, getBlockFn, getMetaFn, setMetaFn, notifyFn, isSolidFn);
        }
    }

    // ─── Also check for wire at this exact position ──────────────────
    if (getBlockIdInWorld(x, y, z) == mccpp::RedstoneBlocks::REDSTONE_WIRE) {
        mccpp::RedstoneSignal::updateSurroundingRedstone(
            x, y, z, getBlockFn, getMetaFn, setMetaFn, notifyFn, isSolidFn);
    }

    // ─── Update redstone lamps at this position ──────────────────────
    notifyFn(x, y, z);
    for (const auto& off : offsets) {
        notifyFn(x + off[0], y + off[1], z + off[2]);
    }

    // ─── Redstone torch state changes — Java: BlockRedstoneTorch.updateTick ─
    // Check if any adjacent redstone torch needs to toggle based on block power
    for (const auto& off : offsets) {
        int32_t nx = x + off[0], ny = y + off[1], nz = z + off[2];
        int32_t nId = getBlockIdInWorld(nx, ny, nz);
        if (nId == mccpp::RedstoneBlocks::REDSTONE_TORCH_ON ||
            nId == mccpp::RedstoneBlocks::REDSTONE_TORCH_OFF) {
            int32_t torchMeta = getBlockMetaInWorld(nx, ny, nz);
            // Determine attached block position from metadata
            int32_t attachX = nx, attachY = ny, attachZ = nz;
            switch (torchMeta) {
                case 1: attachX = nx - 1; break; // east wall → attached to west block
                case 2: attachX = nx + 1; break; // west wall → attached to east block
                case 3: attachZ = nz - 1; break; // south wall → attached to north block
                case 4: attachZ = nz + 1; break; // north wall → attached to south block
                case 5: attachY = ny - 1; break; // floor → attached to block below
                default: continue;
            }
            // Check if the attached block is receiving power
            bool attachedPowered = false;
            for (const auto& off2 : offsets) {
                int32_t ax = attachX + off2[0], ay = attachY + off2[1], az = attachZ + off2[2];
                if (ax == nx && ay == ny && az == nz) continue; // Don't check the torch itself
                int32_t adjId = getBlockIdInWorld(ax, ay, az);
                int32_t adjMeta = getBlockMetaInWorld(ax, ay, az);
                if (adjId == mccpp::RedstoneBlocks::REDSTONE_WIRE && adjMeta > 0) {
                    attachedPowered = true; break;
                }
                if (adjId == mccpp::RedstoneBlocks::LEVER && mccpp::PowerSource::isLeverPowered(adjMeta)) {
                    attachedPowered = true; break;
                }
                if (adjId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON) {
                    attachedPowered = true; break;
                }
            }
            // Toggle torch: powered input → torch OFF; no power → torch ON
            int32_t expectedTorchId = attachedPowered
                ? mccpp::RedstoneBlocks::REDSTONE_TORCH_OFF
                : mccpp::RedstoneBlocks::REDSTONE_TORCH_ON;
            if (expectedTorchId != nId) {
                // ─── Burnout protection — Java: BlockRedstoneTorch.updateTick ───
                // Track this toggle event; 8+ toggles in 60 ticks = burnout
                int64_t currentTick = tickCounter_.load();

                // Clean up old toggles outside the 60-tick window
                torchToggleList_.erase(
                    std::remove_if(torchToggleList_.begin(), torchToggleList_.end(),
                        [currentTick](const auto& t) {
                            return currentTick - std::get<3>(t) > 60;
                        }),
                    torchToggleList_.end());

                // Count recent toggles at this position
                int32_t toggleCount = 0;
                for (const auto& t : torchToggleList_) {
                    if (std::get<0>(t) == nx && std::get<1>(t) == ny && std::get<2>(t) == nz) {
                        ++toggleCount;
                    }
                }

                // Record this toggle
                torchToggleList_.emplace_back(nx, ny, nz, currentTick);

                if (toggleCount >= 8) {
                    // Burned out! Force torch OFF with smoke particles
                    // Java: world.setBlock → unlit + smoke particles + fire.fizz
                    setBlockInWorld(nx, ny, nz, mccpp::RedstoneBlocks::REDSTONE_TORCH_OFF, torchMeta);
                    broadcastSound("random.fizz",
                        static_cast<double>(nx) + 0.5, static_cast<double>(ny) + 0.5,
                        static_cast<double>(nz) + 0.5, 0.5f, 2.6f);
                    // Smoke particles — Java: spawnParticle("smoke", ...)
                    broadcastParticle("smoke",
                        static_cast<float>(nx) + 0.5f, static_cast<float>(ny) + 0.5f,
                        static_cast<float>(nz) + 0.5f,
                        0.0f, 0.0f, 0.0f, 0.02f, 5);
                } else {
                    setBlockInWorld(nx, ny, nz, expectedTorchId, torchMeta);
                    // Torch state change → propagate further
                    redstoneNotifyNeighbors(nx, ny, nz);
                }
            }
        }
    }

    // ─── Piston state update — Java: BlockPistonBase.onNeighborBlockChange ──
    // Check if any adjacent piston needs to extend or retract
    for (const auto& off : offsets) {
        int32_t nx = x + off[0], ny = y + off[1], nz = z + off[2];
        int32_t nId = getBlockIdInWorld(nx, ny, nz);
        if (nId == mccpp::RedstoneBlocks::PISTON ||
            nId == mccpp::RedstoneBlocks::STICKY_PISTON) {
            pistonUpdateState(nx, ny, nz, nId);
        }
    }
    // Also check the block itself (e.g. piston that was just placed)
    {
        int32_t selfId = getBlockIdInWorld(x, y, z);
        if (selfId == mccpp::RedstoneBlocks::PISTON ||
            selfId == mccpp::RedstoneBlocks::STICKY_PISTON) {
            pistonUpdateState(x, y, z, selfId);
        }
    }

    // ─── Dispenser/Dropper firing — Java: BlockDispenser.onNeighborBlockChange ──
    // Check if any adjacent dispenser/dropper should fire
    for (const auto& off : offsets) {
        int32_t nx = x + off[0], ny = y + off[1], nz = z + off[2];
        int32_t nId = getBlockIdInWorld(nx, ny, nz);
        if (nId == mccpp::RedstoneBlocks::DISPENSER ||
            nId == mccpp::RedstoneBlocks::DROPPER) {
            dispenserFire(nx, ny, nz, nId);
        }
    }
    // Self-check for dispenser/dropper placed next to power
    {
        int32_t selfId = getBlockIdInWorld(x, y, z);
        if (selfId == mccpp::RedstoneBlocks::DISPENSER ||
            selfId == mccpp::RedstoneBlocks::DROPPER) {
            dispenserFire(x, y, z, selfId);
        }
    }

    // ─── TNT ignition — Java: BlockTNT.onNeighborBlockChange ────────────
    // TNT ignites immediately when receiving redstone power
    auto tntIgnitePowered = [this](int32_t tx, int32_t ty, int32_t tz) {
        // Check if TNT block is receiving power from any adjacent source
        bool powered = false;
        for (int face = 0; face < 6; ++face) {
            int32_t nx = tx + mccpp::FACING_OFFSETS[face].dx;
            int32_t ny = ty + mccpp::FACING_OFFSETS[face].dy;
            int32_t nz = tz + mccpp::FACING_OFFSETS[face].dz;
            int32_t adjId = getBlockIdInWorld(nx, ny, nz);
            int32_t adjMeta = getBlockMetaInWorld(nx, ny, nz);
            if (adjId == mccpp::RedstoneBlocks::REDSTONE_WIRE && adjMeta > 0) { powered = true; break; }
            if (adjId == mccpp::RedstoneBlocks::REDSTONE_TORCH_ON) { powered = true; break; }
            if (adjId == mccpp::RedstoneBlocks::REDSTONE_BLOCK) { powered = true; break; }
            if (adjId == mccpp::RedstoneBlocks::LEVER && mccpp::PowerSource::isLeverPowered(adjMeta)) {
                powered = true; break;
            }
            if ((adjId == mccpp::RedstoneBlocks::STONE_BUTTON ||
                 adjId == mccpp::RedstoneBlocks::WOODEN_BUTTON) && (adjMeta & 0x08)) {
                powered = true; break;
            }
            if (adjId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON) {
                powered = true; break;
            }
            if (adjId == mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_ON) {
                powered = true; break;
            }
            if ((adjId == 70 || adjId == 72 || adjId == 147 || adjId == 148) &&
                (adjMeta & 0x01)) {
                powered = true; break;
            }
            if ((adjId == 151 || adjId == 178) && getDaylightSensorPower(adjId == 178) > 0) {
                powered = true; break;
            }
            // Tripwire hook — provides power when powered (entity on wire)
            if (adjId == 131 && (adjMeta & 0x08)) {
                powered = true; break;
            }
            // Detector rail — provides power when entity on rail
            if (adjId == 28 && (adjMeta & 0x08)) {
                powered = true; break;
            }
        }
        if (powered) {
            // Java: onBlockDestroyedByPlayer(world, x,y,z, 1) → func_150114_a → spawn EntityTNTPrimed
            setBlockInWorld(tx, ty, tz, 0, 0); // Remove TNT block
            broadcastSound("game.tnt.primed",
                static_cast<double>(tx) + 0.5, static_cast<double>(ty) + 0.5,
                static_cast<double>(tz) + 0.5, 1.0f, 1.0f);
            // Schedule explosion (80 tick fuse → instant for our simplified server)
            createExplosion(
                static_cast<double>(tx) + 0.5,
                static_cast<double>(ty) + 0.5,
                static_cast<double>(tz) + 0.5,
                4.0f, true, true);
        }
    };

    for (const auto& off : offsets) {
        int32_t nx = x + off[0], ny = y + off[1], nz = z + off[2];
        if (getBlockIdInWorld(nx, ny, nz) == mccpp::RedstoneBlocks::TNT) {
            tntIgnitePowered(nx, ny, nz);
        }
    }
    // Self-check: TNT placed next to power
    if (getBlockIdInWorld(x, y, z) == mccpp::RedstoneBlocks::TNT) {
        tntIgnitePowered(x, y, z);
    }

    // ─── Note block — Java: BlockNote.onNeighborBlockChange ─────────────
    // Rising-edge: play note when receiving power
    auto noteBlockCheckPowered = [this](int32_t nx, int32_t ny, int32_t nz) {
        bool powered = false;
        for (int face = 0; face < 6; ++face) {
            int32_t ax = nx + mccpp::FACING_OFFSETS[face].dx;
            int32_t ay = ny + mccpp::FACING_OFFSETS[face].dy;
            int32_t az = nz + mccpp::FACING_OFFSETS[face].dz;
            int32_t adjId = getBlockIdInWorld(ax, ay, az);
            int32_t adjMeta = getBlockMetaInWorld(ax, ay, az);
            if (adjId == mccpp::RedstoneBlocks::REDSTONE_WIRE && adjMeta > 0) { powered = true; break; }
            if (adjId == mccpp::RedstoneBlocks::REDSTONE_TORCH_ON) { powered = true; break; }
            if (adjId == mccpp::RedstoneBlocks::REDSTONE_BLOCK) { powered = true; break; }
            if (adjId == mccpp::RedstoneBlocks::LEVER && mccpp::PowerSource::isLeverPowered(adjMeta)) {
                powered = true; break;
            }
            if ((adjId == mccpp::RedstoneBlocks::STONE_BUTTON ||
                 adjId == mccpp::RedstoneBlocks::WOODEN_BUTTON) && (adjMeta & 0x08)) {
                powered = true; break;
            }
            if (adjId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON) { powered = true; break; }
            if (adjId == mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_ON) { powered = true; break; }
            if ((adjId == 70 || adjId == 72 || adjId == 147 || adjId == 148) &&
                (adjMeta & 0x01)) { powered = true; break; }
            if ((adjId == 151 || adjId == 178) && getDaylightSensorPower(adjId == 178) > 0) { powered = true; break; }
            // Tripwire hook — provides power when powered (entity on wire)
            if (adjId == 131 && (adjMeta & 0x08)) { powered = true; break; }
            // Detector rail — provides power when entity on rail
            if (adjId == 28 && (adjMeta & 0x08)) { powered = true; break; }
        }
        if (powered) {
            playNoteBlock(nx, ny, nz);
        }
    };

    for (const auto& off : offsets) {
        int32_t nx = x + off[0], ny = y + off[1], nz = z + off[2];
        if (getBlockIdInWorld(nx, ny, nz) == mccpp::RedstoneBlocks::NOTE_BLOCK) {
            noteBlockCheckPowered(nx, ny, nz);
        }
    }
    if (getBlockIdInWorld(x, y, z) == mccpp::RedstoneBlocks::NOTE_BLOCK) {
        noteBlockCheckPowered(x, y, z);
    }

    // ─── Repeater state toggle — Java: BlockRedstoneDiode.onNeighborBlockChange ──
    // Facing (meta & 3): 0=south, 1=west, 2=north, 3=east
    // Input comes from behind the repeater (opposite of facing direction)
    // Delay: repeaterState[(meta & 0xC) >> 2] * 2 = {2,4,6,8} ticks
    static const int repeaterInputDx[] = { 0, -1,  0,  1}; // input offset X for facing 0,1,2,3
    static const int repeaterInputDz[] = { 1,  0, -1,  0}; // input offset Z for facing 0,1,2,3
    static const int repeaterDelays[] = {2, 4, 6, 8}; // delay ticks for settings 0-3

    auto checkRepeaterInput = [this](int32_t rx, int32_t ry, int32_t rz, int32_t facing) -> bool {
        // Check if the block behind the repeater provides power
        int32_t ix = rx + repeaterInputDx[facing];
        int32_t iz = rz + repeaterInputDz[facing];
        int32_t inputId = getBlockIdInWorld(ix, ry, iz);
        int32_t inputMeta = getBlockMetaInWorld(ix, ry, iz);

        // Power sources that can power a repeater input:
        if (inputId == mccpp::RedstoneBlocks::REDSTONE_WIRE && inputMeta > 0) return true;
        if (inputId == mccpp::RedstoneBlocks::REDSTONE_TORCH_ON) return true;
        if (inputId == mccpp::RedstoneBlocks::REDSTONE_BLOCK) return true;
        if (inputId == mccpp::RedstoneBlocks::LEVER && mccpp::PowerSource::isLeverPowered(inputMeta)) return true;
        if ((inputId == mccpp::RedstoneBlocks::STONE_BUTTON || inputId == mccpp::RedstoneBlocks::WOODEN_BUTTON)
            && (inputMeta & 0x08)) return true;
        if ((inputId == 151 || inputId == 178) && getDaylightSensorPower(inputId == 178) > 0) return true;
        // Tripwire hook — provides power when powered
        if (inputId == 131 && (inputMeta & 0x08)) return true;
        // Detector rail — provides power when entity on rail
        if (inputId == 28 && (inputMeta & 0x08)) return true;
        // Another repeater powering into this repeater's input
        if (inputId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON) {
            int32_t otherFacing = inputMeta & 3;
            // The other repeater's output direction
            // Output is in the facing direction (opposite of input)
            int32_t outDx = -repeaterInputDx[otherFacing];
            int32_t outDz = -repeaterInputDz[otherFacing];
            // It powers this repeater if its output points at us
            if (ix + outDx == rx && iz + outDz == rz) return true;
        }
        // Strong power from solid blocks: check if the input block is solid and powered
        // (e.g., a block receiving power from a torch underneath)
        // Simplified: skip for now
        return false;
    };

    auto scheduleDiodeUpdate = [this, &checkRepeaterInput](int32_t rx, int32_t ry, int32_t rz) {
        int32_t blockId = getBlockIdInWorld(rx, ry, rz);
        int32_t delayTicks = 0;
        bool isOn = false;

        if (blockId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_OFF ||
            blockId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON) {
            int32_t meta = getBlockMetaInWorld(rx, ry, rz);
            int32_t delaySetting = (meta & 0x0C) >> 2;
            delayTicks = repeaterDelays[delaySetting];
            isOn = (blockId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON);

            // Java: BlockRedstoneDiode.func_149910_g — locked repeaters skip updates
            auto getBlockFn = [this](int32_t bx, int32_t by, int32_t bz) -> int32_t {
                return getBlockIdInWorld(bx, by, bz);
            };
            auto getMetaFn = [this](int32_t bx, int32_t by, int32_t bz) -> int32_t {
                return getBlockMetaInWorld(bx, by, bz);
            };
            if (mccpp::RedstoneRepeater::isLocked(rx, ry, rz, meta, getBlockFn, getMetaFn)) {
                return; // Locked — do not schedule any state change
            }
        } else if (blockId == mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_OFF ||
                   blockId == mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_ON) {
            delayTicks = 2; // Java: BlockRedstoneComparator.func_149901_b always returns 2
            isOn = (blockId == mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_ON);
        } else {
            return;
        }

        int32_t meta = getBlockMetaInWorld(rx, ry, rz);
        int32_t facing = meta & 3;
        bool inputPowered = checkRepeaterInput(rx, ry, rz, facing);

        // Schedule update if state needs to change
        if (isOn && !inputPowered) {
            scheduleBlockTick(rx, ry, rz, blockId, delayTicks);
        } else if (!isOn && inputPowered) {
            scheduleBlockTick(rx, ry, rz, blockId, delayTicks);
        }
    };

    for (const auto& off : offsets) {
        int32_t nx = x + off[0], ny = y + off[1], nz = z + off[2];
        scheduleDiodeUpdate(nx, ny, nz);
    }
    scheduleDiodeUpdate(x, y, z);

    --recursionDepth;
}

// ═══════════════════════════════════════════════════════════════════════════
// Piston extend/retract — Java: BlockPistonBase.updatePistonState()
// Instant server-side push/pull (no TileEntityPiston animation)
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::pistonUpdateState(int32_t x, int32_t y, int32_t z, int32_t blockId) {
    if (worlds_.empty()) return;
    auto& world = worlds_[0];

    bool isSticky = (blockId == mccpp::RedstoneBlocks::STICKY_PISTON);
    int32_t meta = getBlockMetaInWorld(x, y, z);
    int32_t direction = mccpp::PistonMechanics::getOrientation(meta);
    bool isExtended = mccpp::PistonMechanics::isExtended(meta);

    if (direction == 7) return; // Invalid orientation

    // Build callbacks for PistonMechanics::isIndirectlyPowered
    auto getBlockFn = [this](int32_t bx, int32_t by, int32_t bz) -> int32_t {
        return getBlockIdInWorld(bx, by, bz);
    };
    auto getMetaFn = [this](int32_t bx, int32_t by, int32_t bz) -> int32_t {
        return getBlockMetaInWorld(bx, by, bz);
    };

    bool powered = mccpp::PistonMechanics::isIndirectlyPowered(
        x, y, z, direction, getBlockFn, getMetaFn);

    // Also check lever directly attached to the piston block
    for (int face = 0; face < 6; ++face) {
        if (face == direction) continue;
        int32_t nx = x + mccpp::FACING_OFFSETS[face].dx;
        int32_t ny = y + mccpp::FACING_OFFSETS[face].dy;
        int32_t nz = z + mccpp::FACING_OFFSETS[face].dz;
        int32_t adjId = getBlockIdInWorld(nx, ny, nz);
        int32_t adjMeta = getBlockMetaInWorld(nx, ny, nz);
        if (adjId == mccpp::RedstoneBlocks::LEVER && mccpp::PowerSource::isLeverPowered(adjMeta))
            powered = true;
        if ((adjId == mccpp::RedstoneBlocks::STONE_BUTTON ||
             adjId == mccpp::RedstoneBlocks::WOODEN_BUTTON) && (adjMeta & 0x08))
            powered = true;
    }

    int32_t dx = mccpp::FACING_OFFSETS[direction].dx;
    int32_t dy = mccpp::FACING_OFFSETS[direction].dy;
    int32_t dz = mccpp::FACING_OFFSETS[direction].dz;

    // ─── EXTEND ──────────────────────────────────────────────────────
    if (powered && !isExtended) {
        // Check canExtend — walk forward up to 12 blocks
        bool canPush = true;
        int32_t pushCount = 0;
        for (int32_t i = 1; i <= mccpp::PistonMechanics::MAX_PUSH_DISTANCE + 1; ++i) {
            int32_t bx = x + dx * i, by = y + dy * i, bz = z + dz * i;
            int32_t bid = getBlockIdInWorld(bx, by, bz);
            if (by < 0 || by > 255) { canPush = false; break; }
            if (bid == 0) { pushCount = i - 1; break; } // Air = end of chain
            if (mccpp::PistonMechanics::isUnpushable(bid)) { canPush = false; break; }
            if (!mccpp::PistonMechanics::canPushBlock(bid, true)) { canPush = false; break; }
            if (i > mccpp::PistonMechanics::MAX_PUSH_DISTANCE) { canPush = false; break; }
            pushCount = i;
        }

        if (!canPush) return;

        // Move blocks from farthest to nearest (to avoid overwriting)
        for (int32_t i = pushCount; i >= 1; --i) {
            int32_t srcX = x + dx * i, srcY = y + dy * i, srcZ = z + dz * i;
            int32_t dstX = x + dx * (i + 1), dstY = y + dy * (i + 1), dstZ = z + dz * (i + 1);
            int32_t srcId = getBlockIdInWorld(srcX, srcY, srcZ);
            int32_t srcMeta = getBlockMetaInWorld(srcX, srcY, srcZ);
            setBlockInWorld(dstX, dstY, dstZ, srcId, srcMeta);
        }

        // Place piston head (34) at extend position
        int32_t headX = x + dx, headY = y + dy, headZ = z + dz;
        // Piston head meta: direction | (isSticky ? 0x08 : 0x00)
        int32_t headMeta = direction | (isSticky ? 0x08 : 0x00);
        setBlockInWorld(headX, headY, headZ, mccpp::RedstoneBlocks::PISTON_HEAD, headMeta);

        // Mark piston base as extended (meta bit 0x08)
        world->setBlockMetadata(x, y, z, direction | 0x08);
        broadcastBlockChange(x, y, z, blockId, direction | 0x08);

        // Sound — Java: tile.piston.out
        broadcastSound("tile.piston.out",
            static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5,
            static_cast<double>(z) + 0.5, 0.5f, 0.7f);

        // Trigger redstone updates for moved blocks
        for (int32_t i = 0; i <= pushCount + 1; ++i) {
            redstoneNotifyNeighbors(x + dx * i, y + dy * i, z + dz * i);
        }
    }
    // ─── RETRACT ─────────────────────────────────────────────────────
    else if (!powered && isExtended) {
        // Remove piston head
        int32_t headX = x + dx, headY = y + dy, headZ = z + dz;
        int32_t headId = getBlockIdInWorld(headX, headY, headZ);
        if (headId == mccpp::RedstoneBlocks::PISTON_HEAD) {
            // Sticky piston: pull block from position after head
            if (isSticky) {
                int32_t pullX = x + dx * 2, pullY = y + dy * 2, pullZ = z + dz * 2;
                int32_t pullId = getBlockIdInWorld(pullX, pullY, pullZ);
                int32_t pullMeta = getBlockMetaInWorld(pullX, pullY, pullZ);
                if (pullId != 0 && !mccpp::PistonMechanics::isUnpushable(pullId) &&
                    mccpp::PistonMechanics::canPushBlock(pullId, false)) {
                    // Move pulled block to head position
                    setBlockInWorld(headX, headY, headZ, pullId, pullMeta);
                    // Clear pulled position
                    setBlockInWorld(pullX, pullY, pullZ, 0, 0);
                } else {
                    // Nothing to pull, just clear head
                    setBlockInWorld(headX, headY, headZ, 0, 0);
                }
            } else {
                // Normal piston: just remove head
                setBlockInWorld(headX, headY, headZ, 0, 0);
            }
        }

        // Mark piston base as retracted (clear bit 0x08)
        world->setBlockMetadata(x, y, z, direction);
        broadcastBlockChange(x, y, z, blockId, direction);

        // Sound — Java: tile.piston.in
        broadcastSound("tile.piston.in",
            static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5,
            static_cast<double>(z) + 0.5, 0.5f, 0.7f);

        // Trigger redstone updates around piston
        redstoneNotifyNeighbors(headX, headY, headZ);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Dispenser/Dropper fire — Java: BlockDispenser.func_149941_e()
// Instant fire on redstone power (skips 4-tick schedule for simplicity)
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::dispenserFire(int32_t x, int32_t y, int32_t z, int32_t blockId) {
    if (worlds_.empty()) return;
    auto& world = worlds_[0];

    int32_t meta = getBlockMetaInWorld(x, y, z);
    int32_t direction = meta & 0x07; // facing: 0=down,1=up,2=N,3=S,4=W,5=E
    bool wasTriggered = (meta & 0x08) != 0;

    // Check if receiving power (same as piston check pattern)
    bool powered = false;
    for (int face = 0; face < 6; ++face) {
        int32_t nx = x + mccpp::FACING_OFFSETS[face].dx;
        int32_t ny = y + mccpp::FACING_OFFSETS[face].dy;
        int32_t nz = z + mccpp::FACING_OFFSETS[face].dz;
        int32_t adjId = getBlockIdInWorld(nx, ny, nz);
        int32_t adjMeta = getBlockMetaInWorld(nx, ny, nz);
        if (adjId == mccpp::RedstoneBlocks::REDSTONE_WIRE && adjMeta > 0) { powered = true; break; }
        if (adjId == mccpp::RedstoneBlocks::REDSTONE_TORCH_ON) { powered = true; break; }
        if (adjId == mccpp::RedstoneBlocks::REDSTONE_BLOCK) { powered = true; break; }
        if (adjId == mccpp::RedstoneBlocks::LEVER && mccpp::PowerSource::isLeverPowered(adjMeta)) {
            powered = true; break;
        }
        if ((adjId == mccpp::RedstoneBlocks::STONE_BUTTON ||
             adjId == mccpp::RedstoneBlocks::WOODEN_BUTTON) && (adjMeta & 0x08)) {
            powered = true; break;
        }
        if (adjId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON) {
            // Check repeater facing toward this block
            int32_t repFacing = adjMeta & 0x3;
            int32_t oppFace = mccpp::OPPOSITE_FACE[face];
            int32_t repOutFace = -1;
            switch (repFacing) {
                case 0: repOutFace = 3; break;
                case 1: repOutFace = 4; break;
                case 2: repOutFace = 2; break;
                case 3: repOutFace = 5; break;
            }
            if (repOutFace == oppFace) { powered = true; break; }
        }
    }
    // Also check block above (Java: world.isBlockIndirectlyGettingPowered checks all + above)
    {
        int32_t aboveId = getBlockIdInWorld(x, y + 1, z);
        int32_t aboveMeta = getBlockMetaInWorld(x, y + 1, z);
        if (aboveId == mccpp::RedstoneBlocks::REDSTONE_WIRE && aboveMeta > 0) powered = true;
        if (aboveId == mccpp::RedstoneBlocks::REDSTONE_TORCH_ON) powered = true;
        if ((aboveId == 151 || aboveId == 178) && getDaylightSensorPower(aboveId == 178) > 0) powered = true;
    }

    // Java: triggered on rising edge only (powered && !wasTriggered)
    if (powered && !wasTriggered) {
        // Set triggered bit
        world->setBlockMetadata(x, y, z, meta | 0x08);
        broadcastBlockChange(x, y, z, blockId, meta | 0x08);

        // Get dispenser inventory
        int64_t posKey = packBlockPos(x, y, z);
        auto& disp = getOrCreateDispenser(posKey);

        // Pick random non-empty slot — Java: TileEntityDispenser.func_146017_i()
        std::vector<int> occupiedSlots;
        for (int i = 0; i < 9; ++i) {
            if (disp.slots[i].has_value() && disp.slots[i]->getStackSize() > 0) {
                occupiedSlots.push_back(i);
            }
        }

        if (occupiedSlots.empty()) {
            // Empty dispenser: click sound — Java: world.playAuxSFX(1001)
            broadcastSound("random.click",
                static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5,
                static_cast<double>(z) + 0.5, 1.0f, 1.2f);
            return;
        }

        // Random slot selection
        int slotIdx = occupiedSlots[static_cast<int>(
            std::chrono::steady_clock::now().time_since_epoch().count() % occupiedSlots.size())];
        auto& stack = disp.slots[slotIdx];
        int32_t itemId = stack->getItemId();
        int32_t itemDamage = stack->getDamage();

        // Face offset for dispensing position
        int32_t dx = mccpp::FACING_OFFSETS[direction].dx;
        int32_t dy = mccpp::FACING_OFFSETS[direction].dy;
        int32_t dz = mccpp::FACING_OFFSETS[direction].dz;
        double spawnX = static_cast<double>(x) + 0.5 + static_cast<double>(dx) * 0.7;
        double spawnY = static_cast<double>(y) + 0.5 + static_cast<double>(dy) * 0.7;
        double spawnZ = static_cast<double>(z) + 0.5 + static_cast<double>(dz) * 0.7;

        bool isDropper = (blockId == mccpp::RedstoneBlocks::DROPPER);

        if (isDropper) {
            // Dropper: try to inject into adjacent inventory, else drop as entity
            // Check if facing a container (chest, furnace, hopper, dispenser, dropper)
            int32_t targetX = x + dx, targetY = y + dy, targetZ = z + dz;
            int32_t targetBlock = getBlockIdInWorld(targetX, targetY, targetZ);

            bool injected = false;
            // Try injecting into chest
            if (targetBlock == 54 || targetBlock == 146) { // chest, trapped chest
                auto& chest = getOrCreateChest(targetX, targetY, targetZ);
                for (int i = 0; i < 27; ++i) {
                    if (!chest[i].has_value()) {
                        chest[i] = ItemStack(itemId, 1, itemDamage);
                        injected = true;
                        break;
                    } else if (chest[i]->getItemId() == itemId &&
                               chest[i]->getDamage() == itemDamage &&
                               chest[i]->getStackSize() < 64) {
                        chest[i]->setStackSize(chest[i]->getStackSize() + 1);
                        injected = true;
                        break;
                    }
                }
            }
            // Try furnace
            else if (targetBlock == 61 || targetBlock == 62) { // furnace
                auto& furnace = getOrCreateFurnace(targetX, targetY, targetZ);
                // Try input slot first (0), then fuel slot (1)
                for (int slot : {0, 1}) {
                    if (!furnace.slots[slot].has_value()) {
                        furnace.slots[slot] = ItemStack(itemId, 1, itemDamage);
                        injected = true;
                        break;
                    } else if (furnace.slots[slot]->getItemId() == itemId &&
                               furnace.slots[slot]->getDamage() == itemDamage &&
                               furnace.slots[slot]->getStackSize() < 64) {
                        furnace.slots[slot]->setStackSize(furnace.slots[slot]->getStackSize() + 1);
                        injected = true;
                        break;
                    }
                }
            }
            // Try hopper
            else if (targetBlock == 154) {
                auto& hopper = getOrCreateHopper(packBlockPos(targetX, targetY, targetZ));
                for (int i = 0; i < 5; ++i) {
                    if (!hopper.slots[i].has_value()) {
                        hopper.slots[i] = ItemStack(itemId, 1, itemDamage);
                        injected = true;
                        break;
                    } else if (hopper.slots[i]->getItemId() == itemId &&
                               hopper.slots[i]->getDamage() == itemDamage &&
                               hopper.slots[i]->getStackSize() < 64) {
                        hopper.slots[i]->setStackSize(hopper.slots[i]->getStackSize() + 1);
                        injected = true;
                        break;
                    }
                }
            }
            // Try dispenser/dropper
            else if (targetBlock == 23 || targetBlock == 158) {
                auto& targetDisp = getOrCreateDispenser(packBlockPos(targetX, targetY, targetZ));
                for (int i = 0; i < 9; ++i) {
                    if (!targetDisp.slots[i].has_value()) {
                        targetDisp.slots[i] = ItemStack(itemId, 1, itemDamage);
                        injected = true;
                        break;
                    } else if (targetDisp.slots[i]->getItemId() == itemId &&
                               targetDisp.slots[i]->getDamage() == itemDamage &&
                               targetDisp.slots[i]->getStackSize() < 64) {
                        targetDisp.slots[i]->setStackSize(targetDisp.slots[i]->getStackSize() + 1);
                        injected = true;
                        break;
                    }
                }
            }

            if (!injected) {
                // Drop as entity
                spawnItemDrop(spawnX, spawnY, spawnZ, itemId, itemDamage, 1);
            }
        } else {
            // Dispenser: behavior depends on item type
            // Java: BehaviorProjectileDispense for arrows, snowballs, eggs, etc.
            switch (itemId) {
                case 262: { // Arrow — spawn arrow projectile
                    spawnArrow(spawnX, spawnY, spawnZ,
                        static_cast<double>(dx), static_cast<double>(dy),
                        static_cast<double>(dz),
                        -1, 2.0, 0, false, 1.1f, 6.0f);
                    break;
                }
                case 332: { // Snowball
                    spawnThrowable(ThrowableType::Snowball, spawnX, spawnY, spawnZ,
                        static_cast<double>(dx) * 1.1, static_cast<double>(dy) * 1.1,
                        static_cast<double>(dz) * 1.1,
                        -1, "");
                    break;
                }
                case 344: { // Egg
                    spawnThrowable(ThrowableType::Egg, spawnX, spawnY, spawnZ,
                        static_cast<double>(dx) * 1.1, static_cast<double>(dy) * 1.1,
                        static_cast<double>(dz) * 1.1,
                        -1, "");
                    break;
                }
                case 384: { // Exp bottle
                    spawnThrowable(ThrowableType::ExpBottle, spawnX, spawnY, spawnZ,
                        static_cast<double>(dx) * 1.1, static_cast<double>(dy) * 1.1,
                        static_cast<double>(dz) * 1.1,
                        -1, "");
                    break;
                }
                case 385: { // Fire charge — set fire at target position
                    int32_t fx = x + dx, fy = y + dy, fz = z + dz;
                    if (getBlockIdInWorld(fx, fy, fz) == 0) {
                        setBlockInWorld(fx, fy, fz, 51, 0); // fire block
                    }
                    broadcastSound("mob.ghast.fireball",
                        static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5,
                        static_cast<double>(z) + 0.5, 1.0f, 1.0f);
                    break;
                }
                case 326: { // Water bucket — place water source
                    int32_t wx = x + dx, wy = y + dy, wz = z + dz;
                    if (getBlockIdInWorld(wx, wy, wz) == 0) {
                        setBlockInWorld(wx, wy, wz, 9, 0); // still water
                    }
                    // Replace with empty bucket in slot
                    stack = ItemStack(325, 1, 0); // empty bucket
                    // Sound
                    broadcastSound("random.splash",
                        static_cast<double>(wx) + 0.5, static_cast<double>(wy) + 0.5,
                        static_cast<double>(wz) + 0.5, 1.0f, 1.0f);
                    return; // Don't consume — we replaced item
                }
                case 327: { // Lava bucket — place lava source
                    int32_t lx = x + dx, ly = y + dy, lz = z + dz;
                    if (getBlockIdInWorld(lx, ly, lz) == 0) {
                        setBlockInWorld(lx, ly, lz, 11, 0); // still lava
                    }
                    stack = ItemStack(325, 1, 0); // empty bucket
                    broadcastSound("random.splash",
                        static_cast<double>(lx) + 0.5, static_cast<double>(ly) + 0.5,
                        static_cast<double>(lz) + 0.5, 1.0f, 1.0f);
                    return; // Don't consume  
                }
                case 46: { // TNT — ignite TNT
                    // Place and immediately ignite at target
                    createExplosion(
                        static_cast<double>(x + dx) + 0.5,
                        static_cast<double>(y + dy) + 0.5,
                        static_cast<double>(z + dz) + 0.5,
                        4.0f, true, true);
                    break;
                }
                case 383: { // Spawn Egg — spawn mob of type from damage value
                    // Java: BehaviorMobEggDispense
                    int32_t mobType = itemDamage;
                    if (mobType > 0) {
                        summonMob(static_cast<uint8_t>(mobType),
                            spawnX, spawnY, spawnZ);
                    }
                    break;
                }
                case 259: { // Flint and Steel — place fire or ignite TNT
                    int32_t fx = x + dx, fy = y + dy, fz = z + dz;
                    int32_t targetId = getBlockIdInWorld(fx, fy, fz);
                    if (targetId == 0) {
                        setBlockInWorld(fx, fy, fz, 51, 0); // fire block
                    } else if (targetId == 46) {
                        // TNT ignition
                        setBlockInWorld(fx, fy, fz, 0, 0);
                        createExplosion(
                            static_cast<double>(fx) + 0.5,
                            static_cast<double>(fy) + 0.5,
                            static_cast<double>(fz) + 0.5,
                            4.0f, true, true);
                    }
                    broadcastSound("fire.ignite",
                        static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5,
                        static_cast<double>(z) + 0.5, 1.0f, 1.0f);
                    // Damage flint and steel instead of consuming
                    if (stack->getDamage() + 1 >= 65) { // max durability 64
                        disp.slots[slotIdx] = std::nullopt;
                    } else {
                        stack->setDamage(stack->getDamage() + 1);
                    }
                    return; // Don't consume normally
                }
                case 351: { // Dye — bonemeal (damage 15) grows crops at target
                    if (itemDamage == 15) {
                        int32_t tx = x + dx, ty = y + dy, tz = z + dz;
                        int32_t targetId = getBlockIdInWorld(tx, ty, tz);
                        int32_t targetMeta = getBlockMetaInWorld(tx, ty, tz);
                        bool grew = false;
                        // Wheat/carrot/potato
                        if ((targetId == 59 || targetId == 141 || targetId == 142) && targetMeta < 7) {
                            int32_t growth = targetMeta + 2 + (std::rand() % 4); // +2-5
                            if (growth > 7) growth = 7;
                            setBlockInWorld(tx, ty, tz, targetId, growth);
                            grew = true;
                        }
                        // Sapling
                        else if (targetId == 6 && !(targetMeta & 0x08)) {
                            setBlockInWorld(tx, ty, tz, targetId, targetMeta | 0x08);
                            grew = true;
                        }
                        if (grew) {
                            broadcastEffect(2005, tx, ty, tz, 0); // green particles
                        }
                    }
                    break;
                }
                default: {
                    // Default behavior: drop item as entity
                    spawnItemDrop(spawnX, spawnY, spawnZ, itemId, itemDamage, 1);
                    break;
                }
            }
        }

        // Consume 1 item from the slot
        int32_t remaining = stack->getStackSize() - 1;
        if (remaining <= 0) {
            disp.slots[slotIdx] = std::nullopt;
        } else {
            stack->setStackSize(remaining);
        }

        // Sound effect — Java: random.click
        broadcastSound("random.click",
            static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5,
            static_cast<double>(z) + 0.5, 1.0f, 1.0f);

        // Smoke particle — Java: effect 2000 with data = direction
        broadcastEffect(2000, x, y, z, direction);
    }
    else if (!powered && wasTriggered) {
        // Clear triggered bit on power loss
        world->setBlockMetadata(x, y, z, meta & ~0x08);
        broadcastBlockChange(x, y, z, blockId, meta & ~0x08);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Note block play — Java: TileEntityNote.triggerNote() + BlockNote.onBlockEventReceived()
// Instrument detection from block below, pitch from metadata
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::playNoteBlock(int32_t x, int32_t y, int32_t z) {
    if (worlds_.empty()) return;
    auto& world = worlds_[0];

    int32_t meta = getBlockMetaInWorld(x, y, z); // pitch 0-24

    // Java: BlockNote.onBlockEventReceived — instrument from block below
    // 0=harp (default), 1=bass drum (rock), 2=snare (sand), 3=hat (glass), 4=bass attack (wood)
    int32_t belowId = getBlockIdInWorld(x, y - 1, z);
    int instrument = 0; // harp
    switch (belowId) {
        // Rock-type blocks → bass drum
        case 1: case 4: case 7: case 14: case 15: case 16: case 21:
        case 22: case 23: case 24: case 42: case 43: case 44: case 45:
        case 48: case 49: case 56: case 61: case 62: case 73: case 74:
        case 87: case 98: case 109: case 112: case 113: case 116: case 121:
        case 129: case 145: case 152: case 153: case 155:
            instrument = 1; // bd (bass drum)
            break;
        // Sand-type blocks → snare drum
        case 12: case 13: case 82: case 88: case 159: case 172: case 174:
            instrument = 2; // snare
            break;
        // Glass blocks → hi-hat (sticks)
        case 20: case 89: case 102: case 160:
            instrument = 3; // hat
            break;
        // Wood-type blocks → bass attack
        case 5: case 17: case 25: case 47: case 53: case 54: case 58:
        case 84: case 85: case 96: case 107: case 126: case 134: case 135:
        case 136: case 162: case 163: case 164:
            instrument = 4; // bassattack
            break;
        default:
            instrument = 0; // harp
            break;
    }

    // Java: float f = (float)Math.pow(2.0, (n5 - 12) / 12.0);
    float pitch = static_cast<float>(std::pow(2.0, (meta - 12.0) / 12.0));

    const char* instrumentNames[] = { "harp", "bd", "snare", "hat", "bassattack" };
    std::string soundName = std::string("note.") + instrumentNames[instrument];

    // Java: volume=3.0f
    broadcastSound(soundName,
        static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5,
        static_cast<double>(z) + 0.5, 3.0f, pitch);

    // Java: spawnParticle("note", x+0.5, y+1.2, z+0.5, note/24.0, 0, 0)
    broadcastParticle("note",
        static_cast<float>(x) + 0.5f, static_cast<float>(y) + 1.2f,
        static_cast<float>(z) + 0.5f,
        static_cast<float>(meta) / 24.0f, 0.0f, 0.0f, 0.0f, 0);
}

// ═══════════════════════════════════════════════════════════════════════════
// Tripwire hook chain update — Java: BlockTripWireHook.func_150136_a()
// Scans from hook along its facing direction for up to 42 blocks, finds
// opposing hook, checks entity presence on wire, updates powered/connected.
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::tripwireHookUpdate(int32_t hookX, int32_t hookY, int32_t hookZ,
                                          bool isBeingRemoved, int32_t blockMeta) {
    if (worlds_.empty()) return;

    // Get hook metadata — direction and current flags
    int32_t hookId = getBlockIdInWorld(hookX, hookY, hookZ);
    int32_t meta = blockMeta;
    if (meta < 0) {
        if (hookId != 131) return; // Not a hook
        meta = getBlockMetaInWorld(hookX, hookY, hookZ);
    }

    // Java: BlockTripWireHook direction bits (0-3 in low 2 bits)
    // 0=south(+Z), 1=west(-X), 2=north(-Z), 3=east(+X)
    int32_t facing = meta & 0x03;

    // Scan direction — the direction the hook faces (away from wall, into the wire chain)
    // Java: Facing.offsetsXForSide[facing], Facing.offsetsZForSide[facing]
    int32_t dx = 0, dz = 0;
    switch (facing) {
        case 0: dz =  1; break; // south → scan +Z
        case 1: dx = -1; break; // west  → scan -X
        case 2: dz = -1; break; // north → scan -Z
        case 3: dx =  1; break; // east  → scan +X
    }

    // Scan up to 42 blocks (Java: MAX_TRIPWIRE_LENGTH = 42, exclusive, so 41 wire + 1 hook)
    bool foundOpposingHook = false;
    bool entityOnWire = false;
    int32_t chainLength = 0;
    int32_t opposingHookX = hookX, opposingHookZ = hookZ;

    for (int32_t i = 1; i <= 42; ++i) {
        int32_t scanX = hookX + dx * i;
        int32_t scanZ = hookZ + dz * i;
        int32_t scanId = getBlockIdInWorld(scanX, hookY, scanZ);

        if (scanId == 131) {
            // Found opposing hook — Java: check that it faces back toward us
            int32_t otherMeta = getBlockMetaInWorld(scanX, hookY, scanZ);
            int32_t otherFacing = otherMeta & 0x03;
            // Opposing hook must face the opposite direction
            // facing 0↔2 (south↔north), facing 1↔3 (west↔east)
            if (otherFacing == ((facing + 2) & 0x03)) {
                foundOpposingHook = true;
                opposingHookX = scanX;
                opposingHookZ = scanZ;
                chainLength = i;
            }
            break; // Stop scanning regardless
        }

        if (scanId == 132) {
            // Wire block — Java: BlockTripWire, check for entities
            // Java: func_150140_e() — check if any entity intersects this wire block
            // Check all connected players + mobs
            {
                std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph || ph->isDead()) continue;
                    double px = ph->getPlayerX();
                    double py = ph->getPlayerY();
                    double pz = ph->getPlayerZ();
                    // Java: AxisAlignedBB.addCoord — wire hitbox is 0.0-1.0 X/Z, 0.09375-0.15625 Y
                    // Check if player AABB intersects the wire block AABB
                    // Player AABB: ±0.3 width, 0→1.8 height
                    if (px + 0.3 > static_cast<double>(scanX) &&
                        px - 0.3 < static_cast<double>(scanX) + 1.0 &&
                        py + 1.8 > static_cast<double>(hookY) + 0.09375 &&
                        py       < static_cast<double>(hookY) + 0.15625 &&
                        pz + 0.3 > static_cast<double>(scanZ) &&
                        pz - 0.3 < static_cast<double>(scanZ) + 1.0) {
                        entityOnWire = true;
                        break;
                    }
                }
            }
            if (!entityOnWire) {
                std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
                for (auto& mob : mobEntities_) {
                    if (mob.isDead) continue;
                    // Mob hitbox: simplified ±0.3 width, 0→1.8 height
                    double mx = mob.posX, my = mob.posY, mz = mob.posZ;
                    if (mx + 0.3 > static_cast<double>(scanX) &&
                        mx - 0.3 < static_cast<double>(scanX) + 1.0 &&
                        my + 1.8 > static_cast<double>(hookY) + 0.09375 &&
                        my       < static_cast<double>(hookY) + 0.15625 &&
                        mz + 0.3 > static_cast<double>(scanZ) &&
                        mz - 0.3 < static_cast<double>(scanZ) + 1.0) {
                        entityOnWire = true;
                        break;
                    }
                }
            }
            continue; // Keep scanning
        }

        // Any other block (including air) breaks the chain scan
        break;
    }

    // ─── Update hook states ──────────────────────────────────────────────
    // Java: BlockTripWireHook metadata bits:
    //   bit 2 (0x04) = connected (has valid opposing hook + wire chain)
    //   bit 3 (0x08) = powered (triggered — entity on wire)
    // Only update if we have a valid chain
    bool wasConnected = (meta & 0x04) != 0;
    bool wasPowered   = (meta & 0x08) != 0;

    bool nowConnected = foundOpposingHook;
    bool nowPowered   = foundOpposingHook && entityOnWire;

    // Update this hook's metadata
    if (!isBeingRemoved) {
        int32_t newMeta = (meta & 0x03); // Keep direction bits
        if (nowConnected) newMeta |= 0x04;
        if (nowPowered)   newMeta |= 0x08;

        if (newMeta != meta) {
            setBlockInWorld(hookX, hookY, hookZ, 131, newMeta);
        }
    }

    // Update opposing hook's metadata
    if (foundOpposingHook) {
        int32_t otherMeta = getBlockMetaInWorld(opposingHookX, hookY, opposingHookZ);
        int32_t newOtherMeta = (otherMeta & 0x03);
        if (nowConnected && !isBeingRemoved) newOtherMeta |= 0x04;
        if (nowPowered)   newOtherMeta |= 0x08;

        if (newOtherMeta != otherMeta) {
            setBlockInWorld(opposingHookX, hookY, opposingHookZ, 131, newOtherMeta);
        }
    }

    // Update wire metadata — Java: BlockTripWire metadata bits:
    //   bit 0 (0x01) = powered (entity on wire)
    //   bit 2 (0x04) = connected (part of valid hook chain)
    //   bit 6 (0x40) = disarmed (broken by shears, no trigger)
    if (foundOpposingHook) {
        for (int32_t i = 1; i < chainLength; ++i) {
            int32_t wx = hookX + dx * i;
            int32_t wz = hookZ + dz * i;
            int32_t wireId = getBlockIdInWorld(wx, hookY, wz);
            if (wireId != 132) continue;

            int32_t wireMeta = getBlockMetaInWorld(wx, hookY, wz);
            int32_t newWireMeta = wireMeta & ~0x05; // Clear powered + connected bits
            if (nowConnected && !isBeingRemoved) newWireMeta |= 0x04; // connected
            if (nowPowered)   newWireMeta |= 0x01; // powered

            if (newWireMeta != wireMeta) {
                setBlockInWorld(wx, hookY, wz, 132, newWireMeta);
            }
        }
    }

    // ─── Redstone notification ───────────────────────────────────────────
    // Only notify if power state changed (rising or falling edge)
    if (wasPowered != nowPowered || wasConnected != nowConnected) {
        redstoneNotifyNeighbors(hookX, hookY, hookZ);
        if (foundOpposingHook) {
            redstoneNotifyNeighbors(opposingHookX, hookY, opposingHookZ);
        }

        // Java: BlockTripWireHook also notifies the block the hook is attached to
        // The hook is on the wall at the opposite direction of its facing
        int32_t attachX = hookX - dx;
        int32_t attachZ = hookZ - dz;
        redstoneNotifyNeighbors(attachX, hookY, attachZ);

        if (foundOpposingHook) {
            int32_t otherAttachX = opposingHookX + dx;
            int32_t otherAttachZ = opposingHookZ + dz;
            redstoneNotifyNeighbors(otherAttachX, hookY, otherAttachZ);
        }
    }

    // Play activation sound — Java: "random.click"
    if (wasPowered != nowPowered) {
        float clickPitch = nowPowered ? 0.6f : 0.5f; // Java: powered=0.6, unpowered=0.5
        broadcastSound("random.click",
            static_cast<double>(hookX) + 0.5, static_cast<double>(hookY) + 0.5,
            static_cast<double>(hookZ) + 0.5, 0.4f, clickPitch);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Scheduled block ticks — Java: WorldServer.scheduleBlockUpdateWithPriority
// Used for button auto-reset (20/30 ticks), repeater delay, etc.
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::scheduleBlockTick(int32_t x, int32_t y, int32_t z,
                                         int32_t blockId, int32_t delay) {
    std::lock_guard<std::mutex> lock(scheduledTicksMutex_);
    int64_t fireTick = tickCounter_.load() + delay;
    scheduledTicks_.push_back({x, y, z, blockId, fireTick});
}

void MinecraftServer::tickScheduledBlocks() {
    std::vector<ScheduledBlockTick> expired;
    {
        std::lock_guard<std::mutex> lock(scheduledTicksMutex_);
        int64_t now = tickCounter_.load();
        auto it = scheduledTicks_.begin();
        while (it != scheduledTicks_.end()) {
            if (it->fireTick <= now) {
                expired.push_back(*it);
                it = scheduledTicks_.erase(it);
            } else {
                ++it;
            }
        }
    }

    for (auto& tick : expired) {
        int32_t currentId = getBlockIdInWorld(tick.x, tick.y, tick.z);
        if (currentId != tick.blockId) continue; // Block changed, skip

        // Java: BlockButton.updateTick — depress the button
        if (tick.blockId == 77 || tick.blockId == 143) { // Stone/wooden button
            if (worlds_.empty()) continue;
            auto& world = worlds_[0];
            int32_t meta = world->getBlockMetadata(tick.x, tick.y, tick.z);
            if (meta & 0x08) { // Still pressed
                meta &= ~0x08;
                world->setBlockMetadata(tick.x, tick.y, tick.z, meta);
                broadcastBlockChange(tick.x, tick.y, tick.z, tick.blockId, meta);
                broadcastSound("random.click",
                    static_cast<double>(tick.x) + 0.5,
                    static_cast<double>(tick.y) + 0.5,
                    static_cast<double>(tick.z) + 0.5,
                    0.3f, 0.5f); // Lower pitch for depress
                redstoneNotifyNeighbors(tick.x, tick.y, tick.z);
            }
        }

        // Java: BlockTripWire.updateTick — re-poll entity presence
        // The wire schedules a tick when an entity triggers it; on expiry,
        // re-check and update the connected hooks
        if (tick.blockId == 132) {
            // Scan all 4 directions for connected hooks, same as block break scan
            for (int dir = 0; dir < 4; ++dir) {
                int32_t hdx = 0, hdz = 0;
                switch (dir) {
                    case 0: hdz =  1; break;
                    case 1: hdx = -1; break;
                    case 2: hdz = -1; break;
                    case 3: hdx =  1; break;
                }
                for (int32_t i = 1; i <= 42; ++i) {
                    int32_t sx = tick.x + hdx * i;
                    int32_t sz = tick.z + hdz * i;
                    int32_t sid = getBlockIdInWorld(sx, tick.y, sz);
                    if (sid == 131) {
                        int32_t sm = getBlockMetaInWorld(sx, tick.y, sz);
                        int32_t expectedFacing = (dir + 2) & 0x03;
                        if ((sm & 0x03) == expectedFacing) {
                            tripwireHookUpdate(sx, tick.y, sz);
                        }
                        break;
                    }
                    if (sid != 132) break;
                }
            }
        }

        // Java: BlockRailDetector.updateTick — re-poll entity presence
        // tickRate = 20; if no entity remains, deactivate the rail
        if (tick.blockId == 28) {
            if (!worlds_.empty()) {
                auto& world = worlds_[0];
                int32_t meta = world->getBlockMetadata(tick.x, tick.y, tick.z);
                if (meta & 0x08) {
                    // Check if any player is still on this rail
                    bool entityOnRail = false;
                    double railMinX = static_cast<double>(tick.x);
                    double railMaxX = railMinX + 1.0;
                    double railMinZ = static_cast<double>(tick.z);
                    double railMaxZ = railMinZ + 1.0;
                    double railMinY = static_cast<double>(tick.y);
                    double railMaxY = railMinY + 0.625; // Rail AABB height
                    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
                    for (auto& conn : connections_) {
                        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                        auto handler = conn->getHandler();
                        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                        if (!ph || ph->isDead()) continue;
                        double px = ph->getPlayerX();
                        double py = ph->getPlayerY();
                        double pz = ph->getPlayerZ();
                        // Player AABB: 0.6 wide, 1.8 tall
                        if (px + 0.3 > railMinX && px - 0.3 < railMaxX &&
                            pz + 0.3 > railMinZ && pz - 0.3 < railMaxZ &&
                            py + 1.8 > railMinY && py < railMaxY) {
                            entityOnRail = true;
                            break;
                        }
                    }
                    if (entityOnRail) {
                        // Still occupied — reschedule tick
                        scheduleBlockTick(tick.x, tick.y, tick.z, 28, 20);
                    } else {
                        // Deactivate: clear power bit 0x08
                        world->setBlockMetadata(tick.x, tick.y, tick.z, meta & ~0x08);
                        broadcastBlockChange(tick.x, tick.y, tick.z, 28, meta & ~0x08);
                        broadcastSound("random.click",
                            static_cast<double>(tick.x) + 0.5,
                            static_cast<double>(tick.y) + 0.5,
                            static_cast<double>(tick.z) + 0.5,
                            0.3f, 0.5f); // Lower pitch for depress
                        redstoneNotifyNeighbors(tick.x, tick.y, tick.z);
                        // Also notify block below — Java: notifyBlocksOfNeighborChange(x,y-1,z)
                        redstoneNotifyNeighbors(tick.x, tick.y - 1, tick.z);
                    }
                }
            }
        }

        // Java: BlockRedstoneDiode.updateTick — toggle repeater ON/OFF
        // Repeater OFF (93) → ON (94) or ON (94) → OFF (93)
        if (tick.blockId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_OFF ||
            tick.blockId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON) {
            if (worlds_.empty()) continue;
            auto& world = worlds_[0];
            int32_t meta = world->getBlockMetadata(tick.x, tick.y, tick.z);
            int32_t facing = meta & 3;

            // Re-check input power at the time of the tick
            static const int rInputDx[] = { 0, -1,  0,  1};
            static const int rInputDz[] = { 1,  0, -1,  0};
            int32_t ix = tick.x + rInputDx[facing];
            int32_t iz = tick.z + rInputDz[facing];
            int32_t inputId = getBlockIdInWorld(ix, tick.y, iz);
            int32_t inputMeta = getBlockMetaInWorld(ix, tick.y, iz);

            bool inputPowered = false;
            if (inputId == mccpp::RedstoneBlocks::REDSTONE_WIRE && inputMeta > 0) inputPowered = true;
            else if (inputId == mccpp::RedstoneBlocks::REDSTONE_TORCH_ON) inputPowered = true;
            else if (inputId == mccpp::RedstoneBlocks::REDSTONE_BLOCK) inputPowered = true;
            else if (inputId == mccpp::RedstoneBlocks::LEVER && mccpp::PowerSource::isLeverPowered(inputMeta)) inputPowered = true;
            else if ((inputId == mccpp::RedstoneBlocks::STONE_BUTTON || inputId == mccpp::RedstoneBlocks::WOODEN_BUTTON) && (inputMeta & 0x08)) inputPowered = true;
            else if (inputId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON) {
                int32_t otherFacing = inputMeta & 3;
                int32_t outDx = -rInputDx[otherFacing];
                int32_t outDz = -rInputDz[otherFacing];
                if (ix + outDx == tick.x && iz + outDz == tick.z) inputPowered = true;
            }
            else if ((inputId == 151 || inputId == 178) && getDaylightSensorPower(inputId == 178) > 0) inputPowered = true;

            bool isOn = (tick.blockId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON);

            // Java: BlockRedstoneDiode.func_149910_g — skip toggle if locked
            auto getBlockFn = [this](int32_t bx, int32_t by, int32_t bz) -> int32_t {
                return getBlockIdInWorld(bx, by, bz);
            };
            auto getMetaFn = [this](int32_t bx, int32_t by, int32_t bz) -> int32_t {
                return getBlockMetaInWorld(bx, by, bz);
            };
            if (mccpp::RedstoneRepeater::isLocked(tick.x, tick.y, tick.z, meta, getBlockFn, getMetaFn)) {
                continue; // Locked — state frozen, skip toggle
            }

            if (isOn && !inputPowered) {
                // Turn OFF: switch block 94 → 93, preserve metadata
                world->setBlock(tick.x, tick.y, tick.z, Block::getBlockById(mccpp::RedstoneBlocks::REDSTONE_REPEATER_OFF));
                world->setBlockMetadata(tick.x, tick.y, tick.z, meta);
                broadcastBlockChange(tick.x, tick.y, tick.z, mccpp::RedstoneBlocks::REDSTONE_REPEATER_OFF, meta);
                broadcastSound("random.click",
                    static_cast<double>(tick.x) + 0.5, static_cast<double>(tick.y) + 0.5,
                    static_cast<double>(tick.z) + 0.5, 0.3f, 0.5f);
                redstoneNotifyNeighbors(tick.x, tick.y, tick.z);
            } else if (!isOn && inputPowered) {
                // Turn ON: switch block 93 → 94, preserve metadata
                world->setBlock(tick.x, tick.y, tick.z, Block::getBlockById(mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON));
                world->setBlockMetadata(tick.x, tick.y, tick.z, meta);
                broadcastBlockChange(tick.x, tick.y, tick.z, mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON, meta);
                broadcastSound("random.click",
                    static_cast<double>(tick.x) + 0.5, static_cast<double>(tick.y) + 0.5,
                    static_cast<double>(tick.z) + 0.5, 0.3f, 0.6f);
                redstoneNotifyNeighbors(tick.x, tick.y, tick.z);
            }
        }

        // Java: BlockRedstoneComparator.updateTick — full signal-level logic
        // Comparator OFF (149) → ON (150) or ON (150) → OFF (149)
        // Uses signal levels (0-15), compare/subtract modes, and container input
        if (tick.blockId == mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_OFF ||
            tick.blockId == mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_ON) {
            if (worlds_.empty()) continue;
            auto& world = worlds_[0];
            int32_t meta = world->getBlockMetadata(tick.x, tick.y, tick.z);
            int32_t facing = meta & 3;
            bool isSubtractMode = (meta & 0x4) != 0;

            // Java: Direction.offsetX/Z — input direction (behind comparator)
            // facing: 0=S(+Z), 1=W(-X), 2=N(-Z), 3=E(+X)
            static const int cInputDx[] = { 0, -1,  0,  1};
            static const int cInputDz[] = { 1,  0, -1,  0};
            int32_t ix = tick.x + cInputDx[facing];
            int32_t iz = tick.z + cInputDz[facing];

            // ─── getInputStrength — Java: BlockRedstoneComparator.getInputStrength ───
            // Step 1: Get normal redstone input strength
            int32_t rearSignal = 0;
            {
                int32_t inputId = getBlockIdInWorld(ix, tick.y, iz);
                int32_t inputMeta = getBlockMetaInWorld(ix, tick.y, iz);
                if (inputId == mccpp::RedstoneBlocks::REDSTONE_WIRE) {
                    rearSignal = inputMeta; // Wire power level 0-15
                } else if (inputId == mccpp::RedstoneBlocks::REDSTONE_TORCH_ON) {
                    rearSignal = 15;
                } else if (inputId == mccpp::RedstoneBlocks::REDSTONE_BLOCK) {
                    rearSignal = 15;
                } else if (inputId == mccpp::RedstoneBlocks::LEVER && mccpp::PowerSource::isLeverPowered(inputMeta)) {
                    rearSignal = 15;
                } else if ((inputId == mccpp::RedstoneBlocks::STONE_BUTTON || inputId == mccpp::RedstoneBlocks::WOODEN_BUTTON) && (inputMeta & 0x08)) {
                    rearSignal = 15;
                } else if (inputId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON) {
                    int32_t otherFacing = inputMeta & 3;
                    int32_t outDx = -cInputDx[otherFacing];
                    int32_t outDz = -cInputDz[otherFacing];
                    if (ix + outDx == tick.x && iz + outDz == tick.z) rearSignal = 15;
                } else if (inputId == mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_ON) {
                    rearSignal = 15; // Comparator output is always 15 when ON (simplified)
                } else if (inputId == 151 || inputId == 178) {
                    rearSignal = getDaylightSensorPower(inputId == 178);
                }
            }

            // Step 2: Check for container input override — Java: hasComparatorInputOverride
            {
                int32_t containerSignal = getComparatorContainerSignal(ix, tick.y, iz);
                if (containerSignal >= 0) {
                    rearSignal = containerSignal; // Container signal overrides redstone
                } else if (rearSignal < 15) {
                    // Java: check through solid block for container 2 blocks behind
                    int32_t behindId = getBlockIdInWorld(ix, tick.y, iz);
                    // If input block is a normal solid cube, check one more block behind
                    bool isSolid = (behindId != 0 && behindId != mccpp::RedstoneBlocks::REDSTONE_WIRE &&
                                    behindId != mccpp::RedstoneBlocks::REDSTONE_TORCH_ON &&
                                    behindId != mccpp::RedstoneBlocks::REDSTONE_TORCH_OFF &&
                                    behindId != mccpp::RedstoneBlocks::REDSTONE_REPEATER_OFF &&
                                    behindId != mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON &&
                                    behindId != mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_OFF &&
                                    behindId != mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_ON &&
                                    behindId != 20 && behindId != 102); // not glass/glass_pane
                    if (isSolid) {
                        int32_t ix2 = ix + cInputDx[facing];
                        int32_t iz2 = iz + cInputDz[facing];
                        int32_t behindContainer = getComparatorContainerSignal(ix2, tick.y, iz2);
                        if (behindContainer >= 0) {
                            rearSignal = behindContainer;
                        }
                    }
                }
            }

            // ─── Side input strength — Java: getMaxInputStrengthFromSides ───
            // Check side signals (perpendicular to facing)
            int32_t sideSignal = 0;
            {
                // Side offsets perpendicular to facing
                static const int cSideDx1[] = { -1,  0,  1,  0}; // left
                static const int cSideDz1[] = {  0, -1,  0,  1};
                static const int cSideDx2[] = {  1,  0, -1,  0}; // right
                static const int cSideDz2[] = {  0,  1,  0, -1};

                for (int s = 0; s < 2; ++s) {
                    int32_t sx = tick.x + (s == 0 ? cSideDx1[facing] : cSideDx2[facing]);
                    int32_t sz = tick.z + (s == 0 ? cSideDz1[facing] : cSideDz2[facing]);
                    int32_t sId = getBlockIdInWorld(sx, tick.y, sz);
                    int32_t sMeta = getBlockMetaInWorld(sx, tick.y, sz);

                    int32_t sidePower = 0;
                    if (sId == mccpp::RedstoneBlocks::REDSTONE_WIRE) {
                        sidePower = sMeta;
                    } else if (sId == mccpp::RedstoneBlocks::REDSTONE_REPEATER_ON) {
                        // Only counts if repeater output faces this comparator
                        int32_t repFacing = sMeta & 3;
                        int32_t repOutDx = -cInputDx[repFacing];
                        int32_t repOutDz = -cInputDz[repFacing];
                        if (sx + repOutDx == tick.x && sz + repOutDz == tick.z) {
                            sidePower = 15;
                        }
                    } else if (sId == mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_ON) {
                        sidePower = 15;
                    }
                    sideSignal = std::max(sideSignal, sidePower);
                }
            }

            // ─── getOutputStrength — Java: BlockRedstoneComparator.getOutputStrength ───
            int32_t outputStrength = 0;
            if (isSubtractMode) {
                outputStrength = std::max(0, rearSignal - sideSignal);
            } else {
                // Compare mode: output = rear if rear >= side
                outputStrength = (rearSignal >= sideSignal) ? rearSignal : 0;
            }

            bool shouldBeOn = (outputStrength > 0);
            bool isOn = (tick.blockId == mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_ON);

            if (isOn && !shouldBeOn) {
                // Turn OFF: switch 150 → 149, preserve metadata
                world->setBlock(tick.x, tick.y, tick.z, Block::getBlockById(mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_OFF));
                world->setBlockMetadata(tick.x, tick.y, tick.z, meta);
                broadcastBlockChange(tick.x, tick.y, tick.z, mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_OFF, meta);
                broadcastSound("random.click",
                    static_cast<double>(tick.x) + 0.5, static_cast<double>(tick.y) + 0.5,
                    static_cast<double>(tick.z) + 0.5, 0.3f, 0.5f);
                redstoneNotifyNeighbors(tick.x, tick.y, tick.z);
            } else if (!isOn && shouldBeOn) {
                // Turn ON: switch 149 → 150, preserve metadata
                world->setBlock(tick.x, tick.y, tick.z, Block::getBlockById(mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_ON));
                world->setBlockMetadata(tick.x, tick.y, tick.z, meta);
                broadcastBlockChange(tick.x, tick.y, tick.z, mccpp::RedstoneBlocks::REDSTONE_COMPARATOR_ON, meta);
                broadcastSound("random.click",
                    static_cast<double>(tick.x) + 0.5, static_cast<double>(tick.y) + 0.5,
                    static_cast<double>(tick.z) + 0.5, 0.3f, 0.6f);
                redstoneNotifyNeighbors(tick.x, tick.y, tick.z);
            }
        }
    }
}

// Java: SharedMonsterAttributes.maxHealth base values per entity type
static float getMobMaxHealth(uint8_t mobType) {
    switch (mobType) {
        case 50: return 20.0f;  // Creeper
        case 51: return 20.0f;  // Skeleton
        case 52: return 16.0f;  // Spider
        case 54: return 20.0f;  // Zombie
        case 55: return 16.0f;  // Slime (small; large=16*size)
        case 56: return 10.0f;  // Ghast
        case 57: return 20.0f;  // Zombie Pigman
        case 58: return 40.0f;  // Enderman
        case 59: return 12.0f;  // Cave Spider
        case 60: return 8.0f;   // Silverfish
        case 61: return 20.0f;  // Blaze
        case 62: return 16.0f;  // Magma Cube
        case 63: return 200.0f; // Ender Dragon
        case 64: return 300.0f; // Wither
        case 65: return 10.0f;  // Bat
        case 66: return 26.0f;  // Witch
        case 90: return 10.0f;  // Pig
        case 91: return 4.0f;   // Sheep
        case 92: return 10.0f;  // Cow
        case 93: return 4.0f;   // Chicken
        case 94: return 10.0f;  // Squid
        case 95: return 8.0f;   // Wolf
        case 96: return 6.0f;   // Mooshroom
        case 97: return 4.0f;   // Snow Golem — Java: EntitySnowman.applyEntityAttributes() maxHealth=4
        case 98: return 10.0f;  // Ocelot — Java: EntityOcelot.applyEntityAttributes() maxHealth=10
        case 99: return 100.0f; // Iron Golem
        case 100: return 26.0f; // Horse (base, varies)
        case 120: return 10.0f; // Villager
        default: return 20.0f;
    }
}

// Java: SharedMonsterAttributes.movementSpeed base values per entity type
// These are blocks per tick at walking speed (Java: 0.1 ticks = 2 b/s base rate)
static float getMobMovementSpeed(uint8_t mobType) {
    switch (mobType) {
        case 50: return 0.25f;  // Creeper
        case 51: return 0.25f;  // Skeleton
        case 52: return 0.30f;  // Spider
        case 54: return 0.23f;  // Zombie
        case 55: return 0.20f;  // Slime
        case 56: return 0.00f;  // Ghast (flies, ranged only)
        case 57: return 0.23f;  // Zombie Pigman
        case 58: return 0.30f;  // Enderman
        case 59: return 0.30f;  // Cave Spider
        case 60: return 0.25f;  // Silverfish
        case 61: return 0.23f;  // Blaze
        case 62: return 0.20f;  // Magma Cube
        case 66: return 0.25f;  // Witch
        case 90: return 0.125f; // Pig (wander speed)
        case 91: return 0.115f; // Sheep (wander speed)
        case 92: return 0.10f;  // Cow (wander speed)
        case 93: return 0.125f; // Chicken (wander speed)
        case 95: return 0.30f;  // Wolf — Java: EntityWolf.movementSpeed
        case 97: return 0.20f;  // Snow Golem — Java: EntitySnowman.applyEntityAttributes() movementSpeed=0.2
        case 98: return 0.30f;  // Ocelot — Java: EntityOcelot.applyEntityAttributes() movementSpeed=0.3
        case 99: return 0.20f;  // Iron Golem (wander speed)
        case 100: return 0.225f; // Horse — Java: EntityHorse.applyEntityAttributes() movementSpeed=0.225
        case 120: return 0.50f;  // Villager — Java: EntityVillager.applyEntityAttributes() movementSpeed=0.5
        default: return 0.00f;  // Non-moving mob
    }
}

static bool isMobHostile(uint8_t mobType) {
    // Passive mobs: pig=90, sheep=91, cow=92, chicken=93
    if (mobType >= 90 && mobType <= 100) return false;
    return mobType >= 50 && mobType <= 66 && mobType != 56; // Ghast is ranged-only
}

int32_t MinecraftServer::summonMob(uint8_t mobType, double x, double y, double z) {
    int32_t eid = nextMobEntityId_.fetch_add(1, std::memory_order_relaxed);
    int64_t currentTick = tickCount_.load(std::memory_order_relaxed);
    SpawnedMob mob;
    mob.entityId = eid;
    mob.mobType = mobType;
    mob.posX = x; mob.posY = y; mob.posZ = z;
    mob.yaw = 0.0f; mob.pitch = 0.0f;
    mob.health = getMobMaxHealth(mobType);
    mob.spawnTick = currentTick;
    mob.isDead = false;
    mob.isPassive = (mobType >= 90 && mobType <= 100) || mobType == 120;
    mob.lastSentPosX = static_cast<int32_t>(std::floor(x * 32.0));
    mob.lastSentPosY = static_cast<int32_t>(std::floor(y * 32.0));
    mob.lastSentPosZ = static_cast<int32_t>(std::floor(z * 32.0));
    {
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (ph) ph->sendSpawnMob(*conn, eid, mobType, x, y, z, 0.0f, 0.0f, 0.0f);
        }

        // Wolf-specific DataWatcher metadata — Java: EntityWolf.entityInit()
        // DW 16: byte (bit 0=sitting, bit 1=angry, bit 2=tamed)
        // DW 17: String (owner name, empty for wild)
        // DW 18: float (wolf health)
        // DW 19: byte (begging, 0)
        // DW 20: byte (collar color, default 14=orange)
        if (mobType == 95) {
            auto dw16Pkt = PacketBuilder::entityMetadataByte(eid, 16, 0);
            auto dw18Pkt = PacketBuilder::entityMetadataFloat(eid, 18, mob.health);
            auto dw19Pkt = PacketBuilder::entityMetadataByte(eid, 19, 0);
            auto dw20Pkt = PacketBuilder::entityMetadataByte(eid, 20, static_cast<uint8_t>(mob.collarColor & 0x0F));
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                conn->sendPacket(dw16Pkt);
                conn->sendPacket(dw18Pkt);
                conn->sendPacket(dw19Pkt);
                conn->sendPacket(dw20Pkt);
            }
        }

        // Ocelot-specific DataWatcher metadata — Java: EntityOcelot.entityInit()
        // DW 18: byte (catSkinType: 0=wild, 1=tuxedo, 2=tabby, 3=siamese)
        // Shares DW 16 (sitting/tamed) and DW 17 (owner) with EntityTameable parent
        if (mobType == 98) {
            auto dw16Pkt = PacketBuilder::entityMetadataByte(eid, 16, 0);
            auto dw18Pkt = PacketBuilder::entityMetadataByte(eid, 18, static_cast<uint8_t>(mob.catSkinType));
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                conn->sendPacket(dw16Pkt);
                conn->sendPacket(dw18Pkt);
            }
        }

        // Horse-specific DataWatcher metadata — Java: EntityHorse.entityInit()
        // DW 16: int (bit flags: 2=tame, 4=saddle, 8=chest, 16=bred, 32=eating, 64=rearing, 128=mouth)
        // DW 19: byte (horse type: 0=horse, 1=donkey, 2=mule, 3=zombie, 4=skeleton)
        // DW 20: int (variant: low byte=color 0-6, high byte=marking 0-4)
        // DW 21: string (owner UUID)
        // DW 22: int (armor index: 0=none, 1=iron, 2=gold, 3=diamond)
        if (mobType == 100) {
            int32_t dw16val = 0;
            if (mob.isTamed) dw16val |= 2;
            if (mob.isHorseSaddled) dw16val |= 4;
            if (mob.isHorseChested) dw16val |= 8;
            auto dw16Pkt = PacketBuilder::entityMetadataInt(eid, 16, dw16val);
            auto dw19Pkt = PacketBuilder::entityMetadataByte(eid, 19, static_cast<uint8_t>(mob.horseType));
            auto dw20Pkt = PacketBuilder::entityMetadataInt(eid, 20, mob.horseVariant);
            auto dw21Pkt = PacketBuilder::entityMetadataString(eid, 21, mob.ownerUuid);
            auto dw22Pkt = PacketBuilder::entityMetadataInt(eid, 22, mob.horseArmorIndex);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                conn->sendPacket(dw16Pkt);
                conn->sendPacket(dw19Pkt);
                conn->sendPacket(dw20Pkt);
                conn->sendPacket(dw21Pkt);
                conn->sendPacket(dw22Pkt);
            }
        }

        // Villager-specific DataWatcher metadata — Java: EntityVillager.entityInit()
        // DW 16: int (profession: 0=farmer, 1=librarian, 2=priest, 3=blacksmith, 4=butcher)
        if (mobType == 120) {
            auto dw16Pkt = PacketBuilder::entityMetadataInt(eid, 16, mob.villagerProfession);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                conn->sendPacket(dw16Pkt);
            }
        }

        // Wither-specific DataWatcher metadata — Java: EntityWither.entityInit()
        // DW 17,18,19: int (watched target entity IDs for center+side heads)
        // DW 20: int (invulnerability time, 220 on spawn)
        if (mobType == 64) {
            mob.witherInvulTime = 220;  // Java: setInvulTime(220) on spawn
            auto dw17 = PacketBuilder::entityMetadataInt(eid, 17, 0); // center head target
            auto dw18 = PacketBuilder::entityMetadataInt(eid, 18, 0); // left head target
            auto dw19 = PacketBuilder::entityMetadataInt(eid, 19, 0); // right head target
            auto dw20 = PacketBuilder::entityMetadataInt(eid, 20, 220); // invul time
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                conn->sendPacket(dw17);
                conn->sendPacket(dw18);
                conn->sendPacket(dw19);
                conn->sendPacket(dw20);
            }
            // Java: worldObj.playBroadcastSound(1015, ...) — wither spawn sound
            broadcastEffect(1015, static_cast<int32_t>(x),
                static_cast<int32_t>(y), static_cast<int32_t>(z), 0);
        }
    }
    {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        mobEntities_.push_back(std::move(mob));
    }
    return eid;
}

// ═══════════════════════════════════════════════════════════════════════════
// Villager trade generation — Java: EntityVillager.addDefaultEquipmentAndRecipies()
// Generates profession-specific trades using the vanilla 1.7.10 trade tables.
// villagersSellingList = items villagers BUY from the player (item → emerald)
// blacksmithSellingList = items villagers SELL to the player (emerald → item)
// ═══════════════════════════════════════════════════════════════════════════
void MinecraftServer::generateVillagerTrades(int profession,
                                              std::vector<MerchantRecipe>& out) {
    out.clear();
    static thread_local std::mt19937 rng(std::random_device{}());

    // Helper: add a "villager buys" trade (item → emerald)
    // Java: func_146091_a — random quantity from villagersSellingList range
    auto addBuyTrade = [&](int16_t itemId, float probability, int minQty, int maxQty) {
        if (std::uniform_real_distribution<float>(0.0f, 1.0f)(rng) >= probability) return;
        int qty = minQty;
        if (maxQty > minQty)
            qty = minQty + static_cast<int>(rng() % static_cast<unsigned>(maxQty - minQty));
        MerchantRecipe r;
        r.buyItemId1 = itemId;
        r.buyCount1 = static_cast<int8_t>(std::min(qty, 64));
        r.buyDamage1 = 0;
        r.sellItemId = 388; // emerald
        r.sellCount = 1;
        r.sellDamage = 0;
        out.push_back(r);
    };

    // Helper: add a "villager sells" trade (emerald → item)
    // Java: func_146089_b — quantity from blacksmithSellingList (negative = multi-item per emerald)
    auto addSellTrade = [&](int16_t itemId, float probability, int minPrice, int maxPrice) {
        if (std::uniform_real_distribution<float>(0.0f, 1.0f)(rng) >= probability) return;
        int price = minPrice;
        if (maxPrice > minPrice)
            price = minPrice + static_cast<int>(rng() % static_cast<unsigned>(maxPrice - minPrice));
        MerchantRecipe r;
        if (price < 0) {
            // Negative price = villager sells multiple items for 1 emerald
            r.buyItemId1 = 388; // emerald
            r.buyCount1 = 1;
            r.buyDamage1 = 0;
            r.sellItemId = itemId;
            r.sellCount = static_cast<int8_t>(std::min(-price, 64));
            r.sellDamage = 0;
        } else {
            // Positive price = costs N emeralds for 1 item
            r.buyItemId1 = 388; // emerald
            r.buyCount1 = static_cast<int8_t>(std::min(price, 64));
            r.buyDamage1 = 0;
            r.sellItemId = itemId;
            r.sellCount = 1;
            r.sellDamage = 0;
        }
        out.push_back(r);
    };

    switch (profession) {
        case 0: { // Farmer
            addBuyTrade(296, 0.9f, 18, 22);   // wheat
            addBuyTrade(35,  0.5f, 14, 22);   // wool (block)
            addBuyTrade(365, 0.5f, 14, 18);   // chicken (raw)
            addBuyTrade(350, 0.4f, 9, 13);    // cooked_fish
            addSellTrade(297, 0.9f, -4, -2);  // bread
            addSellTrade(360, 0.3f, -8, -4);  // melon
            addSellTrade(260, 0.3f, -8, -4);  // apple
            addSellTrade(357, 0.3f, -10, -7); // cookie
            addSellTrade(359, 0.3f, 3, 4);    // shears
            addSellTrade(259, 0.3f, 3, 4);    // flint_and_steel
            addSellTrade(366, 0.3f, -8, -6);  // cooked_chicken
            addSellTrade(262, 0.5f, -12, -8); // arrow
            // Gravel + emerald → flint (special dual-input trade)
            if (std::uniform_real_distribution<float>(0.0f, 1.0f)(rng) < 0.5f) {
                MerchantRecipe r;
                r.buyItemId1 = 13;  // gravel
                r.buyCount1 = 10;
                r.buyDamage1 = 0;
                r.buyItemId2 = 388; // emerald
                r.buyCount2 = 1;
                r.buyDamage2 = 0;
                r.sellItemId = 318; // flint
                r.sellCount = static_cast<int8_t>(4 + rng() % 2);
                r.sellDamage = 0;
                out.push_back(r);
            }
            break;
        }
        case 1: { // Librarian
            addBuyTrade(339, 0.8f, 24, 36);   // paper
            addBuyTrade(340, 0.8f, 11, 13);   // book
            addBuyTrade(387, 0.3f, 1, 1);     // written_book
            addSellTrade(47,  0.8f, 3, 4);    // bookshelf (block)
            addSellTrade(20,  0.2f, -5, -3);  // glass (block)
            addSellTrade(345, 0.2f, 10, 12);  // compass
            addSellTrade(347, 0.2f, 10, 12);  // clock
            // Enchanted book trade (rare) — simplified: sell an enchanted book for emeralds
            if (std::uniform_real_distribution<float>(0.0f, 1.0f)(rng) < 0.07f) {
                MerchantRecipe r;
                r.buyItemId1 = 340; // book
                r.buyCount1 = 1;
                r.buyDamage1 = 0;
                r.buyItemId2 = 388; // emerald
                r.buyCount2 = static_cast<int8_t>(5 + rng() % 19); // 5-23 emeralds
                r.buyDamage2 = 0;
                r.sellItemId = 403; // enchanted_book
                r.sellCount = 1;
                r.sellDamage = 0;
                out.push_back(r);
            }
            break;
        }
        case 2: { // Priest/Cleric
            addSellTrade(381, 0.3f, 7, 11);   // eye_of_ender
            addSellTrade(384, 0.2f, -4, -1);  // experience_bottle
            addSellTrade(331, 0.4f, -4, -1);  // redstone
            addSellTrade(89,  0.3f, -3, -1);  // glowstone (block)
            break;
        }
        case 3: { // Blacksmith
            addBuyTrade(263, 0.7f, 16, 24);   // coal
            addBuyTrade(265, 0.5f, 8, 10);    // iron_ingot
            addBuyTrade(266, 0.5f, 8, 10);    // gold_ingot
            addBuyTrade(264, 0.5f, 4, 6);     // diamond
            addSellTrade(267, 0.5f, 7, 11);   // iron_sword
            addSellTrade(276, 0.5f, 12, 14);  // diamond_sword
            addSellTrade(258, 0.3f, 6, 8);    // iron_axe
            addSellTrade(279, 0.3f, 9, 12);   // diamond_axe
            addSellTrade(257, 0.5f, 7, 9);    // iron_pickaxe
            addSellTrade(278, 0.5f, 10, 12);  // diamond_pickaxe
            addSellTrade(256, 0.2f, 4, 6);    // iron_shovel
            addSellTrade(277, 0.2f, 7, 8);    // diamond_shovel
            addSellTrade(292, 0.2f, 4, 6);    // iron_hoe
            addSellTrade(293, 0.2f, 7, 8);    // diamond_hoe
            addSellTrade(309, 0.2f, 4, 6);    // iron_boots
            addSellTrade(313, 0.2f, 7, 8);    // diamond_boots
            addSellTrade(306, 0.2f, 4, 6);    // iron_helmet
            addSellTrade(310, 0.2f, 7, 8);    // diamond_helmet
            addSellTrade(307, 0.2f, 10, 14);  // iron_chestplate
            addSellTrade(311, 0.2f, 16, 19);  // diamond_chestplate
            addSellTrade(308, 0.2f, 8, 10);   // iron_leggings
            addSellTrade(312, 0.2f, 11, 14);  // diamond_leggings
            addSellTrade(305, 0.1f, 5, 7);    // chainmail_boots
            addSellTrade(302, 0.1f, 5, 7);    // chainmail_helmet
            addSellTrade(303, 0.1f, 11, 15);  // chainmail_chestplate
            addSellTrade(304, 0.1f, 9, 11);   // chainmail_leggings
            break;
        }
        case 4: { // Butcher
            addBuyTrade(263, 0.7f, 16, 24);   // coal
            addBuyTrade(319, 0.5f, 14, 18);   // porkchop
            addBuyTrade(363, 0.5f, 14, 18);   // beef
            addSellTrade(329, 0.1f, 6, 8);    // saddle
            addSellTrade(299, 0.3f, 4, 5);    // leather_chestplate
            addSellTrade(301, 0.3f, 2, 4);    // leather_boots
            addSellTrade(298, 0.3f, 2, 4);    // leather_helmet
            addSellTrade(300, 0.3f, 2, 4);    // leather_leggings
            addSellTrade(320, 0.3f, -7, -5);  // cooked_porkchop
            addSellTrade(364, 0.3f, -7, -5);  // cooked_beef
            break;
        }
        default:
            // Unknown profession — fallback: gold_ingot → emerald
            addBuyTrade(266, 1.0f, 8, 10);
            break;
    }

    // Java: if list empty, add gold_ingot → emerald as fallback
    if (out.empty()) {
        addBuyTrade(266, 1.0f, 8, 10);
    }
}

bool MinecraftServer::isRaining() const {
    for (auto& w : worlds_) {
        if (w->isRaining()) return true;
    }
    return false;
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
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    mob.health = getMobMaxHealth(mobType);
    mob.spawnTick = currentTick;
    mob.isDead = false;
    mob.lastSentPosX = static_cast<int32_t>(std::floor(spawnX * 32.0));
    mob.lastSentPosY = static_cast<int32_t>(std::floor(spawnY * 32.0));
    mob.lastSentPosZ = static_cast<int32_t>(std::floor(spawnZ * 32.0));
    // Broadcast S0F SpawnMob to all players
    {
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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

// ─── Passive mob spawning ─────────────────────────────────────────
// Java: SpawnerAnimals.findChunksForSpawning() for EnumCreatureType.creature
// Passive mobs (cow, pig, sheep, chicken) spawn on grass blocks, don't despawn
void MinecraftServer::spawnPassiveMobs() {
    int playerCount = getOnlinePlayerCount();
    if (playerCount == 0) return;
    if (worlds_.empty()) return;

    // Check passive mob cap (separate from hostile)
    {
        std::lock_guard<std::mutex> lock(mobEntitiesMutex_);
        int passiveCount = 0;
        for (auto& mob : mobEntities_) {
            if (!mob.isDead && mob.isPassive) passiveCount++;
        }
        if (passiveCount >= MAX_PASSIVE_MOBS) return;
    }

    // Pick a random player to spawn near
    static thread_local std::mt19937 rng(std::random_device{}());
    PlayHandler* targetPlayer = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
    std::uniform_int_distribution<int> offsetDist(-24, 24);
    double spawnX = targetPlayer->getPlayerX() + offsetDist(rng);
    double spawnZ = targetPlayer->getPlayerZ() + offsetDist(rng);

    // Min 8 blocks away
    double dx = spawnX - targetPlayer->getPlayerX();
    double dz = spawnZ - targetPlayer->getPlayerZ();
    if (dx * dx + dz * dz < 64.0) return;

    WorldServer* world = worlds_[0].get();
    int bx = static_cast<int>(std::floor(spawnX));
    int bz = static_cast<int>(std::floor(spawnZ));

    // Find surface Y
    int surfaceY = 64;
    for (int y = 255; y > 0; --y) {
        Block* block = world->getBlock(bx, y, bz);
        if (block != nullptr) {
            surfaceY = y + 1;
            break;
        }
    }
    if (surfaceY <= 1 || surfaceY > 250) return;

    // Java: EntityAnimal.getCanSpawnHere() — requires grass block below
    // Check that spawn block is grass (block ID 2)
    int32_t groundBlockId = getBlockIdInWorld(bx, surfaceY - 1, bz);
    if (groundBlockId != 2) return; // Must be grass block

    // Check air above (no normal cube at spawn location)
    int32_t blockAtSpawn = getBlockIdInWorld(bx, surfaceY, bz);
    int32_t blockAboveSpawn = getBlockIdInWorld(bx, surfaceY + 1, bz);
    if (blockAtSpawn != 0 || blockAboveSpawn != 0) return;

    double spawnY = static_cast<double>(surfaceY);

    // Pick passive mob type — equal chances
    // 90=Pig, 91=Sheep, 92=Cow, 93=Chicken, 95=Wolf, 98=Ocelot
    static const uint8_t passiveMobs[] = {90, 91, 92, 93, 95, 98, 100};
    uint8_t mobType = passiveMobs[rng() % 7];

    std::uniform_real_distribution<float> yawDist(0.0f, 360.0f);
    float yaw = yawDist(rng);

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
    mob.health = getMobMaxHealth(mobType);
    mob.spawnTick = currentTick;
    mob.isDead = false;
    mob.isPassive = true;
    mob.wanderCooldown = 40 + (rng() % 80);  // Initial wander delay
    mob.wanderYaw = yaw;
    mob.lastSentPosX = static_cast<int32_t>(std::floor(spawnX * 32.0));
    mob.lastSentPosY = static_cast<int32_t>(std::floor(spawnY * 32.0));
    mob.lastSentPosZ = static_cast<int32_t>(std::floor(spawnZ * 32.0));

    // Java: EntitySheep.getRandomFleeceColor() — random sheep color
    if (mobType == 91) {
        int r = rng() % 100;
        if (r < 5)       mob.fleeceColor = 15; // Black (5%)
        else if (r < 10) mob.fleeceColor = 7;  // Light gray (5%)
        else if (r < 15) mob.fleeceColor = 8;  // Gray (5%)
        else if (r < 18) mob.fleeceColor = 12; // Brown (3%)
        else if (rng() % 500 == 0) mob.fleeceColor = 6; // Pink (rare)
        else             mob.fleeceColor = 0;  // White (most common)
    }

    // Java: EntityHorse.onSpawnWithEgg() — random horse type and variant
    if (mobType == 100) {
        // 10% chance donkey (type 1), else normal horse (type 0)
        if (rng() % 10 == 0) {
            mob.horseType = 1; // Donkey
        } else {
            mob.horseType = 0; // Normal horse
            // Random color (0-6) + random marking (0-4) in high byte
            int32_t color = rng() % 7;
            int32_t marking = rng() % 5;
            mob.horseVariant = color | (marking << 8);
        }
        // Random health for normal horses: 15 + rand(0..7) + rand(0..8)
        if (mob.horseType == 0) {
            mob.health = 15.0f + static_cast<float>(rng() % 8) + static_cast<float>(rng() % 9);
        } else {
            mob.health = getMobMaxHealth(100); // Donkeys get base health
        }
    }

    // Broadcast S0F SpawnMob to all players
    {
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (ph) {
                ph->sendSpawnMob(*conn, eid, mobType, spawnX, spawnY, spawnZ,
                                  yaw, 0.0f, yaw);
                // Sheep: send DataWatcher byte 16 (fleeceColor)
                if (mobType == 91) {
                    uint8_t dw16 = static_cast<uint8_t>(mob.fleeceColor & 0x0F);
                    auto metaPkt = PacketBuilder::entityMetadataByte(eid, 16, dw16);
                    conn->sendPacket(std::move(metaPkt));
                }
                // Horse: send DataWatcher int 16 (flags), byte 19 (type), int 20 (variant)
                if (mobType == 100) {
                    conn->sendPacket(PacketBuilder::entityMetadataInt(eid, 16, 0)); // no flags set on wild spawn
                    conn->sendPacket(PacketBuilder::entityMetadataByte(eid, 19, static_cast<uint8_t>(mob.horseType)));
                    conn->sendPacket(PacketBuilder::entityMetadataInt(eid, 20, mob.horseVariant));
                    conn->sendPacket(PacketBuilder::entityMetadataInt(eid, 22, 0)); // no armor
                }
            }
        }
    }

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

            // ─── Ender Dragon tick — Java: EntityDragon.onLivingUpdate() ───
            // Dragon has its own AI; never despawns, never uses generic hostile chase
            if (mob.mobType == 63) {
                tickDragon(mob, currentTick);
                continue;
            }

            // ─── Wither Boss tick — Java: EntityWither.onLivingUpdate() + updateAITasks() ───
            // Wither has its own AI; never despawns (entityAge=0)
            if (mob.mobType == 64) {
                tickWither(mob, currentTick);
                continue;
            }

            // ─── Snow Golem tick — Java: EntitySnowman.onLivingUpdate() ───
            // Snow trail, heat/water damage, ranged snowball attacks
            // Does NOT continue — falls through to passive wander AI for movement
            if (mob.mobType == 97) {
                tickSnowGolem(mob, currentTick);
                if (mob.isDead) {
                    deadIds.push_back(mob.entityId);
                    continue;
                }
            }

            // Check distance to nearest player
            double nearestDistSq = 1e9;
            {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph) continue;
                    double dx = ph->getPlayerX() - mob.posX;
                    double dz = ph->getPlayerZ() - mob.posZ;
                    double distSq = dx * dx + dz * dz;
                    if (distSq < nearestDistSq) nearestDistSq = distSq;
                }
            }

            // Java: EntityLiving.despawnEntity()
            // Passive mobs never despawn — Java: EntityAnimal.despawnEntity() is no-op
            if (!mob.isPassive) {
                // Hard despawn: immediately if >128 blocks (16384 sq) from all players
                if (nearestDistSq > 16384.0) {
                    mob.isDead = true;
                    deadIds.push_back(mob.entityId);
                    continue;
                }

                int64_t age = currentTick - mob.spawnTick;
                // Soft despawn: after 600 ticks if >32 blocks (1024 sq) from all players
                if (age >= 600 && nearestDistSq > 1024.0) {
                    mob.isDead = true;
                    deadIds.push_back(mob.entityId);
                }
            }

            // ─── Mob ambient sounds ──────────────────────────────────
            // Java: EntityLiving.onLivingUpdate() — plays getLivingSound()
            // Random ~1/200 chance per tick = average every 10 seconds
            if ((rand() % 200) == 0) {
                const char* sound = nullptr;
                switch (mob.mobType) {
                    case 50: sound = "mob.creeper.say"; break;
                    case 51: sound = "mob.skeleton.say"; break;
                    case 52: sound = "mob.spider.say"; break;
                    case 54: sound = "mob.zombie.say"; break;
                    case 55: sound = (rand() % 2) ? "mob.slime.big" : "mob.slime.small"; break;
                    case 56: sound = "mob.ghast.moan"; break;
                    case 57: sound = "mob.zombiepig.zpig"; break;
                    case 58: sound = "mob.endermen.idle"; break;
                    case 59: sound = "mob.spider.say"; break;
                    case 60: sound = "mob.silverfish.say"; break;
                    case 61: sound = "mob.blaze.breathe"; break;
                    case 62: sound = (rand() % 2) ? "mob.magmacube.big" : "mob.magmacube.small"; break;
                    case 66: sound = "mob.witch.idle"; break;
                    case 92: sound = "mob.cow.say"; break;
                    case 90: sound = "mob.pig.say"; break;
                    case 91: sound = "mob.sheep.say"; break;
                    case 93: sound = "mob.chicken.say"; break;
                    case 95: // Wolf — Java: EntityWolf.getLivingSound()
                        if (mob.isTamed) {
                            sound = (mob.health < 10.0f) ? "mob.wolf.whine" : "mob.wolf.panting";
                        } else {
                            sound = mob.isAngry ? "mob.wolf.growl" : "mob.wolf.bark";
                        }
                        break;
                    case 98: // Ocelot/Cat — Java: EntityOcelot.getLivingSound()
                        if (mob.isTamed) {
                            // Java: if isInLove → purr; 1/4 → purreow; else meow
                            if (mob.inLoveTicks > 0) {
                                sound = "mob.cat.purr";
                            } else if (rand() % 4 == 0) {
                                sound = "mob.cat.purreow";
                            } else {
                                sound = "mob.cat.meow";
                            }
                        }
                        // Wild ocelot: no ambient sound — Java: returns ""
                        break;
                    case 100: { // Horse — Java: EntityHorse.getLivingSound()
                        int ht = mob.horseType;
                        if (ht == 3) sound = "mob.horse.zombie.idle";
                        else if (ht == 4) sound = "mob.horse.skeleton.idle";
                        else if (ht == 1 || ht == 2) sound = "mob.horse.donkey.idle";
                        else sound = "mob.horse.idle";
                        break;
                    }
                    default: break;
                }
                if (sound) {
                    broadcastSound(sound, mob.posX, mob.posY, mob.posZ, 1.0f,
                        0.8f + ((float)(rand() % 40) / 100.0f)); // Slight pitch randomization
                }
            }

            // ─── Passive mob special behaviors ──────────────────────────
            if (mob.isPassive) {
                // Chicken egg laying — Java: EntityChicken.onLivingUpdate()
                // Lays egg every 6000-12000 ticks (1/6000 chance per tick)
                if (mob.mobType == 93 && (rand() % 6000) == 0) {
                    // Drop egg item (344) at chicken position
                    spawnItemDrop(mob.posX, mob.posY, mob.posZ, 344, 0, 1);
                    broadcastSound("mob.chicken.plop", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
                }
                // Sheep grass eating — Java: EntitySheep.eatGrassBonus()
                // 1/1000 chance per tick = eats grass block → dirt
                if (mob.mobType == 91 && (rand() % 1000) == 0) {
                    int bx = static_cast<int>(std::floor(mob.posX));
                    int by = static_cast<int>(std::floor(mob.posY)) - 1;
                    int bz = static_cast<int>(std::floor(mob.posZ));
                    int32_t blockBelow = getBlockIdInWorld(bx, by, bz);
                    if (blockBelow == 2) { // Grass block
                        setBlockInWorld(bx, by, bz, 3, 0); // Convert to dirt
                        broadcastSound("mob.sheep.shear", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
                        // Java: EntitySheep.eatGrassBonus() → setSheared(false)
                        // Wool regrows when sheep eats grass
                        if (mob.isSheared) {
                            mob.isSheared = false;
                            uint8_t dw16 = static_cast<uint8_t>(mob.fleeceColor & 0x0F);
                            auto metaPkt = PacketBuilder::entityMetadataByte(mob.entityId, 16, dw16);
                            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                            for (auto& c : connections_) {
                                if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                                c->sendPacket(metaPkt);
                            }
                        }
                    }
                }

                // ─── Breeding timer tick — Java: EntityAnimal.onLivingUpdate() ──
                if (mob.breedCooldown > 0) --mob.breedCooldown;

                if (mob.inLoveTicks > 0) {
                    --mob.inLoveTicks;

                    // Heart particles every 10 ticks — Java: inLove % 10 == 0
                    if (mob.inLoveTicks > 0 && mob.inLoveTicks % 10 == 0) {
                        broadcastParticle("heart",
                            static_cast<float>(mob.posX + ((rand() % 100 - 50) / 100.0)),
                            static_cast<float>(mob.posY + 0.5 + ((rand() % 50) / 100.0)),
                            static_cast<float>(mob.posZ + ((rand() % 100 - 50) / 100.0)),
                            0.0f, 0.0f, 0.0f, 0.0f, 1);
                    }

                    // ─── Mate search — Java: EntityAIMate.getNearbyMate() ──
                    // Find nearest same-type, in-love mob within 8 blocks
                    if (mob.mateEntityId == -1) {
                        double nearestDist = 64.0;  // 8 blocks squared
                        int32_t nearestId = -1;
                        for (auto& other : mobEntities_) {
                            if (other.entityId == mob.entityId) continue;
                            if (other.isDead || other.mobType != mob.mobType) continue;
                            if (other.inLoveTicks <= 0 || other.breedCooldown > 0) continue;
                            double dx = other.posX - mob.posX;
                            double dz = other.posZ - mob.posZ;
                            double distSq = dx * dx + dz * dz;
                            if (distSq < nearestDist) {
                                nearestDist = distSq;
                                nearestId = other.entityId;
                            }
                        }
                        if (nearestId != -1) {
                            mob.mateEntityId = nearestId;
                            mob.breedingCounter = 0;
                        }
                    }

                    // ─── Proximity mating — Java: EntityAIMate.updateTask() ──
                    if (mob.mateEntityId != -1) {
                        // Check mate still valid
                        bool mateValid = false;
                        SpawnedMob* mate = nullptr;
                        for (auto& other : mobEntities_) {
                            if (other.entityId == mob.mateEntityId && !other.isDead &&
                                other.inLoveTicks > 0) {
                                mate = &other;
                                mateValid = true;
                                break;
                            }
                        }

                        if (!mateValid) {
                            mob.mateEntityId = -1;
                            mob.breedingCounter = 0;
                        } else {
                            // Check distance — Java: getDistanceSqToEntity < 9.0
                            double dx = mate->posX - mob.posX;
                            double dz = mate->posZ - mob.posZ;
                            double distSq = dx * dx + dz * dz;

                            if (distSq < 9.0) {
                                ++mob.breedingCounter;
                            }

                            // Java: spawnBabyDelay >= 60 && distance < 9
                            if (mob.breedingCounter >= 60 && distSq < 9.0) {
                                // ─── Spawn baby — Java: EntityAIMate.spawnBaby() ──
                                double babyX = (mob.posX + mate->posX) / 2.0;
                                double babyY = mob.posY;
                                double babyZ = (mob.posZ + mate->posZ) / 2.0;

                                summonMob(mob.mobType, babyX, babyY, babyZ);

                                // Both parents enter cooldown — Java: setGrowingAge(6000)
                                mob.inLoveTicks = 0;
                                mob.breedCooldown = 6000;
                                mob.breedingCounter = 0;
                                mob.mateEntityId = -1;

                                mate->inLoveTicks = 0;
                                mate->breedCooldown = 6000;
                                mate->breedingCounter = 0;
                                mate->mateEntityId = -1;

                                // 7 heart particles — Java: for (int i = 0; i < 7; ++i)
                                for (int hp = 0; hp < 7; ++hp) {
                                    broadcastParticle("heart",
                                        static_cast<float>(babyX + ((rand() % 100 - 50) / 100.0)),
                                        static_cast<float>(babyY + 0.5 + ((rand() % 80) / 100.0)),
                                        static_cast<float>(babyZ + ((rand() % 100 - 50) / 100.0)),
                                        0.0f, 0.0f, 0.0f, 0.0f, 1);
                                }

                                // XP orbs — Java: 1-7 XP (give to nearest player)
                                int32_t xpAmount = 1 + (rand() % 7);
                                {
                                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                                    PlayHandler* nearest = nullptr;
                                    double nearestDistSq = 64.0;  // 8 blocks
                                    for (auto& c : connections_) {
                                        if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                                        auto handler = c->getHandler();
                                        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                                        if (!ph) continue;
                                        double pdx = ph->getPlayerX() - babyX;
                                        double pdz = ph->getPlayerZ() - babyZ;
                                        double pd = pdx * pdx + pdz * pdz;
                                        if (pd < nearestDistSq) {
                                            nearestDistSq = pd;
                                            nearest = ph;
                                        }
                                    }
                                    // Java: spawns XP orbs at baby position
                                    if (xpAmount > 0) spawnXPOrbs(babyX, babyY + 0.5, babyZ, xpAmount);
                                }
                            }
                        }
                    }
                } else {
                    // Not in love — reset breeding state
                    mob.breedingCounter = 0;
                    mob.mateEntityId = -1;
                }
            }
        }

        // ─── Mob movement toward players ────────────────────────────
        // Java: EntityCreature.updateEntityActionState() → path to target
        // Simplified: direct movement toward nearest player within 16 blocks
        for (auto& mob : mobEntities_) {
            if (mob.isDead) continue;
            float speed = getMobMovementSpeed(mob.mobType);
            if (speed <= 0.0f) continue; // Non-moving mob

            // ─── Horse riding AI — Java: EntityHorse.moveEntityWithHeading ───
            // If horse (100) is saddled and ridden, use rider-controlled movement
            if (mob.mobType == 100 && mob.isHorseSaddled && mob.riderEntityId >= 0) {
                // Find rider's PlayHandler
                PlayHandler* rider = nullptr;
                Connection* riderConn = nullptr;
                {
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    for (auto& c : connections_) {
                        if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                        auto handler = c->getHandler();
                        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                        if (ph && ph->getEntityId() == mob.riderEntityId) {
                            rider = ph;
                            riderConn = c.get();
                            break;
                        }
                    }
                }

                if (rider) {
                    // ─── Java: EntityHorse.moveEntityWithHeading (line 918-974) ─────
                    // Yaw: horse instantly follows rider yaw (not gradual like pig)
                    mob.yaw = rider->getPlayerYaw();
                    mob.pitch = rider->getPlayerPitch() * 0.5f;

                    // Java: strafe = rider.moveStrafing * 0.5f, forward = rider.moveForward
                    float strafeInput = rider->getMoveStrafing() * 0.5f;
                    float forwardInput = rider->getMoveForward();
                    // Java: if (f2 <= 0.0f) f2 *= 0.25f — backward is quarter speed
                    if (forwardInput <= 0.0f) {
                        forwardInput *= 0.25f;
                        mob.gallopTime = 0;
                    }

                    // ─── Jump physics — Java: EntityHorse.moveEntityWithHeading lines 939-953 ───
                    if (mob.horseJumpPower > 0.0f && !mob.horseIsJumping && mob.horseOnGround) {
                        // Java: this.motionY = this.getHorseJumpStrength() * (double)this.jumpPower
                        mob.horseMotionY = mob.horseJumpStrength * static_cast<double>(mob.horseJumpPower);
                        mob.horseIsJumping = true;
                        mob.horseOnGround = false;

                        // Java: horizontal boost when moving forward
                        if (forwardInput > 0.0f) {
                            float yawRad = mob.yaw / 180.0f * static_cast<float>(M_PI);
                            // Java: this.motionX += (double)(-0.4f * f3 * this.jumpPower)
                            mob.posX += static_cast<double>(-0.4f * std::sin(yawRad) * mob.horseJumpPower);
                            mob.posZ += static_cast<double>(0.4f * std::cos(yawRad) * mob.horseJumpPower);
                        }

                        broadcastSound("mob.horse.jump", mob.posX, mob.posY, mob.posZ, 0.4f, 1.0f);
                        mob.horseJumpPower = 0.0f;
                    }
                    // Java: if (this.onGround) { this.jumpPower = 0; this.setHorseJumping(false); }
                    if (mob.horseOnGround) {
                        mob.horseJumpPower = 0.0f;
                        mob.horseIsJumping = false;
                    }

                    // ─── Movement — Java: super.moveEntityWithHeading(strafe, forward) ───
                    // Java: this.setAIMoveSpeed(getEntityAttribute(movementSpeed).getAttributeValue())
                    constexpr float HORSE_MOVE_SPEED = 0.225f; // Java: EntityHorse.applyEntityAttributes line 551
                    float yawRad = mob.yaw / 180.0f * static_cast<float>(M_PI);
                    float sinYaw = std::sin(yawRad);
                    float cosYaw = std::cos(yawRad);

                    // Forward + strafe combined movement direction
                    double moveX = (-sinYaw * forwardInput - cosYaw * strafeInput) * HORSE_MOVE_SPEED;
                    double moveZ = (cosYaw * forwardInput - sinYaw * strafeInput) * HORSE_MOVE_SPEED;

                    double newX = mob.posX + moveX;
                    double newZ = mob.posZ + moveZ;

                    // ─── Gravity — Java: EntityLivingBase.moveEntityWithHeading ───
                    // motionY -= 0.08; motionY *= 0.98
                    if (!mob.horseOnGround) {
                        mob.horseMotionY -= 0.08;
                        mob.horseMotionY *= 0.98;
                    }

                    double newY = mob.posY + mob.horseMotionY;

                    // ─── Ground/collision check (with 1-block step-up) ───
                    if (!worlds_.empty()) {
                        auto* wld = worlds_[0].get();
                        int bx = static_cast<int>(std::floor(newX));
                        int bz = static_cast<int>(std::floor(newZ));
                        int startY = static_cast<int>(std::floor(newY));

                        Block* feetBlock = wld->getBlock(bx, startY, bz);
                        if (feetBlock != nullptr) {
                            // Solid at feet level — try step up (step height = 1.0)
                            Block* stepBlock = wld->getBlock(bx, startY + 1, bz);
                            Block* headBlock = wld->getBlock(bx, startY + 2, bz);
                            Block* aboveHead = wld->getBlock(bx, startY + 3, bz);
                            if (stepBlock == nullptr && headBlock == nullptr && aboveHead == nullptr) {
                                // Can step up 1 block
                                mob.posX = newX;
                                mob.posZ = newZ;
                                mob.posY = static_cast<double>(startY + 1);
                                mob.horseMotionY = 0.0;
                                mob.horseOnGround = true;
                            }
                            // else: blocked, stay at current X/Z
                        } else {
                            // Air at feet — apply gravity, find ground
                            int groundY = startY;
                            bool foundGround = false;
                            for (int y = startY - 1; y > 0; --y) {
                                Block* b = wld->getBlock(bx, y, bz);
                                if (b != nullptr) {
                                    groundY = y + 1;
                                    foundGround = true;
                                    break;
                                }
                                if (y == 1) groundY = 1;
                            }

                            mob.posX = newX;
                            mob.posZ = newZ;
                            if (newY <= static_cast<double>(groundY)) {
                                // Landed on ground
                                mob.posY = static_cast<double>(groundY);
                                mob.horseMotionY = 0.0;
                                mob.horseOnGround = true;
                            } else {
                                // Still in air
                                mob.posY = newY;
                                mob.horseOnGround = false;
                            }
                        }
                    }

                    // ─── Gallop sounds — Java: EntityHorse.playStepSound ───
                    bool isMoving = (std::abs(moveX) > 0.001 || std::abs(moveZ) > 0.001);
                    if (isMoving && mob.horseOnGround) {
                        ++mob.gallopTime;
                        if (mob.gallopTime > 5 && mob.gallopTime % 3 == 0) {
                            broadcastSound("mob.horse.gallop", mob.posX, mob.posY, mob.posZ, 0.15f, 1.0f);
                            if (mob.horseType == 0 && (rand() % 10) == 0) {
                                broadcastSound("mob.horse.breathe", mob.posX, mob.posY, mob.posZ, 0.6f, 1.0f);
                            }
                        } else if (mob.gallopTime <= 5) {
                            broadcastSound("mob.horse.wood", mob.posX, mob.posY, mob.posZ, 0.15f, 1.0f);
                        }
                    } else {
                        mob.gallopTime = 0;
                    }

                    // ─── Update rider position — Java: Entity.updateRiderPosition() ───
                    // Java: EntityHorse.getMountedYOffset() returns this.height * 0.75
                    // Horse height = 1.6, so offset ≈ 1.2; rider getYOffset() = -0.35
                    // Net: posY + 1.2 - 0.35 ≈ posY + 0.85
                    // In practice the vanilla client uses ~1.36 offset for smooth appearance
                    rider->setPlayerPosition(mob.posX, mob.posY + 1.36, mob.posZ);

                } else {
                    // Rider disconnected — dismount
                    mob.riderEntityId = -1;
                    mob.horseJumpPower = 0.0f;
                    mob.horseMotionY = 0.0;
                    mob.horseIsJumping = false;
                    mob.gallopTime = 0;
                }

                // Broadcast position update for ridden horse
                {
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    for (auto& c : connections_) {
                        if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                        auto handler = c->getHandler();
                        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                        if (ph) {
                            ph->sendEntityTeleport(*c, mob.entityId,
                                mob.posX, mob.posY, mob.posZ, mob.yaw, mob.pitch);
                        }
                    }
                }
                mob.lastSentPosX = static_cast<int32_t>(std::floor(mob.posX * 32.0));
                mob.lastSentPosY = static_cast<int32_t>(std::floor(mob.posY * 32.0));
                mob.lastSentPosZ = static_cast<int32_t>(std::floor(mob.posZ * 32.0));
                continue; // Skip normal wander AI for ridden horse
            }

            // ─── Pig riding AI — Java: EntityAIControlledByPlayer ────────
            // If pig is ridden, use controlled movement instead of wander AI
            if (mob.mobType == 90 && mob.isSaddled && mob.riderEntityId >= 0) {
                // Find rider's PlayHandler to get yaw + check held item
                PlayHandler* rider = nullptr;
                Connection* riderConn = nullptr;
                {
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    for (auto& c : connections_) {
                        if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                        auto handler = c->getHandler();
                        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                        if (ph && ph->getEntityId() == mob.riderEntityId) {
                            rider = ph;
                            riderConn = c.get();
                            break;
                        }
                    }
                }

                if (rider) {
                    // Java: canBeSteered() → rider holding carrot_on_a_stick (398)
                    auto riderHeld = rider->getHeldItem();
                    int32_t riderHeldId = riderHeld ? riderHeld->getItemId() : 0;
                    bool canSteer = (riderHeldId == 398) || mob.pigSpeedBoosted;

                    if (canSteer) {
                        // ─── EntityAIControlledByPlayer.updateTask() ─────────
                        // Java: float f = wrapAngleTo180(player.rotationYaw - thisEntity.rotationYaw) * 0.5f
                        float riderYaw = rider->getPlayerYaw();
                        float yawDelta = riderYaw - mob.yaw;
                        // Wrap to -180..180
                        while (yawDelta > 180.0f) yawDelta -= 360.0f;
                        while (yawDelta < -180.0f) yawDelta += 360.0f;
                        yawDelta *= 0.5f;
                        // Clamp ±5°
                        if (yawDelta > 5.0f) yawDelta = 5.0f;
                        if (yawDelta < -5.0f) yawDelta = -5.0f;
                        mob.yaw += yawDelta;
                        // Wrap yaw to -180..180
                        while (mob.yaw > 180.0f) mob.yaw -= 360.0f;
                        while (mob.yaw < -180.0f) mob.yaw += 360.0f;

                        // Java: currentSpeed ramp: if (currentSpeed < maxSpeed) currentSpeed += (maxSpeed - currentSpeed) * 0.01f
                        constexpr float PIG_MAX_SPEED = 0.3f;  // Java: EntityAIControlledByPlayer(pig, 0.3f)
                        if (mob.pigCurrentSpeed < PIG_MAX_SPEED) {
                            mob.pigCurrentSpeed += (PIG_MAX_SPEED - mob.pigCurrentSpeed) * 0.01f;
                        }
                        if (mob.pigCurrentSpeed > PIG_MAX_SPEED) {
                            mob.pigCurrentSpeed = PIG_MAX_SPEED;
                        }

                        // Java: speed boost (from right-clicking carrot-on-stick while riding)
                        float adjustedSpeed = mob.pigCurrentSpeed;
                        if (mob.pigSpeedBoosted) {
                            ++mob.pigSpeedBoostTime;
                            if (mob.pigSpeedBoostTime > mob.pigMaxSpeedBoostTime) {
                                mob.pigSpeedBoosted = false;
                            }
                            // Java: f2 += f2 * 1.15f * sin(speedBoostTime / maxSpeedBoostTime * PI)
                            float boostFrac = static_cast<float>(mob.pigSpeedBoostTime) /
                                              static_cast<float>(mob.pigMaxSpeedBoostTime);
                            adjustedSpeed += adjustedSpeed * 1.15f * std::sin(boostFrac * static_cast<float>(M_PI));
                        }

                        // Java: moveEntityWithHeading(0.0f, speed)
                        // Simplified: move forward along yaw at adjustedSpeed
                        float yawRad = mob.yaw / 180.0f * static_cast<float>(M_PI);
                        double moveX = -std::sin(yawRad) * adjustedSpeed;
                        double moveZ = std::cos(yawRad) * adjustedSpeed;

                        double newX = mob.posX + moveX;
                        double newZ = mob.posZ + moveZ;

                        // Ground/collision check (same as wander AI)
                        if (!worlds_.empty()) {
                            auto* wld = worlds_[0].get();
                            int bx = static_cast<int>(std::floor(newX));
                            int bz = static_cast<int>(std::floor(newZ));
                            int startY = static_cast<int>(mob.posY);

                            Block* feetBlock = wld->getBlock(bx, startY, bz);
                            if (feetBlock != nullptr) {
                                Block* stepBlock = wld->getBlock(bx, startY + 1, bz);
                                Block* headBlock = wld->getBlock(bx, startY + 2, bz);
                                if (stepBlock == nullptr && headBlock == nullptr) {
                                    mob.posX = newX;
                                    mob.posZ = newZ;
                                    mob.posY = static_cast<double>(startY + 1);
                                }
                            } else {
                                // Apply gravity
                                int groundY = startY;
                                for (int y = startY - 1; y > 0; --y) {
                                    Block* b = wld->getBlock(bx, y, bz);
                                    if (b != nullptr) { groundY = y + 1; break; }
                                    if (y == 1) groundY = 1;
                                }
                                mob.posX = newX;
                                mob.posZ = newZ;
                                mob.posY = static_cast<double>(groundY);
                            }
                        }

                        // Java: carrot_on_a_stick durability damage — 0.6% chance per tick
                        // Java: if (!creative && currentSpeed >= maxSpeed * 0.5f && rand.nextFloat() < 0.006f && !speedBoosted)
                        if (riderHeldId == 398 && rider->getGameMode() != 1 &&
                            mob.pigCurrentSpeed >= PIG_MAX_SPEED * 0.5f &&
                            (rand() % 1000) < 6 && !mob.pigSpeedBoosted) {
                            // Damage the carrot-on-a-stick using PlayHandler API
                            auto heldItem = rider->getHeldItem();
                            if (heldItem.has_value()) {
                                int32_t newDmg = heldItem->getDamage() + 1;
                                if (newDmg >= 25) {
                                    // Breaks → becomes fishing rod (346)
                                    rider->replaceHeldItem(ItemStack(346, 1, 0));
                                    broadcastSound("random.break", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
                                } else {
                                    heldItem->setDamage(newDmg);
                                    rider->replaceHeldItem(heldItem);
                                }
                                rider->sendWindowItems(*riderConn);
                            }
                        }

                        // Update rider position — Java: Entity.updateRiderPosition()
                        rider->setPlayerPosition(mob.posX, mob.posY + 0.63, mob.posZ);

                    } else {
                        // Not holding carrot-on-a-stick: pig stops (speed decays)
                        mob.pigCurrentSpeed = 0.0f;
                    }
                } else {
                    // Rider disconnected — dismount
                    mob.riderEntityId = -1;
                    mob.pigCurrentSpeed = 0.0f;
                    mob.pigSpeedBoosted = false;
                }

                // Broadcast position update for ridden pig
                {
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    for (auto& c : connections_) {
                        if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                        auto handler = c->getHandler();
                        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                        if (ph) {
                            ph->sendEntityTeleport(*c, mob.entityId,
                                mob.posX, mob.posY, mob.posZ, mob.yaw, mob.pitch);
                        }
                    }
                }
                mob.lastSentPosX = static_cast<int32_t>(std::floor(mob.posX * 32.0));
                mob.lastSentPosY = static_cast<int32_t>(std::floor(mob.posY * 32.0));
                mob.lastSentPosZ = static_cast<int32_t>(std::floor(mob.posZ * 32.0));
                continue; // Skip normal wander AI for ridden pig
            }

            // ─── Wolf AI — Java: EntityWolf AI tasks ──────────────────
            // Tamed wolves: sit or follow owner. Wild wolves: wander.
            if (mob.mobType == 95) {
                if (mob.isTamed && mob.isSitting) {
                    // Sitting — don't move at all — Java: EntityAISit
                    // Still broadcast position (entity might have been pushed)
                    {
                        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                        for (auto& conn : connections_) {
                            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                            auto handler = conn->getHandler();
                            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                            if (ph) ph->sendEntityTeleport(*conn, mob.entityId,
                                mob.posX, mob.posY, mob.posZ, mob.yaw, mob.pitch);
                        }
                    }
                    continue;
                }

                if (mob.isTamed && !mob.isSitting) {
                    // ─── Follow owner — Java: EntityAIFollowOwner ───
                    PlayHandler* owner = nullptr;
                    {
                        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                        for (auto& conn : connections_) {
                            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                            auto handler = conn->getHandler();
                            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                            if (ph && ph->getUuid() == mob.ownerUuid) {
                                owner = ph;
                                break;
                            }
                        }
                    }

                    if (owner) {
                        double dx = owner->getPlayerX() - mob.posX;
                        double dz = owner->getPlayerZ() - mob.posZ;
                        double distSq = dx * dx + dz * dz;

                        // Java: EntityAIFollowOwner teleport if > 12 blocks (144 sq)
                        if (distSq > 144.0) {
                            // Teleport near owner — Java: tries random positions within 3 blocks
                            double tpX = owner->getPlayerX() + ((rand() % 5) - 2);
                            double tpZ = owner->getPlayerZ() + ((rand() % 5) - 2);
                            double tpY = owner->getPlayerY();

                            // Find ground at teleport position
                            if (!worlds_.empty()) {
                                auto* wld = worlds_[0].get();
                                int bx = static_cast<int>(std::floor(tpX));
                                int bz = static_cast<int>(std::floor(tpZ));
                                int startY = static_cast<int>(tpY);
                                for (int y = startY; y > 0; --y) {
                                    Block* b = wld->getBlock(bx, y, bz);
                                    if (b != nullptr) {
                                        tpY = static_cast<double>(y + 1);
                                        break;
                                    }
                                }
                            }

                            mob.posX = tpX;
                            mob.posY = tpY;
                            mob.posZ = tpZ;
                        } else if (distSq > 4.0) {
                            // Walk toward owner — Java: EntityAIFollowOwner min dist = 2 blocks
                            double dist = std::sqrt(distSq);
                            double dirX = dx / dist;
                            double dirZ = dz / dist;
                            mob.posX += dirX * speed * 0.8;
                            mob.posZ += dirZ * speed * 0.8;

                            // Update yaw to face owner
                            mob.yaw = static_cast<float>(std::atan2(-dirX, dirZ) * 180.0 / M_PI);

                            // Ground/collision check
                            if (!worlds_.empty()) {
                                auto* wld = worlds_[0].get();
                                int bx = static_cast<int>(std::floor(mob.posX));
                                int bz = static_cast<int>(std::floor(mob.posZ));
                                int startY = static_cast<int>(mob.posY);
                                Block* feetBlock = wld->getBlock(bx, startY, bz);
                                if (feetBlock != nullptr) {
                                    Block* stepBlock = wld->getBlock(bx, startY + 1, bz);
                                    Block* headBlock = wld->getBlock(bx, startY + 2, bz);
                                    if (stepBlock == nullptr && headBlock == nullptr) {
                                        mob.posY = static_cast<double>(startY + 1);
                                    }
                                } else {
                                    for (int y = startY - 1; y > 0; --y) {
                                        Block* b = wld->getBlock(bx, y, bz);
                                        if (b != nullptr) { mob.posY = static_cast<double>(y + 1); break; }
                                        if (y == 1) mob.posY = 1.0;
                                    }
                                }
                            }
                        }
                        // else: within 2 blocks of owner, idle

                        // ─── Wolf attack target — defend owner ───
                        // Java: EntityAIOwnerHurtByTarget / EntityAIOwnerHurtTarget
                        // Simplified: if wolf has an attack target, walk to it and deal damage
                        if (mob.wolfAttackTarget >= 0) {
                            // Find target mob
                            SpawnedMob* target = nullptr;
                            for (auto& other : mobEntities_) {
                                if (other.entityId == mob.wolfAttackTarget && !other.isDead) {
                                    target = &other;
                                    break;
                                }
                            }
                            if (target) {
                                double tdx = target->posX - mob.posX;
                                double tdz = target->posZ - mob.posZ;
                                double tdistSq = tdx * tdx + tdz * tdz;
                                if (tdistSq < 2.0) {
                                    // Attack — Java: EntityWolf attack damage = 4 (tamed)
                                    target->health -= 4.0f;
                                    broadcastEntityEvent(target->entityId, 2); // hurt animation
                                    broadcastSound("mob.wolf.growl", mob.posX, mob.posY, mob.posZ, 0.4f, 1.0f);
                                    if (target->health <= 0.0f) {
                                        target->isDead = true;
                                        mob.wolfAttackTarget = -1;
                                    }
                                } else if (tdistSq < 256.0) {
                                    // Walk toward target
                                    double tdist = std::sqrt(tdistSq);
                                    mob.posX += (tdx / tdist) * speed;
                                    mob.posZ += (tdz / tdist) * speed;
                                    mob.yaw = static_cast<float>(std::atan2(-tdx / tdist, tdz / tdist) * 180.0 / M_PI);
                                } else {
                                    // Target too far, give up
                                    mob.wolfAttackTarget = -1;
                                }
                            } else {
                                mob.wolfAttackTarget = -1;
                            }
                        }
                    }

                    // Broadcast position
                    {
                        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                        for (auto& conn : connections_) {
                            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                            auto handler = conn->getHandler();
                            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                            if (ph) ph->sendEntityTeleport(*conn, mob.entityId,
                                mob.posX, mob.posY, mob.posZ, mob.yaw, mob.pitch);
                        }
                    }
                    mob.lastSentPosX = static_cast<int32_t>(std::floor(mob.posX * 32.0));
                    mob.lastSentPosY = static_cast<int32_t>(std::floor(mob.posY * 32.0));
                    mob.lastSentPosZ = static_cast<int32_t>(std::floor(mob.posZ * 32.0));
                    continue; // Skip generic wander
                }
                // Wild wolf: fall through to normal passive wander AI
            }

            // ─── Ocelot/Cat AI — Java: EntityOcelot AI tasks ──────────────
            // Tamed cats: sit or follow owner. Wild ocelots: wander.
            if (mob.mobType == 98) {
                if (mob.isTamed && mob.isSitting) {
                    // Sitting — don't move — Java: EntityAISit
                    {
                        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                        for (auto& conn : connections_) {
                            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                            auto handler = conn->getHandler();
                            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                            if (ph) ph->sendEntityTeleport(*conn, mob.entityId,
                                mob.posX, mob.posY, mob.posZ, mob.yaw, mob.pitch);
                        }
                    }
                    continue;
                }

                if (mob.isTamed && !mob.isSitting) {
                    // ─── Follow owner — Java: EntityAIFollowOwner (minDist=5, maxDist=10) ───
                    PlayHandler* owner = nullptr;
                    {
                        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                        for (auto& conn : connections_) {
                            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                            auto handler = conn->getHandler();
                            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                            if (ph && ph->getUuid() == mob.ownerUuid) {
                                owner = ph;
                                break;
                            }
                        }
                    }

                    if (owner) {
                        double dx = owner->getPlayerX() - mob.posX;
                        double dz = owner->getPlayerZ() - mob.posZ;
                        double distSq = dx * dx + dz * dz;

                        // Java: EntityAIFollowOwner teleport if > 12 blocks (144 sq)
                        if (distSq > 144.0) {
                            double tpX = owner->getPlayerX() + ((rand() % 5) - 2);
                            double tpZ = owner->getPlayerZ() + ((rand() % 5) - 2);
                            double tpY = owner->getPlayerY();
                            if (!worlds_.empty()) {
                                auto* wld = worlds_[0].get();
                                int bx = static_cast<int>(std::floor(tpX));
                                int bz = static_cast<int>(std::floor(tpZ));
                                int startY = static_cast<int>(tpY);
                                for (int y = startY; y > 0; --y) {
                                    Block* b = wld->getBlock(bx, y, bz);
                                    if (b != nullptr) {
                                        tpY = static_cast<double>(y + 1);
                                        break;
                                    }
                                }
                            }
                            mob.posX = tpX;
                            mob.posY = tpY;
                            mob.posZ = tpZ;
                        } else if (distSq > 25.0) {
                            // Walk toward owner — Java: EntityAIFollowOwner minDist=5
                            double dist = std::sqrt(distSq);
                            double dirX = dx / dist;
                            double dirZ = dz / dist;
                            mob.posX += dirX * speed * 0.8;
                            mob.posZ += dirZ * speed * 0.8;
                            mob.yaw = static_cast<float>(std::atan2(-dirX, dirZ) * 180.0 / M_PI);

                            // Ground/collision check
                            if (!worlds_.empty()) {
                                auto* wld = worlds_[0].get();
                                int bx = static_cast<int>(std::floor(mob.posX));
                                int bz = static_cast<int>(std::floor(mob.posZ));
                                int startY = static_cast<int>(mob.posY);
                                Block* feetBlock = wld->getBlock(bx, startY, bz);
                                if (feetBlock != nullptr) {
                                    Block* stepBlock = wld->getBlock(bx, startY + 1, bz);
                                    Block* headBlock = wld->getBlock(bx, startY + 2, bz);
                                    if (stepBlock == nullptr && headBlock == nullptr) {
                                        mob.posY = static_cast<double>(startY + 1);
                                    }
                                } else {
                                    for (int y = startY - 1; y > 0; --y) {
                                        Block* b = wld->getBlock(bx, y, bz);
                                        if (b != nullptr) { mob.posY = static_cast<double>(y + 1); break; }
                                        if (y == 1) mob.posY = 1.0;
                                    }
                                }
                            }
                        }
                    }

                    // Broadcast position
                    {
                        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                        for (auto& conn : connections_) {
                            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                            auto handler = conn->getHandler();
                            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                            if (ph) ph->sendEntityTeleport(*conn, mob.entityId,
                                mob.posX, mob.posY, mob.posZ, mob.yaw, mob.pitch);
                        }
                    }
                    mob.lastSentPosX = static_cast<int32_t>(std::floor(mob.posX * 32.0));
                    mob.lastSentPosY = static_cast<int32_t>(std::floor(mob.posY * 32.0));
                    mob.lastSentPosZ = static_cast<int32_t>(std::floor(mob.posZ * 32.0));
                    continue; // Skip generic wander
                }
                // Wild ocelot: fall through to normal passive wander AI
            }

            // ─── Passive mob wander AI ───────────────────────────────
            // Java: EntityAIWander — pick random direction, walk slowly
            if (mob.isPassive) {
                if (mob.wanderCooldown > 0) {
                    --mob.wanderCooldown;
                } else {
                    // Pick new random wander direction
                    mob.wanderYaw = static_cast<float>((rand() % 360) - 180);
                    mob.wanderCooldown = 40 + (rand() % 80); // 2-6 seconds
                }

                // Only move 50% of the time (idle periods)
                if (mob.wanderCooldown > 20) {
                    float yawRad = mob.wanderYaw / 180.0f * static_cast<float>(M_PI);
                    double moveX = -std::sin(yawRad) * speed * 0.5; // Half speed wander
                    double moveZ = std::cos(yawRad) * speed * 0.5;

                    double newX = mob.posX + moveX;
                    double newZ = mob.posZ + moveZ;

                    // Check for valid ground at new position
                    if (!worlds_.empty()) {
                        auto* wld = worlds_[0].get();
                        int bx = static_cast<int>(std::floor(newX));
                        int bz = static_cast<int>(std::floor(newZ));
                        int startY = static_cast<int>(mob.posY);

                        Block* feetBlock = wld->getBlock(bx, startY, bz);
                        if (feetBlock != nullptr) {
                            // Try step up
                            Block* stepBlock = wld->getBlock(bx, startY + 1, bz);
                            Block* headBlock = wld->getBlock(bx, startY + 2, bz);
                            if (stepBlock == nullptr && headBlock == nullptr) {
                                mob.posX = newX;
                                mob.posZ = newZ;
                                mob.posY = static_cast<double>(startY + 1);
                            }
                            // Otherwise don't move (wall)
                        } else {
                            // Apply gravity
                            int groundY = startY;
                            for (int y = startY - 1; y > 0; --y) {
                                Block* b = wld->getBlock(bx, y, bz);
                                if (b != nullptr) { groundY = y + 1; break; }
                                if (y == 1) groundY = 1;
                            }
                            mob.posX = newX;
                            mob.posZ = newZ;
                            mob.posY = static_cast<double>(groundY);
                        }
                    }

                    mob.yaw = mob.wanderYaw;
                }

                // Broadcast position update for passive mobs
                {
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    for (auto& conn : connections_) {
                        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                        auto handler = conn->getHandler();
                        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                        if (!ph) continue;
                        ph->sendEntityTeleport(*conn, mob.entityId,
                            mob.posX, mob.posY, mob.posZ, mob.yaw, mob.pitch);
                    }
                }
                mob.lastSentPosX = static_cast<int32_t>(std::floor(mob.posX * 32.0));
                mob.lastSentPosY = static_cast<int32_t>(std::floor(mob.posY * 32.0));
                mob.lastSentPosZ = static_cast<int32_t>(std::floor(mob.posZ * 32.0));
                continue; // Skip hostile AI for passive mobs
            }

            // Zombie pigmen only chase when angry — Java: EntityPigZombie.findPlayerToAttack()
            if (mob.mobType == 57) {
                if (mob.angerLevel <= 0) continue; // Passive unless provoked
                --mob.angerLevel;
            }

            // ─── Skeleton/Zombie flee-from-sun AI ────────────────────
            // Java: EntityAIFleeSun — undead mobs seek shelter during daytime
            if ((mob.mobType == 51 || mob.mobType == 54) && !worlds_.empty()) {
                int64_t wt = getWorldTime() % 24000;
                if (wt >= 0 && wt < 12300 && !isRaining()) {
                    int bx = static_cast<int>(std::floor(mob.posX));
                    int by = static_cast<int>(std::floor(mob.posY));
                    int bz = static_cast<int>(std::floor(mob.posZ));
                    int32_t blockAbove = getBlockIdInWorld(bx, by + 1, bz);
                    if (blockAbove == 0) {
                        // In sunlight — move randomly to seek shelter instead of chasing player
                        double fleeDx = ((double)(rand() % 100) / 100.0 - 0.5) * 2.0;
                        double fleeDz = ((double)(rand() % 100) / 100.0 - 0.5) * 2.0;
                        double dist = std::sqrt(fleeDx * fleeDx + fleeDz * fleeDz);
                        if (dist > 0.01) {
                            mob.posX += (fleeDx / dist) * speed;
                            mob.posZ += (fleeDz / dist) * speed;
                        }
                        continue; // Skip normal targeting while fleeing sun
                    }
                }
            }

            // Find nearest player within 16 blocks
            PlayHandler* nearest = nullptr;
            double nearestDistSq = 256.0; // 16 blocks squared
            {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph || ph->isDead()) continue;
                    if (ph->getGameMode() == 1 || ph->getGameMode() == 3) continue; // Skip creative/spectator

                    double dx = ph->getPlayerX() - mob.posX;
                    double dz = ph->getPlayerZ() - mob.posZ;
                    double distSq = dx * dx + dz * dz;
                    if (distSq < nearestDistSq) {
                        nearestDistSq = distSq;
                        nearest = ph;
                    }
                }
            }

            if (!nearest) continue;
            if (nearestDistSq < 2.25) continue; // Already close enough (1.5 blocks)

            // Move toward target
            double dx = nearest->getPlayerX() - mob.posX;
            double dz = nearest->getPlayerZ() - mob.posZ;
            double dist = std::sqrt(dx * dx + dz * dz);

            if (dist > 0.01) {
                double moveX = (dx / dist) * speed;
                double moveZ = (dz / dist) * speed;
                mob.posX += moveX;
                mob.posZ += moveZ;

                // Update yaw to face target — Java: atan2(-dx, dz) * 180/PI
                mob.yaw = static_cast<float>(std::atan2(-dx, dz) * 180.0 / M_PI);

                // Gravity + step-up: find valid surface Y at new position
                // Java: Entity.moveEntity() with stepHeight=1.0
                if (!worlds_.empty()) {
                    auto* wld = worlds_[0].get();
                    int bx = static_cast<int>(std::floor(mob.posX));
                    int bz = static_cast<int>(std::floor(mob.posZ));
                    int startY = static_cast<int>(mob.posY);

                    // Check if feet position is blocked (solid block at mob.posY)
                    Block* feetBlock = wld->getBlock(bx, startY, bz);
                    if (feetBlock != nullptr) {
                        // Can we step up? Check block at startY+1 and startY+2 (head)
                        Block* stepBlock = wld->getBlock(bx, startY + 1, bz);
                        Block* headBlock = wld->getBlock(bx, startY + 2, bz);
                        if (stepBlock == nullptr && headBlock == nullptr) {
                            // Step up 1 block
                            mob.posY = static_cast<double>(startY + 1);
                        } else {
                            // Can't step up — revert position
                            mob.posX -= moveX;
                            mob.posZ -= moveZ;
                        }
                    } else {
                        // Not blocked at feet — check for ground below (gravity)
                        int groundY = startY;
                        for (int y = startY - 1; y > 0; --y) {
                            Block* b = wld->getBlock(bx, y, bz);
                            if (b != nullptr) {
                                groundY = y + 1;
                                break;
                            }
                            if (y == 1) groundY = 1;
                        }
                        if (groundY < startY) {
                            mob.posY = static_cast<double>(groundY);
                        }
                    }
                }

                // Broadcast S18 EntityTeleport to all players
                {
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    for (auto& conn : connections_) {
                        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                        auto handler = conn->getHandler();
                        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                        if (!ph) continue;
                        ph->sendEntityTeleport(*conn, mob.entityId,
                            mob.posX, mob.posY, mob.posZ, mob.yaw, mob.pitch);
                    }
                }

                // Update tracking
                mob.lastSentPosX = static_cast<int32_t>(std::floor(mob.posX * 32.0));
                mob.lastSentPosY = static_cast<int32_t>(std::floor(mob.posY * 32.0));
                mob.lastSentPosZ = static_cast<int32_t>(std::floor(mob.posZ * 32.0));
            }
        }
        // ─── Spider leap attack ──────────────────────────────────────
        // Java: EntitySpider.attackEntity() — leap at player 2-6 blocks away
        // motionX = dx/dist * 0.5 * 0.8 + motionX * 0.2, motionY = 0.4f
        for (auto& mob : mobEntities_) {
            if (mob.isDead) continue;
            if (mob.mobType != 52 && mob.mobType != 59) continue; // Spider(52) + Cave Spider(59)
            if (mob.attackCooldown > 0) { --mob.attackCooldown; continue; }

            PlayHandler* nearest = nullptr;
            double nearestDistSq = 36.0; // 6 blocks max
            {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph || ph->isDead()) continue;
                    if (ph->getGameMode() == 1 || ph->getGameMode() == 3) continue;
                    double dx = ph->getPlayerX() - mob.posX;
                    double dz = ph->getPlayerZ() - mob.posZ;
                    double distSq = dx * dx + dz * dz;
                    if (distSq > 4.0 && distSq < nearestDistSq) { // 2-6 blocks
                        nearestDistSq = distSq;
                        nearest = ph;
                    }
                }
            }

            if (!nearest || (rand() % 10) != 0) continue; // 1/10 chance per tick

            // Spider leaps — apply velocity-based knockback on target
            // Simplified: deal spider damage (2.0 for spider, 2.0 for cave spider)
            float spiderDmg = 2.0f;
            int32_t armorVal = nearest->getTotalArmorValue();
            if (armorVal > 0) {
                spiderDmg *= (25.0f - armorVal) / 25.0f;
            }
            int32_t protMod = nearest->getEnchantmentProtectionModifier();
            if (protMod > 0) {
                spiderDmg *= (1.0f - std::min(protMod, 20) * 0.04f);
            }
            if (spiderDmg < 0.5f) spiderDmg = 0.5f;

            nearest->applyDamage(spiderDmg);
            broadcastEntityEvent(nearest->getEntityId(), 2);
            broadcastSound("mob.spider.say", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
            broadcastSound("game.player.hurt", nearest->getPlayerX(), nearest->getPlayerY(), nearest->getPlayerZ(), 1.0f, 1.0f);

            if (nearest->getHealth() <= 0.0f) {
                broadcastEntityEvent(nearest->getEntityId(), 3);
                broadcastChatMessage(nearest->getPlayerName() +
                    (mob.mobType == 59 ? " was slain by Cave Spider" : " was slain by Spider"));
            }

            mob.attackCooldown = 20; // 1 second between leap attacks
        }

        // ─── Undead mob sunlight burning ─────────────────────────────
        // Java: EntityZombie/EntitySkeleton.onLivingUpdate() — burns in sunlight
        // Undead mobs (zombie=54, skeleton=51) catch fire during daytime
        {
            int64_t worldTime = getWorldTime() % 24000;
            bool isDaytime = (worldTime >= 0 && worldTime < 12300);
            if (isDaytime && !isRaining()) {
                for (auto& mob : mobEntities_) {
                    if (mob.isDead) continue;
                    // Only undead: zombie(54), skeleton(51)
                    // Note: pigmen(57) are undead but spawn in Nether, not affected here
                    if (mob.mobType != 54 && mob.mobType != 51) continue;

                    // Check if mob can see sky — simplified: Y > 62 (sea level)
                    // Java: world.canBlockSeeTheSky()
                    int bx = static_cast<int>(std::floor(mob.posX));
                    int by = static_cast<int>(std::floor(mob.posY));
                    int bz = static_cast<int>(std::floor(mob.posZ));
                    // Check block above head — if not solid, sunlight can reach
                    int32_t blockAbove = getBlockIdInWorld(bx, by + 1, bz);
                    if (blockAbove != 0) continue; // Block above = sheltered

                    // Java: EntityLiving.onLivingUpdate() → setFire(8)
                    mob.isOnFire = true;
                    mob.health -= 1.0f;
                    broadcastEntityEvent(mob.entityId, 2);
                    if (mob.health <= 0.0f) {
                        mob.isDead = true;
                        broadcastEntityEvent(mob.entityId, 3);
                    }
                }
            }
        }

        // ─── Enderman water damage ──────────────────────────────────
        // Java: EntityEnderman.onLivingUpdate() — isWet() → drown 1.0 dmg
        // Endermen take damage in water and teleport away
        if (!worlds_.empty()) {
            for (auto& mob : mobEntities_) {
                if (mob.isDead || mob.mobType != 58) continue; // Enderman only
                int bx = static_cast<int>(std::floor(mob.posX));
                int by = static_cast<int>(std::floor(mob.posY));
                int bz = static_cast<int>(std::floor(mob.posZ));
                // Check if standing in water (block 8=flowing, 9=source)
                int32_t blockId = getBlockIdInWorld(bx, by, bz);
                if (blockId != 8 && blockId != 9) continue;

                mob.health -= 1.0f; // 1 damage per tick
                broadcastEntityEvent(mob.entityId, 2);
                broadcastSound("game.hostile.hurt", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);

                if (mob.health <= 0.0f) {
                    mob.isDead = true;
                    broadcastEntityEvent(mob.entityId, 3);
                    continue;
                }

                // Try to teleport away — same as teleport-on-hit
                for (int tp = 0; tp < 64; ++tp) {
                    double newX = mob.posX + ((double)rand() / RAND_MAX - 0.5) * 64.0;
                    double newY = mob.posY + (double)(rand() % 64 - 32);
                    double newZ = mob.posZ + ((double)rand() / RAND_MAX - 0.5) * 64.0;
                    int nbx = static_cast<int>(std::floor(newX));
                    int nby = static_cast<int>(std::floor(newY));
                    int nbz = static_cast<int>(std::floor(newZ));
                    if (nby < 1 || nby > 250) continue;
                    // Find ground
                    while (nby > 1 && getBlockIdInWorld(nbx, nby - 1, nbz) == 0) --nby;
                    // Check 2 air blocks above ground
                    if (getBlockIdInWorld(nbx, nby, nbz) != 0 || getBlockIdInWorld(nbx, nby + 1, nbz) != 0) continue;
                    broadcastSound("mob.endermen.portal", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
                    mob.posX = newX;
                    mob.posY = static_cast<double>(nby);
                    mob.posZ = newZ;
                    broadcastSound("mob.endermen.portal", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
                    {
                        std::lock_guard<std::recursive_mutex> cl(connectionsMutex_);
                        for (auto& c : connections_) {
                            if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                            auto h = c->getHandler();
                            auto* p = dynamic_cast<PlayHandler*>(h.get());
                            if (p) p->sendEntityTeleport(*c, mob.entityId, mob.posX, mob.posY, mob.posZ, mob.yaw, 0.0f);
                        }
                    }
                    break;
                }
            }
        }

        // ─── Creeper explosion AI ──────────────────────────────────

        // Java: EntityCreeper.onUpdate() — fuse timer, explode when timeSinceIgnited >= fuseTime(30)
        for (auto& mob : mobEntities_) {
            if (mob.isDead || mob.mobType != 50) continue; // Creeper only

            // ─── Creeper-ocelot avoidance — Java: EntityAIAvoidEntity(EntityOcelot.class, 6.0f, 1.0, 1.2) ───
            // Creepers flee from ocelots/cats within 6 blocks
            {
                bool flee = false;
                double fleeX = 0, fleeZ = 0;
                for (auto& other : mobEntities_) {
                    if (other.isDead || other.mobType != 98) continue;
                    double cdx = other.posX - mob.posX;
                    double cdz = other.posZ - mob.posZ;
                    double cdistSq = cdx * cdx + cdz * cdz;
                    if (cdistSq < 36.0 && cdistSq > 0.01) {  // 6 blocks squared
                        // Flee away from ocelot
                        double cdist = std::sqrt(cdistSq);
                        fleeX = -cdx / cdist;
                        fleeZ = -cdz / cdist;
                        flee = true;
                        break;
                    }
                }
                if (flee) {
                    // Run away from ocelot — Java: speed 1.0-1.2
                    float fleeSpeed = getMobMovementSpeed(50) * 1.5f;
                    mob.posX += fleeX * fleeSpeed;
                    mob.posZ += fleeZ * fleeSpeed;
                    mob.yaw = static_cast<float>(std::atan2(-fleeX, fleeZ) * 180.0 / M_PI);
                    if (mob.fuseTicks > 0) mob.fuseTicks = 0;  // Cancel fuse
                    continue;  // Skip normal creeper AI
                }
            }

            // Find nearest player within 3 blocks
            PlayHandler* nearest = nullptr;
            double nearestDistSq = 9.0; // 3 blocks squared
            {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph || ph->isDead()) continue;
                    if (ph->getGameMode() == 1 || ph->getGameMode() == 3) continue;
                    double dx = ph->getPlayerX() - mob.posX;
                    double dz = ph->getPlayerZ() - mob.posZ;
                    double distSq = dx * dx + dz * dz;
                    if (distSq < nearestDistSq) {
                        nearestDistSq = distSq;
                        nearest = ph;
                    }
                }
            }

            if (nearest) {
                ++mob.fuseTicks;
                // Java: EntityCreeper.onUpdate() — play primed sound when fuse starts
                if (mob.fuseTicks == 1) {
                    broadcastSound("creeper.primed", mob.posX, mob.posY, mob.posZ, 1.0f, 0.5f);
                }
                // Java: EntityCreeper.fuseTime = 30 (1.5 seconds)
                if (mob.fuseTicks >= 30) {
                    // EXPLODE — Java: EntityCreeper.explode() → createExplosion(power=3.0)
                    createExplosion(mob.posX, mob.posY, mob.posZ, 3.0f, false, true);
                    mob.isDead = true;
                    deadIds.push_back(mob.entityId);
                }
                // Broadcast creeper swell sound via S1C metadata (simplified: S1A status 17 = ignite)
            } else {
                // Reset fuse if no player nearby — Java: EntityCreeper.setCreeperState(-1)
                if (mob.fuseTicks > 0) mob.fuseTicks = 0;
            }
        }

        // ─── Skeleton ranged attack AI ─────────────────────────────
        // Java: EntitySkeleton.attackEntityWithRangedAttack() — shoots arrow projectile
        // Now uses real arrow projectiles via spawnArrow() instead of instant-hit
        for (auto& mob : mobEntities_) {
            if (mob.isDead || mob.mobType != 51) continue; // Skeleton only
            if (mob.attackCooldown > 0) { --mob.attackCooldown; continue; }

            // Find nearest player within 16 blocks
            PlayHandler* nearest = nullptr;
            double nearestDistSq = 256.0;
            {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph || ph->isDead()) continue;
                    if (ph->getGameMode() == 1 || ph->getGameMode() == 3) continue;
                    double dx = ph->getPlayerX() - mob.posX;
                    double dz = ph->getPlayerZ() - mob.posZ;
                    double distSq = dx * dx + dz * dz;
                    if (distSq < nearestDistSq) {
                        nearestDistSq = distSq;
                        nearest = ph;
                    }
                }
            }

            if (!nearest || nearestDistSq < 4.0) continue; // Too close = melee

            // Calculate arrow trajectory toward player
            // Java: EntityArrow(world, skeleton, player, 1.6f, inaccuracy)
            // Uses setThrowableHeading(dx, dy+loft, dz, speed=1.6, inaccuracy)
            double dx = nearest->getPlayerX() - mob.posX;
            double dy = (nearest->getPlayerY() + 0.9) - (mob.posY + 1.62); // eye to center
            double dz = nearest->getPlayerZ() - mob.posZ;
            double dist = std::sqrt(dx*dx + dz*dz); // horizontal distance
            if (dist < 0.01) continue;

            // Java line 79: float f5 = (float)d4 * 0.2f → upward loft
            double loft = dist * 0.2;

            // Pass raw direction to spawnArrow, let setThrowableHeading normalize + scale
            // Java: speed=1.6, inaccuracy = 14 - difficulty*4 (Normal=2 → 6)
            broadcastSound("random.bow", mob.posX, mob.posY, mob.posZ, 1.0f, 1.2f);
            spawnArrow(mob.posX, mob.posY + 1.62, mob.posZ,
                        dx, dy + loft, dz,
                        mob.entityId, 2.0, 0, false,
                        1.6f, 6.0f);

            mob.attackCooldown = 60; // 3 seconds between shots
        }

        // ─── Blaze fireball AI ─────────────────────────────────────
        // Java: EntityBlaze.attackEntityWithRangedAttack() — shoots fireballs
        // Simplified: instant-hit fire damage every 60 ticks within 16 blocks
        for (auto& mob : mobEntities_) {
            if (mob.isDead || mob.mobType != 61) continue; // Blaze only
            if (mob.attackCooldown > 0) { --mob.attackCooldown; continue; }

            PlayHandler* nearest = nullptr;
            Connection* nearestConn = nullptr;
            double nearestDistSq = 256.0;
            {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph || ph->isDead()) continue;
                    if (ph->getGameMode() == 1 || ph->getGameMode() == 3) continue;
                    double dx = ph->getPlayerX() - mob.posX;
                    double dz = ph->getPlayerZ() - mob.posZ;
                    double distSq = dx * dx + dz * dz;
                    if (distSq < nearestDistSq) {
                        nearestDistSq = distSq;
                        nearest = ph;
                        nearestConn = conn.get();
                    }
                }
            }

            if (!nearest) continue;

            // Fireball damage — Java: EntitySmallFireball deals 5 fire damage
            float fireballDmg = 5.0f;
            // Fire Protection (damageType=1) reduces fireball damage
            int32_t fireProt = nearest->getEnchantmentProtectionModifier(1);
            if (fireProt > 0) {
                fireballDmg *= (1.0f - std::min(fireProt, 20) * 0.04f);
            }
            nearest->applyDamage(fireballDmg);
            // Set player on fire for 5 seconds (100 ticks)
            nearest->setOnFire(100);
            broadcastEntityEvent(nearest->getEntityId(), 2);
            broadcastSound("game.player.hurt", nearest->getPlayerX(), nearest->getPlayerY(), nearest->getPlayerZ(), 1.0f, 1.0f);
            broadcastSound("mob.blaze.hit", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);

            if (nearest->getHealth() <= 0.0f) {
                broadcastEntityEvent(nearest->getEntityId(), 3);
                broadcastChatMessage(nearest->getPlayerName() + " was fireballed by Blaze");
            }

            mob.attackCooldown = 60;
        }

        // ─── Ghast fireball AI ─────────────────────────────────────
        // Java: EntityGhast.updateEntityActionState() — large fireball attack
        // Range: 64 blocks, explosionStrength=1, attackCounter 0→20→fire→-40
        for (auto& mob : mobEntities_) {
            if (mob.isDead || mob.mobType != 56) continue; // Ghast only (type 56)
            if (mob.attackCooldown > 0) { --mob.attackCooldown; continue; }

            PlayHandler* nearest = nullptr;
            Connection* nearestConn = nullptr;
            double nearestDistSq = 4096.0; // 64 blocks squared
            {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph || ph->isDead()) continue;
                    if (ph->getGameMode() == 1 || ph->getGameMode() == 3) continue;
                    double dx = ph->getPlayerX() - mob.posX;
                    double dz = ph->getPlayerZ() - mob.posZ;
                    double distSq = dx * dx + dz * dz;
                    if (distSq < nearestDistSq) {
                        nearestDistSq = distSq;
                        nearest = ph;
                        nearestConn = conn.get();
                    }
                }
            }

            if (!nearest) continue;

            // Java: EntityGhast fires large fireball — simplified to instant-hit explosion
            // EntityLargeFireball.onImpact() → createExplosion(explosionStrength=1)
            double tx = nearest->getPlayerX();
            double ty = nearest->getPlayerY();
            double tz = nearest->getPlayerZ();

            // Create explosion at target (power=1.0, causesFire=true, breakBlocks=true)
            createExplosion(tx, ty, tz, 1.0f, true, true);

            broadcastSound("mob.ghast.fireball", mob.posX, mob.posY, mob.posZ, 10.0f, 1.0f);

            mob.attackCooldown = 60; // Java: attackCounter = -40, fires at 20 → 60 tick cycle
        }

        // ─── Blaze fireball attack ────────────────────────────────────
        // Java: EntityBlaze.attackEntity() — 3 small fireballs in burst at 30 blocks
        for (auto& mob : mobEntities_) {
            if (mob.isDead || mob.mobType != 61) continue; // Blaze only (type 61)
            if (mob.attackCooldown > 0) { --mob.attackCooldown; continue; }

            PlayHandler* nearest = nullptr;
            Connection* nearestConn = nullptr;
            double nearestDistSq = 900.0; // 30 blocks squared
            {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph || ph->isDead()) continue;
                    if (ph->getGameMode() == 1 || ph->getGameMode() == 3) continue;
                    double dx = ph->getPlayerX() - mob.posX;
                    double dz = ph->getPlayerZ() - mob.posZ;
                    double distSq = dx * dx + dz * dz;
                    if (distSq < nearestDistSq) {
                        nearestDistSq = distSq;
                        nearest = ph;
                        nearestConn = conn.get();
                    }
                }
            }

            if (!nearest) continue;

            // Java: EntitySmallFireball deals 5 fire damage
            float blazeDmg = 5.0f;
            int32_t protMod = nearest->getEnchantmentProtectionModifier();
            if (protMod > 0) {
                blazeDmg *= (1.0f - std::min(protMod, 20) * 0.04f);
            }
            if (blazeDmg < 0.5f) blazeDmg = 0.5f;

            nearest->applyDamage(blazeDmg);
            nearest->sendUpdateHealth(*nearestConn, nearest->getHealth(), nearest->getFood(), nearest->getSaturation());

            broadcastSound("mob.blaze.hit", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
            broadcastSound("game.player.hurt",
                nearest->getPlayerX(), nearest->getPlayerY(), nearest->getPlayerZ(), 1.0f, 1.0f);

            // Effect 1009 = fire charge sound
            broadcastEffect(1009, static_cast<int32_t>(mob.posX), static_cast<int32_t>(mob.posY),
                static_cast<int32_t>(mob.posZ), 0);

            if (nearest->getHealth() <= 0.0f) {
                broadcastEntityEvent(nearest->getEntityId(), 3);
                broadcastChatMessage(nearest->getPlayerName() + " was killed by Blaze");
            }

            mob.attackCooldown = 60; // Java: burst cycle ~60 ticks total
        }

        // ─── Blaze water damage ──────────────────────────────────────
        // Java: EntityBlaze.onLivingUpdate() — isWet() → drown 1.0 dmg
        for (auto& mob : mobEntities_) {
            if (mob.isDead || mob.mobType != 61) continue;
            int bx = static_cast<int>(std::floor(mob.posX));
            int by = static_cast<int>(std::floor(mob.posY));
            int bz = static_cast<int>(std::floor(mob.posZ));
            int32_t blockId = getBlockIdInWorld(bx, by, bz);
            if (blockId != 8 && blockId != 9) continue; // water
            mob.health -= 1.0f;
            broadcastEntityEvent(mob.entityId, 2);
            broadcastSound("game.hostile.hurt", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
            if (mob.health <= 0.0f) {
                mob.isDead = true;
                broadcastEntityEvent(mob.entityId, 3);
            }
        }

        // ─── Iron Golem defense AI ───────────────────────────────────
        // Java: EntityIronGolem.onLivingUpdate() — attacks nearby hostile mobs
        // Golem attacks hostiles within 16 blocks, deals 7-21 damage with knockback
        for (auto& golem : mobEntities_) {
            if (golem.isDead || golem.mobType != 99) continue; // Iron Golem (type 99)
            if (golem.attackCooldown > 0) { --golem.attackCooldown; continue; }

            // Find nearest hostile mob within 16 blocks
            SpawnedMob* target = nullptr;
            double nearestMobDist = 256.0; // 16 blocks squared
            for (auto& mob : mobEntities_) {
                if (mob.isDead || &mob == &golem) continue;
                // Hostile mobs only
                if (mob.mobType != 54 && mob.mobType != 51 && mob.mobType != 50 &&
                    mob.mobType != 52 && mob.mobType != 59 && mob.mobType != 60 &&
                    mob.mobType != 66) continue;
                double dx = mob.posX - golem.posX;
                double dz = mob.posZ - golem.posZ;
                double distSq = dx * dx + dz * dz;
                if (distSq < nearestMobDist) {
                    nearestMobDist = distSq;
                    target = &mob;
                }
            }

            if (!target) continue;

            // Java: EntityIronGolem.attackEntityAsMob() — 7-21 damage + 1 block knockback
            float golemDmg = 7.0f + (float)(rand() % 15); // 7-21 damage
            target->health -= golemDmg;
            broadcastEntityEvent(target->entityId, 2);
            broadcastSound("mob.irongolem.throw", golem.posX, golem.posY, golem.posZ, 1.0f, 1.0f);

            if (target->health <= 0.0f) {
                target->isDead = true;
                broadcastEntityEvent(target->entityId, 3);
            }

            golem.attackCooldown = 20; // 1 second cooldown
        }

        // Java: EntityWitch.attackEntityWithRangedAttack() — EntityPotion projectile
        // Simplified: instant-hit splash potion dealing magic damage at 10 blocks range
        for (auto& mob : mobEntities_) {
            if (mob.isDead || mob.mobType != 66) continue; // Witch only (type 66)
            if (mob.attackCooldown > 0) { --mob.attackCooldown; continue; }

            PlayHandler* nearest = nullptr;
            Connection* nearestConn = nullptr;
            double nearestDistSq = 100.0; // 10 blocks squared
            {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph || ph->isDead()) continue;
                    if (ph->getGameMode() == 1 || ph->getGameMode() == 3) continue;
                    double dx = ph->getPlayerX() - mob.posX;
                    double dz = ph->getPlayerZ() - mob.posZ;
                    double distSq = dx * dx + dz * dz;
                    if (distSq < nearestDistSq) {
                        nearestDistSq = distSq;
                        nearest = ph;
                        nearestConn = conn.get();
                    }
                }
            }

            if (!nearest) continue;

            // Apply potion damage — Java witch throws poison/slowness/weakness potions
            // Simplified: 6 magic damage (similar to harming potion)
            float potionDmg = 6.0f;
            int32_t protMod = nearest->getEnchantmentProtectionModifier();
            if (protMod > 0) {
                potionDmg *= (1.0f - std::min(protMod, 20) * 0.04f);
            }
            if (potionDmg < 0.5f) potionDmg = 0.5f;

            nearest->applyDamage(potionDmg);
            nearest->sendUpdateHealth(*nearestConn, nearest->getHealth(), nearest->getFood(), nearest->getSaturation());

            // Splash potion effect visual (effectId 2002, data=0)
            broadcastEffect(2002,
                static_cast<int32_t>(nearest->getPlayerX()),
                static_cast<int32_t>(nearest->getPlayerY()),
                static_cast<int32_t>(nearest->getPlayerZ()), 0);
            broadcastSound("game.player.hurt",
                nearest->getPlayerX(), nearest->getPlayerY(), nearest->getPlayerZ(), 1.0f, 1.0f);

            if (nearest->getHealth() <= 0.0f) {
                broadcastEntityEvent(nearest->getEntityId(), 3);
                broadcastChatMessage(nearest->getPlayerName() + " was killed by Witch");
            }

            mob.attackCooldown = 60; // Java: EntityAIArrowAttack attack interval = 60 ticks
        }

        // ─── Mob contact damage ─────────────────────────────────────
        // Java: EntityCreature.attackEntityAsMob() — deal damage to nearby players
        // Simplified: proximity-based melee attack, 20-tick cooldown
        auto getMobAttackDamage = [](uint8_t mobType) -> float {
            switch (mobType) {
                case 54: return 3.0f;   // Zombie — Java: 3.0 (Easy: 2, Normal: 3, Hard: 4)
                case 51: return 2.0f;   // Skeleton (melee fallback) — arrows handled separately
                case 50: return 0.0f;   // Creeper — doesn't melee, explodes
                case 52: return 2.0f;   // Spider — Java: 2.0
                case 55: return 3.0f;   // Slime — Java: size-based, simplified to 3
                case 56: return 0.0f;   // Ghast — ranged only (fireballs)
                case 57: return 5.0f;   // Zombie Pigman — Java: 5.0 (Normal)
                case 58: return 7.0f;   // Enderman — Java: 7.0
                case 59: return 2.0f;   // Cave Spider — Java: 2.0
                case 60: return 1.0f;   // Silverfish — Java: 1.0
                case 61: return 6.0f;   // Blaze — Java: 6.0 (melee)
                case 62: return 3.0f;   // Magma Cube — Java: size-based, simplified
                case 66: return 0.0f;   // Witch — uses splash potions, no melee
                default: return 0.0f;
            }
        };

        for (auto& mob : mobEntities_) {
            if (mob.isDead) continue;
            float atkDmg = getMobAttackDamage(mob.mobType);
            if (atkDmg <= 0.0f) continue; // Non-melee mob

            // Attack cooldown: 20 ticks (1 second)
            if (currentTick - mob.lastAttackTick < 20) continue;

            // Check proximity to all players
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (!ph || ph->isDead()) continue;
                if (ph->getGameMode() == 1) continue; // Creative immune

                double dx = ph->getPlayerX() - mob.posX;
                double dy = ph->getPlayerY() - mob.posY;
                double dz = ph->getPlayerZ() - mob.posZ;
                double distSq = dx * dx + dy * dy + dz * dz;

                if (distSq < 4.0) { // Within 2 blocks
                    // Apply armor reduction
                    float dmg = atkDmg;
                    int32_t armorVal = ph->getTotalArmorValue();
                    if (armorVal > 0) {
                        dmg = dmg * static_cast<float>(25 - armorVal) / 25.0f;
                    }
                    // Protection enchantment reduction
                    int32_t protMod = ph->getEnchantmentProtectionModifier();
                    if (protMod > 0) {
                        dmg = dmg * static_cast<float>(25 - protMod) / 25.0f;
                    }
                    if (dmg < 0.5f) dmg = 0.5f; // Minimum 0.5 damage

                    ph->applyDamage(dmg);
                    ph->sendUpdateHealth(*conn, ph->getHealth(), ph->getFood(), ph->getSaturation());
                    ph->damageArmor(atkDmg);

                    // ─── Cave Spider poison ─────────────────────────────
                    // Java: EntityCaveSpider.attackEntity() — 7s Poison on Normal
                    if (mob.mobType == 59) {
                        applyPlayerPotionEffect(ph->getPlayerName(), 19, 140, 0); // Poison, 7s, lvl 1
                    }
                    // Hurt animation + sound
                    broadcastEntityEvent(ph->getEntityId(), 2);
                    broadcastSound("game.player.hurt",
                        ph->getPlayerX(), ph->getPlayerY(), ph->getPlayerZ(), 1.0f, 1.0f);

                    // Knockback away from mob
                    double dist = std::sqrt(distSq);
                    if (dist > 0.01) {
                        double kbX = (dx / dist) * 0.4;
                        double kbZ = (dz / dist) * 0.4;
                        ph->sendEntityVelocity(*conn, ph->getEntityId(), kbX, 0.4, kbZ);
                    }

                    // Death check
                    if (ph->getHealth() <= 0.0f) {
                        broadcastEntityEvent(ph->getEntityId(), 3);
                        // Death messages per mob type
                        const char* deathMsg = "was slain";
                        switch (mob.mobType) {
                            case 54: deathMsg = "was slain by Zombie"; break;
                            case 51: deathMsg = "was shot by Skeleton"; break;
                            case 52: deathMsg = "was slain by Spider"; break;
                            case 58: deathMsg = "was slain by Enderman"; break;
                            case 57: deathMsg = "was slain by Zombie Pigman"; break;
                            case 61: deathMsg = "was slain by Blaze"; break;
                            case 59: deathMsg = "was slain by Cave Spider"; break;
                            case 60: deathMsg = "was slain by Silverfish"; break;
                            case 62: deathMsg = "was slain by Magma Cube"; break;
                            case 55: deathMsg = "was slain by Slime"; break;
                        }
                        broadcastChatMessage(ph->getPlayerName() + " " + deathMsg);
                    }

                    mob.lastAttackTick = currentTick;
                    break; // One attack per tick
                }
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
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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
// Ender Dragon AI tick — Java: EntityDragon.onLivingUpdate() + onDeathUpdate()
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::tickDragon(SpawnedMob& dragon, int64_t currentTick) {
    // ─── Death sequence — Java: EntityDragon.onDeathUpdate() ───
    if (dragon.dragonDeathTicks > 0) {
        ++dragon.dragonDeathTicks;

        // Spawn explosion particles during death — Java: onLivingUpdate() when health<=0
        {
            float px = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 8.0f;
            float py = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 4.0f;
            float pz = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 8.0f;
            broadcastParticle("largeexplode",
                static_cast<float>(dragon.posX) + px,
                static_cast<float>(dragon.posY) + 2.0f + py,
                static_cast<float>(dragon.posZ) + pz,
                0.0f, 0.0f, 0.0f, 0.0f, 1);
        }

        // Java: deathTicks >= 180 && deathTicks <= 200 → hugeexplosion particles
        if (dragon.dragonDeathTicks >= 180 && dragon.dragonDeathTicks <= 200) {
            float px = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 8.0f;
            float py = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 4.0f;
            float pz = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 8.0f;
            broadcastParticle("hugeexplosion",
                static_cast<float>(dragon.posX) + px,
                static_cast<float>(dragon.posY) + 2.0f + py,
                static_cast<float>(dragon.posZ) + pz,
                0.0f, 0.0f, 0.0f, 0.0f, 1);
        }

        // Java: deathTicks > 150 && deathTicks % 5 == 0 → spawn 1000 XP per wave
        if (dragon.dragonDeathTicks > 150 && dragon.dragonDeathTicks % 5 == 0) {
            spawnXPOrbs(dragon.posX, dragon.posY + 2.0, dragon.posZ, 1000);
        }

        // Slowly float upward and spin during death — Java: moveEntity(0, 0.1, 0), yaw += 20
        dragon.posY += 0.1;
        dragon.yaw += 20.0f;
        while (dragon.yaw > 180.0f) dragon.yaw -= 360.0f;
        while (dragon.yaw < -180.0f) dragon.yaw += 360.0f;

        // Broadcast death position/rotation
        {
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (!ph) continue;
                ph->sendEntityTeleport(*conn, dragon.entityId,
                    dragon.posX, dragon.posY, dragon.posZ, dragon.yaw, dragon.pitch);
            }
        }

        // Java: deathTicks == 200 → final XP burst (2000) + create portal + setDead()
        if (dragon.dragonDeathTicks >= 200) {
            spawnXPOrbs(dragon.posX, dragon.posY + 2.0, dragon.posZ, 2000);

            // ─── Create End portal — Java: EntityDragon.createEnderPortal() ───
            // Build bedrock + end_portal structure at Y=64
            int portalX = static_cast<int>(std::floor(dragon.posX));
            int portalZ = static_cast<int>(std::floor(dragon.posZ));
            int portalY = 64;  // Java: int n3 = 64
            int radius = 4;    // Java: int n4 = 4

            std::cout << "[Dragon] Creating End portal at (" << portalX << ", " << portalY << ", " << portalZ << ")\n";

            for (int y = portalY - 1; y <= portalY + 32; ++y) {
                for (int x = portalX - radius; x <= portalX + radius; ++x) {
                    for (int z = portalZ - radius; z <= portalZ + radius; ++z) {
                        double dx = x - portalX;
                        double dz = z - portalZ;
                        double distSq = dx * dx + dz * dz;

                        if (distSq > (radius - 0.5) * (radius - 0.5)) continue;

                        if (y < portalY) {
                            // Below portal level — bedrock floor
                            if (distSq <= (radius - 1 - 0.5) * (radius - 1 - 0.5)) {
                                setBlockInWorld(x, y, z, 7, 0);  // bedrock
                            }
                        } else if (y > portalY) {
                            // Above portal level — clear blocks
                            setBlockInWorld(x, y, z, 0, 0);  // air
                        } else {
                            // At portal level (y == portalY)
                            if (distSq > (radius - 1 - 0.5) * (radius - 1 - 0.5)) {
                                setBlockInWorld(x, y, z, 7, 0);  // bedrock rim
                            } else {
                                setBlockInWorld(x, y, z, 119, 0);  // end_portal (119)
                            }
                        }
                    }
                }
            }

            // Central pillar — Java: bedrock column + torches + dragon egg
            setBlockInWorld(portalX, portalY + 0, portalZ, 7, 0);  // bedrock
            setBlockInWorld(portalX, portalY + 1, portalZ, 7, 0);  // bedrock
            setBlockInWorld(portalX, portalY + 2, portalZ, 7, 0);  // bedrock
            setBlockInWorld(portalX - 1, portalY + 2, portalZ, 50, 1);  // torch (east)
            setBlockInWorld(portalX + 1, portalY + 2, portalZ, 50, 2);  // torch (west)
            setBlockInWorld(portalX, portalY + 2, portalZ - 1, 50, 3);  // torch (south)
            setBlockInWorld(portalX, portalY + 2, portalZ + 1, 50, 4);  // torch (north)
            setBlockInWorld(portalX, portalY + 3, portalZ, 7, 0);  // bedrock
            setBlockInWorld(portalX, portalY + 4, portalZ, 122, 0);  // dragon_egg (122)

            // Mark dragon as truly dead
            dragon.isDead = true;

            // Destroy entity for all clients
            {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                std::vector<int32_t> dead = {dragon.entityId};
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (ph) ph->sendDestroyEntities(*conn, dead);
                }
            }

            broadcastChatMessage("§5The Ender Dragon has been defeated!");
            std::cout << "[Dragon] Ender Dragon defeated. Portal and egg placed.\n";
        }
        return;
    }

    // ─── Living dragon AI — Java: EntityDragon.onLivingUpdate() ───

    // --- Wing flap sound — Java: client-side animTime cos() crossing -0.3
    dragon.dragonPrevAnimTime = dragon.dragonAnimTime;

    // --- Ring buffer update — Java: EntityDragon ring buffer tracking
    if (dragon.dragonRingIndex < 0) {
        // Initialize ring buffer on first tick
        for (int i = 0; i < 64; ++i) {
            dragon.dragonRingBuffer[i][0] = static_cast<double>(dragon.yaw);
            dragon.dragonRingBuffer[i][1] = dragon.posY;
            dragon.dragonRingBuffer[i][2] = 0.0;
        }
    }
    dragon.dragonRingIndex = (dragon.dragonRingIndex + 1) & 0x3F;  // Wrap at 64
    dragon.dragonRingBuffer[dragon.dragonRingIndex][0] = static_cast<double>(dragon.yaw);
    dragon.dragonRingBuffer[dragon.dragonRingIndex][1] = dragon.posY;

    // --- Crystal healing — Java: EntityDragon.updateDragonEnderCrystal()
    // (Simplified: heal 1HP per 10 ticks if dragon health < max)
    if (dragon.health < 200.0f && currentTick % 10 == 0) {
        dragon.health = std::min(200.0f, dragon.health + 1.0f);
        // Update health metadata for boss bar — DataWatcher 6 (float health)
        {
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (!ph) continue;
                conn->sendPacket(PacketBuilder::entityMetadataFloat(dragon.entityId, 6, dragon.health));
            }
        }
    }

    // --- Target acquisition — Java: EntityDragon.setNewTarget()
    double dx = dragon.dragonTargetX - dragon.posX;
    double dy = dragon.dragonTargetY - dragon.posY;
    double dz = dragon.dragonTargetZ - dragon.posZ;
    double distToTargetSq = dx * dx + dy * dy + dz * dz;

    // Update target if targeting a player
    if (dragon.dragonTargetEntityId >= 0) {
        bool found = false;
        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (!ph || ph->getEntityId() != dragon.dragonTargetEntityId) continue;
            dragon.dragonTargetX = ph->getPlayerX();
            dragon.dragonTargetZ = ph->getPlayerZ();
            // Java: targetY = player.boundingBox.minY + (0.4 + dist/80 - 1), capped at 10
            double tdx = dragon.dragonTargetX - dragon.posX;
            double tdz = dragon.dragonTargetZ - dragon.posZ;
            double tdist = std::sqrt(tdx * tdx + tdz * tdz);
            double yOffset = 0.4 + tdist / 80.0 - 1.0;
            if (yOffset > 10.0) yOffset = 10.0;
            dragon.dragonTargetY = ph->getPlayerY() + yOffset;
            found = true;
            break;
        }
        if (!found) {
            dragon.dragonTargetEntityId = -1;  // Player disconnected
        }
    } else {
        // Random drift when no target — Java: targetX += gaussian*2, targetZ += gaussian*2
        dragon.dragonTargetX += ((double)(rand() % 200 - 100) / 50.0);
        dragon.dragonTargetZ += ((double)(rand() % 200 - 100) / 50.0);
    }

    // Force new target if too close, too far, or stuck — Java: d < 100 || d > 22500
    if (dragon.dragonForceNewTarget || distToTargetSq < 100.0 || distToTargetSq > 22500.0) {
        dragon.dragonForceNewTarget = false;

        // Java: 50% chance target random player, 50% random point
        if (rand() % 2 == 0) {
            // Target a random connected player
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            std::vector<int32_t> playerIds;
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (ph) playerIds.push_back(ph->getEntityId());
            }
            if (!playerIds.empty()) {
                dragon.dragonTargetEntityId = playerIds[rand() % playerIds.size()];
            }
        } else {
            // Random target near origin — Java: Y=70+rand*50, X/Z=rand*120-60
            dragon.dragonTargetEntityId = -1;
            bool validTarget = false;
            for (int attempt = 0; attempt < 10 && !validTarget; ++attempt) {
                dragon.dragonTargetX = (double)(rand() % 120 - 60);
                dragon.dragonTargetY = 70.0 + (double)(rand() % 50);
                dragon.dragonTargetZ = (double)(rand() % 120 - 60);
                // Java: must be >100 blocks from current pos
                double ntdx = dragon.posX - dragon.dragonTargetX;
                double ntdy = dragon.posY - dragon.dragonTargetY;
                double ntdz = dragon.posZ - dragon.dragonTargetZ;
                if (ntdx * ntdx + ntdy * ntdy + ntdz * ntdz > 100.0) {
                    validTarget = true;
                }
            }
        }
    }

    // --- Flight physics — Java: EntityDragon.onLivingUpdate() server-side path
    // Recompute dx/dy/dz toward target
    dx = dragon.dragonTargetX - dragon.posX;
    dy = dragon.dragonTargetY - dragon.posY;
    dz = dragon.dragonTargetZ - dragon.posZ;
    double distHorizSq = dx * dx + dz * dz;
    double distHoriz = std::sqrt(distHorizSq);

    // Clamp vertical velocity — Java: d3 = dy / sqrt(dx*dx + dz*dz), clamped to [-0.6, 0.6]
    float clampF = 0.6f;
    if (distHoriz > 0.001) {
        double dyNorm = dy / distHoriz;
        if (dyNorm < -clampF) dyNorm = -clampF;
        if (dyNorm > clampF) dyNorm = clampF;
        dragon.dragonMotionY += dyNorm * 0.1;
    }

    // Steer yaw toward target — Java: atan2 + wrapAngleTo180 + clamp ±50
    double targetYaw = 180.0 - std::atan2(dx, dz) * 180.0 / M_PI;
    double yawDiff = targetYaw - static_cast<double>(dragon.yaw);
    // Wrap to [-180, 180]
    while (yawDiff > 180.0) yawDiff -= 360.0;
    while (yawDiff < -180.0) yawDiff += 360.0;
    if (yawDiff > 50.0) yawDiff = 50.0;
    if (yawDiff < -50.0) yawDiff = -50.0;

    // Java: vec3 = target - pos, normalized; vec32 = forward dir from yaw
    double vecLen = std::sqrt(dx * dx + dy * dy + dz * dz);
    double normX = (vecLen > 0.001) ? dx / vecLen : 0.0;
    double normY = (vecLen > 0.001) ? dy / vecLen : 0.0;
    double normZ = (vecLen > 0.001) ? dz / vecLen : 0.0;

    float yawRad = dragon.yaw * static_cast<float>(M_PI) / 180.0f;
    double fwdX = std::sin(yawRad);
    double fwdZ = -std::cos(yawRad);
    double fwdLen = std::sqrt(fwdX * fwdX + dragon.dragonMotionY * dragon.dragonMotionY + fwdZ * fwdZ);
    if (fwdLen > 0.001) {
        fwdX /= fwdLen;
        fwdZ /= fwdLen;
    }

    // Dot product for speed scaling — Java: f5 = (dot + 0.5) / 1.5, clamped >= 0
    double dotProduct = fwdX * normX + (dragon.dragonMotionY / (fwdLen > 0.001 ? fwdLen : 1.0)) * normY + fwdZ * normZ;
    float f5 = static_cast<float>((dotProduct + 0.5) / 1.5);
    if (f5 < 0.0f) f5 = 0.0f;

    // Dampen yaw velocity — Java: randomYawVelocity *= 0.8
    dragon.dragonRandomYawVelocity *= 0.8f;

    // Compute speed factors — Java: f6 = sqrt(mX*mX + mZ*mZ) + 1
    float f6 = static_cast<float>(std::sqrt(dragon.dragonMotionX * dragon.dragonMotionX +
                                             dragon.dragonMotionZ * dragon.dragonMotionZ)) * 1.0f + 1.0f;
    double d12 = std::sqrt(dragon.dragonMotionX * dragon.dragonMotionX +
                            dragon.dragonMotionZ * dragon.dragonMotionZ) * 1.0 + 1.0;
    if (d12 > 40.0) d12 = 40.0;

    // Apply yaw steering — Java: randomYawVelocity += yawDiff * (0.7/d12/f6)
    dragon.dragonRandomYawVelocity += static_cast<float>(yawDiff * (0.7 / d12 / static_cast<double>(f6)));
    dragon.yaw += dragon.dragonRandomYawVelocity * 0.1f;

    // Wrap yaw
    while (dragon.yaw > 180.0f) dragon.yaw -= 360.0f;
    while (dragon.yaw < -180.0f) dragon.yaw += 360.0f;

    // Apply forward thrust — Java: moveFlying(0, -1, f8 * (f5*f7 + (1-f7)))
    float f7 = static_cast<float>(2.0 / (d12 + 1.0));
    float f8 = 0.06f;
    float accel = f8 * (f5 * f7 + (1.0f - f7));
    float yawRadNew = dragon.yaw * static_cast<float>(M_PI) / 180.0f;
    dragon.dragonMotionX += static_cast<double>(-std::sin(yawRadNew) * accel);
    dragon.dragonMotionZ += static_cast<double>(std::cos(yawRadNew) * accel);

    // Apply movement — Java: moveEntity(motionX, motionY, motionZ)
    if (dragon.dragonSlowed) {
        dragon.posX += dragon.dragonMotionX * 0.8;
        dragon.posY += dragon.dragonMotionY * 0.8;
        dragon.posZ += dragon.dragonMotionZ * 0.8;
    } else {
        dragon.posX += dragon.dragonMotionX;
        dragon.posY += dragon.dragonMotionY;
        dragon.posZ += dragon.dragonMotionZ;
    }

    // Apply drag — Java: motionX *= f9, motionZ *= f9, motionY *= 0.91
    // f9 = (dot(velocity_normalized, forward_normalized) + 1) / 2, then 0.8 + 0.15*f9
    double velLen = std::sqrt(dragon.dragonMotionX * dragon.dragonMotionX +
                               dragon.dragonMotionY * dragon.dragonMotionY +
                               dragon.dragonMotionZ * dragon.dragonMotionZ);
    double vnX = (velLen > 0.001) ? dragon.dragonMotionX / velLen : 0.0;
    double vnZ = (velLen > 0.001) ? dragon.dragonMotionZ / velLen : 0.0;

    double dotVel = vnX * fwdX + vnZ * fwdZ;
    float f9 = static_cast<float>((dotVel + 1.0) / 2.0);
    f9 = 0.8f + 0.15f * f9;
    dragon.dragonMotionX *= static_cast<double>(f9);
    dragon.dragonMotionZ *= static_cast<double>(f9);
    dragon.dragonMotionY *= 0.91;

    // Update animation time — Java: animTime += f3 (speed-based)
    float speedFactor = 0.2f / (static_cast<float>(std::sqrt(dragon.dragonMotionX * dragon.dragonMotionX +
                                                              dragon.dragonMotionZ * dragon.dragonMotionZ)) * 10.0f + 1.0f);
    if (dragon.dragonSlowed) {
        dragon.dragonAnimTime += speedFactor * 0.5f;
    } else {
        float powFactor = static_cast<float>(std::pow(2.0, dragon.dragonMotionY));
        dragon.dragonAnimTime += speedFactor * powFactor;
    }

    // --- Wing flap sound — Java: cos(animTime*PI*2) crossing -0.3
    float cosNew = std::cos(dragon.dragonAnimTime * static_cast<float>(M_PI) * 2.0f);
    float cosOld = std::cos(dragon.dragonPrevAnimTime * static_cast<float>(M_PI) * 2.0f);
    if (cosOld <= -0.3f && cosNew >= -0.3f) {
        broadcastSound("mob.enderdragon.wings", dragon.posX, dragon.posY, dragon.posZ,
                        5.0f, 0.8f + (float)(rand() % 30) / 100.0f);
    }

    // --- Block destruction — Java: EntityDragon.destroyBlocksInAABB()
    // Dragon destroys blocks around head and body (except obsidian, end_stone, bedrock)
    dragon.dragonSlowed = false;
    {
        // Head AABB: centered at posX + sin(yaw)*5.5, posY+2, posZ - cos(yaw)*5.5, radius ~3
        float headYaw = dragon.yaw * static_cast<float>(M_PI) / 180.0f;
        double headX = dragon.posX + std::sin(headYaw) * 5.5;
        double headY = dragon.posY + 2.0;
        double headZ = dragon.posZ - std::cos(headYaw) * 5.5;

        auto destroyBlocks = [&](double cx, double cy, double cz, double hw, double hh) -> bool {
            bool hitSolid = false;
            int minX = static_cast<int>(std::floor(cx - hw));
            int maxX = static_cast<int>(std::floor(cx + hw));
            int minY = static_cast<int>(std::floor(cy - hh));
            int maxY = static_cast<int>(std::floor(cy + hh));
            int minZ = static_cast<int>(std::floor(cz - hw));
            int maxZ = static_cast<int>(std::floor(cz + hw));
            bool destroyed = false;

            for (int bx = minX; bx <= maxX; ++bx) {
                for (int by = minY; by <= maxY; ++by) {
                    for (int bz = minZ; bz <= maxZ; ++bz) {
                        int32_t blockId = getBlockIdInWorld(bx, by, bz);
                        if (blockId == 0) continue;  // air
                        // Indestructible: obsidian(49), end_stone(121), bedrock(7)
                        if (blockId == 49 || blockId == 121 || blockId == 7) {
                            hitSolid = true;
                            continue;
                        }
                        // Destroy block
                        setBlockInWorld(bx, by, bz, 0, 0);
                        destroyed = true;
                    }
                }
            }
            if (destroyed) {
                broadcastParticle("largeexplode",
                    static_cast<float>(cx), static_cast<float>(cy), static_cast<float>(cz),
                    0.0f, 0.0f, 0.0f, 0.0f, 1);
            }
            return hitSolid;
        };

        // Destroy blocks for head (3x3) and body (5x3)
        bool headHit = destroyBlocks(headX, headY, headZ, 3.0, 3.0);
        bool bodyHit = destroyBlocks(dragon.posX, dragon.posY + 1.0, dragon.posZ, 5.0, 3.0);
        dragon.dragonSlowed = headHit || bodyHit;
    }

    // --- Player knockback (wing collision) + head attack — Java: collideWithEntities + attackEntitiesInList
    {
        float yawRadKb = dragon.yaw * static_cast<float>(M_PI) / 180.0f;
        double wing1X = dragon.posX + std::cos(yawRadKb) * 4.5;
        double wing1Z = dragon.posZ + std::sin(yawRadKb) * 4.5;
        double wing2X = dragon.posX - std::cos(yawRadKb) * 4.5;
        double wing2Z = dragon.posZ - std::sin(yawRadKb) * 4.5;
        double headX = dragon.posX + std::sin(yawRadKb) * 5.5;
        double headZ = dragon.posZ - std::cos(yawRadKb) * 5.5;

        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (!ph) continue;

            double px = ph->getPlayerX();
            double py = ph->getPlayerY();
            double pz = ph->getPlayerZ();

            // Wing knockback — Java: within 4 blocks of wing, push away from body center
            auto checkWingKnockback = [&](double wx, double wz) {
                double wdx = px - wx;
                double wdz = pz - wz;
                double wdistSq = wdx * wdx + wdz * wdz;
                if (wdistSq < 16.0 && wdistSq > 0.01) {  // Within 4 blocks
                    double bx = px - dragon.posX;
                    double bz = pz - dragon.posZ;
                    double bdistSq = bx * bx + bz * bz;
                    if (bdistSq > 0.01) {
                        double kbX = (bx / bdistSq) * 4.0;
                        double kbZ = (bz / bdistSq) * 4.0;
                        ph->sendEntityVelocity(*conn, ph->getEntityId(), kbX, 0.2, kbZ);
                    }
                }
            };
            checkWingKnockback(wing1X, wing1Z);
            checkWingKnockback(wing2X, wing2Z);

            // Head attack — Java: 10 damage if within 1.5 blocks of head
            double hdx = px - headX;
            double hdy = py - (dragon.posY + 2.0);
            double hdz = pz - headZ;
            double headDistSq = hdx * hdx + hdy * hdy + hdz * hdz;
            if (headDistSq < 2.25 && currentTick - dragon.lastAttackTick >= 20) {  // 1.5^2 = 2.25
                // Java: EntityDragon.attackEntitiesInList — 10.0 damage
                float damageAmount = 10.0f;
                ph->applyDamage(damageAmount);
                // Knockback away from head
                if (headDistSq > 0.01) {
                    double hd = std::sqrt(headDistSq);
                    ph->sendEntityVelocity(*conn, ph->getEntityId(),
                        (hdx / hd) * 0.4, 0.4, (hdz / hd) * 0.4);
                }
                dragon.lastAttackTick = currentTick;

                // Death check
                if (ph->getHealth() <= 0.0f) {
                    broadcastEntityEvent(ph->getEntityId(), 3);
                    broadcastChatMessage(ph->getPlayerName() + " was slain by Ender Dragon");
                }
            }
        }
    }

    // --- Periodic growl — Java: EntityDragon.getLivingSound() = "mob.enderdragon.growl"
    if (currentTick % 200 == 0) {
        broadcastSound("mob.enderdragon.growl", dragon.posX, dragon.posY, dragon.posZ,
                        5.0f, 0.8f + (float)(rand() % 40) / 100.0f);
    }

    // --- Broadcast entity position to all players
    {
        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (!ph) continue;
            ph->sendEntityTeleport(*conn, dragon.entityId,
                dragon.posX, dragon.posY, dragon.posZ, dragon.yaw, dragon.pitch);
        }
    }

    // Update health metadata for boss bar display
    {
        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (!ph) continue;
            conn->sendPacket(PacketBuilder::entityMetadataFloat(dragon.entityId, 6, dragon.health));
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Drop container contents on block break
// Java reference: BlockContainer.breakBlock() → InventoryHelper.dropInventoryItems()
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::dropContainerContents(int32_t x, int32_t y, int32_t z, int32_t blockId) {
    int64_t key = packBlockPos(x, y, z);
    double dx = static_cast<double>(x) + 0.5;
    double dy = static_cast<double>(y) + 0.5;
    double dz = static_cast<double>(z) + 0.5;

    // Helper lambda: drop a single optional ItemStack
    auto dropSlot = [&](const std::optional<ItemStack>& slot) {
        if (slot && slot->getStackSize() > 0) {
            spawnItemDrop(dx, dy, dz, slot->getItemId(), slot->getDamage(), slot->getStackSize());
        }
    };

    switch (blockId) {
        // Chest / Trapped Chest — 27 slots
        case 54:
        case 146: {
            std::lock_guard<std::mutex> lock(chestMutex_);
            auto it = chestStorage_.find(key);
            if (it != chestStorage_.end()) {
                for (auto& slot : it->second) {
                    dropSlot(slot);
                    slot = std::nullopt;
                }
                chestStorage_.erase(it);
            }
            break;
        }
        // Furnace / Lit Furnace — 3 slots (input, fuel, output)
        case 61:
        case 62: {
            std::lock_guard<std::mutex> lock(furnaceMutex_);
            auto it = furnaceStorage_.find(key);
            if (it != furnaceStorage_.end()) {
                for (auto& slot : it->second.slots) {
                    dropSlot(slot);
                    slot = std::nullopt;
                }
                furnaceStorage_.erase(it);
            }
            break;
        }
        // Dispenser — 9 slots
        case 23: {
            std::lock_guard<std::mutex> lock(dispenserMutex_);
            auto it = dispenserStorage_.find(key);
            if (it != dispenserStorage_.end()) {
                for (auto& slot : it->second.slots) {
                    dropSlot(slot);
                    slot = std::nullopt;
                }
                dispenserStorage_.erase(it);
            }
            break;
        }
        // Dropper — 9 slots (same storage as dispenser)
        case 158: {
            std::lock_guard<std::mutex> lock(dispenserMutex_);
            auto it = dispenserStorage_.find(key);
            if (it != dispenserStorage_.end()) {
                for (auto& slot : it->second.slots) {
                    dropSlot(slot);
                    slot = std::nullopt;
                }
                dispenserStorage_.erase(it);
            }
            break;
        }
        // Hopper — 5 slots
        case 154: {
            std::lock_guard<std::mutex> lock(hopperMutex_);
            auto it = hopperStorage_.find(key);
            if (it != hopperStorage_.end()) {
                for (auto& slot : it->second.slots) {
                    dropSlot(slot);
                    slot = std::nullopt;
                }
                hopperStorage_.erase(it);
            }
            break;
        }
        // Brewing Stand — 4 slots (3 potions + 1 ingredient)
        case 117: {
            std::lock_guard<std::mutex> lock(brewingStandMutex_);
            auto it = brewingStandStorage_.find(key);
            if (it != brewingStandStorage_.end()) {
                for (auto& slot : it->second.slots) {
                    dropSlot(slot);
                    slot = std::nullopt;
                }
                brewingStandStorage_.erase(it);
            }
            break;
        }
        default:
            break;
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
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
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

// ═══════════════════════════════════════════════════════════════════════════
// spawnArrow — Create arrow projectile and broadcast S0E SpawnObject
// Java reference: EntityArrow(world, shooter, speed) + EntityTracker.trackEntity()
// ═══════════════════════════════════════════════════════════════════════════
int32_t MinecraftServer::spawnArrow(double x, double y, double z,
                                     double motionX, double motionY, double motionZ,
                                     int32_t shooterEntityId, double damage,
                                     int32_t knockback, bool critical,
                                     float speed, float inaccuracy) {
    int32_t eid = nextArrowEntityId_++;

    // Java: EntityArrow.setThrowableHeading(motionXYZ, speed, inaccuracy)
    // Normalize direction, add Gaussian inaccuracy, scale by speed
    if (speed > 0.001f) {
        double len = std::sqrt(motionX * motionX + motionY * motionY + motionZ * motionZ);
        if (len > 1e-7) {
            motionX /= len;
            motionY /= len;
            motionZ /= len;
        }
        // Java: d += rand.nextGaussian() * (rand.nextBoolean() ? -1 : 1) * 0.0075 * inaccuracy
        static thread_local std::mt19937 arrowRng(std::random_device{}());
        std::normal_distribution<double> gauss(0.0, 1.0);
        motionX += gauss(arrowRng) * ((rand() % 2 == 0) ? -1.0 : 1.0) * 0.0075 * inaccuracy;
        motionY += gauss(arrowRng) * ((rand() % 2 == 0) ? -1.0 : 1.0) * 0.0075 * inaccuracy;
        motionZ += gauss(arrowRng) * ((rand() % 2 == 0) ? -1.0 : 1.0) * 0.0075 * inaccuracy;
        motionX *= speed;
        motionY *= speed;
        motionZ *= speed;
    }

    SpawnedArrow arrow;
    arrow.entityId = eid;
    arrow.shooterEntityId = shooterEntityId;
    arrow.posX = x; arrow.posY = y; arrow.posZ = z;
    arrow.motionX = motionX; arrow.motionY = motionY; arrow.motionZ = motionZ;
    arrow.damage = damage;
    arrow.knockbackStrength = knockback;
    arrow.isCritical = critical;
    arrow.spawnTick = tickCounter_.load();

    // Compute yaw/pitch from motion
    float horizSpeed = static_cast<float>(std::sqrt(motionX*motionX + motionZ*motionZ));
    arrow.yaw = static_cast<float>(std::atan2(motionX, motionZ) * 180.0 / M_PI);
    arrow.pitch = static_cast<float>(std::atan2(motionY, horizSpeed) * 180.0 / M_PI);

    // Broadcast S0E SpawnObject to all players
    // Type 60 = arrow, data = shooter entity ID (>0 means velocity is included)
    {
        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (!ph) continue;
            ph->sendSpawnObject(*conn, eid, 60,
                x, y, z, arrow.yaw, arrow.pitch,
                shooterEntityId > 0 ? shooterEntityId : 1,
                motionX, motionY, motionZ);
        }
    }

    {
        std::lock_guard<std::mutex> lock(arrowEntitiesMutex_);
        arrowEntities_.push_back(arrow);
    }

    return eid;
}

// ═══════════════════════════════════════════════════════════════════════════
// tickArrows — Flight physics, block/player collision, despawn
// Java reference: EntityArrow.onUpdate()
// ═══════════════════════════════════════════════════════════════════════════
void MinecraftServer::tickArrows() {
    std::lock_guard<std::mutex> lock(arrowEntitiesMutex_);

    for (auto& arrow : arrowEntities_) {
        if (arrow.isDead) continue;

        // ─── Ground state: check for despawn + pickup ────────────────
        if (arrow.inGround) {
            ++arrow.ticksInGround;
            if (arrow.ticksInGround >= SpawnedArrow::GROUND_DESPAWN) {
                arrow.isDead = true;
            }
            if (arrow.arrowShake > 0) { --arrow.arrowShake; continue; }

            // Java: EntityArrow.onCollideWithPlayer() — pickup grounded arrows
            // Must be inGround, arrowShake==0, canBePickedUp > 0
            if (arrow.canBePickedUp > 0) {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph || ph->isDead()) continue;

                    double dx = ph->getPlayerX() - arrow.posX;
                    double dy = (ph->getPlayerY() + 0.9) - arrow.posY;
                    double dz = ph->getPlayerZ() - arrow.posZ;
                    double distSq = dx*dx + dy*dy + dz*dz;
                    if (distSq > 1.0) continue; // ~1 block radius

                    // Java: canBePickedUp==1 → survival pickup (add to inventory)
                    // canBePickedUp==2 → creative only (don't add item)
                    bool picked = false;
                    if (arrow.canBePickedUp == 1) {
                        // Try to add arrow item (ID 262) to player inventory
                        picked = ph->tryPickupItem(*conn, 262, 0, 1);
                    } else if (arrow.canBePickedUp == 2 && ph->getGameMode() == 1) {
                        picked = true; // Creative can always pick up
                    }

                    if (picked) {
                        // Java: random.pop, volume=0.2, pitch=((rand-rand)*0.7+1)*2
                        // NOTE: Can't call broadcastSound here (already holds connectionsMutex_)
                        // Instead, inline send to all connected players
                        float randA = static_cast<float>(rand() % 1000) / 1000.0f;
                        float randB = static_cast<float>(rand() % 1000) / 1000.0f;
                        float popPitch = ((randA - randB) * 0.7f + 1.0f) * 2.0f;
                        for (auto& c : connections_) {
                            if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                            auto h = c->getHandler();
                            auto* p = dynamic_cast<PlayHandler*>(h.get());
                            if (p) p->sendSoundEffect(*c, "random.pop", arrow.posX, arrow.posY, arrow.posZ, 0.2f, popPitch);
                        }
                        arrow.isDead = true;
                        break;
                    }
                }
            }
            continue;
        }

        // ─── Flight physics ──────────────────────────────────────────
        ++arrow.ticksInAir;

        double prevX = arrow.posX, prevY = arrow.posY, prevZ = arrow.posZ;

        // Apply motion
        arrow.posX += arrow.motionX;
        arrow.posY += arrow.motionY;
        arrow.posZ += arrow.motionZ;

        // ─── Block collision raytrace ────────────────────────────────
        // Simplified: check block at new position
        if (!worlds_.empty()) {
            int bx = static_cast<int>(std::floor(arrow.posX));
            int by = static_cast<int>(std::floor(arrow.posY));
            int bz = static_cast<int>(std::floor(arrow.posZ));
            int32_t blockId = getBlockIdInWorld(bx, by, bz);
            // Hit a solid block (non-air, non-liquid)
            if (blockId != 0 && blockId != 8 && blockId != 9 &&
                blockId != 10 && blockId != 11) {
                // Embed in block — Java: EntityArrow.onUpdate() block hit
                arrow.inGround = true;
                arrow.blockX = bx; arrow.blockY = by; arrow.blockZ = bz;
                arrow.inBlockId = blockId;
                arrow.inBlockMeta = getBlockMetaInWorld(bx, by, bz);
                arrow.arrowShake = 7;
                arrow.isCritical = false;

                // Back up slightly
                double mag = std::sqrt(arrow.motionX*arrow.motionX +
                                        arrow.motionY*arrow.motionY +
                                        arrow.motionZ*arrow.motionZ);
                if (mag > 1e-7) {
                    arrow.posX -= arrow.motionX / mag * 0.05;
                    arrow.posY -= arrow.motionY / mag * 0.05;
                    arrow.posZ -= arrow.motionZ / mag * 0.05;
                }

                arrow.motionX = 0; arrow.motionY = 0; arrow.motionZ = 0;

                // Broadcast position update + arrow hit sound
                broadcastSound("random.bowhit", arrow.posX, arrow.posY, arrow.posZ, 1.0f, 1.0f);

                // Send S18 for final position
                {
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    for (auto& conn : connections_) {
                        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                        auto handler = conn->getHandler();
                        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                        if (!ph) continue;
                        ph->sendEntityTeleport(*conn, arrow.entityId,
                            arrow.posX, arrow.posY, arrow.posZ, arrow.yaw, arrow.pitch);
                    }
                }
                continue;
            }
        }

        // ─── Mob collision check ─────────────────────────────────────
        // Java: EntityArrow.onUpdate() — getEntitiesWithinAABBExcludingEntity
        // scans ALL entities (mobs + players). We check mobs separately.
        if (!arrow.isDead) {
            std::lock_guard<std::mutex> mobLock(mobEntitiesMutex_);
            for (auto& mob : mobEntities_) {
                if (mob.isDead) continue;

                double dx = mob.posX - arrow.posX;
                double dy = (mob.posY + 1.0) - arrow.posY; // Center of mob
                double dz = mob.posZ - arrow.posZ;
                double distSq = dx * dx + dy * dy + dz * dz;

                // Java: expand(0.3, 0.3, 0.3) — hit radius ~1.5 blocks
                if (distSq > 2.25) continue;

                // ─── Enderman arrow immunity ─────────────────────────
                // Java: EntityEnderman.attackEntityFrom() returns false for arrows
                if (mob.mobType == 58) {
                    // Arrow bounces off — Java: motionX/Y/Z *= -0.1, yaw += 180
                    arrow.motionX *= -0.1;
                    arrow.motionY *= -0.1;
                    arrow.motionZ *= -0.1;
                    arrow.ticksInAir = 0;
                    // Enderman teleports away from arrow
                    for (int tp = 0; tp < 64; ++tp) {
                        double newX = mob.posX + ((double)rand() / RAND_MAX - 0.5) * 64.0;
                        double newY = mob.posY + (double)(rand() % 64 - 32);
                        double newZ = mob.posZ + ((double)rand() / RAND_MAX - 0.5) * 64.0;
                        int bex = static_cast<int>(std::floor(newX));
                        int bey = static_cast<int>(std::floor(newY));
                        int bez = static_cast<int>(std::floor(newZ));
                        if (bey < 1 || bey > 250) continue;
                        while (bey > 1 && getBlockIdInWorld(bex, bey - 1, bez) == 0) --bey;
                        if (getBlockIdInWorld(bex, bey, bez) != 0 || getBlockIdInWorld(bex, bey + 1, bez) != 0) continue;
                        broadcastSound("mob.endermen.portal", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
                        mob.posX = newX;
                        mob.posY = static_cast<double>(bey);
                        mob.posZ = newZ;
                        broadcastSound("mob.endermen.portal", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
                        {
                            std::lock_guard<std::recursive_mutex> cl(connectionsMutex_);
                            for (auto& c : connections_) {
                                if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                                auto h = c->getHandler();
                                auto* p = dynamic_cast<PlayHandler*>(h.get());
                                if (p) p->sendEntityTeleport(*c, mob.entityId, mob.posX, mob.posY, mob.posZ, mob.yaw, 0.0f);
                            }
                        }
                        break;
                    }
                    break; // Arrow doesn't die but bounces
                }

                // ─── Calculate arrow damage — Java: ceil(speed * damage) ───
                double speed = std::sqrt(arrow.motionX * arrow.motionX +
                                          arrow.motionY * arrow.motionY +
                                          arrow.motionZ * arrow.motionZ);
                int32_t dmg = static_cast<int32_t>(std::ceil(speed * arrow.damage));
                if (dmg < 1) dmg = 1;

                // Critical bonus — Java: rand.nextInt(damage/2 + 2)
                if (arrow.isCritical) {
                    dmg += rand() % (dmg / 2 + 2);
                }

                // Apply damage to mob
                mob.health -= static_cast<float>(dmg);

                // Hurt animation — Java: EntityLivingBase.attackEntityFrom → S1A status 2
                broadcastEntityEvent(mob.entityId, 2);

                // Hurt sound — Java: getHurtSound()
                {
                    const char* hurtSound = "game.hostile.hurt";
                    switch (mob.mobType) {
                        case 50: hurtSound = "mob.creeper.say"; break;
                        case 51: hurtSound = "mob.skeleton.hurt"; break;
                        case 52: hurtSound = "mob.spider.say"; break;
                        case 54: hurtSound = "mob.zombie.hurt"; break;
                        case 55: hurtSound = "mob.slime.small"; break;
                        case 56: hurtSound = "mob.ghast.scream"; break;
                        case 57: hurtSound = "mob.zombiepig.zpighurt"; break;
                        case 59: hurtSound = "mob.spider.say"; break;
                        case 60: hurtSound = "mob.silverfish.hit"; break;
                        case 61: hurtSound = "mob.blaze.hit"; break;
                        case 62: hurtSound = "mob.magmacube.small"; break;
                        case 66: hurtSound = "mob.witch.hurt"; break;
                        case 92: hurtSound = "mob.cow.hurt"; break;
                        case 90: hurtSound = "mob.pig.say"; break;
                        case 91: hurtSound = "mob.sheep.say"; break;
                        case 93: hurtSound = "mob.chicken.hurt"; break;
                        case 95: hurtSound = "mob.wolf.hurt"; break;
                        case 98: hurtSound = "mob.cat.hitt"; break;
                        case 99: hurtSound = "mob.irongolem.hit"; break;
                        case 100: {
                            int ht = mob.horseType;
                            if (ht == 3) hurtSound = "mob.horse.zombie.hit";
                            else if (ht == 4) hurtSound = "mob.horse.skeleton.hit";
                            else if (ht == 1 || ht == 2) hurtSound = "mob.horse.donkey.hit";
                            else hurtSound = "mob.horse.hit";
                            break;
                        }
                        default: break;
                    }
                    broadcastSound(hurtSound, mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
                }

                // Knockback — Java: EntityArrow.onUpdate() → addVelocity
                // knockbackStrength * 0.6 / horizDist
                if (arrow.knockbackStrength > 0) {
                    double horizDist = std::sqrt(arrow.motionX * arrow.motionX +
                                                  arrow.motionZ * arrow.motionZ);
                    if (horizDist > 0.01) {
                        double kbX = arrow.motionX * arrow.knockbackStrength * 0.6 / horizDist;
                        double kbZ = arrow.motionZ * arrow.knockbackStrength * 0.6 / horizDist;
                        mob.posX += kbX;
                        mob.posZ += kbZ;
                    }
                }

                // Flame enchantment — Java: if (isBurning()) target.setFire(5)
                if (arrow.isBurning) {
                    mob.isOnFire = true;
                }

                // Arrow hit sound — Java: random.bowhit, 1.0f, 1.2f / (rand*0.2+0.9)
                float hitPitch = 1.2f / (static_cast<float>(rand() % 1000) / 1000.0f * 0.2f + 0.9f);
                broadcastSound("random.bowhit", arrow.posX, arrow.posY, arrow.posZ, 1.0f, hitPitch);

                // ─── Death check ─────────────────────────────────────
                if (mob.health <= 0.0f) {
                    mob.isDead = true;

                    // Death animation
                    broadcastEntityEvent(mob.entityId, 3);

                    // Death sound — Java: getDeathSound()
                    {
                        const char* deathSound = "game.hostile.die";
                        switch (mob.mobType) {
                            case 50: deathSound = "mob.creeper.death"; break;
                            case 51: deathSound = "mob.skeleton.death"; break;
                            case 52: deathSound = "mob.spider.death"; break;
                            case 54: deathSound = "mob.zombie.death"; break;
                            case 55: deathSound = "mob.slime.big"; break;
                            case 56: deathSound = "mob.ghast.death"; break;
                            case 57: deathSound = "mob.zombiepig.zpigdeath"; break;
                            case 59: deathSound = "mob.spider.death"; break;
                            case 60: deathSound = "mob.silverfish.kill"; break;
                            case 61: deathSound = "mob.blaze.death"; break;
                            case 62: deathSound = "mob.magmacube.big"; break;
                            case 66: deathSound = "mob.witch.death"; break;
                            case 92: deathSound = "mob.cow.hurt"; break;
                            case 90: deathSound = "mob.pig.death"; break;
                            case 91: deathSound = "mob.sheep.say"; break;
                            case 93: deathSound = "mob.chicken.hurt"; break;
                            case 95: deathSound = "mob.wolf.death"; break;
                            case 98: deathSound = "mob.cat.hitt"; break;
                            case 99: deathSound = "mob.irongolem.death"; break;
                            case 100: {
                                int ht = mob.horseType;
                                if (ht == 3) deathSound = "mob.horse.zombie.death";
                                else if (ht == 4) deathSound = "mob.horse.skeleton.death";
                                else if (ht == 1 || ht == 2) deathSound = "mob.horse.donkey.death";
                                else deathSound = "mob.horse.death";
                                break;
                            }
                            default: break;
                        }
                        broadcastSound(deathSound, mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
                    }

                    // Destroy entity
                    {
                        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                        std::vector<int32_t> dead = {mob.entityId};
                        for (auto& c : connections_) {
                            if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                            auto h = c->getHandler();
                            auto* ph = dynamic_cast<PlayHandler*>(h.get());
                            if (ph) ph->sendDestroyEntities(*c, dead);
                        }
                    }

                    // ─── Mob drops + XP — reuse same logic as handlePlayerAttack ───
                    // Looting from shooter's held item
                    int32_t lootingLevel = 0;
                    if (arrow.shooterEntityId >= 0) {
                        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                        for (auto& conn : connections_) {
                            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                            auto handler = conn->getHandler();
                            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                            if (ph && ph->getEntityId() == arrow.shooterEntityId) {
                                auto held = ph->getHeldItem();
                                if (held && held->hasEnchantments()) {
                                    lootingLevel = held->getEnchantmentLevel(21);
                                }
                                break;
                            }
                        }
                    }

                    // XP orbs — Java: EntityLiving.getExperiencePoints()
                    auto getMobXp = [](uint8_t mt) -> int32_t {
                        switch (mt) {
                            case 54: case 51: case 50: case 52: case 58:
                            case 66: case 60: case 59: case 57: return 5;
                            case 61: return 10;
                            case 56: return 5;
                            case 62: case 55: return 1;
                            case 92: case 90: case 91: case 93: case 94:
                                return 1 + (rand() % 3);
                            case 99: return 0;
                            default: return 5;
                        }
                    };
                    int32_t killXp = getMobXp(mob.mobType);
                    if (killXp > 0) {
                        spawnXPOrbs(mob.posX, mob.posY + 0.5, mob.posZ, killXp);
                    }

                    // Mob item drops — Java: dropFewItems()
                    int32_t baseCount = 0, dropId = 0, dropMeta = 0;
                    switch (mob.mobType) {
                        case 54: dropId = 367; baseCount = 1 + (rand() % 2); break;
                        case 51:
                            dropId = 352; baseCount = 1 + (rand() % 2);
                            if (rand() % 2 == 0)
                                spawnItemDrop(mob.posX, mob.posY, mob.posZ, 262, 0, 1 + (rand() % 2) + lootingLevel);
                            break;
                        case 50: dropId = 289; baseCount = rand() % 2; break;
                        case 52:
                            dropId = 287; baseCount = 1 + (rand() % 2);
                            if (rand() % 3 == 0)
                                spawnItemDrop(mob.posX, mob.posY, mob.posZ, 375, 0, 1);
                            break;
                        case 58: dropId = 368; baseCount = rand() % 2; break;
                        case 66: dropId = 331; baseCount = 1 + (rand() % 3); break;
                        case 61: dropId = 369; baseCount = rand() % 2; break;
                        case 56:
                            dropId = 370; baseCount = rand() % 2;
                            spawnItemDrop(mob.posX, mob.posY, mob.posZ, 289, 0, 1 + (rand() % 2));
                            break;
                        case 55: dropId = 341; baseCount = 1; break;
                        case 57:
                            dropId = 367; baseCount = 1;
                            if (rand() % 2 == 0)
                                spawnItemDrop(mob.posX, mob.posY, mob.posZ, 371, 0, 1 + (rand() % 2));
                            break;
                        case 59:
                            dropId = 287; baseCount = 1;
                            if (rand() % 3 == 0)
                                spawnItemDrop(mob.posX, mob.posY, mob.posZ, 375, 0, 1);
                            break;
                        case 60: break; // Silverfish → nothing
                        case 62: dropId = 378; baseCount = rand() % 2; break;
                        case 92:
                            dropId = 334; baseCount = 1 + (rand() % 2);
                            spawnItemDrop(mob.posX, mob.posY, mob.posZ,
                                mob.isOnFire ? 364 : 363, 0, 1 + (rand() % 3));
                            break;
                        case 90:
                            dropId = mob.isOnFire ? 320 : 319;
                            baseCount = 1 + (rand() % 3);
                            if (mob.isSaddled)
                                spawnItemDrop(mob.posX, mob.posY, mob.posZ, 329, 0, 1);
                            break;
                        case 91:
                            if (!mob.isSheared) {
                                dropId = 35; baseCount = 1; dropMeta = mob.fleeceColor;
                            }
                            break;
                        case 93:
                            dropId = 288; baseCount = 1 + (rand() % 2);
                            spawnItemDrop(mob.posX, mob.posY, mob.posZ,
                                mob.isOnFire ? 366 : 365, 0, 1);
                            break;
                        case 94: dropId = 351; baseCount = 1 + (rand() % 3); break;
                        case 99:
                            dropId = 265; baseCount = 3 + (rand() % 3);
                            spawnItemDrop(mob.posX, mob.posY, mob.posZ, 38, 0, 1 + (rand() % 2));
                            break;
                        default: break;
                    }
                    if (dropId > 0) {
                        int32_t totalCount = baseCount + lootingLevel;
                        if (totalCount > 0)
                            spawnItemDrop(mob.posX, mob.posY, mob.posZ, dropId, dropMeta, totalCount);
                    }
                }

                // Arrow dies on mob hit
                arrow.isDead = true;
                break;
            }
        }
        // ─── Player collision check ──────────────────────────────────
        // Java: scan for entities in expanded AABB along motion vector
        if (!arrow.isDead) {
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (!ph || ph->isDead()) continue;
                if (ph->getGameMode() == 1 || ph->getGameMode() == 3) continue;

                // Skip shooter for first 5 ticks
                if (arrow.ticksInAir < SpawnedArrow::SHOOTER_GRACE &&
                    ph->getEntityId() == arrow.shooterEntityId) continue;

                double dx = ph->getPlayerX() - arrow.posX;
                double dy = (ph->getPlayerY() + 0.9) - arrow.posY; // center of player
                double dz = ph->getPlayerZ() - arrow.posZ;
                double distSq = dx*dx + dy*dy + dz*dz;

                // Hit radius ~0.5 blocks (expand 0.3 per Java)
                if (distSq > 1.0) continue;

                // ─── Calculate arrow damage ──────────────────────────
                // Java: ceil(speed * baseDamage)
                double speed = std::sqrt(arrow.motionX*arrow.motionX +
                                          arrow.motionY*arrow.motionY +
                                          arrow.motionZ*arrow.motionZ);
                int32_t dmg = static_cast<int32_t>(std::ceil(speed * arrow.damage));
                if (dmg < 1) dmg = 1;

                // Critical bonus — rand(damage/2 + 2)
                if (arrow.isCritical) {
                    dmg += rand() % (dmg / 2 + 2);
                }

                // Armor reduction — Java: EntityLivingBase.applyArmorCalculations
                float actualDmg = static_cast<float>(dmg);
                int32_t armorVal = ph->getTotalArmorValue();
                if (armorVal > 0) {
                    actualDmg = actualDmg * static_cast<float>(25 - armorVal) / 25.0f;
                }
                // Projectile Protection — Java: damageType=4
                int32_t protMod = ph->getEnchantmentProtectionModifier(4);
                if (protMod > 0) {
                    actualDmg = actualDmg * (1.0f - std::min(protMod, 20) * 0.04f);
                }
                if (actualDmg < 0.5f) actualDmg = 0.5f;

                ph->applyDamage(actualDmg);
                ph->sendUpdateHealth(*conn, ph->getHealth(), ph->getFood(), ph->getSaturation());
                ph->damageArmor(static_cast<float>(dmg));

                // Hurt animation + sound
                broadcastEntityEvent(ph->getEntityId(), 2);
                broadcastSound("game.player.hurt",
                    ph->getPlayerX(), ph->getPlayerY(), ph->getPlayerZ(), 1.0f, 1.0f);

                // Knockback — Java: EntityArrow.onUpdate() → kb from motion + punch enchant
                double horizDist = std::sqrt(arrow.motionX*arrow.motionX + arrow.motionZ*arrow.motionZ);
                if (horizDist > 0.01) {
                    double kbMult = 0.4;
                    // Punch enchantment — Java: knockbackStrength > 0 → multiply
                    if (arrow.knockbackStrength > 0) {
                        kbMult *= (1.0 + arrow.knockbackStrength * 0.5);
                    }
                    double kbX = arrow.motionX / horizDist * kbMult;
                    double kbZ = arrow.motionZ / horizDist * kbMult;
                    ph->sendEntityVelocity(*conn, ph->getEntityId(), kbX, 0.36, kbZ);
                }

                // Flame enchantment — Java: if (isBurning()) target.setFire(5) (5 seconds = 100 ticks)
                if (arrow.isBurning) {
                    ph->setOnFire(100);
                }

                // Death check
                if (ph->getHealth() <= 0.0f) {
                    broadcastEntityEvent(ph->getEntityId(), 3);
                    // Try to find shooter name for death message
                    std::string deathMsg = ph->getPlayerName() + " was shot by arrow";
                    for (auto& conn2 : connections_) {
                        if (!conn2->isConnected() || conn2->getState() != ConnectionState::Play) continue;
                        auto handler2 = conn2->getHandler();
                        auto* shooter = dynamic_cast<PlayHandler*>(handler2.get());
                        if (shooter && shooter->getEntityId() == arrow.shooterEntityId) {
                            deathMsg = ph->getPlayerName() + " was shot by " + shooter->getPlayerName();
                            break;
                        }
                    }
                    broadcastChatMessage(deathMsg);
                }

                // Arrow dies on player hit
                arrow.isDead = true;
                break;
            }
        }

        // ─── Update rotation from motion ─────────────────────────────
        float horizSpeed = static_cast<float>(std::sqrt(arrow.motionX*arrow.motionX +
                                                          arrow.motionZ*arrow.motionZ));
        arrow.yaw = static_cast<float>(std::atan2(arrow.motionX, arrow.motionZ) * 180.0 / M_PI);
        arrow.pitch = static_cast<float>(std::atan2(arrow.motionY, horizSpeed) * 180.0 / M_PI);

        // Apply friction + gravity — Java: EntityArrow.onUpdate()
        arrow.motionX *= SpawnedArrow::AIR_FRICTION;
        arrow.motionY *= SpawnedArrow::AIR_FRICTION;
        arrow.motionZ *= SpawnedArrow::AIR_FRICTION;
        arrow.motionY -= SpawnedArrow::GRAVITY;

        // Broadcast S18 EntityTeleport every tick for smooth flight
        {
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (!ph) continue;
                ph->sendEntityTeleport(*conn, arrow.entityId,
                    arrow.posX, arrow.posY, arrow.posZ, arrow.yaw, arrow.pitch);
            }
        }

        // Max air time — despawn after 1200 ticks if still flying
        if (arrow.ticksInAir >= 1200) {
            arrow.isDead = true;
        }
    }

    // ─── Remove dead arrows + broadcast S13 DestroyEntities ──────────
    std::vector<int32_t> deadIds;
    arrowEntities_.erase(
        std::remove_if(arrowEntities_.begin(), arrowEntities_.end(),
            [&deadIds](const SpawnedArrow& a) {
                if (a.isDead) {
                    deadIds.push_back(a.entityId);
                    return true;
                }
                return false;
            }),
        arrowEntities_.end());

    if (!deadIds.empty()) {
        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (!ph) continue;
            ph->sendDestroyEntities(*conn, deadIds);
        }
    }
}

// ─── Throwable projectile spawning ──────────────────────────────────
// Java: EntityThrowable(world, thrower) + setThrowableHeading(motionXYZ, speed=1.5, inaccuracy=1.0)
int32_t MinecraftServer::spawnThrowable(ThrowableType type, double x, double y, double z,
                                         double motionX, double motionY, double motionZ,
                                         int32_t throwerEntityId, const std::string& throwerName) {
    int32_t eid = nextThrowableEntityId_.fetch_add(1, std::memory_order_relaxed);
    int64_t currentTick = tickCount_.load(std::memory_order_relaxed);

    SpawnedThrowable t;
    t.entityId = eid;
    t.throwerEntityId = throwerEntityId;
    t.throwerName = throwerName;
    t.type = type;
    t.posX = x; t.posY = y; t.posZ = z;

    // Normalize and scale velocity — Java: setThrowableHeading(mx, my, mz, 1.5f, 1.0f)
    double len = std::sqrt(motionX * motionX + motionY * motionY + motionZ * motionZ);
    if (len > 0.001) {
        motionX /= len; motionY /= len; motionZ /= len;
    }
    // Add slight inaccuracy — Java: + rand.nextGaussian() * 0.0075 * inaccuracy
    static thread_local std::mt19937 rng(std::random_device{}());
    std::normal_distribution<double> gauss(0.0, 0.0075);
    motionX += gauss(rng);
    motionY += gauss(rng);
    motionZ += gauss(rng);
    // Scale by speed (1.5 for standard throwables)
    double speed = 1.5;
    t.motionX = motionX * speed;
    t.motionY = motionY * speed;
    t.motionZ = motionZ * speed;
    t.isDead = false;
    t.ticksInAir = 0;
    t.spawnTick = currentTick;

    // Determine SpawnObject type ID for protocol
    // Java: S0E SpawnObject — type 61=snowball, 62=egg, 65=ender pearl, 75=exp bottle, 73=splash potion
    uint8_t objectType = 0;
    switch (type) {
        case ThrowableType::Snowball:    objectType = 61; break;
        case ThrowableType::Egg:         objectType = 62; break;
        case ThrowableType::EnderPearl:  objectType = 65; break;
        case ThrowableType::ExpBottle:   objectType = 75; break;
        case ThrowableType::SplashPotion: objectType = 73; break;
    }

    // Broadcast S0E SpawnObject to all players
    {
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (ph) {
                ph->sendSpawnObject(*conn, eid, objectType, x, y, z,
                                    0.0f, 0.0f,
                                    throwerEntityId > 0 ? throwerEntityId : 0,
                                    t.motionX, t.motionY, t.motionZ);
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(throwableEntitiesMutex_);
        throwableEntities_.push_back(std::move(t));
    }
    return eid;
}

// ─── Throwable projectile ticking ──────────────────────────────────
// Java: EntityThrowable.onUpdate() — gravity, friction, collision, impact
void MinecraftServer::tickThrowables() {
    std::vector<int32_t> deadIds;

    {
        std::lock_guard<std::mutex> lock(throwableEntitiesMutex_);
        for (auto& t : throwableEntities_) {
            if (t.isDead) continue;

            ++t.ticksInAir;
            if (t.ticksInAir > SpawnedThrowable::MAX_TICKS) {
                t.isDead = true;
                deadIds.push_back(t.entityId);
                continue;
            }

            // Apply motion
            double newX = t.posX + t.motionX;
            double newY = t.posY + t.motionY;
            double newZ = t.posZ + t.motionZ;

            // ─── Block collision check ──────────────────────────────
            // Java: world.rayTraceBlocks(pos, pos+motion)
            if (!worlds_.empty()) {
                auto* wld = worlds_[0].get();
                int bx = static_cast<int>(std::floor(newX));
                int by = static_cast<int>(std::floor(newY));
                int bz = static_cast<int>(std::floor(newZ));
                Block* blockHit = wld->getBlock(bx, by, bz);
                if (blockHit != nullptr) {
                    // Hit a block — trigger impact
                    t.isDead = true;
                    deadIds.push_back(t.entityId);

                    // Impact effects based on type
                    switch (t.type) {
                        case ThrowableType::Snowball:
                            broadcastSound("random.bow", t.posX, t.posY, t.posZ, 0.5f, 0.4f);
                            break;
                        case ThrowableType::Egg:
                            // Java: 1/8 chance spawn chicken, 1/32 chance spawn 4
                            if ((rand() % 8) == 0) {
                                int count = ((rand() % 32) == 0) ? 4 : 1;
                                for (int i = 0; i < count; ++i) {
                                    summonMob(93, t.posX, t.posY + 1.0, t.posZ); // chicken
                                }
                            }
                            break;
                        case ThrowableType::EnderPearl: {
                            // Teleport thrower to impact point
                            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                            for (auto& conn : connections_) {
                                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                                auto handler = conn->getHandler();
                                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                                if (!ph) continue;
                                if (ph->getPlayerName() == t.throwerName) {
                                    // Teleport player — Java: setPositionAndUpdate
                                    ph->setPlayerPosition(t.posX, t.posY + 1.0, t.posZ);
                                    ph->sendPlayerPosAndLook(*conn, t.posX, t.posY + 1.0, t.posZ, ph->getPlayerYaw(), ph->getPlayerPitch());
                                    // 5 fall damage — Java: attackEntityFrom(DamageSource.fall, 5.0f)
                                    ph->applyDamage(5.0f);
                                    ph->sendUpdateHealth(*conn, ph->getHealth(), ph->getFood(), ph->getSaturation());
                                    broadcastSound("mob.endermen.portal", t.posX, t.posY, t.posZ, 1.0f, 1.0f);
                                    if (ph->getHealth() <= 0.0f) {
                                        broadcastEntityEvent(ph->getEntityId(), 3);
                                        broadcastChatMessage(ph->getPlayerName() + " hit the ground too hard");
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                        case ThrowableType::ExpBottle: {
                            // Java: 3-11 XP — spawns orbs at impact position
                            int xp = 3 + (rand() % 9);
                            spawnXPOrbs(t.posX, t.posY, t.posZ, xp);
                            broadcastSound("random.glass", t.posX, t.posY, t.posZ, 1.0f, 1.0f);
                            break;
                        }
                        case ThrowableType::SplashPotion:
                            broadcastSound("game.potion.smash", t.posX, t.posY, t.posZ, 1.0f, 1.0f);
                            break;
                    }
                    continue;
                }
            }

            // ─── Entity (player) collision check ────────────────────
            // Java: EntityThrowable.onUpdate() — check entity AABB
            {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph || ph->isDead()) continue;
                    if (ph->getEntityId() == t.throwerEntityId && t.ticksInAir < 5) continue;

                    double dx = ph->getPlayerX() - newX;
                    double dy = (ph->getPlayerY() + 0.9) - newY; // Center of player
                    double dz = ph->getPlayerZ() - newZ;
                    double distSq = dx * dx + dy * dy + dz * dz;

                    if (distSq < 1.0) { // Hit radius ~1 block
                        t.isDead = true;
                        deadIds.push_back(t.entityId);

                        switch (t.type) {
                            case ThrowableType::Snowball: {
                                // Java: 0 damage to entities (3 to blazes, handled via mob collision)
                                broadcastEntityEvent(ph->getEntityId(), 2);
                                broadcastSound("game.player.hurt", ph->getPlayerX(), ph->getPlayerY(), ph->getPlayerZ(), 1.0f, 1.0f);
                                break;
                            }
                            case ThrowableType::Egg: {
                                // Java: 0 damage, knockback
                                broadcastEntityEvent(ph->getEntityId(), 2);
                                break;
                            }
                            case ThrowableType::EnderPearl: {
                                // Teleport thrower to impact location (not the hit player)
                                for (auto& conn2 : connections_) {
                                    if (!conn2->isConnected() || conn2->getState() != ConnectionState::Play) continue;
                                    auto handler2 = conn2->getHandler();
                                    auto* thrower = dynamic_cast<PlayHandler*>(handler2.get());
                                    if (!thrower) continue;
                                    if (thrower->getPlayerName() == t.throwerName) {
                                        thrower->setPlayerPosition(newX, newY, newZ);
                                        thrower->sendPlayerPosAndLook(*conn2, newX, newY, newZ, thrower->getPlayerYaw(), thrower->getPlayerPitch());
                                        thrower->applyDamage(5.0f);
                                        thrower->sendUpdateHealth(*conn2, thrower->getHealth(), thrower->getFood(), thrower->getSaturation());
                                        broadcastSound("mob.endermen.portal", newX, newY, newZ, 1.0f, 1.0f);
                                        break;
                                    }
                                }
                                break;
                            }
                            default:
                                break;
                        }
                        break; // Only hit one player
                    }
                }
            }
            if (t.isDead) continue;

            // ─── Mob collision check ─────────────────────────────────────
        // Java: EntityThrowable.onUpdate() → onImpact(MovingObjectPosition)
        // Snowball: 0 dmg to all, 3 dmg to Blaze (EntitySnowball.onImpact)
        // Egg: 0 dmg (EntityEgg.onImpact), chicken spawn on any impact
        // EnderPearl: teleport thrower to impact point
        {
            std::lock_guard<std::mutex> mobLock(mobEntitiesMutex_);
            for (auto& mob : mobEntities_) {
                if (mob.isDead) continue;
                double dx = mob.posX - newX;
                double dy = (mob.posY + 1.0) - newY;
                double dz = mob.posZ - newZ;
                if (dx * dx + dy * dy + dz * dz > 1.5) continue;

                t.isDead = true;
                deadIds.push_back(t.entityId);

                switch (t.type) {
                    case ThrowableType::Snowball: {
                        // Java: EntitySnowball.onImpact — 0 dmg, 3 to Blaze
                        int32_t dmg = (mob.mobType == 61) ? 3 : 0;
                        mob.health -= static_cast<float>(dmg);

                        // Hurt animation for all mobs (Java: attackEntityFrom triggers even at 0)
                        broadcastEntityEvent(mob.entityId, 2);

                        // Hurt sound
                        const char* hurtSound = "game.hostile.hurt";
                        switch (mob.mobType) {
                            case 50: hurtSound = "mob.creeper.say"; break;
                            case 51: hurtSound = "mob.skeleton.hurt"; break;
                            case 52: hurtSound = "mob.spider.say"; break;
                            case 54: hurtSound = "mob.zombie.hurt"; break;
                            case 55: hurtSound = "mob.slime.small"; break;
                            case 56: hurtSound = "mob.ghast.scream"; break;
                            case 57: hurtSound = "mob.zombiepig.zpighurt"; break;
                            case 59: hurtSound = "mob.spider.say"; break;
                            case 60: hurtSound = "mob.silverfish.hit"; break;
                            case 61: hurtSound = "mob.blaze.hit"; break;
                            case 62: hurtSound = "mob.magmacube.small"; break;
                            case 66: hurtSound = "mob.witch.hurt"; break;
                            case 92: hurtSound = "mob.cow.hurt"; break;
                            case 90: hurtSound = "mob.pig.say"; break;
                            case 91: hurtSound = "mob.sheep.say"; break;
                            case 93: hurtSound = "mob.chicken.hurt"; break;
                            case 95: hurtSound = "mob.wolf.hurt"; break;
                            case 98: hurtSound = "mob.cat.hitt"; break;
                            case 99: hurtSound = "mob.irongolem.hit"; break;
                            default: break;
                        }
                        broadcastSound(hurtSound, mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);

                        // Blaze death check
                        if (mob.health <= 0.0f && mob.mobType == 61) {
                            mob.isDead = true;
                            broadcastEntityEvent(mob.entityId, 3);
                            broadcastSound("mob.blaze.death", mob.posX, mob.posY, mob.posZ, 1.0f, 1.0f);
                            {
                                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                                std::vector<int32_t> dead = {mob.entityId};
                                for (auto& c : connections_) {
                                    if (!c->isConnected() || c->getState() != ConnectionState::Play) continue;
                                    auto h = c->getHandler();
                                    auto* ph = dynamic_cast<PlayHandler*>(h.get());
                                    if (ph) ph->sendDestroyEntities(*c, dead);
                                }
                            }
                            // Blaze drops: blaze rod (369), XP 10
                            int32_t lootingLevel = 0;
                            if (t.throwerEntityId >= 0) {
                                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                                for (auto& conn : connections_) {
                                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                                    auto handler = conn->getHandler();
                                    auto* ph2 = dynamic_cast<PlayHandler*>(handler.get());
                                    if (ph2 && ph2->getEntityId() == t.throwerEntityId) {
                                        auto held = ph2->getHeldItem();
                                        if (held && held->hasEnchantments())
                                            lootingLevel = held->getEnchantmentLevel(21);
                                        break;
                                    }
                                }
                            }
                            int32_t rodCount = (rand() % 2) + lootingLevel;
                            if (rodCount > 0) spawnItemDrop(mob.posX, mob.posY, mob.posZ, 369, 0, rodCount);
                            spawnXPOrbs(mob.posX, mob.posY + 0.5, mob.posZ, 10);
                        }
                        break;
                    }
                    case ThrowableType::Egg: {
                        // Java: EntityEgg.onImpact — 0 damage, triggers knockback
                        broadcastEntityEvent(mob.entityId, 2);
                        // Chicken spawn on any egg impact (not mob-specific)
                        if ((rand() % 8) == 0) {
                            int count = ((rand() % 32) == 0) ? 4 : 1;
                            for (int i = 0; i < count; ++i) {
                                summonMob(93, t.posX, t.posY + 1.0, t.posZ);
                            }
                        }
                        break;
                    }
                    case ThrowableType::EnderPearl: {
                        // Teleport thrower to mob hit position
                        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                        for (auto& conn : connections_) {
                            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                            auto handler = conn->getHandler();
                            auto* thrower = dynamic_cast<PlayHandler*>(handler.get());
                            if (!thrower) continue;
                            if (thrower->getPlayerName() == t.throwerName) {
                                thrower->setPlayerPosition(newX, newY, newZ);
                                thrower->sendPlayerPosAndLook(*conn, newX, newY, newZ, thrower->getPlayerYaw(), thrower->getPlayerPitch());
                                thrower->applyDamage(5.0f);
                                thrower->sendUpdateHealth(*conn, thrower->getHealth(), thrower->getFood(), thrower->getSaturation());
                                broadcastSound("mob.endermen.portal", newX, newY, newZ, 1.0f, 1.0f);
                                if (thrower->getHealth() <= 0.0f) {
                                    broadcastEntityEvent(thrower->getEntityId(), 3);
                                    broadcastChatMessage(thrower->getPlayerName() + " hit the ground too hard");
                                }
                                break;
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
                break; // Only hit one mob
            }
        }
            if (t.isDead) continue;

            // Update position
            t.posX = newX;
            t.posY = newY;
            t.posZ = newZ;

            // Apply physics — Java: EntityThrowable.onUpdate()
            t.motionX *= SpawnedThrowable::AIR_FRICTION;
            t.motionY *= SpawnedThrowable::AIR_FRICTION;
            t.motionZ *= SpawnedThrowable::AIR_FRICTION;
            t.motionY -= SpawnedThrowable::GRAVITY;

            // Broadcast position update
            {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph) continue;
                    ph->sendEntityTeleport(*conn, t.entityId, t.posX, t.posY, t.posZ, 0.0f, 0.0f);
                }
            }
        }

        // Remove dead throwables
        throwableEntities_.erase(
            std::remove_if(throwableEntities_.begin(), throwableEntities_.end(),
                [](const SpawnedThrowable& t) { return t.isDead; }),
            throwableEntities_.end());
    }

    // Broadcast destroy for dead throwables
    if (!deadIds.empty()) {
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (!ph) continue;
            ph->sendDestroyEntities(*conn, deadIds);
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
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
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

                // ─── Cactus growth (81) — Java: BlockCactus.updateTick ───────────
                // Cactus uses meta 0-15 as growth counter, grows up when meta >= 15
                // Max height: 3 blocks
                if (blockId == 81) {
                    Block* above = world->getBlock(bx, by + 1, bz);
                    int aboveId = above ? Block::getIdFromBlock(above) : 0;
                    if (aboveId == 0 && by < 255) {
                        // Count cactus stack height below
                        int height = 1;
                        while (height < 3) {
                            Block* below = world->getBlock(bx, by - height, bz);
                            if (!below || Block::getIdFromBlock(below) != 81) break;
                            ++height;
                        }
                        if (height < 3) {
                            int cactusMeta = world->getBlockMetadata(bx, by, bz);
                            if (cactusMeta >= 15) {
                                world->setBlock(bx, by + 1, bz, Block::getBlockById(81));
                                world->setBlockMetadata(bx, by + 1, bz, 0);
                                broadcastBlockChange(bx, by + 1, bz, 81, 0);
                                world->setBlockMetadata(bx, by, bz, 0);
                                broadcastBlockChange(bx, by, bz, 81, 0);
                            } else {
                                world->setBlockMetadata(bx, by, bz, cactusMeta + 1);
                            }
                        }
                    }
                }

                // ─── Sugar cane growth (83) — Java: BlockReed.updateTick ─────────
                // Sugar cane grows upward, max height 3, meta 0-15 counter
                if (blockId == 83) {
                    Block* above = world->getBlock(bx, by + 1, bz);
                    int aboveId = above ? Block::getIdFromBlock(above) : 0;
                    if (aboveId == 0 && by < 255) {
                        int height = 1;
                        while (height < 3) {
                            Block* below = world->getBlock(bx, by - height, bz);
                            if (!below || Block::getIdFromBlock(below) != 83) break;
                            ++height;
                        }
                        if (height < 3) {
                            int reedMeta = world->getBlockMetadata(bx, by, bz);
                            if (reedMeta >= 15) {
                                world->setBlock(bx, by + 1, bz, Block::getBlockById(83));
                                world->setBlockMetadata(bx, by + 1, bz, 0);
                                broadcastBlockChange(bx, by + 1, bz, 83, 0);
                                world->setBlockMetadata(bx, by, bz, 0);
                                broadcastBlockChange(bx, by, bz, 83, 0);
                            } else {
                                world->setBlockMetadata(bx, by, bz, reedMeta + 1);
                            }
                        }
                    }
                }
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Fish hook entity spawning — Java: EntityFishHook(World, EntityPlayer)
// ═══════════════════════════════════════════════════════════════════════════

int32_t MinecraftServer::spawnFishHook(double x, double y, double z,
                                        double motionX, double motionY, double motionZ,
                                        int32_t anglerEntityId) {
    // Remove any existing hook owned by this angler
    {
        std::lock_guard<std::mutex> lock(fishHookEntitiesMutex_);
        for (auto& h : fishHookEntities_) {
            if (h.anglerEntityId == anglerEntityId && !h.isDead) {
                h.isDead = true;
            }
        }
    }

    int32_t eid = nextFishHookEntityId_.fetch_add(1, std::memory_order_relaxed);
    int64_t currentTick = tickCount_.load(std::memory_order_relaxed);

    SpawnedFishHook hook;
    hook.entityId = eid;
    hook.anglerEntityId = anglerEntityId;
    hook.posX = x; hook.posY = y; hook.posZ = z;

    // Java: handleHookCasting(motionX, motionY, motionZ, 1.5f, 1.0f)
    // Normalize, add Gaussian inaccuracy, scale by 1.5
    double len = std::sqrt(motionX * motionX + motionY * motionY + motionZ * motionZ);
    if (len > 0.001) {
        motionX /= len; motionY /= len; motionZ /= len;
    }
    static thread_local std::mt19937 rng(std::random_device{}());
    std::normal_distribution<double> gauss(0.0, 0.0075);  // Java: 0.0075 * inaccuracy(1.0)
    motionX += gauss(rng);
    motionY += gauss(rng);
    motionZ += gauss(rng);
    double speed = 1.5;  // Java: handleHookCasting speed parameter
    hook.motionX = motionX * speed;
    hook.motionY = motionY * speed;
    hook.motionZ = motionZ * speed;

    // Calculate yaw/pitch from motion
    double hLen = std::sqrt(hook.motionX * hook.motionX + hook.motionZ * hook.motionZ);
    hook.yaw = static_cast<float>(std::atan2(hook.motionX, hook.motionZ) * 180.0 / M_PI);
    hook.pitch = static_cast<float>(std::atan2(hook.motionY, hLen) * 180.0 / M_PI);

    hook.isDead = false;
    hook.inGround = false;
    hook.ticksInGround = 0;
    hook.ticksInAir = 0;
    hook.ticksCaughtDelay = 0;
    hook.ticksCatchableDelay = 0;
    hook.ticksCatchable = 0;
    hook.fishApproachAngle = 0.0f;
    hook.spawnTick = currentTick;

    // Broadcast S0E SpawnObject — type 90 = fishing float, data = angler entity ID
    // Java: S0E objectType=90 for EntityFishHook, data=angler.getEntityId()
    {
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (ph) {
                ph->sendSpawnObject(*conn, eid, 90, x, y, z,
                                    hook.pitch, hook.yaw,
                                    anglerEntityId,
                                    hook.motionX, hook.motionY, hook.motionZ);
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(fishHookEntitiesMutex_);
        fishHookEntities_.push_back(std::move(hook));
    }
    return eid;
}

// ─── Fish hook ticking ──────────────────────────────────────────────────
// Java: EntityFishHook.onUpdate() — physics, water detection, 3-phase catch cycle
void MinecraftServer::tickFishHooks() {
    std::vector<int32_t> deadIds;

    {
        std::lock_guard<std::mutex> lock(fishHookEntitiesMutex_);
        if (fishHookEntities_.empty()) return;

        auto& worlds = getWorlds();
        if (worlds.empty()) return;
        WorldServer* world = worlds[0].get();

        static thread_local std::mt19937 rng(std::random_device{}());

        for (auto& h : fishHookEntities_) {
            if (h.isDead) continue;

            // Check if angler still exists and holds fishing rod
            // Java: if (angler.isDead || getCurrentEquippedItem != fishing_rod || dist > 1024) setDead
            bool anglerValid = false;
            {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (ph && ph->getEntityId() == h.anglerEntityId) {
                        auto held = ph->getHeldItem();
                        if (held && held->getItemId() == 346) {
                            double dx = ph->getPlayerX() - h.posX;
                            double dy = ph->getPlayerY() - h.posY;
                            double dz = ph->getPlayerZ() - h.posZ;
                            if (dx * dx + dy * dy + dz * dz <= 1024.0) {
                                anglerValid = true;
                            }
                        }
                        break;
                    }
                }
            }
            if (!anglerValid) {
                h.isDead = true;
                deadIds.push_back(h.entityId);
                continue;
            }

            // In-ground logic
            if (h.inGround) {
                ++h.ticksInGround;
                if (h.ticksInGround >= SpawnedFishHook::GROUND_DESPAWN) {
                    h.isDead = true;
                    deadIds.push_back(h.entityId);
                }
                continue;
            }

            ++h.ticksInAir;

            // Simple block collision check
            int32_t bx = static_cast<int32_t>(std::floor(h.posX + h.motionX));
            int32_t by = static_cast<int32_t>(std::floor(h.posY + h.motionY));
            int32_t bz = static_cast<int32_t>(std::floor(h.posZ + h.motionZ));
            Block* hitBlock = world->getBlock(bx, by, bz);
            if (hitBlock) {
                int32_t blockId = Block::getIdFromBlock(hitBlock);
                // Skip water (8,9) and air (0)
                if (blockId != 0 && blockId != 8 && blockId != 9) {
                    h.inGround = true;
                    h.ticksInGround = 0;
                    continue;
                }
            }

            // Apply motion
            h.posX += h.motionX;
            h.posY += h.motionY;
            h.posZ += h.motionZ;

            // Water submersion check — Java: sample 5 vertical slices of bounding box
            // Simplified: check if the block at hook position is water
            int32_t hookBlockX = static_cast<int32_t>(std::floor(h.posX));
            int32_t hookBlockY = static_cast<int32_t>(std::floor(h.posY));
            int32_t hookBlockZ = static_cast<int32_t>(std::floor(h.posZ));
            Block* hookBlock = world->getBlock(hookBlockX, hookBlockY, hookBlockZ);
            int32_t hookBlockId = hookBlock ? Block::getIdFromBlock(hookBlock) : 0;
            bool inWater = (hookBlockId == 8 || hookBlockId == 9);

            // Also check block one below for partial submersion
            Block* belowBlock = world->getBlock(hookBlockX, hookBlockY - 1, hookBlockZ);
            int32_t belowBlockId = belowBlock ? Block::getIdFromBlock(belowBlock) : 0;
            double waterSubmersion = 0.0;
            if (inWater) {
                waterSubmersion = 1.0;
            } else if (belowBlockId == 8 || belowBlockId == 9) {
                waterSubmersion = 0.5;
            }

            // ─── 3-phase catch cycle (only when in water) ──────────────
            // Java: EntityFishHook.onUpdate() lines 224-293
            if (waterSubmersion > 0.0) {
                // Speed modifier for rain/sky — Java: lines 226-232
                int32_t speedMod = 1;
                std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
                if (dist01(rng) < 0.25f && isRaining()) {
                    speedMod = 2;
                }
                // Java: if can't see sky, --speedMod (not implemented, keep 1)

                if (h.ticksCatchable > 0) {
                    // Phase 3: Bite window active — decrement
                    --h.ticksCatchable;
                    if (h.ticksCatchable <= 0) {
                        h.ticksCaughtDelay = 0;
                        h.ticksCatchableDelay = 0;
                    }
                } else if (h.ticksCatchableDelay > 0) {
                    // Phase 2: Fish approaching — decrement
                    h.ticksCatchableDelay -= speedMod;
                    if (h.ticksCatchableDelay <= 0) {
                        // Fish bites! — Java: motionY -= 0.2, play splash, set catchable window
                        h.motionY -= 0.2;
                        // Play splash sound
                        broadcastSound("random.splash", h.posX, h.posY, h.posZ,
                                       0.25f, 1.0f + (dist01(rng) - dist01(rng)) * 0.4f);
                        // Spawn bubble and wake particles
                        broadcastParticle("bubble", static_cast<float>(h.posX),
                                         static_cast<float>(hookBlockY) + 1.0f,
                                         static_cast<float>(h.posZ),
                                         0.25f, 0.0f, 0.25f, 0.2f, 6);
                        broadcastParticle("wake", static_cast<float>(h.posX),
                                         static_cast<float>(hookBlockY) + 1.0f,
                                         static_cast<float>(h.posZ),
                                         0.25f, 0.0f, 0.25f, 0.2f, 6);
                        // Java: ticksCatchable = MathHelper.getRandomIntegerInRange(rand, 10, 30)
                        std::uniform_int_distribution<int32_t> catchDist(10, 30);
                        h.ticksCatchable = catchDist(rng);
                    } else {
                        // Fish approaching — update approach angle, spawn particles
                        std::normal_distribution<double> angleDist(0.0, 4.0);
                        h.fishApproachAngle += static_cast<float>(angleDist(rng));
                        float angleRad = h.fishApproachAngle * static_cast<float>(M_PI) / 180.0f;
                        float sinA = std::sin(angleRad);
                        float cosA = std::cos(angleRad);
                        double particleX = h.posX + static_cast<double>(sinA * static_cast<float>(h.ticksCatchableDelay) * 0.1f);
                        double particleY = static_cast<double>(hookBlockY) + 1.0;
                        double particleZ = h.posZ + static_cast<double>(cosA * static_cast<float>(h.ticksCatchableDelay) * 0.1f);
                        // Bubble approaching
                        if (dist01(rng) < 0.15f) {
                            broadcastParticle("bubble",
                                             static_cast<float>(particleX),
                                             static_cast<float>(particleY) - 0.1f,
                                             static_cast<float>(particleZ),
                                             sinA, 0.1f, cosA, 0.0f, 1);
                        }
                        // Wake particles
                        float wakeF7 = sinA * 0.04f;
                        float wakeF8 = cosA * 0.04f;
                        broadcastParticle("wake",
                                         static_cast<float>(particleX),
                                         static_cast<float>(particleY),
                                         static_cast<float>(particleZ),
                                         wakeF8, 0.01f, -wakeF7, 1.0f, 0);
                        broadcastParticle("wake",
                                         static_cast<float>(particleX),
                                         static_cast<float>(particleY),
                                         static_cast<float>(particleZ),
                                         -wakeF8, 0.01f, wakeF7, 1.0f, 0);
                    }
                } else if (h.ticksCaughtDelay > 0) {
                    // Phase 1: Initial wait — decrement, spawn random splash particles
                    h.ticksCaughtDelay -= speedMod;
                    float splashChance = 0.15f;
                    if (h.ticksCaughtDelay < 20) {
                        splashChance += static_cast<float>(20 - h.ticksCaughtDelay) * 0.05f;
                    } else if (h.ticksCaughtDelay < 40) {
                        splashChance += static_cast<float>(40 - h.ticksCaughtDelay) * 0.02f;
                    } else if (h.ticksCaughtDelay < 60) {
                        splashChance += static_cast<float>(60 - h.ticksCaughtDelay) * 0.01f;
                    }
                    if (dist01(rng) < splashChance) {
                        // Random splash particle
                        std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
                        std::uniform_real_distribution<float> radDist(25.0f, 60.0f);
                        float angle = angleDist(rng) * static_cast<float>(M_PI) / 180.0f;
                        float radius = radDist(rng);
                        double splashX = h.posX + static_cast<double>(std::sin(angle) * radius * 0.1f);
                        double splashY = static_cast<double>(hookBlockY) + 1.0;
                        double splashZ = h.posZ + static_cast<double>(std::cos(angle) * radius * 0.1f);
                        std::uniform_int_distribution<int32_t> splashCount(2, 3);
                        broadcastParticle("splash",
                                         static_cast<float>(splashX),
                                         static_cast<float>(splashY),
                                         static_cast<float>(splashZ),
                                         0.1f, 0.0f, 0.1f, 0.0f, splashCount(rng));
                    }
                    if (h.ticksCaughtDelay <= 0) {
                        // Transition to phase 2
                        std::uniform_real_distribution<float> angleDist2(0.0f, 360.0f);
                        h.fishApproachAngle = angleDist2(rng);
                        std::uniform_int_distribution<int32_t> approachDist(20, 80);
                        h.ticksCatchableDelay = approachDist(rng);
                    }
                } else {
                    // No phase active — start phase 1
                    // Java: ticksCaughtDelay = rand(100,900) - Lure*20*5
                    std::uniform_int_distribution<int32_t> waitDist(100, 900);
                    h.ticksCaughtDelay = waitDist(rng);
                    // Lure enchantment reduction applied in retractFishHook via angler lookup
                    // For simplicity, we apply Lure here if we can find it
                    {
                        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                        for (auto& conn : connections_) {
                            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                            auto handler = conn->getHandler();
                            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                            if (ph && ph->getEntityId() == h.anglerEntityId) {
                                auto held = ph->getHeldItem();
                                if (held && held->hasEnchantments()) {
                                    int32_t lureLevel = held->getEnchantmentLevel(62);  // Lure
                                    h.ticksCaughtDelay -= lureLevel * 20 * 5;
                                    if (h.ticksCaughtDelay < 1) h.ticksCaughtDelay = 1;
                                }
                                break;
                            }
                        }
                    }
                }

                // Java: if ticksCatchable > 0, add random downward bobbing
                if (h.ticksCatchable > 0) {
                    std::uniform_real_distribution<float> dist00(0.0f, 1.0f);
                    float bob = dist00(rng) * dist00(rng) * dist00(rng);
                    h.motionY -= static_cast<double>(bob) * 0.2;
                }
            }

            // Buoyancy — Java: motionY += 0.04 * (waterSubmersion * 2.0 - 1.0)
            double buoyancyFactor = waterSubmersion * 2.0 - 1.0;
            h.motionY += 0.04 * buoyancyFactor;

            // Friction
            float friction = 0.92f;  // Java: default friction
            if (waterSubmersion > 0.0) {
                friction = static_cast<float>(static_cast<double>(friction) * 0.9);
                h.motionY *= 0.8;
            }
            h.motionX *= static_cast<double>(friction);
            h.motionY *= static_cast<double>(friction);
            h.motionZ *= static_cast<double>(friction);

            // Update yaw/pitch for broadcast
            double hLen = std::sqrt(h.motionX * h.motionX + h.motionZ * h.motionZ);
            h.yaw = static_cast<float>(std::atan2(h.motionX, h.motionZ) * 180.0 / M_PI);
            h.pitch = static_cast<float>(std::atan2(h.motionY, hLen) * 180.0 / M_PI);
        }

        // Remove dead hooks
        fishHookEntities_.erase(
            std::remove_if(fishHookEntities_.begin(), fishHookEntities_.end(),
                [](const SpawnedFishHook& h) { return h.isDead; }),
            fishHookEntities_.end());
    }

    // Broadcast S13 DestroyEntities for dead hooks
    if (!deadIds.empty()) {
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (ph) {
                ph->sendDestroyEntities(*conn, deadIds);
            }
        }
    }

    // Broadcast position updates via S18 EntityTeleport
    {
        std::lock_guard<std::mutex> lock(fishHookEntitiesMutex_);
        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
        for (auto& h : fishHookEntities_) {
            if (h.isDead) continue;
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (ph) {
                    ph->sendEntityTeleport(*conn, h.entityId,
                                           h.posX, h.posY, h.posZ,
                                           h.yaw, h.pitch);
                }
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Fish hook retraction — Java: EntityFishHook.handleHookRetraction()
// Returns: -1 = no hook found, 0+ = durability damage for fishing rod
// ═══════════════════════════════════════════════════════════════════════════
int32_t MinecraftServer::retractFishHook(int32_t anglerEntityId) {
    std::lock_guard<std::mutex> lock(fishHookEntitiesMutex_);

    // Find the angler's active hook
    SpawnedFishHook* hook = nullptr;
    for (auto& h : fishHookEntities_) {
        if (h.anglerEntityId == anglerEntityId && !h.isDead) {
            hook = &h;
            break;
        }
    }
    if (!hook) return -1; // No active hook

    int32_t damage = 0;

    // Java: if (ticksCatchable > 0) → catch a fish/junk/valuable
    if (hook->ticksCatchable > 0) {
        // ─── Loot table generation — Java: EntityFishHook.func_146033_f() ───
        // Determine loot category based on random roll + enchantments
        static thread_local std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

        // Get Luck of the Sea (enchant 61) and Lure (enchant 62) levels from fishing rod
        int32_t luckLevel = 0;
        int32_t lureLevel = 0;
        {
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (ph && ph->getEntityId() == anglerEntityId) {
                    auto held = ph->getHeldItem();
                    if (held && held->hasEnchantments()) {
                        luckLevel = held->getEnchantmentLevel(61); // Luck of the Sea
                        lureLevel = held->getEnchantmentLevel(62); // Lure
                    }
                    break;
                }
            }
        }

        // Java: junkChance = 0.1 - luck*0.025 - lure*0.01
        // Java: treasureChance = 0.05 + luck*0.01 - lure*0.01
        float junkChance = std::max(0.0f, std::min(1.0f, 0.1f - static_cast<float>(luckLevel) * 0.025f - static_cast<float>(lureLevel) * 0.01f));
        float treasureChance = std::max(0.0f, std::min(1.0f, 0.05f + static_cast<float>(luckLevel) * 0.01f - static_cast<float>(lureLevel) * 0.01f));

        float roll = dist01(rng);

        int32_t lootItemId = 349;  // default: raw fish (cod)
        int32_t lootDamage = 0;
        int32_t lootCount = 1;

        if (roll < junkChance) {
            // ─── JUNK ──────────────────────────────────────────────────────
            // Java: WeightedRandomFishable JUNK list, total weight = 83
            // leather_boots(10), leather(10), bone(10), potionitem(10), string(5),
            // fishing_rod(2), bowl(10), stick(5), dye:0(1), tripwire_hook(10), rotten_flesh(10)
            std::uniform_int_distribution<int32_t> junkRoll(0, 82);
            int32_t j = junkRoll(rng);
            if (j < 10)      { lootItemId = 301; lootDamage = 0; }  // leather_boots (damaged)
            else if (j < 20) { lootItemId = 334; lootDamage = 0; }  // leather
            else if (j < 30) { lootItemId = 352; lootDamage = 0; }  // bone
            else if (j < 40) { lootItemId = 373; lootDamage = 0; }  // water bottle (potion)
            else if (j < 45) { lootItemId = 287; lootDamage = 0; }  // string
            else if (j < 47) { lootItemId = 346; lootDamage = 0; }  // fishing_rod (damaged)
            else if (j < 57) { lootItemId = 281; lootDamage = 0; }  // bowl
            else if (j < 62) { lootItemId = 280; lootDamage = 0; }  // stick
            else if (j < 63) { lootItemId = 351; lootDamage = 0; lootCount = 10; }  // ink sac x10
            else if (j < 73) { lootItemId = 131; lootDamage = 0; }  // tripwire_hook
            else              { lootItemId = 367; lootDamage = 0; }  // rotten_flesh

            std::cout << "[Fish] Caught junk (item " << lootItemId << ")\n";
        } else if (roll < junkChance + treasureChance) {
            // ─── VALUABLES (treasure) ──────────────────────────────────────
            // Java: WeightedRandomFishable VALUABLES list, total weight = 6
            // waterlily(1), name_tag(1), saddle(1), bow(1), fishing_rod(1), book(1)
            std::uniform_int_distribution<int32_t> treasureRoll(0, 5);
            int32_t t = treasureRoll(rng);
            switch (t) {
                case 0: lootItemId = 111; break;  // lily pad
                case 1: lootItemId = 421; break;  // name tag
                case 2: lootItemId = 329; break;  // saddle
                case 3: lootItemId = 261; break;  // bow (possibly enchanted)
                case 4: lootItemId = 346; break;  // fishing rod (possibly enchanted)
                case 5: lootItemId = 340; break;  // book (possibly enchanted)
            }
            std::cout << "[Fish] Caught treasure! (item " << lootItemId << ")\n";
        } else {
            // ─── FISH ──────────────────────────────────────────────────────
            // Java: WeightedRandomFishable FISH list, total weight = 100
            // cod(60), salmon(25), clownfish(2), pufferfish(13)
            std::uniform_int_distribution<int32_t> fishRoll(0, 99);
            int32_t f = fishRoll(rng);
            lootItemId = 349;  // Items.fish
            if (f < 60)       { lootDamage = 0; }  // cod
            else if (f < 85)  { lootDamage = 1; }  // salmon
            else if (f < 87)  { lootDamage = 2; }  // clownfish
            else              { lootDamage = 3; }  // pufferfish

            std::cout << "[Fish] Caught fish (damage variant " << lootDamage << ")\n";
        }

        // Spawn the caught item as a dropped item flying towards the angler
        // Java: entityItem.motionX = dx*0.1, motionY = dy*0.1 + sqrt(dist)*0.08, motionZ = dz*0.1
        spawnItemDrop(hook->posX, hook->posY, hook->posZ,
                      lootItemId, lootDamage, lootCount);

        // Award XP — Java: spawnEntityInWorld(new EntityXPOrb(..., rand.nextInt(6) + 1))
        {
            std::uniform_int_distribution<int32_t> xpDist(1, 6);
            int32_t xp = xpDist(rng);
            spawnXPOrbs(hook->posX, hook->posY, hook->posZ, xp);
        }

        // Play splash sound for the catch
        broadcastSound("random.splash", hook->posX, hook->posY, hook->posZ,
                       0.25f, 1.0f);

        damage = 1;
    }

    // Java: if (inGround) damage = 2
    if (hook->inGround) {
        damage = 2;
    }

    // Mark hook as dead and broadcast destroy
    hook->isDead = true;
    std::vector<int32_t> destroyIds = {hook->entityId};
    {
        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (ph) {
                ph->sendDestroyEntities(*conn, destroyIds);
            }
        }
    }

    return damage;
}

// ═══════════════════════════════════════════════════════════════════════════
// Minecart entity spawning — Java: ItemMinecart.onItemUse → new EntityMinecart
// ═══════════════════════════════════════════════════════════════════════════

int32_t MinecraftServer::spawnMinecart(int32_t type, double x, double y, double z) {
    int32_t eid = nextMinecartEntityId_.fetch_add(1);

    SpawnedMinecart cart;
    cart.entityId = eid;
    cart.minecartType = type;
    cart.isDead = false;
    cart.riderEntityId = -1;
    cart.logic.entityId = eid;
    cart.logic.posX = x;
    cart.logic.posY = y;
    cart.logic.posZ = z;
    cart.logic.prevPosX = x;
    cart.logic.prevPosY = y;
    cart.logic.prevPosZ = z;
    cart.logic.minecartType = type;
    cart.lastSentPosX = static_cast<int32_t>(x * 32.0);
    cart.lastSentPosY = static_cast<int32_t>(y * 32.0);
    cart.lastSentPosZ = static_cast<int32_t>(z * 32.0);

    {
        std::lock_guard<std::mutex> lock(minecartEntitiesMutex_);
        minecartEntities_.push_back(std::move(cart));
    }

    // Broadcast S0E SpawnObject — type 10 = minecart, data = minecart subtype
    // Java: EntityTrackerEntry.func_151260_c → S0EPacketSpawnObject(entity, 10, EntityMinecart.minecartType)
    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (ph) {
            ph->sendSpawnObject(*conn, eid, 10, x, y, z, 0.0f, 0.0f, type, 0.0, 0.0, 0.0);
        }
    }

    std::cout << "[Minecart] Spawned type " << type << " entity " << eid
              << " at " << x << "," << y << "," << z << "\n";
    return eid;
}

// ═══════════════════════════════════════════════════════════════════════════
// Minecart entity ticking — Java: EntityMinecart.onUpdate
// Rail physics, gravity, movement broadcast, rider sync, cleanup
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::tickMinecarts() {
    std::lock_guard<std::mutex> lock(minecartEntitiesMutex_);
    if (minecartEntities_.empty()) return;

    auto* world = worlds_.empty() ? nullptr : worlds_[0].get();
    if (!world) return;

    std::vector<int32_t> destroyIds;

    for (auto& cart : minecartEntities_) {
        if (cart.isDead) continue;

        auto& logic = cart.logic;
        auto result = logic.onUpdate();

        if (result.shouldDie || logic.isDead) {
            cart.isDead = true;
            destroyIds.push_back(cart.entityId);
            continue;
        }

        // Check block underneath for rail
        int32_t bx = static_cast<int32_t>(std::floor(logic.posX));
        int32_t by = static_cast<int32_t>(std::floor(logic.posY));
        int32_t bz = static_cast<int32_t>(std::floor(logic.posZ));

        Block* blockBelow = world->getBlock(bx, by, bz);
        int32_t blockId = blockBelow ? Block::getIdFromBlock(blockBelow) : 0;

        bool onRail = (blockId == 66 || blockId == 27 || blockId == 28 || blockId == 157);

        if (onRail) {
            int32_t railMeta = world->getBlockMetadata(bx, by, bz);
            bool isPoweredRail = (blockId == 27);
            bool isPowered = isPoweredRail && (railMeta & 8);
            bool hasRider = (cart.riderEntityId >= 0);

            // Get rider yaw/forward if present
            float riderYaw = 0;
            double riderForward = 0;
            if (hasRider) {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (ph && ph->getEntityId() == cart.riderEntityId) {
                        riderYaw = ph->getPlayerYaw();
                        // Use C0C SteerVehicle input from player
                        riderForward = static_cast<double>(ph->getMoveForward());
                        break;
                    }
                }
            }

            logic.onRailTick(bx, by, bz, EntityMinecart::MAX_SPEED,
                             EntityMinecart::SLOPE_ACCEL, railMeta,
                             isPoweredRail, isPowered, hasRider, riderYaw, riderForward);
        } else {
            // Apply gravity + off-rail movement
            logic.offRailTick(EntityMinecart::MAX_SPEED);
            // Simple ground check
            Block* groundBlock = world->getBlock(bx, by - 1, bz);
            logic.onGround = (groundBlock != nullptr && Block::getIdFromBlock(groundBlock) != 0);
        }

        // Move entity
        logic.posX += logic.motionX;
        logic.posY += logic.motionY;
        logic.posZ += logic.motionZ;

        // Update yaw from motion
        logic.updateYawFromMotion();
        logic.prevRotationYaw = logic.rotationYaw;

        // Broadcast S18 EntityTeleport
        int32_t newPX = static_cast<int32_t>(std::floor(logic.posX * 32.0));
        int32_t newPY = static_cast<int32_t>(std::floor(logic.posY * 32.0));
        int32_t newPZ = static_cast<int32_t>(std::floor(logic.posZ * 32.0));

        bool moved = (newPX != cart.lastSentPosX || newPY != cart.lastSentPosY || newPZ != cart.lastSentPosZ);
        if (moved) {
            cart.lastSentPosX = newPX;
            cart.lastSentPosY = newPY;
            cart.lastSentPosZ = newPZ;

            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (ph) {
                    ph->sendEntityTeleport(*conn, cart.entityId,
                        logic.posX, logic.posY, logic.posZ,
                        logic.rotationYaw, logic.rotationPitch);
                }
            }
        }
    }

    // Broadcast destroy for dead minecarts
    if (!destroyIds.empty()) {
        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (ph) ph->sendDestroyEntities(*conn, destroyIds);
        }
    }

    // Remove dead minecarts
    minecartEntities_.erase(
        std::remove_if(minecartEntities_.begin(), minecartEntities_.end(),
            [](const SpawnedMinecart& c) { return c.isDead; }),
        minecartEntities_.end());
}

// ═══════════════════════════════════════════════════════════════════════════
// Lightning bolt spawning — Java: WorldServer.addWeatherEffect
// ═══════════════════════════════════════════════════════════════════════════

int32_t MinecraftServer::spawnLightning(double x, double y, double z) {
    int32_t eid = nextLightningEntityId_.fetch_add(1);

    SpawnedLightning bolt;
    bolt.entityId = eid;
    bolt.isDead = false;
    bolt.logic.entityId = eid;
    bolt.logic.spawn(x, y, z, rand(), 1 + (rand() % 3));

    {
        std::lock_guard<std::mutex> lock(lightningEntitiesMutex_);
        lightningEntities_.push_back(std::move(bolt));
    }

    // Broadcast S2C SpawnGlobalEntity — type 1 = lightning
    {
        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (ph) {
                ph->sendSpawnGlobalEntity(*conn, eid, 1, x, y, z);
            }
        }
    }

    // Thunder + explosion sounds — Java: EntityLightningBolt constructor
    broadcastSound("ambient.weather.thunder", x, y, z,
        EntityLightningBolt::THUNDER_VOLUME,
        0.8f + static_cast<float>(rand() % 200) / 1000.0f);
    broadcastSound("random.explode", x, y, z,
        EntityLightningBolt::EXPLODE_VOLUME,
        0.5f + static_cast<float>(rand() % 200) / 1000.0f);

    // Fire at impact — Java: setFire at posX/posY/posZ if air + doFireTick
    if (!worlds_.empty()) {
        auto* world = worlds_[0].get();
        int32_t fx = static_cast<int32_t>(std::floor(x));
        int32_t fy = static_cast<int32_t>(std::floor(y));
        int32_t fz = static_cast<int32_t>(std::floor(z));

        Block* atPos = world->getBlock(fx, fy, fz);
        if (!atPos || Block::getIdFromBlock(atPos) == 0) {
            world->setBlock(fx, fy, fz, Block::getBlockById(51)); // fire
            world->setBlockMetadata(fx, fy, fz, 0);
            broadcastBlockChange(fx, fy, fz, 51, 0);
        }

        // 4 random nearby fires — Java: ±1 block XYZ
        for (int i = 0; i < EntityLightningBolt::INITIAL_FIRE_ATTEMPTS; ++i) {
            int32_t rx = fx + (rand() % 3) - 1;
            int32_t ry = fy + (rand() % 3) - 1;
            int32_t rz = fz + (rand() % 3) - 1;
            Block* nearBlock = world->getBlock(rx, ry, rz);
            if (!nearBlock || Block::getIdFromBlock(nearBlock) == 0) {
                // Check below has solid block
                Block* belowBlock = world->getBlock(rx, ry - 1, rz);
                if (belowBlock && Block::getIdFromBlock(belowBlock) != 0) {
                    world->setBlock(rx, ry, rz, Block::getBlockById(51));
                    world->setBlockMetadata(rx, ry, rz, 0);
                    broadcastBlockChange(rx, ry, rz, 51, 0);
                }
            }
        }
    }

    std::cout << "[Lightning] Spawned entity " << eid << " at " << x << "," << y << "," << z << "\n";
    return eid;
}

// ═══════════════════════════════════════════════════════════════════════════
// Lightning bolt ticking — Java: EntityLightningBolt.onUpdate
// Handles re-strike flashes, entity damage, and despawning
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::tickLightning() {
    std::lock_guard<std::mutex> lock(lightningEntitiesMutex_);
    if (lightningEntities_.empty()) return;

    std::vector<int32_t> destroyIds;

    for (auto& bolt : lightningEntities_) {
        if (bolt.isDead) continue;

        int32_t randRetrigger = rand() % 10;
        int64_t randNewVertex = static_cast<int64_t>(rand()) << 32 | rand();

        auto result = bolt.logic.onUpdate(randRetrigger, randNewVertex);

        if (result.shouldDie || bolt.logic.isDead) {
            bolt.isDead = true;
            destroyIds.push_back(bolt.entityId);
            continue;
        }

        // Re-strike thunder
        if (result.playThunder) {
            broadcastSound("ambient.weather.thunder",
                bolt.logic.posX, bolt.logic.posY, bolt.logic.posZ,
                EntityLightningBolt::THUNDER_VOLUME,
                0.8f + static_cast<float>(rand() % 200) / 1000.0f);
            broadcastSound("random.explode",
                bolt.logic.posX, bolt.logic.posY, bolt.logic.posZ,
                EntityLightningBolt::EXPLODE_VOLUME,
                0.5f + static_cast<float>(rand() % 200) / 1000.0f);
        }

        // Entity damage in radius — Java: 3-block radius AABB scan
        if (result.damageEntities) {
            double minX, minY, minZ, maxX, maxY, maxZ;
            bolt.logic.getDamageBounds(minX, minY, minZ, maxX, maxY, maxZ);

            // Damage nearby players — Java: 5 hearts (10 damage)
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (!ph || ph->isDead() || ph->getGameMode() == 1) continue;

                double px = ph->getPlayerX(), py = ph->getPlayerY(), pz = ph->getPlayerZ();
                if (px >= minX && px <= maxX && py >= minY && py <= maxY && pz >= minZ && pz <= maxZ) {
                    // Java: entity.attackEntityFrom(DamageSource.lightning, 5.0f)
                    ph->applyDamage(5.0f);
                }
            }

            // Damage nearby mobs
            {
                std::lock_guard<std::mutex> mobLock(mobEntitiesMutex_);
                for (auto& mob : mobEntities_) {
                    if (mob.isDead) continue;
                    if (mob.posX >= minX && mob.posX <= maxX &&
                        mob.posY >= minY && mob.posY <= maxY &&
                        mob.posZ >= minZ && mob.posZ <= maxZ) {
                        mob.health -= 5.0f;
                        if (mob.health <= 0) {
                            mob.isDead = true;
                            mob.health = 0;
                        }
                    }
                }
            }
        }

        // Re-strike fire
        if (result.tryFire && !worlds_.empty()) {
            auto* world = worlds_[0].get();
            int32_t fx = static_cast<int32_t>(std::floor(bolt.logic.posX));
            int32_t fy = static_cast<int32_t>(std::floor(bolt.logic.posY));
            int32_t fz = static_cast<int32_t>(std::floor(bolt.logic.posZ));
            Block* atPos = world->getBlock(fx, fy, fz);
            if (!atPos || Block::getIdFromBlock(atPos) == 0) {
                world->setBlock(fx, fy, fz, Block::getBlockById(51));
                world->setBlockMetadata(fx, fy, fz, 0);
                broadcastBlockChange(fx, fy, fz, 51, 0);
            }
        }
    }

    // Broadcast destroy for dead bolts
    if (!destroyIds.empty()) {
        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (ph) ph->sendDestroyEntities(*conn, destroyIds);
        }
    }

    // Remove dead lightning
    lightningEntities_.erase(
        std::remove_if(lightningEntities_.begin(), lightningEntities_.end(),
            [](const SpawnedLightning& l) { return l.isDead; }),
        lightningEntities_.end());
}

// ═══════════════════════════════════════════════════════════════════════════
// Boat entity spawning — Java: ItemBoat.onItemRightClick → new EntityBoat
// ═══════════════════════════════════════════════════════════════════════════

int32_t MinecraftServer::spawnBoat(double x, double y, double z, float yaw) {
    int32_t eid = nextBoatEntityId_.fetch_add(1);

    SpawnedBoat boat;
    boat.entityId = eid;
    boat.posX = boat.prevPosX = x;
    boat.posY = boat.prevPosY = y;
    boat.posZ = boat.prevPosZ = z;
    boat.yaw = yaw;
    boat.isDead = false;
    boat.riderEntityId = -1;
    boat.speedMultiplier = 0.07;
    boat.spawnTick = tickCounter_.load();
    boat.lastSentPosX = static_cast<int32_t>(x * 32.0);
    boat.lastSentPosY = static_cast<int32_t>(y * 32.0);
    boat.lastSentPosZ = static_cast<int32_t>(z * 32.0);

    {
        std::lock_guard<std::mutex> lock(boatEntitiesMutex_);
        boatEntities_.push_back(std::move(boat));
    }

    // Broadcast S0E SpawnObject — type 1 = boat
    // Java: EntityTrackerEntry → S0EPacketSpawnObject(entity, 1)
    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
    for (auto& conn : connections_) {
        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
        auto handler = conn->getHandler();
        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
        if (ph) {
            ph->sendSpawnObject(*conn, eid, 1, x, y, z, yaw, 0.0f, 0, 0.0, 0.0, 0.0);
        }
    }

    std::cout << "[Boat] Spawned entity " << eid << " at " << x << "," << y << "," << z << "\n";
    return eid;
}

// ═══════════════════════════════════════════════════════════════════════════
// Boat entity ticking — Java: EntityBoat.onUpdate
// Water physics, rider steering, collision breaking, movement broadcast
// ═══════════════════════════════════════════════════════════════════════════

void MinecraftServer::tickBoats() {
    std::lock_guard<std::mutex> lock(boatEntitiesMutex_);
    if (boatEntities_.empty()) return;

    auto* world = worlds_.empty() ? nullptr : worlds_[0].get();
    if (!world) return;

    std::vector<int32_t> destroyIds;

    for (auto& boat : boatEntities_) {
        if (boat.isDead) continue;

        // Save previous position — Java: prevPosX/Y/Z
        boat.prevPosX = boat.posX;
        boat.prevPosY = boat.posY;
        boat.prevPosZ = boat.posZ;

        // Decrement timeSinceHit — Java: DataWatcher 17
        if (boat.timeSinceHit > 0) --boat.timeSinceHit;

        // Decrement damageTaken — Java: DataWatcher 19
        if (boat.damageTaken > 0.0f) boat.damageTaken -= 1.0f;

        // ─── Water submersion check — Java: 5 sampling points along AABB height ───
        // Java: d10 = fraction of AABB submerged in water (0.0 to 1.0)
        // We approximate by checking 5 Y-levels between boat bottom and top
        // Boat height = 0.6 (from EntityBoat constructor setSize(1.5, 0.6))
        double boatMinY = boat.posY - 0.3;  // yOffset = height/2 = 0.3
        double boatMaxY = boat.posY + 0.3;
        double d5 = 0.0;
        for (int i = 0; i < 5; ++i) {
            double sliceMin = boatMinY + (boatMaxY - boatMinY) * (double)i / 5.0 - 0.125;
            double sliceMax = boatMinY + (boatMaxY - boatMinY) * (double)(i + 1) / 5.0 - 0.125;
            // Check center of slice for water
            double sliceMid = (sliceMin + sliceMax) / 2.0;
            int32_t checkY = static_cast<int32_t>(std::floor(sliceMid));
            int32_t checkX = static_cast<int32_t>(std::floor(boat.posX));
            int32_t checkZ = static_cast<int32_t>(std::floor(boat.posZ));
            Block* blk = world->getBlock(checkX, checkY, checkZ);
            int32_t blkId = blk ? Block::getIdFromBlock(blk) : 0;
            if (blkId == 8 || blkId == 9) {
                d5 += 1.0 / 5.0;
            }
        }

        // Pre-collision speed for speed multiplier comparison
        double oldSpeed = std::sqrt(boat.motionX * boat.motionX + boat.motionZ * boat.motionZ);

        // ─── Buoyancy — Java: EntityBoat.onUpdate() lines 189-197 ───
        if (d5 < 1.0) {
            // Partially submerged or out of water
            double d4 = d5 * 2.0 - 1.0;
            boat.motionY += 0.04 * d4;
        } else {
            // Fully submerged
            if (boat.motionY < 0.0) {
                boat.motionY /= 2.0;
            }
            boat.motionY += 0.007;
        }

        // ─── Rider steering — Java: EntityBoat lines 198-203 ───
        if (boat.riderEntityId >= 0) {
            float riderYaw = 0;
            float riderForward = 0;
            float riderStrafe = 0;

            // Find rider's PlayHandler to get steering input
            {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (ph && ph->getEntityId() == boat.riderEntityId) {
                        riderYaw = ph->getPlayerYaw();
                        riderForward = ph->getMoveForward();
                        riderStrafe = ph->getMoveStrafing();
                        break;
                    }
                }
            }

            // Java: float f = riddenByEntity.rotationYaw + -moveStrafing * 90.0f
            float steeredYaw = riderYaw + -riderStrafe * 90.0f;
            // Java: motionX += -sin(f * PI/180) * speedMultiplier * moveForward * 0.05
            // Java: motionZ +=  cos(f * PI/180) * speedMultiplier * moveForward * 0.05
            double yawRad = static_cast<double>(steeredYaw) * M_PI / 180.0;
            boat.motionX += -std::sin(yawRad) * boat.speedMultiplier * static_cast<double>(riderForward) * 0.05;
            boat.motionZ +=  std::cos(yawRad) * boat.speedMultiplier * static_cast<double>(riderForward) * 0.05;
        }

        // ─── Speed cap — Java: EntityBoat lines 204-209 ───
        double newSpeed = std::sqrt(boat.motionX * boat.motionX + boat.motionZ * boat.motionZ);
        if (newSpeed > 0.35) {
            double scale = 0.35 / newSpeed;
            boat.motionX *= scale;
            boat.motionZ *= scale;
            newSpeed = 0.35;
        }

        // ─── Speed multiplier ramp — Java: EntityBoat lines 210-220 ───
        if (newSpeed > oldSpeed && boat.speedMultiplier < 0.35) {
            boat.speedMultiplier += (0.35 - boat.speedMultiplier) / 35.0;
            if (boat.speedMultiplier > 0.35) boat.speedMultiplier = 0.35;
        } else {
            boat.speedMultiplier -= (boat.speedMultiplier - 0.07) / 35.0;
            if (boat.speedMultiplier < 0.07) boat.speedMultiplier = 0.07;
        }

        // ─── Ground friction — Java: EntityBoat lines 237-241 ───
        // Applied BEFORE moveEntity in Java, but we apply inline
        Block* groundBlock = world->getBlock(
            static_cast<int32_t>(std::floor(boat.posX)),
            static_cast<int32_t>(std::floor(boat.posY)) - 1,
            static_cast<int32_t>(std::floor(boat.posZ)));
        boat.onGround = (groundBlock != nullptr && Block::getIdFromBlock(groundBlock) != 0
                         && Block::getIdFromBlock(groundBlock) != 8
                         && Block::getIdFromBlock(groundBlock) != 9);
        if (boat.onGround) {
            boat.motionX *= 0.5;
            boat.motionY *= 0.5;
            boat.motionZ *= 0.5;
        }

        // Apply motion — Java: moveEntity(motionX, motionY, motionZ)
        boat.posX += boat.motionX;
        boat.posY += boat.motionY;
        boat.posZ += boat.motionZ;

        // ─── Block collision check — Java: isCollidedHorizontally && d8 > 0.2 ───
        int32_t cx = static_cast<int32_t>(std::floor(boat.posX));
        int32_t cy = static_cast<int32_t>(std::floor(boat.posY));
        int32_t cz = static_cast<int32_t>(std::floor(boat.posZ));
        Block* atPos = world->getBlock(cx, cy, cz);
        int32_t atId = atPos ? Block::getIdFromBlock(atPos) : 0;

        // Solid non-passable block = collision (exclude air, water, lava, plants)
        bool isSolid = (atId != 0 && atId != 8 && atId != 9 && atId != 10 && atId != 11 &&
                        atId != 31 && atId != 32 && atId != 37 && atId != 38 &&
                        atId != 39 && atId != 40 && atId != 6 && atId != 50 &&
                        atId != 51 && atId != 55 && atId != 78 && atId != 106);
        if (isSolid && oldSpeed > 0.2) {
            // Java: isCollidedHorizontally && d8 > 0.2 → break with planks + sticks
            boat.isDead = true;
            destroyIds.push_back(boat.entityId);

            // Dismount rider
            if (boat.riderEntityId >= 0) {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph) continue;
                    if (ph->getEntityId() == boat.riderEntityId) {
                        ph->setRidingEntityId(-1);
                    }
                    ph->sendAttachEntity(*conn, 0, boat.riderEntityId, -1);
                }
            }

            // Java: drop 3 planks (5) + 2 sticks (280)
            spawnItemDrop(boat.posX, boat.posY, boat.posZ, 5, 0, 3);
            spawnItemDrop(boat.posX, boat.posY, boat.posZ, 280, 0, 2);
            broadcastSound("random.break", boat.posX, boat.posY, boat.posZ, 1.0f, 1.0f);
            continue;
        }

        // ─── Non-collision friction — Java: EntityBoat lines 253-257 ───
        if (!isSolid) {
            boat.motionX *= 0.99;   // Java: 0.99f
            boat.motionY *= 0.95;   // Java: 0.95f
            boat.motionZ *= 0.99;   // Java: 0.99f
        }

        // ─── Yaw from motion — Java: EntityBoat lines 258-272 ───
        // Java uses prevPos - pos (note: opposite direction), then wrapAngleTo180 + clamp ±20°
        boat.pitch = 0.0f;
        double dx = boat.prevPosX - boat.posX;
        double dz = boat.prevPosZ - boat.posZ;
        if (dx * dx + dz * dz > 0.001) {
            double targetYaw = std::atan2(dz, dx) * 180.0 / M_PI;
            // Java: wrapAngleTo180_double(targetYaw - rotationYaw)
            double delta = targetYaw - static_cast<double>(boat.yaw);
            // Wrap to -180..180
            while (delta > 180.0) delta -= 360.0;
            while (delta < -180.0) delta += 360.0;
            // Clamp ±20°
            if (delta > 20.0) delta = 20.0;
            if (delta < -20.0) delta = -20.0;
            boat.yaw = static_cast<float>(static_cast<double>(boat.yaw) + delta);
        }

        // ─── Broadcast position — S18 EntityTeleport ───
        int32_t newPX = static_cast<int32_t>(std::floor(boat.posX * 32.0));
        int32_t newPY = static_cast<int32_t>(std::floor(boat.posY * 32.0));
        int32_t newPZ = static_cast<int32_t>(std::floor(boat.posZ * 32.0));

        bool moved = (newPX != boat.lastSentPosX || newPY != boat.lastSentPosY || newPZ != boat.lastSentPosZ);
        ++boat.ticksSinceLastTeleport;

        if (moved || boat.ticksSinceLastTeleport >= 400) {
            boat.lastSentPosX = newPX;
            boat.lastSentPosY = newPY;
            boat.lastSentPosZ = newPZ;
            boat.ticksSinceLastTeleport = 0;

            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (ph) {
                    ph->sendEntityTeleport(*conn, boat.entityId,
                        boat.posX, boat.posY, boat.posZ, boat.yaw, boat.pitch);
                }
            }
        }
    }

    // Broadcast destroy for dead boats
    if (!destroyIds.empty()) {
        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (ph) ph->sendDestroyEntities(*conn, destroyIds);
        }
    }

    // Remove dead boats
    boatEntities_.erase(
        std::remove_if(boatEntities_.begin(), boatEntities_.end(),
            [](const SpawnedBoat& b) { return b.isDead; }),
        boatEntities_.end());
}

// ─── XP Orb Entity System ──────────────────────────────────────────────
// Java reference: net.minecraft.entity.item.EntityXPOrb

int32_t MinecraftServer::getXPSplit(int32_t xpAmount) {
    // Java: EntityXPOrb.getXPSplit(int) — returns the largest standard orb value <= amount
    // Tier thresholds match Java exactly
    if (xpAmount >= 2477) return 2477;
    if (xpAmount >= 1237) return 1237;
    if (xpAmount >= 617) return 617;
    if (xpAmount >= 307) return 307;
    if (xpAmount >= 149) return 149;
    if (xpAmount >= 73) return 73;
    if (xpAmount >= 37) return 37;
    if (xpAmount >= 17) return 17;
    if (xpAmount >= 7) return 7;
    if (xpAmount >= 3) return 3;
    return 1;
}

void MinecraftServer::spawnXPOrbs(double x, double y, double z, int32_t totalXp) {
    // Java: EntityLiving.onDeathUpdate() splits XP via getXPSplit()
    // while (xpRemaining > 0) { split = getXPSplit(xpRemaining); xpRemaining -= split; spawn(split); }
    if (totalXp <= 0) return;

    std::vector<SpawnedXPOrb> newOrbs;
    int32_t remaining = totalXp;
    while (remaining > 0) {
        int32_t split = getXPSplit(remaining);
        remaining -= split;

        SpawnedXPOrb orb;
        orb.entityId = nextXPOrbEntityId_.fetch_add(1);
        orb.posX = x;
        orb.posY = y;
        orb.posZ = z;
        orb.xpValue = split;
        orb.xpOrbAge = 0;
        orb.xpColor = 0;
        orb.pickupDelay = 0;
        orb.isDead = false;
        orb.onGround = false;
        orb.spawnTick = getTickCount();

        // Java: EntityXPOrb constructor — random initial velocity
        // motionX = (rand * 0.2 - 0.1) * 2.0
        // motionY = rand * 0.2 * 2.0
        // motionZ = (rand * 0.2 - 0.1) * 2.0
        orb.motionX = (static_cast<double>(rand() % 1000) / 1000.0 * 0.2 - 0.1) * 2.0;
        orb.motionY = static_cast<double>(rand() % 1000) / 1000.0 * 0.2 * 2.0;
        orb.motionZ = (static_cast<double>(rand() % 1000) / 1000.0 * 0.2 - 0.1) * 2.0;

        newOrbs.push_back(orb);
    }

    // Broadcast S11 SpawnExpOrb to all players
    {
        std::lock_guard<std::recursive_mutex> lock(connectionsMutex_);
        for (auto& orb : newOrbs) {
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (ph) {
                    ph->sendSpawnExpOrb(*conn, orb.entityId,
                        orb.posX, orb.posY, orb.posZ,
                        static_cast<int16_t>(orb.xpValue));
                }
            }
        }
    }

    // Add to tracked entities
    {
        std::lock_guard<std::mutex> lock(xpOrbEntitiesMutex_);
        for (auto& orb : newOrbs) {
            xpOrbEntities_.push_back(std::move(orb));
        }
    }

    std::cout << "[XP] Spawned " << newOrbs.size() << " XP orbs ("
              << totalXp << " total XP) at " << x << "," << y << "," << z << "\n";
}

void MinecraftServer::tickXPOrbs() {
    // Java reference: EntityXPOrb.onUpdate()
    std::lock_guard<std::mutex> lock(xpOrbEntitiesMutex_);
    if (xpOrbEntities_.empty()) return;

    // Collect destroy IDs for dead orbs
    std::vector<int32_t> destroyIds;

    for (auto& orb : xpOrbEntities_) {
        if (orb.isDead) continue;

        // Age and despawn — Java: if (xpOrbAge++ >= 6000) setDead()
        orb.xpOrbAge++;
        if (orb.xpOrbAge >= SpawnedXPOrb::DESPAWN_AGE) {
            orb.isDead = true;
            destroyIds.push_back(orb.entityId);
            continue;
        }

        // Pickup delay countdown — Java: field_70532_c > 0 ? field_70532_c-- : ...
        if (orb.pickupDelay > 0) {
            orb.pickupDelay--;
        }

        // ─── Player attraction — Java: EntityXPOrb.onUpdate() lines 70-90 ───
        // Every 25 ticks (xpColor % 25 == 0), find closest player within 8 blocks
        // Then move toward closest player
        orb.xpColor++;

        double closestDistSq = SpawnedXPOrb::ATTRACT_RANGE * SpawnedXPOrb::ATTRACT_RANGE;
        PlayHandler* closestPlayer = nullptr;
        Connection* closestConn = nullptr;

        {
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (!ph || ph->isDead()) continue;

                double dx = ph->getPlayerX() - orb.posX;
                double dy = (ph->getPlayerY() + 1.0) - orb.posY; // Eye height offset
                double dz = ph->getPlayerZ() - orb.posZ;
                double distSq = dx * dx + dy * dy + dz * dz;

                if (distSq < closestDistSq) {
                    closestDistSq = distSq;
                    closestPlayer = ph;
                    closestConn = conn.get();
                }
            }
        }

        // Java: if (closestEntityPlayer != null && d6 < 64.0) — attract toward player
        if (closestPlayer && closestDistSq < 64.0) { // 8² = 64
            double dx = closestPlayer->getPlayerX() - orb.posX;
            double dy = (closestPlayer->getPlayerY() + 1.0) - orb.posY;
            double dz = closestPlayer->getPlayerZ() - orb.posZ;
            double dist = std::sqrt(closestDistSq);

            // Java: vel = (1.0 - sqrt(distSq) / 8.0) * 0.1
            // Orbs accelerate as they get closer
            if (dist > 0.01) {
                double vel = (1.0 - dist / SpawnedXPOrb::ATTRACT_RANGE) * 0.1;
                orb.motionX += dx / dist * vel;
                orb.motionY += dy / dist * vel;
                orb.motionZ += dz / dist * vel;
            }

            // ─── Pickup check — Java: EntityXPOrb.onUpdate() lines 93-105 ───
            // if (field_70532_c == 0 && player.xpCooldown == 0 && distSq < 1.0)
            if (orb.pickupDelay == 0 && closestDistSq < 1.0) {
                // Pickup! Grant XP to player
                closestPlayer->grantExperience(orb.xpValue);
                closestPlayer->sendExperienceUpdate(*closestConn);

                // Play pickup sound — Java: world.playSoundAtEntity("random.orb", 0.1f, ...)
                // pitch = 0.5f * ((rand * 0.8f) + 0.6f)
                float soundPitch = 0.5f * (static_cast<float>(rand() % 1000) / 1000.0f * 0.8f + 0.6f);
                broadcastSound("random.orb", orb.posX, orb.posY, orb.posZ, 0.1f, soundPitch);

                // Send collect animation — Java: S0D CollectItem
                {
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    for (auto& conn : connections_) {
                        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                        auto handler = conn->getHandler();
                        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                        if (ph) {
                            ph->sendCollectItem(*conn, orb.entityId, closestPlayer->getEntityId());
                        }
                    }
                }

                orb.isDead = true;
                destroyIds.push_back(orb.entityId);
                continue;
            }
        }

        // ─── Physics — Java: EntityXPOrb.onUpdate() ─────────────────────
        // Gravity
        orb.motionY -= SpawnedXPOrb::GRAVITY;

        // Apply motion
        double newX = orb.posX + orb.motionX;
        double newY = orb.posY + orb.motionY;
        double newZ = orb.posZ + orb.motionZ;

        // Simple ground collision — check block at feet
        orb.onGround = false;
        if (!worlds_.empty()) {
            auto* world = worlds_[0].get();
            int32_t blockX = static_cast<int32_t>(std::floor(newX));
            int32_t blockY = static_cast<int32_t>(std::floor(newY));
            int32_t blockZ = static_cast<int32_t>(std::floor(newZ));

            Block* below = world->getBlock(blockX, blockY - 1, blockZ);
            int32_t belowId = below ? Block::getIdFromBlock(below) : 0;
            Block* at = world->getBlock(blockX, blockY, blockZ);
            int32_t atId = at ? Block::getIdFromBlock(at) : 0;

            // Java: EntityXPOrb checks bounding box collision
            // Simplified: if block below is solid and we're near ground level
            if (belowId != 0 && atId == 0 && newY < static_cast<double>(blockY) + 0.25) {
                orb.onGround = true;
                newY = static_cast<double>(blockY);
                orb.motionY *= SpawnedXPOrb::GROUND_BOUNCE; // Bounce (-0.9)
                if (std::abs(orb.motionY) < 0.01) orb.motionY = 0;
            }
        }

        orb.posX = newX;
        orb.posY = newY;
        orb.posZ = newZ;

        // Friction — Java: motionX *= 0.98, motionY *= 0.98 (then -0.03 gravity), motionZ *= 0.98
        orb.motionX *= SpawnedXPOrb::AIR_FRICTION;
        orb.motionY *= SpawnedXPOrb::AIR_FRICTION;
        orb.motionZ *= SpawnedXPOrb::AIR_FRICTION;

        // If on ground, extra horizontal friction — Java: multiply by 0.6 block slipperiness
        if (orb.onGround) {
            orb.motionX *= 0.7;
            orb.motionZ *= 0.7;
        }

        // ─── Broadcast position update (EntityTeleport) every 3 ticks ───
        if (orb.xpOrbAge % 3 == 0) {
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (ph) {
                    ph->sendEntityTeleport(*conn, orb.entityId,
                        orb.posX, orb.posY, orb.posZ, 0.0f, 0.0f);
                }
            }
        }
    }

    // Broadcast destroy for dead orbs
    if (!destroyIds.empty()) {
        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (ph) ph->sendDestroyEntities(*conn, destroyIds);
        }
    }

    // Remove dead orbs
    xpOrbEntities_.erase(
        std::remove_if(xpOrbEntities_.begin(), xpOrbEntities_.end(),
            [](const SpawnedXPOrb& o) { return o.isDead; }),
        xpOrbEntities_.end());
}

// ═══════════════════════════════════════════════════════════════════════════
// Wither Boss — Java: EntityWither.onLivingUpdate() + updateAITasks()
// ═══════════════════════════════════════════════════════════════════════════
void MinecraftServer::tickWither(SpawnedMob& wither, int64_t currentTick) {
    ++wither.witherTicksExisted;

    // ─── Java: EntityWither.onLivingUpdate() ───
    // motionY *= 0.6f — Wither floats, reduced Y velocity
    wither.witherMotionY *= 0.6;

    // ─── Fly toward primary target — Java: EntityWither.onLivingUpdate() lines 110-123 ───
    if (wither.witherWatchedTargets[0] > 0) {
        // Find nearest player as target
        PlayHandler* target = nullptr;
        double targetX = 0, targetY = 0, targetZ = 0;
        {
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (ph && ph->getEntityId() == wither.witherWatchedTargets[0] && !ph->isDead()) {
                    target = ph;
                    targetX = ph->getPlayerX();
                    targetY = ph->getPlayerY();
                    targetZ = ph->getPlayerZ();
                    break;
                }
            }
        }
        if (target) {
            // Java: if (posY < entity.posY || !isArmored() && posY < entity.posY + 5)
            if (wither.posY < targetY || (!wither.isWitherArmored() && wither.posY < targetY + 5.0)) {
                if (wither.witherMotionY < 0.0) wither.witherMotionY = 0.0;
                wither.witherMotionY += (0.5 - wither.witherMotionY) * 0.6;
            }
            // Java: horizontal chase
            double dx = targetX - wither.posX;
            double dz = targetZ - wither.posZ;
            double distSq = dx * dx + dz * dz;
            if (distSq > 9.0) {
                double dist = std::sqrt(distSq);
                wither.witherMotionX += (dx / dist * 0.5 - wither.witherMotionX) * 0.6;
                wither.witherMotionZ += (dz / dist * 0.5 - wither.witherMotionZ) * 0.6;
            }
        }
    }

    // Java: face direction of horizontal motion
    if (wither.witherMotionX * wither.witherMotionX + wither.witherMotionZ * wither.witherMotionZ > 0.05) {
        wither.yaw = static_cast<float>(std::atan2(wither.witherMotionZ, wither.witherMotionX) * 57.295776 - 90.0);
    }

    // Apply motion
    wither.posX += wither.witherMotionX;
    wither.posY += wither.witherMotionY;
    wither.posZ += wither.witherMotionZ;

    // Broadcast position
    {
        int32_t nx = static_cast<int32_t>(std::floor(wither.posX * 32.0));
        int32_t ny = static_cast<int32_t>(std::floor(wither.posY * 32.0));
        int32_t nz = static_cast<int32_t>(std::floor(wither.posZ * 32.0));
        ++wither.ticksSinceLastTeleport;
        if (wither.ticksSinceLastTeleport >= 60 ||
            std::abs(nx - wither.lastSentPosX) > 512 ||
            std::abs(ny - wither.lastSentPosY) > 512 ||
            std::abs(nz - wither.lastSentPosZ) > 512) {
            // Teleport
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto h = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(h.get());
                if (ph) ph->sendEntityTeleport(*conn, wither.entityId, wither.posX, wither.posY, wither.posZ, wither.yaw, 0.0f);
            }
            wither.lastSentPosX = nx;
            wither.lastSentPosY = ny;
            wither.lastSentPosZ = nz;
            wither.ticksSinceLastTeleport = 0;
        } else {
            int8_t relX = static_cast<int8_t>(nx - wither.lastSentPosX);
            int8_t relY = static_cast<int8_t>(ny - wither.lastSentPosY);
            int8_t relZ = static_cast<int8_t>(nz - wither.lastSentPosZ);
            if (relX != 0 || relY != 0 || relZ != 0) {
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto h = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(h.get());
                    if (ph) ph->sendEntityLookRelMove(*conn, wither.entityId, relX, relY, relZ,
                        static_cast<float>(wither.yaw), 0.0f);
                }
                wither.lastSentPosX = nx;
                wither.lastSentPosY = ny;
                wither.lastSentPosZ = nz;
            }
        }
    }

    // ─── Smoke particles on all 3 heads — Java: onLivingUpdate() lines 155-162 ───
    if (wither.witherTicksExisted % 5 == 0) {
        for (int headIdx = 0; headIdx < 3; ++headIdx) {
            double headX = wither.posX;
            double headY = (headIdx <= 0) ? wither.posY + 3.0 : wither.posY + 2.2;
            double headZ = wither.posZ;
            if (headIdx > 0) {
                float angle = (wither.yaw + static_cast<float>(180 * (headIdx - 1))) / 180.0f * static_cast<float>(M_PI);
                headX += static_cast<double>(std::cos(angle)) * 1.3;
                headZ += static_cast<double>(std::sin(angle)) * 1.3;
            }
            broadcastParticle("smoke",
                static_cast<float>(headX), static_cast<float>(headY), static_cast<float>(headZ),
                0.3f, 0.3f, 0.3f, 0.0f, 1);
        }
    }

    // ─── Java: EntityWither.updateAITasks() ───

    // ─── Invulnerability phase (spawn animation) ───
    if (wither.witherInvulTime > 0) {
        int32_t newInvul = wither.witherInvulTime - 1;

        if (newInvul <= 0) {
            // Java: spawn explosion — power 7.0, no fire, respect mobGriefing
            createExplosion(wither.posX, wither.posY + 3.0, wither.posZ, 7.0f, false, true);
            // Java: this.worldObj.playBroadcastSound(1013, ...)
            broadcastEffect(1013, static_cast<int32_t>(wither.posX),
                static_cast<int32_t>(wither.posY), static_cast<int32_t>(wither.posZ), 0);
        }

        wither.witherInvulTime = newInvul;

        // Java: heal 10 HP every 10 ticks during invul
        if (wither.witherTicksExisted % 10 == 0) {
            wither.health = std::min(wither.health + 10.0f, 300.0f);
        }

        // Invul particles — Java: lines 163-167
        if (wither.witherTicksExisted % 3 == 0) {
            broadcastParticle("mobSpell",
                static_cast<float>(wither.posX + ((rand() % 200 - 100) / 100.0)),
                static_cast<float>(wither.posY + ((float)(rand() % 330) / 100.0f)),
                static_cast<float>(wither.posZ + ((rand() % 200 - 100) / 100.0)),
                0.7f, 0.7f, 0.9f, 0.0f, 1);
        }

        // Broadcast DW 20 invul time
        {
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                conn->sendPacket(PacketBuilder::entityMetadataInt(wither.entityId, 20, wither.witherInvulTime));
            }
        }
        return;  // Java: return during invul
    }

    // ─── Side head targeting — Java: updateAITasks() lines 187-230 ───
    for (int headIdx = 1; headIdx < 3; ++headIdx) {
        int sideIdx = headIdx - 1;

        // Check if current tick is past attack timer
        if (wither.witherTicksExisted >= wither.witherSideAttackTimer[sideIdx]) {
            wither.witherSideAttackTimer[sideIdx] = wither.witherTicksExisted + 10 + (rand() % 10);

            // Java: charged shot after 15 charges — fire at random location
            if (wither.witherSideChargeCounter[sideIdx] > 15) {
                double randX = wither.posX + ((double)(rand() % 2000 - 1000) / 100.0);
                double randY = wither.posY + ((double)(rand() % 1000 - 500) / 100.0);
                double randZ = wither.posZ + ((double)(rand() % 2000 - 1000) / 100.0);

                // Fire charged wither skull (blue) — larger explosion
                // Java: launchWitherSkullToCoords(n+1, d, d2, d3, true) → charged skull
                broadcastEffect(1014, static_cast<int32_t>(wither.posX),
                    static_cast<int32_t>(wither.posY), static_cast<int32_t>(wither.posZ), 0);

                // Simplified: instant explosion at random location (no projectile entity)
                createExplosion(randX, randY, randZ, 1.0f, false, true);
                wither.witherSideChargeCounter[sideIdx] = 0;
                continue;
            }

            // Java: find target for this side head
            int32_t sideTarget = wither.witherWatchedTargets[headIdx];
            if (sideTarget > 0) {
                // Check if target is still valid
                bool valid = false;
                double tx = 0, ty = 0, tz = 0;
                {
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    for (auto& conn : connections_) {
                        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                        auto handler = conn->getHandler();
                        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                        if (ph && ph->getEntityId() == sideTarget && !ph->isDead()) {
                            double dx = ph->getPlayerX() - wither.posX;
                            double dz = ph->getPlayerZ() - wither.posZ;
                            if (dx * dx + dz * dz <= 900.0) {  // Java: 30 blocks range
                                valid = true;
                                tx = ph->getPlayerX();
                                ty = ph->getPlayerY() + 0.85; // eye height * 0.5
                                tz = ph->getPlayerZ();
                            }
                            break;
                        }
                    }
                }

                if (valid) {
                    // Fire skull at target — Java: launchWitherSkullToEntity()
                    // Sound effect 1014 = wither skull shoot
                    broadcastEffect(1014, static_cast<int32_t>(wither.posX),
                        static_cast<int32_t>(wither.posY), static_cast<int32_t>(wither.posZ), 0);

                    // Simplified: direct damage + Wither II effect instead of projectile
                    // Java: EntityWitherSkull.onImpact() → 8 damage + Wither II
                    {
                        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                        for (auto& conn : connections_) {
                            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                            auto handler = conn->getHandler();
                            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                            if (ph && ph->getEntityId() == sideTarget && !ph->isDead()) {
                                ph->applyDamage(8.0f);
                                // Java: Wither II for 10s (normal) — Potion.wither.id = 20
                                ph->addPotionEffect(*conn, 20, 200, 1);
                                break;
                            }
                        }
                    }

                    // Small explosion at target — Java: newExplosion(skull, x, y, z, 1.0f, ...)
                    createExplosion(tx, ty, tz, 1.0f, false, true);

                    wither.witherSideAttackTimer[sideIdx] = wither.witherTicksExisted + 40 + (rand() % 20);
                    wither.witherSideChargeCounter[sideIdx] = 0;
                } else {
                    wither.witherWatchedTargets[headIdx] = 0;
                }
            } else {
                // Java: selectEntitiesWithinAABB — find new target
                std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                double nearestDist = 400.0;  // 20 blocks squared
                int32_t nearestId = 0;
                for (auto& conn : connections_) {
                    if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                    auto handler = conn->getHandler();
                    auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                    if (!ph || ph->isDead()) continue;
                    // Java: creative players with disableDamage are skipped
                    if (ph->getGameMode() == 1) continue;
                    double dx = ph->getPlayerX() - wither.posX;
                    double dy = ph->getPlayerY() - wither.posY;
                    double dz = ph->getPlayerZ() - wither.posZ;
                    double distSq = dx * dx + dy * dy + dz * dz;
                    if (distSq < nearestDist) {
                        nearestDist = distSq;
                        nearestId = ph->getEntityId();
                    }
                }
                if (nearestId > 0) {
                    wither.witherWatchedTargets[headIdx] = nearestId;
                }
            }
        }
    }

    // ─── Center head attack (melee range) — Java: attackEntityWithRangedAttack() ───
    // Java: EntityAIArrowAttack — attacks primary target at range 20, cooldown 40 ticks
    if (wither.witherWatchedTargets[0] > 0 && wither.witherTicksExisted % 40 == 0) {
        int32_t centerTarget = wither.witherWatchedTargets[0];
        double tx = 0, ty = 0, tz = 0;
        bool targetExists = false;
        {
            std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
            for (auto& conn : connections_) {
                if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                auto handler = conn->getHandler();
                auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                if (ph && ph->getEntityId() == centerTarget && !ph->isDead()) {
                    targetExists = true;
                    tx = ph->getPlayerX();
                    ty = ph->getPlayerY() + 0.85;
                    tz = ph->getPlayerZ();
                    break;
                }
            }
        }
        if (targetExists) {
            double dx = tx - wither.posX;
            double dz = tz - wither.posZ;
            if (dx * dx + dz * dz <= 400.0) {  // 20 blocks range
                broadcastEffect(1014, static_cast<int32_t>(wither.posX),
                    static_cast<int32_t>(wither.posY), static_cast<int32_t>(wither.posZ), 0);
                // Damage target
                {
                    std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
                    for (auto& conn : connections_) {
                        if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
                        auto handler = conn->getHandler();
                        auto* ph = dynamic_cast<PlayHandler*>(handler.get());
                        if (ph && ph->getEntityId() == centerTarget && !ph->isDead()) {
                            ph->applyDamage(8.0f);
                            ph->addPotionEffect(*conn, 20, 200, 1); // Wither II 10s
                            break;
                        }
                    }
                }
                createExplosion(tx, ty, tz, 1.0f, false, true);
            } else {
                wither.witherWatchedTargets[0] = 0;
            }
        } else {
            wither.witherWatchedTargets[0] = 0;
        }
    }

    // ─── Primary target acquisition — Java: updateAITasks() lines 231-235 ───
    if (wither.witherWatchedTargets[0] == 0 && wither.witherTicksExisted % 20 == 0) {
        // Find nearest visible player
        std::lock_guard<std::recursive_mutex> connLock(connectionsMutex_);
        double nearestDist = 1600.0;  // 40 blocks range (Java: followRange=40)
        int32_t nearestId = 0;
        for (auto& conn : connections_) {
            if (!conn->isConnected() || conn->getState() != ConnectionState::Play) continue;
            auto handler = conn->getHandler();
            auto* ph = dynamic_cast<PlayHandler*>(handler.get());
            if (!ph || ph->isDead()) continue;
            if (ph->getGameMode() == 1) continue;  // Skip creative
            double dx = ph->getPlayerX() - wither.posX;
            double dy = ph->getPlayerY() - wither.posY;
            double dz = ph->getPlayerZ() - wither.posZ;
            double distSq = dx * dx + dy * dy + dz * dz;
            if (distSq < nearestDist) {
                nearestDist = distSq;
                nearestId = ph->getEntityId();
            }
        }
        wither.witherWatchedTargets[0] = nearestId;
    }

    // ─── Block destruction timer — Java: updateAITasks() lines 236-258 ───
    if (wither.witherBlockBreakTimer > 0) {
        --wither.witherBlockBreakTimer;
        if (wither.witherBlockBreakTimer == 0) {
            // Java: mobGriefing check → break blocks in 3×4×3 area around wither
            int bx = static_cast<int>(std::floor(wither.posX));
            int by = static_cast<int>(std::floor(wither.posY));
            int bz = static_cast<int>(std::floor(wither.posZ));
            bool broke = false;
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    for (int k = 0; k <= 3; ++k) {
                        int wx = bx + i;
                        int wy = by + k;
                        int wz = bz + j;
                        int32_t blockId = getBlockIdInWorld(wx, wy, wz);
                        // Java: skip air, bedrock, end_portal, end_portal_frame, command_block
                        if (blockId == 0 || blockId == 7 || blockId == 119 || blockId == 120 || blockId == 137) continue;
                        setBlockInWorld(wx, wy, wz, 0, 0);  // Break to air
                        broke = true;
                    }
                }
            }
            if (broke) {
                // Java: playAuxSFXAtEntity(null, 1012, ...)
                broadcastEffect(1012, bx, by, bz, 0);
            }
        }
    }

    // ─── Regeneration — Java: updateAITasks() line 260 → heal 1 every 20 ticks ───
    if (wither.witherTicksExisted % 20 == 0) {
        wither.health = std::min(wither.health + 1.0f, 300.0f);
    }

    // ─── Ambient sound — Java: getLivingSound() = "mob.wither.idle" ───
    if ((rand() % 200) == 0) {
        broadcastSound("mob.wither.idle", wither.posX, wither.posY, wither.posZ,
            1.0f, 0.8f + ((float)(rand() % 40) / 100.0f));
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Snow Golem AI — Java: EntitySnowman.onLivingUpdate() + attackEntityWithRangedAttack()
// ═══════════════════════════════════════════════════════════════════════════
void MinecraftServer::tickSnowGolem(SpawnedMob& golem, int64_t currentTick) {
    if (worlds_.empty()) return;
    auto* world = worlds_[0].get();

    int bx = static_cast<int>(std::floor(golem.posX));
    int by = static_cast<int>(std::floor(golem.posY));
    int bz = static_cast<int>(std::floor(golem.posZ));

    // ─── Water damage — Java: if (this.isWet()) attackEntityFrom(DamageSource.drown, 1.0f) ───
    // Check if feet block is water (8 or 9) or if it's raining and exposed to sky
    {
        int32_t feetBlockId = getBlockIdInWorld(bx, by, bz);
        bool isWet = (feetBlockId == 8 || feetBlockId == 9);
        // Also check for rain exposure — Java: isWet() includes rain
        if (!isWet && isRaining()) {
            // Check if exposed to sky (no solid block above)
            int32_t aboveId = getBlockIdInWorld(bx, by + 2, bz);
            if (aboveId == 0) isWet = true;
        }
        if (isWet) {
            golem.health -= 1.0f;
            broadcastEntityEvent(golem.entityId, 2); // hurt animation
            if (golem.health <= 0.0f) {
                golem.isDead = true;
                // Drop snowballs — Java: dropFewItems → rand.nextInt(16) snowballs
                int dropCount = rand() % 16;
                for (int i = 0; i < dropCount; ++i) {
                    spawnItemDrop(golem.posX, golem.posY, golem.posZ, 332, 0, 1); // snowball
                }
                broadcastSound("mob.snowman.death", golem.posX, golem.posY, golem.posZ, 1.0f, 1.0f);
                return;
            }
        }
    }

    // ─── Heat damage — Java: getBiomeGenForCoords().getFloatTemperature() > 1.0f ───
    // No biome system yet; approximate by checking ground material
    // Hot biomes: desert(sand=12), mesa(hardened clay=172, stained clay=159), nether(netherrack=87)
    // Also consider savanna-like (red sand meta, etc.)
    {
        int32_t groundId = getBlockIdInWorld(bx, by - 1, bz);
        bool isHot = false;
        // Desert: sand (12)
        if (groundId == 12) isHot = true;
        // Mesa: hardened clay (172) or stained clay (159)
        if (groundId == 172 || groundId == 159) isHot = true;
        // Nether: netherrack (87) or soul sand (88) or nether brick (112)
        if (groundId == 87 || groundId == 88 || groundId == 112) isHot = true;
        // Jungle: same as warm — podzol (3 meta 2), but we just check general temp
        // Mushroom island: mycelium (110) is temperate, skip

        if (isHot) {
            golem.health -= 1.0f;
            golem.isOnFire = true;
            broadcastEntityEvent(golem.entityId, 2); // hurt animation
            if (golem.health <= 0.0f) {
                golem.isDead = true;
                int dropCount = rand() % 16;
                for (int i = 0; i < dropCount; ++i) {
                    spawnItemDrop(golem.posX, golem.posY, golem.posZ, 332, 0, 1);
                }
                broadcastSound("mob.snowman.death", golem.posX, golem.posY, golem.posZ, 1.0f, 1.0f);
                return;
            }
        } else {
            golem.isOnFire = false;
        }
    }

    // ─── Snow trail — Java: EntitySnowman.onLivingUpdate() lines 65-69 ───
    // Place snow layers at 4 corner positions around the golem
    // Java: for (int i = 0; i < 4; ++i) { check (x ± 0.25, z ± 0.25) }
    // Only in cold biomes (temperature < 0.8f) — approximate: not hot ground
    {
        int32_t groundId = getBlockIdInWorld(bx, by - 1, bz);
        bool isCold = !(groundId == 12 || groundId == 172 || groundId == 159 ||
                        groundId == 87 || groundId == 88 || groundId == 112);

        if (isCold) {
            for (int i = 0; i < 4; ++i) {
                int sx = static_cast<int>(std::floor(golem.posX + static_cast<double>((i % 2 * 2 - 1)) * 0.25));
                int sy = static_cast<int>(std::floor(golem.posY));
                int sz = static_cast<int>(std::floor(golem.posZ + static_cast<double>((i / 2 % 2 * 2 - 1)) * 0.25));

                // Java: Material.air check + temperature < 0.8 + canPlaceBlockAt
                int32_t blockHere = getBlockIdInWorld(sx, sy, sz);
                int32_t blockBelow = getBlockIdInWorld(sx, sy - 1, sz);

                // Only place snow layer if:
                // - Current position is air (0)
                // - Block below is solid (not air, water, lava, snow layer)
                if (blockHere == 0 && blockBelow != 0 && blockBelow != 8 && blockBelow != 9 &&
                    blockBelow != 10 && blockBelow != 11 && blockBelow != 78) {
                    setBlockInWorld(sx, sy, sz, 78, 0); // snow_layer = 78
                }
            }
        }
    }

    // ─── Ranged snowball attack — Java: EntityAIArrowAttack + attackEntityWithRangedAttack() ───
    // Attack cooldown: 20 ticks (Java: EntityAIArrowAttack(this, 1.25, 20, 10.0f))
    // Target: nearest hostile mob within 10 blocks
    if (golem.snowGolemAttackCooldown > 0) {
        --golem.snowGolemAttackCooldown;
    }

    if (golem.snowGolemAttackCooldown <= 0) {
        // Find nearest hostile mob within 10 blocks (100 distSq)
        // Java: EntityAINearestAttackableTarget(this, EntityLiving.class, 0, true, false, IMob.mobSelector)
        SpawnedMob* target = nullptr;
        double nearestDistSq = 100.0;  // 10 blocks squared

        for (auto& other : mobEntities_) {
            if (other.entityId == golem.entityId || other.isDead) continue;
            // Only target hostile mobs — Java: IMob.mobSelector
            // Hostile mob types: 50=creeper, 51=skeleton, 52=spider, 54=zombie, 55=slime,
            // 56=ghast, 57=zombie pigman, 58=enderman, 59=cave spider, 60=silverfish,
            // 61=blaze, 62=magma cube, 66=witch
            bool isHostile = false;
            switch (other.mobType) {
                case 50: case 51: case 52: case 54: case 55: case 56:
                case 57: case 58: case 59: case 60: case 61: case 62: case 66:
                    isHostile = true;
                    break;
            }
            if (!isHostile) continue;

            double dx = other.posX - golem.posX;
            double dz = other.posZ - golem.posZ;
            double dy = other.posY - golem.posY;
            double distSq = dx * dx + dy * dy + dz * dz;
            if (distSq < nearestDistSq) {
                nearestDistSq = distSq;
                target = &other;
            }
        }

        if (target) {
            // Java: attackEntityWithRangedAttack(target, f)
            // EntitySnowball thrown at target with heading calculation
            double dx = target->posX - golem.posX;
            double dy = target->posY + 1.0 - 1.1 - (golem.posY + 1.0); // Java: target.getEyeHeight() - 1.1f - snowball.posY
            double dz = target->posZ - golem.posZ;
            // Java: float f2 = MathHelper.sqrt_double(dx*dx + dz*dz) * 0.2f
            double horizDist = std::sqrt(dx * dx + dz * dz);
            dy += horizDist * 0.2; // arc compensation

            // Spawn snowball throwable — Java: setThrowableHeading(dx, dy, dz, 1.6f, 12.0f)
            // Normalize and scale by speed (1.6) with inaccuracy (12.0)
            double len = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (len > 0.0) {
                dx /= len;
                dy /= len;
                dz /= len;
            }
            // Add inaccuracy — Java: 12.0f / 180 * 0.0175... ≈ random scatter
            double scatter = 12.0 / 180.0 * M_PI;
            dx += ((double)(rand() % 100) / 100.0 - 0.5) * scatter * 0.007499999832361937;
            dy += ((double)(rand() % 100) / 100.0 - 0.5) * scatter * 0.007499999832361937;
            dz += ((double)(rand() % 100) / 100.0 - 0.5) * scatter * 0.007499999832361937;
            double speed = 1.6;
            dx *= speed;
            dy *= speed;
            dz *= speed;

            spawnThrowable(ThrowableType::Snowball,
                golem.posX, golem.posY + 1.7, golem.posZ,
                dx, dy, dz, golem.entityId, "");

            // Java: this.playSound("random.bow", 1.0f, 1.0f / (getRNG().nextFloat() * 0.4f + 0.8f))
            float pitch = 1.0f / (((float)(rand() % 100) / 100.0f) * 0.4f + 0.8f);
            broadcastSound("random.bow", golem.posX, golem.posY, golem.posZ, 1.0f, pitch);

            golem.snowGolemAttackCooldown = 20; // 20 ticks = 1 second
        }
    }
}

} // namespace mccpp

