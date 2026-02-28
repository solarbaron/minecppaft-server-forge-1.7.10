/**
 * ChunkProviderGenerate.h — Core terrain generation pipeline.
 *
 * Java reference: net.minecraft.world.gen.ChunkProviderGenerate
 *
 * Pipeline (provideChunk):
 *   1. Seed RNG: chunkX * 341873128712L + chunkZ * 132897987541L
 *   2. Biome lookup for 10×10 area (chunk ± 2 in 4-block grid)
 *   3. 3D density field: 5×33×5 grid via func_147423_a
 *      - 7 noise octave generators
 *      - 5×5 parabolic biome height blending (10/sqrt(d²+0.2)/(minH+2))
 *      - Depth noise from noiseGen6
 *      - denormalizeClamp(lower, upper, interp) for final density
 *      - Sky falloff above y=29
 *   4. Trilinear interpolation: 5×33×5 → 16×256×16
 *      - Stone where density > 0, water where y < 63
 *   5. replaceBlocksForBiome: surface replacement (grass, dirt, sand, etc.)
 *   6. caveGenerator.generate + ravineGenerator.generate
 *   7. Structure generators (if mapFeaturesEnabled)
 *
 * Pipeline (populate):
 *   1. Structure population
 *   2. Water lakes (1/4 chance, not in desert)
 *   3. Lava lakes (1/8 chance, below y=63 or 1/10 above)
 *   4. 8 dungeon attempts
 *   5. Biome decoration (ores, trees, flowers, etc.)
 *   6. World-gen mob spawning
 *   7. Ice/snow at precipitation height
 *
 * Thread safety: Called from chunk generation thread.
 * JNI readiness: Simple data, predictable layout.
 */
#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// ChunkProviderGenerate — Core terrain generation.
// Java reference: net.minecraft.world.gen.ChunkProviderGenerate
// ═══════════════════════════════════════════════════════════════════════════

class ChunkProviderGenerate {
public:
    // Block IDs
    static constexpr int32_t STONE = 1;
    static constexpr int32_t WATER = 9;
    static constexpr int32_t ICE = 79;
    static constexpr int32_t SNOW_LAYER = 78;
    static constexpr int32_t LAVA_STILL = 11;

    static constexpr int32_t SEA_LEVEL = 63;

    // Noise generation callbacks
    using NoiseOctavesFn = std::function<void(double*, int32_t, int32_t, int32_t,
                                               int32_t, int32_t, int32_t,
                                               double, double, double)>;
    using NoisePerlinFn = std::function<void(double*, int32_t, int32_t,
                                              int32_t, int32_t,
                                              double, double, double)>;

    // Biome data for generation
    struct BiomeData {
        float minHeight;    // Biome height offset
        float maxHeight;    // Biome height variation
        int32_t biomeId;    // Biome ID for biome array
    };

    // Configuration
    struct Config {
        int64_t worldSeed;
        bool mapFeaturesEnabled;
        bool amplified;
    };

    ChunkProviderGenerate() = default;
    explicit ChunkProviderGenerate(const Config& cfg) : config_(cfg) {
        // Build parabolic field: 5×5 weights for biome height blending
        for (int32_t i = -2; i <= 2; ++i) {
            for (int32_t j = -2; j <= 2; ++j) {
                parabolicField_[(i + 2) + (j + 2) * 5] =
                    10.0f / std::sqrt(static_cast<float>(i * i + j * j) + 0.2f);
            }
        }
    }

    // ─── Terrain shape: 3D density field ───

