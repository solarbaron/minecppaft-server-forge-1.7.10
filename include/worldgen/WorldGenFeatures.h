/**
 * WorldGenLakes.h — Lake generation (water and lava).
 * WorldGenDungeons.h — Dungeon generation with chests and spawners.
 *
 * Java references:
 *   - net.minecraft.world.gen.feature.WorldGenLakes — Lake carving
 *   - net.minecraft.world.gen.feature.WorldGenDungeons — Dungeon placement
 *
 * Lake algorithm:
 *   1. Find ground level, subtract 4 for base
 *   2. Generate 4-8 random ellipsoids in 16×8×16 boolean grid
 *   3. Validate edges: no liquid above y=4, solid edges below y=4
 *   4. Place liquid (below y=4) and air (above y=4) in carved area
 *   5. Dirt below sky-lit carved blocks → grass/mycelium
 *   6. Lava lakes: stone border on adjacent solid blocks
 *   7. Water lakes: freeze top surface in cold biomes
 *
 * Dungeon algorithm:
 *   1. Room size: (2+rand(2)) × 3 × (2+rand(2))
 *   2. Validate: solid floor/ceiling, 1-5 doorways on walls at floor level
 *   3. Build cobblestone/mossy_cobblestone walls (floor: 75% mossy)
 *   4. Place 2 chests against exactly 1 solid wall
 *   5. Mob spawner at center (25% skeleton, 50% zombie, 25% spider)
 *
 * Thread safety: Called from chunk generation thread.
 * JNI readiness: Simple data, predictable layout.
 */
#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// WorldGenLakes — Lake generation.
// Java reference: net.minecraft.world.gen.feature.WorldGenLakes
// ═══════════════════════════════════════════════════════════════════════════

class WorldGenLakes {
public:
    // Block IDs
    static constexpr int32_t AIR = 0;
    static constexpr int32_t STONE = 1;
    static constexpr int32_t WATER = 9;
    static constexpr int32_t LAVA = 11;
    static constexpr int32_t DIRT = 3;
    static constexpr int32_t GRASS = 2;
    static constexpr int32_t MYCELIUM = 110;
    static constexpr int32_t ICE = 79;

