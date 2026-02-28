/**
 * EnchantmentHelper.h — Enchantment utility functions.
 *
 * Java reference: net.minecraft.enchantment.EnchantmentHelper
 *
 * Key functions:
 *   - getEnchantmentLevel: read enchant level from item NBT
 *   - getEnchantmentModifierDamage: protection factor (capped 25, randomized)
 *   - getKnockbackModifier, getFireAspectModifier, getLootingModifier, etc.
 *   - calcItemStackEnchantability: enchanting table slot formula
 *   - buildEnchantmentList: random enchantment selection with compatibility
 *
 * Thread safety: Uses thread-local RNG.
 *
 * JNI readiness: Static functions, simple types.
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// Forward reference to enchantment IDs
namespace EnchantmentID {
    // Already defined in Enchantment.h
    constexpr int32_t _PROTECTION        = 0;
    constexpr int32_t _FIRE_PROTECTION   = 1;
    constexpr int32_t _FEATHER_FALLING   = 2;
    constexpr int32_t _BLAST_PROTECTION  = 3;
    constexpr int32_t _PROJECTILE_PROTECTION = 4;
    constexpr int32_t _RESPIRATION       = 5;
    constexpr int32_t _AQUA_AFFINITY     = 6;
    constexpr int32_t _THORNS            = 7;
    constexpr int32_t _SHARPNESS        = 16;
    constexpr int32_t _SMITE            = 17;
    constexpr int32_t _BANE_OF_ARTHROPODS = 18;
    constexpr int32_t _KNOCKBACK         = 19;
    constexpr int32_t _FIRE_ASPECT       = 20;
    constexpr int32_t _LOOTING           = 21;
    constexpr int32_t _EFFICIENCY        = 32;
    constexpr int32_t _SILK_TOUCH        = 33;
    constexpr int32_t _UNBREAKING        = 34;
    constexpr int32_t _FORTUNE           = 35;
    constexpr int32_t _POWER            = 48;
    constexpr int32_t _PUNCH            = 49;
    constexpr int32_t _FLAME            = 50;
    constexpr int32_t _INFINITY         = 51;
    constexpr int32_t _LUCK_OF_THE_SEA  = 61;
    constexpr int32_t _LURE             = 62;
}

// ═══════════════════════════════════════════════════════════════════════════
// EnchantEntry — id:level pair on an item.
// ═══════════════════════════════════════════════════════════════════════════

struct EnchantEntry {
    int16_t id;
    int16_t level;
};

// ═══════════════════════════════════════════════════════════════════════════
// EnchantmentHelper — Static utility functions.
// Java reference: net.minecraft.enchantment.EnchantmentHelper
// ═══════════════════════════════════════════════════════════════════════════

namespace EnchantmentHelper {

    // ─── Reading enchantments from item data ───

    // Java: getEnchantmentLevel — get level of specific enchantment on item
    inline int32_t getEnchantmentLevel(int32_t enchId, const std::vector<EnchantEntry>& enchants) {
        for (const auto& e : enchants) {
            if (e.id == enchId) return e.level;
        }
        return 0;
    }

    // Java: getMaxEnchantmentLevel — max level across armor slots
    inline int32_t getMaxEnchantmentLevel(int32_t enchId,
                                           const std::vector<EnchantEntry> armorSlots[], int32_t slotCount) {
        int32_t max = 0;
        for (int32_t i = 0; i < slotCount; ++i) {
            int32_t lvl = getEnchantmentLevel(enchId, armorSlots[i]);
            if (lvl > max) max = lvl;
        }
        return max;
    }

    // ─── Protection calculation ───

    // Java: getEnchantmentModifierDamage
    // Sums protection values from all armor, caps at 25, then randomizes.
    // Returns effective protection factor.
    //
    // DamageType enum for protection modifier calculation
    enum class DamageType { GENERIC, FIRE, EXPLOSION, PROJECTILE, FALL };

    // Java: EnchantmentProtection.calcModifierDamage
    // Protection type 0: all damage (1 per level)
    // Protection type 1: fire (2 per level)
    // Protection type 2: fall (3 per level)
    // Protection type 3: explosion (2 per level)
    // Protection type 4: projectile (2 per level)
    inline int32_t calcProtectionModifier(int32_t enchId, int32_t level, DamageType dmgType) {
        switch (enchId) {
            case 0: // Protection
                return level; // 1 per level, all damage
            case 1: // Fire Protection
                if (dmgType == DamageType::FIRE) return level * 2;
                return 0;
            case 2: // Feather Falling
                if (dmgType == DamageType::FALL) return level * 3;
                return 0;
            case 3: // Blast Protection
                if (dmgType == DamageType::EXPLOSION) return level * 2;
                return 0;
            case 4: // Projectile Protection
                if (dmgType == DamageType::PROJECTILE) return level * 2;
                return 0;
            default: return 0;
        }
    }

    inline int32_t getEnchantmentModifierDamage(
        const std::vector<EnchantEntry> armorSlots[], int32_t slotCount,
        DamageType dmgType, std::mt19937& rng)
    {
        int32_t total = 0;
        for (int32_t s = 0; s < slotCount; ++s) {
            for (const auto& e : armorSlots[s]) {
                total += calcProtectionModifier(e.id, e.level, dmgType);
            }
        }
        // Java: cap at 25
        if (total > 25) total = 25;

        // Java: (total + 1 >> 1) + rand.nextInt((total >> 1) + 1)
        std::uniform_int_distribution<int32_t> dist(0, (total >> 1));
        return ((total + 1) >> 1) + dist(rng);
    }

    // ─── Common enchantment lookups ───

    // Java: getKnockbackModifier
    inline int32_t getKnockbackModifier(const std::vector<EnchantEntry>& heldItem) {
        return getEnchantmentLevel(EnchantmentID::_KNOCKBACK, heldItem);
    }

    // Java: getFireAspectModifier
    inline int32_t getFireAspectModifier(const std::vector<EnchantEntry>& heldItem) {
        return getEnchantmentLevel(EnchantmentID::_FIRE_ASPECT, heldItem);
    }

    // Java: getLootingModifier
    inline int32_t getLootingModifier(const std::vector<EnchantEntry>& heldItem) {
        return getEnchantmentLevel(EnchantmentID::_LOOTING, heldItem);
    }

    // Java: getEfficiencyModifier
    inline int32_t getEfficiencyModifier(const std::vector<EnchantEntry>& heldItem) {
        return getEnchantmentLevel(EnchantmentID::_EFFICIENCY, heldItem);
    }

    // Java: getSilkTouchModifier
    inline bool getSilkTouchModifier(const std::vector<EnchantEntry>& heldItem) {
        return getEnchantmentLevel(EnchantmentID::_SILK_TOUCH, heldItem) > 0;
    }

    // Java: getFortuneModifier
    inline int32_t getFortuneModifier(const std::vector<EnchantEntry>& heldItem) {
        return getEnchantmentLevel(EnchantmentID::_FORTUNE, heldItem);
    }

    // Java: getRespiration — max across all armor
    inline int32_t getRespiration(const std::vector<EnchantEntry> armor[], int32_t slotCount) {
        return getMaxEnchantmentLevel(EnchantmentID::_RESPIRATION, armor, slotCount);
    }

    // Java: getAquaAffinityModifier
    inline bool getAquaAffinityModifier(const std::vector<EnchantEntry> armor[], int32_t slotCount) {
        return getMaxEnchantmentLevel(EnchantmentID::_AQUA_AFFINITY, armor, slotCount) > 0;
    }

    // Java: func_151386_g — Luck of the Sea
    inline int32_t getLuckOfTheSeaModifier(const std::vector<EnchantEntry>& heldItem) {
        return getEnchantmentLevel(EnchantmentID::_LUCK_OF_THE_SEA, heldItem);
    }

    // Java: func_151387_h — Lure
    inline int32_t getLureModifier(const std::vector<EnchantEntry>& heldItem) {
        return getEnchantmentLevel(EnchantmentID::_LURE, heldItem);
    }

    // ─── Unbreaking chance ───
    // Java: EnchantmentDurability.negateDamage
    // Returns true if durability damage should be negated
    inline bool shouldNegateDurability(int32_t unbreakingLevel, std::mt19937& rng) {
        if (unbreakingLevel <= 0) return false;
        // Java: rand.nextInt(level + 1) > 0 → chance = level/(level+1)
        std::uniform_int_distribution<int32_t> dist(0, unbreakingLevel);
        return dist(rng) > 0;
    }

    // ─── Enchanting table slot calculation ───

    // Java: calcItemStackEnchantability
    // Slot 0 (top): max(result/3, 1)
    // Slot 1 (mid): result*2/3 + 1
    // Slot 2 (bot): max(result, bookshelves*2)
    inline int32_t calcItemStackEnchantability(std::mt19937& rng, int32_t slot,
                                                int32_t bookshelves, int32_t itemEnchantability) {
        if (itemEnchantability <= 0) return 0;

        int32_t shelves = std::min(bookshelves, 15);
        std::uniform_int_distribution<int32_t> dist1(1, 8);
        std::uniform_int_distribution<int32_t> dist2(0, shelves);

        int32_t base = dist1(rng) + (shelves >> 1) + dist2(rng);

        if (slot == 0) return std::max(base / 3, 1);
        if (slot == 1) return base * 2 / 3 + 1;
        return std::max(base, shelves * 2);
    }

    // ─── Enchantment table: build random enchantments ───

    // Java: buildEnchantmentList
    // Returns list of {enchantId, level} pairs for an enchanting result.
    // Uses 50% chain probability (nextInt(50) <= adjustedLevel).
    inline std::vector<EnchantEntry> buildRandomEnchantments(
        std::mt19937& rng, int32_t enchantPower, int32_t itemEnchantability)
    {
        std::vector<EnchantEntry> result;
        if (itemEnchantability <= 0) return result;

        // Java: modify enchant power
        int32_t halfEnch = itemEnchantability / 2;
        std::uniform_int_distribution<int32_t> distHalf(0, halfEnch > 0 ? (halfEnch >> 1) : 0);
        int32_t modified = 1 + distHalf(rng) + distHalf(rng);
        int32_t power = modified + enchantPower;

        // Java: random float modifier ±15%
        std::uniform_real_distribution<float> distFloat(0.0f, 1.0f);
        float bonus = (distFloat(rng) + distFloat(rng) - 1.0f) * 0.15f;
        int32_t finalPower = static_cast<int32_t>(static_cast<float>(power) * (1.0f + bonus) + 0.5f);
        if (finalPower < 1) finalPower = 1;

        // First enchantment would be selected from valid pool
        // Additional enchantments: while rand.nextInt(50) <= adjustedPower
        // Each additional halves the adjusted power
        // (Pool filtering and actual enchantment selection requires EnchantmentRegistry)

        return result; // Actual selection requires item type and enchantment pool
    }
}

} // namespace mccpp
