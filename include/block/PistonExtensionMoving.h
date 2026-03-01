/**
 * PistonExtensionMoving.h — Piston head and animating piston block.
 *
 * Java references:
 *   - net.minecraft.block.BlockPistonExtension (180 lines)
 *   - net.minecraft.block.BlockPistonMoving (178 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PISTON EXTENSION / HEAD (BlockPistonExtension)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: piston, hardness 0.5, step sound piston
 * Not opaque, not normal, render type 17
 * Cannot be placed by player (canPlaceBlockAt = false)
 * No drops (quantityDropped = 0)
 *
 * Metadata: bits 0-2 = direction (0-5), bit 3 = sticky flag
 *   getDirectionMeta(meta) = clamp(meta & 7, 0, 5)
 *
 * Compound collision per direction (2 boxes):
 *   Face plate (4/16 thick full face) + arm rod (4/16 × 4/16)
 *
 *   Dir 0 (down):  plate (0,0,0)→(1,0.25,1)      arm (0.375,0.25,0.375)→(0.625,1,0.625)
 *   Dir 1 (up):    plate (0,0.75,0)→(1,1,1)       arm (0.375,0,0.375)→(0.625,0.75,0.625)
 *   Dir 2 (north): plate (0,0,0)→(1,1,0.25)       arm (0.25,0.375,0.25)→(0.75,0.625,1)
 *   Dir 3 (south): plate (0,0,0.75)→(1,1,1)       arm (0.25,0.375,0)→(0.75,0.625,0.75)
 *   Dir 4 (west):  plate (0,0,0)→(0.25,1,1)       arm (0.375,0.25,0.25)→(0.625,0.75,1)
 *   Dir 5 (east):  plate (0.75,0,0)→(1,1,1)       arm (0,0.375,0.25)→(0.75,0.625,0.75)
 *
 * Render bounds (setBlockBoundsBasedOnState) — face plate only:
 *   Dir 0: (0,0,0)→(1,0.25,1)
 *   Dir 1: (0,0.75,0)→(1,1,1)
 *   Dir 2: (0,0,0)→(1,1,0.25)
 *   Dir 3: (0,0,0.75)→(1,1,1)
 *   Dir 4: (0,0,0)→(0.25,1,1)
 *   Dir 5: (0.75,0,0)→(1,1,1)
 *
 * breakBlock: looks behind at piston base, drops and removes if extended
 * onBlockHarvested (creative): silently removes piston base behind
 * onNeighborBlockChange: validates piston base exists behind
 *
 * Block IDs: piston_head (34)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PISTON MOVING (BlockPistonMoving)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: piston, hardness -1.0 (indestructible by non-creative)
 * Extends BlockContainer (TileEntityPiston)
 * createNewTileEntity returns null (created via static getTileEntity)
 * Render type -1 (invisible — rendered by TileEntityPistonRenderer)
 * Not opaque, not normal
 * Cannot be placed by player
 * No item drops
 *
 * Animated collision:
 *   TileEntityPiston stores: storedBlock, metadata, orientation,
 *   extending flag, progress (0→1 over 2 ticks)
 *
 *   func_145860_a(partialTick) → interpolated progress
 *   Collision AABB offset: storedBlock's AABB - Facing.offset * progress
 *   If extending: offset = 1 - progress (pushing away)
 *   If retracting: offset = progress (pulling in)
 *
 *   func_149964_a: adjusts AABB per-axis:
 *     Negative facing offset → adjusts min
 *     Positive facing offset → adjusts max
 *
 * dropBlockAsItemWithChance: stored block drops itself
 * onBlockActivated: if no tile entity → setBlockToAir
 * breakBlock: TileEntityPiston.clearPistonTileEntity()
 *
 * Static getTileEntity(block, meta, orientation, extending, renderHead):
 *   Creates TileEntityPiston with parameters
 *
 * Block IDs: piston_extension (36)
 *
 * Thread safety: Piston movement on server thread only.
 * JNI readiness: TileEntityPiston accessible for Forge.
 */
