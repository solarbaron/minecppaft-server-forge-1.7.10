/**
 * FlowerGrassDeadBush.h — Flowers, tall grass, and dead bush.
 *
 * Java references:
 *   - net.minecraft.block.BlockFlower (53 lines)
 *   - net.minecraft.block.BlockTallGrass (87 lines)
 *   - net.minecraft.block.BlockDeadBush (47 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FLOWER (BlockFlower)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: plants, extends BlockBush
 *
 * Two block types:
 *   Type 0 = yellow_flower (37): 1 variant
 *     Meta 0: dandelion
 *
 *   Type 1 = red_flower (38): 9 variants
 *     Meta 0: poppy
 *     Meta 1: blue orchid
 *     Meta 2: allium
 *     Meta 3: azure bluet (houstonia)
 *     Meta 4: red tulip
 *     Meta 5: orange tulip
 *     Meta 6: white tulip
 *     Meta 7: pink tulip
 *     Meta 8: oxeye daisy
 *
 * damageDropped = meta (preserves variant)
 *
 * Static helpers:
 *   func_149857_e(name) → returns yellow_flower or red_flower block
 *   func_149856_f(name) → returns meta index for variant name
 *
 * Block IDs: yellow_flower (37), red_flower (38)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TALL GRASS (BlockTallGrass)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: vine, extends BlockBush, implements IGrowable
 * Bounds: (0.1, 0, 0.1) → (0.9, 0.8, 0.9) — 80% tall, 80% wide
 *
 * Types (metadata):
 *   0 = dead bush (shrub)
 *   1 = tall grass
 *   2 = fern
 *
 * Drops:
 *   Normal: 1/8 chance wheat_seeds (rand(8)==0), else nothing
 *   Fortune: 1 + rand(fortune * 2 + 1) seeds when seeds drop
 *   Shears: drops self (tallgrass block with meta)
 *
 * Bonemeal (IGrowable):
 *   canFertilize: meta != 0 (not dead shrub)
 *   shouldFertilize: always true
 *   fertilize: grows to double_plant
 *     Meta 1 (grass) → double_plant type 2 (double tall grass)
 *     Meta 2 (fern) → double_plant type 3 (large fern)
 *     Requires canPlaceBlockAt for double_plant
 *
 * Block ID: 31
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DEAD BUSH (BlockDeadBush)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: vine, extends BlockBush
 * Bounds: same as tall grass (0.1, 0, 0.1) → (0.9, 0.8, 0.9)
 *
 * Placement restriction:
 *   Can only be placed on: sand, hardened_clay, stained_hardened_clay, dirt
 *   (NOT on grass blocks)
 *
 * Drops:
 *   Normal: nothing (null)
 *   Shears: drops self (deadbush block)
 *
 * Block ID: 32
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Simple metadata and name lookups.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Flower Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FlowerConstants {
    // ─── Block IDs ───
    static constexpr int32_t YELLOW_FLOWER_ID = 37;
    static constexpr int32_t RED_FLOWER_ID = 38;

    // ─── Flower type groups ───
    static constexpr int32_t TYPE_YELLOW = 0;
    static constexpr int32_t TYPE_RED = 1;

    // ─── Yellow variants (1 total) ───
    static constexpr int32_t YELLOW_DANDELION = 0;
    static constexpr int32_t NUM_YELLOW = 1;

    static constexpr const char* YELLOW_NAMES[] = {"dandelion"};

    // ─── Red variants (9 total) ───
    static constexpr int32_t RED_POPPY = 0;
    static constexpr int32_t RED_BLUE_ORCHID = 1;
    static constexpr int32_t RED_ALLIUM = 2;
    static constexpr int32_t RED_HOUSTONIA = 3;    // azure bluet
    static constexpr int32_t RED_TULIP_RED = 4;
    static constexpr int32_t RED_TULIP_ORANGE = 5;
    static constexpr int32_t RED_TULIP_WHITE = 6;
    static constexpr int32_t RED_TULIP_PINK = 7;
    static constexpr int32_t RED_OXEYE_DAISY = 8;
    static constexpr int32_t NUM_RED = 9;

    static constexpr const char* RED_NAMES[] = {
        "poppy", "blueOrchid", "allium", "houstonia",
        "tulipRed", "tulipOrange", "tulipWhite", "tulipPink",
        "oxeyeDaisy"
    };
}

// ═══════════════════════════════════════════════════════════════════════════
// Tall Grass Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TallGrassConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 31;

    // ─── Types (metadata) ───
    static constexpr int32_t TYPE_DEAD_SHRUB = 0;
    static constexpr int32_t TYPE_TALL_GRASS = 1;
    static constexpr int32_t TYPE_FERN = 2;
    static constexpr int32_t NUM_TYPES = 3;

    static constexpr const char* TYPE_NAMES[] = {"deadbush", "tallgrass", "fern"};

    // ─── Bounds ───
    static constexpr float HALF_WIDTH = 0.4f;   // centered ±0.4
    static constexpr float HEIGHT = 0.8f;        // 80% tall

    // ─── Drops ───
    static constexpr int32_t SEED_CHANCE = 8;    // 1 in 8 chance
    static constexpr int32_t WHEAT_SEEDS_ID = 295;

    // Fortune: 1 + rand(fortune * 2 + 1)

    // ─── Bonemeal → Double Plant ───
    // Tall grass (1) → double_plant type 2 (double tall grass)
    // Fern (2) → double_plant type 3 (large fern)
    static constexpr int32_t BONEMEAL_DOUBLE_GRASS = 2;
    static constexpr int32_t BONEMEAL_DOUBLE_FERN = 3;
}

// ═══════════════════════════════════════════════════════════════════════════
// Dead Bush Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DeadBushConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 32;

    // ─── Bounds ───
    // Same as tall grass: (0.1, 0, 0.1) → (0.9, 0.8, 0.9)
    static constexpr float HALF_WIDTH = 0.4f;
    static constexpr float HEIGHT = 0.8f;

    // ─── Placement blocks ───
    static constexpr int32_t SAND_ID = 12;
    static constexpr int32_t HARDENED_CLAY_ID = 172;
    static constexpr int32_t STAINED_HARDENED_CLAY_ID = 159;
    static constexpr int32_t DIRT_ID = 3;
    // canPlaceBlockOn: sand OR hardened_clay OR stained_hardened_clay OR dirt

    // ─── Drops ───
    // Normal: nothing
    // Shears: drops self (deadbush)
    // Shears check: Items.shears item comparison
}

} // namespace mccpp
