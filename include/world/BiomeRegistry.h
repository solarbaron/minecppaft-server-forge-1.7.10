#pragma once
// BiomeRegistry — biome definitions, temperature/rainfall, and generation.
// Ported from vanilla 1.7.10 BiomeGenBase (aig.java), GenLayer (axn.java).
//
// Each chunk column stores 256 biome bytes (16x16 XZ grid).
// Biome IDs are sent in ChunkData packet after block/light data.
// Temperature and rainfall determine grass/foliage color and weather.

#include <cstdint>
#include <cmath>
#include <string>
#include <array>
#include <vector>
#include <random>
#include <unordered_map>

namespace mc {

// ============================================================
// Biome definition — aig.java (BiomeGenBase)
// ============================================================
struct BiomeDef {
    uint8_t     id;
    std::string name;
    float       temperature;   // 0.0-2.0
    float       rainfall;      // 0.0-1.0
    int32_t     waterColor;    // Default 0x0000FF
    float       height;        // Base height
    float       heightVar;     // Height variation
    bool        snowy;         // Can snow
    bool        rainy;         // Can rain

    // Grass/foliage color from temperature/rainfall (vanilla formula)
    int32_t grassColor() const {
        double t = std::max(0.0, std::min(1.0, static_cast<double>(temperature)));
        double r = std::max(0.0, std::min(1.0, static_cast<double>(rainfall))) * t;
        // Simplified color interpolation (vanilla uses a texture lookup)
        int red   = static_cast<int>(60 + 120 * t);
        int green = static_cast<int>(100 + 155 * t * (0.5 + 0.5 * r));
        int blue  = static_cast<int>(40 + 40 * r);
        return (red << 16) | (green << 8) | blue;
    }

