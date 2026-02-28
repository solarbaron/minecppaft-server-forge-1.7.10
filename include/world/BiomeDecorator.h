/**
 * BiomeDecorator.h — Biome decoration pipeline.
 *
 * Java reference: net.minecraft.world.biome.BiomeDecorator
 *
 * Complete decoration order (per chunk, called during populate):
 *   1. generateOres:
 *      - dirt:      20 × veinSize=32,  y=[0,  256), uniform
 *      - gravel:    10 × veinSize=32,  y=[0,  256), uniform
 *      - coal:      20 × veinSize=16,  y=[0,  128), uniform
 *      - iron:      20 × veinSize=8,   y=[0,   64), uniform
 *      - gold:       2 × veinSize=8,   y=[0,   32), uniform
 *      - redstone:   8 × veinSize=7,   y=[0,   16), uniform
 *      - diamond:    1 × veinSize=7,   y=[0,   16), uniform
 *      - lapis:      1 × veinSize=6,   y≈16 triangular (center=16, spread=16)
 *
 *   2. Surface features:
 *      sand(×3) → clay(×1) → gravel(×1) → trees(treesPerChunk + 10% bonus) →
 *      bigMushrooms → flowers(×2) → grass(×1) → deadBush → waterlily →
 *      mushrooms(brown ¼, red ⅛) → extraMushrooms(brown ¼, red ⅛) →
 *      reeds(custom + 10) → pumpkin(1/32) → cacti → liquidSprings(50 water + 20 lava)
 *
 * Thread safety: Called from chunk generation thread.
 * JNI readiness: Simple data, functional callbacks.
 */
#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// BiomeDecorator — Feature placement engine.
// Java reference: net.minecraft.world.biome.BiomeDecorator
// ═══════════════════════════════════════════════════════════════════════════

class BiomeDecorator {
public:
    // ─── Per-biome feature counts (Java fields) ───

    int32_t waterlilyPerChunk = 0;
    int32_t treesPerChunk = 0;
    int32_t flowersPerChunk = 2;
    int32_t grassPerChunk = 1;
    int32_t deadBushPerChunk = 0;
    int32_t mushroomsPerChunk = 0;
    int32_t reedsPerChunk = 0;
    int32_t cactiPerChunk = 0;
    int32_t sandPerChunk = 1;      // gravel-as-sand
    int32_t sandPerChunk2 = 3;     // sand
    int32_t clayPerChunk = 1;
    int32_t bigMushroomsPerChunk = 0;
    bool generateLakes = true;

    // ─── Ore distribution table ───

    struct OreConfig {
        std::string name;
        int32_t blockId;
        int32_t veinSize;
        int32_t count;
        int32_t minY;
        int32_t maxY;
        bool triangular;   // genStandardOre2 vs genStandardOre1
    };

    // Block IDs
    static constexpr int32_t DIRT = 3;
    static constexpr int32_t GRAVEL = 13;
    static constexpr int32_t COAL_ORE = 16;
    static constexpr int32_t IRON_ORE = 15;
    static constexpr int32_t GOLD_ORE = 14;
    static constexpr int32_t REDSTONE_ORE = 73;
    static constexpr int32_t DIAMOND_ORE = 56;
    static constexpr int32_t LAPIS_ORE = 21;
    static constexpr int32_t SAND = 12;
    static constexpr int32_t CLAY = 82;
    static constexpr int32_t FLOWING_WATER = 8;
    static constexpr int32_t FLOWING_LAVA = 10;
    static constexpr int32_t YELLOW_FLOWER = 37;
    static constexpr int32_t RED_MUSHROOM = 40;
    static constexpr int32_t BROWN_MUSHROOM = 39;
    static constexpr int32_t PUMPKIN = 86;
    static constexpr int32_t CACTUS = 81;
    static constexpr int32_t REEDS = 83;
    static constexpr int32_t DEAD_BUSH = 32;
    static constexpr int32_t WATERLILY = 111;

    // Java LCG
    struct RNG {
        int64_t seed;
        void setSeed(int64_t s) {
            seed = (s ^ 0x5DEECE66DLL) & ((1LL << 48) - 1);
        }
        int32_t nextInt(int32_t bound) {
            if (bound <= 0) return 0;
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t bits = static_cast<int32_t>(seed >> 17);
            return ((bits % bound) + bound) % bound;
        }
    };

