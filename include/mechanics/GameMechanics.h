/**
 * GameMechanics.h — Core game mechanics: DamageSource, FoodStats, EnumDifficulty.
 *
 * Java references:
 *   - net.minecraft.util.DamageSource
 *   - net.minecraft.util.FoodStats
 *   - net.minecraft.world.EnumDifficulty
 *
 * Thread safety:
 *   - DamageSource instances are immutable after construction (static singletons).
 *   - FoodStats is per-player, updated on the server tick thread only.
 *
 * JNI readiness: Simple POD-like structures with string damage types
 * that map directly to Java's DamageSource.damageType.
 */
#pragma once

#include <cstdint>
#include <string>
#include <cmath>
#include <algorithm>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EnumDifficulty — Game difficulty levels.
// Java reference: net.minecraft.world.EnumDifficulty
// ═══════════════════════════════════════════════════════════════════════════

enum class EnumDifficulty : int32_t {
    PEACEFUL = 0,
    EASY     = 1,
    NORMAL   = 2,
    HARD     = 3,
};

// ═══════════════════════════════════════════════════════════════════════════
// DamageSource — Describes the source and properties of damage.
// Java reference: net.minecraft.util.DamageSource
//
// All 14 static damage sources match Java's constants exactly.
// Flag system: isUnblockable, fireDamage, damageIsAbsolute, etc.
// ═══════════════════════════════════════════════════════════════════════════

class DamageSource {
public:
    explicit DamageSource(const std::string& type)
        : damageType_(type) {}

    // ─── Flags (builder pattern matching Java) ─────────────────────────
    DamageSource& setDamageBypassesArmor() {
        isUnblockable_ = true;
        hungerDamage_ = 0.0f;
        return *this;
    }

    DamageSource& setDamageAllowedInCreativeMode() {
        isDamageAllowedInCreativeMode_ = true;
        return *this;
    }

    DamageSource& setDamageIsAbsolute() {
        damageIsAbsolute_ = true;
        hungerDamage_ = 0.0f;
        return *this;
    }

    DamageSource& setFireDamage() {
        fireDamage_ = true;
        return *this;
    }

    DamageSource& setProjectile() {
        projectile_ = true;
        return *this;
    }

    DamageSource& setExplosion() {
        explosion_ = true;
        return *this;
    }

    DamageSource& setDifficultyScaled() {
        difficultyScaled_ = true;
        return *this;
    }

    DamageSource& setMagicDamage() {
        magicDamage_ = true;
        return *this;
    }

    // ─── Queries ───────────────────────────────────────────────────────
    bool isUnblockable() const { return isUnblockable_; }
    bool canHarmInCreative() const { return isDamageAllowedInCreativeMode_; }
    bool isDamageAbsolute() const { return damageIsAbsolute_; }
    bool isFireDamage() const { return fireDamage_; }
    bool isProjectile() const { return projectile_; }
    bool isExplosion() const { return explosion_; }
    bool isDifficultyScaled() const { return difficultyScaled_; }
    bool isMagicDamage() const { return magicDamage_; }
    float getHungerDamage() const { return hungerDamage_; }
    const std::string& getDamageType() const { return damageType_; }

    // ─── Static damage sources (matching Java exactly) ─────────────────
    // Java: DamageSource.inFire = new DamageSource("inFire").setFireDamage()
    static DamageSource inFire;
    static DamageSource onFire;
    static DamageSource lava;
    static DamageSource inWall;
    static DamageSource drown;
    static DamageSource starve;
    static DamageSource cactus;
    static DamageSource fall;
    static DamageSource outOfWorld;
    static DamageSource generic;
    static DamageSource magic;
    static DamageSource wither;
    static DamageSource anvil;
    static DamageSource fallingBlock;

private:
    std::string damageType_;
    bool isUnblockable_ = false;
    bool isDamageAllowedInCreativeMode_ = false;
    bool damageIsAbsolute_ = false;
    bool fireDamage_ = false;
    bool projectile_ = false;
    bool explosion_ = false;
    bool difficultyScaled_ = false;
    bool magicDamage_ = false;
    float hungerDamage_ = 0.3f;
};

