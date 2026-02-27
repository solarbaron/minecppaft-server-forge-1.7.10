#pragma once
// TerrainGenerator — world generation with height noise, biomes, ores, caves.
// Ported from acl.java (ChunkProviderGenerate) and related classes.
//
// Vanilla terrain generation pipeline:
//   1. Base terrain shape from layered Perlin noise (acr.java / NoiseGeneratorOctaves)
//   2. Biome selection from temperature/rainfall (ael.java / BiomeGenBase)
//   3. Surface decoration: grass/dirt/sand/sandstone per biome
//   4. Cave carving (abw.java / MapGenCaves)
//   5. Ore vein placement (abg.java / WorldGenMinable)
//   6. Structure generation (villages, mineshafts, dungeons)
//
// This implements a simplified but authentic terrain generator.

#include <cstdint>
#include <cmath>
#include <array>
#include <memory>
#include <random>
#include <algorithm>

#include "world/Chunk.h"
#include "world/TreeGenerator.h"
#include "world/StructureGenerator.h"

namespace mc {

// ---- Simplified Perlin noise (acr.java / NoiseGeneratorImproved) ----
class PerlinNoise {
public:
    PerlinNoise(uint64_t seed = 0) {
        std::mt19937_64 rng(seed);
        for (int i = 0; i < 256; ++i) perm_[i] = i;
        for (int i = 255; i > 0; --i) {
            int j = rng() % (i + 1);
            std::swap(perm_[i], perm_[j]);
        }
        for (int i = 0; i < 256; ++i) perm_[i + 256] = perm_[i];
    }

    double noise(double x, double y) const {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;
        x -= std::floor(x);
        y -= std::floor(y);
        double u = fade(x), v = fade(y);
        int A = perm_[X] + Y, AA = perm_[A], AB = perm_[A + 1];
        int B = perm_[X + 1] + Y, BA = perm_[B], BB = perm_[B + 1];
        return lerp(v, lerp(u, grad(perm_[AA], x, y),
                               grad(perm_[BA], x - 1, y)),
                       lerp(u, grad(perm_[AB], x, y - 1),
                               grad(perm_[BB], x - 1, y - 1)));
    }

    double noise3D(double x, double y, double z) const {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;
        int Z = static_cast<int>(std::floor(z)) & 255;
        x -= std::floor(x); y -= std::floor(y); z -= std::floor(z);
        double u = fade(x), v = fade(y), w = fade(z);
        int A = perm_[X] + Y, AA = perm_[A] + Z, AB = perm_[A+1] + Z;
        int B = perm_[X+1] + Y, BA = perm_[B] + Z, BB = perm_[B+1] + Z;
        return lerp(w,
            lerp(v, lerp(u, grad3(perm_[AA], x, y, z),
                            grad3(perm_[BA], x-1, y, z)),
                    lerp(u, grad3(perm_[AB], x, y-1, z),
                            grad3(perm_[BB], x-1, y-1, z))),
            lerp(v, lerp(u, grad3(perm_[AA+1], x, y, z-1),
                            grad3(perm_[BA+1], x-1, y, z-1)),
                    lerp(u, grad3(perm_[AB+1], x, y-1, z-1),
                            grad3(perm_[BB+1], x-1, y-1, z-1))));
    }

    // Octave noise — multiple layers at different frequencies
    double octaveNoise(double x, double y, int octaves, double persistence = 0.5) const {
        double total = 0, amplitude = 1, frequency = 1, maxVal = 0;
        for (int i = 0; i < octaves; ++i) {
            total += noise(x * frequency, y * frequency) * amplitude;
            maxVal += amplitude;
            amplitude *= persistence;
            frequency *= 2.0;
        }
        return total / maxVal;
    }

private:
    int perm_[512];

    static double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    static double lerp(double t, double a, double b) { return a + t * (b - a); }

    static double grad(int hash, double x, double y) {
        int h = hash & 3;
        double u = h < 2 ? x : y;
        double v = h < 2 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    }

