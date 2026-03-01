/**
 * QuartzColoredClay.h — Quartz pillar variants and colored block system.
 *
 * Java references:
 *   - net.minecraft.block.BlockQuartz (74 lines)
 *   - net.minecraft.block.BlockColored (37 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * QUARTZ BLOCK (BlockQuartz)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: rock, render type 39
 * MapColor: quartz
 *
 * Metadata / Subtypes:
 *   0 = "default" — plain quartz block
 *   1 = "chiseled" — chiseled quartz
 *   2 = "lines" — pillar quartz (Y axis, vertical)
 *   3 = "lines" — pillar quartz (X axis, east-west)
 *   4 = "lines" — pillar quartz (Z axis, north-south)
 *
 * Placement (onBlockPlaced, only when meta == 2 i.e. lines):
 *   side 0,1 (top/bottom) → meta 2 (Y axis)
 *   side 2,3 (north/south) → meta 4 (Z axis)
 *   side 4,5 (east/west)   → meta 3 (X axis)
 *
 * Drops:
 *   meta 0 → drops 0 (default)
 *   meta 1 → drops 1 (chiseled)
 *   meta 2 → drops 2 (lines Y)
 *   meta 3 → drops 2 (rotated X → normalizes to lines)
 *   meta 4 → drops 2 (rotated Z → normalizes to lines)
 *   All rotations collapse to meta 2 on drop
 *
 * Silk touch: meta 3,4 → creates stack with meta 2
 *
 * Block ID: 155
 *
 * ═══════════════════════════════════════════════════════════════════════
 * COLORED BLOCK (BlockColored)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Base class for 16-color blocks.
 * Used by: wool (35), stained hardened clay (159)
 *
 * Metadata: 0-15 = dye color
 * damageDropped = metadata (preserves color)
 *
 * Color inversion (func_150031_c / func_150032_b):
 *   Converts between dye damage and block color:
 *   result = ~n & 0xF
 *   This is a bitwise NOT masked to 4 bits.
 *   0↔15, 1↔14, 2↔13, ... 7↔8
 *
 *   Used because wool/dye colors are ordered inversely:
 *     Block meta 0 = white, Dye damage 15 = bone meal (white)
 *     Block meta 15 = black, Dye damage 0 = ink sac (black)
 *
 * MapColor: uses getMapColorForBlockColored(meta)
 *
 * Hardened clay variants:
 *   Regular hardened clay (172): plain block, no metadata
 *   Stained hardened clay (159): BlockColored with 16 colors
 *
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Simple metadata layout.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Quartz Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace QuartzConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 155;

    // ─── Subtypes ───
    static constexpr int32_t DEFAULT = 0;      // plain quartz
    static constexpr int32_t CHISELED = 1;     // chiseled quartz
    static constexpr int32_t LINES_Y = 2;      // pillar vertical
    static constexpr int32_t LINES_X = 3;      // pillar east-west
    static constexpr int32_t LINES_Z = 4;      // pillar north-south

    // ─── Type names ───
    static constexpr const char* TYPE_NAMES[] = {"default", "chiseled", "lines"};
    static constexpr int32_t NUM_ITEM_TYPES = 3;

    // ─── Placement axis mapping (for lines type, meta == 2) ───
    // Side clicked → resulting meta
    // side 0,1 (top/bottom) → 2 (Y axis)
    // side 2,3 (north/south) → 4 (Z axis)
    // side 4,5 (east/west) → 3 (X axis)
    inline int32_t getAxisMeta(int32_t side) {
        switch (side) {
            case 0: case 1: return 2;  // Y axis
            case 2: case 3: return 4;  // Z axis
            case 4: case 5: return 3;  // X axis
            default: return 2;
        }
    }

    // ─── Drop normalization ───
    // Rotated pillar meta 3,4 → drop as meta 2
    inline int32_t damageDropped(int32_t meta) {
        return (meta == 3 || meta == 4) ? 2 : meta;
    }

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 39;
}

// ═══════════════════════════════════════════════════════════════════════════
// Colored Block Constants (Wool, Stained Clay, etc.)
// ═══════════════════════════════════════════════════════════════════════════

namespace ColoredBlockConstants {
    // ─── Block IDs ───
    static constexpr int32_t WOOL_ID = 35;
    static constexpr int32_t STAINED_CLAY_ID = 159;
    static constexpr int32_t HARDENED_CLAY_ID = 172;  // plain, no color variants

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

    // ─── Color inversion ───
    // Java: ~n & 0xF
    // Converts between block metadata and dye damage
    // Block white(0) ↔ Dye bone meal(15)
    // Block black(15) ↔ Dye ink sac(0)
    inline int32_t invertColor(int32_t meta) {
        return ~meta & 0xF;
    }

    // ─── Dye damage values (inverse of block meta) ───
    static constexpr int32_t DYE_INK_SAC = 0;       // → block black(15)
    static constexpr int32_t DYE_RED = 1;            // → block red(14)
    static constexpr int32_t DYE_GREEN = 2;          // → block green(13)
    static constexpr int32_t DYE_COCOA = 3;          // → block brown(12)
    static constexpr int32_t DYE_LAPIS = 4;          // → block blue(11)
    static constexpr int32_t DYE_PURPLE = 5;         // → block purple(10)
    static constexpr int32_t DYE_CYAN = 6;           // → block cyan(9)
    static constexpr int32_t DYE_LIGHT_GRAY = 7;     // → block silver(8)
    static constexpr int32_t DYE_GRAY = 8;           // → block gray(7)
    static constexpr int32_t DYE_PINK = 9;           // → block pink(6)
    static constexpr int32_t DYE_LIME = 10;          // → block lime(5)
    static constexpr int32_t DYE_YELLOW = 11;        // → block yellow(4)
    static constexpr int32_t DYE_LIGHT_BLUE = 12;    // → block light_blue(3)
    static constexpr int32_t DYE_MAGENTA = 13;       // → block magenta(2)
    static constexpr int32_t DYE_ORANGE = 14;        // → block orange(1)
    static constexpr int32_t DYE_BONE_MEAL = 15;     // → block white(0)
}

} // namespace mccpp
