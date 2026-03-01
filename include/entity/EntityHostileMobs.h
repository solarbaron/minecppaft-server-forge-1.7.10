/**
 * EntityHostileMobs.h — Core hostile mob implementations.
 *
 * Java references:
 *   - net.minecraft.entity.monster.EntityZombie (491 lines)
 *   - net.minecraft.entity.monster.EntityCreeper (219 lines)
 *   - net.minecraft.entity.monster.EntitySkeleton (300 lines)
 *   - net.minecraft.entity.monster.EntitySpider (170 lines)
 *
 * Thread safety: Entity tick on single thread per entity.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityZombie — Undead hostile mob with variants.
// Java: net.minecraft.entity.monster.EntityZombie (491 lines)
//
//   Size: 0.6×1.8 (baby: 0.3×0.9)
//   Attributes: followRange 40, moveSpeed 0.23, attackDamage 3
//   Baby: +0.5 speed (multiply op), XP ×2.5, size ×0.5
//   Villager zombie: can be cured with golden apple + weakness
//   Sunlight: burns if brightness>0.5, rand*30 < (bright-0.4)*2, can see sky,
//     helmet absorbs but takes damage (rand 0-1)
//   Reinforcements (Hard only): on damage, reinforcementChance (0-10%),
//     spawn at ±7-40 blocks, solid below, light <10, -5% per call
//   Fire attack: if burning + no held item, chance = difficulty * 30%,
//     fire duration = difficulty * 2
//   Villager kill: convert to zombie villager (Normal: 50%, Hard: 100%)
//   Conversion: 3600-6000 ticks, boost by nearby iron bars/beds (1% check,
//     ±4 blocks, 30% per bar/bed, max 14 checks)
//   Break doors: based on tension factor (10% chance)
//   Drops: rotten_flesh, rare: iron_ingot/carrot/potato (equal 1/3)
//   Equipment: Hard 5%, Normal 1% chance iron_sword or iron_shovel
//   Spawn: 5% baby, 5% villager, baby 5% chicken jockey,
//     knockback resistance +0-5%, follow range bonus ×1.5*tension,
//     leader 5%*tension (+25-75% reinforcement, +1-4 HP multiply, break doors)
//   Halloween: pumpkin/lit_pumpkin head (25%, 10% lit)
//   DataWatcher: 12 (isChild byte), 13 (isVillager byte), 14 (isConverting byte)
// ═══════════════════════════════════════════════════════════════════════════

class EntityZombie {
public:
    // ─── Entity base ───
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0;
    bool isDead = false;
    int32_t entityId = 0;

    // ─── Zombie-specific ───
    bool isChild_ = false;
    bool isVillager_ = false;
    bool isConverting_ = false;
    int32_t conversionTime = -1;
    bool canBreakDoors = false;
    double reinforcementChance = 0;  // 0.0 to 0.1 base, modified
    int32_t experienceValue = 5;

    // ─── Constants ───
    static constexpr float WIDTH = 0.6f;
    static constexpr float HEIGHT = 1.8f;
    static constexpr float CHILD_SCALE = 0.5f;
    static constexpr double FOLLOW_RANGE = 40.0;
    static constexpr double MOVE_SPEED = 0.23;
    static constexpr double ATTACK_DAMAGE = 3.0;
    static constexpr double BABY_SPEED_BOOST = 0.5;
    static constexpr float BABY_XP_MULT = 2.5f;
    static constexpr int32_t ARMOR_BONUS = 2;
    static constexpr int32_t MAX_ARMOR = 20;

    // Sunlight
    static constexpr float BURN_BRIGHTNESS_THRESHOLD = 0.5f;
    static constexpr int32_t BURN_FIRE_TICKS = 8;

    // Reinforcement
    static constexpr int32_t REINFORCE_SEARCH_ATTEMPTS = 50;
    static constexpr int32_t REINFORCE_RANGE_MIN = 7;
    static constexpr int32_t REINFORCE_RANGE_MAX = 40;
    static constexpr int32_t REINFORCE_MAX_LIGHT = 10;
    static constexpr double REINFORCE_CHARGE = -0.05;

    // Conversion
    static constexpr int32_t CONVERSION_TIME_MIN = 3600;
    static constexpr int32_t CONVERSION_TIME_MAX = 6001; // rand bound (exclusive)
    static constexpr int32_t CONVERSION_SCAN_RANGE = 4;
    static constexpr int32_t CONVERSION_MAX_BLOCKS = 14;
    static constexpr float CONVERSION_SCAN_CHANCE = 0.01f;
    static constexpr float CONVERSION_BLOCK_CHANCE = 0.3f;

    // Drops
    static constexpr int32_t DROP_ROTTEN_FLESH = 367;
    static constexpr int32_t RARE_IRON_INGOT = 265;
    static constexpr int32_t RARE_CARROT = 391;
    static constexpr int32_t RARE_POTATO = 392;

    // ─── Sunlight burning ───
    struct SunlightResult {
        bool shouldBurn;
        bool damageHelmet;
    };

    static SunlightResult checkSunlight(float brightness, bool isDaytime, bool canSeeSky,
                                          bool hasHelmet, bool isChild, float randFloat) {
        SunlightResult result{};
        if (!isDaytime || isChild) return result;
        if (brightness <= BURN_BRIGHTNESS_THRESHOLD) return result;
        if (randFloat * 30.0f >= (brightness - 0.4f) * 2.0f) return result;
        if (!canSeeSky) return result;
        if (hasHelmet) { result.damageHelmet = true; return result; }
        result.shouldBurn = true;
        return result;
    }

    // ─── Fire on attack ───
    static bool shouldSetFireOnAttack(bool isBurning, bool hasHeldItem, int32_t difficultyId, float randFloat) {
        return !hasHeldItem && isBurning && randFloat < difficultyId * 0.3f;
    }
    static int32_t fireAttackDuration(int32_t difficultyId) { return 2 * difficultyId; }

    // ─── Villager kill → zombie conversion ───
    static bool shouldConvertVillager(int32_t difficultyId, bool randBool) {
        if (difficultyId < 2) return false; // Normal or Hard only
        if (difficultyId == 3) return true;  // Hard: always
        return !randBool; // Normal: 50%
    }

    // ─── Conversion boost ───
    int32_t getConversionBoost(int32_t ironBarsNearby, int32_t bedsNearby,
                                float scanRand, const float* blockRands) const {
        int32_t boost = 1;
        if (scanRand < CONVERSION_SCAN_CHANCE) {
            int32_t total = ironBarsNearby + bedsNearby;
            int32_t checked = 0;
            for (int32_t i = 0; i < total && checked < CONVERSION_MAX_BLOCKS; ++i) {
                if (blockRands[i] < CONVERSION_BLOCK_CHANCE) ++boost;
                ++checked;
            }
        }
        return boost;
    }

    // Sounds
    static constexpr const char* LIVING_SOUND = "mob.zombie.say";
    static constexpr const char* HURT_SOUND = "mob.zombie.hurt";
    static constexpr const char* DEATH_SOUND = "mob.zombie.death";
    static constexpr const char* STEP_SOUND = "mob.zombie.step";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityCreeper — Explosive mob with fuse mechanic.
// Java: net.minecraft.entity.monster.EntityCreeper (219 lines)
//
//   Size: 0.6×1.7
//   Attributes: moveSpeed 0.25
//   Fuse: 30 ticks default, timeSinceIgnited increments/decrements per tick
//     based on creeperState (+1 or -1 from EntityAICreeperSwell)
//   Explosion: radius 3 (powered: ×2 = 6), mobGriefing gamerule
//   Powered: DataWatcher 17 byte, set by lightning strike
//   Fall: timeSinceIgnited += fallDist * 1.5, clamped to fuseTime-5
//   Ignition: flint+steel → DataWatcher 18 byte = 1
//   Record drop: when killed by skeleton, random record_13..record_wait
//   Flee: EntityAIAvoidEntity from ocelots, 6 blocks, speed 1.0/1.2
//   Max fall height: 3 + (health - 1) when has target, else 3
//   Sounds: creeper.primed on fuse start, mob.creeper.say (hurt), mob.creeper.death
//   DataWatcher: 16 (state byte: -1/1), 17 (powered byte), 18 (ignited byte)
// ═══════════════════════════════════════════════════════════════════════════

class EntityCreeper {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    // Creeper state
    int32_t timeSinceIgnited = 0;
    int32_t lastActiveTime = 0;
    int32_t fuseTime = 30;
    int32_t explosionRadius = 3;
    bool powered = false;
    bool ignited = false;
    int8_t creeperState = -1; // -1 = idle, 1 = swelling

    // ─── Constants ───
    static constexpr float WIDTH = 0.6f;
    static constexpr float HEIGHT = 1.7f;
    static constexpr double MOVE_SPEED = 0.25;
    static constexpr int32_t DEFAULT_FUSE = 30;
    static constexpr int32_t DEFAULT_RADIUS = 3;
    static constexpr int32_t POWERED_MULTIPLIER = 2;
    static constexpr float FALL_FUSE_MULTIPLIER = 1.5f;
    static constexpr int32_t FUSE_SAFETY_MARGIN = 5;
    static constexpr float OCELOT_FLEE_RANGE = 6.0f;

    // Drop items
    static constexpr int32_t DROP_GUNPOWDER = 289;
    static constexpr int32_t RECORD_13_ID = 2256;
    static constexpr int32_t RECORD_WAIT_ID = 2267;

    // ═══════════════════════════════════════════════════════════════════════
    // onUpdate — Fuse state machine.
    // Java: EntityCreeper.onUpdate
    //   - lastActiveTime = timeSinceIgnited (for render interpolation)
    //   - If ignited → force creeperState = 1
    //   - If state > 0 and timeSinceIgnited == 0 → play "creeper.primed"
    //   - timeSinceIgnited += state
    //   - Clamp: if < 0, set to 0
    //   - If >= fuseTime → explode
    // ═══════════════════════════════════════════════════════════════════════

    struct FuseTickResult {
        bool playPrimeSound;
        bool shouldExplode;
        int32_t effectiveRadius;
        bool mobGriefing; // caller provides
    };

    FuseTickResult tickFuse(bool isAlive) {
        FuseTickResult result{};
        if (!isAlive) return result;

        lastActiveTime = timeSinceIgnited;
        if (ignited) creeperState = 1;

        if (creeperState > 0 && timeSinceIgnited == 0) {
            result.playPrimeSound = true;
        }

        timeSinceIgnited += creeperState;
        if (timeSinceIgnited < 0) timeSinceIgnited = 0;

        if (timeSinceIgnited >= fuseTime) {
            timeSinceIgnited = fuseTime;
            result.shouldExplode = true;
            result.effectiveRadius = powered ? explosionRadius * POWERED_MULTIPLIER : explosionRadius;
        }

        return result;
    }

    // Fall accelerates fuse
    void onFall(float fallDistance) {
        timeSinceIgnited += static_cast<int32_t>(fallDistance * FALL_FUSE_MULTIPLIER);
        if (timeSinceIgnited > fuseTime - FUSE_SAFETY_MARGIN) {
            timeSinceIgnited = fuseTime - FUSE_SAFETY_MARGIN;
        }
    }

    // Lightning → powered
    void onStruckByLightning() { powered = true; }

    // Flint and steel → ignite
    void onIgnite() { ignited = true; }

    int32_t getMaxFallHeight(bool hasTarget, float health) const {
        return hasTarget ? 3 + static_cast<int32_t>(health - 1.0f) : 3;
    }

    // Record drop when killed by skeleton
    static int32_t getRecordDrop(int32_t randInt) {
        return RECORD_13_ID + (randInt % (RECORD_WAIT_ID - RECORD_13_ID + 1));
    }

    // Sounds
    static constexpr const char* PRIME_SOUND = "creeper.primed";
    static constexpr const char* HURT_SOUND = "mob.creeper.say";
    static constexpr const char* DEATH_SOUND = "mob.creeper.death";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntitySkeleton — Ranged undead mob with wither variant.
// Java: net.minecraft.entity.monster.EntitySkeleton (300 lines)
//
//   Size: 0.6×1.8 (wither: 0.72×2.34)
//   Attributes: moveSpeed 0.25
//   Type 0 (regular): bow, ranged attack
//     Arrow: speed 1.6, inaccuracy = 14 - difficulty*4
//     Damage: pullStrength * 2 + Gaussian*0.25 + difficulty*0.11
//     Power enchant bonus: +level*0.5 + 0.5
//     Punch knockback
//     Flame enchant or wither skeleton → fire 100 ticks
//   Type 1 (wither): stone_sword, melee, fire immune
//     Size 0.72×2.34, attackDamage 4
//     Wither effect on hit: 200 ticks
//     Drops coal, rare: wither_skull
//   Sunlight burning: same as zombie (brightness>0.5, helmets absorb)
//   Flee sun AI, restrict sun
//   Nether spawn: 80% chance wither skeleton
//   Drops: type 0: arrows+bones, type 1: coal+bones
//   Snipe achievement: player kills skeleton with arrow at ≥50 blocks
//   Halloween: pumpkin head same as zombie
//   DataWatcher: 13 (skeletonType byte)
// ═══════════════════════════════════════════════════════════════════════════

class EntitySkeleton {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    int32_t skeletonType = 0; // 0 = regular, 1 = wither

    // ─── Constants ───
    static constexpr float WIDTH_NORMAL = 0.6f;
    static constexpr float HEIGHT_NORMAL = 1.8f;
    static constexpr float WIDTH_WITHER = 0.72f;
    static constexpr float HEIGHT_WITHER = 2.34f;
    static constexpr double MOVE_SPEED = 0.25;
    static constexpr double WITHER_ATTACK_DAMAGE = 4.0;

    // Ranged attack
    static constexpr float ARROW_SPEED = 1.6f;
    static constexpr int32_t BASE_INACCURACY = 14;
    static constexpr int32_t INACCURACY_PER_DIFF = 4;
    static constexpr double DIFFICULTY_DAMAGE_BONUS = 0.11;
    static constexpr int32_t WITHER_EFFECT_DURATION = 200;
    static constexpr int32_t FLAME_FIRE_TICKS = 100;

    // Snipe achievement distance
    static constexpr double SNIPE_DISTANCE_SQ = 2500.0;

    // Drops
    static constexpr int32_t DROP_ARROW = 262;
    static constexpr int32_t DROP_BONE = 352;
    static constexpr int32_t DROP_COAL = 263;
    static constexpr int32_t RARE_WITHER_SKULL_DAMAGE = 1;

    // ═══════════════════════════════════════════════════════════════════════
    // Ranged attack calculation
    // Java: EntitySkeleton.attackEntityWithRangedAttack
    //   - Arrow(world, this, target, 1.6f, 14 - difficulty*4)
    //   - damage = pullStrength * 2 + Gaussian*0.25 + difficulty*0.11
    //   - Power: + level*0.5 + 0.5
    //   - Punch: knockback
    //   - Flame or wither → fire 100
    // ═══════════════════════════════════════════════════════════════════════

    struct RangedAttackResult {
        float arrowSpeed;
        int32_t inaccuracy;
        double damage;
        int32_t knockback;
        bool setFire;
    };

    RangedAttackResult calculateRangedAttack(float pullStrength, int32_t difficultyId,
                                               double gaussian,
                                               int32_t powerLevel, int32_t punchLevel,
                                               int32_t flameLevel) const {
        RangedAttackResult result{};
        result.arrowSpeed = ARROW_SPEED;
        result.inaccuracy = BASE_INACCURACY - difficultyId * INACCURACY_PER_DIFF;

        result.damage = pullStrength * 2.0 + gaussian * 0.25 + difficultyId * DIFFICULTY_DAMAGE_BONUS;
        if (powerLevel > 0) result.damage += powerLevel * 0.5 + 0.5;
        result.knockback = punchLevel;
        result.setFire = flameLevel > 0 || skeletonType == 1;

        return result;
    }

    // Wither skeleton melee → wither effect
    bool shouldApplyWither() const { return skeletonType == 1; }

    // Nether spawn: 80% wither
    static bool shouldBeWitherSkeleton(int32_t rand5) { return rand5 > 0; } // rand(5) > 0 = 80%

    // Sunlight: same as zombie
    static bool checkSunlight(float brightness, bool isDaytime, bool canSeeSky,
                                bool hasHelmet, float randFloat) {
        if (!isDaytime) return false;
        if (brightness <= 0.5f) return false;
        if (randFloat * 30.0f >= (brightness - 0.4f) * 2.0f) return false;
        return canSeeSky && !hasHelmet;
    }

    // Sounds
    static constexpr const char* LIVING_SOUND = "mob.skeleton.say";
    static constexpr const char* HURT_SOUND = "mob.skeleton.hurt";
    static constexpr const char* DEATH_SOUND = "mob.skeleton.death";
    static constexpr const char* STEP_SOUND = "mob.skeleton.step";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntitySpider — Arthropod mob with wall climbing and leap attack.
// Java: net.minecraft.entity.monster.EntitySpider (170 lines)
//
//   Size: 1.4×0.9
//   Attributes: maxHealth 16, moveSpeed 0.8
//   Wall climbing: set climbing flag when collidedHorizontally (DW 16 bit 0)
//     isOnLadder() → isBesideClimbableBlock()
//   Hostile only in darkness: brightness < 0.5 → find player, else null
//     In light: 1% chance per tick to disengage target
//   Leap attack: distance 2-6, 10% chance per tick
//     motionX = (dx/dist)*0.5*0.8 + motionX*0.2
//     motionZ = (dz/dist)*0.5*0.8 + motionZ*0.2
//     motionY = 0.4
//   Immune to poison (Potion ID check)
//   Immune to webs (setInWeb is empty)
//   Creature attribute: ARTHROPOD
//   Drops: string, spider_eye (33% or looting bonus)
//   Spider jockey: 1% spawn with skeleton rider
//   Hard: 10% × tension → random potion effect (infinite duration)
//   DataWatcher: 16 (climbing byte, bit 0)
// ═══════════════════════════════════════════════════════════════════════════

class EntitySpider {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    bool isDead = false;
    bool onGround = false;
    bool isCollidedHorizontally = false;
    int32_t entityId = 0;

    bool besideClimbableBlock = false;
    int32_t spawnPotionEffect = 0; // 0 = none

    // ─── Constants ───
    static constexpr float WIDTH = 1.4f;
    static constexpr float HEIGHT = 0.9f;
    static constexpr double MAX_HEALTH = 16.0;
    static constexpr double MOVE_SPEED = 0.8;

    // Hostility
    static constexpr float HOSTILE_BRIGHTNESS_THRESHOLD = 0.5f;
    static constexpr int32_t DISENGAGE_CHANCE = 100; // 1/100

    // Leap attack
    static constexpr float LEAP_RANGE_MIN = 2.0f;
    static constexpr float LEAP_RANGE_MAX = 6.0f;
    static constexpr int32_t LEAP_CHANCE = 10; // 1/10
    static constexpr float LEAP_Y = 0.4f;
    static constexpr float LEAP_XZ_SPEED = 0.5f * 0.8f; // 0.4
    static constexpr float LEAP_CURRENT_BLEND = 0.2f;

    // Drops
    static constexpr int32_t DROP_STRING = 287;
    static constexpr int32_t DROP_SPIDER_EYE = 375;
    static constexpr int32_t SPIDER_EYE_CHANCE = 3; // 1/3

    // Spawn
    static constexpr int32_t JOCKEY_CHANCE = 100; // 1/100
    static constexpr float POTION_CHANCE_MULT = 0.1f;

    // ═══════════════════════════════════════════════════════════════════════
    // Wall climbing — set when collided horizontally.
    // ═══════════════════════════════════════════════════════════════════════

    void updateClimbing() { besideClimbableBlock = isCollidedHorizontally; }
    bool isOnLadder() const { return besideClimbableBlock; }

    // ═══════════════════════════════════════════════════════════════════════
    // Leap attack
    // Java: EntitySpider.attackEntity
    //   - Light > 0.5: 1% chance disengage
    //   - Distance 2-6, 10% chance, must be onGround
    //   - motionX = (dx/dist)*0.5*0.8 + motionX*0.2
    //   - motionZ = (dz/dist)*0.5*0.8 + motionZ*0.2
    //   - motionY = 0.4
    // ═══════════════════════════════════════════════════════════════════════

    struct LeapResult {
        bool shouldLeap;
        bool shouldDisengage;
        double newMotionX, newMotionZ;
        double newMotionY;
    };

    LeapResult tryLeapAttack(float distance, float brightness,
                                double targetX, double targetZ,
                                int32_t disengageRand, int32_t leapRand) const {
        LeapResult result{};

        // Disengage in daylight
        if (brightness > HOSTILE_BRIGHTNESS_THRESHOLD && disengageRand == 0) {
            result.shouldDisengage = true;
            return result;
        }

        // Leap
        if (distance > LEAP_RANGE_MIN && distance < LEAP_RANGE_MAX &&
            leapRand == 0 && onGround) {
            double dx = targetX - posX;
            double dz = targetZ - posZ;
            double dist = std::sqrt(dx * dx + dz * dz);
            if (dist > 1e-7) {
                result.shouldLeap = true;
                result.newMotionX = (dx / dist) * LEAP_XZ_SPEED + motionX * LEAP_CURRENT_BLEND;
                result.newMotionZ = (dz / dist) * LEAP_XZ_SPEED + motionZ * LEAP_CURRENT_BLEND;
                result.newMotionY = LEAP_Y;
            }
        }

        return result;
    }

    // Poison immunity
    static bool isPotionApplicable(int32_t potionId) {
        return potionId != 19; // Potion.poison.id = 19
    }

    // Web immunity
    void setInWeb() { /* no-op for spiders */ }

    // Sounds
    static constexpr const char* LIVING_SOUND = "mob.spider.say";
    static constexpr const char* HURT_SOUND = "mob.spider.say";
    static constexpr const char* DEATH_SOUND = "mob.spider.death";
    static constexpr const char* STEP_SOUND = "mob.spider.step";
};

} // namespace mccpp
