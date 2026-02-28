/**
 * NoiseGen.h — World generation noise generators.
 *
 * Java references:
 *   - net.minecraft.world.gen.NoiseGeneratorSimplex — 2D simplex noise
 *   - net.minecraft.world.gen.NoiseGeneratorImproved — 3D improved Perlin noise
 *   - net.minecraft.world.gen.NoiseGeneratorPerlin — Multi-octave simplex
 *   - net.minecraft.world.gen.NoiseGeneratorOctaves — Multi-octave 3D Perlin
 *
 * All constants, gradient tables, and permutation initialization match the
 * decompiled Java source exactly for deterministic world generation parity.
 *
 * Thread safety:
 *   - Each generator instance is seeded once and read-only after construction.
 *   - Noise evaluation is pure function (thread-safe for concurrent reads).
 *   - Array-filling methods write to caller-owned buffers.
 *
 * JNI readiness: Simple structs with no virtual dispatch.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// NoiseGeneratorSimplex — 2D simplex noise.
// Java reference: net.minecraft.world.gen.NoiseGeneratorSimplex
//
// Uses 12-entry gradient table and 512-entry permutation table.
// Produces noise in range approximately [-1, 1] scaled by 70.
// ═══════════════════════════════════════════════════════════════════════════

class NoiseGeneratorSimplex {
public:
    NoiseGeneratorSimplex();
    explicit NoiseGeneratorSimplex(std::mt19937& rng);

    // Java: func_151605_a — Evaluate 2D simplex noise at (x, y)
    double getValue(double x, double y) const;

    // Java: func_151606_a — Fill array with noise (accumulating)
    void fillArray(std::vector<double>& out,
                   double xOffset, double yOffset,
                   int xSize, int ySize,
                   double xScale, double yScale,
                   double amplitude) const;

    double xOffset, yOffset, zOffset;

private:
    int perm[512];

    // Java: field_151611_e — 12 gradient vectors (3D, but only 2D used)
    static constexpr int GRAD3[12][3] = {
        {1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
        {1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
        {0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1}
    };

    static int fastFloor(double d) {
        return d > 0.0 ? static_cast<int>(d) : static_cast<int>(d) - 1;
    }

    static double dot2(const int g[3], double x, double y) {
        return static_cast<double>(g[0]) * x + static_cast<double>(g[1]) * y;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// NoiseGeneratorImproved — 3D improved Perlin noise.
// Java reference: net.minecraft.world.gen.NoiseGeneratorImproved
//
// Uses quintic fade curve: 6t^5 - 15t^4 + 10t^3
// 512-entry permutation table, 16-entry gradient tables.
// ═══════════════════════════════════════════════════════════════════════════

class NoiseGeneratorImproved {
public:
    NoiseGeneratorImproved();
    explicit NoiseGeneratorImproved(std::mt19937& rng);

    // Java: populateNoiseArray — Fill 3D noise volume (accumulating)
    void populateNoiseArray(std::vector<double>& out,
                            double xOff, double yOff, double zOff,
                            int xSize, int ySize, int zSize,
                            double xScale, double yScale, double zScale,
                            double amplitude) const;

    double xCoord, yCoord, zCoord;

private:
    int permutations[512];

    // Gradient tables — Java: field_152381-385
    static constexpr double GRAD_X[16] = {1,-1,1,-1,1,-1,1,-1,0,0,0,0,1,0,-1,0};
    static constexpr double GRAD_Y[16] = {1,1,-1,-1,0,0,0,0,1,-1,1,-1,1,-1,1,-1};
    static constexpr double GRAD_Z[16] = {0,0,0,0,1,1,-1,-1,1,1,-1,-1,0,1,0,-1};
    // 2D gradient (for y=1 path)
    static constexpr double GRAD2_X[16] = {1,-1,1,-1,1,-1,1,-1,0,0,0,0,1,0,-1,0};
    static constexpr double GRAD2_Z[16] = {0,0,0,0,1,1,-1,-1,1,1,-1,-1,0,1,0,-1};

    static double lerp(double t, double a, double b) { return a + t * (b - a); }

    static double grad3(int hash, double x, double y, double z) {
        int h = hash & 0xF;
        return GRAD_X[h] * x + GRAD_Y[h] * y + GRAD_Z[h] * z;
    }

    static double grad2(int hash, double x, double z) {
        int h = hash & 0xF;
        return GRAD2_X[h] * x + GRAD2_Z[h] * z;
    }

    // Quintic fade: 6t^5 - 15t^4 + 10t^3
    static double fade(double t) {
        return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// NoiseGeneratorPerlin — Multi-octave 2D simplex noise.
// Java reference: net.minecraft.world.gen.NoiseGeneratorPerlin
//
// Combines N octaves of simplex noise with halving amplitude.
// ═══════════════════════════════════════════════════════════════════════════

class NoiseGeneratorPerlin {
public:
    NoiseGeneratorPerlin(std::mt19937& rng, int octaves);

    // Java: func_151601_a — Single 2D sample
    double getValue(double x, double y) const;

    // Java: func_151600_a — Fill 2D array with multi-octave noise
    std::vector<double> fillArray(std::vector<double>& out,
                                   double xOff, double yOff,
                                   int xSize, int ySize,
                                   double xScale, double yScale,
                                   double lacunarity, double persistence = 0.5) const;

private:
    std::vector<NoiseGeneratorSimplex> generators;
    int octaveCount;
};

// ═══════════════════════════════════════════════════════════════════════════
// NoiseGeneratorOctaves — Multi-octave 3D improved Perlin noise.
// Java reference: net.minecraft.world.gen.NoiseGeneratorOctaves
//
// Combines N octaves of 3D Perlin noise with halving amplitude.
// Coordinates wrap at 16,777,216 (0x1000000) to prevent floating-point drift.
// ═══════════════════════════════════════════════════════════════════════════

class NoiseGeneratorOctaves {
public:
    NoiseGeneratorOctaves(std::mt19937& rng, int octaves);

    // Java: generateNoiseOctaves (3D)
    std::vector<double> generateNoiseOctaves3D(std::vector<double>& out,
                                                 int xOff, int yOff, int zOff,
                                                 int xSize, int ySize, int zSize,
                                                 double xScale, double yScale, double zScale) const;

    // Java: generateNoiseOctaves (2D — fixed Y=10, ySize=1)
    std::vector<double> generateNoiseOctaves2D(std::vector<double>& out,
                                                 int xOff, int zOff,
                                                 int xSize, int zSize,
                                                 double xScale, double zScale) const;

private:
    std::vector<NoiseGeneratorImproved> generators;
    int octaveCount;
};

} // namespace mccpp
