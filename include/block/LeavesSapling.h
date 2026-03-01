/**
 * LeavesSapling.h — Leaf decay algorithm and sapling tree generation.
 *
 * Java references:
 *   - net.minecraft.block.BlockLeaves (182 lines)
 *   - net.minecraft.block.BlockSapling (162 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LEAVES (BlockLeaves)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: leaves, hardness 0.2, light opacity 1, step sound grass
 * Tick randomly (decay checks), creative tab: decorations
 *
 * Metadata:
 *   bits 0-1: leaf type (0-3 for old, 0-1 for new)
 *   bit 2 (0x4): player-placed flag (prevents decay)
 *   bit 3 (0x8): check-decay flag (set by log breakBlock)
 *
 * DECAY ALGORITHM (updateTick):
 *   Condition: bit 3 set AND bit 2 NOT set
 *
 *   1. Allocate 32×32×32 distance field (field_150128_a)
 *      Index: (x+16)*1024 + (y+16)*32 + (z+16)
 *
 *   2. Scan 9×9×9 cube (radius 4):
 *      - Log blocks → 0 (distance source)
 *      - Leaf blocks → -2 (unvisited leaf)
 *      - Other → -1 (impassable)
 *
 *   3. BFS flood fill, 4 iterations (distance 1→4):
 *      For each cell with value == (iteration-1):
 *        Set all 6 orthogonal neighbors from -2 to iteration value
 *
 *   4. Check center cell [16][16][16]:
 *      Value ≥ 0 → connected to log, clear bit 3 (meta &= ~8)
 *      Value < 0 → not connected, destroy leaf (drop + setBlockToAir)
 *
 * breakBlock: propagates decay check to 3×3×3 neighbors
 *   Sets bit 3 on neighboring leaves that don't have bit 2
 *
 * Drops:
 *   Sapling: base chance 1/20, fortune reduces by 2<<fortune (min 10)
 *   Apple: base chance 1/200, fortune reduces by 10<<fortune (min 40)
 *     (apple drop via func_150124_c, only oak leaves override)
 *   Shears: drops self with meta & 3
 *
 * Block IDs: leaves (18), leaves2 (161)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SAPLING (BlockSapling)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: plants, extends BlockBush, implements IGrowable
 * Bounds: (0.1, 0, 0.1) → (0.9, 0.8, 0.9)
 *
 * 6 types (metadata & 7, clamped 0-5):
 *   0 = oak, 1 = spruce, 2 = birch, 3 = jungle
 *   4 = acacia, 5 = roofed oak (dark oak)
 *
 * Growth (updateTick):
 *   Requires light level ≥ 9 above sapling
 *   1/7 chance per random tick to call markOrGrowMarked
 *
 * markOrGrowMarked:
 *   If bit 3 NOT set: set bit 3 (first stage)
 *   If bit 3 already set: call growTree (generate tree)
 *
 * Bonemeal (IGrowable):
 *   canFertilize: always true
 *   shouldFertilize: 45% chance (rand.nextFloat() < 0.45)
 *   fertilize: calls markOrGrowMarked
 *
 * Tree generation per type:
 *   0 (oak): 1/10 big tree (WorldGenBigTree), else WorldGenTrees
 *   1 (spruce): 2×2 check → WorldGenMegaPineTree, else WorldGenTaiga2
 *   2 (birch): WorldGenForest (birch)
 *   3 (jungle): 2×2 check → WorldGenMegaJungle(10,20,3,3),
 *               else WorldGenTrees(4+rand(7), log=3, leaf=3)
 *   4 (acacia): WorldGenSavannaTree
 *   5 (roofed oak): 2×2 check → WorldGenCanopyTree,
 *                   else fail (single dark oak cannot grow)
 *
 * 2×2 mega tree check (func_149880_a):
 *   Searches offsets (0,0), (-1,0), (0,-1), (-1,-1)
 *   All 4 blocks must be same sapling type
 *   If found: clears all 4 to air, generates mega tree
 *   If generation fails: restores all 4 saplings
 *
 * Block ID: 6
 *
 * Thread safety: Decay field per-block instance; tree gen on server thread.
 * JNI readiness: WorldGen hooks accessible for Forge tree events.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Leaf Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LeafConstants {
    // ─── Block IDs ───
    static constexpr int32_t LEAVES_ID = 18;
    static constexpr int32_t LEAVES2_ID = 161;

    // ─── Properties ───
    static constexpr float HARDNESS = 0.2f;
    static constexpr int32_t LIGHT_OPACITY = 1;

    // ─── Metadata ───
    static constexpr int32_t TYPE_MASK = 3;        // bits 0-1
    static constexpr int32_t PLAYER_PLACED = 4;    // bit 2 — prevents decay
    static constexpr int32_t CHECK_DECAY = 8;      // bit 3 — needs decay check
    static constexpr int32_t CLEAR_DECAY = ~8;     // &= to clear bit 3

    // ─── Decay algorithm ───
    static constexpr int32_t DECAY_RADIUS = 4;
    static constexpr int32_t DECAY_CHECK_RADIUS = 5; // radius + 1
    static constexpr int32_t FIELD_SIZE = 32;
    static constexpr int32_t FIELD_CENTER = 16;       // FIELD_SIZE / 2
    static constexpr int32_t FIELD_STRIDE_Y = 32;     // FIELD_SIZE
    static constexpr int32_t FIELD_STRIDE_X = 1024;   // FIELD_SIZE * FIELD_SIZE
    static constexpr int32_t FIELD_TOTAL = 32768;      // 32^3

    // Field values:
    static constexpr int32_t DIST_LOG = 0;          // log block (source)
    static constexpr int32_t DIST_UNVISITED = -2;   // leaf (not yet reached)
    static constexpr int32_t DIST_IMPASSABLE = -1;  // air/solid (can't traverse)
    static constexpr int32_t MAX_BFS_ITERATIONS = 4; // flood fill 4 steps

    // ─── breakBlock propagation ───
    static constexpr int32_t BREAK_PROPAGATION = 1; // 3×3×3 radius

    // ─── Drop chances ───
    static constexpr int32_t SAPLING_BASE_CHANCE = 20;   // 1 in 20
    static constexpr int32_t SAPLING_FORTUNE_SHIFT = 2;  // subtract 2 << fortune
    static constexpr int32_t SAPLING_MIN_CHANCE = 10;

    static constexpr int32_t APPLE_BASE_CHANCE = 200;    // 1 in 200
    static constexpr int32_t APPLE_FORTUNE_SHIFT = 10;   // subtract 10 << fortune
    static constexpr int32_t APPLE_MIN_CHANCE = 40;
}

// ═══════════════════════════════════════════════════════════════════════════
// Sapling Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SaplingConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 6;

    // ─── Types (meta & 7, clamped 0-5) ───
    static constexpr int32_t OAK = 0;
    static constexpr int32_t SPRUCE = 1;
    static constexpr int32_t BIRCH = 2;
    static constexpr int32_t JUNGLE = 3;
    static constexpr int32_t ACACIA = 4;
    static constexpr int32_t DARK_OAK = 5;
    static constexpr int32_t NUM_TYPES = 6;

    static constexpr const char* TYPE_NAMES[] = {
        "oak", "spruce", "birch", "jungle", "acacia", "roofed_oak"
    };

    // ─── Metadata ───
    static constexpr int32_t TYPE_MASK = 7;        // bits 0-2
    static constexpr int32_t GROWTH_STAGE = 8;     // bit 3 — second stage grows tree

    // ─── Bounds ───
    static constexpr float HALF_WIDTH = 0.4f;
    static constexpr float HEIGHT = 0.8f;

    // ─── Growth requirements ───
    static constexpr int32_t MIN_LIGHT = 9;        // above sapling
    static constexpr int32_t GROWTH_CHANCE = 7;    // 1 in 7 per random tick

    // ─── Bonemeal ───
    static constexpr float BONEMEAL_CHANCE = 0.45f; // 45% success

    // ─── Tree generation ───
    static constexpr int32_t BIG_TREE_CHANCE = 10;  // 1 in 10 for oak

    // Jungle single: trunk height = 4 + rand(7), log ID 3, leaf ID 3
    static constexpr int32_t JUNGLE_SINGLE_MIN_HEIGHT = 4;
    static constexpr int32_t JUNGLE_SINGLE_HEIGHT_RANGE = 7;
    static constexpr int32_t JUNGLE_LOG_META = 3;
    static constexpr int32_t JUNGLE_LEAF_META = 3;

    // Mega jungle: baseHeight=10, heightVariation=20, log=3, leaf=3
    static constexpr int32_t MEGA_JUNGLE_BASE = 10;
    static constexpr int32_t MEGA_JUNGLE_VAR = 20;

    // ─── 2×2 mega tree check offsets ───
    // Searches: (0,0), (-1,0), (0,-1), (-1,-1) for matching saplings
    // All 4 must be same type
}

} // namespace mccpp