// ═══════════════════════════════════════════════════════════════════════════
// FoodStats — Player hunger, saturation, and exhaustion mechanics.
// Java reference: net.minecraft.util.FoodStats
//
// Tick logic (FoodStats.onUpdate):
//   1. If exhaustion >= 4.0: subtract 4.0 from exhaustion
//      - If saturation > 0: reduce saturation by 1.0
//      - Else if not peaceful: reduce food level by 1
//   2. If food >= 18 and naturalRegeneration: 
//      - Increment foodTimer; at 80 ticks: heal 1.0, exhaust 3.0
//   3. If food <= 0:
//      - Increment foodTimer; at 80 ticks: deal starvation damage
//        (hard: always, normal: until 1 HP, easy: never)
//   4. Otherwise: reset foodTimer to 0
// ═══════════════════════════════════════════════════════════════════════════

class FoodStats {
public:
    FoodStats() = default;

    // ─── Food modification ─────────────────────────────────────────────

    // Java: FoodStats.addStats(int, float)
    void addStats(int32_t foodAmount, float saturationModifier) {
        foodLevel_ = std::min(foodAmount + foodLevel_, 20);
        foodSaturationLevel_ = std::min(
            foodSaturationLevel_ + static_cast<float>(foodAmount) * saturationModifier * 2.0f,
            static_cast<float>(foodLevel_)
        );
    }

    // Java: FoodStats.addExhaustion(float)
    void addExhaustion(float amount) {
        foodExhaustionLevel_ = std::min(foodExhaustionLevel_ + amount, 40.0f);
    }

    // ─── Tick update ───────────────────────────────────────────────────

    /**
     * Called once per server tick for each player.
     * Java reference: FoodStats.onUpdate(EntityPlayer)
     *
     * @param difficulty  Current world difficulty
     * @param naturalRegen  Whether naturalRegeneration game rule is enabled
     * @param playerHealth  Player's current health
     * @param maxHealth  Player's max health
     * @param healCallback  Called with heal amount when player should heal
     * @param damageCallback  Called with DamageSource and amount when player should take damage
     */
    void onUpdate(EnumDifficulty difficulty, bool naturalRegen,
                  float playerHealth, float maxHealth,
                  void (*healCallback)(float amount, void* userData),
                  void (*damageCallback)(const DamageSource& source, float amount, void* userData),
                  void* userData);

    // ─── Getters ───────────────────────────────────────────────────────
    int32_t getFoodLevel() const { return foodLevel_; }
    float getSaturationLevel() const { return foodSaturationLevel_; }
    float getExhaustionLevel() const { return foodExhaustionLevel_; }
    int32_t getPrevFoodLevel() const { return prevFoodLevel_; }
    bool needFood() const { return foodLevel_ < 20; }

    // ─── Setters (for deserialization) ─────────────────────────────────
    void setFoodLevel(int32_t level) { foodLevel_ = level; }
    void setSaturationLevel(float level) { foodSaturationLevel_ = level; }
    void setExhaustionLevel(float level) { foodExhaustionLevel_ = level; }
    void setFoodTimer(int32_t timer) { foodTimer_ = timer; }

private:
    // Java: private int foodLevel = 20
    int32_t foodLevel_ = 20;
    // Java: private float foodSaturationLevel = 5.0f
    float foodSaturationLevel_ = 5.0f;
    // Java: private float foodExhaustionLevel
    float foodExhaustionLevel_ = 0.0f;
    // Java: private int foodTimer
    int32_t foodTimer_ = 0;
    // Java: private int prevFoodLevel = 20
    int32_t prevFoodLevel_ = 20;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityPhysics — Simplified entity physics constants and calculations.
// Java reference: net.minecraft.entity.Entity.moveEntity()
//
// Constants match Java's hardcoded values for gravity, drag, etc.
// ═══════════════════════════════════════════════════════════════════════════

namespace EntityPhysics {
    // Java: Entity.moveEntity() — gravity acceleration
    // Applied in EntityLivingBase.moveEntityWithHeading()
    constexpr double GRAVITY = 0.08;

