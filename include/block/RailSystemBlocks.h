/**
 * RailSystemBlocks.h — Rail base, normal rail, powered rail, detector rail.
 *
 * Java references:
 *   - net.minecraft.block.BlockRailBase (163 lines)
 *   - net.minecraft.block.BlockRail (25 lines)
 *   - net.minecraft.block.BlockRailPowered (131 lines)
 *   - net.minecraft.block.BlockRailDetector (127 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * RAIL BASE (BlockRailBase) — Abstract
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, render type 9
 * Not opaque, not normal, no collision AABB
 * Mobility: 0
 *
 * isPowered: false=normal rail, true=powered/detector/activator
 *
 * Height:
 *   Meta 0-1 (flat): 2/16 (0.125)
 *   Meta 2-5 (slope): 10/16 (0.625)
 *
 * Rail block set: {rail, golden_rail, detector_rail, activator_rail}
 *
 * canPlaceBlockAt: solid top surface below
 *
 * onBlockAdded: refreshTrackShape + if powered → neighborChange
 *
 * onNeighborBlockChange:
 *   No solid below → drop + air
 *   Slope meta 2: needs solid at X+1
 *   Slope meta 3: needs solid at X-1
 *   Slope meta 4: needs solid at Z-1
 *   Slope meta 5: needs solid at Z+1
 *   If valid → onRedstoneSignal hook
 *
 * breakBlock: slopes → notify above, powered → notify self + below
 *
 * Track shapes (meta, powered variant uses bits 0-2):
 *   0=NS flat, 1=EW flat
 *   2=ascending east, 3=ascending west
 *   4=ascending north, 5=ascending south
 *   (Normal rail only) 6=SE curve, 7=SW curve, 8=NW curve, 9=NE curve
 *
 * Block IDs: rail(66), golden_rail(27), detector_rail(28), activator_rail(157)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * NORMAL RAIL (BlockRail)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * isPowered=false (allows curves)
 * onRedstoneSignal: if power provider + 3 adjacent rails → refresh shape
 * Meta 0-9 (10 shapes including 4 curves)
 *
 * Block ID: rail(66)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * POWERED RAIL (BlockRailPowered)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * isPowered=true (no curves, meta bits 0-2 = shape, bit 3 = powered)
 *
 * Power propagation (func_150058_a):
 *   Chain length: max 8 rails in each direction
 *   Slope-aware traversal:
 *     Meta 0(NS): walk Z±
 *     Meta 1(EW): walk X±
 *     Meta 2(asc E): forward→X-, backward→(X+, Y+)
 *     Meta 3(asc W): forward→(X-, Y+), backward→X+
 *     Meta 4(asc N): forward→Z+, backward→(Z-, Y+)
 *     Meta 5(asc S): forward→(Z+, Y+), backward→Z-
 *   Continuation check (func_150057_a):
 *     Must be same block type
 *     Direction must be compatible (no perpendicular)
 *     If powered bit set: check indirect power OR recurse
 *   Also checks: isBlockIndirectlyGettingPowered on each rail
 *
 * onRedstoneSignal:
 *   powered = indirect OR chain in either direction
 *   Toggle bit 3, notify below, slopes notify above
 *
 * Block ID: golden_rail(27), activator_rail(157)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DETECTOR RAIL (BlockRailDetector)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * isPowered=true, tick rate 20, tick randomly true
 * canProvidePower: true
 *
 * Entity detection (func_150054_a):
 *   AABB: block ± 0.125 inset XZ, full Y height
 *   Detects EntityMinecart
 *   Edge-triggered: sets bit 3 on cart enter, clears on leave
 *   While active: schedules re-check at tick rate 20
 *
 * Power output:
 *   Weak: 15 when active (bit 3), 0 when inactive
 *   Strong: 15 only on side 1 (up), 0 otherwise
 *
 * Comparator override:
 *   EntityMinecartCommandBlock → successCount
 *   IInventory minecart → calcRedstoneFromInventory
 *
 * Block ID: detector_rail(28)
 *
 * Thread safety: Rail state changes on server thread.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Rail Base Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RailBaseConst {
    // ─── Block IDs ───
    static constexpr int32_t RAIL_ID = 66;
    static constexpr int32_t GOLDEN_RAIL_ID = 27;
    static constexpr int32_t DETECTOR_RAIL_ID = 28;
    static constexpr int32_t ACTIVATOR_RAIL_ID = 157;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 9;

    // ─── Height ───
    static constexpr float FLAT_HEIGHT = 0.125f;         // 2/16
    static constexpr float SLOPE_HEIGHT = 0.625f;        // 10/16

    // ─── Mobility ───
    static constexpr int32_t MOBILITY = 0;

    // ─── Track shapes (meta) ───
    static constexpr int32_t NS_FLAT = 0;
    static constexpr int32_t EW_FLAT = 1;
    static constexpr int32_t ASC_EAST = 2;
    static constexpr int32_t ASC_WEST = 3;
    static constexpr int32_t ASC_NORTH = 4;
    static constexpr int32_t ASC_SOUTH = 5;
    // Normal rail curves:
    static constexpr int32_t CURVE_SE = 6;
    static constexpr int32_t CURVE_SW = 7;
    static constexpr int32_t CURVE_NW = 8;
    static constexpr int32_t CURVE_NE = 9;
}

// ═══════════════════════════════════════════════════════════════════════════
// Powered Rail Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PoweredRailConst {
    // ─── Meta ───
    static constexpr int32_t SHAPE_MASK = 7;             // bits 0-2
    static constexpr int32_t POWERED_FLAG = 8;           // bit 3

    // ─── Chain propagation ───
    static constexpr int32_t MAX_CHAIN = 8;              // max rails in chain
}

// ═══════════════════════════════════════════════════════════════════════════
// Detector Rail Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DetectorRailConst {
    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 20;

    // ─── Detection AABB inset ───
    static constexpr float DETECTION_INSET = 0.125f;     // 2/16

    // ─── Power output ───
    static constexpr int32_t OUTPUT_POWER = 15;
    // Strong power only on side 1 (up)

    // ─── Meta ───
    static constexpr int32_t SHAPE_MASK = 7;
    static constexpr int32_t ACTIVE_FLAG = 8;            // bit 3
}

} // namespace mccpp
