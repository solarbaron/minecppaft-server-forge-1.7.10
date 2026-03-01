/**
 * StairsSlabs.h — Stair compound collision and slab half-block placement.
 *
 * Java references:
 *   - net.minecraft.block.BlockStairs (397 lines)
 *   - net.minecraft.block.BlockSlab (102 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * STAIRS (BlockStairs)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Inherits material/hardness/resistance/step sound from model block.
 * Resistance is model's resistance / 3.
 * Light opacity: 255 (full)
 * Not opaque, not normal, render type 10
 *
 * Metadata layout:
 *   bits 0-1: facing direction
 *     0 = east  (+X ascending)
 *     1 = west  (-X ascending)
 *     2 = south (+Z ascending)
 *     3 = north (-Z ascending)
 *   bit 2: upside-down flag (0=bottom, 1=top)
 *
 * Placement facing (onBlockPlacedBy):
 *   Java: floor(yaw * 4 / 360 + 0.5) & 3
 *   yaw 0→meta 2, yaw 1→meta 1, yaw 2→meta 3, yaw 3→meta 0
 *
 * Upside-down (onBlockPlaced):
 *   If placed from below (side 0) OR (side != 1 AND hitY > 0.5):
 *     → set bit 2 (upside-down)
 *
 * Collision system (compound boxes):
 *   Up to 3 collision boxes depending on neighbor stairs:
 *   1. Base slab: full XZ, half Y (top or bottom based on bit 2)
 *   2. Step block: half XZ offset by facing, other half Y
 *   3. Corner piece: if inner/outer corner detected
 *
 * Octant system (field_150150_a):
 *   8 sub-cubes addressed by 3-bit index:
 *     bit 0 (X): 0=min, 1=max half
 *     bit 1 (Y): 0=bottom, 1=top half
 *     bit 2 (Z): 0=min, 1=max half
 *   Bounds: (0.5*(bit), 0.5*(bit)) → (0.5+0.5*(bit), 0.5+0.5*(bit))
 *
 * Corner detection:
 *   func_150145_f: inner corner — step extends into neighbor
 *   func_150144_g: outer corner — step cuts away for neighbor
 *   Both check adjacent stair block type, matching upside-down flag,
 *   and perpendicular facing direction.
 *
 * Ray trace: tests all 8 octants, returns best hit based on distance
 *   field_150150_a[facing + (upside_down ? 4 : 0)] defines which
 *   octants to skip (those covered by the step geometry)
 *
 * Model block delegation:
 *   All non-geometry behavior forwarded to model block:
 *   onBlockClicked, onBlockDestroyedByPlayer, getExplosionResistance,
 *   tickRate, modifyEntityVelocity, isCollidable, canPlaceBlockAt,
 *   onBlockAdded, breakBlock, onEntityWalking, updateTick,
 *   onBlockActivated, onBlockDestroyedByExplosion, getMapColor
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SLABS (BlockSlab)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Abstract base for all slab types (stone, wood, etc.)
 * Light opacity: 255
 *
 * isFullBlock flag:
 *   true = double slab (full block, drops 2)
 *   false = half slab (0.5 height, drops 1)
 *
 * Metadata layout:
 *   bits 0-2: slab type (material variant)
 *   bit 3: upper/lower placement (0=bottom, 1=top)
 *   (For double slabs, all bits are type)
 *
 * Slab types (stone slab, meta bits 0-2):
 *   0 = stone slab
 *   1 = sandstone slab
 *   2 = wood slab (old)
 *   3 = cobblestone slab
 *   4 = brick slab
 *   5 = stone brick slab
 *   6 = nether brick slab
 *   7 = quartz slab
 *
 * Wood slab types (meta bits 0-2):
 *   0 = oak, 1 = spruce, 2 = birch, 3 = jungle
 *   4 = acacia, 5 = dark oak
 *
 * Bounds based on state:
 *   Full: (0, 0, 0) → (1, 1, 1)
 *   Bottom (bit 3 clear): (0, 0, 0) → (1, 0.5, 1)
 *   Top (bit 3 set): (0, 0.5, 0) → (1, 1, 1)
 *
 * Placement (onBlockPlaced):
 *   Same as stairs upside-down logic:
 *   side 0 OR (side != 1 AND hitY > 0.5) → set bit 3 (top)
 *
 * Drops:
 *   Double slab: quantityDropped = 2, damageDropped = meta & 7
 *   Half slab: quantityDropped = 1, damageDropped = meta & 7
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Simple metadata layout and model delegation.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Stair Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace StairConstants {
    // ─── Metadata ───
    static constexpr int32_t FACING_MASK = 3;           // bits 0-1
    static constexpr int32_t UPSIDE_DOWN_FLAG = 4;       // bit 2

    // ─── Facing values ───
    static constexpr int32_t FACING_EAST = 0;    // +X ascending
    static constexpr int32_t FACING_WEST = 1;    // -X ascending
    static constexpr int32_t FACING_SOUTH = 2;   // +Z ascending
    static constexpr int32_t FACING_NORTH = 3;   // -Z ascending

    inline bool isUpsideDown(int32_t meta) {
        return (meta & UPSIDE_DOWN_FLAG) != 0;
    }
    inline int32_t getFacing(int32_t meta) {
        return meta & FACING_MASK;
    }

    // ─── Placement: yaw index → facing ───
    // Java: yaw 0→2(south), 1→1(west), 2→3(north), 3→0(east)
    static constexpr int32_t YAW_TO_FACING[] = {2, 1, 3, 0};

    inline int32_t getFacingFromYaw(float yaw) {
        int32_t index = static_cast<int32_t>(std::floor(yaw * 4.0f / 360.0f + 0.5f)) & 3;
        return YAW_TO_FACING[index];
    }

    // ─── Resistance ───
    // Java: model.blockResistance / 3.0f
    static constexpr float RESISTANCE_DIVISOR = 3.0f;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 10;
    static constexpr int32_t LIGHT_OPACITY = 255;

    // ─── Octant system ───
    // 8 sub-cubes indexed by 3-bit key
    struct Octant {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
    };

    inline Octant getSubCube(int32_t index) {
        return {
            0.5f * static_cast<float>(index % 2),
            0.5f * static_cast<float>((index / 2) % 2),
            0.5f * static_cast<float>((index / 4) % 2),
            0.5f + 0.5f * static_cast<float>(index % 2),
            0.5f + 0.5f * static_cast<float>((index / 2) % 2),
            0.5f + 0.5f * static_cast<float>((index / 4) % 2)
        };
    }

    // ─── Ray trace skip octants ───
    // field_150150_a[facing + (upside_down ? 4 : 0)]
    // Octants to skip (covered by step geometry)
    static constexpr int32_t SKIP_OCTANTS[8][2] = {
        {2, 6},  // facing 0 (east), bottom
        {3, 7},  // facing 1 (west), bottom
        {2, 3},  // facing 2 (south), bottom
        {6, 7},  // facing 3 (north), bottom
        {0, 4},  // facing 0, upside-down
        {1, 5},  // facing 1, upside-down
        {0, 1},  // facing 2, upside-down
        {4, 5},  // facing 3, upside-down
    };
}

// ═══════════════════════════════════════════════════════════════════════════
// Slab Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SlabConstants {
    // ─── Metadata ───
    static constexpr int32_t TYPE_MASK = 7;       // bits 0-2: material variant
    static constexpr int32_t UPPER_FLAG = 8;       // bit 3: top placement

    inline bool isUpperSlab(int32_t meta) {
        return (meta & UPPER_FLAG) != 0;
    }
    inline int32_t getSlabType(int32_t meta) {
        return meta & TYPE_MASK;
    }

    // ─── Properties ───
    static constexpr int32_t LIGHT_OPACITY = 255;
    static constexpr float HALF_HEIGHT = 0.5f;

    // ─── Bounds ───
    // Bottom: (0, 0, 0) → (1, 0.5, 1)
    // Top:    (0, 0.5, 0) → (1, 1, 1)
    // Full:   (0, 0, 0) → (1, 1, 1)

    // ─── Drops ───
    static constexpr int32_t FULL_DROP_COUNT = 2;   // double slab drops 2
    static constexpr int32_t HALF_DROP_COUNT = 1;    // half slab drops 1

    // ─── Stone slab types (meta 0-7) ───
    static constexpr int32_t STONE_SLAB_ID = 44;
    static constexpr int32_t STONE_DOUBLE_SLAB_ID = 43;

    static constexpr int32_t SLAB_STONE = 0;
    static constexpr int32_t SLAB_SANDSTONE = 1;
    static constexpr int32_t SLAB_OLD_WOOD = 2;
    static constexpr int32_t SLAB_COBBLESTONE = 3;
    static constexpr int32_t SLAB_BRICK = 4;
    static constexpr int32_t SLAB_STONE_BRICK = 5;
    static constexpr int32_t SLAB_NETHER_BRICK = 6;
    static constexpr int32_t SLAB_QUARTZ = 7;

    // ─── Wood slab types (meta 0-5) ───
    static constexpr int32_t WOOD_SLAB_ID = 126;
    static constexpr int32_t WOOD_DOUBLE_SLAB_ID = 125;

    static constexpr int32_t WOOD_OAK = 0;
    static constexpr int32_t WOOD_SPRUCE = 1;
    static constexpr int32_t WOOD_BIRCH = 2;
    static constexpr int32_t WOOD_JUNGLE = 3;
    static constexpr int32_t WOOD_ACACIA = 4;
    static constexpr int32_t WOOD_DARK_OAK = 5;
}

// ═══════════════════════════════════════════════════════════════════════════
// Stair Block IDs (all vanilla stair blocks)
// ═══════════════════════════════════════════════════════════════════════════

namespace StairBlockIds {
    static constexpr int32_t OAK_STAIRS = 53;
    static constexpr int32_t COBBLESTONE_STAIRS = 67;
    static constexpr int32_t BRICK_STAIRS = 108;
    static constexpr int32_t STONE_BRICK_STAIRS = 109;
    static constexpr int32_t NETHER_BRICK_STAIRS = 114;
    static constexpr int32_t SANDSTONE_STAIRS = 128;
    static constexpr int32_t SPRUCE_STAIRS = 134;
    static constexpr int32_t BIRCH_STAIRS = 135;
    static constexpr int32_t JUNGLE_STAIRS = 136;
    static constexpr int32_t QUARTZ_STAIRS = 156;
    static constexpr int32_t ACACIA_STAIRS = 163;
    static constexpr int32_t DARK_OAK_STAIRS = 164;
}

} // namespace mccpp
