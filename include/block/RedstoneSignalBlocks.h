/**
 * RedstoneSignalBlocks.h — Redstone wire and repeater/diode.
 *
 * Java references:
 *   - net.minecraft.block.BlockRedstoneWire (323 lines)
 *   - net.minecraft.block.BlockRedstoneDiode (278 lines)
 *   - net.minecraft.block.BlockRedstoneRepeater (76 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * REDSTONE WIRE (BlockRedstoneWire)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, no collision, render type 5
 * Height: 1/16 (0.0625)
 * Not opaque, not normal
 * Drops: redstone item (331)
 *
 * Power level: metadata 0-15
 *   0 = unpowered, 15 = max
 *
 * Placement: solid top surface below OR glowstone
 *
 * canProvidePower flag:
 *   Toggled to false during own propagation to prevent feedback loops
 *   Then restored to true
 *
 * Propagation algorithm (func_150175_a):
 *   1. Get max wire neighbor power (func_150178_a on 4 cardinal)
 *   2. Get strongest indirect power (external sources)
 *   3. Disable canProvidePower, query getStrongestIndirectPower
 *   4. Re-enable canProvidePower
 *   5. New power = max(wire_neighbor, indirect_power)
 *   6. If wire_neighbor > indirect: new = wire_neighbor - 1
 *      Else if indirect > 0: new = indirect
 *      Else: 0
 *   7. If changed: update metadata + add 7 positions to notify set
 *
 * Stair connections (4 cardinal):
 *   Up: if neighbor isNormalCube AND self above is NOT normal
 *     → check wire at neighbor+1 Y
 *   Down: if neighbor is NOT normal → check wire at neighbor-1 Y
 *
 * Notification: HashSet dedup, batch notify after propagation
 *
 * Weak power (isProvidingWeakPower):
 *   Face 1 (below): always provides meta value
 *   Other faces: only if wire connects in that direction
 *   Connection-aware filtering:
 *     Compute 4 cardinal connections (same level, stair up/down)
 *     If no connections at all AND face 2-5 → provide power
 *     Specific face → provide only if that direction connected
 *       AND perpendicular directions NOT connected
 *
 * Strong power = weak power (when canProvidePower)
 *
 * isPowerProviderOrWire(block, direction):
 *   Wire → always true
 *   Repeater → true if facing matches direction or opposite
 *   Other → canProvidePower AND direction != -1
 *
 * func_150176_g (connection check):
 *   isPowerProviderOrWire → true
 *   OR powered_repeater facing matches → true
 *
 * Block ID: redstone_wire (55)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * REDSTONE DIODE BASE (BlockRedstoneDiode)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Abstract base for repeaters and comparators.
 * Material: circuits, extends BlockDirectional
 * Height: 2/16 (0.125), not opaque, not normal
 * canProvidePower = true
 *
 * isRepeaterPowered field: powered vs unpowered variant
 *
 * Placement: yaw → direction = (floor(yaw*4/360+0.5) & 3 + 2) % 4
 *   Requires solid top surface below
 *
 * Input strength (getInputStrength):
 *   From behind: Direction offsets based on facing
 *   Takes max(indirectPowerLevelTo, redstone_wire metadata)
 *
 * Output: directional, 15 strength (func_149904_f)
 *   Direction → face mapping:
 *     0→face 3, 1→face 4, 2→face 2, 3→face 5
 *
 * Update logic (onNeighborBlockChange → func_149897_b):
 *   If state changed AND not already scheduled:
 *     Schedule with priority:
 *       -3 if locked by adjacent repeater (func_149912_i)
 *       -2 if powered
 *       -1 default
 *
 * updateTick:
 *   If powered AND no input → switch to unpowered block
 *   If unpowered AND input → switch to powered, schedule if no input
 *
 * Side input (func_149902_h):
 *   Direction 0,2: max of x-1/x+1 side inputs
 *   Direction 1,3: max of z+1/z-1 side inputs
 *
 * isRedstoneRepeaterBlockID: repeater or comparator variants
 *
 * ═══════════════════════════════════════════════════════════════════════
 * REPEATER (BlockRedstoneRepeater)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockRedstoneDiode. Render type: 15.
 *
 * Metadata:
 *   bits 0-1: facing direction (0-3)
 *   bits 2-3: delay setting (0-3 → 1-4 ticks → 2-8 game ticks)
 *
 * Delay: repeaterState[delay] * 2 game ticks
 *   [1, 2, 3, 4] → [2, 4, 6, 8] game ticks
 *
 * Torch offsets: {-0.0625, 0.0625, 0.1875, 0.3125} per delay
 *
 * Interaction: right-click cycles delay (bits 2-3)
 *   delay = ((meta >> 2) + 1) << 2 & 0xC
 *
 * func_149910_g (locked check):
 *   Returns true if side input > 0 (locked by adjacent repeater)
 *
 * Drops: repeater item (356)
 *
 * Block IDs: unpowered_repeater(93), powered_repeater(94)
 *
 * Thread safety: Redstone updates on server thread with tick priority.
 * JNI readiness: Power provider interface for Forge redstone events.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Redstone Wire Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RedstoneWireConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 55;

    // ─── Power ───
    static constexpr int32_t MAX_POWER = 15;
    static constexpr int32_t MIN_POWER = 0;
    // Meta directly stores power level 0-15

    // ─── Height ───
    static constexpr float HEIGHT = 0.0625f;   // 1/16

    // ─── Placement ───
    static constexpr int32_t GLOWSTONE_ID = 89;
    // Requires solid top surface below OR glowstone

    // ─── Drops ───
    static constexpr int32_t REDSTONE_ITEM_ID = 331;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 5;

    // ─── Direction utility ───
    // Direction.offsetX[4] = {0, -1, 0, 1}
    // Direction.offsetZ[4] = {-1, 0, 1, 0}
    // Direction.rotateOpposite[4] = {2, 3, 0, 1}
    static constexpr int32_t DIR_OFFSET_X[4] = {0, -1, 0, 1};
    static constexpr int32_t DIR_OFFSET_Z[4] = {-1, 0, 1, 0};
    static constexpr int32_t DIR_OPPOSITE[4] = {2, 3, 0, 1};
}

// ═══════════════════════════════════════════════════════════════════════════
// Redstone Diode (Base) Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RedstoneDiodeConstants {
    // ─── Height ───
    static constexpr float HEIGHT = 0.125f;    // 2/16

    // ─── Direction mapping ───
    // Placement: (floor(yaw*4/360+0.5) & 3 + 2) % 4
    // Output face: direction→face: 0→3, 1→4, 2→2, 3→5
    static constexpr int32_t OUTPUT_FACE[4] = {3, 4, 2, 5};

    // Direction.directionToFacing[4] = {3, 4, 2, 5}
    // (same as output face mapping)

    // ─── Input ───
    // getInputStrength: from Direction.offsetX/Z[facing] behind
    // Max of indirectPowerLevelTo and redstone_wire metadata

    // ─── Priority ───
    static constexpr int32_t PRIORITY_DEFAULT = -1;
    static constexpr int32_t PRIORITY_POWERED = -2;
    static constexpr int32_t PRIORITY_LOCKED = -3;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 36;
}

// ═══════════════════════════════════════════════════════════════════════════
// Repeater Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RepeaterConstants {
    // ─── Block IDs ───
    static constexpr int32_t UNPOWERED_ID = 93;
    static constexpr int32_t POWERED_ID = 94;

    // ─── Item ID ───
    static constexpr int32_t REPEATER_ITEM_ID = 356;

    // ─── Metadata ───
    static constexpr int32_t FACING_MASK = 3;       // bits 0-1
    static constexpr int32_t DELAY_MASK = 0xC;       // bits 2-3
    static constexpr int32_t DELAY_SHIFT = 2;

    // ─── Delay ───
    // repeaterState[delay] = {1, 2, 3, 4} → ×2 = {2, 4, 6, 8} game ticks
    static constexpr int32_t DELAY_TICKS[4] = {1, 2, 3, 4};
    inline int32_t getDelayTicks(int32_t meta) {
        return DELAY_TICKS[(meta & DELAY_MASK) >> DELAY_SHIFT] * 2;
    }

    // ─── Torch offsets (visual) ───
    static constexpr double TORCH_OFFSETS[4] = {-0.0625, 0.0625, 0.1875, 0.3125};

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 15;
}

} // namespace mccpp
