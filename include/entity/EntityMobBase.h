/**
 * EntityMobBase.h — Mob entity hierarchy: EntityCreature, EntityMob, EntityAnimal.
 *
 * Java references:
 *   - net.minecraft.entity.EntityCreature (272 lines)
 *   - net.minecraft.entity.monster.EntityMob (174 lines)
 *   - net.minecraft.entity.passive.EntityAnimal (285 lines)
 *
 * Hierarchy: Entity → EntityLivingBase → EntityLiving → EntityCreature
 *                                                       → EntityMob (hostile)
 *                                                       → EntityAgeable → EntityAnimal (passive)
 *
 * EntityCreature:
 *   - Home position system: homeX/Y/Z, maximumHomeDistance (-1 = no home)
 *   - Fleeing: fleeingTick, speed bonus UUID E199AD21 (+2.0 multiply op)
 *   - Path following: 16-block search, 30° yaw clamp per tick, jump on Y>0
 *   - Wander: 10 random spots ±6/±3, pick best getBlockPathWeight
 *   - Leash: pull at >6 blocks (0.4*abs force), break at >10
 *
 * EntityMob:
 *   - XP value: 5
 *   - Peaceful difficulty → setDead
 *   - findPlayerToAttack: 16 blocks, line of sight
 *   - attackEntityAsMob: attribute damage + enchantment modifier,
 *     knockback (0.5 * level, +0.1 Y), fire aspect (*4 ticks)
 *   - attackEntity: range < 2 blocks, attackTime = 20 ticks cooldown
 *   - Light-based spawning: skyLight > rand(32) = no,
 *     blockLight <= rand(8) = yes, thunder skips sky check
 *   - Path weight: 0.5 - lightBrightness (prefer dark areas)
 *   - Sounds: game.hostile.{swim,hurt,die,hurt.fall.big/small}
 *
 * EntityAnimal:
 *   - Breeding: inLove = 600 ticks, breeding counter → procreate at 60,
 *     parents get 6000 tick cooldown, baby growingAge = -24000
 *   - Find player with breeding item in 8 blocks
 *   - Find mate: same class, both inLove, 8-block range
 *   - Flee when hurt: 60 ticks + speed bonus (+2.0 multiply)
 *   - Spawn: on grass, fullBlockLight > 8
 *   - canDespawn: false
 *   - XP: 1 + rand(3)
 *   - Default breeding item: wheat (ID 296)
 *   - Path weight: grass = 10.0, else lightBrightness - 0.5
 *   - Talk interval: 120 ticks
 *   - Heart particles: every 10 ticks while in love
 *
 * Thread safety: Entity tick on single thread per entity.
 * JNI readiness: Simple struct hierarchy with virtual-like type fields.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityCreature — Base for all creatures with pathfinding and home areas.
// Java: net.minecraft.entity.EntityCreature (272 lines)
// ═══════════════════════════════════════════════════════════════════════════

class EntityCreature {
public:
    // ─── Entity base (from EntityLiving) ───
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    float moveForward = 0, moveStrafing = 0;
    bool isDead = false;
    bool isJumping = false;
    bool isCollidedHorizontally = false;
    bool hasAttacked = false;
    float width = 0.6f, height = 1.8f;
    int32_t entityId = 0;
    int32_t entityAge = 0;
    int32_t attackTime = 0;

    // ─── Creature-specific ───
    int32_t targetEntityId = -1;
    int32_t fleeingTick = 0;

    // Home system
    int32_t homeX = 0, homeY = 0, homeZ = 0;
    float maximumHomeDistance = -1.0f; // -1 = no home

    // Fleeing speed bonus UUID: E199AD21-BA8A-4C53-8D13-6182D5C69D3A
    static constexpr double FLEE_SPEED_BONUS = 2.0; // multiply operation (op 2)

    // ─── Path following constants ───
    static constexpr float SEARCH_RANGE = 16.0f;
    static constexpr float YAW_CLAMP = 30.0f;
    static constexpr int32_t WANDER_ATTEMPTS = 10;
    static constexpr int32_t WANDER_XZ_RANGE = 6;
    static constexpr int32_t WANDER_Y_RANGE = 3;

    // ═══════════════════════════════════════════════════════════════════════
    // Home position
    // ═══════════════════════════════════════════════════════════════════════

    bool hasHome() const { return maximumHomeDistance != -1.0f; }

    void setHomeArea(int32_t x, int32_t y, int32_t z, int32_t dist) {
        homeX = x; homeY = y; homeZ = z;
        maximumHomeDistance = static_cast<float>(dist);
    }

    void detachHome() { maximumHomeDistance = -1.0f; }

    bool isWithinHomeDistance(int32_t x, int32_t y, int32_t z) const {
        if (!hasHome()) return true;
        double dx = x - homeX, dy = y - homeY, dz = z - homeZ;
        return (dx*dx + dy*dy + dz*dz) < maximumHomeDistance * maximumHomeDistance;
    }

    bool isWithinHomeDistanceCurrent() const {
        return isWithinHomeDistance(
            static_cast<int32_t>(posX),
            static_cast<int32_t>(posY),
            static_cast<int32_t>(posZ));
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Path following
    // Java: EntityCreature.updateEntityActionState
    //   - Face next waypoint with ±30° yaw clamp
    //   - moveForward = movementSpeed attribute
    //   - Jump when target Y > current Y or collided horizontally
    //   - Jump in water/lava with 80% chance
    //   - When hasAttacked && target: calculate strafing from angle difference
    // ═══════════════════════════════════════════════════════════════════════

    struct PathFollowResult {
        float targetYaw;
        bool shouldJump;
        float forward;
        float strafe;
    };

    PathFollowResult followPath(double wpX, double wpY, double wpZ,
                                  double targetX, double targetZ,
                                  float movementSpeed, bool inWater, bool inLava) const {
        PathFollowResult result{};
        result.forward = movementSpeed;

        // Calculate yaw to waypoint
        double dx = wpX - posX;
        double dz = wpZ - posZ;
        float targetYaw = static_cast<float>(std::atan2(dz, dx) * 180.0 / M_PI) - 90.0f;

        // Clamp yaw delta
        float yawDelta = targetYaw - rotationYaw;
        while (yawDelta > 180) yawDelta -= 360;
        while (yawDelta < -180) yawDelta += 360;
        if (yawDelta > YAW_CLAMP) yawDelta = YAW_CLAMP;
        if (yawDelta < -YAW_CLAMP) yawDelta = -YAW_CLAMP;

        result.targetYaw = rotationYaw + yawDelta;

        // Jump conditions
        int32_t currentY = static_cast<int32_t>(posY + 0.5); // bbMinY + 0.5
        double deltaY = wpY - currentY;
        if (deltaY > 0) result.shouldJump = true;

        // Strafing when attacking
        if (hasAttacked && targetEntityId >= 0) {
            double aDx = targetX - posX;
            double aDz = targetZ - posZ;
            float attackYaw = static_cast<float>(std::atan2(aDz, aDx) * 180.0 / M_PI) - 90.0f;
            float angleDiff = (result.targetYaw - attackYaw + 90.0f) * static_cast<float>(M_PI) / 180.0f;
            result.strafe = -std::sin(angleDiff) * result.forward;
            result.forward = std::cos(angleDiff) * result.forward;
        }

        // Jump in water/lava
        if (inWater || inLava) result.shouldJump = true;

        // Collided horizontally → jump
        if (isCollidedHorizontally) result.shouldJump = true;

        return result;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Leash system
    // Java: EntityCreature.updateLeashedState
    //   - Set home to leash holder position, radius 5
    //   - Distance > 4: navigate toward holder
    //   - Distance > 6: pull with force 0.4 * abs(component)
    //   - Distance > 10: break leash
    // ═══════════════════════════════════════════════════════════════════════

    struct LeashResult {
        bool shouldBreak;
        double pullX, pullY, pullZ;
        bool shouldNavigate;
    };

    LeashResult updateLeash(double holderX, double holderY, double holderZ) {
        LeashResult result{};
        setHomeArea(static_cast<int32_t>(holderX), static_cast<int32_t>(holderY),
                     static_cast<int32_t>(holderZ), 5);

        double dx = holderX - posX;
        double dy = holderY - posY;
        double dz = holderZ - posZ;
        float dist = static_cast<float>(std::sqrt(dx*dx + dy*dy + dz*dz));

        if (dist > 10.0f) { result.shouldBreak = true; return result; }
        if (dist > 4.0f) result.shouldNavigate = true;

        if (dist > 6.0f) {
            double normX = dx / dist, normY = dy / dist, normZ = dz / dist;
            result.pullX = normX * std::abs(normX) * 0.4;
            result.pullY = normY * std::abs(normY) * 0.4;
            result.pullZ = normZ * std::abs(normZ) * 0.4;
        }

        return result;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Spawning
    // Java: getCanSpawnHere → super + getBlockPathWeight >= 0
    // ═══════════════════════════════════════════════════════════════════════

    virtual float getBlockPathWeight(int32_t x, int32_t y, int32_t z) const {
        return 0.0f;
    }

    virtual ~EntityCreature() = default;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityMob — Base for hostile mobs.
// Java: net.minecraft.entity.monster.EntityMob (174 lines)
// ═══════════════════════════════════════════════════════════════════════════

class EntityMob : public EntityCreature {
public:
    int32_t experienceValue = 5;

    // ─── Constants ───
    static constexpr float FIND_PLAYER_RANGE = 16.0f;
    static constexpr float ATTACK_RANGE = 2.0f;
    static constexpr int32_t ATTACK_COOLDOWN = 20;
    static constexpr float KNOCKBACK_VELOCITY = 0.5f;
    static constexpr float KNOCKBACK_Y = 0.1f;

    // ═══════════════════════════════════════════════════════════════════════
    // onLivingUpdate — Age faster in bright areas (2× in >0.5 brightness)
    // ═══════════════════════════════════════════════════════════════════════

    void onLivingUpdate(float brightness) {
        if (brightness > 0.5f) entityAge += 2;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // attackEntityAsMob — Melee attack with enchantment support.
    // Java: EntityMob.attackEntityAsMob
    //   - Base damage from attackDamage attribute
    //   - + enchantment modifier for living targets
    //   - Knockback: -sin(yaw) * level * 0.5, +0.1 Y, cos(yaw) * level * 0.5
    //   - Self slows: motionX/Z *= 0.6
    //   - Fire aspect: fireLevel * 4 ticks
    // ═══════════════════════════════════════════════════════════════════════

    struct AttackResult {
        float totalDamage;
        int32_t knockbackLevel;
        int32_t fireAspectLevel;
        double knockbackX, knockbackZ;
        bool hit;
    };

    AttackResult attackEntityAsMob(float baseDamage, float enchantBonus,
                                      int32_t knockbackEnchant, int32_t fireAspect) const {
        AttackResult result{};
        result.totalDamage = baseDamage + enchantBonus;
        result.knockbackLevel = knockbackEnchant;
        result.fireAspectLevel = fireAspect;

        if (knockbackEnchant > 0) {
            float yawRad = rotationYaw * static_cast<float>(M_PI) / 180.0f;
            result.knockbackX = -std::sin(yawRad) * knockbackEnchant * KNOCKBACK_VELOCITY;
            result.knockbackZ = std::cos(yawRad) * knockbackEnchant * KNOCKBACK_VELOCITY;
        }

        result.hit = true;
        return result;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // attackEntity — Range check and cooldown.
    // Java: range < 2, attackTime <= 0, target Y overlap, cooldown = 20
    // ═══════════════════════════════════════════════════════════════════════

    bool canMeleeAttack(float distance, double targetBBMaxY, double targetBBMinY,
                          double selfBBMinY, double selfBBMaxY) const {
        return attackTime <= 0 &&
               distance < ATTACK_RANGE &&
               targetBBMaxY > selfBBMinY &&
               targetBBMinY < selfBBMaxY;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Light-based spawning
    // Java: isValidLightLevel
    //   - Sky light saved value > rand(32) → TOO BRIGHT, no spawn
    //   - Block light value <= rand(8) → DARK ENOUGH, can spawn
    //   - Thunder: temporarily set skylightSubtracted = 10 for light check
    // ═══════════════════════════════════════════════════════════════════════

    struct LightCheck {
        bool valid;
    };

    static LightCheck isValidLightLevel(int32_t skyLight, int32_t blockLight,
                                          bool isThundering, int32_t blockLightThunder,
                                          int32_t rand32, int32_t rand8) {
        if (skyLight > rand32) return {false};
        int32_t effectiveBlock = isThundering ? blockLightThunder : blockLight;
        return {effectiveBlock <= rand8};
    }

    // Path weight: 0.5 - lightBrightness (prefer dark)
    float getBlockPathWeight(int32_t x, int32_t y, int32_t z) const override {
        return 0.5f; // Actual: 0.5 - world.getLightBrightness(); caller provides
    }

    // Sounds
    static constexpr const char* SWIM_SOUND = "game.hostile.swim";
    static constexpr const char* SPLASH_SOUND = "game.hostile.swim.splash";
    static constexpr const char* HURT_SOUND = "game.hostile.hurt";
    static constexpr const char* DEATH_SOUND = "game.hostile.die";
    static constexpr const char* FALL_BIG = "game.hostile.hurt.fall.big";
    static constexpr const char* FALL_SMALL = "game.hostile.hurt.fall.small";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityAnimal — Base for passive animals with breeding.
// Java: net.minecraft.entity.passive.EntityAnimal (285 lines)
// ═══════════════════════════════════════════════════════════════════════════

class EntityAnimal : public EntityCreature {
public:
    // ─── Breeding system ───
    int32_t inLove = 0;
    int32_t breeding = 0;
    int32_t growingAge = 0;
    int32_t playerInLoveId = -1; // Entity ID of player who fed
    int32_t experienceValue = 1; // + rand(3) by caller

    // ─── Constants ───
    static constexpr int32_t LOVE_DURATION = 600;      // Ticks in love mode
    static constexpr int32_t BREEDING_THRESHOLD = 60;   // breeding counter to procreate
    static constexpr int32_t PARENT_COOLDOWN = 6000;    // Ticks before can breed again
    static constexpr int32_t BABY_AGE = -24000;          // Ticks until baby grows up (20 minutes)
    static constexpr float FLEE_DURATION = 60;           // Flee ticks when hurt
    static constexpr float FIND_RANGE = 8.0f;
    static constexpr int32_t TALK_INTERVAL = 120;
    static constexpr int32_t WHEAT_ID = 296;             // Default breeding item
    static constexpr int32_t GRASS_BLOCK_ID = 2;         // Blocks.grass
    static constexpr int32_t MIN_SPAWN_LIGHT = 8;
    static constexpr float GRASS_PATH_WEIGHT = 10.0f;

    // ═══════════════════════════════════════════════════════════════════════
    // Breeding
    // ═══════════════════════════════════════════════════════════════════════

    void setInLove(int32_t playerEntityId) {
        inLove = LOVE_DURATION;
        playerInLoveId = playerEntityId;
        targetEntityId = -1;
    }

    bool isInLove() const { return inLove > 0; }
    void resetInLove() { inLove = 0; }
    bool canDespawn() const { return false; }

    bool canMateWith(int32_t otherInLove, int32_t otherEntityId, bool sameClass) const {
        if (otherEntityId == entityId) return false;
        if (!sameClass) return false;
        return isInLove() && otherInLove > 0;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // onLivingUpdate — Love particles and breeding counter.
    // Java: EntityAnimal.onLivingUpdate
    //   - Reset inLove if growingAge != 0
    //   - Heart particles every 10 ticks while inLove
    //   - Reset breeding when not in love
    // ═══════════════════════════════════════════════════════════════════════

    struct LoveTickResult {
        bool showHeart;
        bool resetBreeding;
    };

    LoveTickResult tickLove() {
        LoveTickResult result{};

        if (growingAge != 0) inLove = 0;

        if (inLove > 0) {
            --inLove;
            if (inLove % 10 == 0) result.showHeart = true;
        } else {
            breeding = 0;
            result.resetBreeding = true;
        }

        return result;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Breeding interaction
    // Java: EntityAnimal.attackEntity
    //   - Two animals both inLove and within 3.5 blocks:
    //     ++inLove, ++breeding on both
    //     Heart particle every 4th breeding tick
    //     Procreate at breeding == 60
    //   - Parent: face child within 2.5 blocks
    // ═══════════════════════════════════════════════════════════════════════

    struct BreedingResult {
        bool showHeart;
        bool procreate;
        bool resetBreeding;
    };

    BreedingResult tickBreeding(float distance, int32_t& otherInLove, int32_t& otherBreeding,
                                  int32_t otherGrowingAge) {
        BreedingResult result{};

        // Both in love and close enough
        if (inLove > 0 && otherInLove > 0) {
            if (distance < 3.5f) {
                ++otherInLove;
                ++inLove;
                ++breeding;
                if (breeding % 4 == 0) result.showHeart = true;
                if (breeding == BREEDING_THRESHOLD) {
                    result.procreate = true;
                }
            } else {
                breeding = 0;
                result.resetBreeding = true;
            }
        } else {
            breeding = 0;
            result.resetBreeding = true;
        }

        return result;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Procreation — reset parents, set baby age.
    // Java: EntityAnimal.procreate
    //   - Both parents: growingAge = 6000, inLove = 0, breeding = 0
    //   - Baby: growingAge = -24000
    //   - 7 heart particles at parent position
    // ═══════════════════════════════════════════════════════════════════════

    void resetAfterProcreation() {
        growingAge = PARENT_COOLDOWN;
        inLove = 0;
        breeding = 0;
        targetEntityId = -1;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Flee when hurt
    // Java: EntityAnimal.attackEntityFrom
    //   - fleeingTick = 60
    //   - Apply flee speed bonus (+2.0 multiply) if not AI-enabled
    //   - Reset inLove and target
    // ═══════════════════════════════════════════════════════════════════════

    void onHurt() {
        fleeingTick = static_cast<int32_t>(FLEE_DURATION);
        targetEntityId = -1;
        inLove = 0;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Spawning
    // Java: EntityAnimal.getCanSpawnHere
    //   - Block below must be grass
    //   - Full block light value > 8
    // ═══════════════════════════════════════════════════════════════════════

    static bool canSpawnHere(int32_t blockBelowId, int32_t lightLevel) {
        return blockBelowId == GRASS_BLOCK_ID && lightLevel > MIN_SPAWN_LIGHT;
    }

    // Path weight: grass = 10.0, else lightBrightness - 0.5
    float getBlockPathWeight(int32_t x, int32_t y, int32_t z) const override {
        return GRASS_PATH_WEIGHT; // Actual: check block at y-1, caller provides
    }

    // Sounds
    static constexpr const char* SWIM_SOUND = "game.neutral.swim";
    static constexpr const char* SPLASH_SOUND = "game.neutral.swim.splash";
    static constexpr const char* HURT_SOUND = "game.neutral.hurt";
    static constexpr const char* DEATH_SOUND = "game.neutral.die";
    static constexpr const char* FALL_BIG = "game.neutral.hurt.fall.big";
    static constexpr const char* FALL_SMALL = "game.neutral.hurt.fall.small";
};

} // namespace mccpp
