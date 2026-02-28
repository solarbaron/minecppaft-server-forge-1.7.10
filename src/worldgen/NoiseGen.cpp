/**
 * NoiseGen.cpp — World generation noise implementation.
 *
 * Java references:
 *   net.minecraft.world.gen.NoiseGeneratorSimplex — 2D simplex (exact algorithm)
 *   net.minecraft.world.gen.NoiseGeneratorImproved — 3D improved Perlin (exact)
 *   net.minecraft.world.gen.NoiseGeneratorPerlin — Multi-octave simplex
 *   net.minecraft.world.gen.NoiseGeneratorOctaves — Multi-octave 3D Perlin
 *
 * CRITICAL: All permutation shuffling, gradient evaluation, fade curves,
 * and coordinate wrapping must match Java exactly for deterministic
 * world generation (same seed = same world).
 */

#include "worldgen/NoiseGen.h"
#include <cmath>

namespace mccpp {

// Java RNG helper: nextDouble() and nextInt() equivalents using mt19937
static double nextDouble(std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

static int nextInt(std::mt19937& rng, int bound) {
    std::uniform_int_distribution<int> dist(0, bound - 1);
    return dist(rng);
}

// ═════════════════════════════════════════════════════════════════════════════
// NoiseGeneratorSimplex
// ═════════════════════════════════════════════════════════════════════════════

// Java: sqrt(3.0) constants
static const double SQRT3 = std::sqrt(3.0);
static const double F2 = 0.5 * (SQRT3 - 1.0);     // Skew factor
static const double G2 = (3.0 - SQRT3) / 6.0;       // Unskew factor

NoiseGeneratorSimplex::NoiseGeneratorSimplex() {
    std::mt19937 rng(std::random_device{}());
    *this = NoiseGeneratorSimplex(rng);
}

NoiseGeneratorSimplex::NoiseGeneratorSimplex(std::mt19937& rng) {
    xOffset = nextDouble(rng) * 256.0;
    yOffset = nextDouble(rng) * 256.0;
    zOffset = nextDouble(rng) * 256.0;

    // Initialize permutation table (Fisher-Yates shuffle)
    for (int i = 0; i < 256; ++i) perm[i] = i;
    for (int i = 0; i < 256; ++i) {
        int j = nextInt(rng, 256 - i) + i;
        int tmp = perm[i];
        perm[i] = perm[j];
        perm[j] = tmp;
        perm[i + 256] = perm[i];
    }
}

double NoiseGeneratorSimplex::getValue(double x, double y) const {
    // Java: func_151605_a — exact 2D simplex noise algorithm
    double s = (x + y) * F2;
    int i = fastFloor(x + s);
    int j = fastFloor(y + s);

    double t = static_cast<double>(i + j) * G2;
    double X0 = static_cast<double>(i) - t;
    double x0 = x - X0;
    double Y0 = static_cast<double>(j) - t;
    double y0 = y - Y0;

    int i1, j1;
    if (x0 > y0) { i1 = 1; j1 = 0; }
    else          { i1 = 0; j1 = 1; }

    double x1 = x0 - static_cast<double>(i1) + G2;
    double y1 = y0 - static_cast<double>(j1) + G2;
    double x2 = x0 - 1.0 + 2.0 * G2;
    double y2 = y0 - 1.0 + 2.0 * G2;

    int ii = i & 0xFF;
    int jj = j & 0xFF;
    int gi0 = perm[ii + perm[jj]] % 12;
    int gi1 = perm[ii + i1 + perm[jj + j1]] % 12;
    int gi2 = perm[ii + 1 + perm[jj + 1]] % 12;

    double n0, n1, n2;
    double t0 = 0.5 - x0*x0 - y0*y0;
    if (t0 < 0.0) { n0 = 0.0; }
    else { t0 *= t0; n0 = t0 * t0 * dot2(GRAD3[gi0], x0, y0); }

    double t1 = 0.5 - x1*x1 - y1*y1;
    if (t1 < 0.0) { n1 = 0.0; }
    else { t1 *= t1; n1 = t1 * t1 * dot2(GRAD3[gi1], x1, y1); }

    double t2 = 0.5 - x2*x2 - y2*y2;
    if (t2 < 0.0) { n2 = 0.0; }
    else { t2 *= t2; n2 = t2 * t2 * dot2(GRAD3[gi2], x2, y2); }

    return 70.0 * (n0 + n1 + n2);
}

void NoiseGeneratorSimplex::fillArray(std::vector<double>& out,
                                        double xOff, double yOff,
                                        int xSize, int ySize,
                                        double xScale, double yScale,
                                        double amplitude) const {
    // Java: func_151606_a — exact array fill with simplex noise
    int idx = 0;
    for (int j = 0; j < ySize; ++j) {
        double yPos = (yOff + static_cast<double>(j)) * yScale + yOffset;
        for (int i = 0; i < xSize; ++i) {
            double xPos = (xOff + static_cast<double>(i)) * xScale + xOffset;

            double s = (xPos + yPos) * F2;
            int gi = fastFloor(xPos + s);
            int gj = fastFloor(yPos + s);

            double t = static_cast<double>(gi + gj) * G2;
            double X0 = static_cast<double>(gi) - t;
            double x0 = xPos - X0;
            double Y0 = static_cast<double>(gj) - t;
            double y0 = yPos - Y0;

            int i1, j1;
            if (x0 > y0) { i1 = 1; j1 = 0; }
            else          { i1 = 0; j1 = 1; }

            double x1 = x0 - static_cast<double>(i1) + G2;
            double y1 = y0 - static_cast<double>(j1) + G2;
            double x2 = x0 - 1.0 + 2.0 * G2;
            double y2 = y0 - 1.0 + 2.0 * G2;

            int ii = gi & 0xFF;
            int jj = gj & 0xFF;
            int g0 = perm[ii + perm[jj]] % 12;
            int g1 = perm[ii + i1 + perm[jj + j1]] % 12;
            int g2 = perm[ii + 1 + perm[jj + 1]] % 12;

            double n0, n1, n2;
            double t0 = 0.5 - x0*x0 - y0*y0;
            if (t0 < 0.0) n0 = 0.0;
            else { t0 *= t0; n0 = t0 * t0 * dot2(GRAD3[g0], x0, y0); }

            double t1 = 0.5 - x1*x1 - y1*y1;
            if (t1 < 0.0) n1 = 0.0;
            else { t1 *= t1; n1 = t1 * t1 * dot2(GRAD3[g1], x1, y1); }

            double t2 = 0.5 - x2*x2 - y2*y2;
            if (t2 < 0.0) n2 = 0.0;
            else { t2 *= t2; n2 = t2 * t2 * dot2(GRAD3[g2], x2, y2); }

            out[idx++] += 70.0 * (n0 + n1 + n2) * amplitude;
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// NoiseGeneratorImproved
// ═════════════════════════════════════════════════════════════════════════════

NoiseGeneratorImproved::NoiseGeneratorImproved() {
    std::mt19937 rng(std::random_device{}());
    *this = NoiseGeneratorImproved(rng);
}

NoiseGeneratorImproved::NoiseGeneratorImproved(std::mt19937& rng) {
    xCoord = nextDouble(rng) * 256.0;
    yCoord = nextDouble(rng) * 256.0;
    zCoord = nextDouble(rng) * 256.0;

    for (int i = 0; i < 256; ++i) permutations[i] = i;
    for (int i = 0; i < 256; ++i) {
        int j = nextInt(rng, 256 - i) + i;
        int tmp = permutations[i];
        permutations[i] = permutations[j];
        permutations[j] = tmp;
        permutations[i + 256] = permutations[i];
    }
}

void NoiseGeneratorImproved::populateNoiseArray(std::vector<double>& out,
                                                  double xOff, double yOff, double zOff,
                                                  int xSize, int ySize, int zSize,
                                                  double xScale, double yScale, double zScale,
                                                  double amplitude) const {
    double invAmp = 1.0 / amplitude;

    if (ySize == 1) {
        // Java: optimized 2D path (y=0 plane)
        int idx = 0;
        for (int xi = 0; xi < xSize; ++xi) {
            double dx = xOff + static_cast<double>(xi) * xScale + xCoord;
            int X = static_cast<int>(dx);
            if (dx < static_cast<double>(X)) --X;
            int X0 = X & 0xFF;
            dx -= static_cast<double>(X);
            double u = fade(dx);

            for (int zi = 0; zi < zSize; ++zi) {
                double dz = zOff + static_cast<double>(zi) * zScale + zCoord;
                int Z = static_cast<int>(dz);
                if (dz < static_cast<double>(Z)) --Z;
                int Z0 = Z & 0xFF;
                dz -= static_cast<double>(Z);
                double w = fade(dz);

                int A  = permutations[X0] + 0;
                int AA = permutations[A] + Z0;
                int B  = permutations[X0 + 1] + 0;
                int BA = permutations[B] + Z0;

                double l1 = lerp(u,
                    grad2(permutations[AA], dx, dz),
                    grad3(permutations[BA], dx - 1.0, 0.0, dz));
                double l2 = lerp(u,
                    grad3(permutations[AA + 1], dx, 0.0, dz - 1.0),
                    grad3(permutations[BA + 1], dx - 1.0, 0.0, dz - 1.0));

                out[idx++] += lerp(w, l1, l2) * invAmp;
            }
        }
        return;
    }

    // Full 3D path
    int idx = 0;
    int prevY = -1;
    double d17 = 0, d18 = 0, d19 = 0, d20 = 0;

    for (int xi = 0; xi < xSize; ++xi) {
        double dx = xOff + static_cast<double>(xi) * xScale + xCoord;
        int X = static_cast<int>(dx);
        if (dx < static_cast<double>(X)) --X;
        int X0 = X & 0xFF;
        dx -= static_cast<double>(X);
        double u = fade(dx);

        for (int zi = 0; zi < zSize; ++zi) {
            double dz = zOff + static_cast<double>(zi) * zScale + zCoord;
            int Z = static_cast<int>(dz);
            if (dz < static_cast<double>(Z)) --Z;
            int Z0 = Z & 0xFF;
            dz -= static_cast<double>(Z);
            double w = fade(dz);

            for (int yi = 0; yi < ySize; ++yi) {
                double dy = yOff + static_cast<double>(yi) * yScale + yCoord;
                int Y = static_cast<int>(dy);
                if (dy < static_cast<double>(Y)) --Y;
                int Y0 = Y & 0xFF;
                dy -= static_cast<double>(Y);
                double v = fade(dy);

                if (yi == 0 || Y0 != prevY) {
                    prevY = Y0;
                    int A  = permutations[X0] + Y0;
                    int AA = permutations[A] + Z0;
                    int AB = permutations[A + 1] + Z0;
                    int B  = permutations[X0 + 1] + Y0;
                    int BA = permutations[B] + Z0;
                    int BB = permutations[B + 1] + Z0;

                    d17 = lerp(u, grad3(permutations[AA], dx, dy, dz),
                                  grad3(permutations[BA], dx-1.0, dy, dz));
                    d18 = lerp(u, grad3(permutations[AB], dx, dy-1.0, dz),
                                  grad3(permutations[BB], dx-1.0, dy-1.0, dz));
                    d19 = lerp(u, grad3(permutations[AA+1], dx, dy, dz-1.0),
                                  grad3(permutations[BA+1], dx-1.0, dy, dz-1.0));
                    d20 = lerp(u, grad3(permutations[AB+1], dx, dy-1.0, dz-1.0),
                                  grad3(permutations[BB+1], dx-1.0, dy-1.0, dz-1.0));
                }

                double yz = lerp(v, d17, d18);
                double yz1 = lerp(v, d19, d20);
                out[idx++] += lerp(w, yz, yz1) * invAmp;
            }
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// NoiseGeneratorPerlin — Multi-octave simplex
// ═════════════════════════════════════════════════════════════════════════════

NoiseGeneratorPerlin::NoiseGeneratorPerlin(std::mt19937& rng, int octaves)
    : octaveCount(octaves) {
    generators.reserve(octaves);
    for (int i = 0; i < octaves; ++i) {
        generators.emplace_back(rng);
    }
}

double NoiseGeneratorPerlin::getValue(double x, double y) const {
    // Java: func_151601_a — sum octaves with halving frequency
    double total = 0.0;
    double freq = 1.0;
    for (int i = 0; i < octaveCount; ++i) {
        total += generators[i].getValue(x * freq, y * freq) / freq;
        freq /= 2.0;
    }
    return total;
}

std::vector<double> NoiseGeneratorPerlin::fillArray(std::vector<double>& out,
                                                      double xOff, double yOff,
                                                      int xSize, int ySize,
                                                      double xScale, double yScale,
                                                      double lacunarity, double persistence) const {
    if (out.empty() || static_cast<int>(out.size()) < xSize * ySize) {
        out.assign(xSize * ySize, 0.0);
    } else {
        std::fill(out.begin(), out.end(), 0.0);
    }

    double freq = 1.0;
    double amp = 1.0;
    for (int i = 0; i < octaveCount; ++i) {
        generators[i].fillArray(out, xOff, yOff, xSize, ySize,
                                xScale * freq * amp, yScale * freq * amp,
                                0.55 / amp);
        freq *= lacunarity;
        amp *= persistence;
    }
    return out;
}

// ═════════════════════════════════════════════════════════════════════════════
// NoiseGeneratorOctaves — Multi-octave 3D Perlin
// ═════════════════════════════════════════════════════════════════════════════

NoiseGeneratorOctaves::NoiseGeneratorOctaves(std::mt19937& rng, int octaves)
    : octaveCount(octaves) {
    generators.reserve(octaves);
    for (int i = 0; i < octaves; ++i) {
        generators.emplace_back(rng);
    }
}

// Java: MathHelper.floor_double_long
static int64_t floorLong(double d) {
    int64_t l = static_cast<int64_t>(d);
    return d < static_cast<double>(l) ? l - 1 : l;
}

std::vector<double> NoiseGeneratorOctaves::generateNoiseOctaves3D(
    std::vector<double>& out,
    int xOff, int yOff, int zOff,
    int xSize, int ySize, int zSize,
    double xScale, double yScale, double zScale) const {

    int total = xSize * ySize * zSize;
    if (out.empty() || static_cast<int>(out.size()) < total) {
        out.assign(total, 0.0);
    } else {
        std::fill(out.begin(), out.end(), 0.0);
    }

    double amp = 1.0;
    for (int i = 0; i < octaveCount; ++i) {
        // Java: coordinate wrapping at 16M to prevent FP drift
        double dx = static_cast<double>(xOff) * amp * xScale;
        double dy = static_cast<double>(yOff) * amp * yScale;
        double dz = static_cast<double>(zOff) * amp * zScale;

        int64_t lx = floorLong(dx);
        int64_t lz = floorLong(dz);
        dx -= static_cast<double>(lx % 0x1000000L);
        dz -= static_cast<double>(lz % 0x1000000L);

        generators[i].populateNoiseArray(out,
            dx, dy, dz,
            xSize, ySize, zSize,
            xScale * amp, yScale * amp, zScale * amp,
            amp);
        amp /= 2.0;
    }
    return out;
}

std::vector<double> NoiseGeneratorOctaves::generateNoiseOctaves2D(
    std::vector<double>& out,
    int xOff, int zOff,
    int xSize, int zSize,
    double xScale, double zScale) const {
    return generateNoiseOctaves3D(out, xOff, 10, zOff, xSize, 1, zSize,
                                   xScale, 1.0, zScale);
}

} // namespace mccpp
