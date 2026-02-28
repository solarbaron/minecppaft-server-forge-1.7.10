/**
 * BiomeGenBase.h — Complete biome registry with all 40+ biome definitions.
 *
 * Java reference: net.minecraft.world.biome.BiomeGenBase
 *
 * Contains:
 *   - 14 height presets (ocean, plains, hills, shores, etc.)
 *   - 40 base biomes (IDs 0-39) with exact height, temperature, rainfall
 *   - 20 mutated biomes (IDs 128+)
 *   - Mob spawn lists (7 monsters, 4 creatures, 1 water, 1 cave)
 *   - genBiomeTerrain: top-down column replacement
 *     (bedrock → stone → topBlock → fillerBlock, ice/water below sea, sandstone transition)
 *   - Temperature noise for altitude-based cooling
 *   - explorationBiomesList (excludes hell, sky, frozenOcean, extremeHillsEdge)
 *
 * Thread safety: Biome data is immutable after registration.
 * JNI readiness: Simple data, array-based registry.
 */
#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// SpawnListEntry — Mob spawn weight/group data.
// Java reference: net.minecraft.world.biome.BiomeGenBase$SpawnListEntry
// ═══════════════════════════════════════════════════════════════════════════

struct SpawnListEntry {
    std::string entityName;
    int32_t weight;
    int32_t minGroupCount;
    int32_t maxGroupCount;
};

// ═══════════════════════════════════════════════════════════════════════════
// BiomeHeight — Height preset.
// Java reference: net.minecraft.world.biome.BiomeGenBase$Height
// ═══════════════════════════════════════════════════════════════════════════

struct BiomeHeight {
    float rootHeight;
    float variation;

