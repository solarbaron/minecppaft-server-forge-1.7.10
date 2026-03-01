/**
 * MushroomMyceliumCropsGrass.h — Growth, spread, and agriculture blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockMushroom (110 lines)
 *   - net.minecraft.block.BlockHugeMushroom (37 lines)
 *   - net.minecraft.block.BlockMycelium (47 lines)
 *   - net.minecraft.block.BlockCrops (144 lines)
 *   - net.minecraft.block.BlockGrass (88 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * MUSHROOM (BlockMushroom)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockBush, implements IGrowable
 * Material: plants (from BlockBush)
 * Bounds: 0.4 wide centered (0.3-0.7 XZ), 0.4 tall
 * setTickRandomly(true)
 *
 * Spreading (updateTick):
 *   1/25 chance per tick
 *   Density check: max 5 in 9×3×9 area (±4 XZ, ±1 Y)
 *   4-step random walk: ±1 XZ, ±(rand(2)-rand(2)) Y
 *   Final: place if air + canBlockStay
 *
 * canBlockStay:
 *   On mycelium: always
 *   On dirt with meta 2 (podzol): always
 *   On fullBlock: if block light < 13
 *   Y range: [0, 256)
 *
 * Bonemeal (IGrowable):
 *   canFertilize: always true
 *   shouldFertilize: rand < 0.4 (40% chance)
 *   fertilize: WorldGenBigMushroom
 *     brown_mushroom → type 0
 *     red_mushroom → type 1
 *     If generation fails, restore original block
 *
 * Block IDs: brown_mushroom(39), red_mushroom(40)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * HUGE MUSHROOM (BlockHugeMushroom)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood
 * 2 types: brown (0), red (1)
 *
 * Drops: rand(10) - 7, capped at 0 minimum
 *   → 30% chance of 0, then 1, 2, or 3
 *   Drop item: brown_mushroom or red_mushroom (by type offset)
 *
 * Block IDs: brown_mushroom_block(99), red_mushroom_block(100)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * MYCELIUM (BlockMycelium)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: grass, setTickRandomly(true)
 *
 * Spread (identical to grass):
 *   Decay: light<4 above + opacity>2 → dirt
 *   Spread: if light≥9, 4 attempts:
 *     ±1 XZ, ±3 Y (rand(5)-3)
 *     Target must be dirt meta 0
 *     Target above: light≥4 + opacity≤2
 *
 * Drops: dirt item
 *
 * Block ID: mycelium(110)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * CROPS (BlockCrops)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockBush, implements IGrowable
 * Material: plants, hardness 0.0, soundTypeGrass
 * Bounds: full XZ, 0.25 tall, not in creative tab
 *
 * Placement: only on farmland
 *
 * Growth (updateTick):
 *   Requires light ≥ 9 at Y+1
 *   Meta 0-6 (growing), 7 (mature)
 *   Growth rate (func_149864_n):
 *     3×3 soil scan at Y-1:
 *       farmland → 1.0 (dry) or 3.0 (hydrated, meta>0)
 *       Non-center: ÷4
 *     Row penalty: if diagonal crop OR (X-row AND Z-row) → ÷2
 *   Chance: 1/(25/rate + 1)
 *
 * Bonemeal: +rand(2,5), capped at 7
 *
 * Drops:
 *   Meta 7 (mature): getCrop() = wheat
 *   Else: getSeed() = wheat_seeds
 *   Extra seeds at meta 7: (3+fortune) attempts, rand(15)>meta
 *
 * Render type: 6
 *
 * Block ID: wheat(59)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * GRASS (BlockGrass)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: grass, implements IGrowable
 * setTickRandomly(true)
 *
 * Spread (same as mycelium):
 *   Decay: light<4 + opacity>2 → dirt
 *   Spread: 4 attempts to dirt meta 0, light≥4, ≤2 opacity
 *
 * Bonemeal (fertilize): 128 attempts
 *   Random walk: ±1 XZ per step, complex Y walk
 *   Steps: i/16 walk iterations
 *   7/8 chance: tallgrass(meta 1) if canBlockStay
 *   1/8 chance: biome flower via func_150572_a
 *
 * Drops: dirt item
 *
 * Block ID: grass(2)
 *
 * Thread safety: Growth and spread on server tick thread.
 * JNI readiness: Growth/bonemeal events for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Mushroom Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace MushroomConstants {
    // ─── Block IDs ───
    static constexpr int32_t BROWN_ID = 39;
    static constexpr int32_t RED_ID = 40;

    // ─── Bounds ───
    static constexpr float HALF_WIDTH = 0.2f;
    static constexpr float HEIGHT = 0.4f;              // 2 × 0.2

    // ─── Spreading ───
    static constexpr int32_t SPREAD_CHANCE = 25;        // 1 in 25
    static constexpr int32_t MAX_DENSITY = 5;
    static constexpr int32_t DENSITY_RADIUS_XZ = 4;
    static constexpr int32_t DENSITY_RADIUS_Y = 1;
    static constexpr int32_t WALK_STEPS = 4;

    // ─── canBlockStay ───
    static constexpr int32_t MAX_BLOCK_LIGHT = 13;     // light < 13 for full blocks
    static constexpr int32_t MYCELIUM_ID = 110;
    static constexpr int32_t DIRT_ID = 3;
    static constexpr int32_t PODZOL_META = 2;

    // ─── Bonemeal ───
    static constexpr float FERTILIZE_CHANCE = 0.4f;     // 40%
}

// ═══════════════════════════════════════════════════════════════════════════
// Huge Mushroom Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace HugeMushroomConstants {
    // ─── Block IDs ───
    static constexpr int32_t BROWN_BLOCK_ID = 99;
    static constexpr int32_t RED_BLOCK_ID = 100;

    // ─── Types ───
    static constexpr int32_t BROWN_TYPE = 0;
    static constexpr int32_t RED_TYPE = 1;

    // ─── Drops ───
    // quantity = max(0, rand(10) - 7) → 0-3
    // Drop: brown_mushroom(39) + type offset
    static constexpr int32_t DROP_ROLL = 10;
    static constexpr int32_t DROP_OFFSET = 7;
}

// ═══════════════════════════════════════════════════════════════════════════
// Mycelium Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace MyceliumConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 110;

    // ─── Spread ───
    static constexpr int32_t SPREAD_ATTEMPTS = 4;
    static constexpr int32_t SPREAD_RADIUS_XZ = 1;     // ±1
    static constexpr int32_t SPREAD_RADIUS_Y = 3;      // rand(5)-3

    // ─── Decay ───
    static constexpr int32_t DARK_THRESHOLD = 4;        // light < 4
    static constexpr int32_t OPACITY_THRESHOLD = 2;     // opacity > 2 blocks above

    // ─── Target ───
    static constexpr int32_t DIRT_ID = 3;
    static constexpr int32_t DIRT_META = 0;             // normal dirt only
}

// ═══════════════════════════════════════════════════════════════════════════
// Crops Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CropsConstants {
    // ─── Block ID ───
    static constexpr int32_t WHEAT_ID = 59;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 6;

    // ─── Bounds ───
    static constexpr float HEIGHT = 0.25f;

    // ─── Growth ───
    static constexpr int32_t MATURE_META = 7;
    static constexpr int32_t MIN_LIGHT = 9;

    // ─── Growth rate formula ───
    // Base: farmland below = 1.0 (dry) or 3.0 (hydrated)
    // Non-center: ÷4
    // Row/diagonal penalty: ÷2
    // Chance: 1 / (floor(25.0/rate) + 1)
    static constexpr float BASE_CHANCE_DIVISOR = 25.0f;
    static constexpr float DRY_FARMLAND = 1.0f;
    static constexpr float WET_FARMLAND = 3.0f;
    static constexpr float NON_CENTER_DIVISOR = 4.0f;
    static constexpr float ROW_PENALTY = 2.0f;

    // ─── Placement ───
    static constexpr int32_t FARMLAND_ID = 60;

    // ─── Bonemeal ───
    static constexpr int32_t BONEMEAL_MIN = 2;
    static constexpr int32_t BONEMEAL_MAX = 5;

    // ─── Drops ───
    static constexpr int32_t WHEAT_ITEM_ID = 296;
    static constexpr int32_t WHEAT_SEEDS_ITEM_ID = 295;
    // Extra seeds: (3+fortune) attempts, rand(15)>meta
    static constexpr int32_t SEED_DROP_CHANCE = 15;
    static constexpr int32_t BASE_SEED_DROPS = 3;
}

// ═══════════════════════════════════════════════════════════════════════════
// Grass Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace GrassConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 2;

    // ─── Spread ───
    // Same as mycelium: 4 attempts, ±1 XZ, ±3 Y, dirt meta 0

    // ─── Bonemeal ───
    static constexpr int32_t FERTILIZE_ATTEMPTS = 128;
    static constexpr int32_t TALLGRASS_CHANCE = 8;      // 1/8 flower, 7/8 tallgrass
    static constexpr int32_t TALLGRASS_ID = 31;
    static constexpr int32_t TALLGRASS_META = 1;         // tall grass variety

    // ─── Drops ───
    static constexpr int32_t DIRT_ID = 3;
}

} // namespace mccpp
