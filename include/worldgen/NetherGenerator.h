/**
 * NetherGenerator.h — Nether terrain generator implementing IChunkGenerator.
 *
 * Java reference: net.minecraft.world.gen.ChunkProviderHell (415 lines)
 *
 * Pipeline:
 *   1. Initialize 7 noise octave generators from world seed
 *   2. Compute 5×17×5 density field with Nether cosine profile
 *   3. Trilinear interpolation to 16×128×16 block array
 *      → netherrack where density > 0, lava below y=32
 *   4. Surface replacement: soul sand + gravel at lava level, bedrock top/bottom
 *   5. Nether cave carving (MapGenCavesHell equivalent)
 *   6. Quartz ore, glowstone clusters, fire, mushroom, lava spring decoration
 *
 * Thread safety: Each instance has its own noise state.
 */
#pragma once

#include "world/World.h"
#include "world/Chunk.h"
#include "block/Block.h"
#include "worldgen/NoiseGeneratorOctaves.h"
#include "worldgen/MapGenNetherFortress.h"

#include <array>
#include <cmath>
#include <cstring>
#include <random>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// MapGenCavesHell — Nether cave generation.
// Java reference: net.minecraft.world.gen.MapGenCavesHell
//
// Differences from overworld MapGenCaves:
//   - 128-high block array indexing: (x*16+z)*128+y
//   - Max carving Y = 120 (not 248)
//   - Checks for lava adjacency (not water)
//   - Carves netherrack only (no grass→biome logic)
//   - rand(rand(rand(10)+1)+1) with 1/5 zero-out (not 1/7)
//   - Worm size *= 2.0 for wider tunnels
//   - yScale = 0.5 for flat caves
// ═══════════════════════════════════════════════════════════════════════════

class MapGenCavesHell {
public:
    MapGenCavesHell() = default;

    /**
     * Generate Nether caves for a target chunk.
     * blockArray: int32_t[32768] indexed as (x*16+z)*128+y (128-height)
     */
    void generate(int64_t worldSeed, int32_t chunkX, int32_t chunkZ,
                  int32_t* blockArray) {
        constexpr int32_t RANGE = 8;
        JavaRandom seedRng(worldSeed);
        int64_t seedXMul = seedRng.nextLong();
        int64_t seedZMul = seedRng.nextLong();

        for (int32_t cx = chunkX - RANGE; cx <= chunkX + RANGE; ++cx) {
            for (int32_t cz = chunkZ - RANGE; cz <= chunkZ + RANGE; ++cz) {
                int64_t chunkSeed = static_cast<int64_t>(cx) * seedXMul ^
                                    static_cast<int64_t>(cz) * seedZMul ^
                                    worldSeed;
                rand_.setSeed(chunkSeed);
                generateChunkCaves(cx, cz, chunkX, chunkZ, blockArray);
            }
        }
    }

private:
    static constexpr int32_t NETHERRACK = 87;
    static constexpr int32_t DIRT = 3;
    static constexpr int32_t GRASS = 2;
    static constexpr int32_t FLOWING_LAVA = 10;
    static constexpr int32_t LAVA = 11;

    // Java: func_151538_a — MapGenCavesHell override
    void generateChunkCaves(int32_t cx, int32_t cz,
                            int32_t targetX, int32_t targetZ,
                            int32_t* blockArray) {
        // Java: rand(rand(rand(10)+1)+1)
        int32_t numCaves = rand_.nextInt(rand_.nextInt(rand_.nextInt(10) + 1) + 1);

        // Java: 1/5 chance to zero out (vs 1/7 for overworld)
        if (rand_.nextInt(5) != 0) {
            numCaves = 0;
        }

        for (int32_t i = 0; i < numCaves; ++i) {
            double startX = static_cast<double>(cx * 16 + rand_.nextInt(16));
            double startY = static_cast<double>(rand_.nextInt(128));
            double startZ = static_cast<double>(cz * 16 + rand_.nextInt(16));
            int32_t numWorms = 1;

            // Java: 1/4 chance for a room + extra worms
            if (rand_.nextInt(4) == 0) {
                carveRoom(rand_.nextLong(), targetX, targetZ, blockArray,
                          startX, startY, startZ);
                numWorms += rand_.nextInt(4);
            }

            for (int32_t j = 0; j < numWorms; ++j) {
                float yaw = rand_.nextFloat() * static_cast<float>(M_PI) * 2.0f;
                float pitch = (rand_.nextFloat() - 0.5f) * 2.0f / 8.0f;
                // Java: size * 2.0f for Nether (wider tunnels)
                float size = rand_.nextFloat() * 2.0f + rand_.nextFloat();

                carveWorm(rand_.nextLong(), targetX, targetZ, blockArray,
                          startX, startY, startZ,
                          size * 2.0f, yaw, pitch, 0, 0, 0.5);
            }
        }
    }

