/**
 * RedstoneTorchWireLampDaylight.h — Torch, wire, lamp, daylight detector.
 *
 * Java references:
 *   - net.minecraft.block.BlockRedstoneTorch (180 lines)
 *   - net.minecraft.block.BlockRedstoneWire (323 lines)
 *   - net.minecraft.block.BlockRedstoneLight (66 lines)
 *   - net.minecraft.block.BlockDaylightDetector (92 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * REDSTONE TORCH (BlockRedstoneTorch)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockTorch, tick rate: 2
 * field_150113_a: true=lit, false=unlit (burnout)
 * Tick randomly: true
 *
 * Burnout protection (func_150111_a):
 *   Per-world Toggle list: (x, y, z, totalWorldTime)
 *   Purge entries older than 60 ticks
 *   If 8+ toggles at same position → burnout
 *   Burnout sound: "random.fizz" vol=0.5, pitch=2.6±0.8
 *   5 smoke particles at random offsets (0.2+rand*0.6)
 *
 * Power input detection (func_150110_m):
 *   Checks attached face for indirect power output
 *   Meta→face: 5→below(0), 3→north(2), 4→south(3),
 *              1→west(4), 2→east(5)
 *
 * Weak power:
 *   If unlit: 0
 *   Excludes attached face (same meta→side rules)
 *   Otherwise: 15
 *
 * Strong power: only on side 0 (down) → returns weak power
 *
 * State transitions (updateTick):
 *   Lit + input → switch to unlit (+ burnout check)
 *   Unlit + no input + no burnout → switch to lit
 *
 * Add/break: notify all 6 neighbors if lit
 *
 * Block IDs: redstone_torch(76), unlit_redstone_torch(75)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * REDSTONE WIRE (BlockRedstoneWire)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, render type 5
 * Height: 1/16 (0.0625), no collision AABB
 * canPlaceBlockAt: solid top surface OR glowstone below
 * Drops: Items.redstone
 *
 * Meta = power level 0-15, decreases by 1 per wire block
 *
 * canProvidePower: toggleable field (prevents feedback loops)
 *
 * Power propagation (func_150175_a / func_150177_e):
 *   1. Find max neighbor wire power (func_150178_a)
 *   2. Disable canProvidePower
 *   3. Get strongest indirect power
 *   4. Re-enable canProvidePower
 *   5. result = max(indirect, max(neighborWire-1, 0))
 *   6. If indirect > result-1: result = indirect
 *   7. Notify via Set<ChunkPosition> (self + 6 neighbors)
 *
 * Slope connection logic:
 *   Neighbor normalCube + self+1 not normalCube → check up slope
 *   Neighbor not normalCube → check down slope (-1 Y)
 *
 * Weak power output (directional):
 *   Side 1 (up): always returns power
 *   No connections detected (all 4 false): all sides get power
 *   Otherwise: per-connection geometry rules
 *     side 2(north): only if connectedNorth && !connectedWest && !connectedEast
 *     side 3(south): only if connectedSouth (same filter)
 *     side 4(west): if connectedWest && !connectedNorth && !connectedSouth
 *     side 5(east): if connectedEast (same filter)
 *
 * Strong power: same as weak (if canProvidePower)
 *
 * Connection check (isPowerProviderOrWire):
 *   Wire → always true
 *   Repeater → direction match (input or output side)
 *   Other power provider → true if direction != -1
 * func_150176_g: also checks powered_repeater output direction
 *
 * Place/break: extensive neighbor notification including slope wires
 *
 * Block ID: redstone_wire(55)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * REDSTONE LAMP (BlockRedstoneLight)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: redstoneLight
 * field_150171_a: true=lit, false=unlit
 * Lit: lightLevel 1.0
 *
 * Turn ON: instant setBlock to lit_redstone_lamp
 * Turn OFF: 4-tick delay (scheduleBlockUpdate)
 *   updateTick: if still no power → setBlock to redstone_lamp
 *
 * Drop/createStackedBlock: always unlit lamp
 *
 * Block IDs: redstone_lamp(123), lit_redstone_lamp(124)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DAYLIGHT DETECTOR (BlockDaylightDetector)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, material: wood
 * Height: 6/16 (0.375)
 * TileEntity: TileEntityDaylightDetector
 * canProvidePower: true
 *
 * Light calculation (func_149957_e):
 *   If hasNoSky: return (no output in nether/end)
 *   skyLight = getSavedLightValue(Sky) - skylightSubtracted
 *   angle = getCelestialAngleRadians(1.0)
 *   Smoothing:
 *     angle < PI → += (0 - angle) * 0.2
 *     angle >= PI → += (2*PI - angle) * 0.2
 *   power = round(skyLight * cos(smoothedAngle))
 *   Clamped 0-15
 *
 * Weak power: meta directly (0-15)
 * No strong power, no comparator override
 * Empty stubs: onBlockAdded, onNeighborBlockChange, updateTick
 *
 * Block ID: daylight_detector(151)
 *
 * Thread safety: Wire propagation set-based, torch toggle map per-world.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Redstone Torch Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RedstoneTorchConst {
    // ─── Block IDs ───
    static constexpr int32_t LIT_ID = 76;
    static constexpr int32_t UNLIT_ID = 75;

    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 2;

    // ─── Burnout protection ───
    static constexpr int32_t BURNOUT_THRESHOLD = 8;
    static constexpr int64_t BURNOUT_WINDOW = 60;

    // ─── Burnout sound ───
    static constexpr float FIZZ_VOLUME = 0.5f;
    static constexpr float FIZZ_BASE_PITCH = 2.6f;
    static constexpr float FIZZ_PITCH_RANGE = 0.8f;
    static constexpr int32_t SMOKE_PARTICLES = 5;

    // ─── Power output ───
    static constexpr int32_t OUTPUT_POWER = 15;
}

// ═══════════════════════════════════════════════════════════════════════════
// Redstone Wire Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RedstoneWireConst {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 55;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 5;

    // ─── Bounds ───
    static constexpr float HEIGHT = 0.0625f;             // 1/16

    // ─── Power ───
    static constexpr int32_t MAX_POWER = 15;
    // Power decreases by 1 per wire block

    // ─── Glowstone ID for alternative placement ───
    static constexpr int32_t GLOWSTONE_ID = 89;
}

// ═══════════════════════════════════════════════════════════════════════════
// Redstone Lamp Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RedstoneLampConst {
    // ─── Block IDs ───
    static constexpr int32_t UNLIT_ID = 123;
    static constexpr int32_t LIT_ID = 124;

    // ─── Turn off delay ───
    static constexpr int32_t OFF_DELAY = 4;

    // ─── Light ───
    static constexpr float LIT_LIGHT_LEVEL = 1.0f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Daylight Detector Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DaylightDetectorConst {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 151;

    // ─── Bounds ───
    static constexpr float HEIGHT = 0.375f;              // 6/16

    // ─── Light calculation ───
    static constexpr float SMOOTHING = 0.2f;
    // power = round(skyLight * cos(smoothedAngle))
    // Clamped 0-15
}

} // namespace mccpp
