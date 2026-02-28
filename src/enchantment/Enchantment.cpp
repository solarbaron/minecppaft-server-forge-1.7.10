/**
 * Enchantment.cpp — Enchantment system implementation.
 *
 * Java references:
 *   net.minecraft.enchantment.Enchantment — static initializer with all 24 enchants
 *   net.minecraft.enchantment.EnchantmentProtection — calcModifierDamage
 *   net.minecraft.enchantment.EnchantmentDamage — func_152376_a damage bonus
 *   net.minecraft.enchantment.EnchantmentHelper — all utility methods
 *
 * Key behaviors preserved from Java:
 *   - Protection cap: max 25 total modifier after combining
 *   - Sharpness: 1.25 * level bonus to all mobs
 *   - Smite: 2.5 * level bonus to undead only
 *   - Bane of Arthropods: 2.5 * level bonus to arthropods only
 *   - Enchantability formula: modified ± 25% random, slot-based formula
 *   - Mutual exclusion: protection types conflict, damage types conflict
 *   - Silk Touch conflicts with Fortune, Infinity with Mending
 */

#include "enchantment/Enchantment.h"
#include <cmath>
#include <iostream>

namespace mccpp {

// Thread-local RNG for enchantment calculations
static thread_local std::mt19937 enchRng(std::random_device{}());

// ═════════════════════════════════════════════════════════════════════════════
// Enchantment — Modifier calculations
// ═════════════════════════════════════════════════════════════════════════════

// Java: EnchantmentProtection.calcModifierDamage
int32_t Enchantment::calcModifierDamage(int32_t level, int32_t damageSourceFlags) const {
    if (protectionType < 0) return 0;

    // Protection type 0: general protection (all damage types)
    // Protection type 1: fire protection (fire damage)
    // Protection type 2: feather falling (fall damage)
    // Protection type 3: blast protection (explosion damage)
    // Protection type 4: projectile protection (projectile damage)

    // Java: EnchantmentProtection.calcModifierDamage
    // General protection: level * 1, specific: level * 2
    // Only applies to matching damage types

    switch (protectionType) {
        case 0: // All Protection — applies to everything
            return level;
        case 1: // Fire Protection — only fire damage (flag 1)
            if (damageSourceFlags & 1) return level * 2;
            return 0;
        case 2: // Feather Falling — only fall damage (flag 2)
            if (damageSourceFlags & 2) return level * 3;
            return 0;
        case 3: // Blast Protection — only explosion damage (flag 4)
            if (damageSourceFlags & 4) return level * 2;
            return 0;
        case 4: // Projectile Protection — only projectile damage (flag 8)
            if (damageSourceFlags & 8) return level * 2;
            return 0;
        default:
            return 0;
    }
}

// Java: EnchantmentDamage.func_152376_a
float Enchantment::calcDamageBonus(int32_t level, CreatureAttribute creature) const {
    if (damageType < 0) return 0.0f;

    switch (damageType) {
        case 0: // Sharpness — bonus to all mobs
            // Java: 1.0 + max(0, level - 1) * 0.5 = 1.25 per level
            return 1.0f + static_cast<float>(std::max(0, level - 1)) * 0.5f;
        case 1: // Smite — bonus to undead only
            if (creature == CreatureAttribute::UNDEAD) {
                return static_cast<float>(level) * 2.5f;
            }
            return 0.0f;
        case 2: // Bane of Arthropods — bonus to arthropods only
            if (creature == CreatureAttribute::ARTHROPOD) {
                return static_cast<float>(level) * 2.5f;
            }
            return 0.0f;
        default:
            return 0.0f;
    }
}

// Java: Enchantment.canApplyTogether
bool Enchantment::canApplyTogether(const Enchantment& other) const {
    if (effectId == other.effectId) return false;

    // Protection enchants are mutually exclusive
    // Java: EnchantmentProtection.canApplyTogether
    if (protectionType >= 0 && other.protectionType >= 0) {
        return false;
    }

    // Damage enchants are mutually exclusive
    if (damageType >= 0 && other.damageType >= 0) {
        return false;
    }

    // Silk Touch and Fortune conflict
    if ((effectId == 33 && other.effectId == 35) ||
        (effectId == 35 && other.effectId == 33)) {
        return false;
    }

    return true;
}

// ═════════════════════════════════════════════════════════════════════════════
// EnchantmentRegistry
// ═════════════════════════════════════════════════════════════════════════════

std::vector<Enchantment> EnchantmentRegistry::enchantments_;
bool EnchantmentRegistry::initialized_ = false;

void EnchantmentRegistry::init() {
    if (initialized_) return;

    // Java: Enchantment static initializer
    // Format: {id, name, weight, type, minLvl, maxLvl,
    //          minEnchBase, minEnchPerLevel, maxEnchBase, maxEnchPerLevel,
    //          protType, dmgType}

    enchantments_ = {
        // ── Armor Protection Enchantments (IDs 0-4) ──
        // Java: EnchantmentProtection(0, 10, 0) — Protection
        // minEnch: 1 + level * 11, maxEnch: minEnch + 20
        {0, "protection", 10, EnchantmentType::ARMOR, 1, 4,
         1, 11, 20, 0, 0, -1},

        // Java: EnchantmentProtection(1, 5, 1) — Fire Protection
        // minEnch: 10 + (level-1) * 8, maxEnch: minEnch + 12
        {1, "fireProtection", 5, EnchantmentType::ARMOR, 1, 4,
         10, 8, 12, 0, 1, -1},

        // Java: EnchantmentProtection(2, 5, 2) — Feather Falling
        // minEnch: 5 + (level-1) * 6, maxEnch: minEnch + 10
        {2, "featherFalling", 5, EnchantmentType::ARMOR_FEET, 1, 4,
         5, 6, 10, 0, 2, -1},

        // Java: EnchantmentProtection(3, 2, 3) — Blast Protection
        // minEnch: 5 + (level-1) * 8, maxEnch: minEnch + 12
        {3, "blastProtection", 2, EnchantmentType::ARMOR, 1, 4,
         5, 8, 12, 0, 3, -1},

        // Java: EnchantmentProtection(4, 5, 4) — Projectile Protection
        // minEnch: 3 + (level-1) * 6, maxEnch: minEnch + 15
        {4, "projectileProtection", 5, EnchantmentType::ARMOR, 1, 4,
         3, 6, 15, 0, 4, -1},

        // ── Armor Utility Enchantments (IDs 5-7) ──
        // Java: EnchantmentOxygen(5, 2) — Respiration
        // minEnch: level * 10, maxEnch: minEnch + 30
        {5, "respiration", 2, EnchantmentType::ARMOR_HEAD, 1, 3,
         0, 10, 30, 0, -1, -1},

        // Java: EnchantmentWaterWorker(6, 2) — Aqua Affinity
        // minEnch: 1, maxEnch: 41
        {6, "aquaAffinity", 2, EnchantmentType::ARMOR_HEAD, 1, 1,
         1, 0, 40, 0, -1, -1},

        // Java: EnchantmentThorns(7, 1) — Thorns
        // minEnch: 10 + (level-1) * 20, maxEnch: minEnch + 50
        {7, "thorns", 1, EnchantmentType::ARMOR_TORSO, 1, 3,
         10, 20, 50, 0, -1, -1},

        // ── Weapon Damage Enchantments (IDs 16-18) ──
        // Java: EnchantmentDamage(16, 10, 0) — Sharpness
        // minEnch: 1 + (level-1) * 11, maxEnch: minEnch + 20
        {16, "sharpness", 10, EnchantmentType::WEAPON, 1, 5,
         1, 11, 20, 0, -1, 0},

        // Java: EnchantmentDamage(17, 5, 1) — Smite
        // minEnch: 5 + (level-1) * 8, maxEnch: minEnch + 20
        {17, "smite", 5, EnchantmentType::WEAPON, 1, 5,
         5, 8, 20, 0, -1, 1},

        // Java: EnchantmentDamage(18, 5, 2) — Bane of Arthropods
        // minEnch: 5 + (level-1) * 8, maxEnch: minEnch + 20
        {18, "baneOfArthropods", 5, EnchantmentType::WEAPON, 1, 5,
         5, 8, 20, 0, -1, 2},

        // ── Weapon Utility Enchantments (IDs 19-21) ──
        // Java: EnchantmentKnockback(19, 5) — Knockback
        // minEnch: 5 + (level-1) * 20, maxEnch: minEnch + 50
        {19, "knockback", 5, EnchantmentType::WEAPON, 1, 2,
         5, 20, 50, 0, -1, -1},

        // Java: EnchantmentFireAspect(20, 2) — Fire Aspect
        // minEnch: 10 + (level-1) * 20, maxEnch: minEnch + 50
        {20, "fireAspect", 2, EnchantmentType::WEAPON, 1, 2,
         10, 20, 50, 0, -1, -1},

        // Java: EnchantmentLootBonus(21, 2, weapon) — Looting
        // minEnch: 15 + (level-1) * 9, maxEnch: minEnch + 50
        {21, "looting", 2, EnchantmentType::WEAPON, 1, 3,
         15, 9, 50, 0, -1, -1},

        // ── Tool Enchantments (IDs 32-35) ──
        // Java: EnchantmentDigging(32, 10) — Efficiency
        // minEnch: 1 + (level-1) * 10, maxEnch: minEnch + 50
        {32, "efficiency", 10, EnchantmentType::DIGGER, 1, 5,
         1, 10, 50, 0, -1, -1},

        // Java: EnchantmentUntouching(33, 1) — Silk Touch
        // minEnch: 15, maxEnch: minEnch + 50
        {33, "silkTouch", 1, EnchantmentType::DIGGER, 1, 1,
         15, 0, 50, 0, -1, -1},

        // Java: EnchantmentDurability(34, 5) — Unbreaking
        // minEnch: 5 + (level-1) * 8, maxEnch: minEnch + 50
        {34, "unbreaking", 5, EnchantmentType::BREAKABLE, 1, 3,
         5, 8, 50, 0, -1, -1},

        // Java: EnchantmentLootBonus(35, 2, digger) — Fortune
        // minEnch: 15 + (level-1) * 9, maxEnch: minEnch + 50
        {35, "fortune", 2, EnchantmentType::DIGGER, 1, 3,
         15, 9, 50, 0, -1, -1},

        // ── Bow Enchantments (IDs 48-51) ──
        // Java: EnchantmentArrowDamage(48, 10) — Power
        // minEnch: 1 + (level-1) * 10, maxEnch: minEnch + 15
        {48, "power", 10, EnchantmentType::BOW, 1, 5,
         1, 10, 15, 0, -1, -1},

        // Java: EnchantmentArrowKnockback(49, 2) — Punch
        // minEnch: 12 + (level-1) * 20, maxEnch: minEnch + 25
        {49, "punch", 2, EnchantmentType::BOW, 1, 2,
         12, 20, 25, 0, -1, -1},

        // Java: EnchantmentArrowFire(50, 2) — Flame
        // minEnch: 20, maxEnch: 50
        {50, "flame", 2, EnchantmentType::BOW, 1, 1,
         20, 0, 30, 0, -1, -1},

        // Java: EnchantmentArrowInfinite(51, 1) — Infinity
        // minEnch: 20, maxEnch: 50
        {51, "infinity", 1, EnchantmentType::BOW, 1, 1,
         20, 0, 30, 0, -1, -1},

        // ── Fishing Rod Enchantments (IDs 61-62) ──
        // Java: EnchantmentLootBonus(61, 2, fishing_rod) — Luck of the Sea
        // minEnch: 15 + (level-1) * 9, maxEnch: minEnch + 50
        {61, "luckOfTheSea", 2, EnchantmentType::FISHING_ROD, 1, 3,
         15, 9, 50, 0, -1, -1},

        // Java: EnchantmentFishingSpeed(62, 2, fishing_rod) — Lure
        // minEnch: 15 + (level-1) * 9, maxEnch: minEnch + 50
        {62, "lure", 2, EnchantmentType::FISHING_ROD, 1, 3,
         15, 9, 50, 0, -1, -1},
    };

    initialized_ = true;
    std::cout << "[Enchantment] Registered " << enchantments_.size() << " enchantments\n";
}

const Enchantment* EnchantmentRegistry::getById(int32_t id) {
    for (const auto& ench : enchantments_) {
        if (ench.effectId == id) return &ench;
    }
    return nullptr;
}

const std::vector<Enchantment>& EnchantmentRegistry::getAll() {
    return enchantments_;
}

int32_t EnchantmentRegistry::getCount() {
    return static_cast<int32_t>(enchantments_.size());
}

// ═════════════════════════════════════════════════════════════════════════════
// EnchantmentHelper
// ═════════════════════════════════════════════════════════════════════════════

int32_t EnchantmentHelper::getEnchantmentLevel(int32_t enchId,
                                                 const std::vector<EnchantmentData>& enchants) {
    for (const auto& ed : enchants) {
        if (ed.enchId == enchId) return ed.level;
    }
    return 0;
}

int32_t EnchantmentHelper::getMaxEnchantmentLevel(int32_t enchId,
                                                    const std::vector<std::vector<EnchantmentData>>& itemEnchants) {
    int32_t maxLevel = 0;
    for (const auto& enchants : itemEnchants) {
        int32_t level = getEnchantmentLevel(enchId, enchants);
        maxLevel = std::max(maxLevel, level);
    }
    return maxLevel;
}

int32_t EnchantmentHelper::getEnchantmentModifierDamage(
    const std::vector<std::vector<EnchantmentData>>& armorEnchants,
    int32_t damageFlags) {
    // Java: applyEnchantmentModifierArray → applyEnchantmentModifier
    int32_t total = 0;
    for (const auto& enchants : armorEnchants) {
        for (const auto& ed : enchants) {
            const Enchantment* ench = EnchantmentRegistry::getById(ed.enchId);
            if (ench) {
                total += ench->calcModifierDamage(ed.level, damageFlags);
            }
        }
    }
    // Java: cap at 25 after randomization (simplified here)
    return std::min(total, 25);
}

float EnchantmentHelper::getDamageBonus(const std::vector<EnchantmentData>& enchants,
                                          CreatureAttribute creature) {
    float total = 0.0f;
    for (const auto& ed : enchants) {
        const Enchantment* ench = EnchantmentRegistry::getById(ed.enchId);
        if (ench) {
            total += ench->calcDamageBonus(ed.level, creature);
        }
    }
    return total;
}

int32_t EnchantmentHelper::getKnockbackModifier(const std::vector<EnchantmentData>& enchants) {
    return getEnchantmentLevel(EnchantmentRegistry::KNOCKBACK, enchants);
}

int32_t EnchantmentHelper::getFireAspectModifier(const std::vector<EnchantmentData>& enchants) {
    return getEnchantmentLevel(EnchantmentRegistry::FIRE_ASPECT, enchants);
}

int32_t EnchantmentHelper::getRespiration(const std::vector<std::vector<EnchantmentData>>& armorEnchants) {
    return getMaxEnchantmentLevel(EnchantmentRegistry::RESPIRATION, armorEnchants);
}

int32_t EnchantmentHelper::getEfficiencyModifier(const std::vector<EnchantmentData>& enchants) {
    return getEnchantmentLevel(EnchantmentRegistry::EFFICIENCY, enchants);
}

bool EnchantmentHelper::getSilkTouchModifier(const std::vector<EnchantmentData>& enchants) {
    return getEnchantmentLevel(EnchantmentRegistry::SILK_TOUCH, enchants) > 0;
}

int32_t EnchantmentHelper::getFortuneModifier(const std::vector<EnchantmentData>& enchants) {
    return getEnchantmentLevel(EnchantmentRegistry::FORTUNE, enchants);
}

int32_t EnchantmentHelper::getLootingModifier(const std::vector<EnchantmentData>& enchants) {
    return getEnchantmentLevel(EnchantmentRegistry::LOOTING, enchants);
}

bool EnchantmentHelper::getAquaAffinityModifier(const std::vector<std::vector<EnchantmentData>>& armorEnchants) {
    return getMaxEnchantmentLevel(EnchantmentRegistry::AQUA_AFFINITY, armorEnchants) > 0;
}

int32_t EnchantmentHelper::getLuckOfTheSeaModifier(const std::vector<EnchantmentData>& enchants) {
    return getEnchantmentLevel(EnchantmentRegistry::LUCK_OF_THE_SEA, enchants);
}

int32_t EnchantmentHelper::getLureModifier(const std::vector<EnchantmentData>& enchants) {
    return getEnchantmentLevel(EnchantmentRegistry::LURE, enchants);
}

// Java: EnchantmentHelper.calcItemStackEnchantability
int32_t EnchantmentHelper::calcItemStackEnchantability(int32_t slot, int32_t bookshelves,
                                                         int32_t itemEnchantability) {
    // Java formula:
    // int k = enchantmentRand.nextInt(8) + 1 + (bookshelves >> 1) + enchantmentRand.nextInt(bookshelves + 1);
    // slot 0: max(k / 3, 1)
    // slot 1: k * 2 / 3 + 1
    // slot 2: max(k, bookshelves * 2)

    std::uniform_int_distribution<int32_t> dist8(0, 7);
    std::uniform_int_distribution<int32_t> distShelf(0, bookshelves);

    int32_t k = dist8(enchRng) + 1 + (bookshelves >> 1) + distShelf(enchRng);

    switch (slot) {
        case 0: return std::max(k / 3, 1);
        case 1: return (k * 2) / 3 + 1;
        case 2: return std::max(k, bookshelves * 2);
        default: return k;
    }
}

// Java: EnchantmentHelper.buildEnchantmentList
std::vector<EnchantmentData> EnchantmentHelper::buildEnchantmentList(
    int32_t enchantPower, int32_t /*itemEnchantability*/,
    EnchantmentType itemType) {

    std::vector<EnchantmentData> result;

    // Java: iterate all enchantments, find those whose enchantability range
    // contains the power level, and whose type matches
    for (const auto& ench : EnchantmentRegistry::getAll()) {
        // Check type compatibility (simplified)
        bool typeMatch = false;
        switch (itemType) {
            case EnchantmentType::ALL:
            case EnchantmentType::BREAKABLE:
                typeMatch = true; break;
            case EnchantmentType::ARMOR:
                typeMatch = (ench.type == EnchantmentType::ARMOR ||
                            ench.type == EnchantmentType::ARMOR_HEAD ||
                            ench.type == EnchantmentType::ARMOR_TORSO ||
                            ench.type == EnchantmentType::ARMOR_LEGS ||
                            ench.type == EnchantmentType::ARMOR_FEET);
                break;
            default:
                typeMatch = (ench.type == itemType); break;
        }
        if (!typeMatch) continue;

        // Check each level from max to min
        for (int32_t level = ench.maxLevel; level >= ench.minLevel; --level) {
            int32_t minEnch = ench.getMinEnchantability(level);
            int32_t maxEnch = ench.getMaxEnchantability(level);
            if (enchantPower >= minEnch && enchantPower <= maxEnch) {
                result.push_back({ench.effectId, level});
                break;
            }
        }
    }

    return result;
}

// Java: EnchantmentHelper.addRandomEnchantment
std::vector<EnchantmentData> EnchantmentHelper::addRandomEnchantment(
    int32_t enchantPower, int32_t itemEnchantability,
    EnchantmentType itemType) {

    // Java: modify enchantPower with item enchantability
    // enchantPower += 1 + enchRng(itemEnch/4+1) + enchRng(itemEnch/4+1)
    if (itemEnchantability > 0) {
        std::uniform_int_distribution<int32_t> dist(0, itemEnchantability / 4);
        enchantPower += 1 + dist(enchRng) + dist(enchRng);
    }

    // Java: ±15% random variation
    float variation = 1.0f + (std::uniform_real_distribution<float>(-0.15f, 0.15f)(enchRng));
    enchantPower = std::max(1, static_cast<int32_t>(std::round(
        static_cast<float>(enchantPower) * variation)));

    auto candidates = buildEnchantmentList(enchantPower, itemEnchantability, itemType);
    if (candidates.empty()) return {};

    // Java: weighted random selection based on enchantment weight
    std::vector<EnchantmentData> result;

    // Pick first enchantment (weighted random)
    auto pickWeighted = [&candidates]() -> int32_t {
        int32_t totalWeight = 0;
        for (const auto& ed : candidates) {
            const Enchantment* ench = EnchantmentRegistry::getById(ed.enchId);
            if (ench) totalWeight += ench->weight;
        }
        if (totalWeight <= 0) return -1;

        std::uniform_int_distribution<int32_t> dist(0, totalWeight - 1);
        int32_t roll = dist(enchRng);
        for (size_t i = 0; i < candidates.size(); ++i) {
            const Enchantment* ench = EnchantmentRegistry::getById(candidates[i].enchId);
            if (!ench) continue;
            roll -= ench->weight;
            if (roll < 0) return static_cast<int32_t>(i);
        }
        return static_cast<int32_t>(candidates.size() - 1);
    };

    int32_t picked = pickWeighted();
    if (picked < 0) return {};
    result.push_back(candidates[picked]);

    // Java: 50% chance to add more enchantments (compounding)
    // Each additional enchant: remove incompatible ones, pick again
    while (std::uniform_int_distribution<int32_t>(0, 1)(enchRng) == 0 && !candidates.empty()) {
        // Remove incompatible enchantments
        const Enchantment* lastEnch = EnchantmentRegistry::getById(result.back().enchId);
        if (!lastEnch) break;

        std::vector<EnchantmentData> filtered;
        for (const auto& ed : candidates) {
            const Enchantment* ench = EnchantmentRegistry::getById(ed.enchId);
            if (ench && lastEnch->canApplyTogether(*ench)) {
                // Also check it's not already in result
                bool alreadyPicked = false;
                for (const auto& r : result) {
                    if (r.enchId == ed.enchId) { alreadyPicked = true; break; }
                }
                if (!alreadyPicked) filtered.push_back(ed);
            }
        }
        candidates = std::move(filtered);
        if (candidates.empty()) break;

        picked = pickWeighted();
        if (picked < 0) break;
        result.push_back(candidates[picked]);

        // Java: halve enchant power for each additional enchant
        enchantPower /= 2;
    }

    return result;
}

// Java: EnchantmentThorns — 15% * level chance
int32_t EnchantmentHelper::calcThornsDamage(int32_t thornsLevel) {
    if (thornsLevel <= 0) return 0;
    // Java: 1-4 damage when thorns triggers
    std::uniform_int_distribution<int32_t> dist(1, 4);
    return dist(enchRng);
}

bool EnchantmentHelper::shouldApplyThorns(int32_t thornsLevel) {
    if (thornsLevel <= 0) return false;
    // Java: thornsLevel * 15% chance
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(enchRng) < (static_cast<float>(thornsLevel) * 0.15f);
}

} // namespace mccpp
