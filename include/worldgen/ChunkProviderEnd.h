/**
 * ChunkProviderEnd.h — End dimension terrain generation.
 *
 * Java reference: net.minecraft.world.gen.ChunkProviderEnd (300 lines)
 *
 * Generates the End dimension terrain using 3D noise density fields
 * interpolated over a 3×33×3 grid with trilinear interpolation into
 * 16×128×16 block arrays.
 *
 * Key algorithm:
 *   1. Initialize 5 noise generators from world seed:
 *      - noiseGen1: 16 octaves (low frequency terrain shape)
 *      - noiseGen2: 16 octaves (high frequency terrain detail)
 *      - noiseGen3: 8 octaves (interpolation selector)
 *      - noiseGen4: 10 octaves (terrain height variation)
 *      - noiseGen5: 16 octaves (depth variation, scale 200.0)
 *
 *   2. Build density field (3×33×3):
 *      - Base frequency: 684.412 * 2.0 = 1368.824
 *      - Island distance falloff: f3 = 100 - sqrt(x² + z²) * 8
 *        Clamped to [-100, 80] — creates the floating island shape
 *      - Noise interpolation: lerp(noiseGen1, noiseGen2, noiseGen3)
 *      - Top ceiling fadeout at y > n5/2 - 2 (blends to -3000)
 *      - Bottom floor at y < 8 (blends to -30)
 *
 *   3. Trilinear interpolation:
 *      - 4:1 Y interpolation (33 samples → 128 blocks, step 0.25)
 *      - 8:1 X/Z interpolation (3 samples → 16 blocks, step 0.125)
 *      - density > 0 → end_stone, density <= 0 → air
 *
 *   4. provideChunk seed: x * 341873128712 + z * 132897987541
 *   5. populate: biome.decorate with BlockFalling.fallInstantly = true
 *
 * Thread safety: Per-dimension provider, no shared mutable state.
 * JNI readiness: Simple state with clear lifecycle.
 */
#pragma once

