/**
 * EntityFishHook.h — Fishing hook entity with complete fishing mechanics.
 *
 * Java reference: net.minecraft.entity.projectile.EntityFishHook (393 lines)
 *
 * Architecture:
 *   - Size: 0.25×0.25
 *   - Cast: velocity 0.4, Gaussian spread 0.0075, normalize+scale 1.5
 *   - Physics:
 *     - Air/water friction: 0.92 (0.5 if grounded or collided horizontally)
 *     - Water buoyancy: 5-layer depth sampling, motionY += 0.04 * (2*depth - 1)
 *     - In-water: friction *= 0.9, motionY *= 0.8
 *     - Rotation: atan2 + 0.2 smoothing
 *   - Fishing state machine (server-side only, when in water):
 *     Phase 1: ticksCaughtDelay (100-900 ticks, -Lure*100)
 *       - Random splash particles with probability ramp (0.15 + bonus)
 *       - When expired → set ticksCatchableDelay (20-80 ticks)
 *     Phase 2: ticksCatchableDelay (fish approaching)
 *       - fishApproachAngle += Gaussian*4, animate bubble/wake at approach
 *       - Bubble particles 15% chance
 *       - When expired → motionY -= 0.2, splash sound, ticksCatchable (10-30)
 *     Phase 3: ticksCatchable (fish hooked, window to reel in)
 *       - motionY -= rand³ * 0.2 (bobbing)
 *       - When expired → all timers reset
 *     Rain: 25% chance to double speed (n2 = 2)
 *     No sky: 50% chance to halve speed (n2 -= 1)
 *   - Loot system (func_146033_f):
 *     Junk: base 10% - (Luck * 2.5% + Lure * 1%)
 *     Valuables: base 5% + (Luck * 1% - Lure * 1%)
 *     Fish: remainder
 *     Luck = EnchantmentHelper.func_151386_g (Luck of the Sea)
 *     Lure = EnchantmentHelper.func_151387_h
 *   - Hook retraction (handleHookRetraction):
 *     Entity caught: pull toward angler (0.1 force + sqrt(dist)*0.08 Y), return 3
 *     Fish catchable: spawn item + XP (1-6), return 1
 *     In ground: return 2
 *     Else: return 0
 *   - Block embedding: same as EntityArrow (xTile/yTile/zTile, inGround, 1200 despawn)
 *   - Die if: angler dead, no fishing rod equipped, distance > 1024 (32 blocks)
 *   - Entity collision: 0.3 expand, skip angler for 5 ticks, 0 damage on hit
 *   - NBT: xTile, yTile, zTile (short), inTile (byte), shake (byte), inGround (byte)
 *
 * Junk loot table (total weight ~82):
 *   leather_boots(10,.9dmg), leather(10), bone(10), potion(10), string(5),
 *   fishing_rod(2,.9dmg), bowl(10), stick(5), ink_sac(1,x10), tripwire_hook(10),
 *   rotten_flesh(10)
 *
 * Valuables loot table (total weight 6):
 *   waterlily(1), name_tag(1), saddle(1), bow(1,.25dmg,ench), fishing_rod(1,.25dmg,ench),
 *   book(1,ench)
 *
 * Fish loot table (total weight 100):
 *   cod(60), salmon(25), clownfish(2), pufferfish(13)
 *
 * Thread safety: Entity tick on single thread per entity.
 */
#pragma once

#include <cmath>
#include <cstdint>

namespace mccpp {

class EntityFishHook {
public:
    // ─── Entity base ───
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    float prevRotationYaw = 0, prevRotationPitch = 0;
    bool isDead = false;
    bool onGround = false;
    bool isCollidedHorizontally = false;
    int32_t entityId = 0;

    // ─── Fish hook specifics ───
    int32_t anglerEntityId = -1;
    int32_t caughtEntityId = -1;

    // Block embedding
    int32_t xTile = -1, yTile = -1, zTile = -1;
    int32_t inTileId = 0;
    bool inGround = false;
    int32_t shake = 0;
    int32_t ticksInGround = 0;
    int32_t ticksInAir = 0;

