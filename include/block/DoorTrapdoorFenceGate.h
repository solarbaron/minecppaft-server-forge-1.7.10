/**
 * DoorTrapdoorFenceGate.h — Openable interactive blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockDoor (251 lines)
 *   - net.minecraft.block.BlockTrapDoor (204 lines)
 *   - net.minecraft.block.BlockFenceGate (121 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DOOR (BlockDoor)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood or iron
 * 2-block tall structure (bottom + top half)
 * Render type 7, not opaque, not normal
 * Mobility flag: 1 (push only)
 *
 * Metadata (per half, 4 bits):
 *   Bottom half (bit 3 = 0):
 *     bits 0-1: direction (0-3)
 *     bit 2: open/closed
 *   Top half (bit 3 = 1):
 *     bit 0: hinge side (left/right)
 *
 * Full metadata (getFullMetadata, 5 bits combined):
 *   bits 0-2: direction + open from bottom
 *   bit 3: is upper half
 *   bit 4: hinge from top
 *
 * Bounds: 3/16 thick (0.1875), rotated by direction
 *   Closed: thin panel on one face
 *   Open: thin panel rotated 90° (with hinge flip)
 *   4 directions × 2 open states × 2 hinge sides = 16 configs
 *
 * Interaction:
 *   Wood: toggles open bit (XOR 4), plays sound 1003
 *   Iron: cannot be hand-opened (returns true, no toggle)
 *   Opens both halves via metadata on bottom
 *
 * Redstone (func_150014_a):
 *   Powered → open, unpowered → close
 *   Checks power at both bottom and top Y
 *   Only toggles if state actually changes
 *
 * Support validation:
 *   Bottom: solid top surface below required
 *   Top: bottom half must exist below
 *   Y ≤ 254 (needs 2 blocks)
 *   Missing half → break with drop
 *
 * Drops:
 *   Only from bottom half (top meta & 8 → null)
 *   Wood: wooden_door, Iron: iron_door
 *
 * Creative harvest: upper half breaks lower without drop
 *
 * Block IDs: wooden_door(64), iron_door(71)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TRAPDOOR (BlockTrapDoor)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood or iron
 * Render type 0, not opaque, not normal
 *
 * Metadata (4 bits):
 *   bits 0-1: direction (0=south, 1=north, 2=east, 3=west)
 *   bit 2: open (1) / closed (0)
 *   bit 3: top half (1) / bottom half (0) of block
 *
 * Bounds: 3/16 thick (0.1875)
 *   Closed bottom: 0 to 3/16 Y
 *   Closed top: 13/16 to 1 Y
 *   Open: thin panel on face (rotated by direction)
 *
 * Item render: centered at half height
 *
 * Interaction:
 *   Wood: toggles open (XOR 4), plays sound 1003
 *   Iron: cannot be hand-opened
 *
 * Redstone (func_150120_a):
 *   Same as door: powered → open, only toggles if changed
 *
 * Placement:
 *   Face placement: side 2→dir 0, 3→1, 4→2, 5→3
 *   Cannot place on top(0) or bottom(1) face
 *   Upper/lower: if hitY > 0.5 and not top/bottom → set bit 3
 *
 * Support validation (isValidSupportBlock):
 *   Opaque + renderAsNormalBlock, OR glowstone, OR slab, OR stairs
 *   Direction determines which neighbor to check
 *
 * Passability: passable when NOT open (closed = flat, entities walk on)
 *
 * Block IDs: trapdoor(96), iron_trapdoor doesn't exist in 1.7.10
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FENCE GATE (BlockFenceGate)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockDirectional, material: wood
 * Render type 21, not opaque, not normal
 *
 * Metadata (4 bits):
 *   bits 0-1: direction (0-3, from player yaw)
 *   bit 2: open (1) / closed (0)
 *
 * Collision:
 *   Closed: 4/16 thick (0.375-0.625) along axis, 1.5 tall
 *     N/S (dir 0,2): full X, 0.375-0.625 Z
 *     E/W (dir 1,3): 0.375-0.625 X, full Z
 *   Open: null (no collision)
 *
 * Visual bounds (setBlockBoundsBasedOnState):
 *   Same axis orientation as collision, but 1.0 tall
 *
 * Interaction (onBlockActivated):
 *   If open: close (clear bit 2)
 *   If closed: set bit 2
 *     Smart facing: if player behind gate, flip direction
 *     ((direction + 2) % 4 == playerDir) → use playerDir
 *
 * Redstone (onNeighborBlockChange):
 *   Powered + closed → open
 *   Unpowered + open → close
 *   Sound 1003
 *
 * Placement:
 *   Needs solid material below
 *   Direction from player yaw
 *
 * Passability: passable when open
 *
 * Block ID: fence_gate(107)
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Door/trapdoor/gate events for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Door Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DoorConstants {
    // ─── Block IDs ───
    static constexpr int32_t WOODEN_DOOR_ID = 64;
    static constexpr int32_t IRON_DOOR_ID = 71;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 7;

    // ─── Metadata bits ───
    static constexpr int32_t DIRECTION_MASK = 3;        // bits 0-1
    static constexpr int32_t OPEN_FLAG = 4;             // bit 2
    static constexpr int32_t UPPER_FLAG = 8;            // bit 3
    static constexpr int32_t HINGE_FLAG = 16;           // bit 4 (full meta)
    static constexpr int32_t LOWER_MASK = 7;            // bits 0-2 from bottom

    // ─── Bounds ───
    static constexpr float THICKNESS = 0.1875f;         // 3/16

    // ─── Mobility ───
    static constexpr int32_t MOBILITY_FLAG = 1;         // push only

    // ─── Max Y for placement ───
    static constexpr int32_t MAX_PLACE_Y = 254;         // needs Y+1 for top

    // ─── Sound ───
    static constexpr int32_t DOOR_SOUND_ID = 1003;

    // ─── Drop items ───
    static constexpr int32_t WOODEN_DOOR_ITEM_ID = 324;
    static constexpr int32_t IRON_DOOR_ITEM_ID = 330;
}

// ═══════════════════════════════════════════════════════════════════════════
// Trapdoor Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TrapdoorConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 96;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 0;

    // ─── Metadata bits ───
    static constexpr int32_t DIRECTION_MASK = 3;        // bits 0-1
    static constexpr int32_t OPEN_FLAG = 4;             // bit 2
    static constexpr int32_t TOP_FLAG = 8;              // bit 3

    // ─── Directions ───
    // 0=south face, 1=north face, 2=east face, 3=west face
    // Face placement: side 2→0, 3→1, 4→2, 5→3

    // ─── Bounds ───
    static constexpr float THICKNESS = 0.1875f;         // 3/16

    // ─── Support ───
    // Opaque + normalBlock, OR glowstone(89), OR slab, OR stairs
    static constexpr int32_t GLOWSTONE_ID = 89;

    // ─── Sound ───
    static constexpr int32_t TRAPDOOR_SOUND_ID = 1003;

    // ─── isTrapdoorOpen ───
    inline bool isOpen(int32_t meta) { return (meta & OPEN_FLAG) != 0; }
}

// ═══════════════════════════════════════════════════════════════════════════
// Fence Gate Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FenceGateConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 107;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 21;

    // ─── Metadata bits ───
    static constexpr int32_t DIRECTION_MASK = 3;        // bits 0-1
    static constexpr int32_t OPEN_FLAG = 4;             // bit 2
    static constexpr int32_t CLOSE_MASK = ~OPEN_FLAG;   // 0xFFFFFFFB

    // ─── Collision ───
    static constexpr float GATE_MIN = 0.375f;           // 6/16
    static constexpr float GATE_MAX = 0.625f;           // 10/16
    static constexpr float GATE_HEIGHT = 1.5f;          // collision taller than block

    // ─── Sound ───
    static constexpr int32_t GATE_SOUND_ID = 1003;

    // ─── isFenceGateOpen ───
    inline bool isOpen(int32_t meta) { return (meta & OPEN_FLAG) != 0; }

    // ─── Smart facing ───
    // On open: if player behind → flip direction
    // (direction + 2) % 4 == playerDirection → use playerDirection
}

} // namespace mccpp
