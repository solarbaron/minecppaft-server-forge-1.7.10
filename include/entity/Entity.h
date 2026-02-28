/**
 * Entity.h — Entity hierarchy: Entity, EntityLivingBase, EntityPlayer, EntityPlayerMP.
 *
 * Java references:
 *   - net.minecraft.entity.Entity
 *   - net.minecraft.entity.EntityLivingBase
 *   - net.minecraft.entity.player.EntityPlayer
 *   - net.minecraft.entity.player.EntityPlayerMP
 *
 * This implements the core entity data model needed for player management
 * and protocol packets. Game mechanics (collision, AI) will be added later.
 *
 * Thread safety:
 *   - Entity ID counter is atomic (thread-safe allocation).
 *   - Entity instances are single-owner (owned by the world that ticks them).
 *   - EntityPlayerMP may be accessed from the network thread for position
 *     updates; per-field atomics are used where necessary.
 *
 * JNI readiness: POD-like field layout, int/double types match Java exactly.
 */
#pragma once

#include <atomic>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <array>

namespace mccpp {

class Connection;  // forward decl
class WorldServer;  // forward decl

// ═══════════════════════════════════════════════════════════════════════════
// AxisAlignedBB — Simple bounding box.
// Java reference: net.minecraft.util.AxisAlignedBB
// ═══════════════════════════════════════════════════════════════════════════

struct AxisAlignedBB {
    double minX = 0.0, minY = 0.0, minZ = 0.0;
    double maxX = 0.0, maxY = 0.0, maxZ = 0.0;

    void setBounds(double x1, double y1, double z1, double x2, double y2, double z2) {
        minX = x1; minY = y1; minZ = z1;
        maxX = x2; maxY = y2; maxZ = z2;
    }

