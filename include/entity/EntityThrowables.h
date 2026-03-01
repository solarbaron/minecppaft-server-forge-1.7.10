/**
 * EntityThrowables.h — Remaining throwable projectiles and special item entities.
 *
 * Java references:
 *   - net.minecraft.entity.projectile.EntityEgg (54 lines)
 *   - net.minecraft.entity.projectile.EntityPotion (122 lines)
 *   - net.minecraft.entity.item.EntityExpBottle (55 lines)
 *   - net.minecraft.entity.item.EntityEnderEye (137 lines)
 *   - net.minecraft.entity.item.EntityFireworkRocket (122 lines)
 *
 * Thread safety: Entity tick on single thread per entity.
 */
#pragma once

#include <cmath>
#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityEgg — Thrown egg, chance to spawn baby chickens.
// Java: net.minecraft.entity.projectile.EntityEgg (54 lines)
//
//   Extends EntityThrowable (0.25×0.25, gravity 0.03, speed 1.5)
//   Impact damage: 0 (knockback only)
//   Chicken spawn: 1/8 chance, 1/32 quad spawn
//   Baby chicken age: -24000
//   8 snowballpoof particles on impact
// ═══════════════════════════════════════════════════════════════════════════

class EntityEgg {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    bool isDead = false;
    int32_t entityId = 0;

    static constexpr float IMPACT_DAMAGE = 0.0f;
    static constexpr int32_t CHICKEN_CHANCE = 8;       // 1/8
    static constexpr int32_t QUAD_CHICKEN_CHANCE = 32;  // 1/32
    static constexpr int32_t BABY_AGE = -24000;
    static constexpr int32_t POOF_PARTICLES = 8;
    static constexpr const char* PARTICLE = "snowballpoof";