    static double grad3(int hash, double x, double y, double z) {
        int h = hash & 15;
        double u = h < 8 ? x : y;
        double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    }
};

// ---- Biome types ----
enum class BiomeType : uint8_t {
    OCEAN        = 0,
    PLAINS       = 1,
    DESERT       = 2,
    EXTREME_HILLS = 3,
    FOREST       = 4,
    TAIGA        = 5,
    SWAMPLAND    = 6,
    RIVER        = 7,
    FROZEN_OCEAN = 10,
    FROZEN_RIVER = 11,
    ICE_PLAINS   = 12,
    ICE_MOUNTAINS = 13,
    MUSHROOM_IS  = 14,
    BEACH        = 16,
    JUNGLE       = 21,
    BIRCH_FOREST = 27,
    ROOFED_FOREST = 29,
    SAVANNA      = 35,
    MESA         = 37,
};

// ---- Ore definition ----
struct OreVein {
    uint16_t blockId;
    int minY, maxY;
    int veinSize;     // Max blocks per vein
    int veinsPerChunk;
};

// Vanilla ore generation — from abg.java (WorldGenMinable)
static const OreVein VANILLA_ORES[] = {
    {16,  0, 128, 17, 20},  // Coal ore
    {15,  0,  64,  9, 20},  // Iron ore
    {14,  0,  32,  9,  2},  // Gold ore
    {56,  0,  16,  8,  1},  // Diamond ore
    {73,  0,  16,  8,  8},  // Redstone ore
    {21,  0,  32,  7,  1},  // Lapis lazuli ore
    {129, 0,  32,  7,  1},  // Emerald ore (extreme hills)
};

// ---- Main terrain generator ----
class TerrainGenerator {
public:
    TerrainGenerator(uint64_t seed = 42)
        : seed_(seed)
        , heightNoise_(seed)
        , biomeNoise_(seed + 1)
        , caveNoise_(seed + 2)
        , oreRng_(seed + 3) {}

    // Generate a full chunk column at chunk coordinates (cx, cz)
    std::unique_ptr<ChunkColumn> generateChunk(int cx, int cz) {
        auto chunk = std::make_unique<ChunkColumn>();
        chunk->chunkX = cx;
        chunk->chunkZ = cz;

        // 1. Base terrain height
        std::array<int, 256> heightMap{};
        std::array<BiomeType, 256> biomeMap{};

        for (int bz = 0; bz < 16; ++bz) {
            for (int bx = 0; bx < 16; ++bx) {
                double wx = (cx * 16 + bx) / 64.0;
                double wz = (cz * 16 + bz) / 64.0;

                // Multi-octave height noise
                double h = heightNoise_.octaveNoise(wx, wz, 6, 0.5);
                // Scale to terrain range: 48-96 (sea level at 63)
                int height = static_cast<int>(64 + h * 24);
                height = std::clamp(height, 1, 250);
                heightMap[bz * 16 + bx] = height;

                // Biome selection from temperature/rainfall noise
                double temp = biomeNoise_.octaveNoise(wx * 0.5, wz * 0.5, 4, 0.5);
                double rain = biomeNoise_.octaveNoise(wx * 0.5 + 100, wz * 0.5 + 100, 4, 0.5);
                biomeMap[bz * 16 + bx] = selectBiome(temp, rain, height);
            }
        }

        // 2. Fill terrain
        for (int bz = 0; bz < 16; ++bz) {
            for (int bx = 0; bx < 16; ++bx) {
                int idx = bz * 16 + bx;
                int height = heightMap[idx];
                BiomeType biome = biomeMap[idx];

                // Bedrock layer
                setBlock(*chunk, bx, 0, bz, 7, 0);

                // Stone fill
                for (int y = 1; y < height - 4; ++y) {
                    setBlock(*chunk, bx, y, bz, 1, 0); // Stone
                }

                // Surface layers per biome
                applySurface(*chunk, bx, bz, height, biome);

                // Water fill (sea level = 63)
                for (int y = height; y <= 63; ++y) {
                    if (getBlock(*chunk, bx, y, bz) == 0) {
                        setBlock(*chunk, bx, y, bz, 9, 0); // Water
                    }
                }

                // Set biome
                chunk->biomes[idx] = static_cast<uint8_t>(biome);
            }
        }

        // 3. Cave carving
        carveCaves(*chunk, cx, cz);

        // 4. Ore veins
        generateOres(*chunk, cx, cz);

        // 5. Scattered bedrock (y=1-4)
        std::mt19937 bedrockRng(seed_ ^ (cx * 341873128712LL + cz * 132897987541LL));
        for (int bz = 0; bz < 16; ++bz) {
            for (int bx = 0; bx < 16; ++bx) {
                for (int y = 1; y <= 4; ++y) {
                    if (static_cast<int>(bedrockRng() % 5) <= (5 - y)) {
                        setBlock(*chunk, bx, y, bz, 7, 0); // Bedrock
                    }
                }
            }
        }

        // 6. Tree decoration
        generateTrees(*chunk, cx, cz, heightMap, biomeMap);

        // 7. Structure generation (dungeons, mineshafts)
        {
            auto getBlockWorld = [&](int gx, int gy, int gz) -> uint16_t {
                int lx = gx - cx * 16, lz = gz - cz * 16;
                if (lx < 0 || lx > 15 || lz < 0 || lz > 15 || gy < 0 || gy > 255) return 1;
                return getBlock(*chunk, lx, gy, lz);
            };
            auto setBlockWorld = [&](int gx, int gy, int gz, uint16_t id, uint8_t meta) {
                int lx = gx - cx * 16, lz = gz - cz * 16;
                if (lx < 0 || lx > 15 || lz < 0 || lz > 15 || gy < 0 || gy > 255) return;
                setBlock(*chunk, lx, gy, gz, id, meta);
            };
            StructureGenerator::generateDungeons(cx, cz, seed_, getBlockWorld, setBlockWorld);
            StructureGenerator::generateMineshafts(cx, cz, seed_, getBlockWorld, setBlockWorld);
        }

        return chunk;
    }

private:
    uint64_t seed_;
    PerlinNoise heightNoise_;
    PerlinNoise biomeNoise_;
    PerlinNoise caveNoise_;
    std::mt19937 oreRng_;

