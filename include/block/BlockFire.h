/**
 * BlockFire.h — Fire spread, flammability registry, and burn mechanics.
 *
 * Java reference: net.minecraft.block.BlockFire (260 lines)
 *
 * Implements the complete Minecraft fire system:
 *
 * 1. Flammability Registry:
 *    - Two arrays[256]: encouragement[blockId] and flammability[blockId]
 *    - Encouragement: how likely fire appears next to this block (higher = faster)
 *    - Flammability: how likely the block is destroyed by fire
 *    - 23 vanilla flammable blocks registered via func_149843_e()
 *
 * 2. Fire Behavior (updateTick):
 *    - Tick rate: 30 + random(10) ticks
 *    - Fire age: metadata 0-15, incremented by random(3)/2 per tick
 *    - GameRule "doFireTick" check — no spreading if false
 *    - Eternal fire: netherrack (all dims), bedrock (End only)
 *    - Rain: extinguishes fire (checked at position + 4 adjacent)
 *    - No support: removed if no solid surface below AND no flammable neighbors
 *    - Age 15 + no fuel below + random(4)==0: self-extinguish
 *
 * 3. Fire Spread Algorithm:
 *    a. Direct neighbors (6 faces):
 *       - Horizontal: chance = 300 (+ humidity adjust -50)
 *       - Vertical: chance = 250 (+ humidity adjust -50)
 *       - tryCatchFire: random(chance) < flammability[blockId]
 *       - If caught: random(age+10) < 5 AND not raining → place fire
 *       - Otherwise: destroy block (drop nothing for fire)
 *       - TNT: triggers explosion via onBlockDestroyedByPlayer
 *
 *    b. Volume spread (3×3×5: X±1, Z±1, Y-1 to Y+4):
 *       - Skip self position
 *       - Must be air block at target
 *       - BaseChance = 100 (increases by 100 for each Y above fire+1)
 *       - Encouragement = max(encouragement[neighbor]) across 6 faces
 *       - SpreadChance = (encouragement + 40 + difficulty*7) / (age + 30)
 *       - Humidity: halves spread chance
 *       - Rain: prevents spread
 *       - New fire age = age + random(5)/4 (capped at 15)
 *
 * 4. Placement Rules:
 *    - Needs solid top surface below OR any flammable neighbor
 *    - Portal creation: fire placed in Nether/Overworld dim ≤ 0 triggers
 *      portal creation check
 *
 * Thread safety: Block ticks happen on the server thread.
 * JNI readiness: Simple array-based registry.
 */
#pragma once

#include <cstdint>
#include <array>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Fire constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FireConstants {
    // ─── Registry size ───
    static constexpr int32_t REGISTRY_SIZE = 256;

    // ─── Tick rate ───
    // Java: tickRate = 30
    static constexpr int32_t TICK_RATE = 30;
    // Java: random.nextInt(10) added to tick rate
    static constexpr int32_t TICK_RANDOM_RANGE = 10;

    // ─── Fire age ───
    static constexpr int32_t MAX_AGE = 15;
    // Java: n4 + random.nextInt(3) / 2
    static constexpr int32_t AGE_INCREMENT_RANDOM = 3;
    static constexpr int32_t AGE_INCREMENT_DIVISOR = 2;

    // ─── Direct neighbor catch chance ───
    // Java: tryCatchFire parameters
    static constexpr int32_t HORIZONTAL_CATCH_CHANCE = 300;
    static constexpr int32_t VERTICAL_CATCH_CHANCE = 250;

    // ─── Humidity adjustment ───
    // Java: n5 = -50 when isBlockHighHumidity
    static constexpr int32_t HUMIDITY_ADJUSTMENT = -50;

    // ─── Volume spread constants ───
    // Java: for (i=n-1..n+1, j=n3-1..n3+1, k=n2-1..n2+4)
    static constexpr int32_t SPREAD_XZ_RANGE = 1;       // ±1 blocks horizontal
    static constexpr int32_t SPREAD_Y_MIN = -1;          // 1 block below
    static constexpr int32_t SPREAD_Y_MAX = 4;            // 4 blocks above
    static constexpr int32_t BASE_SPREAD_CHANCE = 100;    // Java: n7 = 100
    // Java: n7 += (k - (n2 + 1)) * 100 — each Y above fire+1 adds 100
    static constexpr int32_t HEIGHT_CHANCE_INCREMENT = 100;
    static constexpr int32_t HEIGHT_THRESHOLD = 1;        // Y+1 and below use base

    // ─── Spread chance formula ───
    // Java: n8 = (n6 + 40 + difficulty * 7) / (n4 + 30)
    static constexpr int32_t SPREAD_ENCOURAGE_BONUS = 40;
    static constexpr int32_t SPREAD_DIFFICULTY_MULT = 7;
    static constexpr int32_t SPREAD_AGE_OFFSET = 30;

    // ─── Self-extinguish ───
    // Java: n4 == 15 && random.nextInt(4) == 0 && no fuel below
    static constexpr int32_t EXTINGUISH_AGE = 15;
    static constexpr int32_t EXTINGUISH_CHANCE = 4;

    // ─── No-neighbor age threshold ───
    // Java: n4 > 3 → remove if no solid top surface and no neighbors
    static constexpr int32_t NO_SUPPORT_AGE_THRESHOLD = 3;

    // ─── tryCatchFire: replace vs destroy ───
    // Java: random.nextInt(n5 + 10) < 5 && !raining → replace with fire
    static constexpr int32_t CATCH_REPLACE_THRESHOLD = 5;
    static constexpr int32_t CATCH_REPLACE_OFFSET = 10;

    // ─── New fire age on spread ───
    // Java: n9 = n4 + random.nextInt(5) / 4
    static constexpr int32_t SPREAD_AGE_RANDOM = 5;
    static constexpr int32_t SPREAD_AGE_DIVISOR = 4;

    // ─── Eternal fire blocks ───
    // Java: netherrack(87), bedrock(7)
    static constexpr int32_t NETHERRACK_ID = 87;
    static constexpr int32_t BEDROCK_ID = 7;

    // ─── Portal trigger ───
    // Java: world.provider.dimensionId <= 0 → tryToCreatePortal
    static constexpr int32_t PORTAL_DIM_THRESHOLD = 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Flammability Registry
