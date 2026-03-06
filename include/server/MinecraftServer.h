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
#include "entity/EntityMinecart.h"
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
#include "scoreboard/Scoreboard.h"
#include "server/MerchantRecipe.h"

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
    friend class PlayHandler;  // Needs access to mob/connection internals for horse inventory, vehicle dismount
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
    WorldServer* getWorldForDimension(int dim);

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
        int32_t ingredientId = 0; // Java: ingredientID — tracks ingredient during brew
    };
    BrewingStandData& getOrCreateBrewingStand(int64_t posKey);
    void tickBrewingStands();
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
     * Hopper storage (5 slots + transfer cooldown).
     * Java reference: TileEntityHopper — 8-tick cooldown, pull up/push down
     */
    struct HopperData {
        std::array<std::optional<ItemStack>, 5> slots;
        int32_t transferCooldown = -1;
        bool isOnCooldown() const { return transferCooldown > 0; }
    };
    HopperData& getOrCreateHopper(int64_t posKey);
    void tickHoppers();
    std::map<int64_t, HopperData> hopperStorage_;
    mutable std::mutex hopperMutex_;

    /**
     * Mob spawner tile entity storage.
     * Java reference: TileEntityMobSpawner + MobSpawnerBaseLogic
     * Spawns mobs periodically when a player is within activatingRange.
     */
    struct SpawnerData {
        std::string entityId = "Pig";   // Java: mobID — entity name to spawn
        int16_t spawnDelay = 20;        // Java: spawnDelay — current countdown (ticks)
        int16_t minDelay = 200;         // Java: minSpawnDelay
        int16_t maxDelay = 800;         // Java: maxSpawnDelay
        int16_t spawnCount = 4;         // Java: spawnCount — entities per activation
        int16_t maxNearby = 6;          // Java: maxNearbyEntities
        int16_t activatingRange = 16;   // Java: activatingRangeFromPlayer
        int16_t spawnRange = 4;         // Java: spawnRange (XZ half-extent)
    };
    SpawnerData& getOrCreateSpawner(int64_t posKey);
    void tickMobSpawners();
    std::map<int64_t, SpawnerData> spawnerStorage_;
    mutable std::mutex spawnerMutex_;

    /**
     * Save all world chunks to disk.\n     * Java reference: MinecraftServer.saveAllWorlds()\n     */
    void saveAllWorlds();

    /**
     * Inject tile entity NBT data for a chunk into its Level compound.
     * Java reference: AnvilChunkLoader.writeChunkToNBT() — TileEntities section
     */
    void saveTileEntitiesForChunk(nbt::NBTTagCompound& levelTag, int chunkX, int chunkZ);

    /**
     * Load tile entity data from a chunk's Level compound into server storage maps.
     * Java reference: AnvilChunkLoader.readChunkFromNBT() — TileEntities section
     */
    void loadTileEntitiesFromChunk(const nbt::NBTTagCompound& levelTag);

    /**
     * Spawn a dropped item entity at the given position.
     * Broadcasts S0E + S1C to all connected players.
     * Returns the entity ID of the spawned item.
     */
    int32_t spawnItemDrop(double x, double y, double z,
                          int32_t blockId, int32_t metadata, int32_t count = 1);

    /**
     * Spawn a dropped item entity from an ItemStack (preserves NBT: enchantments, names, etc.)
     * Java reference: EntityPlayer.func_146097_a — drops individual item stacks on death
     * @param motionX,motionY,motionZ custom velocity (Java: random ±0.1, 0.2, ±0.1)
     */
    int32_t spawnItemDropStack(double x, double y, double z, const ItemStack& stack,
                               double motionX = 0.0, double motionY = 0.2, double motionZ = 0.0);

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
    void broadcastAttachEntity(int32_t leashId, int32_t riderId, int32_t vehicleId);

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
    void handleEntityInteract(PlayHandler& player, Connection& conn, int32_t targetEntityId);
    /** Boost speed of pig ridden by this player. Java: ItemCarrotOnAStick.onItemRightClick → boostSpeed() */
    bool boostRiddenPig(int32_t riderEntityId);

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
     * Add XP levels to a player (can be negative).
     * Java reference: EntityPlayer.addExperienceLevel()
     */
    void addPlayerLevels(const std::string& playerName, int32_t levels);

    /**
     * Add XP points to a player.
     * Java reference: EntityPlayer.addExperience()
     */
    void addPlayerExperience(const std::string& playerName, int32_t amount);

    /** Enchant a player's held item with the given enchantment ID and level. */
    void enchantPlayerItem(const std::string& playerName, int32_t enchId, int32_t level);

    /** Clear a player's inventory. Returns number of items cleared.
     *  If itemId >= 0, only clear items matching that ID (and damage if >= 0). */
    int32_t clearPlayerInventory(const std::string& playerName, int32_t itemId = -1, int32_t damage = -1);

    /** Set a player's individual spawn point (bed spawn). */
    void setPlayerSpawnPoint(const std::string& playerName, int32_t x, int32_t y, int32_t z);

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

    /** Check if it's currently raining. */
    bool isRaining() const;

    /** Set the default game mode for new players. */
    void setDefaultGameMode(int32_t mode) { defaultGameMode_ = mode; }
    int32_t getDefaultGameMode() const { return defaultGameMode_; }

    /** Broadcast chat message to all players. */
    void broadcastChat(const std::string& message) { broadcastChatMessage(message); }

    /** Send a private chat message to a specific player by name. */
    void sendPrivateMessage(const std::string& playerName, const std::string& message);

    /** Kick a player by name with a reason message. */
    void kickPlayer(const std::string& playerName, const std::string& reason);

    /** Set a block in the world and broadcast the change to all players. */
    void setBlockInWorld(int32_t x, int32_t y, int32_t z, int32_t blockId, int32_t meta = 0);

    /**
     * Trigger redstone signal propagation around a block position.
     * Java reference: World.notifyBlocksOfNeighborChange() → BlockRedstoneWire.func_150177_e
     * Called after lever toggle, wire placement/break, torch/repeater state changes.
     * Updates wire power levels, toggles redstone lamps, and schedules torch/repeater ticks.
     */
    void redstoneNotifyNeighbors(int32_t x, int32_t y, int32_t z);

    /**
     * Piston extend/retract — Java: BlockPistonBase.updatePistonState()
     * Called from redstoneNotifyNeighbors when a piston (33/29) detects power change.
     * Handles up to 12/block push limit, sticky piston pull, and piston head placement.
     */
    void pistonUpdateState(int32_t x, int32_t y, int32_t z, int32_t blockId);

    /**
     * Dispenser/Dropper fire — Java: BlockDispenser.func_149941_e()
     * Called from redstoneNotifyNeighbors when dispenser (23) or dropper (158) receives power.
     * Picks random occupied slot, dispenses item: arrows shoot, droppers drop/inject.
     */
    void dispenserFire(int32_t x, int32_t y, int32_t z, int32_t blockId);

    /**
     * Play note block at position — Java: TileEntityNote.triggerNote()
     * Detects instrument from block below, plays sound and note particle.
     * Used by right-click activation and redstone rising-edge trigger.
     */
    void playNoteBlock(int32_t x, int32_t y, int32_t z);

    /**
     * Tripwire hook chain update — Java: BlockTripWireHook.func_150136_a()
     * Scans from a hook position along its facing direction for up to 42 blocks,
     * looking for an opposing hook. Updates the connected/powered state of both
     * hooks and all intermediate wire blocks. Called on hook/wire place or break.
     * @param hookX,hookY,hookZ Position of the tripwire hook being updated
     * @param isBeingRemoved True when called from block break (hook position is already air)
     * @param blockMeta The metadata of the hook (direction + flags), needed when isBeingRemoved
     */
    void tripwireHookUpdate(int32_t hookX, int32_t hookY, int32_t hookZ,
                            bool isBeingRemoved = false, int32_t blockMeta = -1);

    /** Get block ID at position, returns 0 if unloaded. */
    int32_t getBlockIdInWorld(int32_t x, int32_t y, int32_t z) const;

    /** Get block metadata at position. */
    int32_t getBlockMetaInWorld(int32_t x, int32_t y, int32_t z) const;

    /**
     * Get comparator signal strength from a container at position.
     * Java reference: Block.getComparatorInputOverride() + Container.calcRedstoneFromInventory()
     * Returns 0-15 based on fill ratio, or -1 if block has no comparator override.
     */
    int32_t getComparatorContainerSignal(int32_t x, int32_t y, int32_t z) const;

    /**
     * Compute daylight sensor power level (0-15) from world time.
     * Java reference: BlockDaylightDetector.func_149957_e()
     * Normal sensor (151) emits signal during day, Inverted (178) emits at night.
     */
    int32_t getDaylightSensorPower(bool inverted) const;

    /** Summon a mob at position. Returns entity ID or -1 on failure. */
    int32_t summonMob(uint8_t mobType, double x, double y, double z);

    /**
     * Apply a potion effect to a player by name.
     * Java reference: EntityLivingBase.addPotionEffect()
     */
    void applyPlayerPotionEffect(const std::string& playerName, int32_t effectId,
                                  int32_t durationTicks, int32_t amplifier);

    /**
     * Clear all potion effects from a player by name.
     * Java reference: EntityLivingBase.clearActivePotions()
     */
    void clearPlayerPotionEffects(const std::string& playerName);

    // ─── Scoreboard ─────────────────────────────────────────────────────
    // Java reference: ServerScoreboard — broadcasting scoreboard state

    /** Get the server scoreboard. Java: MinecraftServer.getWorldScoreboard() */
    Scoreboard& getScoreboard() { return scoreboard_; }
    const Scoreboard& getScoreboard() const { return scoreboard_; }

    /**
     * Broadcast S3B ScoreboardObjective to all players.
     * Java: ServerScoreboard.func_96522_a / func_96532_b / func_96533_c
     * @param mode 0=create, 1=remove, 2=update display name
     */
    void broadcastScoreboardObjective(const std::string& objName, const std::string& displayName, int8_t mode);

    /**
     * Broadcast S3C UpdateScore to all players.
     * Java: ServerScoreboard.func_96536_a (update) / func_96516_a (remove)
     * @param action 0=create/update, 1=remove
     */
    void broadcastUpdateScore(const std::string& playerName, const std::string& objName,
                              int32_t value, int8_t action);

    /**
     * Broadcast S3C UpdateScore (remove all scores for a player).
     * Java: S3CPacketUpdateScore(String) constructor — removes all scores for itemName.
     */
    void broadcastRemoveScore(const std::string& playerName);

    /**
     * Broadcast S3D DisplayScoreboard to all players.
     * Java: ServerScoreboard.setObjectiveInDisplaySlot
     */
    void broadcastDisplayScoreboard(int8_t position, const std::string& objName);

    /**
     * Broadcast S3E Teams to all players.
     * Java: ServerScoreboard.broadcastTeamCreated/broadcastTeamRemoved/func_96513_c
     * @param mode 0=create, 1=remove, 2=update, 3=add players, 4=remove players
     */
    void broadcastTeams(const ScorePlayerTeam& team, int8_t mode,
                        const std::vector<std::string>& players = {});

    /**
     * Send full scoreboard state to a single player (used on join).
     * Java: ServerScoreboard.func_96549_e — sends objectives, scores, display slots, and teams.
     */
    void sendScoreboardState(Connection& conn);

