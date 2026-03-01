/**
 * RailBlocks.h — Rail system: base, normal, powered, detector.
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
 * Material: circuits, no collision
 * Height: 2/16 flat (0.125), 10/16 ascending (0.625)
 * Not opaque, not normal, render type 9
 * Mobility: 0 (pushable by piston)
 *
 * isPowered flag: true for golden_rail, detector_rail, activator_rail
 *
 * isRailBlock: rail(66), golden_rail(27), detector_rail(28), activator_rail(157)
 *
 * Track shapes (metadata):
 *   0: NS flat, 1: EW flat
 *   2: ascending E, 3: ascending W
 *   4: ascending N, 5: ascending S
 *   6: NE curve, 7: SE curve, 8: SW curve, 9: NW curve
 *   (curves only for normal rail, not powered variants)
 *
 * Placement: requires solid top surface below
 * Ascending: requires solid surface at bottom of slope
 *   2(ascending E): solid at x+1
 *   3(ascending W): solid at x-1
 *   4(ascending N): solid at z-1
 *   5(ascending S): solid at z+1
 *
 * refreshTrackShape: uses BlockRailBase$Rail inner class
 *   Handles auto-connecting to adjacent rails, junctions, curves
 *
 * breakBlock: ascending rails notify above, powered notify self+below
 *
 * ═══════════════════════════════════════════════════════════════════════
 * NORMAL RAIL (BlockRail)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * isPowered = false. Supports curves (meta 6-9).
 *
 * onRedstoneSignal: if 3-way junction + power provider → refresh shape
 *   Allows redstone to toggle T-junction switches
 *
 * Block ID: rail (66)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * POWERED RAIL / ACTIVATOR RAIL (BlockRailPowered)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * isPowered = true. No curves (meta 0-5 only).
 *
 * Metadata:
 *   bits 0-2: shape (0-5, no curves)
 *   bit 3: powered flag
 *
 * Power propagation (func_150058_a):
 *   Chain up to 8 blocks in each direction along connected rails
 *   Follows rail connections including slopes (Y transitions)
 *   Axis matching: rejects if adjacent rail axis doesn't match
 *     NS(0) rejects EW(1) connections, etc.
 *   Receives from: redstone OR adjacent powered rail in chain
 *
 * Direction scanning:
 *   shape 0: Z+/Z- direction
 *   shape 1: X-/X+ direction
 *   shape 2: ascending E → X- flat, X+ up
 *   shape 3: ascending W → X- up, X+ flat
 *   shape 4: ascending N → Z+ flat, Z- up
 *   shape 5: ascending S → Z+ up, Z- flat
 *
 * onRedstoneSignal:
 *   Check redstone + chain both directions
 *   State changed → update meta, notify below + above if ascending
 *
 * Effect on minecarts:
 *   Golden rail: powered = boost, unpowered = brake
 *   Activator rail: powered = activate hopper/tnt/command minecarts
 *
 * Block IDs: golden_rail(27), activator_rail(157)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DETECTOR RAIL (BlockRailDetector)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * isPowered = true. Entity detection.
 * Tick rate: 20, setTickRandomly(true)
 * canProvidePower = true
 *
 * Detection AABB: 2/16 inset XZ, full Y (n to n+1-2/16)
 *   Detects EntityMinecart within bounds
 *
 * Metadata:
 *   bits 0-2: shape
 *   bit 3: active (minecart present)
 *
 * State machine:
 *   onEntityCollided (not active): scan → if found → activate
 *   updateTick (active): scan → if empty → deactivate
 *   Schedule next tick while active
 *
 * Power:
 *   Weak: 15 when active, 0 off
 *   Strong: 15 only face 1 (below)
 *
 * Comparator override:
 *   Command block minecart → getSuccessCount
 *   Inventory minecart → calcRedstoneFromInventory
 *   Other → 0
 *
 * Block ID: detector_rail (28)
 *
 * Thread safety: Rail shape updates on server thread.
 * JNI readiness: Rail connection for Forge rail events.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Rail Base Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RailBaseConstants {
    // ─── Block IDs ───
    static constexpr int32_t RAIL_ID = 66;
    static constexpr int32_t GOLDEN_RAIL_ID = 27;
    static constexpr int32_t DETECTOR_RAIL_ID = 28;
    static constexpr int32_t ACTIVATOR_RAIL_ID = 157;

    // ─── Height ───
    static constexpr float FLAT_HEIGHT = 0.125f;      // 2/16
    static constexpr float ASCENDING_HEIGHT = 0.625f;  // 10/16

    // ─── Track shapes ───
    static constexpr int32_t SHAPE_NS = 0;
    static constexpr int32_t SHAPE_EW = 1;
    static constexpr int32_t SHAPE_ASCENDING_E = 2;
    static constexpr int32_t SHAPE_ASCENDING_W = 3;
    static constexpr int32_t SHAPE_ASCENDING_N = 4;
    static constexpr int32_t SHAPE_ASCENDING_S = 5;
    static constexpr int32_t SHAPE_CURVE_NE = 6;
    static constexpr int32_t SHAPE_CURVE_SE = 7;
    static constexpr int32_t SHAPE_CURVE_SW = 8;
    static constexpr int32_t SHAPE_CURVE_NW = 9;

    // ─── Metadata ───
    static constexpr int32_t SHAPE_MASK = 7;        // bits 0-2 (powered rails)
    static constexpr int32_t SHAPE_MASK_FULL = 15;  // bits 0-3 (normal rail)
    static constexpr int32_t POWERED_FLAG = 8;       // bit 3

    // ─── Mobility ───
    static constexpr int32_t MOBILITY = 0;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 9;
}

// ═══════════════════════════════════════════════════════════════════════════
// Powered Rail Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PoweredRailConstants {
    // ─── Chain propagation ───
    static constexpr int32_t MAX_CHAIN_LENGTH = 8;

    // ─── Effect ───
    // Golden rail: powered = speed boost (0.06 velocity per tick)
    // Golden rail: unpowered = brake (halve velocity)
    // Activator rail: powered = activate special minecarts
    static constexpr double BOOST_FORCE = 0.06;
    static constexpr double BRAKE_FACTOR = 0.5;
    static constexpr double MIN_LAUNCH_SPEED = 0.02;  // minimum speed when starting
}

// ═══════════════════════════════════════════════════════════════════════════
// Detector Rail Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DetectorRailConstants {
    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 20;

    // ─── Detection AABB ───
    static constexpr float DETECT_INSET = 0.125f;     // 2/16

    // ─── Power ───
    static constexpr int32_t POWER_ON = 15;
    static constexpr int32_t POWER_OFF = 0;
    static constexpr int32_t STRONG_FACE = 1;  // below

    // ─── Comparator ───
    // Command block minecart → successCount
    // Inventory minecart → calcRedstoneFromInventory
}

} // namespace mccpp
