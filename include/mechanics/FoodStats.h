#pragma once
// FoodStats — hunger system, 1:1 port of zr.java (FoodStats).
//
// Fields (zr.java):
//   a = foodLevel (int, starts 20)
//   b = saturation (float, starts 5.0)
//   c = exhaustion (float)
//   d = foodTickTimer (int)
//   e = prevFoodLevel (int)
//
// Mechanics:
//   - Exhaustion accumulates from actions (walking, sprinting, jumping, etc.)
//   - When exhaustion >= 4.0: drains saturation first, then food level
//   - When food >= 18: natural regen 1 HP every 80 ticks
//   - When food == 0: starvation damage every 80 ticks
//
// Exhaustion values from vanilla (yz.java / wq.java):
//   Walking:   0.01 per meter
//   Sprinting: 0.1  per meter
//   Swimming:  0.015 per meter
//   Jumping:   0.05 per jump
//   Sprint-jump: 0.2 per jump
//   Taking damage: 0.3 per hit
//   Attack:    0.3 per swing
//   Mining:    0.025 per block

#include <cstdint>
#include <cmath>
#include <algorithm>

#include "nbt/NBT.h"

namespace mc {

// Exhaustion constants from vanilla
namespace Exhaustion {
    constexpr float WALK        = 0.01f;
    constexpr float SPRINT      = 0.1f;
    constexpr float SWIM        = 0.015f;
    constexpr float JUMP        = 0.05f;
    constexpr float SPRINT_JUMP = 0.2f;
    constexpr float DAMAGE      = 0.3f;
    constexpr float ATTACK      = 0.3f;
    constexpr float MINE        = 0.025f;
    constexpr float REGEN       = 3.0f;   // Cost of natural regen (per heal)
    constexpr float MAX         = 40.0f;  // zr.java line 76: Math.min(c + f2, 40.0f)
}

class FoodStats {
public:
    int foodLevel = 20;        // zr.a — max 20
    float saturation = 5.0f;   // zr.b — starts at 5.0, capped at foodLevel
    float exhaustion = 0.0f;   // zr.c — accumulates from actions
    int foodTickTimer = 0;     // zr.d — counts ticks for regen/starvation
    int prevFoodLevel = 20;    // zr.e — for change detection

    // zr.a(int, float) — add food from eating
    void addFood(int food, float satMod) {
        foodLevel = std::min(food + foodLevel, 20);
        saturation = std::min(saturation + static_cast<float>(food) * satMod * 2.0f,
                              static_cast<float>(foodLevel));
    }

    // zr.a(float) — add exhaustion from actions
    void addExhaustion(float amount) {
        exhaustion = std::min(exhaustion + amount, Exhaustion::MAX);
    }

    // zr.a(yz) — main tick, called once per server tick
    // Returns: negative = starvation damage dealt, positive = health healed, 0 = no change
    struct TickResult {
        float healthChange = 0.0f;  // positive = heal, negative = damage
        bool foodChanged = false;
    };

    TickResult tick(float playerHealth, float maxHealth, bool naturalRegen = true) {
        TickResult result;
        prevFoodLevel = foodLevel;

        // Exhaustion drain — zr.java lines 23-29
        if (exhaustion > 4.0f) {
            exhaustion -= 4.0f;
            if (saturation > 0.0f) {
                saturation = std::max(saturation - 1.0f, 0.0f);
            } else {
                // Drain food level (not in peaceful)
                foodLevel = std::max(foodLevel - 1, 0);
            }
        }

        // Natural regeneration — zr.java lines 31-37
        // When food >= 18 and player can heal: regen 1 HP every 80 ticks
        if (naturalRegen && foodLevel >= 18 && playerHealth < maxHealth) {
            ++foodTickTimer;
            if (foodTickTimer >= 80) {
                result.healthChange = 1.0f;
                addExhaustion(Exhaustion::REGEN);
                foodTickTimer = 0;
            }
        }
        // Starvation — zr.java lines 38-45
        // When food == 0: deal 1 damage every 80 ticks
        else if (foodLevel <= 0) {
            ++foodTickTimer;
            if (foodTickTimer >= 80) {
                // Starvation damage: deal 1.0 damage
                // In Easy: stop at 10 HP, Normal: stop at 1 HP, Hard: kill
                if (playerHealth > 1.0f) {
                    result.healthChange = -1.0f;
                }
                foodTickTimer = 0;
            }
        } else {
            foodTickTimer = 0;
        }

        result.foodChanged = (foodLevel != prevFoodLevel);
        return result;
    }

    // NBT save — zr.b(dh) in zr.java lines 60-65
    void saveToNBT(nbt::NBTTagCompound& tag) const {
        tag.setInt("foodLevel", foodLevel);
        tag.setInt("foodTickTimer", foodTickTimer);
        tag.setFloat("foodSaturationLevel", saturation);
        tag.setFloat("foodExhaustionLevel", exhaustion);
    }

    // NBT load — zr.a(dh) in zr.java lines 51-57
    void loadFromNBT(const nbt::NBTTagCompound& tag) {
        if (tag.hasKey("foodLevel", 99)) {
            foodLevel = tag.getInt("foodLevel");
            foodTickTimer = tag.getInt("foodTickTimer");
            saturation = tag.getFloat("foodSaturationLevel");
            exhaustion = tag.getFloat("foodExhaustionLevel");
        }
    }

    // zr.c() — can eat
    bool needsFood() const { return foodLevel < 20; }
};

} // namespace mc
