/**
 * EndPortalBlocks.h — End Portal block and End Portal Frame with Eye of Ender.
 *
 * Java references:
 *   - net.minecraft.block.BlockEndPortal (85 lines)
 *   - net.minecraft.block.BlockEndPortalFrame (81 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * END PORTAL BLOCK (BlockEndPortal)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: portal (custom), light level: 1.0 (max, 15)
 * Extends BlockContainer (has TileEntityEndPortal)
 * Bounds: full XZ, 1/16 height (0, 0, 0 → 1, 0.0625, 1)
 * No collision boxes (entities fall through)
 * Not opaque, not normal, render type -1 (TESR only)
 * Drops nothing (quantityDropped = 0)
 * MapColor: obsidian (dark purple)
 *
 * Entity teleportation:
 *   Java: entity.ridingEntity == null && entity.riddenByEntity == null
 *         && !world.isRemote
 *     → entity.travelToDimension(1)  (The End, dimension ID 1)
 *   Only solo entities can teleport (no passengers/riders)
 *
 * field_149948_a (static boolean):
 *   Dragon death portal generation bypass — when true, portal blocks
 *   can be placed freely. When false, portal blocks auto-remove if
 *   placed outside the overworld (dimensionId != 0).
 *
 * onBlockAdded:
 *   - If field_149948_a: skip (dragon is creating portal)
 *   - If not in overworld (dim != 0): setBlockToAir
 *
 * ═══════════════════════════════════════════════════════════════════════
 * END PORTAL FRAME (BlockEndPortalFrame)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: rock
 * Not opaque, render type 26
 * No item drops (getItemDropped returns null)
 *
 * Metadata layout:
 *   bits 0-1: facing direction (0-3)
 *   bit 2: Eye of Ender inserted flag
 *
 * Facing (onBlockPlacedBy):
 *   Java: ((floor(yaw * 4 / 360 + 0.5) & 3) + 2) % 4
 *   This rotates the standard yaw mapping by 180° (faces center)
 *
 * Collision boxes (compound):
 *   Base: (0, 0, 0) → (1, 0.8125, 1) — 13/16 height
 *   Eye bump (when inserted): (0.3125, 0.8125, 0.3125) → (0.6875, 1.0, 0.6875)
 *     — 6/16 wide, 3/16 tall, centered on top
 *
 * Comparator output:
 *   - Eye inserted: 15 (full signal)
 *   - No eye: 0
 *
 * Thread safety: Block/tile entity on server thread.
 * JNI readiness: Simple constants and metadata layout.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// End Portal Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EndPortalConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 119;

    // ─── Light ───
    // Java: setLightLevel(1.0f) → lightValue = 15
    static constexpr float LIGHT_LEVEL = 1.0f;
    static constexpr int32_t LIGHT_VALUE = 15;

    // ─── Bounds ───
    // Java: (0, 0, 0) → (1, 0.0625, 1) — 1/16 height
    static constexpr float BOUNDS_HEIGHT = 0.0625f;

    // ─── Teleportation ───
    // Destination dimension: 1 (The End)
    static constexpr int32_t TARGET_DIMENSION = 1;

    // Requirements:
    // - entity.ridingEntity == null (not riding anything)
    // - entity.riddenByEntity == null (nobody riding entity)
    // - !world.isRemote (server only)

    // ─── Placement rules ───
    // Auto-remove if placed outside overworld (dim != 0)
    // Unless field_149948_a is true (dragon death bypass)
    static constexpr int32_t OVERWORLD_DIMENSION = 0;

    // ─── Rendering ───
    static constexpr int32_t RENDER_TYPE = -1;  // TESR only

    // ─── Map color ───
    // obsidianColor (dark purple/black)
}

// ═══════════════════════════════════════════════════════════════════════════
// End Portal Frame Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EndPortalFrameConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 120;

    // ─── Metadata layout ───
    static constexpr int32_t FACING_MASK = 3;         // bits 0-1
    static constexpr int32_t EYE_INSERTED_FLAG = 4;    // bit 2

    // Java: isEnderEyeInserted(meta) → (meta & 4) != 0
    inline bool isEyeInserted(int32_t meta) {
        return (meta & EYE_INSERTED_FLAG) != 0;
    }

    inline int32_t getFacing(int32_t meta) {
        return meta & FACING_MASK;
    }

    // ─── Facing from yaw ───
    // Java: ((floor(yaw * 4 / 360 + 0.5) & 3) + 2) % 4
    // The +2 % 4 rotates facing 180° to face inward toward portal center
    inline int32_t getFacingFromYaw(float yaw) {
        int32_t raw = static_cast<int32_t>(std::floor(yaw * 4.0f / 360.0f + 0.5f)) & 3;
        return (raw + 2) % 4;
    }

    // ─── Collision boxes ───
    // Base block: (0, 0, 0) → (1, 0.8125, 1) — 13/16 height
    static constexpr float BASE_HEIGHT = 0.8125f;  // 13/16

    // Eye bump (when inserted):
    // (0.3125, 0.8125, 0.3125) → (0.6875, 1.0, 0.6875)
    static constexpr float EYE_MIN_XZ = 0.3125f;   // 5/16
    static constexpr float EYE_MAX_XZ = 0.6875f;   // 11/16
    static constexpr float EYE_MIN_Y = 0.8125f;    // 13/16 (on top of base)
    static constexpr float EYE_MAX_Y = 1.0f;       // full height
    // Eye bump size: 6/16 wide, 3/16 tall

    // ─── Comparator output ───
    // Java: hasComparatorInputOverride() = true
    // Eye inserted → 15, no eye → 0
    static constexpr int32_t COMPARATOR_WITH_EYE = 15;
    static constexpr int32_t COMPARATOR_NO_EYE = 0;

    // ─── Item render bounds ───
    // (0, 0, 0) → (1, 0.8125, 1) — same as base
    static constexpr float ITEM_RENDER_HEIGHT = 0.8125f;

    // ─── Rendering ───
    static constexpr int32_t RENDER_TYPE = 26;

    // ─── Drops ───
    // getItemDropped returns null — frame is not obtainable
}

// ═══════════════════════════════════════════════════════════════════════════
// End Portal Formation
// ═══════════════════════════════════════════════════════════════════════════

namespace EndPortalFormation {
    // The stronghold end portal room consists of 12 frame blocks
    // arranged in a 5×5 ring (corners missing).
    //
    // Layout (F = frame, . = empty, P = portal when active):
    //
    //   . F F F .
    //   F P P P F
    //   F P P P F
    //   F P P P F
    //   . F F F .
    //
    // Each frame faces INWARD toward the center.
    // All 12 frames must have Eye of Ender inserted to activate.

    static constexpr int32_t FRAME_COUNT = 12;
    static constexpr int32_t PORTAL_SIZE = 3;  // 3×3 portal area

    // Frame positions relative to center (north edge, then clockwise):
    // North (facing south, meta 0): Z-2, X = -1, 0, +1
    // East  (facing west, meta 3):  X+2, Z = -1, 0, +1
    // South (facing north, meta 2): Z+2, X = -1, 0, +1
    // West  (facing east, meta 1):  X-2, Z = -1, 0, +1
}

} // namespace mccpp
