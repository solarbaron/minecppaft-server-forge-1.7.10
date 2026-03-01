/**
 * EnchantingAnvil.h — Enchantment table and anvil container mechanics.
 *
 * Java references:
 *   - net.minecraft.inventory.ContainerEnchantment (202 lines)
 *   - net.minecraft.inventory.ContainerRepair (329 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ENCHANTMENT TABLE (ContainerEnchantment)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Inventory: 1 slot (the item to enchant)
 * enchantLevels[3]: the three enchantment options presented to player
 *
 * Bookshelf counting algorithm:
 *   - Scan 3×3 ring (excluding center) at table level and +1
 *   - For each position (i,j) in the ring:
 *     1. Check air at (posX+i, posY, posZ+j) and (posX+i, posY+1, posZ+j)
 *        (air path between table and potential bookshelf)
 *     2. Check bookshelf at (posX + i*2, posY, posZ + j*2) — 2 blocks out
 *     3. Also check (posX + i*2, posY+1, posZ + j*2) — upper bookshelf
 *     4. At corners (i!=0 && j!=0): also check the two L-shaped bookshelves
 *        at (posX+i*2, Y, posZ+j) and (posX+i, Y, posZ+j*2)
 *   - Maximum effective bookshelves: 15 (capped in enchantability formula)
 *
 * Enchant level calculation:
 *   - Java: EnchantmentHelper.calcItemStackEnchantability(random, slot, bookshelves, item)
 *   - Slot 0: minimum level options
 *   - Slot 1: medium level options
 *   - Slot 2: maximum level options
 *   - Level formula: base_enchantability + random(1..enchantability/4+1) + random(1..enchantability/4+1)
 *   - Modified by slot: slot 0 = max(1, level/3), slot 1 = level*2/3+1, slot 2 = max(level, bookshelves*2)
 *
 * Enchanting process (enchantItem):
 *   - Requires: enchantLevels[slot] > 0, item in slot, player has enough XP
 *   - Builds enchantment list: EnchantmentHelper.buildEnchantmentList(random, item, level)
 *   - If item is book → converts to enchanted_book (Items.enchanted_book)
 *     - Books randomly skip one enchantment (if list.size > 1)
 *   - Deducts XP levels from player
 *   - Recalculates enchantment options for next item
 *
 * On close: drop item back to player
 * Interaction range: 64.0 distance² from enchanting_table block
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ANVIL (ContainerRepair)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Inventory: 2 input slots + 1 output slot
 * maximumCost: total XP level cost displayed to player
 * materialCost: number of repair materials consumed
 *
 * Three repair modes (determined by input combination):
 *
 * 1. MATERIAL REPAIR (slot1 is repair material):
 *    - Condition: item.isRepairable && getIsRepairable(item, material)
 *    - Each material repairs 25% of max durability
 *    - Cost per material: max(1, repairAmount/100) + numEnchantments
 *    - materialCost tracks how many consumed
 *
 * 2. ITEM MERGE (slot1 is same item type, damageable):
 *    - Durability: remaining1 + remaining2 + 12% bonus
 *    - Cost: max(1, bonusRepair/100)
 *    - Enchantments are combined:
 *      - Same enchantment same level → level + 1
 *      - Same enchantment different level → max(level1, level2)
 *      - Incompatible enchantments → not applied (but still count toward cost!)
 *      - Capped at enchantment.maxLevel
 *
 * 3. ENCHANTED BOOK (slot1 is enchanted_book):
 *    - Works like item merge but:
 *      - canApply forced true for creative mode
 *      - Weight cost halved: max(1, weightCost / 2)
 *      - Prior repair cost halved: max(1, priorCost / 2)
 *
 * Rename cost:
 *   - Damageable items: 7 XP levels
 *   - Stackable items: stackSize × 5 XP levels
 *   - If item already has a name: adds half rename cost to prior cost
 *
 * Weight-to-cost mapping (enchantment rarity):
 *   Weight 10 (common: Protection, Sharpness) → cost 1 per level
 *   Weight 5 (uncommon: Fire Protection, Knockback) → cost 2 per level
 *   Weight 2 (rare: Silk Touch, Infinity) → cost 4 per level
 *   Weight 1 (very rare: Fortune, Thorns) → cost 8 per level
 *
 * Level cap:
 *   - maximumCost >= 40 → output null (too expensive) in survival
 *   - Exception: if ONLY rename and cost >= 40, cap at 39
 *
 * Repair cost escalation:
 *   - New repair cost = max(input1.repairCost, input2.repairCost) + 2
 *   - Named items: subtract 9 from repair cost (min 0)
 *
 * On close: drop both input items back to player
 * Interaction range: 64.0 distance² from anvil block
 *
 * Thread safety: Container operations happen on the server thread.
 * JNI readiness: Simple numeric constants and weight mappings.
 */
