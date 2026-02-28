/**
 * SpawnerAnimals.h — Natural mob spawning system.
 *
 * Java reference: net.minecraft.world.SpawnerAnimals
 *
 * Algorithm:
 *   1. Build eligible chunk map: 8-chunk radius around each player.
 *      Border chunks (±8) are marked as ineligible (edge = true).
 *      Interior chunks are eligible (edge = false).
 *
 *   2. For each creature type (monster, creature, waterCreature, ambient):
 *      - Skip if current count > maxCreature × eligibleChunks / 256
 *      - For each eligible (non-border) chunk:
 *        - Pick random position within chunk
 *        - Attempt 3 packs × 4 entities each
 *        - Random walk position ±6 blocks per attempt
 *        - Check canCreatureTypeSpawnAtLocation
 *        - Avoid 24-block radius from players
 *        - Avoid 576-block² from spawn point
 *
 *   3. performWorldGenSpawning — initial creature spawning during
 *      world generation using biome spawn lists and weighted random.
 *
 * Thread safety: Called from world tick thread.
 * JNI readiness: Simple data, standard containers.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EnumCreatureType — Creature categories for spawn caps.
// Java reference: net.minecraft.entity.EnumCreatureType
// ═══════════════════════════════════════════════════════════════════════════

enum class CreatureType : int32_t {
    MONSTER = 0,          // maxCount=70, material=air, peaceful=false, animal=false
    CREATURE = 1,         // maxCount=10, material=air, peaceful=true,  animal=true
    WATER_CREATURE = 2,   // maxCount=5,  material=water, peaceful=true, animal=false
    AMBIENT = 3           // maxCount=15, material=air, peaceful=true,  animal=false
};

struct CreatureTypeInfo {
    CreatureType type;
    int32_t maxCount;
    bool isPeaceful;    // Spawns in peaceful mode
    bool isAnimal;      // Only spawn in initial world gen
    bool isWaterBased;  // Lives in water

    static constexpr CreatureTypeInfo MONSTER  = {CreatureType::MONSTER,         70, false, false, false};
    static constexpr CreatureTypeInfo CREATURE = {CreatureType::CREATURE,        10, true,  true,  false};
    static constexpr CreatureTypeInfo WATER    = {CreatureType::WATER_CREATURE,   5, true,  false, true};
    static constexpr CreatureTypeInfo AMBIENT  = {CreatureType::AMBIENT,         15, true,  false, false};

    static const CreatureTypeInfo& get(CreatureType t) {
        static const CreatureTypeInfo types[4] = {MONSTER, CREATURE, WATER, AMBIENT};
        return types[static_cast<int32_t>(t)];
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// SpawnListEntry — Weighted entry for biome spawn lists.
// Java reference: net.minecraft.world.biome.BiomeGenBase$SpawnListEntry
// ═══════════════════════════════════════════════════════════════════════════

struct SpawnListEntry {
    int32_t entityTypeId;     // Maps to entity type
    int32_t weight;           // Weighted random selection weight
    int32_t minGroupCount;
    int32_t maxGroupCount;

    SpawnListEntry() = default;
    SpawnListEntry(int32_t type, int32_t w, int32_t minG, int32_t maxG)
        : entityTypeId(type), weight(w), minGroupCount(minG), maxGroupCount(maxG) {}
};

// ═══════════════════════════════════════════════════════════════════════════
// ChunkCoordKey — Hashable chunk coordinate for eligible map.
// ═══════════════════════════════════════════════════════════════════════════

struct ChunkCoordKey {
    int32_t x, z;
    bool operator==(const ChunkCoordKey& o) const { return x == o.x && z == o.z; }
};

struct ChunkCoordKeyHash {
    size_t operator()(const ChunkCoordKey& k) const {
        return static_cast<size_t>(k.x) * 73856093u ^ static_cast<size_t>(k.z) * 83492791u;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// SpawnerAnimals — Natural mob spawning algorithm.
// Java reference: net.minecraft.world.SpawnerAnimals
// ═══════════════════════════════════════════════════════════════════════════

class SpawnerAnimals {
public:
    // Player position for eligible chunk calculation
    struct PlayerPos {
        double x, y, z;
    };

    // Spawn point position
    struct SpawnPoint {
        int32_t x, y, z;
    };

    // Callbacks for world interaction
    using CountEntitiesFn = std::function<int32_t(CreatureType)>;
    using GetTopSegmentFn = std::function<int32_t(int32_t, int32_t)>; // chunkX, chunkZ → topFilledSegment
    using IsNormalCubeFn = std::function<bool(int32_t, int32_t, int32_t)>;
    using IsLiquidFn = std::function<bool(int32_t, int32_t, int32_t)>;
    using HasSolidTopFn = std::function<bool(int32_t, int32_t, int32_t)>;
    using IsBedrockFn = std::function<bool(int32_t, int32_t, int32_t)>;
    using GetClosestPlayerDistFn = std::function<double(double, double, double)>; // Returns dist, -1 if none

    // Spawn result — what mob to spawn where
    struct SpawnRequest {
        int32_t entityTypeId;
        double x, y, z;
        float yaw;
    };

    SpawnerAnimals() = default;

    // Java: findChunksForSpawning
    std::vector<SpawnRequest> findChunksForSpawning(
            const std::vector<PlayerPos>& players,
            const SpawnPoint& spawnPoint,
            bool spawnHostile, bool spawnPeaceful, bool spawnAnimals,
            CountEntitiesFn countEntities,
            GetTopSegmentFn getTopSegment,
            IsNormalCubeFn isNormalCube,
            IsLiquidFn isLiquid,
            HasSolidTopFn hasSolidTop,
            IsBedrockFn isBedrock,
            GetClosestPlayerDistFn getClosestPlayerDist,
            uint64_t randomSeed) {

        std::vector<SpawnRequest> results;

        if (!spawnHostile && !spawnPeaceful) return results;

        // 1. Build eligible chunk map
        eligibleChunks_.clear();
        for (const auto& player : players) {
            int32_t chunkX = floorDiv(player.x, 16.0);
            int32_t chunkZ = floorDiv(player.z, 16.0);
            constexpr int32_t RANGE = 8;

            for (int32_t dx = -RANGE; dx <= RANGE; ++dx) {
                for (int32_t dz = -RANGE; dz <= RANGE; ++dz) {
                    bool isBorder = (dx == -RANGE || dx == RANGE || dz == -RANGE || dz == RANGE);
                    ChunkCoordKey key{dx + chunkX, dz + chunkZ};

                    if (!isBorder) {
                        eligibleChunks_[key] = false; // Interior = eligible
                    } else {
                        if (eligibleChunks_.find(key) == eligibleChunks_.end()) {
                            eligibleChunks_[key] = true; // Border = ineligible
                        }
                    }
                }
            }
        }

        int32_t numEligible = static_cast<int32_t>(eligibleChunks_.size());

        // 2. For each creature type
        for (int32_t typeIdx = 0; typeIdx < 4; ++typeIdx) {
            CreatureType type = static_cast<CreatureType>(typeIdx);
            const auto& info = CreatureTypeInfo::get(type);

            // Filter by spawn settings
            if (info.isPeaceful && !spawnPeaceful) continue;
            if (!info.isPeaceful && !spawnHostile) continue;
            if (info.isAnimal && !spawnAnimals) continue;

            // Check spawn cap: count > maxCreature * eligibleChunks / 256
            int32_t currentCount = countEntities(type);
            if (currentCount > info.maxCount * numEligible / 256) continue;

            // Iterate eligible chunks
            for (const auto& [coord, isBorder] : eligibleChunks_) {
                if (isBorder) continue;

                // Random position within chunk
                int32_t topSeg = getTopSegment(coord.x, coord.z);
                int32_t maxY = topSeg + 16 - 1;
                if (maxY <= 0) maxY = 256; // Java: world.getActualHeight()

                int32_t startX = coord.x * 16 + nextInt(randomSeed, 16);
                int32_t startZ = coord.z * 16 + nextInt(randomSeed, 16);
                int32_t startY = nextInt(randomSeed, maxY);

                // Check starting block
                if (isNormalCube(startX, startY, startZ)) continue;
                bool correctMaterial = info.isWaterBased
                    ? isLiquid(startX, startY, startZ)
                    : !isLiquid(startX, startY, startZ); // Air/non-liquid for land mobs
                if (!correctMaterial) continue;

                // 3 packs × 4 attempts
                int32_t spawned = 0;
                for (int32_t pack = 0; pack < 3 && spawned < 4; ++pack) {
                    int32_t wx = startX;
                    int32_t wy = startY;
                    int32_t wz = startZ;
                    constexpr int32_t WANDER = 6;

                    for (int32_t attempt = 0; attempt < 4; ++attempt) {
                        wx += nextInt(randomSeed, WANDER) - nextInt(randomSeed, WANDER);
                        wy += nextInt(randomSeed, 1) - nextInt(randomSeed, 1);
                        wz += nextInt(randomSeed, WANDER) - nextInt(randomSeed, WANDER);

                        // canCreatureTypeSpawnAtLocation
                        if (!canSpawnAt(type, isNormalCube, isLiquid, hasSolidTop, isBedrock,
                                         wx, wy, wz)) continue;

                        // Player distance check (24 blocks)
                        double px = static_cast<double>(wx) + 0.5;
                        double py = static_cast<double>(wy);
                        double pz = static_cast<double>(wz) + 0.5;

                        double closestPlayer = getClosestPlayerDist(px, py, pz);
                        if (closestPlayer >= 0.0 && closestPlayer < 24.0) continue;

                        // Spawn point distance check (576 = 24²)
                        double spx = px - static_cast<double>(spawnPoint.x);
                        double spy = py - static_cast<double>(spawnPoint.y);
                        double spz = pz - static_cast<double>(spawnPoint.z);
                        if (spx * spx + spy * spy + spz * spz < 576.0) continue;

                        SpawnRequest req;
                        req.entityTypeId = 0; // Will be determined by biome spawn list
                        req.x = px;
                        req.y = py;
                        req.z = pz;
                        req.yaw = nextFloat(randomSeed) * 360.0f;
                        results.push_back(req);
                        ++spawned;
                    }
                }
            }
        }

        return results;
    }

    // Java: canCreatureTypeSpawnAtLocation
    static bool canSpawnAt(CreatureType type,
                             IsNormalCubeFn& isNormalCube,
                             IsLiquidFn& isLiquid,
                             HasSolidTopFn& hasSolidTop,
                             IsBedrockFn& isBedrock,
                             int32_t x, int32_t y, int32_t z) {
        const auto& info = CreatureTypeInfo::get(type);

        if (info.isWaterBased) {
            // Water creature: liquid at pos and below, not solid above
            return isLiquid(x, y, z) && isLiquid(x, y - 1, z) && !isNormalCube(x, y + 1, z);
        }

        // Land creature: solid top below, not bedrock, not solid at pos or above
        if (!hasSolidTop(x, y - 1, z)) return false;
        if (isBedrock(x, y - 1, z)) return false;
        if (isNormalCube(x, y, z)) return false;
        if (isLiquid(x, y, z)) return false;
        if (isNormalCube(x, y + 1, z)) return false;
        return true;
    }

    // Java: performWorldGenSpawning (static helper)
    struct WorldGenSpawnRequest {
        int32_t entityTypeId;
        double x, y, z;
        float yaw;
    };

    static std::vector<WorldGenSpawnRequest> performWorldGenSpawning(
            const std::vector<SpawnListEntry>& spawnList,
            float spawningChance,
            int32_t chunkX, int32_t chunkZ,
            int32_t sizeX, int32_t sizeZ,
            uint64_t& randomSeed,
            std::function<int32_t(int32_t, int32_t)> getTopSolidBlock,
            IsNormalCubeFn isNormalCube,
            IsLiquidFn isLiquid,
            HasSolidTopFn hasSolidTop,
            IsBedrockFn isBedrock) {

        std::vector<WorldGenSpawnRequest> results;
        if (spawnList.empty()) return results;

        // Java: while (random.nextFloat() < spawningChance)
        while (nextFloat(randomSeed) < spawningChance) {
            // Weighted random selection
            int32_t totalWeight = 0;
            for (const auto& entry : spawnList) totalWeight += entry.weight;
            if (totalWeight <= 0) break;

            int32_t pick = nextInt(randomSeed, totalWeight);
            const SpawnListEntry* selected = nullptr;
            for (const auto& entry : spawnList) {
                pick -= entry.weight;
                if (pick < 0) { selected = &entry; break; }
            }
            if (!selected) break;

            int32_t groupSize = selected->minGroupCount +
                nextInt(randomSeed, 1 + selected->maxGroupCount - selected->minGroupCount);

            int32_t sx = chunkX + nextInt(randomSeed, sizeX);
            int32_t sz = chunkZ + nextInt(randomSeed, sizeZ);
            int32_t origX = sx, origZ = sz;

            for (int32_t i = 0; i < groupSize; ++i) {
                bool spawned = false;
                for (int32_t attempt = 0; !spawned && attempt < 4; ++attempt) {
                    int32_t topY = getTopSolidBlock(sx, sz);
                    if (canSpawnAt(CreatureType::CREATURE, isNormalCube, isLiquid,
                                    hasSolidTop, isBedrock, sx, topY, sz)) {
                        WorldGenSpawnRequest req;
                        req.entityTypeId = selected->entityTypeId;
                        req.x = static_cast<double>(sx) + 0.5;
                        req.y = static_cast<double>(topY);
                        req.z = static_cast<double>(sz) + 0.5;
                        req.yaw = nextFloat(randomSeed) * 360.0f;
                        results.push_back(req);
                        spawned = true;
                    }
                    sx += nextInt(randomSeed, 5) - nextInt(randomSeed, 5);
                    sz += nextInt(randomSeed, 5) - nextInt(randomSeed, 5);
                    // Clamp to chunk bounds
                    while (sx < chunkX || sx >= chunkX + sizeX ||
                           sz < chunkZ || sz >= chunkZ + sizeZ) {
                        sx = origX + nextInt(randomSeed, 5) - nextInt(randomSeed, 5);
                        sz = origZ + nextInt(randomSeed, 5) - nextInt(randomSeed, 5);
                    }
                }
            }
        }
        return results;
    }

private:
    static int32_t floorDiv(double d, double divisor) {
        int32_t i = static_cast<int32_t>(d / divisor);
        return (d < 0.0 && static_cast<double>(i) * divisor != d) ? i - 1 : i;
    }

    // Simple LCG random
    static int32_t nextInt(uint64_t& seed, int32_t bound) {
        if (bound <= 0) return 0;
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<int32_t>((seed >> 33) % static_cast<uint64_t>(bound));
    }

    static float nextFloat(uint64_t& seed) {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<float>((seed >> 33) & 0x7FFFFF) / static_cast<float>(0x800000);
    }

    std::unordered_map<ChunkCoordKey, bool, ChunkCoordKeyHash> eligibleChunks_;
};

} // namespace mccpp