    // Fishing state machine
    int32_t ticksCatchable = 0;       // Phase 3: fish hooked, reeling window
    int32_t ticksCaughtDelay = 0;     // Phase 1: waiting for fish
    int32_t ticksCatchableDelay = 0;  // Phase 2: fish approaching
    float fishApproachAngle = 0;

    // ─── Constants ───
    static constexpr float WIDTH = 0.25f;
    static constexpr float HEIGHT = 0.25f;
    static constexpr float CAST_SPEED = 0.4f;
    static constexpr float CAST_NORMALIZE_SPEED = 1.5f;
    static constexpr float GAUSSIAN_SPREAD = 0.0075f;
    static constexpr float AIR_FRICTION = 0.92f;
    static constexpr float GROUND_FRICTION = 0.5f;
    static constexpr float WATER_Y_DAMPEN = 0.8f;
    static constexpr float WATER_FRICTION_MULT = 0.9f;
    static constexpr float BUOYANCY = 0.04f;
    static constexpr float ENTITY_EXPAND = 0.3f;
    static constexpr int32_t ANGLER_GRACE = 5;
    static constexpr double MAX_DISTANCE_SQ = 1024.0;
    static constexpr int32_t GROUND_DESPAWN = 1200;
    static constexpr int32_t WATER_SAMPLE_LAYERS = 5;
    static constexpr float PULL_FORCE = 0.1f;
    static constexpr float PULL_Y_BONUS = 0.08f;
    static constexpr float CATCH_BOB_SCALE = 0.2f;

    // Loot probabilities (base, before enchantment modification)
    static constexpr float BASE_JUNK_CHANCE = 0.1f;
    static constexpr float BASE_VALUABLE_CHANCE = 0.05f;
    static constexpr float LUCK_JUNK_REDUCE = 0.025f;
    static constexpr float LUCK_VALUABLE_BOOST = 0.01f;
    static constexpr float LURE_JUNK_REDUCE = 0.01f;
    static constexpr float LURE_VALUABLE_REDUCE = 0.01f;
    static constexpr int32_t LURE_DELAY_REDUCE = 100; // 20 * 5 per level

    // ═══════════════════════════════════════════════════════════════════════
    // Cast — initial velocity from angler yaw/pitch.
    // Java: EntityFishHook(world, player)
    //   - Position: angler + eyeHeight(1.62) - yOffset - 0.1 Y
    //   - Offset: -cos(yaw)*0.16 X, -sin(yaw)*0.16 Z
    //   - Velocity: -sin(yaw)*cos(pitch), -sin(pitch), cos(yaw)*cos(pitch) * 0.4
    //   - Then handleHookCasting: normalize, Gaussian 0.0075, scale 1.5
    // ═══════════════════════════════════════════════════════════════════════

    struct CastParams {
        double anglerX, anglerY, anglerZ;
        float anglerYaw, anglerPitch;
        float yOffset;
        double gaussian1, gaussian2, gaussian3;
    };