private:
    Scoreboard scoreboard_;
    int32_t defaultGameMode_ = 0; // Java: MinecraftServer.defaultGameType (0=survival)

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

    mutable std::recursive_mutex connectionsMutex_;
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

    // ─── Skull storage ───────────────────────────────────────────────────
    // Java reference: TileEntitySkull — stores skull type per position
    // Skull types: 0=skeleton, 1=wither_skeleton, 2=zombie, 3=player, 4=creeper
    mutable std::mutex skullsMutex_;
    std::map<int64_t, int32_t> skulls_;  // packed pos → skullType

    // ─── Jukebox disc storage ────────────────────────────────────────────
    // Java reference: TileEntityJukebox — stores which disc item is in each jukebox
    mutable std::mutex jukeboxDiscsMutex_;
    std::map<int64_t, int32_t> jukeboxDiscs_;  // packed pos → disc item ID (2256-2267)

    // ─── Flower pot contents storage ─────────────────────────────────────
    // Java reference: TileEntityFlowerPot — stores which plant item ID + meta is in each pot
    mutable std::mutex flowerPotsMutex_;
    std::map<int64_t, std::pair<int32_t, int32_t>> flowerPots_;  // packed pos → {itemId, itemMeta}

    static void generateVillagerTrades(int profession, std::vector<MerchantRecipe>& out);

    // ─── Mob entities ───────────────────────────────────────────────
    // Java reference: SpawnerAnimals.findChunksForSpawning()
    struct SpawnedMob {
        int32_t entityId = 0;
        uint8_t mobType = 0;     // Entity type ID (54=zombie, 51=skel, 50=creeper, 52=spider)
        double posX = 0, posY = 0, posZ = 0;
        float yaw = 0, pitch = 0;
        float health = 20.0f;
        int64_t spawnTick = 0;
        int64_t lastAttackTick = 0;  // Cooldown between attacks (20 ticks = 1s)
        bool isDead = false;
        bool isOnFire = false;      // Tracks fire state for cooked drops
        int32_t fuseTicks = 0;      // Creeper fuse timer (30 ticks = 1.5s)
        int32_t attackCooldown = 0; // Skeleton arrow cooldown
        int32_t angerLevel = 0;     // Zombie Pigman anger (Java: EntityPigZombie.angerLevel)
        int32_t angerTarget = -1;   // Entity ID of anger target
        // Passive mob AI — Java: EntityCreature wander task
        bool isPassive = false;     // Passive mobs don't despawn
        int32_t wanderCooldown = 0; // Ticks until next wander direction change
        float wanderYaw = 0.0f;     // Current wander heading
        // Movement tracking — Java: EntityTrackerEntry last sent position
        int32_t lastSentPosX = 0, lastSentPosY = 0, lastSentPosZ = 0;
        int32_t ticksSinceLastTeleport = 0;
        // Sheep state — Java: EntitySheep dataWatcher(16)
        int32_t fleeceColor = 0;  // 0=white, 1=orange, ... 15=black
        bool isSheared = false;   // true = wool already sheared off
        // Pig state — Java: EntityPig dataWatcher(16) bit 0x01
        bool isSaddled = false;   // true = pig has saddle
        // ─── Rider state (pig riding) — Java: EntityLiving.riddenByEntity ───
        int32_t riderEntityId = -1;   // Entity ID of riding player (-1 = none)
        // Java: EntityAIControlledByPlayer fields
        float pigCurrentSpeed = 0.0f;      // Java: currentSpeed (ramps 0→0.3)
        bool pigSpeedBoosted = false;      // Java: speedBoosted
        int32_t pigSpeedBoostTime = 0;     // Java: speedBoostTime
        int32_t pigMaxSpeedBoostTime = 0;  // Java: maxSpeedBoostTime
        // Breeding state — Java: EntityAnimal.inLove + EntityAIMate
        int32_t inLoveTicks = 0;    // 600 ticks (30s) when fed breeding item
        int32_t breedCooldown = 0;  // 6000 ticks (5 min) after breeding
        int32_t breedingCounter = 0; // Proximity mating countdown (60 ticks)
        int32_t mateEntityId = -1;  // Entity ID of the mate being approached
        // Wolf state — Java: EntityWolf / EntityTameable DataWatcher
        bool isTamed = false;           // DataWatcher 16 bit 0x04
        bool isSitting = false;         // DataWatcher 16 bit 0x01
        bool isAngry = false;           // DataWatcher 16 bit 0x02
        std::string ownerUuid;          // DataWatcher 17 (String, owner UUID)
        int32_t collarColor = 14;       // DataWatcher 20 (byte, default=14=orange, Java: BlockColored.func_150032_b(1))
        int32_t wolfAttackTarget = -1;  // Entity ID of current attack target
        int32_t wolfFollowTicks = 0;    // Follow-owner movement timer
        // Ocelot/Cat state — Java: EntityOcelot DataWatcher(18)
        int32_t catSkinType = 0;        // 0=wild ocelot, 1=tuxedo, 2=tabby, 3=siamese
        // Horse state — Java: EntityHorse DataWatcher
        int32_t horseType = 0;           // DW 19 byte: 0=horse, 1=donkey, 2=mule, 3=zombie horse, 4=skeleton horse
        int32_t horseVariant = 0;        // DW 20 int: low byte=color(0-6), high byte=marking(0-4)
        int32_t horseTemper = 0;         // Java: EntityHorse.temper (0-100, maxTemper=100)
        int32_t horseArmorIndex = 0;     // DW 22 int: 0=none, 1=iron(+5), 2=gold(+7), 3=diamond(+11)
        bool isHorseSaddled = false;     // DW 16 bit 4
        bool isHorseChested = false;     // DW 16 bit 8 (donkey/mule only)
        float horseJumpPower = 0.0f;     // Java: EntityHorse.horseJumpPower (0.0-1.0, set by C0B action 6)
        double horseMotionY = 0.0;       // Java: EntityHorse.motionY — vertical velocity for jump physics
        double horseJumpStrength = 0.68; // Java: horseJumpStrength attribute — default ~0.7, range [0.4, 1.0]
        bool horseOnGround = true;       // Java: EntityHorse.onGround — for jump gating
        bool horseIsJumping = false;     // Java: EntityHorse.isHorseJumping() — mid-jump flag
        int32_t gallopTime = 0;          // Java: EntityHorse.gallopTime — for gallop sound interval
        std::optional<ItemStack> horseChestInventory[15]; // Java: AnimalChest slots 2-16 (chested donkey/mule)
        // Villager state — Java: EntityVillager DataWatcher(16)
        int32_t villagerProfession = 0;  // DW 16 int: 0=farmer, 1=librarian, 2=priest, 3=blacksmith, 4=butcher
        std::vector<MerchantRecipe> villagerTrades;  // Generated on first interaction
        bool villagerTradesGenerated = false;
        // ─── Ender Dragon state — Java: EntityDragon ───
        // Ring buffer for smooth movement interpolation (64 entries of [yaw, posY])
        double dragonRingBuffer[64][3] = {};  // Java: EntityDragon.ringBuffer[64][3]
        int32_t dragonRingIndex = -1;          // Java: EntityDragon.ringBufferIndex
        // Flight target — Java: EntityDragon.targetX/Y/Z
        double dragonTargetX = 0, dragonTargetY = 100.0, dragonTargetZ = 0;
        int32_t dragonTargetEntityId = -1;     // Entity ID of target player (-1 = none)
        // Motion — Java: EntityLivingBase.motionX/Y/Z
        double dragonMotionX = 0, dragonMotionY = 0, dragonMotionZ = 0;
        // Animation — Java: EntityDragon.animTime, prevAnimTime
        float dragonAnimTime = 0.0f;
        float dragonPrevAnimTime = 0.0f;
        float dragonRandomYawVelocity = 0.0f; // Java: EntityLiving.randomYawVelocity
        bool dragonForceNewTarget = false;     // Java: EntityDragon.forceNewTarget
        bool dragonSlowed = false;             // Java: EntityDragon.slowed
        // Death sequence — Java: EntityDragon.deathTicks
        int32_t dragonDeathTicks = 0;          // 0 = alive, 1-200 = dying
        // Crystal healing — Java: EntityDragon.healingEnderCrystal
        int32_t dragonHealingCrystalId = -1;   // Entity ID of linked ender crystal (-1 = none)
        int32_t dragonCrystalSearchTimer = 0;  // Ticks until next crystal search
        // ─── Wither Boss state — Java: EntityWither ───
        int32_t witherInvulTime = 0;               // DW 20: 220 on spawn, counts down
        int32_t witherWatchedTargets[3] = {};       // DW 17,18,19: entity IDs of center+side head targets
        float witherSideHeadYaw[2] = {};            // Java: field_82221_e — side head yaw
        float witherSideHeadPitch[2] = {};          // Java: field_82220_d — side head pitch
        float witherPrevSideHeadYaw[2] = {};        // Java: field_82218_g
        float witherPrevSideHeadPitch[2] = {};      // Java: field_82217_f
        int32_t witherSideAttackTimer[2] = {};      // Java: field_82223_h — next attack tick
        int32_t witherSideChargeCounter[2] = {};    // Java: field_82224_i — charged shot counter
        int32_t witherBlockBreakTimer = 0;          // Java: field_82222_j
        double witherMotionX = 0, witherMotionY = 0, witherMotionZ = 0;
        int32_t witherTicksExisted = 0;             // Java: ticksExisted counter
        bool isWitherArmored() const { return health <= 150.0f; } // <= maxHealth/2
        // ─── Snow Golem state — Java: EntitySnowman ───
        int32_t snowGolemAttackCooldown = 0;        // Java: EntityAIArrowAttack attackTime (20 tick interval)
        // ─── Bat state — Java: EntityBat ───
        bool isBatHanging = true;                   // DW 16 bit 0x01: true = hanging from ceiling
        double batMotionX = 0, batMotionY = 0, batMotionZ = 0; // Java: motionX/Y/Z
        // Java: EntityBat.spawnPosition — random flight target
        int32_t batTargetX = 0, batTargetY = 0, batTargetZ = 0;
        bool batHasTarget = false;                  // true if spawnPosition is set
        // ─── Squid state — Java: EntitySquid ───
        float squidRotation = 0.0f;                 // Java: squidRotation — pulse cycle phase
        float squidRotationVelocity = 0.2f;         // Java: rotationVelocity — cycle speed
        float squidRandomMotionSpeed = 0.0f;        // Java: randomMotionSpeed — propulsion
        float squidField70871 = 0.0f;               // Java: field_70871_bB — tentacle animation speed
        float squidRandomMotionVecX = 0.0f;         // Java: randomMotionVecX
        float squidRandomMotionVecY = 0.0f;         // Java: randomMotionVecY
        float squidRandomMotionVecZ = 0.0f;         // Java: randomMotionVecZ
        double squidMotionX = 0, squidMotionY = 0, squidMotionZ = 0; // Java: motionX/Y/Z
        int32_t squidEntityAge = 0;                 // Java: entityAge — direction reset timer
        // ─── Silverfish state — Java: EntitySilverfish ───
        int32_t allySummonCooldown = 0;              // Java: allySummonCooldown — 20 ticks after hit, triggers monster_egg search
        // ─── Skeleton type — Java: EntitySkeleton DataWatcher(13) ───
        int32_t skeletonType = 0;                    // 0=normal skeleton (bow), 1=wither skeleton (sword, Wither I on hit)
        // ─── Leash state — Java: EntityLiving.isLeashed / leashedToEntity ───
        bool isLeashed = false;                      // Java: EntityLiving.isLeashed
        int32_t leashedToEntityId = -1;              // Entity ID of leash holder (player or leash knot, -1 = none)
        // ─── Custom name — Java: EntityLiving DataWatcher(10) string, DataWatcher(11) byte ───
        std::string customName;                      // Java: EntityLiving.getCustomNameTag() — DW 10
        bool alwaysShowName = false;                 // Java: EntityLiving.getAlwaysRenderNameTag() — DW 11
        bool isPersistenceRequired = false;          // Java: EntityLiving.persistenceRequired — prevents despawn
    };
    mutable std::mutex mobEntitiesMutex_;
    std::vector<SpawnedMob> mobEntities_;
    std::atomic<int32_t> nextMobEntityId_{200000};
    static constexpr int MAX_HOSTILE_MOBS = 70;  // Java: EnumCreatureType.MONSTER.maxNumber
    static constexpr int MAX_PASSIVE_MOBS = 10;  // Java: EnumCreatureType.creature.maxNumber
    static constexpr int MAX_WATER_MOBS = 5;     // Java: EnumCreatureType.waterCreature.maxNumber

    // ─── Leash knot entities ────────────────────────────────────────────
    // Java reference: EntityLeashKnot — spawned on fence posts when tying mobs with leads
    struct SpawnedLeashKnot {
        int32_t entityId = 0;
        int32_t blockX = 0, blockY = 0, blockZ = 0;  // Java: field_146063_b/c/d (fence position)
        bool isDead = false;
        int32_t tickCounter = 0;  // Java: EntityHanging.tickCounter1 — check onValidSurface every 100 ticks
    };
    mutable std::mutex leashKnotEntitiesMutex_;
    std::vector<SpawnedLeashKnot> leashKnotEntities_;
    std::atomic<int32_t> nextLeashKnotEntityId_{1500000};

    void spawnNaturalMobs();
    void spawnPassiveMobs();
    void spawnWaterMobs();
    void tickMobs();
    /** Tick leash knot entities — Java: EntityHanging.onUpdate() + EntityLeashKnot.onValidSurface() */
    void tickLeashKnots();
    void tickRandomBlocks();
    /** Tick a single Ender Dragon — Java: EntityDragon.onLivingUpdate() + onDeathUpdate() */
    void tickDragon(SpawnedMob& dragon, int64_t currentTick);
    /** Tick a single Wither Boss — Java: EntityWither.onLivingUpdate() + updateAITasks() */
    void tickWither(SpawnedMob& wither, int64_t currentTick);
    /** Tick a single Snow Golem — Java: EntitySnowman.onLivingUpdate() + ranged attack AI */
    void tickSnowGolem(SpawnedMob& golem, int64_t currentTick);
    /** Tick a single Bat — Java: EntityBat.onUpdate() + updateAITasks() */
    void tickBat(SpawnedMob& bat, int64_t currentTick);
    /** Tick a single Squid — Java: EntitySquid.onLivingUpdate() + updateEntityActionState() */
    void tickSquid(SpawnedMob& squid, int64_t currentTick);
    /** Tick a single Silverfish — Java: EntitySilverfish.updateEntityActionState() */
    void tickSilverfish(SpawnedMob& sf, int64_t currentTick);

