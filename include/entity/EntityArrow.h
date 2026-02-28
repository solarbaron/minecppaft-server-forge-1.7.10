/**
 * EntityArrow.h — Arrow projectile entity.
 *
 * Java reference: net.minecraft.entity.projectile.EntityArrow (392 lines)
 *
 * Architecture:
 *   - Size: 0.5×0.5
 *   - Initial velocity: direction from shooter yaw/pitch, normalized,
 *     with Gaussian spread (0.0075 * inaccuracy), scaled by speed
 *   - setThrowableHeading: normalize → add Gaussian noise → multiply speed
 *   - Flight physics:
 *     - Gravity: 0.05 per tick
 *     - Air friction: 0.99 (all axes)
 *     - Water friction: 0.8 (replaces 0.99)
 *     - Rotation: atan2(motionX, motionZ) for yaw, atan2(motionY, horizSpeed) for pitch
 *     - Rotation smoothing: prev + (current - prev) * 0.2
 *   - Collision:
 *     - Block raytrace from pos to pos+motion
 *     - Entity AABB scan with 0.3 expansion, skip shooter for first 5 ticks
 *     - Closest entity hit wins
 *   - Entity hit:
 *     - Damage = ceil(velocity_magnitude * base_damage)
 *     - Critical: +rand(damage/2 + 2)
 *     - Knockback: 0.6 * knockbackStrength / horizSpeed, +0.1 Y
 *     - Fire: 5 seconds if arrow burning, not if Enderman
 *     - Enderman: immune to arrows entirely
 *     - Arrow count increment on living entities
 *     - Player hit notification (S2B GameState 6)
 *     - Bounce on failed damage: motion *= -0.1, yaw += 180
 *   - Block hit:
 *     - Store block position + block ID + metadata
 *     - Embed position: back up 0.05 * normalized_motion
 *     - arrowShake = 7, clear critical flag
 *     - inGround = true
 *   - Ground state:
 *     - Check if block changed → dislodge (motion *= rand*0.2)
 *     - Despawn after 1200 ticks in ground (1 minute)
 *   - Pickup:
 *     - canBePickedUp: 0=none, 1=survival, 2=creative
 *     - Only when inGround && arrowShake == 0
 *     - Drops arrow item if canBePickedUp==1 and room in inventory
 *   - DataWatcher slot 16: byte, bit 0 = critical flag
 *   - NBT: xTile, yTile, zTile (short), life (short), inTile (byte),
 *          inData (byte), shake (byte), inGround (byte), pickup (byte), damage (double)
 *   - Base damage: 2.0
 *   - Render distance weight: 10.0
 *
 * Thread safety: Entity tick on single thread per entity.
 * JNI readiness: DataWatcher slot 16 for critical flag.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <optional>

namespace mccpp {

class EntityArrow {
public:
    // ─── Entity base fields ───
    double posX = 0, posY = 0, posZ = 0;
    double prevPosX = 0, prevPosY = 0, prevPosZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    float prevRotationYaw = 0, prevRotationPitch = 0;
    bool isDead = false;
    bool isBurning = false;
    bool isInWater = false;
    int32_t entityId = 0;
    int32_t ticksExisted = 0;

    // ─── Arrow-specific ───
    int32_t blockX = -1, blockY = -1, blockZ = -1; // field_145791/2/89
    int32_t inBlockId = 0;     // field_145790_g
    int32_t inData = 0;
    bool inGround = false;
    int32_t canBePickedUp = 0; // 0=none, 1=survival, 2=creative
    int32_t arrowShake = 0;
    int32_t shooterEntityId = -1;
    int32_t ticksInGround = 0;
    int32_t ticksInAir = 0;
    double damage = 2.0;
    int32_t knockbackStrength = 0;
    bool isCritical = false;

    // ─── Constants ───
    static constexpr float WIDTH = 0.5f;
    static constexpr float HEIGHT = 0.5f;
    static constexpr float GRAVITY = 0.05f;
    static constexpr float AIR_FRICTION = 0.99f;
    static constexpr float WATER_FRICTION = 0.8f;
    static constexpr float ENTITY_EXPAND = 0.3f;
    static constexpr float EMBED_OFFSET = 0.05f;
    static constexpr float GAUSSIAN_SPREAD = 0.0075f;
    static constexpr float KNOCKBACK_FACTOR = 0.6f;
    static constexpr float BOUNCE_FACTOR = -0.1f;
    static constexpr int32_t GROUND_DESPAWN = 1200; // 1 minute
    static constexpr int32_t SHOOTER_GRACE = 5;     // Can't hit shooter for 5 ticks
    static constexpr int32_t BLOCK_HIT_SHAKE = 7;
    static constexpr double RENDER_DIST_WEIGHT = 10.0;

    // ═══════════════════════════════════════════════════════════════════════
    // setThrowableHeading — Normalize, add Gaussian spread, multiply speed.
    // Java: EntityArrow.setThrowableHeading
    //   - Normalize to unit vector
    //   - Add Gaussian * (±1) * 0.0075 * inaccuracy to each component
    //   - Multiply by speed
    //   - Set rotation from resulting motion
    // ═══════════════════════════════════════════════════════════════════════

    struct InitParams {
        double dx, dy, dz;
        float speed;
        float inaccuracy;
        double gaussianX, gaussianY, gaussianZ;
        bool signX, signY, signZ;
    };

    void setThrowableHeading(const InitParams& p) {
        double dx = p.dx, dy = p.dy, dz = p.dz;

        // Normalize
        double mag = std::sqrt(dx*dx + dy*dy + dz*dz);
        if (mag < 1e-7) return;
        dx /= mag; dy /= mag; dz /= mag;

        // Add Gaussian spread
        dx += p.gaussianX * (p.signX ? -1.0 : 1.0) * GAUSSIAN_SPREAD * p.inaccuracy;
        dy += p.gaussianY * (p.signY ? -1.0 : 1.0) * GAUSSIAN_SPREAD * p.inaccuracy;
        dz += p.gaussianZ * (p.signZ ? -1.0 : 1.0) * GAUSSIAN_SPREAD * p.inaccuracy;

        // Scale by speed
        dx *= p.speed; dy *= p.speed; dz *= p.speed;

        motionX = dx;
        motionY = dy;
        motionZ = dz;

        // Set rotation
        float horizSpeed = static_cast<float>(std::sqrt(dx*dx + dz*dz));
        prevRotationYaw = rotationYaw = static_cast<float>(std::atan2(dx, dz) * 180.0 / M_PI);
        prevRotationPitch = rotationPitch = static_cast<float>(std::atan2(dy, horizSpeed) * 180.0 / M_PI);

        ticksInGround = 0;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Spawn from shooter — compute initial position and direction.
    // Java: EntityArrow(world, shooter, speed)
    //   - Position: shooter pos + eyeHeight - 0.1 Y
    //   - Offset X/Z by -cos/sin(yaw) * 0.16
    //   - Motion from yaw/pitch: -sin(yaw)*cos(pitch), -sin(pitch), cos(yaw)*cos(pitch)
    //   - Speed multiplied by 1.5
    // ═══════════════════════════════════════════════════════════════════════

    struct ShooterInfo {
        double posX, posY, posZ;
        float eyeHeight;
        float yaw, pitch;
        bool isPlayer;
    };

    void spawnFromShooter(const ShooterInfo& shooter, float speed, const InitParams& headingParams) {
        if (shooter.isPlayer) canBePickedUp = 1;

        float yawRad = shooter.yaw / 180.0f * static_cast<float>(M_PI);
        float pitchRad = shooter.pitch / 180.0f * static_cast<float>(M_PI);

        posX = shooter.posX - std::cos(yawRad) * 0.16;
        posY = shooter.posY + shooter.eyeHeight - 0.1;
        posZ = shooter.posZ - std::sin(yawRad) * 0.16;

        rotationYaw = shooter.yaw;
        rotationPitch = shooter.pitch;

        motionX = -std::sin(yawRad) * std::cos(pitchRad);
        motionZ = std::cos(yawRad) * std::cos(pitchRad);
        motionY = -std::sin(pitchRad);

        InitParams p = headingParams;
        p.dx = motionX; p.dy = motionY; p.dz = motionZ;
        p.speed = speed * 1.5f;
        setThrowableHeading(p);
    }

    // ═══════════════════════════════════════════════════════════════════════
    // onUpdate — Main flight physics tick.
    //
    // Returns what happened this tick for the caller to process collisions.
    // The actual raytrace and entity scanning is done by the world system;
    // this class handles the physics math.
    // ═══════════════════════════════════════════════════════════════════════

    enum class TickState {
        FLYING,        // Still in flight
        IN_GROUND,     // Stuck in block
        DEAD           // Should be removed
    };

    // ─── Ground state tick ───
    TickState tickGround(int32_t currentBlockId, int32_t currentBlockMeta) {
        if (arrowShake > 0) --arrowShake;

        // Check if block changed → dislodge
        if (currentBlockId != inBlockId || currentBlockMeta != inData) {
            inGround = false;
            // Random small motion when dislodged
            ticksInGround = 0;
            ticksInAir = 0;
            return TickState::FLYING;
        }

        ++ticksInGround;
        if (ticksInGround >= GROUND_DESPAWN) {
            isDead = true;
            return TickState::DEAD;
        }
        return TickState::IN_GROUND;
    }

    // ─── Hit block ───
    void onBlockHit(double hitX, double hitY, double hitZ,
                     int32_t bx, int32_t by, int32_t bz,
                     int32_t blockId, int32_t blockMeta) {
        blockX = bx; blockY = by; blockZ = bz;
        inBlockId = blockId;
        inData = blockMeta;

        // Set motion to remaining distance
        motionX = hitX - posX;
        motionY = hitY - posY;
        motionZ = hitZ - posZ;

        // Back up slightly from hit point
        double mag = std::sqrt(motionX*motionX + motionY*motionY + motionZ*motionZ);
        if (mag > 1e-7) {
            posX -= motionX / mag * EMBED_OFFSET;
            posY -= motionY / mag * EMBED_OFFSET;
            posZ -= motionZ / mag * EMBED_OFFSET;
        }

        inGround = true;
        arrowShake = BLOCK_HIT_SHAKE;
        isCritical = false;
    }

    // ─── Hit entity — calculate damage ───
    struct HitResult {
        int32_t damage;
        bool knockback;
        double knockbackX, knockbackZ;
        double knockbackY;
        bool setFire;
        int32_t fireTicks;
        bool died; // arrow dies
    };

    HitResult onEntityHit(bool targetIsEnderman, bool targetTookDamage,
                            int32_t critBonusRand) {
        HitResult result{};

        if (targetIsEnderman) {
            // Enderman immune to arrows
            result.died = false;
            return result;
        }

        // Calculate damage: ceil(velocity * baseDamage)
        double speed = std::sqrt(motionX*motionX + motionY*motionY + motionZ*motionZ);
        result.damage = static_cast<int32_t>(std::ceil(speed * damage));

        // Critical bonus
        if (isCritical) {
            result.damage += critBonusRand; // caller provides rand.nextInt(damage/2 + 2)
        }

        // Fire
        if (isBurning) {
            result.setFire = true;
            result.fireTicks = 5;
        }

        // Knockback
        if (knockbackStrength > 0) {
            float horizSpeed = static_cast<float>(std::sqrt(motionX*motionX + motionZ*motionZ));
            if (horizSpeed > 0) {
                result.knockback = true;
                result.knockbackX = motionX * knockbackStrength * KNOCKBACK_FACTOR / horizSpeed;
                result.knockbackZ = motionZ * knockbackStrength * KNOCKBACK_FACTOR / horizSpeed;
                result.knockbackY = 0.1;
            }
        }

        if (targetTookDamage) {
            result.died = true;
        } else {
            // Bounce off
            motionX *= BOUNCE_FACTOR;
            motionY *= BOUNCE_FACTOR;
            motionZ *= BOUNCE_FACTOR;
            rotationYaw += 180.0f;
            prevRotationYaw += 180.0f;
            ticksInAir = 0;
            result.died = false;
        }

        return result;
    }

    // ─── Flight physics — called each tick when not inGround ───
    void tickFlight() {
        ++ticksInAir;

        // Position update
        posX += motionX;
        posY += motionY;
        posZ += motionZ;

        // Update rotation from motion
        float horizSpeed = static_cast<float>(std::sqrt(motionX*motionX + motionZ*motionZ));
        rotationYaw = static_cast<float>(std::atan2(motionX, motionZ) * 180.0 / M_PI);
        rotationPitch = static_cast<float>(std::atan2(motionY, horizSpeed) * 180.0 / M_PI);

        // Wrap rotation deltas
        while (rotationPitch - prevRotationPitch < -180.0f) prevRotationPitch -= 360.0f;
        while (rotationPitch - prevRotationPitch >= 180.0f) prevRotationPitch += 360.0f;
        while (rotationYaw - prevRotationYaw < -180.0f) prevRotationYaw -= 360.0f;
        while (rotationYaw - prevRotationYaw >= 180.0f) prevRotationYaw += 360.0f;

        // Smooth rotation (0.2 interpolation)
        rotationPitch = prevRotationPitch + (rotationPitch - prevRotationPitch) * 0.2f;
        rotationYaw = prevRotationYaw + (rotationYaw - prevRotationYaw) * 0.2f;

        // Friction
        float friction = isInWater ? WATER_FRICTION : AIR_FRICTION;
        motionX *= friction;
        motionY *= friction;
        motionZ *= friction;

        // Gravity
        motionY -= GRAVITY;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Pickup
    // Java: EntityArrow.onCollideWithPlayer
    //   - Must be inGround && arrowShake == 0
    //   - canBePickedUp: 1 = only if room in inventory
    //   - canBePickedUp: 2 = creative mode always
    // ═══════════════════════════════════════════════════════════════════════

    struct PickupResult {
        bool pickedUp;
        bool giveArrowItem; // Only if canBePickedUp == 1
    };

    PickupResult tryPickup(bool isCreativeMode, bool hasInventoryRoom) {
        if (!inGround || arrowShake > 0) return {false, false};

        if (canBePickedUp == 2 && isCreativeMode) {
            isDead = true;
            return {true, false};
        }

        if (canBePickedUp == 1) {
            if (hasInventoryRoom) {
                isDead = true;
                return {true, true};
            }
            return {false, false};
        }

        return {false, false};
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Setters/Getters
    // ═══════════════════════════════════════════════════════════════════════

    void setDamage(double d) { damage = d; }
    double getDamage() const { return damage; }
    void setKnockbackStrength(int32_t k) { knockbackStrength = k; }
    void setIsCritical(bool c) { isCritical = c; }
    bool getIsCritical() const { return isCritical; }

    // Check if shooter grace period active
    bool isShooterGraceActive() const { return ticksInAir < SHOOTER_GRACE; }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityThrowable — Base for thrown projectiles (ender pearl, snowball, etc.)
//
// Java reference: net.minecraft.entity.projectile.EntityThrowable
// Simpler than EntityArrow:
//   - Gravity: 0.03 per tick
//   - Friction: 0.99 air, 0.8 water
//   - Initial velocity: from shooter yaw/pitch, speed 1.5, inaccuracy 1.0
//   - Offset: eye height - 0.1
//   - No damage tracking, no ground embedding
//   - Dies on any hit (block or entity) → onImpact callback
//   - Despawn after 1200 ticks (not in ground, just age)
// ═══════════════════════════════════════════════════════════════════════════

class EntityThrowable {
public:
    double posX = 0, posY = 0, posZ = 0;
    double prevPosX = 0, prevPosY = 0, prevPosZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    float prevRotationYaw = 0, prevRotationPitch = 0;
    bool isDead = false;
    bool isInWater = false;
    int32_t entityId = 0;
    int32_t shooterEntityId = -1;
    int32_t ticksInAir = 0;

    static constexpr float WIDTH = 0.25f;
    static constexpr float HEIGHT = 0.25f;
    static constexpr float GRAVITY = 0.03f;
    static constexpr float AIR_FRICTION = 0.99f;
    static constexpr float WATER_FRICTION = 0.8f;
    static constexpr float ENTITY_EXPAND = 0.3f;
    static constexpr float DEFAULT_SPEED = 1.5f;
    static constexpr float DEFAULT_INACCURACY = 1.0f;
    static constexpr int32_t MAX_TICKS = 1200;

    // ─── Spawn from shooter ───
    // Java: EntityThrowable(world, shooter)
    //   - Position: x - cos(yaw)*0.16, y + eyeHeight - 0.1, z - sin(yaw)*0.16
    //   - Motion: -sin(yaw)*cos(pitch), -sin(pitch), cos(yaw)*cos(pitch)
    //   - Speed: 1.5, inaccuracy: 1.0

    void spawnFromShooter(double sx, double sy, double sz,
                           float eyeHeight, float yaw, float pitch,
                           float speed = DEFAULT_SPEED, float inaccuracy = DEFAULT_INACCURACY) {
        float yawRad = yaw / 180.0f * static_cast<float>(M_PI);
        float pitchRad = pitch / 180.0f * static_cast<float>(M_PI);

        posX = sx - std::cos(yawRad) * 0.16;
        posY = sy + eyeHeight - 0.1;
        posZ = sz - std::sin(yawRad) * 0.16;

        motionX = -std::sin(yawRad) * std::cos(pitchRad);
        motionZ = std::cos(yawRad) * std::cos(pitchRad);
        motionY = -std::sin(pitchRad);

        // Normalize and scale (simplified — caller does full setThrowableHeading)
        double mag = std::sqrt(motionX*motionX + motionY*motionY + motionZ*motionZ);
        if (mag > 1e-7) {
            motionX = motionX / mag * speed;
            motionY = motionY / mag * speed;
            motionZ = motionZ / mag * speed;
        }

        rotationYaw = yaw;
        rotationPitch = pitch;
    }

    // ─── Flight tick ───
    void tickFlight() {
        ++ticksInAir;

        posX += motionX;
        posY += motionY;
        posZ += motionZ;

        float horizSpeed = static_cast<float>(std::sqrt(motionX*motionX + motionZ*motionZ));
        rotationYaw = static_cast<float>(std::atan2(motionX, motionZ) * 180.0 / M_PI);
        rotationPitch = static_cast<float>(std::atan2(motionY, horizSpeed) * 180.0 / M_PI);

        while (rotationPitch - prevRotationPitch < -180.0f) prevRotationPitch -= 360.0f;
        while (rotationPitch - prevRotationPitch >= 180.0f) prevRotationPitch += 360.0f;
        while (rotationYaw - prevRotationYaw < -180.0f) prevRotationYaw -= 360.0f;
        while (rotationYaw - prevRotationYaw >= 180.0f) prevRotationYaw += 360.0f;

        rotationPitch = prevRotationPitch + (rotationPitch - prevRotationPitch) * 0.2f;
        rotationYaw = prevRotationYaw + (rotationYaw - prevRotationYaw) * 0.2f;

        float friction = isInWater ? WATER_FRICTION : AIR_FRICTION;
        motionX *= friction;
        motionY *= friction;
        motionZ *= friction;
        motionY -= GRAVITY;

        if (ticksInAir >= MAX_TICKS) {
            isDead = true;
        }
    }

    // ─── Impact — dies on any hit ───
    void onImpact() { isDead = true; }
};

// ═══════════════════════════════════════════════════════════════════════════
// Specific throwable types — differ only in onImpact behavior.
// Java classes: EntitySnowball, EntityEgg, EntityEnderPearl, EntityPotion
//
// EntitySnowball: 0 damage to most, 3 damage to Blaze, knockback
// EntityEgg: 1/8 chance to spawn chicken
// EntityEnderPearl: teleport thrower, 5 fall damage
// EntityPotion: splash effect in 4-block radius
// ═══════════════════════════════════════════════════════════════════════════

struct ThrowableType {
    enum Value : int32_t {
        SNOWBALL = 0,
        EGG = 1,
        ENDER_PEARL = 2,
        POTION = 3
    };

    // EntitySnowball: 0 damage, 3 to Blaze
    static constexpr int32_t SNOWBALL_BLAZE_DAMAGE = 3;

    // EntityEgg: 1/8 chance to spawn baby chicken
    static constexpr int32_t EGG_CHICKEN_CHANCE = 8; // 1 in 8

    // EntityEnderPearl: teleport + 5 damage to thrower
    static constexpr float PEARL_TELEPORT_DAMAGE = 5.0f;

    // EntityPotion: splash radius
    static constexpr double POTION_SPLASH_RADIUS = 4.0;
    static constexpr double POTION_SPLASH_RADIUS_SQ = 16.0;
};

} // namespace mccpp