    void cast(const CastParams& p) {
        float yawRad = p.anglerYaw / 180.0f * static_cast<float>(M_PI);
        float pitchRad = p.anglerPitch / 180.0f * static_cast<float>(M_PI);

        posX = p.anglerX - std::cos(yawRad) * 0.16;
        posY = p.anglerY + 1.62 - p.yOffset - 0.1;
        posZ = p.anglerZ - std::sin(yawRad) * 0.16;

        rotationYaw = p.anglerYaw;
        rotationPitch = p.anglerPitch;

        motionX = -std::sin(yawRad) * std::cos(pitchRad) * CAST_SPEED;
        motionZ = std::cos(yawRad) * std::cos(pitchRad) * CAST_SPEED;
        motionY = -std::sin(pitchRad) * CAST_SPEED;

        // handleHookCasting: normalize, add Gaussian, scale
        double mag = std::sqrt(motionX*motionX + motionY*motionY + motionZ*motionZ);
        if (mag > 1e-7) {
            double dx = motionX / mag, dy = motionY / mag, dz = motionZ / mag;
            dx += p.gaussian1 * GAUSSIAN_SPREAD;
            dy += p.gaussian2 * GAUSSIAN_SPREAD;
            dz += p.gaussian3 * GAUSSIAN_SPREAD;
            motionX = dx * CAST_NORMALIZE_SPEED;
            motionY = dy * CAST_NORMALIZE_SPEED;
            motionZ = dz * CAST_NORMALIZE_SPEED;
        }

        float horizSpeed = static_cast<float>(std::sqrt(motionX*motionX + motionZ*motionZ));
        prevRotationYaw = rotationYaw = static_cast<float>(std::atan2(motionX, motionZ) * 180.0 / M_PI);
        prevRotationPitch = rotationPitch = static_cast<float>(std::atan2(motionY, horizSpeed) * 180.0 / M_PI);
        ticksInGround = 0;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Fishing state machine — called each tick when in water (waterDepth > 0).
    // Java: complex state machine in EntityFishHook.onUpdate
    //
    // Returns what events occurred for the caller to process.
    // ═══════════════════════════════════════════════════════════════════════

    struct FishingTickResult {
        bool startedCaughtDelay;   // Phase 1 just started
        bool showSplashParticle;   // Random splash during Phase 1
        bool showBubbleWake;       // Bubble/wake during Phase 2
        bool fishBite;             // Phase 2 → Phase 3 transition (splash + dip)
        bool fishBobbing;          // Phase 3 active (random Y dips)
        bool timedOut;             // Phase 3 expired, timers reset
        double approachX, approachZ; // Particle positions for Phase 2
        float approachSin, approachCos;
    };

    FishingTickResult tickFishing(int32_t speedMultiplier,
                                    int32_t lureLevel,
                                    float randFloat1, float randFloat2,
                                    float randGaussian,
                                    float randAngle, float randDist,
                                    float bobRand1, float bobRand2, float bobRand3,
                                    int32_t randCaughtDelay,
                                    int32_t randCatchableDelay,
                                    int32_t randCatchable) {
        FishingTickResult result{};

        if (ticksCatchable > 0) {
            // Phase 3: Fish is hooked, waiting for player to reel
            --ticksCatchable;
            if (ticksCatchable <= 0) {
                ticksCaughtDelay = 0;
                ticksCatchableDelay = 0;
                result.timedOut = true;
            }
        } else if (ticksCatchableDelay > 0) {
            // Phase 2: Fish approaching
            ticksCatchableDelay -= speedMultiplier;
            if (ticksCatchableDelay <= 0) {
                // Fish bites!
                motionY -= 0.2;
                result.fishBite = true;
                // ticksCatchable = rand(10, 30) — provided by caller
                ticksCatchable = randCatchable;
            } else {
                // Animate approach
                fishApproachAngle += static_cast<float>(randGaussian * 4.0);
                float angleRad = fishApproachAngle * static_cast<float>(M_PI) / 180.0f;
                result.approachSin = std::sin(angleRad);
                result.approachCos = std::cos(angleRad);
                result.approachX = posX + result.approachSin * ticksCatchableDelay * 0.1;
                result.approachZ = posZ + result.approachCos * ticksCatchableDelay * 0.1;
                result.showBubbleWake = true;
            }
        } else if (ticksCaughtDelay > 0) {
            // Phase 1: Waiting for fish to notice
            ticksCaughtDelay -= speedMultiplier;

            // Splash particles with increasing probability
            float splashChance = 0.15f;
            if (ticksCaughtDelay < 20) splashChance += (20 - ticksCaughtDelay) * 0.05f;
            else if (ticksCaughtDelay < 40) splashChance += (40 - ticksCaughtDelay) * 0.02f;
            else if (ticksCaughtDelay < 60) splashChance += (60 - ticksCaughtDelay) * 0.01f;

            if (randFloat1 < splashChance) {
                result.showSplashParticle = true;
            }

            if (ticksCaughtDelay <= 0) {
                // Start Phase 2
                fishApproachAngle = randAngle; // randomFloatClamp(0, 360)
                ticksCatchableDelay = randCatchableDelay; // rand(20, 80)
            }
        } else {
            // Start Phase 1
            // ticksCaughtDelay = rand(100, 900) - lureLevel * 100
            ticksCaughtDelay = randCaughtDelay - lureLevel * LURE_DELAY_REDUCE;
            if (ticksCaughtDelay < 1) ticksCaughtDelay = 1;
            result.startedCaughtDelay = true;
        }

        // Bobbing when fish is catchable
        if (ticksCatchable > 0) {
            motionY -= bobRand1 * bobRand2 * bobRand3 * CATCH_BOB_SCALE;
            result.fishBobbing = true;
        }

        return result;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Flight physics — called each tick.
    // ═══════════════════════════════════════════════════════════════════════

    void tickFlight(double waterDepth) {
        // Buoyancy
        double buoyancy = waterDepth * 2.0 - 1.0;
        motionY += BUOYANCY * buoyancy;

        // Friction
        float friction = (onGround || isCollidedHorizontally) ? GROUND_FRICTION : AIR_FRICTION;
        if (waterDepth > 0) {
            friction *= WATER_FRICTION_MULT;
            motionY *= WATER_Y_DAMPEN;
        }

        motionX *= friction;
        motionY *= friction;
        motionZ *= friction;

        // Position update
        posX += motionX;
        posY += motionY;
        posZ += motionZ;

        // Rotation
        float horizSpeed = static_cast<float>(std::sqrt(motionX*motionX + motionZ*motionZ));
        rotationYaw = static_cast<float>(std::atan2(motionX, motionZ) * 180.0 / M_PI);
        rotationPitch = static_cast<float>(std::atan2(motionY, horizSpeed) * 180.0 / M_PI);

        // Wrap
        while (rotationPitch - prevRotationPitch < -180.0f) prevRotationPitch -= 360.0f;
        while (rotationPitch - prevRotationPitch >= 180.0f) prevRotationPitch += 360.0f;
        while (rotationYaw - prevRotationYaw < -180.0f) prevRotationYaw -= 360.0f;
        while (rotationYaw - prevRotationYaw >= 180.0f) prevRotationYaw += 360.0f;

        // Smooth
        rotationPitch = prevRotationPitch + (rotationPitch - prevRotationPitch) * 0.2f;
        rotationYaw = prevRotationYaw + (rotationYaw - prevRotationYaw) * 0.2f;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Hook retraction — reel in the line.
    // Java: EntityFishHook.handleHookRetraction
    //   Returns: 0=nothing, 1=fish caught, 2=was in ground, 3=entity caught
    // ═══════════════════════════════════════════════════════════════════════

    struct RetractionResult {
        int32_t returnCode; // 0, 1, 2, or 3
        double pullX, pullY, pullZ;    // For entity/item pull
        bool spawnItem;
        bool spawnXP;
        int32_t xpAmount;
    };

    RetractionResult handleRetraction(double anglerX, double anglerY, double anglerZ,
                                        int32_t xpRand) {
        RetractionResult result{};

        if (caughtEntityId >= 0) {
            // Entity caught: pull toward angler
            double dx = anglerX - posX;
            double dy = anglerY - posY;
            double dz = anglerZ - posZ;
            double dist = std::sqrt(dx*dx + dy*dy + dz*dz);
            result.pullX = dx * PULL_FORCE;
            result.pullY = dy * PULL_FORCE + std::sqrt(dist) * PULL_Y_BONUS;
            result.pullZ = dz * PULL_FORCE;
            result.returnCode = 3;
        } else if (ticksCatchable > 0) {
            // Fish caught: spawn item + XP
            double dx = anglerX - posX;
            double dy = anglerY - posY;
            double dz = anglerZ - posZ;
            double dist = std::sqrt(dx*dx + dy*dy + dz*dz);
            result.pullX = dx * PULL_FORCE;
            result.pullY = dy * PULL_FORCE + std::sqrt(dist) * PULL_Y_BONUS;
            result.pullZ = dz * PULL_FORCE;
            result.spawnItem = true;
            result.spawnXP = true;
            result.xpAmount = xpRand; // rand(6) + 1 → caller provides
            result.returnCode = 1;
        }

        if (inGround) {
            result.returnCode = 2;
        }

        isDead = true;
        return result;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Loot selection — determine caught item category.
    // Java: func_146033_f
    //   junkChance = 0.1 - luck * 0.025 - lure * 0.01   (clamped 0..1)
    //   valuableChance = 0.05 + luck * 0.01 - lure * 0.01 (clamped 0..1)
    //   rand < junkChance → JUNK
    //   rand - junk < valuableChance → VALUABLE
    //   else → FISH
    // ═══════════════════════════════════════════════════════════════════════

    enum class LootCategory : int32_t {
        JUNK = 0,
        VALUABLE = 1,
        FISH = 2
    };

    static LootCategory selectLootCategory(float randFloat, int32_t luckLevel, int32_t lureLevel) {
        float junkChance = BASE_JUNK_CHANCE - luckLevel * LUCK_JUNK_REDUCE - lureLevel * LURE_JUNK_REDUCE;
        float valuableChance = BASE_VALUABLE_CHANCE + luckLevel * LUCK_VALUABLE_BOOST - lureLevel * LURE_VALUABLE_REDUCE;

        // Clamp
        if (junkChance < 0) junkChance = 0;
        if (junkChance > 1) junkChance = 1;
        if (valuableChance < 0) valuableChance = 0;
        if (valuableChance > 1) valuableChance = 1;

        if (randFloat < junkChance) return LootCategory::JUNK;
        if (randFloat - junkChance < valuableChance) return LootCategory::VALUABLE;
        return LootCategory::FISH;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Loot tables — weighted random items.
    // ═══════════════════════════════════════════════════════════════════════

    struct LootEntry {
        int32_t itemId;
        int32_t damage;
        int32_t count;
        int32_t weight;
        float maxDamagePercent; // 0 = no random damage
        bool enchantable;
    };

    // Junk: total weight = 82
    static constexpr LootEntry JUNK_LOOT[] = {
        {301, 0, 1, 10, 0.9f, false},  // leather_boots
        {334, 0, 1, 10, 0, false},      // leather
        {352, 0, 1, 10, 0, false},      // bone
        {373, 0, 1, 10, 0, false},      // potion (water bottle)
        {287, 0, 1, 5, 0, false},       // string
        {346, 0, 1, 2, 0.9f, false},    // fishing_rod
        {281, 0, 1, 10, 0, false},      // bowl
        {280, 0, 1, 5, 0, false},       // stick
        {351, 0, 10, 1, 0, false},      // ink_sac (dye meta 0, count 10)
        {131, 0, 1, 10, 0, false},      // tripwire_hook (block)
        {367, 0, 1, 10, 0, false},      // rotten_flesh
    };
    static constexpr int32_t JUNK_TOTAL_WEIGHT = 82;

    // Valuables: total weight = 6
    static constexpr LootEntry VALUABLE_LOOT[] = {
        {111, 0, 1, 1, 0, false},      // waterlily (block)
        {421, 0, 1, 1, 0, false},      // name_tag
        {329, 0, 1, 1, 0, false},      // saddle
        {261, 0, 1, 1, 0.25f, true},   // bow (enchantable)
        {346, 0, 1, 1, 0.25f, true},   // fishing_rod (enchantable)
        {340, 0, 1, 1, 0, true},       // book (enchantable)
    };
    static constexpr int32_t VALUABLE_TOTAL_WEIGHT = 6;

    // Fish: total weight = 100
    static constexpr LootEntry FISH_LOOT[] = {
        {349, 0, 1, 60, 0, false},     // cod (raw fish meta 0)
        {349, 1, 1, 25, 0, false},     // salmon (raw fish meta 1)
        {349, 2, 1, 2, 0, false},      // clownfish (raw fish meta 2)
        {349, 3, 1, 13, 0, false},     // pufferfish (raw fish meta 3)
    };
    static constexpr int32_t FISH_TOTAL_WEIGHT = 100;

    // ═══════════════════════════════════════════════════════════════════════
    // Validation — check if hook should die.
    // ═══════════════════════════════════════════════════════════════════════

    bool shouldDie(bool anglerAlive, bool anglerHasRod, double distSqToAngler) const {
        if (!anglerAlive) return true;
        if (!anglerHasRod) return true;
        if (distSqToAngler > MAX_DISTANCE_SQ) return true;
        return false;
    }

    // Ground state
    bool tickGround(int32_t currentBlockId) {
        if (shake > 0) --shake;
        if (currentBlockId == inTileId) {
            ++ticksInGround;
            if (ticksInGround >= GROUND_DESPAWN) {
                isDead = true;
                return true;
            }
            return false;
        }
        // Block changed → dislodge
        inGround = false;
        ticksInGround = 0;
        ticksInAir = 0;
        return false;
    }
};

} // namespace mccpp
