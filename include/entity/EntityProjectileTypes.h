/**
 * EntityProjectileTypes.h — Projectile entity physics and subtypes.
 *
 * Java references:
 *   - net.minecraft.entity.projectile.EntityThrowable (233 lines)
 *   - net.minecraft.entity.projectile.EntityArrow (392 lines)
 *   - net.minecraft.entity.projectile.EntitySnowball (37 lines)
 *   - net.minecraft.entity.projectile.EntityEgg (62 lines)
 *   - net.minecraft.entity.projectile.EntityPotion (113 lines)
 *   - net.minecraft.entity.projectile.EntityFireball (230 lines)
 *   - net.minecraft.entity.projectile.EntityLargeFireball (60 lines)
 *   - net.minecraft.entity.projectile.EntitySmallFireball (69 lines)
 *   - net.minecraft.entity.projectile.EntityWitherSkull (73 lines)
 *
 * Projectile physics (shared):
 *   - Initial position: thrower eye height - 0.1, offset -cos(yaw)*0.16, -sin(yaw)*0.16
 *   - setThrowableHeading: normalize, gaussian noise 0.0075*inaccuracy, scale by velocity
 *   - Rotation: atan2(motionX, motionZ)*180/PI for yaw, atan2(motionY, horizSpeed)*180/PI for pitch
 *   - Angle smoothing: 0.2 interpolation factor
 *   - Ground despawn: 1200 ticks (60 seconds)
 *
 * Thread safety: Per-entity, ticked on entity's thread.
 * JNI readiness: Simple fields and POD state.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// ProjectilePhysics — Shared projectile physics constants and computation.
// ═══════════════════════════════════════════════════════════════════════════

namespace ProjectilePhysics {
    static constexpr double PI = 3.1415927410125732;  // Java float PI cast to double
    static constexpr float DEG_TO_RAD = static_cast<float>(PI) / 180.0f;
    static constexpr float THROWER_OFFSET_Y = -0.1f;
    static constexpr float THROWER_OFFSET_XZ = 0.16f;
    static constexpr double NOISE_SCALE = 0.0075;
    static constexpr float ANGLE_SMOOTH = 0.2f;
    static constexpr int32_t GROUND_DESPAWN_TICKS = 1200;
}

// ═══════════════════════════════════════════════════════════════════════════
// EntityThrowableData — Base throwable projectile state and constants.
// Java: net.minecraft.entity.projectile.EntityThrowable (233 lines)
//
// Size: 0.25 x 0.25
// Initial velocity: 0.4f base, then setThrowableHeading(1.5f, 1.0f)
// Gravity: 0.03f, Air drag: 0.99f, Water drag: 0.8f
// Collision: ray trace + entity AABB intercept (0.3f expand)
// Thrower immunity: ticksInAir < 5
// ═══════════════════════════════════════════════════════════════════════════

struct EntityThrowableData {
    static constexpr float WIDTH = 0.25f;
    static constexpr float HEIGHT = 0.25f;
    static constexpr float INITIAL_VELOCITY = 0.4f;
    static constexpr float THROW_SPEED = 1.5f;
    static constexpr float INACCURACY = 1.0f;
    static constexpr float GRAVITY = 0.03f;
    static constexpr float AIR_DRAG = 0.99f;
    static constexpr float WATER_DRAG = 0.8f;
    static constexpr float PITCH_OFFSET = 0.0f;
    static constexpr float ENTITY_EXPAND = 0.3f;
    static constexpr int32_t THROWER_IMMUNITY_TICKS = 5;

    int32_t xTile = -1, yTile = -1, zTile = -1;
    int32_t inBlockId = 0;
    bool inGround = false;
    int32_t throwableShake = 0;
    int32_t ticksInGround = 0;
    int32_t ticksInAir = 0;
    std::string throwerName;
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    float prevRotationYaw = 0, prevRotationPitch = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityArrowData — Arrow with damage, knockback, critical hits.
// Java: net.minecraft.entity.projectile.EntityArrow (392 lines)
//
// Size: 0.5 x 0.5, Gravity: 0.05f
// Base damage: 2.0
// Critical: +rand(damage/2 + 2)
// Knockback: velocity * 0.6 * strength / horizSpeed, +0.1 upward
// Fire: 5 seconds on hit
// Enderman immune, Pickup: 0=no, 1=survival, 2=creative
// DataWatcher[16]: bit 0 = critical
//
// Damage = ceil(sqrt(mX²+mY²+mZ²) * this.damage)
// Bounce on fail: velocity *= -0.1, yaw += 180
// ═══════════════════════════════════════════════════════════════════════════

struct EntityArrowData {
    static constexpr float WIDTH = 0.5f;
    static constexpr float HEIGHT = 0.5f;
    static constexpr float GRAVITY = 0.05f;
    static constexpr float AIR_DRAG = 0.99f;
    static constexpr float WATER_DRAG = 0.8f;
    static constexpr double BASE_DAMAGE = 2.0;
    static constexpr float POWER_MULTIPLIER = 1.5f;
    static constexpr float KNOCKBACK_FACTOR = 0.6f;
    static constexpr double KNOCKBACK_UPWARD = 0.1;
    static constexpr int32_t FIRE_DURATION = 5;
    static constexpr float ENTITY_EXPAND = 0.3f;
    static constexpr int32_t SHOOTER_IMMUNITY = 5;
    static constexpr float BLOCK_PUSHBACK = 0.05f;
    static constexpr float BOUNCE_VELOCITY = -0.1f;
    static constexpr int32_t STUCK_SHAKE = 7;
    static constexpr int32_t PICKUP_NONE = 0;
    static constexpr int32_t PICKUP_SURVIVAL = 1;
    static constexpr int32_t PICKUP_CREATIVE = 2;

    int32_t xTile = -1, yTile = -1, zTile = -1;
    int32_t inBlockId = 0, inBlockMeta = 0;
    bool inGround = false;
    int32_t canBePickedUp = 0;
    int32_t arrowShake = 0;
    int32_t ticksInGround = 0, ticksInAir = 0;
    double damage = BASE_DAMAGE;
    int32_t knockbackStrength = 0;
    bool isCritical = false;

    int32_t calculateDamage(double mX, double mY, double mZ, bool crit) const {
        float speed = static_cast<float>(std::sqrt(mX*mX + mY*mY + mZ*mZ));
        int32_t dmg = static_cast<int32_t>(std::ceil(speed * damage));
        // If crit: caller adds rand(dmg/2 + 2)
        (void)crit;
        return dmg;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntitySnowballData — Snowball: 0 damage, 3 to Blazes.
// ═══════════════════════════════════════════════════════════════════════════

struct EntitySnowballData {
    static constexpr float BLAZE_DAMAGE = 3.0f;
    static constexpr float OTHER_DAMAGE = 0.0f;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityEggData — Egg: 1/8 chicken spawn, 1/32 quad (4) spawn.
// ═══════════════════════════════════════════════════════════════════════════

struct EntityEggData {
    static constexpr float IMPACT_DAMAGE = 0.0f;
    static constexpr int32_t SPAWN_CHANCE = 8;
    static constexpr int32_t QUAD_SPAWN_CHANCE = 32;
    static constexpr int32_t NORMAL_COUNT = 1;
    static constexpr int32_t QUAD_COUNT = 4;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityPotionData — Splash potion: 0.05 gravity, 4.0 splash radius.
// Java: EntityPotion (113 lines)
//   Distance factor: 1.0 - sqrt(distSq)/4.0
//   Duration scaling: duration * 0.75 + 0.5
// ═══════════════════════════════════════════════════════════════════════════

struct EntityPotionData {
    static constexpr float GRAVITY = 0.05f;
    static constexpr float THROW_SPEED = 0.5f;
    static constexpr float PITCH_OFFSET = -20.0f;
    static constexpr double SPLASH_RADIUS = 4.0;
    static constexpr double DURATION_FACTOR = 0.75;
    int32_t potionDamage = 0;  // ItemStack metadata for potion type
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityFireballData — Base fireball (no gravity, 0.95 drag).
// Java: EntityFireball (230 lines)
//   Acceleration: direction * 0.1 / distance each tick
//   Size: 1.0 x 1.0
// ═══════════════════════════════════════════════════════════════════════════

struct EntityFireballData {
    static constexpr float WIDTH = 1.0f;
    static constexpr float HEIGHT = 1.0f;
    static constexpr float AIR_DRAG = 0.95f;
    static constexpr double ACCEL_SCALE = 0.1;
    static constexpr float ENTITY_EXPAND = 0.3f;
    double accelX = 0, accelY = 0, accelZ = 0;
    int32_t ticksInAir = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityLargeFireballData — Ghast fireball: 6 damage + explosion.
// ═══════════════════════════════════════════════════════════════════════════

struct EntityLargeFireballData {
    static constexpr float IMPACT_DAMAGE = 6.0f;
    int32_t explosionPower = 1;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntitySmallFireballData — Blaze fireball: 5 damage + 5s fire.
// ═══════════════════════════════════════════════════════════════════════════

struct EntitySmallFireballData {
    static constexpr float IMPACT_DAMAGE = 5.0f;
    static constexpr int32_t FIRE_DURATION = 5;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityWitherSkullData — Wither skull: 8 damage + Wither II 10s.
// ═══════════════════════════════════════════════════════════════════════════

struct EntityWitherSkullData {
    static constexpr float IMPACT_DAMAGE = 8.0f;
    static constexpr float EXPLOSION_POWER = 1.0f;
    static constexpr int32_t WITHER_TICKS = 200;
    static constexpr int32_t WITHER_AMPLIFIER = 1;
    bool isCharged = false;
};

} // namespace mccpp
