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
#include "inventory/Inventory.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>
#include <map>
#include <array>

#include "command/CommandSystem.h"
#include "crafting/Crafting.h"

namespace mccpp {

// Position struct for getPlayerPosition result
struct PlayerPosition {
    double x, y, z;
};

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

    /**
     * Request server shutdown (callable from commands).
     * Java reference: MinecraftServer.initiateShutdown()
     */
    void requestShutdown() { stop(); }

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

    /** Get current world time of day (0-23999). Java reference: WorldServer.getWorldTime() */
    int64_t getWorldTime() const;
    /** Get total ticks since server started. Used as world age in S03 TimeUpdate. */
    int64_t getWorldAge() const { return tickCounter_.load(); }
    /** Broadcast S03 TimeUpdate to all connected players. */
    void broadcastTimeUpdate();

    /**
     * Create an explosion at the given position.
     * Java reference: World.createExplosion() + Explosion.doExplosionA/B
     * @param x, y, z - center of explosion
     * @param power - explosion radius (TNT=4.0, creeper=3.0, charged creeper=6.0)
     * @param causesFire - whether the explosion sets fires
     * @param breakBlocks - whether the explosion destroys blocks
     */
    void createExplosion(double x, double y, double z, float power,
                         bool causesFire, bool breakBlocks);

    /**
     * Get block explosion resistance. Java reference: Block.getExplosionResistance()
     * Resistance values from vanilla 1.7.10.
     */
    static float getBlockExplosionResistance(int32_t blockId);

    /**
     * Store sign text at a position and broadcast S33 UpdateSign to all players.
     * Java reference: TileEntitySign + NetHandlerPlayServer.processUpdateSign()
     */
    void setSignText(int32_t x, int32_t y, int32_t z,
                     const std::string& l1, const std::string& l2,
                     const std::string& l3, const std::string& l4);

    /**
     * Send sign text at a position to a single player (used on chunk load).
     */
    void sendSignToPlayer(PlayHandler& handler, Connection& conn,
                          int32_t x, int32_t y, int32_t z);

    /**
     * Broadcast S2A Particle to all connected players.
     * Java reference: WorldServer.spawnParticle()
     */
    void broadcastParticle(const std::string& particleName,
                           float x, float y, float z,
                           float offsetX, float offsetY, float offsetZ,
                           float speed, int32_t count);

    /**
     * Broadcast S28 Effect to all connected players.
     * Java reference: WorldServer.playAuxSFXAtEntity() / WorldManager.playAuxSFX()
     * effectId 2001 = block break particles, data = blockID
     * effectId 2002 = splash potion, data = potionID
     * effectId 1000 = click sound, 1001 = fire charge, 1003 = door, 1005 = bone meal
     */
    void broadcastEffect(int32_t effectId, int32_t x, int32_t y, int32_t z,
                          int32_t data, bool disableRelativeVolume = false);

    /**
     * Open a brewing stand container for a player.
     * Java reference: TileEntityBrewingStand + ContainerBrewingStand
     */
    struct BrewingStandData {
        std::array<std::optional<ItemStack>, 4> slots; // 0-2 = potion, 3 = ingredient
        int32_t brewTime = 0;
    };
    BrewingStandData& getOrCreateBrewingStand(int64_t posKey);
    std::map<int64_t, BrewingStandData> brewingStandStorage_;
    mutable std::mutex brewingStandMutex_;

    /**
     * Dispenser/Dropper storage (9 slots each).
     * Java reference: TileEntityDispenser
     */
    struct DispenserData {
        std::array<std::optional<ItemStack>, 9> slots;
    };
    DispenserData& getOrCreateDispenser(int64_t posKey);
    std::map<int64_t, DispenserData> dispenserStorage_;
    mutable std::mutex dispenserMutex_;

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
     * Get or create a chest inventory at the given block position.
     * Returns a reference to 27-slot array for the chest at (x,y,z).
     * Creates empty storage on first access.
     * Java reference: TileEntityChest — simplified to in-memory storage.
     */
    std::array<std::optional<ItemStack>, 27>& getOrCreateChest(int32_t x, int32_t y, int32_t z);

    /**
     * Get or create a furnace at the given block position.
     * Java reference: TileEntityFurnace
     */
    struct FurnaceData {
        std::optional<ItemStack> slots[3]; // 0=input, 1=fuel, 2=output
        int32_t furnaceBurnTime = 0;
        int32_t currentItemBurnTime = 0;
        int32_t furnaceCookTime = 0;

        bool isBurning() const { return furnaceBurnTime > 0; }

