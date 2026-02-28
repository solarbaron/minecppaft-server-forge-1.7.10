/**
 * MapGenCaves.h — Cave generation using worm-based carving.
 *
 * Java references:
 *   - net.minecraft.world.gen.MapGenBase — Base structgen framework
 *   - net.minecraft.world.gen.MapGenCaves — Cave carving algorithm
 *
 * Algorithm:
 *   1. For each chunk in an 8-chunk radius, seed RNG from world seed + chunk coords.
 *   2. Roll for number of cave starts (nested rand(rand(rand(15)+1)+1), 1/7 chance to zero out).
 *   3. Each cave start spawns 1+ worms that march forward, carving ellipsoidal
 *      cross-sections. Worm direction changes randomly each step.
 *   4. At midpoint, worms can branch (±π/2 yaw split).
 *   5. Blocks are only carved if: stone/dirt/grass, no adjacent water,
 *      and y<10 gets lava instead of air.
 *   6. Grass blocks below carving become the biome's topBlock.
 *
 * Block array uses [x*16+z]*256+y indexing (same as Java).
 *
 * Thread safety: Each MapGenCaves instance is per-chunk-generation,
 * no shared mutable state.
 * JNI readiness: Simple arrays, predictable layout.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Block IDs for cave generation — matching vanilla 1.7.10
// ═══════════════════════════════════════════════════════════════════════════

namespace CaveBlocks {
    constexpr int32_t AIR = 0;
    constexpr int32_t STONE = 1;
    constexpr int32_t GRASS = 2;
    constexpr int32_t DIRT = 3;
    constexpr int32_t FLOWING_WATER = 8;
    constexpr int32_t WATER = 9;
    constexpr int32_t LAVA = 11;
}

// ═══════════════════════════════════════════════════════════════════════════
// Simple LCG RNG matching java.util.Random behavior.
// ═══════════════════════════════════════════════════════════════════════════

class JavaRandom {
public:
    JavaRandom() = default;
    explicit JavaRandom(int64_t seed) { setSeed(seed); }

    void setSeed(int64_t seed) {
        seed_ = (seed ^ 0x5DEECE66DLLU) & ((1LLU << 48) - 1);
    }

    int32_t nextInt() {
        return next(32);
    }

    int32_t nextInt(int32_t bound) {
        if (bound <= 0) return 0;
        // Java: same as (int)((bound * (long)next(31)) >> 31)
        int64_t r = (static_cast<int64_t>(bound) * static_cast<int64_t>(next(31))) >> 31;
        return static_cast<int32_t>(r);
    }

    int64_t nextLong() {
        return (static_cast<int64_t>(next(32)) << 32) + next(32);
    }

    float nextFloat() {
        return static_cast<float>(next(24)) / static_cast<float>(1 << 24);
    }

private:
    int32_t next(int32_t bits) {
        seed_ = (seed_ * 0x5DEECE66DLLU + 0xBLLU) & ((1LLU << 48) - 1);
        return static_cast<int32_t>(seed_ >> (48 - bits));
    }

    uint64_t seed_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// MapGenCaves — Cave generation algorithm.
// Java reference: net.minecraft.world.gen.MapGenCaves
// ═══════════════════════════════════════════════════════════════════════════

class MapGenCaves {
public:
    // Callback: (x, z) → biome top block ID for that column
    using BiomeTopBlockFn = std::function<int32_t(int32_t, int32_t)>;

    MapGenCaves() = default;

    // Java: generate — called for each chunk being generated
    // blockArray: int32_t[65536] indexed as (x*16+z)*256+y
    void generate(int64_t worldSeed, int32_t chunkX, int32_t chunkZ,
                    int32_t* blockArray, BiomeTopBlockFn biomeTopBlock) {
        biomeTopBlock_ = std::move(biomeTopBlock);

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
    // Java: func_151538_a (MapGenCaves override)
    void generateChunkCaves(int32_t cx, int32_t cz,
                              int32_t targetX, int32_t targetZ,
                              int32_t* blockArray) {
        // Java: rand(rand(rand(15)+1)+1)
        int32_t numCaves = rand_.nextInt(rand_.nextInt(rand_.nextInt(15) + 1) + 1);

        // Java: 1/7 chance to zero out
        if (rand_.nextInt(7) != 0) {
            numCaves = 0;
        }

        for (int32_t i = 0; i < numCaves; ++i) {
            double startX = static_cast<double>(cx * 16 + rand_.nextInt(16));
            double startY = static_cast<double>(rand_.nextInt(rand_.nextInt(120) + 8));
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
                float size = rand_.nextFloat() * 2.0f + rand_.nextFloat();

                // Java: 1/10 chance for extra-large cave
                if (rand_.nextInt(10) == 0) {
                    size *= rand_.nextFloat() * rand_.nextFloat() * 3.0f + 1.0f;
                }

                carveWorm(rand_.nextLong(), targetX, targetZ, blockArray,
                          startX, startY, startZ, size, yaw, pitch,
                          0, 0, 1.0);
            }
        }
    }

    // Java: func_151542_a — start a room (wider cave, then delegate to worm)
    void carveRoom(int64_t seed, int32_t targetX, int32_t targetZ,
                     int32_t* blockArray, double x, double y, double z) {
        carveWorm(seed, targetX, targetZ, blockArray, x, y, z,
                  1.0f + rand_.nextFloat() * 6.0f,
                  0.0f, 0.0f, -1, -1, 0.5);
    }

    // Java: func_151541_a — the core worm carving algorithm
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
            // Java: ellipsoid radius
            double radiusXZ = 1.5 + static_cast<double>(
                std::sin(static_cast<float>(step) * static_cast<float>(M_PI) / static_cast<float>(totalSteps)) *
                size * 1.0f);
            double radiusY = radiusXZ * yScale;

            // Advance position
            float cosPitch = std::cos(pitch);
            float sinPitch = std::sin(pitch);
            x += static_cast<double>(std::cos(yaw) * cosPitch);
            y += static_cast<double>(sinPitch);
            z += static_cast<double>(std::sin(yaw) * cosPitch);

            // Java: pitch damping (steep vs normal)
            pitch = steepCave ? (pitch * 0.92f) : (pitch * 0.7f);
            pitch += pitchDelta * 0.1f;
            yaw += yawDelta * 0.1f;

            pitchDelta *= 0.9f;
            yawDelta *= 0.75f;
            pitchDelta += (wormRng.nextFloat() - wormRng.nextFloat()) * wormRng.nextFloat() * 2.0f;
            yawDelta += (wormRng.nextFloat() - wormRng.nextFloat()) * wormRng.nextFloat() * 4.0f;

            // Java: branching at midpoint
            if (!isRoom && step == branchPoint && size > 1.0f && totalSteps > 0) {
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

            // Java: skip some steps for rooms
            if (isRoom || wormRng.nextInt(4) != 0) {
                // Distance check to target chunk
                double dx = x - chunkCenterX;
                double dz = z - chunkCenterZ;
                double remaining = static_cast<double>(totalSteps - step);
                double maxReach = static_cast<double>(size) + 2.0 + 16.0;
                if (dx * dx + dz * dz - remaining * remaining > maxReach * maxReach) {
                    return;
                }

                // AABB check for chunk overlap
                if (x < chunkCenterX - 16.0 - radiusXZ * 2.0 ||
                    z < chunkCenterZ - 16.0 - radiusXZ * 2.0 ||
                    x > chunkCenterX + 16.0 + radiusXZ * 2.0 ||
                    z > chunkCenterZ + 16.0 + radiusXZ * 2.0) {
                    continue;
                }

                // Compute carving bounds within the target chunk
                int32_t minX = std::max(0, floorD(x - radiusXZ) - targetX * 16 - 1);
                int32_t maxX = std::min(16, floorD(x + radiusXZ) - targetX * 16 + 1);
                int32_t minY = std::max(1, floorD(y - radiusY) - 1);
                int32_t maxY = std::min(248, floorD(y + radiusY) + 1);
                int32_t minZ = std::max(0, floorD(z - radiusXZ) - targetZ * 16 - 1);
                int32_t maxZ = std::min(16, floorD(z + radiusXZ) - targetZ * 16 + 1);

                // Water check — abort if water adjacent
                bool hasWater = false;
                for (int32_t bx = minX; !hasWater && bx < maxX; ++bx) {
                    for (int32_t bz = minZ; !hasWater && bz < maxZ; ++bz) {
                        for (int32_t by = maxY + 1; !hasWater && by >= minY - 1; --by) {
                            int32_t idx = (bx * 16 + bz) * 256 + by;
                            if (by < 0 || by >= 256) continue;
                            int32_t block = blockArray[idx];
                            if (block == CaveBlocks::FLOWING_WATER || block == CaveBlocks::WATER) {
                                hasWater = true;
                            }
                            // Skip interior (only check borders)
                            if (by != minY - 1 && bx != minX && bx != maxX - 1 &&
                                bz != minZ && bz != maxZ - 1) {
                                by = minY;
                            }
                        }
                    }
                }

                if (!hasWater) {
                    // Carve blocks
                    for (int32_t bx = minX; bx < maxX; ++bx) {
                        double normX = (static_cast<double>(bx + targetX * 16) + 0.5 - x) / radiusXZ;

                        for (int32_t bz = minZ; bz < maxZ; ++bz) {
                            double normZ = (static_cast<double>(bz + targetZ * 16) + 0.5 - z) / radiusXZ;

                            if (normX * normX + normZ * normZ >= 1.0) continue;

                            int32_t idx = (bx * 16 + bz) * 256 + maxY;
                            bool hadGrass = false;

                            for (int32_t by = maxY - 1; by >= minY; --by) {
                                double normY = (static_cast<double>(by) + 0.5 - y) / radiusY;

                                if (normY > -0.7 &&
                                    normX * normX + normY * normY + normZ * normZ < 1.0) {
                                    int32_t block = blockArray[idx];

                                    if (block == CaveBlocks::GRASS) {
                                        hadGrass = true;
                                    }

                                    if (block == CaveBlocks::STONE ||
                                        block == CaveBlocks::DIRT ||
                                        block == CaveBlocks::GRASS) {
                                        if (by < 10) {
                                            blockArray[idx] = CaveBlocks::LAVA;
                                        } else {
                                            blockArray[idx] = CaveBlocks::AIR;
                                            // Java: grass below → biome topBlock
                                            if (hadGrass && blockArray[idx - 1] == CaveBlocks::DIRT) {
                                                if (biomeTopBlock_) {
                                                    blockArray[idx - 1] = biomeTopBlock_(
                                                        bx + targetX * 16, bz + targetZ * 16);
                                                } else {
                                                    blockArray[idx - 1] = CaveBlocks::GRASS;
                                                }
                                            }
                                        }
                                    }
                                }
                                --idx;
                            }
                        }
                    }

                    if (isRoom) break; // Room only carves once
                }
            }
        }
    }

    static int32_t floorD(double d) {
        int32_t i = static_cast<int32_t>(d);
        return d < static_cast<double>(i) ? i - 1 : i;
    }

    JavaRandom rand_;
    BiomeTopBlockFn biomeTopBlock_;
};

} // namespace mccpp
