#pragma once
// EnchantmentRegistry — all vanilla 1.7.10 enchantments.
// Ported from bau.java (Enchantment) registry and enchanting table mechanics.
//
// Vanilla enchantment IDs: 0-62 (not all used)
// Each enchantment has: ID, name, max level, applicable item types, weight (rarity)
//
// Enchanting table mechanics (from qi.java / ContainerEnchantment):
//   - 3 slots with level requirements based on bookshelves (0-15)
//   - Slot costs: base+0..2, base+3..5, base*2+1..5
//   - Random enchantments selected based on level and item type

#include <cstdint>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <unordered_map>
#include <optional>
#include <algorithm>

namespace mc {

// Item category for enchantment applicability
enum class EnchantTarget {
    ALL,
    ARMOR,
    ARMOR_HEAD,
    ARMOR_CHEST,
    ARMOR_LEGS,
    ARMOR_FEET,
    SWORD,
    TOOL,      // Pickaxe, shovel, axe
    BOW,
    FISHING_ROD,
    BREAKABLE  // Anything with durability
};

// Enchantment rarity weight — higher = more common
enum class EnchantWeight {
    COMMON = 10,
    UNCOMMON = 5,
    RARE = 2,
    VERY_RARE = 1
};

struct Enchantment {
    int16_t id;
    std::string name;
    int maxLevel;
    EnchantTarget target;
    EnchantWeight weight;

    // Min/max modified enchantment level for each enchantment level
    // Used to determine if this enchantment can appear at a given power level
    int minPower(int level) const {
        // Default formula from vanilla — overridden per enchantment type
        return 1 + (level - 1) * 10;
    }

    int maxPower(int level) const {
        return minPower(level) + 5;
    }
};

// Enchantment instance on an item
struct EnchantmentInstance {
    int16_t id;
    int16_t level;
};

class EnchantmentRegistry {
public:
    EnchantmentRegistry() { registerAll(); }

    const Enchantment* getById(int16_t id) const {
        auto it = byId_.find(id);
        return it != byId_.end() ? &it->second : nullptr;
    }

    const std::vector<Enchantment>& all() const { return enchantments_; }

    // Check if two enchantments conflict (mutually exclusive)
    static bool areConflicting(int16_t a, int16_t b) {
        // Protection group: 0,1,3,4 conflict with each other
        if (isProtection(a) && isProtection(b) && a != b) return true;
        // Sharpness/Smite/Bane group
        if (isDamage(a) && isDamage(b) && a != b) return true;
        // Silk Touch vs Fortune
        if ((a == 33 && b == 35) || (a == 35 && b == 33)) return true;
        // Infinity vs Mending (1.7.10 doesn't have mending, but future-proof)
        return false;
    }

    // Check if an enchantment can apply to an item
    static bool canApplyTo(const Enchantment& ench, int16_t itemId) {
        switch (ench.target) {
            case EnchantTarget::SWORD:
                return isSword(itemId);
            case EnchantTarget::TOOL:
                return isTool(itemId);
            case EnchantTarget::BOW:
                return itemId == 261;
            case EnchantTarget::FISHING_ROD:
                return itemId == 346;
            case EnchantTarget::ARMOR:
                return isArmor(itemId);
            case EnchantTarget::ARMOR_HEAD:
                return isHelmet(itemId);
            case EnchantTarget::ARMOR_CHEST:
                return isChestplate(itemId);
            case EnchantTarget::ARMOR_LEGS:
                return isLeggings(itemId);
            case EnchantTarget::ARMOR_FEET:
                return isBoots(itemId);
            case EnchantTarget::ALL:
            case EnchantTarget::BREAKABLE:
                return true;
        }
        return false;
    }

    // Generate enchanting table slot levels based on bookshelf count (0-15)
    // Returns 3 level requirements for the 3 enchanting slots
    static std::array<int, 3> generateSlotLevels(int bookshelves, std::mt19937& rng) {
        int b = std::min(bookshelves, 15);
        std::array<int, 3> levels;

        // Vanilla formula from qi.java (ContainerEnchantment)
        for (int slot = 0; slot < 3; ++slot) {
            int base = generateBaseLevel(rng, slot, b);
            levels[slot] = std::max(base, slot + 1);
        }
        return levels;
    }

