/**
 * EndGenerator.h — End dimension terrain generator implementing IChunkGenerator.
 *
 * Java reference: net.minecraft.world.gen.ChunkProviderEnd (300 lines)
 *
 * Pipeline:
 *   1. Initialize 5 noise octave generators from world seed
 *   2. Compute 3×33×3 density field with End island profile
 *      → f3 = 100 - sqrt(gridX² + gridZ²) * 8, clamped [-100, 80]
 *   3. Trilinear interpolation to 16×128×16 block array
 *      → end_stone where density > 0
 *   4. Basic surface replacement (func_147421_b)
 *   5. Obsidian pillar decoration (WorldGenSpikes, 1/5 chance per chunk)
 *   6. End spawn platform (5×5 obsidian at origin, y=64)
 *   7. Fill chunk sections + biome (sky=9) + height map
 *
 * Thread safety: Each instance has its own noise state.
 */
#pragma once

#include "world/World.h"
#include "world/Chunk.h"
#include "block/Block.h"
#include "worldgen/NoiseGeneratorOctaves.h"

#include <array>
#include <cmath>
#include <cstring>

namespace mccpp {

class EndGenerator : public IChunkGenerator {
public:
    explicit EndGenerator(int64_t seed) : seed_(seed) {
        // Java: ChunkProviderEnd constructor — 5 NoiseGeneratorOctaves
        NoiseGeneratorImproved::RNG rng;
        rng.setSeed(seed);

        noiseGen1_ = std::make_unique<NoiseGeneratorOctaves>(rng, 16);  // 16 octaves
        noiseGen2_ = std::make_unique<NoiseGeneratorOctaves>(rng, 16);  // 16 octaves
        noiseGen3_ = std::make_unique<NoiseGeneratorOctaves>(rng, 8);   // 8 octaves
        noiseGen4_ = std::make_unique<NoiseGeneratorOctaves>(rng, 10);  // 10 octaves (depth)
        noiseGen5_ = std::make_unique<NoiseGeneratorOctaves>(rng, 16);  // 16 octaves (scale)
    }

    std::unique_ptr<Chunk> provideChunk(int chunkX, int chunkZ) override {
        auto chunk = std::make_unique<Chunk>(chunkX, chunkZ);

        // Java uses 128-high block array with (x*16+z)*128+y indexing
        // Block array: 32768 elements (16×16×128)
        // Java: Block[] blockArray = new Block[32768]
        std::array<int32_t, 32768> blocks{};

        // ── Per-chunk RNG (Java: endRNG.setSeed) ──
        NoiseGeneratorImproved::RNG endRNG;
        endRNG.setSeed(static_cast<int64_t>(chunkX) * 341873128712LL +
                       static_cast<int64_t>(chunkZ) * 132897987541LL);

        // ── Step 1: Generate 3D density field → fill end stone ──
        // Java: func_147420_a(n, n2, blockArray, biomesForGeneration)
        generateTerrain(chunkX, chunkZ, blocks.data());

        // ── Step 2: Surface replacement ──
        // Java: func_147421_b — replaces stone with end_stone (basically a no-op
        // since we already place end stone directly, included for parity)
        replaceSurface(chunkX, chunkZ, blocks.data());

        // ── Step 3: End obsidian pillar decoration ──
        // Java: ChunkProviderEnd.populate → BiomeEndDecorator.genDecorations
        //   → WorldGenSpikes.generate with 1/5 chance
        pendingCrystals_ = &chunk->pendingEnderCrystals;
        generateSpikes(chunkX, chunkZ, blocks.data(), endRNG);
        pendingCrystals_ = nullptr;

        // ── Step 4: End spawn platform (5×5 obsidian at origin y=64) ──
        generateSpawnPlatform(chunkX, chunkZ, blocks.data());

        // ── Step 5: Fill chunk sections ──
        for (int x = 0; x < 16; ++x) {
            for (int z = 0; z < 16; ++z) {
                for (int y = 0; y < 128; ++y) {
                    int32_t blockId = blocks[(x * 16 + z) * 128 + y];
                    if (blockId != 0) {
                        int sectionIdx = y >> 4;
                        if (!chunk->sections[sectionIdx]) {
                            chunk->sections[sectionIdx] = std::make_unique<ChunkSection>(sectionIdx << 4);
                        }
                        int localY = y & 0xF;
                        chunk->sections[sectionIdx]->setBlock(x, localY, z, Block::getBlockById(blockId));
                    }
                }
            }
        }

        // Biome array: End biome = 9 (sky)
        std::memset(chunk->biomes.data(), 9, chunk->biomes.size());

        // Height map
        for (int x = 0; x < 16; ++x) {
            for (int z = 0; z < 16; ++z) {
                int highest = 0;
                for (int y = 127; y >= 0; --y) {
                    if (blocks[(x * 16 + z) * 128 + y] != 0) {
                        highest = y + 1;
                        break;
                    }
                }
                chunk->heightMap[z * 16 + x] = highest;
            }
        }

        // Mark chunk as fully generated
        chunk->isTerrainPopulated = true;

        return chunk;
    }

