/**
 * OverworldGenerator.h — Overworld terrain generator implementing IChunkGenerator.
 *
 * Wires ChunkProviderGenerate's density pipeline with NoiseGeneratorOctaves
 * to produce full overworld terrain chunks with stone, water, and surface blocks.
 *
 * Java reference: net.minecraft.world.gen.ChunkProviderGenerate
 *
 * Pipeline:
 *   1. Initialize 7 noise octave generators from world seed
 *   2. Generate biome data (simplified: plains everywhere for now)
 *   3. Compute 5×33×5 density field via noise
 *   4. Trilinear interpolation to 16×256×16 block array
 *   5. Surface replacement: grass/dirt/sand based on biome
 *   6. Bedrock at y=0..4 (random), cave carving
 *
 * Thread safety: Each instance has its own noise state.
 */
#pragma once

#include "world/World.h"
#include "world/Chunk.h"
#include "block/Block.h"
#include "worldgen/ChunkProviderGenerate.h"
#include "worldgen/NoiseGeneratorOctaves.h"
#include "worldgen/MapGenCaves.h"
#include "worldgen/WorldGenOre.h"
#include "worldgen/WorldGenTrees.h"

#include <array>
#include <cstring>
#include <random>

namespace mccpp {

class OverworldGenerator : public IChunkGenerator {
public:
    explicit OverworldGenerator(int64_t seed) : seed_(seed) {
        ChunkProviderGenerate::Config cfg;
        cfg.worldSeed = seed;
        cfg.mapFeaturesEnabled = false;
        cfg.amplified = false;
        terrain_ = ChunkProviderGenerate(cfg);

        // Initialize noise generators — Java: ChunkProviderGenerate constructor
        // Same order as Java: 7 NoiseGeneratorOctaves created from same RNG sequence
        NoiseGeneratorImproved::RNG rng;
        rng.setSeed(seed);

        noiseGen1_ = std::make_unique<NoiseGeneratorOctaves>(rng, 16);  // field_147431_j (lower)
        noiseGen2_ = std::make_unique<NoiseGeneratorOctaves>(rng, 16);  // field_147432_k (upper)
        noiseGen3_ = std::make_unique<NoiseGeneratorOctaves>(rng, 8);   // field_147429_l (interp)
        noiseGen4_ = std::make_unique<NoiseGeneratorOctaves>(rng, 4);   // field_147435_p (surface)
        noiseGen5_ = std::make_unique<NoiseGeneratorOctaves>(rng, 4);   // field_147436_q (surface2)
        noiseGen6_ = std::make_unique<NoiseGeneratorOctaves>(rng, 10);  // field_147430_m (depth)
        noiseGen7_ = std::make_unique<NoiseGeneratorOctaves>(rng, 16);  // field_147433_n (stone)
    }

