/**
 * RedstoneTorchComparator.h — Redstone torch and comparator.
 *
 * Java references:
 *   - net.minecraft.block.BlockRedstoneTorch (180 lines)
 *   - net.minecraft.block.BlockRedstoneComparator (191 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * REDSTONE TORCH (BlockRedstoneTorch)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockTorch. Material: circuits.
 * Two variants: lit (redstone_torch) and unlit (unlit_redstone_torch)
 * field_150113_a = isLit (lit torch provides power)
 *
 * Tick rate: 2 game ticks
 * setTickRandomly(true), creative tab null
 *
 * Burnout protection:
 *   Static HashMap<World, List<Toggle>> tracking toggle events
 *   Toggle = {x, y, z, worldTime}
 *   func_150111_a: if >= 8 toggles at same position within 60 ticks → burnout
 *   Old toggles (age > 60) pruned each updateTick
 *
 * Metadata: same as BlockTorch (1-5 = wall direction)
 *   1: x- wall, 2: x+ wall, 3: z- wall, 4: z+ wall, 5: floor
 *
 * Power detection (func_150110_m):
 *   Checks getIndirectPowerOutput on attached surface:
 *     meta 5 → below (face 0)
 *     meta 3 → z- (face 2)
 *     meta 4 → z+ (face 3)
 *     meta 1 → x- (face 4)
 *     meta 2 → x+ (face 5)
 *
 * Weak power (isProvidingWeakPower):
 *   Returns 15 to all faces EXCEPT the attached face
 *   meta→excluded face: 5→1, 3→3, 4→2, 1→5, 2→4
 *
 * Strong power (isProvidingStrongPower):
 *   Only face 0 (below): returns weak power value
 *   All other faces: 0
 *
 * updateTick:
 *   Lit + powered → switch to unlit, check burnout (fizz + smoke)
 *   Unlit + not powered + no burnout → switch to lit
 *
 * onNeighborBlockChange:
 *   If surface valid: schedule tick if state needs change
 *   func_150108_b: inherited from BlockTorch, drops if no support
 *
 * Add/break: notify all 6 neighbors when lit
 *
 * canProvidePower = true
 * Drops: redstone_torch item always
 *
 * Block IDs: redstone_torch(76), unlit_redstone_torch(75)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * COMPARATOR (BlockRedstoneComparator)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockRedstoneDiode, implements ITileEntityProvider.
 * TileEntityComparator stores outputSignal (analog value 0-15).
 * isBlockContainer = true
 *
 * Metadata:
 *   bits 0-1: facing direction (inherited from BlockDirectional)
 *   bit 2: subtraction mode flag (func_149969_d)
 *   bit 3: powered flag (separate from isRepeaterPowered variant)
 *
 * Delay: always 2 game ticks (func_149901_b returns 2)
 *
 * Two modes:
 *   Comparison (default, bit 2 = 0):
 *     Output = input if input >= side input, else 0
 *   Subtraction (bit 2 = 1):
 *     Output = max(input - sideInput, 0)
 *
 * getInputStrength (overridden):
 *   Checks block behind for hasComparatorInputOverride → uses it
 *   If not and block is normalCube: checks TWO blocks behind
 *     (through-block reading for containers behind solid blocks)
 *
 * isGettingInput (overridden):
 *   input >= 15 → true
 *   input == 0 → false
 *   side == 0 → true (no side input, any signal passes)
 *   input >= side → true
 *
 * func_149905_c: powered if isRepeaterPowered OR bit 3 set
 *   (allows output even when block variant is unpowered)
 *
 * func_149904_f: returns TileEntityComparator.getOutputSignal()
 *   (analog output, not fixed 15 like repeater)
 *
 * Interaction (onBlockActivated):
 *   Toggle subtraction mode (bit 2)
 *   Sound: "random.click" 0.3f, 0.55 (subtraction) / 0.5 (comparison)
 *   Recalculate output immediately
 *
 * updateTick:
 *   If powered variant → switch to unpowered with bit 3 set
 *   Recalculate output
 *
 * func_149897_b (neighbor update, overridden):
 *   Schedule update if output changed OR powered state changed
 *   Priority: -1 if locked, 0 otherwise
 *
 * Block IDs: unpowered_comparator(149), powered_comparator(150)
 * Drops: comparator item (404)
 *
 * Thread safety: Redstone updates on server thread.
 * JNI readiness: TileEntityComparator for Forge comparator events.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Redstone Torch Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RedstoneTorchConstants {
    // ─── Block IDs ───
    static constexpr int32_t LIT_ID = 76;        // redstone_torch
    static constexpr int32_t UNLIT_ID = 75;       // unlit_redstone_torch

    // ─── Properties ───
    static constexpr int32_t TICK_RATE = 2;
    static constexpr int32_t POWER_LEVEL = 15;

    // ─── Metadata (same as BlockTorch) ───
    // 1: x- wall, 2: x+ wall, 3: z- wall, 4: z+ wall, 5: floor
    static constexpr int32_t META_WALL_X_NEG = 1;
    static constexpr int32_t META_WALL_X_POS = 2;
    static constexpr int32_t META_WALL_Z_NEG = 3;
    static constexpr int32_t META_WALL_Z_POS = 4;
    static constexpr int32_t META_FLOOR = 5;

    // ─── Power detection: meta → attached face ───
    // Used for getIndirectPowerOutput check:
    //   meta 5 → face 0 (below)
    //   meta 3 → face 2 (z-)
    //   meta 4 → face 3 (z+)
    //   meta 1 → face 4 (x-)
    //   meta 2 → face 5 (x+)
    inline int32_t getAttachedFace(int32_t meta) {
        switch(meta) {
            case 5: return 0;
            case 3: return 2;
            case 4: return 3;
            case 1: return 4;
            case 2: return 5;
            default: return -1;
        }
    }

    // ─── Weak power: meta → excluded output face ───
    // Provides 15 to all faces EXCEPT:
    //   meta 5 → face 1 (above)
    //   meta 3 → face 3 (z+)
    //   meta 4 → face 2 (z-)
    //   meta 1 → face 5 (x+)
    //   meta 2 → face 4 (x-)
    inline int32_t getExcludedFace(int32_t meta) {
        switch(meta) {
            case 5: return 1;
            case 3: return 3;
            case 4: return 2;
            case 1: return 5;
            case 2: return 4;
            default: return -1;
        }
    }

    // ─── Burnout protection ───
    static constexpr int32_t MAX_TOGGLES = 8;       // 8 toggles → burnout
    static constexpr int64_t BURNOUT_WINDOW = 60;    // ticks

    // ─── Burnout effects ───
    static constexpr float FIZZ_VOLUME = 0.5f;
    static constexpr float FIZZ_PITCH_BASE = 2.6f;
    static constexpr float FIZZ_PITCH_RANGE = 0.8f;
    static constexpr int32_t SMOKE_PARTICLES = 5;

    // ─── Power ───
    // Strong: only face 0 (below), returns weak
    // Weak: 15 to all except attached face
    static constexpr int32_t STRONG_FACE = 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Comparator Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace ComparatorConstants {
    // ─── Block IDs ───
    static constexpr int32_t UNPOWERED_ID = 149;   // unpowered_comparator
    static constexpr int32_t POWERED_ID = 150;      // powered_comparator

    // ─── Item ID ───
    static constexpr int32_t COMPARATOR_ITEM_ID = 404;

    // ─── Metadata ───
    static constexpr int32_t FACING_MASK = 3;          // bits 0-1
    static constexpr int32_t SUBTRACTION_FLAG = 4;     // bit 2
    static constexpr int32_t POWERED_FLAG = 8;         // bit 3
    static constexpr int32_t POWERED_CLEAR = ~8;       // 0xFFFFFFF7

    // ─── Delay ───
    static constexpr int32_t DELAY_TICKS = 2;          // always 2 game ticks

    // ─── Modes ───
    enum class Mode : int32_t {
        COMPARISON = 0,    // output = input if input >= side, else 0
        SUBTRACTION = 1    // output = max(input - side, 0)
    };

    inline bool isSubtractionMode(int32_t meta) {
        return (meta & SUBTRACTION_FLAG) != 0;
    }

    // ─── Through-block reading ───
    // Comparator checks hasComparatorInputOverride on block behind
    // If not: checks through solid normalCube to second block behind
    // Used for: chests, furnaces, hoppers, jukeboxes, brewing stands, etc.

    // ─── Sound ───
    static constexpr float CLICK_VOLUME = 0.3f;
    static constexpr float CLICK_PITCH_SUBTRACTION = 0.55f;
    static constexpr float CLICK_PITCH_COMPARISON = 0.5f;

    // ─── Priority ───
    static constexpr int32_t PRIORITY_LOCKED = -1;
    static constexpr int32_t PRIORITY_DEFAULT = 0;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 37;
}

} // namespace mccpp