    // Java: Entity y velocity drag (1.0 - 0.02) per tick after gravity
    constexpr double Y_DRAG = 0.98;

    // Java: Entity ground friction (multiplied each tick when on ground)
    constexpr double GROUND_DRAG = 0.91;

    // Java: Entity air drag (same as ground drag in vanilla)
    constexpr double AIR_DRAG = 0.91;

    // Java: Entity.moveFlying() — sprint modifier
    // EntityLivingBase.moveEntityWithHeading applies:
    //   motionY -= 0.08 (gravity)
    //   motionY *= 0.98 (y drag)
    //   motionX *= slipperiness * 0.91 (friction)
    //   motionZ *= slipperiness * 0.91 (friction)
    constexpr double DEFAULT_SLIPPERINESS = 0.6;

    // Java: fall velocity in blocks/tick (terminal velocity ≈ 3.92 b/tick)
    // Derived: velocity = sum of (0.08 * 0.98^n) for n→∞ = 0.08 / (1 - 0.98) = 4.0
    // But drag is 0.98, so terminal = 0.08 / 0.02 = 3.92 with numeric precision

    // Java: fall damage formula
    // damage = distance - 3.0 (first 3 blocks are free)
    constexpr float FALL_DAMAGE_THRESHOLD = 3.0f;

    // Calculate fall damage from distance fallen
    // Java: EntityLivingBase.fall(float)
    inline int32_t calculateFallDamage(float distance) {
        int32_t damage = static_cast<int32_t>(std::ceil(distance - FALL_DAMAGE_THRESHOLD));
        return damage > 0 ? damage : 0;
    }

    // Java: block slipperiness for ice
    constexpr double ICE_SLIPPERINESS = 0.98;

    // Java: Entity.handleWaterMovement() — water drag
    constexpr double WATER_DRAG = 0.8;

    // Java: Entity.handleLavaMovement() — lava drag
    constexpr double LAVA_DRAG = 0.5;

    // Java: player movement speed multipliers
    constexpr float WALK_SPEED = 0.1f;      // Survival walk
    constexpr float SPRINT_SPEED = 0.13f;   // Sprint (1.3×)
    constexpr float SNEAK_SPEED = 0.03f;    // Sneaking
    constexpr float FLY_SPEED = 0.05f;      // Creative flight

    // Java: jump velocity
    // EntityLivingBase.jump(): motionY = 0.42
    constexpr double JUMP_VELOCITY = 0.42;

    // Java: sprint jump boost
    // EntityLivingBase.jump(): adds horizontal velocity based on yaw
    constexpr float SPRINT_JUMP_BOOST = 0.2f;

    // Java: exhaustion values for actions
    // EntityPlayer: jump = 0.05 per jump (0.2 when sprinting)
    constexpr float EXHAUSTION_JUMP = 0.05f;
    constexpr float EXHAUSTION_SPRINT_JUMP = 0.2f;
    constexpr float EXHAUSTION_SPRINT = 0.1f;   // Per meter sprinting
    constexpr float EXHAUSTION_SWIM = 0.01f;    // Per meter swimming
    constexpr float EXHAUSTION_ATTACK = 0.3f;   // Per attack
    constexpr float EXHAUSTION_DAMAGE = 0.3f;   // Taking any damage (except starvation)
    constexpr float EXHAUSTION_REGEN = 3.0f;    // Per HP regenerated from food
}

} // namespace mccpp