    // Java: func_147423_a — compute 5×33×5 density field
    // biomes10x10: 10×10 biome data for blending
    // noiseGen6Data: depth noise (5×5)
    // noiseLower/Upper/Interp: 3D noise for density
    void computeDensityField(double* densityOut,
                               int32_t gridX, int32_t gridY, int32_t gridZ,
                               const BiomeData* biomes10x10,
                               const double* depthNoise,
                               const double* noiseLower,
                               const double* noiseUpper,
                               const double* noiseInterp) {
        int32_t n3 = 0;  // 3D noise index
        int32_t n5 = 0;  // depth noise index

        for (int32_t i = 0; i < 5; ++i) {
            for (int32_t j = 0; j < 5; ++j) {
                // 5×5 parabolic biome height blending
                float totalVariation = 0.0f;
                float totalHeight = 0.0f;
                float totalWeight = 0.0f;

                const BiomeData& centerBiome = biomes10x10[(i + 2) + (j + 2) * 10];

                for (int32_t di = -2; di <= 2; ++di) {
                    for (int32_t dj = -2; dj <= 2; ++dj) {
                        const BiomeData& neighbor = biomes10x10[(i + di + 2) + (j + dj + 2) * 10];

                        float minH = neighbor.minHeight;
                        float maxH = neighbor.maxHeight;

                        // Amplified world type
                        if (config_.amplified && minH > 0.0f) {
                            minH = 1.0f + minH * 2.0f;
                            maxH = 1.0f + maxH * 4.0f;
                        }

                        float weight = parabolicField_[(di + 2) + (dj + 2) * 5] / (minH + 2.0f);

                        // Java: higher neighbors get halved weight
                        if (neighbor.minHeight > centerBiome.minHeight) {
                            weight /= 2.0f;
                        }

                        totalVariation += maxH * weight;
                        totalHeight += minH * weight;
                        totalWeight += weight;
                    }
                }

                totalVariation /= totalWeight;
                totalHeight /= totalWeight;
                totalVariation = totalVariation * 0.9f + 0.1f;
                totalHeight = (totalHeight * 4.0f - 1.0f) / 8.0f;

                // Depth noise processing
                double depthVal = depthNoise[n5] / 8000.0;
                if (depthVal < 0.0) depthVal = -depthVal * 0.3;
                depthVal = depthVal * 3.0 - 2.0;

                if (depthVal < 0.0) {
                    depthVal /= 2.0;
                    if (depthVal < -1.0) depthVal = -1.0;
                    depthVal /= 1.4;
                    depthVal /= 2.0;
                } else {
                    if (depthVal > 1.0) depthVal = 1.0;
                    depthVal /= 8.0;
                }
                ++n5;

                double baseHeight = static_cast<double>(totalHeight);
                double variation = static_cast<double>(totalVariation);
                baseHeight += depthVal * 0.2;
                baseHeight = baseHeight * 8.5 / 8.0;
                double seaLevelD = 8.5 + baseHeight * 4.0;

                // Compute 33 density values for this column
                for (int32_t k = 0; k < 33; ++k) {
                    double densityBias = (static_cast<double>(k) - seaLevelD) *
                                          12.0 * 128.0 / 256.0 / variation;
                    if (densityBias < 0.0) densityBias *= 4.0;

                    double lower = noiseLower[n3] / 512.0;
                    double upper = noiseUpper[n3] / 512.0;
                    double interp = (noiseInterp[n3] / 10.0 + 1.0) / 2.0;

                    // Java: MathHelper.denormalizeClamp
                    double density;
                    if (interp < 0.0) {
                        density = lower;
                    } else if (interp > 1.0) {
                        density = upper;
                    } else {
                        density = lower + (upper - lower) * interp;
                    }
                    density -= densityBias;

                    // Sky falloff above y=29
                    if (k > 29) {
                        double falloff = static_cast<double>(k - 29) / 3.0;
                        density = density * (1.0 - falloff) + -10.0 * falloff;
                    }

                    densityOut[n3] = density;
                    ++n3;
                }
            }
        }
    }

    // ─── Trilinear interpolation: 5×33×5 density → 16×256×16 blocks ───

    // Java: func_147424_a
    // densityField: 5×33×5 = 825 doubles
    // blocksOut: 16×16×256 = 65536 block IDs (XZY ordering: (x*16+z)*256+y)
    void interpolateBlocks(const double* densityField, int32_t* blocksOut) {
        for (int32_t i = 0; i < 4; ++i) {
            int32_t i0 = i * 5;
            int32_t i1 = (i + 1) * 5;

            for (int32_t j = 0; j < 4; ++j) {
                int32_t idx00 = (i0 + j) * 33;
                int32_t idx01 = (i0 + j + 1) * 33;
                int32_t idx10 = (i1 + j) * 33;
                int32_t idx11 = (i1 + j + 1) * 33;

                for (int32_t k = 0; k < 32; ++k) {
                    double d00 = densityField[idx00 + k];
                    double d01 = densityField[idx01 + k];
                    double d10 = densityField[idx10 + k];
                    double d11 = densityField[idx11 + k];

                    double dd00 = (densityField[idx00 + k + 1] - d00) * 0.125;
                    double dd01 = (densityField[idx01 + k + 1] - d01) * 0.125;
                    double dd10 = (densityField[idx10 + k + 1] - d10) * 0.125;
                    double dd11 = (densityField[idx11 + k + 1] - d11) * 0.125;

                    for (int32_t yi = 0; yi < 8; ++yi) {
                        double dX0 = d00;
                        double dX1 = d01;
                        double ddX0 = (d10 - d00) * 0.25;
                        double ddX1 = (d11 - d01) * 0.25;

                        for (int32_t xi = 0; xi < 4; ++xi) {
                            double dZ = dX0;
                            double ddZ = (dX1 - dX0) * 0.25;

                            for (int32_t zi = 0; zi < 4; ++zi) {
                                int32_t blockX = xi + i * 4;
                                int32_t blockZ = zi + j * 4;
                                int32_t blockY = k * 8 + yi;
                                int32_t index = (blockX * 16 + blockZ) * 256 + blockY;

                                dZ += ddZ;

                                if (dZ > 0.0) {
                                    blocksOut[index] = STONE;
                                } else if (blockY < SEA_LEVEL) {
                                    blocksOut[index] = WATER;
                                } else {
                                    blocksOut[index] = 0;  // Air
                                }
                            }

                            dX0 += ddX0;
                            dX1 += ddX1;
                        }

                        d00 += dd00;
                        d01 += dd01;
                        d10 += dd10;
                        d11 += dd11;
                    }
                }
            }
        }
    }

