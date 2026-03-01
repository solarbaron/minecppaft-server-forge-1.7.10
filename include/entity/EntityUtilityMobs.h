/**
 * EntityUtilityMobs.h — Ambient, utility, and tameable mob implementations.
 *
 * Java references:
 *   - net.minecraft.entity.passive.EntityBat (213 lines)
 *   - net.minecraft.entity.passive.EntitySquid (166 lines)
 *   - net.minecraft.entity.passive.EntityOcelot (296 lines)
 *   - net.minecraft.entity.monster.EntityIronGolem (219 lines)
 *   - net.minecraft.entity.monster.EntitySnowman (98 lines)
 *
 * Thread safety: Entity tick on single thread per entity.
 */
#pragma once

#include <cmath>
#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityBat — Ambient flying mob that hangs from ceilings.
// Java: net.minecraft.entity.passive.EntityBat (213 lines)
//
//   Size: 0.5×0.9, HP 6
//   Sound volume: 0.1, pitch ×0.95
//   Hanging: DW 16 bit 0, motionXYZ=0, snap to Y=(floor(Y)+1-height)
//   Not hanging: slow fall motionY *= 0.6
//   Hanging AI:
//     - Block above removed → wake, play SFX 1015
//     - Player within 4 → wake, play SFX 1015
//     - rand(200)==0 → rotate head
//   Flying AI:
//     - Fly toward spawnPosition waypoint (±7 XZ, +6/-2 Y)
//     - Motion: += (signum * speed - current) * 0.1
//       X/Z speed 0.5, Y speed 0.7
//     - Re-pick if rand(30)==0 or dist²<4 or invalid
//     - rand(100)==0 && normalCube above → hang
//   No fall, no walking, no pressure plate
//   Can't be pushed, no entity collision
//   Won't hang if damaged
//   Spawn: Y<63, light≤rand(4), Halloween (Oct 20-Nov 3) max light 7,
//     else 50% fail
//   NBT: "BatFlags" byte
// ═══════════════════════════════════════════════════════════════════════════

class EntityBat {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0;
    bool isDead = false;
    int32_t entityId = 0;

    bool hanging = true;
    int32_t spawnX = 0, spawnY = 0, spawnZ = 0;
    bool hasSpawnPos = false;

    static constexpr float WIDTH = 0.5f;
    static constexpr float HEIGHT = 0.9f;
    static constexpr double MAX_HEALTH = 6.0;
    static constexpr float SOUND_VOLUME = 0.1f;
    static constexpr float PITCH_MULT = 0.95f;
    static constexpr int32_t HEAD_ROTATE_CHANCE = 200;
    static constexpr double WAKE_RANGE = 4.0;
    static constexpr float SLOW_FALL = 0.6f;
    static constexpr double FLY_SPEED_XZ = 0.5;
    static constexpr double FLY_SPEED_Y = 0.7;
    static constexpr double FLY_ACCEL = 0.1;
    static constexpr int32_t WAYPOINT_RANGE_XZ = 7;
    static constexpr int32_t WAYPOINT_RANGE_Y_UP = 6;
    static constexpr int32_t WAYPOINT_RANGE_Y_DOWN = 2;
    static constexpr int32_t HANG_CHANCE = 100;
    static constexpr int32_t REPICK_CHANCE = 30;
    static constexpr float REPICK_MIN_DIST_SQ = 4.0f;
    static constexpr int32_t MAX_SPAWN_Y = 63;
    static constexpr int32_t SPAWN_LIGHT_NORMAL = 4;
    static constexpr int32_t SPAWN_LIGHT_HALLOWEEN = 7;

    // ─── Hanging behavior ───
    void tickHanging() {
        motionX = motionY = motionZ = 0;
        posY = std::floor(posY) + 1.0 - HEIGHT;
    }

    // ─── Flying behavior ───
    struct FlyResult {
        bool pickNewWaypoint;
        bool shouldHang;
    };

