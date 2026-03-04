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
#include "worldgen/MapGenMineshaft.h"
#include "worldgen/MapGenScatteredFeature.h"
#include "worldgen/MapGenStronghold.h"
#include "worldgen/WorldGenOre.h"
#include "worldgen/WorldGenTrees.h"

#include <array>
#include <cstring>
#include <random>

namespace mccpp {

class OverworldGenerator : public IChunkGenerator {
public:
    explicit OverworldGenerator(int64_t seed) : seed_(seed), mineshaftGen_(seed), scatteredGen_(seed), strongholdGen_(seed) {
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

        // ── Step 6.5: Mineshaft generation ──
        // Java reference: MapGenMineshaft — abandoned mine corridors
        mineshaftGen_.generate(seed_, chunkX, chunkZ, blocks.data(), meta.data(),
                              chunk->pendingSpawners);

        // ── Step 6.6: Scattered feature generation (desert/jungle pyramids, swamp huts) ──
        scatteredGen_.generate(seed_, chunkX, chunkZ, blocks.data(), meta.data(),
                              chunk->pendingSpawners, chunk->pendingChests);

        // ── Step 6.7: Stronghold generation ──
        // Java reference: MapGenStronghold + StructureStrongholdPieces
        strongholdGen_.generate(seed_, chunkX, chunkZ, blocks.data(), meta.data(),
                               chunk->pendingSpawners, chunk->pendingChests);

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

        // ── Step 8.5: Dungeon generation ──
        // Java reference: net.minecraft.world.gen.feature.WorldGenDungeons
        // 8 attempts per chunk at random positions Y:2-64
        {
            NoiseGeneratorImproved::RNG dungeonRng;
            dungeonRng.setSeed(seed_ ^ (static_cast<int64_t>(chunkX) * 341873128712LL +
                                         static_cast<int64_t>(chunkZ) * 132897987541LL + 777));

            for (int32_t attempt = 0; attempt < 8; ++attempt) {
                int32_t dx = dungeonRng.nextInt(16);
                int32_t dy = dungeonRng.nextInt(62) + 2;  // Y: 2-63
                int32_t dz = dungeonRng.nextInt(16);

                int32_t roomHalfX = dungeonRng.nextInt(2) + 2; // 2-3
                int32_t roomHalfZ = dungeonRng.nextInt(2) + 2; // 2-3
                int32_t roomHeight = 3;

                // Check: room bounds within chunk, enough solid walls + 1-5 openings
                // Simplified: only generate if center is in stone and below surface
                if (dx - roomHalfX - 1 < 0 || dx + roomHalfX + 1 > 15 ||
                    dz - roomHalfZ - 1 < 0 || dz + roomHalfZ + 1 > 15 ||
                    dy + roomHeight + 1 > 255) continue;

                // Count openings (walls at edge that have air)
                int32_t openings = 0;
                bool floorSolid = true;
                for (int32_t wx = dx - roomHalfX - 1; wx <= dx + roomHalfX + 1; ++wx) {
                    for (int32_t wy = dy - 1; wy <= dy + roomHeight + 1; ++wy) {
                        for (int32_t wz = dz - roomHalfZ - 1; wz <= dz + roomHalfZ + 1; ++wz) {
                            int32_t bid = blocks[(wx * 16 + wz) * 256 + wy];
                            if (wy == dy - 1 && bid == 0) floorSolid = false;
                            if (wx == dx - roomHalfX - 1 || wx == dx + roomHalfX + 1 ||
                                wz == dz - roomHalfZ - 1 || wz == dz + roomHalfZ + 1) {
                                if (wy == dy && bid == 0) {
                                    int32_t above = blocks[(wx * 16 + wz) * 256 + wy + 1];
                                    if (above == 0) ++openings;
                                }
                            }
                        }
                    }
                }
                if (!floorSolid || openings < 1 || openings > 5) continue;

                // Carve room: walls cobblestone/mossy, interior air
                for (int32_t wx = dx - roomHalfX - 1; wx <= dx + roomHalfX + 1; ++wx) {
                    for (int32_t wy = dy + roomHeight; wy >= dy - 1; --wy) {
                        for (int32_t wz = dz - roomHalfZ - 1; wz <= dz + roomHalfZ + 1; ++wz) {
                            bool isWall = (wx == dx - roomHalfX - 1 || wy == dy - 1 ||
                                          wz == dz - roomHalfZ - 1 || wx == dx + roomHalfX + 1 ||
                                          wy == dy + roomHeight + 1 || wz == dz + roomHalfZ + 1);
                            if (isWall) {
                                int32_t cur = blocks[(wx * 16 + wz) * 256 + wy];
                                if (cur == 0) continue; // Don't fill air walls
                                if (wy == dy - 1 && dungeonRng.nextInt(4) != 0) {
                                    blocks[(wx * 16 + wz) * 256 + wy] = MOSSY_COBBLESTONE;
                                } else {
                                    blocks[(wx * 16 + wz) * 256 + wy] = COBBLESTONE;
                                }
                            } else {
                                blocks[(wx * 16 + wz) * 256 + wy] = 0; // Air interior
                            }
                        }
                    }
                }
                // Place mob spawner at center (block 52)
                blocks[(dx * 16 + dz) * 256 + dy] = MOB_SPAWNER;

                // Pick random dungeon mob type — Java: WorldGenDungeons.func_76521_a()
                // Vanilla 1.7.10: { "Skeleton", "Zombie", "Zombie", "Spider" }
                // → 25% Skeleton, 50% Zombie, 25% Spider
                static const char* const dungeonMobs[] = {
                    "Skeleton", "Zombie", "Zombie", "Spider"
                };
                int32_t mobIdx = dungeonRng.nextInt(4);
                Chunk::SpawnerInfo spawnerInfo;
                spawnerInfo.x = chunkX * 16 + dx;
                spawnerInfo.y = dy;
                spawnerInfo.z = chunkZ * 16 + dz;
                spawnerInfo.entityId = dungeonMobs[mobIdx];
                chunk->pendingSpawners.push_back(std::move(spawnerInfo));
            }
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

                // Tree type selection: 1/6 oak, 1/6 birch, 1/6 spruce, 1/6 jungle, 1/6 acacia, 1/6 dark oak
                // Java ref: BiomeDecorator.getRandomWorldGenForTrees
                int32_t treeType = treeRng.nextInt(6);
                int32_t minHeight = 4;
                int32_t metaWood = 0;
                int32_t metaLeaves = 0;
                int32_t logBlockId = 17;    // default log
                int32_t leavesBlockId = 18; // default leaves
                if (treeType == 1) {
                    metaWood = 2;    // Birch log (meta 2)
                    metaLeaves = 2;  // Birch leaves (meta 2)
                    minHeight = 5;
                } else if (treeType == 2) {
                    metaWood = 1;    // Spruce log (meta 1)
                    metaLeaves = 1;  // Spruce leaves (meta 1)
                    minHeight = 6;
                } else if (treeType == 3) {
                    metaWood = 3;    // Jungle log (meta 3)
                    metaLeaves = 3;  // Jungle leaves (meta 3)
                    minHeight = 5;
                } else if (treeType == 4) {
                    logBlockId = 162;   // Log2 (acacia/dark oak)
                    leavesBlockId = 161; // Leaves2
                    metaWood = 0;    // Acacia log2 (meta 0)
                    metaLeaves = 0;  // Acacia leaves2 (meta 0)
                    minHeight = 5;
                } else if (treeType == 5) {
                    logBlockId = 162;   // Log2 (acacia/dark oak)
                    leavesBlockId = 161; // Leaves2
                    metaWood = 1;    // Dark oak log2 (meta 1)
                    metaLeaves = 1;  // Dark oak leaves2 (meta 1)
                    minHeight = 6;
                }

                auto placements = TreeGenerator::generateTree(
                    tx, ty, tz,
                    minHeight,
                    metaWood,
                    metaLeaves,
                    false, // no vines
                    treeRng,
                    getBlockForTree,
                    isReplaceable,
                    logBlockId,
                    leavesBlockId
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

        // ── Step 9.5: Flower, tallgrass, sugar cane decoration ──
        // Java reference: BiomeDecorator — flowersPerChunk, grassPerChunk, etc.
        {
            NoiseGeneratorImproved::RNG decoRng;
            decoRng.setSeed(seed_ ^ (static_cast<int64_t>(chunkX) * 6364136223846793005LL +
                                      static_cast<int64_t>(chunkZ) * 1442695040888963407LL));

            // Plains: flowersPerChunk=4, grassPerChunk=10
            // Flowers: dandelion (37) or poppy (38)
            for (int32_t f = 0; f < 4; ++f) {
                int32_t fx = decoRng.nextInt(16);
                int32_t fz = decoRng.nextInt(16);
                for (int32_t fy = 255; fy >= 1; --fy) {
                    int32_t below = blocks[(fx * 16 + fz) * 256 + fy - 1];
                    int32_t at = blocks[(fx * 16 + fz) * 256 + fy];
                    if (at == 0 && (below == GRASS || below == DIRT)) {
                        blocks[(fx * 16 + fz) * 256 + fy] = decoRng.nextInt(3) == 0 ? 38 : 37;
                        break;
                    }
                }
            }

            // Tallgrass patches: block 31, meta 1 (tall grass, not fern)
            for (int32_t g = 0; g < 10; ++g) {
                int32_t gx = decoRng.nextInt(16);
                int32_t gz = decoRng.nextInt(16);
                for (int32_t gy = 255; gy >= 1; --gy) {
                    int32_t below = blocks[(gx * 16 + gz) * 256 + gy - 1];
                    int32_t at = blocks[(gx * 16 + gz) * 256 + gy];
                    if (at == 0 && below == GRASS) {
                        blocks[(gx * 16 + gz) * 256 + gy] = 31; // Tallgrass
                        meta[(gx * 16 + gz) * 256 + gy] = 1;    // Type: tall grass
                        break;
                    }
                }
            }

            // Sugar cane: 1/4 chance per chunk, near water at Y=1+
            // Java: BiomeDecorator.reedsPerChunk=10 for swamp, 0 for plains
            // Simplified: 1 attempt per chunk
            if (decoRng.nextInt(4) == 0) {
                int32_t sx = decoRng.nextInt(16);
                int32_t sz = decoRng.nextInt(16);
                for (int32_t sy = 255; sy >= 2; --sy) {
                    int32_t below = blocks[(sx * 16 + sz) * 256 + sy - 1];
                    int32_t at = blocks[(sx * 16 + sz) * 256 + sy];
                    if (at == 0 && (below == GRASS || below == DIRT || below == SAND)) {
                        // Check adjacent water (simplified: check ±1 X/Z at same Y-1)
                        bool nearWater = false;
                        for (int32_t dx = -1; dx <= 1; dx += 2) {
                            int32_t nx = sx + dx;
                            if (nx >= 0 && nx < 16) {
                                int32_t adj = blocks[(nx * 16 + sz) * 256 + sy - 1];
                                if (adj == WATER || adj == 8) nearWater = true;
                            }
                        }
                        for (int32_t dz = -1; dz <= 1; dz += 2) {
                            int32_t nz = sz + dz;
                            if (nz >= 0 && nz < 16) {
                                int32_t adj = blocks[(sx * 16 + nz) * 256 + sy - 1];
                                if (adj == WATER || adj == 8) nearWater = true;
                            }
                        }
                        if (nearWater) {
                            blocks[(sx * 16 + sz) * 256 + sy] = 83; // Sugar cane
                            // Stack 1-2 extra
                            int32_t stackH = decoRng.nextInt(2) + 1;
                            for (int32_t h = 1; h <= stackH && sy + h < 256; ++h) {
                                if (blocks[(sx * 16 + sz) * 256 + sy + h] == 0) {
                                    blocks[(sx * 16 + sz) * 256 + sy + h] = 83;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }

        // ── Step 9.6: Lake generation (water pools) ──
        // Java reference: net.minecraft.world.gen.feature.WorldGenLakes
        // Simplified: ellipsoid carving method — 1/4 chance per chunk
        {
            NoiseGeneratorImproved::RNG lakeRng;
            lakeRng.setSeed(seed_ ^ (static_cast<int64_t>(chunkX) * 2345678901LL +
                                      static_cast<int64_t>(chunkZ) * 987654321LL));

            if (lakeRng.nextInt(4) == 0) {
                int32_t lx = lakeRng.nextInt(16);
                int32_t lz = lakeRng.nextInt(16);
                // Find surface Y
                int32_t ly = 0;
                for (int32_t y = 80; y >= 10; --y) {
                    if (blocks[(lx * 16 + lz) * 256 + y] != 0 &&
                        blocks[(lx * 16 + lz) * 256 + y] != WATER) {
                        ly = y;
                        break;
                    }
                }
                if (ly > 10 && ly < 80) {
                    ly -= 4; // Dig below surface
                    // Carve ellipsoid pool: 6×4×6 with water at bottom, air at top
                    int32_t radiusX = lakeRng.nextInt(2) + 3; // 3-4
                    int32_t radiusZ = lakeRng.nextInt(2) + 3; // 3-4
                    int32_t depth = 3;
                    // Check bounds
                    if (lx - radiusX >= 0 && lx + radiusX < 16 &&
                        lz - radiusZ >= 0 && lz + radiusZ < 16) {
                        for (int32_t dx = -radiusX; dx <= radiusX; ++dx) {
                            for (int32_t dz = -radiusZ; dz <= radiusZ; ++dz) {
                                // Ellipsoid test
                                double ex = static_cast<double>(dx) / radiusX;
                                double ez = static_cast<double>(dz) / radiusZ;
                                if (ex * ex + ez * ez > 1.0) continue;
                                int32_t bx = lx + dx;
                                int32_t bz = lz + dz;
                                for (int32_t dy = 0; dy < depth; ++dy) {
                                    int32_t by = ly + dy;
                                    if (by < 1 || by > 254) continue;
                                    if (dy < depth - 1) {
                                        blocks[(bx * 16 + bz) * 256 + by] = 9; // Still water
                                    } else {
                                        blocks[(bx * 16 + bz) * 256 + by] = 0; // Air above
                                    }
                                }
                                // Replace exposed dirt with grass
                                int32_t topY = ly + depth;
                                if (topY < 256 && blocks[(bx * 16 + bz) * 256 + topY] == DIRT) {
                                    blocks[(bx * 16 + bz) * 256 + topY] = GRASS;
                                }
                            }
                        }
                    }
                }
            }
        }

        // ── Step 9.7: Pumpkin patches ──
        // Java reference: net.minecraft.world.gen.feature.WorldGenPumpkin
        // 1/32 chance per chunk, place up to 10 pumpkins on grass
        {
            NoiseGeneratorImproved::RNG pumpkinRng;
            pumpkinRng.setSeed(seed_ ^ (static_cast<int64_t>(chunkX) * 123456789LL +
                                         static_cast<int64_t>(chunkZ) * 987654321LL + 42));

            if (pumpkinRng.nextInt(32) == 0) {
                int32_t px = pumpkinRng.nextInt(16);
                int32_t pz = pumpkinRng.nextInt(16);
                // Find surface
                for (int32_t py = 255; py >= 1; --py) {
                    int32_t below = blocks[(px * 16 + pz) * 256 + py - 1];
                    int32_t at = blocks[(px * 16 + pz) * 256 + py];
                    if (at == 0 && below == GRASS) {
                        blocks[(px * 16 + pz) * 256 + py] = 86; // Pumpkin
                        meta[(px * 16 + pz) * 256 + py] =
                            static_cast<uint8_t>(pumpkinRng.nextInt(4)); // Random facing
                        break;
                    }
                }
            }
        }

        // ── Step 9.8: Clay deposit generation ──
        // Java reference: BiomeDecorator/WorldGenClay — clay discs on river/lake bottoms
        // Simplified: 1/3 chance per chunk, place clay below water
        {
            NoiseGeneratorImproved::RNG clayRng;
            clayRng.setSeed(seed_ ^ (static_cast<int64_t>(chunkX) * 3141592653LL +
                                      static_cast<int64_t>(chunkZ) * 2718281828LL));

            if (clayRng.nextInt(3) == 0) {
                int32_t cx = clayRng.nextInt(16);
                int32_t cz = clayRng.nextInt(16);
                // Find underwater surface
                for (int32_t cy = 80; cy >= 2; --cy) {
                    int32_t at = blocks[(cx * 16 + cz) * 256 + cy];
                    int32_t below = blocks[(cx * 16 + cz) * 256 + cy - 1];
                    if ((at == WATER || at == 8) && (below == DIRT || below == SAND || below == 13)) {
                        // Place clay disc: radius 2
                        int32_t radius = clayRng.nextInt(2) + 1; // 1-2
                        for (int32_t dx = -radius; dx <= radius; ++dx) {
                            for (int32_t dz = -radius; dz <= radius; ++dz) {
                                if (dx * dx + dz * dz > radius * radius) continue;
                                int32_t px = cx + dx, pz = cz + dz;
                                if (px < 0 || px >= 16 || pz < 0 || pz >= 16) continue;
                                int32_t b = blocks[(px * 16 + pz) * 256 + cy - 1];
                                if (b == DIRT || b == SAND || b == 13) {
                                    blocks[(px * 16 + pz) * 256 + cy - 1] = 82; // Clay block
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }

        // ── Step 9.9: Mushroom generation ──
        // Java reference: BiomeDecorator — mushroomsPerChunk, WorldGenFlowers(Blocks.mushroom)
        // 1/4 chance per chunk, place 1-2 mushrooms
        {
            NoiseGeneratorImproved::RNG mushRng;
            mushRng.setSeed(seed_ ^ (static_cast<int64_t>(chunkX) * 8675309LL +
                                      static_cast<int64_t>(chunkZ) * 5551234LL));

            if (mushRng.nextInt(4) == 0) {
                int32_t count = mushRng.nextInt(2) + 1;
                for (int32_t m = 0; m < count; ++m) {
                    int32_t mx = mushRng.nextInt(16);
                    int32_t mz = mushRng.nextInt(16);
                    for (int32_t my = 60; my >= 1; --my) {
                        int32_t below = blocks[(mx * 16 + mz) * 256 + my - 1];
                        int32_t at = blocks[(mx * 16 + mz) * 256 + my];
                        if (at == 0 && (below == GRASS || below == DIRT || below == 110)) { // 110=mycelium
                            blocks[(mx * 16 + mz) * 256 + my] =
                                mushRng.nextInt(2) == 0 ? 39 : 40; // Brown or red mushroom
                            break;
                        }
                    }
                }
            }
        }

        // ── Step 9.10: Cactus generation ──
        // Java reference: BiomeDecorator cactiPerChunk + WorldGenCactus
        // 1/6 chance per chunk, 1-3 blocks tall on sand
        {
            NoiseGeneratorImproved::RNG cactusRng;
            cactusRng.setSeed(seed_ ^ (static_cast<int64_t>(chunkX) * 1234509876LL +
                                        static_cast<int64_t>(chunkZ) * 6789012345LL));

            if (cactusRng.nextInt(6) == 0) {
                int32_t cx = cactusRng.nextInt(16);
                int32_t cz = cactusRng.nextInt(16);
                for (int32_t cy = 80; cy >= 2; --cy) {
                    int32_t below = blocks[(cx * 16 + cz) * 256 + cy - 1];
                    int32_t at = blocks[(cx * 16 + cz) * 256 + cy];
                    if (at == 0 && below == SAND) {
                        // Check no adjacent blocks (cactus needs air on all sides)
                        bool canPlace = true;
                        if (cx > 0 && blocks[((cx-1) * 16 + cz) * 256 + cy] != 0) canPlace = false;
                        if (cx < 15 && blocks[((cx+1) * 16 + cz) * 256 + cy] != 0) canPlace = false;
                        if (cz > 0 && blocks[(cx * 16 + (cz-1)) * 256 + cy] != 0) canPlace = false;
                        if (cz < 15 && blocks[(cx * 16 + (cz+1)) * 256 + cy] != 0) canPlace = false;
                        if (canPlace) {
                            int32_t height = cactusRng.nextInt(3) + 1; // 1-3
                            for (int32_t h = 0; h < height && cy + h < 256; ++h) {
                                blocks[(cx * 16 + cz) * 256 + cy + h] = 81; // Cactus
                            }
                        }
                        break;
                    }
                }
            }
        }

        // ── Step 9.11: Dead bush generation ──
        // Java reference: BiomeDecorator deadBushPerChunk + WorldGenDeadBush
        // 1/8 chance per chunk, place dead bush (32) on sand
        {
            NoiseGeneratorImproved::RNG bushRng;
            bushRng.setSeed(seed_ ^ (static_cast<int64_t>(chunkX) * 2468013579LL +
                                      static_cast<int64_t>(chunkZ) * 1357924680LL));

            if (bushRng.nextInt(8) == 0) {
                int32_t bx = bushRng.nextInt(16);
                int32_t bz = bushRng.nextInt(16);
                for (int32_t by = 80; by >= 2; --by) {
                    int32_t below = blocks[(bx * 16 + bz) * 256 + by - 1];
                    int32_t at = blocks[(bx * 16 + bz) * 256 + by];
                    if (at == 0 && below == SAND) {
                        blocks[(bx * 16 + bz) * 256 + by] = 32; // Dead bush
                        break;
                    }
                }
            }
        }

        // ── Step 9.12: Vine generation on trees ──
        // Java ref: BiomeDecorator.generateVines + WorldGenVines
        // 1/3 chance per chunk, attaches to log faces, grows 1-3 down
        {
            NoiseGeneratorImproved::RNG vineRng;
            vineRng.setSeed(seed_ ^ (static_cast<int64_t>(chunkX) * 3579024681LL +
                                      static_cast<int64_t>(chunkZ) * 9753102468LL));

            if (vineRng.nextInt(3) == 0) {
                int32_t attempts = 3 + vineRng.nextInt(4); // 3-6 vines per chunk
                for (int32_t a = 0; a < attempts; ++a) {
                    int32_t vx = vineRng.nextInt(16);
                    int32_t vz = vineRng.nextInt(16);
                    for (int32_t vy = 80; vy >= 40; --vy) {
                        int32_t bl = blocks[(vx * 16 + vz) * 256 + vy];
                        if (bl == 17 || bl == 162) { // Log or log2
                            // Try each face: N,S,E,W
                            // Vine (106) meta: 1=S, 2=W, 4=N, 8=E
                            int32_t vineLen = vineRng.nextInt(3) + 1;
                            int32_t dx = 0, dz = 0;
                            uint8_t vineMeta = 0;
                            int32_t face = vineRng.nextInt(4);
                            if (face == 0) { dz = -1; vineMeta = 1; }
                            else if (face == 1) { dz = 1; vineMeta = 4; }
                            else if (face == 2) { dx = -1; vineMeta = 8; }
                            else { dx = 1; vineMeta = 2; }

                            int32_t nx = vx + dx, nz = vz + dz;
                            if (nx >= 0 && nx < 16 && nz >= 0 && nz < 16) {
                                for (int32_t vl = 0; vl < vineLen && vy - 1 - vl >= 0; ++vl) {
                                    int32_t py = vy - 1 - vl;
                                    if (blocks[(nx * 16 + nz) * 256 + py] == 0) {
                                        blocks[(nx * 16 + nz) * 256 + py] = 106; // Vine
                                        meta[(nx * 16 + nz) * 256 + py] = vineMeta;
                                    } else break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }

        // ── Step 9.13: Lily pad generation (swamp-like) ──
        // Java ref: BiomeDecorator.waterlilyPerChunk + WorldGenWaterlily
        // 1/4 chance, 1-3 lilypads on water (block 111 on water source 9)
        {
            NoiseGeneratorImproved::RNG lilyRng;
            lilyRng.setSeed(seed_ ^ (static_cast<int64_t>(chunkX) * 5318008LL +
                                      static_cast<int64_t>(chunkZ) * 1337420LL));
            if (lilyRng.nextInt(4) == 0) {
                int32_t count = 1 + lilyRng.nextInt(3);
                for (int32_t c = 0; c < count; ++c) {
                    int32_t lx = lilyRng.nextInt(16);
                    int32_t lz = lilyRng.nextInt(16);
                    // Find water surface: scan down for water, place lily above
                    for (int32_t ly = 70; ly >= 60; --ly) {
                        int32_t blockAt = blocks[(lx * 16 + lz) * 256 + ly];
                        int32_t blockAbove = (ly + 1 < 256) ?
                            blocks[(lx * 16 + lz) * 256 + ly + 1] : 0;
                        if ((blockAt == 9 || blockAt == 8) && blockAbove == 0) {
                            blocks[(lx * 16 + lz) * 256 + ly + 1] = 111; // Lily pad
                            break;
                        }
                    }
                }
            }
        }

        // ── Step 9.14: Gravel beach generation ──
        // Replace grass/dirt at water level edges with gravel/sand
        {
            for (int32_t bx = 0; bx < 16; ++bx) {
                for (int32_t bz = 0; bz < 16; ++bz) {
                    int32_t y62 = blocks[(bx * 16 + bz) * 256 + 62];
                    int32_t y63 = blocks[(bx * 16 + bz) * 256 + 63];
                    // If at Y=62 water, and Y=63 is grass/dirt → sand beach
                    if ((y62 == 8 || y62 == 9) && (y63 == 2 || y63 == 3)) {
                        blocks[(bx * 16 + bz) * 256 + 63] = SAND;
                        // Also replace Y=62 dirt with sand if below water
                        if (blocks[(bx * 16 + bz) * 256 + 61] == 3 ||
                            blocks[(bx * 16 + bz) * 256 + 61] == 2) {
                            blocks[(bx * 16 + bz) * 256 + 61] = SAND;
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
    MapGenCaves caveGen_;       // Cave carver
    MapGenMineshaft mineshaftGen_;  // Mineshaft structure generator
    MapGenScatteredFeature scatteredGen_;  // Scattered feature generator (temples, huts)
    MapGenStronghold strongholdGen_;  // Stronghold structure generator
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
    static constexpr int32_t COBBLESTONE = 4;
    static constexpr int32_t MOSSY_COBBLESTONE = 48;
    static constexpr int32_t MOB_SPAWNER = 52;

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
