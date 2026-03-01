/**
 * ClimbableSurfaceBlocks.h — Ladder, vine, web, and lily pad.
 *
 * Java references:
 *   - net.minecraft.block.BlockLadder (125 lines)
 *   - net.minecraft.block.BlockVine (289 lines)
 *   - net.minecraft.block.BlockWeb (59 lines)
 *   - net.minecraft.block.BlockLilyPad (57 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LADDER (BlockLadder)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, render type 8
 * Not opaque, not normal
 *
 * Bounds: 2/16 thick panel on attached wall
 *   meta 2: Z 0.875-1.0 (south face)
 *   meta 3: Z 0.0-0.125 (north face)
 *   meta 4: X 0.875-1.0 (east face)
 *   meta 5: X 0.0-0.125 (west face)
 *
 * Placement: prefers clicked side, requires normalCube behind
 *   canPlaceBlockAt: any of 4 horizontal neighbors normalCube
 *   onBlockPlaced: side→meta, fallback to first valid
 *
 * Support: drops if normalCube behind removed
 *
 * Block ID: ladder(65)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * VINE (BlockVine)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: vine, render type 20
 * Not opaque, not normal, no collision AABB
 * Tick randomly: true
 *
 * Metadata: bitmask for 4 faces
 *   bit 0: south
 *   bit 1: west
 *   bit 2: north
 *   bit 3: east
 *
 * Placement: side→bit mapping
 *   side 2→bit 0(S), 3→bit 2(N), 4→bit 3(E), 5→bit 1(W)
 *
 * Support check (func_150093_a):
 *   renderAsNormalBlock + blocksMovement
 *
 * Validation (func_150094_e):
 *   Each face: must have solid behind OR vine above with same face
 *   No faces + no solid above → invalid
 *
 * Growth (updateTick, 1/4 chance):
 *   Density limit: max 5 vines in 9×3×9 area
 *   6 growth directions:
 *     Up (1): random subset of faces, only valid faces kept
 *     Horizontal (2-5): corner wrapping spread
 *     Down (default): random subset extends downward
 *   Can spread to air blocks and add faces to existing vines
 *
 * Harvest: shears → drops vine block, otherwise no drop
 *
 * Block ID: vine(106)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * WEB (BlockWeb)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: web, render type 1
 * Not opaque, not normal
 * No collision AABB
 *
 * Entity collision: calls entity.setInWeb()
 * Drop: Items.string
 * Silk harvest: true (drops web block itself)
 *
 * Block ID: web(30)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LILY PAD (BlockLilyPad)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockBush, render type 23
 * Height: 1/64 (0.015625)
 * Full width (0-1 XZ)
 *
 * Has collision (unlike most plants) but boats pass through
 *   addCollisionBoxesToList: skip if entity instanceof EntityBoat
 *
 * Placement: only on still water (material=water, meta=0)
 * canBlockStay: water material below + meta 0, Y in bounds
 *
 * Block ID: waterlily(111)
 *
 * Thread safety: Vine growth on server tick thread.
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

    // ─── Thickness ───
    static constexpr float THICKNESS = 0.125f;           // 2/16

    // ─── Meta → direction ───
    // 2=south (+Z behind), 3=north (-Z behind)
    // 4=east (+X behind), 5=west (-X behind)
}

// ═══════════════════════════════════════════════════════════════════════════
// Vine Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace VineConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 106;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 20;

    // ─── Face bitmask ───
    static constexpr int32_t SOUTH = 1;                  // bit 0
    static constexpr int32_t WEST = 2;                   // bit 1
    static constexpr int32_t NORTH = 4;                  // bit 2
    static constexpr int32_t EAST = 8;                   // bit 3

    // ─── Placement: side → bit ───
    // side 2→1(S), 3→4(N), 4→8(E), 5→2(W)

    // ─── Growth ───
    static constexpr int32_t GROWTH_CHANCE = 4;           // 1 in 4
    static constexpr int32_t DENSITY_RADIUS = 4;          // 9×3×9 area
    static constexpr int32_t MAX_DENSITY = 5;             // max nearby vines

    // ─── Bounds ───
    static constexpr float FACE_OFFSET = 0.0625f;        // 1/16
}

// ═══════════════════════════════════════════════════════════════════════════
// Web Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace WebConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 30;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 1;

    // ─── Drop: Items.string ───
    // Silk harvest: drops web block
}

// ═══════════════════════════════════════════════════════════════════════════
// Lily Pad Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LilyPadConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 111;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 23;

    // ─── Height ───
    static constexpr float HEIGHT = 0.015625f;           // 1/64

    // ─── Placement ───
    // Only on still water (material=water, meta=0)
    // Boats pass through collision
}

} // namespace mccpp