    int32_t foliageColor() const {
        double t = std::max(0.0, std::min(1.0, static_cast<double>(temperature)));
        double r = std::max(0.0, std::min(1.0, static_cast<double>(rainfall))) * t;
        int red   = static_cast<int>(50 + 100 * t);
        int green = static_cast<int>(80 + 130 * t * (0.5 + 0.5 * r));
        int blue  = static_cast<int>(30 + 30 * r);
        return (red << 16) | (green << 8) | blue;
    }
};

// Biome IDs — vanilla 1.7.10
namespace BiomeID {
    constexpr uint8_t OCEAN               = 0;
    constexpr uint8_t PLAINS              = 1;
    constexpr uint8_t DESERT              = 2;
    constexpr uint8_t EXTREME_HILLS       = 3;
    constexpr uint8_t FOREST              = 4;
    constexpr uint8_t TAIGA               = 5;
    constexpr uint8_t SWAMPLAND           = 6;
    constexpr uint8_t RIVER               = 7;
    constexpr uint8_t NETHER              = 8;
    constexpr uint8_t THE_END             = 9;
    constexpr uint8_t FROZEN_OCEAN        = 10;
    constexpr uint8_t FROZEN_RIVER        = 11;
    constexpr uint8_t ICE_PLAINS          = 12;
    constexpr uint8_t ICE_MOUNTAINS      = 13;
    constexpr uint8_t MUSHROOM_ISLAND     = 14;
    constexpr uint8_t MUSHROOM_SHORE      = 15;
    constexpr uint8_t BEACH               = 16;
    constexpr uint8_t DESERT_HILLS        = 17;
    constexpr uint8_t FOREST_HILLS        = 18;
    constexpr uint8_t TAIGA_HILLS         = 19;
    constexpr uint8_t EXTREME_HILLS_EDGE  = 20;
    constexpr uint8_t JUNGLE              = 21;
    constexpr uint8_t JUNGLE_HILLS        = 22;
    constexpr uint8_t JUNGLE_EDGE         = 23;
    constexpr uint8_t DEEP_OCEAN          = 24;
    constexpr uint8_t STONE_BEACH         = 25;
    constexpr uint8_t COLD_BEACH          = 26;
    constexpr uint8_t BIRCH_FOREST        = 27;
    constexpr uint8_t BIRCH_FOREST_HILLS  = 28;
    constexpr uint8_t ROOFED_FOREST       = 29;
    constexpr uint8_t COLD_TAIGA          = 30;
    constexpr uint8_t COLD_TAIGA_HILLS    = 31;
    constexpr uint8_t MEGA_TAIGA          = 32;
    constexpr uint8_t MEGA_TAIGA_HILLS    = 33;
    constexpr uint8_t EXTREME_HILLS_PLUS  = 34;
    constexpr uint8_t SAVANNA             = 35;
    constexpr uint8_t SAVANNA_PLATEAU     = 36;
    constexpr uint8_t MESA                = 37;
    constexpr uint8_t MESA_PLATEAU_F      = 38;
    constexpr uint8_t MESA_PLATEAU        = 39;
    // Mutated variants (128+)
    constexpr uint8_t SUNFLOWER_PLAINS    = 129;
    constexpr uint8_t FLOWER_FOREST       = 132;
    constexpr uint8_t ICE_PLAINS_SPIKES   = 140;
}

// ============================================================
// Biome registry — all vanilla biome definitions
// ============================================================
class BiomeRegistry {
public:
    BiomeRegistry() {
        // Register all vanilla biomes with temperature, rainfall, height
        reg(BiomeID::OCEAN,              "Ocean",              0.5f,  0.5f,  -1.0f,  0.1f);
        reg(BiomeID::PLAINS,             "Plains",             0.8f,  0.4f,   0.125f, 0.05f);
        reg(BiomeID::DESERT,             "Desert",             2.0f,  0.0f,   0.125f, 0.05f);
        reg(BiomeID::EXTREME_HILLS,      "Extreme Hills",      0.2f,  0.3f,   1.0f,   0.5f);
        reg(BiomeID::FOREST,             "Forest",             0.7f,  0.8f,   0.1f,   0.2f);
        reg(BiomeID::TAIGA,              "Taiga",              0.25f, 0.8f,   0.2f,   0.2f);
        reg(BiomeID::SWAMPLAND,          "Swampland",          0.8f,  0.9f,  -0.2f,   0.1f);
        reg(BiomeID::RIVER,              "River",              0.5f,  0.5f,  -0.5f,   0.0f);
        reg(BiomeID::NETHER,             "Hell",               2.0f,  0.0f,   0.1f,   0.2f);
        reg(BiomeID::THE_END,            "The End",            0.5f,  0.5f,   0.1f,   0.2f);
        reg(BiomeID::FROZEN_OCEAN,       "FrozenOcean",        0.0f,  0.5f,  -1.0f,   0.1f, true);
        reg(BiomeID::FROZEN_RIVER,       "FrozenRiver",        0.0f,  0.5f,  -0.5f,   0.0f, true);
        reg(BiomeID::ICE_PLAINS,         "Ice Plains",         0.0f,  0.5f,   0.125f, 0.05f, true);
        reg(BiomeID::ICE_MOUNTAINS,      "Ice Mountains",      0.0f,  0.5f,   0.45f,  0.3f, true);
        reg(BiomeID::MUSHROOM_ISLAND,    "MushroomIsland",     0.9f,  1.0f,   0.2f,   0.3f);
        reg(BiomeID::MUSHROOM_SHORE,     "MushroomIslandShore",0.9f,  1.0f,   0.0f,   0.025f);
        reg(BiomeID::BEACH,              "Beach",              0.8f,  0.4f,   0.0f,   0.025f);
        reg(BiomeID::DESERT_HILLS,       "DesertHills",        2.0f,  0.0f,   0.45f,  0.3f);
        reg(BiomeID::FOREST_HILLS,       "ForestHills",        0.7f,  0.8f,   0.45f,  0.3f);
        reg(BiomeID::TAIGA_HILLS,        "TaigaHills",         0.25f, 0.8f,   0.45f,  0.3f);
        reg(BiomeID::EXTREME_HILLS_EDGE, "Extreme Hills Edge", 0.2f,  0.3f,   0.8f,   0.3f);
        reg(BiomeID::JUNGLE,             "Jungle",             0.95f, 0.9f,   0.1f,   0.2f);
        reg(BiomeID::JUNGLE_HILLS,       "JungleHills",        0.95f, 0.9f,   0.45f,  0.3f);
        reg(BiomeID::JUNGLE_EDGE,        "JungleEdge",         0.95f, 0.8f,   0.1f,   0.2f);
        reg(BiomeID::DEEP_OCEAN,         "Deep Ocean",         0.5f,  0.5f,  -1.8f,   0.1f);
        reg(BiomeID::STONE_BEACH,        "Stone Beach",        0.2f,  0.3f,   0.1f,   0.8f);
        reg(BiomeID::COLD_BEACH,         "Cold Beach",         0.05f, 0.3f,   0.0f,   0.025f, true);
        reg(BiomeID::BIRCH_FOREST,       "Birch Forest",       0.6f,  0.6f,   0.1f,   0.2f);
        reg(BiomeID::BIRCH_FOREST_HILLS, "Birch Forest Hills", 0.6f,  0.6f,   0.45f,  0.3f);
        reg(BiomeID::ROOFED_FOREST,      "Roofed Forest",      0.7f,  0.8f,   0.1f,   0.2f);
        reg(BiomeID::COLD_TAIGA,         "Cold Taiga",         -0.5f, 0.4f,   0.2f,   0.2f, true);
        reg(BiomeID::COLD_TAIGA_HILLS,   "Cold Taiga Hills",   -0.5f, 0.4f,   0.45f,  0.3f, true);
        reg(BiomeID::MEGA_TAIGA,         "Mega Taiga",         0.3f,  0.8f,   0.2f,   0.2f);
        reg(BiomeID::MEGA_TAIGA_HILLS,   "Mega Taiga Hills",   0.3f,  0.8f,   0.45f,  0.3f);
        reg(BiomeID::EXTREME_HILLS_PLUS, "Extreme Hills+",     0.2f,  0.3f,   1.0f,   0.5f);
        reg(BiomeID::SAVANNA,            "Savanna",            1.2f,  0.0f,   0.125f, 0.05f);
        reg(BiomeID::SAVANNA_PLATEAU,    "Savanna Plateau",    1.0f,  0.0f,   1.5f,   0.025f);
        reg(BiomeID::MESA,               "Mesa",               2.0f,  0.0f,   0.1f,   0.2f);
        reg(BiomeID::MESA_PLATEAU_F,     "Mesa Plateau F",     2.0f,  0.0f,   1.5f,   0.025f);
        reg(BiomeID::MESA_PLATEAU,       "Mesa Plateau",       2.0f,  0.0f,   1.5f,   0.025f);
    }

