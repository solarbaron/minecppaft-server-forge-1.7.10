/**
 * DoorTrapdoor.h — Door and trapdoor mechanics.
 *
 * Java references:
 *   - net.minecraft.block.BlockDoor (251 lines)
 *   - net.minecraft.block.BlockTrapDoor (204 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DOOR (BlockDoor)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood or iron, 2-block tall (lower + upper half)
 * Not opaque, not normal, render type 7, mobility 1
 *
 * Metadata (per block):
 *   Lower half (bit 3 = 0):
 *     bits 0-1: facing direction (0-3)
 *     bit 2: open flag
 *     bit 3: 0 (lower)
 *   Upper half (bit 3 = 1):
 *     bit 0: hinge side (0=left, 1=right)
 *     bit 3: 1 (upper)
 *
 * Full metadata (getFullMetadata):
 *   Combines lower + upper into 5-bit value:
 *     bits 0-2: from lower (facing + open)
 *     bit 3: isUpper flag
 *     bit 4: hinge from upper
 *
 * Bounds (3/16 = 0.1875 thick):
 *   4 directions × 2 states (open/closed) × 2 hinge sides = 16 combos
 *   Closed: thin slab on facing edge
 *   Open: thin slab on perpendicular edge (hinge determines which)
 *
 * Interaction:
 *   Iron: no right-click (redstone only)
 *   Wood: right-click toggles open bit (XOR 4), SFX 1003
 *     Updates lower half's metadata (even if clicking upper)
 *
 * Redstone (onNeighborBlockChange):
 *   Lower half handles logic:
 *     1. Validate: upper half exists, solid below
 *     2. If powered (either half) → open, unpowered → close
 *     3. func_150014_a toggles open state + plays SFX
 *   Upper half: if no lower → remove self
 *
 * Placement: needs solid top surface below, y < 255
 * Drops: lower half drops door item (iron_door or wooden_door)
 *   Upper half drops nothing
 * Harvest: creative + upper → removes lower too
 *
 * isPassable: open = true (passable when open)
 *
 * Block IDs: wooden_door(64), iron_door(71)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TRAPDOOR (BlockTrapDoor)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood or iron, single block
 * Not opaque, not normal, render type 0
 *
 * Metadata:
 *   bits 0-1: facing direction (0-3)
 *     0=south face, 1=north face, 2=east face, 3=west face
 *   bit 2: open flag
 *   bit 3: top half flag (attached to top of block)
 *
 * Placement face→meta:
 *   face 2→0, face 3→1, face 4→2, face 5→3
 *   hitY > 0.5 → set bit 3 (top half)
 *
 * Bounds (3/16 = 0.1875 thick):
 *   Closed bottom: y 0 to 3/16
 *   Closed top (bit 3): y (1-3/16) to 1
 *   Open: full height, 3/16 thick on facing edge
 *     dir 0: z face (1-3/16 to 1)
 *     dir 1: z face (0 to 3/16)
 *     dir 2: x face (1-3/16 to 1)
 *     dir 3: x face (0 to 3/16)
 *
 * Interaction:
 *   Iron: no right-click
 *   Wood: toggle bit 2, SFX 1003
 *
 * Support block validation:
 *   Must attach to: opaque+renderAsNormalBlock, OR glowstone,
 *     OR BlockSlab, OR BlockStairs
 *   Checked on neighbor change — drops self if no support
 *
 * Redstone: powered → open, unpowered → close
 * isPassable: closed = true (solid when closed)
 *
 * Block IDs: trapdoor(96), iron_trapdoor (not in 1.7.10)
 *
 * Thread safety: Block interactions on server thread.
 * JNI readiness: Full metadata combining for Forge door events.
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

    // ─── Item IDs ───
    static constexpr int32_t WOODEN_DOOR_ITEM = 324;
    static constexpr int32_t IRON_DOOR_ITEM = 330;

    // ─── Metadata masks ───
    static constexpr int32_t FACING_MASK = 3;     // bits 0-1
    static constexpr int32_t OPEN_FLAG = 4;        // bit 2
    static constexpr int32_t IS_UPPER = 8;          // bit 3
    static constexpr int32_t HINGE_RIGHT = 1;      // bit 0 of upper half

    // ─── Full metadata ───
    static constexpr int32_t FULL_FACING_MASK = 7;  // bits 0-2 from lower
    static constexpr int32_t FULL_UPPER_FLAG = 8;    // bit 3
    static constexpr int32_t FULL_HINGE_FLAG = 16;   // bit 4

    // ─── Bounds ───
    static constexpr float THICKNESS = 0.1875f;    // 3/16

    // ─── Mobility ───
    static constexpr int32_t MOBILITY = 1;  // destroyable by piston

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 7;

    // ─── Sound ───
    static constexpr int32_t SFX_ID = 1003;
}

// ═══════════════════════════════════════════════════════════════════════════
// Trapdoor Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TrapdoorConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 96;

    // ─── Metadata ───
    static constexpr int32_t FACING_MASK = 3;      // bits 0-1
    static constexpr int32_t OPEN_FLAG = 4;         // bit 2
    static constexpr int32_t TOP_HALF = 8;          // bit 3

    // Facing directions:
    //   0 = south (z+), 1 = north (z-), 2 = east (x+), 3 = west (x-)

    // Placement face → meta:
    //   face 2→0, face 3→1, face 4→2, face 5→3
    inline int32_t faceToMeta(int32_t face) {
        switch(face) {
            case 2: return 0;
            case 3: return 1;
            case 4: return 2;
            case 5: return 3;
            default: return 0;
        }
    }

    // ─── Bounds ───
    static constexpr float THICKNESS = 0.1875f;    // 3/16

    // ─── Support validation ───
    // Adjacent support block must be:
    //   opaque + renderAsNormalBlock
    //   OR glowstone (89)
    //   OR BlockSlab instance
    //   OR BlockStairs instance
    static constexpr int32_t GLOWSTONE_ID = 89;

    // Support direction from meta:
    //   meta 0 → z+1, meta 1 → z-1, meta 2 → x+1, meta 3 → x-1
    static constexpr int32_t SUPPORT_OFFSET_X[4] = {0, 0, 1, -1};
    static constexpr int32_t SUPPORT_OFFSET_Z[4] = {1, -1, 0, 0};

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 0;

    // ─── Sound ───
    static constexpr int32_t SFX_ID = 1003;

    // ─── Utility ───
    inline bool isOpen(int32_t meta) { return (meta & OPEN_FLAG) != 0; }
    inline bool isTopHalf(int32_t meta) { return (meta & TOP_HALF) != 0; }
}

} // namespace mccpp
