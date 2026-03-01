/**
 * GrassSnowIce.h — Grass spread, layered snow, and ice melting.
 *
 * Java references:
 *   - net.minecraft.block.BlockGrass (88 lines)
 *   - net.minecraft.block.BlockSnow (120 lines)
 *   - net.minecraft.block.BlockIce (74 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * GRASS (BlockGrass)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: grass, tick randomly, implements IGrowable
 *
 * updateTick — Grass spread/death algorithm:
 *   DEATH: if light above < 4 AND block above opacity > 2
 *     → turn to dirt
 *   SPREAD: if light above >= 9
 *     4 random attempts per tick:
 *     target = (x ± rand(3)-1, y ± rand(5)-3, z ± rand(3)-1)
 *     Conditions for spread:
 *       - Target must be dirt with meta 0 (not coarse/podzol)
 *       - Light above target >= 4
 *       - Block above target opacity <= 2
 *     → set target to grass block
 *
 * Drops: same as dirt
 *
 * Bonemeal (IGrowable):
 *   canFertilize: always true
 *   shouldFertilize: always true
 *   fertilize: 128 random vegetation attempts:
 *     Random walk: x ± rand(3)-1, y adjusted by rand(3)-1*rand(3)/2
 *     Each step: verify grass below, non-solid at target
 *     7/8 chance: spawn tallgrass (meta 1) if canBlockStay
 *     1/8 chance: spawn biome-specific flower via func_150572_a
 *
 * Block ID: 2
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SNOW LAYER (BlockSnow)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: snow, tick randomly
 *
 * 8 layers via metadata (0-7):
 *   Each layer height: (2 * (1 + meta)) / 16.0
 *   Meta 0 = 2/16 = 0.125
 *   Meta 7 = 16/16 = 1.0 (full block)
 *
 * Collision: same width as full block, height = meta * 0.125
 *   (Note: collision uses meta, not meta+1 — thinner than visual)
 *
 * Placement rules:
 *   Cannot place on: ice, packed_ice
 *   Can place on: leaves, opaque+blocksMovement, full snow (meta 7)
 *
 * Melting (updateTick):
 *   If block light value (EnumSkyBlock.Block) > 11 → drop + destroy
 *
 * Drops:
 *   harvestBlock: (meta & 7) + 1 snowballs
 *   getItemDropped: snowball
 *   quantityDropped: 0 (harvestBlock handles drops manually)
 *
 * Neighbor change: validates support, drops if invalid
 *
 * Block ID: 78
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ICE (BlockIce)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: ice, extends BlockBreakable (translucent)
 * Slipperiness: 0.98 (vs default 0.6)
 * Tick randomly
 *
 * harvestBlock:
 *   Silk touch → drops ice block itself
 *   No silk touch:
 *     Nether: air (no water)
 *     Overworld: drops nothing, sets flowing_water if
 *       block below blocksMovement or isLiquid
 *
 * Melting (updateTick):
 *   If block light > 11 - lightOpacity:
 *     Nether → air
 *     Overworld → still water (Blocks.water, not flowing)
 *     Also drops item
 *
 * No drops (quantityDropped = 0)
 * Mobility: 0 (normal)
 *
 * Block IDs: ice (79), packed_ice (174, no melt, no water)
 *
 * Thread safety: Block tick on server thread.
 * JNI readiness: Simple metadata and biome lookups.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Grass Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace GrassConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 2;
    static constexpr int32_t DIRT_ID = 3;

    // ─── Death conditions ───
    static constexpr int32_t DEATH_LIGHT_THRESHOLD = 4;     // light < 4
    static constexpr int32_t DEATH_OPACITY_THRESHOLD = 2;    // opacity > 2

    // ─── Spread conditions ───
    static constexpr int32_t SPREAD_LIGHT_MIN = 9;       // light >= 9 to initiate
    static constexpr int32_t SPREAD_ATTEMPTS = 4;         // per tick
    static constexpr int32_t SPREAD_RANGE_XZ = 3;         // ±1 from center
    static constexpr int32_t SPREAD_RANGE_Y = 5;          // ±2 from center (biased)
    static constexpr int32_t SPREAD_TARGET_LIGHT_MIN = 4; // target needs light >= 4
    static constexpr int32_t SPREAD_TARGET_OPACITY_MAX = 2;
    static constexpr int32_t SPREAD_TARGET_META = 0;       // only on dirt meta 0

    // ─── Bonemeal ───
    static constexpr int32_t BONEMEAL_ITERATIONS = 128;
    static constexpr int32_t BONEMEAL_TALLGRASS_CHANCE = 8; // 7/8 grass, 1/8 flower
    static constexpr int32_t BONEMEAL_TALLGRASS_META = 1;   // tall grass type
}

// ═══════════════════════════════════════════════════════════════════════════
// Snow Layer Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SnowLayerConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 78;

    // ─── Layer mechanics ───
    static constexpr int32_t META_MASK = 7;             // bits 0-2
    static constexpr int32_t MAX_LAYERS = 7;             // meta 0-7 = 1-8 layers

    // Height formula: (2 * (1 + meta)) / 16.0
    inline float layerHeight(int32_t meta) {
        return static_cast<float>(2 * (1 + (meta & META_MASK))) / 16.0f;
    }

    // Collision height: meta * 0.125
    static constexpr float LAYER_COLLISION_STEP = 0.125f;

    // ─── Melting ───
    static constexpr int32_t MELT_LIGHT_THRESHOLD = 11;  // block light > 11

    // ─── Placement ───
    static constexpr int32_t ICE_ID = 79;
    static constexpr int32_t PACKED_ICE_ID = 174;
    // Can't place on ice or packed_ice
    // Can place on: leaves, opaque+blocksMovement, full snow (meta 7)

    // ─── Drops ───
    // harvestBlock: (meta & 7) + 1 snowballs
    static constexpr int32_t SNOWBALL_ID = 332;
}

// ═══════════════════════════════════════════════════════════════════════════
// Ice Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace IceConstants {
    // ─── Block IDs ───
    static constexpr int32_t ICE_ID = 79;
    static constexpr int32_t PACKED_ICE_ID = 174;  // no melt, no water

    // ─── Slipperiness ───
    static constexpr float SLIPPERINESS = 0.98f;   // default is 0.6

    // ─── Melting ───
    // Condition: block light > 11 - lightOpacity
    // Overworld: → still water (Blocks.water)
    // Nether: → air
    static constexpr int32_t MELT_LIGHT_BASE = 11;

    // ─── Harvest ───
    // Silk touch → ice block
    // No silk + overworld: → flowing_water if below blocksMovement or isLiquid
    // No silk + nether: → air
    static constexpr float EXHAUSTION = 0.025f;

    // ─── Water blocks ───
    static constexpr int32_t FLOWING_WATER_ID = 8;
    static constexpr int32_t STILL_WATER_ID = 9;

    // ─── Mobility ───
    static constexpr int32_t MOBILITY_FLAG = 0;     // normal
}

} // namespace mccpp
