/**
 * StemSaplingCocoa.h — Stems, saplings, and cocoa beans.
 *
 * Java references:
 *   - net.minecraft.block.BlockStem (187 lines)
 *   - net.minecraft.block.BlockSapling (162 lines)
 *   - net.minecraft.block.BlockCocoa (161 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * STEM (BlockStem)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockBush, implements IGrowable
 * Material: plants, render type 19
 * Bounds: 2/16 wide centered, variable height (meta*2+2)/16
 *
 * field_149877_a: fruit block (pumpkin or melon_block)
 * Placement: only on farmland
 *
 * Growth (updateTick):
 *   Requires light ≥ 9 at Y+1
 *   Same rate formula as BlockCrops (3×3 farmland scan)
 *   Chance: 1/(25/rate + 1)
 *   Meta 0-6: grow stem (+1)
 *   Meta 7 (mature): spawn fruit
 *     Check 4 adjacent for existing fruit → abort
 *     Random direction (0-3)
 *     Target must be air with farmland/dirt/grass below
 *
 * Height: (meta * 2 + 2) / 16.0
 *   meta 0: 2/16, meta 7: 16/16 (full)
 *
 * Bonemeal: +rand(2,5), capped at 7
 *
 * Drops: pumpkin_seeds or melon_seeds
 *   3 attempts, each if rand(15) > meta
 *   getItemDropped returns null (handled by dropBlockAsItemWithChance)
 *
 * Block IDs: pumpkin_stem(104), melon_stem(105)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SAPLING (BlockSapling)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockBush, implements IGrowable
 * Material: plants
 * Bounds: 0.8 wide centered (0.1-0.9 XZ), 0.8 tall
 *
 * 6 types (metadata bits 0-2):
 *   0: oak, 1: spruce, 2: birch, 3: jungle, 4: acacia, 5: roofed_oak
 * Bit 3 (mask 8): growth stage marker
 *
 * Growth (updateTick):
 *   Requires light ≥ 9 at Y+1
 *   1/7 chance per tick → markOrGrowMarked
 *   Two-stage:
 *     Stage 1: set bit 3 (mark for growth)
 *     Stage 2: growTree (generate tree)
 *
 * Tree generators per type:
 *   0 (oak): 1/10 WorldGenBigTree, else WorldGenTrees
 *   1 (spruce): 2×2 → WorldGenMegaPineTree (random type), single → WorldGenTaiga2
 *   2 (birch): WorldGenForest
 *   3 (jungle): 2×2 → WorldGenMegaJungle(10,20,3,3), single → WorldGenTrees(4+rand(7),3,3)
 *   4 (acacia): WorldGenSavannaTree
 *   5 (roofed_oak): 2×2 → WorldGenCanopyTree, single → fail (return)
 *
 * 2×2 mega trees: checks 4 saplings of same type, tries (0,0) then (-1,0) then (0,-1) then (-1,-1)
 *   On failure: restores saplings
 *
 * Bonemeal: shouldFertilize rand < 0.45 (45%)
 *
 * Block ID: sapling(6)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * COCOA (BlockCocoa)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockDirectional, implements IGrowable
 * Material: plants, render type 28
 * setTickRandomly(true)
 *
 * Metadata:
 *   bits 0-1: direction (facing)
 *   bits 2-3: growth stage (0-2)
 *   func_149987_c(meta) = (meta & 0xC) >> 2
 *
 * Growth (updateTick):
 *   1/5 chance, stages 0→1→2
 *   New meta: (stage+1) << 2 | direction
 *
 * Attachment: must be on jungle log (log block, sub-type 3)
 *   Direction lookup via Direction.offsetX/Z
 *
 * Bounds: directional, grows with stage
 *   Width: 4 + stage*2 (4, 6, 8)
 *   Height: 5 + stage*2 (5, 7, 9)
 *   All in /16 scale, positioned against attached log face
 *
 * Drops:
 *   Stage < 2: 1 dye(meta 3) = cocoa bean
 *   Stage >= 2: 3 dye(meta 3) = cocoa beans
 *
 * Bonemeal: canFertilize if stage < 2
 *   fertilize: advance stage by 1
 *
 * Block ID: cocoa(127)
 *
 * Thread safety: Growth on server tick thread.
 * JNI readiness: Growth/bonemeal events for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Stem Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace StemConstants {
    // ─── Block IDs ───
    static constexpr int32_t PUMPKIN_STEM_ID = 104;
    static constexpr int32_t MELON_STEM_ID = 105;

    // ─── Fruit blocks ───
    static constexpr int32_t PUMPKIN_ID = 86;
    static constexpr int32_t MELON_BLOCK_ID = 103;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 19;

    // ─── Bounds ───
    static constexpr float HALF_WIDTH = 0.125f;         // 2/16
    // Height = (meta * 2 + 2) / 16.0

    inline float stemHeight(int32_t meta) {
        return static_cast<float>(meta * 2 + 2) / 16.0f;
    }

    // ─── Growth ───
    static constexpr int32_t MATURE_META = 7;
    static constexpr int32_t MIN_LIGHT = 9;

    // ─── Bonemeal ───
    static constexpr int32_t BONEMEAL_MIN = 2;
    static constexpr int32_t BONEMEAL_MAX = 5;

    // ─── Drops ───
    static constexpr int32_t PUMPKIN_SEEDS_ID = 361;
    static constexpr int32_t MELON_SEEDS_ID = 362;
    static constexpr int32_t SEED_DROP_ATTEMPTS = 3;
    static constexpr int32_t SEED_DROP_CHANCE = 15;     // rand(15) > meta

    // ─── Fruit placement ───
    // farmland_id = 60, dirt_id = 3, grass_id = 2
    static constexpr int32_t FARMLAND_ID = 60;
    static constexpr int32_t DIRT_ID = 3;
    static constexpr int32_t GRASS_ID = 2;
}

// ═══════════════════════════════════════════════════════════════════════════
// Sapling Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SaplingConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 6;

    // ─── Types ───
    static constexpr int32_t OAK = 0;
    static constexpr int32_t SPRUCE = 1;
    static constexpr int32_t BIRCH = 2;
    static constexpr int32_t JUNGLE = 3;
    static constexpr int32_t ACACIA = 4;
    static constexpr int32_t ROOFED_OAK = 5;
    static constexpr int32_t TYPE_COUNT = 6;
    static constexpr int32_t TYPE_MASK = 7;             // bits 0-2

    // ─── Growth stage ───
    static constexpr int32_t GROWTH_FLAG = 8;           // bit 3

    // ─── Bounds ───
    static constexpr float HALF_WIDTH = 0.4f;
    static constexpr float HEIGHT = 0.8f;               // 2 × 0.4

    // ─── Growth chance ───
    static constexpr int32_t GROW_CHANCE = 7;           // 1/7
    static constexpr int32_t MIN_LIGHT = 9;

    // ─── Tree generators ───
    // Oak: 1/10 BigTree, else normal Trees
    static constexpr int32_t BIG_TREE_CHANCE = 10;
    // Jungle single: 4+rand(7) height, log 3, leaves 3
    static constexpr int32_t JUNGLE_MIN_HEIGHT = 4;
    static constexpr int32_t JUNGLE_HEIGHT_RANGE = 7;
    // MegaJungle: minHeight 10, extraHeight 20, log 3, leaves 3

    // ─── Bonemeal ───
    static constexpr float FERTILIZE_CHANCE = 0.45f;    // 45%
}

// ═══════════════════════════════════════════════════════════════════════════
// Cocoa Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CocoaConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 127;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 28;

    // ─── Metadata ───
    static constexpr int32_t DIRECTION_MASK = 3;        // bits 0-1
    static constexpr int32_t STAGE_MASK = 0xC;          // bits 2-3
    static constexpr int32_t STAGE_SHIFT = 2;
    static constexpr int32_t MAX_STAGE = 2;

    inline int32_t getStage(int32_t meta) { return (meta & STAGE_MASK) >> STAGE_SHIFT; }
    inline int32_t getDirection(int32_t meta) { return meta & DIRECTION_MASK; }

    // ─── Growth ───
    static constexpr int32_t GROW_CHANCE = 5;           // 1/5

    // ─── Attachment ───
    static constexpr int32_t LOG_ID = 17;
    static constexpr int32_t JUNGLE_SUBTYPE = 3;

    // ─── Bounds per stage (in /16 units) ───
    // Width: 4, 6, 8  Height: 5, 7, 9
    inline int32_t podWidth(int32_t stage) { return 4 + stage * 2; }
    inline int32_t podHeight(int32_t stage) { return 5 + stage * 2; }

    // ─── Drops ───
    static constexpr int32_t DYE_ITEM_ID = 351;
    static constexpr int32_t COCOA_DYE_META = 3;
    // Stage < 2: 1 bean, stage >= 2: 3 beans
    static constexpr int32_t IMMATURE_DROP = 1;
    static constexpr int32_t MATURE_DROP = 3;
}

} // namespace mccpp
