/**
 * WorldGenOre.h — Ore vein generation and standard ore distribution.
 *
 * Java references:
 *   - net.minecraft.world.gen.feature.WorldGenMinable — Ore vein placement
 *   - net.minecraft.world.biome.BiomeDecorator — Ore distribution config
 *
 * Vein algorithm (WorldGenMinable.generate):
 *   1. Random angle → parametric line through block space
 *   2. Two endpoints offset ± sin(angle)*count/8 in XZ, ± rand(3)-2 in Y
 *   3. For each of (numberOfBlocks+1) steps along line:
 *      - Interpolate position, compute radius = (sin(step/count*π)+1) * randRadius + 1
 *      - Iterate all blocks in bounding box, check ellipsoid membership
 *      - Replace target block (default: stone) with ore block
 *
 * Standard ore distribution (BiomeDecorator):
 *   - genStandardOre1: count attempts at uniform Y in [minY, maxY)
 *   - genStandardOre2: count attempts at triangle-distributed Y (center ± spread)
 *
 * Block IDs: coal(16), iron(15), gold(14), diamond(56), redstone(73),
 *            lapis(21), emerald(129), dirt(3), gravel(13), stone(1)
 *
 * Thread safety: Called from chunk generation thread.
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
// OreVeinGenerator — Single ore vein placement.
// Java reference: net.minecraft.world.gen.feature.WorldGenMinable
// ═══════════════════════════════════════════════════════════════════════════

class OreVeinGenerator {
public:
    // Callbacks
    using GetBlockFn = std::function<int32_t(int32_t, int32_t, int32_t)>;
    using SetBlockFn = std::function<void(int32_t, int32_t, int32_t, int32_t)>;

    // RNG — Java LCG (matching Random.nextFloat, nextInt, nextDouble)
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
        float nextFloat() {
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            return static_cast<float>(static_cast<int32_t>(seed >> 24)) / static_cast<float>(1 << 24);
        }
        double nextDouble() {
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t hi = static_cast<int32_t>(seed >> 22);
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t lo = static_cast<int32_t>(seed >> 22);
            return (static_cast<double>((static_cast<int64_t>(hi) << 27) + lo)) / static_cast<double>(1LL << 53);
        }
    };

    OreVeinGenerator() = default;

    // Java: WorldGenMinable.generate
    // oreBlockId: the ore to place
    // numberOfBlocks: vein size
    // replaceBlockId: block to replace (default: stone=1)
    static void generateVein(int32_t x, int32_t y, int32_t z,
                               int32_t oreBlockId, int32_t numberOfBlocks,
                               int32_t replaceBlockId,
                               RNG& rng,
                               GetBlockFn getBlock, SetBlockFn setBlock) {

        float angle = rng.nextFloat() * static_cast<float>(M_PI);

        // Parametric line endpoints
        double x1 = static_cast<double>(static_cast<float>(x + 8) +
            std::sin(angle) * static_cast<float>(numberOfBlocks) / 8.0f);
        double x2 = static_cast<double>(static_cast<float>(x + 8) -
            std::sin(angle) * static_cast<float>(numberOfBlocks) / 8.0f);
        double z1 = static_cast<double>(static_cast<float>(z + 8) +
            std::cos(angle) * static_cast<float>(numberOfBlocks) / 8.0f);
        double z2 = static_cast<double>(static_cast<float>(z + 8) -
            std::cos(angle) * static_cast<float>(numberOfBlocks) / 8.0f);
        double y1 = static_cast<double>(y + rng.nextInt(3) - 2);
        double y2 = static_cast<double>(y + rng.nextInt(3) - 2);

        for (int32_t step = 0; step <= numberOfBlocks; ++step) {
            // Interpolate position along line
            double t = static_cast<double>(step) / static_cast<double>(numberOfBlocks);
            double cx = x1 + (x2 - x1) * t;
            double cy = y1 + (y2 - y1) * t;
            double cz = z1 + (z2 - z1) * t;

            // Random radius (sin-shaped profile)
            double randRadius = rng.nextDouble() * static_cast<double>(numberOfBlocks) / 16.0;
            double sinVal = static_cast<double>(std::sin(static_cast<float>(step) *
                static_cast<float>(M_PI) / static_cast<float>(numberOfBlocks)) + 1.0f);
            double hRadius = sinVal * randRadius + 1.0;
            double vRadius = sinVal * randRadius + 1.0;

            // Bounding box
            int32_t minX = floorD(cx - hRadius / 2.0);
            int32_t minY = floorD(cy - vRadius / 2.0);
            int32_t minZ = floorD(cz - hRadius / 2.0);
            int32_t maxX = floorD(cx + hRadius / 2.0);
            int32_t maxY = floorD(cy + vRadius / 2.0);
            int32_t maxZ = floorD(cz + hRadius / 2.0);

            // Place ore in ellipsoid
            for (int32_t bx = minX; bx <= maxX; ++bx) {
                double nx = (static_cast<double>(bx) + 0.5 - cx) / (hRadius / 2.0);
                if (nx * nx >= 1.0) continue;

                for (int32_t by = minY; by <= maxY; ++by) {
                    double ny = (static_cast<double>(by) + 0.5 - cy) / (vRadius / 2.0);
                    if (nx * nx + ny * ny >= 1.0) continue;

                    for (int32_t bz = minZ; bz <= maxZ; ++bz) {
                        double nz = (static_cast<double>(bz) + 0.5 - cz) / (hRadius / 2.0);
                        if (nx * nx + ny * ny + nz * nz >= 1.0) continue;

                        if (getBlock(bx, by, bz) == replaceBlockId) {
                            setBlock(bx, by, bz, oreBlockId);
                        }
                    }
                }
            }
        }
    }

private:
    static int32_t floorD(double d) {
        int32_t i = static_cast<int32_t>(d);
        return d < static_cast<double>(i) ? i - 1 : i;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// OreDistribution — Standard ore generation parameters.
// Java reference: net.minecraft.world.biome.BiomeDecorator
// ═══════════════════════════════════════════════════════════════════════════

struct OreConfig {
    int32_t oreBlockId;       // Block ID of the ore
    int32_t veinSize;         // Blocks per vein
    int32_t attemptsPerChunk; // Veins per chunk
    int32_t minY;             // Minimum Y for vein center
    int32_t maxY;             // Maximum Y for vein center
    int32_t replaceBlockId;   // Block to replace (default: stone=1)
    bool triangleDistribution; // True = centered distribution (lapis)
};

namespace OreBlocks {
    constexpr int32_t STONE = 1;
    constexpr int32_t DIRT = 3;
    constexpr int32_t GRAVEL = 13;
    constexpr int32_t COAL_ORE = 16;
    constexpr int32_t IRON_ORE = 15;
    constexpr int32_t GOLD_ORE = 14;
    constexpr int32_t DIAMOND_ORE = 56;
    constexpr int32_t REDSTONE_ORE = 73;
    constexpr int32_t LAPIS_ORE = 21;
    constexpr int32_t EMERALD_ORE = 129;
}

class OreDistribution {
public:
    // Java: BiomeDecorator standard ore config
    static std::vector<OreConfig> getStandardOres() {
        return {
            // Java: genStandardOre1(20, dirtGen, 0, 256) — dirt veins in stone
            {OreBlocks::DIRT, 33, 20, 0, 256, OreBlocks::STONE, false},
            // Java: genStandardOre1(10, gravelGen, 0, 256) — gravel veins in stone
            {OreBlocks::GRAVEL, 33, 10, 0, 256, OreBlocks::STONE, false},
            // Java: genStandardOre1(20, coalGen, 0, 128)
            {OreBlocks::COAL_ORE, 17, 20, 0, 128, OreBlocks::STONE, false},
            // Java: genStandardOre1(20, ironGen, 0, 64)
            {OreBlocks::IRON_ORE, 9, 20, 0, 64, OreBlocks::STONE, false},
            // Java: genStandardOre1(2, goldGen, 0, 32)
            {OreBlocks::GOLD_ORE, 9, 2, 0, 32, OreBlocks::STONE, false},
            // Java: genStandardOre1(8, redstoneGen, 0, 16)
            {OreBlocks::REDSTONE_ORE, 8, 8, 0, 16, OreBlocks::STONE, false},
            // Java: genStandardOre1(1, diamondGen, 0, 16)
            {OreBlocks::DIAMOND_ORE, 8, 1, 0, 16, OreBlocks::STONE, false},
            // Java: genStandardOre2(1, lapisGen, 16, 16) — triangle/centered Y
            {OreBlocks::LAPIS_ORE, 7, 1, 16, 16, OreBlocks::STONE, true},
        };
    }

    // Java: genStandardOre1 — uniform Y distribution
    // Y = minY + rand(maxY - minY)
    static int32_t getUniformY(int32_t minY, int32_t maxY, OreVeinGenerator::RNG& rng) {
        return minY + rng.nextInt(maxY - minY);
    }

    // Java: genStandardOre2 — triangle/centered Y distribution
    // Y = rand(center) + rand(spread)  (centered around center, clipped to [0,255])
    static int32_t getTriangleY(int32_t center, int32_t spread, OreVeinGenerator::RNG& rng) {
        return rng.nextInt(center) + rng.nextInt(spread);
    }

    // Generate all standard ores for one chunk
    static void generateChunkOres(int32_t chunkX, int32_t chunkZ,
                                    OreVeinGenerator::RNG& rng,
                                    OreVeinGenerator::GetBlockFn getBlock,
                                    OreVeinGenerator::SetBlockFn setBlock) {
        int32_t baseX = chunkX * 16;
        int32_t baseZ = chunkZ * 16;

        auto ores = getStandardOres();
        for (const auto& config : ores) {
            for (int32_t attempt = 0; attempt < config.attemptsPerChunk; ++attempt) {
                int32_t x = baseX + rng.nextInt(16);
                int32_t z = baseZ + rng.nextInt(16);
                int32_t y;

                if (config.triangleDistribution) {
                    y = getTriangleY(config.minY, config.maxY, rng);
                } else {
                    y = getUniformY(config.minY, config.maxY, rng);
                }

                OreVeinGenerator::generateVein(x, y, z,
                    config.oreBlockId, config.veinSize,
                    config.replaceBlockId, rng, getBlock, setBlock);
            }
        }
    }
};

} // namespace mccpp
