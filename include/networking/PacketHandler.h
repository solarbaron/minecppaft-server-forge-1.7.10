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
    bool isSprinting() const { return isSprinting_; }

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
    void handlePlayerPosition(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerLook(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerPosAndLook(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerGround(const uint8_t* data, size_t length, Connection& conn);
    void handleClientSettings(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerDigging(const uint8_t* data, size_t length, Connection& conn);
    void handlePlayerBlockPlace(const uint8_t* data, size_t length, Connection& conn);
    void handleUseEntity(const uint8_t* data, size_t length, Connection& conn);
    void handleClientStatus(const uint8_t* data, size_t length, Connection& conn);
    void handleHeldItemChange(const uint8_t* data, size_t length, Connection& conn);
    void handleAnimation(const uint8_t* data, size_t length, Connection& conn);
    void handleEntityAction(const uint8_t* data, size_t length, Connection& conn);
    void handleTabComplete(const uint8_t* data, size_t length, Connection& conn);
    void handleCreativeInventory(const uint8_t* data, size_t length, Connection& conn);

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

    // Combat state — Java: EntityPlayer fields
    float health_ = 20.0f;        // EntityLivingBase.health
    FoodStats foodStats_;         // Java: EntityPlayer.foodStats
    int hurtResistantTime_ = 0;   // EntityLivingBase.hurtResistantTime (20 tick cooldown)
    bool dead_ = false;           // EntityLivingBase.dead
    float fallDistance_ = 0.0f;   // Entity.fallDistance — accumulated fall distance

    // Game mode — Java: ItemInWorldManager.theGameType
    int32_t gameMode_ = 0;        // 0=Survival, 1=Creative, 2=Adventure, 3=Spectator

    // Held item & movement state
    int16_t currentSlot_ = 0;      // InventoryPlayer.currentItem (0-8)
    bool isSneaking_ = false;      // EntityPlayer.isSneaking
    bool isSprinting_ = false;     // EntityPlayer.isSprinting

    // Item drop throttle — Java: NetHandlerPlayServer.itemDropThreshold
    int32_t itemDropThreshold_ = 0;
};

} // namespace mccpp
