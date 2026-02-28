/**
 * WorldProvider.h — Dimension system.
 *
 * Java references:
 *   - net.minecraft.world.WorldProvider — Abstract base
 *   - net.minecraft.world.WorldProviderSurface — Overworld (ID 0)
 *   - net.minecraft.world.WorldProviderHell — Nether (ID -1)
 *   - net.minecraft.world.WorldProviderEnd — The End (ID 1)
 *
 * Key properties per dimension:
 *   dimensionId, isHellWorld, hasNoSky, isSurfaceWorld,
 *   canRespawnHere, celestialAngle, moonPhase, lightBrightnessTable
 *
 * Thread safety: Read-only after construction. Per-world instance.
 *
 * JNI readiness: Integer dimension IDs, simple struct layout.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// WorldProvider — Dimension properties and calculations.
// Java reference: net.minecraft.world.WorldProvider
// ═══════════════════════════════════════════════════════════════════════════

struct WorldProvider {
    int32_t dimensionId = 0;
    std::string dimensionName = "Overworld";
    bool isHellWorld = false;
    bool hasNoSky = false;
    bool surfaceWorld = true;
    bool canRespawn = true;
    int32_t averageGroundLevel = 64;

    // Java: WorldProviderEnd.getEntrancePortalLocation = (100, 50, 0)
    int32_t portalX = 0, portalY = 0, portalZ = 0;
    bool hasEntrancePortal = false;

    // Java: moonPhaseFactors — 8 moon phases
    static constexpr float moonPhaseFactors[8] = {
        1.0f, 0.75f, 0.5f, 0.25f, 0.0f, 0.25f, 0.5f, 0.75f
    };

    // Java: lightBrightnessTable[16] — computed per dimension
    float lightBrightnessTable[16] = {};

    // ─── Light brightness table generation ───

    // Java: WorldProvider.generateLightBrightnessTable (overworld: ambient=0.0)
    void generateLightBrightnessTable(float ambient = 0.0f) {
        for (int i = 0; i <= 15; ++i) {
            float f = 1.0f - static_cast<float>(i) / 15.0f;
            lightBrightnessTable[i] = (1.0f - f) / (f * 3.0f + 1.0f) * (1.0f - ambient) + ambient;
        }
    }

    // ─── Celestial calculations ───

    // Java: WorldProvider.calculateCelestialAngle
    float calculateCelestialAngle(int64_t worldTime, float partialTicks) const {
        if (dimensionId == -1) return 0.5f;  // Nether: always dusk
        if (dimensionId == 1) return 0.0f;    // End: always midnight

        // Overworld: day/night cycle
        int32_t timeOfDay = static_cast<int32_t>(worldTime % 24000L);
        float angle = (static_cast<float>(timeOfDay) + partialTicks) / 24000.0f - 0.25f;
        if (angle < 0.0f) angle += 1.0f;
        if (angle > 1.0f) angle -= 1.0f;

        float raw = angle;
        angle = 1.0f - static_cast<float>((std::cos(static_cast<double>(angle) * M_PI) + 1.0) / 2.0);
        angle = raw + (angle - raw) / 3.0f;
        return angle;
    }

    // Java: WorldProvider.getMoonPhase
    int32_t getMoonPhase(int64_t worldTime) const {
        return static_cast<int32_t>((worldTime / 24000L % 8L + 8L) % 8L);
    }

    // Java: WorldProvider.getAverageGroundLevel
    // Overworld: 64 (or 4 for flat), Nether: 64, End: 50
    int32_t getAverageGround() const { return averageGroundLevel; }
};

// ═══════════════════════════════════════════════════════════════════════════
// Dimension factory and presets.
// Java reference: WorldProvider.getProviderForDimension
// ═══════════════════════════════════════════════════════════════════════════

namespace Dimensions {

    // Java: WorldProviderSurface — Overworld (ID 0)
    inline WorldProvider createOverworld() {
        WorldProvider p;
        p.dimensionId = 0;
        p.dimensionName = "Overworld";
        p.isHellWorld = false;
        p.hasNoSky = false;
        p.surfaceWorld = true;
        p.canRespawn = true;
        p.averageGroundLevel = 64;
        p.generateLightBrightnessTable(0.0f);  // ambient = 0.0
        return p;
    }

    // Java: WorldProviderHell — Nether (ID -1)
    inline WorldProvider createNether() {
        WorldProvider p;
        p.dimensionId = -1;
        p.dimensionName = "Nether";
        p.isHellWorld = true;
        p.hasNoSky = true;
        p.surfaceWorld = false;
        p.canRespawn = false;
        p.averageGroundLevel = 64;
        p.generateLightBrightnessTable(0.1f);  // ambient = 0.1 (nether glow)
        return p;
    }

    // Java: WorldProviderEnd — The End (ID 1)
    inline WorldProvider createEnd() {
        WorldProvider p;
        p.dimensionId = 1;
        p.dimensionName = "The End";
        p.isHellWorld = false;
        p.hasNoSky = true;
        p.surfaceWorld = false;
        p.canRespawn = false;
        p.averageGroundLevel = 50;
        p.hasEntrancePortal = true;
        p.portalX = 100; p.portalY = 50; p.portalZ = 0;
        p.generateLightBrightnessTable(0.0f);
        return p;
    }

    // Java: WorldProvider.getProviderForDimension
    inline WorldProvider createForDimension(int32_t id) {
        switch (id) {
            case -1: return createNether();
            case 0:  return createOverworld();
            case 1:  return createEnd();
            default: return createOverworld();
        }
    }

    // Dimension ID constants
    constexpr int32_t NETHER_ID    = -1;
    constexpr int32_t OVERWORLD_ID = 0;
    constexpr int32_t END_ID       = 1;

    // World height constants
    constexpr int32_t OVERWORLD_HEIGHT = 256;
    constexpr int32_t NETHER_HEIGHT    = 128;  // Ceiling at 128
    constexpr int32_t END_HEIGHT       = 256;

    // Nether coordinate scaling
    constexpr double NETHER_SCALE = 8.0;  // 1 nether block = 8 overworld blocks
}

} // namespace mccpp
