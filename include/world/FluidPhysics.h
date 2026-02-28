/**
 * FluidPhysics.h — Water and lava flow simulation.
 *
 * Java references:
 *   - net.minecraft.block.BlockLiquid — Base liquid block
 *   - net.minecraft.block.BlockDynamicLiquid — Flowing liquid (updateTick)
 *   - net.minecraft.block.BlockStaticLiquid — Still liquid
 *
 * Flow algorithm (per tick):
 *   1. Calculate new flow level from 4 neighbors (minimum + decay)
 *   2. Source conversion: above liquid with meta>=8 → falling (meta+8)
 *   3. Water spring: 2+ adjacent sources + solid below → source (meta=0)
 *   4. Lava: random 4× slowdown for retreat
 *   5. Downward flow: priority over horizontal. Lava into water → stone.
 *   6. Horizontal spread: path-finding to nearest drop-off (recursive 4-deep)
 *      Flow to all directions with equal minimum cost
 *   7. Blocking: doors, signs, ladders, reeds, portals, solid blocks
 *   8. Lava-water interaction: source → obsidian, flowing ≤4 → cobblestone
 *
 * Metadata:
 *   - 0 = source block
 *   - 1-7 = flow distance (higher = further from source)
 *   - 8-15 = falling liquid (meta & 7 = original level)
 *
 * Tick rates: water=5, lava=30 (overworld) / 10 (nether)
 *
 * Thread safety: Called from world tick thread.
 * JNI readiness: Simple data, predictable layout.
 */
#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// FluidType — Water or lava
// ═══════════════════════════════════════════════════════════════════════════

enum class FluidType : int32_t {
    WATER = 0,
    LAVA = 1
};

// ═══════════════════════════════════════════════════════════════════════════
// Block IDs for fluid interactions
// ═══════════════════════════════════════════════════════════════════════════

namespace FluidBlocks {
    constexpr int32_t AIR = 0;
    constexpr int32_t STONE = 1;
    constexpr int32_t COBBLESTONE = 4;
    constexpr int32_t OBSIDIAN = 49;
    constexpr int32_t FLOWING_WATER = 8;
    constexpr int32_t WATER = 9;
    constexpr int32_t FLOWING_LAVA = 10;
    constexpr int32_t LAVA = 11;
}

// ═══════════════════════════════════════════════════════════════════════════
// FluidPhysics — Fluid flow simulation engine.
// Java reference: net.minecraft.block.BlockDynamicLiquid
// ═══════════════════════════════════════════════════════════════════════════

class FluidPhysics {
public:
    // Callbacks for world interaction
    using GetBlockFn = std::function<int32_t(int32_t, int32_t, int32_t)>;       // → block ID
    using GetMetaFn = std::function<int32_t(int32_t, int32_t, int32_t)>;        // → metadata
    using IsSolidFn = std::function<bool(int32_t, int32_t, int32_t)>;           // → is solid material
    using BlocksMovementFn = std::function<bool(int32_t, int32_t, int32_t)>;    // → blocks movement

    // Block update result
    struct FluidUpdate {
        int32_t x, y, z;
        int32_t newBlockId;
        int32_t newMeta;
        int32_t scheduleTickDelay;  // 0 = no reschedule
        bool playFizzSound;
    };

    FluidPhysics() = default;

    // ─── Static helpers ───

    // Java: getLiquidHeightPercent
    static float getLiquidHeightPercent(int32_t meta) {
        if (meta >= 8) meta = 0;
        return static_cast<float>(meta + 1) / 9.0f;
    }

    // Java: getEffectiveFlowDecay
    static int32_t getEffectiveFlowDecay(int32_t blockId, int32_t meta, FluidType type) {
        bool isFluid = (type == FluidType::WATER)
            ? (blockId == FluidBlocks::FLOWING_WATER || blockId == FluidBlocks::WATER)
            : (blockId == FluidBlocks::FLOWING_LAVA || blockId == FluidBlocks::LAVA);
        if (!isFluid) return -1;
        if (meta >= 8) meta = 0;
        return meta;
    }