    // Select random enchantments for an item at a given power level
    std::vector<EnchantmentInstance> selectEnchantments(
            int16_t itemId, int powerLevel, std::mt19937& rng) const {
        std::vector<EnchantmentInstance> result;

        // Collect applicable enchantments
        std::vector<const Enchantment*> applicable;
        for (auto& ench : enchantments_) {
            if (canApplyTo(ench, itemId)) {
                applicable.push_back(&ench);
            }
        }

        if (applicable.empty()) return result;

        // Modified level (add some randomness)
        int modifiedLevel = powerLevel;
        // Enchantability bonus
        int enchantability = getEnchantability(itemId);
        modifiedLevel += 1 + (rng() % (enchantability / 4 + 1))
                       + (rng() % (enchantability / 4 + 1));
        // Random ±15% variation
        float variation = 1.0f + ((rng() % 100) / 100.0f - 0.5f) * 0.3f;
        modifiedLevel = static_cast<int>(modifiedLevel * variation + 0.5f);
        if (modifiedLevel < 1) modifiedLevel = 1;

        // Pick first enchantment weighted by rarity
        auto picked = pickWeighted(applicable, rng);
        if (!picked) return result;

        int level = pickLevel(*picked, modifiedLevel);
        if (level > 0) {
            result.push_back({picked->id, static_cast<int16_t>(level)});
        }

        // Chance for additional enchantments (decreasing probability)
        while (true) {
            modifiedLevel = modifiedLevel / 2;
            if ((rng() % 50) >= (modifiedLevel + 1)) break;

            // Filter out conflicting enchantments
            std::vector<const Enchantment*> remaining;
            for (auto* e : applicable) {
                bool conflicts = false;
                for (auto& existing : result) {
                    if (existing.id == e->id || areConflicting(existing.id, e->id)) {
                        conflicts = true;
                        break;
                    }
                }
                if (!conflicts) remaining.push_back(e);
            }

            if (remaining.empty()) break;

            auto extra = pickWeighted(remaining, rng);
            if (!extra) break;
            int extraLevel = pickLevel(*extra, modifiedLevel);
            if (extraLevel > 0) {
                result.push_back({extra->id, static_cast<int16_t>(extraLevel)});
            }
        }

        return result;
    }

    static EnchantmentRegistry& instance() {
        static EnchantmentRegistry reg;
        return reg;
    }

private:
    std::vector<Enchantment> enchantments_;
    std::unordered_map<int16_t, Enchantment> byId_;

    void add(int16_t id, const std::string& name, int maxLvl,
             EnchantTarget target, EnchantWeight weight) {
        Enchantment e{id, name, maxLvl, target, weight};
        enchantments_.push_back(e);
        byId_[id] = e;
    }

    void registerAll() {
        // === Armor Enchantments ===
        add(0,  "Protection",           4, EnchantTarget::ARMOR,      EnchantWeight::COMMON);
        add(1,  "Fire Protection",      4, EnchantTarget::ARMOR,      EnchantWeight::UNCOMMON);
        add(2,  "Feather Falling",      4, EnchantTarget::ARMOR_FEET, EnchantWeight::UNCOMMON);
        add(3,  "Blast Protection",     4, EnchantTarget::ARMOR,      EnchantWeight::RARE);
        add(4,  "Projectile Protection",4, EnchantTarget::ARMOR,      EnchantWeight::UNCOMMON);
        add(5,  "Respiration",          3, EnchantTarget::ARMOR_HEAD, EnchantWeight::RARE);
        add(6,  "Aqua Affinity",        1, EnchantTarget::ARMOR_HEAD, EnchantWeight::RARE);
        add(7,  "Thorns",               3, EnchantTarget::ARMOR_CHEST,EnchantWeight::VERY_RARE);

        // === Sword Enchantments ===
        add(16, "Sharpness",            5, EnchantTarget::SWORD,      EnchantWeight::COMMON);
        add(17, "Smite",                5, EnchantTarget::SWORD,      EnchantWeight::UNCOMMON);
        add(18, "Bane of Arthropods",   5, EnchantTarget::SWORD,      EnchantWeight::UNCOMMON);
        add(19, "Knockback",            2, EnchantTarget::SWORD,      EnchantWeight::UNCOMMON);
        add(20, "Fire Aspect",          2, EnchantTarget::SWORD,      EnchantWeight::RARE);
        add(21, "Looting",              3, EnchantTarget::SWORD,      EnchantWeight::RARE);

        // === Tool Enchantments ===
        add(32, "Efficiency",           5, EnchantTarget::TOOL,       EnchantWeight::COMMON);
        add(33, "Silk Touch",           1, EnchantTarget::TOOL,       EnchantWeight::VERY_RARE);
        add(34, "Unbreaking",           3, EnchantTarget::BREAKABLE,  EnchantWeight::UNCOMMON);
        add(35, "Fortune",              3, EnchantTarget::TOOL,       EnchantWeight::RARE);

        // === Bow Enchantments ===
        add(48, "Power",                5, EnchantTarget::BOW,        EnchantWeight::COMMON);
        add(49, "Punch",                2, EnchantTarget::BOW,        EnchantWeight::RARE);
        add(50, "Flame",                1, EnchantTarget::BOW,        EnchantWeight::RARE);
        add(51, "Infinity",             1, EnchantTarget::BOW,        EnchantWeight::VERY_RARE);

        // === Fishing Rod Enchantments ===
        add(61, "Luck of the Sea",      3, EnchantTarget::FISHING_ROD,EnchantWeight::RARE);
        add(62, "Lure",                 3, EnchantTarget::FISHING_ROD,EnchantWeight::RARE);
    }

