/**
 * EntityNetherMobs.h — Nether and dungeon mob implementations.
 *
 * Java references:
 *   - net.minecraft.entity.monster.EntityGhast (223 lines)
 *   - net.minecraft.entity.monster.EntityBlaze (163 lines)
 *   - net.minecraft.entity.monster.EntityWitch (188 lines)
 *   - net.minecraft.entity.monster.EntityMagmaCube (124 lines)
 *
 * Thread safety: Entity tick on single thread per entity.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <array>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityGhast — Flying Nether mob that shoots fireballs.
// Java: net.minecraft.entity.monster.EntityGhast (223 lines)
//
//   Size: 4.0×4.0, fire immune, XP 5
//   Attributes: maxHealth 10
//   Flight AI:
//     - Waypoint: random ±16 in each axis
//     - Course change cooldown: rand(5)+2 ticks
//     - Course traversable: AABB sweep test (step each block)
//     - Motion: += toWaypoint * 0.1 per valid step
//     - Re-pick waypoint if dist² < 1 or dist² > 3600
//   Target: closest player within 100 blocks, aggro cooldown 20 ticks
//   Fireball attack:
//     - Range: 64 blocks, must see target
//     - At counter 10: play aux SFX 1007 (charge sound)
//     - At counter 20: play aux SFX 1008, spawn LargeFireball
//       - Fireball spawned 4 blocks ahead along look vector
//       - Y = center + 0.5
//       - explosionStrength from NBT (default 1)
//     - After fire: counter = -40 (cooldown)
//     - No LOS: counter decrements
//   Reflected fireball: 1000 damage, player gets ghast achievement
//   Yaw: atan2(motionX, motionZ) when idle, atan2(dx, dz) when targeting
//   Sound volume: 10.0
//   Spawn: rand(20)==0, not peaceful, max 1 per chunk
//   Drops: ghast_tear (rand(2)+looting), gunpowder (rand(3)+looting)
//   DataWatcher: 16 (attacking byte: 1 when counter > 10)
//   NBT: "ExplosionPower" integer
// ═══════════════════════════════════════════════════════════════════════════

class EntityGhast {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0;
    bool isDead = false;
    int32_t entityId = 0;

    // Ghast state
    double waypointX = 0, waypointY = 0, waypointZ = 0;
    int32_t courseChangeCooldown = 0;
    int32_t aggroCooldown = 0;
    int32_t attackCounter = 0;
    int32_t prevAttackCounter = 0;
    int32_t explosionStrength = 1;

    // ─── Constants ───
    static constexpr float WIDTH = 4.0f;
    static constexpr float HEIGHT = 4.0f;
    static constexpr double MAX_HEALTH = 10.0;
    static constexpr int32_t XP_VALUE = 5;
    static constexpr float SOUND_VOLUME = 10.0f;
    static constexpr double TARGET_RANGE = 100.0;
    static constexpr double ATTACK_RANGE = 64.0;
    static constexpr int32_t AGGRO_COOLDOWN = 20;
    static constexpr int32_t CHARGE_START = 10;
    static constexpr int32_t FIRE_AT = 20;
    static constexpr int32_t POST_FIRE_COOLDOWN = -40;
    static constexpr double WAYPOINT_RANGE = 16.0;
    static constexpr double WAYPOINT_MIN_DIST_SQ = 1.0;
    static constexpr double WAYPOINT_MAX_DIST_SQ = 3600.0;
    static constexpr double FLIGHT_ACCEL = 0.1;
    static constexpr double FIREBALL_OFFSET = 4.0;
    static constexpr float REFLECTED_DAMAGE = 1000.0f;
    static constexpr int32_t SPAWN_CHANCE = 20;
    static constexpr int32_t MAX_PER_CHUNK = 1;

    // Items
    static constexpr int32_t DROP_GHAST_TEAR = 370;
    static constexpr int32_t DROP_GUNPOWDER = 289;

    // ─── Flight AI ───
    struct FlightResult {
        bool pickNewWaypoint;
        double accelX, accelY, accelZ;
        bool courseBlocked;
    };

    FlightResult tickFlight(double randFloatX, double randFloatY, double randFloatZ,
                              int32_t randCooldown, bool courseTraversable) {
        FlightResult result{};

        double dx = waypointX - posX;
        double dy = waypointY - posY;
        double dz = waypointZ - posZ;
        double distSq = dx*dx + dy*dy + dz*dz;

        if (distSq < WAYPOINT_MIN_DIST_SQ || distSq > WAYPOINT_MAX_DIST_SQ) {
            waypointX = posX + (randFloatX * 2.0 - 1.0) * WAYPOINT_RANGE;
            waypointY = posY + (randFloatY * 2.0 - 1.0) * WAYPOINT_RANGE;
            waypointZ = posZ + (randFloatZ * 2.0 - 1.0) * WAYPOINT_RANGE;
            result.pickNewWaypoint = true;
        }

        if (--courseChangeCooldown <= 0) {
            courseChangeCooldown += randCooldown + 2;
            double dist = std::sqrt(distSq);
            if (dist > 1e-7 && courseTraversable) {
                result.accelX = dx / dist * FLIGHT_ACCEL;
                result.accelY = dy / dist * FLIGHT_ACCEL;
                result.accelZ = dz / dist * FLIGHT_ACCEL;
            } else {
                waypointX = posX;
                waypointY = posY;
                waypointZ = posZ;
                result.courseBlocked = true;
            }
        }

        return result;
    }

    // ─── Attack state machine ───
    struct AttackResult {
        bool playChargeSound;   // SFX 1007
        bool shootFireball;     // SFX 1008 + spawn
        double fireballDX, fireballDY, fireballDZ;
        double fireballSpawnX, fireballSpawnY, fireballSpawnZ;
    };

    AttackResult tickAttack(bool hasTarget, bool canSee, double targetX, double targetY,
                              double targetZ, float selfHeight, double lookX, double lookZ) {
        AttackResult result{};
        prevAttackCounter = attackCounter;

        if (!hasTarget) {
            if (attackCounter > 0) --attackCounter;
            return result;
        }

        double distSq = (targetX - posX) * (targetX - posX) +
                         (targetY - posY) * (targetY - posY) +
                         (targetZ - posZ) * (targetZ - posZ);

        if (distSq >= ATTACK_RANGE * ATTACK_RANGE) {
            if (attackCounter > 0) --attackCounter;
            return result;
        }

        double dx = targetX - posX;
        double dy = targetY - (posY + selfHeight / 2.0);
        double dz = targetZ - posZ;

        if (canSee) {
            if (attackCounter == CHARGE_START) result.playChargeSound = true;
            ++attackCounter;
            if (attackCounter == FIRE_AT) {
                result.shootFireball = true;
                result.fireballDX = dx;
                result.fireballDY = dy;
                result.fireballDZ = dz;
                result.fireballSpawnX = posX + lookX * FIREBALL_OFFSET;
                result.fireballSpawnY = posY + selfHeight / 2.0 + 0.5;
                result.fireballSpawnZ = posZ + lookZ * FIREBALL_OFFSET;
                attackCounter = POST_FIRE_COOLDOWN;
            }
        } else {
            if (attackCounter > 0) --attackCounter;
        }

        return result;
    }

    // DataWatcher byte: 1 when counter > 10
    uint8_t getAttackingByte() const { return attackCounter > CHARGE_START ? 1 : 0; }

    struct DropResult {
        int32_t ghastTearCount;
        int32_t gunpowderCount;
    };

    static DropResult calculateDrops(int32_t rand2a, int32_t lootA,
                                       int32_t rand3b, int32_t lootB) {
        return {rand2a + lootA, rand3b + lootB};
    }

    static constexpr const char* LIVING_SOUND = "mob.ghast.moan";
    static constexpr const char* HURT_SOUND = "mob.ghast.scream";
    static constexpr const char* DEATH_SOUND = "mob.ghast.death";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityBlaze — Nether fortress mob with fireball volley.
// Java: net.minecraft.entity.monster.EntityBlaze (163 lines)
//
//   Fire immune, XP 10, attackDamage 6
//   Height offset: 0.5 + Gaussian*3, updates every 100 ticks
//   Flight: motionY += (0.3 - motionY) * 0.3 when below target
//   Slow fall: motionY *= 0.6 when falling
//   No fall damage
//   Water: 1 damage (drown)
//   Fire sound: 1/24 chance, offset +0.5 XYZ
//   Smoke: 2 largesmoke particles per tick
//   Melee: attackTime<=0 && dist<2 && overlapping AABB
//   Ranged attack cycle (dist < 30):
//     1st: charge 60 ticks, set fire mode ON
//     2nd-4th: fire small fireballs every 6 ticks
//     5th+: rest 100 ticks, set fire mode OFF
//     Fireball aim: Gaussian * sqrt(dist)*0.5 in X/Z
//     Fireball Y: center + 0.5
//   Always valid light for spawning
//   Drops: blaze_rod (only player kill, rand(2+looting))
//   DataWatcher: 16 (fire mode byte, bit 0)
//   Brightness: always 1.0
// ═══════════════════════════════════════════════════════════════════════════

class EntityBlaze {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionY = 0;
    bool isDead = false;
    bool onGround = false;
    int32_t entityId = 0;

    // Blaze state
    float heightOffset = 0.5f;
    int32_t heightOffsetUpdateTime = 0;
    int32_t attackTime = 0;
    int32_t volleyCount = 0;
    bool fireMode = false;

    // ─── Constants ───
    static constexpr double ATTACK_DAMAGE = 6.0;
    static constexpr int32_t XP_VALUE = 10;
    static constexpr float SLOW_FALL_MULT = 0.6f;
    static constexpr int32_t HEIGHT_UPDATE_INTERVAL = 100;
    static constexpr float HEIGHT_OFFSET_BASE = 0.5f;
    static constexpr float HEIGHT_GAUSSIAN_MULT = 3.0f;
    static constexpr double HOVER_ACCEL = 0.3;
    static constexpr double HOVER_BLEND = 0.3;
    static constexpr float MELEE_RANGE = 2.0f;
    static constexpr float RANGED_RANGE = 30.0f;
    static constexpr int32_t CHARGE_DELAY = 60;
    static constexpr int32_t VOLLEY_DELAY = 6;
    static constexpr int32_t REST_DELAY = 100;
    static constexpr int32_t MAX_VOLLEYS = 4;
    static constexpr int32_t FIRE_SOUND_CHANCE = 24;
    static constexpr float WATER_DAMAGE = 1.0f;
    static constexpr int32_t DROP_BLAZE_ROD = 369;

    // ─── Height offset update ───
    void tickHeightOffset(double gaussian) {
        if (--heightOffsetUpdateTime <= 0) {
            heightOffsetUpdateTime = HEIGHT_UPDATE_INTERVAL;
            heightOffset = HEIGHT_OFFSET_BASE + static_cast<float>(gaussian) * HEIGHT_GAUSSIAN_MULT;
        }
    }

    // ─── Hover toward target ───
    void hoverTowardTarget(double targetEyeY, double selfY, float selfEyeHeight) {
        if (targetEyeY > selfY + selfEyeHeight + heightOffset) {
            motionY += (HOVER_ACCEL - motionY) * HOVER_BLEND;
        }
    }

    // ─── Slow fall ───
    void applySlowFall() {
        if (!onGround && motionY < 0) motionY *= SLOW_FALL_MULT;
    }

    // No fall damage
    void fall(float /*dist*/) { /* no-op */ }

    // ─── Ranged attack cycle ───
    struct AttackTickResult {
        bool doMelee;
        bool fireProjectile;
        bool setFireMode;
        bool clearFireMode;
        double projectileDX, projectileDY, projectileDZ;
        double projectileSpawnY;
    };

    AttackTickResult tickAttack(float distToTarget, bool targetAbove, bool targetBelow,
                                  double targetX, double targetY, double targetZ,
                                  float selfHeight, double gaussianX, double gaussianZ) {
        AttackTickResult result{};

        // Melee
        if (attackTime <= 0 && distToTarget < MELEE_RANGE && targetAbove && targetBelow) {
            attackTime = 20;
            result.doMelee = true;
            return result;
        }

        // Ranged
        if (distToTarget < RANGED_RANGE && attackTime == 0) {
            ++volleyCount;
            if (volleyCount == 1) {
                attackTime = CHARGE_DELAY;
                result.setFireMode = true;
            } else if (volleyCount <= MAX_VOLLEYS) {
                attackTime = VOLLEY_DELAY;
            } else {
                attackTime = REST_DELAY;
                volleyCount = 0;
                result.clearFireMode = true;
            }

            if (volleyCount > 1) {
                float scatter = std::sqrt(distToTarget) * 0.5f;
                double dx = targetX - posX;
                double dy = targetY + selfHeight / 2.0 - (posY + selfHeight / 2.0);
                double dz = targetZ - posZ;
                result.fireProjectile = true;
                result.projectileDX = dx + gaussianX * scatter;
                result.projectileDY = dy;
                result.projectileDZ = dz + gaussianZ * scatter;
                result.projectileSpawnY = posY + selfHeight / 2.0 + 0.5;
            }
        }

        if (attackTime > 0) --attackTime;
        return result;
    }

    // Brightness always 1.0
    static constexpr float getBrightness() { return 1.0f; }
    static constexpr bool isValidLightLevel() { return true; }

    static constexpr const char* LIVING_SOUND = "mob.blaze.breathe";
    static constexpr const char* HURT_SOUND = "mob.blaze.hit";
    static constexpr const char* DEATH_SOUND = "mob.blaze.death";
    static constexpr const char* FIRE_AMBIENT = "fire.fire";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityWitch — Potion-using hostile mob.