    BiomeHeight attenuate() const {
        return {rootHeight * 0.8f, variation * 0.6f};
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Height presets from Java.
// ═══════════════════════════════════════════════════════════════════════════

namespace Heights {
    constexpr BiomeHeight Default           = {0.1f,   0.2f};
    constexpr BiomeHeight ShallowWaters     = {-0.5f,  0.0f};
    constexpr BiomeHeight Oceans            = {-1.0f,  0.1f};
    constexpr BiomeHeight DeepOceans        = {-1.8f,  0.1f};
    constexpr BiomeHeight LowPlains         = {0.125f, 0.05f};
    constexpr BiomeHeight MidPlains         = {0.2f,   0.2f};
    constexpr BiomeHeight LowHills          = {0.45f,  0.3f};
    constexpr BiomeHeight HighPlateaus      = {1.5f,   0.025f};
    constexpr BiomeHeight MidHills          = {1.0f,   0.5f};
    constexpr BiomeHeight Shores            = {0.0f,   0.025f};
    constexpr BiomeHeight RockyWaters       = {0.1f,   0.8f};
    constexpr BiomeHeight LowIslands        = {0.2f,   0.3f};
    constexpr BiomeHeight PartiallySubmerged= {-0.2f,  0.1f};
}

// ═══════════════════════════════════════════════════════════════════════════
// TempCategory
// ═══════════════════════════════════════════════════════════════════════════

enum class TempCategory { COLD, MEDIUM, WARM };

// ═══════════════════════════════════════════════════════════════════════════
// BiomeGenBase — Core biome data.
// Java reference: net.minecraft.world.biome.BiomeGenBase
// ═══════════════════════════════════════════════════════════════════════════

struct BiomeGenBase {
    int32_t biomeID;
    std::string biomeName;
    int32_t color;
    int32_t field_150609_ah;

    // Surface blocks (block IDs)
    int32_t topBlock = 2;           // grass
    int32_t topBlockMetadata = 0;
    int32_t fillerBlock = 3;        // dirt
    int32_t fillerBlockMetadata = 0;

    // Height
    float minHeight = 0.1f;
    float maxHeight = 0.2f;

    // Climate
    float temperature = 0.5f;
    float rainfall = 0.5f;
    int32_t waterColorMultiplier = 0xFFFFFF;

    // Flags
    bool enableSnow = false;
    bool enableRain = true;

    // Spawn lists
    std::vector<SpawnListEntry> spawnableMonsterList;
    std::vector<SpawnListEntry> spawnableCreatureList;
    std::vector<SpawnListEntry> spawnableWaterCreatureList;
    std::vector<SpawnListEntry> spawnableCaveCreatureList;

    // ─── Methods ───

    bool getEnableSnow() const { return enableSnow; }

    bool canSpawnLightningBolt() const {
        return !enableSnow && enableRain;
    }

    bool isHighHumidity() const { return rainfall > 0.85f; }
    float getSpawningChance() const { return 0.1f; }
    int32_t getIntRainfall() const { return static_cast<int32_t>(rainfall * 65536.0f); }

    // Java: getFloatTemperature — altitude-based cooling above y=64
    float getFloatTemperature(int32_t x, int32_t y, int32_t z) const {
        if (y > 64) {
            // Simplified noise stub — actual impl needs NoiseGeneratorPerlin(1234L, 1)
            float reduction = static_cast<float>(y - 64) * 0.05f / 30.0f;
            return temperature - reduction;
        }
        return temperature;
    }

    TempCategory getTempCategory() const {
        if (temperature < 0.2f) return TempCategory::COLD;
        if (temperature < 1.0f) return TempCategory::MEDIUM;
        return TempCategory::WARM;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// BiomeRegistry — Static registry of all vanilla biomes.
// Java: BiomeGenBase static fields + biomeList[256]
// ═══════════════════════════════════════════════════════════════════════════

class BiomeRegistry {
public:
    static constexpr int32_t MAX_BIOMES = 256;

    // Block IDs
    static constexpr int32_t GRASS = 2;
    static constexpr int32_t DIRT = 3;
    static constexpr int32_t SAND = 12;
    static constexpr int32_t GRAVEL = 13;
    static constexpr int32_t STONE = 1;
    static constexpr int32_t SANDSTONE = 24;
    static constexpr int32_t MYCELIUM = 110;
    static constexpr int32_t NETHERRACK = 87;
    static constexpr int32_t END_STONE = 121;
    static constexpr int32_t HARDENED_CLAY = 172;
    static constexpr int32_t STAINED_CLAY = 159;
    static constexpr int32_t RED_SAND = 12;  // sand meta=1

    // Initialize all biomes
    static void init() {
        if (initialized_) return;

        // Default spawn lists
        auto defaultMonsters = std::vector<SpawnListEntry>{
            {"Spider", 100, 4, 4}, {"Zombie", 100, 4, 4},
            {"Skeleton", 100, 4, 4}, {"Creeper", 100, 4, 4},
            {"Slime", 100, 4, 4}, {"Enderman", 10, 1, 4},
            {"Witch", 5, 1, 1}
        };
        auto defaultCreatures = std::vector<SpawnListEntry>{
            {"Sheep", 12, 4, 4}, {"Pig", 10, 4, 4},
            {"Chicken", 10, 4, 4}, {"Cow", 8, 4, 4}
        };
        auto defaultWater = std::vector<SpawnListEntry>{{"Squid", 10, 4, 4}};
        auto defaultCave = std::vector<SpawnListEntry>{{"Bat", 10, 8, 8}};

        auto makeBiome = [&](int32_t id, const std::string& name, int32_t col,
                              BiomeHeight h, float temp, float rain,
                              bool snow = false, bool noRain = false,
                              int32_t top = GRASS, int32_t filler = DIRT) {
            auto& b = biomes_[id];
            b.biomeID = id;
            b.biomeName = name;
            b.color = col;
            b.field_150609_ah = col;
            b.minHeight = h.rootHeight;
            b.maxHeight = h.variation;
            b.temperature = temp;
            b.rainfall = rain;
            b.enableSnow = snow;
            b.enableRain = !noRain;
            b.topBlock = top;
            b.fillerBlock = filler;
            b.spawnableMonsterList = defaultMonsters;
            b.spawnableCreatureList = defaultCreatures;
            b.spawnableWaterCreatureList = defaultWater;
            b.spawnableCaveCreatureList = defaultCave;
            valid_[id] = true;
        };

        // ─── Base biomes (IDs 0-39) ───
        makeBiome(0,  "Ocean",              112,      Heights::Oceans,    0.5f,  0.5f);
        makeBiome(1,  "Plains",             9286496,  Heights::Default,   0.8f,  0.4f);
        makeBiome(2,  "Desert",             16421912, Heights::LowPlains, 2.0f,  0.0f, false, true, SAND, SAND);
        makeBiome(3,  "Extreme Hills",      0x606060, Heights::MidHills,  0.2f,  0.3f);
        makeBiome(4,  "Forest",             353825,   Heights::Default,   0.7f,  0.8f);
        makeBiome(5,  "Taiga",              747097,   Heights::MidPlains, 0.25f, 0.8f);
        makeBiome(6,  "Swampland",          522674,   Heights::PartiallySubmerged, 0.8f, 0.9f);
        makeBiome(7,  "River",              255,      Heights::ShallowWaters, 0.5f, 0.5f);
        makeBiome(8,  "Hell",               0xFF0000, Heights::Default,   2.0f,  0.0f, false, true, NETHERRACK, NETHERRACK);
        makeBiome(9,  "Sky",                0x8080FF, Heights::Default,   0.5f,  0.5f, false, true, END_STONE, END_STONE);
        makeBiome(10, "FrozenOcean",        0x9090A0, Heights::Oceans,    0.0f,  0.5f, true);
        makeBiome(11, "FrozenRiver",        0xA0A0FF, Heights::ShallowWaters, 0.0f, 0.5f, true);
        makeBiome(12, "Ice Plains",         0xFFFFFF, Heights::LowPlains, 0.0f,  0.5f, true);
        makeBiome(13, "Ice Mountains",      0xA0A0A0, Heights::LowHills,  0.0f,  0.5f, true);
        makeBiome(14, "MushroomIsland",     0xFF00FF, Heights::LowIslands, 0.9f, 1.0f, false, false, MYCELIUM);
        makeBiome(15, "MushroomIslandShore",0xA000FF, Heights::Shores,    0.9f,  1.0f, false, false, MYCELIUM);
        makeBiome(16, "Beach",              16440917, Heights::Shores,    0.8f,  0.4f, false, false, SAND, SAND);
        makeBiome(17, "DesertHills",        13786898, Heights::LowHills,  2.0f,  0.0f, false, true, SAND, SAND);
        makeBiome(18, "ForestHills",        2250012,  Heights::LowHills,  0.7f,  0.8f);
        makeBiome(19, "TaigaHills",         1456435,  Heights::LowHills,  0.25f, 0.8f);
        makeBiome(20, "Extreme Hills Edge", 7501978,  {Heights::MidHills.rootHeight * 0.8f, Heights::MidHills.variation * 0.6f}, 0.2f, 0.3f);
        makeBiome(21, "Jungle",             5470985,  Heights::Default,   0.95f, 0.9f);
        makeBiome(22, "JungleHills",        2900485,  Heights::LowHills,  0.95f, 0.9f);
        makeBiome(23, "JungleEdge",         6458135,  Heights::Default,   0.95f, 0.8f);
        makeBiome(24, "Deep Ocean",         48,       Heights::DeepOceans, 0.5f, 0.5f);
        makeBiome(25, "Stone Beach",        10658436, Heights::RockyWaters, 0.2f, 0.3f, false, false, STONE, STONE);
        makeBiome(26, "Cold Beach",         16445632, Heights::Shores,    0.05f, 0.3f, true, false, SAND, SAND);
        makeBiome(27, "Birch Forest",       3175492,  Heights::Default,   0.6f,  0.6f);
        makeBiome(28, "Birch Forest Hills", 2055986,  Heights::LowHills,  0.6f,  0.6f);
        makeBiome(29, "Roofed Forest",      4215066,  Heights::Default,   0.7f,  0.8f);
        makeBiome(30, "Cold Taiga",         3233098,  Heights::MidPlains, -0.5f, 0.4f, true);
        makeBiome(31, "Cold Taiga Hills",   2375478,  Heights::LowHills,  -0.5f, 0.4f, true);
        makeBiome(32, "Mega Taiga",         5858897,  Heights::MidPlains, 0.3f,  0.8f);
        makeBiome(33, "Mega Taiga Hills",   4542270,  Heights::LowHills,  0.3f,  0.8f);
        makeBiome(34, "Extreme Hills+",     0x507050, Heights::MidHills,  0.2f,  0.3f);
        makeBiome(35, "Savanna",            12431967, Heights::LowPlains, 1.2f,  0.0f, false, true);
        makeBiome(36, "Savanna Plateau",    10984804, Heights::HighPlateaus, 1.0f, 0.0f, false, true);
        makeBiome(37, "Mesa",               14238997, Heights::Default,   2.0f,  0.0f, false, true, RED_SAND, HARDENED_CLAY);
        makeBiome(38, "Mesa Plateau F",     11573093, Heights::HighPlateaus, 2.0f, 0.0f, false, true, RED_SAND, HARDENED_CLAY);
        makeBiome(39, "Mesa Plateau",       13274213, Heights::HighPlateaus, 2.0f, 0.0f, false, true, RED_SAND, HARDENED_CLAY);

        // Hell and Sky: clear creature spawns
        biomes_[8].spawnableCreatureList.clear();
        biomes_[8].spawnableWaterCreatureList.clear();
        biomes_[9].spawnableCreatureList.clear();
        biomes_[9].spawnableWaterCreatureList.clear();

        // ─── Mutated biomes (IDs 128+) — copy base biome with modified height ───
        auto createMutation = [&](int32_t baseId) {
            int32_t mutId = baseId + 128;
            if (mutId < MAX_BIOMES && valid_[baseId]) {
                biomes_[mutId] = biomes_[baseId];
                biomes_[mutId].biomeID = mutId;
                biomes_[mutId].biomeName = biomes_[baseId].biomeName + " M";
                biomes_[mutId].minHeight += 0.1f;
                biomes_[mutId].maxHeight += 0.2f;
                valid_[mutId] = true;
            }
        };

        // Java: 20 mutated biomes
        createMutation(1);   // Plains
        createMutation(2);   // Desert
        createMutation(4);   // Forest
        createMutation(5);   // Taiga
        createMutation(6);   // Swampland
        createMutation(12);  // Ice Plains
        createMutation(21);  // Jungle
        createMutation(23);  // JungleEdge
        createMutation(30);  // Cold Taiga
        createMutation(35);  // Savanna
        createMutation(36);  // Savanna Plateau
        createMutation(37);  // Mesa
        createMutation(38);  // Mesa Plateau F
        createMutation(39);  // Mesa Plateau
        createMutation(27);  // Birch Forest
        createMutation(28);  // Birch Forest Hills
        createMutation(29);  // Roofed Forest
        createMutation(32);  // Mega Taiga
        createMutation(3);   // Extreme Hills
        createMutation(34);  // Extreme Hills+

        // Java: megaTaigaHills mutation = megaTaiga mutation
        if (valid_[32 + 128]) {
            biomes_[33 + 128] = biomes_[32 + 128];
            biomes_[33 + 128].biomeID = 33 + 128;
            valid_[33 + 128] = true;
        }

        // Build exploration list (excludes hell, sky, frozenOcean, extremeHillsEdge)
        explorationBiomes_.clear();
        for (int32_t i = 0; i < 128; ++i) {
            if (!valid_[i]) continue;
            if (i == 8 || i == 9 || i == 10 || i == 20) continue;
            explorationBiomes_.push_back(i);
        }

        initialized_ = true;
    }

    // ─── Queries ───

    static const BiomeGenBase* getBiome(int32_t id) {
        if (id < 0 || id >= MAX_BIOMES || !valid_[id]) return &biomes_[0]; // ocean default
        return &biomes_[id];
    }

    static const std::vector<int32_t>& getExplorationBiomes() { return explorationBiomes_; }
    static int32_t getBiomeCount() {
        int32_t c = 0;
        for (int32_t i = 0; i < MAX_BIOMES; ++i) if (valid_[i]) ++c;
        return c;
    }

    // ─── Terrain generation ───

    // Java: BiomeGenBase.genBiomeTerrain
    // Top-down column replacement: stone → topBlock + fillerBlock, bedrock at bottom
    struct TerrainConfig {
        int32_t topBlock;
        int32_t topBlockMeta;
        int32_t fillerBlock;
        float temperature;
    };

    // Process one column (x, z) of a chunk
    // blockColumn: 256 block IDs for this column
    // metaColumn: 256 metadata values
    // stoneNoise: noise value for this column (determines filler depth)
    static void genBiomeTerrain(int32_t* blockColumn, int8_t* metaColumn,
                                   const TerrainConfig& cfg, double stoneNoise,
                                   int32_t rngSeed) {
        constexpr int32_t WATER = 9;
        constexpr int32_t ICE = 79;
        constexpr int32_t BEDROCK = 7;

        // Simple LCG for bedrock randomness
        auto nextInt = [&rngSeed](int32_t bound) -> int32_t {
            rngSeed = rngSeed * 1103515245 + 12345;
            return ((rngSeed >> 16) & 0x7fff) % bound;
        };
        auto nextDouble = [&rngSeed]() -> double {
            rngSeed = rngSeed * 1103515245 + 12345;
            return static_cast<double>((rngSeed >> 16) & 0x7fff) / 32768.0;
        };

        int32_t topBlock = cfg.topBlock;
        int8_t topMeta = static_cast<int8_t>(cfg.topBlockMeta & 0xFF);
        int32_t fillerBlock = cfg.fillerBlock;
        int32_t depth = -1;
        int32_t fillerDepth = static_cast<int32_t>(stoneNoise / 3.0 + 3.0 + nextDouble() * 0.25);

        for (int32_t y = 255; y >= 0; --y) {
            // Bedrock at bottom
            if (y <= nextInt(5)) {
                blockColumn[y] = BEDROCK;
                continue;
            }

            int32_t block = blockColumn[y];
            if (block == 0) {  // air
                depth = -1;
                continue;
            }
            if (block != STONE) continue;

            if (depth == -1) {
                if (fillerDepth <= 0) {
                    topBlock = 0;
                    topMeta = 0;
                    fillerBlock = STONE;
                } else if (y >= 59 && y <= 64) {
                    topBlock = cfg.topBlock;
                    topMeta = static_cast<int8_t>(cfg.topBlockMeta & 0xFF);
                    fillerBlock = cfg.fillerBlock;
                }

                // Below sea level: ice or water
                if (y < 63 && (topBlock == 0)) {
                    if (cfg.temperature < 0.15f) {
                        topBlock = ICE;
                        topMeta = 0;
                    } else {
                        topBlock = WATER;
                        topMeta = 0;
                    }
                }

                depth = fillerDepth;
                if (y >= 62) {
                    blockColumn[y] = topBlock;
                    metaColumn[y] = topMeta;
                } else if (y < 56 - fillerDepth) {
                    topBlock = 0;
                    fillerBlock = STONE;
                    blockColumn[y] = GRAVEL;
                } else {
                    blockColumn[y] = fillerBlock;
                }
            } else if (depth > 0) {
                --depth;
                blockColumn[y] = fillerBlock;
                if (depth == 0 && fillerBlock == SAND) {
                    depth = nextInt(4) + std::max(0, y - 63);
                    fillerBlock = SANDSTONE;
                }
            }
        }
    }

private:
    static inline std::array<BiomeGenBase, MAX_BIOMES> biomes_{};
    static inline std::array<bool, MAX_BIOMES> valid_{};
    static inline std::vector<int32_t> explorationBiomes_;
    static inline bool initialized_ = false;
};

} // namespace mccpp
