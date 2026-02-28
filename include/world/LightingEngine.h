/**
 * LightingEngine.h — Block and sky light propagation engine.
 *
 * Java references:
 *   - net.minecraft.world.EnumSkyBlock — Light type enum (Sky=15, Block=0)
 *   - net.minecraft.world.World.updateLightByType — BFS light propagation
 *   - net.minecraft.world.World.computeLightValue — Light value computation
 *   - net.minecraft.world.World.updateAllLightTypes — Updates both sky and block
 *
 * The lighting algorithm uses a flat array queue with packed coordinates:
 *   bits 0-5:   x offset from origin (biased by 32)
 *   bits 6-11:  y offset from origin (biased by 32)
 *   bits 12-17: z offset from origin (biased by 32)
 *   bits 18-21: light level (for darkening phase)
 *
 * Two phases:
 *   1. DARKEN: If new light < old light, BFS outward zeroing values,
 *      adding neighbors that were lit by this source.
 *   2. BRIGHTEN: BFS outward from all queue entries, propagating
 *      computed light values to neighbors that are dimmer.
 *
 * Max propagation radius: 17 blocks from origin.
 *
 * Block access abstracted via callbacks for thread safety.
 * JNI readiness: Simple arrays, predictable layout.
 */
#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EnumSkyBlock — Light type enum matching Java.
// Java reference: net.minecraft.world.EnumSkyBlock
// ═══════════════════════════════════════════════════════════════════════════

enum class SkyBlockType : int32_t {
    SKY = 0,    // Sky light (default value 15)
    BLOCK = 1   // Block light (default value 0)
};

