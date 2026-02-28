/**
 * EntityTNTPrimed.h — TNT primed entity and Boat vehicle entity.
 *
 * Java references:
 *   - net.minecraft.entity.item.EntityTNTPrimed (96 lines)
 *   - net.minecraft.entity.item.EntityBoat (368 lines)
 *
 * EntityTNTPrimed:
 *   - Size: 0.98×0.98
 *   - Gravity: 0.04 per tick
 *   - All-axis friction: 0.98
 *   - Ground friction: 0.7 (X, Z), bounce: -0.5 (Y)
 *   - Initial motion: random radial ±sin/cos(rand*2PI)*0.02, Y=0.2
 *   - Fuse: 80 ticks (4 seconds), counts down
 *   - Explosion: radius 4.0, fire=true (createExplosion)
 *   - Particles: "smoke" at Y+0.5 while fuse > 0
 *   - canBeCollidedWith: !isDead
 *   - preventEntitySpawning: true
 *   - NBT: Fuse (byte)
 *
 * EntityBoat:
 *   - Size: 1.5×0.6
 *   - Water depth: 5-layer sampling of bounding box
 *   - Buoyancy: submerged < 1.0 → motionY += 0.04*(2*depth-1)
 *              submerged ≥ 1.0 → halve negative Y, motionY += 0.007
 *   - Rider steering: yaw + (-moveStrafing * 90), forward * 0.05 * speedMultiplier
 *   - Speed cap: 0.35 horizSpeed
 *   - Speed ramp: 0.07 → 0.35 over 35 ticks (accelerating), 0.35 → 0.07 (decelerating)
 *   - Ground friction: all axes * 0.5
 *   - Air friction: X/Z * 0.99, Y * 0.95
 *   - Snow/waterlily destruction in 4-corner ±0.8 area
 *   - Wall collision at speed > 0.2: break → 3 planks + 2 sticks
 *   - Fall damage > 3.0: break → 3 planks + 2 sticks
 *   - Damage: hit → timeSinceHit=10, damageTaken += f*10, break at > 40
 *   - Mounted Y offset: height * 0.0 - 0.3
 *   - Rider position: posX + cos(yaw)*0.4, posZ + sin(yaw)*0.4
 *   - Yaw update: atan2(deltaZ, deltaX), clamped ±20° per tick
 *   - DataWatcher: slot 17 (timeSinceHit int), 18 (forwardDir int), 19 (damageTaken float)
 *   - No NBT save/load
 *
 * Thread safety: Entity tick on single thread per entity.
 */
#pragma once

#include <cmath>
#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityTNTPrimed — Primed TNT entity.
// Java: net.minecraft.entity.item.EntityTNTPrimed (96 lines)
// ═══════════════════════════════════════════════════════════════════════════

class EntityTNTPrimed {
public:
    // ─── Entity base ───
    double posX = 0, posY = 0, posZ = 0;
    double prevPosX = 0, prevPosY = 0, prevPosZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    bool onGround = false;
    bool isDead = false;
    int32_t entityId = 0;

    // ─── TNT-specific ───
    int32_t fuse = 80;
    int32_t placedByEntityId = -1;

    // ─── Constants ───
    static constexpr float WIDTH = 0.98f;
    static constexpr float HEIGHT = 0.98f;
    static constexpr float GRAVITY = 0.04f;
    static constexpr float FRICTION = 0.98f;
    static constexpr float GROUND_FRICTION_XZ = 0.7f;
    static constexpr float GROUND_BOUNCE_Y = -0.5f;
    static constexpr float INITIAL_MOTION_Y = 0.2f;
    static constexpr float INITIAL_MOTION_HORIZ = 0.02f;
    static constexpr float EXPLOSION_RADIUS = 4.0f;
    static constexpr bool EXPLOSION_FIRE = true;

    // ═══════════════════════════════════════════════════════════════════════
    // Spawn — random radial ejection.
    // Java: EntityTNTPrimed(world, x, y, z, placer)
    //   angle = rand * 2PI
    //   motionX = -sin(angle) * 0.02
    //   motionY = 0.2
    //   motionZ = -cos(angle) * 0.02
    //   fuse = 80
    // ═══════════════════════════════════════════════════════════════════════