    // Returns number of chickens to spawn (0, 1, or 4)
    static int32_t getChickenCount(int32_t rand8, int32_t rand32) {
        if (rand8 != 0) return 0; // 1/8 chance
        return rand32 == 0 ? 4 : 1; // 1/32 quad, else single
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityPotion — Splash potion bottle.
// Java: net.minecraft.entity.projectile.EntityPotion (122 lines)
//
//   Extends EntityThrowable
//   Gravity: 0.05 (vs 0.03 default), speed: 0.5, pitch offset: -20
//   Splash area: 4×2×4 AABB expansion
//   Max effect range: distSq < 16 (4 blocks)
//   Effect multiplier: 1.0 - sqrt(dist)/4.0
//     Direct hit entity: multiplier = 1.0 (override)
//   Instant potions: affectEntity(thrower, target, amplifier, multiplier)
//   Duration potions: duration = floor(multiplier * baseDuration + 0.5)
//     Skip if duration ≤ 20 ticks
//   Sound effect 2002 with potion damage value
//   NBT: "Potion" compound or "potionValue" int
// ═══════════════════════════════════════════════════════════════════════════

class EntityPotion {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    int32_t potionDamage = 0;

    static constexpr float GRAVITY = 0.05f;
    static constexpr float SPEED = 0.5f;
    static constexpr float PITCH_OFFSET = -20.0f;
    static constexpr double SPLASH_EXPAND_XZ = 4.0;
    static constexpr double SPLASH_EXPAND_Y = 2.0;
    static constexpr double MAX_EFFECT_DIST_SQ = 16.0;
    static constexpr int32_t MIN_DURATION_TICKS = 20;
    static constexpr int32_t SOUND_ID = 2002;

    // ─── Effect multiplier ───
    // Java: 1.0 - sqrt(distSq) / 4.0, clamped; direct hit = 1.0
    static double calculateMultiplier(double distSq, bool isDirectHit) {
        if (isDirectHit) return 1.0;
        return 1.0 - std::sqrt(distSq) / 4.0;
    }

    // Duration scaling
    static int32_t scaleDuration(int32_t baseDuration, double multiplier) {
        return static_cast<int32_t>(multiplier * baseDuration + 0.5);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityExpBottle — Bottle o' Enchanting, spawns XP orbs.
// Java: net.minecraft.entity.item.EntityExpBottle (55 lines)
//
//   Extends EntityThrowable
//   Gravity: 0.07, speed: 0.7, pitch offset: -20
//   On impact: XP = 3 + rand(5) + rand(5) (range 3-13, avg 7)
//   Split into orbs using EntityXPOrb.getXPSplit()
//   Sound effect 2002 (glass break)
// ═══════════════════════════════════════════════════════════════════════════

class EntityExpBottle {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    static constexpr float GRAVITY = 0.07f;
    static constexpr float SPEED = 0.7f;
    static constexpr float PITCH_OFFSET = -20.0f;
    static constexpr int32_t XP_BASE = 3;
    static constexpr int32_t XP_RAND = 5;  // two rolls
    static constexpr int32_t SOUND_ID = 2002;

    // XP value: 3 + rand(5) + rand(5) = range [3,13]
    static int32_t calculateXP(int32_t rand1, int32_t rand2) {
        return XP_BASE + rand1 + rand2;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityEnderEye — Eye of Ender, flies toward stronghold.
// Java: net.minecraft.entity.item.EntityEnderEye (137 lines)
//
//   Size: 0.25×0.25
//   Always bright (1.0), can't be attacked
//   moveTowards(x, y, z):
//     If dist > 12: target = pos + dir*12, targetY = posY+8
//     Else: target = exact coords
//     shatterOrDrop: 80% drop ender_eye, 20% shatter
//   Flight physics:
//     Approach accel: horzSpeed + (targetDist - horzSpeed) * 0.0025
//     If targetDist < 1: speed *= 0.8, motionY *= 0.8
//     motionX = cos(angle) * speed, motionZ = sin(angle) * speed
//     Vertical: motionY += (1-motionY)*0.015 if below, (-1-motionY)*0.015 if above
//   Despawn: 80 ticks
//   Water: 4 bubble particles, else: portal particle
//   Rotation: smoothed 0.2
//   No NBT save/load
// ═══════════════════════════════════════════════════════════════════════════

class EntityEnderEye {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    float prevRotationYaw = 0, prevRotationPitch = 0;
    bool isDead = false;
    int32_t entityId = 0;

    double targetX = 0, targetY = 0, targetZ = 0;
    int32_t despawnTimer = 0;
    bool shatterOrDrop = true; // true = drop ender_eye

    static constexpr float WIDTH = 0.25f;
    static constexpr float HEIGHT = 0.25f;
    static constexpr float BRIGHTNESS = 1.0f;
    static constexpr double MAX_APPROACH_DIST = 12.0;
    static constexpr double APPROACH_ACCEL = 0.0025;
    static constexpr double CLOSE_SLOW = 0.8;
    static constexpr double CLOSE_THRESHOLD = 1.0;
    static constexpr double VERTICAL_ACCEL = 0.015;
    static constexpr int32_t DESPAWN_TIME = 80;
    static constexpr float ROTATION_SMOOTH = 0.2f;
    static constexpr int32_t DROP_CHANCE_DENOM = 5; // rand(5)>0 = 80% drop
    static constexpr int32_t DROP_ENDER_EYE = 381;
    static constexpr int32_t SHATTER_EFFECT = 2003;

    // ─── Move towards stronghold ───
    void moveTowards(double x, int32_t y, double z, int32_t rand5) {
        double dx = x - posX;
        double dz = z - posZ;
        float dist = static_cast<float>(std::sqrt(dx * dx + dz * dz));
        if (dist > MAX_APPROACH_DIST) {
            targetX = posX + dx / dist * MAX_APPROACH_DIST;
            targetZ = posZ + dz / dist * MAX_APPROACH_DIST;
            targetY = posY + 8.0;
        } else {
            targetX = x;
            targetY = y;
            targetZ = z;
        }
        despawnTimer = 0;
        shatterOrDrop = rand5 > 0; // 80% true = drop
    }

    // ─── Flight tick ───
    void tickFlight() {
        double dx = targetX - posX;
        double dz = targetZ - posZ;
        float targetDist = static_cast<float>(std::sqrt(dx * dx + dz * dz));
        float horzSpeed = static_cast<float>(std::sqrt(motionX * motionX + motionZ * motionZ));
        float angle = static_cast<float>(std::atan2(dz, dx));

        double speed = horzSpeed + (targetDist - horzSpeed) * APPROACH_ACCEL;
        if (targetDist < CLOSE_THRESHOLD) {
            speed *= CLOSE_SLOW;
            motionY *= CLOSE_SLOW;
        }
        motionX = std::cos(angle) * speed;
        motionZ = std::sin(angle) * speed;

        if (posY < targetY) {
            motionY += (1.0 - motionY) * VERTICAL_ACCEL;
        } else {
            motionY += (-1.0 - motionY) * VERTICAL_ACCEL;
        }

        posX += motionX;
        posY += motionY;
        posZ += motionZ;
    }

    bool shouldDespawn() { return ++despawnTimer > DESPAWN_TIME; }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityFireworkRocket — Firework rocket entity.
// Java: net.minecraft.entity.item.EntityFireworkRocket (122 lines)
//
//   Size: 0.25×0.25
//   DW 8: ItemStack (firework item)
//   Lifetime: 10 * (1 + flight_level) + rand(6) + rand(7)
//   motionXZ: Gaussian * 0.001 initially, *= 1.15 each tick (accelerating)
//   motionY: 0.05 initial, += 0.04 each tick (upward accel)
//   Launch sound at tick 0: "fireworks.launch" volume 3.0
//   fireworksSpark particle every 2 ticks (client)
//   Death: entity state byte 17 (triggers explosion effect)
//   Rotation: smoothed 0.2
//   NBT: "Life" int, "LifeTime" int, "FireworksItem" compound
// ═══════════════════════════════════════════════════════════════════════════

class EntityFireworkRocket {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    float prevRotationYaw = 0, prevRotationPitch = 0;
    bool isDead = false;
    int32_t entityId = 0;

    int32_t fireworkAge = 0;
    int32_t lifetime = 0;

    static constexpr float WIDTH = 0.25f;
    static constexpr float HEIGHT = 0.25f;
    static constexpr double INITIAL_Y_SPEED = 0.05;
    static constexpr double Y_ACCEL = 0.04;
    static constexpr double XZ_ACCEL_MULT = 1.15;
    static constexpr double XZ_INITIAL_SCATTER = 0.001;
    static constexpr int32_t LIFETIME_BASE_MULT = 10;
    static constexpr int32_t LIFETIME_RAND_1 = 6;
    static constexpr int32_t LIFETIME_RAND_2 = 7;
    static constexpr float ROTATION_SMOOTH = 0.2f;
    static constexpr int8_t DEATH_ENTITY_STATE = 17;

    // ─── Lifetime calculation ───
    // Java: 10 * (1 + flight_byte) + rand(6) + rand(7)
    static int32_t calculateLifetime(int32_t flightLevel, int32_t rand6, int32_t rand7) {
        return LIFETIME_BASE_MULT * (1 + flightLevel) + rand6 + rand7;
    }

    // ─── Physics tick ───
    void tickMotion() {
        motionX *= XZ_ACCEL_MULT;
        motionZ *= XZ_ACCEL_MULT;
        motionY += Y_ACCEL;
        posX += motionX;
        posY += motionY;
        posZ += motionZ;
    }

    bool isFirstTick() const { return fireworkAge == 0; }
    bool shouldExplode() const { return fireworkAge > lifetime; }

    void tick() { ++fireworkAge; }

    static constexpr const char* LAUNCH_SOUND = "fireworks.launch";
    static constexpr float LAUNCH_VOLUME = 3.0f;
    static constexpr float LAUNCH_PITCH = 1.0f;
    static constexpr const char* PARTICLE = "fireworksSpark";
};

} // namespace mccpp
