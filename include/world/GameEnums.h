/**
 * GameEnums.h — Core game enumerations.
 *
 * Java references:
 *   - net.minecraft.world.WorldType — World generator types
 *   - net.minecraft.world.EnumDifficulty — Difficulty levels
 *   - net.minecraft.world.WorldSettings$GameType — Game modes
 *   - net.minecraft.world.WorldSettings — World creation settings
 *
 * Thread safety: Immutable after init.
 *
 * JNI readiness: Simple enums and POD structs.
 */
#pragma once

#include <cstdint>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EnumDifficulty — Difficulty levels 0-3.
// Java reference: net.minecraft.world.EnumDifficulty
// ═══════════════════════════════════════════════════════════════════════════

enum class Difficulty : int32_t {
    PEACEFUL = 0,
    EASY     = 1,
    NORMAL   = 2,
    HARD     = 3
};

inline Difficulty getDifficultyById(int32_t id) {
    return static_cast<Difficulty>(id % 4);
}

inline const char* getDifficultyName(Difficulty d) {
    switch (d) {
        case Difficulty::PEACEFUL: return "peaceful";
        case Difficulty::EASY:     return "easy";
        case Difficulty::NORMAL:   return "normal";
        case Difficulty::HARD:     return "hard";
    }
    return "normal";
}

// ═══════════════════════════════════════════════════════════════════════════
// GameType — Game modes.
// Java reference: net.minecraft.world.WorldSettings$GameType
// ═══════════════════════════════════════════════════════════════════════════

enum class GameType : int32_t {
    NOT_SET   = -1,
    SURVIVAL  = 0,
    CREATIVE  = 1,
    ADVENTURE = 2,
    SPECTATOR = 3  // Not in 1.7.10 but reserved
};

inline GameType getGameTypeById(int32_t id) {
    switch (id) {
        case 0: return GameType::SURVIVAL;
        case 1: return GameType::CREATIVE;
        case 2: return GameType::ADVENTURE;
        default: return GameType::SURVIVAL;
    }
}

inline const char* getGameTypeName(GameType g) {
    switch (g) {
        case GameType::NOT_SET:   return "not_set";
        case GameType::SURVIVAL:  return "survival";
        case GameType::CREATIVE:  return "creative";
        case GameType::ADVENTURE: return "adventure";
        case GameType::SPECTATOR: return "spectator";
    }
    return "survival";
}

inline bool isCreativeMode(GameType g) { return g == GameType::CREATIVE; }
inline bool isSurvivalOrAdventure(GameType g) {
    return g == GameType::SURVIVAL || g == GameType::ADVENTURE;
}

// ═══════════════════════════════════════════════════════════════════════════
// WorldType — World generator types.
// Java reference: net.minecraft.world.WorldType
// ═══════════════════════════════════════════════════════════════════════════

struct WorldTypeData {
    int32_t id;
    std::string name;
    int32_t generatorVersion;
    bool canBeCreated;
    bool isVersioned;
    bool hasNotificationData;
};

namespace WorldType {
    // Java: worldTypes[16] with specific IDs
    constexpr int32_t DEFAULT      = 0;
    constexpr int32_t FLAT         = 1;
    constexpr int32_t LARGE_BIOMES = 2;
    constexpr int32_t AMPLIFIED    = 3;
    constexpr int32_t DEFAULT_1_1  = 8;

    inline const char* getName(int32_t id) {
        switch (id) {
            case DEFAULT:      return "default";
            case FLAT:         return "flat";
            case LARGE_BIOMES: return "largeBiomes";
            case AMPLIFIED:    return "amplified";
            case DEFAULT_1_1:  return "default_1_1";
            default:           return "default";
        }
    }

    // Java: parseWorldType — case-insensitive name lookup
    inline int32_t parseWorldType(const std::string& name) {
        if (name == "default" || name == "DEFAULT") return DEFAULT;
        if (name == "flat" || name == "FLAT") return FLAT;
        if (name == "largeBiomes" || name == "LARGEBIOMES") return LARGE_BIOMES;
        if (name == "amplified" || name == "AMPLIFIED") return AMPLIFIED;
        if (name == "default_1_1" || name == "DEFAULT_1_1") return DEFAULT_1_1;
        return DEFAULT; // Fallback
    }