        // Java: TileEntityFurnace.getItemBurnTime(ItemStack)
        static int32_t getItemBurnTime(const std::optional<ItemStack>& stack) {
            if (!stack) return 0;
            int32_t id = stack->getItemId();
            // Wooden slabs (id=44 meta>=0? actually 126)
            if (id == 126) return 150; // wooden_slab
            // Wood-based blocks: planks(5), logs(17,162), bookshelf(47), chest(54),
            // crafting_table(58), noteblock(25), jukebox(84), fence(85), stairs(53,134,135,136)
            if (id == 5 || id == 17 || id == 162 || id == 47 || id == 54 ||
                id == 58 || id == 25 || id == 84 || id == 85 ||
                id == 53 || id == 134 || id == 135 || id == 136 ||
                id == 163 || id == 164 || id == 107 ||
                id == 96  || id == 167) return 300; // wood material
            if (id == 173) return 16000; // coal_block
            // Wooden tools (268-271=sword/shovel/pick/axe, 290=hoe)
            if (id == 268 || id == 269 || id == 270 || id == 271 || id == 290) return 200;
            if (id == 280) return 100;  // stick
            if (id == 263) return 1600; // coal
            if (id == 327) return 20000; // lava_bucket
            if (id == 6) return 100;    // sapling
            if (id == 369) return 2400; // blaze_rod
            return 0;
        }

        bool canSmelt() const {
            if (!slots[0]) return false;
            auto result = FurnaceRecipes::instance().getSmeltingResult(
                slots[0]->getItemId(), slots[0]->getDamage());
            if (!result) return false;
            if (!slots[2]) return true;
            if (slots[2]->getItemId() != result->getItemId() ||
                slots[2]->getDamage() != result->getDamage()) return false;
            return slots[2]->getStackSize() < 64 &&
                   slots[2]->getStackSize() < result->getMaxStackSize();
        }

        void smeltItem() {
            if (!canSmelt()) return;
            auto result = FurnaceRecipes::instance().getSmeltingResult(
                slots[0]->getItemId(), slots[0]->getDamage());
            if (!result) return;
            if (!slots[2]) {
                slots[2] = result->copy();
            } else {
                slots[2]->setStackSize(slots[2]->getStackSize() + 1);
            }
            int32_t sz = slots[0]->getStackSize() - 1;
            if (sz <= 0) slots[0] = std::nullopt;
            else slots[0]->setStackSize(sz);
        }

        // Returns true if state changed (dirty)
        bool tick() {
            bool wasBurning = furnaceBurnTime > 0;
            bool dirty = false;
            if (furnaceBurnTime > 0) --furnaceBurnTime;
            if (furnaceBurnTime != 0 || (slots[1] && slots[0])) {
                if (furnaceBurnTime == 0 && canSmelt()) {
                    currentItemBurnTime = furnaceBurnTime = getItemBurnTime(slots[1]);
                    if (furnaceBurnTime > 0) {
                        dirty = true;
                        if (slots[1]) {
                            int32_t sz = slots[1]->getStackSize() - 1;
                            if (sz <= 0) slots[1] = std::nullopt;
                            else slots[1]->setStackSize(sz);
                        }
                    }
                }
                if (isBurning() && canSmelt()) {
                    ++furnaceCookTime;
                    if (furnaceCookTime == 200) {
                        furnaceCookTime = 0;
                        smeltItem();
                        dirty = true;
                    }
                } else {
                    furnaceCookTime = 0;
                }
            }
            if (wasBurning != (furnaceBurnTime > 0)) dirty = true;
            return dirty;
        }
    };
    FurnaceData& getOrCreateFurnace(int32_t x, int32_t y, int32_t z);
    void tickFurnaces();

    /** Pack block position into int64 key for tile entity storage. */
    static int64_t packBlockPos(int32_t x, int32_t y, int32_t z) {
        return (static_cast<int64_t>(x) << 40) |
               ((static_cast<int64_t>(z) & 0xFFFFF) << 20) |
               (static_cast<int64_t>(y) & 0xFFFFF);
    }

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
     * Broadcast S0B Animation (e.g. arm swing) to all players except the source.
     * Java reference: WorldServer.broadcastAnimation() → S0BPacketAnimation
     */
    void broadcastAnimation(int32_t entityId, uint8_t animationType);

    /**
     * Broadcast S1C EntityMetadata flags byte (sneaking/sprinting) to all players except the source.
     * Java reference: DataWatcher index 0 byte → S1CPacketEntityMetadata
     */
    void broadcastEntityMetadataFlags(int32_t entityId, uint8_t flags);

    // Broadcast S04 EntityEquipment for a player to all other players
    void broadcastEquipment(PlayHandler& handler, int16_t equipSlot);