    // Java: tickRate
    static int32_t getTickRate(FluidType type, bool isNether) {
        if (type == FluidType::WATER) return 5;
        return isNether ? 10 : 30;
    }

    // ─── Core flow update ───

    // Java: BlockDynamicLiquid.updateTick
    std::vector<FluidUpdate> calculateFlowUpdate(
            int32_t x, int32_t y, int32_t z,
            FluidType type, bool isNether,
            GetBlockFn getBlock, GetMetaFn getMeta,
            IsSolidFn isSolid, BlocksMovementFn blocksMovement,
            int32_t randValue) {  // Random int for lava slowdown

        std::vector<FluidUpdate> updates;
        int32_t flowingId = (type == FluidType::WATER) ? FluidBlocks::FLOWING_WATER : FluidBlocks::FLOWING_LAVA;
        int32_t decay = (type == FluidType::LAVA && !isNether) ? 2 : 1;
        int32_t tickRate = getTickRate(type, isNether);

        // Current flow level
        int32_t currentLevel = getFlowLevel(getBlock, getMeta, type, x, y, z);
        if (currentLevel < 0) return updates;

        if (currentLevel > 0) {
            // Find minimum neighbor level
            int32_t minNeighbor = -100;
            int32_t adjacentSources = 0;

            minNeighbor = checkNeighborLevel(getBlock, getMeta, type, x - 1, y, z, minNeighbor, adjacentSources);
            minNeighbor = checkNeighborLevel(getBlock, getMeta, type, x + 1, y, z, minNeighbor, adjacentSources);
            minNeighbor = checkNeighborLevel(getBlock, getMeta, type, x, y, z - 1, minNeighbor, adjacentSources);
            minNeighbor = checkNeighborLevel(getBlock, getMeta, type, x, y, z + 1, minNeighbor, adjacentSources);

            int32_t newLevel = minNeighbor + decay;
            if (newLevel >= 8 || minNeighbor < 0) {
                newLevel = -1;
            }

            // Source from above
            int32_t aboveLevel = getFlowLevel(getBlock, getMeta, type, x, y + 1, z);
            if (aboveLevel >= 0) {
                newLevel = (aboveLevel >= 8) ? aboveLevel : aboveLevel + 8;
            }

            // Water spring creation: 2+ adjacent sources + solid below
            if (adjacentSources >= 2 && type == FluidType::WATER) {
                if (isSolid(x, y - 1, z)) {
                    newLevel = 0;
                } else {
                    int32_t belowBlock = getBlock(x, y - 1, z);
                    int32_t belowMeta = getMeta(x, y - 1, z);
                    if (isFluidOfType(belowBlock, type) && belowMeta == 0) {
                        newLevel = 0;
                    }
                }
            }

            // Lava random slowdown for retreat
            if (type == FluidType::LAVA && currentLevel < 8 && newLevel < 8 &&
                newLevel > currentLevel && (randValue & 3) != 0) {
                tickRate *= 4;
            }

            if (newLevel == currentLevel) {
                // Convert flowing to static
                // (In Java this calls func_149811_n to switch block ID)
            } else {
                if (newLevel < 0) {
                    // Disappear
                    updates.push_back({x, y, z, FluidBlocks::AIR, 0, 0, false});
                } else {
                    // Update level
                    updates.push_back({x, y, z, flowingId, newLevel, tickRate, false});
                }
            }
            currentLevel = newLevel;
        }

        // ─── Downward flow ───
        if (canFlowInto(getBlock, type, blocksMovement, x, y - 1, z)) {
            int32_t belowBlock = getBlock(x, y - 1, z);

            // Lava into water → stone
            if (type == FluidType::LAVA && isWater(belowBlock)) {
                updates.push_back({x, y - 1, z, FluidBlocks::STONE, 0, 0, true});
                return updates;
            }

            // Flow downward: meta >= 8 → keep meta, else meta + 8
            int32_t downMeta = (currentLevel >= 8) ? currentLevel : currentLevel + 8;
            updates.push_back({x, y - 1, z, flowingId, downMeta, tickRate, false});

        } else if (currentLevel >= 0 && (currentLevel == 0 || isBlocking(getBlock, blocksMovement, x, y - 1, z))) {
            // ─── Horizontal spread ───
            auto flowDirs = findFlowDirections(x, y, z, type, getBlock, getMeta, blocksMovement);

            int32_t spreadLevel = currentLevel + decay;
            if (currentLevel >= 8) spreadLevel = 1;
            if (spreadLevel >= 8) return updates;

            // 4 directions: -X, +X, -Z, +Z
            constexpr int32_t dx[4] = {-1, 1, 0, 0};
            constexpr int32_t dz[4] = {0, 0, -1, 1};

            for (int32_t dir = 0; dir < 4; ++dir) {
                if (!flowDirs[dir]) continue;
                int32_t nx = x + dx[dir];
                int32_t nz = z + dz[dir];

                if (canFlowInto(getBlock, type, blocksMovement, nx, y, nz)) {
                    int32_t destBlock = getBlock(nx, y, nz);
                    bool fizzle = (type == FluidType::LAVA);
                    updates.push_back({nx, y, nz, flowingId, spreadLevel, tickRate, fizzle && !isAir(destBlock)});
                }
            }
        }

        return updates;
    }