#include <cstdint>
#include <cmath>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// End dimension constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EndGenConstants {
    // ─── Noise generator octave counts ───
    static constexpr int32_t NOISE1_OCTAVES = 16;  // Low frequency shape
    static constexpr int32_t NOISE2_OCTAVES = 16;  // High frequency detail
    static constexpr int32_t NOISE3_OCTAVES = 8;   // Interpolation selector
    static constexpr int32_t NOISE4_OCTAVES = 10;  // Height variation
    static constexpr int32_t NOISE5_OCTAVES = 16;  // Depth variation

    // ─── Noise frequencies ───
    // Java: double d = 684.412; d *= 2.0;
    static constexpr double BASE_FREQUENCY = 684.412;
    static constexpr double FREQUENCY_MULTIPLIER = 2.0;
    static constexpr double DOUBLED_FREQUENCY = BASE_FREQUENCY * FREQUENCY_MULTIPLIER;

    // Java: d / 80.0, d2 / 160.0, d / 80.0 for noise3 (selector)
    static constexpr double SELECTOR_XZ_SCALE = DOUBLED_FREQUENCY / 80.0;
    static constexpr double SELECTOR_Y_SCALE = BASE_FREQUENCY / 160.0;

    // Java: noiseGen4 scale = 1.121, 1.121, 0.5
    static constexpr double NOISE4_X_SCALE = 1.121;
    static constexpr double NOISE4_Z_SCALE = 1.121;
    static constexpr double NOISE4_FALLOFF = 0.5;

    // Java: noiseGen5 scale = 200.0, 200.0, 0.5
    static constexpr double NOISE5_X_SCALE = 200.0;
    static constexpr double NOISE5_Z_SCALE = 200.0;
    static constexpr double NOISE5_FALLOFF = 0.5;

    // ─── Density field dimensions ───
    // Java: n3 = 2 → n4 = 3, n5 = 33, n6 = 3
    static constexpr int32_t DENSITY_XZ_SIZE = 3;   // n4 = n6 = 2 + 1
    static constexpr int32_t DENSITY_Y_SIZE = 33;    // n5 = 33
    static constexpr int32_t NOISE_XZ_STEP = 2;      // n3 = 2

    // ─── Interpolation steps ───
    static constexpr double Y_INTERPOLATION_STEP = 0.25;   // 4 blocks per sample
    static constexpr double XZ_INTERPOLATION_STEP = 0.125;  // 8 blocks per sample
    static constexpr int32_t Y_BLOCKS_PER_SAMPLE = 4;
    static constexpr int32_t XZ_BLOCKS_PER_SAMPLE = 8;

    // ─── Island distance falloff ───
    // Java: f3 = 100.0f - sqrt(fx*fx + fz*fz) * 8.0f
    static constexpr float ISLAND_BASE = 100.0f;
    static constexpr float ISLAND_DISTANCE_SCALE = 8.0f;
    static constexpr float ISLAND_MAX = 80.0f;     // clamp upper
    static constexpr float ISLAND_MIN = -100.0f;   // clamp lower

    // ─── Top ceiling fadeout ───
    // Java: if (k > n5 / 2 - n9) where n9 = 2
    // Blends density from current to -3000
    static constexpr int32_t CEILING_FADE_OFFSET = 2;
    static constexpr double CEILING_FADE_DIVISOR = 64.0;
    static constexpr double CEILING_FADE_TARGET = -3000.0;

    // ─── Bottom floor ───
    // Java: if (k < n9) where n9 = 8
    // Blends density from current to -30
    static constexpr int32_t FLOOR_THRESHOLD = 8;
    static constexpr double FLOOR_FADE_TARGET = -30.0;

    // ─── Noise processing ───
    static constexpr double NOISE_HEIGHT_SCALE = 256.0;
    static constexpr double NOISE_DEPTH_SCALE = 8000.0;
    static constexpr double NOISE_DEPTH_NEGATE_SCALE = 0.3;
    static constexpr double NOISE_DEPTH_MULTIPLY = 3.0;
    static constexpr double NOISE_DEPTH_OFFSET = -2.0;
    static constexpr double NOISE_DEPTH_DIVISOR = 8.0;
    static constexpr double HEIGHT_Y_SCALE = 8.0;
    static constexpr double NOISE_RESULT_OFFSET = -8.0;

    // Java: noise/512.0 for terrain blending
    static constexpr double NOISE_TERRAIN_SCALE = 512.0;
    // Java: (noiseData1/10.0 + 1.0) / 2.0 for selector
    static constexpr double NOISE_SELECTOR_SCALE = 10.0;

    // ─── Chunk seed ───
    // Java: (long)n * 341873128712L + (long)n2 * 132897987541L
    static constexpr int64_t CHUNK_SEED_X = 341873128712LL;
    static constexpr int64_t CHUNK_SEED_Z = 132897987541LL;

    // ─── Block array ───
    static constexpr int32_t BLOCKS_PER_CHUNK = 32768;  // 16 * 128 * 16
    static constexpr int32_t CHUNK_HEIGHT = 128;
}

// ═══════════════════════════════════════════════════════════════════════════
// ChunkProviderEnd — End dimension terrain generator.
// ═══════════════════════════════════════════════════════════════════════════

class ChunkProviderEnd {
public:
    // ─── Island distance falloff ───
    // Java: The core shape function that creates floating islands
    // f3 = 100 - sqrt(x² + z²) * 8, clamped to [-100, 80]
    static float computeIslandFalloff(float chunkX, float chunkZ) {
        float dist = std::sqrt(chunkX * chunkX + chunkZ * chunkZ);
        float f3 = EndGenConstants::ISLAND_BASE - dist * EndGenConstants::ISLAND_DISTANCE_SCALE;
        if (f3 > EndGenConstants::ISLAND_MAX) f3 = EndGenConstants::ISLAND_MAX;
        if (f3 < EndGenConstants::ISLAND_MIN) f3 = EndGenConstants::ISLAND_MIN;
        return f3;
    }