    /**
     * Handle a player attacking another entity (player-to-player combat).
     * Java reference: EntityPlayerMP.attackTargetEntityWithCurrentItem()
     */
    void handlePlayerAttack(PlayHandler& attacker, Connection& attackerConn, int32_t targetEntityId);

    /**
     * Teleport a player to a position. Sends S08 PlayerPosAndLook.
     * Java reference: EntityPlayerMP.setPositionAndUpdate()
     */
    void teleportPlayer(const std::string& playerName, double x, double y, double z);

    /**
     * Set world time for all worlds and broadcast S03 TimeUpdate.
     * Java reference: CommandTime → WorldServer.setWorldTime()
     */
    void setWorldTime(int64_t time);

    /**
     * Add to world time for all worlds and broadcast S03 TimeUpdate.
     * Java reference: CommandTime → WorldServer.setWorldTime(getWorldTime()+val)
     */
    void addWorldTime(int64_t amount);

    /**
     * Kill a player (set health to 0, trigger death).
     * Java reference: CommandKill → EntityPlayerMP.attackEntityFrom(DamageSource.OUT_OF_WORLD)
     */
    void killPlayer(const std::string& playerName);

    /**
     * Set a player's gamemode and send S2B ChangeGameState.
     * Java reference: EntityPlayerMP.setGameType()
     */
    void setPlayerGameMode(const std::string& playerName, int32_t gameMode);

    /**
     * Get a list of all online player names.
     * Java reference: ServerConfigurationManager.func_152600_g()
     */
    std::vector<std::string> getOnlinePlayerNames() const;

    /**
     * Get a player's current position by name.
     * Java reference: EntityPlayerMP.posX/posY/posZ
     */
    std::optional<PlayerPosition> getPlayerPosition(const std::string& playerName) const;

    /**
     * Give an item to a player by adding to their inventory + sending S2F.
     * Java reference: CommandGive.processCommand() → EntityItem spawn
     */
    void givePlayerItem(const std::string& playerName, int32_t itemId, int32_t amount, int32_t damage);

    /**
     * Set weather state for all worlds and broadcast S2B ChangeGameState.
     * Java reference: CommandWeather.processCommand() → WorldInfo.setRaining/setThundering
     * @param mode 0=clear, 1=rain, 2=thunder
     * @param durationTicks duration in ticks (0 = use default random)
     */
    void setWeather(int32_t mode, int32_t durationTicks);

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
    std::atomic<int64_t> tickCounter_{0}; // Total ticks since server start (world age)
    using Clock = std::chrono::steady_clock;

    // ─── Item entities ──────────────────────────────────────────────────
    struct DroppedItem {
        EntityItem entity;
        int64_t spawnTick = 0;
    };
    mutable std::mutex itemEntitiesMutex_;
    std::vector<DroppedItem> itemEntities_;
    std::atomic<int32_t> nextItemEntityId_{100000};

    // ─── Sign storage ────────────────────────────────────────────────────
    // Java reference: TileEntitySign
    struct SignData {
        std::string lines[4];
    };
    mutable std::mutex signsMutex_;
    std::map<int64_t, SignData> signs_;  // packed pos → text

    // ─── Mob entities ───────────────────────────────────────────────
    // Java reference: SpawnerAnimals.findChunksForSpawning()
    struct SpawnedMob {
        int32_t entityId = 0;
        uint8_t mobType = 0;     // Entity type ID (54=zombie, 51=skel, 50=creeper, 52=spider)
        double posX = 0, posY = 0, posZ = 0;
        float yaw = 0, pitch = 0;
        float health = 20.0f;
        int64_t spawnTick = 0;
        bool isDead = false;
    };
    mutable std::mutex mobEntitiesMutex_;
    std::vector<SpawnedMob> mobEntities_;
    std::atomic<int32_t> nextMobEntityId_{200000};
    static constexpr int MAX_HOSTILE_MOBS = 70;  // Java: EnumCreatureType.MONSTER.maxNumber

    void spawnNaturalMobs();
    void tickMobs();

    // ─── Chest storage (in-memory tile entities) ─────────────────────
    // Key: packed position (x << 40 | (z & 0xFFFFF) << 20 | (y & 0xFFFFF))
    mutable std::mutex chestMutex_;
    std::map<int64_t, std::array<std::optional<ItemStack>, 27>> chestStorage_;

    // ─── Furnace storage (in-memory tile entities) ───────────────────
    mutable std::mutex furnaceMutex_;
    std::map<int64_t, FurnaceData> furnaceStorage_;
};

} // namespace mccpp
