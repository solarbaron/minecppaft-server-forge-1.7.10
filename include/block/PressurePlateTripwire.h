/**
 * PressurePlateTripwire.h — Pressure plates (base, normal, weighted) and
 *                           tripwire system (wire + hook).
 *
 * Java references:
 *   - net.minecraft.block.BlockBasePressurePlate (183 lines)
 *   - net.minecraft.block.BlockPressurePlate (56 lines)
 *   - net.minecraft.block.BlockPressurePlateWeighted (47 lines)
 *   - net.minecraft.block.BlockTripWire (174 lines)
 *   - net.minecraft.block.BlockTripWireHook (309 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PRESSURE PLATE BASE (BlockBasePressurePlate) — Abstract
 * ═══════════════════════════════════════════════════════════════════════
 *
 * No collision AABB, not opaque, not normal, passable
 * Tick rate: 20 (overridden by weighted)
 * Tick randomly: true
 * canProvidePower: true
 * Mobility flag: 1 (destroyable by pistons)
 *
 * Placement: solid top surface OR fence below
 *
 * Bounds: 1/16 inset XZ (0.0625-0.9375)
 *   Released: 0-0.0625 Y (1/16)
 *   Pressed: 0-0.03125 Y (0.5/16)
 *
 * Detection AABB (getSensitiveAABB):
 *   2/16 inset XZ, Y range 0 to 0.25
 *
 * State machine (setStateIfMobInteractsWithPlate):
 *   Get plate state (entity detection)
 *   If changed: update meta + notify neighbors + re-render
 *   Newly pressed: sound pitch 0.6
 *   Newly released: sound pitch 0.5
 *   Volume: 0.3, Y position: +0.1
 *   If pressed: schedule tick
 *
 * Weak power: getPowerFromMeta(meta)
 * Strong power: same, but only on side 1 (up)
 * Notify neighbors: self + one block below
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PRESSURE PLATE (BlockPressurePlate)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Sensitivity enum:
 *   everything: any entity
 *   mobs: EntityLivingBase
 *   players: EntityPlayer
 *
 * Output: binary 0 or 15
 *   getPowerFromMeta: meta==1 → 15, else 0
 *   getMetaFromPower: power>0 → 1, else 0
 *
 * Entity check: skips doesEntityNotTriggerPressurePlate
 *
 * Block IDs: stone_pressure_plate(70), wooden_pressure_plate(72)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * WEIGHTED PRESSURE PLATE (BlockPressurePlateWeighted)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Tick rate: 10 (NOT 20)
 *
 * Output: entity count → analog power
 *   count = min(entities, maxWeight)
 *   power = ceiling(count / maxWeight × 15)
 *   Meta = power directly (0-15)
 *
 * maxWeight:
 *   Light (gold): 15 (1 entity per level)
 *   Heavy (iron): 150 (10 entities per level)
 *
 * Block IDs: light_weighted_pressure_plate(147),
 *            heavy_weighted_pressure_plate(148)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TRIPWIRE (BlockTripWire)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, render type 30
 * No collision, tick 10
 * Drops: Items.string
 *
 * Metadata:
 *   bit 0: triggered (entity on wire)
 *   bit 1: suspended (no solid below)
 *   bit 2: connected to hooks (attached)
 *   bit 3: disarmed (cut with shears)
 *
 * Bounds by state:
 *   Ground (not suspended): 0-0.09375 Y (1.5/16)
 *   Suspended, not connected: 0-0.5 Y
 *   Suspended, connected: 0.0625-0.15625 Y (1/16 to 2.5/16)
 *
 * Entity detection (func_150140_e):
 *   AABB from block bounds
 *   Checks doesEntityNotTriggerPressurePlate
 *   State change: update meta + notify hooks
 *   If triggered: schedule tick
 *
 * Hook notification (func_150138_a):
 *   Scans up to 42 blocks in both Direction.offsetX/Z pairs
 *   Stops at hook (validates rotateOpposite direction match)
 *   Stops at non-tripwire block
 *
 * Shears: onBlockHarvested sets bit 3 (disarmed)
 *
 * Block ID: tripwire(132)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TRIPWIRE HOOK (BlockTripWireHook)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, render type 29
 * No collision, tick 10
 * canProvidePower: true
 * 4 wall directions (normalCube required behind)
 *
 * Metadata:
 *   bits 0-1: direction (0=north, 1=east, 2=south, 3=west)
 *   bit 2: connected (hooked to opposite hook)
 *   bit 3: powered (entity on wire)
 *
 * Main logic (func_150136_a, 68-line switch):
 *   Scans forward up to 42 blocks for matching hook
 *   Along the way: records wire metadata, checks suspended state
 *   connected = valid opposite hook + all wires attached
 *   powered = connected + any wire triggered
 *   Updates both hooks and all wire between
 *
 * 4 distinct sounds:
 *   Powered→active: "random.click" vol=0.4, pitch=0.6
 *   Active→powered: "random.click" vol=0.4, pitch=0.5
 *   Connected new: "random.click" vol=0.4, pitch=0.7
 *   Disconnected: "random.bowhit" vol=0.4, pitch=1.2/(rand*0.2+0.9)
 *
 * Weak power: 15 when bit 3 set
 * Strong power: 15 on attached face only
 *   dir 2→side 2, 0→3, 1→4, 3→5
 *
 * Block ID: tripwire_hook(131)
 *
 * Thread safety: Plate/tripwire detection on server thread.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Pressure Plate Base Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PressurePlateBase {
    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 20;

    // ─── Mobility ───
    static constexpr int32_t MOBILITY = 1;

    // ─── Bounds (1/16 inset XZ) ───
    static constexpr float INSET = 0.0625f;             // 1/16
    static constexpr float RELEASED_HEIGHT = 0.0625f;   // 1/16
    static constexpr float PRESSED_HEIGHT = 0.03125f;   // 0.5/16

    // ─── Detection AABB ───
    static constexpr float DETECT_INSET = 0.125f;       // 2/16
    static constexpr double DETECT_Y_MAX = 0.25;

    // ─── Sound ───
    static constexpr float SOUND_VOLUME = 0.3f;
    static constexpr float PITCH_PRESS = 0.6f;
    static constexpr float PITCH_RELEASE = 0.5f;
    static constexpr double SOUND_Y = 0.1;
}

// ═══════════════════════════════════════════════════════════════════════════
// Pressure Plate (Normal) Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PressurePlateNormal {
    // ─── Block IDs ───
    static constexpr int32_t STONE_ID = 70;
    static constexpr int32_t WOODEN_ID = 72;

    // ─── Sensitivity enum ───
    static constexpr int32_t SENSITIVITY_EVERYTHING = 0;
    static constexpr int32_t SENSITIVITY_MOBS = 1;
    static constexpr int32_t SENSITIVITY_PLAYERS = 2;

    // ─── Output ───
    // Binary: 0 or 15
    // Meta: 0 = released, 1 = pressed
}

// ═══════════════════════════════════════════════════════════════════════════
// Weighted Pressure Plate Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace WeightedPressurePlate {
    // ─── Block IDs ───
    static constexpr int32_t LIGHT_ID = 147;            // gold
    static constexpr int32_t HEAVY_ID = 148;             // iron

    // ─── Max weights ───
    static constexpr int32_t LIGHT_MAX = 15;
    static constexpr int32_t HEAVY_MAX = 150;

    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 10;

    // ─── Output: ceiling(count / maxWeight × 15) ───
    // Meta = power directly (0-15)
}

// ═══════════════════════════════════════════════════════════════════════════
// Tripwire Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TripwireConstants {
    // ─── Block ID ───
    static constexpr int32_t WIRE_ID = 132;
    static constexpr int32_t HOOK_ID = 131;

    // ─── Render ───
    static constexpr int32_t WIRE_RENDER = 30;
    static constexpr int32_t HOOK_RENDER = 29;

    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 10;

    // ─── Max scan distance ───
    static constexpr int32_t MAX_SCAN = 42;

    // ─── Wire metadata bits ───
    static constexpr int32_t TRIGGERED_BIT = 1;          // bit 0
    static constexpr int32_t SUSPENDED_BIT = 2;          // bit 1
    static constexpr int32_t CONNECTED_BIT = 4;          // bit 2
    static constexpr int32_t DISARMED_BIT = 8;           // bit 3

    // ─── Hook metadata ───
    static constexpr int32_t HOOK_DIR_MASK = 3;          // bits 0-1
    static constexpr int32_t HOOK_CONNECTED = 4;         // bit 2
    static constexpr int32_t HOOK_POWERED = 8;           // bit 3

    // ─── Hook directions ───
    static constexpr int32_t DIR_NORTH = 0;
    static constexpr int32_t DIR_EAST = 1;
    static constexpr int32_t DIR_SOUTH = 2;
    static constexpr int32_t DIR_WEST = 3;

    // ─── Wire bounds ───
    static constexpr float GROUND_HEIGHT = 0.09375f;     // 1.5/16
    static constexpr float SUSPENDED_HEIGHT = 0.5f;
    static constexpr float CONNECTED_Y_MIN = 0.0625f;   // 1/16
    static constexpr float CONNECTED_Y_MAX = 0.15625f;  // 2.5/16

    // ─── Hook bounds ───
    static constexpr float HOOK_THICKNESS = 0.1875f;     // 3/16

    // ─── Hook sounds ───
    static constexpr float HOOK_VOLUME = 0.4f;
    static constexpr float PITCH_TRIP = 0.6f;
    static constexpr float PITCH_RELEASE = 0.5f;
    static constexpr float PITCH_CONNECT = 0.7f;
    // Disconnect: "random.bowhit" 1.2/(rand*0.2+0.9)

    // ─── Hook strong power side mapping ───
    // dir 2→side 2, 0→side 3, 1→side 4, 3→side 5
}

} // namespace mccpp
