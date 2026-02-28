/**
 * BiomeRegistry.cpp — Biome registry implementation.
 *
 * Java reference: net.minecraft.world.biome.BiomeGenBase static initializer
 *
 * All 40 vanilla biomes (IDs 0-39) with exact values from Java source.
 * Temperature/rainfall/height values match the decompiled BiomeGenBase.
 * Default mob spawn lists match the BiomeGenBase constructor.
 */

#include "biome/BiomeRegistry.h"
#include <cmath>
#include <iostream>

namespace mccpp {

// ═════════════════════════════════════════════════════════════════════════════
// Biome::getFloatTemperature — Temperature with altitude adjustment
// ═════════════════════════════════════════════════════════════════════════════

float Biome::getFloatTemperature(int32_t x, int32_t y, int32_t z) const {
    // Java: BiomeGenBase.getFloatTemperature
    // Above Y=64, temperature decreases with altitude
    if (y > 64) {
        // Simplified noise (Java uses PerlinNoise at x/8, z/8 * 4.0)
        // We approximate the noise contribution
        float altitudeAdjust = (static_cast<float>(y) - 64.0f) * 0.05f / 30.0f;
        return temperature - altitudeAdjust;
    }
    return temperature;
}

// ═════════════════════════════════════════════════════════════════════════════
// Default spawn lists
// ═════════════════════════════════════════════════════════════════════════════

std::vector<SpawnEntry> BiomeRegistry::defaultMonsters() {
    return {
        {"Spider",   100, 4, 4},
        {"Zombie",   100, 4, 4},
        {"Skeleton", 100, 4, 4},
        {"Creeper",  100, 4, 4},
        {"Slime",    100, 4, 4},
        {"Enderman",  10, 1, 4},
        {"Witch",      5, 1, 1}
    };
}

std::vector<SpawnEntry> BiomeRegistry::defaultCreatures() {
    return {
        {"Sheep",    12, 4, 4},
        {"Pig",      10, 4, 4},
        {"Chicken",  10, 4, 4},
        {"Cow",       8, 4, 4}
    };
}

std::vector<SpawnEntry> BiomeRegistry::defaultWaterCreatures() {
    return {{"Squid", 10, 4, 4}};
}

std::vector<SpawnEntry> BiomeRegistry::defaultCaveCreatures() {
    return {{"Bat", 10, 8, 8}};
}

// ═════════════════════════════════════════════════════════════════════════════
// BiomeRegistry
// ═════════════════════════════════════════════════════════════════════════════

std::vector<Biome> BiomeRegistry::biomes_;
bool BiomeRegistry::initialized_ = false;

void BiomeRegistry::init() {
    if (initialized_) return;

    auto dm = defaultMonsters();
    auto dc = defaultCreatures();
    auto dw = defaultWaterCreatures();
    auto dca = defaultCaveCreatures();

    // Helper: create biome with defaults
    auto makeBiome = [&](int32_t id, const std::string& name, int32_t color,
                          float temp, float rain, float minH, float maxH,
                          bool snow, bool rainEn, int32_t topBlock = 2,
                          int32_t fillerBlock = 3) -> Biome {
        return {id, name, color, temp, rain, minH, maxH, snow, rainEn,
                topBlock, fillerBlock, 0xFFFFFF,
                dm, dc, dw, dca, -1};
    };

    biomes_ = {
        // ID 0: Ocean — deep water, default temp/rain
        makeBiome(0, "Ocean", 112, 0.5f, 0.5f,
                  BiomeHeights::OCEANS.rootHeight, BiomeHeights::OCEANS.variation,
                  false, true),

        // ID 1: Plains — flat, temperate
        makeBiome(1, "Plains", 9286496, 0.8f, 0.4f,
                  BiomeHeights::DEFAULT.rootHeight, BiomeHeights::DEFAULT.variation,
                  false, true),

        // ID 2: Desert — hot, no rain, sand surface
        makeBiome(2, "Desert", 16421912, 2.0f, 0.0f,
                  BiomeHeights::LOW_PLAINS.rootHeight, BiomeHeights::LOW_PLAINS.variation,
                  false, false, 12, 12), // sand on sand

        // ID 3: Extreme Hills — cold, elevated
        makeBiome(3, "Extreme Hills", 0x606060, 0.2f, 0.3f,
                  BiomeHeights::MID_HILLS.rootHeight, BiomeHeights::MID_HILLS.variation,
                  false, true),

        // ID 4: Forest — temperate, trees
        makeBiome(4, "Forest", 353825, 0.7f, 0.8f,
                  BiomeHeights::DEFAULT.rootHeight, BiomeHeights::DEFAULT.variation,
                  false, true),

        // ID 5: Taiga — cool, spruce trees
        makeBiome(5, "Taiga", 747097, 0.25f, 0.8f,
                  BiomeHeights::MID_PLAINS.rootHeight, BiomeHeights::MID_PLAINS.variation,
                  false, true),

        // ID 6: Swampland — warm, wet, partially submerged
        makeBiome(6, "Swampland", 522674, 0.8f, 0.9f,
                  BiomeHeights::PARTIALLY_SUBMERGED.rootHeight, BiomeHeights::PARTIALLY_SUBMERGED.variation,
                  false, true),

        // ID 7: River — shallow water
        makeBiome(7, "River", 255, 0.5f, 0.5f,
                  BiomeHeights::SHALLOW_WATERS.rootHeight, BiomeHeights::SHALLOW_WATERS.variation,
                  false, true),

        // ID 8: Hell (Nether) — hot, no rain
        makeBiome(8, "Hell", 0xFF0000, 2.0f, 0.0f,
                  BiomeHeights::DEFAULT.rootHeight, BiomeHeights::DEFAULT.variation,
                  false, false, 87, 87), // netherrack

        // ID 9: Sky (The End) — no rain
        makeBiome(9, "Sky", 0x8080FF, 0.5f, 0.5f,
                  BiomeHeights::DEFAULT.rootHeight, BiomeHeights::DEFAULT.variation,
                  false, false, 121, 121), // end stone

        // ID 10: Frozen Ocean — cold, snowy
        makeBiome(10, "FrozenOcean", 0x9090A0, 0.0f, 0.5f,
                  BiomeHeights::OCEANS.rootHeight, BiomeHeights::OCEANS.variation,
                  true, true),

        // ID 11: Frozen River — cold, snowy
        makeBiome(11, "FrozenRiver", 0xA0A0FF, 0.0f, 0.5f,
                  BiomeHeights::SHALLOW_WATERS.rootHeight, BiomeHeights::SHALLOW_WATERS.variation,
                  true, true),

        // ID 12: Ice Plains — cold, flat, snowy
        makeBiome(12, "Ice Plains", 0xFFFFFF, 0.0f, 0.5f,
                  BiomeHeights::LOW_PLAINS.rootHeight, BiomeHeights::LOW_PLAINS.variation,
                  true, true),

        // ID 13: Ice Mountains — cold, elevated, snowy
        makeBiome(13, "Ice Mountains", 0xA0A0A0, 0.0f, 0.5f,
                  BiomeHeights::LOW_HILLS.rootHeight, BiomeHeights::LOW_HILLS.variation,
                  true, true),

        // ID 14: Mushroom Island — warm, mycelium surface
        makeBiome(14, "MushroomIsland", 0xFF00FF, 0.9f, 1.0f,
                  BiomeHeights::LOW_ISLANDS.rootHeight, BiomeHeights::LOW_ISLANDS.variation,
                  false, true, 110, 3), // mycelium

        // ID 15: Mushroom Island Shore — warm, flat shore
        makeBiome(15, "MushroomIslandShore", 0xA000FF, 0.9f, 1.0f,
                  BiomeHeights::SHORES.rootHeight, BiomeHeights::SHORES.variation,
                  false, true, 110, 3), // mycelium

        // ID 16: Beach — warm, sand
        makeBiome(16, "Beach", 16440917, 0.8f, 0.4f,
                  BiomeHeights::SHORES.rootHeight, BiomeHeights::SHORES.variation,
                  false, true, 12, 12), // sand

        // ID 17: Desert Hills — hot, elevated, no rain
        makeBiome(17, "DesertHills", 13786898, 2.0f, 0.0f,
                  BiomeHeights::LOW_HILLS.rootHeight, BiomeHeights::LOW_HILLS.variation,
                  false, false, 12, 12),

        // ID 18: Forest Hills — temperate, elevated
        makeBiome(18, "ForestHills", 2250012, 0.7f, 0.8f,
                  BiomeHeights::LOW_HILLS.rootHeight, BiomeHeights::LOW_HILLS.variation,
                  false, true),

        // ID 19: Taiga Hills — cool, elevated
        makeBiome(19, "TaigaHills", 1456435, 0.25f, 0.8f,
                  BiomeHeights::LOW_HILLS.rootHeight, BiomeHeights::LOW_HILLS.variation,
                  false, true),

        // ID 20: Extreme Hills Edge — cool, transitional
        makeBiome(20, "Extreme Hills Edge", 7501978, 0.2f, 0.3f,
                  BiomeHeights::MID_HILLS.rootHeight * 0.5f + 0.05f,
                  BiomeHeights::MID_HILLS.variation * 0.5f,
                  false, true),

        // ID 21: Jungle — hot, dense vegetation
        makeBiome(21, "Jungle", 5470985, 0.95f, 0.9f,
                  BiomeHeights::DEFAULT.rootHeight, BiomeHeights::DEFAULT.variation,
                  false, true),

        // ID 22: Jungle Hills — hot, elevated
        makeBiome(22, "JungleHills", 2900485, 0.95f, 0.9f,
                  BiomeHeights::LOW_HILLS.rootHeight, BiomeHeights::LOW_HILLS.variation,
                  false, true),

        // ID 23: Jungle Edge — hot, transitional
        makeBiome(23, "JungleEdge", 6458135, 0.95f, 0.8f,
                  BiomeHeights::DEFAULT.rootHeight, BiomeHeights::DEFAULT.variation,
                  false, true),

        // ID 24: Deep Ocean — very deep water
        makeBiome(24, "Deep Ocean", 48, 0.5f, 0.5f,
                  BiomeHeights::DEEP_OCEANS.rootHeight, BiomeHeights::DEEP_OCEANS.variation,
                  false, true),

        // ID 25: Stone Beach — cold, rocky coast
        makeBiome(25, "Stone Beach", 10658436, 0.2f, 0.3f,
                  BiomeHeights::ROCKY_WATERS.rootHeight, BiomeHeights::ROCKY_WATERS.variation,
                  false, true, 1, 1), // stone

        // ID 26: Cold Beach — cold, snowy beach
        makeBiome(26, "Cold Beach", 16445632, 0.05f, 0.3f,
                  BiomeHeights::SHORES.rootHeight, BiomeHeights::SHORES.variation,
                  true, true, 12, 12), // sand

        // ID 27: Birch Forest — temperate, birch trees
        makeBiome(27, "Birch Forest", 3175492, 0.6f, 0.6f,
                  BiomeHeights::DEFAULT.rootHeight, BiomeHeights::DEFAULT.variation,
                  false, true),

        // ID 28: Birch Forest Hills — temperate, elevated
        makeBiome(28, "Birch Forest Hills", 2055986, 0.6f, 0.6f,
                  BiomeHeights::LOW_HILLS.rootHeight, BiomeHeights::LOW_HILLS.variation,
                  false, true),

        // ID 29: Roofed Forest — dark oak, dense
        makeBiome(29, "Roofed Forest", 4215066, 0.7f, 0.8f,
                  BiomeHeights::DEFAULT.rootHeight, BiomeHeights::DEFAULT.variation,
                  false, true),

        // ID 30: Cold Taiga — very cold, snowy
        makeBiome(30, "Cold Taiga", 3233098, -0.5f, 0.4f,
                  BiomeHeights::MID_PLAINS.rootHeight, BiomeHeights::MID_PLAINS.variation,
                  true, true),

        // ID 31: Cold Taiga Hills — very cold, elevated
        makeBiome(31, "Cold Taiga Hills", 2375478, -0.5f, 0.4f,
                  BiomeHeights::LOW_HILLS.rootHeight, BiomeHeights::LOW_HILLS.variation,
                  true, true),

        // ID 32: Mega Taiga — cool, giant spruce
        makeBiome(32, "Mega Taiga", 5858897, 0.3f, 0.8f,
                  BiomeHeights::MID_PLAINS.rootHeight, BiomeHeights::MID_PLAINS.variation,
                  false, true),

        // ID 33: Mega Taiga Hills — cool, elevated
        makeBiome(33, "Mega Taiga Hills", 4542270, 0.3f, 0.8f,
                  BiomeHeights::LOW_HILLS.rootHeight, BiomeHeights::LOW_HILLS.variation,
                  false, true),

        // ID 34: Extreme Hills+ — cool, with trees
        makeBiome(34, "Extreme Hills+", 0x507050, 0.2f, 0.3f,
                  BiomeHeights::MID_HILLS.rootHeight, BiomeHeights::MID_HILLS.variation,
                  false, true),

        // ID 35: Savanna — warm, flat, dry
        makeBiome(35, "Savanna", 12431967, 1.2f, 0.0f,
                  BiomeHeights::LOW_PLAINS.rootHeight, BiomeHeights::LOW_PLAINS.variation,
                  false, false),

        // ID 36: Savanna Plateau — warm, elevated
        makeBiome(36, "Savanna Plateau", 10984804, 1.0f, 0.0f,
                  BiomeHeights::HIGH_PLATEAUS.rootHeight, BiomeHeights::HIGH_PLATEAUS.variation,
                  false, false),

        // ID 37: Mesa — warm, clay terrain
        makeBiome(37, "Mesa", 14238997, 2.0f, 0.0f,
                  BiomeHeights::DEFAULT.rootHeight, BiomeHeights::DEFAULT.variation,
                  false, false, 12, 172), // sand on hardened clay

        // ID 38: Mesa Plateau F — warm, forested plateau
        makeBiome(38, "Mesa Plateau F", 11573093, 2.0f, 0.0f,
                  BiomeHeights::HIGH_PLATEAUS.rootHeight, BiomeHeights::HIGH_PLATEAUS.variation,
                  false, false, 12, 172),

        // ID 39: Mesa Plateau — warm, barren plateau
        makeBiome(39, "Mesa Plateau", 13274213, 2.0f, 0.0f,
                  BiomeHeights::HIGH_PLATEAUS.rootHeight, BiomeHeights::HIGH_PLATEAUS.variation,
                  false, false, 12, 172),
    };

    // Set mutated variant IDs (base ID + 128)
    // Java: static initializer calls createMutation() for specific biomes
    int32_t mutatedBases[] = {1, 2, 4, 5, 6, 12, 21, 23, 30, 35, 36, 37, 38, 39,
                               27, 28, 29, 32, 3, 34};
    for (int32_t baseId : mutatedBases) {
        for (auto& biome : biomes_) {
            if (biome.id == baseId) {
                biome.mutatedVariantId = baseId + MUTATION_OFFSET;
                break;
            }
        }
    }

    // Customize special biomes' spawn lists
    // Mushroom Island: no monsters
    for (auto& biome : biomes_) {
        if (biome.id == 14 || biome.id == 15) {
            biome.monstersSpawns.clear();
            biome.creatureSpawns = {{"Mooshroom", 8, 4, 8}};
        }
        // Hell: nether mobs
        if (biome.id == 8) {
            biome.monstersSpawns = {
                {"Ghast",          50, 4, 4},
                {"ZombiePigman",  100, 4, 4},
                {"MagmaCube",       2, 4, 4},
                {"Blaze",           1, 4, 4}
            };
            biome.creatureSpawns.clear();
            biome.waterCreatureSpawns.clear();
            biome.caveCreatureSpawns.clear();
        }
        // Sky (End): endermen
        if (biome.id == 9) {
            biome.monstersSpawns = {{"Enderman", 10, 4, 4}};
            biome.creatureSpawns.clear();
            biome.waterCreatureSpawns.clear();
        }
    }

    initialized_ = true;
    std::cout << "[Biome] Registered " << biomes_.size() << " biomes\n";
}

const Biome* BiomeRegistry::getById(int32_t id) {
    for (const auto& biome : biomes_) {
        if (biome.id == id) return &biome;
    }
    // Java: defaults to Ocean if out of bounds
    if (!biomes_.empty()) return &biomes_[0];
    return nullptr;
}

const std::vector<Biome>& BiomeRegistry::getAll() {
    return biomes_;
}

int32_t BiomeRegistry::getCount() {
    return static_cast<int32_t>(biomes_.size());
}

} // namespace mccpp