    // Java: func_151544_a — room carving
    void carveRoom(int64_t seed, int32_t targetX, int32_t targetZ,
                   int32_t* blockArray, double x, double y, double z) {
        carveWorm(seed, targetX, targetZ, blockArray, x, y, z,
                  1.0f + rand_.nextFloat() * 6.0f,
                  0.0f, 0.0f, -1, -1, 0.5);
    }

    // Java: func_151543_a — core worm carving for Nether
    void carveWorm(int64_t seed, int32_t targetX, int32_t targetZ,
                   int32_t* blockArray,
                   double x, double y, double z,
                   float size, float yaw, float pitch,
                   int32_t startStep, int32_t totalSteps, double yScale) {

        double chunkCenterX = static_cast<double>(targetX * 16 + 8);
        double chunkCenterZ = static_cast<double>(targetZ * 16 + 8);
        float yawDelta = 0.0f;
        float pitchDelta = 0.0f;
        JavaRandom wormRng(seed);

        constexpr int32_t RANGE = 8;

        if (totalSteps <= 0) {
            int32_t maxLen = RANGE * 16 - 16;
            totalSteps = maxLen - wormRng.nextInt(maxLen / 4);
        }

        bool isRoom = false;
        if (startStep == -1) {
            startStep = totalSteps / 2;
            isRoom = true;
        }

        int32_t branchPoint = wormRng.nextInt(totalSteps / 2) + totalSteps / 4;
        bool steepCave = wormRng.nextInt(6) == 0;

        for (int32_t step = startStep; step < totalSteps; ++step) {
            double radiusXZ = 1.5 + static_cast<double>(
                std::sin(static_cast<float>(step) * static_cast<float>(M_PI) /
                         static_cast<float>(totalSteps)) * size * 1.0f);
            double radiusY = radiusXZ * yScale;

            float cosPitch = std::cos(pitch);
            float sinPitch = std::sin(pitch);
            x += static_cast<double>(std::cos(yaw) * cosPitch);
            y += static_cast<double>(sinPitch);
            z += static_cast<double>(std::sin(yaw) * cosPitch);

            pitch = steepCave ? (pitch * 0.92f) : (pitch * 0.7f);
            pitch += pitchDelta * 0.1f;
            yaw += yawDelta * 0.1f;

            pitchDelta *= 0.9f;
            yawDelta *= 0.75f;
            pitchDelta += (wormRng.nextFloat() - wormRng.nextFloat()) * wormRng.nextFloat() * 2.0f;
            yawDelta += (wormRng.nextFloat() - wormRng.nextFloat()) * wormRng.nextFloat() * 4.0f;

            // Branching at midpoint
            if (!isRoom && step == branchPoint && size > 1.0f) {
                carveWorm(wormRng.nextLong(), targetX, targetZ, blockArray,
                          x, y, z,
                          wormRng.nextFloat() * 0.5f + 0.5f,
                          yaw - 1.5707964f, pitch / 3.0f,
                          step, totalSteps, 1.0);
                carveWorm(wormRng.nextLong(), targetX, targetZ, blockArray,
                          x, y, z,
                          wormRng.nextFloat() * 0.5f + 0.5f,
                          yaw + 1.5707964f, pitch / 3.0f,
                          step, totalSteps, 1.0);
                return;
            }

            if (isRoom || wormRng.nextInt(4) != 0) {
                double dx = x - chunkCenterX;
                double dz = z - chunkCenterZ;
                double remaining = static_cast<double>(totalSteps - step);
                double maxReach = static_cast<double>(size) + 2.0 + 16.0;
                if (dx * dx + dz * dz - remaining * remaining > maxReach * maxReach) {
                    return;
                }

                if (x < chunkCenterX - 16.0 - radiusXZ * 2.0 ||
                    z < chunkCenterZ - 16.0 - radiusXZ * 2.0 ||
                    x > chunkCenterX + 16.0 + radiusXZ * 2.0 ||
                    z > chunkCenterZ + 16.0 + radiusXZ * 2.0) {
                    continue;
                }

                // Carving bounds within target chunk
                int32_t minX = std::max(0, floorD(x - radiusXZ) - targetX * 16 - 1);
                int32_t maxX = std::min(16, floorD(x + radiusXZ) - targetX * 16 + 1);
                // Java: maxY clamped to 120 for Nether (not 248)
                int32_t minY = std::max(1, floorD(y - radiusY) - 1);
                int32_t maxY = std::min(120, floorD(y + radiusY) + 1);
                int32_t minZ = std::max(0, floorD(z - radiusXZ) - targetZ * 16 - 1);
                int32_t maxZ = std::min(16, floorD(z + radiusXZ) - targetZ * 16 + 1);

                // Lava adjacency check (Nether checks lava, not water)
                bool hasLava = false;
                for (int32_t bx = minX; !hasLava && bx < maxX; ++bx) {
                    for (int32_t bz = minZ; !hasLava && bz < maxZ; ++bz) {
                        for (int32_t by = maxY + 1; !hasLava && by >= minY - 1; --by) {
                            // Java: (x*16+z)*128+y indexing
                            int32_t idx = (bx * 16 + bz) * 128 + by;
                            if (by < 0 || by >= 128) continue;
                            int32_t block = blockArray[idx];
                            if (block == FLOWING_LAVA || block == LAVA) {
                                hasLava = true;
                            }
                            if (by != minY - 1 && bx != minX && bx != maxX - 1 &&
                                bz != minZ && bz != maxZ - 1) {
                                by = minY;
                            }
                        }
                    }
                }

                if (!hasLava) {
                    for (int32_t bx = minX; bx < maxX; ++bx) {
                        double normX = (static_cast<double>(bx + targetX * 16) + 0.5 - x) / radiusXZ;

                        for (int32_t bz = minZ; bz < maxZ; ++bz) {
                            double normZ = (static_cast<double>(bz + targetZ * 16) + 0.5 - z) / radiusXZ;

                            if (normX * normX + normZ * normZ >= 1.0) continue;

                            // Java: (x*16+z)*128+maxY indexing
                            int32_t idx = (bx * 16 + bz) * 128 + maxY;

                            for (int32_t by = maxY - 1; by >= minY; --by) {
                                double normY = (static_cast<double>(by) + 0.5 - y) / radiusY;

                                if (normY > -0.7 &&
                                    normX * normX + normY * normY + normZ * normZ < 1.0) {
                                    int32_t block = blockArray[idx];
                                    // Java: carves netherrack, dirt, grass → air (null)
                                    if (block == NETHERRACK || block == DIRT || block == GRASS) {
                                        blockArray[idx] = 0; // Air
                                    }
                                }
                                --idx;
                            }
                        }
                    }

                    if (isRoom) break;
                }
            }
        }
    }