    // Biome selection from temperature/rainfall
    BiomeType selectBiome(double temp, double rain, int height) {
        if (height <= 60) return BiomeType::OCEAN;
        if (height <= 63) return BiomeType::BEACH;
        if (height >= 90) return BiomeType::EXTREME_HILLS;

        if (temp < -0.3) {
            return rain > 0 ? BiomeType::TAIGA : BiomeType::ICE_PLAINS;
        } else if (temp < 0.0) {
            return rain > 0.2 ? BiomeType::FOREST : BiomeType::PLAINS;
        } else if (temp < 0.3) {
            return rain > 0.1 ? BiomeType::BIRCH_FOREST : BiomeType::PLAINS;
        } else if (temp < 0.6) {
            return rain < -0.2 ? BiomeType::DESERT : BiomeType::SAVANNA;
        } else {
            return rain > 0 ? BiomeType::JUNGLE : BiomeType::MESA;
        }
    }

    // Apply surface blocks based on biome
    void applySurface(ChunkColumn& chunk, int bx, int bz, int height, BiomeType biome) {
        uint16_t topBlock, fillerBlock;

        switch (biome) {
            case BiomeType::DESERT:
            case BiomeType::MESA:
                topBlock = 12; fillerBlock = 24; // Sand / Sandstone
                break;
            case BiomeType::OCEAN:
            case BiomeType::BEACH:
                topBlock = 12; fillerBlock = 12; // Sand / Sand
                break;
            case BiomeType::ICE_PLAINS:
            case BiomeType::FROZEN_OCEAN:
                topBlock = 2; fillerBlock = 3; // Grass / Dirt with snow
                break;
            case BiomeType::MUSHROOM_IS:
                topBlock = 110; fillerBlock = 3; // Mycelium / Dirt
                break;
            default:
                topBlock = 2; fillerBlock = 3; // Grass / Dirt
                break;
        }

        // Top surface
        if (height > 63) {
            setBlock(chunk, bx, height - 1, bz, topBlock, 0);
        }
        // Filler (3 blocks below surface)
        for (int y = std::max(1, height - 4); y < height - 1; ++y) {
            setBlock(chunk, bx, y, bz, fillerBlock, 0);
        }
    }

