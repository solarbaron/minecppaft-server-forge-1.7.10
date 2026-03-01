/**
 * FenceWall.h — Fence, fence gate, and wall blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockFence (138 lines)
 *   - net.minecraft.block.BlockFenceGate (121 lines)
 *   - net.minecraft.block.BlockWall (105 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FENCE (BlockFence)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood or nether brick, not opaque, not normal
 * Render type: 11
 * Not passable
 *
 * Connection logic (canConnectFenceTo):
 *   Connects to: self, fence_gate, opaque+normal (not gourd material)
 *   Check 4 cardinal neighbors Z-/Z+/X-/X+
 *
 * Selection bounds: extends to 0-1 on connected axes, 6/16-10/16 center
 *   Height 1.0 for selection
 *
 * Collision (1.5 blocks tall!):
 *   Two collision boxes: one for Z-axis, one for X-axis
 *   Z-axis: if Z- or Z+ connected, extends Z bounds
 *   X-axis: if X- or X+ connected (or neither axis), extends X bounds
 *   Both at 1.5 height
 *   Then final bounds set at 1.0 height for rendering
 *
 * Post dimensions: 6/16 to 10/16 (0.375-0.625), full height
 *
 * Interaction: onBlockActivated → ItemLead.func_150909_a (lead attach)
 *
 * isFence static: fence(85) or nether_brick_fence(113)
 *
 * Block IDs: fence(85), nether_brick_fence(113)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FENCE GATE (BlockFenceGate)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood, extends BlockDirectional
 * Render type: 21
 * Not opaque, not normal
 *
 * Metadata:
 *   bits 0-1: facing direction (yaw-based, 0-3)
 *   bit 2: open flag
 *
 * Placement: floor_double(yaw * 4 / 360 + 0.5) & 3
 *   Requires solid material below
 *
 * Collision:
 *   Open: null (passable)
 *   Closed, facing 0 or 2 (N/S): Z-axis 6/16-10/16, 1.5 tall
 *   Closed, facing 1 or 3 (E/W): X-axis 6/16-10/16, 1.5 tall
 *
 * Selection bounds:
 *   Facing 0,2: Z 0.375-0.625, full X
 *   Facing 1,3: X 0.375-0.625, full Z
 *
 * Interaction (onBlockActivated):
 *   If open → close (clear bit 2)
 *   If closed → open:
 *     Compute player yaw direction
 *     If opposite to gate direction → flip gate to face player
 *     Set bit 2 (open)
 *   SFX 1003
 *
 * Redstone (onNeighborBlockChange):
 *   Powered → open, unpowered → close, SFX 1003
 *
 * isPassable: open = true
 * isFenceGateOpen: (meta & 4) != 0
 *
 * Block ID: fence_gate (107)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * WALL (BlockWall)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: stone (from parent block), not opaque, not normal
 * Render type: 32
 * Not passable
 * 2 variants: normal (0), mossy (1) — damageDropped returns meta
 *
 * Connection logic (canConnectWallTo):
 *   Same as fence: self, fence_gate, opaque+normal (not gourd)
 *
 * Selection bounds:
 *   Post: 4/16-12/16 (0.25-0.75) centered, full height
 *   Connected: extends to 0/1 on axis
 *   Straight-line reduction (Z-only or X-only):
 *     Height → 13/16 (0.8125), width → 5/16-11/16 (0.3125-0.6875)
 *
 * Collision: 1.5 blocks tall (maxY = 1.5 override)
 *
 * Block ID: cobblestone_wall (139)
 *
 * Thread safety: Connection checks read-only world access.
 * JNI readiness: Connection predicate for Forge wall/fence events.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Fence Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FenceConstants {
    // ─── Block IDs ───
    static constexpr int32_t FENCE_ID = 85;
    static constexpr int32_t NETHER_FENCE_ID = 113;
    static constexpr int32_t FENCE_GATE_ID = 107;

    // ─── Post dimensions ───
    static constexpr float POST_MIN = 0.375f;   // 6/16
    static constexpr float POST_MAX = 0.625f;   // 10/16

    // ─── Collision ───
    static constexpr float COLLISION_HEIGHT = 1.5f;
    static constexpr float RENDER_HEIGHT = 1.0f;

    // ─── Connection ───
    // Connects to: self, fence_gate, opaque+normal blocks (not gourd)

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 11;
}

// ═══════════════════════════════════════════════════════════════════════════
// Fence Gate Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FenceGateConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 107;

    // ─── Metadata ───
    static constexpr int32_t FACING_MASK = 3;     // bits 0-1
    static constexpr int32_t OPEN_FLAG = 4;        // bit 2
    static constexpr int32_t CLOSE_MASK = ~4;      // 0xFFFFFFFB

    // ─── Facing ───
    // Yaw-based: floor(yaw * 4 / 360 + 0.5) & 3
    // 0=south, 1=west, 2=north, 3=east (BlockDirectional convention)

    // ─── Bounds ───
    static constexpr float GATE_MIN = 0.375f;     // 6/16
    static constexpr float GATE_MAX = 0.625f;     // 10/16
    static constexpr float COLLISION_HEIGHT = 1.5f;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 21;

    // ─── Sound ───
    static constexpr int32_t SFX_ID = 1003;

    // ─── Utility ───
    inline bool isOpen(int32_t meta) { return (meta & OPEN_FLAG) != 0; }
    inline int32_t getDirection(int32_t meta) { return meta & FACING_MASK; }
}

// ═══════════════════════════════════════════════════════════════════════════
// Wall Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace WallConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 139;

    // ─── Variants ───
    static constexpr int32_t VARIANT_NORMAL = 0;
    static constexpr int32_t VARIANT_MOSSY = 1;

    // ─── Post dimensions ───
    static constexpr float POST_MIN = 0.25f;     // 4/16
    static constexpr float POST_MAX = 0.75f;     // 12/16

    // ─── Straight-line pillar reduction ───
    static constexpr float PILLAR_HEIGHT = 0.8125f;   // 13/16
    static constexpr float PILLAR_MIN = 0.3125f;      // 5/16
    static constexpr float PILLAR_MAX = 0.6875f;      // 11/16

    // ─── Collision ───
    static constexpr float COLLISION_HEIGHT = 1.5f;
    static constexpr float RENDER_HEIGHT = 1.0f;

    // ─── Connection ───
    // Same as fence: self, fence_gate, opaque+normal (not gourd)

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 32;
}

} // namespace mccpp