    // ─── Decoration action callbacks ───

    struct DecorationAction {
        enum Type {
            ORE,            // Place ore vein
            SAND_DEPOSIT,   // Sand/gravel near water
            CLAY_DEPOSIT,   // Clay near water
            TREE,           // Generate tree
            BIG_MUSHROOM,   // Generate big mushroom
            FLOWER,         // Place flower
            GRASS,          // Place tall grass
            DEAD_BUSH,      // Place dead bush
            WATERLILY_PLACE,// Place waterlily
            MUSHROOM,       // Place mushroom
            REED,           // Place sugar cane
            PUMPKIN_PLACE,  // Place pumpkin
            CACTUS_PLACE,   // Place cactus
            LIQUID_SPRING,  // Liquid spring block
        };

        Type type;
        int32_t x, y, z;
        int32_t blockId;
        int32_t count;       // For ores: vein size
    };

    // ─── Main decoration pipeline ───

    // Java: generateOres — ore distribution
    static void generateOres(int32_t chunkX, int32_t chunkZ, RNG& rng,
                                std::function<void(const DecorationAction&)> emit) {
        // Java: genStandardOre1(count, gen, minY, maxY)
        // Uniform distribution: y = rand(maxY - minY) + minY
        auto genOre1 = [&](int32_t count, int32_t blockId, int32_t veinSize,
                             int32_t minY, int32_t maxY) {
            for (int32_t i = 0; i < count; ++i) {
                int32_t x = chunkX + rng.nextInt(16);
                int32_t y = rng.nextInt(maxY - minY) + minY;
                int32_t z = chunkZ + rng.nextInt(16);
                emit({DecorationAction::ORE, x, y, z, blockId, veinSize});
            }
        };

        // Java: genStandardOre2(count, gen, center, spread)
        // Triangular distribution: y = rand(spread) + rand(spread) + (center - spread)
        auto genOre2 = [&](int32_t count, int32_t blockId, int32_t veinSize,
                             int32_t center, int32_t spread) {
            for (int32_t i = 0; i < count; ++i) {
                int32_t x = chunkX + rng.nextInt(16);
                int32_t y = rng.nextInt(spread) + rng.nextInt(spread) + (center - spread);
                int32_t z = chunkZ + rng.nextInt(16);
                emit({DecorationAction::ORE, x, y, z, blockId, veinSize});
            }
        };

        genOre1(20, DIRT,         32, 0, 256);
        genOre1(10, GRAVEL,       32, 0, 256);
        genOre1(20, COAL_ORE,     16, 0, 128);
        genOre1(20, IRON_ORE,      8, 0,  64);
        genOre1( 2, GOLD_ORE,      8, 0,  32);
        genOre1( 8, REDSTONE_ORE,  7, 0,  16);
        genOre1( 1, DIAMOND_ORE,   7, 0,  16);
        genOre2( 1, LAPIS_ORE,     6, 16, 16);
    }

    // Java: genDecorations — full decoration pipeline
    void decorate(int32_t chunkX, int32_t chunkZ, RNG& rng,
                    std::function<int32_t(int32_t, int32_t)> getHeight,
                    std::function<int32_t(int32_t, int32_t)> getTopSolid,
                    std::function<void(const DecorationAction&)> emit) {

        // 1. Ores
        generateOres(chunkX, chunkZ, rng, emit);

        // 2. Sand deposits (×3)
        for (int32_t i = 0; i < sandPerChunk2; ++i) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t y = getTopSolid(x, z);
            emit({DecorationAction::SAND_DEPOSIT, x, y, z, SAND, 7});
        }

