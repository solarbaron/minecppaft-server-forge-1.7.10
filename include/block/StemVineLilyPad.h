/**
 * StemVineLilyPad.h — Melon/pumpkin stems, vine spreading, lily pad.
 *
 * Java references:
 *   - net.minecraft.block.BlockStem (187 lines)
 *   - net.minecraft.block.BlockVine (289 lines)
 *   - net.minecraft.block.BlockLilyPad (57 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * STEM (BlockStem)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: plants, extends BlockBush, implements IGrowable
 * Placement: farmland only, no creative tab
 * Width: 2/16 centered (0.375→0.625), render type 19
 *
 * Height based on growth: (meta * 2 + 2) / 16.0
 *   Meta 0 = 2/16 = 0.125
 *   Meta 7 = 16/16 = 1.0
 *
 * Growth: same rate formula as crops (func_149875_n = func_149864_n)
 *   Light >= 9, chance = 1 in (floor(25/rate) + 1)
 *
 * Fruit spawning (meta == 7, mature):
 *   1. Check all 4 cardinal neighbors for existing fruit → abort if found
 *   2. Pick random direction (0-3)
 *   3. Target pos: (x ± 1, y, z ± 1)
 *   4. Target must be air AND below must be farmland/dirt/grass
 *   5. Place fruit block (field_149877_a = pumpkin or melon_block)
 *
 * Bonemeal: +rand(2,5) capped at 7
 *
 * Drops:
 *   getItemDropped: null (handled by dropBlockAsItemWithChance)
 *   3 drop attempts: if rand(15) <= meta → drop seed
 *   Pumpkin stem → pumpkin_seeds, Melon stem → melon_seeds
 *
 * Block IDs: pumpkin_stem (104), melon_stem (105)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * VINE (BlockVine)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: vine, tick randomly, no collision (passable)
 *
 * Metadata bitmask (4 faces):
 *   bit 0 (1) = south face
 *   bit 1 (2) = west face
 *   bit 2 (4) = north face
 *   bit 3 (8) = east face
 *
 * Placement face→meta mapping (onBlockPlaced):
 *   face 2 → meta 1 (south)
 *   face 3 → meta 4 (north)
 *   face 4 → meta 8 (east)
 *   face 5 → meta 2 (west)
 *
 * Support check (func_150093_a):
 *   Block must be renderAsNormalBlock AND material.blocksMovement
 *
 * Bounds per face (1/16 thick):
 *   South (bit 0): z = 15/16→1
 *   West (bit 1): x = 0→1/16
 *   North (bit 2): z = 0→1/16
 *   East (bit 3): x = 15/16→1
 *   No sides + solid above: ceiling (y = 15/16→1)
 *
 * Neighbor validation (func_150094_e):
 *   Each face bit: must have wall support OR vine above with same bit
 *   If no faces left and no solid above → unsupported → break
 *
 * Spread algorithm (updateTick, 1/4 chance):
 *   Density limit: max 4 vines in 9×9×3 area (radius 4 xz, ±1 y)
 *   Counts to 5 then stops spreading.
 *
 *   Random direction (0-5):
 *   1 (up): extend upward, copy random subset of faces, validate walls
 *   2-5 (horizontal): wrap around corners to adjacent blocks
 *     - Try CW neighbor wall, CCW neighbor wall
 *     - Try CW wrap, CCW wrap
 *     - Try opposite wall above
 *   0 or fallthrough (down): extend downward, copy random face subset
 *     Or merge with existing vine below
 *
 * Drops: nothing (shears = vine block)
 * Render type: 20
 *
 * Block ID: 106
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LILY PAD (BlockLilyPad)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: plants, extends BlockBush
 * Height: 1/64 (0.015625), full width
 * Render type: 23
 *
 * Placement: only on still water (Blocks.water)
 * canBlockStay: material below = water AND meta below = 0 (still)
 *   y must be 0-255
 *
 * Collision: has collision box (unlike most plants)
 *   BUT boats pass through (EntityBoat excluded from collision)
 *
 * Block ID: 111
 *
 * Thread safety: Block tick on server thread.
 * JNI readiness: Direction arrays for vine spread.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Stem Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace StemConstants {
    // ─── Block IDs ───
    static constexpr int32_t PUMPKIN_STEM_ID = 104;
    static constexpr int32_t MELON_STEM_ID = 105;

    // ─── Fruit blocks ───
    static constexpr int32_t PUMPKIN_ID = 86;
    static constexpr int32_t MELON_BLOCK_ID = 103;

    // ─── Seeds ───
    static constexpr int32_t PUMPKIN_SEEDS_ID = 361;
    static constexpr int32_t MELON_SEEDS_ID = 362;

    // ─── Growth ───
    static constexpr int32_t MAX_GROWTH = 7;
    static constexpr int32_t MIN_LIGHT = 9;

    // ─── Height formula ───
    // (meta * 2 + 2) / 16.0
    inline float stemHeight(int32_t meta) {
        return static_cast<float>(meta * 2 + 2) / 16.0f;
    }

    // ─── Bounds ───
    static constexpr float HALF_WIDTH = 0.125f;  // 2/16 = 1/8

    // ─── Bonemeal ───
    static constexpr int32_t BONEMEAL_MIN = 2;
    static constexpr int32_t BONEMEAL_MAX = 5;

    // ─── Fruit placement ───
    // Target must be air, below must be farmland (60) or dirt (3) or grass (2)
    static constexpr int32_t FARMLAND_ID = 60;
    static constexpr int32_t DIRT_ID = 3;
    static constexpr int32_t GRASS_ID = 2;

    // ─── Seed drops ───
    static constexpr int32_t SEED_DROP_ATTEMPTS = 3;
    static constexpr int32_t SEED_DROP_DENOMINATOR = 15;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 19;
}

// ═══════════════════════════════════════════════════════════════════════════
// Vine Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace VineConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 106;

    // ─── Face bitmask ───
    static constexpr int32_t FACE_SOUTH = 1;   // bit 0
    static constexpr int32_t FACE_WEST = 2;    // bit 1
    static constexpr int32_t FACE_NORTH = 4;   // bit 2
    static constexpr int32_t FACE_EAST = 8;    // bit 3

    // ─── Placement face→meta mapping ───
    // face 2→1(S), face 3→4(N), face 4→8(E), face 5→2(W)
    inline int32_t faceToMeta(int32_t face) {
        switch(face) {
            case 2: return FACE_SOUTH;
            case 3: return FACE_NORTH;
            case 4: return FACE_EAST;
            case 5: return FACE_WEST;
            default: return 0;
        }
    }

    // ─── Bounds (1/16 thick per face) ───
    static constexpr float THICKNESS = 0.0625f;  // 1/16

    // ─── Spread ───
    static constexpr int32_t SPREAD_CHANCE = 4;      // 1 in 4 per tick
    static constexpr int32_t DENSITY_RADIUS_XZ = 4;  // 9×9
    static constexpr int32_t DENSITY_RADIUS_Y = 1;   // ±1 = 3 tall
    static constexpr int32_t DENSITY_LIMIT = 5;      // max 4 vines (stop at 5th)

    // ─── Direction offsets ───
    // Direction.offsetX: {0, -1, 0, 1} for indices 0-3
    // Direction.offsetZ: {-1, 0, 1, 0} for indices 0-3
    static constexpr int32_t OFFSET_X[4] = {0, -1, 0, 1};
    static constexpr int32_t OFFSET_Z[4] = {-1, 0, 1, 0};

    // Direction.facingToDirection maps face indices to direction indices
    static constexpr int32_t FACING_TO_DIRECTION[6] = {-1, -1, 2, 0, 1, 3};

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 20;
}

// ═══════════════════════════════════════════════════════════════════════════
// Lily Pad Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LilyPadConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 111;

    // ─── Bounds ───
    static constexpr float HEIGHT = 0.015625f;  // 1/64

    // ─── Placement ───
    static constexpr int32_t STILL_WATER_ID = 9;   // Blocks.water
    // canBlockStay: material below = water AND meta below = 0

    // ─── Collision ───
    // Has collision for all entities EXCEPT EntityBoat

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 23;
}

} // namespace mccpp
