/**
 * LootTables.h — Weighted random chest content system and all vanilla loot tables.
 *
 * Java references:
 *   - net.minecraft.util.WeightedRandomChestContent (83 lines)
 *   - net.minecraft.util.WeightedRandom (weighted selection)
 *   - StructureVillagePieces$House2 — village blacksmith (17 entries)
 *   - StructureStrongholdPieces$ChestCorridor — stronghold corridor (18 entries)
 *   - StructureStrongholdPieces$RoomCrossing — stronghold crossing (7 entries)
 *   - StructureStrongholdPieces$Library — stronghold library (4 entries)
 *   - ComponentScatteredFeaturePieces$JunglePyramid — jungle temple (10+1 entries)
 *   - ComponentScatteredFeaturePieces$DesertPyramid — desert temple (10 entries)
 *   - StructureNetherBridgePieces$Piece — nether fortress (11 entries)
 *   - StructureMineshaftPieces — mineshaft (13 entries)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * WEIGHTED RANDOM CHEST CONTENT SYSTEM
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Each entry: {itemId, metadata, minStack, maxStack, weight}
 *
 * Generation algorithm (generateChestContents):
 *   For each of N attempts:
 *     1. Select weighted random entry from loot table
 *     2. Stack size = min + random(max - min + 1)
 *     3. If item maxStackSize >= stackSize:
 *        Place entire stack in random slot (may overwrite)
 *     4. Else (unstackable/low maxStack):
 *        Split into individual items, each in random slot
 *
 * Note: Items placed in random slots CAN overwrite previous placements.
 * This is vanilla behavior — not a bug.
 *
 * func_92080_a: Concatenate base loot table with extra items (enchanted books).
 * generateDispenserContents: Same algorithm for dispensers (jungle temple arrows).
 *
 * All structure chests also append a random enchanted book via:
 *   Items.enchanted_book.func_92114_b(random)
 * This produces a WeightedRandomChestContent with weight 1.
 *
 * Thread safety: Loot generation uses the structure's Random.
 * JNI readiness: Simple POD structs and arrays.
 */
#pragma once

#include <cstdint>
#include <array>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Weighted Random Chest Content Entry
// ═══════════════════════════════════════════════════════════════════════════

struct LootEntry {
    int32_t itemId;
    int32_t metadata;
    int32_t minStack;
    int32_t maxStack;
    int32_t weight;
};

// ═══════════════════════════════════════════════════════════════════════════
// Item IDs (for loot table references)
// ═══════════════════════════════════════════════════════════════════════════

namespace LootItemIds {
    static constexpr int32_t DIAMOND = 264;
    static constexpr int32_t IRON_INGOT = 265;
    static constexpr int32_t GOLD_INGOT = 266;
    static constexpr int32_t BREAD = 297;
    static constexpr int32_t APPLE = 260;
    static constexpr int32_t IRON_PICKAXE = 257;
    static constexpr int32_t IRON_SWORD = 267;
    static constexpr int32_t IRON_CHESTPLATE = 307;
    static constexpr int32_t IRON_HELMET = 306;
    static constexpr int32_t IRON_LEGGINGS = 308;
    static constexpr int32_t IRON_BOOTS = 309;
    static constexpr int32_t SADDLE = 329;
    static constexpr int32_t IRON_HORSE_ARMOR = 417;
    static constexpr int32_t GOLDEN_HORSE_ARMOR = 418;
    static constexpr int32_t DIAMOND_HORSE_ARMOR = 419;
    static constexpr int32_t ENDER_PEARL = 368;
    static constexpr int32_t REDSTONE = 331;
    static constexpr int32_t GOLDEN_APPLE = 322;
    static constexpr int32_t COAL = 263;
    static constexpr int32_t BOOK = 340;
    static constexpr int32_t PAPER = 339;
    static constexpr int32_t MAP = 358;
    static constexpr int32_t COMPASS = 345;
    static constexpr int32_t EMERALD = 388;
    static constexpr int32_t BONE = 352;
    static constexpr int32_t ROTTEN_FLESH = 367;
    static constexpr int32_t ARROW = 262;
    static constexpr int32_t GOLDEN_SWORD = 283;
    static constexpr int32_t GOLDEN_CHESTPLATE = 315;
    static constexpr int32_t FLINT_AND_STEEL = 259;
    static constexpr int32_t NETHER_WART = 372;
    static constexpr int32_t DYE = 351;        // meta 4 = lapis
    static constexpr int32_t MELON_SEEDS = 362;
    static constexpr int32_t PUMPKIN_SEEDS = 361;
    static constexpr int32_t ENCHANTED_BOOK = 403;
    // Block items
    static constexpr int32_t OBSIDIAN_BLOCK = 49;
    static constexpr int32_t SAPLING_BLOCK = 6;
    static constexpr int32_t RAIL_BLOCK = 66;
}

