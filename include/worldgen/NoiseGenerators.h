/**
 * NoiseGenerators.h — Perlin, Simplex, and octave noise generators.
 *
 * Java references:
 *   - net.minecraft.world.gen.NoiseGeneratorImproved (156 lines) — Improved Perlin
 *   - net.minecraft.world.gen.NoiseGeneratorSimplex (166 lines) — 2D Simplex
 *   - net.minecraft.world.gen.NoiseGeneratorPerlin (56 lines) — Octave Simplex
 *
 * Critical for seed-identical terrain generation. All constants, permutation
 * table construction, and fade curves must match Java exactly.
 *
 * Thread safety: Each generator instance is used by a single chunk gen thread.
 * JNI readiness: Simple arrays, predictable layout.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Java Random — Matches java.util.Random LCG for seed-identical generation
// ═══════════════════════════════════════════════════════════════════════════

class JavaRandom {
public:
    explicit JavaRandom(int64_t seed = 0) { setSeed(seed); }

    void setSeed(int64_t seed) {
        seed_ = (seed ^ 0x5DEECE66DLL) & ((1LL << 48) - 1);
    }

    int32_t next(int32_t bits) {
        seed_ = (seed_ * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1);
        return static_cast<int32_t>(seed_ >> (48 - bits));
    }

    int32_t nextInt() { return next(32); }

    int32_t nextInt(int32_t bound) {
        if ((bound & (bound - 1)) == 0) {
            return static_cast<int32_t>((static_cast<int64_t>(bound) * next(31)) >> 31);
        }
        int32_t bits, val;
        do {
            bits = next(31);
            val = bits % bound;
        } while (bits - val + (bound - 1) < 0);
        return val;
    }

    double nextDouble() {
        return (static_cast<double>(
            (static_cast<int64_t>(next(26)) << 27) + next(27)) / static_cast<double>(1LL << 53));
    }

private:
    int64_t seed_;
};

// ═══════════════════════════════════════════════════════════════════════════
// NoiseGeneratorImproved — Ken Perlin's improved noise.
// Java: net.minecraft.world.gen.NoiseGeneratorImproved (156 lines)
//
// Key details:
//   - 512-entry permutation table (256 unique, mirrored)
//   - Fade curve: t³(6t²-15t+10) — Perlin's 5th-order polynomial
//   - 16 gradient vectors (field_152381-152385)
//   - xCoord/yCoord/zCoord: random offsets [0, 256)
//   - populateNoiseArray: ADDITIVE (dArray[n] += noise * scale)
//   - 2D mode (height==1): uses func_76309_a (2D gradient)
//   - 3D mode: uses grad (3D gradient), Y-layer caching optimization
// ═══════════════════════════════════════════════════════════════════════════

class NoiseGeneratorImproved {
public:
    int32_t permutations[512];
    double xCoord, yCoord, zCoord;

    // 16 gradient vectors — must match Java exactly
    static constexpr double GRAD_X[] = {1,-1,1,-1,1,-1,1,-1,0,0,0,0,1,0,-1,0};
    static constexpr double GRAD_Y[] = {1,1,-1,-1,0,0,0,0,1,-1,1,-1,1,-1,1,-1};
    static constexpr double GRAD_Z[] = {0,0,0,0,1,1,-1,-1,1,1,-1,-1,0,1,0,-1};
    // 2D gradient vectors (for height==1 optimization)
    static constexpr double GRAD2_X[] = {1,-1,1,-1,1,-1,1,-1,0,0,0,0,1,0,-1,0};
    static constexpr double GRAD2_Z[] = {0,0,0,0,1,1,-1,-1,1,1,-1,-1,0,1,0,-1};

    NoiseGeneratorImproved() { init(JavaRandom(0)); }
    explicit NoiseGeneratorImproved(JavaRandom& rng) { init(rng); }

    void init(JavaRandom& rng) {
        xCoord = rng.nextDouble() * 256.0;
        yCoord = rng.nextDouble() * 256.0;
        zCoord = rng.nextDouble() * 256.0;
        for (int32_t i = 0; i < 256; ++i) permutations[i] = i;
        for (int32_t i = 0; i < 256; ++i) {
            int32_t j = rng.nextInt(256 - i) + i;
            int32_t tmp = permutations[i];
            permutations[i] = permutations[j];
            permutations[j] = tmp;
            permutations[i + 256] = permutations[i];
        }
    }

    static double lerp(double t, double a, double b) { return a + t * (b - a); }

    // Perlin fade: t³(6t²-15t+10)
    static double fade(double t) { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }

    // 2D gradient (used when height == 1)
    double grad2d(int32_t hash, double x, double z) const {
        int32_t h = hash & 0xF;
        return GRAD2_X[h] * x + GRAD2_Z[h] * z;
    }

    // 3D gradient
    static double grad(int32_t hash, double x, double y, double z) {
        int32_t h = hash & 0xF;
        return GRAD_X[h] * x + GRAD_Y[h] * y + GRAD_Z[h] * z;
    }

    // ─── Main noise array population ───
    // Java: populateNoiseArray — ADDITIVE to output array
    // d7 = 1/amplitude, used as scale: dArray[n] += noise / d7... actually: * (1/d7)
    void populateNoiseArray(double* out, double startX, double startY, double startZ,
                             int32_t sizeX, int32_t sizeY, int32_t sizeZ,
                             double scaleX, double scaleY, double scaleZ,
                             double amplitude) const {
        double invAmp = 1.0 / amplitude;

        if (sizeY == 1) {
            // 2D mode — height is 1, skip Y computation
            int32_t idx = 0;
            for (int32_t ix = 0; ix < sizeX; ++ix) {
                double dx = startX + ix * scaleX + xCoord;
                int32_t xi = static_cast<int32_t>(dx);
                if (dx < xi) --xi;
                int32_t xw = xi & 0xFF;
                dx -= xi;
                double fx = fade(dx);

                for (int32_t iz = 0; iz < sizeZ; ++iz) {
                    double dz = startZ + iz * scaleZ + zCoord;
                    int32_t zi = static_cast<int32_t>(dz);
                    if (dz < zi) --zi;
                    int32_t zw = zi & 0xFF;
                    dz -= zi;
                    double fz = fade(dz);

                    int32_t A = permutations[xw];
                    int32_t AA = permutations[A] + zw;
                    int32_t B = permutations[xw + 1];
                    int32_t BA = permutations[B] + zw;

                    double l1 = lerp(fx, grad2d(permutations[AA], dx, dz),
                                          grad(permutations[BA], dx - 1.0, 0.0, dz));
                    double l2 = lerp(fx, grad(permutations[AA + 1], dx, 0.0, dz - 1.0),
                                          grad(permutations[BA + 1], dx - 1.0, 0.0, dz - 1.0));
                    double result = lerp(fz, l1, l2);
                    out[idx++] += result * invAmp;
                }
            }
            return;
        }

        // 3D mode with Y-layer caching
        int32_t idx = 0;
        int32_t lastYi = -1;
        double d17 = 0, d18 = 0, d19 = 0, d20 = 0;

        for (int32_t ix = 0; ix < sizeX; ++ix) {
            double dx = startX + ix * scaleX + xCoord;
            int32_t xi = static_cast<int32_t>(dx);
            if (dx < xi) --xi;
            int32_t xw = xi & 0xFF;
            dx -= xi;
            double fx = fade(dx);

            for (int32_t iz = 0; iz < sizeZ; ++iz) {
                double dz = startZ + iz * scaleZ + zCoord;
                int32_t zi = static_cast<int32_t>(dz);
                if (dz < zi) --zi;
                int32_t zw = zi & 0xFF;
                dz -= zi;
                double fz = fade(dz);

                for (int32_t iy = 0; iy < sizeY; ++iy) {
                    double dy = startY + iy * scaleY + yCoord;
                    int32_t yi = static_cast<int32_t>(dy);
                    if (dy < yi) --yi;
                    int32_t yw = yi & 0xFF;
                    dy -= yi;
                    double fy = fade(dy);

                    if (iy == 0 || yw != lastYi) {
                        lastYi = yw;
                        int32_t A = permutations[xw] + yw;
                        int32_t AA = permutations[A] + zw;
                        int32_t AB = permutations[A + 1] + zw;
                        int32_t B = permutations[xw + 1] + yw;
                        int32_t BA = permutations[B] + zw;
                        int32_t BB = permutations[B + 1] + zw;

                        d17 = lerp(fx, grad(permutations[AA], dx, dy, dz),
                                        grad(permutations[BA], dx-1, dy, dz));
                        d18 = lerp(fx, grad(permutations[AB], dx, dy-1, dz),
                                        grad(permutations[BB], dx-1, dy-1, dz));
                        d19 = lerp(fx, grad(permutations[AA+1], dx, dy, dz-1),
                                        grad(permutations[BA+1], dx-1, dy, dz-1));
                        d20 = lerp(fx, grad(permutations[AB+1], dx, dy-1, dz-1),
                                        grad(permutations[BB+1], dx-1, dy-1, dz-1));
                    }

                    double ly1 = lerp(fy, d17, d18);
                    double ly2 = lerp(fy, d19, d20);
                    double result = lerp(fz, ly1, ly2);
                    out[idx++] += result * invAmp;
                }
            }
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// NoiseGeneratorSimplex — 2D Simplex noise.
// Java: net.minecraft.world.gen.NoiseGeneratorSimplex (166 lines)
//
// Key details:
//   - Skew factor F = 0.5*(sqrt(3)-1)
//   - Unskew factor G = (3-sqrt(3))/6
//   - 12 gradient vectors (3D, but only x,y used for 2D)
//   - Contribution radius: 0.5 - dx²-dz²
//   - Final scale: 70.0
//   - func_151606_a: ADDITIVE to output array (like populateNoiseArray)
// ═══════════════════════════════════════════════════════════════════════════

class NoiseGeneratorSimplex {
public:
    static constexpr double SQRT3 = 1.7320508075688772;
    static constexpr double F2 = 0.5 * (SQRT3 - 1.0);
    static constexpr double G2 = (3.0 - SQRT3) / 6.0;

    // 12 gradient vectors
    static constexpr int32_t GRAD3[12][3] = {
        {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
        {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
        {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}
    };

    int32_t perm[512];
    double xOff, yOff, zOff;

    NoiseGeneratorSimplex() { init(JavaRandom(0)); }
    explicit NoiseGeneratorSimplex(JavaRandom& rng) { init(rng); }

    void init(JavaRandom& rng) {
        xOff = rng.nextDouble() * 256.0;
        yOff = rng.nextDouble() * 256.0;
        zOff = rng.nextDouble() * 256.0;
        for (int32_t i = 0; i < 256; ++i) perm[i] = i;
        for (int32_t i = 0; i < 256; ++i) {
            int32_t j = rng.nextInt(256 - i) + i;
            int32_t tmp = perm[i];
            perm[i] = perm[j];
            perm[j] = tmp;
            perm[i + 256] = perm[i];
        }
    }

    static int32_t fastFloor(double v) { return v > 0 ? static_cast<int32_t>(v) : static_cast<int32_t>(v) - 1; }

    static double dot2(const int32_t g[3], double x, double y) {
        return static_cast<double>(g[0]) * x + static_cast<double>(g[1]) * y;
    }

    // Single point 2D simplex noise
    double getValue(double xin, double yin) const {
        double s = (xin + yin) * F2;
        int32_t i = fastFloor(xin + s);
        int32_t j = fastFloor(yin + s);
        double t = static_cast<double>(i + j) * G2;
        double x0 = xin - (static_cast<double>(i) - t);
        double y0 = yin - (static_cast<double>(j) - t);

        int32_t i1, j1;
        if (x0 > y0) { i1 = 1; j1 = 0; }
        else { i1 = 0; j1 = 1; }

        double x1 = x0 - i1 + G2;
        double y1 = y0 - j1 + G2;
        double x2 = x0 - 1.0 + 2.0 * G2;
        double y2 = y0 - 1.0 + 2.0 * G2;

        int32_t ii = i & 0xFF;
        int32_t jj = j & 0xFF;
        int32_t gi0 = perm[ii + perm[jj]] % 12;
        int32_t gi1 = perm[ii + i1 + perm[jj + j1]] % 12;
        int32_t gi2 = perm[ii + 1 + perm[jj + 1]] % 12;

        double n0, n1, n2;
        double t0 = 0.5 - x0*x0 - y0*y0;
        if (t0 < 0) n0 = 0;
        else { t0 *= t0; n0 = t0 * t0 * dot2(GRAD3[gi0], x0, y0); }

        double t1 = 0.5 - x1*x1 - y1*y1;
        if (t1 < 0) n1 = 0;
        else { t1 *= t1; n1 = t1 * t1 * dot2(GRAD3[gi1], x1, y1); }

        double t2 = 0.5 - x2*x2 - y2*y2;
        if (t2 < 0) n2 = 0;
        else { t2 *= t2; n2 = t2 * t2 * dot2(GRAD3[gi2], x2, y2); }

        return 70.0 * (n0 + n1 + n2);
    }

    // Populate array — ADDITIVE
    // Java: func_151606_a
    void populateArray(double* out, double startX, double startZ,
                        int32_t sizeX, int32_t sizeZ,
                        double scaleX, double scaleZ, double amplitude) const {
        int32_t idx = 0;
        for (int32_t iz = 0; iz < sizeZ; ++iz) {
            double zIn = (startZ + iz) * scaleZ + yOff;
            for (int32_t ix = 0; ix < sizeX; ++ix) {
                double xIn = (startX + ix) * scaleX + xOff;

                double s = (xIn + zIn) * F2;
                int32_t i = fastFloor(xIn + s);
                int32_t j = fastFloor(zIn + s);
                double t = static_cast<double>(i + j) * G2;
                double x0 = xIn - (static_cast<double>(i) - t);
                double y0 = zIn - (static_cast<double>(j) - t);

                int32_t i1, j1;
                if (x0 > y0) { i1 = 1; j1 = 0; }
                else { i1 = 0; j1 = 1; }

                double x1 = x0 - i1 + G2;
                double y1 = y0 - j1 + G2;
                double x2 = x0 - 1.0 + 2.0 * G2;
                double y2 = y0 - 1.0 + 2.0 * G2;

                int32_t ii = i & 0xFF;
                int32_t jj = j & 0xFF;
                int32_t gi0 = perm[ii + perm[jj]] % 12;
                int32_t gi1 = perm[ii + i1 + perm[jj + j1]] % 12;
                int32_t gi2 = perm[ii + 1 + perm[jj + 1]] % 12;

                double n0, n1, n2;
                double t0 = 0.5 - x0*x0 - y0*y0;
                if (t0 < 0) n0 = 0;
                else { t0 *= t0; n0 = t0 * t0 * dot2(GRAD3[gi0], x0, y0); }

                double t1 = 0.5 - x1*x1 - y1*y1;
                if (t1 < 0) n1 = 0;
                else { t1 *= t1; n1 = t1 * t1 * dot2(GRAD3[gi1], x1, y1); }

                double t2 = 0.5 - x2*x2 - y2*y2;
                if (t2 < 0) n2 = 0;
                else { t2 *= t2; n2 = t2 * t2 * dot2(GRAD3[gi2], x2, y2); }

                out[idx++] += 70.0 * (n0 + n1 + n2) * amplitude;
            }
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// NoiseGeneratorPerlin — Octave simplex noise.
// Java: net.minecraft.world.gen.NoiseGeneratorPerlin (56 lines)
//
// Multiple simplex octaves with lacunarity and persistence.
// func_151601_a: single-point octave sum
// func_151600_a: array population with octave sum
// ═══════════════════════════════════════════════════════════════════════════

class NoiseGeneratorPerlin {
public:
    std::vector<NoiseGeneratorSimplex> octaves;
    int32_t numOctaves;

    NoiseGeneratorPerlin(JavaRandom& rng, int32_t octaveCount)
        : numOctaves(octaveCount) {
        octaves.reserve(octaveCount);
        for (int32_t i = 0; i < octaveCount; ++i) {
            octaves.emplace_back(rng);
        }
    }

    // Single point evaluation
    double getValue(double x, double z) const {
        double sum = 0;
        double freq = 1.0;
        for (int32_t i = 0; i < numOctaves; ++i) {
            sum += octaves[i].getValue(x * freq, z * freq) / freq;
            freq /= 2.0;
        }
        return sum;
    }

    // Array population — Java: func_151600_a
    // lacunarity controls frequency doubling, persistence controls amplitude
    std::vector<double> populateArray(std::vector<double>& out,
                                        double startX, double startZ,
                                        int32_t sizeX, int32_t sizeZ,
                                        double scaleX, double scaleZ,
                                        double lacunarity, double persistence = 0.5) {
        if (out.empty() || static_cast<int32_t>(out.size()) < sizeX * sizeZ) {
            out.resize(sizeX * sizeZ);
        }
        std::fill(out.begin(), out.end(), 0.0);

        double freq = 1.0;
        double amp = 1.0;
        for (int32_t i = 0; i < numOctaves; ++i) {
            octaves[i].populateArray(out.data(), startX, startZ,
                                      sizeX, sizeZ,
                                      scaleX * freq * amp,
                                      scaleZ * freq * amp,
                                      0.55 / amp);
            freq *= lacunarity;
            amp *= persistence;
        }
        return out;
    }
};

} // namespace mccpp
