/**
 * LogDirtNetherWart.h — Wood logs with leaf decay, dirt variants, nether wart growth.
 *
 * Java references:
 *   - net.minecraft.block.BlockLog (55 lines)
 *   - net.minecraft.block.BlockDirt (44 lines)
 *   - net.minecraft.block.BlockNetherWart (76 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LOG (BlockLog)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood, extends BlockRotatedPillar
 * Hardness: 2.0, step sound: wood
 *
 * Metadata:
 *   bits 0-1: wood type (func_150165_c = meta & 3)
 *     BlockOldLog: 0=oak, 1=spruce, 2=birch, 3=jungle
 *     BlockNewLog: 0=acacia, 1=dark_oak
 *   bits 2-3: axis orientation (from BlockRotatedPillar)
 *     0x0=Y axis, 0x4=X axis, 0x8=Z axis, 0xC=bark on all sides
 *
 * Leaf decay (breakBlock):
 *   When a log is broken:
 *   searchRadius = 4 (checks 5-block radius for chunk existence)
 *   Scans 9×9×9 cube centered on broken log
 *   For each leaf block found:
 *     If bit 3 (0x8) NOT set (not player-placed), set bit 3
 *     This marks leaves for decay check in their random tick
 *
 * Block IDs: log (17), log2 (162)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DIRT (BlockDirt)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: ground
 *
 * Variants (metadata):
 *   0 = dirt (default)
 *   1 = coarse dirt (grassless, does NOT grow grass)
 *   2 = podzol (has side texture variants)
 *
 * Drop behavior:
 *   damageDropped: always 0 (all variants drop regular dirt)
 *   getDamageValue: meta, except coarse (1) → 0
 *   createStackedBlock (silk): coarse (1) → 0 (no way to silk coarse)
 *
 * Block ID: 3
 *
 * ═══════════════════════════════════════════════════════════════════════
 * NETHER WART (BlockNetherWart)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockBush, tick randomly
 * Placement: only on soul_sand
 * No creative tab (null)
 * Render type: 6 (cross pattern)
 *
 * Bounds: full width (0, 0, 0) → (1, 0.25, 1) — 25% tall
 *
 * Growth (updateTick):
 *   Meta 0-2 → chance 1/10 per random tick to advance
 *   Meta 3 = mature (no more growth)
 *
 * Drops (dropBlockAsItemWithChance):
 *   Immature (meta < 3): 1 nether wart
 *   Mature (meta >= 3): 2 + rand(3) = 2-4 nether wart
 *   Fortune: + rand(fortune + 1) extra when mature
 *   Drops as ItemStack (not via getItemDropped)
 *
 * Item: Items.nether_wart (372)
 * Block ID: 115
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Simple metadata.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Log Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LogConstants {
    // ─── Block IDs ───
    static constexpr int32_t LOG_ID = 17;
    static constexpr int32_t LOG2_ID = 162;

    // ─── Metadata ───
    static constexpr int32_t WOOD_TYPE_MASK = 3;   // bits 0-1
    static constexpr int32_t AXIS_MASK = 0xC;       // bits 2-3
    static constexpr int32_t AXIS_Y = 0x0;
    static constexpr int32_t AXIS_X = 0x4;
    static constexpr int32_t AXIS_Z = 0x8;
    static constexpr int32_t AXIS_BARK = 0xC;      // all bark, no top/bottom

    inline int32_t getWoodType(int32_t meta) { return meta & WOOD_TYPE_MASK; }

    // ─── Old log wood types (BlockOldLog, ID 17) ───
    static constexpr int32_t OAK = 0;
    static constexpr int32_t SPRUCE = 1;
    static constexpr int32_t BIRCH = 2;
    static constexpr int32_t JUNGLE = 3;

    // ─── New log wood types (BlockNewLog, ID 162) ───
    static constexpr int32_t ACACIA = 0;
    static constexpr int32_t DARK_OAK = 1;

    // ─── Leaf decay ───
    static constexpr int32_t DECAY_SEARCH_RADIUS = 4;
    static constexpr int32_t DECAY_CHECK_RADIUS = 5;  // DECAY_SEARCH_RADIUS + 1
    // Leaf metadata bit 3 (0x8): set by breakBlock → marks for decay check
    static constexpr int32_t LEAF_DECAY_FLAG = 8;      // bit 3

    // ─── Properties ───
    static constexpr float HARDNESS = 2.0f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Dirt Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DirtConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 3;

    // ─── Variants ───
    static constexpr int32_t DIRT_DEFAULT = 0;
    static constexpr int32_t DIRT_COARSE = 1;   // grassless — never grows grass
    static constexpr int32_t DIRT_PODZOL = 2;
    static constexpr int32_t NUM_VARIANTS = 3;

    static constexpr const char* VARIANT_NAMES[] = {"default", "default", "podzol"};

    // ─── Drop ───
    // All variants drop regular dirt (damage 0)
    // Coarse dirt getDamageValue → 0 (no coarse via pick block)
    // Podzol getDamageValue → 2 (silk touch → podzol)
}

// ═══════════════════════════════════════════════════════════════════════════
// Nether Wart Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace NetherWartConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 115;
    static constexpr int32_t ITEM_ID = 372;

    // ─── Soul sand requirement ───
    static constexpr int32_t SOUL_SAND_ID = 88;

    // ─── Growth ───
    static constexpr int32_t MAX_GROWTH = 3;       // meta 0-3
    static constexpr int32_t GROWTH_CHANCE = 10;    // 1 in 10 per random tick

    // ─── Bounds ───
    static constexpr float HEIGHT = 0.25f;         // 25% tall

    // ─── Drops ───
    static constexpr int32_t DROP_IMMATURE = 1;     // meta < 3
    static constexpr int32_t DROP_MATURE_BASE = 2;   // meta >= 3
    static constexpr int32_t DROP_MATURE_RANDOM = 3;  // 2 + rand(3) = 2-4
    // Fortune: + rand(fortune + 1) when mature

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 6;       // cross pattern
}

} // namespace mccpp
