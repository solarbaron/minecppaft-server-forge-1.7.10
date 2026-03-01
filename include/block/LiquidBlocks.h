/**
 * LiquidBlocks.h — Fluid mechanics: water and lava.
 *
 * Java references:
 *   - net.minecraft.block.BlockLiquid (246 lines) — abstract base
 *   - net.minecraft.block.BlockStaticLiquid (69 lines)
 *   - net.minecraft.block.BlockDynamicLiquid (241 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BLOCK LIQUID (Abstract Base)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: water or lava, setTickRandomly(true)
 * Full block bounds (0-1 all axes)
 * No collision (null), not opaque, render type 4
 * Drops: nothing
 *
 * Height:
 *   getLiquidHeightPercent(meta): (meta >= 8 ? 0 : meta) + 1) / 9.0
 *   meta 0 = source (fullest), meta 7 = thinnest
 *   meta >= 8 = falling (same height as source)
 *
 * Flow decay (getEffectiveFlowDecay):
 *   Returns meta if same material, -1 if different
 *   Falling (meta >= 8) treated as 0
 *
 * Flow vector (getFlowVector):
 *   4-neighbor gradient calculation
 *   Stair cascade: if neighbor blocks movement, check below
 *   Falling water: downslope bias -6.0 Y if adjacent solid
 *   Normalized to unit vector
 *
 * Entity velocity modification:
 *   Adds flow vector to entity motion
 *
 * Tick rates:
 *   Water: 5 ticks
 *   Lava overworld: 30 ticks
 *   Lava nether (hasNoSky): 10 ticks
 *
 * Lava-water interaction (func_149805_n):
 *   Checks 5 adjacent faces for water
 *   Lava source (meta 0) + water → obsidian
 *   Lava flowing (meta 1-4) + water → cobblestone
 *   Sound: "random.fizz" 0.5 vol, pitch 2.6±0.8
 *   Particles: 8× "largesmoke"
 *
 * Passability: water=passable, lava=not
 * Solidity: same material=false, face 1=true, ice=false
 *
 * Block IDs:
 *   flowing_water(8), water(9)
 *   flowing_lava(10), lava(11)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * STATIC LIQUID (BlockStaticLiquid)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockLiquid
 * Tick randomly: only for lava (fire spreading), not water
 *
 * On neighbor change: converts to dynamic form (ID - 1)
 *   Preserves metadata, schedules tick
 *
 * Lava fire spreading (updateTick):
 *   Random walk: rand(3) attempts, each ±1 XZ and +1 Y
 *   Checks air + adjacent flammable material (canBurn)
 *   Sets fire block
 *   If attempts == 0: extra pass with 3 iterations at Y+1
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DYNAMIC LIQUID (BlockDynamicLiquid)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockLiquid
 * requiresUpdates = true
 *
 * Flow algorithm (updateTick):
 *   1. Calculate new decay from 4 neighbors
 *      Water decay: +1, Lava decay: +2 (overworld), +1 (nether)
 *   2. Source above feeds current (meta >= 8 = falling from above)
 *   3. Water infinite source: adjacent sources >= 2 + solid below
 *   4. Lava: 4× slower decay chance (rand(4) != 0 → tickRate × 4)
 *   5. Unchanged + stable → convert to static (ID + 1)
 *   6. Changed → update meta, reschedule, notify neighbors
 *
 * Vertical flow:
 *   Can flow down through displaceable blocks
 *   Lava flowing into water → stone
 *   Falling preserves meta: already falling=same, else +8
 *
 * Horizontal flow (func_149808_o):
 *   BFS shortest path to hole, max depth 4
 *   4 directions, skips reverse direction
 *   Selection: all directions with minimum distance
 *
 * Flow blocking (func_149807_p):
 *   Blocks: door, sign, ladder, reeds, portal
 *   Also: any material that blocksMovement()
 *
 * Displacement (func_149809_q):
 *   Cannot displace same material, lava, or flow-blocking
 *   On displace: lava → fizz, water → drop block as item
 *
 * Thread safety: Liquid updates on server tick thread.
 * JNI readiness: Fluid events for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Liquid Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LiquidConstants {
    // ─── Block IDs ───
    static constexpr int32_t FLOWING_WATER_ID = 8;
    static constexpr int32_t STILL_WATER_ID = 9;
    static constexpr int32_t FLOWING_LAVA_ID = 10;
    static constexpr int32_t STILL_LAVA_ID = 11;

    // ─── Tick rates ───
    static constexpr int32_t WATER_TICK = 5;
    static constexpr int32_t LAVA_TICK_NETHER = 10;
    static constexpr int32_t LAVA_TICK_OVERWORLD = 30;

    // ─── Metadata ───
    static constexpr int32_t MAX_DECAY = 7;            // meta 0=source, 7=thinnest
    static constexpr int32_t FALLING_FLAG = 8;         // meta >= 8 = falling
    static constexpr int32_t DECAY_MASK = 7;

    // ─── Height ───
    // getLiquidHeightPercent(meta): ((meta >= 8 ? 0 : meta) + 1) / 9.0
    inline float heightPercent(int32_t meta) {
        int32_t effective = (meta >= FALLING_FLAG) ? 0 : meta;
        return static_cast<float>(effective + 1) / 9.0f;
    }

    // ─── Flow decay ───
    static constexpr int32_t WATER_DECAY = 1;
    static constexpr int32_t LAVA_DECAY_OVERWORLD = 2;
    static constexpr int32_t LAVA_DECAY_NETHER = 1;

    // ─── Infinite source ───
    static constexpr int32_t INFINITE_SOURCE_COUNT = 2; // adjacent sources needed
    // Also requires solid below or source below

    // ─── Lava slow decay ───
    static constexpr int32_t LAVA_SLOW_CHANCE = 4;     // rand(4) != 0
    static constexpr int32_t LAVA_SLOW_MULTIPLIER = 4;

    // ─── Flow BFS ───
    static constexpr int32_t MAX_FLOW_DEPTH = 4;
    static constexpr int32_t NO_PATH = 1000;

    // ─── Interaction ───
    static constexpr int32_t OBSIDIAN_ID = 49;         // lava source + water
    static constexpr int32_t COBBLESTONE_ID = 4;       // lava flowing + water (meta 1-4)
    static constexpr int32_t STONE_ID = 1;             // lava flows into water below

    // ─── Flow-blocking blocks ───
    static constexpr int32_t WOODEN_DOOR_ID = 64;
    static constexpr int32_t IRON_DOOR_ID = 71;
    static constexpr int32_t SIGN_ID = 63;
    static constexpr int32_t LADDER_ID = 65;
    static constexpr int32_t REEDS_ID = 83;
    // + portal material + blocksMovement()

    // ─── Sound ───
    static constexpr float FIZZ_VOLUME = 0.5f;
    static constexpr float FIZZ_BASE_PITCH = 2.6f;
    static constexpr float FIZZ_PITCH_RANGE = 0.8f;
    static constexpr int32_t SMOKE_PARTICLES = 8;

    // ─── Flow vector ───
    static constexpr double FALLING_Y_BIAS = -6.0;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 4;
}

} // namespace mccpp
