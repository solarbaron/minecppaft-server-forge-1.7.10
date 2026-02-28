/**
 * Experience.h — Player experience, leveling, and XP bar system.
 *
 * Java reference: net.minecraft.entity.player.EntityPlayer
 *   - xpBarCap() — XP needed per level
 *   - addExperience() — XP gain with level-up
 *   - addExperienceLevel() — Direct level change
 *   - getExperiencePoints() — XP dropped on death
 *
 * XP thresholds (1.7.10):
 *   Level  0-14: 17 XP per level
 *   Level 15-29: 17 + 3*(level-15) XP per level
 *   Level 30+:   62 + 7*(level-30) XP per level
 *
 * Thread safety: Per-player, accessed from server thread only.
 *
 * JNI readiness: Simple struct with primitive fields.
 */
#pragma once

#include <cstdint>
#include <algorithm>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// ExperienceStats — Per-player XP state.
// Java reference: EntityPlayer fields experienceLevel, experienceTotal, experience
// ═══════════════════════════════════════════════════════════════════════════

struct ExperienceStats {
    int32_t experienceLevel = 0;   // Current level
    int32_t experienceTotal = 0;   // Lifetime total XP points
    float experience = 0.0f;       // Progress to next level (0.0-1.0)

    // Java: EntityPlayer.xpBarCap()
    // Returns XP points needed to reach next level from current level
    int32_t xpBarCap() const {
        if (experienceLevel >= 30) {
            return 62 + (experienceLevel - 30) * 7;
        }
        if (experienceLevel >= 15) {
            return 17 + (experienceLevel - 15) * 3;
        }
        return 17;
    }

    // Java: EntityPlayer.addExperience(int amount)
    // Adds XP points, handles level-up overflow loop
    // Returns number of levels gained
    int32_t addExperience(int32_t amount) {
        int32_t levelsGained = 0;

        // Cap to prevent overflow
        int32_t maxAdd = INT32_MAX - experienceTotal;
        if (amount > maxAdd) {
            amount = maxAdd;
        }

        // Add as fraction of current level cap
        experience += static_cast<float>(amount) / static_cast<float>(xpBarCap());
        experienceTotal += amount;

        // Level-up loop: while bar overflows
        while (experience >= 1.0f) {
            experience = (experience - 1.0f) * static_cast<float>(xpBarCap());
            addExperienceLevel(1);
            ++levelsGained;
            experience /= static_cast<float>(xpBarCap());
        }

        return levelsGained;
    }

    // Java: EntityPlayer.addExperienceLevel(int levels)
    void addExperienceLevel(int32_t levels) {
        experienceLevel += levels;
        if (experienceLevel < 0) {
            experienceLevel = 0;
            experience = 0.0f;
            experienceTotal = 0;
        }
    }

    // Java: EntityPlayer.getExperiencePoints(EntityPlayer killer)
    // XP dropped on death (if keepInventory is false)
    // Returns level * 7, capped at 100
    int32_t getDeathXpDrop() const {
        int32_t xp = experienceLevel * 7;
        return std::min(xp, 100);
    }

    // Calculate total XP for a given level (from level 0)
    // Useful for display and calculations
    static int32_t getTotalXpForLevel(int32_t level) {
        if (level <= 0) return 0;

        int32_t total = 0;
        for (int32_t i = 0; i < level; ++i) {
            if (i >= 30) {
                total += 62 + (i - 30) * 7;
            } else if (i >= 15) {
                total += 17 + (i - 15) * 3;
            } else {
                total += 17;
            }
        }
        return total;
    }

    // Reset XP on death (when keepInventory is false)
    void resetOnDeath() {
        experienceLevel = 0;
        experienceTotal = 0;
        experience = 0.0f;
    }

    // NBT field names
    static constexpr const char* NBT_XP_LEVEL = "XpLevel";
    static constexpr const char* NBT_XP_TOTAL = "XpTotal";
    static constexpr const char* NBT_XP_P     = "XpP";  // Progress fraction
};

// ═══════════════════════════════════════════════════════════════════════════
// XpOrb — Experience orb values.
// Java reference: net.minecraft.entity.item.EntityXPOrb
// ═══════════════════════════════════════════════════════════════════════════

namespace XpOrb {
    // Java: EntityXPOrb.getXPSplit(int amount)
    // Determines orb size from XP amount
    inline int32_t getXpSplit(int32_t amount) {
        if (amount >= 2477) return 2477;
        if (amount >= 1237) return 1237;
        if (amount >= 617)  return 617;
        if (amount >= 307)  return 307;
        if (amount >= 149)  return 149;
        if (amount >= 73)   return 73;
        if (amount >= 37)   return 37;
        if (amount >= 17)   return 17;
        if (amount >= 7)    return 7;
        if (amount >= 3)    return 3;
        return 1;
    }

    // Java: EntityXPOrb.getTextureByXP()
    // Orb texture index from XP value
    inline int32_t getTextureByXP(int32_t xpValue) {
        if (xpValue >= 2477) return 10;
        if (xpValue >= 1237) return 9;
        if (xpValue >= 617)  return 8;
        if (xpValue >= 307)  return 7;
        if (xpValue >= 149)  return 6;
        if (xpValue >= 73)   return 5;
        if (xpValue >= 37)   return 4;
        if (xpValue >= 17)   return 3;
        if (xpValue >= 7)    return 2;
        if (xpValue >= 3)    return 1;
        return 0;
    }

    // XP values from various sources
    constexpr int32_t COAL_SMELT     = 1;   // Smelting coal ore → XP
    constexpr int32_t IRON_SMELT     = 7;   // 0.7 * 10
    constexpr int32_t GOLD_SMELT     = 10;  // 1.0 * 10
    constexpr int32_t DIAMOND_SMELT  = 10;
    constexpr int32_t EMERALD_SMELT  = 10;
    constexpr int32_t LAPIS_SMELT    = 2;

    constexpr int32_t BREEDING       = 1;   // Breeding animals (1-7 random)
    constexpr int32_t FISHING        = 1;   // Fishing (1-6 random)
    constexpr int32_t BOTTLE_O_ENCH  = 3;   // Bottle o' Enchanting (3-11 random)
}

// ═══════════════════════════════════════════════════════════════════════════
// Enchantment XP costs
// Java reference: net.minecraft.enchantment.EnchantmentHelper
// ═══════════════════════════════════════════════════════════════════════════

namespace EnchantmentXP {
    // Java: ContainerEnchantment.func_78913_b
    // Enchanting costs are level-based (1, 2, or 3 levels in 1.7.10)
    // The actual XP cost is the level requirement itself
    constexpr int32_t MIN_ENCHANT_LEVEL = 1;
    constexpr int32_t MAX_ENCHANT_LEVEL = 30;

    // Repair cost formula for anvil
    // Java: ContainerRepair
    constexpr int32_t MAX_ANVIL_COST = 39;
}

} // namespace mccpp