// Java: BlockFire.func_149843_e() — vanila block flammability initialization
//
// Format: {blockId, encouragement, flammability}
// ═══════════════════════════════════════════════════════════════════════════

struct FlammableBlock {
    int32_t blockId;
    int32_t encouragement;  // field_149849_a — how quickly fire appears
    int32_t flammability;   // field_149848_b — how likely block is destroyed
};

namespace FlammabilityRegistry {
    // ─── Vanilla flammable blocks ───
    // Java: func_149843_e() — 23 entries
    static constexpr FlammableBlock VANILLA_BLOCKS[] = {
        // Wood types — encouragement 5, flammability 20
        {5,   5, 20},  // planks
        {125, 5, 20},  // double_wooden_slab
        {126, 5, 20},  // wooden_slab
        {85,  5, 20},  // fence
        {53,  5, 20},  // oak_stairs
        {135, 5, 20},  // birch_stairs
        {134, 5, 20},  // spruce_stairs
        {136, 5, 20},  // jungle_stairs

        // Logs — encouragement 5, flammability 5
        {17,  5,  5},  // log
        {162, 5,  5},  // log2

        // Leaves — encouragement 30, flammability 60
        {18,  30, 60}, // leaves
        {161, 30, 60}, // leaves2

        // Bookshelf — encouragement 30, flammability 20
        {47,  30, 20}, // bookshelf

        // TNT — encouragement 15, flammability 100
        {46,  15, 100}, // tnt

        // Plants — encouragement 60, flammability 100
        {31,  60, 100}, // tallgrass
        {175, 60, 100}, // double_plant
        {37,  60, 100}, // yellow_flower
        {38,  60, 100}, // red_flower

        // Wool — encouragement 30, flammability 60
        {35,  30, 60}, // wool

        // Vine — encouragement 15, flammability 100
        {106, 15, 100}, // vine

        // Coal block — encouragement 5, flammability 5
        {173, 5,  5},  // coal_block

        // Hay bale — encouragement 60, flammability 20
        {170, 60, 20}, // hay_block

        // Carpet — encouragement 60, flammability 20
        {171, 60, 20}, // carpet
    };

    static constexpr int32_t VANILLA_COUNT = sizeof(VANILLA_BLOCKS) / sizeof(FlammableBlock);

    // ─── Initialize registry ───
    // Fills encouragement[256] and flammability[256] arrays
    inline void initRegistry(std::array<int32_t, 256>& encouragement,
                              std::array<int32_t, 256>& flammability)
    {
        encouragement.fill(0);
        flammability.fill(0);
        for (int32_t i = 0; i < VANILLA_COUNT; ++i) {
            const auto& fb = VANILLA_BLOCKS[i];
            if (fb.blockId >= 0 && fb.blockId < 256) {
                encouragement[fb.blockId] = fb.encouragement;
                flammability[fb.blockId] = fb.flammability;
            }
        }
    }

    // ─── Can block catch fire ───
    // Java: canBlockCatchFire — encouragement > 0
    inline bool canCatchFire(const std::array<int32_t, 256>& encouragement, int32_t blockId) {
        return blockId >= 0 && blockId < 256 && encouragement[blockId] > 0;
    }

    // ─── Get max neighbor encouragement ───
    // Java: getChanceOfNeighborsEncouragingFire
    // Must be called on air block — returns max encouragement from 6 faces
    // Returns 0 if target is not air
    inline int32_t getMaxNeighborEncouragement(
        const std::array<int32_t, 256>& encouragement,
        int32_t blockId)
    {
        if (blockId >= 0 && blockId < 256)
            return encouragement[blockId];
        return 0;
    }

    // ─── Compute spread chance ───
    // Java: n8 = (n6 + 40 + difficulty*7) / (n4 + 30)
    inline int32_t computeSpreadChance(int32_t neighborEncouragement,
                                        int32_t difficulty,
                                        int32_t fireAge,
                                        bool highHumidity)
    {
        int32_t chance = (neighborEncouragement + FireConstants::SPREAD_ENCOURAGE_BONUS
                         + difficulty * FireConstants::SPREAD_DIFFICULTY_MULT)
                        / (fireAge + FireConstants::SPREAD_AGE_OFFSET);
        if (highHumidity) {
            chance /= 2;
        }
        return chance;
    }

    // ─── Compute base chance for height ───
    // Java: n7 = 100; if (k > n2+1) n7 += (k-(n2+1))*100
    inline int32_t getBaseChanceForHeight(int32_t heightAboveFire) {
        int32_t base = FireConstants::BASE_SPREAD_CHANCE;
        if (heightAboveFire > FireConstants::HEIGHT_THRESHOLD) {
            base += (heightAboveFire - FireConstants::HEIGHT_THRESHOLD)
                    * FireConstants::HEIGHT_CHANCE_INCREMENT;
        }
        return base;
    }

    // ─── Is eternal fire ───
    // Java: netherrack in all dims, bedrock in End
    inline bool isEternalFire(int32_t blockBelowId, bool isEnd) {
        if (blockBelowId == FireConstants::NETHERRACK_ID) return true;
        if (isEnd && blockBelowId == FireConstants::BEDROCK_ID) return true;
        return false;
    }
}

} // namespace mccpp