// Java: net.minecraft.entity.monster.EntityWitch (188 lines)
//
//   Attributes: maxHealth 26, moveSpeed 0.25
//   Self-potions (priority order, 15% chance each except heal 5%):
//     1. Water breathing (8237) if underwater and no effect
//     2. Fire resistance (16307) if burning and no effect
//     3. Healing (16341) if hurt (5% chance)
//     4. Speed (16274) if target > 11 blocks and no effect (25% chance)
//   Drinking: -0.25 speed modifier, hold potion in hand
//     Timer = potion.maxUseDuration, aggressive=true
//     Effects applied when timer expires
//   Attack potions (thrown):
//     Default: harming (32732)
//     > 8 blocks and no slowness: slowness (32698)
//     ≥ 8 HP and no poison: poison (32660)
//     ≤ 3 blocks and no weakness: weakness 25% (32696)
//     Speed: 0.75, inaccuracy: 8.0, Y offset: dist*0.2
//   Damage reduction: self-damage → 0, magic → ×0.15
//   Drops: 1-3 random items from 8-type pool (glowstone, sugar, redstone,
//     spider_eye, glass_bottle, gunpowder, stick, stick)
//   Nose wiggle: 0.075% per tick (entity state 15)
//   DataWatcher: 21 (aggressive byte)
//   AI: swim, ranged(1.0,60,10), wander(1.0), watch(player,8), lookIdle,
//       hurtBy, nearestTarget(player)
// ═══════════════════════════════════════════════════════════════════════════