    // Java: getGeneratorVersion
    inline int32_t getGeneratorVersion(int32_t id) {
        switch (id) {
            case DEFAULT:     return 1;
            case DEFAULT_1_1: return 0;
            default:          return 0;
        }
    }

    // Java: getWorldTypeForGeneratorVersion
    inline int32_t getWorldTypeForVersion(int32_t id, int32_t version) {
        if (id == DEFAULT && version == 0) return DEFAULT_1_1;
        return id;
    }

    // Java: getAverageGroundLevel (from WorldProvider)
    inline int32_t getAverageGroundLevel(int32_t id) {
        return (id == FLAT) ? 4 : 64;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// WorldSettings — World creation parameters.
// Java reference: net.minecraft.world.WorldSettings
// ═══════════════════════════════════════════════════════════════════════════

struct WorldSettings {
    int64_t seed = 0;
    GameType gameType = GameType::SURVIVAL;
    bool mapFeaturesEnabled = true;
    bool hardcoreEnabled = false;
    int32_t worldTypeId = WorldType::DEFAULT;
    bool commandsAllowed = false;
    bool bonusChestEnabled = false;
    std::string generatorOptions;

    WorldSettings() = default;
    WorldSettings(int64_t s, GameType gt, bool mapFeatures, bool hardcore, int32_t wt)
        : seed(s), gameType(gt), mapFeaturesEnabled(mapFeatures),
          hardcoreEnabled(hardcore), worldTypeId(wt) {}
};

// ═══════════════════════════════════════════════════════════════════════════
// Moon phase factors — 8 phases.
// Java: WorldProvider.moonPhaseFactors
// ═══════════════════════════════════════════════════════════════════════════

namespace MoonPhase {
    constexpr float FACTORS[8] = {1.0f, 0.75f, 0.5f, 0.25f, 0.0f, 0.25f, 0.5f, 0.75f};

    // Java: getMoonPhase(long worldTime)
    inline int32_t getMoonPhase(int64_t worldTime) {
        return static_cast<int32_t>((worldTime / 24000LL % 8LL + 8LL) % 8LL);
    }

    inline float getFactor(int64_t worldTime) {
        return FACTORS[getMoonPhase(worldTime)];
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Light brightness table — 16 entries.
// Java: WorldProvider.generateLightBrightnessTable
// ═══════════════════════════════════════════════════════════════════════════

namespace LightBrightness {
    // Precomputed table for overworld (ambientLight = 0)
    inline void generate(float table[16], float ambientLight = 0.0f) {
        for (int i = 0; i <= 15; ++i) {
            float invLevel = 1.0f - static_cast<float>(i) / 15.0f;
            table[i] = (1.0f - invLevel) / (invLevel * 3.0f + 1.0f) * (1.0f - ambientLight) + ambientLight;
        }
    }

    // Nether ambient = 0.1f
    inline void generateNether(float table[16]) {
        generate(table, 0.1f);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Celestial angle calculation.
// Java: WorldProvider.calculateCelestialAngle
// ═══════════════════════════════════════════════════════════════════════════

namespace CelestialAngle {
    inline float calculate(int64_t worldTime, float partialTicks) {
        int32_t timeOfDay = static_cast<int32_t>(worldTime % 24000L);
        float angle = (static_cast<float>(timeOfDay) + partialTicks) / 24000.0f - 0.25f;
        if (angle < 0.0f) angle += 1.0f;
        if (angle > 1.0f) angle -= 1.0f;
        float original = angle;
        // Java: cosine interpolation
        angle = 1.0f - static_cast<float>((std::cos(static_cast<double>(angle) * 3.141592653589793) + 1.0) / 2.0);
        angle = original + (angle - original) / 3.0f;
        return angle;
    }
}

} // namespace mccpp
