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

#include <set>
#include <map>
#include <utility>

#include <atomic>
#include <climits>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "command/CommandSystem.h"
#include "inventory/Inventory.h"
#include "mechanics/FoodStats.h"
#include "server/MerchantRecipe.h"

namespace mccpp {

class Connection;      // forward decl
class MinecraftServer; // forward decl
class Chunk;           // forward decl

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
                const std::string& uuid);

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

    /**
     * Send a single chunk's data to the client.
     * Java reference: S21PacketChunkData — ground-up continuous mode.
     * Serializes block IDs, metadata, blocklight, skylight, and biomes.
     */
    void sendChunkData(Connection& conn, Chunk* chunk);

    const std::string& getPlayerName() const { return playerName_; }
    int getKeepAliveId() const { return lastKeepAliveId_; }

    // ─── Outbound Play Packets ─────────────────────────────────────────

    /**
     * Send a time update to the client.
     * Java reference: S03PacketTimeUpdate
     */
    void sendTimeUpdate(Connection& conn, int64_t worldAge, int64_t timeOfDay);

    /**
     * Send health/food/saturation update to the client.
     * Java reference: S06PacketUpdateHealth
     */
    void sendUpdateHealth(Connection& conn, float health, int32_t food, float saturation);

    /**
     * Send a block change to the client.
     * Java reference: S23PacketBlockChange
     */
    void sendEntityEquipment(Connection& conn, int32_t entityId, int16_t equipSlot,
                              const std::optional<ItemStack>& stack);
    void sendBlockChange(Connection& conn, int32_t x, int32_t y, int32_t z,
                         int32_t blockId, int32_t metadata);

    /**
     * Send player list item (tab list) update.
     * Java reference: S38PacketPlayerListItem
     */
    void sendPlayerListItem(Connection& conn, const std::string& playerName,
                            bool online, int16_t ping);

    /**
     * Send a SpawnPlayer packet to make a player entity visible.
     * Java reference: S0CPacketSpawnPlayer
     */
    void sendSpawnPlayer(Connection& conn, int32_t entityId,
                         const std::string& uuid, const std::string& name,
                         double x, double y, double z,
                         float yaw, float pitch, int16_t heldItem);

    /**
     * Send a DestroyEntities packet to remove entity(s) from client.
     * Java reference: S13PacketDestroyEntities
     */
    void sendDestroyEntities(Connection& conn, const std::vector<int32_t>& entityIds);

    /**
     * Send entire window contents to the client.
     * Java reference: S30PacketWindowItems
     */
    void sendWindowItems(Connection& conn);

    /**
     * Send a single slot update to the client.
     * Java reference: S2FPacketSetSlot
     */
    void sendSetSlot(Connection& conn, int8_t windowId, int16_t slot,
                     const std::optional<ItemStack>& stack);

    /**
     * Send S2D OpenWindow to the client.
     * Java reference: S2DPacketOpenWindow
     */
    void sendOpenWindow(Connection& conn, int8_t windowId, int8_t windowType,
                        const std::string& windowTitle, int8_t slotCount);

    /**
     * Open a 3×3 crafting table (workbench) for this player.
     * Sends S2D OpenWindow + S30 (window contents).
     */
    void openWorkbench(Connection& conn, int32_t blockX, int32_t blockY, int32_t blockZ);

    /**
     * Close the current open window, dropping items from the crafting grid.
     */
    void closeOpenWindow(Connection& conn);

    /**
     * Open a chest container for this player.
     * Sends S2D OpenWindow + S30 (window contents).
     * Java reference: TileEntityChest.openInventory
     */
    void openChest(Connection& conn, int32_t blockX, int32_t blockY, int32_t blockZ);

    /**
     * Open a furnace container for this player.
     * Sends S2D OpenWindow type 2 + S30 (window contents) + S31 (progress bars).
     * Java reference: TileEntityFurnace + ContainerFurnace
     */
    void openFurnace(Connection& conn, int32_t blockX, int32_t blockY, int32_t blockZ);

    /**
     * Open an ender chest for this player.
     * Per-player storage (27 slots), S2D OpenWindow type 0.
     * Java reference: InventoryEnderChest
     */
    void openEnderChest(Connection& conn, int32_t blockX, int32_t blockY, int32_t blockZ);

    /**
     * Open a villager trading GUI for this player.
     * Sends S2D OpenWindow type 6 + S3F MC|TrList + S30 WindowItems.
     * Java reference: EntityPlayer.displayGUIMerchant() → ContainerMerchant
     */
    void openVillagerTrading(Connection& conn, int32_t villagerEntityId,
                             const std::vector<MerchantRecipe>& trades);

    /**
     * Send S31 WindowProperty to the client (furnace progress bars).
     * Java reference: S31PacketWindowProperty
     */
    void sendWindowProperty(Connection& conn, int8_t windowId, int16_t property, int16_t value);

    /**
     * Send entity teleport (absolute position) to the client.
     * Java reference: S18PacketEntityTeleport
     */
    void sendEntityTeleport(Connection& conn, int32_t entityId,
                            double x, double y, double z,
                            float yaw, float pitch);

    /**
     * Send entity head rotation to the client.
     * Java reference: S19PacketEntityHeadLook
     */
    void sendEntityHeadLook(Connection& conn, int32_t entityId, float yaw);

    /**
     * Send S15 EntityRelativeMove to the client.
     * Java reference: S15PacketEntityRelMove
     * Delta encoded as fixedPoint = value * 32 (1/32 of a block)
     */
    void sendEntityRelMove(Connection& conn, int32_t entityId,
                           int8_t dx, int8_t dy, int8_t dz);

    /**
     * Send S16 EntityLook to the client.
     * Java reference: S16PacketEntityLook
     */
    void sendEntityLook(Connection& conn, int32_t entityId,
                        float yaw, float pitch);

    /**
     * Send S17 EntityLookAndRelativeMove to the client.
     * Java reference: S17PacketEntityLookMove
     */
    void sendEntityLookRelMove(Connection& conn, int32_t entityId,
                               int8_t dx, int8_t dy, int8_t dz,
                               float yaw, float pitch);

    /**
     * Send a named sound effect to the client.
     * Java reference: S29PacketSoundEffect
     */
    void sendSoundEffect(Connection& conn, const std::string& soundName,
                         double x, double y, double z,
                         float volume, float pitch);

    /**
     * Send a SpawnObject packet (for item drops, projectiles, etc.)
     * Java reference: S0EPacketSpawnObject
     */
    void sendSpawnObject(Connection& conn, int32_t entityId, int8_t type,
                         double x, double y, double z,
                         float yaw, float pitch, int32_t data,
                         double motionX, double motionY, double motionZ);

    /**
     * Send entity metadata with an ItemStack at DataWatcher slot 10.
     * Java reference: S1CPacketEntityMetadata + DataWatcher
     */
    void sendEntityMetadataItem(Connection& conn, int32_t entityId,
                                int16_t itemId, int8_t stackSize, int16_t damage);

    /**
     * Send entity metadata with a full ItemStack (including NBT) at DataWatcher slot 10.
     * Java reference: S1CPacketEntityMetadata — used for death drops with enchantments.
     */
    void sendEntityMetadataItemStack(Connection& conn, int32_t entityId, const ItemStack& stack);

    /**
     * Send collect item animation (item flies to player).
     * Java reference: S0DPacketCollectItem
     */
    void sendCollectItem(Connection& conn, int32_t collectedEntityId, int32_t collectorEntityId);

    /**
     * Send S1A EntityStatus to the client.
     * Java reference: S1APacketEntityStatus
     */
    void sendEntityStatus(Connection& conn, int32_t entityId, int8_t status);

    /**
     * Send S12 EntityVelocity to the client.
     * Java reference: S12PacketEntityVelocity
     */
    void sendEntityVelocity(Connection& conn, int32_t entityId, double vx, double vy, double vz);

    /**
     * Send S1B AttachEntity to mount/dismount an entity.
     * Java reference: S1BPacketEntityAttach
     * @param leash 0 for riding, 1 for leash
     * @param entityId the entity being attached
     * @param vehicleId the vehicle entity (-1 to dismount)
     */
    void sendAttachEntity(Connection& conn, int32_t leash, int32_t entityId, int32_t vehicleId);

    /**
     * Send S2C SpawnGlobalEntity to the client (lightning bolt).
     * Java reference: S2CPacketSpawnGlobalEntity
     * @param type 1 = lightning bolt
     */
    void sendSpawnGlobalEntity(Connection& conn, int32_t entityId, int8_t type,
                                double x, double y, double z);

    /**
     * Send S11 SpawnExpOrb to the client.
     * Java reference: S11PacketSpawnExperienceOrb
     */
    void sendSpawnExpOrb(Connection& conn, int32_t entityId,
                         double x, double y, double z, int16_t xpValue);

    /**
     * Send S10 SpawnPainting to the client.
     * Java reference: S10PacketSpawnPainting
     */
    void sendSpawnPainting(Connection& conn, int32_t entityId,
                           const std::string& artTitle,
                           int32_t blockX, int32_t blockY, int32_t blockZ,
                           int32_t direction);

    /**
     * Send S1C EntityMetadata for an item frame entity.
     * Java reference: EntityItemFrame DataWatcher: index 2 (ItemStack), index 3 (Byte rotation)
     * If itemId == 0, sends an empty item slot (Short -1).
     */
    void sendEntityMetadataItemFrame(Connection& conn, int32_t entityId,
                                      int16_t itemId, int8_t stackSize, int16_t damage,
                                      int8_t rotation);

    /**
     * Send S0F SpawnMob to the client.
     * Java reference: S0FPacketSpawnMob
     */
    void sendSpawnMob(Connection& conn, int32_t entityId, uint8_t mobType,
                      double x, double y, double z,
                      float yaw, float pitch, float headYaw);

    /**
     * Apply damage to this player, reducing health.
     * Java reference: EntityLivingBase.damageEntity()
     */
    void applyDamage(float amount);

    /**
     * Send S1D EntityEffect to the client.
     * Java reference: S1DPacketEntityEffect
     */
    void sendEntityEffect(Connection& conn, int32_t entityId, int8_t effectId,
                          int8_t amplifier, int16_t duration);

    /**
     * Send S1E RemoveEntityEffect to the client.
     * Java reference: S1EPacketRemoveEntityEffect
     */
    void sendRemoveEntityEffect(Connection& conn, int32_t entityId, int8_t effectId);

    /**
     * Add a potion effect to this player. Sends S1D to client.
     * Java reference: EntityLivingBase.addPotionEffect()
     */
    void addPotionEffect(Connection& conn, int32_t effectId, int32_t duration, int32_t amplifier);

    /**
     * Remove a potion effect from this player. Sends S1E to client.
     * Java reference: EntityLivingBase.removePotionEffect()
     */
    void removePotionEffect(Connection& conn, int32_t effectId);

    /**
     * Remove all potion effects (on death/respawn).
     */
    void clearPotionEffects(Connection& conn);

    /**
     * Tick active potion effects — decrement duration, apply per-tick effects.
     * Java reference: EntityLivingBase.onUpdate() → activePotionsMap iteration
     */
    void tickPotionEffects(Connection& conn);

    // ─── Getters for player state ──────────────────────────────────────
    int32_t getEntityId() const { return entityId_; }
    const std::string& getUuid() const { return uuid_; }
    double getPlayerX() const { return playerX_; }
    double getPlayerY() const { return playerY_; }
    double getPlayerZ() const { return playerZ_; }
    float getPlayerYaw() const { return playerYaw_; }
    float getPlayerPitch() const { return playerPitch_; }
    float getHealth() const { return health_; }
    int32_t getFood() const { return foodStats_.getFoodLevel(); }
    float getSaturation() const { return foodStats_.getSaturationLevel(); }
    FoodStats& getFoodStats() { return foodStats_; }
    const FoodStats& getFoodStats() const { return foodStats_; }
    bool isDead() const { return dead_; }
    int16_t getCurrentSlot() const { return currentSlot_; }
    bool isSneaking() const { return isSneaking_; }
    int32_t getPlayerDimension() const { return playerDimension_; }

    // C0C SteerVehicle input — Java: EntityLivingBase.moveForward/moveStrafing
    float getMoveForward() const { return moveForward_; }
    float getMoveStrafing() const { return moveStrafing_; }

    // Riding state — tracks which entity this player is riding
    int32_t getRidingEntityId() const { return ridingEntityId_; }
    void setRidingEntityId(int32_t id) { ridingEntityId_ = id; }
    bool isSprinting() const { return isSprinting_; }

    // Java: Entity.setFire(int seconds) — sets fire ticks
    void setOnFire(int32_t ticks) { fireTicks_ = ticks; }

    /**
     * Transfer the player to another dimension (Nether portal transit).
     * Java reference: ServerConfigurationManager.transferPlayerToDimension()
     */
    void transferDimension(Connection& conn, bool isEndPortal);

    /**
     * Tick portal standing detection/cooldown. Called once per server tick.
     * Java reference: Entity.handlePortal() / EntityPlayerMP.onUpdate()
     */
    void tickPortal(Connection& conn);

    /**
     * Tick the food/hunger system once per server tick.
     * Java reference: EntityPlayer.onUpdate() → FoodStats.onUpdate()
     * Handles exhaustion→saturation→hunger drain, natural health regen,
     * starvation damage, and sends S06 UpdateHealth when values change.
     */
    void tickFood(Connection& conn);

    // ─── Setters for player state (used by commands) ────────────────────
    void setPlayerPosition(double x, double y, double z) {
        playerX_ = x; playerY_ = y; playerZ_ = z;
    }
    void setGameMode(int32_t mode) { gameMode_ = mode; }
    void resetFallDistance() { fallDistance_ = 0.0f; }
    int32_t getGameMode() const { return gameMode_; }

    /**
     * Send S08 PlayerPosAndLook (teleport the client to a position).
     * Java reference: EntityPlayerMP.setPositionAndUpdate()
     */
    void sendPlayerPosAndLook(Connection& conn, double x, double y, double z,
                               float yaw, float pitch);

    /**
     * Send S1F SetExperience to the client.
     * Java reference: S1FPacketSetExperience
     */
    void sendSetExperience(Connection& conn, float bar, int32_t level, int32_t totalXp);

    // ─── XP accessors (used by MinecraftServer combat) ──────────────
    float getExperienceBar() const { return experienceBar_; }
    int32_t getExperienceLevel() const { return experienceLevel_; }
    int32_t getExperienceTotal() const { return experienceTotal_; }
    void grantExperience(int32_t amount) { addExperience(amount); }
    void resetExperience() { experienceBar_ = 0.0f; experienceLevel_ = 0; experienceTotal_ = 0; }
    void addLevels(int32_t levels) {
        experienceLevel_ += levels;
        if (experienceLevel_ < 0) experienceLevel_ = 0;
        // Recalculate total XP from level (simplified)
        experienceBar_ = 0.0f;
    }
    void sendExperienceUpdate(Connection& conn) {
        sendSetExperience(conn, experienceBar_, experienceLevel_, experienceTotal_);
    }

    // Clear player inventory: if itemId<0, clear all; otherwise only matching items.
    // Returns count of items removed.
    int32_t clearInventory(int32_t itemId = -1, int32_t damage = -1) {
        int32_t count = 0;
        for (int32_t i = 0; i < InventoryPlayer::TOTAL_SIZE; ++i) {
            auto stack = inventory_.getStackInSlot(i);
            if (!stack) continue;
            if (itemId >= 0 && stack->getItemId() != itemId) continue;
            if (damage >= 0 && stack->getDamage() != damage) continue;
            count += stack->getStackSize();
            inventory_.setInventorySlotContents(i, std::nullopt);
        }
        return count;
    }

    // Add enchantment to held item via NBT
    void enchantHeldItem(int32_t enchId, int32_t level) {
        auto held = inventory_.getCurrentItem();
        if (!held) return;
        held->addEnchantment(enchId, level);
        inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), held);
    }

    /**
     * Send a chunk unload (S21 with bitmask=0) to the client.
     * Java reference: S21PacketChunkData with groundUp=true, bitmask=0
     */
    void sendChunkUnload(Connection& conn, int32_t chunkX, int32_t chunkZ);

    /**
     * Check and update chunks around the player based on current position.
     * Called on player movement. Sends new chunks, unloads distant ones.
     * Java reference: PlayerManager / ServerConfigurationManager chunk tracking
     */
    void updateChunks(Connection& conn);

    /**
     * Send S2B ChangeGameState to the client.
     * Java reference: S2BPacketChangeGameState
     * Reason 3 = change game mode (value = gamemode int)
     */
    void sendChangeGameState(Connection& conn, uint8_t reason, float value);

    // Equipment accessors — for S04 EntityEquipment broadcasting
    std::optional<ItemStack> getHeldItem() const {
        return inventory_.getCurrentItem();
    }
    // equipSlot: 1=boots, 2=leggings, 3=chestplate, 4=helmet → armor index 0-3
    std::optional<ItemStack> getArmorItem(int16_t equipSlot) const {
        int32_t armorIdx = equipSlot - 1; // S04 slot 1→armor[0], 2→armor[1], etc.
        if (armorIdx < 0 || armorIdx >= InventoryPlayer::ARMOR_SIZE) return std::nullopt;
        return inventory_.getArmorInventory()[armorIdx];
    }

    /**
     * Try to pick up an item entity into this player's inventory.
     * Returns true if the item was successfully added.
     * Java reference: EntityPlayer.onItemPickup + InventoryPlayer.addItemStackToInventory
     */
    bool tryPickupItem(Connection& conn, int32_t itemId, int32_t damage, int32_t count) {
        ItemStack stack(itemId, count, damage);
        bool added = inventory_.addItemStackToInventory(stack);
        if (added) {
            // Sync entire inventory to client
            sendWindowItems(conn);
        }
        return added;
    }

    // Java: SharedMonsterAttributes.attackDamage base=2.0, modified by held weapon
    // Sword: 4 + matDmg, Axe: 3 + matDmg, Pick: 2 + matDmg, Shovel: 1 + matDmg
    // matDmg: wood/gold=0, stone=1, iron=2, diamond=3
    float getWeaponDamage() const {
        auto held = inventory_.getCurrentItem();
        if (!held) return 2.0f; // fist
        int32_t id = held->getItemId();
        float baseDmg = 2.0f;
        switch (id) {
            // Swords: 4.0 + material damage
            case 268: baseDmg = 4.0f; break;  // Wooden sword (4+0)
            case 272: baseDmg = 5.0f; break;  // Stone sword (4+1)
            case 267: baseDmg = 6.0f; break;  // Iron sword (4+2)
            case 276: baseDmg = 7.0f; break;  // Diamond sword (4+3)
            case 283: baseDmg = 4.0f; break;  // Golden sword (4+0)
            // Axes: 3.0 + material damage
            case 271: baseDmg = 3.0f; break;  // Wooden axe (3+0)
            case 275: baseDmg = 4.0f; break;  // Stone axe (3+1)
            case 258: baseDmg = 5.0f; break;  // Iron axe (3+2)
            case 279: baseDmg = 6.0f; break;  // Diamond axe (3+3)
            case 286: baseDmg = 3.0f; break;  // Golden axe (3+0)
            // Pickaxes: 2.0 + material damage
            case 270: baseDmg = 2.0f; break;  // Wooden pick (2+0)
            case 274: baseDmg = 3.0f; break;  // Stone pick (2+1)
            case 257: baseDmg = 4.0f; break;  // Iron pick (2+2)
            case 278: baseDmg = 5.0f; break;  // Diamond pick (2+3)
            case 285: baseDmg = 2.0f; break;  // Golden pick (2+0)
            // Shovels: 1.0 + material damage
            case 269: baseDmg = 1.0f; break;  // Wooden shovel (1+0)
            case 273: baseDmg = 2.0f; break;  // Stone shovel (1+1)
            case 256: baseDmg = 3.0f; break;  // Iron shovel (1+2)
            case 277: baseDmg = 4.0f; break;  // Diamond shovel (1+3)
            case 284: baseDmg = 1.0f; break;  // Golden shovel (1+0)
            default: break;   // Unknown item = fist base
        }
        // Java: EnchantmentHelper.func_152377_a — Sharpness (ID 16) adds level * 1.25f
        // Smite (17) and Bane of Arthropods (18) need target creature type, simplified to Sharpness only
        int16_t sharpness = held->getEnchantmentLevel(16);
        if (sharpness > 0) baseDmg += static_cast<float>(sharpness) * 1.25f;
        return baseDmg;
    }

    // Java: EntityLivingBase.getTotalArmorValue() → sum of ItemArmor.damageReduceAmount
    // Armor indices: S04 slot 1=boots, 2=legs, 3=chest, 4=helmet
    // damageReductionAmountArray per material (helmet, chest, legs, boots):
    //   Leather: [1,3,2,1]=7  Chain: [2,5,4,1]=12  Iron: [2,6,5,2]=15
    //   Gold: [2,5,3,1]=11  Diamond: [3,8,6,3]=20
    int32_t getTotalArmorValue() const {
        int32_t total = 0;
        for (int16_t slot = 1; slot <= 4; ++slot) {
            auto armor = getArmorItem(slot);
            if (!armor) continue;
            int32_t id = armor->getItemId();
            // Helmets (slot 4 → armorType 0)
            // Chestplates (slot 3 → armorType 1)
            // Leggings (slot 2 → armorType 2)
            // Boots (slot 1 → armorType 3)
            switch (id) {
                // Leather: helmet=1, chest=3, legs=2, boots=1
                case 298: total += 1; break; // Leather helmet
                case 299: total += 3; break; // Leather chestplate
                case 300: total += 2; break; // Leather leggings
                case 301: total += 1; break; // Leather boots
                // Chain: helmet=2, chest=5, legs=4, boots=1
                case 302: total += 2; break; // Chain helmet
                case 303: total += 5; break; // Chain chestplate
                case 304: total += 4; break; // Chain leggings
                case 305: total += 1; break; // Chain boots
                // Iron: helmet=2, chest=6, legs=5, boots=2
                case 306: total += 2; break; // Iron helmet
                case 307: total += 6; break; // Iron chestplate
                case 308: total += 5; break; // Iron leggings
                case 309: total += 2; break; // Iron boots
                // Diamond: helmet=3, chest=8, legs=6, boots=3
                case 310: total += 3; break; // Diamond helmet
                case 311: total += 8; break; // Diamond chestplate
                case 312: total += 6; break; // Diamond leggings
                case 313: total += 3; break; // Diamond boots
                // Gold: helmet=2, chest=5, legs=3, boots=1
                case 314: total += 2; break; // Gold helmet
                case 315: total += 5; break; // Gold chestplate
                case 316: total += 3; break; // Gold leggings
                case 317: total += 1; break; // Gold boots
            }
        }
        return total;
    }

    // Java: EnchantmentHelper.getEnchantmentModifierDamage() — sum protection enchant values
    // Protection (ID 0): applies to ALL damage types — floor((6 + level²) / 3.0 * 0.75)
    // Fire Protection (ID 1): applies to fire damage — floor((6 + level²) / 3.0 * 1.25)
    // Blast Protection (ID 3): applies to explosion — floor((6 + level²) / 3.0 * 1.5)
    // Projectile Protection (ID 4): applies to projectile — floor((6 + level²) / 3.0 * 1.5)
    // damageType: 0=generic/melee, 1=fire, 2=fall, 3=explosion, 4=projectile
    int32_t getEnchantmentProtectionModifier(int32_t damageType = 0) const {
        int32_t totalMod = 0;
        for (int16_t slot = 1; slot <= 4; ++slot) {
            auto armor = getArmorItem(slot);
            if (!armor || !armor->hasEnchantments()) continue;
            // Protection (0) — always contributes (type factor 0.75)
            int16_t protLevel = armor->getEnchantmentLevel(0);
            if (protLevel > 0) {
                float f = static_cast<float>(6 + protLevel * protLevel) / 3.0f;
                totalMod += static_cast<int32_t>(f * 0.75f);
            }
            // Fire Protection (1) — fire damage only (type factor 1.25)
            if (damageType == 1) {
                int16_t fpLevel = armor->getEnchantmentLevel(1);
                if (fpLevel > 0) {
                    float f = static_cast<float>(6 + fpLevel * fpLevel) / 3.0f;
                    totalMod += static_cast<int32_t>(f * 1.25f);
                }
            }
            // Feather Falling (2) — fall damage only (type factor 2.5)
            // Java: EnchantmentProtection protectionType=2, only on boots
            if (damageType == 2) {
                int16_t ffLevel = armor->getEnchantmentLevel(2);
                if (ffLevel > 0) {
                    float f = static_cast<float>(6 + ffLevel * ffLevel) / 3.0f;
                    totalMod += static_cast<int32_t>(f * 2.5f);
                }
            }
            // Blast Protection (3) — explosion damage only (type factor 1.5)
            if (damageType == 3) {
                int16_t bpLevel = armor->getEnchantmentLevel(3);
                if (bpLevel > 0) {
                    float f = static_cast<float>(6 + bpLevel * bpLevel) / 3.0f;
                    totalMod += static_cast<int32_t>(f * 1.5f);
                }
            }
            // Projectile Protection (4) — projectile damage only (type factor 1.5)
            if (damageType == 4) {
                int16_t ppLevel = armor->getEnchantmentLevel(4);
                if (ppLevel > 0) {
                    float f = static_cast<float>(6 + ppLevel * ppLevel) / 3.0f;
                    totalMod += static_cast<int32_t>(f * 1.5f);
                }
            }
        }
        // Java: capped at 20 (EnchantmentHelper)
        return std::min(totalMod, 20);
    }

    // Java: Item.getMaxDamage() for damageable items
    // Returns max durability for an item ID, or 0 if not damageable
    static int32_t getMaxDurability(int32_t itemId) {
        switch (itemId) {
            // Wood tools: 59 durability
            case 268: case 269: case 270: case 271: return 59;  // sword, shovel, pick, axe
            // Stone tools: 131
            case 272: case 273: case 274: case 275: return 131;
            // Iron tools: 250
            case 256: case 257: case 258: case 267: return 250; // shovel, pick, axe, sword
            // Diamond tools: 1561
            case 276: case 277: case 278: case 279: return 1561;
            // Gold tools: 32
            case 283: case 284: case 285: case 286: return 32;
            // Hoe durabilities (same pattern)
            case 290: return 59;   // Wooden hoe
            case 291: return 131;  // Stone hoe
            case 292: return 250;  // Iron hoe
            case 293: return 1561; // Diamond hoe
            case 294: return 32;   // Gold hoe
            // Leather armor: 5*maxDamageFactor, factors [11,16,15,13]
            case 298: return 55;  case 299: return 80;  case 300: return 75;  case 301: return 65;
            // Chain armor: 15*[11,16,15,13]
            case 302: return 165; case 303: return 240; case 304: return 225; case 305: return 195;
            // Iron armor: 15*[11,16,15,13]
            case 306: return 165; case 307: return 240; case 308: return 225; case 309: return 195;
            // Diamond armor: 33*[11,16,15,13]
            case 310: return 363; case 311: return 528; case 312: return 495; case 313: return 429;
            // Gold armor: 7*[11,16,15,13]
            case 314: return 77;  case 315: return 112; case 316: return 105; case 317: return 91;
            // Bow: 384
            case 261: return 384;
            // Shears: 238
            case 359: return 238;
            // Fishing rod: 64
            case 346: return 64;
            // Flint and steel: 64
            case 259: return 64;
            // Carrot on a stick: 25
            case 398: return 25;
            default: return 0; // Not damageable
        }
    }

    // Damage the held item by n points. If durability exceeded, item breaks (removed).
    // Java: ItemStack.damageItem(n, entity)
    void damageHeldItem(int32_t amount) {
        if (gameMode_ == 1) return; // Creative: no durability loss
        auto held = inventory_.getCurrentItem();
        if (!held) return;
        int32_t maxDur = getMaxDurability(held->getItemId());
        if (maxDur <= 0) return; // Not damageable
        // Java: Unbreaking (ID 34) — 1/(level+1) chance to actually apply damage
        // ItemStack.attemptDamageItem() → if random < 1/(unbreaking+1) then apply
        int16_t unbreaking = held->getEnchantmentLevel(34);
        if (unbreaking > 0) {
            if (rand() % (unbreaking + 1) != 0) return; // Survived durability check
        }
        int32_t newDamage = held->getDamage() + amount;
        if (newDamage > maxDur) {
            // Item breaks — remove from hotbar
            inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), std::nullopt);
        } else {
            held->setDamage(newDamage);
            inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), held);
        }
    }

    // Replace the held item entirely (e.g. bucket → milk bucket).
    // Java: EntityPlayer.inventory.setInventorySlotContents(currentItem, stack)
    void replaceHeldItem(const std::optional<ItemStack>& stack) {
        inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), stack);
    }

    // Decrement held item stack size by 1. If stack becomes empty, removes item.
    // Java: ItemStack.stackSize-- (used by bucket milking, etc.)
    void decrHeldItem() {
        auto held = inventory_.getCurrentItem();
        if (!held) return;
        int32_t newCount = held->getStackSize() - 1;
        if (newCount <= 0) {
            inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), std::nullopt);
        } else {
            held->setStackSize(newCount);
            inventory_.setInventorySlotContents(inventory_.getCurrentSlot(), held);
        }
    }

    // Try to add an item to the player's inventory.
    // Returns true if added successfully, false if inventory full.
    // Java: InventoryPlayer.addItemStackToInventory(ItemStack)
    bool addItemToInventory(ItemStack stack) {
        return inventory_.addItemStackToInventory(stack);
    }

    // Java: InventoryPlayer.damageArmor(float f)
    // Damage all worn armor pieces by (incomingDamage / 4, min 1)
    void damageArmor(float incomingDamage) {
        if (gameMode_ == 1) return; // Creative: no durability loss
        int32_t dmg = static_cast<int32_t>(incomingDamage / 4.0f);
        if (dmg < 1) dmg = 1;
        // Armor inventory indices: 36=boots, 37=legs, 38=chest, 39=head
        for (int32_t i = 0; i < InventoryPlayer::ARMOR_SIZE; ++i) {
            int32_t invIdx = InventoryPlayer::MAIN_SIZE + i; // 36-39
            auto armor = inventory_.getStackInSlot(invIdx);
            if (!armor) continue;
            int32_t maxDur = getMaxDurability(armor->getItemId());
            if (maxDur <= 0) continue; // Not armor (shouldn't happen)
            int32_t newDamage = armor->getDamage() + dmg;
            if (newDamage > maxDur) {
                // Armor breaks
                inventory_.setInventorySlotContents(invIdx, std::nullopt);
            } else {
                armor->setDamage(newDamage);
                inventory_.setInventorySlotContents(invIdx, armor);
            }
        }
    }

    /**
     * Save player data to world/playerdata/<uuid>.dat
     * Java reference: SaveHandler.writePlayerData()
     */
    void savePlayerData();

    /**
     * Load player data from world/playerdata/<uuid>.dat
     * Java reference: SaveHandler.readPlayerData()
     * Returns true if data was loaded, false if no save exists.
     */
    bool loadPlayerData();