    FlyResult tickFlying(int32_t rand30, int32_t rand100, bool ceilingNormalCube) {
        FlyResult result{};
        motionY *= SLOW_FALL;

        if (!hasSpawnPos || rand30 == 0) {
            result.pickNewWaypoint = true;
        } else {
            double dx = spawnX + 0.5 - posX;
            double dy = spawnY + 0.1 - posY;
            double dz = spawnZ + 0.5 - posZ;
            if (dx*dx + dy*dy + dz*dz < REPICK_MIN_DIST_SQ) {
                result.pickNewWaypoint = true;
            } else {
                double sx = (dx > 0 ? 1.0 : (dx < 0 ? -1.0 : 0.0));
                double sy = (dy > 0 ? 1.0 : (dy < 0 ? -1.0 : 0.0));
                double sz = (dz > 0 ? 1.0 : (dz < 0 ? -1.0 : 0.0));
                motionX += (sx * FLY_SPEED_XZ - motionX) * FLY_ACCEL;
                motionY += (sy * FLY_SPEED_Y - motionY) * FLY_ACCEL;
                motionZ += (sz * FLY_SPEED_XZ - motionZ) * FLY_ACCEL;
                rotationYaw = static_cast<float>(std::atan2(motionZ, motionX) * 180.0 / M_PI) - 90.0f;
            }
        }

        if (rand100 == 0 && ceilingNormalCube) result.shouldHang = true;
        return result;
    }

    // Spawn check
    static bool canSpawn(int32_t y, int32_t blockLight, bool isHalloween, bool randBool, int32_t randLight) {
        if (y >= MAX_SPAWN_Y) return false;
        int32_t maxLight = isHalloween ? SPAWN_LIGHT_HALLOWEEN : SPAWN_LIGHT_NORMAL;
        if (!isHalloween && randBool) return false; // 50% fail outside Halloween
        return blockLight <= randLight;
    }

    static constexpr const char* LIVING_SOUND = "mob.bat.idle";
    static constexpr const char* HURT_SOUND = "mob.bat.hurt";
    static constexpr const char* DEATH_SOUND = "mob.bat.death";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntitySquid — Water mob with jet propulsion.
// Java: net.minecraft.entity.passive.EntitySquid (166 lines)
//
//   Size: 0.95×0.95, HP 10
//   No sounds (silent)
//   Drops: ink sac (dye:0, rand(3+looting)+1)
//   Sound volume: 0.4
//   Water physics:
//     - Jet cycle: squidRotation += rotationVelocity
//     - rotationVelocity = 1/(rand+1) * 0.2 (reset at 2π, 10% chance)
//     - When rotation < π: tentacle = sin(f²*π)*π*0.25
//       If f > 0.75: speed=1, yawSpeed=1
//       Else: yawSpeed *= 0.8
//     - When rotation ≥ π: tentacle=0, speed*=0.9, yawSpeed*=0.99
//     - motion = randomMotionVec * speed
//     - Yaw: atan2(motionX, motionZ)
//   Out of water: tentacle = |sin(rotation)| * π * 0.25
//     gravity 0.08, drag 0.98, motionXZ=0
//   Random motion: cos/sin * 0.2, Y = -0.1 + rand*0.2
//     Update: rand(50)==0 or no motion, or age>100→zero
//   Spawn: Y 45-63
//   No walking trigger
// ═══════════════════════════════════════════════════════════════════════════

class EntitySquid {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    bool isDead = false;
    bool inWater = false;
    int32_t entityId = 0;
    int32_t entityAge = 0;

    // Animation
    float squidPitch = 0, prevSquidPitch = 0;
    float squidYaw = 0, prevSquidYaw = 0;
    float squidRotation = 0, prevSquidRotation = 0;
    float tentacleAngle = 0, lastTentacleAngle = 0;
    float randomMotionSpeed = 0;
    float rotationVelocity = 0;
    float yawSpeed = 0;
    float randomMotionVecX = 0, randomMotionVecY = 0, randomMotionVecZ = 0;

    static constexpr float WIDTH = 0.95f;
    static constexpr float HEIGHT = 0.95f;
    static constexpr double MAX_HEALTH = 10.0;
    static constexpr float SOUND_VOLUME = 0.4f;
    static constexpr int32_t DROP_INK_SAC = 351; // dye, damage 0
    static constexpr double SPAWN_Y_MIN = 45.0;
    static constexpr double SPAWN_Y_MAX = 63.0;
    static constexpr double GRAVITY = 0.08;
    static constexpr double DRAG = 0.98;

    void initRotation(float randFloat) {
        rotationVelocity = 1.0f / (randFloat + 1.0f) * 0.2f;
    }

    // ─── Jet propulsion physics ───
    struct JetResult {
        bool resetVelocity;
    };

