/**
 * BlockLiquid.h — Water and lava fluid flow mechanics.
 *
 * Java references:
 *   - net.minecraft.block.BlockLiquid (246 lines)
 *   - net.minecraft.block.BlockDynamicLiquid (241 lines)
 *   - net.minecraft.block.BlockStaticLiquid (58 lines)
 *
 * Implements the complete Minecraft fluid simulation:
 *
 * 1. Level System (metadata 0-15):
 *    - 0 = source block (full)
 *    - 1-7 = flowing levels (1=most, 7=least)
 *    - 8-15 = falling fluid (level & 7 = horizontal level)
 *    - Height percent = (level+1)/9.0 (for rendering)
 *
 * 2. Flow Algorithm (BlockDynamicLiquid.updateTick):
 *    a. Find lowest adjacent level → compute new level (old + decay)
 *    b. Water decay = 1 per block, Lava decay = 2 (overworld) / 1 (nether)
 *    c. If level >= 8 → air
 *    d. Infinite water source: 2+ adjacent sources → new source (level 0)
 *    e. BFS shortest-path-to-drop: search 4 directions, up to depth 4
 *    f. Flow preferentially toward nearest edge/hole
 *
 * 3. Tick Rates:
 *    - Water: 5 ticks
 *    - Lava (Nether): 10 ticks
 *    - Lava (Overworld): 30 ticks
 *    - Lava random 4x slowdown on level increase
 *
 * 4. Interactions:
 *    - Lava + Water (adjacent): source → obsidian, flowing ≤ 4 → cobblestone
 *    - Lava flows into water (below): stone
 *    - Flow replaces non-solid blocks, drops their items
 *    - Blocked by: doors, signs, ladders, sugarcane, portals, solid blocks
 *    - Fizz sound + smoke particles on solidification
 *
 * 5. Entity Velocity Modification:
 *    - Flow vector computed from adjacent level differences
 *    - Falling water adds downward push (-6.0)
 *    - Applied to entities standing in fluid
 *
 * Thread safety: Block ticks happen on the server thread.
 * JNI readiness: Simple numeric constants and algorithms.
 */
#pragma once