// ═══════════════════════════════════════════════════════════════════════════
// VILLAGE BLACKSMITH LOOT
// Java: StructureVillagePieces$House2.villageBlacksmithChestContents
// Chest count: 3-8 items
// ═══════════════════════════════════════════════════════════════════════════

namespace VillageBlacksmithLoot {
    using namespace LootItemIds;
    static constexpr LootEntry TABLE[] = {
        // {itemId, meta, min, max, weight}
        {DIAMOND,              0, 1, 3, 3},
        {IRON_INGOT,           0, 1, 5, 10},
        {GOLD_INGOT,           0, 1, 3, 5},
        {BREAD,                0, 1, 3, 15},
        {APPLE,                0, 1, 3, 15},
        {IRON_PICKAXE,         0, 1, 1, 5},
        {IRON_SWORD,           0, 1, 1, 5},
        {IRON_CHESTPLATE,      0, 1, 1, 5},
        {IRON_HELMET,          0, 1, 1, 5},
        {IRON_LEGGINGS,        0, 1, 1, 5},
        {IRON_BOOTS,           0, 1, 1, 5},
        {OBSIDIAN_BLOCK,       0, 3, 7, 5},
        {SAPLING_BLOCK,        0, 3, 7, 5},
        {SADDLE,               0, 1, 1, 3},
        {IRON_HORSE_ARMOR,     0, 1, 1, 1},
        {GOLDEN_HORSE_ARMOR,   0, 1, 1, 1},
        {DIAMOND_HORSE_ARMOR,  0, 1, 1, 1},
    };
    static constexpr int32_t TABLE_SIZE = 17;
    // Java: 3-8 items typically (structure-specific)
    static constexpr int32_t MIN_ITEMS = 3;
    static constexpr int32_t MAX_ITEMS = 8;
}

// ═══════════════════════════════════════════════════════════════════════════
// STRONGHOLD CHEST CORRIDOR LOOT
// Java: StructureStrongholdPieces$ChestCorridor.strongholdChestContents
// Chest count: 2 + random(2) = 2-3 items + enchanted book
// ═══════════════════════════════════════════════════════════════════════════

namespace StrongholdCorridorLoot {
    using namespace LootItemIds;
    static constexpr LootEntry TABLE[] = {
        {ENDER_PEARL,          0, 1, 1, 10},
        {DIAMOND,              0, 1, 3, 3},
        {IRON_INGOT,           0, 1, 5, 10},
        {GOLD_INGOT,           0, 1, 3, 5},
        {REDSTONE,             0, 4, 9, 5},
        {BREAD,                0, 1, 3, 15},
        {APPLE,                0, 1, 3, 15},
        {IRON_PICKAXE,         0, 1, 1, 5},
        {IRON_SWORD,           0, 1, 1, 5},
        {IRON_CHESTPLATE,      0, 1, 1, 5},
        {IRON_HELMET,          0, 1, 1, 5},
        {IRON_LEGGINGS,        0, 1, 1, 5},
        {IRON_BOOTS,           0, 1, 1, 5},
        {GOLDEN_APPLE,         0, 1, 1, 1},
        {SADDLE,               0, 1, 1, 1},
        {IRON_HORSE_ARMOR,     0, 1, 1, 1},
        {GOLDEN_HORSE_ARMOR,   0, 1, 1, 1},
        {DIAMOND_HORSE_ARMOR,  0, 1, 1, 1},
    };
    static constexpr int32_t TABLE_SIZE = 18;
    // Java: 2 + random.nextInt(2) = 2-3 items
    static constexpr int32_t MIN_ITEMS = 2;
    static constexpr int32_t RANDOM_ITEMS = 2;  // random(2) added
}