    std::unique_ptr<Chunk> provideChunk(int chunkX, int chunkZ) override {
        auto chunk = std::make_unique<Chunk>(chunkX, chunkZ);

        // Arrays for block IDs and metadata (XZY ordering: (x*16+z)*256+y)
        std::array<int32_t, 65536> blocks{};
        std::array<uint8_t, 65536> meta{};

        // ── Step 1: Biome data (simplified: all plains) ──
        // Java uses 10×10 biome grid for height blending
        std::array<ChunkProviderGenerate::BiomeData, 100> biomes10x10;
        for (auto& b : biomes10x10) {
            b.minHeight = 0.1f;   // Plains height
            b.maxHeight = 0.3f;   // Plains variation
            b.biomeId = 1;        // Plains biome ID
        }

        // ── Step 2: Generate noise for density field ──
        int32_t gridX = chunkX * 4;
        int32_t gridZ = chunkZ * 4;

        // Depth noise: 5×5
        std::array<double, 25> depthNoise{};
        noiseGen6_->generateNoiseOctaves2D(depthNoise.data(), gridX, gridZ,
            5, 5,
            ChunkProviderGenerate::NoiseParams::DEPTH_XZ,
            ChunkProviderGenerate::NoiseParams::DEPTH_XZ);

        // 3D noise: 5×33×5 = 825 values each
        constexpr int32_t NOISE_SIZE = 5 * 33 * 5;
        std::array<double, NOISE_SIZE> noiseLower{};
        std::array<double, NOISE_SIZE> noiseUpper{};
        std::array<double, NOISE_SIZE> noiseInterp{};

        double densityScale = ChunkProviderGenerate::NoiseParams::DENSITY_SCALE;
        noiseGen1_->generateNoiseOctaves(noiseLower.data(), gridX, 0, gridZ,
            5, 33, 5, densityScale, densityScale, densityScale);
        noiseGen2_->generateNoiseOctaves(noiseUpper.data(), gridX, 0, gridZ,
            5, 33, 5, densityScale, densityScale, densityScale);
        noiseGen3_->generateNoiseOctaves(noiseInterp.data(), gridX, 0, gridZ,
            5, 33, 5,
            ChunkProviderGenerate::NoiseParams::INTERP_XZ,
            ChunkProviderGenerate::NoiseParams::INTERP_Y,
            ChunkProviderGenerate::NoiseParams::INTERP_XZ);

        // ── Step 3: Compute density field ──
        std::array<double, NOISE_SIZE> densityField{};
        terrain_.computeDensityField(densityField.data(),
            gridX, 0, gridZ,
            biomes10x10.data(),
            depthNoise.data(),
            noiseLower.data(),
            noiseUpper.data(),
            noiseInterp.data());

        // ── Step 4: Trilinear interpolation to 16×256×16 ──
        terrain_.interpolateBlocks(densityField.data(), blocks.data());

        // ── Step 5: Surface replacement ──
        replaceBlocksForBiome(chunkX, chunkZ, blocks.data(), meta.data());

        // ── Step 6: Cave generation ──
        // Java reference: ChunkProviderGenerate.provideChunk → caveGenerator.generate
        caveGen_.generate(seed_, chunkX, chunkZ, blocks.data(),
            [](int32_t /*x*/, int32_t /*z*/) -> int32_t { return GRASS; });

        // ── Step 7: Bedrock ──
        placeBedrock(chunkX, chunkZ, blocks.data());

        // ── Step 8: Ore generation ──
        // Java reference: ChunkProviderGenerate.populate → BiomeDecorator.genStandardOre
        {
            OreVeinGenerator::RNG oreRng;
            oreRng.setSeed(seed_ + ChunkProviderGenerate::getChunkSeed(chunkX, chunkZ));

            auto getBlock = [&blocks](int32_t x, int32_t y, int32_t z) -> int32_t {
                if (x < 0 || x > 15 || z < 0 || z > 15 || y < 0 || y > 255) return 0;
                return blocks[(x * 16 + z) * 256 + y];
            };
            auto setBlock = [&blocks](int32_t x, int32_t y, int32_t z, int32_t id) {
                if (x < 0 || x > 15 || z < 0 || z > 15 || y < 0 || y > 255) return;
                blocks[(x * 16 + z) * 256 + y] = id;
            };

            OreDistribution::generateChunkOres(chunkX, chunkZ, oreRng, getBlock, setBlock);
        }

        // ── Step 9: Tree generation ──
        // Java reference: BiomeDecorator — plains has treesPerChunk = -1
        // meaning 0 trees + 1/10 chance of an extra tree
        {
            TreeGenerator::RNG treeRng;
            treeRng.setSeed(seed_ ^ (static_cast<int64_t>(chunkX) * 341873128712LL +
                                      static_cast<int64_t>(chunkZ) * 132897987541LL));

            int32_t treesPerChunk = 0;
            if (treeRng.nextInt(10) == 0) treesPerChunk = 1;

            auto getBlockForTree = [&blocks](int32_t x, int32_t y, int32_t z) -> int32_t {
                if (x < 0 || x > 15 || z < 0 || z > 15 || y < 0 || y > 255) return 0;
                return blocks[(x * 16 + z) * 256 + y];
            };
            auto isReplaceable = [](int32_t blockId) -> bool {
                return blockId == 0 || blockId == 18 || blockId == 161;  // air, leaves, leaves2
            };

            for (int32_t t = 0; t < treesPerChunk; ++t) {
                int32_t tx = treeRng.nextInt(16);
                int32_t tz = treeRng.nextInt(16);

                // Find surface Y
                int32_t ty = 0;
                for (int32_t y = 255; y >= 0; --y) {
                    int32_t bid = blocks[(tx * 16 + tz) * 256 + y];
                    if (bid != 0 && bid != 18 && bid != 161) {
                        ty = y + 1;
                        break;
                    }
                }

                auto placements = TreeGenerator::generateTree(
                    tx, ty, tz,
                    4,     // minTreeHeight (oak)
                    0,     // metaWood (oak)
                    0,     // metaLeaves (oak)
                    false, // no vines (standard oak)
                    treeRng,
                    getBlockForTree,
                    isReplaceable
                );

                // Apply placements within chunk bounds
                for (const auto& bp : placements) {
                    if (bp.x >= 0 && bp.x < 16 && bp.z >= 0 && bp.z < 16 &&
                        bp.y >= 0 && bp.y < 256) {
                        blocks[(bp.x * 16 + bp.z) * 256 + bp.y] = bp.blockId;
                        if (bp.meta != 0) {
                            meta[(bp.x * 16 + bp.z) * 256 + bp.y] = static_cast<uint8_t>(bp.meta);
                        }
                    }
                }
            }
        }

        // ── Step 10: Fill chunk sections ──
        for (int x = 0; x < 16; ++x) {
            for (int z = 0; z < 16; ++z) {
                for (int y = 0; y < 256; ++y) {
                    int32_t blockId = blocks[(x * 16 + z) * 256 + y];
                    if (blockId != 0) {
                        int sectionIdx = y >> 4;
                        if (!chunk->sections[sectionIdx]) {
                            chunk->sections[sectionIdx] = std::make_unique<ChunkSection>(sectionIdx);
                        }
                        int localY = y & 0xF;
                        chunk->sections[sectionIdx]->setBlock(x, localY, z, Block::getBlockById(blockId));
                        uint8_t m = meta[(x * 16 + z) * 256 + y];
                        if (m != 0) {
                            chunk->sections[sectionIdx]->setBlockMetadata(x, localY, z, m);
                        }
                    }
                }
            }
        }

        // Fill biome array with plains
        std::memset(chunk->biomes.data(), 1, chunk->biomes.size());

        // Height map (find highest non-air block per column)
        for (int x = 0; x < 16; ++x) {
            for (int z = 0; z < 16; ++z) {
                int highest = 0;
                for (int y = 255; y >= 0; --y) {
                    if (blocks[(x * 16 + z) * 256 + y] != 0) {
                        highest = y + 1;
                        break;
                    }
                }
                chunk->heightMap[z * 16 + x] = highest;
            }
        }

        return chunk;
    }

