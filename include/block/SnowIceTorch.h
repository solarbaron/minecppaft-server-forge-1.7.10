/**
 * SnowIceTorch.h — Environmental and light-source blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockSnow (120 lines) — snow layers
 *   - net.minecraft.block.BlockSnowBlock (42 lines) — snow block
 *   - net.minecraft.block.BlockIce (74 lines) — ice
 *   - net.minecraft.block.BlockPackedIce (25 lines) — packed ice
 *   - net.minecraft.block.BlockTorch (182 lines) — torch
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SNOW LAYER (BlockSnow)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: snow, setTickRandomly(true)
 * Not opaque, not normal, collision = layer-based
 *
 * Metadata: 0-7 = layer count (0 = 1 layer, 7 = 8 layers = full block)
 *
 * Height formula: (2 * (1 + meta)) / 16.0
 *   meta 0: 2/16 (0.125)
 *   meta 7: 16/16 (1.0, full block)
 *
 * Collision: full XZ, 0 to meta*0.125 Y
 *   (Note: collision height = meta * 0.125, NOT bounds height)
 *
 * Placement:
 *   Cannot place on ice or packed_ice
 *   Can place on leaves
 *   Can stack on snow with meta==7 (full)
 *   Can place on opaque + blocksMovement
 *
 * Melting: block light > 11 → drop + air
 *
 * Harvest: drops (meta+1) snowballs
 * Normal drop: quantityDropped = 0 (handled by harvestBlock)
 *
 * Block ID: snow_layer(78)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SNOW BLOCK (BlockSnowBlock)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: craftedSnow, setTickRandomly(true)
 * Full opaque block
 *
 * Drops: 4 snowballs
 * Melting: block light > 11 → drop + air
 *
 * Block ID: snow(80)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ICE (BlockIce)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: ice, extends BlockBreakable("ice")
 * Slipperiness: 0.98 (reduced friction for entities)
 * setTickRandomly(true)
 *
 * Harvest:
 *   Silk Touch → drop ice block
 *   No Silk Touch:
 *     Nether → air (no water)
 *     Else → flowing_water (if below has blocksMovement or isLiquid)
 *
 * Melting: block light > (11 - lightOpacity) → same as harvest
 * Mobility: 0 (normal)
 * quantityDropped: 0 (handled by harvestBlock)
 *
 * Block ID: ice(79)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PACKED ICE (BlockPackedIce)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: packedIce
 * Slipperiness: 0.98
 * Does NOT melt (no updateTick, no tickRandomly)
 * quantityDropped: 0 (only silk touch yields item)
 *
 * Block ID: packed_ice(174)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TORCH (BlockTorch)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, no collision (null), render type 2
 * Not opaque, not normal, setTickRandomly(true)
 *
 * Metadata:
 *   1: wall -X (attached to X+ face)
 *   2: wall +X (attached to X- face)
 *   3: wall -Z (attached to Z+ face)
 *   4: wall +Z (attached to Z- face)
 *   5: floor (standing on top)
 *   0: uninitialized (auto-detect in onBlockAdded)
 *
 * Placement:
 *   Wall: requires isBlockNormalCubeDefault on attached face
 *   Floor: canPlaceTorchOn:
 *     doesBlockHaveSolidTopSurface
 *     OR fence/nether_brick_fence/glass/cobblestone_wall
 *
 * Placement face → meta:
 *   clicked 1(top) → 5(floor)
 *   clicked 2(N)   → 4(wall Z+)
 *   clicked 3(S)   → 3(wall Z-)
 *   clicked 4(W)   → 2(wall X+)
 *   clicked 5(E)   → 1(wall X-)
 *
 * Ray trace bounds (for selection box):
 *   Wall 1(-X): 0 to 0.3, 0.2-0.8 Y, center ±0.15 Z
 *   Wall 2(+X): 0.7 to 1.0, 0.2-0.8 Y, center ±0.15 Z
 *   Wall 3(-Z): center ±0.15 X, 0.2-0.8 Y, 0 to 0.3
 *   Wall 4(+Z): center ±0.15 X, 0.2-0.8 Y, 0.7 to 1.0
 *   Floor 5: center ±0.1 XZ, 0.0-0.6 Y
 *
 * Support validation: dropTorchIfCantStay
 *   On neighbor change: check if attached face still valid
 *   If not → drop + air
 *
 * Block IDs: torch(50), note: redstone_torch handled separately
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Weather/environment events for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Snow Layer Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SnowLayerConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 78;

    // ─── Metadata ───
    static constexpr int32_t MAX_LAYERS = 7;           // 0-7, 8 layers
    static constexpr int32_t LAYER_MASK = 7;

    // ─── Height formula ───
    // height = (2 * (1 + meta)) / 16.0
    inline float layerHeight(int32_t meta) {
        return static_cast<float>(2 * (1 + (meta & LAYER_MASK))) / 16.0f;
    }

    // ─── Collision ───
    // collision top = meta * 0.125
    static constexpr float COLLISION_STEP = 0.125f;

    // ─── Melting ───
    static constexpr int32_t MELT_LIGHT_THRESHOLD = 11;

    // ─── Drops ───
    static constexpr int32_t SNOWBALL_ITEM_ID = 332;
    // harvest drops: meta + 1 snowballs

    // ─── Placement exclusions ───
    static constexpr int32_t ICE_ID = 79;
    static constexpr int32_t PACKED_ICE_ID = 174;
}

// ═══════════════════════════════════════════════════════════════════════════
// Snow Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SnowBlockConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 80;

    // ─── Drops ───
    static constexpr int32_t SNOWBALL_COUNT = 4;
    static constexpr int32_t SNOWBALL_ITEM_ID = 332;

    // ─── Melting ───
    static constexpr int32_t MELT_LIGHT_THRESHOLD = 11;
}

// ═══════════════════════════════════════════════════════════════════════════
// Ice Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace IceConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 79;

    // ─── Physics ───
    static constexpr float SLIPPERINESS = 0.98f;

    // ─── Melting ───
    // Threshold: block light > 11 - lightOpacity
    static constexpr int32_t BASE_MELT_THRESHOLD = 11;

    // ─── On break ───
    static constexpr int32_t FLOWING_WATER_ID = 8;
    // Nether: air. Else: flowing_water if below blocksMovement or isLiquid

    // ─── Mobility ───
    static constexpr int32_t MOBILITY = 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Packed Ice Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PackedIceConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 174;

    // ─── Physics ───
    static constexpr float SLIPPERINESS = 0.98f;

    // Does NOT melt
    // quantityDropped = 0 (silk touch only)
}

// ═══════════════════════════════════════════════════════════════════════════
// Torch Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TorchConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 50;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 2;

    // ─── Metadata ───
    static constexpr int32_t WALL_NEG_X = 1;   // attached to +X face
    static constexpr int32_t WALL_POS_X = 2;   // attached to -X face
    static constexpr int32_t WALL_NEG_Z = 3;   // attached to +Z face
    static constexpr int32_t WALL_POS_Z = 4;   // attached to -Z face
    static constexpr int32_t FLOOR = 5;         // standing on top
    static constexpr int32_t UNINITIALIZED = 0;

    // ─── Ray trace bounds ───
    static constexpr float WALL_SIZE = 0.15f;
    static constexpr float WALL_Y_MIN = 0.2f;
    static constexpr float WALL_Y_MAX = 0.8f;
    static constexpr float FLOOR_SIZE = 0.1f;
    static constexpr float FLOOR_HEIGHT = 0.6f;

    // ─── Special placement targets ───
    static constexpr int32_t FENCE_ID = 85;
    static constexpr int32_t NETHER_FENCE_ID = 113;
    static constexpr int32_t GLASS_ID = 20;
    static constexpr int32_t COBBLE_WALL_ID = 139;
}

} // namespace mccpp