    // Carve caves using 3D noise worms
    void carveCaves(ChunkColumn& chunk, int cx, int cz) {
        std::mt19937 caveRng(seed_ ^ (cx * 198491317LL + cz * 776531419LL));

        // Generate cave worms in this and neighboring chunks
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dz = -1; dz <= 1; ++dz) {
                std::mt19937 localRng(seed_ ^ ((cx+dx) * 198491317LL + (cz+dz) * 776531419LL));
                int numWorms = localRng() % 8; // 0-7 cave starts

                for (int w = 0; w < numWorms; ++w) {
                    // Cave start position
                    double sx = (cx + dx) * 16 + (localRng() % 16);
                    double sy = 10 + (localRng() % 50); // y 10-60
                    double sz = (cz + dz) * 16 + (localRng() % 16);

                    double yaw = (localRng() % 360) * 3.14159265 / 180.0;
                    double pitch = ((localRng() % 90) - 45) * 3.14159265 / 180.0;
                    double radius = 1.0 + (localRng() % 30) / 10.0; // 1.0-4.0

                    int length = 64 + (localRng() % 64);

                    for (int step = 0; step < length; ++step) {
                        // Move worm
                        sx += std::cos(yaw) * std::cos(pitch);
                        sy += std::sin(pitch);
                        sz += std::sin(yaw) * std::cos(pitch);

                        // Wobble direction
                        yaw += (static_cast<double>(localRng() % 100) - 50) / 200.0;
                        pitch += (static_cast<double>(localRng() % 100) - 50) / 400.0;
                        pitch = std::clamp(pitch, -0.7, 0.7);

                        // Varying radius
                        double r = radius * (0.5 + 0.5 * std::sin(step * 0.1));

                        // Carve sphere
                        int minBx = static_cast<int>(sx - r) - cx * 16;
                        int maxBx = static_cast<int>(sx + r) - cx * 16;
                        int minBz = static_cast<int>(sz - r) - cz * 16;
                        int maxBz = static_cast<int>(sz + r) - cz * 16;

                        for (int by = std::max(1, static_cast<int>(sy - r));
                             by <= std::min(255, static_cast<int>(sy + r)); ++by) {
                            for (int bx2 = std::max(0, minBx); bx2 <= std::min(15, maxBx); ++bx2) {
                                for (int bz2 = std::max(0, minBz); bz2 <= std::min(15, maxBz); ++bz2) {
                                    double dx2 = (cx * 16 + bx2) - sx;
                                    double dy2 = by - sy;
                                    double dz2 = (cz * 16 + bz2) - sz;
                                    if (dx2*dx2 + dy2*dy2 + dz2*dz2 < r*r) {
                                        uint16_t existing = getBlock(chunk, bx2, by, bz2);
                                        // Don't carve through water or bedrock
                                        if (existing != 7 && existing != 9 && existing != 8) {
                                            // Below sea level: fill with air only if above is not water
                                            if (by < 10) {
                                                setBlock(chunk, bx2, by, bz2, 11, 0); // Lava
                                            } else {
                                                setBlock(chunk, bx2, by, bz2, 0, 0); // Air
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Generate ore veins
    void generateOres(ChunkColumn& chunk, int cx, int cz) {
        std::mt19937 rng(seed_ ^ (cx * 567890123LL + cz * 987654321LL));

        for (auto& ore : VANILLA_ORES) {
            for (int v = 0; v < ore.veinsPerChunk; ++v) {
                int bx = rng() % 16;
                int by = ore.minY + (rng() % (ore.maxY - ore.minY + 1));
                int bz = rng() % 16;

                // Place vein as a small blob
                for (int i = 0; i < ore.veinSize; ++i) {
                    int ox = bx + (static_cast<int>(rng() % 3) - 1);
                    int oy = by + (static_cast<int>(rng() % 3) - 1);
                    int oz = bz + (static_cast<int>(rng() % 3) - 1);

                    if (ox >= 0 && ox < 16 && oy >= 1 && oy < 256 &&
                        oz >= 0 && oz < 16) {
                        if (getBlock(chunk, ox, oy, oz) == 1) { // Only replace stone
                            setBlock(chunk, ox, oy, oz, ore.blockId, 0);
                        }
                    }
                    bx = ox; by = oy; bz = oz;
                    bx = std::clamp(bx, 0, 15);
                    by = std::clamp(by, 1, 255);
                    bz = std::clamp(bz, 0, 15);
                }
            }
        }
    }

    // Tree decoration pass
    void generateTrees(ChunkColumn& chunk, int cx, int cz,
                       const std::array<int, 256>& heightMap,
                       const std::array<BiomeType, 256>& biomeMap) {
        std::mt19937 treeRng(seed_ ^ (cx * 456789123LL + cz * 321654987LL));

        // Determine tree count by dominant biome
        BiomeType dominant = biomeMap[8 * 16 + 8]; // Center of chunk
        int treeCount = 0;
        switch (dominant) {
            case BiomeType::FOREST:
            case BiomeType::ROOFED_FOREST:
            case BiomeType::BIRCH_FOREST:
                treeCount = 5 + (treeRng() % 5); break; // 5-9
            case BiomeType::JUNGLE:
                treeCount = 8 + (treeRng() % 5); break; // 8-12
            case BiomeType::TAIGA:
                treeCount = 4 + (treeRng() % 4); break; // 4-7
            case BiomeType::PLAINS:
            case BiomeType::SAVANNA:
                treeCount = (treeRng() % 3 == 0) ? 1 : 0; break; // 0-1
            case BiomeType::EXTREME_HILLS:
                treeCount = 1 + (treeRng() % 3); break; // 1-3
            case BiomeType::SWAMPLAND:
                treeCount = 2 + (treeRng() % 3); break; // 2-4
            default:
                treeCount = 0; break; // Ocean, desert, beach, etc
        }

        for (int t = 0; t < treeCount; ++t) {
            int bx = 2 + (treeRng() % 12); // 2-13 to avoid edge overflow
            int bz = 2 + (treeRng() % 12);
            int surfaceY = heightMap[bz * 16 + bx];

            // Only place on land above sea level
            if (surfaceY <= 63) continue;

            BiomeType localBiome = biomeMap[bz * 16 + bx];
            TreeType treeType = TreeGenerator::treeForBiome(static_cast<uint8_t>(localBiome));

            auto getBlockFn = [&](int gx, int gy, int gz) -> uint16_t {
                int lx = gx - cx * 16, lz = gz - cz * 16;
                if (lx < 0 || lx > 15 || lz < 0 || lz > 15) return 0;
                return getBlock(chunk, lx, gy, lz);
            };
            auto setBlockFn = [&](int gx, int gy, int gz, uint16_t id, uint8_t meta) {
                int lx = gx - cx * 16, lz = gz - cz * 16;
                if (lx < 0 || lx > 15 || lz < 0 || lz > 15) return;
                setBlock(chunk, lx, gy, gz, id, meta);
            };

            TreeGenerator::generate(treeType,
                cx * 16 + bx, surfaceY, cz * 16 + bz,
                treeRng, getBlockFn, setBlockFn);
        }
    }

    // Block helpers using ChunkColumn sections
    void setBlock(ChunkColumn& chunk, int x, int y, int z, uint16_t id, uint8_t meta) {
        int sIdx = y >> 4;
        if (sIdx < 0 || sIdx >= 16) return;
        if (!chunk.sections[sIdx]) {
            if (id == 0) return; // Don't create section for air
            chunk.sections[sIdx] = std::make_unique<ChunkSection>();
        }
        chunk.sections[sIdx]->setBlockId(x, y & 0xF, z, id);
        if (meta) chunk.sections[sIdx]->setMetadata(x, y & 0xF, z, meta);
    }

    uint16_t getBlock(ChunkColumn& chunk, int x, int y, int z) {
        int sIdx = y >> 4;
        if (sIdx < 0 || sIdx >= 16 || !chunk.sections[sIdx]) return 0;
        return chunk.sections[sIdx]->getBlockId(x, y & 0xF, z);
    }
};

} // namespace mc