#pragma once

#include <cstdint>
#include <array>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Enchantment Table Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EnchantTableConstants {
    // ─── Block ID ───
    static constexpr int32_t ENCHANTING_TABLE_ID = 116;

    // ─── Inventory ───
    static constexpr int32_t INVENTORY_SIZE = 1;
    static constexpr int32_t ENCHANT_SLOT = 0;

    // ─── Enchantment option slots ───
    static constexpr int32_t NUM_OPTIONS = 3;

    // ─── Bookshelf detection ───
    // Java: 3×3 ring around table, check at Y and Y+1
    // Air gap required between table and bookshelf
    // Bookshelves checked at 2-block distance in each direction
    static constexpr int32_t BOOKSHELF_SEARCH_DIST = 2;
    static constexpr int32_t MAX_BOOKSHELVES = 15;  // effective cap in formula

    // ─── Bookshelf block ID ───
    static constexpr int32_t BOOKSHELF_ID = 47;

    // ─── Book → Enchanted Book conversion ───
    static constexpr int32_t BOOK_ID = 340;
    static constexpr int32_t ENCHANTED_BOOK_ID = 403;

    // ─── Interaction range ───
    static constexpr double USE_DISTANCE_SQ = 64.0;

    // ─── GUI slot positions ───
    // Java: enchant slot at (25, 47)
    // Player inventory: 3 rows at y=84+row*18, hotbar at y=142
    static constexpr int32_t PLAYER_INV_START_SLOT = 1;
    static constexpr int32_t PLAYER_INV_END_SLOT = 37;  // exclusive

    // ─── Container window properties ───
    // Java: progress bar IDs 0, 1, 2 = enchantLevels[0..2]
    static constexpr int32_t PROGRESS_ENCHANT_0 = 0;
    static constexpr int32_t PROGRESS_ENCHANT_1 = 1;
    static constexpr int32_t PROGRESS_ENCHANT_2 = 2;
}

// ═══════════════════════════════════════════════════════════════════════════
// Enchantability Calculation
// Java: EnchantmentHelper.calcItemStackEnchantability
// ═══════════════════════════════════════════════════════════════════════════

namespace EnchantabilityCalc {
    // ─── Formula components ───
    // Java: level = base + random(1 + base/4) + random(1 + base/4)
    //       where base = item.getItemEnchantability()
    //       then random float ±15% modification
    //       then slot-based scaling

    // ─── Base enchantability values by material ───
    // Java: ToolMaterial and ArmorMaterial enchantability
    // Wood/Leather: 15, Stone/Chain: 12, Iron: 9, Diamond: 10, Gold: 25
    static constexpr int32_t WOOD_ENCHANTABILITY = 15;
    static constexpr int32_t STONE_ENCHANTABILITY = 5;
    static constexpr int32_t IRON_ENCHANTABILITY = 14;
    static constexpr int32_t DIAMOND_ENCHANTABILITY = 10;
    static constexpr int32_t GOLD_ENCHANTABILITY = 22;
    static constexpr int32_t LEATHER_ENCHANTABILITY = 15;
    static constexpr int32_t CHAIN_ENCHANTABILITY = 12;

    // ─── Slot-based level scaling ───
    // Java: slot 0 = max(level / 3, 1)
    //       slot 1 = level * 2 / 3 + 1
    //       slot 2 = max(level, bookshelves * 2)
    inline int32_t scaleForSlot(int32_t level, int32_t slot, int32_t bookshelves) {
        switch (slot) {
            case 0: {
                int32_t v = level / 3;
                return v < 1 ? 1 : v;
            }
            case 1:
                return level * 2 / 3 + 1;
            case 2: {
                int32_t min = bookshelves * 2;
                return level > min ? level : min;
            }
            default:
                return level;
        }
    }

