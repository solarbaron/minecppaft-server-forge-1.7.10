/**
 * EntityProjectiles.h — Projectile entities and XP orbs.
 *
 * Java references:
 *   - net.minecraft.entity.projectile.EntityThrowable (233 lines)
 *   - net.minecraft.entity.projectile.EntityFireball (232 lines)
 *   - net.minecraft.entity.projectile.EntitySnowball (46 lines)
 *   - net.minecraft.entity.projectile.EntityLargeFireball (51 lines)
 *   - net.minecraft.entity.projectile.EntitySmallFireball (85 lines)
 *   - net.minecraft.entity.projectile.EntityWitherSkull (101 lines)
 *   - net.minecraft.entity.item.EntityXPOrb (199 lines)
 *
 * Thread safety: Entity tick on single thread per entity.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <array>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityThrowable — Base class for thrown projectiles (snowball, egg, potion).
// Java: net.minecraft.entity.projectile.EntityThrowable (233 lines)
//
//   Size: 0.25×0.25
//   Initial velocity: 0.4 base, normalized to speed 1.5
//   Gravity: 0.03/tick, air drag: 0.99, water drag: 0.8
//   Gaussian scatter: 0.0075 per axis
//   Launch offset: -cos(yaw)*0.16 X, -0.1 Y, -sin(yaw)*0.16 Z
//   Thrower immune: first 5 ticks
//   Entity collision: expand 0.3 AABB
//   In-ground: die after 1200 ticks, else jitter motionXYZ *= rand*0.2
//   Water: 4 bubble particles, drag 0.8
//   Yaw/pitch smoothed 0.2
//   Portal entry on block hit
//   NBT: xTile, yTile, zTile, inTile, shake, inGround, ownerName
// ═══════════════════════════════════════════════════════════════════════════

class EntityThrowable {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    float prevRotationYaw = 0, prevRotationPitch = 0;
    bool isDead = false;
    int32_t entityId = 0;

    // Ground state
    int32_t xTile = -1, yTile = -1, zTile = -1;
    int32_t inBlockId = 0;
    bool inGround = false;
    int32_t throwableShake = 0;
    int32_t ticksInGround = 0;
    int32_t ticksInAir = 0;
    std::string throwerName;

    static constexpr float WIDTH = 0.25f;
    static constexpr float HEIGHT = 0.25f;
    static constexpr float INITIAL_SPEED = 0.4f;
    static constexpr float VELOCITY = 1.5f;
    static constexpr float SCATTER = 0.0075f;  // Gaussian deviation per inaccuracy
    static constexpr float INACCURACY = 1.0f;
    static constexpr float GRAVITY = 0.03f;
    static constexpr float AIR_DRAG = 0.99f;
    static constexpr float WATER_DRAG = 0.8f;
    static constexpr float LAUNCH_OFFSET = 0.16f;
    static constexpr float LAUNCH_Y_OFFSET = -0.1f;
    static constexpr int32_t THROWER_IMMUNE_TICKS = 5;
    static constexpr float ENTITY_EXPAND = 0.3f;
    static constexpr int32_t GROUND_TIMEOUT = 1200;
    static constexpr float ROTATION_SMOOTH = 0.2f;
    static constexpr int32_t WATER_BUBBLES = 4;
    static constexpr float BUBBLE_OFFSET = 0.25f;
    static constexpr float PITCH_OFFSET = 0.0f; // func_70183_g default

    // ─── Launch calculation ───
    // Java: setThrowableHeading(dx, dy, dz, speed, inaccuracy)
    struct LaunchResult {
        double motionX, motionY, motionZ;
        float yaw, pitch;
    };

    static LaunchResult calculateLaunch(double dx, double dy, double dz,
                                         float speed, float inaccuracy,
                                         double gaussX, double gaussY, double gaussZ) {
        LaunchResult r{};
        double len = std::sqrt(dx * dx + dy * dy + dz * dz);
        dx /= len; dy /= len; dz /= len;
        dx += gaussX * SCATTER * inaccuracy;
        dy += gaussY * SCATTER * inaccuracy;
        dz += gaussZ * SCATTER * inaccuracy;
        r.motionX = dx * speed;
        r.motionY = dy * speed;
        r.motionZ = dz * speed;
        float horzDist = static_cast<float>(std::sqrt(r.motionX * r.motionX + r.motionZ * r.motionZ));
        r.yaw = static_cast<float>(std::atan2(r.motionX, r.motionZ) * 180.0 / M_PI);
        r.pitch = static_cast<float>(std::atan2(r.motionY, horzDist) * 180.0 / M_PI);
        return r;
    }

    // ─── Position update ───
    void tickMotion(bool isInWater) {
        float drag = isInWater ? WATER_DRAG : AIR_DRAG;
        posX += motionX;
        posY += motionY;
        posZ += motionZ;
        motionX *= drag;
        motionY *= drag;
        motionZ *= drag;
        motionY -= GRAVITY;
    }

    // Yaw/pitch update
    void updateRotation() {
        float horzDist = static_cast<float>(std::sqrt(motionX * motionX + motionZ * motionZ));
        float newYaw = static_cast<float>(std::atan2(motionX, motionZ) * 180.0 / M_PI);
        float newPitch = static_cast<float>(std::atan2(motionY, horzDist) * 180.0 / M_PI);
        rotationPitch = prevRotationPitch + (newPitch - prevRotationPitch) * ROTATION_SMOOTH;
        rotationYaw = prevRotationYaw + (newYaw - prevRotationYaw) * ROTATION_SMOOTH;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityFireball — Base for acceleration-driven projectiles (ghast, blaze).
// Java: net.minecraft.entity.projectile.EntityFireball (232 lines)
//
//   Size: 1×1, acceleration = normalize(direction) * 0.1
//   Gaussian scatter on shooter constructor: 0.4 per axis
//   Drag: 0.95 air, 0.8 water
//   In-ground timeout: 600 ticks
//   Fire: set on fire 1 tick every tick
//   Reflectable: on attack, motion = attacker.lookVec, accel = motion*0.1
//     Shooter changes to attacker if EntityLivingBase
//   Shooter immune: first 25 ticks
//   Can be collided with (collision border 1.0)
//   Always bright (brightness 1.0)
//   Smoke particle each tick
//   yaw = atan2(motionZ, motionX)*180/PI + 90
//   pitch = atan2(horzSpeed, motionY)*180/PI - 90
//   NBT: xTile, yTile, zTile, inTile, inGround, direction[3 doubles]
// ═══════════════════════════════════════════════════════════════════════════

class EntityFireball {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    double accelerationX = 0, accelerationY = 0, accelerationZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    float prevRotationYaw = 0, prevRotationPitch = 0;
    bool isDead = false;
    int32_t entityId = 0;
    int32_t shooterEntityId = -1;

    // Ground state
    int32_t xTile = -1, yTile = -1, zTile = -1;
    int32_t inBlockId = 0;
    bool inGround = false;
    int32_t ticksAlive = 0;
    int32_t ticksInAir = 0;

    static constexpr float WIDTH = 1.0f;
    static constexpr float HEIGHT = 1.0f;
    static constexpr double ACCEL_FACTOR = 0.1;
    static constexpr double GAUSSIAN_SCATTER = 0.4;
    static constexpr float MOTION_FACTOR = 0.95f;
    static constexpr float WATER_DRAG = 0.8f;
    static constexpr int32_t GROUND_TIMEOUT = 600;
    static constexpr int32_t SHOOTER_IMMUNE_TICKS = 25;
    static constexpr float COLLISION_BORDER = 1.0f;
    static constexpr float ENTITY_EXPAND = 0.3f;
    static constexpr float BRIGHTNESS = 1.0f;
    static constexpr float ROTATION_SMOOTH = 0.2f;

    // ─── Acceleration from direction ───
    // Java: normalize(dx,dy,dz) * 0.1
    static void normalizeAcceleration(double dx, double dy, double dz,
                                        double& accelX, double& accelY, double& accelZ) {
        double len = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (len > 0) {
            accelX = dx / len * ACCEL_FACTOR;
            accelY = dy / len * ACCEL_FACTOR;
            accelZ = dz / len * ACCEL_FACTOR;
        }
    }

    // ─── Physics tick ───
    void tickMotion(bool isInWater) {
        float drag = isInWater ? WATER_DRAG : MOTION_FACTOR;
        motionX += accelerationX;
        motionY += accelerationY;
        motionZ += accelerationZ;
        motionX *= drag;
        motionY *= drag;
        motionZ *= drag;
        posX += motionX;
        posY += motionY;
        posZ += motionZ;
    }

    // Yaw: atan2(motionZ, motionX)*180/PI + 90
    // Pitch: atan2(horzSpeed, motionY)*180/PI - 90
    void updateRotation() {
        float horzSpeed = static_cast<float>(std::sqrt(motionX * motionX + motionZ * motionZ));
        float newYaw = static_cast<float>(std::atan2(motionZ, motionX) * 180.0 / M_PI) + 90.0f;
        float newPitch = static_cast<float>(std::atan2(horzSpeed, motionY) * 180.0 / M_PI) - 90.0f;
        rotationPitch = prevRotationPitch + (newPitch - prevRotationPitch) * ROTATION_SMOOTH;
        rotationYaw = prevRotationYaw + (newYaw - prevRotationYaw) * ROTATION_SMOOTH;
    }

    // ─── Reflect ───
    // Java: on attack, redirect to attacker's lookVec
    void reflect(double lookX, double lookY, double lookZ) {
        motionX = lookX;
        motionY = lookY;
        motionZ = lookZ;
        accelerationX = motionX * ACCEL_FACTOR;
        accelerationY = motionY * ACCEL_FACTOR;
        accelerationZ = motionZ * ACCEL_FACTOR;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntitySnowball — Thrown by snow golems, 3 damage to blazes.
// Java: net.minecraft.entity.projectile.EntitySnowball (46 lines)
//   Damage: 0 (3 vs Blaze), 8 snowballpoof particles, die on impact
// ═══════════════════════════════════════════════════════════════════════════

class EntitySnowball : public EntityThrowable {
public:
    static constexpr int32_t BLAZE_DAMAGE = 3;
    static constexpr int32_t NORMAL_DAMAGE = 0;
    static constexpr int32_t POOF_PARTICLES = 8;
    static constexpr const char* PARTICLE = "snowballpoof";

    static int32_t getDamage(bool isBlaze) { return isBlaze ? BLAZE_DAMAGE : NORMAL_DAMAGE; }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityLargeFireball — Ghast fireball, explodes on impact.
// Java: net.minecraft.entity.projectile.EntityLargeFireball (51 lines)
//
//   Damage: 6 (fireball damage source)
//   Explosion power: 1 (default, saved in NBT "ExplosionPower")
//   Causes fire: true
//   Obeys mobGriefing
// ═══════════════════════════════════════════════════════════════════════════

class EntityLargeFireball : public EntityFireball {
public:
    int32_t explosionPower = 1;

    static constexpr float IMPACT_DAMAGE = 6.0f;
    static constexpr bool CAUSES_FIRE = true;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntitySmallFireball — Blaze fireball, sets fire on block hit.
// Java: net.minecraft.entity.projectile.EntitySmallFireball (85 lines)
//
//   Size: 0.3125×0.3125
//   Damage: 5 (fire immune targets take 0)
//   Fire: 5 seconds on entity hit
//   Block hit: place fire block on adjacent face (sideHit offset)
//   Can't be collided with, can't be damaged (not reflectable)
// ═══════════════════════════════════════════════════════════════════════════

class EntitySmallFireball : public EntityFireball {
public:
    static constexpr float SM_WIDTH = 0.3125f;
    static constexpr float SM_HEIGHT = 0.3125f;
    static constexpr float IMPACT_DAMAGE = 5.0f;
    static constexpr int32_t FIRE_DURATION = 5; // seconds
    static constexpr bool CAN_COLLIDE = false;
    static constexpr bool CAN_BE_DAMAGED = false;

    // Side hit → block offset for fire placement
    // Java: switch(sideHit) 0:y--, 1:y++, 2:z--, 3:z++, 4:x--, 5:x++
    struct BlockOffset { int32_t dx, dy, dz; };
    static constexpr std::array<BlockOffset, 6> SIDE_OFFSETS = {{
        {0, -1, 0},  // bottom
        {0,  1, 0},  // top
        {0,  0, -1}, // north
        {0,  0,  1}, // south
        {-1, 0, 0},  // west
        { 1, 0, 0},  // east
    }};
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityWitherSkull — Wither projectile, applies wither effect.
// Java: net.minecraft.entity.projectile.EntityWitherSkull (101 lines)
//
//   Size: 0.3125×0.3125
//   DW 10: invulnerable byte (blue skull)
//   Blue skull: drag 0.73 (vs normal 0.95), reduces block resistance to 0.8
//   Damage: 8 with shooter (heal 5 on kill), 5 magic without shooter
//   Wither effect: Normal=10s, Hard=40s, amplifier 1
//   Explosion: power 1.0, no fire, obeys mobGriefing
//   Can't be collided with, can't be damaged (not reflectable)
//   Not burning
//   Immune blocks for blue: bedrock, end_portal, end_portal_frame, command_block
// ═══════════════════════════════════════════════════════════════════════════

class EntityWitherSkull : public EntityFireball {
public:
    bool invulnerable = false; // DW 10

    static constexpr float WS_WIDTH = 0.3125f;
    static constexpr float WS_HEIGHT = 0.3125f;
    static constexpr float BLUE_DRAG = 0.73f;
    static constexpr float NORMAL_DRAG = 0.95f;
    static constexpr float SHOOTER_DAMAGE = 8.0f;
    static constexpr float NO_SHOOTER_DAMAGE = 5.0f;
    static constexpr float KILL_HEAL = 5.0f;
    static constexpr float EXPLOSION_POWER = 1.0f;
    static constexpr float BLUE_RESISTANCE_CAP = 0.8f;
    static constexpr bool CAUSES_FIRE = false;

    // Wither effect
    static constexpr int32_t WITHER_POTION_ID = 20;
    static constexpr int32_t WITHER_AMPLIFIER = 1;
    static constexpr int32_t WITHER_EASY = 0;
    static constexpr int32_t WITHER_NORMAL = 10 * 20;  // 200 ticks
    static constexpr int32_t WITHER_HARD = 40 * 20;    // 800 ticks

    float getMotionFactor() const { return invulnerable ? BLUE_DRAG : NORMAL_DRAG; }

    static int32_t getWitherDuration(int32_t difficulty) {
        switch (difficulty) {
            case 2: return WITHER_NORMAL;
            case 3: return WITHER_HARD;
            default: return WITHER_EASY;
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityXPOrb — Experience orb entity.
// Java: net.minecraft.entity.item.EntityXPOrb (199 lines)
//
//   Size: 0.5×0.5 (with value), 0.25×0.25 (empty)
//   yOffset = height/2
//   Initial motion: random yaw, motionXZ = (rand*0.2-0.1)*2, motionY = rand*0.2*2
//   Gravity: 0.03
//   In lava: bounce motionY=0.2, randomXZ ±0.2, fizz sound
//   Player attraction: within 8 blocks, accel = (1-dist/8)² * 0.1
//   Player search: every 20+entityId%100 ticks
//   Ground friction: block.slipperiness * 0.98, Y always 0.98
//   Ground bounce: motionY *= -0.9
//   Lifetime: 6000 ticks
//   HP: 5, takes damage
//   Pickup cooldown: player.xpCooldown=2, sound "random.orb" vol 0.1
//   NBT: Health, Age, Value (all shorts)
//
//   XP split table (for dragon death XP distribution):
//     ≥2477→2477, ≥1237→1237, ≥617→617, ≥307→307,
//     ≥149→149, ≥73→73, ≥37→37, ≥17→17, ≥7→7, ≥3→3, else→1
// ═══════════════════════════════════════════════════════════════════════════

class EntityXPOrb {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    bool isDead = false;
    bool onGround = false;
    int32_t entityId = 0;

    int32_t xpColor = 0;      // animation counter
    int32_t xpOrbAge = 0;     // age in ticks
    int32_t pickupDelay = 0;  // field_70532_c
    int32_t xpOrbHealth = 5;
    int32_t xpValue = 0;
    int32_t xpTargetColor = 0;

    static constexpr float WIDTH = 0.5f;
    static constexpr float HEIGHT = 0.5f;
    static constexpr float GRAVITY = 0.03f;
    static constexpr double SEARCH_RANGE = 8.0;
    static constexpr double ATTRACTION_ACCEL = 0.1;
    static constexpr float GROUND_Y_DRAG = 0.98f;
    static constexpr float GROUND_BOUNCE = -0.9f;
    static constexpr int32_t LIFETIME = 6000;
    static constexpr int32_t LAVA_MOTION_Y = 0.2f;
    static constexpr float LAVA_SPREAD = 0.2f;
    static constexpr int32_t PLAYER_XP_COOLDOWN = 2;
    static constexpr float PICKUP_VOLUME = 0.1f;

    // ─── XP split table ───
    // Java: EntityXPOrb.getXPSplit(int)
    static int32_t getXPSplit(int32_t total) {
        if (total >= 2477) return 2477;
        if (total >= 1237) return 1237;
        if (total >= 617) return 617;
        if (total >= 307) return 307;
        if (total >= 149) return 149;
        if (total >= 73) return 73;
        if (total >= 37) return 37;
        if (total >= 17) return 17;
        if (total >= 7) return 7;
        if (total >= 3) return 3;
        return 1;
    }

    // ─── Player attraction ───
    struct AttractionResult {
        double accelX, accelY, accelZ;
        bool attracted;
    };

    AttractionResult calculateAttraction(double playerX, double playerY, double playerZ) const {
        AttractionResult r{};
        double dx = (playerX - posX) / SEARCH_RANGE;
        double dy = (playerY - posY) / SEARCH_RANGE;
        double dz = (playerZ - posZ) / SEARCH_RANGE;
        double dist = std::sqrt(dx * dx + dy * dy + dz * dz);
        double pull = 1.0 - dist;
        if (pull > 0) {
            pull *= pull;
            r.accelX = dx / dist * pull * ATTRACTION_ACCEL;
            r.accelY = dy / dist * pull * ATTRACTION_ACCEL;
            r.accelZ = dz / dist * pull * ATTRACTION_ACCEL;
            r.attracted = true;
        }
        return r;
    }

    // Should search for player this tick?
    bool shouldSearchPlayer() const {
        return xpTargetColor < xpColor - 20 + entityId % 100;
    }

    void tick() {
        ++xpColor;
        ++xpOrbAge;
        if (pickupDelay > 0) --pickupDelay;
        motionY -= GRAVITY;
    }

    bool isExpired() const { return xpOrbAge >= LIFETIME; }

    static constexpr const char* PICKUP_SOUND = "random.orb";
    static constexpr const char* LAVA_SOUND = "random.fizz";
    static constexpr float LAVA_SOUND_VOLUME = 0.4f;
};

} // namespace mccpp
