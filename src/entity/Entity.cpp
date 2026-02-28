/**
 * Entity.cpp — Entity hierarchy implementation.
 *
 * Java references:
 *   net.minecraft.entity.Entity
 *   net.minecraft.entity.EntityLivingBase
 *   net.minecraft.entity.player.EntityPlayer
 *   net.minecraft.entity.player.EntityPlayerMP
 *
 * Implements:
 *   - Entity: atomic ID counter, position/rotation/bounding box
 *   - EntityLivingBase: health defaults, player-size constructor
 *   - EntityPlayer: player constants setup
 *   - EntityPlayerMP: network player with per-tick processing
 *
 * Multi-threading adaptation:
 *   - Entity ID counter uses std::atomic for thread-safe allocation
 *   - UUID generation uses random_device + mt19937_64 (one-shot per entity)
 */

#include "entity/Entity.h"

#include <random>

namespace mccpp {

// ═════════════════════════════════════════════════════════════════════════════
// Entity
// ═════════════════════════════════════════════════════════════════════════════

// Java: private static int nextEntityID = 0;
// Made atomic for thread-safe entity creation
std::atomic<int> Entity::nextEntityID_{0};

Entity::Entity()
    : entityId_(nextEntityID_.fetch_add(1, std::memory_order_relaxed))
{
    // Generate random UUID (version 4)
    // Java: entityUniqueID = UUID.randomUUID()
    static thread_local std::mt19937_64 rng(std::random_device{}());
    uuidMost = rng();
    uuidLeast = rng();
    // Set version (4) and variant (2) bits per RFC 4122
    uuidMost = (uuidMost & ~(0xFULL << 12)) | (0x4ULL << 12);
    uuidLeast = (uuidLeast & ~(0x3ULL << 62)) | (0x2ULL << 62);
}

void Entity::setPosition(double x, double y, double z) {
    // Java: Entity.setPosition(double, double, double)
    posX = x;
    posY = y;
    posZ = z;
    float halfW = width / 2.0f;
    float h = height;
    boundingBox.setBounds(
        x - halfW, y - yOffset + yOffset2, z - halfW,
        x + halfW, y - yOffset + yOffset2 + h, z + halfW
    );
}

void Entity::setPositionAndRotation(double x, double y, double z, float yaw, float pitch) {
    // Java: Entity.setPositionAndRotation(double, double, double, float, float)
    prevPosX = posX = x;
    prevPosY = posY = y;
    prevPosZ = posZ = z;
    prevRotationYaw = rotationYaw = yaw;
    prevRotationPitch = rotationPitch = pitch;
    yOffset2 = 0.0f;

    // Normalize prevRotationYaw
    double delta = prevRotationYaw - yaw;
    if (delta < -180.0) prevRotationYaw += 360.0f;
    if (delta >= 180.0) prevRotationYaw -= 360.0f;

    setPosition(posX, posY, posZ);
    rotationYaw = std::fmod(yaw, 360.0f);
    rotationPitch = std::fmod(pitch, 360.0f);
}

void Entity::setLocationAndAngles(double x, double y, double z, float yaw, float pitch) {
    // Java: Entity.setLocationAndAngles(double, double, double, float, float)
    prevPosX = posX = x;
    lastTickPosX = posX;
    prevPosY = posY = y + yOffset;
    lastTickPosY = posY;
    prevPosZ = posZ = z;
    lastTickPosZ = posZ;
    rotationYaw = yaw;
    rotationPitch = pitch;
    setPosition(posX, posY, posZ);
}

// ═════════════════════════════════════════════════════════════════════════════
// EntityLivingBase
// ═════════════════════════════════════════════════════════════════════════════

EntityLivingBase::EntityLivingBase() {
    // Java: EntityLivingBase.applyEntityAttributes()
    // Default max health = 20.0 (SharedMonsterAttributes.maxHealth)
    maxHealth_ = 20.0f;
    health_ = maxHealth_;

    // Java: EntityLivingBase width=0.6, height=1.8 (inherited from Entity)
    // stepHeight = 0.5 for living entities (set in subclasses like EntityPlayer)
    stepHeight = 0.6f;

    // Air supply default
    air = 300;
}

// ═════════════════════════════════════════════════════════════════════════════
// EntityPlayer
// ═════════════════════════════════════════════════════════════════════════════

EntityPlayer::EntityPlayer() {
    // Java: EntityPlayer constructor
    // setSize(0.6f, 1.8f)
    width = PLAYER_WIDTH;
    height = PLAYER_HEIGHT;

    // Java: EntityPlayer.applyEntityAttributes()
    // maxHealth = 20.0 (already set in EntityLivingBase)
    // movementSpeed = 0.1
    // attackDamage = 1.0

    // Java: EntityPlayer yOffset = 1.62f (eye height)
    yOffset = PLAYER_EYE_HEIGHT;

    // Food stats defaults
    foodLevel = 20;
    foodSaturationLevel = 5.0f;
    foodExhaustionLevel = 0.0f;

    // Default game mode
    gameType = GameType::Survival;

    // Fire resistance
    fireResistance = 20;
}

// ═════════════════════════════════════════════════════════════════════════════
// EntityPlayerMP
// ═════════════════════════════════════════════════════════════════════════════

EntityPlayerMP::EntityPlayerMP(const std::string& name, uint64_t uuidMsb, uint64_t uuidLsb) {
    playerName = name;
    uuidMost = uuidMsb;
    uuidLeast = uuidLsb;

    // Java: EntityPlayerMP constructor
    // joinInvulnerabilityTicks = 60 (3 seconds)
    invulnerabilityTicks = 60;

    // Set managed position to current position
    managedPosX = posX;
    managedPosZ = posZ;

    // Set last good position
    lastGoodX = posX;
    lastGoodY = posY;
    lastGoodZ = posZ;
}

void EntityPlayerMP::onUpdate() {
    // Java: EntityPlayerMP.onUpdate()
    // Simplified server tick for player entity

    // Update previous position
    prevPosX = posX;
    prevPosY = posY;
    prevPosZ = posZ;
    prevRotationYaw = rotationYaw;
    prevRotationPitch = rotationPitch;
    prevRotationYawHead = rotationYawHead;
    prevRenderYawOffset = renderYawOffset;

    // Tick invulnerability
    if (invulnerabilityTicks > 0) {
        --invulnerabilityTicks;
    }

    // Tick food exhaustion
    // Java: EntityPlayer.onUpdate() calls FoodStats.onUpdate()
    // Simplified: just decrement food level on exhaustion threshold
    if (gameType == GameType::Survival) {
        // Every 80 ticks of exhaustion >= 4.0, decrease food by 1
        if (foodExhaustionLevel >= 4.0f) {
            foodExhaustionLevel -= 4.0f;
            if (foodSaturationLevel > 0.0f) {
                foodSaturationLevel = std::max(0.0f, foodSaturationLevel - 1.0f);
            } else {
                foodLevel = std::max(0, foodLevel - 1);
            }
        }
    }

    ++ticksExisted;
}

} // namespace mccpp
