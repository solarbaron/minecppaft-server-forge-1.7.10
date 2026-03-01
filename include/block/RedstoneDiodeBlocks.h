/**
 * RedstoneDiodeBlocks.h — Redstone diode base, repeater, and comparator.
 *
 * Java references:
 *   - net.minecraft.block.BlockRedstoneDiode (278 lines)
 *   - net.minecraft.block.BlockRedstoneRepeater (76 lines)
 *   - net.minecraft.block.BlockRedstoneComparator (191 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * REDSTONE DIODE BASE (BlockRedstoneDiode) — Abstract
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockDirectional, material: circuits
 * Height: 2/16 (0.125)
 * Not opaque, not normal
 * canProvidePower: true
 * canBlockStay: requires solid top surface below
 *
 * Placement direction: yaw → ((floor(yaw*4/360+0.5) & 3) + 2) % 4
 *   Maps: N→2, E→3, S→0, W→1
 *   Schedules tick if getting input on placement
 *
 * Power output (isProvidingWeakPower = isProvidingStrongPower):
 *   Only outputs in facing direction
 *   dir 0→side 3, 1→side 4, 2→side 2, 3→side 5
 *   Output value: func_149904_f (15 for repeater, TE signal for comparator)
 *
 * Input detection:
 *   getInputStrength: checks indirect power from input face
 *     Reads redstone wire meta directly if wire
 *     Takes max of indirect power and wire power
 *
 * Side input (func_149902_h):
 *   For N/S facing: checks ±X
 *   For E/W facing: checks ±Z
 *   Reads from redstone wire (meta) or repeaters (power)
 *
 * Neighbor change logic:
 *   If can't stay: drop, notify all 6 neighbors
 *   Otherwise: schedule update if state mismatch
 *   Priority: -3 if facing diode behind, -2 if powered, -1 otherwise
 *
 * func_149911_e: notifies output block of neighbor change
 *   dir 1→+X, 3→-X, 2→+Z, 0→-Z
 *
 * ═══════════════════════════════════════════════════════════════════════
 * REPEATER (BlockRedstoneRepeater)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Render type: 15
 * isRepeaterPowered: true (powered_repeater) / false (unpowered_repeater)
 *
 * 4 delay settings in meta bits 2-3:
 *   repeaterState = [1, 2, 3, 4]
 *   delay ticks = repeaterState[meta>>2] * 2 = 2, 4, 6, 8
 *   Right-click cycles: (delay+1)%4
 *
 * Torch offset for rendering: [-0.0625, 0.0625, 0.1875, 0.3125]
 *
 * Output: fixed 15 (func_149904_f defaults)
 * Locked: func_149910_g checks side repeater input > 0
 * Side filter: only repeaters (func_149908_a checks isRedstoneRepeaterBlockID)
 *
 * Block IDs: unpowered_repeater(93), powered_repeater(94)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * COMPARATOR (BlockRedstoneComparator)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Render type: 37
 * isBlockContainer: true
 * TileEntity: TileEntityComparator (stores outputSignal)
 * isRepeaterPowered: true (powered) / false (unpowered)
 *
 * Delay: always 2 ticks
 *
 * Mode (bit 2):
 *   0 = compare mode
 *   4 = subtract mode
 *
 * Bit 3: powered flag (from isGettingInput)
 *
 * Right-click: toggles mode (bit 2)
 *   Sound: "random.click" pitch 0.55(subtract)/0.5(compare)
 *
 * Input strength override (getInputStrength):
 *   Checks block directly behind for comparatorInputOverride
 *   If not, and behind is normalCube, checks 2 blocks behind
 *   (through-block detection for containers)
 *
 * Output logic:
 *   Compare mode: output = input (if input >= side or side == 0)
 *   Subtract mode: output = max(input - side, 0)
 *
 * isGettingInput logic:
 *   input >= 15 → true
 *   input == 0 → false
 *   side == 0 → true (any input passes)
 *   input >= side → true
 *
 * func_149905_c: powered if isRepeaterPowered OR bit 3 set
 * Output signal from TE (not fixed 15)
 *
 * Schedule priority: -1 if facing change, 0 otherwise
 *
 * Block IDs: unpowered_comparator(149), powered_comparator(150)
 *
 * Thread safety: TE access on server tick thread.
 * JNI readiness: Diode block events for Forge.
 */
#pragma once

#include <cstdint>
#include <algorithm>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Redstone Diode Base Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DiodeBase {
    // ─── Bounds ───
    static constexpr float HEIGHT = 0.125f;              // 2/16

    // ─── Direction → output side mapping ───
    // dir 0→side 3(south), 1→side 4(west), 2→side 2(north), 3→side 5(east)
    static constexpr int32_t DIR_TO_SIDE[4] = {3, 4, 2, 5};

    // ─── Placement direction ─── 
    // ((floor(yaw*4/360+0.5) & 3) + 2) % 4

    // ─── Schedule priorities ───
    static constexpr int32_t PRIORITY_NORMAL = -1;
    static constexpr int32_t PRIORITY_POWERED = -2;
    static constexpr int32_t PRIORITY_FACING = -3;
}

// ═══════════════════════════════════════════════════════════════════════════
// Repeater Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RepeaterConstants {
    // ─── Block IDs ───
    static constexpr int32_t UNPOWERED_ID = 93;
    static constexpr int32_t POWERED_ID = 94;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 15;

    // ─── Metadata ───
    static constexpr int32_t DIR_MASK = 3;               // bits 0-1
    static constexpr int32_t DELAY_MASK = 0x0C;          // bits 2-3
    static constexpr int32_t DELAY_SHIFT = 2;

    // ─── Delay lookup (multiplied by 2 for ticks) ───
    static constexpr int32_t DELAY_STATE[4] = {1, 2, 3, 4};
    // Actual ticks: 2, 4, 6, 8

    inline int32_t getDelayTicks(int32_t meta) {
        return DELAY_STATE[(meta & DELAY_MASK) >> DELAY_SHIFT] * 2;
    }

    // ─── Torch render offsets ───
    static constexpr double TORCH_OFFSET[4] = {-0.0625, 0.0625, 0.1875, 0.3125};

    // ─── Output ───
    static constexpr int32_t OUTPUT_POWER = 15;
}

// ═══════════════════════════════════════════════════════════════════════════
// Comparator Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace ComparatorConstants {
    // ─── Block IDs ───
    static constexpr int32_t UNPOWERED_ID = 149;
    static constexpr int32_t POWERED_ID = 150;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 37;

    // ─── Delay ───
    static constexpr int32_t DELAY_TICKS = 2;

    // ─── Metadata ───
    static constexpr int32_t DIR_MASK = 3;               // bits 0-1
    static constexpr int32_t SUBTRACT_FLAG = 4;          // bit 2
    static constexpr int32_t POWERED_FLAG = 8;           // bit 3

    inline bool isSubtractMode(int32_t meta) { return (meta & SUBTRACT_FLAG) != 0; }
    inline bool isPowered(int32_t meta) { return (meta & POWERED_FLAG) != 0; }

    // ─── Sound ───
    static constexpr float SOUND_VOLUME = 0.3f;
    static constexpr float PITCH_SUBTRACT = 0.55f;
    static constexpr float PITCH_COMPARE = 0.5f;

    // ─── Schedule priorities ───
    static constexpr int32_t PRIORITY_FACING_CHANGE = -1;
    static constexpr int32_t PRIORITY_DEFAULT = 0;
}

} // namespace mccpp