    // ─── Lava-water interaction check ───

    struct LavaWaterResult {
        bool shouldTransform;
        int32_t resultBlockId;  // Obsidian or cobblestone
    };

    // Java: func_149805_n — check if lava is adjacent to water
    static LavaWaterResult checkLavaWaterInteraction(
            int32_t x, int32_t y, int32_t z, int32_t meta,
            GetBlockFn getBlock) {

        bool adjacentWater = false;
        if (isWater(getBlock(x, y, z - 1))) adjacentWater = true;
        if (!adjacentWater && isWater(getBlock(x, y, z + 1))) adjacentWater = true;
        if (!adjacentWater && isWater(getBlock(x - 1, y, z))) adjacentWater = true;
        if (!adjacentWater && isWater(getBlock(x + 1, y, z))) adjacentWater = true;
        if (!adjacentWater && isWater(getBlock(x, y + 1, z))) adjacentWater = true;

        if (adjacentWater) {
            if (meta == 0) {
                return {true, FluidBlocks::OBSIDIAN};
            } else if (meta <= 4) {
                return {true, FluidBlocks::COBBLESTONE};
            }
        }
        return {false, 0};
    }

private:
    // Java: func_149804_e — get flow level if same fluid type
    static int32_t getFlowLevel(GetBlockFn& getBlock, GetMetaFn& getMeta,
                                  FluidType type, int32_t x, int32_t y, int32_t z) {
        int32_t blockId = getBlock(x, y, z);
        if (!isFluidOfType(blockId, type)) return -1;
        return getMeta(x, y, z);
    }

    // Java: func_149810_a — check and accumulate neighbor level
    static int32_t checkNeighborLevel(GetBlockFn& getBlock, GetMetaFn& getMeta,
                                        FluidType type,
                                        int32_t x, int32_t y, int32_t z,
                                        int32_t currentMin, int32_t& sourceCount) {
        int32_t blockId = getBlock(x, y, z);
        if (!isFluidOfType(blockId, type)) return currentMin;
        int32_t meta = getMeta(x, y, z);
        if (meta == 0) ++sourceCount;
        if (meta >= 8) meta = 0;
        return (currentMin < 0 || meta < currentMin) ? meta : currentMin;
    }

    // Java: func_149809_q — can the fluid flow into this position?
    static bool canFlowInto(GetBlockFn& getBlock, FluidType type,
                              BlocksMovementFn& blocksMovement,
                              int32_t x, int32_t y, int32_t z) {
        int32_t blockId = getBlock(x, y, z);
        if (isFluidOfType(blockId, type)) return false;
        if (blockId == FluidBlocks::FLOWING_LAVA || blockId == FluidBlocks::LAVA) return false;
        return !isBlocking(getBlock, blocksMovement, x, y, z);
    }