class EntityWitch {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    // Witch state
    bool aggressive = false;
    int32_t witchAttackTimer = 0;
    int32_t heldPotionDamage = -1;

    // ─── Constants ───
    static constexpr double MAX_HEALTH = 26.0;
    static constexpr double MOVE_SPEED = 0.25;
    static constexpr double SPEED_PENALTY = -0.25;
    static constexpr float SELF_DAMAGE_MULT = 0.0f;
    static constexpr float MAGIC_DAMAGE_MULT = 0.15f;
    static constexpr float NOSE_WIGGLE_CHANCE = 7.5e-4f;
    static constexpr float RANGED_SPEED = 0.75f;
    static constexpr float RANGED_INACCURACY = 8.0f;
    static constexpr int32_t RANGED_COOLDOWN = 60;
    static constexpr float RANGED_RANGE = 10.0f;

    // Self-potion IDs
    static constexpr int32_t POTION_WATER_BREATHING = 8237;
    static constexpr int32_t POTION_FIRE_RESISTANCE = 16307;
    static constexpr int32_t POTION_HEALING = 16341;
    static constexpr int32_t POTION_SPEED = 16274;

    // Attack potion IDs
    static constexpr int32_t ATTACK_HARMING = 32732;
    static constexpr int32_t ATTACK_SLOWNESS = 32698;
    static constexpr int32_t ATTACK_POISON = 32660;
    static constexpr int32_t ATTACK_WEAKNESS = 32696;