    std::string makeString() const override { return "RandomLevelSource"; }

private:
    int64_t seed_;
    ChunkProviderGenerate terrain_;
    MapGenCaves caveGen_;   // Cave carver
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen1_;  // lower density
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen2_;  // upper density
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen3_;  // interpolation
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen4_;  // surface
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen5_;  // surface2
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen6_;  // depth variation
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen7_;  // stone noise

    // Block IDs for surface replacement
    static constexpr int32_t GRASS = 2;
    static constexpr int32_t DIRT = 3;
    static constexpr int32_t BEDROCK = 7;
    static constexpr int32_t SAND = 12;
    static constexpr int32_t GRAVEL = 13;
    static constexpr int32_t SANDSTONE = 24;
    static constexpr int32_t STONE = 1;
    static constexpr int32_t WATER = 9;

    // Java: replaceBlocksForBiome — replace stone surface with grass/dirt/sand
    void replaceBlocksForBiome(int32_t chunkX, int32_t chunkZ,
                                int32_t* blocks, uint8_t* meta) {
        // Stone noise for surface variation
        std::array<double, 256> stoneNoise{};
        noiseGen4_->generateNoiseOctaves2D(stoneNoise.data(),
            chunkX * 16, chunkZ * 16, 16, 16,
            ChunkProviderGenerate::NoiseParams::SURFACE_SCALE * 2.0,
            ChunkProviderGenerate::NoiseParams::SURFACE_SCALE * 2.0);

        // Per-column surface replacement
        for (int32_t x = 0; x < 16; ++x) {
            for (int32_t z = 0; z < 16; ++z) {
                // Plains biome: grass on top, dirt below, sand near water
                int32_t topBlock = GRASS;
                int32_t fillerBlock = DIRT;
                int32_t noiseIdx = x * 16 + z;
                int32_t depth = -1;
                int32_t fillerDepth = static_cast<int32_t>(stoneNoise[noiseIdx] / 3.0 + 3.0 +
                    (static_cast<double>(rand()) / RAND_MAX) * 0.25);

                for (int32_t y = 255; y >= 0; --y) {
                    int32_t idx = (x * 16 + z) * 256 + y;

                    if (blocks[idx] == STONE) {
                        if (depth == -1) {
                            if (fillerDepth <= 0) {
                                topBlock = 0;  // Air
                                fillerBlock = STONE;
                            } else if (y >= 59 && y <= 64) {
                                // Near sea level: could be sand
                                topBlock = GRASS;
                                fillerBlock = DIRT;
                            }

                            if (y < 63 && topBlock == 0) {
                                topBlock = WATER;
                            }

                            depth = fillerDepth;
                            if (y >= 62) {
                                blocks[idx] = topBlock;
                            } else if (y >= 56) {
                                blocks[idx] = fillerBlock;
                            } else {
                                blocks[idx] = fillerBlock;
                                topBlock = 0;
                                fillerBlock = STONE;
                            }
                        } else if (depth > 0) {
                            --depth;
                            blocks[idx] = fillerBlock;

                            // Sandstone under sand
                            if (depth == 0 && fillerBlock == SAND) {
                                depth = static_cast<int32_t>((static_cast<double>(rand()) / RAND_MAX) * 2.0) + 1;
                                fillerBlock = SANDSTONE;
                            }
                        }
                    } else {
                        depth = -1;
                    }
                }
            }
        }
    }

    // Place bedrock at y=0..4 with decreasing probability
    void placeBedrock(int32_t chunkX, int32_t chunkZ, int32_t* blocks) {
        // Java LCG for deterministic bedrock
        NoiseGeneratorImproved::RNG rng;
        rng.setSeed(seed_ + ChunkProviderGenerate::getChunkSeed(chunkX, chunkZ));

        for (int32_t x = 0; x < 16; ++x) {
            for (int32_t z = 0; z < 16; ++z) {
                // y=0: always bedrock
                blocks[(x * 16 + z) * 256 + 0] = BEDROCK;
                // y=1..4: random bedrock
                for (int32_t y = 1; y <= 4; ++y) {
                    if (rng.nextInt(y + 1) == 0) {
                        blocks[(x * 16 + z) * 256 + y] = BEDROCK;
                    }
                }
            }
        }
    }
};

} // namespace mccpp
