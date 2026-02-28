/**
 * GameMechanics.cpp — Core game mechanics implementation.
 *
 * Java references:
 *   net.minecraft.util.DamageSource — static damage source instances
 *   net.minecraft.util.FoodStats — hunger/saturation/exhaustion tick logic
 *
 * Key behaviors preserved from Java:
 *   - DamageSource: 14 static instances with exact flag combinations
 *   - FoodStats.onUpdate: exhaustion→saturation→hunger drain, natural regen
 *     at food>=18 every 80 ticks, starvation at food<=0 every 80 ticks
 *   - Starvation damage: hard=always, normal=until 1HP, easy/peaceful=never
 */

#include "mechanics/GameMechanics.h"

namespace mccpp {

// ═════════════════════════════════════════════════════════════════════════════
// DamageSource — Static instances
// Java: DamageSource.<clinit>()
// ═════════════════════════════════════════════════════════════════════════════

// Java: public static DamageSource inFire = new DamageSource("inFire").setFireDamage();
DamageSource DamageSource::inFire       = DamageSource("inFire").setFireDamage();
// Java: public static DamageSource onFire = new DamageSource("onFire").setDamageBypassesArmor().setFireDamage();
DamageSource DamageSource::onFire       = DamageSource("onFire").setDamageBypassesArmor().setFireDamage();
DamageSource DamageSource::lava         = DamageSource("lava").setFireDamage();
DamageSource DamageSource::inWall       = DamageSource("inWall").setDamageBypassesArmor();
DamageSource DamageSource::drown        = DamageSource("drown").setDamageBypassesArmor();
// Java: DamageSource.starve = new DamageSource("starve").setDamageBypassesArmor().setDamageIsAbsolute();
DamageSource DamageSource::starve       = DamageSource("starve").setDamageBypassesArmor().setDamageIsAbsolute();
DamageSource DamageSource::cactus       = DamageSource("cactus");
DamageSource DamageSource::fall         = DamageSource("fall").setDamageBypassesArmor();
// Java: DamageSource.outOfWorld = new DamageSource("outOfWorld").setDamageBypassesArmor().setDamageAllowedInCreativeMode();
DamageSource DamageSource::outOfWorld   = DamageSource("outOfWorld").setDamageBypassesArmor().setDamageAllowedInCreativeMode();
DamageSource DamageSource::generic      = DamageSource("generic").setDamageBypassesArmor();
DamageSource DamageSource::magic        = DamageSource("magic").setDamageBypassesArmor().setMagicDamage();
DamageSource DamageSource::wither       = DamageSource("wither").setDamageBypassesArmor();
DamageSource DamageSource::anvil        = DamageSource("anvil");
DamageSource DamageSource::fallingBlock = DamageSource("fallingBlock");

// ═════════════════════════════════════════════════════════════════════════════
// FoodStats.onUpdate — The hunger tick
// Java: FoodStats.onUpdate(EntityPlayer)
// ═════════════════════════════════════════════════════════════════════════════

void FoodStats::onUpdate(EnumDifficulty difficulty, bool naturalRegen,
                          float playerHealth, float maxHealth,
                          void (*healCallback)(float amount, void* userData),
                          void (*damageCallback)(const DamageSource& source, float amount, void* userData),
                          void* userData) {
    prevFoodLevel_ = foodLevel_;

    // Step 1: Process exhaustion
    // Java: if (this.foodExhaustionLevel > 4.0f) {
    if (foodExhaustionLevel_ > 4.0f) {
        foodExhaustionLevel_ -= 4.0f;

        if (foodSaturationLevel_ > 0.0f) {
            // Java: foodSaturationLevel = Math.max(foodSaturationLevel - 1.0f, 0.0f)
            foodSaturationLevel_ = std::max(foodSaturationLevel_ - 1.0f, 0.0f);
        } else if (difficulty != EnumDifficulty::PEACEFUL) {
            // Java: foodLevel = Math.max(foodLevel - 1, 0)
            foodLevel_ = std::max(foodLevel_ - 1, 0);
        }
    }

    // Step 2: Natural regeneration (food >= 18)
    // Java: if (naturalRegeneration && foodLevel >= 18 && entityPlayer.shouldHeal())
    bool shouldHeal = (playerHealth > 0.0f && playerHealth < maxHealth);
    if (naturalRegen && foodLevel_ >= 18 && shouldHeal) {
        ++foodTimer_;
        if (foodTimer_ >= 80) {
            // Java: entityPlayer.heal(1.0f)
            if (healCallback) healCallback(1.0f, userData);
            // Java: this.addExhaustion(3.0f)
            addExhaustion(3.0f);
            foodTimer_ = 0;
        }
    }
    // Step 3: Starvation (food <= 0)
    // Java: else if (foodLevel <= 0) {
    else if (foodLevel_ <= 0) {
        ++foodTimer_;
        if (foodTimer_ >= 80) {
            // Java: Starvation damage rules by difficulty
            // Hard: always damage
            // Normal: until 1 HP
            // Easy: never starve to death (but foodLevel never reaches <0 in easy either)
            bool shouldDamage = false;
            if (playerHealth > 10.0f || difficulty == EnumDifficulty::HARD) {
                shouldDamage = true;
            } else if (playerHealth > 1.0f && difficulty == EnumDifficulty::NORMAL) {
                shouldDamage = true;
            }

            if (shouldDamage && damageCallback) {
                damageCallback(DamageSource::starve, 1.0f, userData);
            }
            foodTimer_ = 0;
        }
    }
    // Step 4: Reset timer when not regenerating or starving
    else {
        foodTimer_ = 0;
    }
}

} // namespace mccpp