    std::string makeString() const override { return "RandomLevelSource"; }

private:
    int64_t seed_;
    std::vector<Chunk::EnderCrystalInfo>* pendingCrystals_ = nullptr;  // Set during provideChunk for deferred crystal spawning

    // 5 noise generators matching Java ChunkProviderEnd constructor order
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen1_;  // 16 octaves
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen2_;  // 16 octaves
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen3_;  // 8 octaves
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen4_;  // 10 octaves (depth)
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen5_;  // 16 octaves (scale)

    // Block IDs
    static constexpr int32_t END_STONE = 121;
    static constexpr int32_t OBSIDIAN = 49;
    static constexpr int32_t BEDROCK = 7;

    // Noise frequency (Java: d = 684.412, doubled to 1368.824)
    static constexpr double BASE_FREQ = 684.412;

    // ═══════════════════════════════════════════════════════════════════════
    // Helper: get top solid block in column (local within blocks array)
    // Java: World.getTopSolidOrLiquidBlock
    // ═══════════════════════════════════════════════════════════════════════

    static int32_t getTopSolidLocal(const int32_t* blocks, int lx, int lz) {
        for (int y = 127; y >= 0; --y) {
            int32_t bId = blocks[(lx * 16 + lz) * 128 + y];
            if (bId != 0) return y + 1;
        }
        return 0;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // WorldGenSpikes — End obsidian pillar generation
    // Java reference: net.minecraft.world.gen.feature.WorldGenSpikes (58 lines)
    //
    // Called from BiomeEndDecorator.genDecorations with 1/5 chance per chunk.
    // Generates a cylindrical obsidian column at a random position with:
    //   - Height: 6-37 blocks (random.nextInt(32) + 6)
    //   - Radius: 1-4 blocks (random.nextInt(4) + 1)
    //   - Bedrock cap block
    //   - Ender Crystal entity on top (entity spawn deferred)
    //
    // Validation:
    //   - Position must be air with end_stone below
    //   - All blocks within radius at y-1 must be end_stone
    // ═══════════════════════════════════════════════════════════════════════

    void generateSpikes(int32_t chunkX, int32_t chunkZ, int32_t* blocks, NoiseGeneratorImproved::RNG& rng) {
        // Java: BiomeEndDecorator.genDecorations — 1/5 chance
        if (rng.nextInt(5) != 0) return;

        // Java: n = chunk_X + random.nextInt(16) + 8
        //        n2 = chunk_Z + random.nextInt(16) + 8
        // chunk_X/chunk_Z are already world coords (chunkX*16)
        int32_t worldX = chunkX * 16 + rng.nextInt(16) + 8;
        int32_t worldZ = chunkZ * 16 + rng.nextInt(16) + 8;

        // Convert to chunk-local coordinates
        int32_t localX = worldX & 15;
        int32_t localZ = worldZ & 15;

        // Get top solid block (Java: World.getTopSolidOrLiquidBlock)
        int32_t baseY = getTopSolidLocal(blocks, localX, localZ);
        if (baseY == 0) return;

        // Java: WorldGenSpikes.generate validation
        // 1. Position must be air: !world.isAirBlock(x, y, z)
        int32_t centerIdx = (localX * 16 + localZ) * 128 + baseY;
        if (baseY < 128 && blocks[centerIdx] != 0) return;

        // 2. Block below must be end_stone
        if (baseY == 0 || blocks[centerIdx - 1] != END_STONE) return;

        // Java: n8 = random.nextInt(32) + 6  (height)
        //        n9 = random.nextInt(4) + 1   (radius)
        int32_t height = rng.nextInt(32) + 6;
        int32_t radius = rng.nextInt(4) + 1;

        // Java validation: all blocks within radius at y-1 must be end_stone
        for (int32_t dx = -radius; dx <= radius; ++dx) {
            for (int32_t dz = -radius; dz <= radius; ++dz) {
                int32_t sx = localX + dx;
                int32_t sz = localZ + dz;
                if (sx < 0 || sx >= 16 || sz < 0 || sz >= 16) continue;
                if (dx * dx + dz * dz > radius * radius + 1) continue;
                // Java: world.getBlock(n7, n2 - 1, n6) == this.field_150520_a
                int32_t checkIdx = (sx * 16 + sz) * 128 + (baseY - 1);
                if (blocks[checkIdx] != END_STONE) return;
            }
        }

        // Generate obsidian cylinder
        for (int32_t y = baseY; y < baseY + height && y < 128; ++y) {
            for (int32_t dx = -radius; dx <= radius; ++dx) {
                for (int32_t dz = -radius; dz <= radius; ++dz) {
                    int32_t sx = localX + dx;
                    int32_t sz = localZ + dz;
                    if (sx < 0 || sx >= 16 || sz < 0 || sz >= 16) continue;
                    if (dx * dx + dz * dz > radius * radius + 1) continue;
                    blocks[(sx * 16 + sz) * 128 + y] = OBSIDIAN;
                }
            }
        }

        // Java: world.setBlock(n, n2 + n8, n3, Blocks.bedrock, 0, 2)
        // Bedrock cap at top of pillar
        int32_t capY = baseY + height;
        if (capY < 128) {
            blocks[(localX * 16 + localZ) * 128 + capY] = BEDROCK;
        }

        // Java: EntityEnderCrystal spawned at (n+0.5, n2+n8, n3+0.5)
        // Defer entity spawning to chunk load via pendingEnderCrystals
        if (capY < 128 && pendingCrystals_) {
            Chunk::EnderCrystalInfo info;
            info.x = worldX + 0.5;
            info.y = static_cast<double>(capY + 1);  // On top of bedrock cap
            info.z = worldZ + 0.5;
            pendingCrystals_->push_back(info);
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // End spawn platform — 5×5 obsidian platform at (0, 64, 0)
    // Java reference: net.minecraft.world.Teleporter
    //   → When a player teleports to The End, a 5×5 obsidian platform
    //     is generated at (0, 64, 0) with air clearance above.
    //
    // We generate this during chunk generation for the chunk containing
    // the origin (chunk 0,0). In Java this happens on teleport, but
    // pre-generating ensures the platform exists for server starts.
    // ═══════════════════════════════════════════════════════════════════════

    void generateSpawnPlatform(int32_t chunkX, int32_t chunkZ, int32_t* blocks) {
        // Platform is centered at world (0, 64, 0) = chunk (0, 0) local (0, 0)
        // Only the chunk at (0, 0) contains the platform center
        // Platform spans from (-2, 64, -2) to (2, 64, 2) in world coords
        // This means chunks (-1, -1), (-1, 0), (0, -1), (0, 0) could contain parts
        int32_t startWorldX = -2;
        int32_t endWorldX = 2;
        int32_t startWorldZ = -2;
        int32_t endWorldZ = 2;
        int32_t platformY = 64;

        // Check if this chunk intersects the platform area
        int32_t chunkStartX = chunkX * 16;
        int32_t chunkStartZ = chunkZ * 16;
        int32_t chunkEndX = chunkStartX + 15;
        int32_t chunkEndZ = chunkStartZ + 15;

        // No overlap check
        if (chunkEndX < startWorldX || chunkStartX > endWorldX) return;
        if (chunkEndZ < startWorldZ || chunkStartZ > endWorldZ) return;

        // Place obsidian platform and clear air above
        for (int32_t wx = startWorldX; wx <= endWorldX; ++wx) {
            for (int32_t wz = startWorldZ; wz <= endWorldZ; ++wz) {
                // Check if this world position is in this chunk
                if (wx < chunkStartX || wx > chunkEndX) continue;
                if (wz < chunkStartZ || wz > chunkEndZ) continue;

                int32_t lx = wx - chunkStartX;
                int32_t lz = wz - chunkStartZ;

                // Place obsidian at platform level
                blocks[(lx * 16 + lz) * 128 + platformY] = OBSIDIAN;

                // Clear 3 blocks of air above for player clearance
                for (int32_t y = platformY + 1; y <= platformY + 3 && y < 128; ++y) {
                    blocks[(lx * 16 + lz) * 128 + y] = 0;
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Java: func_147420_a — 3D density field to block array
    // Grid: 3×33×3 (n3=2 → n4=3, n5=33, n6=3)
    // Interpolation: 8×4×8 blocks per cell
    // ═══════════════════════════════════════════════════════════════════════

    void generateTerrain(int32_t chunkX, int32_t chunkZ, int32_t* blocks) {
        constexpr int32_t GRID_XZ = 2;     // Java: n3 = 2
        constexpr int32_t GRID_XZP = 3;    // n4 = n6 = GRID_XZ + 1 = 3
        constexpr int32_t GRID_Y = 33;     // Java: n5 = 33

        // Compute the density noise field
        std::array<double, GRID_XZP * GRID_Y * GRID_XZP> noiseField{};
        initializeNoiseField(noiseField.data(),
                             chunkX * GRID_XZ, 0, chunkZ * GRID_XZ,
                             GRID_XZP, GRID_Y, GRID_XZP);

        // Trilinear interpolation: each noise cell → 8×4×8 blocks
        // Java: i = x grid, j = z grid, k = y grid (0..31)
        for (int32_t xi = 0; xi < GRID_XZ; ++xi) {
            for (int32_t zi = 0; zi < GRID_XZ; ++zi) {
                for (int32_t yi = 0; yi < 32; ++yi) {
                    // Java: d = 0.25 (1/4 for 4 y-steps per cell)
                    double d = 0.25;

                    // Corner densities
                    // Java: densities[((i+0)*n6 + (j+0))*n5 + (k+0)]
                    double d00 = noiseField[((xi + 0) * GRID_XZP + (zi + 0)) * GRID_Y + (yi + 0)];
                    double d01 = noiseField[((xi + 0) * GRID_XZP + (zi + 1)) * GRID_Y + (yi + 0)];
                    double d10 = noiseField[((xi + 1) * GRID_XZP + (zi + 0)) * GRID_Y + (yi + 0)];
                    double d11 = noiseField[((xi + 1) * GRID_XZP + (zi + 1)) * GRID_Y + (yi + 0)];

                    // Y-gradient
                    double dy00 = (noiseField[((xi + 0) * GRID_XZP + (zi + 0)) * GRID_Y + (yi + 1)] - d00) * d;
                    double dy01 = (noiseField[((xi + 0) * GRID_XZP + (zi + 1)) * GRID_Y + (yi + 1)] - d01) * d;
                    double dy10 = (noiseField[((xi + 1) * GRID_XZP + (zi + 0)) * GRID_Y + (yi + 1)] - d10) * d;
                    double dy11 = (noiseField[((xi + 1) * GRID_XZP + (zi + 1)) * GRID_Y + (yi + 1)] - d11) * d;

                    for (int32_t dy = 0; dy < 4; ++dy) {
                        // Java: d10 = 0.125 (1/8 for 8 x-steps per cell)
                        double xStep = 0.125;
                        double val00 = d00;
                        double val01 = d01;
                        double dx0 = (d10 - d00) * xStep;
                        double dx1 = (d11 - d01) * xStep;

                        for (int32_t dxi = 0; dxi < 8; ++dxi) {
                            // Java: n7 = i3 + i * 8 << 11 | 0 + j * 8 << 7 | k * 4 + i2
                            // This is (x << 11 | z << 7 | y) = (x*2048 + z*128 + y)
                            // Same as (x*16 + z)*128 + y when 16 z-values map to 128 stride
                            // Actually: x << 11 = x * 2048, z << 7 = z * 128
                            // n7 = (dxi + xi*8) * 2048 | (0 + zi*8) * 128 | (yi*4 + dy)
                            int32_t bx = dxi + xi * 8;
                            int32_t by = yi * 4 + dy;

                            // Java: d15 = 0.125 (1/8 for 8 z-steps per cell)
                            double zStep = 0.125;
                            double density = val00;
                            double dz = (val01 - val00) * zStep;

                            for (int32_t dzi = 0; dzi < 8; ++dzi) {
                                int32_t bz = dzi + zi * 8;

                                // Java: n7 indexing uses << 11 and << 7
                                // n7 = bx << 11 | bz << 7 | by
                                // = bx * 2048 + bz * 128 + by
                                // But our array uses (x*16+z)*128+y
                                // Java's n7 = i3 + i*8 << 11 | 0 + j*8 << 7 | k*4+i2
                                // where i3 goes 0..7 (x inner), j*8+i4 (z inner 0..7)
                                // n7 increments by n8=128 per z step
                                // So it's: (bx * 16 + bz) * 128 + by
                                // Wait: bx << 11 = bx * 2048, 2048/128 = 16 z-slots. Yes.
                                int32_t idx = (bx * 16 + bz) * 128 + by;

                                int32_t blockId = 0; // null/air
                                if (density > 0.0) {
                                    blockId = END_STONE;
                                }
                                blocks[idx] = blockId;

                                density += dz;
                            }

                            val00 += dx0;
                            val01 += dx1;
                        }

                        d00 += dy00;
                        d01 += dy01;
                        d10 += dy10;
                        d11 += dy11;
                    }
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Java: initializeNoiseField — compute density values
    // End-specific: island shape via sqrt(x²+z²)*8 clamping
    // ═══════════════════════════════════════════════════════════════════════

    void initializeNoiseField(double* field, int32_t x, int32_t y, int32_t z,
                              int32_t xSize, int32_t ySize, int32_t zSize) {
        // Java: d = 684.412, d *= 2.0 → 1368.824
        double d = BASE_FREQ * 2.0;   // 1368.824
        double d2 = BASE_FREQ;        // 684.412

        // noiseGen4 (10 oct): depth 2D, freq = 1.121, 1.121
        std::vector<double> noiseData4(xSize * zSize);
        noiseGen4_->generateNoiseOctaves2D(noiseData4.data(), x, z,
            xSize, zSize, 1.121, 1.121);

        // noiseGen5 (16 oct): scale 2D, freq = 200.0, 200.0
        std::vector<double> noiseData5(xSize * zSize);
        noiseGen5_->generateNoiseOctaves2D(noiseData5.data(), x, z,
            xSize, zSize, 200.0, 200.0);

        int32_t total = xSize * ySize * zSize;

        // noiseGen3 (8 oct): interpolation 3D, freq = d/80, d2/160, d/80
        std::vector<double> noiseData1(total);
        noiseGen3_->generateNoiseOctaves(noiseData1.data(), x, y, z,
            xSize, ySize, zSize, d / 80.0, d2 / 160.0, d / 80.0);

        // noiseGen1 (16 oct): lower 3D, freq = d, d2, d
        std::vector<double> noiseData2(total);
        noiseGen1_->generateNoiseOctaves(noiseData2.data(), x, y, z,
            xSize, ySize, zSize, d, d2, d);

        // noiseGen2 (16 oct): upper 3D, freq = d, d2, d
        std::vector<double> noiseData3(total);
        noiseGen2_->generateNoiseOctaves(noiseData3.data(), x, y, z,
            xSize, ySize, zSize, d, d2, d);

        int32_t idx3d = 0;
        int32_t idx2d = 0;

        for (int32_t xi = 0; xi < xSize; ++xi) {
            for (int32_t zi = 0; zi < zSize; ++zi) {

                // Java: d4 = (noiseData4[n8] + 256.0) / 512.0, capped at 1.0
                double d4 = (noiseData4[idx2d] + 256.0) / 512.0;
                if (d4 > 1.0) d4 = 1.0;

                // Java: d3 = noiseData5[n8] / 8000.0
                double d3 = noiseData5[idx2d] / 8000.0;
                if (d3 < 0.0) d3 = -d3 * 0.3;
                d3 = d3 * 3.0 - 2.0;

                // Java: f = (float)(i + n - 0) / 1.0f  → grid world x
                //        f2 = (float)(j + n3 - 0) / 1.0f → grid world z
                //        f3 = 100.0f - sqrt(f*f + f2*f2) * 8.0f
                float f = static_cast<float>(xi + x);
                float f2 = static_cast<float>(zi + z);
                float f3 = 100.0f - std::sqrt(f * f + f2 * f2) * 8.0f;
                if (f3 > 80.0f) f3 = 80.0f;
                if (f3 < -100.0f) f3 = -100.0f;

                // Java: d3 clamped, then /8, then set to 0 (!)
                if (d3 > 1.0) d3 = 1.0;
                d3 /= 8.0;
                d3 = 0.0;  // Java literally sets d3 = 0.0 after all the computation

                if (d4 < 0.0) d4 = 0.0;
                d4 += 0.5;

                // Java: d3 = d3 * (double)n5 / 16.0; but d3 is 0, so this is 0
                d3 = d3 * static_cast<double>(ySize) / 16.0;

                ++idx2d;

                double d5 = static_cast<double>(ySize) / 2.0;

                for (int32_t yi = 0; yi < ySize; ++yi) {
                    double d7 = 0.0;

                    // Java: d8 = ((double)k - d5) * 8.0 / d4;
                    double d8 = (static_cast<double>(yi) - d5) * 8.0 / d4;
                    if (d8 < 0.0) d8 *= -1.0;

                    // Noise interpolation
                    double d9 = noiseData2[idx3d] / 512.0;
                    double d10 = noiseData3[idx3d] / 512.0;
                    double d11 = (noiseData1[idx3d] / 10.0 + 1.0) / 2.0;

                    if (d11 < 0.0) {
                        d7 = d9;
                    } else if (d11 > 1.0) {
                        d7 = d10;
                    } else {
                        d7 = d9 + (d10 - d9) * d11;
                    }

                    d7 -= 8.0;
                    d7 += static_cast<double>(f3);

                    // Top clamping: last 2 layers above center
                    // Java: n9 = 2; if (k > n5/2 - n9)
                    constexpr int32_t TOP_MARGIN = 2;
                    if (yi > ySize / 2 - TOP_MARGIN) {
                        double d6 = static_cast<double>(yi - (ySize / 2 - TOP_MARGIN)) / 64.0;
                        if (d6 < 0.0) d6 = 0.0;
                        if (d6 > 1.0) d6 = 1.0;
                        d7 = d7 * (1.0 - d6) + -3000.0 * d6;
                    }

                    // Bottom clamping: first 8 layers
                    // Java: n9 = 8; if (k < n9)
                    constexpr int32_t BOTTOM_MARGIN = 8;
                    if (yi < BOTTOM_MARGIN) {
                        double d6 = static_cast<double>(BOTTOM_MARGIN - yi) /
                                     (static_cast<double>(BOTTOM_MARGIN) - 1.0);
                        d7 = d7 * (1.0 - d6) + -30.0 * d6;
                    }

                    field[idx3d] = d7;
                    ++idx3d;
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Java: func_147421_b — Surface replacement
    // In The End, this replaces stone (1) with end_stone (121).
    // Since we already place end_stone directly, this is mostly a no-op.
    // Included for completeness/parity.
    // ═══════════════════════════════════════════════════════════════════════

    void replaceSurface(int32_t chunkX, int32_t chunkZ, int32_t* blocks) {
        // Java iterates columns (j=z, i=x), scans down, replaces stone→end_stone
        // Since our generateTerrain already places END_STONE directly,
        // this function has no effect. Kept for Java method parity.
        // The Java code checks for block3 == Blocks.stone, but we never place stone.
        (void)chunkX;
        (void)chunkZ;
        (void)blocks;
    }
};

} // namespace mccpp