    static int32_t floorD(double d) {
        int32_t i = static_cast<int32_t>(d);
        return d < static_cast<double>(i) ? i - 1 : i;
    }

    JavaRandom rand_;
};

// ═══════════════════════════════════════════════════════════════════════════
// NetherGenerator — Nether terrain generator implementing IChunkGenerator.
// Java reference: net.minecraft.world.gen.ChunkProviderHell
//
// Uses:
//   - 7 noise octave generators from world seed
//   - 5×17×5 density field with Nether cosine profile
//   - 128-height block array (not 256)
//   - Lava sea at y=32
//   - Surface: soul sand + gravel bands, bedrock top/bottom
//   - Nether caves (MapGenCavesHell)
//   - Quartz ore, glowstone, fire, mushrooms, lava springs
// ═══════════════════════════════════════════════════════════════════════════

class NetherGenerator : public IChunkGenerator {
public:
    explicit NetherGenerator(int64_t seed) : seed_(seed) {
        // Java: ChunkProviderHell constructor — 7 NoiseGeneratorOctaves
        // Same RNG sequence as Java
        NoiseGeneratorImproved::RNG rng;
        rng.setSeed(seed);

        noiseGen1_ = std::make_unique<NoiseGeneratorOctaves>(rng, 16);  // netherNoiseGen1
        noiseGen2_ = std::make_unique<NoiseGeneratorOctaves>(rng, 16);  // netherNoiseGen2
        noiseGen3_ = std::make_unique<NoiseGeneratorOctaves>(rng, 8);   // netherNoiseGen3
        noiseGen4_ = std::make_unique<NoiseGeneratorOctaves>(rng, 4);   // slowsandGravelNoiseGen
        noiseGen5_ = std::make_unique<NoiseGeneratorOctaves>(rng, 4);   // netherrackExculsivityNoiseGen
        noiseGen6_ = std::make_unique<NoiseGeneratorOctaves>(rng, 10);  // netherNoiseGen6
        noiseGen7_ = std::make_unique<NoiseGeneratorOctaves>(rng, 16);  // netherNoiseGen7
    }

