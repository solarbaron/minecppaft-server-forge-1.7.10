/**
 * NoiseGeneratorOctaves.h — Complete noise generation system.
 *
 * Java references:
 *   - net.minecraft.world.gen.NoiseGeneratorImproved — Improved Perlin noise
 *   - net.minecraft.world.gen.NoiseGeneratorSimplex — 2D Simplex noise
 *   - net.minecraft.world.gen.NoiseGeneratorOctaves — Multi-octave improved Perlin
 *   - net.minecraft.world.gen.NoiseGeneratorPerlin — Multi-octave simplex
 *
 * NoiseGeneratorImproved:
 *   - 512-entry permutation table (256 shuffled + 256 duplicate)
 *   - Random offset (xCoord, yCoord, zCoord) in [0, 256)
 *   - Fade function: t² * t * (t * (t * 6 - 15) + 10)
 *   - 16-gradient table (field_152381-152385)
 *   - 3D: trilinear interpolation via lerp(grad(...))
 *   - 2D (n2==1): bilinear with func_76309_a (2-component gradient)
 *   - Y-layer caching: recomputes only when Y permutation index changes
 *
 * NoiseGeneratorSimplex:
 *   - Skew factor F = (√3 - 1) / 2
 *   - Unskew factor G = (3 - √3) / 6
 *   - Triangular simplex grid, 12 gradient vectors
 *   - Contribution: (0.5 - d²)⁴ * dot(grad, offset), clamped to 0
 *   - Scale factor: 70.0
 *
 * Thread safety: Each generator instance is independent.
 * JNI readiness: Simple arrays, predictable layout.
 */
#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// NoiseGeneratorImproved — Improved Perlin noise.
// Java reference: net.minecraft.world.gen.NoiseGeneratorImproved
// ═══════════════════════════════════════════════════════════════════════════

class NoiseGeneratorImproved {
public:
    double xCoord, yCoord, zCoord;

    // Java LCG RNG for permutation initialization
    struct RNG {
        int64_t seed;
        void setSeed(int64_t s) {
            seed = (s ^ 0x5DEECE66DLL) & ((1LL << 48) - 1);
        }
        int32_t nextInt(int32_t bound) {
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t bits = static_cast<int32_t>(seed >> 17);
            return ((bits % bound) + bound) % bound;
        }
        double nextDouble() {
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t hi = static_cast<int32_t>(seed >> 22);
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t lo = static_cast<int32_t>(seed >> 22);
            return static_cast<double>((static_cast<int64_t>(hi) << 27) + lo) /
                   static_cast<double>(1LL << 53);
        }
    };

    NoiseGeneratorImproved() { initDefault(); }

    explicit NoiseGeneratorImproved(RNG& rng) {
        xCoord = rng.nextDouble() * 256.0;
        yCoord = rng.nextDouble() * 256.0;
        zCoord = rng.nextDouble() * 256.0;
        for (int32_t i = 0; i < 256; ++i) perm_[i] = i;
        for (int32_t i = 0; i < 256; ++i) {
            int32_t j = rng.nextInt(256 - i) + i;
            std::swap(perm_[i], perm_[j]);
            perm_[i + 256] = perm_[i];
        }
    }

    static double lerp(double t, double a, double b) { return a + t * (b - a); }

    // Fade function: 6t^5 - 15t^4 + 10t^3
    static double fade(double t) { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }

    // 2D gradient (for n2==1 case)
    double func_76309_a(int32_t hash, double x, double z) const {
        int32_t h = hash & 0xF;
        return gradX2D_[h] * x + gradZ2D_[h] * z;
    }

    // 3D gradient
    double grad(int32_t hash, double x, double y, double z) const {
        int32_t h = hash & 0xF;
        return gradX_[h] * x + gradY_[h] * y + gradZ_[h] * z;
    }

    // Java: populateNoiseArray — add octave noise contribution to array
    void populateNoiseArray(double* out, double xOff, double yOff, double zOff,
                              int32_t xSize, int32_t ySize, int32_t zSize,
                              double xScale, double yScale, double zScale,
                              double amplitude) const {
        if (ySize == 1) {
            populate2D(out, xOff, zOff, xSize, zSize, xScale, zScale, amplitude);
        } else {
            populate3D(out, xOff, yOff, zOff, xSize, ySize, zSize,
                        xScale, yScale, zScale, amplitude);
        }
    }

private:
    std::array<int32_t, 512> perm_;