public:
    // ─── Arrow projectile entities ──────────────────────────────────
    // Java reference: EntityArrow — projectile with flight physics, block/entity collision
    struct SpawnedArrow {
        int32_t entityId = 0;
        int32_t shooterEntityId = -1;
        double posX = 0, posY = 0, posZ = 0;
        double motionX = 0, motionY = 0, motionZ = 0;
        float yaw = 0, pitch = 0;
        double damage = 2.0;
        int32_t knockbackStrength = 0;
        bool isCritical = false;
        bool isBurning = false;
        bool inGround = false;
        bool isDead = false;
        int32_t ticksInAir = 0;
        int32_t ticksInGround = 0;
        int32_t arrowShake = 0;
        int32_t inBlockId = 0;
        int32_t inBlockMeta = 0;
        int32_t blockX = -1, blockY = -1, blockZ = -1;
        int64_t spawnTick = 0;
        // Java: 0=no pickup, 1=survival pickup, 2=creative-only (Infinity arrows)
        int32_t canBePickedUp = 0;

        static constexpr float GRAVITY = 0.05f;
        static constexpr float AIR_FRICTION = 0.99f;
        static constexpr int32_t GROUND_DESPAWN = 1200;
        static constexpr int32_t SHOOTER_GRACE = 5;
    };
    mutable std::mutex arrowEntitiesMutex_;
    std::vector<SpawnedArrow> arrowEntities_;
    std::atomic<int32_t> nextArrowEntityId_{300000};

    /**
     * Spawn an arrow projectile.
     * Java reference: EntityArrow(world, shooter, speed)
     * @return entity ID of the spawned arrow
     */
    int32_t spawnArrow(double x, double y, double z,
                       double motionX, double motionY, double motionZ,
                       int32_t shooterEntityId, double damage = 2.0,
                       int32_t knockback = 0, bool critical = false,
                       float speed = 0.0f, float inaccuracy = 1.0f);

    /** Tick all arrow projectiles (flight physics, collision, despawn). */
    void tickArrows();

    // ─── Throwable projectile entities ──────────────────────────────
    // Java reference: EntityThrowable — snowball, egg, ender pearl, exp bottle
    enum class ThrowableType : uint8_t {
        Snowball = 0,
        Egg = 1,
        EnderPearl = 2,
        ExpBottle = 3,
        SplashPotion = 4
    };

    struct SpawnedThrowable {
        int32_t entityId = 0;
        int32_t throwerEntityId = -1;
        std::string throwerName;  // For ender pearl teleport
        ThrowableType type = ThrowableType::Snowball;
        double posX = 0, posY = 0, posZ = 0;
        double motionX = 0, motionY = 0, motionZ = 0;
        bool isDead = false;
        int32_t ticksInAir = 0;
        int64_t spawnTick = 0;

        // Java: EntityThrowable physics constants
        static constexpr float GRAVITY = 0.03f;   // vs arrow's 0.05
        static constexpr float AIR_FRICTION = 0.99f;
        static constexpr int32_t MAX_TICKS = 1200; // Despawn after 60 seconds
    };
    mutable std::mutex throwableEntitiesMutex_;
    std::vector<SpawnedThrowable> throwableEntities_;
    std::atomic<int32_t> nextThrowableEntityId_{400000};

    /**
     * Spawn a throwable projectile.
     * Java reference: EntityThrowable(world, thrower) → setThrowableHeading
     * @return entity ID of the spawned throwable
     */