    void spawn(double x, double y, double z, double randAngle) {
        posX = x; posY = y; posZ = z;
        prevPosX = x; prevPosY = y; prevPosZ = z;
        motionX = -std::sin(randAngle) * INITIAL_MOTION_HORIZ;
        motionY = INITIAL_MOTION_Y;
        motionZ = -std::cos(randAngle) * INITIAL_MOTION_HORIZ;
        fuse = 80;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // onUpdate — Physics and fuse countdown.
    // Java: EntityTNTPrimed.onUpdate
    //   - Gravity: 0.04
    //   - moveEntity
    //   - All friction: 0.98
    //   - Ground: XZ * 0.7, Y * -0.5
    //   - Fuse countdown, explode at 0
    //   - Smoke particle at Y+0.5
    // ═══════════════════════════════════════════════════════════════════════

    struct TickResult {
        bool shouldExplode;
        bool shouldDie;
        bool showSmoke;
        double smokeX, smokeY, smokeZ;
    };

    TickResult onUpdate() {
        prevPosX = posX; prevPosY = posY; prevPosZ = posZ;

        // Gravity
        motionY -= GRAVITY;

        // Move (caller handles collision)
        // posX += motionX; posY += motionY; posZ += motionZ; // done by moveEntity

        // Friction
        motionX *= FRICTION;
        motionY *= FRICTION;
        motionZ *= FRICTION;

        if (onGround) {
            motionX *= GROUND_FRICTION_XZ;
            motionZ *= GROUND_FRICTION_XZ;
            motionY *= GROUND_BOUNCE_Y;
        }

        --fuse;
        if (fuse <= 0) {
            isDead = true;
            return {true, true, false, 0, 0, 0};
        }

        return {false, false, true, posX, posY + 0.5, posZ};
    }

    bool canBeCollidedWith() const { return !isDead; }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityBoat — Boat vehicle entity.
// Java: net.minecraft.entity.item.EntityBoat (368 lines)
// ═══════════════════════════════════════════════════════════════════════════

class EntityBoat {
public:
    // ─── Entity base ───
    double posX = 0, posY = 0, posZ = 0;
    double prevPosX = 0, prevPosY = 0, prevPosZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    bool onGround = false;
    bool isDead = false;
    bool isCollidedHorizontally = false;
    float fallDistance = 0;
    int32_t entityId = 0;
    int32_t riderEntityId = -1;

    // ─── Boat-specific ───
    double speedMultiplier = 0.07;
    int32_t timeSinceHit = 0;
    int32_t forwardDirection = 1;
    float damageTaken = 0.0f;

    // ─── Constants ───
    static constexpr float WIDTH = 1.5f;
    static constexpr float HEIGHT = 0.6f;
    static constexpr float MOUNTED_Y_OFFSET = -0.3f; // height * 0.0 - 0.3
    static constexpr float RIDER_OFFSET = 0.4f;
    static constexpr double SPEED_CAP = 0.35;
    static constexpr double SPEED_MIN = 0.07;
    static constexpr double SPEED_RAMP_RATE = 35.0;
    static constexpr float STEERING_FORCE = 0.05f;
    static constexpr float BUOYANCY_ABOVE = 0.04f;
    static constexpr float BUOYANCY_SUBMERGED = 0.007f;
    static constexpr float GROUND_FRICTION = 0.5f;
    static constexpr float AIR_FRICTION_XZ = 0.99f;
    static constexpr float AIR_FRICTION_Y = 0.95f;
    static constexpr float DAMAGE_THRESHOLD = 40.0f;
    static constexpr float DAMAGE_MULTIPLIER = 10.0f;
    static constexpr float FALL_BREAK_THRESHOLD = 3.0f;
    static constexpr float WALL_BREAK_SPEED = 0.2f;
    static constexpr float SPLASH_SPEED_THRESHOLD = 0.2625f; // 0.26249999999999996
    static constexpr float YAW_LIMIT = 20.0f;
    static constexpr int32_t HIT_TIME = 10;
    static constexpr int32_t WATER_SAMPLE_LAYERS = 5;

    // ─── Drop items when broken ───
    struct BreakDrops {
        static constexpr int32_t PLANKS_COUNT = 3;
        static constexpr int32_t PLANKS_ID = 5;  // Blocks.planks
        static constexpr int32_t STICKS_COUNT = 2;
        static constexpr int32_t STICKS_ID = 280; // Items.stick
    };

    // ═══════════════════════════════════════════════════════════════════════
    // Water depth — 5-layer buoyancy sampling.
    // Java: samples bounding box in 5 equal vertical layers,
    //   each layer ±0.125 offset, counts how many are in water.
    //   waterDepth = fraction submerged (0.0 to 1.0)
    // ═══════════════════════════════════════════════════════════════════════

    void applyBuoyancy(double waterDepth) {
        if (waterDepth < 1.0) {
            // Partially above water → buoyancy proportional to submersion
            double force = waterDepth * 2.0 - 1.0;
            motionY += BUOYANCY_ABOVE * force;
        } else {
            // Fully submerged → slow upward drift
            if (motionY < 0) motionY /= 2.0;
            motionY += BUOYANCY_SUBMERGED;
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Rider steering
    // Java: yaw + (-moveStrafing * 90), forward * 0.05 * speedMultiplier
    //   Speed cap at 0.35, ramp speedMultiplier towards 0.35 or 0.07
    // ═══════════════════════════════════════════════════════════════════════

    void applyRiderInput(float riderYaw, float moveStrafing, float moveForward) {
        float steerYaw = riderYaw + (-moveStrafing * 90.0f);
        float yawRad = steerYaw * static_cast<float>(M_PI) / 180.0f;
        motionX += -std::sin(yawRad) * speedMultiplier * moveForward * STEERING_FORCE;
        motionZ += std::cos(yawRad) * speedMultiplier * moveForward * STEERING_FORCE;
    }

    void clampSpeed(double prevHorizSpeed) {
        double horizSpeed = std::sqrt(motionX * motionX + motionZ * motionZ);

        // Cap at 0.35
        if (horizSpeed > SPEED_CAP) {
            double scale = SPEED_CAP / horizSpeed;
            motionX *= scale;
            motionZ *= scale;
            horizSpeed = SPEED_CAP;
        }

        // Ramp speed multiplier
        if (horizSpeed > prevHorizSpeed && speedMultiplier < SPEED_CAP) {
            speedMultiplier += (SPEED_CAP - speedMultiplier) / SPEED_RAMP_RATE;
            if (speedMultiplier > SPEED_CAP) speedMultiplier = SPEED_CAP;
        } else {
            speedMultiplier -= (speedMultiplier - SPEED_MIN) / SPEED_RAMP_RATE;
            if (speedMultiplier < SPEED_MIN) speedMultiplier = SPEED_MIN;
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // onUpdate — Main tick.
    // Returns what events occurred for the caller to process.
    // ═══════════════════════════════════════════════════════════════════════

    struct TickResult {
        bool shouldBreakWall;
        bool shouldBreakFall;
        bool shouldDie;
        double prevHorizSpeed;
    };

    TickResult onUpdate(double waterDepth, bool hasRider) {
        TickResult result{};

        // Damage recovery
        if (timeSinceHit > 0) --timeSinceHit;
        if (damageTaken > 0) damageTaken -= 1.0f;

        prevPosX = posX; prevPosY = posY; prevPosZ = posZ;

        // Compute previous horiz speed for splash check
        result.prevHorizSpeed = std::sqrt(motionX * motionX + motionZ * motionZ);

        // Buoyancy
        applyBuoyancy(waterDepth);

        // Ground friction
        if (onGround) {
            motionX *= GROUND_FRICTION;
            motionY *= GROUND_FRICTION;
            motionZ *= GROUND_FRICTION;
        }

        // moveEntity done by caller
        // posX += motionX; posY += motionY; posZ += motionZ;

        // Wall collision break
        if (isCollidedHorizontally && result.prevHorizSpeed > WALL_BREAK_SPEED) {
            result.shouldBreakWall = true;
            result.shouldDie = true;
            isDead = true;
        } else {
            // Air friction
            motionX *= AIR_FRICTION_XZ;
            motionY *= AIR_FRICTION_Y;
            motionZ *= AIR_FRICTION_XZ;
        }

        // Update yaw from motion
        rotationPitch = 0;
        double dx = prevPosX - posX;
        double dz = prevPosZ - posZ;
        if (dx * dx + dz * dz > 0.001) {
            double targetYaw = std::atan2(dz, dx) * 180.0 / M_PI;
            double delta = targetYaw - rotationYaw;
            // Wrap to ±180
            while (delta > 180) delta -= 360;
            while (delta < -180) delta += 360;
            // Clamp ±20 per tick
            if (delta > YAW_LIMIT) delta = YAW_LIMIT;
            if (delta < -YAW_LIMIT) delta = -YAW_LIMIT;
            rotationYaw += static_cast<float>(delta);
        }

        // Dismount dead rider
        if (hasRider && riderEntityId >= 0) {
            // Handled externally
        }

        return result;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Damage handling
    // Java: EntityBoat.attackEntityFrom
    //   - Flip forwardDirection
    //   - timeSinceHit = 10
    //   - damageTaken += damage * 10
    //   - Break if damageTaken > 40 or creative mode
    // ═══════════════════════════════════════════════════════════════════════

    struct DamageResult {
        bool broken;
        bool dropItems; // false if creative
    };

    DamageResult attackEntityFrom(float damage, bool isCreative, bool isInvulnerable) {
        if (isInvulnerable || isDead) return {false, false};

        forwardDirection = -forwardDirection;
        timeSinceHit = HIT_TIME;
        damageTaken += damage * DAMAGE_MULTIPLIER;

        if (isCreative || damageTaken > DAMAGE_THRESHOLD) {
            isDead = true;
            return {true, !isCreative};
        }
        return {false, false};
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Fall damage
    // Java: EntityBoat.updateFallState
    //   - Fall > 3.0: break (3 planks + 2 sticks)
    //   - Track fall distance when not in water and descending
    // ═══════════════════════════════════════════════════════════════════════

    struct FallResult {
        bool broken;
    };

    FallResult updateFallState(double deltaY, bool onGround_, bool aboveWater) {
        if (onGround_) {
            if (fallDistance > FALL_BREAK_THRESHOLD) {
                isDead = true;
                fallDistance = 0;
                return {true};
            }
            fallDistance = 0;
        } else if (!aboveWater && deltaY < 0) {
            fallDistance += static_cast<float>(-deltaY);
        }
        return {false};
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Rider position update
    // Java: EntityBoat.updateRiderPosition
    //   - X + cos(yaw) * 0.4, Z + sin(yaw) * 0.4
    //   - Y + mountedYOffset + rider.yOffset
    // ═══════════════════════════════════════════════════════════════════════

    void getRiderPosition(double& outX, double& outY, double& outZ, float riderYOffset) const {
        double yawRad = rotationYaw * M_PI / 180.0;
        outX = posX + std::cos(yawRad) * RIDER_OFFSET;
        outY = posY + MOUNTED_Y_OFFSET + riderYOffset;
        outZ = posZ + std::sin(yawRad) * RIDER_OFFSET;
    }

    bool canBeCollidedWith() const { return !isDead; }
    bool canBePushed() const { return true; }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityFallingBlock — Falling sand/gravel entity.
//
// Java: net.minecraft.entity.item.EntityFallingBlock
// Key mechanics:
//   - Created when sand/gravel below has no support
//   - Falls with gravity 0.04, friction 0.98
//   - Converts back to block when landing on solid surface
//   - Can deal damage to entities below (damage = min(40, ceil(fallDistance-1)*2))
//   - Drops as item if can't place at landing position
//   - Carries tile entity data (for anvils, etc.)
//   - Despawn: 600 ticks if didn't land
//   - NBT: TileID (byte), Data (byte), Time (byte), DropItem (byte),
//          HurtEntities (byte), FallHurtMax (int), FallHurtAmount (float)
// ═══════════════════════════════════════════════════════════════════════════

class EntityFallingBlock {
public:
    double posX = 0, posY = 0, posZ = 0;
    double prevPosX = 0, prevPosY = 0, prevPosZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    bool onGround = false;
    bool isDead = false;
    int32_t entityId = 0;

    // Block data
    int32_t blockId = 12;  // Default: sand
    int32_t blockMeta = 0;
    int32_t fallTime = 0;
    bool dropItem = true;
    bool hurtEntities = false;
    int32_t fallHurtMax = 40;
    float fallHurtAmount = 2.0f;
    float fallDistance = 0;

    static constexpr float GRAVITY = 0.04f;
    static constexpr float FRICTION = 0.98f;
    static constexpr int32_t MAX_FALL_TIME = 600;

    struct TickResult {
        bool shouldPlace; // Convert back to block
        bool shouldDrop;  // Drop as item
        bool shouldDie;
        int32_t landX, landY, landZ;
    };

    TickResult onUpdate(bool canPlaceAt, int32_t floorX, int32_t floorY, int32_t floorZ) {
        prevPosX = posX; prevPosY = posY; prevPosZ = posZ;

        ++fallTime;
        motionY -= GRAVITY;

        // moveEntity done by caller

        motionX *= FRICTION;
        motionY *= FRICTION;
        motionZ *= FRICTION;

        if (onGround) {
            motionX *= 0.7;
            motionZ *= 0.7;
            motionY *= -0.5;

            if (canPlaceAt) {
                isDead = true;
                return {true, false, true, floorX, floorY, floorZ};
            }
        }

        if (fallTime >= MAX_FALL_TIME) {
            isDead = true;
            if (dropItem) return {false, true, true, 0, 0, 0};
            return {false, false, true, 0, 0, 0};
        }

        return {false, false, false, 0, 0, 0};
    }

    // Fall damage to entities below
    // damage = min(fallHurtMax, ceil(fallDistance - 1) * fallHurtAmount)
    int32_t calculateFallDamage() const {
        if (!hurtEntities || fallDistance <= 1.0f) return 0;
        int32_t dmg = static_cast<int32_t>(std::ceil(fallDistance - 1.0f) * fallHurtAmount);
        return (dmg > fallHurtMax) ? fallHurtMax : dmg;
    }
};

} // namespace mccpp