    // Drops (8 items, stick appears twice for equal weighting)
    static constexpr std::array<int32_t, 8> WITCH_DROPS = {
        348,  // glowstone_dust
        353,  // sugar
        331,  // redstone
        375,  // spider_eye
        374,  // glass_bottle
        289,  // gunpowder
        280,  // stick
        280,  // stick
    };

    // ─── Self-potion selection ───
    // Java: EntityWitch.onLivingUpdate — priority-based potion drinking
    struct SelfPotionResult {
        bool shouldDrink;
        int32_t potionDamage;
    };

    static SelfPotionResult selectSelfPotion(bool inWater, bool onFire, bool hurt,
                                               bool hasTarget, double targetDistSq,
                                               bool hasWaterBreathing, bool hasFireResist,
                                               bool hasSpeed,
                                               float rand1, float rand2, float rand3,
                                               float rand4) {
        SelfPotionResult result{};
        if (rand1 < 0.15f && inWater && !hasWaterBreathing) {
            result.shouldDrink = true;
            result.potionDamage = POTION_WATER_BREATHING;
        } else if (rand2 < 0.15f && onFire && !hasFireResist) {
            result.shouldDrink = true;
            result.potionDamage = POTION_FIRE_RESISTANCE;
        } else if (rand3 < 0.05f && hurt) {
            result.shouldDrink = true;
            result.potionDamage = POTION_HEALING;
        } else if (rand4 < 0.25f && hasTarget && !hasSpeed && targetDistSq > 121.0) {
            result.shouldDrink = true;
            result.potionDamage = POTION_SPEED;
        }
        return result;
    }

