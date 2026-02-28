/**
 * MapGenRavine.h — Ravine generation using worm-based carving.
 *
 * Java reference: net.minecraft.world.gen.MapGenRavine
 *
 * Algorithm:
 *   1. 1/50 chance per chunk to generate a ravine.
 *   2. Random start position: x/z within chunk, y = rand(rand(40)+8)+20
 *   3. Width = (rand*2 + rand) * 2, direction = rand*2π, pitch = (rand-0.5)/4
 *   4. Worm path: advances along direction with pitch, random walk on angles.
 *   5. Cross-section: elliptical with sin-shaped width profile.
 *      - Horizontal radius = 1.5 + sin(step/total * π) * width * rand[0.75,1.0]
 *      - Vertical radius = d8 * d4 (d4=3.0 aspect ratio for tall ravines)
 *   6. 256-entry width multiplier table (field_75046_d): randomized per Y level.
 *   7. Water occlusion: skip carving if water detected in carving area.
 *   8. Below y=10: replace with flowing lava. Above y=10: air.
 *   9. Grass blocks: replace dirt below with biome top block.
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

// Forward: JavaRandom from MapGenCaves
class MapGenRavine {
public:
    // Block IDs for carving
    static constexpr int32_t STONE = 1;
    static constexpr int32_t GRASS = 2;
    static constexpr int32_t DIRT = 3;
    static constexpr int32_t FLOWING_WATER = 8;
    static constexpr int32_t WATER = 9;
    static constexpr int32_t FLOWING_LAVA = 10;

    // Callback: (worldX, worldZ) → biome top block ID
    using GetBiomeTopBlockFn = std::function<int32_t(int32_t, int32_t)>;

    MapGenRavine() = default;

    // RNG — Java LCG
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
        int64_t nextLong() {
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t hi = static_cast<int32_t>(seed >> 16);
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t lo = static_cast<int32_t>(seed >> 16);
            return (static_cast<int64_t>(hi) << 32) | (static_cast<int64_t>(lo) & 0xFFFFFFFFL);
        }
    };

    // Java: func_151538_a — called per chunk in range
    // Returns block modifications for chunk at (targetChunkX, targetChunkZ)
    struct BlockMod {
        int32_t index;      // (x*16+z)*256+y within chunk column array
        int32_t blockId;    // New block ID (0=air, FLOWING_LAVA, biome top block)
    };

    std::vector<BlockMod> generateForChunk(
            int64_t worldSeed, int32_t sourceChunkX, int32_t sourceChunkZ,
            int32_t targetChunkX, int32_t targetChunkZ,
            const int32_t* chunkBlocks,  // (16×16×256) block IDs for target chunk
            GetBiomeTopBlockFn getBiomeTopBlock,
            int32_t range = 8) {

        std::vector<BlockMod> mods;

        // Seed RNG for this source chunk
        RNG rng;
        rng.setSeed(worldSeed);
        int64_t rx = rng.nextLong();
        int64_t rz = rng.nextLong();
        rng.setSeed(static_cast<int64_t>(sourceChunkX) * rx ^
                     static_cast<int64_t>(sourceChunkZ) * rz ^ worldSeed);

        // Java: 1/50 chance
        if (rng.nextInt(50) != 0) return mods;

        // Starting position
        double startX = sourceChunkX * 16 + rng.nextInt(16);
        double startY = rng.nextInt(rng.nextInt(40) + 8) + 20;
        double startZ = sourceChunkZ * 16 + rng.nextInt(16);

        // Direction and size
        float yaw = rng.nextFloat() * static_cast<float>(M_PI) * 2.0f;
        float pitch = (rng.nextFloat() - 0.5f) * 2.0f / 8.0f;
        float width = (rng.nextFloat() * 2.0f + rng.nextFloat()) * 2.0f;

        // Carve the ravine
        int64_t carveSeed = rng.nextLong();
        carveRavine(carveSeed, targetChunkX, targetChunkZ, chunkBlocks,
                     startX, startY, startZ, width, yaw, pitch,
                     0, 0, 3.0, getBiomeTopBlock, mods, range);

        return mods;
    }

private:
    // Java: func_151540_a — the core worm carving
    void carveRavine(int64_t seed, int32_t chunkX, int32_t chunkZ,
                       const int32_t* blocks,
                       double posX, double posY, double posZ,
                       float width, float yaw, float pitch,
                       int32_t startStep, int32_t totalSteps,
                       double verticalAspect,
                       GetBiomeTopBlockFn& getBiomeTopBlock,
                       std::vector<BlockMod>& mods,
                       int32_t range) {

        RNG rng;
        rng.setSeed(seed);

        double chunkCenterX = chunkX * 16 + 8;
        double chunkCenterZ = chunkZ * 16 + 8;

        float pitchDelta = 0.0f;
        float yawDelta = 0.0f;

        if (totalSteps <= 0) {
            int32_t maxSteps = range * 16 - 16;
            totalSteps = maxSteps - rng.nextInt(maxSteps / 4);
        }

        bool singlePass = false;
        if (startStep == -1) {
            startStep = totalSteps / 2;
            singlePass = true;
        }

        // Build width table (256 entries)
        float widthFactor = 1.0f;
        for (int32_t i = 0; i < 256; ++i) {
            if (i == 0 || rng.nextInt(3) == 0) {
                widthFactor = 1.0f + rng.nextFloat() * rng.nextFloat() * 1.0f;
            }
            widthTable_[i] = widthFactor * widthFactor;
        }

        // Worm loop
        for (int32_t step = startStep; step < totalSteps; ++step) {
            // Cross-section radii
            double hRadius = 1.5 + static_cast<double>(
                std::sin(static_cast<float>(step) * static_cast<float>(M_PI) / static_cast<float>(totalSteps)) * width * 1.0f);
            double vRadius = hRadius * verticalAspect;

            // Apply random variation
            hRadius *= static_cast<double>(rng.nextFloat()) * 0.25 + 0.75;
            vRadius *= static_cast<double>(rng.nextFloat()) * 0.25 + 0.75;

            // Advance position
            float cosPitch = std::cos(pitch);
            float sinPitch = std::sin(pitch);
            posX += static_cast<double>(std::cos(yaw) * cosPitch);
            posY += static_cast<double>(sinPitch);
            posZ += static_cast<double>(std::sin(yaw) * cosPitch);

            // Update angles
            pitch *= 0.7f;
            pitch += pitchDelta * 0.05f;
            yaw += yawDelta * 0.05f;
            pitchDelta *= 0.8f;
            yawDelta *= 0.5f;
            pitchDelta += (rng.nextFloat() - rng.nextFloat()) * rng.nextFloat() * 2.0f;
            yawDelta += (rng.nextFloat() - rng.nextFloat()) * rng.nextFloat() * 4.0f;

            if (singlePass || rng.nextInt(4) != 0) {
                // Distance check to chunk center
                double dx = posX - chunkCenterX;
                double dz = posZ - chunkCenterZ;
                double stepsLeft = totalSteps - step;
                double maxDist = width + 2.0f + 16.0f;
                if (dx * dx + dz * dz - stepsLeft * stepsLeft > maxDist * maxDist) return;

                // Bounds check
                if (posX < chunkCenterX - 16.0 - hRadius * 2.0 ||
                    posZ < chunkCenterZ - 16.0 - hRadius * 2.0 ||
                    posX > chunkCenterX + 16.0 + hRadius * 2.0 ||
                    posZ > chunkCenterZ + 16.0 + hRadius * 2.0) continue;

                // Compute block bounds within chunk
                int32_t minX = static_cast<int32_t>(std::floor(posX - hRadius)) - chunkX * 16 - 1;
                int32_t maxX = static_cast<int32_t>(std::floor(posX + hRadius)) - chunkX * 16 + 1;
                int32_t minY = static_cast<int32_t>(std::floor(posY - vRadius)) - 1;
                int32_t maxY = static_cast<int32_t>(std::floor(posY + vRadius)) + 1;
                int32_t minZ = static_cast<int32_t>(std::floor(posZ - hRadius)) - chunkZ * 16 - 1;
                int32_t maxZ = static_cast<int32_t>(std::floor(posZ + hRadius)) - chunkZ * 16 + 1;

                if (minX < 0) minX = 0;
                if (maxX > 16) maxX = 16;
                if (minY < 1) minY = 1;
                if (maxY > 248) maxY = 248;
                if (minZ < 0) minZ = 0;
                if (maxZ > 16) maxZ = 16;

                // Water occlusion check
                bool hasWater = false;
                for (int32_t bx = minX; !hasWater && bx < maxX; ++bx) {
                    for (int32_t bz = minZ; !hasWater && bz < maxZ; ++bz) {
                        for (int32_t by = maxY + 1; !hasWater && by >= minY - 1; --by) {
                            int32_t idx = (bx * 16 + bz) * 256 + by;
                            if (by < 0 || by >= 256) continue;
                            int32_t block = blocks[idx];
                            if (block == FLOWING_WATER || block == WATER) {
                                hasWater = true;
                            }
                            if (by != minY - 1 && bx != minX && bx != maxX - 1 &&
                                bz != minZ && bz != maxZ - 1) {
                                by = minY; // Skip interior
                            }
                        }
                    }
                }

                if (!hasWater) {
                    // Carve blocks
                    for (int32_t bx = minX; bx < maxX; ++bx) {
                        double nx = (static_cast<double>(bx + chunkX * 16) + 0.5 - posX) / hRadius;
                        for (int32_t bz = minZ; bz < maxZ; ++bz) {
                            double nz = (static_cast<double>(bz + chunkZ * 16) + 0.5 - posZ) / hRadius;
                            int32_t idx = (bx * 16 + bz) * 256 + maxY;
                            bool hadGrass = false;

                            if (nx * nx + nz * nz >= 1.0) continue;

                            for (int32_t by = maxY - 1; by >= minY; --by) {
                                double ny = (static_cast<double>(by) + 0.5 - posY) / vRadius;

                                if ((nx * nx + nz * nz) * static_cast<double>(widthTable_[by]) +
                                    ny * ny / 6.0 < 1.0) {
                                    int32_t block = blocks[idx];
                                    if (block == GRASS) hadGrass = true;

                                    if (block == STONE || block == DIRT || block == GRASS) {
                                        if (by < 10) {
                                            mods.push_back({idx, FLOWING_LAVA});
                                        } else {
                                            mods.push_back({idx, 0}); // Air
                                            if (hadGrass && blocks[idx - 1] == DIRT) {
                                                int32_t worldX = bx + chunkX * 16;
                                                int32_t worldZ = bz + chunkZ * 16;
                                                mods.push_back({idx - 1, getBiomeTopBlock(worldX, worldZ)});
                                            }
                                        }
                                    }
                                }
                                --idx;
                            }
                        }
                    }
                    if (singlePass) return;
                }
            }
        }
    }

    std::array<float, 256> widthTable_{};
};

} // namespace mccpp
