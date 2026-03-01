/**
 * SnowIceLeaves.h — Snow layer stacking, ice formation/melting, and leaf decay.
 *
 * Java references:
 *   - net.minecraft.block.BlockSnow (120 lines)
 *   - net.minecraft.block.BlockIce (74 lines)
 *   - net.minecraft.block.BlockLeaves (182 lines)
 *   - net.minecraft.block.BlockOldLeaf (38 lines)
 *   - net.minecraft.block.BlockNewLeaf (30 lines)
 *   - net.minecraft.block.BlockLeavesBase (24 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SNOW LAYER (BlockSnow)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: snow, random tick enabled
 *
 * Layer system (metadata 0-7):
 *   - Each meta value = 1 layer
 *   - Height = 2 * (1 + meta) / 16 blocks
 *   - meta 0 = 2/16 = 0.125 blocks
 *   - meta 7 = 16/16 = 1.0 full block
 *
 * Collision box: full XZ, height = meta * 0.125
 *   (Note: collision differs from visual bounds)
 *
 * Placement rules:
 *   - Cannot place on ice or packed ice
 *   - Can place on leaves, opaque+movementBlocking blocks
 *   - Can place on top of full-height snow (meta 7)
 *
 * Melting (updateTick):
 *   Java: if (savedLightValue(Block_type, x, y, z) > 11)
 *     → drop as item, set to air
 *
 * Harvest: drops (meta+1) snowballs, NOT from regular break
 *   Regular Item drop returns snowball but quantityDropped = 0
 *   (harvesting is handled specially via harvestBlock)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ICE BLOCK (BlockIce)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: ice, slipperiness: 0.98, random tick enabled
 * Extends BlockBreakable (translucent)
 *
 * Melting (updateTick):
 *   Java: if (savedLightValue(Block_type, x, y, z) > 11 - getLightOpacity())
 *     - In Nether: → air (no water)
 *     - Overworld: drop item, → water (Blocks.water, still)
 *
 * Harvest:
 *   - Silk Touch: drops ice block
 *   - Otherwise:
 *     - In Nether: → air (evaporates)
 *     - Normal: drop nothing, → flowing_water if block below is
 *       solid (blocksMovement) or liquid (isLiquid)
 *   - Always adds exhaustion 0.025f
 *
 * Mobility: 0 (can be pushed by pistons)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LEAF DECAY (BlockLeaves)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: leaves, hardness 0.2, lightOpacity 1, random tick
 *
 * Metadata layout:
 *   bits 0-1: tree type (0=oak, 1=spruce, 2=birch, 3=jungle)
 *   bit 2: player-placed flag (prevents decay)
 *   bit 3: "needs decay check" flag (set when nearby log is broken)
 *
 * Break propagation (breakBlock):
 *   When a leaf block is broken, mark all leaves within ±1 radius
 *   with bit 3 (needs check), if they don't have bit 2 (player-placed)
 *
 * Decay algorithm (updateTick — BFS flood fill):
 *   Triggered when bit 3 is set AND bit 2 is NOT set
 *
 *   1. Allocate 32×32×32 grid (32768 cells)
 *   2. Scan ±4 radius around leaf block:
 *      - Log (Blocks.log or Blocks.log2) → value 0 (source)
 *      - Leaves → value -2 (unvisited)
 *      - Other → value -1 (barrier)
 *   3. BFS: 4 iterations (distance 1 to 4):
 *      For each cell with value (iteration-1):
 *        Flood fill 6 neighbors (±X, ±Y, ±Z)
 *        If neighbor is -2 → set to current iteration
 *   4. Check center cell:
 *      - value >= 0: supported (within 4 of log) → clear bit 3
 *      - value < 0: unsupported → decay (drop items, set air)
 *
 * Drop rates:
 *   - Sapling: 1/20 base chance (1/40 for jungle)
 *   - Fortune: reduces denominator by 2<<fortune (min 10)
 *   - Apple (oak only): 1/200 base (fortune: reduces by 10<<fortune, min 40)
 *
 * Shears harvest: drops leaf block itself (preserving tree type metadata)
 *
 * Thread safety: Block updates on server thread. BFS grid is instance-level
 * in Java (field_150128_a), needs per-call allocation in multi-threaded C++.
 * JNI readiness: Simple constants and BFS algorithm.
 */
#pragma once