public:
    int32_t spawnThrowable(ThrowableType type, double x, double y, double z,
                           double motionX, double motionY, double motionZ,
                           int32_t throwerEntityId, const std::string& throwerName);

    /** Tick all throwable projectiles (flight physics, collision, despawn). */
    void tickThrowables();

    // ─── Fish hook entities ─────────────────────────────────────────
    // Java reference: EntityFishHook — fishing rod projectile with catch mechanics
    struct SpawnedFishHook {
        int32_t entityId = 0;
        int32_t anglerEntityId = -1;    // Entity ID of the player who cast
        double posX = 0, posY = 0, posZ = 0;
        double motionX = 0, motionY = 0, motionZ = 0;
        float yaw = 0, pitch = 0;
        bool isDead = false;
        bool inGround = false;
        int32_t ticksInGround = 0;      // Despawn after 1200 ticks in ground
        int32_t ticksInAir = 0;

        // Java: EntityFishHook 3-phase catch cycle
        int32_t ticksCaughtDelay = 0;    // Phase 1: initial wait (100-900 ticks)
        int32_t ticksCatchableDelay = 0; // Phase 2: fish approach (20-80 ticks)
        int32_t ticksCatchable = 0;      // Phase 3: bite window (10-30 ticks)
        float fishApproachAngle = 0.0f;  // Angle for approach particle effects

        int64_t spawnTick = 0;

        static constexpr float GRAVITY_WATER = 0.04f;  // buoyancy in water
        static constexpr float AIR_FRICTION = 0.92f;
        static constexpr float WATER_FRICTION = 0.5f;
        static constexpr int32_t GROUND_DESPAWN = 1200;
    };
    mutable std::mutex fishHookEntitiesMutex_;
    std::vector<SpawnedFishHook> fishHookEntities_;
    std::atomic<int32_t> nextFishHookEntityId_{800000};

    /**
     * Spawn a fish hook projectile.
     * Java reference: EntityFishHook(World, EntityPlayer) constructor
     * @return entity ID of the spawned hook
     */
    int32_t spawnFishHook(double x, double y, double z,
                          double motionX, double motionY, double motionZ,
                          int32_t anglerEntityId);

    /** Tick all fish hook entities (water physics, catch cycle, despawn). */
    void tickFishHooks();

    /**
     * Retract a fish hook — Java: EntityFishHook.handleHookRetraction()
     * Returns durability damage to apply to fishing rod (0=nothing, 1=caught fish, 2=ground, 3=entity)
     */
    int32_t retractFishHook(int32_t anglerEntityId);

    // ─── Minecart entities ───────────────────────────────────────────
    // Java reference: EntityMinecart — rail physics, 7 subtypes
    struct SpawnedMinecart {
        int32_t entityId = 0;
        int32_t minecartType = 0;  // 0=empty, 1=chest, 2=furnace, 3=TNT, 4=spawner, 5=hopper, 6=command
        EntityMinecart logic;       // Header-only rail physics engine
        int32_t riderEntityId = -1; // Entity ID of the riding player (-1 = none)
        bool isDead = false;
        int64_t spawnTick = 0;
        // Movement tracking for S15/S18 broadcast
        int32_t lastSentPosX = 0, lastSentPosY = 0, lastSentPosZ = 0;
        int32_t ticksSinceLastTeleport = 0;
    };
    mutable std::mutex minecartEntitiesMutex_;
    std::vector<SpawnedMinecart> minecartEntities_;
    std::atomic<int32_t> nextMinecartEntityId_{500000};

    /** Spawn a minecart entity on a rail and broadcast to clients. */
    int32_t spawnMinecart(int32_t type, double x, double y, double z);

    /** Tick all minecart entities (rail physics, movement broadcast, despawn). */
    void tickMinecarts();

    // ─── Lightning bolt entities ────────────────────────────────────
    // Java reference: EntityLightningBolt — thunder weather effect
    struct SpawnedLightning {
        int32_t entityId = 0;
        EntityLightningBolt logic;  // Header-only state machine
        bool isDead = false;
        int64_t spawnTick = 0;
    };
    mutable std::mutex lightningEntitiesMutex_;
    std::vector<SpawnedLightning> lightningEntities_;
    std::atomic<int32_t> nextLightningEntityId_{600000};

    /** Spawn a lightning bolt at position and broadcast S2C to clients. */
    int32_t spawnLightning(double x, double y, double z);

    /** Tick all lightning bolts (state countdown, fire, entity damage). */
    void tickLightning();

    // ─── Boat entities ──────────────────────────────────────────────────
    // Java reference: EntityBoat — water physics, rider steering, collision breaking
    struct SpawnedBoat {
        int32_t entityId = 0;
        double posX = 0, posY = 0, posZ = 0;
        double motionX = 0, motionY = 0, motionZ = 0;
        float yaw = 0, pitch = 0;
        double prevPosX = 0, prevPosY = 0, prevPosZ = 0;
        double speedMultiplier = 0.07;     // Java: EntityBoat.speedMultiplier
        float damageTaken = 0.0f;          // Java: DataWatcher 19
        int32_t timeSinceHit = 0;          // Java: DataWatcher 17
        int32_t forwardDirection = 1;      // Java: DataWatcher 18
        int32_t riderEntityId = -1;        // Entity ID of riding player (-1 = none)
        bool isDead = false;
        bool onGround = false;
        int64_t spawnTick = 0;
        // Movement tracking for S18 broadcast
        int32_t lastSentPosX = 0, lastSentPosY = 0, lastSentPosZ = 0;
        int32_t ticksSinceLastTeleport = 0;
    };
    mutable std::mutex boatEntitiesMutex_;
    std::vector<SpawnedBoat> boatEntities_;
    std::atomic<int32_t> nextBoatEntityId_{700000};

    /** Spawn a boat entity at position and broadcast S0E to clients. */
    int32_t spawnBoat(double x, double y, double z, float yaw);

    /** Tick all boat entities (water physics, steering, collision, broadcast). */
    void tickBoats();

    // ─── Painting entities ──────────────────────────────────────────────
    // Java reference: EntityPainting — wall-hung decorative entity
    struct SpawnedPainting {
        int32_t entityId = 0;
        int32_t blockX = 0, blockY = 0, blockZ = 0;  // Java: field_146063_b/c/d (tile position)
        int32_t hangingDirection = 0;   // Java: EntityHanging.hangingDirection (0-3)
        std::string artTitle;           // Java: EntityPainting.art.title
        int32_t sizeX = 16, sizeY = 16; // Pixel dimensions (16/32/48/64)
        bool isDead = false;
    };
    mutable std::mutex paintingEntitiesMutex_;
    std::vector<SpawnedPainting> paintingEntities_;
    std::atomic<int32_t> nextPaintingEntityId_{1200000};

    /**
     * Spawn a painting entity on a wall and broadcast S10 to all clients.
     * Java reference: EntityPainting(World, x, y, z, direction) + ItemHangingEntity.onItemUse()
     * @return entity ID of the spawned painting, or -1 on failure
     */
    int32_t spawnPainting(int32_t blockX, int32_t blockY, int32_t blockZ,
                          int32_t hangingDirection, const std::string& artTitle,
                          int32_t sizeX, int32_t sizeY);

    /**
     * Remove a painting entity by ID, broadcast S13, drop item in survival.
     * Java reference: EntityHanging.attackEntityFrom() → onBroken()
     */
    void removePainting(int32_t entityId, bool dropItem);

    // ─── Item frame entities ────────────────────────────────────────────
    // Java reference: EntityItemFrame — wall-hung entity that displays an item
    struct SpawnedItemFrame {
        int32_t entityId = 0;
        int32_t blockX = 0, blockY = 0, blockZ = 0;  // Java: field_146063_b/c/d (tile position)
        int32_t hangingDirection = 0;   // Java: EntityHanging.hangingDirection (0-3)
        // Displayed item — Java: EntityItemFrame.getDisplayedItem() (DataWatcher index 2)
        int32_t displayedItemId = 0;    // 0 = no item
        int32_t displayedItemDamage = 0;
        int32_t displayedItemCount = 0;
        int32_t rotation = 0;           // Java: EntityItemFrame.getRotation() (DataWatcher index 3, 0-7)
        bool isDead = false;
    };
    mutable std::mutex itemFrameEntitiesMutex_;
    std::vector<SpawnedItemFrame> itemFrameEntities_;
    std::atomic<int32_t> nextItemFrameEntityId_{1300000};

    /**
     * Spawn an item frame entity on a wall and broadcast S0E type 71 to all clients.
     * Java reference: EntityItemFrame(World, x, y, z, direction) + ItemHangingEntity.onItemUse()
     * @return entity ID of the spawned item frame, or -1 on failure
     */
    int32_t spawnItemFrame(int32_t blockX, int32_t blockY, int32_t blockZ,
                           int32_t hangingDirection);

    /**
     * Remove an item frame entity by ID, broadcast S13, drop item frame + contents in survival.
     * Java reference: EntityItemFrame.attackEntityFrom() → onBroken() → dropItemOrSelf()
     */
    void removeItemFrame(int32_t entityId, bool dropItem);

    /**
     * Interact with an item frame (right-click) — place/rotate displayed item.
     * Java reference: EntityItemFrame.interactFirst() + setDisplayedItem() / func_82334_a()
     */
    void interactItemFrame(PlayHandler& player, Connection& conn, int32_t entityId);

    // ─── Ender Crystal entities ─────────────────────────────────────────
    // Java reference: EntityEnderCrystal — End pillar decoration, dragon healing beam target
    struct SpawnedEnderCrystal {
        int32_t entityId = 0;
        double posX = 0, posY = 0, posZ = 0;
        int32_t health = 5;              // Java: EntityEnderCrystal.health = 5
        int32_t innerRotation = 0;       // Java: EntityEnderCrystal.innerRotation (animation)
        bool isDead = false;

        static constexpr float EXPLOSION_POWER = 6.0f;  // Java: createExplosion(null, x, y, z, 6.0f, true)
    };
    mutable std::mutex enderCrystalEntitiesMutex_;
    std::vector<SpawnedEnderCrystal> enderCrystalEntities_;
    std::atomic<int32_t> nextEnderCrystalEntityId_{1400000};

    /**
     * Spawn an Ender Crystal entity and broadcast S0E SpawnObject type 51.
     * Java reference: EntityEnderCrystal(World) + WorldGenSpikes.generate()
     * @return entity ID of the spawned crystal
     */
    int32_t spawnEnderCrystal(double x, double y, double z);

    /**
     * Remove an Ender Crystal by ID — creates power 6.0 explosion.
     * Java reference: EntityEnderCrystal.attackEntityFrom() → createExplosion()
     */
    void removeEnderCrystal(int32_t entityId);

    // ─── Primed TNT entities ────────────────────────────────────────────
    // Java reference: EntityTNTPrimed — fuse countdown with gravity physics
    struct SpawnedTNTPrimed {
        int32_t entityId = 0;
        double posX = 0, posY = 0, posZ = 0;
        double motionX = 0, motionY = 0, motionZ = 0;
        int32_t fuse = 80;              // Java: EntityTNTPrimed.fuse — 80 ticks = 4 seconds
        bool isDead = false;
        bool onGround = false;
        int64_t spawnTick = 0;
        // Movement tracking for S18 EntityTeleport broadcast
        int32_t lastSentPosX = 0, lastSentPosY = 0, lastSentPosZ = 0;
        int32_t ticksSinceLastTeleport = 0;

        static constexpr float GRAVITY = 0.04f;        // Java: motionY -= 0.04
        static constexpr float FRICTION = 0.98f;        // Java: motionX/Y/Z *= 0.98
        static constexpr float GROUND_FRICTION = 0.7f;  // Java: motionX/Z *= 0.7 on ground
        static constexpr float GROUND_BOUNCE = -0.5f;   // Java: motionY *= -0.5 on ground
        static constexpr float EXPLOSION_POWER = 4.0f;   // Java: Explosion power
    };
    mutable std::mutex tntPrimedEntitiesMutex_;
    std::vector<SpawnedTNTPrimed> tntPrimedEntities_;
    std::atomic<int32_t> nextTNTPrimedEntityId_{1000000};

    /**
     * Spawn a primed TNT entity at position with default 80-tick fuse.
     * Java reference: EntityTNTPrimed(world, x, y, z, igniter)
     * @param fuseOverride Optional custom fuse (default 80; chain reactions use shorter fuse)
     * @return entity ID of the spawned TNT
     */
    int32_t spawnTNTPrimed(double x, double y, double z, int32_t fuseOverride = 80);

    /** Tick all primed TNT entities (gravity, fuse countdown, explosion on fuse=0). */
    void tickTNTPrimed();

    // ─── Falling block entities ─────────────────────────────────────────
    // Java reference: EntityFallingBlock — sand, gravel, anvil with gravity physics
    struct SpawnedFallingBlock {
        int32_t entityId = 0;
        int32_t blockId = 0;            // Java: Block.getIdFromBlock(blockObj)
        int32_t metadata = 0;           // Java: EntityFallingBlock.metadata
        double posX = 0, posY = 0, posZ = 0;
        double motionX = 0, motionY = 0, motionZ = 0;
        int32_t fallTime = 0;           // Java: EntityFallingBlock.fallTime
        bool isDead = false;
        bool onGround = false;
        bool hurtEntities = false;      // Java: true for anvils
        int64_t spawnTick = 0;
        // Movement tracking for S18 EntityTeleport broadcast
        int32_t lastSentPosX = 0, lastSentPosY = 0, lastSentPosZ = 0;
        int32_t ticksSinceLastTeleport = 0;

        static constexpr float GRAVITY = 0.04f;        // Java: motionY -= 0.04
        static constexpr float FRICTION = 0.98f;        // Java: motionX/Y/Z *= 0.98
        static constexpr float GROUND_FRICTION = 0.7f;  // Java: motionX/Z *= 0.7 on ground
        static constexpr float GROUND_BOUNCE = -0.5f;   // Java: motionY *= -0.5 on ground
        static constexpr int32_t MAX_FALL_TIME = 600;    // Java: fallTime > 600 → drop item, die
    };
    mutable std::mutex fallingBlockEntitiesMutex_;
    std::vector<SpawnedFallingBlock> fallingBlockEntities_;
    std::atomic<int32_t> nextFallingBlockEntityId_{1100000};

    /**
     * Spawn a falling block entity at position.
     * Java reference: BlockFalling.func_149830_m → new EntityFallingBlock(world, x+0.5, y+0.5, z+0.5, block, meta)
     * @return entity ID of the spawned falling block
     */
    int32_t spawnFallingBlock(double x, double y, double z, int32_t blockId, int32_t metadata = 0);

    /** Tick all falling block entities (gravity, landing, placement/item drop). */
    void tickFallingBlock();

    // ─── XP orb entities ────────────────────────────────────────────────
    // Java reference: EntityXPOrb — experience orb physics, player attraction, pickup
    struct SpawnedXPOrb {
        int32_t entityId = 0;
        double posX = 0, posY = 0, posZ = 0;
        double motionX = 0, motionY = 0, motionZ = 0;
        int32_t xpValue = 0;        // Java: EntityXPOrb.xpValue
        int32_t xpOrbAge = 0;       // Java: EntityXPOrb.xpOrbAge — despawn after 6000 ticks
        int32_t xpColor = 0;        // Java: EntityXPOrb.xpColor — tick counter for target refresh
        int32_t pickupDelay = 0;    // Java: EntityXPOrb.field_70532_c — delay before pickup
        bool isDead = false;
        bool onGround = false;
        int64_t spawnTick = 0;

        static constexpr float GRAVITY = 0.03f;        // Java: motionY -= 0.03
        static constexpr float AIR_FRICTION = 0.98f;    // Java: motionY *= 0.98
        static constexpr float GROUND_BOUNCE = -0.9f;   // Java: motionY *= -0.9 on ground
        static constexpr double ATTRACT_RANGE = 8.0;    // Java: d6 = 8.0
        static constexpr int32_t DESPAWN_AGE = 6000;     // Java: 6000 ticks = 5 minutes
    };
    mutable std::mutex xpOrbEntitiesMutex_;
    std::vector<SpawnedXPOrb> xpOrbEntities_;
    std::atomic<int32_t> nextXPOrbEntityId_{900000};

    /**
     * Spawn XP orbs at a position, splitting large amounts via getXPSplit().
     * Java reference: EntityXPOrb constructor + EntityLiving.onDeathUpdate() splitting
     * @param totalXp total experience to distribute across orbs
     */