    const BiomeDef* get(uint8_t id) const {
        auto it = biomes_.find(id);
        return it != biomes_.end() ? &it->second : nullptr;
    }

    const BiomeDef& getOrDefault(uint8_t id) const {
        auto it = biomes_.find(id);
        if (it != biomes_.end()) return it->second;
        return biomes_.at(BiomeID::PLAINS); // Fallback
    }

    bool canSnowAt(uint8_t biomeId, int y) const {
        auto* b = get(biomeId);
        if (!b) return false;
        return b->temperature < 0.15f;
    }

    bool canRainAt(uint8_t biomeId) const {
        auto* b = get(biomeId);
        if (!b) return false;
        return b->rainfall > 0.0f && b->temperature < 2.0f;
    }

private:
    void reg(uint8_t id, const std::string& name, float temp, float rain,
             float h, float hvar, bool snowy = false) {
        biomes_[id] = {id, name, temp, rain, 0x0000FF, h, hvar, snowy, rain > 0.0f && temp < 2.0f};
    }

    std::unordered_map<uint8_t, BiomeDef> biomes_;
};

// ============================================================
// Biome generator — simplified Voronoi-based biome placement
// Matching vanilla's GenLayer approach (axn.java)
// ============================================================
class BiomeGenerator {
public:
    explicit BiomeGenerator(int64_t seed) : seed_(seed) {
        // Biome distribution weights for overworld generation
        // Vanilla uses layered GenLayer system; we approximate with zones
        temperatePool_ = {BiomeID::PLAINS, BiomeID::FOREST, BiomeID::BIRCH_FOREST,
                          BiomeID::ROOFED_FOREST, BiomeID::SWAMPLAND};
        warmPool_      = {BiomeID::DESERT, BiomeID::SAVANNA, BiomeID::MESA, BiomeID::PLAINS};
        coldPool_      = {BiomeID::TAIGA, BiomeID::ICE_PLAINS, BiomeID::COLD_TAIGA,
                          BiomeID::EXTREME_HILLS};
        tropicalPool_  = {BiomeID::JUNGLE, BiomeID::JUNGLE_EDGE};
    }

