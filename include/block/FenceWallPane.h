/**
 * FenceWallPane.h — Fence, wall, and pane connection-based geometry blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockFence (138 lines)
 *   - net.minecraft.block.BlockWall (105 lines)
 *   - net.minecraft.block.BlockPane (133 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FENCE (BlockFence)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Post: 6/16 wide centered (0.375 to 0.625 on X and Z)
 * Visual height: 1.0 (16/16)
 * Collision height: 1.5 (24/16) — prevents jumping over
 * Not opaque, not normal, not passable, render type 11
 *
 * Connection rules (canConnectFenceTo):
 *   - Connect to same block type (fence)
 *   - Connect to fence_gate
 *   - Connect to opaque + renderAsNormalBlock blocks
 *     EXCEPT gourd material (pumpkin, melon)
 *
 * Collision geometry:
 *   Up to 2 compound boxes:
 *   1. Z-axis bar: if connecting north OR south, extend Z to 0/1
 *   2. X-axis bar: if connecting west OR east, extend X to 0/1
 *      Also if NO connections at all (standalone post)
 *   All bars: full Y from 0 to 1.5
 *   Then reset bounds to visual 1.0 height
 *
 * Special: right-click attaches leads (ItemLead.func_150909_a)
 *
 * Block IDs: fence (85), nether_brick_fence (113)
 * isFence: only checks these two specific blocks
 *
 * ═══════════════════════════════════════════════════════════════════════
 * WALL (BlockWall)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Post: 8/16 wide centered (0.25 to 0.75 on X and Z)
 * Visual height: 1.0
 * Collision height: 1.5 (maxY override)
 * Resistance: model block / 3
 * Not opaque, not normal, not passable, render type 32
 *
 * Connection rules (canConnectWallTo):
 *   Same as fence: self, fence_gate, opaque+normal (not gourd)
 *
 * Post height reduction:
 *   When wall is straight (no cross/T/L):
 *   - N+S only (no E/W): height → 0.8125 (13/16), X narrows to 5/16
 *   - E+W only (no N/S): height → 0.8125 (13/16), Z narrows to 5/16
 *   Gives straight walls a lower profile without pillar
 *
 * 2 types (metadata): 0=cobblestone, 1=mossy cobblestone
 * damageDropped = metadata (preserves type)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PANE (BlockPane) — Glass Panes and Iron Bars
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Center strip: 2/16 wide (0.4375 to 0.5625)
 * Full height: 1.0
 * Not opaque, not normal, silk harvestable
 * Render type: glass=41, iron bars=18
 *
 * Connection rules (canPaneConnectToBlock):
 *   - Full blocks (isFullBlock)
 *   - Same block type (this)
 *   - Glass (glass block, stained glass, stained glass pane)
 *   - Any BlockPane instance (iron bars connects to glass panes)
 *
 * Collision geometry:
 *   X-axis bar: centered on Z (0.4375-0.5625)
 *     - Both E+W or no connections: full X (0-1)
 *     - Only W: X 0→0.5
 *     - Only E: X 0.5→1
 *   Z-axis bar: centered on X (0.4375-0.5625)
 *     - Both N+S or no connections: full Z (0-1)
 *     - Only N: Z 0→0.5
 *     - Only S: Z 0.5→1
 *
 * canDrop (field_150099_b):
 *   true = drops item (iron bars)
 *   false = drops nothing on break (glass pane)
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Simple connection checks.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Fence Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FenceConstants {
    // ─── Block IDs ───
    static constexpr int32_t OAK_FENCE_ID = 85;
    static constexpr int32_t NETHER_BRICK_FENCE_ID = 113;
    static constexpr int32_t FENCE_GATE_ID = 107;

    // ─── Post dimensions ───
    // 6/16 wide centered:
    static constexpr float POST_MIN = 0.375f;   // 6/16
    static constexpr float POST_MAX = 0.625f;   // 10/16

    // ─── Heights ───
    static constexpr float VISUAL_HEIGHT = 1.0f;
    static constexpr float COLLISION_HEIGHT = 1.5f;  // prevents jumping

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 11;

    // ─── Connection check ───
    // Connects to: same block, fence_gate, opaque+normalBlock (not gourd)
    // Material.gourd: pumpkin, melon
}

// ═══════════════════════════════════════════════════════════════════════════
// Wall Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace WallConstants {
    // ─── Block ID ───
    static constexpr int32_t COBBLESTONE_WALL_ID = 139;

    // ─── Variants ───
    static constexpr int32_t TYPE_NORMAL = 0;   // cobblestone
    static constexpr int32_t TYPE_MOSSY = 1;    // mossy cobblestone
    static constexpr const char* TYPE_NAMES[] = {"normal", "mossy"};

    // ─── Post dimensions ───
    // 8/16 wide centered:
    static constexpr float POST_MIN = 0.25f;    // 4/16
    static constexpr float POST_MAX = 0.75f;    // 12/16

    // ─── Heights ───
    static constexpr float VISUAL_HEIGHT = 1.0f;
    static constexpr float COLLISION_HEIGHT = 1.5f;

    // ─── Straight wall profile ───
    // When wall is straight (N+S only or E+W only):
    static constexpr float STRAIGHT_HEIGHT = 0.8125f;  // 13/16
    static constexpr float STRAIGHT_NARROW_MIN = 0.3125f;  // 5/16
    static constexpr float STRAIGHT_NARROW_MAX = 0.6875f;  // 11/16

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 32;

    // ─── Resistance ───
    static constexpr float RESISTANCE_DIVISOR = 3.0f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Glass Pane / Iron Bars Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PaneConstants {
    // ─── Block IDs ───
    static constexpr int32_t GLASS_PANE_ID = 102;
    static constexpr int32_t IRON_BARS_ID = 101;
    static constexpr int32_t STAINED_GLASS_PANE_ID = 160;

    // ─── Connection block IDs ───
    static constexpr int32_t GLASS_BLOCK_ID = 20;
    static constexpr int32_t STAINED_GLASS_BLOCK_ID = 95;

    // ─── Center strip dimensions ───
    // 2/16 wide centered:
    static constexpr float CENTER_MIN = 0.4375f;  // 7/16
    static constexpr float CENTER_MAX = 0.5625f;  // 9/16

    // ─── Heights ───
    static constexpr float HEIGHT = 1.0f;

    // ─── Render types ───
    static constexpr int32_t RENDER_GLASS = 41;     // glass pane
    static constexpr int32_t RENDER_IRON_BARS = 18;  // iron bars

    // ─── Connection check ───
    // Connects to:
    //   - Any full block (isFullBlock)
    //   - Same block instance
    //   - Glass block, stained glass, stained glass pane
    //   - Any BlockPane subclass
}

} // namespace mccpp