    void offset(double dx, double dy, double dz) {
        minX += dx; minY += dy; minZ += dz;
        maxX += dx; maxY += dy; maxZ += dz;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GameType — Survival, Creative, Adventure, Spectator
// Java reference: net.minecraft.world.WorldSettings.GameType
// ═══════════════════════════════════════════════════════════════════════════

enum class GameType : int32_t {
    Survival  = 0,
    Creative  = 1,
    Adventure = 2,
    // Note: Spectator is 1.8+, not in 1.7.10
};

// ═══════════════════════════════════════════════════════════════════════════
// Entity — Base class for all entities.
// Java reference: net.minecraft.entity.Entity
//
// Thread safety: nextEntityID_ is atomic for concurrent entity creation.
// ═══════════════════════════════════════════════════════════════════════════

class Entity {
public:
    Entity();
    virtual ~Entity() = default;

    // ─── Identity ──────────────────────────────────────────────────────
    int getEntityId() const { return entityId_; }
    void setEntityId(int id) { entityId_ = id; }

    // ─── Position / Motion / Rotation ──────────────────────────────────
    // Java: posX, posY, posZ, motionX, motionY, motionZ, rotationYaw, rotationPitch
    double posX = 0.0, posY = 0.0, posZ = 0.0;
    double prevPosX = 0.0, prevPosY = 0.0, prevPosZ = 0.0;
    double lastTickPosX = 0.0, lastTickPosY = 0.0, lastTickPosZ = 0.0;
    double motionX = 0.0, motionY = 0.0, motionZ = 0.0;
    float rotationYaw = 0.0f, rotationPitch = 0.0f;
    float prevRotationYaw = 0.0f, prevRotationPitch = 0.0f;

    // Java: Entity.setPosition(double, double, double)
    void setPosition(double x, double y, double z);

    // Java: Entity.setPositionAndRotation(double, double, double, float, float)
    void setPositionAndRotation(double x, double y, double z, float yaw, float pitch);

    // Java: Entity.setLocationAndAngles(double, double, double, float, float)
    void setLocationAndAngles(double x, double y, double z, float yaw, float pitch);

    // ─── Bounding box ──────────────────────────────────────────────────
    AxisAlignedBB boundingBox;
    float width = 0.6f;
    float height = 1.8f;
    float yOffset = 0.0f;
    float yOffset2 = 0.0f;
    float stepHeight = 0.0f;

    // ─── Status flags ──────────────────────────────────────────────────
    bool onGround = false;
    bool isDead = false;
    bool isCollidedHorizontally = false;
    bool isCollidedVertically = false;
    bool isCollided = false;
    bool velocityChanged = false;
    bool noClip = false;
    bool isAirBorne = false;
    bool isSneaking_ = false;
    bool isSprinting_ = false;

    // ─── Fire / Water ──────────────────────────────────────────────────
    int fire = 0;
    int fireResistance = 1;
    bool inWater = false;
    bool isImmuneToFire_ = false;

    // ─── Fall distance ─────────────────────────────────────────────────
    float fallDistance = 0.0f;

    // ─── Tick tracking ─────────────────────────────────────────────────
    int ticksExisted = 0;
    int hurtResistantTime = 0;
    int dimension = 0;

    // ─── Chunk tracking ────────────────────────────────────────────────
    bool addedToChunk = false;
    int chunkCoordX = 0, chunkCoordY = 0, chunkCoordZ = 0;

    // ─── UUID ──────────────────────────────────────────────────────────
    // Java: entityUniqueID = UUID.randomUUID()
    // For now stored as two 64-bit values (msb, lsb)
    uint64_t uuidMost = 0;
    uint64_t uuidLeast = 0;

    // ─── Virtual methods ───────────────────────────────────────────────
    virtual void onUpdate() {}
    void setDead() { isDead = true; }

    bool isSneaking() const { return isSneaking_; }
    void setSneaking(bool v) { isSneaking_ = v; }
    bool isSprinting() const { return isSprinting_; }
    void setSprinting(bool v) { isSprinting_ = v; }

    // ─── Distance ──────────────────────────────────────────────────────
    double getDistanceSq(double x, double y, double z) const {
        double dx = posX - x, dy = posY - y, dz = posZ - z;
        return dx*dx + dy*dy + dz*dz;
    }

    // ─── Velocity ──────────────────────────────────────────────────────
    void addVelocity(double dx, double dy, double dz) {
        motionX += dx; motionY += dy; motionZ += dz;
        isAirBorne = true;
    }

private:
    int entityId_;

    // Thread-safe global entity ID counter
    // Java: private static int nextEntityID (not atomic in Java, but we make it atomic)
    static std::atomic<int> nextEntityID_;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityLivingBase — Living entity with health, attributes, potion effects.
// Java reference: net.minecraft.entity.EntityLivingBase
// ═══════════════════════════════════════════════════════════════════════════

class EntityLivingBase : public Entity {
public:
    EntityLivingBase();

    // ─── Health ────────────────────────────────────────────────────────
    // Java: EntityLivingBase fields
    float getHealth() const { return health_; }
    void setHealth(float h) { health_ = h; }
    float getMaxHealth() const { return maxHealth_; }
    void setMaxHealth(float h) { maxHealth_ = h; }
    float getAbsorptionAmount() const { return absorptionAmount_; }
    void setAbsorptionAmount(float a) { absorptionAmount_ = a; }
    bool isEntityAlive() const { return !isDead && health_ > 0.0f; }

    // ─── Combat ────────────────────────────────────────────────────────
    int hurtTime = 0;
    int maxHurtTime = 0;
    float attackedAtYaw = 0.0f;
    int deathTime = 0;

    // ─── Movement ──────────────────────────────────────────────────────
    float moveForward = 0.0f;
    float moveStrafing = 0.0f;
    bool isJumping = false;

    // ─── Head rotation ─────────────────────────────────────────────────
    float rotationYawHead = 0.0f;
    float prevRotationYawHead = 0.0f;
    float renderYawOffset = 0.0f;
    float prevRenderYawOffset = 0.0f;

    // ─── Air supply ────────────────────────────────────────────────────
    // Java: dataWatcher index 1 = air (Short, default 300)
    int16_t air = 300;

    // ─── Equipment (5 slots: held + 4 armor) ───────────────────────────
    // Simplified: just track item IDs for now (full ItemStack later)
    // Java: getEquipmentInSlot(0-4)
    int32_t heldItemId = 0;
    std::array<int32_t, 4> armorIds{0, 0, 0, 0};

private:
    float health_ = 20.0f;
    float maxHealth_ = 20.0f;
    float absorptionAmount_ = 0.0f;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityPlayer — Player entity with inventory, game mode, XP, food.
// Java reference: net.minecraft.entity.player.EntityPlayer
// ═══════════════════════════════════════════════════════════════════════════

class EntityPlayer : public EntityLivingBase {
public:
    EntityPlayer();

    // ─── Player identity ───────────────────────────────────────────────
    // Java: GameProfile (name + UUID)
    std::string playerName;
    // UUID stored in base Entity class

    // ─── Game mode ─────────────────────────────────────────────────────
    // Java: PlayerInteractionManager.getGameType()
    GameType gameType = GameType::Survival;

    // ─── Experience ────────────────────────────────────────────────────
    // Java: EntityPlayer fields
    float experience = 0.0f;          // 0.0-1.0 progress in current level
    int experienceLevel = 0;
    int experienceTotal = 0;

    // ─── Food stats ────────────────────────────────────────────────────
    // Java: FoodStats (simplified inline)
    int foodLevel = 20;
    float foodSaturationLevel = 5.0f;
    float foodExhaustionLevel = 0.0f;

    // ─── Score ──────────────────────────────────────────────────────────
    int score = 0;

    // ─── Capabilities ──────────────────────────────────────────────────
    // Java: PlayerCapabilities
    bool isFlying = false;
    bool allowFlying = false;
    bool isCreativeMode = false;
    bool disableDamage = false;
    float flySpeed = 0.05f;
    float walkSpeed = 0.1f;

    // ─── Bed/Spawn ─────────────────────────────────────────────────────
    bool sleeping = false;
    int sleepTimer = 0;

    // Player-specific constants
    // Java: EntityPlayer constructor sets size(0.6, 1.8)
    static constexpr float PLAYER_WIDTH = 0.6f;
    static constexpr float PLAYER_HEIGHT = 1.8f;
    static constexpr float PLAYER_EYE_HEIGHT = 1.62f; // standing
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityPlayerMP — Server-side player with network connection.
// Java reference: net.minecraft.entity.player.EntityPlayerMP
//
// This is the main server-side player entity. It holds the network
// connection, tracks loaded chunks, and manages server-side state.
// ═══════════════════════════════════════════════════════════════════════════

class EntityPlayerMP : public EntityPlayer {
public:
    EntityPlayerMP(const std::string& name, uint64_t uuidMsb, uint64_t uuidLsb);

    // ─── Network ───────────────────────────────────────────────────────
    // Java: playerNetServerHandler (NetHandlerPlayServer)
    std::shared_ptr<Connection> connection;

    // ─── Chunk tracking ────────────────────────────────────────────────
    // Java: loadedChunks, managedPosX/Z for PlayerManager
    double managedPosX = 0.0;
    double managedPosZ = 0.0;

    // ─── Ping ──────────────────────────────────────────────────────────
    // Java: EntityPlayerMP.ping
    int ping = 0;

    // ─── Language ──────────────────────────────────────────────────────
    // Java: EntityPlayerMP.translator = "en_US"
    std::string language = "en_US";

    // ─── View distance ─────────────────────────────────────────────────
    int renderDistance = 8;

    // ─── Chat settings ─────────────────────────────────────────────────
    int chatVisibility = 0; // 0=full, 1=system, 2=hidden
    bool chatColours = true;

    // ─── Container ─────────────────────────────────────────────────────
    // Java: currentWindowId — incremented for each opened container
    int currentWindowId = 0;

    // ─── Last known position (for movement validation) ─────────────────
    double lastGoodX = 0.0, lastGoodY = 0.0, lastGoodZ = 0.0;

    // ─── Invulnerability timer ─────────────────────────────────────────
    // Java: joinInvulnerabilityTicks (initially 60 ticks / 3 seconds)
    int invulnerabilityTicks = 60;

    // ─── Tick methods ──────────────────────────────────────────────────
    void onUpdate() override;
};

} // namespace mccpp