    // Generate biome array for a chunk (16x16 = 256 bytes)
    std::array<uint8_t, 256> generateChunkBiomes(int chunkX, int chunkZ) const {
        std::array<uint8_t, 256> biomes;
        for (int z = 0; z < 16; ++z) {
            for (int x = 0; x < 16; ++x) {
                int worldX = chunkX * 16 + x;
                int worldZ = chunkZ * 16 + z;
                biomes[z * 16 + x] = getBiomeAt(worldX, worldZ);
            }
        }
        return biomes;
    }

    // Get biome at a specific world coordinate
    uint8_t getBiomeAt(int worldX, int worldZ) const {
        // Large-scale climate zones based on position hash
        // This approximates vanilla's GenLayer zoom/island/temperature system

        // Scale down for large biome regions (~256 blocks per biome)
        int regionX = static_cast<int>(std::floor(static_cast<double>(worldX) / 256.0));
        int regionZ = static_cast<int>(std::floor(static_cast<double>(worldZ) / 256.0));

        // Determine climate zone from region hash
        int64_t regionHash = hashPos(regionX, regionZ);
        int climate = static_cast<int>(((regionHash >> 8) & 0x3)); // 0-3

        // Pick biome from climate pool
        const std::vector<uint8_t>* pool;
        switch (climate) {
            case 0: pool = &temperatePool_; break;
            case 1: pool = &warmPool_; break;
            case 2: pool = &coldPool_; break;
            case 3: pool = &tropicalPool_; break;
            default: pool = &temperatePool_; break;
        }

        // Sub-region variation (~64 blocks)
        int subX = static_cast<int>(std::floor(static_cast<double>(worldX) / 64.0));
        int subZ = static_cast<int>(std::floor(static_cast<double>(worldZ) / 64.0));
        int64_t subHash = hashPos(subX, subZ);
        int idx = static_cast<int>((subHash & 0x7FFFFFFF) % pool->size());

        uint8_t baseBiome = (*pool)[idx];

        // River overlay check (~32 block width rivers)
        int riverX = static_cast<int>(std::floor(static_cast<double>(worldX) / 32.0));
        int riverZ = static_cast<int>(std::floor(static_cast<double>(worldZ) / 32.0));
        int64_t riverHash = hashPos(riverX * 7, riverZ * 13);
        if ((riverHash & 0xF) == 0) {
            return BiomeID::RIVER;
        }

        // Ocean check for low areas
        int oceanX = static_cast<int>(std::floor(static_cast<double>(worldX) / 512.0));
        int oceanZ = static_cast<int>(std::floor(static_cast<double>(worldZ) / 512.0));
        int64_t oceanHash = hashPos(oceanX * 3, oceanZ * 5);
        if ((oceanHash & 0x7) == 0) {
            return BiomeID::OCEAN;
        }

        return baseBiome;
    }

private:
    int64_t hashPos(int x, int z) const {
        int64_t h = seed_;
        h = h * 6364136223846793005LL + 1442695040888963407LL;
        h += x;
        h = h * 6364136223846793005LL + 1442695040888963407LL;
        h += z;
        h = h * 6364136223846793005LL + 1442695040888963407LL;
        h += x;
        h = h * 6364136223846793005LL + 1442695040888963407LL;
        h += z;
        return h;
    }

    int64_t seed_;
    std::vector<uint8_t> temperatePool_;
    std::vector<uint8_t> warmPool_;
    std::vector<uint8_t> coldPool_;
    std::vector<uint8_t> tropicalPool_;
};

} // namespace mc
