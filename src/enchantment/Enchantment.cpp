/**
 * Enchantment.cpp — All 24 vanilla enchantment registrations.
 *
 * Java reference: net.minecraft.enchantment.Enchantment static initializer
 *
 * Exact IDs, weights, max levels, and enchantability formulas from:
 *   EnchantmentProtection, EnchantmentDamage, EnchantmentDigging,
 *   EnchantmentDurability, EnchantmentUntouching, EnchantmentLootBonus,
 *   EnchantmentKnockback, EnchantmentFireAspect, EnchantmentOxygen,
 *   EnchantmentWaterWorker, EnchantmentThorns, EnchantmentArrowDamage,
 *   EnchantmentArrowKnockback, EnchantmentArrowFire, EnchantmentArrowInfinite,
 *   EnchantmentFishingSpeed
 */

#include "enchantment/Enchantment.h"
#include <iostream>

namespace mccpp {

std::vector<EnchantmentData> EnchantmentRegistry::enchantments_;
std::unordered_map<int32_t, size_t> EnchantmentRegistry::idIndex_;
std::unordered_map<std::string, size_t> EnchantmentRegistry::nameIndex_;
bool EnchantmentRegistry::initialized_ = false;

void EnchantmentRegistry::init() {
    if (initialized_) return;

    // All 24 vanilla enchantments
    // {id, name, unlocalizedName, weight, maxLevel, minEnchBase, minEnchPerLvl, maxEnchBase, maxEnchPerLvl, type}
    static const EnchantmentData allEnchants[] = {
        // ─── Protection enchantments (armor) ───
        // Java: EnchantmentProtection — protection types 0-4
        { 0, "protection",           "enchantment.protect.all",        10, 4,  1, 11, 12, 11, EnchantmentType::ARMOR},
        { 1, "fire_protection",      "enchantment.protect.fire",        5, 4, 10,  8, 18,  8, EnchantmentType::ARMOR},
        { 2, "feather_falling",      "enchantment.protect.fall",        5, 4,  5,  6, 11,  6, EnchantmentType::ARMOR_FEET},
        { 3, "blast_protection",     "enchantment.protect.explosion",   2, 4, 12,  8, 20,  8, EnchantmentType::ARMOR},
        { 4, "projectile_protection","enchantment.protect.projectile",  5, 4,  3,  6,  9,  6, EnchantmentType::ARMOR},

        // ─── Armor utility ───
        { 5, "respiration",          "enchantment.oxygen",              2, 3, 10, 10, 40, 10, EnchantmentType::ARMOR_HEAD},
        { 6, "aqua_affinity",        "enchantment.waterWorker",         2, 1,  1,  0, 41,  0, EnchantmentType::ARMOR_HEAD},
        { 7, "thorns",               "enchantment.thorns",              1, 3, 10, 20, 60, 20, EnchantmentType::ARMOR},

        // ─── Weapon enchantments ───
        // Java: EnchantmentDamage — damage types 0-2
        {16, "sharpness",            "enchantment.damage.all",         10, 5,  1, 11, 21, 11, EnchantmentType::WEAPON},
        {17, "smite",                "enchantment.damage.undead",       5, 5,  5,  8, 25,  8, EnchantmentType::WEAPON},
        {18, "bane_of_arthropods",   "enchantment.damage.arthropods",   5, 5,  5,  8, 25,  8, EnchantmentType::WEAPON},
        {19, "knockback",            "enchantment.knockback",           5, 2,  5, 20, 55, 20, EnchantmentType::WEAPON},
        {20, "fire_aspect",          "enchantment.fire",                2, 2, 10, 20, 60, 20, EnchantmentType::WEAPON},
        {21, "looting",              "enchantment.lootBonus",           2, 3, 15,  9, 65,  9, EnchantmentType::WEAPON},

        // ─── Tool enchantments ───
        {32, "efficiency",           "enchantment.digging",            10, 5,  1, 10, 51, 10, EnchantmentType::DIGGER},
        {33, "silk_touch",           "enchantment.untouching",          1, 1, 15,  0, 65,  0, EnchantmentType::DIGGER},
        {34, "unbreaking",           "enchantment.durability",          5, 3,  5,  8, 55,  8, EnchantmentType::BREAKABLE},
        {35, "fortune",              "enchantment.lootBonusDigger",     2, 3, 15,  9, 65,  9, EnchantmentType::DIGGER},

        // ─── Bow enchantments ───
        {48, "power",                "enchantment.arrowDamage",        10, 5,  1, 10, 16, 10, EnchantmentType::BOW},
        {49, "punch",                "enchantment.arrowKnockback",      2, 2, 12, 20, 37, 20, EnchantmentType::BOW},
        {50, "flame",                "enchantment.arrowFire",           2, 1, 20,  0, 50,  0, EnchantmentType::BOW},
        {51, "infinity",             "enchantment.arrowInfinite",       1, 1, 20,  0, 50,  0, EnchantmentType::BOW},

        // ─── Fishing rod enchantments ───
        {61, "luck_of_the_sea",      "enchantment.lootBonusFishing",    2, 3, 15,  9, 65,  9, EnchantmentType::FISHING_ROD},
        {62, "lure",                 "enchantment.fishingSpeed",        2, 3, 15,  9, 65,  9, EnchantmentType::FISHING_ROD},
    };

    int32_t count = static_cast<int32_t>(sizeof(allEnchants) / sizeof(allEnchants[0]));
    enchantments_.reserve(count);

    for (int32_t i = 0; i < count; ++i) {
        enchantments_.push_back(allEnchants[i]);
        idIndex_[allEnchants[i].id] = i;
        nameIndex_[allEnchants[i].name] = i;
    }

    initialized_ = true;
    std::cout << "[Enchantment] Registered " << enchantments_.size() << " enchantments\n";
}

const EnchantmentData* EnchantmentRegistry::getById(int32_t id) {
    auto it = idIndex_.find(id);
    return it != idIndex_.end() ? &enchantments_[it->second] : nullptr;
}

const EnchantmentData* EnchantmentRegistry::getByName(const std::string& name) {
    auto it = nameIndex_.find(name);
    return it != nameIndex_.end() ? &enchantments_[it->second] : nullptr;
}

const std::vector<EnchantmentData>& EnchantmentRegistry::getAll() {
    return enchantments_;
}

int32_t EnchantmentRegistry::getCount() {
    return static_cast<int32_t>(enchantments_.size());
}

bool EnchantmentRegistry::canApplyTogether(int32_t id1, int32_t id2) {
    if (id1 == id2) return false;

    // Java: Protection types are mutually exclusive (0,1,3,4)
    // Note: feather falling (2) is compatible with other protections
    auto isProtection = [](int32_t id) { return id == 0 || id == 1 || id == 3 || id == 4; };
    if (isProtection(id1) && isProtection(id2)) return false;

    // Java: Damage types are mutually exclusive (16,17,18)
    auto isDamage = [](int32_t id) { return id >= 16 && id <= 18; };
    if (isDamage(id1) && isDamage(id2)) return false;

    // Java: Silk Touch (33) and Fortune (35) are mutually exclusive
    if ((id1 == 33 && id2 == 35) || (id1 == 35 && id2 == 33)) return false;

    return true;
}

} // namespace mccpp
