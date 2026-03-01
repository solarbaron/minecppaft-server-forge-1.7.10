/**
 * RailSystem.h — Rail track placement, powered rail propagation, and detector rail.
 *
 * Java references:
 *   - net.minecraft.block.BlockRailBase (163 lines)
 *   - net.minecraft.block.BlockRail (25 lines)
 *   - net.minecraft.block.BlockRailPowered (131 lines)
 *   - net.minecraft.block.BlockRailDetector (127 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * RAIL BASE (BlockRailBase)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits
 * No collision box (minecarts ride on top)
 * Not opaque, not normal block, render type 9
 * Mobility: 0 (pushable by pistons)
 * Requires solid top surface below to stay
 *
 * Rail block IDs (isRailBlock check):
 *   - rail (66), golden_rail (27), detector_rail (28), activator_rail (157)
 *
 * Bounds based on metadata:
 *   - Flat (meta 0-1, 6-9): 2/16 height (0.125)
 *   - Slope (meta 2-5): 10/16 height (0.625)
 *
 * Track shapes (metadata):
 *   0 = north-south (Z axis)
 *   1 = east-west (X axis)
 *   2 = ascending east  (+X, going up)
 *   3 = ascending west  (-X, going up)
 *   4 = ascending north (-Z, going up)
 *   5 = ascending south (+Z, going up)
 *   6 = south-east curve
 *   7 = south-west curve
 *   8 = north-west curve
 *   9 = north-east curve
 *   (curves only for regular rail, not powered/detector/activator)
 *
 * Slope break checks (onNeighborBlockChange):
 *   meta 2: needs solid at (x+1, y, z) — east supporting block
 *   meta 3: needs solid at (x-1, y, z) — west supporting block
 *   meta 4: needs solid at (x, y, z-1) — north supporting block
 *   meta 5: needs solid at (x, y, z+1) — south supporting block
 *   If support missing → break (drop + air)
 *
 * Track shape refresh uses inner Rail class for T-junction resolution
 *
 * breakBlock notifications:
 *   Slopes (2-5): notify blocks above (y+1)
 *   Powered rails: notify block below (y-1)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * REGULAR RAIL (BlockRail)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * isPowered = false
 * Supports all 10 shapes including curves (6-9)
 *
 * Redstone response:
 *   When a redstone-providing block changes AND 3 adjacent rails:
 *   → refresh track shape (enables T-junction switching)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * POWERED RAIL / GOLDEN RAIL (BlockRailPowered)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * isPowered = true
 * Supports shapes 0-5 only (no curves)
 * Metadata: bits 0-2 = shape, bit 3 = powered flag
 *
 * Power propagation (func_150058_a):
 *   - Chain propagation up to 8 blocks
 *   - Follows rail direction, including slopes (Y ±1)
 *   - Direction-specific: won't cross perpendicular rails
 *   - Each rail in chain must also be powered (via redstone or chain)
 *   - Checks both directions from source
 *
 * Slope scanning direction:
 *   Shape 0 (N-S): forward=+Z, backward=-Z
 *   Shape 1 (E-W): forward=-X, backward=+X
 *   Shape 2 (asc E): forward=-X (level), backward=+X and +Y (slope up)
 *   Shape 3 (asc W): forward=-X and +Y (slope up), backward=+X (level)
 *   Shape 4 (asc N): forward=+Z (level), backward=-Z and +Y (slope up)
 *   Shape 5 (asc S): forward=+Z and +Y (slope up), backward=-Z (level)
 *
 * func_150057_a: validates chain rail is:
 *   - Same block type (powered rail)
 *   - Compatible direction (no perpendicular)
 *   - Has powered bit set
 *   - Either redstone-powered OR chain continues
 *   Also checks Y-1 for downhill continuation
 *
 * On state change: notify blocks below, above if slope
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DETECTOR RAIL (BlockRailDetector)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * isPowered = true, tick randomly, tick rate: 20
 * Provides redstone power when minecart is on top
 *
 * Detection box: block inset by 1/8 (0.125) on XZ, full Y
 *   Java: (x+0.125, y, z+0.125) → (x+0.875, y+0.875, z+0.875)
 *
 * Metadata: bits 0-2 = shape, bit 3 = activated flag
 *
 * onEntityCollidedWithBlock:
 *   If not activated → check for minecarts → activate if found
 *
 * updateTick (every 20 ticks):
 *   If activated → recheck → deactivate if no minecarts
 *
 * Activation: set bit 3, notify neighbors + below, schedule tick
 * Deactivation: clear bit 3, notify neighbors + below
 *
 * Redstone output:
 *   Weak power: 15 on all sides when active
 *   Strong power: 15 only upward (side 1) when active
 *
 * Comparator output:
 *   - Command block minecart: successCount of command
 *   - Inventory minecart: calcRedstoneFromInventory (fullness %)
 *   - Others: 0
 *
 * Thread safety: Block updates on server thread.
 * JNI readiness: Simple constants and state management.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Rail Block IDs
// ═══════════════════════════════════════════════════════════════════════════

namespace RailBlockIds {
    static constexpr int32_t RAIL = 66;
    static constexpr int32_t GOLDEN_RAIL = 27;       // powered rail
    static constexpr int32_t DETECTOR_RAIL = 28;
    static constexpr int32_t ACTIVATOR_RAIL = 157;

    inline bool isRailBlock(int32_t blockId) {
        return blockId == RAIL || blockId == GOLDEN_RAIL
            || blockId == DETECTOR_RAIL || blockId == ACTIVATOR_RAIL;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Track Shape Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TrackShapes {
    // ─── Straight shapes ───
    static constexpr int32_t NORTH_SOUTH = 0;    // Z axis
    static constexpr int32_t EAST_WEST = 1;      // X axis

    // ─── Ascending shapes ───
    static constexpr int32_t ASCENDING_EAST = 2;   // +X up
    static constexpr int32_t ASCENDING_WEST = 3;   // -X up
    static constexpr int32_t ASCENDING_NORTH = 4;  // -Z up
    static constexpr int32_t ASCENDING_SOUTH = 5;  // +Z up

    // ─── Curve shapes (regular rail only) ───
    static constexpr int32_t CURVE_SE = 6;  // south-east
    static constexpr int32_t CURVE_SW = 7;  // south-west
    static constexpr int32_t CURVE_NW = 8;  // north-west
    static constexpr int32_t CURVE_NE = 9;  // north-east

    // ─── Bounds ───
    inline bool isSlope(int32_t shape) {
        return shape >= 2 && shape <= 5;
    }
    inline bool isCurve(int32_t shape) {
        return shape >= 6 && shape <= 9;
    }
    static constexpr int32_t MAX_SHAPE_POWERED = 5;  // no curves for powered
    static constexpr int32_t MAX_SHAPE_REGULAR = 9;   // includes curves
}

// ═══════════════════════════════════════════════════════════════════════════
// Rail Base Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RailBaseConstants {
    // ─── Bounds ───
    // Flat: 2/16 (0.125)
    static constexpr float FLAT_HEIGHT = 0.125f;
    // Slope: 10/16 (0.625)
    static constexpr float SLOPE_HEIGHT = 0.625f;

    // ─── Shape mask ───
    // For powered rails: bits 0-2 = shape (mask 7)
    static constexpr int32_t SHAPE_MASK = 7;
    // Bit 3 = powered/activated flag
    static constexpr int32_t POWERED_FLAG = 8;

    // ─── Slope support blocks ───
    // Each ascending shape requires support block at higher end:
    // Shape 2 (asc east): (x+1, y, z)
    // Shape 3 (asc west): (x-1, y, z)
    // Shape 4 (asc north): (x, y, z-1)
    // Shape 5 (asc south): (x, y, z+1)

    struct SlopeSupport {
        int32_t dx, dy, dz;
    };

    static constexpr SlopeSupport SLOPE_SUPPORTS[] = {
        {+1, 0,  0},  // shape 2: east
        {-1, 0,  0},  // shape 3: west
        { 0, 0, -1},  // shape 4: north
        { 0, 0, +1},  // shape 5: south
    };

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 9;

    // ─── Mobility ───
    static constexpr int32_t MOBILITY = 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Regular Rail Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RegularRailConstants {
    // T-junction redstone switching:
    // When a power-providing block changes AND 3 adjacent rails →
    // refresh track shape (allows T-junction to switch corners)
    static constexpr int32_t T_JUNCTION_RAIL_COUNT = 3;
}

// ═══════════════════════════════════════════════════════════════════════════
// Powered Rail Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PoweredRailConstants {
    // ─── Chain propagation ───
    // Max chain length: 8 blocks in each direction
    static constexpr int32_t MAX_CHAIN_LENGTH = 8;

    // ─── Perpendicular blocking ───
    // Shape 0 (N-S) won't chain with shapes 1, 2, 3 (E-W and ascending E/W)
    // Shape 1 (E-W) won't chain with shapes 0, 4, 5 (N-S and ascending N/S)

    // ─── Power state ───
    // Becomes powered when:
    // 1. Block is directly redstone-powered, OR
    // 2. Adjacent powered rail in chain is powered (up to 8)
    // Changes bit 3 in metadata
}

// ═══════════════════════════════════════════════════════════════════════════
// Detector Rail Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DetectorRailConstants {
    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 20;  // 1 second recheck

    // ─── Detection box ───
    // Inset 1/8 (0.125) on XZ, full Y minus 1/8 top
    static constexpr float DETECT_INSET = 0.125f;
    // Box: (x+0.125, y, z+0.125) → (x+0.875, y+0.875, z+0.875)

    // ─── Redstone output ───
    static constexpr int32_t WEAK_POWER = 15;    // all sides when active
    static constexpr int32_t STRONG_POWER = 15;   // upward only (side 1)
    static constexpr int32_t STRONG_SIDE = 1;     // upward direction

    // ─── Comparator output ───
    // Command block minecart: commandBlock.getSuccessCount()
    // Inventory minecart: Container.calcRedstoneFromInventory(cart)
    // Other: 0
}

} // namespace mccpp