    // ─── Chunk seeding ───

    // Java: provideChunk seed
    static int64_t getChunkSeed(int32_t chunkX, int32_t chunkZ) {
        return static_cast<int64_t>(chunkX) * 341873128712LL +
               static_cast<int64_t>(chunkZ) * 132897987541LL;
    }

    // Java: populate seed
    struct PopulateSeed {
        int64_t seed;
    };

    static PopulateSeed getPopulateSeed(int64_t worldSeed, int32_t chunkX, int32_t chunkZ) {
        // Java: rand.setSeed(seed); l = rand.nextLong()/2*2+1; l2 = rand.nextLong()/2*2+1
        // Then: rand.setSeed(chunkX * l + chunkZ * l2 ^ seed)
        // We precompute l and l2 from worldSeed
        RNG rng;
        rng.setSeed(worldSeed);
        int64_t l = rng.nextLong() / 2 * 2 + 1;
        int64_t l2 = rng.nextLong() / 2 * 2 + 1;
        return {static_cast<int64_t>(chunkX) * l +
                static_cast<int64_t>(chunkZ) * l2 ^ worldSeed};
    }

    // ─── Populate configuration ───

    struct PopulateConfig {
        bool isDesert;           // Skip water lakes in desert
        bool hasVillage;         // Skip lakes near villages
        bool mapFeaturesEnabled;
    };

    struct PopulateAction {
        enum Type { WATER_LAKE, LAVA_LAKE, DUNGEON, ICE_BLOCK, SNOW_BLOCK };
        Type type;
        int32_t x, y, z;
    };

    // Generate populate actions for a chunk
    static std::vector<PopulateAction> getPopulateActions(
            int32_t chunkX, int32_t chunkZ,
            const PopulateConfig& config, RNG& rng) {

        std::vector<PopulateAction> actions;
        int32_t baseX = chunkX * 16;
        int32_t baseZ = chunkZ * 16;

        // Water lakes: 1/4 chance (not in desert, not near villages)
        if (!config.isDesert && !config.hasVillage && rng.nextInt(4) == 0) {
            int32_t x = baseX + rng.nextInt(16) + 8;
            int32_t y = rng.nextInt(256);
            int32_t z = baseZ + rng.nextInt(16) + 8;
            actions.push_back({PopulateAction::WATER_LAKE, x, y, z});
        }

        // Lava lakes: 1/8 chance (below y=63 or 1/10 above)
        if (!config.hasVillage && rng.nextInt(8) == 0) {
            int32_t x = baseX + rng.nextInt(16) + 8;
            int32_t y = rng.nextInt(rng.nextInt(248) + 8);
            int32_t z = baseZ + rng.nextInt(16) + 8;
            if (y < SEA_LEVEL || rng.nextInt(10) == 0) {
                actions.push_back({PopulateAction::LAVA_LAKE, x, y, z});
            }
        }

        // Dungeons: 8 attempts
        for (int32_t i = 0; i < 8; ++i) {
            int32_t x = baseX + rng.nextInt(16) + 8;
            int32_t y = rng.nextInt(256);
            int32_t z = baseZ + rng.nextInt(16) + 8;
            actions.push_back({PopulateAction::DUNGEON, x, y, z});
        }

        return actions;
    }

    // ─── Noise parameters (constants from Java) ───

    struct NoiseParams {
        // field_147429_l (3D interpolation noise)
        static constexpr double INTERP_XZ = 8.555150000000001;
        static constexpr double INTERP_Y = 4.277575000000001;
        // field_147431_j, field_147432_k (3D density noise)
        static constexpr double DENSITY_SCALE = 684.412;
        // noiseGen6 (depth variation)
        static constexpr double DEPTH_XZ = 200.0;
        static constexpr double DEPTH_Y = 0.5;
        // field_147430_m (surface stone noise)
        static constexpr double SURFACE_SCALE = 0.03125;
    };

    const Config& getConfig() const { return config_; }

private:
    // Java LCG RNG
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
        int64_t nextLong() {
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t hi = static_cast<int32_t>(seed >> 16);
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t lo = static_cast<int32_t>(seed >> 16);
            return (static_cast<int64_t>(hi) << 32) | (static_cast<int64_t>(lo) & 0xFFFFFFFFL);
        }
    };

    Config config_{};
    std::array<float, 25> parabolicField_{};
};

} // namespace mccpp