constexpr int32_t getDefaultLightValue(SkyBlockType type) {
    return type == SkyBlockType::SKY ? 15 : 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Facing offsets — 6 cardinal directions.
// Java reference: net.minecraft.util.Facing
// ═══════════════════════════════════════════════════════════════════════════

namespace Facing {
    // Order: Down, Up, North, South, West, East
    constexpr int32_t offsetsX[6] = { 0,  0,  0,  0, -1,  1};
    constexpr int32_t offsetsY[6] = {-1,  1,  0,  0,  0,  0};
    constexpr int32_t offsetsZ[6] = { 0,  0, -1,  1,  0,  0};
}

// ═══════════════════════════════════════════════════════════════════════════
// LightingEngine — BFS-based light update propagation.
// Java reference: net.minecraft.world.World.updateLightByType
// ═══════════════════════════════════════════════════════════════════════════

class LightingEngine {
public:
    // Callback: (type, x, y, z) → saved light value [0..15]
    using GetLightFn = std::function<int32_t(SkyBlockType, int32_t, int32_t, int32_t)>;

    // Callback: (type, x, y, z, value) → set light value
    using SetLightFn = std::function<void(SkyBlockType, int32_t, int32_t, int32_t, int32_t)>;

    // Callback: (x, y, z) → block light opacity [0..255]
    using GetOpacityFn = std::function<int32_t(int32_t, int32_t, int32_t)>;

    // Callback: (x, y, z) → block light emission value [0..15]
    using GetEmissionFn = std::function<int32_t(int32_t, int32_t, int32_t)>;

    // Callback: (x, y, z) → can this block see the sky? (for sky light)
    using CanSeeSkyFn = std::function<bool(int32_t, int32_t, int32_t)>;

    // Callback: (x, y, z, radius) → do chunks near this position exist?
    using ChunksExistFn = std::function<bool(int32_t, int32_t, int32_t, int32_t)>;

    LightingEngine() = default;

    void setCallbacks(GetLightFn getLight, SetLightFn setLight,
                       GetOpacityFn getOpacity, GetEmissionFn getEmission,
                       CanSeeSkyFn canSeeSky, ChunksExistFn chunksExist) {
        getLight_ = std::move(getLight);
        setLight_ = std::move(setLight);
        getOpacity_ = std::move(getOpacity);
        getEmission_ = std::move(getEmission);
        canSeeSky_ = std::move(canSeeSky);
        chunksExist_ = std::move(chunksExist);
    }

    // Java: updateAllLightTypes — update both sky and block light
    bool updateAllLightTypes(int32_t x, int32_t y, int32_t z, bool hasSky) {
        bool changed = false;
        if (hasSky) {
            changed |= updateLightByType(SkyBlockType::SKY, x, y, z);
        }
        changed |= updateLightByType(SkyBlockType::BLOCK, x, y, z);
        return changed;
    }

    // Java: updateLightByType — the core BFS light propagation
    // Returns true if light was updated
    bool updateLightByType(SkyBlockType type, int32_t x, int32_t y, int32_t z) {
        if (!chunksExist_(x, y, z, 17)) {
            return false;
        }

        int32_t queueStart = 0;
        int32_t queueEnd = 0;

        int32_t savedLight = getLight_(type, x, y, z);
        int32_t computedLight = computeLightValue(x, y, z, type);

        if (computedLight > savedLight) {
            // Light increased: add origin to brighten queue
            // Java: 133152 = (32 | 32<<6 | 32<<12) = the origin packed
            queue_[queueEnd++] = 133152;
        } else if (computedLight < savedLight) {
            // Light decreased: darken phase
            // Java: 0x20820 | savedLight << 18
            queue_[queueEnd++] = 0x20820 | (savedLight << 18);

            while (queueStart < queueEnd) {
                int32_t packed = queue_[queueStart++];
                int32_t bx = (packed & 0x3F) - 32 + x;
                int32_t by = ((packed >> 6) & 0x3F) - 32 + y;
                int32_t bz = ((packed >> 12) & 0x3F) - 32 + z;
                int32_t level = (packed >> 18) & 0xF;

                int32_t currentLight = getLight_(type, bx, by, bz);
                if (currentLight != level) continue;

                // Zero out this position
                setLight_(type, bx, by, bz, 0);

                if (level <= 0) continue;

                // Manhattan distance check
                int32_t dx = std::abs(bx - x);
                int32_t dy = std::abs(by - y);
                int32_t dz = std::abs(bz - z);
                if (dx + dy + dz >= 17) continue;

                // Check 6 neighbors
                for (int32_t face = 0; face < 6; ++face) {
                    int32_t nx = bx + Facing::offsetsX[face];
                    int32_t ny = by + Facing::offsetsY[face];
                    int32_t nz = bz + Facing::offsetsZ[face];

                    int32_t opacity = std::max(1, getOpacity_(nx, ny, nz));
                    int32_t neighborLight = getLight_(type, nx, ny, nz);

                    if (neighborLight == level - opacity &&
                        queueEnd < static_cast<int32_t>(queue_.size())) {
                        queue_[queueEnd++] =
                            (nx - x + 32) |
                            ((ny - y + 32) << 6) |
                            ((nz - z + 32) << 12) |
                            ((level - opacity) << 18);
                    }
                }
            }

            queueStart = 0;
        }

        // Brighten phase: BFS propagation
        while (queueStart < queueEnd) {
            int32_t packed = queue_[queueStart++];
            int32_t bx = (packed & 0x3F) - 32 + x;
            int32_t by = ((packed >> 6) & 0x3F) - 32 + y;
            int32_t bz = ((packed >> 12) & 0x3F) - 32 + z;

            int32_t currentLight = getLight_(type, bx, by, bz);
            int32_t computedVal = computeLightValue(bx, by, bz, type);

            if (computedVal == currentLight) continue;

            setLight_(type, bx, by, bz, computedVal);

            if (computedVal <= currentLight) continue;

            // Manhattan distance check
            int32_t dx = std::abs(bx - x);
            int32_t dy = std::abs(by - y);
            int32_t dz = std::abs(bz - z);
            bool hasSpace = queueEnd < static_cast<int32_t>(queue_.size()) - 6;

            if (dx + dy + dz >= 17 || !hasSpace) continue;

            // Add dimmer neighbors to queue
            // Java: check each of 6 neighbors explicitly
            if (getLight_(type, bx - 1, by, bz) < computedVal) {
                queue_[queueEnd++] = (bx - 1 - x + 32) +
                    ((by - y + 32) << 6) + ((bz - z + 32) << 12);
            }
            if (getLight_(type, bx + 1, by, bz) < computedVal) {
                queue_[queueEnd++] = (bx + 1 - x + 32) +
                    ((by - y + 32) << 6) + ((bz - z + 32) << 12);
            }
            if (getLight_(type, bx, by - 1, bz) < computedVal) {
                queue_[queueEnd++] = (bx - x + 32) +
                    ((by - 1 - y + 32) << 6) + ((bz - z + 32) << 12);
            }
            if (getLight_(type, bx, by + 1, bz) < computedVal) {
                queue_[queueEnd++] = (bx - x + 32) +
                    ((by + 1 - y + 32) << 6) + ((bz - z + 32) << 12);
            }
            if (getLight_(type, bx, by, bz - 1) < computedVal) {
                queue_[queueEnd++] = (bx - x + 32) +
                    ((by - y + 32) << 6) + ((bz - 1 - z + 32) << 12);
            }
            if (getLight_(type, bx, by, bz + 1) < computedVal) {
                queue_[queueEnd++] = (bx - x + 32) +
                    ((by - y + 32) << 6) + ((bz + 1 - z + 32) << 12);
            }
        }

        return true;
    }

    // Java: computeLightValue — compute what the light value should be
    int32_t computeLightValue(int32_t x, int32_t y, int32_t z, SkyBlockType type) {
        // Sky light: if block can see sky, return 15
        if (type == SkyBlockType::SKY && canSeeSky_(x, y, z)) {
            return 15;
        }

        // Block emission (only for block light type)
        int32_t emission = (type == SkyBlockType::SKY) ? 0 : getEmission_(x, y, z);

        // Opacity of the block at this position
        int32_t opacity = getOpacity_(x, y, z);

        // Java: if opacity >= 15 && emission > 0, reduce opacity to 1
        if (opacity >= 15 && getEmission_(x, y, z) > 0) {
            opacity = 1;
        }
        if (opacity < 1) opacity = 1;

        // If fully opaque and not emitting, no light passes
        if (opacity >= 15) return 0;

        // If emission is already max-1, skip neighbor check
        if (emission >= 14) return emission;

        // Check 6 neighbors for max light minus opacity
        for (int32_t face = 0; face < 6; ++face) {
            int32_t nx = x + Facing::offsetsX[face];
            int32_t ny = y + Facing::offsetsY[face];
            int32_t nz = z + Facing::offsetsZ[face];

            int32_t neighborLight = getLight_(type, nx, ny, nz) - opacity;
            if (neighborLight > emission) {
                emission = neighborLight;
            }
            if (emission >= 14) return emission;
        }

        return emission;
    }

    // ─── Skylight subtraction (time-of-day dimming) ───

    // Java: calculateSkylightSubtracted
    static int32_t calculateSkylightSubtracted(float celestialAngle) {
        float f = 1.0f - (std::cos(celestialAngle * static_cast<float>(M_PI) * 2.0f) * 2.0f + 0.5f);
        if (f < 0.0f) f = 0.0f;
        if (f > 1.0f) f = 1.0f;
        return static_cast<int32_t>(f * 11.0f);
    }

    // ─── Light-related constants ───

    static constexpr int32_t MAX_LIGHT = 15;
    static constexpr int32_t MAX_PROPAGATION_RADIUS = 17;

    // Queue size: Java uses lightUpdateBlockList with 32768 entries
    static constexpr int32_t QUEUE_SIZE = 32768;

private:
    // Queue for BFS propagation — matches Java lightUpdateBlockList
    std::array<int32_t, QUEUE_SIZE> queue_{};

    // Callbacks
    GetLightFn getLight_;
    SetLightFn setLight_;
    GetOpacityFn getOpacity_;
    GetEmissionFn getEmission_;
    CanSeeSkyFn canSeeSky_;
    ChunksExistFn chunksExist_;
};

} // namespace mccpp
