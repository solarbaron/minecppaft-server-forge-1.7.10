/**
 * MapGenVillage.h - Village structure generation.
 *
 * Java reference: net.minecraft.world.gen.structure.MapGenVillage
 *               + StructureVillagePieces (all inner classes)
 *
 * Generates villages with 13 piece types:
 *   Well (center), Path, Torch, WoodHut, Church, House1 (library),
 *   House2 (blacksmith), House3, House4Garden, Hall (butcher),
 *   Field1, Field2
 *
 * Grid-based spawning: maxDist=32, minDist=8, salt=10387312.
 * Recursive piece assembly from well center via paths.
 * Desert variant substitutes sandstone for cobblestone/wood.
 *
 * Thread safety: Each OverworldGenerator instance owns its own MapGenVillage.
 */
#pragma once

#include "world/Chunk.h"
#include "worldgen/LootTables.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <random>
#include <vector>

namespace mccpp {

class MapGenVillage {
public:
    using BiomeQueryFn = std::function<int(int, int)>;

    explicit MapGenVillage(int64_t seed, BiomeQueryFn biomeQuery = nullptr)
        : worldSeed_(seed), biomeQuery_(std::move(biomeQuery)) {}

    void setBiomeQuery(BiomeQueryFn fn) { biomeQuery_ = std::move(fn); }

    void generate(int64_t seed, int chunkX, int chunkZ,
                  int32_t* blocks, uint8_t* meta,
                  std::vector<Chunk::SpawnerInfo>& spawners,
                  std::vector<Chunk::ChestInfo>& chests) {
        int worldMinX = chunkX * 16;
        int worldMinZ = chunkZ * 16;

        // Check 4-chunk radius for village centers
        for (int cx = chunkX - 4; cx <= chunkX + 4; ++cx) {
            for (int cz = chunkZ - 4; cz <= chunkZ + 4; ++cz) {
                if (!isVillageChunk(cx, cz)) continue;

                JavaRNG rng(getVillageSeed(cx, cz));
                std::vector<VillagePiece> pieces;
                bool isDesert = (getBiome(cx * 16 + 8, cz * 16 + 8) == 2);
                generateVillage(cx, cz, rng, pieces, isDesert);

                for (auto& piece : pieces) {
                    if (piece.bb.maxX < worldMinX || piece.bb.minX > worldMinX + 15) continue;
                    if (piece.bb.maxZ < worldMinZ || piece.bb.minZ > worldMinZ + 15) continue;
                    placePiece(piece, worldMinX, worldMinZ, blocks, meta, chests, isDesert);
                }
            }
        }
    }

private:
    int64_t worldSeed_;
    BiomeQueryFn biomeQuery_;

    // Block IDs
    static constexpr int32_t AIR = 0;
    static constexpr int32_t STONE = 1;
    static constexpr int32_t GRASS_BLOCK = 2;
    static constexpr int32_t DIRT = 3;
    static constexpr int32_t COBBLESTONE = 4;
    static constexpr int32_t OAK_PLANKS = 5;
    static constexpr int32_t OAK_LOG = 17;
    static constexpr int32_t GLASS_PANE = 102;
    static constexpr int32_t FENCE = 85;
    static constexpr int32_t TORCH = 50;
    static constexpr int32_t WOODEN_DOOR = 64;
    static constexpr int32_t LADDER = 65;
    static constexpr int32_t GRAVEL = 13;
    static constexpr int32_t SANDSTONE = 24;
    static constexpr int32_t OAK_STAIRS = 53;
    static constexpr int32_t STONE_STAIRS = 67;
    static constexpr int32_t COBBLESTONE_STAIRS = 67;
    static constexpr int32_t STONE_SLAB = 44;
    static constexpr int32_t DOUBLE_STONE_SLAB = 43;
    static constexpr int32_t IRON_BARS = 101;
    static constexpr int32_t FURNACE = 61;
    static constexpr int32_t LAVA = 11;
    static constexpr int32_t CHEST = 54;
    static constexpr int32_t CRAFTING_TABLE = 58;
    static constexpr int32_t BOOKSHELF = 47;
    static constexpr int32_t WOOL = 35;
    static constexpr int32_t WATER = 9;
    static constexpr int32_t FARMLAND = 60;
    static constexpr int32_t WHEAT = 59;
    static constexpr int32_t CARROTS = 141;
    static constexpr int32_t POTATOES = 142;
    static constexpr int32_t WOODEN_PRESSURE_PLATE = 72;
    static constexpr int32_t SAND = 12;
    static constexpr int32_t SANDSTONE_STAIRS = 128;
    static constexpr int32_t LOG2 = 162;

    // Village generation parameters (Java: MapGenVillage)
    static constexpr int MAX_DIST = 32;
    static constexpr int MIN_DIST = 8;
    static constexpr int64_t VILLAGE_SALT = 10387312LL;

    // ──── Java-like RNG ────
    struct JavaRNG {
        int64_t seed;
        JavaRNG() : seed(0) {}
        explicit JavaRNG(int64_t s) { setSeed(s); }
        void setSeed(int64_t s) {
            seed = (s ^ 0x5DEECE66DLL) & ((1LL << 48) - 1);
        }
        int32_t next(int bits) {
            seed = (seed * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1);
            return static_cast<int32_t>(seed >> (48 - bits));
        }
        int32_t nextInt(int bound) {
            if (bound <= 0) return 0;
            if ((bound & (bound - 1)) == 0)
                return static_cast<int32_t>((static_cast<int64_t>(bound) * static_cast<int64_t>(next(31))) >> 31);
            int32_t bits, val;
            do { bits = next(31); val = bits % bound; } while (bits - val + (bound - 1) < 0);
            return val;
        }
        bool nextBoolean() { return next(1) != 0; }
        float nextFloat() { return next(24) / static_cast<float>(1 << 24); }
        int32_t nextIntRange(int min, int max) {
            return min + nextInt(max - min + 1);
        }
    };

    // ──── Village chunk detection (Java: MapGenStructure.canSpawnStructureAtCoords) ────
    bool isVillageChunk(int chunkX, int chunkZ) const {
        int cx = chunkX;
        int cz = chunkZ;
        if (cx < 0) cx -= MAX_DIST - 1;
        if (cz < 0) cz -= MAX_DIST - 1;
        int rx = cx / MAX_DIST;
        int rz = cz / MAX_DIST;
        JavaRNG rng(static_cast<int64_t>(rx) * 341873128712LL +
                    static_cast<int64_t>(rz) * 132897987541LL + worldSeed_ + VILLAGE_SALT);
        int offX = rng.nextInt(MAX_DIST - MIN_DIST);
        int offZ = rng.nextInt(MAX_DIST - MIN_DIST);
        if (chunkX == rx * MAX_DIST + offX && chunkZ == rz * MAX_DIST + offZ) {
            // Biome check: plains(1), desert(2), savanna(35)
            int biome = getBiome(chunkX * 16 + 8, chunkZ * 16 + 8);
            return biome == 1 || biome == 2 || biome == 35;
        }
        return false;
    }

    int64_t getVillageSeed(int cx, int cz) const {
        return worldSeed_ + static_cast<int64_t>(cx) * 341873128712LL
                          + static_cast<int64_t>(cz) * 132897987541LL + VILLAGE_SALT;
    }

    int getBiome(int x, int z) const {
        if (biomeQuery_) return biomeQuery_(x, z);
        return 1; // Default plains
    }

    // ──── Bounding Box ────
    struct BBox {
        int minX, minY, minZ, maxX, maxY, maxZ;
        BBox() : minX(0), minY(0), minZ(0), maxX(0), maxY(0), maxZ(0) {}
        BBox(int x1, int y1, int z1, int x2, int y2, int z2)
            : minX(x1), minY(y1), minZ(z1), maxX(x2), maxY(y2), maxZ(z2) {}
        bool intersects(const BBox& o) const {
            return maxX >= o.minX && minX <= o.maxX &&
                   maxZ >= o.minZ && minZ <= o.maxZ &&
                   maxY >= o.minY && minY <= o.maxY;
        }
        void offset(int dx, int dy, int dz) {
            minX += dx; maxX += dx;
            minY += dy; maxY += dy;
            minZ += dz; maxZ += dz;
        }
        int xSize() const { return maxX - minX + 1; }
        int ySize() const { return maxY - minY + 1; }
        int zSize() const { return maxZ - minZ + 1; }
    };

