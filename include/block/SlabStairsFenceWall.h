/**
 * SlabStairsFenceWall.h — Structural half-blocks, stairs, fences, and walls.
 *
 * Java references:
 *   - net.minecraft.block.BlockSlab (102 lines)
 *   - net.minecraft.block.BlockStairs (397 lines)
 *   - net.minecraft.block.BlockFence (138 lines)
 *   - net.minecraft.block.BlockWall (105 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SLAB (BlockSlab)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Abstract base class, material varies
 * isFullBlock: true for double slabs, false for single
 * LightOpacity: 255 (full)
 *
 * Metadata:
 *   bits 0-2: sub-type (stone, sandstone, wood, etc.)
 *   bit 3: upper half (1) / lower half (0) — single slabs only
 *
 * Bounds:
 *   Full: 0-1 all axes
 *   Lower half: 0.0 to 0.5 Y
 *   Upper half: 0.5 to 1.0 Y
 *
 * Placement (onBlockPlaced):
 *   Full slab: no change
 *   Face 0 (bottom) OR (not face 1 AND hitY > 0.5) → upper (bit 3)
 *   Else → lower
 *
 * Drops:
 *   Full slab: 2 single slabs
 *   Single: 1 slab
 *   damageDropped: meta & 7 (strip upper bit)
 *
 * Opaque: only if fullBlock
 * renderAsNormalBlock: only if fullBlock
 *
 * Sub-types (stone_slab):
 *   0: stone, 1: sandstone, 2: wood(old), 3: cobblestone
 *   4: brick, 5: stone_brick, 6: nether_brick, 7: quartz
 *
 * Block IDs: stone_slab(44), double_stone_slab(43),
 *   wooden_slab(126), double_wooden_slab(125)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * STAIRS (BlockStairs)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: from model block (field_150149_b)
 * Render type 10, not opaque, not normal
 * LightOpacity: 255
 *
 * Delegates to model block: hardness, resistance, sound, tick,
 *   onBlockClicked, onBlockActivated, onBlockDestroyedByPlayer,
 *   onBlockDestroyedByExplosion, onEntityWalking, breakBlock,
 *   canPlaceBlockAt, isCollidable, canStopRayTrace,
 *   modifyEntityVelocity, getMapColor(uses field_150151_M)
 *
 * Metadata:
 *   bits 0-1: direction (0: east, 1: west, 2: south, 3: north)
 *   bit 2: upside-down (1) / right-side-up (0)
 *
 * Placement:
 *   onBlockPlaced: face 0 OR (not face 1 AND hitY > 0.5) → upside-down
 *   onBlockPlacedBy: yaw → direction mapping:
 *     yaw 0→dir 2, yaw 1→dir 1, yaw 2→dir 3, yaw 3→dir 0
 *
 * Collision: up to 3 octant sub-boxes
 *   1. Base half-slab (top or bottom per bit 2)
 *   2. Step corner (func_150145_f — inner corner detection)
 *   3. Extra corner (func_150144_g — outer corner detection)
 *   Then resets to full bounds
 *
 * Corner detection:
 *   Checks adjacent stair blocks for matching upside-down flag
 *   4 direction × 4 neighbor combos = inner/outer L-shape
 *
 * Ray trace: 8-octant subdivision
 *   field_150150_a[8][2]: octant exclusion table per direction
 *   Tests all 8 octant boxes, excludes per direction
 *   Returns furthest hit from end vector
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FENCE (BlockFence)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood (normal fence) or nether_brick
 * Render type 11, not opaque, not normal, not passable
 *
 * Center post: 6/16 wide (0.375-0.625) both axes
 * Connection extends to edge (0 or 1) when connected
 *
 * Visual bounds: 1.0 tall
 * Collision: 1.5 tall (prevents jumping over)
 *   Split into Z-axis and X-axis collision boxes
 *   If no connections: single center post collision
 *
 * canConnectFenceTo:
 *   Same block: yes
 *   fence_gate: yes
 *   Opaque + normalBlock + NOT gourd material: yes
 *   Else: no
 *
 * Lead attachment: onBlockActivated → ItemLead.func_150909_a
 *
 * isFence check: fence(85) or nether_brick_fence(113)
 *
 * Block IDs: fence(85), nether_brick_fence(113)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * WALL (BlockWall)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: from model block (cobblestone)
 * Render type 32, not opaque, not normal, not passable
 *
 * 2 variants: 0=normal, 1=mossy
 *
 * Center post: 8/16 wide (0.25-0.75) both axes
 * Connection extends to edge (0 or 1) when connected
 *
 * Straight wall (N-S or E-W only):
 *   Height reduced to 13/16 (0.8125)
 *   Post narrowed to 5/16 (0.3125-0.6875) on cross-axis
 *
 * Visual bounds: 1.0 tall (or 13/16 if straight)
 * Collision: 1.5 tall
 *
 * canConnectWallTo:
 *   Same as fence: self, fence_gate, opaque+normal-not-gourd
 *
 * damageDropped = meta (preserves variant)
 *
 * Block ID: cobblestone_wall(139)
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Structural block events for Forge.
 */