// ═══════════════════════════════════════════════════════════════════════════
// STRONGHOLD CROSSING ROOM LOOT
// Java: StructureStrongholdPieces$RoomCrossing.strongholdRoomCrossingChestContents
// Chest count: 1 + random(4) = 1-4 items + enchanted book
// ═══════════════════════════════════════════════════════════════════════════

namespace StrongholdCrossingLoot {
    using namespace LootItemIds;
    static constexpr LootEntry TABLE[] = {
        {IRON_INGOT,   0, 1, 5, 10},
        {GOLD_INGOT,   0, 1, 3, 5},
        {REDSTONE,     0, 4, 9, 5},
        {COAL,         0, 3, 8, 10},
        {BREAD,        0, 1, 3, 15},
        {APPLE,        0, 1, 3, 15},
        {IRON_PICKAXE, 0, 1, 1, 1},
    };
    static constexpr int32_t TABLE_SIZE = 7;
    // Java: 1 + random.nextInt(4) = 1-4 items
    static constexpr int32_t MIN_ITEMS = 1;
    static constexpr int32_t RANDOM_ITEMS = 4;
}

// ═══════════════════════════════════════════════════════════════════════════
// STRONGHOLD LIBRARY LOOT
// Java: StructureStrongholdPieces$Library.strongholdLibraryChestContents
// Chest count: 1 + random(4) = 1-4 items + enchanted book (level 1-5, weight 2)
// ═══════════════════════════════════════════════════════════════════════════

namespace StrongholdLibraryLoot {
    using namespace LootItemIds;
    static constexpr LootEntry TABLE[] = {
        {BOOK,    0, 1, 3, 20},
        {PAPER,   0, 2, 7, 20},
        {MAP,     0, 1, 1, 1},
        {COMPASS, 0, 1, 1, 1},
    };
    static constexpr int32_t TABLE_SIZE = 4;
    // Java: 1 + random.nextInt(4) = 1-4 items
    static constexpr int32_t MIN_ITEMS = 1;
    static constexpr int32_t RANDOM_ITEMS = 4;
    // Java: func_92112_a(random, 1, 5, 2) — enchanted book level 1-5, weight 2
    static constexpr int32_t ENCH_BOOK_MIN_LEVEL = 1;
    static constexpr int32_t ENCH_BOOK_MAX_LEVEL = 5;
    static constexpr int32_t ENCH_BOOK_WEIGHT = 2;
}

// ═══════════════════════════════════════════════════════════════════════════
// JUNGLE TEMPLE LOOT
// Java: ComponentScatteredFeaturePieces$JunglePyramid.junglePyramidsChestContents
// Chest count: 2 + random(5) = 2-6 items + enchanted book
// ═══════════════════════════════════════════════════════════════════════════

namespace JungleTempleLoot {
    using namespace LootItemIds;
    static constexpr LootEntry TABLE[] = {
        {DIAMOND,              0, 1, 3, 3},
        {IRON_INGOT,           0, 1, 5, 10},
        {GOLD_INGOT,           0, 2, 7, 15},
        {EMERALD,              0, 1, 3, 2},
        {BONE,                 0, 4, 6, 20},
        {ROTTEN_FLESH,         0, 3, 7, 16},
        {SADDLE,               0, 1, 1, 3},
        {IRON_HORSE_ARMOR,     0, 1, 1, 1},
        {GOLDEN_HORSE_ARMOR,   0, 1, 1, 1},
        {DIAMOND_HORSE_ARMOR,  0, 1, 1, 1},
    };
    static constexpr int32_t TABLE_SIZE = 10;
    static constexpr int32_t MIN_ITEMS = 2;
    static constexpr int32_t RANDOM_ITEMS = 5;