    // ─── Density computation for a single column ───
    // Java: initializeNoiseField inner loop
    // Computes density at each Y level for trilinear interpolation
    static double computeDensity(double noiseData1, double noiseData2, double noiseData3,
                                  double halfHeight, int32_t y, double heightVar,
                                  float islandFalloff)
    {
        // Height-dependent density curve
        double d8 = (static_cast<double>(y) - halfHeight) * EndGenConstants::HEIGHT_Y_SCALE / heightVar;
        if (d8 < 0.0) d8 = -d8;

        // Noise interpolation: lerp between low/high based on selector
        double d9 = noiseData2 / EndGenConstants::NOISE_TERRAIN_SCALE;
        double d10 = noiseData3 / EndGenConstants::NOISE_TERRAIN_SCALE;
        double selector = (noiseData1 / EndGenConstants::NOISE_SELECTOR_SCALE + 1.0) / 2.0;

        double density;
        if (selector < 0.0) {
            density = d9;
        } else if (selector > 1.0) {
            density = d10;
        } else {
            density = d9 + (d10 - d9) * selector;
        }

        density += EndGenConstants::NOISE_RESULT_OFFSET;
        density += static_cast<double>(islandFalloff);

        // Top ceiling fadeout
        int32_t n5 = EndGenConstants::DENSITY_Y_SIZE;
        int32_t ceilingStart = n5 / 2 - EndGenConstants::CEILING_FADE_OFFSET;
        if (y > ceilingStart) {
            double fade = static_cast<double>(y - ceilingStart) / EndGenConstants::CEILING_FADE_DIVISOR;
            if (fade < 0.0) fade = 0.0;
            if (fade > 1.0) fade = 1.0;
            density = density * (1.0 - fade) + EndGenConstants::CEILING_FADE_TARGET * fade;
        }

        // Bottom floor
        if (y < EndGenConstants::FLOOR_THRESHOLD) {
            double fade = static_cast<double>(EndGenConstants::FLOOR_THRESHOLD - y)
                          / (static_cast<double>(EndGenConstants::FLOOR_THRESHOLD) - 1.0);
            density = density * (1.0 - fade) + EndGenConstants::FLOOR_FADE_TARGET * fade;
        }

        return density;
    }

    // ─── Trilinear interpolation constants ───
    // Java: The block placement loop uses trilinear interpolation
    // from the 3×33×3 density grid to fill a 16×128×16 block array.
    //
    // Array indexing:
    //   index = (x + i*8) << 11 | (0 + j*8) << 7 | (k*4 + subY)
    //   Then iterate with stride 128 for each Z sub-step
    //
    // Density > 0.0 → end_stone (Block ID 121)
    // Density <= 0.0 → air (null/ID 0)

    static constexpr int32_t END_STONE_ID = 121;

    // ─── Block index encoding ───
    // Java: n7 = i3 + i * 8 << 11 | 0 + j * 8 << 7 | k * 4 + i2
    // This matches the old chunk format: x << 11 | z << 7 | y
    static int32_t blockIndex(int32_t x, int32_t z, int32_t y) {
        return (x << 11) | (z << 7) | y;
    }

    // ─── Height variation computation ───
    // Java: Uses noiseData4/noiseData5 for height variation
    // d4 = (noiseData4 + 256) / 512, clamped [0, ∞]
    // Then d4 += 0.5 → height scale factor
    static double computeHeightVariation(double noise4, double noise5) {
        double d4 = (noise4 + EndGenConstants::NOISE_HEIGHT_SCALE) / (EndGenConstants::NOISE_HEIGHT_SCALE * 2.0);
        // d3 from noise5 — but Java sets d3 = 0.0 unconditionally
        // (the End dimension ignores the depth noise result)
        if (d4 < 0.0) d4 = 0.0;
        d4 += 0.5;
        return d4;
    }

    // ─── Populate ───
    // Java: BlockFalling.fallInstantly = true, biome.decorate, fallInstantly = false
    // The End has minimal decoration — mainly chorus plants in 1.9+,
    // but in 1.7.10 it's just the biome decorator (which does nothing for Sky biome)
};

} // namespace mccpp