#pragma once

#include <cstdint>
#include <array>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Slab Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SlabConstants {
    // ─── Block IDs ───
    static constexpr int32_t STONE_SLAB_ID = 44;
    static constexpr int32_t DOUBLE_STONE_SLAB_ID = 43;
    static constexpr int32_t WOODEN_SLAB_ID = 126;
    static constexpr int32_t DOUBLE_WOODEN_SLAB_ID = 125;

    // ─── Metadata ───
    static constexpr int32_t TYPE_MASK = 7;             // bits 0-2
    static constexpr int32_t UPPER_FLAG = 8;            // bit 3

    // ─── Bounds ───
    static constexpr float HALF_HEIGHT = 0.5f;

    // ─── Light ───
    static constexpr int32_t LIGHT_OPACITY = 255;

    // ─── Sub-types (stone_slab) ───
    static constexpr int32_t STONE = 0;
    static constexpr int32_t SANDSTONE = 1;
    static constexpr int32_t WOOD_OLD = 2;
    static constexpr int32_t COBBLESTONE = 3;
    static constexpr int32_t BRICK = 4;
    static constexpr int32_t STONE_BRICK = 5;
    static constexpr int32_t NETHER_BRICK = 6;
    static constexpr int32_t QUARTZ = 7;
}

// ═══════════════════════════════════════════════════════════════════════════
// Stairs Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace StairsConstants {
    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 10;

    // ─── Light ───
    static constexpr int32_t LIGHT_OPACITY = 255;

    // ─── Metadata ───
    static constexpr int32_t DIRECTION_MASK = 3;        // bits 0-1
    static constexpr int32_t UPSIDE_DOWN_FLAG = 4;      // bit 2

    // ─── Directions ───
    // 0=east, 1=west, 2=south, 3=north
    // Yaw mapping: yaw 0→2, 1→1, 2→3, 3→0

    // ─── 8-octant ray trace exclusion table ───
    // field_150150_a[8][2]: per direction+flip, octants to exclude
    static constexpr int32_t OCTANT_TABLE[8][2] = {
        {2, 6}, {3, 7}, {2, 3}, {6, 7},
        {0, 4}, {1, 5}, {0, 1}, {4, 5}
    };

    // ─── Octant bounds ───
    // Octant i: x=[0.5*(i%2), 0.5+0.5*(i%2)]
    //           y=[0.5*((i/2)%2), 0.5+0.5*((i/2)%2)]
    //           z=[0.5*((i/4)%2), 0.5+0.5*((i/4)%2)]
}

// ═══════════════════════════════════════════════════════════════════════════
// Fence Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FenceConstants {
    // ─── Block IDs ───
    static constexpr int32_t FENCE_ID = 85;
    static constexpr int32_t NETHER_FENCE_ID = 113;
    static constexpr int32_t FENCE_GATE_ID = 107;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 11;

    // ─── Center post ───
    static constexpr float POST_MIN = 0.375f;           // 6/16
    static constexpr float POST_MAX = 0.625f;           // 10/16

    // ─── Collision height ───
    static constexpr float VISUAL_HEIGHT = 1.0f;
    static constexpr float COLLISION_HEIGHT = 1.5f;

    // ─── Connection ───
    // Connects to: self, fence_gate, opaque+normalBlock (not gourd)
}

// ═══════════════════════════════════════════════════════════════════════════
// Wall Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace WallConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 139;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 32;

    // ─── Variants ───
    static constexpr int32_t NORMAL = 0;
    static constexpr int32_t MOSSY = 1;

    // ─── Center post ───
    static constexpr float POST_MIN = 0.25f;            // 4/16
    static constexpr float POST_MAX = 0.75f;            // 12/16

    // ─── Straight wall (N-S or E-W only) ───
    static constexpr float STRAIGHT_HEIGHT = 0.8125f;   // 13/16
    static constexpr float STRAIGHT_MIN = 0.3125f;      // 5/16
    static constexpr float STRAIGHT_MAX = 0.6875f;      // 11/16

    // ─── Collision height ───
    static constexpr float VISUAL_HEIGHT = 1.0f;
    static constexpr float COLLISION_HEIGHT = 1.5f;

    // ─── Connection ───
    // Same as fence: self, fence_gate, opaque+normalBlock (not gourd)
}

} // namespace mccpp