#include <cstdint>
#include <array>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Snow Layer Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SnowLayerConstants {
    // ─── Block ID ───
    static constexpr int32_t SNOW_LAYER_ID = 78;
    static constexpr int32_t SNOW_BLOCK_ID = 80;

    // ─── Layer metadata ───
    static constexpr int32_t META_MASK = 7;
    static constexpr int32_t MIN_LAYERS = 0;
    static constexpr int32_t MAX_LAYERS = 7;
    static constexpr int32_t NUM_LAYERS = 8;

    // ─── Height calculation ───
    // Java: float f = (float)(2 * (1 + meta)) / 16.0f
    inline float getHeight(int32_t meta) {
        return static_cast<float>(2 * (1 + (meta & META_MASK))) / 16.0f;
    }

    // ─── Collision height ───
    // Java: (float)meta * 0.125f (collision, not visual)
    static constexpr float LAYER_HEIGHT = 0.125f;
    inline float getCollisionHeight(int32_t meta) {
        return static_cast<float>(meta & META_MASK) * LAYER_HEIGHT;
    }

    // ─── Placement constraints ───
    static constexpr int32_t ICE_ID = 79;
    static constexpr int32_t PACKED_ICE_ID = 174;

    // ─── Melting ───
    // Java: savedLightValue(Block, x, y, z) > 11
    static constexpr int32_t MELT_LIGHT_THRESHOLD = 11;

    // ─── Drops ───
    static constexpr int32_t SNOWBALL_ITEM_ID = 332;
    // Harvest drops: meta + 1 snowballs
    // Regular break: quantityDropped = 0

    // ─── Default bounds ───
    static constexpr float DEFAULT_HEIGHT = 0.125f;  // 2/16
}

// ═══════════════════════════════════════════════════════════════════════════
// Ice Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace IceConstants {
    // ─── Block IDs ───
    static constexpr int32_t ICE_ID = 79;
    static constexpr int32_t PACKED_ICE_ID = 174;  // doesn't melt
    static constexpr int32_t WATER_ID = 9;          // Blocks.water (still)
    static constexpr int32_t FLOWING_WATER_ID = 8;  // Blocks.flowing_water

    // ─── Slipperiness ───
    // Java: this.slipperiness = 0.98f  (default for other blocks is 0.6f)
    static constexpr float SLIPPERINESS = 0.98f;
    static constexpr float DEFAULT_SLIPPERINESS = 0.6f;

    // ─── Melting ───
    // Java: savedLightValue(Block, x,y,z) > 11 - getLightOpacity()
    // For ice: lightOpacity = 3 (from parent), so threshold = 11 - 3 = 8
    static constexpr int32_t MELT_BASE = 11;
    static constexpr int32_t ICE_LIGHT_OPACITY = 3;
    inline int32_t getMeltThreshold(int32_t lightOpacity) {
        return MELT_BASE - lightOpacity;
    }

    // ─── Harvest ───
    // Java: entityPlayer.addExhaustion(0.025f)
    static constexpr float HARVEST_EXHAUSTION = 0.025f;

    // ─── Mobility ───
    static constexpr int32_t MOBILITY_FLAG = 0;  // pushable by pistons
}