    // ─── Attack potion selection ───
    static int32_t selectAttackPotion(float distToTarget, bool targetHasSlowness,
                                        bool targetHasPoison, bool targetHasWeakness,
                                        float targetHP, float randFloat) {
        if (distToTarget >= 8.0f && !targetHasSlowness) return ATTACK_SLOWNESS;
        if (targetHP >= 8.0f && !targetHasPoison) return ATTACK_POISON;
        if (distToTarget <= 3.0f && !targetHasWeakness && randFloat < 0.25f) return ATTACK_WEAKNESS;
        return ATTACK_HARMING;
    }

    // ─── Damage reduction ───
    static float reduceDamage(float damage, bool selfDamage, bool magicDamage) {
        if (selfDamage) return SELF_DAMAGE_MULT;
        if (magicDamage) return damage * MAGIC_DAMAGE_MULT;
        return damage;
    }

    static constexpr const char* LIVING_SOUND = "mob.witch.idle";
    static constexpr const char* HURT_SOUND = "mob.witch.hurt";
    static constexpr const char* DEATH_SOUND = "mob.witch.death";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityMagmaCube — Nether variant of Slime.
// Java: net.minecraft.entity.monster.EntityMagmaCube (124 lines)
//
//   Extends EntitySlime (see EntitySpecialMobs.h)
//   Fire immune, never displays burning
//   moveSpeed: 0.2
//   Armor: 3 × size
//   Brightness: always 1.0
//   Particle: "flame" (instead of "slime")
//   Drops: magma_cream (only size > 1, count = rand(4)-2+looting)
//   Jump: motionY = 0.42 + size * 0.1
//   Jump delay: base × 4
//   Squish decay: 0.9 (vs 0.6 for regular slime)
//   canDamagePlayer: always true (size 1 can also damage)
//   Attack damage: size + 2
//   No fall damage
//   Sound: "mob.magmacube.big" / "mob.magmacube.small"
//   Spawn: not peaceful, no collision, no liquid
//   makesSoundOnLand: always true
// ═══════════════════════════════════════════════════════════════════════════

class EntityMagmaCube {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    bool onGround = false;
    int32_t entityId = 0;
    int32_t slimeSize = 1;

    // ─── Constants ───
    static constexpr double MOVE_SPEED = 0.2;
    static constexpr float SQUISH_DECAY = 0.9f;
    static constexpr int32_t JUMP_DELAY_MULT = 4;
    static constexpr float JUMP_BASE_Y = 0.42f;
    static constexpr float JUMP_SIZE_BONUS = 0.1f;
    static constexpr int32_t ARMOR_PER_SIZE = 3;
    static constexpr int32_t ATTACK_BONUS = 2;
    static constexpr int32_t DROP_MAGMA_CREAM = 378;

    // Size-based calculations
    int32_t getArmorValue() const { return slimeSize * ARMOR_PER_SIZE; }
    int32_t getAttackDamage() const { return slimeSize + ATTACK_BONUS; }
    bool canDamagePlayer() const { return true; }  // always, unlike regular slime
    float getJumpY() const { return JUMP_BASE_Y + slimeSize * JUMP_SIZE_BONUS; }

    // Drops: only size > 1, rand(4) - 2 + looting
    struct DropResult {
        bool shouldDrop;
        int32_t count;
    };

    static DropResult calculateDrops(int32_t size, int32_t rand4, int32_t looting) {
        DropResult result{};
        if (size > 1) {
            int32_t count = rand4 - 2;
            if (looting > 0) count += looting;
            result.shouldDrop = count > 0;
            result.count = count > 0 ? count : 0;
        }
        return result;
    }

    // Always bright, never burning
    static constexpr float getBrightness() { return 1.0f; }
    static constexpr bool isBurning() { return false; }
    void fall(float /*dist*/) { /* no-op */ }
    static constexpr bool makesSoundOnLand() { return true; }

    const char* getSound() const {
        return slimeSize > 1 ? "mob.magmacube.big" : "mob.magmacube.small";
    }
};

} // namespace mccpp