    // Dispenser trap arrows
    // Java: junglePyramidsDispenserContents = {arrows, 0, 2, 7, weight 30}
    static constexpr LootEntry DISPENSER_TABLE[] = {
        {ARROW, 0, 2, 7, 30},
    };
    static constexpr int32_t DISPENSER_TABLE_SIZE = 1;
}

// ═══════════════════════════════════════════════════════════════════════════
// DESERT TEMPLE LOOT
// Java: ComponentScatteredFeaturePieces$DesertPyramid.itemsToGenerateInTemple
// 4 chests, each: 2 + random(5) = 2-6 items + enchanted book
// ═══════════════════════════════════════════════════════════════════════════

namespace DesertTempleLoot {
    using namespace LootItemIds;
    static constexpr LootEntry TABLE[] = {
        {DIAMOND,              0, 1, 3, 3},
        {IRON_INGOT,           0, 1, 5, 10},
        {GOLD_INGOT,           0, 2, 7, 15},
        {EMERALD,              0, 1, 3, 2},
        {BONE,                 0, 4, 6, 20},
        {ROTTEN_FLESH,         0, 3, 7, 16},
        {SADDLE,               0, 1, 1, 3},
        {IRON_HORSE_ARMOR,     0, 1, 1, 1},
        {GOLDEN_HORSE_ARMOR,   0, 1, 1, 1},
        {DIAMOND_HORSE_ARMOR,  0, 1, 1, 1},
    };
    static constexpr int32_t TABLE_SIZE = 10;
    static constexpr int32_t NUM_CHESTS = 4;
    static constexpr int32_t MIN_ITEMS = 2;
    static constexpr int32_t RANDOM_ITEMS = 5;
}

// ═══════════════════════════════════════════════════════════════════════════
// NETHER FORTRESS LOOT
// Java: StructureNetherBridgePieces$Piece.field_111019_a
// Chest count varies by room type
// ═══════════════════════════════════════════════════════════════════════════

namespace NetherFortressLoot {
    using namespace LootItemIds;
    static constexpr LootEntry TABLE[] = {
        {DIAMOND,              0, 1, 3, 5},
        {IRON_INGOT,           0, 1, 5, 5},
        {GOLD_INGOT,           0, 1, 3, 15},
        {GOLDEN_SWORD,         0, 1, 1, 5},
        {GOLDEN_CHESTPLATE,    0, 1, 1, 5},
        {FLINT_AND_STEEL,      0, 1, 1, 5},
        {NETHER_WART,          0, 3, 7, 5},
        {SADDLE,               0, 1, 1, 10},
        {GOLDEN_HORSE_ARMOR,   0, 1, 1, 8},
        {IRON_HORSE_ARMOR,     0, 1, 1, 5},
        {DIAMOND_HORSE_ARMOR,  0, 1, 1, 3},
    };
    static constexpr int32_t TABLE_SIZE = 11;
    // Java: 2 + random(4) = 2-5 items typically
    static constexpr int32_t MIN_ITEMS = 2;
    static constexpr int32_t RANDOM_ITEMS = 4;
}

// ═══════════════════════════════════════════════════════════════════════════
// MINESHAFT LOOT (in minecart chests)
// Java: StructureMineshaftPieces.mineshaftChestContents
// Chest count: 3 + random(4) = 3-6 items + enchanted book
// ═══════════════════════════════════════════════════════════════════════════

