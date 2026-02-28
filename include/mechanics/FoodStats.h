/**
 * FoodStats.h — Player hunger, saturation, and exhaustion system.
 *
 * Java reference: net.minecraft.util.FoodStats
 *
 * Core hunger mechanics:
 *   - foodLevel: 0-20 (20 = full, displayed as 10 hunger shanks)
 *   - foodSaturationLevel: 0.0-foodLevel (hidden buffer before hunger drains)
 *   - foodExhaustionLevel: accumulates from actions, drains saturation/food at 4.0
 *   - foodTimer: ticks since last heal/starve (triggers at 80 ticks)
 *
 * Natural regeneration: foodLevel >= 18, heals 1 HP every 80 ticks, costs 3.0 exhaustion
 * Starvation: foodLevel <= 0, deals 1 damage every 80 ticks
 *   - Hard: starves to death
 *   - Normal: starves to 1 HP (0.5 hearts)
 *   - Easy/Peaceful: no starvation
 *
 * Thread safety: Per-player, accessed from server thread only.
 *
 * JNI readiness: Simple struct with primitive fields.
 */
#pragma once

#include <algorithm>
#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// FoodStats — Per-player hunger state.
// Java reference: net.minecraft.util.FoodStats
// ═══════════════════════════════════════════════════════════════════════════

struct FoodStats {
    int32_t foodLevel = 20;
    float foodSaturationLevel = 5.0f;
    float foodExhaustionLevel = 0.0f;
    int32_t foodTimer = 0;
    int32_t prevFoodLevel = 20;

    // Java: addStats(int healAmount, float saturationModifier)
    // saturation gained = healAmount * saturationModifier * 2.0
    // food capped at 20, saturation capped at foodLevel
    void addStats(int32_t healAmount, float saturationModifier) {
        foodLevel = std::min(healAmount + foodLevel, 20);
        foodSaturationLevel = std::min(
            foodSaturationLevel + static_cast<float>(healAmount) * saturationModifier * 2.0f,
            static_cast<float>(foodLevel)
        );
    }

    // Java: needFood
    bool needFood() const {
        return foodLevel < 20;
    }

    // Java: addExhaustion — capped at 40.0
    void addExhaustion(float amount) {
        foodExhaustionLevel = std::min(foodExhaustionLevel + amount, 40.0f);
    }

    // Java: getFoodLevel
    int32_t getFoodLevel() const { return foodLevel; }

    // Java: getSaturationLevel
    float getSaturationLevel() const { return foodSaturationLevel; }

    float getExhaustionLevel() const { return foodExhaustionLevel; }

    // Java: onUpdate — called every tick per player
    // Returns: {shouldHeal, shouldStarve, starvationDamage}
    struct TickResult {
        bool shouldHeal = false;       // Player should be healed 1.0 HP
        bool shouldStarve = false;     // Player should take starvation damage
        float starveDamage = 0.0f;     // Amount of starvation damage
        bool foodLevelChanged = false; // foodLevel changed since last tick
    };

    // difficulty: 0=peaceful, 1=easy, 2=normal, 3=hard
    // naturalRegeneration: from game rules
    // playerHealth: current player health
    // playerCanHeal: player.shouldHeal() (health < maxHealth)
    TickResult onUpdate(int32_t difficulty, bool naturalRegeneration,
                        float playerHealth, bool playerCanHeal) {
        TickResult result;
        prevFoodLevel = foodLevel;

        // Java: exhaustion threshold = 4.0
        if (foodExhaustionLevel > 4.0f) {
            foodExhaustionLevel -= 4.0f;
            if (foodSaturationLevel > 0.0f) {
                // Drain saturation first
                foodSaturationLevel = std::max(foodSaturationLevel - 1.0f, 0.0f);
            } else if (difficulty != 0) {
                // Then drain food (not on peaceful)
                foodLevel = std::max(foodLevel - 1, 0);
            }
        }

        // Java: natural regeneration — food >= 18, heal every 80 ticks
        if (naturalRegeneration && foodLevel >= 18 && playerCanHeal) {
            ++foodTimer;
            if (foodTimer >= 80) {
                result.shouldHeal = true;
                addExhaustion(3.0f);
                foodTimer = 0;
            }
        }
        // Java: starvation — food <= 0, damage every 80 ticks
        else if (foodLevel <= 0) {
            ++foodTimer;
            if (foodTimer >= 80) {
                // Hard: starves to death (any health)
                // Normal: starves to 1 HP
                // Easy/Peaceful: no starvation
                if (playerHealth > 10.0f || difficulty == 3 ||
                    (playerHealth > 1.0f && difficulty == 2)) {
                    result.shouldStarve = true;
                    result.starveDamage = 1.0f;
                }
                foodTimer = 0;
            }
        } else {
            foodTimer = 0;
        }

        result.foodLevelChanged = (foodLevel != prevFoodLevel);
        return result;
    }

    // NBT field names
    static constexpr const char* NBT_FOOD_LEVEL       = "foodLevel";
    static constexpr const char* NBT_FOOD_TIMER        = "foodTickTimer";
    static constexpr const char* NBT_SATURATION_LEVEL  = "foodSaturationLevel";
    static constexpr const char* NBT_EXHAUSTION_LEVEL  = "foodExhaustionLevel";
};

// ═══════════════════════════════════════════════════════════════════════════
// FoodValues — Healing and saturation modifier for each food item.
// Java reference: net.minecraft.item.ItemFood constructor calls
//
// Each food item has:
//   healAmount: hunger points restored
//   saturationModifier: multiplier for saturation (sat = heal * mod * 2)
// ═══════════════════════════════════════════════════════════════════════════

struct FoodValue {
    int32_t itemId;
    int32_t healAmount;
    float saturationModifier;
};

class FoodValues {
public:
    static void init();
    static const FoodValue* getByItemId(int32_t itemId);
    static int32_t getCount();

private:
    static FoodValue values_[];
    static int32_t count_;
    static bool initialized_;
};

// ═══════════════════════════════════════════════════════════════════════════
// Exhaustion constants — Amount added per player action.
// Java reference: Various EntityPlayer methods
// ═══════════════════════════════════════════════════════════════════════════

namespace Exhaustion {
    constexpr float SPRINT_JUMP  = 0.8f;   // EntityPlayer.jump() while sprinting
    constexpr float JUMP         = 0.2f;   // EntityPlayer.jump()
    constexpr float SWIM         = 0.015f; // per meter swimming
    constexpr float WALK         = 0.01f;  // per meter walking (L2186)
    constexpr float SPRINT       = 0.1f;   // per meter sprinting
    constexpr float ATTACK       = 0.3f;   // per attack
    constexpr float DAMAGE       = 0.3f;   // per damage taken (L1206)
    constexpr float HEAL         = 3.0f;   // per natural regen heal
    constexpr float HUNGER       = 0.025f; // hunger effect per tick
}

} // namespace mccpp
