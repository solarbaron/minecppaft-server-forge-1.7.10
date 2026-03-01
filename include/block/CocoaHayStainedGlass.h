/**
 * CocoaHayStainedGlass.h — Cocoa pods, hay bales, and stained glass.
 *
 * Java references:
 *   - net.minecraft.block.BlockCocoa (161 lines)
 *   - net.minecraft.block.BlockHay (18 lines)
 *   - net.minecraft.block.BlockStainedGlass (42 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * COCOA POD (BlockCocoa)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: plants, extends BlockDirectional, implements IGrowable
 * Not opaque, not normal, render type 28, tick randomly
 *
 * Metadata layout:
 *   bits 0-1: facing direction (0-3)
 *   bits 2-3: growth stage (0-2)
 *     func_149987_c(meta) = (meta & 0xC) >> 2
 *
 * Growth stages:
 *   Stage 0: 4/16 wide, 5/16 tall
 *   Stage 1: 6/16 wide, 7/16 tall
 *   Stage 2 (mature): 8/16 wide, 9/16 tall
 *   Width formula: 4 + stage * 2
 *   Height formula: 5 + stage * 2
 *
 * Bounds per facing (all at Y: (12-height)/16 → 0.75):
 *   Dir 0 (south): pod on south side of log
 *     X: (8-w/2)/16 → (8+w/2)/16
 *     Z: (15-w)/16 → 15/16
 *   Dir 2 (north): pod on north side
 *     X: (8-w/2)/16 → (8+w/2)/16
 *     Z: 1/16 → (1+w)/16
 *   Dir 1 (west): pod on west side
 *     X: 1/16 → (1+w)/16
 *     Z: (8-w/2)/16 → (8+w/2)/16
 *   Dir 3 (east): pod on east side
 *     X: (15-w)/16 → 15/16
 *     Z: (8-w/2)/16 → (8+w/2)/16
 *
 * Growth: 1/5 chance per random tick if stage < 2
 * canBlockStay: adjacent block in facing direction is jungle log
 *   (Blocks.log with func_150165_c(meta) == 3 → jungle)
 *
 * Drops:
 *   Stage 0-1: 1× dye:3 (cocoa beans)
 *   Stage 2 (mature): 3× dye:3
 *
 * Bonemeal: always succeeds, advances 1 stage
 * Placement: yaw → direction, side → rotateOpposite
 *
 * Block ID: 127
 *
 * ═══════════════════════════════════════════════════════════════════════
 * HAY BALE (BlockHay)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: grass, extends BlockRotatedPillar
 * Metadata: axis rotation (same as log)
 *   bits 0-1: unused (subtypes)
 *   bits 2-3: axis (0=YY, 4=XX, 8=ZZ)
 *
 * Simply a rotatable pillar block with grass material
 * Full block, opaque, normal
 *
 * Block ID: 170
 *
 * ═══════════════════════════════════════════════════════════════════════
 * STAINED GLASS (BlockStainedGlass)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: glass, extends BlockBreakable
 * Not normal (renderAsNormalBlock = false)
 * 16 colors (metadata 0-15)
 *
 * Drops: NOTHING on normal break (quantityDropped = 0)
 * Silk harvest: yes → drops colored glass block
 * damageDropped = metadata (preserves color)
 *
 * Block ID: 95
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Simple metadata layout.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Cocoa Pod Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CocoaConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 127;

    // ─── Metadata ───
    static constexpr int32_t FACING_MASK = 3;      // bits 0-1
    static constexpr int32_t STAGE_MASK = 0xC;      // bits 2-3
    static constexpr int32_t STAGE_SHIFT = 2;

    inline int32_t getDirection(int32_t meta) { return meta & FACING_MASK; }
    inline int32_t getStage(int32_t meta) { return (meta & STAGE_MASK) >> STAGE_SHIFT; }
    inline int32_t makeMeta(int32_t direction, int32_t stage) {
        return (stage << STAGE_SHIFT) | (direction & FACING_MASK);
    }

    // ─── Growth ───
    static constexpr int32_t MAX_STAGE = 2;
    static constexpr int32_t GROWTH_CHANCE = 5;  // 1/5 per tick

    // ─── Size per stage ───
    // Width: 4 + stage * 2 (in sixteenths)
    // Height: 5 + stage * 2 (in sixteenths)
    inline int32_t getWidth(int32_t stage) { return 4 + stage * 2; }
    inline int32_t getHeight(int32_t stage) { return 5 + stage * 2; }

    // ─── Drops ───
    static constexpr int32_t DROP_IMMATURE = 1;  // stages 0-1
    static constexpr int32_t DROP_MATURE = 3;     // stage 2
    static constexpr int32_t DYE_DAMAGE = 3;      // cocoa beans dye color

    // ─── Support ───
    // Requires jungle log (Blocks.log with wood type 3)
    static constexpr int32_t LOG_ID = 17;
    static constexpr int32_t JUNGLE_WOOD_TYPE = 3;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 28;
}

// ═══════════════════════════════════════════════════════════════════════════
// Hay Bale Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace HayConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 170;

    // ─── Rotation axis (same as rotated pillar) ───
    static constexpr int32_t AXIS_MASK = 0xC;   // bits 2-3
    static constexpr int32_t AXIS_Y = 0;         // 0b00 = vertical
    static constexpr int32_t AXIS_X = 4;         // 0b01 = east-west
    static constexpr int32_t AXIS_Z = 8;         // 0b10 = north-south
}

// ═══════════════════════════════════════════════════════════════════════════
// Stained Glass Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace StainedGlassConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 95;

    // ─── Colors (metadata 0-15) ───
    static constexpr int32_t NUM_COLORS = 16;
    static constexpr int32_t WHITE = 0;
    static constexpr int32_t ORANGE = 1;
    static constexpr int32_t MAGENTA = 2;
    static constexpr int32_t LIGHT_BLUE = 3;
    static constexpr int32_t YELLOW = 4;
    static constexpr int32_t LIME = 5;
    static constexpr int32_t PINK = 6;
    static constexpr int32_t GRAY = 7;
    static constexpr int32_t SILVER = 8;
    static constexpr int32_t CYAN = 9;
    static constexpr int32_t PURPLE = 10;
    static constexpr int32_t BLUE = 11;
    static constexpr int32_t BROWN = 12;
    static constexpr int32_t GREEN = 13;
    static constexpr int32_t RED = 14;
    static constexpr int32_t BLACK = 15;

    // ─── Drops ───
    static constexpr int32_t DROP_QUANTITY = 0;  // no drops, silk only
}

} // namespace mccpp