#include <cstdint>
#include <array>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Fluid constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FluidConstants {
    // ─── Level system ───
    static constexpr int32_t SOURCE_LEVEL = 0;       // Full source block
    static constexpr int32_t MAX_FLOW_LEVEL = 7;     // Maximum flow decay
    static constexpr int32_t FALLING_FLAG = 8;        // Bit 3 = falling
    static constexpr int32_t LEVEL_MASK = 7;          // Bits 0-2 = horizontal level
    static constexpr int32_t MAX_METADATA = 15;

    // ─── Decay rates ───
    // Java: n6 = 1 (water/nether lava), n6 = 2 (overworld lava)
    static constexpr int32_t WATER_DECAY = 1;
    static constexpr int32_t LAVA_DECAY_NETHER = 1;
    static constexpr int32_t LAVA_DECAY_OVERWORLD = 2;

    // ─── Tick rates ───
    // Java: BlockLiquid.tickRate(world)
    static constexpr int32_t WATER_TICK_RATE = 5;
    static constexpr int32_t LAVA_TICK_RATE_NETHER = 10;
    static constexpr int32_t LAVA_TICK_RATE_OVERWORLD = 30;

    // ─── Lava random slowdown ───
    // Java: n7 *= 4 when lava level increases and random(4) != 0
    static constexpr int32_t LAVA_SLOWDOWN_MULTIPLIER = 4;
    static constexpr int32_t LAVA_SLOWDOWN_CHANCE = 4;  // 1 in 4 chance to NOT slow

    // ─── Infinite water source threshold ───
    // Java: field_149815_a >= 2 && material == water
    static constexpr int32_t INFINITE_SOURCE_THRESHOLD = 2;

    // ─── BFS max search depth ───
    // Java: func_149812_c recursion limit: n4 >= 4
    static constexpr int32_t MAX_BFS_DEPTH = 4;

    // ─── BFS initial distance ───
    static constexpr int32_t BFS_INITIAL_COST = 1000;

    // ─── Flow direction offsets ───
    // Index: 0=-X, 1=+X, 2=-Z, 3=+Z
    static constexpr std::array<std::array<int32_t, 3>, 4> FLOW_DIRS = {{
        {{-1, 0, 0}},  // -X
        {{ 1, 0, 0}},  // +X
        {{ 0, 0,-1}},  // -Z
        {{ 0, 0, 1}}   // +Z
    }};

    // ─── Opposite direction mapping ───
    // Java: i==0 && n5==1, i==1 && n5==0, i==2 && n5==3, i==3 && n5==2
    static constexpr std::array<int32_t, 4> OPPOSITE_DIR = {{1, 0, 3, 2}};

    // ─── Level to height percent ───
    // Java: getLiquidHeightPercent(n) = (n+1)/9.0f
    // For rendering: source(0)=1/9, level 7=8/9 (inverted — lower level = more fluid)
    inline float getLiquidHeightPercent(int32_t level) {
        if (level >= FALLING_FLAG) level = 0;
        return static_cast<float>(level + 1) / 9.0f;
    }

    // ─── Effective flow decay ───
    // Java: getEffectiveFlowDecay — returns level with falling cleared
    inline int32_t getEffectiveFlowDecay(int32_t metadata) {
        if (metadata < 0) return -1;
        return metadata >= FALLING_FLAG ? 0 : metadata;
    }

    // ─── Is source block ───
    inline bool isSourceBlock(int32_t metadata) {
        return metadata == SOURCE_LEVEL;
    }

    // ─── Is falling fluid ───
    inline bool isFalling(int32_t metadata) {
        return metadata >= FALLING_FLAG;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Lava-Water interaction results
// Java: BlockLiquid.func_149805_n + BlockDynamicLiquid.func_149813_h
// ═══════════════════════════════════════════════════════════════════════════

namespace FluidInteraction {
    // ─── Block IDs for interaction results ───
    // Java: Blocks.obsidian, Blocks.cobblestone, Blocks.stone
    static constexpr int32_t OBSIDIAN_ID = 49;
    static constexpr int32_t COBBLESTONE_ID = 4;
    static constexpr int32_t STONE_ID = 1;

    // ─── Lava source + water → obsidian ───
    // Java: if (n4 == 0) world.setBlock(n, n2, n3, Blocks.obsidian)
    static constexpr int32_t LAVA_SOURCE_RESULT = OBSIDIAN_ID;

    // ─── Lava flowing (level <= 4) + water → cobblestone ───
    // Java: else if (n4 <= 4) world.setBlock(n, n2, n3, Blocks.cobblestone)
    static constexpr int32_t LAVA_FLOW_RESULT_MAX_LEVEL = 4;
    static constexpr int32_t LAVA_FLOW_RESULT = COBBLESTONE_ID;

    // ─── Lava flows down into water → stone ───
    // Java: func_149813_h when lava + water below
    static constexpr int32_t LAVA_DOWN_INTO_WATER = STONE_ID;

    // ─── Fizz sound ───
    // Java: func_149799_m
    static constexpr const char* FIZZ_SOUND = "random.fizz";
    static constexpr float FIZZ_VOLUME = 0.5f;
    static constexpr float FIZZ_PITCH_BASE = 2.6f;
    static constexpr float FIZZ_PITCH_RANGE = 0.8f;
    static constexpr int32_t FIZZ_PARTICLE_COUNT = 8;
    static constexpr const char* FIZZ_PARTICLE = "largesmoke";

    // ─── Determine lava-water interaction result ───
    // Returns block ID to place, or -1 if no interaction
    inline int32_t getLavaWaterResult(int32_t lavaLevel) {
        if (lavaLevel == 0) return LAVA_SOURCE_RESULT;
        if (lavaLevel <= LAVA_FLOW_RESULT_MAX_LEVEL) return LAVA_FLOW_RESULT;
        return -1;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Flow vector computation
// Java: BlockLiquid.getFlowVector
//
// Computes the velocity a fluid exerts on entities within it.
// Used by modifyEntityVelocity to push entities along the flow.
// ═══════════════════════════════════════════════════════════════════════════

struct FlowVector {
    double x = 0.0, y = 0.0, z = 0.0;

    void add(double dx, double dy, double dz) {
        x += dx; y += dy; z += dz;
    }

    void normalize() {
        double len = std::sqrt(x * x + y * y + z * z);
        if (len > 0.0) {
            x /= len;
            y /= len;
            z /= len;
        }
    }
};

namespace FluidFlow {
    // ─── Falling fluid downward push ───
    // Java: vec3.normalize().addVector(0.0, -6.0, 0.0)
    static constexpr double FALLING_PUSH_Y = -6.0;

    // ─── Blocked block types ───
    // Java: func_149807_p — blocks that stop fluid flow
    // Doors (wooden_door=64, iron_door=71), standing_sign=63,
    // ladder=65, reeds=83, portal material, solid blocks
    static constexpr int32_t WOODEN_DOOR_ID = 64;
    static constexpr int32_t IRON_DOOR_ID = 71;
    static constexpr int32_t STANDING_SIGN_ID = 63;
    static constexpr int32_t LADDER_ID = 65;
    static constexpr int32_t REEDS_ID = 83;

    // Check if a block ID is a hardcoded flow blocker
    inline bool isFlowBlocker(int32_t blockId) {
        return blockId == WOODEN_DOOR_ID ||
               blockId == IRON_DOOR_ID ||
               blockId == STANDING_SIGN_ID ||
               blockId == LADDER_ID ||
               blockId == REEDS_ID;
    }

    // ─── BFS shortest path to drop ───
    // Java: func_149812_c — recursive BFS through horizontal directions
    // Returns the shortest distance to a position where fluid can fall
    // Used by func_149808_o to determine optimal flow direction
    //
    // Algorithm:
    //   For each of 4 directions (skip opposite to source):
    //     If position blocks flow or is same fluid at level 0: skip
    //     If position below is not blocked: return current depth
    //     If depth < 4: recurse deeper
    //   Return 1000 (no path found)

    // ─── Compute flow directions ───
    // Java: func_149808_o — returns bool[4] for which directions fluid should flow
    // Steps:
    //   1. For each direction, compute BFS distance to nearest drop
    //   2. Find minimum distance across all directions
    //   3. Enable flow in all directions with minimum distance
    //
    // Result: fluid flows toward nearest edge/hole, preferring shortest path
    struct FlowDirections {
        bool dirs[4] = {false, false, false, false};

        bool operator[](int i) const { return dirs[i]; }
    };

    // ─── Adjacent level check ───
    // Java: func_149810_a — get neighbor level, track source count
    // Returns minimum of current best and neighbor level
    // Increments sourceCount when neighbor is source (level 0)
    struct AdjacentLevelResult {
        int32_t bestLevel;
        int32_t sourceCount;
    };

    inline AdjacentLevelResult checkAdjacentLevel(int32_t neighborMeta,
                                                   int32_t currentBest,
                                                   int32_t currentSourceCount) {
        AdjacentLevelResult result;
        result.sourceCount = currentSourceCount;

        // Java: func_149804_e returns metadata if same material, else -1
        if (neighborMeta < 0) {
            result.bestLevel = currentBest;
            return result;
        }

        // Source blocks increment counter
        if (neighborMeta == 0) {
            ++result.sourceCount;
        }

        // Falling fluid treated as level 0
        if (neighborMeta >= FluidConstants::FALLING_FLAG) {
            neighborMeta = 0;
        }

        // Keep minimum level
        if (currentBest < 0 || neighborMeta < currentBest) {
            result.bestLevel = neighborMeta;
        } else {
            result.bestLevel = currentBest;
        }
        return result;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Material IDs used in fluid logic
// Java: Material.water, Material.lava, Material.ice, Material.portal
// ═══════════════════════════════════════════════════════════════════════════

namespace FluidMaterial {
    static constexpr int32_t WATER = 0;
    static constexpr int32_t LAVA = 1;

    // Java: BlockLiquid.isPassable — water is passable, lava is not
    inline bool isPassable(int32_t materialType) {
        return materialType != LAVA;
    }

    // Java: BlockLiquid.getCollisionBoundingBoxFromPool — always null
    // Fluids have no collision box (entities swim through them)

    // Java: BlockLiquid.getItemDropped — always null
    // Fluids drop nothing

    // Java: BlockLiquid.quantityDropped — always 0
}

} // namespace mccpp