private:
    void handleKeepAlive(const uint8_t* data, size_t length, Connection& conn);
    void handleChatMessage(const uint8_t* data, size_t length, Connection& conn);
    void handleSteerVehicle(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerPosition(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerLook(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerPosAndLook(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerGround(const uint8_t* data, size_t length, Connection& conn);
    void handleClientSettings(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerDigging(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerBlockPlace(const uint8_t* data, size_t length, Connection& conn);
    void handleClickWindow(const uint8_t* data, size_t length, Connection& conn);
    void handleUseEntity(const uint8_t* data, size_t length, Connection& conn);
    void handleClientStatus(const uint8_t* data, size_t length, Connection& conn);
    void handleHeldItemChange(const uint8_t* data, size_t length, Connection& conn);
    void handleAnimation(const uint8_t* data, size_t length, Connection& conn);
    void handleEntityAction(const uint8_t* data, size_t length, Connection& conn);
    void handleTabComplete(const uint8_t* data, size_t length, Connection& conn);
    void handleCreativeInventory(const uint8_t* data, size_t length, Connection& conn);
    void handleEnchantItem(const uint8_t* data, size_t length, Connection& conn);

    // ─── Anvil helpers ───
    void updateAnvilOutput(Connection& conn);
    static bool getIsRepairable(int32_t itemId, int32_t materialId);
    static bool canApplyEnchantment(int32_t enchId, int32_t itemId);

    // ─── Villager trading helpers ───
    void updateMerchantOutput(Connection& conn);

    MinecraftServer& server_;
    std::string playerName_;
    std::string uuid_;
    int32_t entityId_ = 0;  // Assigned on login via atomic counter

    // Keep Alive tracking
    // Java: NetHandlerPlayServer.field_147378_h (keepAlive ID)
    int lastKeepAliveId_ = 0;
    int ticksSinceLastKeepAlive_ = 0;

    // Player position (serverside)
    double playerX_ = 0.0, playerY_ = 0.0, playerZ_ = 0.0;
    float playerYaw_ = 0.0f, playerPitch_ = 0.0f;
    bool playerOnGround_ = false;

    // C0C SteerVehicle input — Java: EntityLivingBase moveForward/moveStrafing
    float moveForward_ = 0.0f;
    float moveStrafing_ = 0.0f;
    int32_t ridingEntityId_ = -1;  // Entity this player is currently riding (-1 = none)

public:
    // Entity tracker: last sent position/rotation for relative movement
    // Java reference: EntityTrackerEntry fields. Public for MinecraftServer access.
    int32_t lastSentPosX_ = 0, lastSentPosY_ = 0, lastSentPosZ_ = 0; // fixed-point (value * 32)
    float lastSentYaw_ = 0.0f, lastSentPitch_ = 0.0f;
    int32_t ticksSinceLastTeleport_ = 0; // Force S18 every 400 ticks
private:

    // Chunk tracking — Java: PlayerManager tracks loaded chunks per player
    std::set<std::pair<int,int>> loadedChunks_;  // set of (chunkX, chunkZ)
    int lastChunkX_ = INT_MIN;  // sentinel: forces initial chunk load
    int lastChunkZ_ = INT_MIN;
    static constexpr int VIEW_DISTANCE = 7;  // 15×15 chunks (7 radius + center)

    // Static atomic entity ID counter
    // Java reference: Entity.nextEntityID (global counter)
    static std::atomic<int32_t> nextEntityId_;

    // Player inventory
    InventoryPlayer inventory_;
    std::unique_ptr<ContainerPlayer> container_;
    std::optional<ItemStack> cursorItem_; // Java: InventoryPlayer.getItemStack() — item on cursor

    // ─── Open window tracking (workbench/chest/furnace/etc.) ───
    int8_t openWindowId_ = 0;       // 0 = player inventory, >0 = open container
    int8_t openWindowType_ = -1;    // -1 = none, 1 = crafting_table
    int8_t nextWindowId_ = 1;       // Counter for window IDs (increments)

    // ─── Workbench (3×3 crafting table) ───
    // Java: ContainerWorkbench.craftMatrix (InventoryCrafting 3×3) + craftResult
    std::optional<ItemStack> workbenchGrid_[9]; // 3×3 crafting grid
    std::optional<ItemStack> workbenchResult_;   // Crafting output slot

    // ─── Chest container ───
    // Pointer(s) to server-side chest storage (owned by MinecraftServer::chestStorage_)
    // For double chests, chestInventory_ = upper/left half, chestInventory2_ = lower/right half
    // Java: InventoryLargeChest merges two TileEntityChest inventories
    std::array<std::optional<ItemStack>, 27>* chestInventory_ = nullptr;
    std::array<std::optional<ItemStack>, 27>* chestInventory2_ = nullptr; // nullptr if single chest
    int32_t chestSlotCount_ = 27; // 27 for single, 54 for double
    int32_t openChestX_ = 0, openChestY_ = 0, openChestZ_ = 0;
    int32_t openChest2X_ = 0, openChest2Y_ = 0, openChest2Z_ = 0; // second half position
    bool isEnderChest_ = false; // true when chestInventory_ points to enderChestInventory_
    bool isTrappedChest_ = false; // true when chest is a trapped chest (redstone output)

    // ─── Ender chest (per-player, 27 slots) ───
    std::array<std::optional<ItemStack>, 27> enderChestInventory_;

    // ─── Furnace container ───
    // Pointer to server-side furnace (owned by MinecraftServer::furnaceStorage_)
    // Typed as void* to avoid MinecraftServer forward-declaration issues;
    // cast to MinecraftServer::FurnaceData* in PacketHandler.cpp
    void* furnaceData_ = nullptr;
    int64_t openFurnaceKey_ = 0;

    // ─── Enchanting table ───
    // Java: ContainerEnchantment fields
    int32_t enchantTableX_ = 0, enchantTableY_ = 0, enchantTableZ_ = 0;
    int enchantLevels_[3] = {0, 0, 0};
    int bookshelfCount_ = 0;  // Java: ContainerEnchantment bookshelf count (cached)
    std::optional<ItemStack> enchantSlotItem_;  // Java: ContainerEnchantment.tableInventory slot 0

    // ─── Anvil container ───
    // Java: ContainerRepair fields
    std::optional<ItemStack> anvilSlots_[2];     // Input slots 0,1
    std::optional<ItemStack> anvilOutput_;        // Output slot 2 (computed)
    std::string anvilRepairedName_;               // Java: repairedItemName
    int32_t anvilMaxCost_ = 0;                    // Java: maximumCost (XP levels)
    int32_t anvilMaterialCost_ = 0;               // Java: materialCost

    // ─── Horse inventory ───
    // Java: ContainerHorseInventory — saddle slot 0, armor slot 1, chest slots 2-16
    int32_t horseEntityId_ = -1;                  // Entity ID of horse with open GUI
    int32_t horseSlotCount_ = 2;                  // 2 (normal horse) or 17 (chested donkey/mule)

    // ─── Villager trading ───
    // Java: ContainerMerchant — 3 slots (buy1, buy2, result) + trade list
    int32_t villagerEntityId_ = -1;               // Entity ID of villager being traded with
    std::vector<MerchantRecipe> villagerRecipes_;  // Copy of trades
    std::optional<ItemStack> merchantSlots_[3];    // Slot 0=buy1, 1=buy2, 2=result
    int32_t currentRecipeIndex_ = -1;             // Selected trade index

public:
    // Accessors for server-side furnace ticking
    int32_t getOpenWindowType() const { return openWindowType_; }
    int32_t getOpenWindowId() const { return openWindowId_; }
    int64_t getOpenFurnaceKey() const { return openFurnaceKey_; }
    void* getRawFurnaceData() const { return furnaceData_; }
private:

    // Combat state — Java: EntityPlayer fields
    float health_ = 20.0f;        // EntityLivingBase.health
    FoodStats foodStats_;         // Java: EntityPlayer.foodStats
    int hurtResistantTime_ = 0;   // EntityLivingBase.hurtResistantTime (20 tick cooldown)
    bool dead_ = false;           // EntityLivingBase.dead
    float fallDistance_ = 0.0f;   // Entity.fallDistance — accumulated fall distance
    int32_t airSupply_ = 300;     // Entity.air — ticks of air remaining (300 = 15 seconds)
    int32_t fireTicks_ = 0;       // Entity.fire — ticks remaining on fire (>0 = burning)
    int64_t bowChargeStartTick_ = -1; // ItemBow: server tick when bow draw started (-1 = not drawing)

    // ─── Dimension / portal state ────────────────────────────────────
    int32_t playerDimension_ = 0;   // Current dimension (0=overworld, -1=nether, 1=end)
    int32_t portalCooldown_ = 0;    // Ticks until portal re-entry allowed (Java: 300 = 15s)
    int32_t portalTicks_ = 0;       // Ticks player has been standing in portal block

    // Pressure plate tracking — deactivate when player steps off
    int32_t pressurePlateX_ = INT_MIN, pressurePlateY_ = INT_MIN, pressurePlateZ_ = INT_MIN;

    // Detector rail tracking — Java: BlockRailDetector.onEntityCollidedWithBlock
    int32_t detectorRailX_ = INT_MIN, detectorRailY_ = INT_MIN, detectorRailZ_ = INT_MIN;

    // Game mode — Java: ItemInWorldManager.theGameType
    int32_t gameMode_ = 0;        // 0=Survival, 1=Creative, 2=Adventure, 3=Spectator

    // Held item & movement state
    int16_t currentSlot_ = 0;      // InventoryPlayer.currentItem (0-8)
    bool isSneaking_ = false;      // EntityPlayer.isSneaking
    bool isSprinting_ = false;     // EntityPlayer.isSprinting

    // Item drop throttle — Java: NetHandlerPlayServer.itemDropThreshold
    int32_t itemDropThreshold_ = 0;

    // ─── Active potion effects ──────────────────────────────────────
    // Java: EntityLivingBase.activePotionsMap (HashMap<Integer, PotionEffect>)
    struct ActiveEffect {
        int32_t effectId;    // Potion ID (1=speed, 2=slowness, etc.)
        int32_t amplifier;   // 0-based level (0=I, 1=II, etc.)
        int32_t duration;    // Remaining ticks
    };
    std::map<int32_t, ActiveEffect> activePotionEffects_;

    // ─── Experience (Java: EntityPlayer) ─────────────────────────────
    float experienceBar_ = 0.0f;    // Java: experience (0.0-1.0 progress within level)
    int32_t experienceLevel_ = 0;   // Java: experienceLevel
    int32_t experienceTotal_ = 0;   // Java: experienceTotal

    // Java: EntityPlayer.xpBarCap() — XP needed for next level
    int32_t xpBarCap() const {
        if (experienceLevel_ >= 30) return 62 + (experienceLevel_ - 30) * 7;
        if (experienceLevel_ >= 15) return 17 + (experienceLevel_ - 15) * 3;
        return 7 + experienceLevel_ * 2;
    }

    // Java: EntityPlayer.addExperience(int)
    void addExperience(int32_t amount) {
        if (amount <= 0) return;
        int32_t cap = Integer_MAX - experienceTotal_;
        if (amount > cap) amount = cap;
        experienceBar_ += static_cast<float>(amount) / static_cast<float>(xpBarCap());
        experienceTotal_ += amount;
        while (experienceBar_ >= 1.0f) {
            experienceBar_ = (experienceBar_ - 1.0f) * static_cast<float>(xpBarCap());
            ++experienceLevel_;
            experienceBar_ /= static_cast<float>(xpBarCap());
        }
    }

    static constexpr int32_t Integer_MAX = 2147483647;
};

} // namespace mccpp