    static bool isProtection(int16_t id) { return id >= 0 && id <= 4; }
    static bool isDamage(int16_t id) { return id >= 16 && id <= 18; }

    static bool isSword(int16_t id) {
        return id == 267 || id == 268 || id == 272 || id == 276 || id == 283;
    }

    static bool isTool(int16_t id) {
        // Pickaxes: 270,274,257,278,285 Axes: 271,275,258,279,286
        // Shovels: 269,273,256,277,284
        return (id >= 256 && id <= 258) || (id >= 269 && id <= 279) ||
               (id >= 284 && id <= 286);
    }

    static bool isArmor(int16_t id) {
        return isHelmet(id) || isChestplate(id) || isLeggings(id) || isBoots(id);
    }

    static bool isHelmet(int16_t id) {
        return id == 298 || id == 302 || id == 306 || id == 310 || id == 314;
    }

    static bool isChestplate(int16_t id) {
        return id == 299 || id == 303 || id == 307 || id == 311 || id == 315;
    }

    static bool isLeggings(int16_t id) {
        return id == 300 || id == 304 || id == 308 || id == 312 || id == 316;
    }

    static bool isBoots(int16_t id) {
        return id == 301 || id == 305 || id == 309 || id == 313 || id == 317;
    }

    // Item enchantability (from vanilla item material)
    static int getEnchantability(int16_t itemId) {
        // Gold: 25, Leather: 15, Chain: 12, Iron: 9, Diamond: 10
        // Gold tools: 22, Wood: 15, Stone: 5, Iron: 14, Diamond: 10
        if (itemId >= 314 && itemId <= 317) return 25; // Gold armor
        if (itemId >= 298 && itemId <= 301) return 15; // Leather armor
        if (itemId >= 302 && itemId <= 305) return 12; // Chain armor
        if (itemId >= 306 && itemId <= 309) return 9;  // Iron armor
        if (itemId >= 310 && itemId <= 313) return 10; // Diamond armor
        if (itemId == 283 || itemId == 285 || itemId == 286 ||
            itemId == 284 || itemId == 294) return 22; // Gold tools
        if (itemId == 268 || itemId == 269 || itemId == 270 ||
            itemId == 271 || itemId == 290) return 15; // Wood tools
        if (itemId == 272 || itemId == 273 || itemId == 274 ||
            itemId == 275 || itemId == 291) return 5;  // Stone tools
        if (itemId == 267 || itemId == 256 || itemId == 257 ||
            itemId == 258 || itemId == 292) return 14; // Iron tools
        if (itemId == 276 || itemId == 277 || itemId == 278 ||
            itemId == 279 || itemId == 293) return 10; // Diamond tools
        if (itemId == 261) return 1;  // Bow
        if (itemId == 346) return 1;  // Fishing rod
        return 1;
    }

    // Generate base enchanting level for a slot
    static int generateBaseLevel(std::mt19937& rng, int slot, int bookshelves) {
        // Vanilla: base = rand(1..8) + floor(bookshelves/2) + rand(0..bookshelves)
        int base = 1 + (rng() % 8) + (bookshelves / 2) + (rng() % (bookshelves + 1));
        // Slot modifiers
        if (slot == 0) return std::max(base / 3, 1);
        if (slot == 1) return (base * 2) / 3 + 1;
        return std::max(base, bookshelves * 2);
    }

    // Pick random enchantment weighted by rarity
    static const Enchantment* pickWeighted(
            const std::vector<const Enchantment*>& pool, std::mt19937& rng) {
        if (pool.empty()) return nullptr;

        int totalWeight = 0;
        for (auto* e : pool) totalWeight += static_cast<int>(e->weight);

        int roll = rng() % totalWeight;
        int acc = 0;
        for (auto* e : pool) {
            acc += static_cast<int>(e->weight);
            if (roll < acc) return e;
        }
        return pool.back();
    }

    // Pick the best level for an enchantment at a given modified power
    static int pickLevel(const Enchantment& ench, int modifiedLevel) {
        for (int lvl = ench.maxLevel; lvl >= 1; --lvl) {
            int minP = ench.minPower(lvl);
            int maxP = ench.maxPower(lvl);
            if (modifiedLevel >= minP && modifiedLevel <= maxP) return lvl;
        }
        // Fallback: if power is high enough for at least level 1
        if (modifiedLevel >= ench.minPower(1)) return 1;
        return 0;
    }
};

} // namespace mc