public:
    void spawnXPOrbs(double x, double y, double z, int32_t totalXp);

    /** Tick all XP orb entities (physics, player attraction, pickup, despawn). */
    void tickXPOrbs();

    /**
     * Java: EntityXPOrb.getXPSplit() — determine orb size for splitting large XP values.
     * Returns the largest standard orb value <= the remaining XP.
     */
    static int32_t getXPSplit(int32_t xpAmount);

    /**
     * Drop all items from a container tile entity at the given position.
     * Java reference: BlockContainer.breakBlock() → InventoryHelper.dropInventoryItems()
     * Handles: chest(54/146), furnace(61/62), dispenser(23), dropper(158),
     *          hopper(154), brewing stand(117).
     * Also removes the tile entity data from storage after dropping.
     */
    void dropContainerContents(int32_t x, int32_t y, int32_t z, int32_t blockId);

private:
    // ─── Chest storage (in-memory tile entities) ─────────────────────
    // Key: packed position (x << 40 | (z & 0xFFFFF) << 20 | (y & 0xFFFFF))
    mutable std::mutex chestMutex_;
    std::map<int64_t, std::array<std::optional<ItemStack>, 27>> chestStorage_;

    // ─── Furnace storage (in-memory tile entities) ───────────────────
    mutable std::mutex furnaceMutex_;
    std::map<int64_t, FurnaceData> furnaceStorage_;

    // ─── Scheduled block ticks ──────────────────────────────────────
    // Java: WorldServer.scheduleBlockUpdateWithPriority / tickUpdates
    // Used for button auto-reset, repeater delay, etc.
    struct ScheduledBlockTick {
        int32_t x, y, z;
        int32_t blockId;      // Expected block ID at fire time
        int64_t fireTick;     // Server tick when this should fire
    };
    mutable std::mutex scheduledTicksMutex_;
    std::vector<ScheduledBlockTick> scheduledTicks_;

    // ─── Redstone torch burnout tracking ─────────────────────────────
    // Java: BlockRedstoneTorch.field_150112_b — list of recent toggles
    // 8 toggles within 60 ticks = burnout (torch stays OFF)
    std::vector<std::tuple<int32_t,int32_t,int32_t,int64_t>> torchToggleList_;

public:
    /** Schedule a block tick (e.g. button reset after N ticks). */
    void scheduleBlockTick(int32_t x, int32_t y, int32_t z, int32_t blockId, int32_t delay);

    /** Process expired scheduled ticks — called from main tick(). */
    void tickScheduledBlocks();
};

} // namespace mccpp