// ═══════════════════════════════════════════════════════════════════════════
// Leaf Decay Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LeafConstants {
    // ─── Block IDs ───
    static constexpr int32_t LEAVES_ID = 18;      // Old leaves (oak/spruce/birch/jungle)
    static constexpr int32_t LEAVES2_ID = 161;     // New leaves (acacia/dark oak)
    static constexpr int32_t LOG_ID = 17;          // Old logs
    static constexpr int32_t LOG2_ID = 162;        // New logs
    static constexpr int32_t SAPLING_ID = 6;

    // ─── Metadata layout ───
    static constexpr int32_t TYPE_MASK = 3;          // bits 0-1: tree type
    static constexpr int32_t PLAYER_PLACED_FLAG = 4;  // bit 2: no decay
    static constexpr int32_t NEEDS_CHECK_FLAG = 8;    // bit 3: pending check

    inline int32_t getTreeType(int32_t meta) { return meta & TYPE_MASK; }
    inline bool isPlayerPlaced(int32_t meta) { return (meta & PLAYER_PLACED_FLAG) != 0; }
    inline bool needsDecayCheck(int32_t meta) { return (meta & NEEDS_CHECK_FLAG) != 0; }

    // ─── Tree types ───
    static constexpr int32_t TREE_OAK = 0;
    static constexpr int32_t TREE_SPRUCE = 1;
    static constexpr int32_t TREE_BIRCH = 2;
    static constexpr int32_t TREE_JUNGLE = 3;
    // For Leaves2:
    static constexpr int32_t TREE_ACACIA = 0;
    static constexpr int32_t TREE_DARK_OAK = 1;

    // ─── Block properties ───
    static constexpr float HARDNESS = 0.2f;
    static constexpr int32_t LIGHT_OPACITY = 1;

    // ─── BFS decay algorithm ───
    // Java: 32×32×32 grid, ±4 scan radius, 4 BFS iterations
    static constexpr int32_t GRID_SIZE = 32;
    static constexpr int32_t GRID_TOTAL = GRID_SIZE * GRID_SIZE * GRID_SIZE;  // 32768
    static constexpr int32_t GRID_HALF = GRID_SIZE / 2;  // 16 = center offset
    static constexpr int32_t SCAN_RADIUS = 4;
    static constexpr int32_t CHUNK_CHECK_RADIUS = SCAN_RADIUS + 1;  // 5
    static constexpr int32_t BFS_ITERATIONS = 4;

    // Grid cell values:
    static constexpr int32_t CELL_LOG = 0;          // Source (log block)
    static constexpr int32_t CELL_UNVISITED = -2;   // Leaf (not yet reached)
    static constexpr int32_t CELL_BARRIER = -1;      // Non-leaf non-log (air, etc)

    // Grid index calculation:
    // Java: (x + 16) * 1024 + (y + 16) * 32 + (z + 16)
    inline int32_t gridIndex(int32_t dx, int32_t dy, int32_t dz) {
        return (dx + GRID_HALF) * (GRID_SIZE * GRID_SIZE)
             + (dy + GRID_HALF) * GRID_SIZE
             + (dz + GRID_HALF);
    }

    // ─── Break propagation ───
    // When leaf broken: mark leaves within ±1 with NEEDS_CHECK_FLAG
    static constexpr int32_t BREAK_PROPAGATION_RADIUS = 1;

    // ─── Drop rates ───
    // Base sapling drop chance: 1/20
    static constexpr int32_t BASE_SAPLING_RATE = 20;
    // Jungle: 1/40
    static constexpr int32_t JUNGLE_SAPLING_RATE = 40;

    // Fortune modifier:
    // Java: rate -= 2 << fortune; if (rate < 10) rate = 10;
    static constexpr int32_t FORTUNE_SHIFT = 2;       // 2 << fortune
    static constexpr int32_t MIN_SAPLING_RATE = 10;

    // Apple drop (oak only):
    // Base: 1/200
    static constexpr int32_t BASE_APPLE_RATE = 200;
    // Fortune modifier:
    // Java: rate -= 10 << fortune; if (rate < 40) rate = 40;
    static constexpr int32_t APPLE_FORTUNE_SHIFT = 10; // 10 << fortune
    static constexpr int32_t MIN_APPLE_RATE = 40;

    // Apple item ID
    static constexpr int32_t APPLE_ITEM_ID = 260;
}

// ═══════════════════════════════════════════════════════════════════════════
// Leaf BFS Grid (thread-safe per-call allocation)
// ═══════════════════════════════════════════════════════════════════════════

// In Java, field_150128_a is a reusable instance-level int[32768].
// For multi-threaded C++, each decay check uses a stack-allocated grid.

struct LeafDecayGrid {
    std::array<int32_t, LeafConstants::GRID_TOTAL> cells;

    void clear() {
        cells.fill(LeafConstants::CELL_BARRIER);
    }

    int32_t& at(int32_t dx, int32_t dy, int32_t dz) {
        return cells[LeafConstants::gridIndex(dx, dy, dz)];
    }

    const int32_t& at(int32_t dx, int32_t dy, int32_t dz) const {
        return cells[LeafConstants::gridIndex(dx, dy, dz)];
    }

    // Java: center = field_150128_a[16 * 1024 + 16 * 32 + 16]
    int32_t centerValue() const {
        return cells[LeafConstants::gridIndex(0, 0, 0)];
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Packed Ice Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PackedIceConstants {
    // Java: BlockPackedIce — does NOT melt, no random tick
    // Same slipperiness as ice
    static constexpr int32_t BLOCK_ID = 174;
    static constexpr float SLIPPERINESS = 0.98f;
    // Does not naturally generate water when broken
    // quantityDropped = 0 without silk touch
}

} // namespace mccpp
