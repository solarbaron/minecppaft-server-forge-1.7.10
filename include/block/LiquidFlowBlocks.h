/**
 * LiquidFlowBlocks.h — Liquid base, static liquid, dynamic (flowing) liquid.
 *
 * Java references:
 *   - net.minecraft.block.BlockLiquid (246 lines)
 *   - net.minecraft.block.BlockStaticLiquid (69 lines)
 *   - net.minecraft.block.BlockDynamicLiquid (241 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BLOCK LIQUID — Abstract Base
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: water or lava. Render type 4. Tick randomly: true.
 * Not opaque, not normal, no collision AABB. Drops: none.
 *
 * Height: getLiquidHeightPercent(meta): meta>=8 → meta=0; (meta+1)/9.0
 * Effective flow decay: same material → meta (>=8 treated as 0), else -1
 * Passable: water=true, lava=false
 *
 * isBlockSolid: same material→false, side 1(top)→true, ice→false
 *
 * Flow vector: 4 horizontal neighbor decay deltas, below fallback -8 offset
 *   Meta>=8 downstream: solid check → normalize + (0, -6, 0)
 *
 * Tick rates: water=5, lava nether(hasNoSky)=10, lava overworld=30
 *
 * Lava/water interaction: 5 neighbor water check
 *   Source (meta 0) → obsidian | Flow (meta 1-4) → cobblestone
 *   "random.fizz" vol=0.5, pitch=2.6±0.8, 8 largesmoke particles
 *
 * IDs: flowing_water(8), water(9), flowing_lava(10), lava(11)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * STATIC LIQUID (BlockStaticLiquid)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Tick randomly: water=false, lava=true
 * onNeighborBlockChange → setNotStationary: setBlock(ID-1, meta)
 *
 * Lava fire spread: random walk up to 3 steps (X±1, Y+1, Z±1)
 *   Air + flammable neighbor → fire. Fallback: 3 attempts at Y+1.
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DYNAMIC LIQUID (BlockDynamicLiquid)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Full flow simulation:
 *
 * Decay step: water=1, overworld lava=2
 * 4-neighbor smallest decay → newMeta = smallest + decay
 *   Source above → meta = above>=8 ? above : above+8
 *   Water infinite source: >=2 adjacent sources + solid/source below
 *   Lava random delay: growing → tickRate × 4
 *   Same meta → static (ID+1) | >= 8 or no source → die (air)
 *
 * Down flow: lava+water below→stone+fizz, meta>=8→same, else meta+8
 *
 * Horizontal spread: 4-direction BFS, max depth 4
 *   Blocked by: doors, signs, ladders, reeds, portal, blocksMovement
 *   Finds shortest path to hole → boolean[4] spread directions
 *   newMeta = old+decay (or 1 if downstream)
 *
 * Displace: not same/lava/blocked → drop+set (water) or fizz+set (lava)
 *
 * Thread safety: Flow updates on server tick thread.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Liquid Base Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LiquidConst {
    // IDs
    static constexpr int32_t FLOWING_WATER = 8;
    static constexpr int32_t STILL_WATER = 9;
    static constexpr int32_t FLOWING_LAVA = 10;
    static constexpr int32_t STILL_LAVA = 11;

    // Render
    static constexpr int32_t RENDER_TYPE = 4;

    // Meta
    static constexpr int32_t LEVEL_MASK = 7;             // bits 0-2
    static constexpr int32_t FALLING_FLAG = 8;           // bit 3
    static constexpr int32_t MAX_LEVEL = 7;

    // Height: (level+1)/9.0
    static constexpr float HEIGHT_DIV = 9.0f;

    // Tick rates
    static constexpr int32_t WATER_TICK = 5;
    static constexpr int32_t LAVA_NETHER_TICK = 10;
    static constexpr int32_t LAVA_OVERWORLD_TICK = 30;

    // Flow vector downstream push
    static constexpr double DOWNSTREAM_Y = -6.0;

    // Fizz
    static constexpr float FIZZ_VOL = 0.5f;
    static constexpr float FIZZ_PITCH = 2.6f;
    static constexpr float FIZZ_RANGE = 0.8f;
    static constexpr int32_t FIZZ_PARTICLES = 8;
}

// ═══════════════════════════════════════════════════════════════════════════
// Dynamic Liquid Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DynLiquidConst {
    // Decay
    static constexpr int32_t WATER_DECAY = 1;
    static constexpr int32_t LAVA_OW_DECAY = 2;

    // Infinite source
    static constexpr int32_t MIN_SOURCES = 2;

    // BFS
    static constexpr int32_t BFS_MAX_DEPTH = 4;
    static constexpr int32_t NO_PATH = 1000;

    // Lava delay
    static constexpr int32_t LAVA_DELAY_MULT = 4;
}

} // namespace mccpp
