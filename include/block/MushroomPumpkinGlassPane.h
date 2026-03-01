/**
 * MushroomPumpkinGlassPane.h — Huge mushroom, pumpkin golem spawns, colored panes.
 *
 * Java references:
 *   - net.minecraft.block.BlockHugeMushroom (37 lines)
 *   - net.minecraft.block.BlockPumpkin (94 lines)
 *   - net.minecraft.block.BlockStainedGlassPane (27 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * HUGE MUSHROOM (BlockHugeMushroom)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood (for brown) or wood (for red)
 * field_149792_b: 0 = brown, 1 = red (offset from brown_mushroom_block)
 *
 * Drop quantity: max(0, rand.nextInt(10) - 7)
 *   Range: 0 (70% chance), 1 (10%), 2 (10%), (but clamp means 0-2)
 *   Actually: rand(10) produces 0-9, minus 7 gives -7 to 2
 *   Clamped to 0 → 70% chance 0, 10% chance 1, 10% chance 2
 *
 * Drop item: brown_mushroom + field_149792_b offset
 *   Brown huge → drops brown_mushroom (39)
 *   Red huge → drops red_mushroom (40)
 *
 * Block IDs: brown_mushroom_block (99), red_mushroom_block (100)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PUMPKIN (BlockPumpkin)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: gourd, extends BlockDirectional
 * Tick randomly, needs solidTopSurface below
 * field_149985_a: true = jack o'lantern, false = pumpkin
 *
 * Placement: yaw → meta (floor(yaw * 4 / 360 + 2.5) & 3)
 * canPlaceBlock: replaceable material + solid below
 *
 * Golem spawn (onBlockAdded):
 *
 * 1. SNOW GOLEM — Pumpkin + 2 snow blocks below
 *    Pattern:
 *      [P]  ← pumpkin (placed block)
 *      [S]  ← snow block
 *      [S]  ← snow block
 *    Clears all 3 blocks, spawns EntitySnowman at (x+0.5, y-1.95, z+0.5)
 *    120 snowshovel particles
 *
 * 2. IRON GOLEM — Pumpkin + T-shape iron blocks
 *    Pattern (X-axis arm):
 *              [P]
 *      [Fe] [Fe] [Fe]     ← 3 iron blocks at y-1 (arms + body)
 *              [Fe]       ← 1 iron block at y-2 (body)
 *    OR pattern (Z-axis arm):
 *      Same but rotated 90°
 *
 *    Checks: 2 iron blocks below (body) + 2 iron blocks to either side
 *    of middle body block (arms in either X or Z axis)
 *    Clears T-shape + pumpkin, spawns EntityIronGolem with setPlayerCreated(true)
 *    at (x+0.5, y-1.95, z+0.5), 120 snowballpoof particles
 *
 * Block IDs: pumpkin (86), lit_pumpkin (91)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * STAINED GLASS PANE (BlockStainedGlassPane)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: glass, extends BlockPane
 * 16 colors (meta 0-15), damageDropped = meta
 * Same connection logic as glass panes/iron bars
 *
 * Block ID: 160
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Simple metadata and golem creation hooks.
 */
#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Huge Mushroom Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace HugeMushroomConstants {
    // ─── Block IDs ───
    static constexpr int32_t BROWN_HUGE_ID = 99;
    static constexpr int32_t RED_HUGE_ID = 100;

    // ─── Mushroom type (field_149792_b) ───
    static constexpr int32_t TYPE_BROWN = 0;
    static constexpr int32_t TYPE_RED = 1;

    // ─── Small mushroom IDs (drop targets) ───
    static constexpr int32_t BROWN_MUSHROOM_ID = 39;
    static constexpr int32_t RED_MUSHROOM_ID = 40;

    // ─── Drop quantity ───
    // Java: max(0, rand.nextInt(10) - 7)
    // 0-7 → 0 (80%), 8 → 1 (10%), 9 → 2 (10%)
    static constexpr int32_t DROP_RAND_BOUND = 10;
    static constexpr int32_t DROP_OFFSET = 7;

    inline int32_t quantityDropped(int32_t randVal) {
        return std::max(0, randVal - DROP_OFFSET);
    }

    // ─── Metadata (mushroom face configuration) ───
    // 0 = all pore faces (interior)
    // 1-9 = skin on specific faces
    // 10 = stem (pore top/bottom, stem sides)
    // 14 = all faces are skin
    // 15 = all faces are stem
    static constexpr int32_t META_ALL_PORES = 0;
    static constexpr int32_t META_STEM = 10;
    static constexpr int32_t META_ALL_FACES = 14;
    static constexpr int32_t META_ALL_STEM = 15;
}

// ═══════════════════════════════════════════════════════════════════════════
// Pumpkin Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PumpkinConstants {
    // ─── Block IDs ───
    static constexpr int32_t PUMPKIN_ID = 86;
    static constexpr int32_t LIT_PUMPKIN_ID = 91;

    // ─── Golem materials ───
    static constexpr int32_t SNOW_BLOCK_ID = 80;
    static constexpr int32_t IRON_BLOCK_ID = 42;

    // ─── Snow Golem spawn ───
    // Pattern: pumpkin (top) + 2 snow blocks below
    // Entity spawns at (x+0.5, y-1.95, z+0.5)
    static constexpr double GOLEM_Y_OFFSET = -1.95;
    static constexpr int32_t SNOW_GOLEM_BODY_HEIGHT = 2;  // 2 snow blocks
    static constexpr int32_t PARTICLE_COUNT = 120;

    // ─── Iron Golem spawn ───
    // T-shape: 2 iron vertical (body) + 2 iron horizontal (arms)
    // Iron golem is setPlayerCreated(true) when built by player
    // Body: (x, y-1) and (x, y-2) — both iron
    // Arms: either (x-1, y-1) + (x+1, y-1) for X-axis
    //    or (x, y-1, z-1) + (x, y-1, z+1) for Z-axis

    // ─── Placement yaw ───
    // Java: (floor(yaw * 4 / 360 + 2.5) & 3)
    inline int32_t getFacingFromYaw(float yaw) {
        return static_cast<int32_t>(std::floor(yaw * 4.0f / 360.0f + 2.5f)) & 3;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Stained Glass Pane Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace StainedGlassPaneConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 160;

    // ─── Colors (same as stained glass, meta 0-15) ───
    static constexpr int32_t NUM_COLORS = 16;

    // Extends BlockPane — uses same connection logic
    // damageDropped = meta (preserves color)
    // Glass material, not solid
}

} // namespace mccpp
