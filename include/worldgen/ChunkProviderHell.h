/**
 * ChunkProviderHell.h — Nether terrain generation.
 *
 * Java reference: net.minecraft.world.gen.ChunkProviderHell (415 lines)
 *
 * Architecture:
 *   - 7 noise generators: noiseGen1-3 (octaves 16,16,8), slowsand/gravel (4),
 *     netherrack exclusivity (4), noiseGen6 (10), noiseGen7 (16)
 *   - Terrain: netherrack with lava sea at y=32
 *   - Surface: soul sand + gravel bands at lava level (±4)
 *   - Bedrock: top (127-rand(5)) and bottom (0+rand(5))
 *   - Caves: MapGenCavesHell
 *   - Structures: Nether Fortress (MapGenNetherBridge)
 *   - Populate: lava springs(8×), fire(rand(rand(10)+1)+1), glowstone1(rand(rand(10)+1)),
 *     glowstone2(10×), mushrooms(1/1), quartz ore(16×13 y10-118), lava hidden(16×)
 *   - Noise field: 5×17×5 grid interpolated to 16×128×16
 *   - Nether cosine profile: cos(y*PI*6/height)*2 with cubic falloff at edges
 *   - Chunk seed: x*341873128712 + z*132897987541
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <vector>

namespace mccpp {

class ChunkProviderHell {
public:
    int64_t worldSeed;

    // Block IDs
    static constexpr int32_t NETHERRACK = 87;
    static constexpr int32_t LAVA = 11;       // flowing_lava
    static constexpr int32_t SOUL_SAND = 88;
    static constexpr int32_t GRAVEL = 13;
    static constexpr int32_t BEDROCK = 7;
    static constexpr int32_t QUARTZ_ORE = 153;
    static constexpr int32_t NETHER_BRICK = 112;
    static constexpr int32_t GLOWSTONE = 89;
    static constexpr int32_t FIRE = 51;

    // Lava sea level
    static constexpr int32_t LAVA_LEVEL = 32;

    // Noise parameters
    // Java: base = 684.412, secondary = 2053.236
    static constexpr double BASE_FREQ = 684.412;
    static constexpr double NETHER_FREQ2 = 2053.236;

    explicit ChunkProviderHell(int64_t seed) : worldSeed(seed) {}

    // ═══════════════════════════════════════════════════════════════════════
    // Terrain generation — 3D density field interpolation.
    // Java: func_147419_a — builds 5×17×5 noise field, interpolates to 4×8×4
    //   blocks within each cell, fills netherrack where density>0, lava below y=32.
    //
    // Noise field layout:
    //   noiseGen3 (8 octaves): main shape, freq = BASE/80, FREQ2/60, BASE/80
    //   noiseGen1/2 (16 octaves each): detail, freq = BASE, FREQ2, BASE
    //   noiseGen6 (10 octaves): depth variation
    //   noiseGen7 (16 octaves): scale variation
    //
    // Nether cosine profile (Java's dArray2):
    //   cos(y * PI * 6 / 17) * 2, with cubic falloff at top/bottom 4 layers:
    //   if y > 13: subtract (y-13)^3 * 10
    //   if y < 4: subtract (4-y)^3 * 10
    // ═══════════════════════════════════════════════════════════════════════

    static std::vector<double> computeNetherProfile(int32_t height) {
        std::vector<double> profile(height);
        for (int32_t y = 0; y < height; ++y) {
            profile[y] = std::cos(static_cast<double>(y) * M_PI * 6.0 / height) * 2.0;
            double dist = y;
            if (y > height / 2) dist = height - 1 - y;
            if (dist < 4.0) {
                dist = 4.0 - dist;
                profile[y] -= dist * dist * dist * 10.0;
            }
        }
        return profile;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Surface decoration
    // Java: func_147418_b — places soul sand, gravel, bedrock
    //   - slowsand noise + rand*0.2 > 0 → soul sand at lava level
    //   - gravel noise + rand*0.2 > 0 → gravel at lava level
    //   - Bedrock: y >= 127-rand(5) or y <= 0+rand(5)
    //   - Netherrack exclusivity: depth = noise/3 + 3 + rand*0.25
    // ═══════════════════════════════════════════════════════════════════════

    struct SurfaceConfig {
        bool useSoulSand;
        bool useGravel;
        int32_t surfaceDepth;  // noise/3 + 3 + rand*0.25
    };

    // ═══════════════════════════════════════════════════════════════════════
    // Population — Nether features
    // Java: populate
    //   - Nether Fortress structures
    //   - 8× lava springs (y=4-124)
    //   - Fire: rand(rand(10)+1)+1 times
    //   - Glowstone1: rand(rand(10)+1) clusters
    //   - Glowstone2: 10× clusters
    //   - Mushrooms: always 1 brown + 1 red (rand(1)==0 is always true)
    //   - Quartz ore: 16×13 blocks, y=10-118
    //   - Hidden lava: 16× springs with solid check
    // ═══════════════════════════════════════════════════════════════════════

    struct NetherFeature {
        enum Type { LAVA_SPRING, FIRE, GLOWSTONE1, GLOWSTONE2, MUSHROOM_BROWN,
                    MUSHROOM_RED, QUARTZ_ORE, HIDDEN_LAVA };
        Type type;
        int32_t x, y, z;
    };

    struct PopulateConfig {
        int32_t lavaSpringCount = 8;
        int32_t quartzOreCount = 16;
        int32_t quartzOreSize = 13;
        int32_t quartzOreMinY = 10;
        int32_t quartzOreMaxY = 118;
        int32_t hiddenLavaCount = 16;
        int32_t glowstone2Count = 10;
    };

    // Chunk seed computation
    static int64_t chunkSeed(int32_t chunkX, int32_t chunkZ) {
        return static_cast<int64_t>(chunkX) * 341873128712LL
             + static_cast<int64_t>(chunkZ) * 132897987541LL;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// ChunkProviderEnd — End terrain generation.
//
// Java reference: net.minecraft.world.gen.ChunkProviderEnd (300 lines)
//
// Architecture:
//   - 5 noise generators: noiseGen1-3 (16,16,8), noiseGen4 (10), noiseGen5 (16)
//   - Terrain: end_stone where density > 0
//   - Island shape: f3 = 100 - sqrt(x² + z²) * 8, clamped to [-100, 80]
//   - No bedrock, no lava, no structures
//   - Noise field: 3×33×3 grid interpolated to 16×256×16
//   - Base frequency: 684.412 (doubled to 1368.824)
//   - Bottom/top clamping: y<8 → lerp to -30, y>height/2-2 → lerp to -3000
//   - Populate: biome decoration only
// ═══════════════════════════════════════════════════════════════════════════

class ChunkProviderEnd {
public:
    int64_t worldSeed;

    static constexpr int32_t END_STONE = 121;
    static constexpr double BASE_FREQ = 684.412;

    explicit ChunkProviderEnd(int64_t seed) : worldSeed(seed) {}

    // ═══════════════════════════════════════════════════════════════════════
    // Island shape calculation
    // Java: f3 = 100 - sqrt((x+chunkX)² + (z+chunkZ)²) * 8
    //   Clamped to [-100, 80]
    //   Added to density at each y-level
    //   Creates the floating island shape
    // ═══════════════════════════════════════════════════════════════════════

    static float computeIslandFactor(float worldX, float worldZ) {
        float dist = std::sqrt(worldX * worldX + worldZ * worldZ);
        float f = 100.0f - dist * 8.0f;
        if (f > 80.0f) f = 80.0f;
        if (f < -100.0f) f = -100.0f;
        return f;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Density field computation
    // Java: initializeNoiseField
    //   - noiseGen3 (8 oct): interp, freq = 2*684.412/80
    //   - noiseGen1/2 (16 oct each): detail, freq = 2*684.412
    //   - noiseGen4 (10 oct): depth mod, freq = 1.121
    //   - noiseGen5 (16 oct): unused in practice (d3 set to 0)
    //   - Vertical: (y - height/2) * 8 / depthScale
    //   - Top clamp (y > height/2 - 2): lerp to -3000
    //   - Bottom clamp (y < 8): lerp to -30
    //   - Final: density - 8 + islandFactor
    // ═══════════════════════════════════════════════════════════════════════

    struct NoiseConfig {
        // 3×33×3 noise field
        static constexpr int32_t FIELD_X = 3;   // 2 + 1
        static constexpr int32_t FIELD_Y = 33;
        static constexpr int32_t FIELD_Z = 3;   // 2 + 1

        // Interpolation: each cell maps to 8×4×8 blocks
        static constexpr int32_t INTERP_X = 8;
        static constexpr int32_t INTERP_Y = 4;  // 128 / 33 ≈ 4
        static constexpr int32_t INTERP_Z = 8;
    };

    // Compute single density value at noise grid position
    static double computeDensity(double noiseInterp, double noise1, double noise2,
                                   float islandFactor, int32_t y, int32_t height,
                                   double depthScale) {
        // Y stretching
        double halfHeight = static_cast<double>(height) / 2.0;
        double yFactor = (static_cast<double>(y) - halfHeight) * 8.0 / depthScale;
        if (yFactor < 0) yFactor = -yFactor;

        // Interpolate between noise1 and noise2 using noiseInterp
        double density;
        double interp = (noiseInterp / 10.0 + 1.0) / 2.0;
        if (interp < 0) density = noise1 / 512.0;
        else if (interp > 1) density = noise2 / 512.0;
        else density = noise1 / 512.0 + (noise2 / 512.0 - noise1 / 512.0) * interp;

        density -= 8.0;
        density += static_cast<double>(islandFactor);

        // Top clamping: y > height/2 - 2
        int32_t topThresh = 2;
        if (y > height / 2 - topThresh) {
            double factor = static_cast<double>(y - (height / 2 - topThresh)) / 64.0;
            factor = std::clamp(factor, 0.0, 1.0);
            density = density * (1.0 - factor) + -3000.0 * factor;
        }

        // Bottom clamping: y < 8
        if (y < 8) {
            double factor = static_cast<double>(8 - y) / 7.0;
            density = density * (1.0 - factor) + -30.0 * factor;
        }

        return density;
    }

    // Chunk seed computation (same formula as Nether)
    static int64_t chunkSeed(int32_t chunkX, int32_t chunkZ) {
        return static_cast<int64_t>(chunkX) * 341873128712LL
             + static_cast<int64_t>(chunkZ) * 132897987541LL;
    }
};

} // namespace mccpp
