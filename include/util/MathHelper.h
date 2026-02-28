/**
 * MathHelper.h — Minecraft math utilities with exact Java parity.
 *
 * Java reference: net.minecraft.util.MathHelper
 *
 * Key features:
 *   - 65536-entry sin lookup table (pre-computed)
 *   - floor_float/double with negative correction
 *   - Angle wrapping to [-180, 180)
 *   - De Bruijn bit position table
 *
 * Thread safety: Immutable after static init.
 *
 * JNI readiness: All static functions, trivially callable.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <random>

namespace mccpp {
namespace MathHelper {

// ═══════════════════════════════════════════════════════════════════════════
// Sin lookup table — 65536 entries, pre-computed.
// Java: private static float[] SIN_TABLE = new float[65536]
// ═══════════════════════════════════════════════════════════════════════════

namespace detail {
    struct SinTable {
        float table[65536];
        SinTable() {
            for (int i = 0; i < 65536; ++i) {
                table[i] = static_cast<float>(
                    std::sin(static_cast<double>(i) * 3.141592653589793 * 2.0 / 65536.0));
            }
        }
    };
    inline const SinTable& getSinTable() {
        static const SinTable inst;
        return inst;
    }
}

// Java: sin(float) — lookup table sin
inline float sin(float f) {
    return detail::getSinTable().table[static_cast<int>(f * 10430.378f) & 0xFFFF];
}

// Java: cos(float) — lookup table cos
inline float cos(float f) {
    return detail::getSinTable().table[static_cast<int>(f * 10430.378f + 16384.0f) & 0xFFFF];
}

// Java: sqrt_float
inline float sqrt_float(float f) {
    return static_cast<float>(std::sqrt(f));
}

// Java: sqrt_double
inline float sqrt_double(double d) {
    return static_cast<float>(std::sqrt(d));
}

// ═══════════════════════════════════════════════════════════════════════════
// Floor — Java's exact negative-correcting floor.
// Java: floor_float, floor_double, floor_double_long
// ═══════════════════════════════════════════════════════════════════════════

inline int32_t floor_float(float f) {
    int32_t n = static_cast<int32_t>(f);
    return f < static_cast<float>(n) ? n - 1 : n;
}

inline int32_t floor_double(double d) {
    int32_t n = static_cast<int32_t>(d);
    return d < static_cast<double>(n) ? n - 1 : n;
}

inline int64_t floor_double_long(double d) {
    int64_t n = static_cast<int64_t>(d);
    return d < static_cast<double>(n) ? n - 1L : n;
}

// ═══════════════════════════════════════════════════════════════════════════
// Ceiling
// Java: ceiling_float_int, ceiling_double_int
// ═══════════════════════════════════════════════════════════════════════════

inline int32_t ceiling_float_int(float f) {
    int32_t n = static_cast<int32_t>(f);
    return f > static_cast<float>(n) ? n + 1 : n;
}

inline int32_t ceiling_double_int(double d) {
    int32_t n = static_cast<int32_t>(d);
    return d > static_cast<double>(n) ? n + 1 : n;
}

// ═══════════════════════════════════════════════════════════════════════════
// Abs
// Java: abs(float), abs_int(int)
// ═══════════════════════════════════════════════════════════════════════════

inline float abs(float f) { return f >= 0.0f ? f : -f; }
inline int32_t abs_int(int32_t n) { return n >= 0 ? n : -n; }

// ═══════════════════════════════════════════════════════════════════════════
// Clamp — Java's exact clamp (not std::clamp, 3-way if)
// Java: clamp_int, clamp_float, clamp_double
// ═══════════════════════════════════════════════════════════════════════════

inline int32_t clamp_int(int32_t value, int32_t min, int32_t max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

inline float clamp_float(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

inline double clamp_double(double value, double min, double max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// ═══════════════════════════════════════════════════════════════════════════
// Interpolation
// Java: denormalizeClamp
// ═══════════════════════════════════════════════════════════════════════════

inline double denormalizeClamp(double min, double max, double t) {
    if (t < 0.0) return min;
    if (t > 1.0) return max;
    return min + (max - min) * t;
}

// Java: abs_max — larger absolute value
inline double abs_max(double a, double b) {
    if (a < 0.0) a = -a;
    if (b < 0.0) b = -b;
    return a > b ? a : b;
}

// ═══════════════════════════════════════════════════════════════════════════
// Angle wrapping — to [-180, 180)
// Java: wrapAngleTo180_float, wrapAngleTo180_double
// ═══════════════════════════════════════════════════════════════════════════

inline float wrapAngleTo180_float(float f) {
    f = std::fmod(f, 360.0f);
    if (f >= 180.0f) f -= 360.0f;
    if (f < -180.0f) f += 360.0f;
    return f;
}

inline double wrapAngleTo180_double(double d) {
    d = std::fmod(d, 360.0);
    if (d >= 180.0) d -= 360.0;
    if (d < -180.0) d += 360.0;
    return d;
}

// ═══════════════════════════════════════════════════════════════════════════
// String parsing with defaults
// Java: parseIntWithDefault, parseIntWithDefaultAndMax,
//       parseDoubleWithDefault, parseDoubleWithDefaultAndMax
// ═══════════════════════════════════════════════════════════════════════════

inline int32_t parseIntWithDefault(const std::string& str, int32_t def) {
    try { return std::stoi(str); } catch (...) { return def; }
}

inline int32_t parseIntWithDefaultAndMax(const std::string& str, int32_t def, int32_t min) {
    int32_t val = parseIntWithDefault(str, def);
    return val < min ? min : val;
}

inline double parseDoubleWithDefault(const std::string& str, double def) {
    try { return std::stod(str); } catch (...) { return def; }
}

inline double parseDoubleWithDefaultAndMax(const std::string& str, double def, double min) {
    double val = parseDoubleWithDefault(str, def);
    return val < min ? min : val;
}

// ═══════════════════════════════════════════════════════════════════════════
// Random helpers
// Java: getRandomIntegerInRange, randomFloatClamp, getRandomDoubleInRange
// ═══════════════════════════════════════════════════════════════════════════

inline int32_t getRandomIntegerInRange(std::mt19937& rng, int32_t min, int32_t max) {
    if (min >= max) return min;
    std::uniform_int_distribution<int32_t> dist(min, max);
    return dist(rng);
}

inline float randomFloatClamp(std::mt19937& rng, float min, float max) {
    if (min >= max) return min;
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

inline double getRandomDoubleInRange(std::mt19937& rng, double min, double max) {
    if (min >= max) return min;
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}

// ═══════════════════════════════════════════════════════════════════════════
// De Bruijn bit position
// Java: multiplyDeBruijnBitPosition
// ═══════════════════════════════════════════════════════════════════════════

constexpr int32_t DEBRUIJN_BIT_POS[32] = {
    0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
    31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

// ═══════════════════════════════════════════════════════════════════════════
// Average of long array
// Java: average(long[])
// ═══════════════════════════════════════════════════════════════════════════

inline double average(const int64_t* arr, int32_t len) {
    int64_t sum = 0;
    for (int32_t i = 0; i < len; ++i) sum += arr[i];
    return static_cast<double>(sum) / static_cast<double>(len);
}

} // namespace MathHelper
} // namespace mccpp