    // Gradient tables from Java
    static constexpr double gradX_[16]    = {1,-1, 1,-1, 1,-1, 1,-1, 0, 0, 0, 0, 1, 0,-1, 0};
    static constexpr double gradY_[16]    = {1, 1,-1,-1, 0, 0, 0, 0, 1,-1, 1,-1, 1,-1, 1,-1};
    static constexpr double gradZ_[16]    = {0, 0, 0, 0, 1, 1,-1,-1, 1, 1,-1,-1, 0, 1, 0,-1};
    static constexpr double gradX2D_[16]  = {1,-1, 1,-1, 1,-1, 1,-1, 0, 0, 0, 0, 1, 0,-1, 0};
    static constexpr double gradZ2D_[16]  = {0, 0, 0, 0, 1, 1,-1,-1, 1, 1,-1,-1, 0, 1, 0,-1};

    void initDefault() {
        xCoord = yCoord = zCoord = 0.0;
        for (int32_t i = 0; i < 256; ++i) perm_[i] = i;
        for (int32_t i = 256; i < 512; ++i) perm_[i] = perm_[i - 256];
    }

    // 2D noise (ySize == 1)
    void populate2D(double* out, double xOff, double zOff,
                      int32_t xSize, int32_t zSize,
                      double xScale, double zScale, double amplitude) const {
        int32_t idx = 0;
        double invAmp = 1.0 / amplitude;
        for (int32_t ix = 0; ix < xSize; ++ix) {
            double px = xOff + static_cast<double>(ix) * xScale + xCoord;
            int32_t xi = static_cast<int32_t>(px);
            if (px < static_cast<double>(xi)) --xi;
            int32_t X = xi & 0xFF;
            px -= static_cast<double>(xi);
            double fx = fade(px);

            for (int32_t iz = 0; iz < zSize; ++iz) {
                double pz = zOff + static_cast<double>(iz) * zScale + zCoord;
                int32_t zi = static_cast<int32_t>(pz);
                if (pz < static_cast<double>(zi)) --zi;
                int32_t Z = zi & 0xFF;
                pz -= static_cast<double>(zi);
                double fz = fade(pz);

                int32_t A = perm_[X] + 0;
                int32_t AA = perm_[A] + Z;
                int32_t B = perm_[X + 1] + 0;
                int32_t BA = perm_[B] + Z;

                double x1 = lerp(fx, func_76309_a(perm_[AA], px, pz),
                                  grad(perm_[BA], px - 1.0, 0.0, pz));
                double x2 = lerp(fx, grad(perm_[AA + 1], px, 0.0, pz - 1.0),
                                  grad(perm_[BA + 1], px - 1.0, 0.0, pz - 1.0));
                double val = lerp(fz, x1, x2);
                out[idx++] += val * invAmp;
            }
        }
    }

