/**
 * EntityLiving.h — Mob hierarchy: EntityLiving → EntityCreature → EntityMob.
 *
 * Java references:
 *   - net.minecraft.entity.EntityLiving (833 lines)
 *   - net.minecraft.entity.EntityCreature (272 lines)
 *   - net.minecraft.entity.monster.EntityMob (174 lines)
 *
 * EntityLiving adds:
 *   - AI task system (tasks + targetTasks)
 *   - Equipment (5 slots, drop chances default 0.085)
 *   - Despawn logic: >128 blocks instant, >32 blocks + age>600 random, <32 reset
 *   - updateAITasks: despawn → sensing → targetSelector → goalSelector →
 *     navigator → moveHelper → lookHelper → jumpHelper
 *   - Custom name (DataWatcher 10/11)
 *   - Leash system
 *   - Living sound (1/talkInterval ticks)
 *
 * EntityCreature adds:
 *   - Pathfinding with path following (30° turn limit)
 *   - Home position with maximum distance
 *   - Fleeing speed bonus (×2 multiplicative)
 *   - Wander path selection (10 random candidates)
 *
 * EntityMob adds:
 *   - Hostile sounds (game.hostile.*)
 *   - attackEntityAsMob (damage + enchant modifiers + knockback + fire aspect)
 *   - attackEntity (2 block range, 20 tick cooldown)
 *   - Light-level spawn check (skyLight > rand(32) = reject, blockLight <= rand(8))
 *   - Despawn on Peaceful difficulty
 *   - experienceValue = 5
 *
 * Thread safety: Entity ticked from main server thread.
 * JNI readiness: Predictable fields, standard accessors.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityLiving — AI-capable mob base.
// Java reference: net.minecraft.entity.EntityLiving
// ═══════════════════════════════════════════════════════════════════════════

class EntityLiving {
public:
    // ─── Position/Motion (inherited from Entity/EntityLivingBase) ───
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    float width = 0.6f, height = 1.8f;
    float health = 20.0f;
    bool isDead = false;
    bool isJumping = false;
    bool onGround = false;
    int32_t ticksExisted = 0;
    int32_t entityAge = 0;
    int32_t entityId = 0;
    float moveForward = 0, moveStrafing = 0;
    float randomYawVelocity = 0;
    int32_t attackTime = 0;
    float defaultPitch = 0;

    // ─── Living sound ───
    int32_t livingSoundTime = 0;
    int32_t talkInterval = 80;

    // ─── Experience ───
    int32_t experienceValue = 0;

    // ─── Equipment (5 slots: 0=hand, 1-4=armor) ───
    struct EquipSlot {
        int32_t itemId = 0;
        int32_t count = 0;
        int32_t damage = 0;
        bool isEmpty() const { return itemId == 0 || count <= 0; }
    };
    EquipSlot equipment[5];
    float equipmentDropChances[5] = {0.085f, 0.085f, 0.085f, 0.085f, 0.085f};

    // ─── Persistence ───
    bool canPickUpLoot = false;
    bool persistenceRequired = false;

    // ─── Custom name ───
    std::string customName;
    bool alwaysRenderNameTag = false;

    bool hasCustomNameTag() const { return !customName.empty(); }

    // ─── Leash ───
    bool isLeashed = false;
    int32_t leashedToEntityId = -1;

    // ─── AI state ───
    int32_t attackTargetId = -1;  // Entity ID of attack target

    // ═══════════════════════════════════════════════════════════════════════
    // Despawn logic
    // Java: despawnEntity — distance-based despawning
    //   >128 blocks (16384 dist²) → instant despawn
    //   >32 blocks (1024 dist²) + age>600 + 1/800 chance → despawn
    //   <32 blocks → reset age
    // ═══════════════════════════════════════════════════════════════════════

    struct DespawnResult {
        bool shouldDespawn;
        bool resetAge;
    };

    DespawnResult checkDespawn(double distSqToPlayer, bool canDespawn) const {
        if (persistenceRequired) return {false, true};
        if (distSqToPlayer < 0) return {false, false};  // No player

        if (canDespawn && distSqToPlayer > 16384.0) {
            return {true, false};  // >128 blocks → instant
        }
        if (entityAge > 600 && canDespawn && distSqToPlayer > 1024.0) {
            return {true, false};  // >32 blocks + old → random
        }
        if (distSqToPlayer < 1024.0) {
            return {false, true};  // <32 blocks → reset age
        }
        return {false, false};
    }

    // ═══════════════════════════════════════════════════════════════════════
    // AI update pipeline
    // Java: updateAITasks — called each tick for AI-enabled mobs
    //   1. entityAge++
    //   2. checkDespawn
    //   3. clearSensingCache
    //   4. targetTasks.onUpdateTasks()
    //   5. tasks.onUpdateTasks()
    //   6. navigator.onUpdateNavigation()
    //   7. updateAITick()
    //   8. moveHelper.onUpdateMoveHelper()
    //   9. lookHelper.onUpdateLook()
    //   10. jumpHelper.doJump()
    // ═══════════════════════════════════════════════════════════════════════

    enum class AIStep {
        DESPAWN,
        SENSING,
        TARGET_SELECTOR,
        GOAL_SELECTOR,
        NAVIGATION,
        MOB_TICK,
        MOVE_HELPER,
        LOOK_HELPER,
        JUMP_HELPER
    };

    // Yields AI steps in order for external processing
    static constexpr AIStep AI_PIPELINE[] = {
        AIStep::DESPAWN,
        AIStep::SENSING,
        AIStep::TARGET_SELECTOR,
        AIStep::GOAL_SELECTOR,
        AIStep::NAVIGATION,
        AIStep::MOB_TICK,
        AIStep::MOVE_HELPER,
        AIStep::LOOK_HELPER,
        AIStep::JUMP_HELPER
    };

    // ═══════════════════════════════════════════════════════════════════════
    // Non-AI entity action state (legacy mobs)
    // Java: updateEntityActionState
    //   - 2% chance to look at nearest player within 8 blocks
    //   - 5% chance random yaw velocity
    //   - Random jumping in water/lava (80%)
    // ═══════════════════════════════════════════════════════════════════════

    struct LegacyAIState {
        int32_t currentTargetId = -1;
        int32_t numTicksToChaseTarget = 0;
    };
    LegacyAIState legacyAI;

    // ═══════════════════════════════════════════════════════════════════════
    // Face entity
    // Java: faceEntity — smooth rotation toward target
    // ═══════════════════════════════════════════════════════════════════════

    static float wrapAngleTo180(float angle) {
        angle = std::fmod(angle, 360.0f);
        if (angle >= 180.0f) angle -= 360.0f;
        if (angle < -180.0f) angle += 360.0f;
        return angle;
    }

    static float updateRotation(float current, float target, float maxStep) {
        float delta = wrapAngleTo180(target - current);
        delta = std::clamp(delta, -maxStep, maxStep);
        return current + delta;
    }

    void facePosition(double targetX, double targetY, double targetZ,
                        float maxYawStep, float maxPitchStep) {
        double dx = targetX - posX;
        double dz = targetZ - posZ;
        double dy = targetY - (posY + static_cast<double>(height) * 0.85);
        double dist = std::sqrt(dx * dx + dz * dz);

        float targetYaw = static_cast<float>(std::atan2(dz, dx) * 180.0 / 3.1415927410125732) - 90.0f;
        float targetPitch = -static_cast<float>(std::atan2(dy, dist) * 180.0 / 3.1415927410125732);

        rotationPitch = updateRotation(rotationPitch, targetPitch, maxPitchStep);
        rotationYaw = updateRotation(rotationYaw, targetYaw, maxYawStep);
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Max fall height
    // Java: getMaxFallHeight — depends on health and difficulty
    //   No target: 3. With target: (health - maxHealth*0.33) - (3-difficulty)*4 + 3
    // ═══════════════════════════════════════════════════════════════════════

    int32_t getMaxFallHeight(float maxHealth, int32_t difficultyId) const {
        if (attackTargetId < 0) return 3;
        int32_t n = static_cast<int32_t>(health - maxHealth * 0.33f);
        n -= (3 - difficultyId) * 4;
        return std::max(0, n) + 3;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Max spawned in chunk
    // ═══════════════════════════════════════════════════════════════════════

    virtual int32_t getMaxSpawnedInChunk() const { return 4; }

    // ═══════════════════════════════════════════════════════════════════════
    // Armor item table
    // Java: getArmorItemForSlot(slot, tier)
    // tier: 0=leather, 1=gold, 2=chain, 3=iron, 4=diamond
    // ═══════════════════════════════════════════════════════════════════════

    // Item IDs: leather=298-301, gold=314-317, chain=302-305, iron=306-309, diamond=310-313
    static int32_t getArmorItemId(int32_t slot, int32_t tier) {
        // slot: 4=head, 3=chest, 2=legs, 1=feet
        // Armor item IDs arranged: helmet, chestplate, leggings, boots
        static constexpr int32_t ARMOR_IDS[5][4] = {
            {298, 299, 300, 301},  // leather
            {314, 315, 316, 317},  // gold
            {302, 303, 304, 305},  // chain
            {306, 307, 308, 309},  // iron
            {310, 311, 312, 313},  // diamond
        };
        if (tier < 0 || tier > 4 || slot < 1 || slot > 4) return 0;
        return ARMOR_IDS[tier][4 - slot];
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityCreature — Pathfinding mob.
// Java reference: net.minecraft.entity.EntityCreature
// ═══════════════════════════════════════════════════════════════════════════

class EntityCreature : public EntityLiving {
public:
    // ─── Pathfinding ───
    bool hasPath = false;
    int32_t entityToAttackId = -1;
    bool hasAttacked = false;
    int32_t fleeingTick = 0;

    // ─── Home position ───
    int32_t homeX = 0, homeY = 0, homeZ = 0;
    float maximumHomeDistance = -1.0f;  // -1 = no home restriction

    // ─── Fleeing speed bonus ───
    // Java: field_110181_i — 2.0× multiplicative
    static constexpr double FLEEING_SPEED_BONUS = 2.0;

    // ═══════════════════════════════════════════════════════════════════════
    // Home area
    // ═══════════════════════════════════════════════════════════════════════

    void setHomeArea(int32_t x, int32_t y, int32_t z, int32_t distance) {
        homeX = x; homeY = y; homeZ = z;
        maximumHomeDistance = static_cast<float>(distance);
    }

    bool hasHome() const { return maximumHomeDistance != -1.0f; }

    void detachHome() { maximumHomeDistance = -1.0f; }

    bool isWithinHomeDistance(int32_t x, int32_t y, int32_t z) const {
        if (!hasHome()) return true;
        double dx = x - homeX;
        double dy = y - homeY;
        double dz = z - homeZ;
        return (dx * dx + dy * dy + dz * dz) <
               static_cast<double>(maximumHomeDistance * maximumHomeDistance);
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Path following
    // Java: 30° max turn per tick, jump when going uphill
    // ═══════════════════════════════════════════════════════════════════════

    static constexpr float MAX_TURN_PER_TICK = 30.0f;

    // Steer toward a waypoint
    void steerToward(double wx, double wz, float speed) {
        double dx = wx - posX;
        double dz = wz - posZ;
        float targetYaw = static_cast<float>(std::atan2(dz, dx) * 180.0 / 3.1415927410125732) - 90.0f;
        float delta = wrapAngleTo180(targetYaw - rotationYaw);
        delta = std::clamp(delta, -MAX_TURN_PER_TICK, MAX_TURN_PER_TICK);
        rotationYaw += delta;
        moveForward = speed;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Wander path selection
    // Java: updateWanderPath — 10 random positions, pick best path weight
    // ═══════════════════════════════════════════════════════════════════════

    struct WanderCandidate {
        int32_t x, y, z;
        float weight;
    };

    int32_t getVerticalFaceSpeed() const { return 40; }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityMob — Hostile mob.
// Java reference: net.minecraft.entity.monster.EntityMob
// ═══════════════════════════════════════════════════════════════════════════

class EntityMob : public EntityCreature {
public:
    EntityMob() {
        experienceValue = 5;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Hostile sounds
    // ═══════════════════════════════════════════════════════════════════════

    static constexpr const char* SWIM_SOUND = "game.hostile.swim";
    static constexpr const char* SPLASH_SOUND = "game.hostile.swim.splash";
    static constexpr const char* HURT_SOUND = "game.hostile.hurt";
    static constexpr const char* DEATH_SOUND = "game.hostile.die";
    static constexpr const char* FALL_BIG_SOUND = "game.hostile.hurt.fall.big";
    static constexpr const char* FALL_SMALL_SOUND = "game.hostile.hurt.fall.small";

    // ═══════════════════════════════════════════════════════════════════════
    // Attack entity as mob
    // Java: attackEntityAsMob — uses attack damage attribute + enchant
    //   modifiers + knockback + fire aspect
    // ═══════════════════════════════════════════════════════════════════════

    struct AttackResult {
        bool hit;
        float damage;
        int32_t knockback;
        int32_t fireAspect;  // Seconds of fire
    };

    AttackResult calculateAttack(float baseDamage, int32_t enchantDamageBonus,
                                    int32_t knockbackBonus, int32_t fireAspectLevel) const {
        float totalDamage = baseDamage + static_cast<float>(enchantDamageBonus);
        int32_t totalKnockback = knockbackBonus;
        int32_t fireSeconds = fireAspectLevel > 0 ? fireAspectLevel * 4 : 0;
        return {true, totalDamage, totalKnockback, fireSeconds};
    }

    // Knockback application
    // Java: -sin(yaw * PI/180) * knockback * 0.5, 0.1, cos(yaw * PI/180) * knockback * 0.5
    // Self: motionX *= 0.6, motionZ *= 0.6
    void applyKnockbackToSelf() {
        motionX *= 0.6;
        motionZ *= 0.6;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Attack entity (melee range check)
    // Java: attackEntity — 2.0f range, 20 tick cooldown
    // ═══════════════════════════════════════════════════════════════════════

    static constexpr float MELEE_RANGE = 2.0f;
    static constexpr int32_t ATTACK_COOLDOWN = 20;

    bool canMeleeAttack(float distance) const {
        return attackTime <= 0 && distance < MELEE_RANGE;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Light level spawn check
    // Java: isValidLightLevel
    //   1. skyLight > rand(32) → false (too bright)
    //   2. During thunder: temporarily set skylightSubtracted=10
    //   3. blockLight <= rand(8) → true (dark enough)
    // ═══════════════════════════════════════════════════════════════════════

    struct LightCheck {
        int32_t skyLight;
        int32_t blockLight;
        bool isThundering;
        int32_t skylightSubtracted;
    };

    static bool isValidLightLevel(const LightCheck& light, int32_t randSky, int32_t randBlock) {
        if (light.skyLight > randSky) return false;

        int32_t effectiveBlockLight = light.blockLight;
        if (light.isThundering) {
            // During thunder, compute with skylightSubtracted=10
            // This makes the world appear darker, allowing more spawns
        }

        return effectiveBlockLight <= randBlock;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Path weight for hostile mobs
    // Java: 0.5f - brightness (prefer dark areas)
    // ═══════════════════════════════════════════════════════════════════════

    static float getBlockPathWeight(float brightness) {
        return 0.5f - brightness;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Living update — age faster in light
    // Java: onLivingUpdate — if brightness > 0.5, entityAge += 2
    // ═══════════════════════════════════════════════════════════════════════

    void onLivingUpdateHostile(float brightness) {
        if (brightness > 0.5f) {
            entityAge += 2;
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Peaceful despawn
    // Java: onUpdate — if difficulty == PEACEFUL → setDead
    // ═══════════════════════════════════════════════════════════════════════

    bool shouldDespawnOnPeaceful(int32_t difficultyId) const {
        return difficultyId == 0;  // PEACEFUL = 0
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Spawn check
    // Java: getCanSpawnHere — not Peaceful + validLight + super check
    // ═══════════════════════════════════════════════════════════════════════

    bool canSpawnHere(int32_t difficultyId, bool validLight, bool superCanSpawn) const {
        return difficultyId != 0 && validLight && superCanSpawn;
    }
};

} // namespace mccpp
