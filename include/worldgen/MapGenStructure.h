/**
 * MapGenStructure.h — Structure generation framework.
 *
 * Java references:
 *   - net.minecraft.world.gen.MapGenBase — Base map generator
 *   - net.minecraft.world.gen.structure.MapGenStructure — Structure generator
 *   - net.minecraft.world.gen.structure.StructureBoundingBox — AABB for structures
 *   - net.minecraft.world.gen.structure.StructureComponent — Building block
 *   - net.minecraft.world.gen.structure.StructureStart — Generated structure
 *   - net.minecraft.world.gen.structure.MapGenVillage — Village generation
 *   - net.minecraft.world.gen.structure.MapGenMineshaft — Mineshaft generation
 *   - net.minecraft.world.gen.structure.MapGenStronghold — Stronghold generation
 *   - net.minecraft.world.gen.structure.MapGenScatteredFeature — Temple generation
 *
 * Architecture:
 *   1. MapGenBase: scans range=8 radius around target chunk,
 *      seeds RNG per chunk: (chunkX * randL) ^ (chunkZ * randL2) ^ worldSeed
 *   2. MapGenStructure: maintains structureMap (chunkXZ2Int → StructureStart),
 *      test canSpawnStructureAtCoords, create getStructureStart,
 *      populate in generateStructuresInChunk via bounding box intersection,
 *      find nearest via getNearestInstance
 *   3. Specific generators: Village (32-spacing, 8-separation),
 *      Mineshaft (1%, any chunk), Stronghold (3 per world),
 *      ScatteredFeature (32-spacing, desert/jungle/swamp temples)
 *
 * Thread safety: Called from chunk generation thread.
 * JNI readiness: Simple data, predictable layout.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// StructureBoundingBox — Axis-aligned bounding box for structures.
// Java reference: net.minecraft.world.gen.structure.StructureBoundingBox
// ═══════════════════════════════════════════════════════════════════════════

struct StructureBoundingBox {
    int32_t minX, minY, minZ;
    int32_t maxX, maxY, maxZ;

    StructureBoundingBox() : minX(0), minY(0), minZ(0), maxX(0), maxY(0), maxZ(0) {}
    StructureBoundingBox(int32_t x1, int32_t z1, int32_t x2, int32_t z2)
        : minX(x1), minY(1), minZ(z1), maxX(x2), maxY(512), maxZ(z2) {}
    StructureBoundingBox(int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2)
        : minX(x1), minY(y1), minZ(z1), maxX(x2), maxY(y2), maxZ(z2) {}

    bool intersectsWith(int32_t x1, int32_t z1, int32_t x2, int32_t z2) const {
        return maxX >= x1 && minX <= x2 && maxZ >= z1 && minZ <= z2;
    }

    bool intersectsWith(const StructureBoundingBox& other) const {
        return maxX >= other.minX && minX <= other.maxX &&
               maxZ >= other.minZ && minZ <= other.maxZ &&
               maxY >= other.minY && minY <= other.maxY;
    }

    bool isVecInside(int32_t x, int32_t y, int32_t z) const {
        return x >= minX && x <= maxX && y >= minY && y <= maxY && z >= minZ && z <= maxZ;
    }

    void expandTo(const StructureBoundingBox& other) {
        minX = std::min(minX, other.minX);
        minY = std::min(minY, other.minY);
        minZ = std::min(minZ, other.minZ);
        maxX = std::max(maxX, other.maxX);
        maxY = std::max(maxY, other.maxY);
        maxZ = std::max(maxZ, other.maxZ);
    }

    void offset(int32_t x, int32_t y, int32_t z) {
        minX += x; maxX += x;
        minY += y; maxY += y;
        minZ += z; maxZ += z;
    }

    int32_t getXSize() const { return maxX - minX + 1; }
    int32_t getYSize() const { return maxY - minY + 1; }
    int32_t getZSize() const { return maxZ - minZ + 1; }

    int32_t getCenterX() const { return minX + (maxX - minX + 1) / 2; }
    int32_t getCenterY() const { return minY + (maxY - minY + 1) / 2; }
    int32_t getCenterZ() const { return minZ + (maxZ - minZ + 1) / 2; }
};

// ═══════════════════════════════════════════════════════════════════════════
// StructureComponent — A single building piece within a structure.
// Java reference: net.minecraft.world.gen.structure.StructureComponent
// ═══════════════════════════════════════════════════════════════════════════

struct StructureComponent {
    StructureBoundingBox boundingBox;
    int32_t coordBaseMode = -1;  // Rotation: 0=south, 1=west, 2=north, 3=east
    int32_t componentType = 0;

    struct Position {
        int32_t x, y, z;
    };

    Position getCenter() const {
        return {boundingBox.getCenterX(), boundingBox.getCenterY(), boundingBox.getCenterZ()};
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// StructureStart — A generated structure containing components.
// Java reference: net.minecraft.world.gen.structure.StructureStart
// ═══════════════════════════════════════════════════════════════════════════

struct StructureStart {
    std::vector<StructureComponent> components;
    StructureBoundingBox boundingBox;
    int32_t chunkPosX = 0;
    int32_t chunkPosZ = 0;

    bool isSizeableStructure() const { return true; }

    const StructureBoundingBox& getBoundingBox() const { return boundingBox; }

    void updateBoundingBox() {
        if (components.empty()) return;
        boundingBox = components[0].boundingBox;
        for (size_t i = 1; i < components.size(); ++i) {
            boundingBox.expandTo(components[i].boundingBox);
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// ChunkCoordIntPair hash function.
// Java: ChunkCoordIntPair.chunkXZ2Int
// ═══════════════════════════════════════════════════════════════════════════

inline int64_t chunkXZ2Int(int32_t x, int32_t z) {
    return static_cast<int64_t>(x) & 0xFFFFFFFFL |
           (static_cast<int64_t>(z) & 0xFFFFFFFFL) << 32;
}

// ═══════════════════════════════════════════════════════════════════════════
// MapGenBase — Base map feature generator.
// Java reference: net.minecraft.world.gen.MapGenBase
// ═══════════════════════════════════════════════════════════════════════════

class MapGenBase {
public:
    int32_t range = 8;

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
        int64_t nextLong() {
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t hi = static_cast<int32_t>(seed >> 16);
            seed = (seed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            int32_t lo = static_cast<int32_t>(seed >> 16);
            return (static_cast<int64_t>(hi) << 32) | (static_cast<int64_t>(lo) & 0xFFFFFFFFL);
        }
    };

    // Java: generate — scan range radius, seed per chunk
    using GenerateChunkFn = std::function<void(int32_t chunkX, int32_t chunkZ, RNG& rng)>;

    static void generate(int64_t worldSeed, int32_t targetChunkX, int32_t targetChunkZ,
                           int32_t genRange, GenerateChunkFn callback) {
        RNG rng;
        rng.setSeed(worldSeed);
        int64_t randL = rng.nextLong();
        int64_t randL2 = rng.nextLong();

        for (int32_t cx = targetChunkX - genRange; cx <= targetChunkX + genRange; ++cx) {
            for (int32_t cz = targetChunkZ - genRange; cz <= targetChunkZ + genRange; ++cz) {
                int64_t chunkSeed = static_cast<int64_t>(cx) * randL ^
                                     static_cast<int64_t>(cz) * randL2 ^ worldSeed;
                RNG chunkRng;
                chunkRng.setSeed(chunkSeed);
                callback(cx, cz, chunkRng);
            }
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// MapGenStructure — Structure generation framework.
// Java reference: net.minecraft.world.gen.structure.MapGenStructure
// ═══════════════════════════════════════════════════════════════════════════

class MapGenStructure {
public:
    using RNG = MapGenBase::RNG;

    std::unordered_map<int64_t, StructureStart> structureMap;

    virtual ~MapGenStructure() = default;
    virtual std::string getStructureName() const = 0;
    virtual bool canSpawnStructureAtCoords(int32_t chunkX, int32_t chunkZ, RNG& rng) = 0;
    virtual StructureStart getStructureStart(int32_t chunkX, int32_t chunkZ, RNG& rng) = 0;

    // Generate structure placement pass (called during chunk terrain gen)
    void generate(int64_t worldSeed, int32_t chunkX, int32_t chunkZ) {
        MapGenBase::generate(worldSeed, chunkX, chunkZ, 8,
            [this](int32_t cx, int32_t cz, RNG& rng) {
                int64_t key = chunkXZ2Int(cx, cz);
                if (structureMap.count(key)) return;
                rng.nextInt(1);  // Java: rand.nextInt() consumed
                if (canSpawnStructureAtCoords(cx, cz, rng)) {
                    StructureStart start = getStructureStart(cx, cz, rng);
                    structureMap[key] = std::move(start);
                }
            });
    }

    // Java: generateStructuresInChunk — populate structures in chunk
    bool generateStructuresInChunk(int32_t chunkX, int32_t chunkZ) {
        int32_t baseX = (chunkX << 4) + 8;
        int32_t baseZ = (chunkZ << 4) + 8;
        bool generated = false;

        for (auto& [key, start] : structureMap) {
            if (!start.isSizeableStructure()) continue;
            if (!start.getBoundingBox().intersectsWith(baseX, baseZ, baseX + 15, baseZ + 15)) continue;
            generated = true;
        }
        return generated;
    }

    // Java: hasStructureAt — point-in-component test
    bool hasStructureAt(int32_t x, int32_t y, int32_t z) {
        for (auto& [key, start] : structureMap) {
            if (!start.isSizeableStructure()) continue;
            if (!start.getBoundingBox().intersectsWith(x, z, x, z)) continue;
            for (auto& comp : start.components) {
                if (comp.boundingBox.isVecInside(x, y, z)) return true;
            }
        }
        return false;
    }

    // Java: getNearestInstance — find closest structure
    struct NearestResult {
        int32_t x, y, z;
        bool found;
    };

    NearestResult getNearestInstance(int32_t x, int32_t y, int32_t z) {
        double minDist = 1e30;
        NearestResult result{0, 0, 0, false};

        for (auto& [key, start] : structureMap) {
            if (!start.isSizeableStructure() || start.components.empty()) continue;
            auto center = start.components[0].getCenter();
            double dx = center.x - x;
            double dy = center.y - y;
            double dz = center.z - z;
            double dist = dx * dx + dy * dy + dz * dz;
            if (dist < minDist) {
                minDist = dist;
                result = {center.x, center.y, center.z, true};
            }
        }
        return result;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// MapGenVillage — Village generation.
// Java reference: net.minecraft.world.gen.structure.MapGenVillage
// Spacing: 32, separation: 8
// ═══════════════════════════════════════════════════════════════════════════

class MapGenVillage : public MapGenStructure {
public:
    int32_t spacing = 32;
    int32_t separation = 8;

    std::string getStructureName() const override { return "Village"; }

    bool canSpawnStructureAtCoords(int32_t chunkX, int32_t chunkZ, RNG& rng) override {
        int32_t gridX = chunkX;
        int32_t gridZ = chunkZ;

        if (chunkX < 0) gridX = chunkX - spacing + 1;
        if (chunkZ < 0) gridZ = chunkZ - spacing + 1;

        gridX /= spacing;
        gridZ /= spacing;

        // Per-grid-cell seed
        RNG gridRng;
        gridRng.setSeed(static_cast<int64_t>(gridX) * 341873128712LL +
                          static_cast<int64_t>(gridZ) * 132897987541LL + 10387312LL);

        gridX = gridX * spacing + gridRng.nextInt(spacing - separation);
        gridZ = gridZ * spacing + gridRng.nextInt(spacing - separation);

        return chunkX == gridX && chunkZ == gridZ;
    }

    StructureStart getStructureStart(int32_t chunkX, int32_t chunkZ, RNG& rng) override {
        StructureStart start;
        start.chunkPosX = chunkX;
        start.chunkPosZ = chunkZ;
        // Village well at center
        StructureComponent well;
        well.boundingBox = {chunkX * 16, 64, chunkZ * 16,
                            chunkX * 16 + 9, 78, chunkZ * 16 + 9};
        start.components.push_back(well);
        start.updateBoundingBox();
        return start;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// MapGenMineshaft — Mineshaft generation.
// Java reference: net.minecraft.world.gen.structure.MapGenMineshaft
// 1% chance per chunk
// ═══════════════════════════════════════════════════════════════════════════

class MapGenMineshaft : public MapGenStructure {
public:
    double chance = 0.01;

    std::string getStructureName() const override { return "Mineshaft"; }

    bool canSpawnStructureAtCoords(int32_t chunkX, int32_t chunkZ, RNG& rng) override {
        // Java: nextDouble() < chance
        // Simplified: nextInt(100) == 0
        return rng.nextInt(100) == 0;
    }

    StructureStart getStructureStart(int32_t chunkX, int32_t chunkZ, RNG& rng) override {
        StructureStart start;
        start.chunkPosX = chunkX;
        start.chunkPosZ = chunkZ;
        // Main corridor
        int32_t y = rng.nextInt(40) + 10;
        StructureComponent corridor;
        corridor.boundingBox = {chunkX * 16, y, chunkZ * 16,
                                chunkX * 16 + 15, y + 4, chunkZ * 16 + 15};
        corridor.coordBaseMode = rng.nextInt(4);
        start.components.push_back(corridor);
        start.updateBoundingBox();
        return start;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// MapGenStronghold — Stronghold generation.
// Java reference: net.minecraft.world.gen.structure.MapGenStronghold
// 3 strongholds per world in first ring (1408-2688 blocks from origin)
// ═══════════════════════════════════════════════════════════════════════════

class MapGenStronghold : public MapGenStructure {
public:
    static constexpr int32_t NUM_STRONGHOLDS = 3;

    struct StrongholdPos {
        int32_t chunkX, chunkZ;
    };

    std::vector<StrongholdPos> positions;
    bool positionsGenerated = false;

    std::string getStructureName() const override { return "Stronghold"; }

    void generatePositions(int64_t worldSeed) {
        if (positionsGenerated) return;
        positionsGenerated = true;

        RNG rng;
        rng.setSeed(worldSeed);

        double angle = rng.nextInt(360);  // Simplified from nextDouble() * PI * 2
        for (int32_t i = 0; i < NUM_STRONGHOLDS; ++i) {
            double dist = (1408.0 + rng.nextInt(1280)) / 16.0;  // In chunks
            double rad = angle * 3.14159265358979 / 180.0;
            int32_t cx = static_cast<int32_t>(std::round(std::cos(rad) * dist));
            int32_t cz = static_cast<int32_t>(std::round(std::sin(rad) * dist));
            positions.push_back({cx, cz});
            angle += 120.0;  // 360/3 = 120 degrees apart
        }
    }

    bool canSpawnStructureAtCoords(int32_t chunkX, int32_t chunkZ, RNG& rng) override {
        for (auto& pos : positions) {
            if (pos.chunkX == chunkX && pos.chunkZ == chunkZ) return true;
        }
        return false;
    }

    StructureStart getStructureStart(int32_t chunkX, int32_t chunkZ, RNG& rng) override {
        StructureStart start;
        start.chunkPosX = chunkX;
        start.chunkPosZ = chunkZ;
        StructureComponent portal;
        portal.boundingBox = {chunkX * 16, 20, chunkZ * 16,
                              chunkX * 16 + 11, 32, chunkZ * 16 + 11};
        start.components.push_back(portal);
        start.updateBoundingBox();
        return start;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// MapGenScatteredFeature — Temple/witch hut generation.
// Java reference: net.minecraft.world.gen.structure.MapGenScatteredFeature
// Spacing: 32, distance: 8 (same grid as villages but different seed)
// Biome-dependent: desert temple, jungle temple, swamp hut
// ═══════════════════════════════════════════════════════════════════════════

class MapGenScatteredFeature : public MapGenStructure {
public:
    int32_t maxDistance = 32;
    int32_t minDistance = 8;

    std::string getStructureName() const override { return "Temple"; }

    bool canSpawnStructureAtCoords(int32_t chunkX, int32_t chunkZ, RNG& rng) override {
        int32_t gridX = chunkX;
        int32_t gridZ = chunkZ;

        if (chunkX < 0) gridX = chunkX - maxDistance + 1;
        if (chunkZ < 0) gridZ = chunkZ - maxDistance + 1;

        gridX /= maxDistance;
        gridZ /= maxDistance;

        RNG gridRng;
        gridRng.setSeed(static_cast<int64_t>(gridX) * 341873128712LL +
                          static_cast<int64_t>(gridZ) * 132897987541LL + 14357617LL);

        gridX = gridX * maxDistance + gridRng.nextInt(maxDistance - minDistance);
        gridZ = gridZ * maxDistance + gridRng.nextInt(maxDistance - minDistance);

        return chunkX == gridX && chunkZ == gridZ;
    }

    StructureStart getStructureStart(int32_t chunkX, int32_t chunkZ, RNG& rng) override {
        StructureStart start;
        start.chunkPosX = chunkX;
        start.chunkPosZ = chunkZ;
        StructureComponent temple;
        temple.boundingBox = {chunkX * 16, 64, chunkZ * 16,
                              chunkX * 16 + 21, 78, chunkZ * 16 + 21};
        start.components.push_back(temple);
        start.updateBoundingBox();
        return start;
    }

    // Java: func_143030_a — check if a position is inside a scattered feature
    bool isInsideFeature(int32_t x, int32_t y, int32_t z) {
        return hasStructureAt(x, y, z);
    }

    // Java: getScatteredFeatureSpawnList — witch hut spawns
    struct SpawnEntry {
        std::string entityName;
        int32_t weight, min, max;
    };

    static std::vector<SpawnEntry> getScatteredFeatureSpawnList() {
        return {{"Witch", 1, 1, 1}};
    }
};

} // namespace mccpp