    // Java: func_149807_p — does this block prevent fluid flow?
    static bool isBlocking(GetBlockFn& getBlock, BlocksMovementFn& blocksMovement,
                             int32_t x, int32_t y, int32_t z) {
        int32_t blockId = getBlock(x, y, z);
        // Special blocks that block fluid
        // 64=oak_door, 71=iron_door, 63=standing_sign, 65=ladder, 83=reeds
        if (blockId == 64 || blockId == 71 || blockId == 63 || blockId == 65 || blockId == 83) {
            return true;
        }
        // Portal blocks
        if (blockId == 90) return true; // portal
        return blocksMovement(x, y, z);
    }

    // Java: func_149808_o — shortest path to drop-off for horizontal spread
    std::array<bool, 4> findFlowDirections(
            int32_t x, int32_t y, int32_t z, FluidType type,
            GetBlockFn& getBlock, GetMetaFn& getMeta,
            BlocksMovementFn& blocksMovement) {

        std::array<int32_t, 4> costs;
        constexpr int32_t dx[4] = {-1, 1, 0, 0};
        constexpr int32_t dz[4] = {0, 0, -1, 1};

        for (int32_t dir = 0; dir < 4; ++dir) {
            int32_t nx = x + dx[dir];
            int32_t nz = z + dz[dir];
            costs[dir] = 1000;

            if (isBlocking(getBlock, blocksMovement, nx, y, nz) ||
                (isFluidOfType(getBlock(nx, y, nz), type) && getMeta(nx, y, nz) == 0)) {
                continue;
            }

            if (!isBlocking(getBlock, blocksMovement, nx, y - 1, nz)) {
                costs[dir] = 0;
            } else {
                costs[dir] = findDropOff(x, y, z, 1, dir, type, getBlock, getMeta, blocksMovement);
            }
        }

        // Find minimum cost
        int32_t minCost = costs[0];
        for (int32_t i = 1; i < 4; ++i) {
            if (costs[i] < minCost) minCost = costs[i];
        }

        // Enable all directions with minimum cost
        std::array<bool, 4> result;
        for (int32_t i = 0; i < 4; ++i) {
            result[i] = (costs[i] == minCost);
        }
        return result;
    }

    // Java: func_149812_c — recursive BFS for shortest path to drop-off
    int32_t findDropOff(int32_t x, int32_t y, int32_t z,
                          int32_t depth, int32_t fromDir, FluidType type,
                          GetBlockFn& getBlock, GetMetaFn& getMeta,
                          BlocksMovementFn& blocksMovement) {
        constexpr int32_t dx[4] = {-1, 1, 0, 0};
        constexpr int32_t dz[4] = {0, 0, -1, 1};
        // Opposite: 0↔1, 2↔3
        constexpr int32_t opposite[4] = {1, 0, 3, 2};

        int32_t minCost = 1000;
        for (int32_t dir = 0; dir < 4; ++dir) {
            if (dir == opposite[fromDir]) continue; // Don't go back

            int32_t nx = x + dx[dir];
            int32_t nz = z + dz[dir];

            if (isBlocking(getBlock, blocksMovement, nx, y, nz) ||
                (isFluidOfType(getBlock(nx, y, nz), type) && getMeta(nx, y, nz) == 0)) {
                continue;
            }

            if (!isBlocking(getBlock, blocksMovement, nx, y - 1, nz)) {
                return depth;
            }

            if (depth < 4) {
                int32_t cost = findDropOff(nx, y, nz, depth + 1, dir, type,
                                             getBlock, getMeta, blocksMovement);
                if (cost < minCost) minCost = cost;
            }
        }
        return minCost;
    }

    // ─── Helpers ───
    static bool isFluidOfType(int32_t blockId, FluidType type) {
        if (type == FluidType::WATER) {
            return blockId == FluidBlocks::FLOWING_WATER || blockId == FluidBlocks::WATER;
        }
        return blockId == FluidBlocks::FLOWING_LAVA || blockId == FluidBlocks::LAVA;
    }

    static bool isWater(int32_t blockId) {
        return blockId == FluidBlocks::FLOWING_WATER || blockId == FluidBlocks::WATER;
    }

    static bool isAir(int32_t blockId) {
        return blockId == FluidBlocks::AIR;
    }
};

} // namespace mccpp