#pragma once

#include <cstdint>
#include <algorithm>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Piston Extension (Head) Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PistonExtensionConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 34;

    // ─── Related IDs ───
    static constexpr int32_t PISTON_ID = 33;
    static constexpr int32_t STICKY_PISTON_ID = 29;

    // ─── Metadata ───
    static constexpr int32_t DIRECTION_MASK = 7;    // bits 0-2
    static constexpr int32_t STICKY_FLAG = 8;        // bit 3
    static constexpr int32_t MAX_DIRECTION = 5;

    inline int32_t getDirectionMeta(int32_t meta) {
        int32_t dir = meta & DIRECTION_MASK;
        return std::clamp(dir, 0, MAX_DIRECTION);
    }

    inline bool isSticky(int32_t meta) {
        return (meta & STICKY_FLAG) != 0;
    }

    // ─── Dimensions ───
    static constexpr float PLATE_THICKNESS = 0.25f;   // 4/16
    static constexpr float ARM_MIN = 0.375f;           // 6/16
    static constexpr float ARM_MAX = 0.625f;           // 10/16

    // ─── Compound collision boxes per direction ───
    struct CollisionPair {
        struct Box { float x0, y0, z0, x1, y1, z1; };
        Box plate;
        Box arm;
    };

    static constexpr CollisionPair COLLISION[6] = {
        // Dir 0 (down)
        {{{0,0,0, 1,0.25f,1}}, {{0.375f,0.25f,0.375f, 0.625f,1,0.625f}}},
        // Dir 1 (up)
        {{{0,0.75f,0, 1,1,1}}, {{0.375f,0,0.375f, 0.625f,0.75f,0.625f}}},
        // Dir 2 (north)
        {{{0,0,0, 1,1,0.25f}}, {{0.25f,0.375f,0.25f, 0.75f,0.625f,1}}},
        // Dir 3 (south)
        {{{0,0,0.75f, 1,1,1}}, {{0.25f,0.375f,0, 0.75f,0.625f,0.75f}}},
        // Dir 4 (west)
        {{{0,0,0, 0.25f,1,1}}, {{0.375f,0.25f,0.25f, 0.625f,0.75f,1}}},
        // Dir 5 (east)
        {{{0.75f,0,0, 1,1,1}}, {{0,0.375f,0.25f, 0.75f,0.625f,0.75f}}}
    };

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 17;

    // ─── Drops ───
    static constexpr int32_t DROP_QUANTITY = 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Piston Moving (Animating) Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PistonMovingConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 36;

    // ─── Properties ───
    static constexpr float HARDNESS = -1.0f;  // indestructible
    static constexpr int32_t RENDER_TYPE = -1;  // invisible (TE renderer)

    // ─── Animation ───
    // TileEntityPiston progress: 0.0 → 1.0 over 2 ticks
    // lastProgress and progress updated each tick
    // Interpolation: lastProgress + (progress - lastProgress) * partialTick
    static constexpr float PROGRESS_PER_TICK = 0.5f;  // 2 ticks total
    static constexpr float PROGRESS_COMPLETE = 1.0f;

    // ─── Collision offset formula ───
    // If extending: offset = 1.0 - progress (block moves away from piston)
    // If retracting: offset = progress (block moves toward piston)
    // AABB adjustment: storedBlock.AABB - Facing.offset[direction] * computedOffset
    // Per-axis: negative facing → adjust min, positive facing → adjust max

    // ─── Facing offsets (from net.minecraft.util.Facing) ───
    static constexpr int32_t FACING_OFFSETS_X[6] = {0, 0, 0, 0, -1, 1};
    static constexpr int32_t FACING_OFFSETS_Y[6] = {-1, 1, 0, 0, 0, 0};
    static constexpr int32_t FACING_OFFSETS_Z[6] = {0, 0, -1, 1, 0, 0};
    static constexpr int32_t FACING_OPPOSITE[6] = {1, 0, 3, 2, 5, 4};
}

} // namespace mccpp