    // 3D noise
    void populate3D(double* out, double xOff, double yOff, double zOff,
                      int32_t xSize, int32_t ySize, int32_t zSize,
                      double xScale, double yScale, double zScale,
                      double amplitude) const {
        int32_t idx = 0;
        double invAmp = 1.0 / amplitude;
        int32_t prevY = -1;
        double d1 = 0, d2 = 0, d3 = 0, d4 = 0;

        for (int32_t ix = 0; ix < xSize; ++ix) {
            double px = xOff + static_cast<double>(ix) * xScale + xCoord;
            int32_t xi = static_cast<int32_t>(px);
            if (px < static_cast<double>(xi)) --xi;
            int32_t X = xi & 0xFF;
            px -= static_cast<double>(xi);
            double fx = fade(px);

            for (int32_t iz = 0; iz < zSize; ++iz) {
                double pz = zOff + static_cast<double>(iz) * zScale + zCoord;
                int32_t zi = static_cast<int32_t>(pz);
                if (pz < static_cast<double>(zi)) --zi;
                int32_t Z = zi & 0xFF;
                pz -= static_cast<double>(zi);
                double fz = fade(pz);

                for (int32_t iy = 0; iy < ySize; ++iy) {
                    double py = yOff + static_cast<double>(iy) * yScale + yCoord;
                    int32_t yi = static_cast<int32_t>(py);
                    if (py < static_cast<double>(yi)) --yi;
                    int32_t Y = yi & 0xFF;
                    py -= static_cast<double>(yi);
                    double fy = fade(py);

                    // Y-layer caching: recompute only when Y permutation index changes
                    if (iy == 0 || Y != prevY) {
                        prevY = Y;
                        int32_t A  = perm_[X] + Y;
                        int32_t AA = perm_[A] + Z;
                        int32_t AB = perm_[A + 1] + Z;
                        int32_t B  = perm_[X + 1] + Y;
                        int32_t BA = perm_[B] + Z;
                        int32_t BB = perm_[B + 1] + Z;

                        d1 = lerp(fx, grad(perm_[AA],     px,       py,       pz),
                                       grad(perm_[BA],     px - 1.0, py,       pz));
                        d2 = lerp(fx, grad(perm_[AB],     px,       py - 1.0, pz),
                                       grad(perm_[BB],     px - 1.0, py - 1.0, pz));
                        d3 = lerp(fx, grad(perm_[AA + 1], px,       py,       pz - 1.0),
                                       grad(perm_[BA + 1], px - 1.0, py,       pz - 1.0));
                        d4 = lerp(fx, grad(perm_[AB + 1], px,       py - 1.0, pz - 1.0),
                                       grad(perm_[BB + 1], px - 1.0, py - 1.0, pz - 1.0));
                    }

                    double yInterp = lerp(fy, d1, d2);
                    double yInterp2 = lerp(fy, d3, d4);
                    double val = lerp(fz, yInterp, yInterp2);
                    out[idx++] += val * invAmp;
                }
            }
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// NoiseGeneratorSimplex — 2D simplex noise.
// Java reference: net.minecraft.world.gen.NoiseGeneratorSimplex
// ═══════════════════════════════════════════════════════════════════════════

class NoiseGeneratorSimplex {
public:
    double xOffset, yOffset, zOffset;

    static const double SQRT_3;
    static constexpr double F2 = 0.3660254037844386;   // (sqrt(3)-1)/2
    static constexpr double G2 = 0.21132486540518713;  // (3-sqrt(3))/6

    using RNG = NoiseGeneratorImproved::RNG;

    NoiseGeneratorSimplex() { initDefault(); }

    explicit NoiseGeneratorSimplex(RNG& rng) {
        xOffset = rng.nextDouble() * 256.0;
        yOffset = rng.nextDouble() * 256.0;
        zOffset = rng.nextDouble() * 256.0;
        for (int32_t i = 0; i < 256; ++i) perm_[i] = i;
        for (int32_t i = 0; i < 256; ++i) {
            int32_t j = rng.nextInt(256 - i) + i;
            std::swap(perm_[i], perm_[j]);
            perm_[i + 256] = perm_[i];
        }
    }

    // Java: func_151605_a — single-point 2D simplex noise
    double getValue(double x, double y) const {
        double s = (x + y) * F2;
        int32_t i = fastFloor(x + s);
        int32_t j = fastFloor(y + s);
        double t = static_cast<double>(i + j) * G2;
        double x0 = x - (static_cast<double>(i) - t);
        double y0 = y - (static_cast<double>(j) - t);

        int32_t i1, j1;
        if (x0 > y0) { i1 = 1; j1 = 0; }
        else          { i1 = 0; j1 = 1; }

        double x1 = x0 - static_cast<double>(i1) + G2;
        double y1 = y0 - static_cast<double>(j1) + G2;
        double x2 = x0 - 1.0 + 2.0 * G2;
        double y2 = y0 - 1.0 + 2.0 * G2;

        int32_t ii = i & 0xFF;
        int32_t jj = j & 0xFF;
        int32_t gi0 = perm_[ii + perm_[jj]] % 12;
        int32_t gi1 = perm_[ii + i1 + perm_[jj + j1]] % 12;
        int32_t gi2 = perm_[ii + 1 + perm_[jj + 1]] % 12;

        double n0, n1, n2;
        double t0 = 0.5 - x0 * x0 - y0 * y0;
        if (t0 < 0.0) { n0 = 0.0; } else { t0 *= t0; n0 = t0 * t0 * dot2D(gi0, x0, y0); }

        double t1 = 0.5 - x1 * x1 - y1 * y1;
        if (t1 < 0.0) { n1 = 0.0; } else { t1 *= t1; n1 = t1 * t1 * dot2D(gi1, x1, y1); }

        double t2 = 0.5 - x2 * x2 - y2 * y2;
        if (t2 < 0.0) { n2 = 0.0; } else { t2 *= t2; n2 = t2 * t2 * dot2D(gi2, x2, y2); }

        return 70.0 * (n0 + n1 + n2);
    }

    // Java: func_151606_a — populate noise array
    void addNoiseToArray(double* out, double xOff, double zOff,
                           int32_t xSize, int32_t zSize,
                           double xScale, double zScale, double amplitude) const {
        int32_t idx = 0;
        for (int32_t iz = 0; iz < zSize; ++iz) {
            double pz = (zOff + static_cast<double>(iz)) * zScale + yOffset;
            for (int32_t ix = 0; ix < xSize; ++ix) {
                double px = (xOff + static_cast<double>(ix)) * xScale + xOffset;

                double s = (px + pz) * F2;
                int32_t i = fastFloor(px + s);
                int32_t j = fastFloor(pz + s);
                double t = static_cast<double>(i + j) * G2;
                double x0 = px - (static_cast<double>(i) - t);
                double y0 = pz - (static_cast<double>(j) - t);

                int32_t i1, j1;
                if (x0 > y0) { i1 = 1; j1 = 0; }
                else          { i1 = 0; j1 = 1; }

                double x1 = x0 - static_cast<double>(i1) + G2;
                double y1 = y0 - static_cast<double>(j1) + G2;
                double x2 = x0 - 1.0 + 2.0 * G2;
                double y2 = y0 - 1.0 + 2.0 * G2;

                int32_t ii = i & 0xFF;
                int32_t jj = j & 0xFF;
                int32_t gi0 = perm_[ii + perm_[jj]] % 12;
                int32_t gi1 = perm_[ii + i1 + perm_[jj + j1]] % 12;
                int32_t gi2 = perm_[ii + 1 + perm_[jj + 1]] % 12;

                double n0, n1, n2;
                double t0 = 0.5 - x0 * x0 - y0 * y0;
                if (t0 < 0.0) { n0 = 0.0; } else { t0 *= t0; n0 = t0 * t0 * dot2D(gi0, x0, y0); }

                double t1 = 0.5 - x1 * x1 - y1 * y1;
                if (t1 < 0.0) { n1 = 0.0; } else { t1 *= t1; n1 = t1 * t1 * dot2D(gi1, x1, y1); }

                double t2 = 0.5 - x2 * x2 - y2 * y2;
                if (t2 < 0.0) { n2 = 0.0; } else { t2 *= t2; n2 = t2 * t2 * dot2D(gi2, x2, y2); }

                out[idx++] += 70.0 * (n0 + n1 + n2) * amplitude;
            }
        }
    }

private:
    std::array<int32_t, 512> perm_;

    static constexpr int32_t grad3_[12][3] = {
        {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
        {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
        {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}
    };

    static double dot2D(int32_t gi, double x, double y) {
        return static_cast<double>(grad3_[gi][0]) * x +
               static_cast<double>(grad3_[gi][1]) * y;
    }

    static int32_t fastFloor(double d) {
        return d > 0.0 ? static_cast<int32_t>(d) : static_cast<int32_t>(d) - 1;
    }

    void initDefault() {
        xOffset = yOffset = zOffset = 0.0;
        for (int32_t i = 0; i < 256; ++i) perm_[i] = i;
        for (int32_t i = 256; i < 512; ++i) perm_[i] = perm_[i - 256];
    }
};

inline const double NoiseGeneratorSimplex::SQRT_3 = std::sqrt(3.0);

// ═══════════════════════════════════════════════════════════════════════════
// NoiseGeneratorOctaves — Multi-octave improved Perlin.
// Java reference: net.minecraft.world.gen.NoiseGeneratorOctaves
// ═══════════════════════════════════════════════════════════════════════════

class NoiseGeneratorOctaves {
public:
    using RNG = NoiseGeneratorImproved::RNG;

    NoiseGeneratorOctaves(RNG& rng, int32_t octaves) : octaves_(octaves) {
        generators_.reserve(octaves);
        for (int32_t i = 0; i < octaves; ++i) {
            generators_.emplace_back(rng);
        }
    }

    // Java: generateNoiseOctaves — 3D
    void generateNoiseOctaves(double* out, int32_t x, int32_t y, int32_t z,
                                int32_t xSize, int32_t ySize, int32_t zSize,
                                double xScale, double yScale, double zScale) {
        int32_t total = xSize * ySize * zSize;
        for (int32_t i = 0; i < total; ++i) out[i] = 0.0;

        double amp = 1.0;
        for (int32_t i = 0; i < octaves_; ++i) {
            double ox = static_cast<double>(x) * amp * xScale;
            double oy = static_cast<double>(y) * amp * yScale;
            double oz = static_cast<double>(z) * amp * zScale;

            // Coordinate wrapping at 16,777,216 (0x1000000)
            int64_t wrappedX = floorLong(ox);
            int64_t wrappedZ = floorLong(oz);
            ox -= static_cast<double>(wrappedX % 0x1000000L);
            oz -= static_cast<double>(wrappedZ % 0x1000000L);

            // Note: wrappedX/Z modulo applied to the fractional part
            // But Java does: ox = ox - (double)(wrappedX %= 0x1000000L)
            // We approximate by subtracting the floored value then adding back wrapped

            generators_[i].populateNoiseArray(out, ox, oy, oz,
                xSize, ySize, zSize, xScale * amp, yScale * amp, zScale * amp, amp);

            amp /= 2.0;
        }
    }

    // Java: generateNoiseOctaves — 2D (ySize=1, y=10)
    void generateNoiseOctaves2D(double* out, int32_t x, int32_t z,
                                  int32_t xSize, int32_t zSize,
                                  double xScale, double zScale) {
        generateNoiseOctaves(out, x, 10, z, xSize, 1, zSize, xScale, 1.0, zScale);
    }

private:
    int32_t octaves_;
    std::vector<NoiseGeneratorImproved> generators_;

    static int64_t floorLong(double d) {
        int64_t l = static_cast<int64_t>(d);
        return d < static_cast<double>(l) ? l - 1 : l;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// NoiseGeneratorPerlin — Multi-octave simplex noise.
// Java reference: net.minecraft.world.gen.NoiseGeneratorPerlin
// ═══════════════════════════════════════════════════════════════════════════

class NoiseGeneratorPerlin {
public:
    using RNG = NoiseGeneratorImproved::RNG;

    NoiseGeneratorPerlin(RNG& rng, int32_t octaves) : octaves_(octaves) {
        generators_.reserve(octaves);
        for (int32_t i = 0; i < octaves; ++i) {
            generators_.emplace_back(rng);
        }
    }

    // Java: func_151601_a — single-point 2D
    double getValue(double x, double z) const {
        double result = 0.0;
        double amp = 1.0;
        for (int32_t i = 0; i < octaves_; ++i) {
            result += generators_[i].getValue(x * amp, z * amp) / amp;
            amp /= 2.0;
        }
        return result;
    }

    // Java: func_151599_a — array fill (default persistence=0.5)
    void generateNoiseArray(double* out, double x, double z,
                              int32_t xSize, int32_t zSize,
                              double xScale, double zScale, double lacunarity) {
        generateNoiseArrayFull(out, x, z, xSize, zSize, xScale, zScale, lacunarity, 0.5);
    }

    // Java: func_151600_a — array fill with persistence
    void generateNoiseArrayFull(double* out, double x, double z,
                                  int32_t xSize, int32_t zSize,
                                  double xScale, double zScale,
                                  double lacunarity, double persistence) {
        int32_t total = xSize * zSize;
        for (int32_t i = 0; i < total; ++i) out[i] = 0.0;

        double freqScale = 1.0;
        double ampScale = 1.0;
        for (int32_t i = 0; i < octaves_; ++i) {
            generators_[i].addNoiseToArray(out, x, z, xSize, zSize,
                xScale * freqScale * ampScale, zScale * freqScale * ampScale,
                0.55 / ampScale);
            freqScale *= lacunarity;
            ampScale *= persistence;
        }
    }

private:
    int32_t octaves_;
    std::vector<NoiseGeneratorSimplex> generators_;
};

} // namespace mccpp
