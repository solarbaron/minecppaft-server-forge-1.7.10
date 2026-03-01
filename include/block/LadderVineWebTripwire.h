/**
 * LadderVineWebTripwire.h — Movement and trap blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockLadder (125 lines)
 *   - net.minecraft.block.BlockVine (289 lines)
 *   - net.minecraft.block.BlockWeb (59 lines)
 *   - net.minecraft.block.BlockTripWire (174 lines)
 *   - net.minecraft.block.BlockTripWireHook (309 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LADDER (BlockLadder)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, render type 8
 * Not opaque, collision = bounds-based
 *
 * Metadata: facing direction
 *   2(N→Z+): bounds 0-1 X, full Y, 14/16-1 Z (2/16 thick)
 *   3(S→Z-): bounds 0-1 X, full Y, 0-2/16 Z
 *   4(W→X+): bounds 14/16-1 X, full Y, full Z
 *   5(E→X-): bounds 0-2/16 X, full Y, full Z
 *
 * Placement: prefers clicked face, needs normalCube behind
 * Support: requires normalCube on attached face
 *   If unsupported: drop + setBlockToAir
 *
 * Climbing: isLadder method on EntityLivingBase
 *   Prevents fall damage when moving along
 *
 * Block ID: ladder(65)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * VINE (BlockVine)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: vine, setTickRandomly(true), render type 20
 * No collision (null), not opaque
 *
 * Metadata: 4 bits, one per direction
 *   bit 0: South (Z+), bit 1: West (X-), bit 2: North (Z-), bit 3: East (X+)
 *
 * Placement: side→meta mapping
 *   2→1(S), 3→4(N), 4→8(E), 5→2(W)
 *
 * Attachment: func_150093_a
 *   Block must renderAsNormalBlock() + material.blocksMovement()
 *
 * Support validation (func_150094_e):
 *   Each direction bit: requires solid block behind OR vine above with same bit
 *   If all bits stripped + no solid above → break
 *
 * Growth (updateTick, 1/4 chance):
 *   Density limit: max 5 vines in 9×3×9 area
 *   Can grow: up, sideways (wrap corners), down
 *   Random direction selection from 6 faces
 *
 * Harvest: shears → drop vine item, else nothing
 *
 * Block ID: vine(106)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * WEB (BlockWeb)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: web, render type 1
 * No collision (null), not opaque
 *
 * Entity collision: entity.setInWeb()
 *   Slows movement to 25% speed, prevents fall damage
 *
 * Drops: string (287)
 * Silk harvest: true (drops web itself with silk touch)
 *
 * Block ID: web(30)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TRIPWIRE (BlockTripWire)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, render type 30, tick rate 10
 * No collision (null), not opaque, setTickRandomly(true)
 *
 * Metadata:
 *   bit 0: activated (entity present)
 *   bit 1: suspended (no solid below)
 *   bit 2: connected to hooks
 *   bit 3: disarmed (shears used, suppress on break)
 *
 * Bounds: 3 states
 *   Floor (not suspended): 0 to 1.5/16 Y
 *   Suspended + not activated: 0 to 8/16 Y
 *   Suspended + activated: 1/16 to 2.5/16 Y
 *
 * Entity detection (func_150140_e):
 *   AABB within block bounds
 *   Skips entities with doesEntityNotTriggerPressurePlate()
 *   On state change: notify connected hooks
 *   While entities present: reschedule tick
 *
 * Wire→hook scan (func_150138_a):
 *   Checks 2 directions (Direction 0 and 1)
 *   Max 42 blocks in each direction
 *   Finds tripwire_hook with matching facing
 *   Triggers hook update (func_150136_a)
 *
 * Harvest: shears set bit 3 (disarmed, no activation on break)
 * Drops: string (287)
 *
 * Block IDs: tripwire(132)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TRIPWIRE HOOK (BlockTripWireHook)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, render type 29, tick rate 10
 * No collision (null), not opaque, canProvidePower=true
 *
 * Metadata:
 *   bits 0-1: direction (0-3)
 *   bit 2: connected (wire attached)
 *   bit 3: activated (triggered, provides power)
 *
 * Placement: needs normalCube behind
 *   Face 2→dir 2, 3→0, 4→1, 5→3
 *
 * Bounds: directional 3/16 thick (0.1875 × 2)
 *   Y: 0.2 to 0.8
 *
 * Hook scan (func_150136_a):
 *   Scans up to 42 blocks in hook direction
 *   Finds opposite hook with rotateOpposite facing
 *   Validates all wire between (suspended, activated)
 *   Updates connected/activated bits on both hooks
 *   Updates all wire between (connected bit 2)
 *
 * Sounds (func_150135_a):
 *   Activated → click 0.6 pitch
 *   Deactivated → click 0.5 pitch
 *   Connected → click 0.7 pitch
 *   Disconnected → bowhit 1.2/(rand*0.2+0.9)
 *
 * Redstone:
 *   Weak: bit 3 set → 15
 *   Strong: only toward attached wall face → 15
 *     dir 2/face 2, dir 0/face 3, dir 1/face 4, dir 3/face 5
 *
 * Block ID: tripwire_hook(131)
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Trap events for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Ladder Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LadderConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 65;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 8;

    // ─── Bounds ───
    static constexpr float THICKNESS = 0.125f;         // 2/16

    // ─── Facing directions ───
    // 2(N→Z+), 3(S→Z-), 4(W→X+), 5(E→X-)
    static constexpr int32_t FACE_NORTH = 2;
    static constexpr int32_t FACE_SOUTH = 3;
    static constexpr int32_t FACE_WEST = 4;
    static constexpr int32_t FACE_EAST = 5;
}

// ═══════════════════════════════════════════════════════════════════════════
// Vine Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace VineConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 106;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 20;

    // ─── Metadata bits ───
    static constexpr int32_t SOUTH_BIT = 1;   // bit 0
    static constexpr int32_t WEST_BIT  = 2;   // bit 1
    static constexpr int32_t NORTH_BIT = 4;   // bit 2
    static constexpr int32_t EAST_BIT  = 8;   // bit 3

    // ─── Placement mapping (face → meta) ───
    // face 2 → 1(S), 3 → 4(N), 4 → 8(E), 5 → 2(W)

    // ─── Growth ───
    static constexpr int32_t GROWTH_CHANCE = 4;        // 1 in 4
    static constexpr int32_t MAX_DENSITY = 5;          // in scan area
    static constexpr int32_t DENSITY_RADIUS_XZ = 4;
    static constexpr int32_t DENSITY_RADIUS_Y = 1;

    // ─── Bounds ───
    static constexpr float EDGE_THICKNESS = 0.0625f;   // 1/16
}

// ═══════════════════════════════════════════════════════════════════════════
// Web Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace WebConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 30;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 1;

    // ─── Drops ───
    static constexpr int32_t STRING_ITEM_ID = 287;

    // ─── Movement ───
    // setInWeb: movement * 0.25, prevents fall damage
}

// ═══════════════════════════════════════════════════════════════════════════
// Tripwire Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TripwireConstants {
    // ─── Block ID ───
    static constexpr int32_t WIRE_ID = 132;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 30;

    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 10;

    // ─── Metadata ───
    static constexpr int32_t ACTIVATED_FLAG = 1;       // bit 0
    static constexpr int32_t SUSPENDED_FLAG = 2;       // bit 1
    static constexpr int32_t CONNECTED_FLAG = 4;       // bit 2
    static constexpr int32_t DISARMED_FLAG = 8;        // bit 3

    // ─── Bounds (Y heights) ───
    static constexpr float FLOOR_HEIGHT = 0.09375f;          // 1.5/16
    static constexpr float SUSPENDED_HEIGHT = 0.5f;          // 8/16
    static constexpr float ACTIVATED_Y_MIN = 0.0625f;        // 1/16
    static constexpr float ACTIVATED_Y_MAX = 0.15625f;       // 2.5/16

    // ─── Scan ───
    static constexpr int32_t MAX_WIRE_LENGTH = 42;

    // ─── Drops ───
    static constexpr int32_t STRING_ITEM_ID = 287;
}

// ═══════════════════════════════════════════════════════════════════════════
// Tripwire Hook Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TripwireHookConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 131;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 29;

    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 10;

    // ─── Metadata ───
    static constexpr int32_t DIRECTION_MASK = 3;       // bits 0-1
    static constexpr int32_t CONNECTED_FLAG = 4;       // bit 2
    static constexpr int32_t ACTIVATED_FLAG = 8;       // bit 3

    // ─── Scan ───
    static constexpr int32_t MAX_WIRE_LENGTH = 42;

    // ─── Bounds ───
    static constexpr float HOOK_SIZE = 0.1875f;        // 3/16
    static constexpr float HOOK_Y_MIN = 0.2f;
    static constexpr float HOOK_Y_MAX = 0.8f;

    // ─── Redstone ───
    // Weak: activated → 15
    // Strong: directional toward attached wall → 15
    static constexpr int32_t SIGNAL_STRENGTH = 15;

    // ─── Sound pitches ───
    static constexpr float ACTIVATED_PITCH = 0.6f;
    static constexpr float DEACTIVATED_PITCH = 0.5f;
    static constexpr float CONNECTED_PITCH = 0.7f;
    // Disconnected: bowhit, 1.2/(rand*0.2+0.9)
    static constexpr float SOUND_VOLUME = 0.4f;
}

} // namespace mccpp
