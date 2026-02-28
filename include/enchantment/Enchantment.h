/**
 * Enchantment.h — Enchantment system.
 *
 * Java references:
 *   - net.minecraft.enchantment.Enchantment — Base enchantment class
 *   - net.minecraft.enchantment.EnchantmentProtection — Protection variants
 *   - net.minecraft.enchantment.EnchantmentDamage — Damage variants
 *   - net.minecraft.enchantment.EnchantmentHelper — Utility functions
 *   - net.minecraft.enchantment.EnumEnchantmentType — Item type filters
 *   - net.minecraft.enchantment.EnchantmentData — ID + level pair
 *
 * All 24 vanilla 1.7.10 enchantments with exact IDs, weights, max levels,
 * and enchantability ranges from the decompiled source.
 *
 * Thread safety:
 *   - Enchantment registry is static/const after initialization.
 *   - EnchantmentHelper methods are stateless (thread-safe).
 *   - Thread-local RNG for random enchantment selection.
 *
 * JNI readiness: Integer enchantment IDs for easy JVM mapping.
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <random>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EnumEnchantmentType — Item category filter for enchantments.
// Java reference: net.minecraft.enchantment.EnumEnchantmentType
// ═══════════════════════════════════════════════════════════════════════════

enum class EnchantmentType : int32_t {
    ALL,        // Any item
    ARMOR,      // All armor pieces
    ARMOR_FEET,
    ARMOR_LEGS,
    ARMOR_TORSO,
    ARMOR_HEAD,
    WEAPON,     // Swords
    DIGGER,     // Pickaxe, shovel, axe
    FISHING_ROD,
    BREAKABLE,  // Any item with durability
    BOW
};

// ═══════════════════════════════════════════════════════════════════════════
// EnumCreatureAttribute — Creature type for damage enchantments.
// Java reference: net.minecraft.entity.EnumCreatureAttribute
// ═══════════════════════════════════════════════════════════════════════════

enum class CreatureAttribute : int32_t {
    UNDEFINED = 0,
    UNDEAD    = 1,
    ARTHROPOD = 2
};

// ═══════════════════════════════════════════════════════════════════════════
// Enchantment — Base enchantment definition.
// Java reference: net.minecraft.enchantment.Enchantment
//
// Each enchantment has:
//   - effectId: unique ID (0-255)
//   - weight: rarity weight for random selection (higher = more common)
//   - type: item category it applies to
//   - minLevel/maxLevel: allowed level range
//   - minEnchantability/maxEnchantability: level cost range per level
// ═══════════════════════════════════════════════════════════════════════════

struct Enchantment {
    int32_t effectId;
    std::string name;
    int32_t weight;           // Rarity weight (1=rare, 10=common)
    EnchantmentType type;
    int32_t minLevel;
    int32_t maxLevel;

    // Enchantability range calculation
    // Java: getMinEnchantability(level), getMaxEnchantability(level)
    // Stored as coefficients: minEnch = base + perLevel * level
    // where base and perLevel vary per enchantment type
    int32_t minEnchBase;
    int32_t minEnchPerLevel;
    int32_t maxEnchBase;      // Added on top of min to get max
    int32_t maxEnchPerLevel;  // 0 means max = min + fixed offset

    // Protection type (for protection enchantments)
    int32_t protectionType;   // -1 = not a protection enchant
    // Damage type (for damage enchantments)
    int32_t damageType;       // -1 = not a damage enchant

    // Get minimum enchantability for a given level
    int32_t getMinEnchantability(int32_t level) const {
        return minEnchBase + minEnchPerLevel * level;
    }

    // Get maximum enchantability for a given level
    int32_t getMaxEnchantability(int32_t level) const {
        if (maxEnchPerLevel > 0) {
            return maxEnchBase + maxEnchPerLevel * level;
        }
        return getMinEnchantability(level) + maxEnchBase;
    }

    // Calculate damage modifier for protection enchants
    // Java: EnchantmentProtection.calcModifierDamage
    int32_t calcModifierDamage(int32_t level, int32_t damageSourceFlags) const;

    // Calculate damage bonus for weapon enchants
    // Java: EnchantmentDamage.func_152376_a
    float calcDamageBonus(int32_t level, CreatureAttribute creature) const;

    // Check if two enchantments can coexist
    // Java: Enchantment.canApplyTogether
    bool canApplyTogether(const Enchantment& other) const;
};

// ═══════════════════════════════════════════════════════════════════════════
// EnchantmentData — ID + level pair stored on items.
// Java reference: net.minecraft.enchantment.EnchantmentData
// ═══════════════════════════════════════════════════════════════════════════

struct EnchantmentData {
    int32_t enchId;
    int32_t level;
};

// ═══════════════════════════════════════════════════════════════════════════
// EnchantmentRegistry — Static registry of all vanilla enchantments.
// Java reference: net.minecraft.enchantment.Enchantment static initializer
//
// Thread safety: initialized once at startup, read-only after that.
// ═══════════════════════════════════════════════════════════════════════════

class EnchantmentRegistry {
public:
    // Initialize all vanilla enchantments
    static void init();

    // Get enchantment by ID (returns nullptr if invalid)
    static const Enchantment* getById(int32_t id);

    // Get all registered enchantments
    static const std::vector<Enchantment>& getAll();

    // Get enchantment count
    static int32_t getCount();

    // Enchantment IDs — matching Java's Enchantment static fields
    // Java: Enchantment.protection, etc.
    static constexpr int32_t PROTECTION           = 0;
    static constexpr int32_t FIRE_PROTECTION      = 1;
    static constexpr int32_t FEATHER_FALLING      = 2;
    static constexpr int32_t BLAST_PROTECTION     = 3;
    static constexpr int32_t PROJECTILE_PROTECTION = 4;
    static constexpr int32_t RESPIRATION          = 5;
    static constexpr int32_t AQUA_AFFINITY        = 6;
    static constexpr int32_t THORNS               = 7;
    static constexpr int32_t SHARPNESS            = 16;
    static constexpr int32_t SMITE                = 17;
    static constexpr int32_t BANE_OF_ARTHROPODS   = 18;
    static constexpr int32_t KNOCKBACK            = 19;
    static constexpr int32_t FIRE_ASPECT          = 20;
    static constexpr int32_t LOOTING              = 21;
    static constexpr int32_t EFFICIENCY           = 32;
    static constexpr int32_t SILK_TOUCH           = 33;
    static constexpr int32_t UNBREAKING           = 34;
    static constexpr int32_t FORTUNE              = 35;
    static constexpr int32_t POWER                = 48;
    static constexpr int32_t PUNCH                = 49;
    static constexpr int32_t FLAME                = 50;
    static constexpr int32_t INFINITY_ENCH        = 51;
    static constexpr int32_t LUCK_OF_THE_SEA      = 61;
    static constexpr int32_t LURE                 = 62;

private:
    static std::vector<Enchantment> enchantments_;
    static bool initialized_;
};

// ═══════════════════════════════════════════════════════════════════════════
// EnchantmentHelper — Utility functions for enchantment operations.
// Java reference: net.minecraft.enchantment.EnchantmentHelper
//
// All methods are stateless and thread-safe.
// ═══════════════════════════════════════════════════════════════════════════

class EnchantmentHelper {
public:
    // Java: EnchantmentHelper.getEnchantmentLevel(int, ItemStack)
    // Get enchantment level from a list of enchantments on an item
    static int32_t getEnchantmentLevel(int32_t enchId,
                                        const std::vector<EnchantmentData>& enchants);

    // Java: EnchantmentHelper.getMaxEnchantmentLevel(int, ItemStack[])
    // Get highest level of an enchantment across multiple items
    static int32_t getMaxEnchantmentLevel(int32_t enchId,
                                            const std::vector<std::vector<EnchantmentData>>& itemEnchants);

    // Java: EnchantmentHelper.getEnchantmentModifierDamage(ItemStack[], DamageSource)
    // Calculate total protection modifier from all armor enchantments
    // damageFlags: bitmask of damage type (fire=1, fall=2, blast=4, projectile=8)
    static int32_t getEnchantmentModifierDamage(
        const std::vector<std::vector<EnchantmentData>>& armorEnchants,
        int32_t damageFlags);

    // Java: EnchantmentHelper.func_152377_a(ItemStack, EnumCreatureAttribute)
    // Calculate damage bonus from weapon enchantments
    static float getDamageBonus(const std::vector<EnchantmentData>& enchants,
                                 CreatureAttribute creature);

    // Java: EnchantmentHelper.getKnockbackModifier
    static int32_t getKnockbackModifier(const std::vector<EnchantmentData>& enchants);

    // Java: EnchantmentHelper.getFireAspectModifier
    static int32_t getFireAspectModifier(const std::vector<EnchantmentData>& enchants);

    // Java: EnchantmentHelper.getRespiration
    static int32_t getRespiration(const std::vector<std::vector<EnchantmentData>>& armorEnchants);

    // Java: EnchantmentHelper.getEfficiencyModifier
    static int32_t getEfficiencyModifier(const std::vector<EnchantmentData>& enchants);

    // Java: EnchantmentHelper.getSilkTouchModifier
    static bool getSilkTouchModifier(const std::vector<EnchantmentData>& enchants);

    // Java: EnchantmentHelper.getFortuneModifier
    static int32_t getFortuneModifier(const std::vector<EnchantmentData>& enchants);

    // Java: EnchantmentHelper.getLootingModifier
    static int32_t getLootingModifier(const std::vector<EnchantmentData>& enchants);

    // Java: EnchantmentHelper.getAquaAffinityModifier
    static bool getAquaAffinityModifier(const std::vector<std::vector<EnchantmentData>>& armorEnchants);

    // Java: EnchantmentHelper.func_151386_g (Luck of the Sea)
    static int32_t getLuckOfTheSeaModifier(const std::vector<EnchantmentData>& enchants);

    // Java: EnchantmentHelper.func_151387_h (Lure)
    static int32_t getLureModifier(const std::vector<EnchantmentData>& enchants);

    // Java: EnchantmentHelper.calcItemStackEnchantability
    // Calculate the enchantment power based on slot, bookshelves, and item
    static int32_t calcItemStackEnchantability(int32_t slot, int32_t bookshelves,
                                                 int32_t itemEnchantability);

    // Java: EnchantmentHelper.buildEnchantmentList
    // Build possible enchantments for an item at a given power level
    static std::vector<EnchantmentData> buildEnchantmentList(
        int32_t enchantPower, int32_t itemEnchantability,
        EnchantmentType itemType);

    // Java: EnchantmentHelper.addRandomEnchantment
    // Apply random enchantments to an item
    static std::vector<EnchantmentData> addRandomEnchantment(
        int32_t enchantPower, int32_t itemEnchantability,
        EnchantmentType itemType);

    // Java: EnchantmentHelper.calcModifierDamage for thorns
    // Thorns: 15% * level chance to deal 1-4 damage to attacker
    static int32_t calcThornsDamage(int32_t thornsLevel);

    // Check if thorns triggers at the given level
    static bool shouldApplyThorns(int32_t thornsLevel);
};

} // namespace mccpp
