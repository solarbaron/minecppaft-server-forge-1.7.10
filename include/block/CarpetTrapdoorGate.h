/**
 * CarpetTrapdoorGate.h — Carpet, trapdoor, and fence gate blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockCarpet (86 lines)
 *   - net.minecraft.block.BlockTrapDoor (204 lines)
 *   - net.minecraft.block.BlockFenceGate (121 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * CARPET (BlockCarpet)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: carpet
 * Height: 1/16 (0.0625) — thinnest non-air block
 * Full XZ coverage
 * Not opaque, not normal, tick randomly
 *
 * Collision: technically has bbox but height = 0 (boolean false cast)
 * Java: (float)n2 + (float)false * 0.0625f → effectively 0 collision height
 * This is a vanilla bug — carpet has visual height but zero collision
 *
 * Placement: requires non-air block below (canBlockStay)
 * Break: drops if block below removed (onNeighborBlockChange)
 * Metadata: 16 wool colors (0-15), preserved on drop (damageDropped)
 *
 * Block ID: 171
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TRAPDOOR (BlockTrapDoor)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood or iron
 * Thickness: 3/16 (0.1875)
 * Not opaque, not normal, render type 0
 *
 * Metadata layout:
 *   bits 0-1: facing direction (where hinge attaches)
 *     0 = south side (hinge on +Z face)
 *     1 = north side (hinge on -Z face)
 *     2 = east side (hinge on +X face)
 *     3 = west side (hinge on -X face)
 *   bit 2: open flag (0=closed, 1=open)
 *   bit 3: top/bottom flag (0=bottom, 1=top)
 *
 * Bounds when closed:
 *   Bottom (bit 3=0): (0, 0, 0) → (1, 0.1875, 1)
 *   Top (bit 3=1): (0, 0.8125, 0) → (1, 1, 1)
 *
 * Bounds when open:
 *   Facing 0 (south): (0, 0, 0.8125) → (1, 1, 1)
 *   Facing 1 (north): (0, 0, 0) → (1, 1, 0.1875)
 *   Facing 2 (east):  (0.8125, 0, 0) → (1, 1, 1)
 *   Facing 3 (west):  (0, 0, 0) → (0.1875, 1, 1)
 *
 * Passability: passable when open (entities walk through)
 *
 * Interaction:
 *   - Wood: toggles open/close (XOR bit 2), plays sound 1003
 *   - Iron: no hand toggle (redstone only)
 *
 * Placement:
 *   Side 2→meta 0, 3→1, 4→2, 5→3
 *   Cannot place on top (side 0) or bottom (side 1)
 *   If hitY > 0.5 and side != 0,1: set bit 3 (top)
 *
 * Valid support: opaque+normalBlock OR glowstone OR slab OR stairs
 * Support block is at the hinge direction offset
 *
 * Redstone: opens/closes via power (func_150120_a)
 *
 * Block IDs: wooden_trapdoor (96), iron_trapdoor (167)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FENCE GATE (BlockFenceGate)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood
 * Extends BlockDirectional
 * Not opaque, not normal, render type 21
 * Requires solid block below
 *
 * Metadata layout:
 *   bits 0-1: facing direction (0-3, from player yaw)
 *   bit 2: open flag
 *
 * Facing: (floor(yaw * 4 / 360 + 0.5) & 3) % 4
 *
 * Bounds (visual):
 *   N-S (facing 0,2): (0, 0, 0.375) → (1, 1, 0.625) — 4/16 Z
 *   E-W (facing 1,3): (0.375, 0, 0) → (0.625, 1, 1) — 4/16 X
 *
 * Collision:
 *   Open: null (no collision, passable)
 *   Closed N-S: (x, y, z+0.375) → (x+1, y+1.5, z+0.625)
 *   Closed E-W: (x+0.375, y, z) → (x+0.625, y+1.5, z+1)
 *   1.5 height prevents jumping over (same as fence)
 *
 * Opening:
 *   If opening: check if player is facing opposite direction,
 *     if so, flip gate facing to match player (opens toward player)
 *   Formula: if direction == (playerDir + 2) % 4, change to playerDir
 *   Set bit 2 (|= 4)
 *
 * Closing: clear bit 2 (&= ~4, i.e. & 0xFFFFFFFB)
 *
 * Redstone: opens on power, closes without power
 *   Checks isBlockIndirectlyGettingPowered OR canProvidePower
 *   Plays sound 1003 on state change
 *
 * Block ID: 107
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Simple metadata layout.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Carpet Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CarpetConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 171;

    // ─── Dimensions ───
    static constexpr float HEIGHT = 0.0625f;  // 1/16

    // ─── Colors (same as wool) ───
    static constexpr int32_t NUM_COLORS = 16;
    static constexpr int32_t WHITE = 0;
    static constexpr int32_t ORANGE = 1;
    static constexpr int32_t MAGENTA = 2;
    static constexpr int32_t LIGHT_BLUE = 3;
    static constexpr int32_t YELLOW = 4;
    static constexpr int32_t LIME = 5;
    static constexpr int32_t PINK = 6;
    static constexpr int32_t GRAY = 7;
    static constexpr int32_t SILVER = 8;
    static constexpr int32_t CYAN = 9;
    static constexpr int32_t PURPLE = 10;
    static constexpr int32_t BLUE = 11;
    static constexpr int32_t BROWN = 12;
    static constexpr int32_t GREEN = 13;
    static constexpr int32_t RED = 14;
    static constexpr int32_t BLACK = 15;

    // ─── Placement ───
    // Requires non-air block below
    // Drops when support removed
}

// ═══════════════════════════════════════════════════════════════════════════
// Trapdoor Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TrapdoorConstants {
    // ─── Block IDs ───
    static constexpr int32_t WOODEN_ID = 96;
    static constexpr int32_t IRON_ID = 167;

    // ─── Thickness ───
    static constexpr float THICKNESS = 0.1875f;  // 3/16

    // ─── Metadata ───
    static constexpr int32_t FACING_MASK = 3;      // bits 0-1
    static constexpr int32_t OPEN_FLAG = 4;          // bit 2
    static constexpr int32_t TOP_FLAG = 8;           // bit 3

    // ─── Facing values ───
    static constexpr int32_t FACING_SOUTH = 0;  // hinge on +Z
    static constexpr int32_t FACING_NORTH = 1;  // hinge on -Z
    static constexpr int32_t FACING_EAST = 2;   // hinge on +X
    static constexpr int32_t FACING_WEST = 3;   // hinge on -X

    inline bool isOpen(int32_t meta) {
        return (meta & OPEN_FLAG) != 0;
    }
    inline bool isTop(int32_t meta) {
        return (meta & TOP_FLAG) != 0;
    }
    inline int32_t getFacing(int32_t meta) {
        return meta & FACING_MASK;
    }

    // ─── Placement side → facing ───
    // Side 2→0(south), 3→1(north), 4→2(east), 5→3(west)
    static constexpr int32_t SIDE_TO_FACING[] = {-1, -1, 0, 1, 2, 3};

    // ─── Hinge offset (for support check) ───
    struct HingeOffset {
        int32_t dx, dz;
    };
    static constexpr HingeOffset HINGE_OFFSETS[] = {
        { 0, +1},  // facing 0 (south): support at +Z
        { 0, -1},  // facing 1 (north): support at -Z
        {+1,  0},  // facing 2 (east): support at +X
        {-1,  0},  // facing 3 (west): support at -X
    };

    // ─── Valid support blocks ───
    // opaque + normalBlock, OR glowstone (89), OR slab, OR stairs

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 0;

    // ─── Sound ───
    static constexpr int32_t TOGGLE_SOUND = 1003;

    // ─── Item render bounds ───
    // Centered vertically: (0, 0.5-thickness/2, 0) → (1, 0.5+thickness/2, 1)
    static constexpr float ITEM_MIN_Y = 0.5f - THICKNESS / 2.0f;  // 0.40625
    static constexpr float ITEM_MAX_Y = 0.5f + THICKNESS / 2.0f;  // 0.59375
}

// ═══════════════════════════════════════════════════════════════════════════
// Fence Gate Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FenceGateConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 107;

    // ─── Metadata ───
    static constexpr int32_t FACING_MASK = 3;    // bits 0-1
    static constexpr int32_t OPEN_FLAG = 4;       // bit 2
    static constexpr int32_t CLOSE_MASK = ~OPEN_FLAG;  // 0xFFFFFFFB

    inline bool isOpen(int32_t meta) {
        return (meta & OPEN_FLAG) != 0;
    }
    inline int32_t getDirection(int32_t meta) {
        return meta & FACING_MASK;
    }

    // ─── Facing from yaw ───
    inline int32_t getFacingFromYaw(float yaw) {
        return (static_cast<int32_t>(std::floor(yaw * 4.0f / 360.0f + 0.5f)) & 3) % 4;
    }

    // ─── Dimensions ───
    // Bar width: 4/16 (0.25) centered
    static constexpr float BAR_MIN = 0.375f;   // 6/16
    static constexpr float BAR_MAX = 0.625f;   // 10/16

    // ─── Collision height ───
    static constexpr float COLLISION_HEIGHT = 1.5f;  // same as fence

    // ─── Opening logic ───
    // When opening: if gate faces opposite to player (dir == (playerDir+2)%4),
    // gate flips to face player's direction before opening
    // This makes the gate always open toward the player

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 21;

    // ─── Sound ───
    static constexpr int32_t TOGGLE_SOUND = 1003;
}

} // namespace mccpp
