/**
 * FluidBlocks.h — Water and lava flow mechanics.
 *
 * Java references:
 *   - net.minecraft.block.BlockLiquid (246 lines)
 *   - net.minecraft.block.BlockDynamicLiquid (241 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LIQUID BASE (BlockLiquid)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Abstract base for all fluid blocks.
 * Material: water or lava, tick randomly
 *
 * Height formula:
 *   getLiquidHeightPercent(meta): if meta >= 8, meta = 0; return (meta+1)/9.0
 *   Meta 0 = source (1/9 = 11.1%), meta 7 = nearly empty (8/9)
 *
 * Effective flow decay (getEffectiveFlowDecay):
 *   If block material != this material → -1
 *   If meta >= 8, treat as 0 (falling)
 *
 * Flow vector (getFlowVector):
 *   For each cardinal direction: compute effective decay difference
 *   If neighbor not same material but non-blocking:
 *     Check one block below for liquid → use (decay - (selfDecay-8))
 *   Falling water (meta >= 8): check 4+4 adjacent for solid → push -6Y
 *   Normalize result
 *
 * modifyEntityVelocity: adds flow vector to entity velocity
 *
 * Tick rates:
 *   Water: 5 ticks
 *   Lava (overworld): 30 ticks
 *   Lava (nether/hasNoSky): 10 ticks
 *
 * Lava-water interaction (func_149805_n):
 *   If lava block has water neighbor (4 horizontal + above):
 *     Meta 0 (source) → obsidian
 *     Meta 1-4 (flowing) → cobblestone
 *     Play fizz sound + smoke particles
 *
 * isPassable: water=true, lava=false
 * No collision, no drops, render type 4
 *
 * Block IDs: flowing_water(8), water(9), flowing_lava(10), lava(11)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DYNAMIC LIQUID (BlockDynamicLiquid) — Flowing water/lava
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockLiquid. This is the "flowing" variant (IDs 8, 10).
 *
 * Still conversion (func_149811_n):
 *   When flow stabilizes: block ID → blockId + 1
 *   flowing_water(8) → water(9), flowing_lava(10) → lava(11)
 *
 * updateTick flow algorithm:
 *
 * 1. DECAY CALCULATION (when meta > 0):
 *    Find min neighbor decay from 4 cardinal directions
 *    New decay = min_neighbor + flowDecay (1 for water, 2 for overworld lava)
 *    If new decay >= 8 OR no neighbor → new decay = -1 (disappear)
 *    If liquid above: new decay = above_meta >= 8 ? above_meta : above_meta+8
 *
 * 2. SOURCE CREATION (water only):
 *    If >= 2 adjacent cardinal sources (field_149815_a >= 2):
 *      If solid below or same liquid source below → become source (meta=0)
 *
 * 3. LAVA SLOWDOWN:
 *    If lava, meta < 8, new decay > old decay, rand(4) != 0 → tick × 4
 *
 * 4. APPLY:
 *    Same as current → convert to still
 *    Changed → update metadata + schedule next tick + notify neighbors
 *    New = -1 → remove (setBlockToAir)
 *
 * 5. DOWNWARD FLOW:
 *    Check block below (func_149809_q: not same material, not lava, not blocking)
 *    Lava over water → stone + fizz
 *    Otherwise: place self below with meta (falling=meta, else meta+8)
 *
 * 6. HORIZONTAL FLOW (if not flowing down, or source/filling):
 *    BFS pathfinding (func_149808_o / func_149812_c):
 *      For each cardinal direction, recursively search up to depth 4
 *      Find shortest path to an edge (where block below is not blocking)
 *      Skip reverse direction to avoid backtracking
 *    All directions with shortest distance get flow
 *    New horizontal meta = decay + flowDecay (1 or 2)
 *    If meta >= 8 (falling): horizontal meta = 1
 *    If horizontal meta >= 8: stop (can't extend further)
 *
 * Block displacement (func_149813_h):
 *   Can displace if: not same material, not lava, not blocking
 *   Lava → fizz sound
 *   Water → drop block items
 *   Then place self with given meta
 *
 * Blocking check (func_149807_p):
 *   Doors, signs, ladders, reeds, portals → blocking
 *   material.blocksMovement → blocking
 *
 * Thread safety: Flow on server tick thread.
 * JNI readiness: Flow direction arrays for Forge fluid events.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Base Liquid Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LiquidConstants {
    // ─── Block IDs ───
    static constexpr int32_t FLOWING_WATER_ID = 8;
    static constexpr int32_t STILL_WATER_ID = 9;
    static constexpr int32_t FLOWING_LAVA_ID = 10;
    static constexpr int32_t STILL_LAVA_ID = 11;

    // ─── Height formula ───
    // Height percent = (meta + 1) / 9.0, with meta >= 8 → treat as 0
    static constexpr float HEIGHT_DIVISOR = 9.0f;

    inline float getLiquidHeightPercent(int32_t meta) {
        if (meta >= 8) meta = 0;
        return static_cast<float>(meta + 1) / HEIGHT_DIVISOR;
    }

    // ─── Meta interpretation ───
    static constexpr int32_t SOURCE_META = 0;
    static constexpr int32_t FALLING_FLAG = 8;   // meta >= 8 = falling
    static constexpr int32_t MAX_DECAY = 7;

    // ─── Tick rates ───
    static constexpr int32_t WATER_TICK = 5;
    static constexpr int32_t LAVA_TICK_NETHER = 10;
    static constexpr int32_t LAVA_TICK_OVERWORLD = 30;

    // ─── Flow decay per step ───
    static constexpr int32_t WATER_FLOW_DECAY = 1;
    static constexpr int32_t LAVA_FLOW_DECAY_NETHER = 1;  // hasNoSky
    static constexpr int32_t LAVA_FLOW_DECAY_OVERWORLD = 2;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 4;

    // ─── Flow vector ───
    static constexpr double WATERFALL_PUSH_Y = -6.0;

    // ─── Lava-water interaction ───
    static constexpr int32_t OBSIDIAN_ID = 49;
    static constexpr int32_t COBBLESTONE_ID = 4;
    static constexpr int32_t STONE_ID = 1;
    // Meta 0 (source) + water → obsidian
    // Meta 1-4 (flowing) + water → cobblestone
    // Lava flowing over water below → stone

    // ─── Sound ───
    // "random.fizz", volume 0.5, pitch 2.6 ± rand*0.8
    static constexpr float FIZZ_VOLUME = 0.5f;
    static constexpr float FIZZ_PITCH_BASE = 2.6f;
    static constexpr float FIZZ_PITCH_RANGE = 0.8f;
    static constexpr int32_t FIZZ_PARTICLES = 8;  // largesmoke × 8
}

// ═══════════════════════════════════════════════════════════════════════════
// Dynamic Liquid Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DynamicLiquidConstants {
    // ─── BFS pathfinding ───
    static constexpr int32_t MAX_FLOW_SEARCH_DEPTH = 4;  // func_149812_c depth limit
    static constexpr int32_t NO_PATH = 1000;               // sentinel value

    // ─── Source creation ───
    // Water only: >= 2 adjacent source blocks → become source
    static constexpr int32_t SOURCE_MIN_NEIGHBORS = 2;

    // ─── Lava slowdown ───
    // Lava, meta < 8, new > old, rand(4) != 0 → tick × 4
    static constexpr int32_t LAVA_SLOWDOWN_CHANCE = 4;
    static constexpr int32_t LAVA_SLOWDOWN_MULTIPLIER = 4;

    // ─── Blocking blocks ───
    // Explicitly blocked: wooden_door(64), iron_door(71),
    //   standing_sign(63), ladder(65), reeds(83), portal material
    //   + material.blocksMovement()
    static constexpr int32_t WOODEN_DOOR_ID = 64;
    static constexpr int32_t IRON_DOOR_ID = 71;
    static constexpr int32_t STANDING_SIGN_ID = 63;
    static constexpr int32_t LADDER_ID = 65;
    static constexpr int32_t REEDS_ID = 83;

    // ─── Still conversion ───
    // Dynamic block ID + 1 = still block ID
    // flowing_water(8) → water(9)
    // flowing_lava(10) → lava(11)
}

} // namespace mccpp