    static BBox getComponentBB(int x, int y, int z, int offX, int offY, int offZ,
                                int sizeX, int sizeY, int sizeZ, int coordBaseMode) {
        switch (coordBaseMode) {
            case 0: return BBox(x + offX, y + offY, z + offZ,
                                x + offX + sizeX - 1, y + offY + sizeY - 1, z + offZ + sizeZ - 1);
            case 1: return BBox(x - sizeZ + 1 + offZ, y + offY, z + offX,
                                x + offZ, y + offY + sizeY - 1, z + offX + sizeX - 1);
            case 2: return BBox(x + offX, y + offY, z - sizeZ + 1 + offZ,
                                x + offX + sizeX - 1, y + offY + sizeY - 1, z + offZ);
            case 3: return BBox(x + offZ, y + offY, z + offX,
                                x + offZ + sizeZ - 1, y + offY + sizeY - 1, z + offX + sizeX - 1);
        }
        return BBox(x + offX, y + offY, z + offZ,
                    x + offX + sizeX - 1, y + offY + sizeY - 1, z + offZ + sizeZ - 1);
    }

    static bool canVillageGoDeeper(const BBox& bb) {
        return bb.minY > 10;
    }

    // ──── Coordinate transforms ────
    struct StructureCtx {
        BBox bb;
        int coordBaseMode;

        int getXWithOffset(int n, int n2) const {
            switch (coordBaseMode) {
                case 0: case 2: return bb.minX + n;
                case 1: return bb.maxX - n2;
                case 3: return bb.minX + n2;
            }
            return n;
        }
        int getYWithOffset(int n) const {
            return (coordBaseMode == -1) ? n : n + bb.minY;
        }
        int getZWithOffset(int n, int n2) const {
            switch (coordBaseMode) {
                case 2: return bb.maxZ - n2;
                case 0: return bb.minZ + n2;
                case 1: case 3: return bb.minZ + n;
            }
            return n2;
        }
    };

    // ──── Block helpers ────
    static inline void setBlockAt(int32_t* blocks, uint8_t* meta,
                                  int worldMinX, int worldMinZ,
                                  int wx, int wy, int wz,
                                  int32_t blockId, uint8_t blockMeta = 0) {
        int lx = wx - worldMinX;
        int lz = wz - worldMinZ;
        if (lx < 0 || lx > 15 || lz < 0 || lz > 15 || wy < 1 || wy > 254) return;
        int idx = (lx * 16 + lz) * 256 + wy;
        blocks[idx] = blockId;
        meta[idx] = blockMeta;
    }

    static inline int32_t getBlockAt(const int32_t* blocks,
                                     int worldMinX, int worldMinZ,
                                     int wx, int wy, int wz) {
        int lx = wx - worldMinX;
        int lz = wz - worldMinZ;
        if (lx < 0 || lx > 15 || lz < 0 || lz > 15 || wy < 0 || wy > 255) return STONE;
        return blocks[(lx * 16 + lz) * 256 + wy];
    }

    // ──── Metadata rotation ────
    static int getMetadataWithOffset(int32_t blockId, int meta, int coordBaseMode) {
        // Stairs
        if (blockId == OAK_STAIRS || blockId == STONE_STAIRS || blockId == SANDSTONE_STAIRS) {
            if (coordBaseMode == 0) {
                if (meta == 2) return 3;
                if (meta == 3) return 2;
            } else if (coordBaseMode == 1) {
                if (meta == 0) return 2; if (meta == 1) return 3;
                if (meta == 2) return 0; if (meta == 3) return 1;
            } else if (coordBaseMode == 3) {
                if (meta == 0) return 2; if (meta == 1) return 3;
                if (meta == 2) return 1; if (meta == 3) return 0;
            }
        }
        // Wooden door
        else if (blockId == WOODEN_DOOR) {
            if (coordBaseMode == 0) {
                if (meta == 0 || meta == 2) { return meta ^ 2; }
            } else if (coordBaseMode == 1) {
                return (meta + 1) & 3;
            } else if (coordBaseMode == 3) {
                return (meta + 3) & 3;
            }
        }
        // Ladder
        else if (blockId == LADDER) {
            if (coordBaseMode == 0) {
                if (meta == 2) return 3; if (meta == 3) return 2;
            } else if (coordBaseMode == 1) {
                if (meta == 2) return 4; if (meta == 3) return 5;
                if (meta == 4) return 2; if (meta == 5) return 3;
            } else if (coordBaseMode == 3) {
                if (meta == 2) return 5; if (meta == 3) return 4;
                if (meta == 4) return 2; if (meta == 5) return 3;
            }
        }
        // Torch
        else if (blockId == TORCH) {
            if (coordBaseMode == 0) {
                if (meta == 3) return 4; if (meta == 4) return 3;
            } else if (coordBaseMode == 1) {
                if (meta == 1) return 3; if (meta == 2) return 4;
                if (meta == 3) return 1; if (meta == 4) return 2;
            } else if (coordBaseMode == 3) {
                if (meta == 1) return 4; if (meta == 2) return 3;
                if (meta == 3) return 1; if (meta == 4) return 2;
            }
        }
        // Furnace (block 61) - same rotation as chest
        else if (blockId == FURNACE) {
            if (coordBaseMode == 0) {
                if (meta == 2) return 3; if (meta == 3) return 2;
            } else if (coordBaseMode == 1) {
                if (meta == 2) return 4; if (meta == 3) return 5;
                if (meta == 4) return 2; if (meta == 5) return 3;
            } else if (coordBaseMode == 3) {
                if (meta == 2) return 5; if (meta == 3) return 4;
                if (meta == 4) return 2; if (meta == 5) return 3;
            }
        }
        return meta;
    }

    // ──── High-level placement helpers ────
    void placeBlock(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                    int worldMinX, int worldMinZ,
                    int32_t blockId, uint8_t blockMeta, int lx, int ly, int lz) {
        int wx = ctx.getXWithOffset(lx, lz);
        int wy = ctx.getYWithOffset(ly);
        int wz = ctx.getZWithOffset(lx, lz);
        setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, blockId, blockMeta);
    }

    void fillWithBlocks(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                        int worldMinX, int worldMinZ,
                        int x1, int y1, int z1, int x2, int y2, int z2,
                        int32_t outer, int32_t inner, bool skipAir) {
        for (int y = y1; y <= y2; ++y)
            for (int x = x1; x <= x2; ++x)
                for (int z = z1; z <= z2; ++z) {
                    if (skipAir) {
                        int wx = ctx.getXWithOffset(x, z);
                        int wy = ctx.getYWithOffset(y);
                        int wz = ctx.getZWithOffset(x, z);
                        if (getBlockAt(blocks, worldMinX, worldMinZ, wx, wy, wz) == AIR) continue;
                    }
                    bool isBorder = (y == y1 || y == y2 || x == x1 || x == x2 || z == z1 || z == z2);
                    placeBlock(ctx, blocks, meta, worldMinX, worldMinZ,
                               isBorder ? outer : inner, 0, x, y, z);
                }
    }

