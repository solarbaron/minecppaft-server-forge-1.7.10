/**
 * Enchantment.h — Enchantment registry with all vanilla enchantments.
 *
 * Java reference: net.minecraft.enchantment.Enchantment
 *
 * 24 vanilla enchantments, IDs 0-62 (sparse), weights 1-10.
 * Types: armor, armor_feet, armor_legs, armor_head, weapon, digger, fishing_rod, breakable, bow
 *
 * Thread safety: Static/const after init.
 *
 * JNI readiness: Integer IDs, string-based names.
 */
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EnchantmentType — What items this enchantment can apply to.
// Java reference: net.minecraft.enchantment.EnumEnchantmentType
// ═══════════════════════════════════════════════════════════════════════════

enum class EnchantmentType : int32_t {
    ALL,         // All items
    ARMOR,       // Any armor piece
    ARMOR_FEET,  // Boots only
    ARMOR_LEGS,  // Leggings only
    ARMOR_HEAD,  // Helmet only
    WEAPON,      // Swords
    DIGGER,      // Pickaxe, shovel, axe
    FISHING_ROD, // Fishing rod
    BREAKABLE,   // Any item with durability
    BOW          // Bow
};

// ═══════════════════════════════════════════════════════════════════════════
// EnchantmentData — One vanilla enchantment definition.
// Java reference: net.minecraft.enchantment.Enchantment
// ═══════════════════════════════════════════════════════════════════════════

struct EnchantmentData {
    int32_t id;
    std::string name;        // e.g., "protection"
    std::string unlocalizedName; // e.g., "enchantment.protect.all"
    int32_t weight;          // Rarity weight (1=rare, 10=common)
    int32_t maxLevel;        // Max enchantment level
    int32_t minEnchantBase;  // Base min enchantability
    int32_t minEnchantPerLevel; // Min enchantability increase per level
    int32_t maxEnchantBase;  // Base max enchantability
    int32_t maxEnchantPerLevel; // Max enchantability increase per level
    EnchantmentType type;

    // Java: getMinEnchantability
    int32_t getMinEnchantability(int32_t level) const {
        return minEnchantBase + (level - 1) * minEnchantPerLevel;
    }

    // Java: getMaxEnchantability
    int32_t getMaxEnchantability(int32_t level) const {
        return maxEnchantBase + (level - 1) * maxEnchantPerLevel;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Enchantment IDs — Constants matching Java.
// Java reference: net.minecraft.enchantment.Enchantment static fields
// ═══════════════════════════════════════════════════════════════════════════

namespace EnchantmentID {
    constexpr int32_t PROTECTION          = 0;
    constexpr int32_t FIRE_PROTECTION     = 1;
    constexpr int32_t FEATHER_FALLING     = 2;
    constexpr int32_t BLAST_PROTECTION    = 3;
    constexpr int32_t PROJECTILE_PROTECTION = 4;
    constexpr int32_t RESPIRATION         = 5;
    constexpr int32_t AQUA_AFFINITY       = 6;
    constexpr int32_t THORNS              = 7;
    constexpr int32_t SHARPNESS           = 16;
    constexpr int32_t SMITE              = 17;
    constexpr int32_t BANE_OF_ARTHROPODS  = 18;
    constexpr int32_t KNOCKBACK           = 19;
    constexpr int32_t FIRE_ASPECT         = 20;
    constexpr int32_t LOOTING             = 21;
    constexpr int32_t EFFICIENCY          = 32;
    constexpr int32_t SILK_TOUCH          = 33;
    constexpr int32_t UNBREAKING          = 34;
    constexpr int32_t FORTUNE             = 35;
    constexpr int32_t POWER              = 48;
    constexpr int32_t PUNCH              = 49;
    constexpr int32_t FLAME              = 50;
    constexpr int32_t INFINITY           = 51;
    constexpr int32_t LUCK_OF_THE_SEA    = 61;
    constexpr int32_t LURE               = 62;
}

// ═══════════════════════════════════════════════════════════════════════════
// EnchantmentRegistry — All 24 vanilla enchantments.
// Java reference: net.minecraft.enchantment.Enchantment static initializer
// ═══════════════════════════════════════════════════════════════════════════

class EnchantmentRegistry {
public:
    static void init();

    static const EnchantmentData* getById(int32_t id);
    static const EnchantmentData* getByName(const std::string& name);
    static const std::vector<EnchantmentData>& getAll();
    static int32_t getCount();

    // Java: canApplyTogether — most enchantments are compatible except:
    // - Protection types are mutually exclusive (0,1,3,4)
    // - Damage types are mutually exclusive (16,17,18)
    // - Silk Touch and Fortune are mutually exclusive (33,35)
    static bool canApplyTogether(int32_t id1, int32_t id2);

private:
    static std::vector<EnchantmentData> enchantments_;
    static std::unordered_map<int32_t, size_t> idIndex_;
    static std::unordered_map<std::string, size_t> nameIndex_;
    static bool initialized_;
};

} // namespace mccpp