    JetResult tickJet(int32_t rand10) {
        JetResult result{};
        prevSquidPitch = squidPitch;
        prevSquidYaw = squidYaw;
        prevSquidRotation = squidRotation;
        lastTentacleAngle = tentacleAngle;
        squidRotation += rotationVelocity;

        if (squidRotation > static_cast<float>(M_PI) * 2.0f) {
            squidRotation -= static_cast<float>(M_PI) * 2.0f;
            if (rand10 == 0) result.resetVelocity = true;
        }

        if (inWater) {
            if (squidRotation < static_cast<float>(M_PI)) {
                float f = squidRotation / static_cast<float>(M_PI);
                tentacleAngle = std::sin(f * f * static_cast<float>(M_PI)) *
                                 static_cast<float>(M_PI) * 0.25f;
                if (f > 0.75f) {
                    randomMotionSpeed = 1.0f;
                    yawSpeed = 1.0f;
                } else {
                    yawSpeed *= 0.8f;
                }
            } else {
                tentacleAngle = 0;
                randomMotionSpeed *= 0.9f;
                yawSpeed *= 0.99f;
            }
            motionX = randomMotionVecX * randomMotionSpeed;
            motionY = randomMotionVecY * randomMotionSpeed;
            motionZ = randomMotionVecZ * randomMotionSpeed;
        } else {
            tentacleAngle = std::abs(std::sin(squidRotation)) *
                             static_cast<float>(M_PI) * 0.25f;
            motionX = 0;
            motionY -= GRAVITY;
            motionY *= DRAG;
            motionZ = 0;
        }

        return result;
    }

    // Random motion update
    void updateRandomMotion(float randAngle, float randY) {
        randomMotionVecX = std::cos(randAngle) * 0.2f;
        randomMotionVecY = -0.1f + randY * 0.2f;
        randomMotionVecZ = std::sin(randAngle) * 0.2f;
    }