        // 3. Clay deposits (×1)
        for (int32_t i = 0; i < clayPerChunk; ++i) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t y = getTopSolid(x, z);
            emit({DecorationAction::CLAY_DEPOSIT, x, y, z, CLAY, 4});
        }

        // 4. Gravel-as-sand deposits (×1)
        for (int32_t i = 0; i < sandPerChunk; ++i) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t y = getTopSolid(x, z);
            emit({DecorationAction::SAND_DEPOSIT, x, y, z, GRAVEL, 6});
        }

        // 5. Trees (treesPerChunk + 10% bonus)
        int32_t treeCount = treesPerChunk;
        if (rng.nextInt(10) == 0) ++treeCount;
        for (int32_t i = 0; i < treeCount; ++i) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t y = getHeight(x, z);
            emit({DecorationAction::TREE, x, y, z, 0, 0});
        }

        // 6. Big mushrooms
        for (int32_t i = 0; i < bigMushroomsPerChunk; ++i) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t y = getHeight(x, z);
            emit({DecorationAction::BIG_MUSHROOM, x, y, z, 0, 0});
        }

        // 7. Flowers (×2)
        for (int32_t i = 0; i < flowersPerChunk; ++i) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t h = getHeight(x, z);
            int32_t y = (h + 32 > 0) ? rng.nextInt(h + 32) : 0;
            emit({DecorationAction::FLOWER, x, y, z, YELLOW_FLOWER, 0});
        }

        // 8. Grass (×grassPerChunk)
        for (int32_t i = 0; i < grassPerChunk; ++i) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t h = getHeight(x, z);
            int32_t y = (h * 2 > 0) ? rng.nextInt(h * 2) : 0;
            emit({DecorationAction::GRASS, x, y, z, 0, 0});
        }

        // 9. Dead bushes
        for (int32_t i = 0; i < deadBushPerChunk; ++i) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t h = getHeight(x, z);
            int32_t y = (h * 2 > 0) ? rng.nextInt(h * 2) : 0;
            emit({DecorationAction::DEAD_BUSH, x, y, z, DEAD_BUSH, 0});
        }

        // 10. Waterlilies
        for (int32_t i = 0; i < waterlilyPerChunk; ++i) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t h = getHeight(x, z);
            int32_t y = (h * 2 > 0) ? rng.nextInt(h * 2) : 0;
            emit({DecorationAction::WATERLILY_PLACE, x, y, z, WATERLILY, 0});
        }

        // 11. Mushrooms (per mushroomsPerChunk)
        for (int32_t i = 0; i < mushroomsPerChunk; ++i) {
            if (rng.nextInt(4) == 0) {
                int32_t x = chunkX + rng.nextInt(16) + 8;
                int32_t z = chunkZ + rng.nextInt(16) + 8;
                int32_t y = getHeight(x, z);
                emit({DecorationAction::MUSHROOM, x, y, z, BROWN_MUSHROOM, 0});
            }
            if (rng.nextInt(8) == 0) {
                int32_t x = chunkX + rng.nextInt(16) + 8;
                int32_t z = chunkZ + rng.nextInt(16) + 8;
                int32_t h = getHeight(x, z);
                int32_t y = (h * 2 > 0) ? rng.nextInt(h * 2) : 0;
                emit({DecorationAction::MUSHROOM, x, y, z, RED_MUSHROOM, 0});
            }
        }

        // 12. Extra mushrooms (always, not per-count)
        if (rng.nextInt(4) == 0) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t h = getHeight(x, z);
            int32_t y = (h * 2 > 0) ? rng.nextInt(h * 2) : 0;
            emit({DecorationAction::MUSHROOM, x, y, z, BROWN_MUSHROOM, 0});
        }
        if (rng.nextInt(8) == 0) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t h = getHeight(x, z);
            int32_t y = (h * 2 > 0) ? rng.nextInt(h * 2) : 0;
            emit({DecorationAction::MUSHROOM, x, y, z, RED_MUSHROOM, 0});
        }

        // 13. Reeds (reedsPerChunk + always 10 extra)
        for (int32_t i = 0; i < reedsPerChunk; ++i) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t h = getHeight(x, z);
            int32_t y = (h * 2 > 0) ? rng.nextInt(h * 2) : 0;
            emit({DecorationAction::REED, x, y, z, REEDS, 0});
        }
        for (int32_t i = 0; i < 10; ++i) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t h = getHeight(x, z);
            int32_t y = (h * 2 > 0) ? rng.nextInt(h * 2) : 0;
            emit({DecorationAction::REED, x, y, z, REEDS, 0});
        }

        // 14. Pumpkins (1/32 chance)
        if (rng.nextInt(32) == 0) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t h = getHeight(x, z);
            int32_t y = (h * 2 > 0) ? rng.nextInt(h * 2) : 0;
            emit({DecorationAction::PUMPKIN_PLACE, x, y, z, PUMPKIN, 0});
        }

        // 15. Cacti
        for (int32_t i = 0; i < cactiPerChunk; ++i) {
            int32_t x = chunkX + rng.nextInt(16) + 8;
            int32_t z = chunkZ + rng.nextInt(16) + 8;
            int32_t h = getHeight(x, z);
            int32_t y = (h * 2 > 0) ? rng.nextInt(h * 2) : 0;
            emit({DecorationAction::CACTUS_PLACE, x, y, z, CACTUS, 0});
        }

        // 16. Liquid springs (if generateLakes)
        if (generateLakes) {
            // 50 water springs
            for (int32_t i = 0; i < 50; ++i) {
                int32_t x = chunkX + rng.nextInt(16) + 8;
                int32_t y = rng.nextInt(rng.nextInt(248) + 8);
                int32_t z = chunkZ + rng.nextInt(16) + 8;
                emit({DecorationAction::LIQUID_SPRING, x, y, z, FLOWING_WATER, 0});
            }
            // 20 lava springs
            for (int32_t i = 0; i < 20; ++i) {
                int32_t x = chunkX + rng.nextInt(16) + 8;
                int32_t y = rng.nextInt(rng.nextInt(rng.nextInt(240) + 8) + 8);
                int32_t z = chunkZ + rng.nextInt(16) + 8;
                emit({DecorationAction::LIQUID_SPRING, x, y, z, FLOWING_LAVA, 0});
            }
        }
    }

    // ─── Biome-specific presets ───

    // Plains: 0 trees, 2 flowers, 1 grass
    static BiomeDecorator plains() {
        BiomeDecorator d;
        d.flowersPerChunk = 4;  // plains has extra flowers
        return d;
    }

    // Forest: 10 trees, 2 flowers
    static BiomeDecorator forest() {
        BiomeDecorator d;
        d.treesPerChunk = 10;
        d.grassPerChunk = 2;
        return d;
    }

    // Desert: 0 trees, 0 flowers, 2 dead bush, 5 cacti
    static BiomeDecorator desert() {
        BiomeDecorator d;
        d.deadBushPerChunk = 2;
        d.cactiPerChunk = 5;
        d.reedsPerChunk = 50;
        d.flowersPerChunk = 0;
        d.grassPerChunk = 0;
        return d;
    }

    // Swampland: 2 trees, 1 flower, 5 grass, 8 mushrooms, 4 waterlily, 10 reeds
    static BiomeDecorator swampland() {
        BiomeDecorator d;
        d.treesPerChunk = 2;
        d.grassPerChunk = 5;
        d.mushroomsPerChunk = 8;
        d.waterlilyPerChunk = 4;
        d.reedsPerChunk = 10;
        return d;
    }

    // Jungle: 50 trees, 25 grass, 4 flowers
    static BiomeDecorator jungle() {
        BiomeDecorator d;
        d.treesPerChunk = 50;
        d.grassPerChunk = 25;
        d.flowersPerChunk = 4;
        return d;
    }

    // Taiga: 10 trees, 1 grass, 1 mushroom
    static BiomeDecorator taiga() {
        BiomeDecorator d;
        d.treesPerChunk = 10;
        d.mushroomsPerChunk = 1;
        return d;
    }

    // Mushroom island: 1 big mushroom, 3 mushroom
    static BiomeDecorator mushroomIsland() {
        BiomeDecorator d;
        d.bigMushroomsPerChunk = 1;
        d.mushroomsPerChunk = 3;
        d.flowersPerChunk = 0;
        d.grassPerChunk = 0;
        return d;
    }

    // Ice plains: 0 trees, 0 flowers, 4 grass
    static BiomeDecorator icePlains() {
        BiomeDecorator d;
        d.flowersPerChunk = 0;
        d.grassPerChunk = 4;
        return d;
    }
};

} // namespace mccpp
