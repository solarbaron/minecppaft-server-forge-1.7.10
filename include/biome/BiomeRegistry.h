/**
 * BiomeRegistry.h — Biome definition and registry.
 *
 * Java references:
 *   - net.minecraft.world.biome.BiomeGenBase — All biome definitions
 *   - net.minecraft.world.biome.BiomeGenBase$Height — Height preset
 *   - net.minecraft.world.biome.BiomeGenBase$SpawnListEntry — Mob spawn entry
 *   - net.minecraft.world.biome.BiomeGenBase$TempCategory — Temperature class
 *
 * All 40 vanilla 1.7.10 biomes (IDs 0-39) plus 20 mutated variants (ID+128)
 * with exact temperature, rainfall, height values from the decompiled source.
 *
 * Thread safety:
 *   - Biome registry is static/const after initialization.
 *   - Temperature noise queries are thread-safe (stateless function).
 *
 * JNI readiness: Integer biome IDs for easy JVM mapping.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// BiomeHeight — Height variation preset.
// Java reference: net.minecraft.world.biome.BiomeGenBase$Height
// ═══════════════════════════════════════════════════════════════════════════

struct BiomeHeight {
    float rootHeight;   // Base terrain height
    float variation;    // Height variation/roughness
};

// Java: BiomeGenBase static height presets
namespace BiomeHeights {
    inline constexpr BiomeHeight DEFAULT           = { 0.1f,   0.2f};
    inline constexpr BiomeHeight SHALLOW_WATERS    = {-0.5f,   0.0f};
    inline constexpr BiomeHeight OCEANS            = {-1.0f,   0.1f};
    inline constexpr BiomeHeight DEEP_OCEANS       = {-1.8f,   0.1f};
    inline constexpr BiomeHeight LOW_PLAINS        = { 0.125f, 0.05f};
    inline constexpr BiomeHeight MID_PLAINS        = { 0.2f,   0.2f};
    inline constexpr BiomeHeight LOW_HILLS         = { 0.45f,  0.3f};
    inline constexpr BiomeHeight HIGH_PLATEAUS     = { 1.5f,   0.025f};
    inline constexpr BiomeHeight MID_HILLS         = { 1.0f,   0.5f};
    inline constexpr BiomeHeight SHORES            = { 0.0f,   0.025f};
    inline constexpr BiomeHeight ROCKY_WATERS      = { 0.1f,   0.8f};
    inline constexpr BiomeHeight LOW_ISLANDS       = { 0.2f,   0.3f};
    inline constexpr BiomeHeight PARTIALLY_SUBMERGED = {-0.2f, 0.1f};
}

// ═══════════════════════════════════════════════════════════════════════════
// SpawnEntry — Mob spawn configuration per biome.
// Java reference: net.minecraft.world.biome.BiomeGenBase$SpawnListEntry
// ═══════════════════════════════════════════════════════════════════════════

struct SpawnEntry {
    std::string entityName;
    int32_t weight;      // Spawn weight (higher = more common)
    int32_t minGroup;    // Minimum group size
    int32_t maxGroup;    // Maximum group size
};

// ═══════════════════════════════════════════════════════════════════════════
// TempCategory — Temperature classification.
// Java reference: net.minecraft.world.biome.BiomeGenBase$TempCategory
// ═══════════════════════════════════════════════════════════════════════════

enum class TempCategory : int32_t {
    COLD   = 0,  // temp < 0.2
    MEDIUM = 1,  // 0.2 <= temp < 1.0
    WARM   = 2   // temp >= 1.0
};

// ═══════════════════════════════════════════════════════════════════════════
// Biome — Complete biome definition.
// Java reference: net.minecraft.world.biome.BiomeGenBase
// ═══════════════════════════════════════════════════════════════════════════

struct Biome {
    int32_t id;
    std::string name;
    int32_t color;            // Map color (packed RGB)
    float temperature;        // 0.0 = cold, 2.0 = hot
    float rainfall;           // 0.0 = dry, 1.0 = wet
    float minHeight;          // Terrain height (rootHeight)
    float maxHeight;          // Terrain variation
    bool enableSnow;          // Snow instead of rain
    bool enableRain;          // Can rain at all
    int32_t topBlockId;       // Surface block (default: grass = 2)
    int32_t fillerBlockId;    // Subsurface block (default: dirt = 3)
    int32_t waterColor;       // Water tint

    // Mob spawn lists
    std::vector<SpawnEntry> monstersSpawns;
    std::vector<SpawnEntry> creatureSpawns;
    std::vector<SpawnEntry> waterCreatureSpawns;
    std::vector<SpawnEntry> caveCreatureSpawns;

    // Mutated variant ID (-1 if none)
    int32_t mutatedVariantId;

    // Java: BiomeGenBase.getTempCategory
    TempCategory getTempCategory() const {
        if (temperature < 0.2f) return TempCategory::COLD;
        if (temperature < 1.0f) return TempCategory::MEDIUM;
        return TempCategory::WARM;
    }

    // Java: BiomeGenBase.isHighHumidity
    bool isHighHumidity() const { return rainfall > 0.85f; }

    // Java: BiomeGenBase.canSpawnLightningBolt
    bool canSpawnLightningBolt() const {
        return !enableSnow && enableRain;
    }

    // Java: BiomeGenBase.getSpawningChance
    float getSpawningChance() const { return 0.1f; }

    // Java: BiomeGenBase.getIntRainfall
    int32_t getIntRainfall() const {
        return static_cast<int32_t>(rainfall * 65536.0f);
    }

    // Java: BiomeGenBase.getFloatTemperature (with altitude adjustment)
    float getFloatTemperature(int32_t x, int32_t y, int32_t z) const;
};

// ═══════════════════════════════════════════════════════════════════════════
// BiomeRegistry — Static registry of all vanilla biomes.
// Java reference: net.minecraft.world.biome.BiomeGenBase static initializer
//
// Thread safety: initialized once at startup, read-only after.
// ═══════════════════════════════════════════════════════════════════════════

class BiomeRegistry {
public:
    static void init();
    static const Biome* getById(int32_t id);
    static const std::vector<Biome>& getAll();
    static int32_t getCount();

    // Biome IDs — matching Java's BiomeGenBase static fields
    static constexpr int32_t OCEAN               = 0;
    static constexpr int32_t PLAINS              = 1;
    static constexpr int32_t DESERT              = 2;
    static constexpr int32_t EXTREME_HILLS       = 3;
    static constexpr int32_t FOREST              = 4;
    static constexpr int32_t TAIGA               = 5;
    static constexpr int32_t SWAMPLAND           = 6;
    static constexpr int32_t RIVER               = 7;
    static constexpr int32_t HELL                = 8;
    static constexpr int32_t SKY                 = 9;
    static constexpr int32_t FROZEN_OCEAN        = 10;
    static constexpr int32_t FROZEN_RIVER        = 11;
    static constexpr int32_t ICE_PLAINS          = 12;
    static constexpr int32_t ICE_MOUNTAINS       = 13;
    static constexpr int32_t MUSHROOM_ISLAND     = 14;
    static constexpr int32_t MUSHROOM_SHORE      = 15;
    static constexpr int32_t BEACH               = 16;
    static constexpr int32_t DESERT_HILLS        = 17;
    static constexpr int32_t FOREST_HILLS        = 18;
    static constexpr int32_t TAIGA_HILLS         = 19;
    static constexpr int32_t EXTREME_HILLS_EDGE  = 20;
    static constexpr int32_t JUNGLE              = 21;
    static constexpr int32_t JUNGLE_HILLS        = 22;
    static constexpr int32_t JUNGLE_EDGE         = 23;
    static constexpr int32_t DEEP_OCEAN          = 24;
    static constexpr int32_t STONE_BEACH         = 25;
    static constexpr int32_t COLD_BEACH          = 26;
    static constexpr int32_t BIRCH_FOREST        = 27;
    static constexpr int32_t BIRCH_FOREST_HILLS  = 28;
    static constexpr int32_t ROOFED_FOREST       = 29;
    static constexpr int32_t COLD_TAIGA          = 30;
    static constexpr int32_t COLD_TAIGA_HILLS    = 31;
    static constexpr int32_t MEGA_TAIGA          = 32;
    static constexpr int32_t MEGA_TAIGA_HILLS    = 33;
    static constexpr int32_t EXTREME_HILLS_PLUS  = 34;
    static constexpr int32_t SAVANNA             = 35;
    static constexpr int32_t SAVANNA_PLATEAU     = 36;
    static constexpr int32_t MESA                = 37;
    static constexpr int32_t MESA_PLATEAU_F      = 38;
    static constexpr int32_t MESA_PLATEAU        = 39;

    // Mutated IDs = base + 128
    static constexpr int32_t MUTATION_OFFSET     = 128;

private:
    static std::vector<Biome> biomes_;
    static bool initialized_;

    // Default mob spawn lists (shared by most biomes)
    static std::vector<SpawnEntry> defaultMonsters();
    static std::vector<SpawnEntry> defaultCreatures();
    static std::vector<SpawnEntry> defaultWaterCreatures();
    static std::vector<SpawnEntry> defaultCaveCreatures();
};

} // namespace mccpp