    std::unique_ptr<Chunk> provideChunk(int chunkX, int chunkZ) override {
        auto chunk = std::make_unique<Chunk>(chunkX, chunkZ);

        // Java uses 128-high block array with (x*16+z)*128+y indexing
        // We use 32768-element array for the generation, then convert to 256-high sections
        std::array<int32_t, 32768> blocks{};
        std::array<uint8_t, 32768> meta{};  // Block metadata (nether brick stairs, etc.)

        // Fortress structure outputs — spawners (blaze) and chests (corridor loot)
        std::vector<Chunk::SpawnerInfo> spawners;
        std::vector<Chunk::ChestInfo> chests;

        // ── RNG seeded per-chunk (Java: hellRNG.setSeed) ──
        JavaRandom hellRNG(static_cast<int64_t>(chunkX) * 341873128712LL +
                           static_cast<int64_t>(chunkZ) * 132897987541LL);

        // ── Step 1: Generate 3D density field → fill netherrack + lava ──
        generateTerrain(chunkX, chunkZ, blocks.data());

        // ── Step 2: Surface replacement (soul sand, gravel, bedrock) ──
        replaceSurface(chunkX, chunkZ, blocks.data(), hellRNG);

        // ── Step 3: Nether cave generation ──
        caveGen_.generate(seed_, chunkX, chunkZ, blocks.data());

        // ── Step 4: Nether Fortress structure generation ──
        // Java: ChunkProviderHell.provideChunk line 195:
        //   genNetherBridge.generate(this, worldObj, chunkX, chunkZ, blockArray)
        fortressGen_.generate(seed_, chunkX, chunkZ,
                              blocks.data(), meta.data(),
                              spawners, chests);

        // ── Step 5: Quartz ore ──
        generateQuartzOre(chunkX, chunkZ, blocks.data(), hellRNG);

        // ── Step 6: Glowstone, fire, mushrooms, lava springs ──
        generateFeatures(chunkX, chunkZ, blocks.data(), hellRNG);

        // ── Step 7: Fill chunk sections (convert 128-high to 256-high Chunk) ──
        for (int x = 0; x < 16; ++x) {
            for (int z = 0; z < 16; ++z) {
                for (int y = 0; y < 128; ++y) {
                    int idx = (x * 16 + z) * 128 + y;
                    int32_t blockId = blocks[idx];
                    if (blockId != 0) {
                        int sectionIdx = y >> 4;
                        if (!chunk->sections[sectionIdx]) {
                            chunk->sections[sectionIdx] = std::make_unique<ChunkSection>(sectionIdx << 4);
                        }
                        int localY = y & 0xF;
                        chunk->sections[sectionIdx]->setBlock(x, localY, z, Block::getBlockById(blockId));
                        // Apply metadata (nether brick stairs, fence orientation, etc.)
                        if (meta[idx] != 0) {
                            chunk->sections[sectionIdx]->setBlockMetadata(x, localY, z, meta[idx]);
                        }
                    }
                }
            }
        }

        // Propagate spawner/chest placements from fortress generation
        chunk->pendingSpawners = std::move(spawners);
        chunk->pendingChests = std::move(chests);

        // Biome array: Nether biome = 8 (hell)
        std::memset(chunk->biomes.data(), 8, chunk->biomes.size());

        // Height map (find highest non-air block per column)
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

    std::string makeString() const override { return "HellRandomLevelSource"; }

private:
    int64_t seed_;
    MapGenCavesHell caveGen_;
    MapGenNetherFortress fortressGen_;

    // 7 noise generators matching Java ChunkProviderHell constructor order
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen1_;  // 16 octaves
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen2_;  // 16 octaves
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen3_;  // 8 octaves
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen4_;  // 4 octaves (slowsand/gravel)
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen5_;  // 4 octaves (netherrack exclusivity)
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen6_;  // 10 octaves (depth)
    std::unique_ptr<NoiseGeneratorOctaves> noiseGen7_;  // 16 octaves (scale)

    // Block IDs
    static constexpr int32_t NETHERRACK = 87;
    static constexpr int32_t FLOWING_LAVA = 10;
    static constexpr int32_t LAVA = 11;
    static constexpr int32_t SOUL_SAND = 88;
    static constexpr int32_t GRAVEL = 13;
    static constexpr int32_t BEDROCK = 7;
    static constexpr int32_t QUARTZ_ORE = 153;
    static constexpr int32_t GLOWSTONE = 89;
    static constexpr int32_t FIRE = 51;
    static constexpr int32_t BROWN_MUSHROOM = 39;
    static constexpr int32_t RED_MUSHROOM = 40;
    static constexpr int32_t LAVA_SEA_LEVEL = 32;

    // Noise frequency constants (Java: base = 684.412, secondary = 2053.236)
    static constexpr double BASE_FREQ = 684.412;
    static constexpr double FREQ2 = 2053.236;

    // ═══════════════════════════════════════════════════════════════════════
    // Step 1: func_147419_a — 3D density field generation
    // Builds 5×17×5 noise field, interpolates to netherrack/lava
    // ═══════════════════════════════════════════════════════════════════════

    void generateTerrain(int32_t chunkX, int32_t chunkZ, int32_t* blocks) {
        constexpr int32_t GRID_X = 4;  // Java: n3 = 4
        constexpr int32_t GRID_Z = 4;
        constexpr int32_t GRID_XP = GRID_X + 1;  // 5
        constexpr int32_t GRID_Y = 17;            // Java: n6 = 17
        constexpr int32_t GRID_ZP = GRID_Z + 1;   // 5

        // Compute the density noise field
        std::array<double, GRID_XP * GRID_Y * GRID_ZP> noiseField{};
        initializeNoiseField(noiseField.data(), chunkX * GRID_X, 0, chunkZ * GRID_Z,
                             GRID_XP, GRID_Y, GRID_ZP);

        // Trilinear interpolation: each noise cell → 4×8×4 blocks
        for (int32_t xi = 0; xi < GRID_X; ++xi) {
            for (int32_t zi = 0; zi < GRID_X; ++zi) {
                for (int32_t yi = 0; yi < 16; ++yi) {
                    // Java: d = 0.125 (1/8 for 8 y-steps per cell)
                    double d = 0.125;

                    // Corner densities for this cell
                    double d00 = noiseField[((xi + 0) * GRID_ZP + (zi + 0)) * GRID_Y + (yi + 0)];
                    double d01 = noiseField[((xi + 0) * GRID_ZP + (zi + 1)) * GRID_Y + (yi + 0)];
                    double d10 = noiseField[((xi + 1) * GRID_ZP + (zi + 0)) * GRID_Y + (yi + 0)];
                    double d11 = noiseField[((xi + 1) * GRID_ZP + (zi + 1)) * GRID_Y + (yi + 0)];

                    // Y-gradient for each corner
                    double dy00 = (noiseField[((xi + 0) * GRID_ZP + (zi + 0)) * GRID_Y + (yi + 1)] - d00) * d;
                    double dy01 = (noiseField[((xi + 0) * GRID_ZP + (zi + 1)) * GRID_Y + (yi + 1)] - d01) * d;
                    double dy10 = (noiseField[((xi + 1) * GRID_ZP + (zi + 0)) * GRID_Y + (yi + 1)] - d10) * d;
                    double dy11 = (noiseField[((xi + 1) * GRID_ZP + (zi + 1)) * GRID_Y + (yi + 1)] - d11) * d;

                    for (int32_t dy = 0; dy < 8; ++dy) {
                        // Java: d10 = 0.25 (1/4 for 4 x-steps per cell)
                        double xStep = 0.25;
                        double val00 = d00;
                        double val01 = d01;
                        double dx0 = (d10 - d00) * xStep;
                        double dx1 = (d11 - d01) * xStep;

                        for (int32_t dxi = 0; dxi < 4; ++dxi) {
                            // Block index within 128-high array
                            // Java: n8 = dxi + xi * 4 << 11 | 0 + zi * 4 << 7 | yi * 8 + dy
                            // But with (x*16+z)*128+y indexing:
                            int32_t bx = dxi + xi * 4;
                            int32_t by = yi * 8 + dy;
                            // Java: n9 = 128 (stride for z increment)

                            double zStep = 0.25;
                            double density = val00;
                            double dz = (val01 - val00) * zStep;

                            for (int32_t dzi = 0; dzi < 4; ++dzi) {
                                int32_t bz = dzi + zi * 4;
                                int32_t idx = (bx * 16 + bz) * 128 + by;

                                int32_t blockId = 0; // air
                                if (by < LAVA_SEA_LEVEL) {
                                    blockId = LAVA; // flowing_lava
                                }
                                if (density > 0.0) {
                                    blockId = NETHERRACK;
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
    // Java: initializeNoiseField — compute density values for noise grid
    // ═══════════════════════════════════════════════════════════════════════

    void initializeNoiseField(double* field, int32_t x, int32_t y, int32_t z,
                              int32_t xSize, int32_t ySize, int32_t zSize) {
        int32_t total = xSize * ySize * zSize;

        // noiseGen6 (10 oct): depth variation, freq = 1.0, 0.0, 1.0
        std::vector<double> noiseData4(xSize * zSize);
        noiseGen6_->generateNoiseOctaves(noiseData4.data(), x, y, z,
            xSize, 1, zSize, 1.0, 0.0, 1.0);

        // noiseGen7 (16 oct): scale variation, freq = 100.0, 0.0, 100.0
        std::vector<double> noiseData5(xSize * zSize);
        noiseGen7_->generateNoiseOctaves(noiseData5.data(), x, y, z,
            xSize, 1, zSize, 100.0, 0.0, 100.0);

        // noiseGen3 (8 oct): interpolation, freq = BASE/80, FREQ2/60, BASE/80
        std::vector<double> noiseData1(total);
        noiseGen3_->generateNoiseOctaves(noiseData1.data(), x, y, z,
            xSize, ySize, zSize,
            BASE_FREQ / 80.0, FREQ2 / 60.0, BASE_FREQ / 80.0);

        // noiseGen1 (16 oct): lower, freq = BASE, FREQ2, BASE
        std::vector<double> noiseData2(total);
        noiseGen1_->generateNoiseOctaves(noiseData2.data(), x, y, z,
            xSize, ySize, zSize, BASE_FREQ, FREQ2, BASE_FREQ);

        // noiseGen2 (16 oct): upper, freq = BASE, FREQ2, BASE
        std::vector<double> noiseData3(total);
        noiseGen2_->generateNoiseOctaves(noiseData3.data(), x, y, z,
            xSize, ySize, zSize, BASE_FREQ, FREQ2, BASE_FREQ);

        // Nether cosine profile: cos(y*PI*6/height)*2 with cubic falloff
        std::vector<double> profile(ySize);
        for (int32_t yi = 0; yi < ySize; ++yi) {
            profile[yi] = std::cos(static_cast<double>(yi) * M_PI * 6.0 / static_cast<double>(ySize)) * 2.0;
            double dist = yi;
            if (yi > ySize / 2) dist = ySize - 1 - yi;
            if (dist < 4.0) {
                dist = 4.0 - dist;
                profile[yi] -= dist * dist * dist * 10.0;
            }
        }

        int32_t idx3d = 0;
        int32_t idx2d = 0;

        for (int32_t xi = 0; xi < xSize; ++xi) {
            for (int32_t zi = 0; zi < zSize; ++zi) {
                // Java: (noiseData4[idx2d] + 256.0) / 512.0, clamped to 1.0
                double depthScale = (noiseData4[idx2d] + 256.0) / 512.0;
                if (depthScale > 1.0) depthScale = 1.0;

                double d5 = 0.0;  // unused vertical offset in Nether

                double d6 = noiseData5[idx2d] / 8000.0;
                if (d6 < 0.0) d6 = -d6;
                d6 = d6 * 3.0 - 3.0;
                if (d6 < 0.0) {
                    d6 /= 2.0;
                    if (d6 < -1.0) d6 = -1.0;
                    d6 /= 1.4;
                    d6 /= 2.0;
                    depthScale = 0.0;
                } else {
                    if (d6 > 1.0) d6 = 1.0;
                    d6 /= 6.0;
                }

                depthScale += 0.5;
                d6 = d6 * static_cast<double>(ySize) / 16.0;

                ++idx2d;

                for (int32_t yi = 0; yi < ySize; ++yi) {
                    double density;
                    double profileVal = profile[yi];

                    double noise1 = noiseData2[idx3d] / 512.0;
                    double noise2 = noiseData3[idx3d] / 512.0;
                    double interp = (noiseData1[idx3d] / 10.0 + 1.0) / 2.0;

                    if (interp < 0.0) {
                        density = noise1;
                    } else if (interp > 1.0) {
                        density = noise2;
                    } else {
                        density = noise1 + (noise2 - noise1) * interp;
                    }

                    density -= profileVal;

                    // Top clamping: last 4 layers
                    if (yi > ySize - 4) {
                        double factor = static_cast<double>(yi - (ySize - 4)) / 3.0;
                        density = density * (1.0 - factor) + -10.0 * factor;
                    }

                    // Bottom clamping check (Java: if ((double)j < d5))
                    if (static_cast<double>(yi) < d5) {
                        double factor = (d5 - static_cast<double>(yi)) / 4.0;
                        if (factor < 0.0) factor = 0.0;
                        if (factor > 1.0) factor = 1.0;
                        density = density * (1.0 - factor) + -10.0 * factor;
                    }

                    field[idx3d] = density;
                    ++idx3d;
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Step 2: func_147418_b — Surface replacement
    // Places soul sand, gravel at lava level, bedrock top/bottom
    // ═══════════════════════════════════════════════════════════════════════

    void replaceSurface(int32_t chunkX, int32_t chunkZ,
                        int32_t* blocks, JavaRandom& rng) {
        constexpr int32_t SEA_LEVEL = 64;  // Java: n3 = 64 (but Nether uses 128 height)
        double scaleD = 0.03125;

        // Generate surface noise arrays (16×16)
        std::array<double, 256> slowsandNoise{};
        noiseGen4_->generateNoiseOctaves(slowsandNoise.data(),
            chunkX * 16, chunkZ * 16, 0, 16, 16, 1, scaleD, scaleD, 1.0);

        std::array<double, 256> gravelNoise{};
        noiseGen4_->generateNoiseOctaves(gravelNoise.data(),
            chunkX * 16, 109, chunkZ * 16, 16, 1, 16, scaleD, 1.0, scaleD);

        std::array<double, 256> exclusivityNoise{};
        noiseGen5_->generateNoiseOctaves(exclusivityNoise.data(),
            chunkX * 16, chunkZ * 16, 0, 16, 16, 1, scaleD * 2.0, scaleD * 2.0, scaleD * 2.0);

        for (int32_t x = 0; x < 16; ++x) {
            for (int32_t z = 0; z < 16; ++z) {
                bool useSoulSand = slowsandNoise[x + z * 16] +
                    static_cast<double>(rng.nextFloat()) * 0.2 > 0.0;    // Java: nextDouble
                bool useGravel = gravelNoise[x + z * 16] +
                    static_cast<double>(rng.nextFloat()) * 0.2 > 0.0;

                int32_t surfaceDepth = static_cast<int32_t>(
                    exclusivityNoise[x + z * 16] / 3.0 + 3.0 +
                    static_cast<double>(rng.nextFloat()) * 0.25);

                int32_t depth = -1;
                int32_t topBlock = NETHERRACK;
                int32_t fillerBlock = NETHERRACK;

                for (int32_t y = 127; y >= 0; --y) {
                    int32_t idx = (z * 16 + x) * 128 + y;  // Java uses (j*16+i)*128+k

                    // Bedrock at top and bottom
                    if (y >= 127 - rng.nextInt(5) || y <= 0 + rng.nextInt(5)) {
                        blocks[idx] = BEDROCK;
                        continue;
                    }

                    int32_t blockId = blocks[idx];
                    if (blockId == 0) {
                        depth = -1;
                        continue;
                    }

                    if (blockId != NETHERRACK) continue;

                    if (depth == -1) {
                        if (surfaceDepth <= 0) {
                            topBlock = 0;   // air
                            fillerBlock = NETHERRACK;
                        } else if (y >= SEA_LEVEL - 4 && y <= SEA_LEVEL + 1) {
                            topBlock = NETHERRACK;
                            fillerBlock = NETHERRACK;
                            if (useGravel) {
                                topBlock = GRAVEL;
                                fillerBlock = NETHERRACK;
                            }
                            if (useSoulSand) {
                                topBlock = SOUL_SAND;
                                fillerBlock = SOUL_SAND;
                            }
                        }

                        // If exposed to air below lava level → fill with lava
                        if (y < SEA_LEVEL && (topBlock == 0)) {
                            topBlock = LAVA;
                        }

                        depth = surfaceDepth;
                        if (y >= SEA_LEVEL - 1) {
                            blocks[idx] = topBlock;
                        } else {
                            blocks[idx] = fillerBlock;
                        }
                    } else if (depth > 0) {
                        --depth;
                        blocks[idx] = fillerBlock;
                    }
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Quartz ore generation
    // Java: populate — 16× WorldGenMinable(quartz_ore, 13, netherrack)
    // ═══════════════════════════════════════════════════════════════════════

    void generateQuartzOre(int32_t chunkX, int32_t chunkZ,
                           int32_t* blocks, JavaRandom& rng) {
        int32_t baseX = chunkX * 16;
        int32_t baseZ = chunkZ * 16;

        for (int32_t i = 0; i < 16; ++i) {
            int32_t x = rng.nextInt(16);
            int32_t y = rng.nextInt(108) + 10;  // y: 10-117
            int32_t z = rng.nextInt(16);

            // Simple quartz vein: place up to 13 blocks in a blob
            // Java: WorldGenMinable generates an ellipsoidal vein
            // Simplified: place ore in a small cluster around the center
            int32_t veinSize = 13;
            for (int32_t v = 0; v < veinSize; ++v) {
                int32_t dx = x + rng.nextInt(5) - 2;
                int32_t dy = y + rng.nextInt(5) - 2;
                int32_t dz = z + rng.nextInt(5) - 2;
                if (dx < 0 || dx >= 16 || dz < 0 || dz >= 16 || dy < 1 || dy >= 127) continue;
                int32_t idx = (dx * 16 + dz) * 128 + dy;
                if (blocks[idx] == NETHERRACK) {
                    blocks[idx] = QUARTZ_ORE;
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Feature generation: lava springs, fire, glowstone, mushrooms
    // Java: ChunkProviderHell.populate
    // ═══════════════════════════════════════════════════════════════════════

    void generateFeatures(int32_t chunkX, int32_t chunkZ,
                          int32_t* blocks, JavaRandom& rng) {
        int32_t baseX = chunkX * 16;
        int32_t baseZ = chunkZ * 16;

        // ── Glowstone clusters (type 2): 10× ──
        // Java: 10× WorldGenGlowStone2.generate
        // Places clusters of glowstone hanging from ceilings
        for (int32_t i = 0; i < 10; ++i) {
            int32_t x = rng.nextInt(16);
            int32_t y = rng.nextInt(128);
            int32_t z = rng.nextInt(16);

            // Only place on netherrack ceiling with air below
            if (y > 0 && y < 127) {
                int32_t idx = (x * 16 + z) * 128 + y;
                int32_t above = (y < 127) ? blocks[(x * 16 + z) * 128 + y + 1] : 0;
                if (blocks[idx] == 0 && above == NETHERRACK) {
                    // Place small glowstone cluster (3-5 blocks)
                    blocks[idx] = GLOWSTONE;
                    // Extend down/sideways
                    for (int32_t j = 0; j < 4; ++j) {
                        int32_t gx = x + rng.nextInt(3) - 1;
                        int32_t gy = y - rng.nextInt(2);
                        int32_t gz = z + rng.nextInt(3) - 1;
                        if (gx >= 0 && gx < 16 && gz >= 0 && gz < 16 && gy > 0 && gy < 128) {
                            int32_t gidx = (gx * 16 + gz) * 128 + gy;
                            if (blocks[gidx] == 0) {
                                // Check at least one neighbor is glowstone
                                bool hasNeighbor = false;
                                if (gx > 0 && blocks[((gx-1)*16+gz)*128+gy] == GLOWSTONE) hasNeighbor = true;
                                if (gx < 15 && blocks[((gx+1)*16+gz)*128+gy] == GLOWSTONE) hasNeighbor = true;
                                if (gz > 0 && blocks[(gx*16+(gz-1))*128+gy] == GLOWSTONE) hasNeighbor = true;
                                if (gz < 15 && blocks[(gx*16+(gz+1))*128+gy] == GLOWSTONE) hasNeighbor = true;
                                if (gy > 0 && blocks[(gx*16+gz)*128+gy-1] == GLOWSTONE) hasNeighbor = true;
                                if (gy < 127 && blocks[(gx*16+gz)*128+gy+1] == GLOWSTONE) hasNeighbor = true;
                                if (hasNeighbor) {
                                    blocks[gidx] = GLOWSTONE;
                                }
                            }
                        }
                    }
                }
            }
        }

        // ── Fire: rand(rand(10)+1)+1 clusters ──
        int32_t fireCount = rng.nextInt(rng.nextInt(10) + 1) + 1;
        for (int32_t i = 0; i < fireCount; ++i) {
            int32_t x = rng.nextInt(16);
            int32_t y = rng.nextInt(120) + 4;
            int32_t z = rng.nextInt(16);

            // Place fire on netherrack with air above
            if (y > 0 && y < 127) {
                int32_t idx = (x * 16 + z) * 128 + y;
                int32_t below = (y > 0) ? blocks[(x * 16 + z) * 128 + y - 1] : 0;
                if (blocks[idx] == 0 && below == NETHERRACK) {
                    blocks[idx] = FIRE;
                }
            }
        }

        // ── Brown mushroom: always 1 (rand(1)==0 is always true) ──
        {
            int32_t x = rng.nextInt(16);
            int32_t y = rng.nextInt(128);
            int32_t z = rng.nextInt(16);
            if (y > 0 && y < 127) {
                int32_t idx = (x * 16 + z) * 128 + y;
                int32_t below = blocks[(x * 16 + z) * 128 + y - 1];
                if (blocks[idx] == 0 && (below == NETHERRACK || below == SOUL_SAND)) {
                    blocks[idx] = BROWN_MUSHROOM;
                }
            }
        }

        // ── Red mushroom: always 1 ──
        {
            int32_t x = rng.nextInt(16);
            int32_t y = rng.nextInt(128);
            int32_t z = rng.nextInt(16);
            if (y > 0 && y < 127) {
                int32_t idx = (x * 16 + z) * 128 + y;
                int32_t below = blocks[(x * 16 + z) * 128 + y - 1];
                if (blocks[idx] == 0 && (below == NETHERRACK || below == SOUL_SAND)) {
                    blocks[idx] = RED_MUSHROOM;
                }
            }
        }
    }
};

} // namespace mccpp
