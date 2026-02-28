/**
 * WorldGenTrees.h — Oak, birch, and jungle tree generation.
 *
 * Java reference: net.minecraft.world.gen.feature.WorldGenTrees
 *
 * Algorithm:
 *   1. Height = rand(3) + minTreeHeight (default 4, total 4-6)
 *   2. Space check: trunk column = 0 radius, top 2 layers = 2 radius
 *      Only replaceable blocks allowed (air, leaves)
 *   3. Require grass(2)/dirt(3)/farmland(60) below base
 *   4. Place dirt at base position
 *   5. Leaf crown: top 4 layers
 *      - Layer offset from top: 0, -1, -2, -3
 *      - Radius = floor(1 + 1 - offset / 2) → 1, 1, 2, 2
 *      - Corners (|dx|==r && |dz|==r): skip if rand(2)==0 on bottom layer
 *      - Only replace air/leaves blocks
 *   6. Trunk: log blocks (metaWood) for height column, only replace air/leaves
 *   7. Jungle vines: 2/3 chance per exposed trunk face, vine meta per direction
 *   8. Jungle leaf vines: 1/4 chance per exposed leaf face, grow downward 4
 *   9. Jungle cocoa: 1/5 × height>5, rand growth stages on 2 layers
 *
 * Block IDs: log(17), leaves(18), dirt(3), grass(2), vine(106), cocoa(127)
 *
 * Thread safety: Called from chunk generation thread.
 * JNI readiness: Simple data, predictable layout.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <functional>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// TreeGenerator — Standard tree (oak, birch, jungle variants).
// Java reference: net.minecraft.world.gen.feature.WorldGenTrees
// ═══════════════════════════════════════════════════════════════════════════

class TreeGenerator {
public:
    // Block IDs
    static constexpr int32_t AIR = 0;
    static constexpr int32_t DIRT = 3;
    static constexpr int32_t GRASS = 2;
    static constexpr int32_t FARMLAND = 60;
    static constexpr int32_t LOG = 17;
    static constexpr int32_t LEAVES = 18;
    static constexpr int32_t VINE = 106;
    static constexpr int32_t COCOA = 127;

    // Vine meta values per direction
    static constexpr int32_t VINE_SOUTH = 1;  // -Z face
    static constexpr int32_t VINE_EAST = 2;   // +X face
    static constexpr int32_t VINE_NORTH = 4;  // +Z face
    static constexpr int32_t VINE_WEST = 8;   // -X face

    // Callbacks
    using GetBlockFn = std::function<int32_t(int32_t, int32_t, int32_t)>;
    using IsReplaceable = std::function<bool(int32_t)>;  // Can tree replace this block?

    // RNG
    struct RNG {
        int64_t seed;
        void setSeed(int64_t s) {
            seed = (s ^ 0x5DEECE66DLL) & ((1LL << 48) - 1);
        }
        int32_t nextInt(int32_t bound) {
            if (bound <= 0) return 0;
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t bits = static_cast<int32_t>(seed >> 17);
            return ((bits % bound) + bound) % bound;
        }
    };

    // Block placement result
    struct BlockPlace {
        int32_t x, y, z;
        int32_t blockId;
        int32_t meta;
    };

    // Java: WorldGenTrees.generate
    static std::vector<BlockPlace> generateTree(
            int32_t x, int32_t y, int32_t z,
            int32_t minTreeHeight, int32_t metaWood, int32_t metaLeaves,
            bool vinesGrow,
            RNG& rng,
            GetBlockFn getBlock,
            IsReplaceable isReplaceable) {

        std::vector<BlockPlace> result;

        int32_t height = rng.nextInt(3) + minTreeHeight;

        // Bounds check
        if (y < 1 || y + height + 1 > 256) return result;

        // Space check
        for (int32_t checkY = y; checkY <= y + 1 + height; ++checkY) {
            int32_t radius = 1;
            if (checkY == y) radius = 0;
            if (checkY >= y + 1 + height - 2) radius = 2;

            for (int32_t cx = x - radius; cx <= x + radius; ++cx) {
                for (int32_t cz = z - radius; cz <= z + radius; ++cz) {
                    if (checkY >= 0 && checkY < 256) {
                        int32_t block = getBlock(cx, checkY, cz);
                        if (!isReplaceable(block)) return result;
                    } else {
                        return result;
                    }
                }
            }
        }

        // Check soil
        int32_t soilBlock = getBlock(x, y - 1, z);
        if (soilBlock != GRASS && soilBlock != DIRT && soilBlock != FARMLAND) return result;
        if (y >= 256 - height - 1) return result;

        // Place dirt at base
        result.push_back({x, y - 1, z, DIRT, 0});

        // ─── Leaf crown (top 4 layers) ───
        for (int32_t ly = y - 3 + height; ly <= y + height; ++ly) {
            int32_t layerOffset = ly - (y + height);
            int32_t leafRadius = 1 - layerOffset / 2;

            for (int32_t lx = x - leafRadius; lx <= x + leafRadius; ++lx) {
                int32_t dx = lx - x;
                for (int32_t lz = z - leafRadius; lz <= z + leafRadius; ++lz) {
                    int32_t dz = lz - z;

                    // Corner cutoff
                    if (std::abs(dx) == leafRadius && std::abs(dz) == leafRadius &&
                        (rng.nextInt(2) == 0 || layerOffset == 0)) {
                        continue;
                    }

                    int32_t block = getBlock(lx, ly, lz);
                    if (block == AIR || isLeaves(block)) {
                        result.push_back({lx, ly, lz, LEAVES, metaLeaves});
                    }
                }
            }
        }

        // ─── Trunk ───
        for (int32_t ty = 0; ty < height; ++ty) {
            int32_t block = getBlock(x, y + ty, z);
            if (block == AIR || isLeaves(block)) {
                result.push_back({x, y + ty, z, LOG, metaWood});
            }

            // Jungle vines on trunk
            if (vinesGrow && ty > 0) {
                if (rng.nextInt(3) > 0 && getBlock(x - 1, y + ty, z) == AIR) {
                    result.push_back({x - 1, y + ty, z, VINE, VINE_WEST});
                }
                if (rng.nextInt(3) > 0 && getBlock(x + 1, y + ty, z) == AIR) {
                    result.push_back({x + 1, y + ty, z, VINE, VINE_EAST});
                }
                if (rng.nextInt(3) > 0 && getBlock(x, y + ty, z - 1) == AIR) {
                    result.push_back({x, y + ty, z - 1, VINE, VINE_SOUTH});
                }
                if (rng.nextInt(3) > 0 && getBlock(x, y + ty, z + 1) == AIR) {
                    result.push_back({x, y + ty, z + 1, VINE, VINE_NORTH});
                }
            }
        }

        // ─── Jungle leaf vines ───
        if (vinesGrow) {
            for (int32_t ly = y - 3 + height; ly <= y + height; ++ly) {
                int32_t layerOffset = ly - (y + height);
                int32_t vineRadius = 2 - layerOffset / 2;

                for (int32_t lx = x - vineRadius; lx <= x + vineRadius; ++lx) {
                    for (int32_t lz = z - vineRadius; lz <= z + vineRadius; ++lz) {
                        if (!isLeaves(getBlock(lx, ly, lz))) continue;

                        if (rng.nextInt(4) == 0 && getBlock(lx - 1, ly, lz) == AIR) {
                            growVines(result, lx - 1, ly, lz, VINE_WEST, getBlock);
                        }
                        if (rng.nextInt(4) == 0 && getBlock(lx + 1, ly, lz) == AIR) {
                            growVines(result, lx + 1, ly, lz, VINE_EAST, getBlock);
                        }
                        if (rng.nextInt(4) == 0 && getBlock(lx, ly, lz - 1) == AIR) {
                            growVines(result, lx, ly, lz - 1, VINE_SOUTH, getBlock);
                        }
                        if (rng.nextInt(4) == 0 && getBlock(lx, ly, lz + 1) == AIR) {
                            growVines(result, lx, ly, lz + 1, VINE_NORTH, getBlock);
                        }
                    }
                }
            }

            // Cocoa pods for tall jungle trees
            if (rng.nextInt(5) == 0 && height > 5) {
                // Direction table (rotateOpposite)
                constexpr int32_t dirOffX[4] = { 0, -1,  0,  1};
                constexpr int32_t dirOffZ[4] = { 1,  0, -1,  0};
                constexpr int32_t rotOpp[4]  = { 2,  3,  0,  1};

                for (int32_t layer = 0; layer < 2; ++layer) {
                    for (int32_t dir = 0; dir < 4; ++dir) {
                        if (rng.nextInt(4 - layer) != 0) continue;
                        int32_t growthStage = rng.nextInt(3);
                        int32_t cx = x + dirOffX[rotOpp[dir]];
                        int32_t cy = y + height - 5 + layer;
                        int32_t cz = z + dirOffZ[rotOpp[dir]];
                        result.push_back({cx, cy, cz, COCOA, (growthStage << 2) | dir});
                    }
                }
            }
        }

        return result;
    }

    // ─── Big oak tree (WorldGenBigTree) parameters ───
    // Simplified — big oaks use ball-and-limb algorithm, full impl is complex.
    // These constants define the standard big oak for reference:
    static constexpr int32_t BIG_OAK_MIN_HEIGHT = 5;
    static constexpr int32_t BIG_OAK_HEIGHT_LIMIT = 12;
    static constexpr double BIG_OAK_LEAF_DENSITY = 1.0;
    static constexpr double BIG_OAK_BRANCH_SLOPE = 0.381;

private:
    static bool isLeaves(int32_t blockId) {
        return blockId == LEAVES || blockId == 161; // leaves2
    }

    // Java: growVines — place vine block, grow downward up to 4 blocks
    static void growVines(std::vector<BlockPlace>& result,
                            int32_t x, int32_t y, int32_t z, int32_t meta,
                            GetBlockFn& getBlock) {
        result.push_back({x, y, z, VINE, meta});
        int32_t remaining = 4;
        --y;
        while (getBlock(x, y, z) == AIR && remaining > 0) {
            result.push_back({x, y, z, VINE, meta});
            --y;
            --remaining;
        }
    }
};

} // namespace mccpp