    void fillDownwards(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                       int worldMinX, int worldMinZ,
                       int32_t blockId, uint8_t blockMeta, int lx, int ly, int lz) {
        int wx = ctx.getXWithOffset(lx, lz);
        int wy = ctx.getYWithOffset(ly);
        int wz = ctx.getZWithOffset(lx, lz);
        int32_t cur = getBlockAt(blocks, worldMinX, worldMinZ, wx, wy, wz);
        while ((cur == AIR || cur == WATER || cur == LAVA || cur == 8 || cur == 10) && wy > 1) {
            setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, blockId, blockMeta);
            --wy;
            cur = getBlockAt(blocks, worldMinX, worldMinZ, wx, wy, wz);
        }
    }

    void clearUpwards(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                      int worldMinX, int worldMinZ,
                      int lx, int ly, int lz) {
        int wx = ctx.getXWithOffset(lx, lz);
        int wy = ctx.getYWithOffset(ly);
        int wz = ctx.getZWithOffset(lx, lz);
        while (wy < 255) {
            int32_t cur = getBlockAt(blocks, worldMinX, worldMinZ, wx, wy, wz);
            if (cur == AIR) break;
            setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, AIR, 0);
            ++wy;
        }
    }

    void placeDoor(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                   int worldMinX, int worldMinZ, JavaRNG& /*rng*/,
                   int lx, int ly, int lz, uint8_t doorMeta) {
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOODEN_DOOR, doorMeta, lx, ly, lz);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOODEN_DOOR, 8, lx, ly + 1, lz);
    }

    // Desert block substitution (Java: StructureVillagePieces$Village.func_151558_b)
    int32_t getBiomeBlock(int32_t blockId, bool isDesert) const {
        if (!isDesert) return blockId;
        if (blockId == OAK_LOG || blockId == COBBLESTONE) return SANDSTONE;
        if (blockId == OAK_PLANKS) return SANDSTONE;
        if (blockId == OAK_STAIRS) return SANDSTONE_STAIRS;
        if (blockId == STONE_STAIRS) return SANDSTONE_STAIRS;
        if (blockId == GRAVEL) return SANDSTONE;
        return blockId;
    }

    uint8_t getBiomeMeta(int32_t blockId, uint8_t blockMeta, bool isDesert) const {
        if (!isDesert) return blockMeta;
        if (blockId == OAK_LOG || blockId == COBBLESTONE || blockId == OAK_PLANKS || blockId == GRAVEL) return 0;
        return blockMeta;
    }

    // Biome-aware placement
    void placeBlockBiome(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                         int worldMinX, int worldMinZ, bool isDesert,
                         int32_t blockId, uint8_t blockMeta, int lx, int ly, int lz) {
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ,
                   getBiomeBlock(blockId, isDesert),
                   getBiomeMeta(blockId, blockMeta, isDesert), lx, ly, lz);
    }

    void fillWithBlocksBiome(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                             int worldMinX, int worldMinZ, bool isDesert,
                             int x1, int y1, int z1, int x2, int y2, int z2,
                             int32_t outer, int32_t inner, bool skipAir) {
        for (int y = y1; y <= y2; ++y)
            for (int x = x1; x <= x2; ++x)
                for (int z = z1; z <= z2; ++z) {
                    if (skipAir) {
                        int wx = ctx.getXWithOffset(x, z);
                        int wy = ctx.getYWithOffset(y);
                        int wz = ctx.getZWithOffset(x, z);
                        if (getBlockAt(blocks, worldMinX, worldMinZ, wx, wy, wz) == AIR) continue;
                    }
                    bool isBorder = (y == y1 || y == y2 || x == x1 || x == x2 || z == z1 || z == z2);
                    int32_t bid = isBorder ? outer : inner;
                    placeBlock(ctx, blocks, meta, worldMinX, worldMinZ,
                               getBiomeBlock(bid, isDesert), getBiomeMeta(bid, 0, isDesert), x, y, z);
                }
    }

    void fillDownwardsBiome(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                            int worldMinX, int worldMinZ, bool isDesert,
                            int32_t blockId, uint8_t blockMeta, int lx, int ly, int lz) {
        fillDownwards(ctx, blocks, meta, worldMinX, worldMinZ,
                      getBiomeBlock(blockId, isDesert), getBiomeMeta(blockId, blockMeta, isDesert), lx, ly, lz);
    }

    // ──── Piece types ────
    enum class PieceType {
        Well, Path, Torch, WoodHut, Church, House1, House2, House3,
        House4Garden, Hall, Field1, Field2
    };

    struct VillagePiece {
        PieceType type;
        BBox bb;
        int coordBaseMode;
        int componentType; // depth
        // Type-specific
        bool isTall = false;       // WoodHut
        bool isRoofAccessible = false; // House4Garden
        bool hasMadeChest = false; // House2
        int cropA = WHEAT, cropB = WHEAT, cropC = WHEAT, cropD = WHEAT; // Fields
        int pathLength = 0;        // Path
    };

    struct PieceWeight {
        PieceType type;
        int weight;
        int spawned = 0;
        int limit;
        PieceWeight(PieceType t, int w, int l) : type(t), weight(w), limit(l) {}
        bool canSpawnMore() const { return limit == 0 || spawned < limit; }
    };

    static bool findIntersecting(const std::vector<VillagePiece>& pieces, const BBox& bb) {
        for (auto& p : pieces) {
            if (p.bb.intersects(bb)) return true;
        }
        return false;
    }

    // Random crop selection (Java: func_151559_a / func_151560_a)
    static int32_t getRandomCrop(JavaRNG& rng) {
        switch (rng.nextInt(5)) {
            case 0: return CARROTS;
            case 1: return POTATOES;
            default: return WHEAT;
        }
    }

    // ──── Get next component positions (Java: Village.getNextComponentNN/PX/PZ) ────
    void getNextComponentNormal(const VillagePiece& from, int offX, int offZ,
                                int& outX, int& outY, int& outZ, int& outFacing) {
        outFacing = from.coordBaseMode;
        switch (from.coordBaseMode) {
            case 0: outX = from.bb.minX + offX; outY = from.bb.minY + offZ - 1; outZ = from.bb.maxZ + 1; break;
            case 1: outX = from.bb.minX - 1; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ + offX; break;
            case 2: outX = from.bb.minX + offX; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ - 1; break;
            case 3: outX = from.bb.maxX + 1; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ + offX; break;
            default: outX = outY = outZ = 0; break;
        }
    }

    void getNextComponentX(const VillagePiece& from, int offX, int offZ,
                           int& outX, int& outY, int& outZ, int& outFacing) {
        switch (from.coordBaseMode) {
            case 0: outFacing = 1; outX = from.bb.minX - 1; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ + offX; break;
            case 1: outFacing = 2; outX = from.bb.minX + offX; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ - 1; break;
            case 2: outFacing = 1; outX = from.bb.minX - 1; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ + offX; break;
            case 3: outFacing = 0; outX = from.bb.minX + offX; outY = from.bb.minY + offZ - 1; outZ = from.bb.maxZ + 1; break;
            default: outFacing = 0; outX = outY = outZ = 0; break;
        }
    }

    void getNextComponentZ(const VillagePiece& from, int offX, int offZ,
                           int& outX, int& outY, int& outZ, int& outFacing) {
        switch (from.coordBaseMode) {
            case 0: outFacing = 3; outX = from.bb.maxX + 1; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ + offX; break;
            case 1: outFacing = 0; outX = from.bb.minX + offX; outY = from.bb.minY + offZ - 1; outZ = from.bb.maxZ + 1; break;
            case 2: outFacing = 3; outX = from.bb.maxX + 1; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ + offX; break;
            case 3: outFacing = 2; outX = from.bb.minX + offX; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ - 1; break;
            default: outFacing = 0; outX = outY = outZ = 0; break;
        }
    }

    // ──── Try to create a piece at given position ────
    bool tryCreatePiece(PieceType type, int depth, JavaRNG& rng,
                        int x, int y, int z, int facing,
                        std::vector<VillagePiece>& pieces,
                        VillagePiece& out) {
        BBox bb;
        out = VillagePiece{};
        out.type = type;
        out.coordBaseMode = facing;
        out.componentType = depth;

        switch (type) {
            case PieceType::Well:
                bb = getComponentBB(x, y, z, 0, 0, 0, 6, 11, 6, facing); break;
            case PieceType::Path:
                bb = getComponentBB(x, y, z, 0, 0, 0, 3, 3, 2 + rng.nextInt(10), facing);
                out.pathLength = bb.zSize() - 1;
                break;
            case PieceType::Torch:
                bb = getComponentBB(x, y, z, 0, 0, 0, 3, 4, 2, facing); break;
            case PieceType::WoodHut:
                bb = getComponentBB(x, y, z, 0, 0, 0, 4, 6, 5, facing);
                out.isTall = rng.nextBoolean();
                break;
            case PieceType::Church:
                bb = getComponentBB(x, y, z, 0, 0, 0, 5, 12, 9, facing); break;
            case PieceType::House1:
                bb = getComponentBB(x, y, z, 0, 0, 0, 9, 9, 6, facing); break;
            case PieceType::House2:
                bb = getComponentBB(x, y, z, 0, 0, 0, 10, 6, 7, facing); break;
            case PieceType::House3:
                bb = getComponentBB(x, y, z, 0, 0, 0, 9, 7, 12, facing); break;
            case PieceType::House4Garden:
                bb = getComponentBB(x, y, z, 0, 0, 0, 5, 6, 5, facing);
                out.isRoofAccessible = rng.nextBoolean();
                break;
            case PieceType::Hall:
                bb = getComponentBB(x, y, z, 0, 0, 0, 9, 7, 11, facing); break;
            case PieceType::Field1:
                bb = getComponentBB(x, y, z, 0, 0, 0, 13, 4, 9, facing);
                out.cropA = getRandomCrop(rng); out.cropB = getRandomCrop(rng);
                out.cropC = getRandomCrop(rng); out.cropD = getRandomCrop(rng);
                break;
            case PieceType::Field2:
                bb = getComponentBB(x, y, z, 0, 0, 0, 7, 4, 9, facing);
                out.cropA = getRandomCrop(rng); out.cropB = getRandomCrop(rng);
                break;
        }

        // canVillageGoDeeper check (most pieces check this)
        if (type != PieceType::Torch && type != PieceType::House4Garden) {
            if (!canVillageGoDeeper(bb)) return false;
        }
        if (findIntersecting(pieces, bb)) return false;

        out.bb = bb;
        return true;
    }

    // ──── Village generation (Java: MapGenVillage$Start) ────
    void generateVillage(int startCX, int startCZ, JavaRNG& rng,
                         std::vector<VillagePiece>& pieces, bool isDesert) {
        // Piece weights (Java: StructureVillagePieces.getStructureVillageWeightedPieceList)
        std::vector<PieceWeight> weights = {
            {PieceType::House4Garden, 4, 2},
            {PieceType::Church, 20, 1},
            {PieceType::House1, 20, 3},
            {PieceType::WoodHut, 3, 5},
            {PieceType::Hall, 15, 2},
            {PieceType::Field1, 3, 3},
            {PieceType::Field2, 3, 3},
            {PieceType::House2, 15, 1},
            {PieceType::House3, 8, 3},
        };

        // Start piece: Well at center
        int startX = startCX * 16 + 8;
        int startZ = startCZ * 16 + 8;
        int startY = 64;
        int facing = rng.nextInt(4);

        VillagePiece well;
        well.type = PieceType::Well;
        well.coordBaseMode = facing;
        well.componentType = 0;
        well.bb = getComponentBB(startX, startY, startZ, 0, 0, 0, 6, 11, 6, facing);
        pieces.push_back(well);

        // Generate paths from well, then buildings from paths
        std::vector<int> pending;
        pending.push_back(0);

        int maxPieces = 25 + rng.nextInt(10);
        int maxDepth = 10;

        // First add paths from well exits
        for (int dir = 0; dir < 4; ++dir) {
            int ox, oy, oz, of;
            switch (dir) {
                case 0: getNextComponentNormal(well, 1, 1, ox, oy, oz, of); break;
                case 1: getNextComponentX(well, 1, 1, ox, oy, oz, of); break;
                case 2: getNextComponentZ(well, 1, 1, ox, oy, oz, of); break;
                case 3: // opposite of normal
                    of = (well.coordBaseMode + 2) & 3;
                    ox = well.bb.minX + 1; oy = well.bb.minY; oz = well.bb.minZ - 1;
                    break;
            }
            VillagePiece path;
            if (tryCreatePiece(PieceType::Path, 1, rng, ox, oy, oz, of, pieces, path)) {
                int idx = static_cast<int>(pieces.size());
                pieces.push_back(path);
                pending.push_back(idx);
            }
        }

        while (!pending.empty() && pieces.size() < static_cast<size_t>(maxPieces)) {
            int idx = rng.nextInt(static_cast<int>(pending.size()));
            int pieceIdx = pending[idx];
            pending.erase(pending.begin() + idx);

            auto& piece = pieces[pieceIdx];
            int depth = piece.componentType + 1;
            if (depth > maxDepth) continue;

            if (piece.type == PieceType::Path) {
                // Paths branch: forward + left/right
                int ox, oy, oz, of;
                // Forward
                getNextComponentNormal(piece, 1, 1, ox, oy, oz, of);
                if (rng.nextInt(3) != 0) { // 2/3 chance continue path
                    VillagePiece newPath;
                    if (tryCreatePiece(PieceType::Path, depth, rng, ox, oy, oz, of, pieces, newPath)) {
                        int ni = static_cast<int>(pieces.size());
                        pieces.push_back(newPath);
                        pending.push_back(ni);
                    }
                }
                // Left
                if (rng.nextInt(3) != 0) {
                    getNextComponentX(piece, 0, 1, ox, oy, oz, of);
                    VillagePiece leftPath;
                    if (tryCreatePiece(PieceType::Path, depth, rng, ox, oy, oz, of, pieces, leftPath)) {
                        int ni = static_cast<int>(pieces.size());
                        pieces.push_back(leftPath);
                        pending.push_back(ni);
                    }
                }
                // Right
                if (rng.nextInt(3) != 0) {
                    getNextComponentZ(piece, 0, 1, ox, oy, oz, of);
                    VillagePiece rightPath;
                    if (tryCreatePiece(PieceType::Path, depth, rng, ox, oy, oz, of, pieces, rightPath)) {
                        int ni = static_cast<int>(pieces.size());
                        pieces.push_back(rightPath);
                        pending.push_back(ni);
                    }
                }
                // Place buildings along path
                for (int side = 0; side < 2; ++side) {
                    if (rng.nextInt(3) == 0) continue; // 1/3 skip
                    // Weighted selection
                    PieceType selectedType = PieceType::Torch;
                    int totalWeight = 0;
                    int selectedIdx = -1;
                    for (size_t i = 0; i < weights.size(); ++i) {
                        if (!weights[i].canSpawnMore()) continue;
                        totalWeight += weights[i].weight;
                    }
                    if (totalWeight > 0) {
                        int r = rng.nextInt(totalWeight);
                        int acc = 0;
                        for (size_t i = 0; i < weights.size(); ++i) {
                            if (!weights[i].canSpawnMore()) continue;
                            acc += weights[i].weight;
                            if (r < acc) {
                                selectedType = weights[i].type;
                                selectedIdx = static_cast<int>(i);
                                break;
                            }
                        }
                    }
                    int bx, by, bz, bf;
                    if (side == 0) {
                        getNextComponentX(piece, 0, 1, bx, by, bz, bf);
                    } else {
                        getNextComponentZ(piece, 0, 1, bx, by, bz, bf);
                    }
                    VillagePiece building;
                    if (tryCreatePiece(selectedType, depth, rng, bx, by, bz, bf, pieces, building)) {
                        if (selectedIdx >= 0) weights[selectedIdx].spawned++;
                        pieces.push_back(building);
                    } else {
                        // Fallback: torch
                        VillagePiece torch;
                        if (tryCreatePiece(PieceType::Torch, depth, rng, bx, by, bz, bf, pieces, torch)) {
                            pieces.push_back(torch);
                        }
                    }
                }
            }
        }

        // Adjust Y positions based on surface (simplified: keep at Y=64-based)
        // In practice the Java code uses getAverageGroundLevel per-piece during placement
    }

    // ──── Average ground level (Java: Village.getAverageGroundLevel) ────
    int getAverageGroundLevel(const BBox& bb, const int32_t* blocks, int worldMinX, int worldMinZ) {
        int total = 0, count = 0;
        for (int x = bb.minX; x <= bb.maxX; ++x) {
            for (int z = bb.minZ; z <= bb.maxZ; ++z) {
                int lx = x - worldMinX;
                int lz = z - worldMinZ;
                if (lx < 0 || lx > 15 || lz < 0 || lz > 15) continue;
                for (int y = 80; y >= 1; --y) {
                    int32_t bid = blocks[(lx * 16 + lz) * 256 + y];
                    if (bid != AIR && bid != 18 && bid != 161 && bid != 31) {
                        total += y + 1;
                        ++count;
                        break;
                    }
                }
            }
        }
        return count > 0 ? total / count : -1;
    }

    // ──── Master piece placement ────
    void placePiece(VillagePiece& piece, int worldMinX, int worldMinZ,
                    int32_t* blocks, uint8_t* meta,
                    std::vector<Chunk::ChestInfo>& chests, bool isDesert) {
        StructureCtx ctx{piece.bb, piece.coordBaseMode};
        int CBM = piece.coordBaseMode;

        // Get average ground level and adjust Y
        int avgY = getAverageGroundLevel(piece.bb, blocks, worldMinX, worldMinZ);
        if (avgY < 0) return;

        switch (piece.type) {
            case PieceType::Well:
                placeWell(ctx, blocks, meta, worldMinX, worldMinZ, isDesert, avgY, piece);
                break;
            case PieceType::Path:
                placePath(ctx, blocks, meta, worldMinX, worldMinZ, isDesert, avgY, piece);
                break;
            case PieceType::Torch:
                placeTorch(ctx, blocks, meta, worldMinX, worldMinZ, isDesert, avgY, piece);
                break;
            case PieceType::WoodHut:
                placeWoodHut(ctx, blocks, meta, worldMinX, worldMinZ, isDesert, avgY, piece);
                break;
            case PieceType::Church:
                placeChurch(ctx, blocks, meta, worldMinX, worldMinZ, isDesert, avgY, piece);
                break;
            case PieceType::House1:
                placeHouse1(ctx, blocks, meta, worldMinX, worldMinZ, isDesert, avgY, piece);
                break;
            case PieceType::House2:
                placeHouse2(ctx, blocks, meta, worldMinX, worldMinZ, isDesert, avgY, piece, chests);
                break;
            case PieceType::House3:
                placeHouse3(ctx, blocks, meta, worldMinX, worldMinZ, isDesert, avgY, piece);
                break;
            case PieceType::House4Garden:
                placeHouse4Garden(ctx, blocks, meta, worldMinX, worldMinZ, isDesert, avgY, piece);
                break;
            case PieceType::Hall:
                placeHall(ctx, blocks, meta, worldMinX, worldMinZ, isDesert, avgY, piece);
                break;
            case PieceType::Field1:
                placeField1(ctx, blocks, meta, worldMinX, worldMinZ, isDesert, avgY, piece);
                break;
            case PieceType::Field2:
                placeField2(ctx, blocks, meta, worldMinX, worldMinZ, isDesert, avgY, piece);
                break;
        }
    }

    // ──── Well (Java: StructureVillagePieces$Well) ────
    void placeWell(StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                   int wmx, int wmz, bool desert, int avgY, VillagePiece& piece) {
        piece.bb.offset(0, avgY - piece.bb.maxY + 4 - 1, 0);
        ctx.bb = piece.bb;
        int CBM = ctx.coordBaseMode;

        // Cobblestone base and walls
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 0, 1, 4, 12, 4, COBBLESTONE, AIR, false);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, AIR, 0, 2, 12, 2);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, AIR, 0, 3, 12, 2);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, AIR, 0, 2, 12, 3);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, AIR, 0, 3, 12, 3);
        // Water
        placeBlock(ctx, blocks, meta, wmx, wmz, WATER, 0, 2, 1, 2);
        placeBlock(ctx, blocks, meta, wmx, wmz, WATER, 0, 3, 1, 2);
        placeBlock(ctx, blocks, meta, wmx, wmz, WATER, 0, 2, 1, 3);
        placeBlock(ctx, blocks, meta, wmx, wmz, WATER, 0, 3, 1, 3);
        // Fence posts
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 1, 1, 1);
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 1, 2, 1);
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 1, 3, 1);
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 4, 1, 1);
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 4, 2, 1);
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 4, 3, 1);
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 1, 1, 4);
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 1, 2, 4);
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 1, 3, 4);
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 4, 1, 4);
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 4, 2, 4);
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 4, 3, 4);
        // Cobblestone roof
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 4, 1, 4, 4, 4, COBBLESTONE, COBBLESTONE, false);
        // Foundation fill-down
        for (int x = 1; x <= 4; ++x)
            for (int z = 1; z <= 4; ++z)
                fillDownwardsBiome(ctx, blocks, meta, wmx, wmz, desert, COBBLESTONE, 0, x, -1, z);
    }

    // ──── Path (Java: StructureVillagePieces$Path) ────
    void placePath(StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                   int wmx, int wmz, bool desert, int avgY, VillagePiece& piece) {
        piece.bb.offset(0, avgY - piece.bb.maxY + 3 - 1, 0);
        ctx.bb = piece.bb;
        int len = piece.bb.zSize();

        for (int z = 0; z < len; ++z) {
            for (int x = 0; x <= 2; ++x) {
                placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, GRAVEL, 0, x, 0, z);
                // Clear above
                placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, x, 1, z);
                placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, x, 2, z);
            }
        }
        // Fill downwards
        for (int z = 0; z < len; ++z) {
            for (int x = 0; x <= 2; ++x) {
                fillDownwardsBiome(ctx, blocks, meta, wmx, wmz, desert, GRAVEL, 0, x, -1, z);
            }
        }
    }

    // ──── Torch (Java: StructureVillagePieces$Torch) ────
    void placeTorch(StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                    int wmx, int wmz, bool /*desert*/, int avgY, VillagePiece& piece) {
        piece.bb.offset(0, avgY - piece.bb.maxY + 4 - 1, 0);
        ctx.bb = piece.bb;

        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 0, 0, 0, 2, 3, 1, AIR, AIR, false);
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 1, 0, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 1, 1, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 1, 2, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, WOOL, 15, 1, 3, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, TORCH, 0, 0, 3, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, TORCH, 0, 1, 3, 1);
        placeBlock(ctx, blocks, meta, wmx, wmz, TORCH, 0, 2, 3, 0);
    }

    // ──── WoodHut (Java: StructureVillagePieces$WoodHut) ────
    void placeWoodHut(StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                      int wmx, int wmz, bool desert, int avgY, VillagePiece& piece) {
        piece.bb.offset(0, avgY - piece.bb.maxY + 6 - 1, 0);
        ctx.bb = piece.bb;
        int CBM = ctx.coordBaseMode;

        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 1, 1, 3, 5, 4, OAK_PLANKS, AIR, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 0, 0, 3, 0, 4, COBBLESTONE, COBBLESTONE, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 1, 0, 1, 2, 0, 3, DIRT, DIRT, false);
        if (piece.isTall) {
            fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 4, 1, 2, 4, 3, OAK_LOG, OAK_LOG, false);
        }
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 1, 1, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 1, 2, 0);
        // Door
        int doorMeta = getMetadataWithOffset(WOODEN_DOOR, 1, CBM);
        placeDoor(ctx, blocks, meta, wmx, wmz, *(JavaRNG*)nullptr, 1, 1, 0, static_cast<uint8_t>(doorMeta));
        // Windows
        if (piece.isTall) {
            placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 0, 2, 2);
            placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 3, 2, 2);
            placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 2, 2, 4);
        } else {
            placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 0, 2, 2);
            placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 2, 2, 4);
        }
        // Foundation
        for (int x = 0; x <= 3; ++x)
            for (int z = 0; z <= 4; ++z) {
                clearUpwards(ctx, blocks, meta, wmx, wmz, x, 6, z);
                fillDownwardsBiome(ctx, blocks, meta, wmx, wmz, desert, COBBLESTONE, 0, x, -1, z);
            }
    }

    // ──── Church (Java: StructureVillagePieces$Church) ────
    void placeChurch(StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                     int wmx, int wmz, bool desert, int avgY, VillagePiece& piece) {
        piece.bb.offset(0, avgY - piece.bb.maxY + 12 - 1, 0);
        ctx.bb = piece.bb;
        int CBM = ctx.coordBaseMode;

        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 1, 1, 3, 3, 7, COBBLESTONE, AIR, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 5, 1, 3, 9, 3, COBBLESTONE, AIR, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 0, 0, 3, 0, 8, COBBLESTONE, COBBLESTONE, false);
        // Stairs for roof
        for (int x = 1; x <= 3; ++x) {
            placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_STAIRS,
                static_cast<uint8_t>(getMetadataWithOffset(OAK_STAIRS, 3, CBM)), x, 4, 2);
            placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_STAIRS,
                static_cast<uint8_t>(getMetadataWithOffset(OAK_STAIRS, 2, CBM)), x, 4, 6);
        }
        // Ladder
        int ladderMeta = getMetadataWithOffset(LADDER, 3, CBM);
        for (int y = 1; y <= 9; ++y)
            placeBlock(ctx, blocks, meta, wmx, wmz, LADDER, static_cast<uint8_t>(ladderMeta), 3, y, 3);
        // Torch
        placeBlock(ctx, blocks, meta, wmx, wmz, TORCH, 0, 2, 4, 1);
        // Door
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 2, 1, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 2, 2, 0);
        int doorMeta = getMetadataWithOffset(WOODEN_DOOR, 1, CBM);
        placeDoor(ctx, blocks, meta, wmx, wmz, *(JavaRNG*)nullptr, 2, 1, 0, static_cast<uint8_t>(doorMeta));
        // Windows
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 0, 2, 4);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 4, 2, 4);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 2, 2, 8);
        // Tower windows
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 0, 6, 2);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 4, 6, 2);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 2, 6, 0);
        // Foundation
        for (int z = 0; z <= 8; ++z)
            for (int x = 0; x <= 4; ++x) {
                clearUpwards(ctx, blocks, meta, wmx, wmz, x, 12, z);
                fillDownwardsBiome(ctx, blocks, meta, wmx, wmz, desert, COBBLESTONE, 0, x, -1, z);
            }
    }

    // ──── House1/Library (Java: StructureVillagePieces$House1) ────
    void placeHouse1(StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                     int wmx, int wmz, bool desert, int avgY, VillagePiece& piece) {
        piece.bb.offset(0, avgY - piece.bb.maxY + 9 - 1, 0);
        ctx.bb = piece.bb;
        int CBM = ctx.coordBaseMode;

        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 1, 1, 7, 5, 4, OAK_PLANKS, AIR, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 0, 0, 8, 0, 5, COBBLESTONE, COBBLESTONE, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 5, 0, 8, 5, 5, COBBLESTONE, COBBLESTONE, false);
        // Planks on top
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 6, 1, 8, 6, 4, OAK_PLANKS, OAK_PLANKS, false);
        // Stairs for roof
        int s3 = getMetadataWithOffset(OAK_STAIRS, 3, CBM);
        int s2 = getMetadataWithOffset(OAK_STAIRS, 2, CBM);
        for (int x = -1; x <= 2; ++x) {
            for (int xx = 0; xx <= 8; ++xx) {
                placeBlock(ctx, blocks, meta, wmx, wmz,
                    getBiomeBlock(OAK_STAIRS, desert), static_cast<uint8_t>(s3), xx, 6 + x, 5 - x);
                placeBlock(ctx, blocks, meta, wmx, wmz,
                    getBiomeBlock(OAK_STAIRS, desert), static_cast<uint8_t>(s2), xx, 6 + x, x);
            }
        }
        // Door
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 2, 1, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 2, 2, 0);
        int doorMeta = getMetadataWithOffset(WOODEN_DOOR, 1, CBM);
        placeDoor(ctx, blocks, meta, wmx, wmz, *(JavaRNG*)nullptr, 2, 1, 0, static_cast<uint8_t>(doorMeta));
        // Windows
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 0, 2, 2);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 0, 2, 3);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 8, 2, 2);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 8, 2, 3);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 4, 2, 5);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 5, 2, 0);
        // Bookshelves
        placeBlock(ctx, blocks, meta, wmx, wmz, BOOKSHELF, 0, 3, 1, 4);
        placeBlock(ctx, blocks, meta, wmx, wmz, BOOKSHELF, 0, 4, 1, 4);
        placeBlock(ctx, blocks, meta, wmx, wmz, BOOKSHELF, 0, 5, 1, 4);
        placeBlock(ctx, blocks, meta, wmx, wmz, BOOKSHELF, 0, 6, 1, 4);
        placeBlock(ctx, blocks, meta, wmx, wmz, BOOKSHELF, 0, 5, 3, 4);
        placeBlock(ctx, blocks, meta, wmx, wmz, BOOKSHELF, 0, 6, 3, 4);
        // Crafting table
        placeBlock(ctx, blocks, meta, wmx, wmz, CRAFTING_TABLE, 0, 1, 1, 4);
        // Foundation
        for (int z = 0; z <= 5; ++z)
            for (int x = 0; x <= 8; ++x) {
                clearUpwards(ctx, blocks, meta, wmx, wmz, x, 9, z);
                fillDownwardsBiome(ctx, blocks, meta, wmx, wmz, desert, COBBLESTONE, 0, x, -1, z);
            }
    }

    // ──── House2/Blacksmith (Java: StructureVillagePieces$House2) ────
    void placeHouse2(StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                     int wmx, int wmz, bool desert, int avgY, VillagePiece& piece,
                     std::vector<Chunk::ChestInfo>& chests) {
        piece.bb.offset(0, avgY - piece.bb.maxY + 6 - 1, 0);
        ctx.bb = piece.bb;
        int CBM = ctx.coordBaseMode;

        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 1, 0, 9, 4, 6, COBBLESTONE, AIR, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 0, 0, 9, 0, 6, COBBLESTONE, COBBLESTONE, false);
        // Iron bars
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 4, 1, 0, 5, 3, 0, IRON_BARS, IRON_BARS, false);
        // Lava
        placeBlock(ctx, blocks, meta, wmx, wmz, LAVA, 0, 1, 1, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, LAVA, 0, 1, 1, 6);
        // Double stone slab counters
        placeBlock(ctx, blocks, meta, wmx, wmz, DOUBLE_STONE_SLAB, 0, 7, 1, 1);
        placeBlock(ctx, blocks, meta, wmx, wmz, DOUBLE_STONE_SLAB, 0, 7, 1, 2);
        placeBlock(ctx, blocks, meta, wmx, wmz, DOUBLE_STONE_SLAB, 0, 7, 1, 3);
        // Furnaces
        placeBlock(ctx, blocks, meta, wmx, wmz, FURNACE, static_cast<uint8_t>(getMetadataWithOffset(FURNACE, 3, CBM)), 8, 1, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, FURNACE, static_cast<uint8_t>(getMetadataWithOffset(FURNACE, 3, CBM)), 9, 1, 0);
        // Door
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 6, 1, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 6, 2, 0);
        int doorMeta = getMetadataWithOffset(WOODEN_DOOR, 1, CBM);
        placeDoor(ctx, blocks, meta, wmx, wmz, *(JavaRNG*)nullptr, 6, 1, 0, static_cast<uint8_t>(doorMeta));
        // Chest (blacksmith loot)
        if (!piece.hasMadeChest) {
            piece.hasMadeChest = true;
            int wx = ctx.getXWithOffset(7, 5);
            int wy = ctx.getYWithOffset(1);
            int wz = ctx.getZWithOffset(7, 5);
            int lx = wx - wmx;
            int lz = wz - wmz;
            if (lx >= 0 && lx <= 15 && lz >= 0 && lz <= 15) {
                placeBlock(ctx, blocks, meta, wmx, wmz, CHEST, 0, 7, 1, 5);
                Chunk::ChestInfo ci;
                ci.x = wx; ci.y = wy; ci.z = wz;
                ci.lootTable = "blacksmith";
                chests.push_back(std::move(ci));
            }
        }
        // Foundation
        for (int z = 0; z <= 6; ++z)
            for (int x = 0; x <= 9; ++x) {
                clearUpwards(ctx, blocks, meta, wmx, wmz, x, 6, z);
                fillDownwardsBiome(ctx, blocks, meta, wmx, wmz, desert, COBBLESTONE, 0, x, -1, z);
            }
    }

    // ──── House3 (Java: StructureVillagePieces$House3) ────
    void placeHouse3(StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                     int wmx, int wmz, bool desert, int avgY, VillagePiece& piece) {
        piece.bb.offset(0, avgY - piece.bb.maxY + 7 - 1, 0);
        ctx.bb = piece.bb;
        int CBM = ctx.coordBaseMode;

        // Interior air
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 1, 1, 1, 7, 4, 4, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 2, 1, 6, 8, 4, 10, AIR, AIR, false);
        // Floors
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 2, 0, 5, 8, 0, 10, OAK_PLANKS, OAK_PLANKS, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 0, 1, 7, 0, 4, OAK_PLANKS, OAK_PLANKS, false);
        // Walls
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 0, 0, 0, 3, 5, COBBLESTONE, COBBLESTONE, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 8, 0, 0, 8, 3, 10, COBBLESTONE, COBBLESTONE, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 0, 0, 7, 2, 0, COBBLESTONE, COBBLESTONE, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 0, 5, 2, 1, 5, COBBLESTONE, COBBLESTONE, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 2, 0, 6, 2, 3, 10, COBBLESTONE, COBBLESTONE, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 3, 0, 10, 7, 3, 10, COBBLESTONE, COBBLESTONE, false);
        // Upper walls
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 2, 0, 7, 3, 0, OAK_PLANKS, OAK_PLANKS, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 2, 5, 2, 3, 5, OAK_PLANKS, OAK_PLANKS, false);
        // Roof
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 4, 1, 8, 4, 1, OAK_PLANKS, OAK_PLANKS, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 4, 4, 3, 4, 4, OAK_PLANKS, OAK_PLANKS, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 5, 2, 8, 5, 3, OAK_PLANKS, OAK_PLANKS, false);
        // Logs and glass
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_LOG, 0, 0, 2, 1);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_LOG, 0, 0, 2, 4);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 0, 2, 2);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 0, 2, 3);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_LOG, 0, 8, 2, 1);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 8, 2, 2);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 8, 2, 3);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_LOG, 0, 8, 2, 4);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_LOG, 0, 2, 2, 6);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 2, 2, 7);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 2, 2, 8);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_LOG, 0, 2, 2, 9);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_LOG, 0, 8, 2, 6);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 8, 2, 7);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 8, 2, 8);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_LOG, 0, 8, 2, 9);
        // Door
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 2, 1, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 2, 2, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, TORCH, 0, 2, 3, 1);
        int doorMeta = getMetadataWithOffset(WOODEN_DOOR, 1, CBM);
        placeDoor(ctx, blocks, meta, wmx, wmz, *(JavaRNG*)nullptr, 2, 1, 0, static_cast<uint8_t>(doorMeta));
        // Foundation
        for (int z = 0; z < 5; ++z)
            for (int x = 0; x < 9; ++x) {
                clearUpwards(ctx, blocks, meta, wmx, wmz, x, 7, z);
                fillDownwardsBiome(ctx, blocks, meta, wmx, wmz, desert, COBBLESTONE, 0, x, -1, z);
            }
        for (int z = 5; z < 11; ++z)
            for (int x = 2; x < 9; ++x) {
                clearUpwards(ctx, blocks, meta, wmx, wmz, x, 7, z);
                fillDownwardsBiome(ctx, blocks, meta, wmx, wmz, desert, COBBLESTONE, 0, x, -1, z);
            }
    }

    // ──── House4Garden (Java: StructureVillagePieces$House4Garden) ────
    void placeHouse4Garden(StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                           int wmx, int wmz, bool desert, int avgY, VillagePiece& piece) {
        piece.bb.offset(0, avgY - piece.bb.maxY + 6 - 1, 0);
        ctx.bb = piece.bb;
        int CBM = ctx.coordBaseMode;

        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 0, 0, 4, 0, 4, COBBLESTONE, COBBLESTONE, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 4, 0, 4, 4, 4, OAK_LOG, OAK_LOG, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 4, 1, 3, 4, 3, OAK_PLANKS, OAK_PLANKS, false);
        // Corner pillars
        for (int y = 1; y <= 3; ++y) {
            placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, COBBLESTONE, 0, 0, y, 0);
            placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, COBBLESTONE, 0, 4, y, 0);
            placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, COBBLESTONE, 0, 0, y, 4);
            placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, COBBLESTONE, 0, 4, y, 4);
        }
        // Walls
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 1, 1, 0, 3, 3, OAK_PLANKS, OAK_PLANKS, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 4, 1, 1, 4, 3, 3, OAK_PLANKS, OAK_PLANKS, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 1, 4, 3, 3, 4, OAK_PLANKS, OAK_PLANKS, false);
        // Front wall
        for (int y = 1; y <= 3; ++y) {
            placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_PLANKS, 0, 1, y, 0);
            placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_PLANKS, 0, 3, y, 0);
        }
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_PLANKS, 0, 2, 3, 0);
        // Windows
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 0, 2, 2);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 2, 2, 4);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 4, 2, 2);
        // Interior air
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 1, 1, 1, 3, 3, 3, AIR, AIR, false);
        // Terrace
        if (piece.isRoofAccessible) {
            for (int x = 0; x <= 4; ++x) {
                placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, x, 5, 0);
                placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, x, 5, 4);
            }
            for (int z = 1; z <= 3; ++z) {
                placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 0, 5, z);
                placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 4, 5, z);
            }
            // Ladder
            int ladderMeta = getMetadataWithOffset(LADDER, 3, CBM);
            for (int y = 1; y <= 4; ++y)
                placeBlock(ctx, blocks, meta, wmx, wmz, LADDER, static_cast<uint8_t>(ladderMeta), 3, y, 3);
        }
        // Torch
        placeBlock(ctx, blocks, meta, wmx, wmz, TORCH, 0, 2, 3, 1);
        // Door
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 2, 1, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 2, 2, 0);
        // Foundation
        for (int z = 0; z <= 4; ++z)
            for (int x = 0; x <= 4; ++x) {
                clearUpwards(ctx, blocks, meta, wmx, wmz, x, 6, z);
                fillDownwardsBiome(ctx, blocks, meta, wmx, wmz, desert, COBBLESTONE, 0, x, -1, z);
            }
    }

    // ──── Hall/Butcher (Java: StructureVillagePieces$Hall) ────
    void placeHall(StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                   int wmx, int wmz, bool desert, int avgY, VillagePiece& piece) {
        piece.bb.offset(0, avgY - piece.bb.maxY + 7 - 1, 0);
        ctx.bb = piece.bb;
        int CBM = ctx.coordBaseMode;

        // Interior air
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 1, 1, 1, 7, 4, 4, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 2, 1, 6, 8, 4, 10, AIR, AIR, false);
        // Dirt courtyard
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 2, 0, 6, 8, 0, 10, DIRT, DIRT, false);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, COBBLESTONE, 0, 6, 0, 6);
        // Fenced courtyard
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 2, 1, 6, 2, 1, 10, FENCE, FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 8, 1, 6, 8, 1, 10, FENCE, FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 3, 1, 10, 7, 1, 10, FENCE, FENCE, false);
        // Floor
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 0, 1, 7, 0, 4, OAK_PLANKS, OAK_PLANKS, false);
        // Walls
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 0, 0, 0, 3, 5, COBBLESTONE, COBBLESTONE, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 8, 0, 0, 8, 3, 5, COBBLESTONE, COBBLESTONE, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 0, 0, 7, 1, 0, COBBLESTONE, COBBLESTONE, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 0, 5, 7, 1, 5, COBBLESTONE, COBBLESTONE, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 2, 0, 7, 3, 0, OAK_PLANKS, OAK_PLANKS, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 1, 2, 5, 7, 3, 5, OAK_PLANKS, OAK_PLANKS, false);
        // Roof (same as House3)
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 4, 1, 8, 4, 1, OAK_PLANKS, OAK_PLANKS, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 4, 4, 8, 4, 4, OAK_PLANKS, OAK_PLANKS, false);
        fillWithBlocksBiome(ctx, blocks, meta, wmx, wmz, desert, 0, 5, 2, 8, 5, 3, OAK_PLANKS, OAK_PLANKS, false);
        // Stair roof
        int s3 = getMetadataWithOffset(OAK_STAIRS, 3, CBM);
        int s2 = getMetadataWithOffset(OAK_STAIRS, 2, CBM);
        for (int i = -1; i <= 2; ++i)
            for (int x = 0; x <= 8; ++x) {
                placeBlock(ctx, blocks, meta, wmx, wmz, getBiomeBlock(OAK_STAIRS, desert), static_cast<uint8_t>(s3), x, 4 + i, i);
                placeBlock(ctx, blocks, meta, wmx, wmz, getBiomeBlock(OAK_STAIRS, desert), static_cast<uint8_t>(s2), x, 4 + i, 5 - i);
            }
        // Log pillars
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_LOG, 0, 0, 2, 1);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_LOG, 0, 0, 2, 4);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_LOG, 0, 8, 2, 1);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_LOG, 0, 8, 2, 4);
        // Windows
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 0, 2, 2);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 0, 2, 3);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 8, 2, 2);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 8, 2, 3);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 2, 2, 5);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 3, 2, 5);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 5, 2, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, GLASS_PANE, 0, 6, 2, 5);
        // Table
        placeBlock(ctx, blocks, meta, wmx, wmz, FENCE, 0, 2, 1, 3);
        placeBlock(ctx, blocks, meta, wmx, wmz, WOODEN_PRESSURE_PLATE, 0, 2, 2, 3);
        placeBlockBiome(ctx, blocks, meta, wmx, wmz, desert, OAK_PLANKS, 0, 1, 1, 4);
        // Counter / double slab
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 5, 0, 1, 7, 0, 3, DOUBLE_STONE_SLAB, DOUBLE_STONE_SLAB, false);
        placeBlock(ctx, blocks, meta, wmx, wmz, DOUBLE_STONE_SLAB, 0, 6, 1, 1);
        placeBlock(ctx, blocks, meta, wmx, wmz, DOUBLE_STONE_SLAB, 0, 6, 1, 2);
        // Front door
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 2, 1, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 2, 2, 0);
        placeBlock(ctx, blocks, meta, wmx, wmz, TORCH, 0, 2, 3, 1);
        int doorMeta = getMetadataWithOffset(WOODEN_DOOR, 1, CBM);
        placeDoor(ctx, blocks, meta, wmx, wmz, *(JavaRNG*)nullptr, 2, 1, 0, static_cast<uint8_t>(doorMeta));
        // Back door
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 6, 1, 5);
        placeBlock(ctx, blocks, meta, wmx, wmz, AIR, 0, 6, 2, 5);
        placeBlock(ctx, blocks, meta, wmx, wmz, TORCH, 0, 6, 3, 4);
        placeDoor(ctx, blocks, meta, wmx, wmz, *(JavaRNG*)nullptr, 6, 1, 5, static_cast<uint8_t>(doorMeta));
        // Foundation
        for (int z = 0; z < 5; ++z)
            for (int x = 0; x < 9; ++x) {
                clearUpwards(ctx, blocks, meta, wmx, wmz, x, 7, z);
                fillDownwardsBiome(ctx, blocks, meta, wmx, wmz, desert, COBBLESTONE, 0, x, -1, z);
            }
    }

    // ──── Field1 (Java: StructureVillagePieces$Field1) ────
    void placeField1(StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                     int wmx, int wmz, bool /*desert*/, int avgY, VillagePiece& piece) {
        piece.bb.offset(0, avgY - piece.bb.maxY + 4 - 1, 0);
        ctx.bb = piece.bb;

        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 0, 1, 0, 12, 4, 8, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 1, 0, 1, 2, 0, 7, FARMLAND, FARMLAND, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 4, 0, 1, 5, 0, 7, FARMLAND, FARMLAND, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 7, 0, 1, 8, 0, 7, FARMLAND, FARMLAND, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 10, 0, 1, 11, 0, 7, FARMLAND, FARMLAND, false);
        // Borders
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 0, 0, 0, 0, 0, 8, OAK_LOG, OAK_LOG, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 6, 0, 0, 6, 0, 8, OAK_LOG, OAK_LOG, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 12, 0, 0, 12, 0, 8, OAK_LOG, OAK_LOG, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 1, 0, 0, 11, 0, 0, OAK_LOG, OAK_LOG, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 1, 0, 8, 11, 0, 8, OAK_LOG, OAK_LOG, false);
        // Water channels
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 3, 0, 1, 3, 0, 7, WATER, WATER, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 9, 0, 1, 9, 0, 7, WATER, WATER, false);
        // Crops (random growth stage 2-7)
        JavaRNG cropRng(piece.bb.minX * 31L + piece.bb.minZ);
        for (int z = 1; z <= 7; ++z) {
            placeBlock(ctx, blocks, meta, wmx, wmz, piece.cropA, static_cast<uint8_t>(cropRng.nextIntRange(2, 7)), 1, 1, z);
            placeBlock(ctx, blocks, meta, wmx, wmz, piece.cropA, static_cast<uint8_t>(cropRng.nextIntRange(2, 7)), 2, 1, z);
            placeBlock(ctx, blocks, meta, wmx, wmz, piece.cropB, static_cast<uint8_t>(cropRng.nextIntRange(2, 7)), 4, 1, z);
            placeBlock(ctx, blocks, meta, wmx, wmz, piece.cropB, static_cast<uint8_t>(cropRng.nextIntRange(2, 7)), 5, 1, z);
            placeBlock(ctx, blocks, meta, wmx, wmz, piece.cropC, static_cast<uint8_t>(cropRng.nextIntRange(2, 7)), 7, 1, z);
            placeBlock(ctx, blocks, meta, wmx, wmz, piece.cropC, static_cast<uint8_t>(cropRng.nextIntRange(2, 7)), 8, 1, z);
            placeBlock(ctx, blocks, meta, wmx, wmz, piece.cropD, static_cast<uint8_t>(cropRng.nextIntRange(2, 7)), 10, 1, z);
            placeBlock(ctx, blocks, meta, wmx, wmz, piece.cropD, static_cast<uint8_t>(cropRng.nextIntRange(2, 7)), 11, 1, z);
        }
        // Foundation
        for (int z = 0; z < 9; ++z)
            for (int x = 0; x < 13; ++x) {
                clearUpwards(ctx, blocks, meta, wmx, wmz, x, 4, z);
                fillDownwards(ctx, blocks, meta, wmx, wmz, DIRT, 0, x, -1, z);
            }
    }

    // ──── Field2 (Java: StructureVillagePieces$Field2) ────
    void placeField2(StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                     int wmx, int wmz, bool /*desert*/, int avgY, VillagePiece& piece) {
        piece.bb.offset(0, avgY - piece.bb.maxY + 4 - 1, 0);
        ctx.bb = piece.bb;

        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 0, 1, 0, 6, 4, 8, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 1, 0, 1, 2, 0, 7, FARMLAND, FARMLAND, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 4, 0, 1, 5, 0, 7, FARMLAND, FARMLAND, false);
        // Borders
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 0, 0, 0, 0, 0, 8, OAK_LOG, OAK_LOG, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 6, 0, 0, 6, 0, 8, OAK_LOG, OAK_LOG, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 1, 0, 0, 5, 0, 0, OAK_LOG, OAK_LOG, false);
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 1, 0, 8, 5, 0, 8, OAK_LOG, OAK_LOG, false);
        // Water
        fillWithBlocks(ctx, blocks, meta, wmx, wmz, 3, 0, 1, 3, 0, 7, WATER, WATER, false);
        // Crops
        JavaRNG cropRng(piece.bb.minX * 31L + piece.bb.minZ + 1);
        for (int z = 1; z <= 7; ++z) {
            placeBlock(ctx, blocks, meta, wmx, wmz, piece.cropA, static_cast<uint8_t>(cropRng.nextIntRange(2, 7)), 1, 1, z);
            placeBlock(ctx, blocks, meta, wmx, wmz, piece.cropA, static_cast<uint8_t>(cropRng.nextIntRange(2, 7)), 2, 1, z);
            placeBlock(ctx, blocks, meta, wmx, wmz, piece.cropB, static_cast<uint8_t>(cropRng.nextIntRange(2, 7)), 4, 1, z);
            placeBlock(ctx, blocks, meta, wmx, wmz, piece.cropB, static_cast<uint8_t>(cropRng.nextIntRange(2, 7)), 5, 1, z);
        }
        // Foundation
        for (int z = 0; z < 9; ++z)
            for (int x = 0; x < 7; ++x) {
                clearUpwards(ctx, blocks, meta, wmx, wmz, x, 4, z);
                fillDownwards(ctx, blocks, meta, wmx, wmz, DIRT, 0, x, -1, z);
            }
    }
};

} // namespace mccpp
