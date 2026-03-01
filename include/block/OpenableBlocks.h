/**
 * OpenableBlocks.h — Door, trapdoor, and fence gate.
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
 * 2-block tall, render type 7
 * Not opaque, not normal
 * Mobility: 1 (destroyable by pistons)
 *
 * Materials: wood (right-clickable) or iron (redstone only)
 *
 * Metadata (per-half):
 *   Bottom half:
 *     bits 0-1: direction (0-3)
 *     bit 2: open (1) / closed (0)
 *     bit 3: 0 (indicates bottom)
 *   Top half:
 *     bit 0: hinge side (0=left, 1=right)
 *     bit 3: 1 (indicates top)
 *
 * getFullMetadata: merges both halves
 *   bits 0-2: from bottom (dir + open)
 *   bit 3: isTop
 *   bit 4: hinge from top half
 *
 * Bounds (func_150011_b), 3/16 thick panel:
 *   Direction determines panel position when closed
 *   Hinge (bit 4) mirrors the open direction
 *   dir 0: closed → west(X=0), open → south/north (hinge)
 *   dir 1: closed → south(Z=0), open → east/west (hinge)
 *   dir 2: closed → east(X=1-f), open → north/south (hinge)
 *   dir 3: closed → north(Z=1-f), open → west/east (hinge)
 *
 * Activation (right-click):
 *   Iron: return true, no toggle
 *   Wood: XOR bit 2 on bottom half, play sound 1003
 *
 * Redstone (onNeighborBlockChange):
 *   Bottom half:
 *     Orphaned (no top) → air
 *     No solid below → air + drop (also clear top)
 *     Check powered state on both halves
 *   Top half:
 *     Orphaned (no bottom) → air
 *     Otherwise delegate to bottom
 *
 * canPlaceBlockAt: Y < 255 + solid below + room for 2 blocks
 * Creative harvest: removes lower half when harvesting top
 * Drop: bottom half only → iron_door / wooden_door
 *
 * Block IDs: wooden_door(64), iron_door(71)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TRAPDOOR (BlockTrapDoor)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * 3/16 thick (0.1875), render type 0
 * Not opaque, not normal
 * Materials: wood (right-clickable) or iron (redstone only)
 *
 * Metadata:
 *   bits 0-1: direction (attached face)
 *     0=south(+Z), 1=north(-Z), 2=east(+X), 3=west(-X)
 *   bit 2: open (1) / closed (0)
 *   bit 3: top (1) / bottom (0)
 *
 * Bounds:
 *   Closed + bottom: 0-0.1875 Y
 *   Closed + top: (1-0.1875)-1.0 Y
 *   Open: full height panel on attached face
 *
 * Placement:
 *   side 2→dir 0, 3→1, 4→2, 5→3
 *   Cannot place on top/bottom faces (side 0 or 1)
 *   hitY > 0.5 (not top/bottom) → top bit set
 *
 * Valid support block:
 *   (opaque material + renderAsNormalBlock) OR
 *   glowstone OR instanceof BlockSlab OR instanceof BlockStairs
 *
 * Redstone: isBlockIndirectlyGettingPowered → toggle open
 *
 * Block ID: trapdoor(96)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FENCE GATE (BlockFenceGate)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockDirectional, material: wood
 * Render type 21, not opaque, not normal
 *
 * Metadata:
 *   bits 0-1: direction (0-3, yaw-based)
 *   bit 2: open (1) / closed (0)
 *
 * Placement: solid material below required
 *   Direction: floor(yaw*4/360+0.5) & 3
 *
 * Collision:
 *   Open: null (no collision)
 *   Closed, dir 0/2 (N/S): Z 0.375-0.625, full X, Y 0-1.5
 *   Closed, dir 1/3 (E/W): X 0.375-0.625, full Z, Y 0-1.5
 *   Note: 1.5 block tall collision (prevents jumping over)
 *
 * Activation (right-click):
 *   Open → close: clear bit 2
 *   Close → open: set bit 2
 *     Face reversal: if facing = (playerDir+2)%4 → swap to playerDir
 *   Sound 1003
 *
 * Redstone: powered→open, unpowered→close, sound 1003
 *
 * Block ID: fence_gate(107)
 *
 * Thread safety: Block state changes on server thread.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Door Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DoorConst {
    // ─── Block IDs ───
    static constexpr int32_t WOODEN_DOOR_ID = 64;
    static constexpr int32_t IRON_DOOR_ID = 71;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 7;

    // ─── Thickness ───
    static constexpr float THICKNESS = 0.1875f;          // 3/16

    // ─── Bottom half metadata ───
    static constexpr int32_t DIR_MASK = 3;               // bits 0-1
    static constexpr int32_t OPEN_FLAG = 4;              // bit 2
    static constexpr int32_t TOP_FLAG = 8;               // bit 3

    // ─── Full metadata (merged) ───
    static constexpr int32_t FULL_DIR_OPEN_MASK = 7;     // bits 0-2
    static constexpr int32_t HINGE_FLAG = 16;            // bit 4

    // ─── Mobility ───
    static constexpr int32_t MOBILITY = 1;

    // ─── Placement ───
    static constexpr int32_t MAX_Y = 255;

    // ─── Sound ───
    static constexpr int32_t DOOR_SOUND = 1003;
}

// ═══════════════════════════════════════════════════════════════════════════
// Trapdoor Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TrapdoorConst {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 96;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 0;

    // ─── Thickness ───
    static constexpr float THICKNESS = 0.1875f;          // 3/16

    // ─── Metadata ───
    static constexpr int32_t DIR_MASK = 3;               // bits 0-1
    static constexpr int32_t OPEN_FLAG = 4;              // bit 2
    static constexpr int32_t TOP_FLAG = 8;               // bit 3

    // ─── Direction → attached face ───
    // 0=south(+Z), 1=north(-Z), 2=east(+X), 3=west(-X)

    // ─── Sound ───
    static constexpr int32_t DOOR_SOUND = 1003;
}

// ═══════════════════════════════════════════════════════════════════════════
// Fence Gate Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FenceGateConst {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 107;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 21;

    // ─── Metadata ───
    static constexpr int32_t DIR_MASK = 3;               // bits 0-1
    static constexpr int32_t OPEN_FLAG = 4;              // bit 2

    // ─── Collision (closed) ───
    static constexpr float FENCE_MIN = 0.375f;           // 6/16
    static constexpr float FENCE_MAX = 0.625f;           // 10/16
    static constexpr float COLLISION_HEIGHT = 1.5f;      // 24/16

    // ─── Sound ───
    static constexpr int32_t GATE_SOUND = 1003;
}

} // namespace mccpp