    // RNG
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
        double nextDouble() {
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t hi = static_cast<int32_t>(seed >> 22);
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t lo = static_cast<int32_t>(seed >> 22);
            return static_cast<double>((static_cast<int64_t>(hi) << 27) + lo) /
                   static_cast<double>(1LL << 53);
        }
    };

    // Block modification
    struct BlockMod {
        int32_t x, y, z;
        int32_t blockId;
        int32_t meta;
    };

    // Callbacks
    using GetBlockFn = std::function<int32_t(int32_t, int32_t, int32_t)>;
    using IsAirFn = std::function<bool(int32_t, int32_t, int32_t)>;
    using IsSolidFn = std::function<bool(int32_t, int32_t, int32_t)>;
    using IsLiquidFn = std::function<bool(int32_t, int32_t, int32_t)>;
    using GetSkyLightFn = std::function<int32_t(int32_t, int32_t, int32_t)>;
    using GetBiomeTopBlockFn = std::function<int32_t(int32_t, int32_t)>;
    using IsFreezableFn = std::function<bool(int32_t, int32_t, int32_t)>;

    // Java: WorldGenLakes.generate
    static std::vector<BlockMod> generate(
            int32_t x, int32_t y, int32_t z, int32_t liquidBlockId,
            RNG& rng,
            GetBlockFn getBlock, IsAirFn isAir, IsSolidFn isSolid,
            IsLiquidFn isLiquid, GetSkyLightFn getSkyLight,
            GetBiomeTopBlockFn getBiomeTopBlock, IsFreezableFn isFreezable) {

        std::vector<BlockMod> mods;

        // Offset by -8
        x -= 8;
        z -= 8;

        // Find ground level
        while (y > 5 && isAir(x, y, z)) --y;
        if (y <= 4) return mods;
        y -= 4;

        // Generate ellipsoid shape in 16×16×8 grid
        std::array<bool, 2048> carved{};
        int32_t numEllipsoids = rng.nextInt(4) + 4;

        for (int32_t e = 0; e < numEllipsoids; ++e) {
            double rx = rng.nextDouble() * 6.0 + 3.0;
            double ry = rng.nextDouble() * 4.0 + 2.0;
            double rz = rng.nextDouble() * 6.0 + 3.0;
            double cx = rng.nextDouble() * (16.0 - rx - 2.0) + 1.0 + rx / 2.0;
            double cy = rng.nextDouble() * (8.0 - ry - 4.0) + 2.0 + ry / 2.0;
            double cz = rng.nextDouble() * (16.0 - rz - 2.0) + 1.0 + rz / 2.0;

            for (int32_t bx = 1; bx < 15; ++bx) {
                for (int32_t bz = 1; bz < 15; ++bz) {
                    for (int32_t by = 1; by < 7; ++by) {
                        double dx = (static_cast<double>(bx) - cx) / (rx / 2.0);
                        double dy = (static_cast<double>(by) - cy) / (ry / 2.0);
                        double dz = (static_cast<double>(bz) - cz) / (rz / 2.0);
                        if (dx * dx + dy * dy + dz * dz < 1.0) {
                            carved[(bx * 16 + bz) * 8 + by] = true;
                        }
                    }
                }
            }
        }

        // Validate edges
        for (int32_t bx = 0; bx < 16; ++bx) {
            for (int32_t bz = 0; bz < 16; ++bz) {
                for (int32_t by = 0; by < 8; ++by) {
                    bool isEdge = !carved[(bx * 16 + bz) * 8 + by] &&
                        ((bx < 15 && carved[((bx + 1) * 16 + bz) * 8 + by]) ||
                         (bx > 0  && carved[((bx - 1) * 16 + bz) * 8 + by]) ||
                         (bz < 15 && carved[(bx * 16 + (bz + 1)) * 8 + by]) ||
                         (bz > 0  && carved[(bx * 16 + (bz - 1)) * 8 + by]) ||
                         (by < 7  && carved[(bx * 16 + bz) * 8 + (by + 1)]) ||
                         (by > 0  && carved[(bx * 16 + bz) * 8 + (by - 1)]));

                    if (!isEdge) continue;

                    // Above y=4: reject if liquid edge
                    if (by >= 4 && isLiquid(x + bx, y + by, z + bz)) return mods;
                    // Below y=4: reject if not solid and not the liquid type
                    if (by < 4 && !isSolid(x + bx, y + by, z + bz) &&
                        getBlock(x + bx, y + by, z + bz) != liquidBlockId) return mods;
                }
            }
        }

        // Place blocks
        for (int32_t bx = 0; bx < 16; ++bx) {
            for (int32_t bz = 0; bz < 16; ++bz) {
                for (int32_t by = 0; by < 8; ++by) {
                    if (!carved[(bx * 16 + bz) * 8 + by]) continue;
                    mods.push_back({x + bx, y + by, z + bz,
                                   by >= 4 ? AIR : liquidBlockId, 0});
                }
            }
        }

        // Dirt → grass/mycelium for sky-lit positions above carved
        for (int32_t bx = 0; bx < 16; ++bx) {
            for (int32_t bz = 0; bz < 16; ++bz) {
                for (int32_t by = 4; by < 8; ++by) {
                    if (!carved[(bx * 16 + bz) * 8 + by]) continue;
                    if (getBlock(x + bx, y + by - 1, z + bz) != DIRT) continue;
                    if (getSkyLight(x + bx, y + by, z + bz) <= 0) continue;
                    int32_t topBlock = getBiomeTopBlock(x + bx, z + bz);
                    mods.push_back({x + bx, y + by - 1, z + bz,
                                   topBlock == MYCELIUM ? MYCELIUM : GRASS, 0});
                }
            }
        }

        // Lava lakes: stone border on adjacent solid blocks
        if (liquidBlockId == LAVA) {
            for (int32_t bx = 0; bx < 16; ++bx) {
                for (int32_t bz = 0; bz < 16; ++bz) {
                    for (int32_t by = 0; by < 8; ++by) {
                        bool isEdge = !carved[(bx * 16 + bz) * 8 + by] &&
                            ((bx < 15 && carved[((bx + 1) * 16 + bz) * 8 + by]) ||
                             (bx > 0  && carved[((bx - 1) * 16 + bz) * 8 + by]) ||
                             (bz < 15 && carved[(bx * 16 + (bz + 1)) * 8 + by]) ||
                             (bz > 0  && carved[(bx * 16 + (bz - 1)) * 8 + by]) ||
                             (by < 7  && carved[(bx * 16 + bz) * 8 + (by + 1)]) ||
                             (by > 0  && carved[(bx * 16 + bz) * 8 + (by - 1)]));
                        if (!isEdge) continue;
                        if (by >= 4 && rng.nextInt(2) == 0) continue;
                        if (!isSolid(x + bx, y + by, z + bz)) continue;
                        mods.push_back({x + bx, y + by, z + bz, STONE, 0});
                    }
                }
            }
        }

        // Water lakes: freeze surface in cold biomes
        if (liquidBlockId == WATER) {
            for (int32_t bx = 0; bx < 16; ++bx) {
                for (int32_t bz = 0; bz < 16; ++bz) {
                    if (isFreezable(x + bx, y + 4, z + bz)) {
                        mods.push_back({x + bx, y + 4, z + bz, ICE, 0});
                    }
                }
            }
        }

        return mods;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// WorldGenDungeons — Dungeon generation.
// Java reference: net.minecraft.world.gen.feature.WorldGenDungeons
// ═══════════════════════════════════════════════════════════════════════════

class WorldGenDungeons {
public:
    // Block IDs
    static constexpr int32_t AIR = 0;
    static constexpr int32_t COBBLESTONE = 4;
    static constexpr int32_t MOSSY_COBBLESTONE = 48;
    static constexpr int32_t CHEST = 54;
    static constexpr int32_t MOB_SPAWNER = 52;

    // RNG
    using RNG = WorldGenLakes::RNG;

    // Block modification
    struct BlockMod {
        int32_t x, y, z;
        int32_t blockId;
        int32_t meta;
    };

    // Chest/spawner placement
    struct ChestPlacement {
        int32_t x, y, z;
    };

    struct SpawnerPlacement {
        int32_t x, y, z;
        std::string mobType;
    };

    struct DungeonResult {
        std::vector<BlockMod> blocks;
        std::vector<ChestPlacement> chests;
        SpawnerPlacement spawner;
        bool success;
    };

    // Dungeon loot table (item IDs and weights)
    struct LootEntry {
        int32_t itemId;
        int32_t minCount, maxCount;
        int32_t weight;
    };

    static std::vector<LootEntry> getLootTable() {
        return {
            {329, 1, 1, 10},   // saddle
            {265, 1, 4, 10},   // iron_ingot
            {297, 1, 1, 10},   // bread
            {296, 1, 4, 10},   // wheat
            {289, 1, 4, 10},   // gunpowder
            {287, 1, 4, 10},   // string
            {325, 1, 1, 10},   // bucket
            {322, 1, 1, 1},    // golden_apple
            {331, 1, 4, 10},   // redstone
            {2256, 1, 1, 10},  // record_13
            {2257, 1, 1, 10},  // record_cat
            {421, 1, 1, 10},   // name_tag
            {418, 1, 1, 2},    // golden_horse_armor
            {417, 1, 1, 5},    // iron_horse_armor
            {419, 1, 1, 1},    // diamond_horse_armor
        };
    }

    // Callbacks
    using IsAirFn = std::function<bool(int32_t, int32_t, int32_t)>;
    using IsSolidFn = std::function<bool(int32_t, int32_t, int32_t)>;

    // Java: WorldGenDungeons.generate
    static DungeonResult generate(int32_t x, int32_t y, int32_t z,
                                     RNG& rng,
                                     IsAirFn isAir, IsSolidFn isSolid) {
        DungeonResult result;
        result.success = false;

        int32_t height = 3;
        int32_t halfX = rng.nextInt(2) + 2;  // 2-3
        int32_t halfZ = rng.nextInt(2) + 2;  // 2-3

        // Count doorways
        int32_t doorways = 0;
        for (int32_t bx = x - halfX - 1; bx <= x + halfX + 1; ++bx) {
            for (int32_t by = y - 1; by <= y + height + 1; ++by) {
                for (int32_t bz = z - halfZ - 1; bz <= z + halfZ + 1; ++bz) {
                    // Floor check
                    if (by == y - 1 && !isSolid(bx, by, bz)) return result;
                    // Ceiling check
                    if (by == y + height + 1 && !isSolid(bx, by, bz)) return result;
                    // Doorway: wall position, floor level, air + air above
                    if ((bx == x - halfX - 1 || bx == x + halfX + 1 ||
                         bz == z - halfZ - 1 || bz == z + halfZ + 1) &&
                        by == y && isAir(bx, by, bz) && isAir(bx, by + 1, bz)) {
                        ++doorways;
                    }
                }
            }
        }

        if (doorways < 1 || doorways > 5) return result;

        // Build walls and clear interior
        for (int32_t bx = x - halfX - 1; bx <= x + halfX + 1; ++bx) {
            for (int32_t by = y + height; by >= y - 1; --by) {
                for (int32_t bz = z - halfZ - 1; bz <= z + halfZ + 1; ++bz) {
                    bool isWall = (bx == x - halfX - 1 || by == y - 1 ||
                                   bz == z - halfZ - 1 || bx == x + halfX + 1 ||
                                   by == y + height + 1 || bz == z + halfZ + 1);

                    if (isWall) {
                        if (by >= 0 && !isSolid(bx, by - 1, bz)) {
                            result.blocks.push_back({bx, by, bz, AIR, 0});
                        } else if (isSolid(bx, by, bz)) {
                            if (by == y - 1 && rng.nextInt(4) != 0) {
                                result.blocks.push_back({bx, by, bz, MOSSY_COBBLESTONE, 0});
                            } else {
                                result.blocks.push_back({bx, by, bz, COBBLESTONE, 0});
                            }
                        }
                    } else {
                        result.blocks.push_back({bx, by, bz, AIR, 0});
                    }
                }
            }
        }

        // Place up to 2 chests
        for (int32_t attempt = 0; attempt < 2; ++attempt) {
            for (int32_t try_ = 0; try_ < 3; ++try_) {
                int32_t cx = x + rng.nextInt(halfX * 2 + 1) - halfX;
                int32_t cz = z + rng.nextInt(halfZ * 2 + 1) - halfZ;

                if (!isAir(cx, y, cz)) continue;

                // Count adjacent solid walls
                int32_t solidWalls = 0;
                if (isSolid(cx - 1, y, cz)) ++solidWalls;
                if (isSolid(cx + 1, y, cz)) ++solidWalls;
                if (isSolid(cx, y, cz - 1)) ++solidWalls;
                if (isSolid(cx, y, cz + 1)) ++solidWalls;

                if (solidWalls != 1) continue;

                result.blocks.push_back({cx, y, cz, CHEST, 0});
                result.chests.push_back({cx, y, cz});
                break;
            }
        }

        // Mob spawner at center
        result.blocks.push_back({x, y, z, MOB_SPAWNER, 0});
        result.spawner = {x, y, z, pickMobSpawner(rng)};
        result.success = true;
        return result;
    }

private:
    // Java: pickMobSpawner — 25% skeleton, 50% zombie, 25% spider
    static std::string pickMobSpawner(RNG& rng) {
        int32_t n = rng.nextInt(4);
        switch (n) {
            case 0: return "Skeleton";
            case 1: return "Zombie";
            case 2: return "Zombie";
            case 3: return "Spider";
            default: return "";
        }
    }
};

} // namespace mccpp