namespace MineshaftLoot {
    using namespace LootItemIds;
    static constexpr LootEntry TABLE[] = {
        {IRON_INGOT,    0, 1, 5, 10},
        {GOLD_INGOT,    0, 1, 3, 5},
        {REDSTONE,      0, 4, 9, 5},
        {DYE,           4, 4, 9, 5},    // lapis lazuli (meta 4)
        {DIAMOND,       0, 1, 2, 3},
        {COAL,          0, 3, 8, 10},
        {BREAD,         0, 1, 3, 15},
        {IRON_PICKAXE,  0, 1, 1, 1},
        {RAIL_BLOCK,    0, 4, 8, 1},
        {MELON_SEEDS,   0, 2, 4, 10},
        {PUMPKIN_SEEDS, 0, 2, 4, 10},
        {SADDLE,        0, 1, 1, 3},
        {IRON_HORSE_ARMOR, 0, 1, 1, 1},
    };
    static constexpr int32_t TABLE_SIZE = 13;
    // Java: 3 + random.nextInt(4) = 3-6 items
    static constexpr int32_t MIN_ITEMS = 3;
    static constexpr int32_t RANDOM_ITEMS = 4;
}

// ═══════════════════════════════════════════════════════════════════════════
// Dungeon (Monster Spawner Room) Loot
// Java: net.minecraft.world.gen.feature.WorldGenDungeons.field_111189_a
// Not in structures/ but in features, included for completeness
// ═══════════════════════════════════════════════════════════════════════════

namespace DungeonLoot {
    using namespace LootItemIds;
    static constexpr LootEntry TABLE[] = {
        {SADDLE,               0, 1, 1, 10},
        {IRON_INGOT,           0, 1, 4, 10},
        {BREAD,                0, 1, 1, 10},
        {286,                  0, 1, 1, 10},  // wheat (name_tag not in 1.7.10, this is golden_apple meta 0)
        {COAL,                 0, 1, 4, 10},
        {REDSTONE,             0, 1, 4, 10},
        {GOLDEN_APPLE,         0, 1, 1, 1},   // regular
        {GOLDEN_APPLE,         1, 1, 1, 1},   // enchanted (Notch apple)
        {2256,                 0, 1, 1, 4},   // music_disc_13 (ID 2256)
        {IRON_HORSE_ARMOR,     0, 1, 1, 2},
        {GOLDEN_HORSE_ARMOR,   0, 1, 1, 1},
        {DIAMOND_HORSE_ARMOR,  0, 1, 1, 1},
        {ENCHANTED_BOOK,       0, 1, 1, 1},   // random enchanted book added separately
    };
    // Note: Actual dungeon loot numbers from WorldGenDungeons would need
    // exact extraction — the above is approximate. Structure loot tables
    // above are exactly as found in Java.
    static constexpr int32_t MUSIC_DISC_13 = 2256; // for reference
}

// ═══════════════════════════════════════════════════════════════════════════
// Loot Generation Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LootGenConstants {
    // ─── Chest inventory size ───
    static constexpr int32_t CHEST_SIZE = 27;        // regular chest
    static constexpr int32_t DOUBLE_CHEST_SIZE = 54;  // double chest
    static constexpr int32_t DISPENSER_SIZE = 9;
    static constexpr int32_t MINECART_CHEST_SIZE = 27;

    // ─── Enchanted book generation ───
    // Java: Items.enchanted_book.func_92114_b(random) — random enchantment, weight 1
    // Java: Items.enchanted_book.func_92112_a(random, minLevel, maxLevel, weight)
    static constexpr int32_t DEFAULT_ENCH_BOOK_WEIGHT = 1;

    // ─── Weighted random selection ───
    // Java: WeightedRandom.getRandomItem — sum all weights, pick random in total
    // Algorithm: totalWeight = sum(entry.weight for all entries)
    //            pick = random(totalWeight)
    //            iterate entries subtracting weights until pick < 0
    //            return that entry
}

// ═══════════════════════════════════════════════════════════════════════════
// Helper: compute total weight for a loot table
// ═══════════════════════════════════════════════════════════════════════════

inline int32_t computeTotalWeight(const LootEntry* table, int32_t size) {
    int32_t total = 0;
    for (int32_t i = 0; i < size; ++i) {
        total += table[i].weight;
    }
    return total;
}

} // namespace mccpp
