/**
 * RailTrackBlocks.h — Rail base, normal rail, powered rail, detector rail,
 *                     and track shape resolution.
 *
 * Java references:
 *   - net.minecraft.block.BlockRailBase (163 lines)
 *   - net.minecraft.block.BlockRail (25 lines)
 *   - net.minecraft.block.BlockRailPowered (131 lines)
 *   - net.minecraft.block.BlockRailDetector (127 lines)
 *   - net.minecraft.block.BlockRailBase$Rail (318 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * RAIL BASE (BlockRailBase) — Abstract
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits
 * Render type 9, not opaque, not normal
 * No collision AABB (entities walk through)
 * Requires solid top surface below
 * Mobility flag: 0 (pushable)
 *
 * Bounds:
 *   Flat (meta 0,1,6-9): 0-1 XZ, 0-0.125 Y (2/16)
 *   Slope (meta 2-5): 0-1 XZ, 0-0.625 Y (10/16)
 *
 * isRailBlock: rail(66), golden_rail(27), detector_rail(28), activator_rail(157)
 *
 * Slope support (onNeighborBlockChange):
 *   meta 2: ascending east → solid at X+1
 *   meta 3: ascending west → solid at X-1
 *   meta 4: ascending north → solid at Z-1
 *   meta 5: ascending south → solid at Z+1
 *   Missing → drop and break
 *
 * Break: slopes notify Y+1; powered rails notify Y-1
 *
 * ═══════════════════════════════════════════════════════════════════════
 * RAIL (BlockRail)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * isPowered: false, supports corners (6-9)
 * onRedstoneSignal: 3 adjacent rails → refresh shape
 *
 * Block ID: rail(66)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * POWERED RAIL (BlockRailPowered)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * isPowered: true, no corners, bit 3 = power state
 *
 * Recursive propagation (func_150058_a):
 *   Max depth: 8 rails
 *   Follows direction only (rejects perpendicular)
 *   Handles slopes (Y±1)
 *   Stops at unpowered rail or depth limit
 *
 * onRedstoneSignal: direct power OR propagation → set/clear bit 3
 *
 * Block IDs: golden_rail(27), activator_rail(157)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DETECTOR RAIL (BlockRailDetector)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * isPowered: true, tickRandomly, tick rate 20
 *
 * Detection: 2/16 inset AABB, EntityMinecart search
 *   Found + inactive → activate (bit 3), schedule tick
 *   Not found + active → deactivate
 *
 * Redstone: weak 15, strong 15 on side 1 (up) only
 *
 * Comparator:
 *   Command minecart → successCount
 *   Inventory minecart → calcRedstoneFromInventory
 *
 * Block ID: detector_rail(28)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TRACK SHAPE (BlockRailBase$Rail)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * 10 states: 0-1 flat, 2-5 slope, 6-9 corner
 *
 * Endpoint pairs per state:
 *   0: (x,y,z-1)+(x,y,z+1)     NS
 *   1: (x-1,y,z)+(x+1,y,z)     EW
 *   2: (x-1,y,z)+(x+1,y+1,z)   asc east
 *   3: (x-1,y+1,z)+(x+1,y,z)   asc west
 *   4: (x,y+1,z-1)+(x,y,z+1)   asc north
 *   5: (x,y,z-1)+(x,y+1,z+1)   asc south
 *   6: (x+1,y,z)+(x,y,z+1)     SE
 *   7: (x-1,y,z)+(x,y,z+1)     SW
 *   8: (x-1,y,z)+(x,y,z-1)     NW
 *   9: (x+1,y,z)+(x,y,z-1)     NE
 *
 * Neighbor scan: ±1 Y per cardinal. Corner priority via powered flag.
 * Refresh cascade: updates self then propagates to connected rails.
 *
 * Thread safety: Rail operations on server thread.
 */
#pragma once

#include <cstdint>
#include <array>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Rail Base Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RailBase {
    // ─── Block IDs ───
    static constexpr int32_t RAIL_ID = 66;
    static constexpr int32_t GOLDEN_RAIL_ID = 27;
    static constexpr int32_t DETECTOR_RAIL_ID = 28;
    static constexpr int32_t ACTIVATOR_RAIL_ID = 157;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 9;

    // ─── Bounds ───
    static constexpr float FLAT_Y = 0.125f;             // 2/16
    static constexpr float SLOPE_Y = 0.625f;            // 10/16

    // ─── Metadata ───
    static constexpr int32_t SHAPE_MASK = 7;
    static constexpr int32_t POWER_BIT = 8;

    // ─── Track shapes ───
    static constexpr int32_t NS = 0;
    static constexpr int32_t EW = 1;
    static constexpr int32_t ASC_EAST = 2;
    static constexpr int32_t ASC_WEST = 3;
    static constexpr int32_t ASC_NORTH = 4;
    static constexpr int32_t ASC_SOUTH = 5;
    static constexpr int32_t CORNER_SE = 6;
    static constexpr int32_t CORNER_SW = 7;
    static constexpr int32_t CORNER_NW = 8;
    static constexpr int32_t CORNER_NE = 9;

    inline bool isSlope(int32_t s) { return s >= 2 && s <= 5; }
    inline bool isCorner(int32_t s) { return s >= 6 && s <= 9; }

    inline bool isRailBlock(int32_t id) {
        return id == RAIL_ID || id == GOLDEN_RAIL_ID ||
               id == DETECTOR_RAIL_ID || id == ACTIVATOR_RAIL_ID;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Powered Rail Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PoweredRail {
    static constexpr int32_t MAX_DEPTH = 8;
}

// ═══════════════════════════════════════════════════════════════════════════
// Detector Rail Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DetectorRail {
    static constexpr int32_t TICK_RATE = 20;
    static constexpr float DETECT_INSET = 0.125f;       // 2/16
    static constexpr int32_t ACTIVE_POWER = 15;
    static constexpr int32_t STRONG_SIDE = 1;            // up
}

// ═══════════════════════════════════════════════════════════════════════════
// Track Endpoint Data
// ═══════════════════════════════════════════════════════════════════════════

namespace TrackEndpoints {
    struct Offset { int32_t dx, dy, dz; };

    // 10 shapes × 2 endpoints
    static constexpr Offset DATA[10][2] = {
        {{ 0, 0,-1}, { 0, 0, 1}},   // 0: NS
        {{-1, 0, 0}, { 1, 0, 0}},   // 1: EW
        {{-1, 0, 0}, { 1, 1, 0}},   // 2: asc east
        {{-1, 1, 0}, { 1, 0, 0}},   // 3: asc west
        {{ 0, 1,-1}, { 0, 0, 1}},   // 4: asc north
        {{ 0, 0,-1}, { 0, 1, 1}},   // 5: asc south
        {{ 1, 0, 0}, { 0, 0, 1}},   // 6: SE corner
        {{-1, 0, 0}, { 0, 0, 1}},   // 7: SW corner
        {{-1, 0, 0}, { 0, 0,-1}},   // 8: NW corner
        {{ 1, 0, 0}, { 0, 0,-1}},   // 9: NE corner
    };
}

} // namespace mccpp
