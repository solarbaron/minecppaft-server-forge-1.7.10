/**
 * EntitySpecialMobs.h — Enderman, Wolf, Slime, PigZombie implementations.
 *
 * Java references:
 *   - net.minecraft.entity.monster.EntityEnderman (343 lines)
 *   - net.minecraft.entity.passive.EntityWolf (423 lines)
 *   - net.minecraft.entity.monster.EntitySlime (242 lines)
 *   - net.minecraft.entity.monster.EntityPigZombie (165 lines)
 *
 * Thread safety: Entity tick on single thread per entity.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <array>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityEnderman — Teleporting mob with block carrying.
// Java: net.minecraft.entity.monster.EntityEnderman (343 lines)
//
//   Size: 0.6×2.9, stepHeight 1.0
//   Attributes: maxHealth 40, moveSpeed 0.3, attackDamage 7
//   Stare mechanic:
//     - Find closest player within 64 blocks
//     - Pumpkin head → immune
//     - Dot product: look.dot(toEnderman) > 1 - 0.025/dist AND canSee
//     - stareTimer 0→5: plays "mob.endermen.stare" at 0, attacks at 5
//   Speed boost: +6.2 additive when attacking (UUID 020E0DFB...)
//   Teleport:
//     Random: ±32 X/Z, ±32 Y (int), validates solid below + no collision + no liquid
//     To entity: 16 blocks away along vector, ±4 X/Z ±8 Y offset
//     128 portal particles on success, "mob.endermen.portal" sound
//   Block carrying: 14 carriable blocks, 5% pick (rand 20), 0.05% place (rand 2000)
//     Carriable: grass, dirt, sand, gravel, dandelion, poppy, brown_mushroom,
//                red_mushroom, tnt, cactus, clay, pumpkin, melon, mycelium
//   Water: 1 damage (drown), flee + clear target
//   Sunlight: flee + clear target (same brightness check as zombie)
//   Projectile: dodge with 64 teleport attempts, return true
//   Screaming: DataWatcher 18 byte, 1% auto-clear when idle
//   Drops: ender_pearl (rand(2 + looting))
//   DataWatcher: 16 (carried block ID byte), 17 (carried data byte), 18 (screaming byte)
// ═══════════════════════════════════════════════════════════════════════════

class EntityEnderman {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    // Enderman state
    int32_t carriedBlockId = 0;
    int32_t carriedData = 0;
    bool screaming = false;
    bool isAggressive = false;
    int32_t stareTimer = 0;
    int32_t teleportDelay = 0;

    // ─── Constants ───
    static constexpr float WIDTH = 0.6f;
    static constexpr float HEIGHT = 2.9f;
    static constexpr float STEP_HEIGHT = 1.0f;
    static constexpr double MAX_HEALTH = 40.0;
    static constexpr double MOVE_SPEED = 0.3;
    static constexpr double ATTACK_DAMAGE = 7.0;
    static constexpr double SPEED_BOOST = 6.2;
    static constexpr double STARE_RANGE = 64.0;
    static constexpr int32_t STARE_THRESHOLD = 5;
    static constexpr double STARE_DOT_BASE = 1.0;
    static constexpr double STARE_DOT_DIVISOR = 0.025;
    static constexpr int32_t TELEPORT_PARTICLES = 128;
    static constexpr double TELEPORT_RANDOM_RANGE = 64.0; // ±32
    static constexpr int32_t TELEPORT_RANDOM_Y_RANGE = 64; // ±32
    static constexpr double TELEPORT_ENTITY_DISTANCE = 16.0;
    static constexpr double TELEPORT_ENTITY_OFFSET_XZ = 8.0;
    static constexpr int32_t TELEPORT_ENTITY_OFFSET_Y = 16;
    static constexpr int32_t PROJECTILE_DODGE_ATTEMPTS = 64;
    static constexpr int32_t BLOCK_PICK_CHANCE = 20;
    static constexpr int32_t BLOCK_PLACE_CHANCE = 2000;
    static constexpr int32_t SCREAMING_CLEAR_CHANCE = 100;
    static constexpr float WATER_DAMAGE = 1.0f;
    static constexpr int32_t DROP_ENDER_PEARL = 368;

    // ─── Carriable blocks (14 types) ───
    static constexpr std::array<int32_t, 14> CARRIABLE_BLOCKS = {
        2,   // grass
        3,   // dirt
        12,  // sand
        13,  // gravel
        37,  // yellow_flower (dandelion)
        38,  // red_flower (poppy)
        39,  // brown_mushroom
        40,  // red_mushroom
        46,  // tnt
        81,  // cactus
        82,  // clay
        86,  // pumpkin
        103, // melon_block
        110, // mycelium
    };

    static bool isCarriable(int32_t blockId) {
        for (auto id : CARRIABLE_BLOCKS) if (id == blockId) return true;
        return false;
    }

    // ─── Stare detection ───
    // Java: shouldAttackPlayer
    //   - Pumpkin head → false
    //   - look = player.getLook(1.0f).normalize()
    //   - toEnderman = normalize(enderman.center - player.eye)
    //   - dot = look.dot(toEnderman)
    //   - return dot > 1 - 0.025/distance AND player.canSee(enderman)
    struct StareCheckResult {
        bool isStaring;
        bool startedStaring;  // stareTimer was 0
        bool shouldAttack;    // stareTimer reached 5
    };

    StareCheckResult checkStare(bool playerHasPumpkin, double dotProduct,
                                  double distance, bool canSee) {
        StareCheckResult result{};
        if (playerHasPumpkin) { stareTimer = 0; return result; }
        if (distance < 0.01) return result;

        bool staring = dotProduct > STARE_DOT_BASE - STARE_DOT_DIVISOR / distance && canSee;
        if (staring) {
            isAggressive = true;
            result.isStaring = true;
            if (stareTimer == 0) result.startedStaring = true;
            if (++stareTimer >= STARE_THRESHOLD) {
                stareTimer = 0;
                screaming = true;
                result.shouldAttack = true;
            }
        } else {
            stareTimer = 0;
        }
        return result;
    }

    // ─── Teleport ───
    struct TeleportTarget {
        double x, y, z;
    };

    static TeleportTarget randomTeleport(double posX, double posY, double posZ,
                                            double randX, double randZ, int32_t randY) {
        return {
            posX + (randX - 0.5) * TELEPORT_RANDOM_RANGE,
            posY + (randY - 32),
            posZ + (randZ - 0.5) * TELEPORT_RANDOM_RANGE
        };
    }

    static TeleportTarget entityTeleport(double selfX, double selfY, double selfZ,
                                            double targetX, double targetY, double targetZ,
                                            float selfHeight,
                                            double randX, double randZ, int32_t randY) {
        double dx = selfX - targetX;
        double dy = (selfY + selfHeight / 2.0) - (targetY + 1.62); // eye height
        double dz = selfZ - targetZ;
        double mag = std::sqrt(dx*dx + dy*dy + dz*dz);
        if (mag < 1e-7) return {selfX, selfY, selfZ};
        dx /= mag; dy /= mag; dz /= mag;
        return {
            selfX + (randX - 0.5) * TELEPORT_ENTITY_OFFSET_XZ - dx * TELEPORT_ENTITY_DISTANCE,
            selfY + (randY - 8) - dy * TELEPORT_ENTITY_DISTANCE,
            selfZ + (randZ - 0.5) * TELEPORT_ENTITY_OFFSET_XZ - dz * TELEPORT_ENTITY_DISTANCE
        };
    }

    // Sounds
    static constexpr const char* IDLE_SOUND = "mob.endermen.idle";
    static constexpr const char* SCREAM_SOUND = "mob.endermen.scream";
    static constexpr const char* STARE_SOUND = "mob.endermen.stare";
    static constexpr const char* PORTAL_SOUND = "mob.endermen.portal";
    static constexpr const char* HIT_SOUND = "mob.endermen.hit";
    static constexpr const char* DEATH_SOUND = "mob.endermen.death";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityWolf — Tameable mob with anger, collar, and sitting.
// Java: net.minecraft.entity.passive.EntityWolf (423 lines)
//
//   Size: 0.6×0.8
//   Attributes: wild HP 8, tamed HP 20, moveSpeed 0.3
//   Taming: right-click with bone, 33% success
//     Success → HP=20, sit, set owner, entity state 7
//     Fail → entity state 6
//   Attack: tamed=4, wild=2
//   Damage reduction: non-player non-arrow → (damage+1)/2
//   Healing: right-click tamed wolf with wolf-food (meat) → heal
//   Collar: DataWatcher 20 (color byte, default 14=red → via BlockColored.func_150032_b(1))
//     Dye: right-click with dye → change collar color
//   Sitting: toggle on owner right-click (aiSit)
//   Anger: DW 16 bit 1, set when untamed and has target
//   Water shake: 0.05 per tick over 2.0 total, splash particles sin-based
//   Eye height: height * 0.8
//   Sounds: angry→growl, tamed low HP→whine, tamed 33%→panting, else→bark
//   Sound volume: 0.4
//   Mating: only tamed, both in love, neither sitting
//   Attack restrictions: can't attack creepers, ghasts, tamed wolves (same owner), horses (tamed)
//   Despawn: only untamed after 2400 ticks
//   Follow owner: 10 blocks max, 2 blocks min
//   Leap: Y=0.4
//   Hunt sheep when untamed (every 200 ticks)
//   Child: inherits owner
//   DataWatcher: 16 (tamed+angry byte), 18 (health float), 19 (begging byte), 20 (collar color byte)
// ═══════════════════════════════════════════════════════════════════════════

class EntityWolf {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    bool onGround = false;
    int32_t entityId = 0;
    int32_t ticksExisted = 0;

    // Wolf state
    bool tamed = false;
    bool sitting = false;
    bool angry = false;
    int32_t collarColor = 14; // default red (BlockColored.func_150032_b(1) = 14)
    float health = 8.0f;
    bool begging = false;

    // Shake animation
    bool isWet_ = false;
    bool isShaking = false;
    float timeShaking = 0;
    float prevTimeShaking = 0;

    // Head rotation
    float headRotation = 0;
    float prevHeadRotation = 0;

    // ─── Constants ───
    static constexpr float WIDTH = 0.6f;
    static constexpr float HEIGHT = 0.8f;
    static constexpr double WILD_HP = 8.0;
    static constexpr double TAMED_HP = 20.0;
    static constexpr double MOVE_SPEED = 0.3;
    static constexpr int32_t WILD_DAMAGE = 2;
    static constexpr int32_t TAMED_DAMAGE = 4;
    static constexpr float LEAP_Y = 0.4f;
    static constexpr float FOLLOW_MAX_DIST = 10.0f;
    static constexpr float FOLLOW_MIN_DIST = 2.0f;
    static constexpr float SOUND_VOLUME = 0.4f;
    static constexpr float EYE_HEIGHT_MULT = 0.8f;
    static constexpr int32_t TAME_CHANCE = 3; // 1/3
    static constexpr int32_t HUNT_SHEEP_INTERVAL = 200;
    static constexpr int32_t DESPAWN_AGE = 2400;
    static constexpr float WHINE_HP_THRESHOLD = 10.0f;
    static constexpr float SHAKE_RATE = 0.05f;
    static constexpr float SHAKE_DURATION = 2.0f;

    // Items
    static constexpr int32_t ITEM_BONE = 352;
    static constexpr int32_t ITEM_DYE = 351;

    // ─── Taming ───
    struct TameResult {
        bool success;
        int8_t entityState; // 7 = success, 6 = fail
    };

    TameResult tryTame(int32_t randInt) {
        if (randInt == 0) { // 1/3 chance
            return {true, 7};
        }
        return {false, 6};
    }

    void setTamed(bool t) {
        tamed = t;
        health = t ? static_cast<float>(TAMED_HP) : static_cast<float>(WILD_HP);
    }

    // ─── Damage ───
    int32_t getAttackDamage() const { return tamed ? TAMED_DAMAGE : WILD_DAMAGE; }

    // Non-player non-arrow damage reduction
    static float reduceDamage(float damage, bool isPlayer, bool isArrow) {
        if (!isPlayer && !isArrow) return (damage + 1.0f) / 2.0f;
        return damage;
    }

    // ─── Water shake ───
    struct ShakeResult {
        bool startShake;
        bool endShake;
        bool playShakeSound;
        int32_t splashParticles;
    };

    ShakeResult tickShake(bool inWater, bool hasPath) {
        ShakeResult result{};

        if (inWater) {
            isWet_ = true;
            isShaking = false;
            timeShaking = 0;
            prevTimeShaking = 0;
        } else if ((isWet_ || isShaking) && isShaking) {
            if (timeShaking == 0) result.playShakeSound = true;
            prevTimeShaking = timeShaking;
            timeShaking += SHAKE_RATE;
            if (prevTimeShaking >= SHAKE_DURATION) {
                isWet_ = false;
                isShaking = false;
                prevTimeShaking = 0;
                timeShaking = 0;
                result.endShake = true;
            }
            if (timeShaking > 0.4f) {
                result.splashParticles = static_cast<int32_t>(
                    std::sin((timeShaking - 0.4f) * static_cast<float>(M_PI)) * 7.0f);
            }
        }

        // Check if should start shaking
        if (isWet_ && !isShaking && !hasPath && onGround) {
            isShaking = true;
            timeShaking = 0;
            prevTimeShaking = 0;
            result.startShake = true;
        }

        return result;
    }

    // ─── Sound selection ───
    const char* getLivingSound(int32_t rand3) const {
        if (angry) return "mob.wolf.growl";
        if (rand3 == 0) {
            if (tamed && health < WHINE_HP_THRESHOLD) return "mob.wolf.whine";
            return "mob.wolf.panting";
        }
        return "mob.wolf.bark";
    }

    // ─── Mating ───
    static bool canMate(bool tamed1, bool tamed2, bool sitting2,
                          bool inLove1, bool inLove2) {
        return tamed1 && tamed2 && !sitting2 && inLove1 && inLove2;
    }

    // ─── Despawn ───
    bool canDespawn() const { return !tamed && ticksExisted > DESPAWN_AGE; }

    static constexpr const char* HURT_SOUND = "mob.wolf.hurt";
    static constexpr const char* DEATH_SOUND = "mob.wolf.death";
    static constexpr const char* STEP_SOUND = "mob.wolf.step";
    static constexpr const char* SHAKE_SOUND = "mob.wolf.shake";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntitySlime — Variable-size bouncing mob that splits on death.
// Java: net.minecraft.entity.monster.EntitySlime (242 lines)
//
//   Size: 0.6*s × 0.6*s (s = slime size 1, 2, or 4)
//   HP: s² (1, 4, or 16)
//   XP: s (1, 2, or 4)
//   Damage: s (size 1 can't damage)
//   Collision attack range: 0.6*s
//   Sound volume: 0.4 * s
//   Squish animation: factor += (amount - factor)*0.5, amount *= 0.6
//     Land: amount = -0.5, Take-off: amount = 1.0
//   Jump: delay 10-30 ticks (÷3 when player nearby)
//     moveStrafing = 1 - rand*2, moveForward = size
//   Split on death (size > 1): 2-4 children at size/2
//     Position: offset ±size/4 in X/Z from center, Y+0.5
//   Spawn:
//     Swamp: Y 50-70, rand<0.5, rand<moonPhase, light≤rand(8)
//     Deep: rand(10)==0 AND chunkSeed(987234911).rand(10)==0 AND Y<40
//     Superflat: rand(4)!=1 → fail
//   Sound: "mob.slime.big" (size>1) / "mob.slime.small" (size 1)
//   Drops: slime_ball (size 1 only)
//   Peaceful: die (size > 0)
//   DataWatcher: 16 (size byte)
// ═══════════════════════════════════════════════════════════════════════════

class EntitySlime {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    bool onGround = false;
    int32_t entityId = 0;

    int32_t slimeSize = 1;
    float squishAmount = 0;
    float squishFactor = 0;
    float prevSquishFactor = 0;
    int32_t jumpDelay = 10;

    // ─── Constants ───
    static constexpr float BASE_SIZE = 0.6f;
    static constexpr float SQUISH_LAND = -0.5f;
    static constexpr float SQUISH_JUMP = 1.0f;
    static constexpr float SQUISH_DECAY = 0.6f;
    static constexpr int32_t JUMP_DELAY_MIN = 10;
    static constexpr int32_t JUMP_DELAY_MAX = 30;
    static constexpr int32_t JUMP_DELAY_PLAYER_DIVISOR = 3;
    static constexpr int32_t SPLIT_MIN = 2;
    static constexpr int32_t SPLIT_RAND = 3;
    static constexpr int32_t DROP_SLIME_BALL = 341;

    // Spawn conditions
    static constexpr double SWAMP_Y_MIN = 50.0;
    static constexpr double SWAMP_Y_MAX = 70.0;
    static constexpr double DEEP_Y_MAX = 40.0;
    static constexpr int32_t DEEP_CHANCE = 10;
    static constexpr int64_t DEEP_CHUNK_SEED = 987234911L;
    static constexpr int32_t SUPERFLAT_CHANCE = 4;

    // ─── Size mechanics ───
    float getWidth() const { return BASE_SIZE * slimeSize; }
    float getHeight() const { return BASE_SIZE * slimeSize; }
    int32_t getMaxHealth() const { return slimeSize * slimeSize; }
    int32_t getXPValue() const { return slimeSize; }
    int32_t getAttackDamage() const { return slimeSize; }
    bool canDamagePlayer() const { return slimeSize > 1; }
    double getAttackRange() const { return 0.6 * slimeSize; }
    float getSoundVolume() const { return 0.4f * slimeSize; }
    bool makesSoundOnJump() const { return slimeSize > 0; }
    bool makesSoundOnLand() const { return slimeSize > 2; }

    // Sound based on size
    const char* getSound() const {
        return slimeSize > 1 ? "mob.slime.big" : "mob.slime.small";
    }

    // ─── Squish animation ───
    void tickSquish() {
        prevSquishFactor = squishFactor;
        squishFactor += (squishAmount - squishFactor) * 0.5f;
        squishAmount *= SQUISH_DECAY;
    }

    void onLand() { squishAmount = SQUISH_LAND; }
    void onJump() { squishAmount = SQUISH_JUMP; }

    // ─── Jump AI ───
    struct JumpResult {
        bool shouldJump;
        float moveStrafing;
        float moveForward;
    };

    JumpResult tickJumpAI(bool playerNearby, float randFloat) {
        JumpResult result{};
        if (onGround && --jumpDelay <= 0) {
            // Reset delay
            jumpDelay = JUMP_DELAY_MIN + static_cast<int32_t>(randFloat * (JUMP_DELAY_MAX - JUMP_DELAY_MIN));
            if (playerNearby) jumpDelay /= JUMP_DELAY_PLAYER_DIVISOR;

            result.shouldJump = true;
            result.moveStrafing = 1.0f - randFloat * 2.0f;
            result.moveForward = static_cast<float>(slimeSize);
        } else if (onGround) {
            result.moveStrafing = 0;
            result.moveForward = 0;
        }
        return result;
    }

    // ─── Split on death ───
    struct SplitResult {
        bool shouldSplit;
        int32_t childCount;
        int32_t childSize;
    };

    SplitResult getSplitInfo(int32_t rand3) const {
        SplitResult result{};
        if (slimeSize > 1) {
            result.shouldSplit = true;
            result.childCount = SPLIT_MIN + rand3;
            result.childSize = slimeSize / 2;
        }
        return result;
    }

    // Child position offset
    static void getChildOffset(int32_t index, int32_t parentSize,
                                 float& offsetX, float& offsetZ) {
        offsetX = ((index % 2) - 0.5f) * parentSize / 4.0f;
        offsetZ = ((index / 2) - 0.5f) * parentSize / 4.0f;
    }

    // Drops: slime_ball only when size == 1
    bool dropsItem() const { return slimeSize == 1; }

    // Spawn conditions
    static bool canSpawnSwamp(double posY, float randFloat1, float randFloat2,
                                float moonPhase, int32_t blockLight, int32_t randInt8) {
        return posY > SWAMP_Y_MIN && posY < SWAMP_Y_MAX &&
               randFloat1 < 0.5f && randFloat2 < moonPhase &&
               blockLight <= randInt8;
    }

    static bool canSpawnDeep(double posY, int32_t rand10a, int32_t chunkRand10) {
        return rand10a == 0 && chunkRand10 == 0 && posY < DEEP_Y_MAX;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityPigZombie — Neutral Nether mob with group aggro.
// Java: net.minecraft.entity.monster.EntityPigZombie (165 lines)
//
//   Extends EntityZombie: fire immune, no AI (uses old mob AI)
//   Attributes: reinforcementChance 0, moveSpeed 0.5, attackDamage 5
//   Anger: 400-800 ticks, resets when expired
//   Group aggro: when player attacks, all PigZombies within ±32 become angry
//   Speed boost: +0.45 additive when attacking (UUID 49455A49...)
//   Passive: findPlayerToAttack returns null when not angry
//   Angry sound: "mob.zombiepig.zpigangry" (volume ×2, pitch ×1.8)
//     randomSoundDelay rand(40) ticks after becoming angry
//   Spawn: not peaceful, no entity collision, no liquid
//   Not villager on spawn
//   Equipment: golden_sword
//   Drops: rotten_flesh (rand(2+looting)), gold_nugget (rand(2+looting))
//   Rare drop: gold_ingot
//   NBT: "Anger" short
// ═══════════════════════════════════════════════════════════════════════════

class EntityPigZombie {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    int32_t angerLevel = 0;
    int32_t randomSoundDelay = 0;
    int32_t lastTargetId = -1;

    // ─── Constants ───
    static constexpr float WIDTH = 0.6f;
    static constexpr float HEIGHT = 1.8f;
    static constexpr double MOVE_SPEED = 0.5;
    static constexpr double ATTACK_DAMAGE = 5.0;
    static constexpr double SPEED_BOOST = 0.45;
    static constexpr double REINFORCEMENT_CHANCE = 0.0;
    static constexpr int32_t ANGER_MIN = 400;
    static constexpr int32_t ANGER_RAND = 400;
    static constexpr double GROUP_AGGRO_RANGE = 32.0;
    static constexpr int32_t ANGRY_SOUND_DELAY_RAND = 40;
    static constexpr float ANGRY_SOUND_VOLUME_MULT = 2.0f;
    static constexpr float ANGRY_SOUND_PITCH_MULT = 1.8f;

    // Items
    static constexpr int32_t DROP_ROTTEN_FLESH = 367;
    static constexpr int32_t DROP_GOLD_NUGGET = 371;
    static constexpr int32_t RARE_GOLD_INGOT = 266;
    static constexpr int32_t WEAPON_GOLDEN_SWORD = 283;

    // ─── Anger ───
    void becomeAngry(int32_t rand400, int32_t rand40) {
        angerLevel = ANGER_MIN + rand400;
        randomSoundDelay = rand40;
    }

    bool isAngry() const { return angerLevel > 0; }

    // Passive when not angry
    bool shouldFindTarget() const { return angerLevel > 0; }

    // ─── Speed boost ───
    bool shouldApplySpeedBoost(int32_t currentTargetId) const {
        return currentTargetId != lastTargetId;
    }

    // ─── Angry sound ───
    struct SoundTickResult {
        bool playAngrySound;
    };

    SoundTickResult tickSound() {
        SoundTickResult result{};
        if (randomSoundDelay > 0 && --randomSoundDelay == 0) {
            result.playAngrySound = true;
        }
        return result;
    }

    // ─── Drops ───
    struct DropResult {
        int32_t rottenFleshCount;
        int32_t goldNuggetCount;
    };

    static DropResult calculateDrops(int32_t rand2a, int32_t lootA,
                                       int32_t rand2b, int32_t lootB) {
        return {rand2a + lootA, rand2b + lootB};
    }

    // Sounds
    static constexpr const char* LIVING_SOUND = "mob.zombiepig.zpig";
    static constexpr const char* HURT_SOUND = "mob.zombiepig.zpighurt";
    static constexpr const char* DEATH_SOUND = "mob.zombiepig.zpigdeath";
    static constexpr const char* ANGRY_SOUND = "mob.zombiepig.zpigangry";
};

} // namespace mccpp