    // ─── Level ±15% random modification ───
    // Java: level = level + 1 + random(5) + random(5) (approximately ±range)
    static constexpr float RANDOM_MODIFICATION_RANGE = 0.15f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Anvil Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace AnvilConstants {
    // ─── Block ID ───
    static constexpr int32_t ANVIL_ID = 145;

    // ─── Inventory layout ───
    static constexpr int32_t INPUT_SLOT_1 = 0;
    static constexpr int32_t INPUT_SLOT_2 = 1;
    static constexpr int32_t OUTPUT_SLOT = 2;
    static constexpr int32_t NUM_INPUT_SLOTS = 2;
    static constexpr int32_t TOTAL_SLOTS = 3;

    // ─── XP level cap ───
    // Java: if (maximumCost >= 40 && !creative) → too expensive
    static constexpr int32_t MAX_COST = 40;
    // Java: rename exception cap at 39
    static constexpr int32_t RENAME_ONLY_CAP = 39;

    // ─── Repair cost escalation ───
    // Java: repairCost = max(input1, input2) + 2
    static constexpr int32_t REPAIR_COST_INCREMENT = 2;
    // Java: named items subtract 9
    static constexpr int32_t NAMED_COST_REDUCTION = 9;

    // ─── Material repair ───
    // Java: each material repairs maxDurability / 4 (25%)
    static constexpr int32_t REPAIR_FRACTION_DIVISOR = 4;
    // Java: cost per material = max(1, repairAmount / 100) + enchantmentCount
    static constexpr int32_t MATERIAL_COST_DIVISOR = 100;

    // ─── Item merge durability bonus ───
    // Java: bonus = maxDurability * 12 / 100 (12%)
    static constexpr int32_t MERGE_BONUS_NUMERATOR = 12;
    static constexpr int32_t MERGE_BONUS_DENOMINATOR = 100;

    inline int32_t getMergeBonus(int32_t maxDurability) {
        return maxDurability * MERGE_BONUS_NUMERATOR / MERGE_BONUS_DENOMINATOR;
    }

    // ─── Rename cost ───
    // Java: damageable → 7, otherwise → stackSize * 5
    static constexpr int32_t RENAME_COST_DAMAGEABLE = 7;
    static constexpr int32_t RENAME_COST_PER_STACK = 5;

    inline int32_t getRenameCost(bool isDamageable, int32_t stackSize) {
        return isDamageable ? RENAME_COST_DAMAGEABLE : stackSize * RENAME_COST_PER_STACK;
    }

    // ─── Enchantment weight-to-cost mapping ───
    // Java: switch (enchantment.getWeight()) { 10→1, 5→2, 2→4, 1→8 }
    inline int32_t getWeightCost(int32_t weight) {
        switch (weight) {
            case 10: return 1;  // Common (Protection, Sharpness, Efficiency)
            case 5:  return 2;  // Uncommon (Fire Protection, Knockback, Unbreaking)
            case 2:  return 4;  // Rare (Silk Touch, Infinity)
            case 1:  return 8;  // Very Rare (Fortune, Thorns)
            default: return 1;
        }
    }

    // ─── Enchanted book cost modifier ───
    // Java: if (enchantedBook) costPerLevel = max(1, costPerLevel / 2)
    inline int32_t getBookWeightCost(int32_t weight) {
        int32_t cost = getWeightCost(weight);
        int32_t halved = cost / 2;
        return halved < 1 ? 1 : halved;
    }

    // ─── Enchantment level combining ───
    // Java: if (level1 == level2) result = level1 + 1; else result = max(level1, level2)
    inline int32_t combineEnchantLevels(int32_t level1, int32_t level2) {
        if (level1 == level2) return level1 + 1;
        return level1 > level2 ? level1 : level2;
    }

    // ─── Interaction range ───
    static constexpr double USE_DISTANCE_SQ = 64.0;

    // ─── Container window properties ───
    // Java: progress bar ID 0 = maximumCost
    static constexpr int32_t PROGRESS_MAX_COST = 0;

    // ─── Anvil damage metadata ───
    // Java: BlockAnvil damage levels 0-2 (bits 2-3 of metadata)
    // 0 = pristine, 1 = slightly damaged, 2 = very damaged
    // Each use has a 12% chance to damage
    static constexpr int32_t MAX_DAMAGE_LEVEL = 2;
    static constexpr float DAMAGE_CHANCE = 0.12f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Bookshelf counting reference positions
// Precomputed positions for the bookshelf scan algorithm
// ═══════════════════════════════════════════════════════════════════════════

namespace BookshelfScan {
    // The scan iterates i=-1..1, j=-1..1 (excluding 0,0)
    // For each (i,j):
    //   1. Check air at (i, 0, j) and (i, 1, j) relative to table
    //   2. Check bookshelf at (i*2, 0, j*2) and (i*2, 1, j*2)
    //   3. If corner (i!=0 && j!=0):
    //      - Check (i*2, 0, j), (i*2, 1, j)
    //      - Check (i, 0, j*2), (i, 1, j*2)

    // Maximum possible bookshelf positions: 8 ring positions
    // Each can have 2 height levels = 16 base checks
    // Corners add 4 extra L-shape checks (each at 2 levels) = 16 extra
    // Total theoretical max: 32, but capped at 15 in enchantability formula

    // Count of ring positions (excluding center)
    static constexpr int32_t RING_POSITIONS = 8;  // 3×3 - 1

    struct ScanPosition {
        int32_t dx, dz;  // ring offset
        bool isCorner;
    };

    static constexpr ScanPosition POSITIONS[] = {
        {-1, -1, true},
        {-1,  0, false},
        {-1,  1, true},
        { 0, -1, false},
        // {0, 0} excluded (center)
        { 0,  1, false},
        { 1, -1, true},
        { 1,  0, false},
        { 1,  1, true},
    };
}

} // namespace mccpp