    void clearMotion() {
        randomMotionVecX = randomMotionVecY = randomMotionVecZ = 0;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityOcelot — Tameable cat with jungle spawning.
// Java: net.minecraft.entity.passive.EntityOcelot (296 lines)
//
//   Size: 0.6×0.8, HP 10, speed 0.3
//   Tameable with raw fish within 3 blocks while tempted, 33% success
//   Cat types: 0=wild, 1=tuxedo, 2=tabby, 3=siamese
//   DataWatcher 18: cat type byte
//   Speed modes: sneaking=0.6, sprinting=1.33
//   No fall damage, no drops on death
//   Attacks chickens when untamed every 750 ticks
//   Flees players 16 blocks (priority 4)
//   Damage: 3.0
//   Sounds: tamed in love→purr, tamed 25%→purreow, else→meow
//     Wild: silent (""), hurt/death: "mob.cat.hitt"
//   Sound volume: 0.4
//   Breeding: raw fish, both tamed + in love
//   Despawn: untamed + 2400 ticks
//   Spawn: rand(3)!=0→fail, no collision, Y≥63, grass or leaves below
//   14% spawn with 2 kittens (age -24000)
//   Child inherits owner + tamed + cat type
//   NBT: "CatType" integer
// ═══════════════════════════════════════════════════════════════════════════

class EntityOcelot {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;
    int32_t ticksExisted = 0;

    bool tamed = false;
    bool sitting = false;
    int32_t catType = 0; // 0=wild, 1-3=tamed variants
    float health = 10.0f;

    static constexpr float WIDTH = 0.6f;
    static constexpr float HEIGHT = 0.8f;
    static constexpr double MAX_HEALTH = 10.0;
    static constexpr double MOVE_SPEED = 0.3;
    static constexpr float ATTACK_DAMAGE = 3.0f;
    static constexpr float SOUND_VOLUME = 0.4f;
    static constexpr double SNEAK_SPEED = 0.6;
    static constexpr double SPRINT_SPEED = 1.33;
    static constexpr float FLEE_RANGE = 16.0f;
    static constexpr int32_t HUNT_CHICKEN_INTERVAL = 750;
    static constexpr int32_t TAME_CHANCE = 3; // 1/3
    static constexpr double TAME_RANGE_SQ = 9.0;
    static constexpr int32_t DESPAWN_AGE = 2400;
    static constexpr int32_t MIN_SPAWN_Y = 63;
    static constexpr int32_t SPAWN_CHANCE = 3; // 1/3 pass
    static constexpr int32_t KITTEN_SPAWN_CHANCE = 7; // 1/7
    static constexpr int32_t KITTEN_COUNT = 2;
    static constexpr int32_t KITTEN_AGE = -24000;
    static constexpr int32_t ITEM_FISH = 349;
    static constexpr int32_t DROP_LEATHER = 334;

    struct TameResult {
        bool success;
        int32_t catType; // 1-3
        int8_t entityState; // 7=success, 6=fail
    };

    TameResult tryTame(int32_t rand3, int32_t randCatType) {
        if (rand3 == 0) return {true, 1 + randCatType, 7};
        return {false, 0, 6};
    }

    const char* getLivingSound(bool isInLove, int32_t rand4) const {
        if (tamed) {
            if (isInLove) return "mob.cat.purr";
            if (rand4 == 0) return "mob.cat.purreow";
            return "mob.cat.meow";
        }
        return "";
    }

    void fall(float /*dist*/) { /* no-op */ }
    bool canDespawn() const { return !tamed && ticksExisted > DESPAWN_AGE; }

    static constexpr const char* HURT_SOUND = "mob.cat.hitt";
    static constexpr const char* DEATH_SOUND = "mob.cat.hitt";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityIronGolem — Village protector golem.
// Java: net.minecraft.entity.monster.EntityIronGolem (219 lines)
//
//   Size: 1.4×2.9, HP 100, speed 0.25
//   Attack: 7 + rand(15) damage, Y knockback 0.4, timer 10, entity state 4
//   Village binding: check 70+rand(50) ticks, radius × 0.6
//   Player-created: DW 16 bit 0
//     - Player-created can't attack players
//     - Not player-created: -5 village rep on death
//   Mob collision: 5% chance to target hostile mob
//   No drowning (decreaseAirSupply returns unchanged)
//   Block crush particles: when moving (speed² > 2.5e-7)
//   Rose holding: 400 ticks, entity state 11
//   Drops: poppies 0-2, iron ingots 3-5
//   AI: attack(1.0), moveToTarget(0.9,32), moveThroughVillage(0.6),
//       moveToRestrict(1.0), lookAtVillager, wander(0.6), watch(player,6),
//       lookIdle, defendVillage, hurtBy, nearestTarget(mobs)
//   NBT: "PlayerCreated" boolean
// ═══════════════════════════════════════════════════════════════════════════

class EntityIronGolem {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    bool playerCreated = false;
    int32_t attackTimer = 0;
    int32_t holdRoseTick = 0;
    int32_t homeCheckTimer = 0;

    static constexpr float WIDTH = 1.4f;
    static constexpr float HEIGHT = 2.9f;
    static constexpr double MAX_HEALTH = 100.0;
    static constexpr double MOVE_SPEED = 0.25;
    static constexpr int32_t BASE_DAMAGE = 7;
    static constexpr int32_t DAMAGE_RAND = 15;
    static constexpr float KNOCKBACK_Y = 0.4f;
    static constexpr int32_t ATTACK_TIMER_DURATION = 10;
    static constexpr int32_t HOME_CHECK_BASE = 70;
    static constexpr int32_t HOME_CHECK_RAND = 50;
    static constexpr float VILLAGE_RADIUS_MULT = 0.6f;
    static constexpr int32_t COLLISION_TARGET_CHANCE = 20; // 1/20=5%
    static constexpr int32_t ROSE_DURATION = 400;
    static constexpr int32_t REPUTATION_PENALTY = -5;
    static constexpr double MOVEMENT_THRESHOLD = 2.5e-7;

    // Drops
    static constexpr int32_t DROP_RED_FLOWER = 38; // block ID
    static constexpr int32_t DROP_IRON_INGOT = 265;
    static constexpr int32_t MIN_IRON = 3;
    static constexpr int32_t MAX_IRON_RAND = 3;
    static constexpr int32_t MAX_FLOWER_RAND = 3;

    // ─── Attack ───
    struct AttackResult {
        int32_t damage;
        float knockbackY;
    };

    AttackResult attack(int32_t randDamage) {
        attackTimer = ATTACK_TIMER_DURATION;
        return {BASE_DAMAGE + randDamage, KNOCKBACK_Y};
    }

    // ─── Rose holding ───
    void setHoldingRose(bool hold) { holdRoseTick = hold ? ROSE_DURATION : 0; }

    // ─── Tick ───
    void tick() {
        if (attackTimer > 0) --attackTimer;
        if (holdRoseTick > 0) --holdRoseTick;
    }

    bool isMoving() const {
        return motionX * motionX + motionZ * motionZ > MOVEMENT_THRESHOLD;
    }

    // No drowning
    static int32_t decreaseAirSupply(int32_t current) { return current; }

    // Player-created can't attack players
    bool canAttackPlayer() const { return !playerCreated; }

    struct DropResult {
        int32_t flowerCount;
        int32_t ironCount;
    };

    static DropResult calculateDrops(int32_t rand3a, int32_t rand3b) {
        return {rand3a, MIN_IRON + rand3b};
    }

    static constexpr const char* HURT_SOUND = "mob.irongolem.hit";
    static constexpr const char* DEATH_SOUND = "mob.irongolem.death";
    static constexpr const char* STEP_SOUND = "mob.irongolem.walk";
    static constexpr const char* ATTACK_SOUND = "mob.irongolem.throw";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntitySnowman — Snow-trail utility golem.
// Java: net.minecraft.entity.monster.EntitySnowman (98 lines)
//
//   Size: 0.4×1.8, HP 4, speed 0.2
//   Ranged attack: snowball, speed 1.6, inaccuracy 12, cooldown 20
//   Water: 1 damage (drown)
//   Hot biome (temp > 1.0): 1 fire damage
//   Snow trail: 4 blocks ±0.25 around feet, temp < 0.8, air block, can place
//   Drops: snowball rand(16)
//   Target: all hostile mobs (IMob)
//   Snowball aim: Y offset = dist * 0.2
//   AI: rangedAttack(1.25,20,10), wander(1.0), watch(player,6), lookIdle,
//       nearestTarget(mobs)
// ═══════════════════════════════════════════════════════════════════════════

class EntitySnowman {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    static constexpr float WIDTH = 0.4f;
    static constexpr float HEIGHT = 1.8f;
    static constexpr double MAX_HEALTH = 4.0;
    static constexpr double MOVE_SPEED = 0.2;
    static constexpr float SNOWBALL_SPEED = 1.6f;
    static constexpr float SNOWBALL_INACCURACY = 12.0f;
    static constexpr int32_t ATTACK_COOLDOWN = 20;
    static constexpr float ATTACK_RANGE = 10.0f;
    static constexpr double ATTACK_WALK_SPEED = 1.25;
    static constexpr float WATER_DAMAGE = 1.0f;
    static constexpr float HOT_THRESHOLD = 1.0f;
    static constexpr float SNOW_THRESHOLD = 0.8f;
    static constexpr float TRAIL_OFFSET = 0.25f;
    static constexpr int32_t DROP_SNOWBALL = 332;
    static constexpr int32_t MAX_DROP_COUNT = 16;

    // ─── Snow trail positions ───
    struct TrailPosition {
        int32_t x, y, z;
    };

    static void getTrailPositions(double posX, double posY, double posZ,
                                    TrailPosition out[4]) {
        int32_t baseY = static_cast<int32_t>(std::floor(posY));
        for (int i = 0; i < 4; ++i) {
            float offX = (i % 2 * 2 - 1) * TRAIL_OFFSET;
            float offZ = (i / 2 % 2 * 2 - 1) * TRAIL_OFFSET;
            out[i].x = static_cast<int32_t>(std::floor(posX + offX));
            out[i].y = baseY;
            out[i].z = static_cast<int32_t>(std::floor(posZ + offZ));
        }
    }

    // ─── Snowball aim ───
    struct SnowballAim {
        double dx, dy, dz;
    };

    static SnowballAim calculateAim(double selfX, double selfY, double selfZ,
                                      double targetX, double targetY, double targetZ,
                                      float targetEyeHeight, float ballY) {
        double dx = targetX - selfX;
        double dz = targetZ - selfZ;
        float dist = static_cast<float>(std::sqrt(dx*dx + dz*dz));
        double dy = targetY + targetEyeHeight - 1.1 - ballY;
        return {dx, dy + dist * 0.2, dz};
    }

    static constexpr const char* ATTACK_SOUND = "random.bow";
};

} // namespace mccpp
