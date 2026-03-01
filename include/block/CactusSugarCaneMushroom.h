/**
 * CactusSugarCaneMushroom.h — Cactus, sugar cane, and mushroom growth mechanics.
 *
 * Java references:
 *   - net.minecraft.block.BlockCactus (104 lines)
 *   - net.minecraft.block.BlockReed (112 lines)
 *   - net.minecraft.block.BlockMushroom (110 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * CACTUS (BlockCactus)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: cactus, random tick enabled
 *
 * Growth (updateTick):
 *   1. Air block above required
 *   2. Count cactus blocks below (height check)
 *   3. If height < 3:
 *      - meta == 15: grow new cactus above, reset meta to 0
 *      - else: meta += 1
 *   → Grows every 16 random ticks (average ~68 minutes)
 *
 * Placement (canBlockStay):
 *   - No solid material on any cardinal side (NSEW)
 *   - Block below must be cactus OR sand
 *
 * Collision box: inset by 1/16 on all sides
 *   Java: (x+0.0625, y, z+0.0625) to (x+1-0.0625, y+1-0.0625, z+1-0.0625)
 *
 * Entity contact damage: 1.0 (DamageSource.cactus)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SUGAR CANE / REEDS (BlockReed)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: plants, random tick enabled
 * Visual bounds: 0.375 wide (3/8 centered), full height
 * No collision box (entities pass through)
 *
 * Growth (updateTick):
 *   Same as cactus: max 3 high, meta 0-15 counter, grows on meta 15
 *   Extra condition: must be on top of reeds OR valid base
 *
 * Placement (canPlaceBlockAt / canBlockStay):
 *   - On top of reeds: always valid
 *   - On grass, dirt, or sand: requires water in cardinal adjacent
 *     at Y-1 level (same Y as soil block)
 *
 * Drop: sugar cane item (Items.reeds)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * MUSHROOM (BlockMushroom)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockBush, implements IGrowable
 * Visual bounds: 0.2 wide (1/5 centered), 0.4 tall
 * Random tick enabled
 *
 * Spreading (updateTick — 4% chance per tick):
 *   1. Density check: count same mushroom in 9×3×9 (x±4, y±1, z±4)
 *      If 5+ found → stop spreading
 *   2. Random walk: 4 attempts
 *      Each step: x ± rand(3)-1, y ± (rand(2)-rand(2)), z ± rand(3)-1
 *      If air & canBlockStay at walk pos → update origin
 *   3. Final position: if air & canBlockStay → place mushroom
 *
 * Placement (canBlockStay):
 *   Priority order:
 *   a) On mycelium → always valid
 *   b) On podzol (dirt meta 2) → always valid
 *   c) Light < 13 AND block below is fullBlock → valid
 *
 * Bone meal (IGrowable):
 *   - canFertilize: always true
 *   - shouldFertilize: 40% chance (rand.nextFloat() < 0.4)
 *   - fertilize: attempt big mushroom generation
 *     - brown_mushroom → WorldGenBigMushroom(type=0)
 *     - red_mushroom → WorldGenBigMushroom(type=1)
 *     - If generation fails: restore original mushroom
 *
 * Thread safety: Block updates on server thread.
 * JNI readiness: Simple constants and growth logic.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Cactus Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CactusConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 81;
    static constexpr int32_t SAND_ID = 12;

    // ─── Growth ───
    // Max height: 3 blocks
    static constexpr int32_t MAX_HEIGHT = 3;
    // Growth counter: meta 0-15, grows when meta reaches 15
    static constexpr int32_t GROWTH_THRESHOLD = 15;

    // ─── Collision box inset ───
    // Java: 0.0625f (1/16 block)
    static constexpr float COLLISION_INSET = 0.0625f;

    // ─── Entity damage ───
    // Java: DamageSource.cactus, 1.0f
    static constexpr float CONTACT_DAMAGE = 1.0f;
    static constexpr const char* DAMAGE_TYPE = "cactus";

    // ─── Placement ───
    // Must be on cactus or sand
    // No solid material on 4 cardinal sides
}

// ═══════════════════════════════════════════════════════════════════════════
// Sugar Cane / Reeds Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SugarCaneConstants {
    // ─── Block IDs ───
    static constexpr int32_t REEDS_BLOCK_ID = 83;
    static constexpr int32_t GRASS_ID = 2;
    static constexpr int32_t DIRT_ID = 3;
    static constexpr int32_t SAND_ID = 12;

    // ─── Growth (identical to cactus) ───
    static constexpr int32_t MAX_HEIGHT = 3;
    static constexpr int32_t GROWTH_THRESHOLD = 15;

    // ─── Visual bounds ───
    // Java: 0.375 wide centered (0.125 to 0.875, full height)
    static constexpr float BOUNDS_WIDTH = 0.375f;
    static constexpr float BOUNDS_MIN_XZ = 0.5f - 0.375f;  // 0.125
    static constexpr float BOUNDS_MAX_XZ = 0.5f + 0.375f;  // 0.875

    // ─── Drop item ───
    static constexpr int32_t REEDS_ITEM_ID = 338;  // Items.reeds (sugar cane item)

    // ─── Placement ───
    // Base: grass, dirt, or sand
    // Requires water adjacent to base block (same Y as soil, cardinal NSEW)
}

// ═══════════════════════════════════════════════════════════════════════════
// Mushroom Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace MushroomConstants {
    // ─── Block IDs ───
    static constexpr int32_t BROWN_MUSHROOM_ID = 39;
    static constexpr int32_t RED_MUSHROOM_ID = 40;
    static constexpr int32_t BROWN_MUSHROOM_BLOCK_ID = 99;   // big mushroom
    static constexpr int32_t RED_MUSHROOM_BLOCK_ID = 100;    // big mushroom
    static constexpr int32_t MYCELIUM_ID = 110;
    static constexpr int32_t DIRT_ID = 3;
    static constexpr int32_t PODZOL_META = 2;   // dirt meta 2 = podzol

    // ─── Visual bounds ───
    // Java: 0.2 wide centered, 0.4 tall
    static constexpr float BOUNDS_WIDTH = 0.2f;
    static constexpr float BOUNDS_MIN_XZ = 0.5f - 0.2f;   // 0.3
    static constexpr float BOUNDS_MAX_XZ = 0.5f + 0.2f;    // 0.7
    static constexpr float BOUNDS_HEIGHT = 0.4f;            // 0.2 * 2

    // ─── Spreading ───
    // 4% chance per random tick: rand.nextInt(25) == 0
    static constexpr int32_t SPREAD_CHANCE = 25;

    // Density limit: same as vine
    static constexpr int32_t DENSITY_LIMIT = 5;
    static constexpr int32_t DENSITY_X_RANGE = 4;
    static constexpr int32_t DENSITY_Y_RANGE = 1;
    static constexpr int32_t DENSITY_Z_RANGE = 4;

    // Random walk: 4 steps
    static constexpr int32_t WALK_STEPS = 4;
    // Walk deltas:
    // X: rand(3) - 1 = -1..+1
    // Y: rand(2) - rand(2) = -1..+1
    // Z: rand(3) - 1 = -1..+1

    // ─── Light check ───
    // Java: getFullBlockLightValue(x, y, z) < 13
    static constexpr int32_t MAX_LIGHT_LEVEL = 13;

    // ─── Bone meal ───
    // canFertilize: always true
    // shouldFertilize: 40% chance
    static constexpr float FERTILIZE_CHANCE = 0.4f;
    // fertilize → WorldGenBigMushroom
    //   brown = type 0, red = type 1

    // ─── Big mushroom generation ───
    static constexpr int32_t BIG_MUSHROOM_TYPE_BROWN = 0;
    static constexpr int32_t BIG_MUSHROOM_TYPE_RED = 1;
}

// ═══════════════════════════════════════════════════════════════════════════
// Column Growth Helper (shared by cactus and sugar cane)
// ═══════════════════════════════════════════════════════════════════════════

namespace ColumnGrowthConstants {
    // Both cactus and sugar cane share:
    // - Max 3 blocks high
    // - Meta 0-15 growth counter (16 random ticks per growth)
    // - Grow only if air above
    // - Count own blocks below to check height

    static constexpr int32_t MAX_HEIGHT = 3;
    static constexpr int32_t META_MAX = 15;

    // Average growth time per block:
    // Random tick every ~68.27 seconds (on average)
    // × 16 ticks = ~18.2 minutes per growth
    // (Random tick: 3 ticks per chunk section per game tick at tickSpeed 3)
}

} // namespace mccpp
