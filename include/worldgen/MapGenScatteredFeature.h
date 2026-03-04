/**
 * MapGenScatteredFeature.h — Scattered feature structure generation.
 *
 * Java reference: net.minecraft.world.gen.structure.MapGenScatteredFeature
 *               + ComponentScatteredFeaturePieces (DesertPyramid, JunglePyramid, SwampHut)
 *
 * Generates three structure types based on biome:
 *   - Desert Pyramid (biome 2): 21x15x21 sandstone with TNT trap + 4 chests
 *   - Jungle Pyramid (biome 21): 12x10x15 cobblestone with tripwire/piston puzzles + 2 chests + 2 dispensers
 *   - Swamp Hut (biome 6): 7x5x9 spruce hut with cauldron + witch spawn
 *
 * Grid-based spawning: maxDist=32, minDist=8, salt=14357
 *
 * Thread safety: Each OverworldGenerator instance owns its own MapGenScatteredFeature.
 */
#pragma once

#include "world/Chunk.h"
#include "worldgen/LootTables.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <random>
#include <vector>

namespace mccpp {

class MapGenScatteredFeature {
public:
    // BiomeQueryFn: given world (x, z), return biome ID
    using BiomeQueryFn = std::function<int(int, int)>;

    explicit MapGenScatteredFeature(int64_t seed, BiomeQueryFn biomeQuery = nullptr)
        : worldSeed_(seed), biomeQuery_(std::move(biomeQuery)) {}

    void setBiomeQuery(BiomeQueryFn fn) { biomeQuery_ = std::move(fn); }

    void generate(int64_t seed, int chunkX, int chunkZ,
                  int32_t* blocks, uint8_t* meta,
                  std::vector<Chunk::SpawnerInfo>& spawners,
                  std::vector<Chunk::ChestInfo>& chests) {
        constexpr int SCAN_RADIUS = 8;
        int worldMinX = chunkX * 16;
        int worldMinZ = chunkZ * 16;

        for (int cx = chunkX - SCAN_RADIUS; cx <= chunkX + SCAN_RADIUS; ++cx) {
            for (int cz = chunkZ - SCAN_RADIUS; cz <= chunkZ + SCAN_RADIUS; ++cz) {
                // Java: MapGenScatteredFeature.canSpawnStructureAtCoords
                int gridX = cx >= 0 ? cx / MAX_DIST : (cx - MAX_DIST + 1) / MAX_DIST;
                int gridZ = cz >= 0 ? cz / MAX_DIST : (cz - MAX_DIST + 1) / MAX_DIST;

                JavaRNG rng(static_cast<int64_t>(gridX) * 341873128712LL +
                            static_cast<int64_t>(gridZ) * 132897987541LL +
                            worldSeed_ + 14357LL);

                int spawnX = gridX * MAX_DIST + rng.nextInt(MAX_DIST - MIN_DIST);
                int spawnZ = gridZ * MAX_DIST + rng.nextInt(MAX_DIST - MIN_DIST);

                if (cx != spawnX || cz != spawnZ) continue;

                // Check biome at center of spawn chunk
                int biome = getBiome(cx * 16 + 8, cz * 16 + 8);
                StructureType type;
                if (biome == BIOME_DESERT || biome == BIOME_DESERT_HILLS) {
                    type = StructureType::DesertPyramid;
                } else if (biome == BIOME_JUNGLE || biome == BIOME_JUNGLE_HILLS) {
                    type = StructureType::JunglePyramid;
                } else if (biome == BIOME_SWAMPLAND) {
                    type = StructureType::SwampHut;
                } else {
                    continue;
                }

                // Determine coordBaseMode from RNG
                JavaRNG structRng(getChunkSeed(cx, cz));
                int coordBaseMode = structRng.nextInt(4);

                // Determine structure Y from terrain sampling (func_74935_a)
                int structY = getAverageGroundLevel(blocks, meta, worldMinX, worldMinZ, cx, cz, type, coordBaseMode);

                placeStructure(type, cx, cz, structY, coordBaseMode, structRng,
                               worldMinX, worldMinZ, blocks, meta, spawners, chests);
            }
        }
    }

private:
    int64_t worldSeed_;
    BiomeQueryFn biomeQuery_;

    static constexpr int MAX_DIST = 32;
    static constexpr int MIN_DIST = 8;

    // Biome IDs
    static constexpr int BIOME_DESERT = 2;
    static constexpr int BIOME_DESERT_HILLS = 17;
    static constexpr int BIOME_JUNGLE = 21;
    static constexpr int BIOME_JUNGLE_HILLS = 22;
    static constexpr int BIOME_SWAMPLAND = 6;

    // Block IDs
    static constexpr int32_t AIR = 0;
    static constexpr int32_t STONE = 1;
    static constexpr int32_t SANDSTONE = 24;
    static constexpr int32_t COBWEB = 30;
    static constexpr int32_t WOOL = 35;
    static constexpr int32_t TNT = 46;
    static constexpr int32_t COBBLESTONE = 4;
    static constexpr int32_t MOSSY_COBBLESTONE = 48;
    static constexpr int32_t OAK_PLANKS = 5;
    static constexpr int32_t OAK_LOG = 17;
    static constexpr int32_t SANDSTONE_STAIRS = 128;
    static constexpr int32_t STONE_STAIRS = 67;
    static constexpr int32_t OAK_STAIRS = 53;
    static constexpr int32_t SPRUCE_STAIRS = 134;
    static constexpr int32_t STONE_SLAB = 44;
    static constexpr int32_t STONE_PRESSURE_PLATE = 70;
    static constexpr int32_t CHEST = 54;
    static constexpr int32_t FENCE = 85;
    static constexpr int32_t CRAFTING_TABLE = 58;
    static constexpr int32_t CAULDRON = 118;
    static constexpr int32_t FLOWER_POT = 140;
    static constexpr int32_t TRIPWIRE_HOOK = 131;
    static constexpr int32_t TRIPWIRE = 132;
    static constexpr int32_t REDSTONE_WIRE = 55;
    static constexpr int32_t DISPENSER = 23;
    static constexpr int32_t VINE = 106;
    static constexpr int32_t STICKY_PISTON = 29;
    static constexpr int32_t UNPOWERED_REPEATER = 93;
    static constexpr int32_t STONEBRICK = 98;
    static constexpr int32_t LEVER = 69;
    static constexpr int32_t SPAWNER = 52;

    // ──── Direction/Facing arrays (from Java Direction.java, Facing.java) ────
    static constexpr int DIR_OFFSET_X[4] = {0, -1, 0, 1};      // S, W, N, E
    static constexpr int DIR_OFFSET_Z[4] = {1, 0, -1, 0};
    static constexpr int DIR_ROTATE_OPPOSITE[4] = {2, 3, 0, 1}; // Direction.rotateOpposite
    static constexpr int FACING_OPPOSITE[6] = {1, 0, 3, 2, 5, 4}; // Facing.oppositeSide

    enum class StructureType { DesertPyramid, JunglePyramid, SwampHut };

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
        double nextDouble() {
            return ((static_cast<int64_t>(next(26)) << 27) + next(27)) / static_cast<double>(1LL << 53);
        }
        bool nextBoolean() { return next(1) != 0; }
        float nextFloat() { return next(24) / static_cast<float>(1 << 24); }
    };

    int64_t getChunkSeed(int cx, int cz) const {
        return worldSeed_ + static_cast<int64_t>(cx) * 341873128712LL
                          + static_cast<int64_t>(cz) * 132897987541LL;
    }

    int getBiome(int x, int z) const {
        if (biomeQuery_) return biomeQuery_(x, z);
        return 1; // default plains
    }

    // ──── Bounding Box ────
    struct BBox {
        int minX, minY, minZ, maxX, maxY, maxZ;
        bool isVecInside(int x, int y, int z) const {
            return x >= minX && x <= maxX && y >= minY && y <= maxY && z >= minZ && z <= maxZ;
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

    static inline bool isLiquid(int32_t id) {
        return id == 8 || id == 9 || id == 10 || id == 11;
    }

    // ──── Coordinate transforms (Java: StructureComponent) ────
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

    // ──── Metadata rotation (Java: StructureComponent.getMetadataWithOffset) ────
    static int getMetadataWithOffset(int32_t blockId, int meta, int coordBaseMode) {
        // Stairs
        if (blockId == STONE_STAIRS || blockId == OAK_STAIRS ||
            blockId == SANDSTONE_STAIRS || blockId == SPRUCE_STAIRS) {
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
        // Tripwire hook / BlockDirectional
        else if (blockId == TRIPWIRE_HOOK || blockId == UNPOWERED_REPEATER) {
            if (coordBaseMode == 0) {
                if (meta == 0 || meta == 2) return DIR_ROTATE_OPPOSITE[meta];
            } else if (coordBaseMode == 1) {
                if (meta == 2) return 1; if (meta == 0) return 3;
                if (meta == 1) return 2; if (meta == 3) return 0;
            } else if (coordBaseMode == 3) {
                if (meta == 2) return 3; if (meta == 0) return 1;
                if (meta == 1) return 2; if (meta == 3) return 0;
            }
        }
        // Piston, Lever, Dispenser
        else if (blockId == STICKY_PISTON || blockId == LEVER || blockId == DISPENSER) {
            if (coordBaseMode == 0) {
                if (meta == 2 || meta == 3) return FACING_OPPOSITE[meta];
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

    // Java: BlockLever.invertMetadata
    static int invertLeverMetadata(int meta) {
        switch (meta) {
            case 0: return 7;
            case 7: return 0;
            case 1: return 6;
            case 6: return 1;
            case 2: return 5;
            case 5: return 2;
            case 3: return 4;
            case 4: return 3;
            default: return meta;
        }
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

    void fillWithMetadataBlocks(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                                int worldMinX, int worldMinZ,
                                int x1, int y1, int z1, int x2, int y2, int z2,
                                int32_t outer, uint8_t outerMeta,
                                int32_t inner, uint8_t innerMeta, bool skipAir) {
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
                               isBorder ? outer : inner,
                               isBorder ? outerMeta : innerMeta, x, y, z);
                }
    }

    // Java: fillWithRandomizedBlocks — for jungle pyramid stones
    void fillWithRandomizedStones(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                                  int worldMinX, int worldMinZ, JavaRNG& rng,
                                  int x1, int y1, int z1, int x2, int y2, int z2) {
        for (int y = y1; y <= y2; ++y)
            for (int x = x1; x <= x2; ++x)
                for (int z = z1; z <= z2; ++z) {
                    int32_t block = (rng.nextFloat() < 0.3f) ? MOSSY_COBBLESTONE : COBBLESTONE;
                    placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, block, 0, x, y, z);
                }
    }

    void fillWithAir(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                     int worldMinX, int worldMinZ,
                     int x1, int y1, int z1, int x2, int y2, int z2) {
        for (int y = y1; y <= y2; ++y)
            for (int x = x1; x <= x2; ++x)
                for (int z = z1; z <= z2; ++z)
                    placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, x, y, z);
    }

    // Java: func_151554_b — fill downward while air/liquid
    void fillDownwards(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                       int worldMinX, int worldMinZ,
                       int32_t blockId, uint8_t blockMeta, int lx, int ly, int lz) {
        int wx = ctx.getXWithOffset(lx, lz);
        int wy = ctx.getYWithOffset(ly);
        int wz = ctx.getZWithOffset(lx, lz);
        while ((getBlockAt(blocks, worldMinX, worldMinZ, wx, wy, wz) == AIR ||
                isLiquid(getBlockAt(blocks, worldMinX, worldMinZ, wx, wy, wz))) && wy > 1) {
            setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, blockId, blockMeta);
            --wy;
        }
    }

    // Java: func_74935_a — average ground level check for scattered features
    int getAverageGroundLevel(const int32_t* blocks, const uint8_t*, int worldMinX, int worldMinZ,
                              int structCX, int structCZ, StructureType type, int coordBaseMode) {
        (void)coordBaseMode;
        // Sample 4 corners of the structure to get average ground level
        // For simplicity, sample the center of the structure chunk
        int cx = structCX * 16 + 8;
        int cz = structCZ * 16 + 8;
        int lx = cx - worldMinX;
        int lz = cz - worldMinZ;
        if (lx < 0 || lx > 15 || lz < 0 || lz > 15) return 64;
        // Find top solid block
        for (int y = 128; y > 1; --y) {
            int32_t id = blocks[(lx * 16 + lz) * 256 + y];
            if (id != AIR && !isLiquid(id)) return y + 1;
        }
        return 64;
    }

    // ──── Structure dispatcher ────
    void placeStructure(StructureType type, int structCX, int structCZ, int structY, int coordBaseMode,
                        JavaRNG& rng, int worldMinX, int worldMinZ,
                        int32_t* blocks, uint8_t* meta,
                        std::vector<Chunk::SpawnerInfo>& spawners,
                        std::vector<Chunk::ChestInfo>& chests) {
        switch (type) {
            case StructureType::DesertPyramid:
                placeDesertPyramid(structCX, structCZ, structY, coordBaseMode, rng,
                                   worldMinX, worldMinZ, blocks, meta, chests);
                break;
            case StructureType::JunglePyramid:
                placeJunglePyramid(structCX, structCZ, structY, coordBaseMode, rng,
                                   worldMinX, worldMinZ, blocks, meta, chests);
                break;
            case StructureType::SwampHut:
                placeSwampHut(structCX, structCZ, structY, coordBaseMode, rng,
                              worldMinX, worldMinZ, blocks, meta, spawners);
                break;
        }
    }

    // ════════════════════════════════════════════════════════════════════
    // Desert Pyramid — Java: ComponentScatteredFeaturePieces$DesertPyramid
    // Size: 21 x 15 x 21, base Y = 64
    // ════════════════════════════════════════════════════════════════════
    void placeDesertPyramid(int structCX, int structCZ, int structY, int coordBaseMode,
                            JavaRNG& rng, int worldMinX, int worldMinZ,
                            int32_t* blocks, uint8_t* meta,
                            std::vector<Chunk::ChestInfo>& chests) {
        constexpr int sizeX = 21, sizeZ = 21;

        // Build bounding box
        int startX = structCX * 16 + 8;
        int startZ = structCZ * 16 + 8;
        StructureCtx ctx;
        ctx.coordBaseMode = coordBaseMode;

        switch (coordBaseMode) {
            case 0: ctx.bb = {startX, structY, startZ, startX + sizeX - 1, structY + 14, startZ + sizeZ - 1}; break;
            case 1: ctx.bb = {startX - sizeZ + 1, structY, startZ, startX, structY + 14, startZ + sizeX - 1}; break;
            case 2: ctx.bb = {startX, structY, startZ - sizeZ + 1, startX + sizeX - 1, structY + 14, startZ}; break;
            case 3: ctx.bb = {startX, structY, startZ, startX + sizeZ - 1, structY + 14, startZ + sizeX - 1}; break;
            default: ctx.bb = {startX, structY, startZ, startX + sizeX - 1, structY + 14, startZ + sizeZ - 1}; break;
        }

        int CBM = coordBaseMode;
        // Stair metadata
        int stairN = getMetadataWithOffset(SANDSTONE_STAIRS, 3, CBM);
        int stairS = getMetadataWithOffset(SANDSTONE_STAIRS, 2, CBM);
        int stairW = getMetadataWithOffset(SANDSTONE_STAIRS, 0, CBM);
        int stairE = getMetadataWithOffset(SANDSTONE_STAIRS, 1, CBM);
        int woolOrange = 1;
        int woolBlue = 11;

        // Line 52: Base platform
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 0, -4, 0, sizeX-1, 0, sizeZ-1, SANDSTONE, SANDSTONE, false);
        // Lines 53-56: Stepped pyramid shell
        for (int i = 1; i <= 9; ++i) {
            fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, i, i, i, sizeX-1-i, i, sizeZ-1-i, SANDSTONE, SANDSTONE, false);
            fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, i+1, i, i+1, sizeX-2-i, i, sizeZ-2-i, AIR, AIR, false);
        }
        // Lines 57-62: Foundation pillars
        for (int x = 0; x < sizeX; ++x)
            for (int z = 0; z < sizeZ; ++z)
                fillDownwards(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 0, x, -5, z);

        // Lines 69-80: Front towers
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 0, 0, 0, 4, 9, 4, SANDSTONE, AIR, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 1, 10, 1, 3, 10, 3, SANDSTONE, SANDSTONE, false);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, stairN, 2, 10, 0);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, stairS, 2, 10, 4);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, stairW, 0, 10, 2);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, stairE, 4, 10, 2);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, sizeX-5, 0, 0, sizeX-1, 9, 4, SANDSTONE, AIR, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, sizeX-4, 10, 1, sizeX-2, 10, 3, SANDSTONE, SANDSTONE, false);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, stairN, sizeX-3, 10, 0);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, stairS, sizeX-3, 10, 4);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, stairW, sizeX-5, 10, 2);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, stairE, sizeX-1, 10, 2);

        // Lines 81-93: Entrance arch and side corridors
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 8, 0, 0, 12, 4, 4, SANDSTONE, AIR, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 9, 1, 0, 11, 3, 4, AIR, AIR, false);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, 9, 1, 1);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, 9, 2, 1);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, 9, 3, 1);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, 10, 3, 1);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, 11, 3, 1);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, 11, 2, 1);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, 11, 1, 1);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 4, 1, 1, 8, 3, 3, SANDSTONE, AIR, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 4, 1, 2, 8, 2, 2, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 12, 1, 1, 16, 3, 3, SANDSTONE, AIR, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 12, 1, 2, 16, 2, 2, AIR, AIR, false);

        // Lines 94-105: Interior platform and pillars
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 5, 4, 5, sizeX-6, 4, sizeZ-6, SANDSTONE, SANDSTONE, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 9, 4, 9, 11, 4, 11, AIR, AIR, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 8, 1, 8, 8, 3, 8, SANDSTONE, 2, SANDSTONE, 2, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 12, 1, 8, 12, 3, 8, SANDSTONE, 2, SANDSTONE, 2, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 8, 1, 12, 8, 3, 12, SANDSTONE, 2, SANDSTONE, 2, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 12, 1, 12, 12, 3, 12, SANDSTONE, 2, SANDSTONE, 2, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 1, 1, 5, 4, 4, 11, SANDSTONE, SANDSTONE, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, sizeX-5, 1, 5, sizeX-2, 4, 11, SANDSTONE, SANDSTONE, false);

        // Lines 102-111: Side wings
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 6, 7, 9, 6, 7, 11, SANDSTONE, SANDSTONE, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, sizeX-7, 7, 9, sizeX-7, 7, 11, SANDSTONE, SANDSTONE, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 5, 5, 9, 5, 7, 11, SANDSTONE, 2, SANDSTONE, 2, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, sizeX-6, 5, 9, sizeX-6, 7, 11, SANDSTONE, 2, SANDSTONE, 2, false);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 5, 5, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 5, 6, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 6, 6, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, sizeX-6, 5, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, sizeX-6, 6, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, sizeX-7, 6, 10);

        // Lines 112-135: Side entrances with stairs and columns
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 2, 4, 4, 2, 6, 4, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, sizeX-3, 4, 4, sizeX-3, 6, 4, AIR, AIR, false);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, stairN, 2, 4, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, stairN, 2, 3, 4);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, stairN, sizeX-3, 4, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, stairN, sizeX-3, 3, 4);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 1, 1, 3, 2, 2, 3, SANDSTONE, SANDSTONE, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, sizeX-3, 1, 3, sizeX-2, 2, 3, SANDSTONE, SANDSTONE, false);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, 0, 1, 1, 2);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, 0, sizeX-2, 1, 2);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_SLAB, 1, 1, 2, 2);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_SLAB, 1, sizeX-2, 2, 2);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, stairE, 2, 1, 2);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE_STAIRS, stairW, sizeX-3, 1, 2);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 4, 3, 5, 4, 3, 18, SANDSTONE, SANDSTONE, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, sizeX-5, 3, 5, sizeX-5, 3, 17, SANDSTONE, SANDSTONE, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 3, 1, 5, 4, 2, 16, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, sizeX-6, 1, 5, sizeX-5, 2, 16, AIR, AIR, false);
        for (int n = 5; n <= 17; n += 2) {
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, 4, 1, n);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 1, 4, 2, n);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, sizeX-5, 1, n);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 1, sizeX-5, 2, n);
        }

        // Lines 136-148: Wool diamond pattern on floor
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, 10, 0, 7);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, 10, 0, 8);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, 9, 0, 9);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, 11, 0, 9);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, 8, 0, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, 12, 0, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, 7, 0, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, 13, 0, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, 9, 0, 11);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, 11, 0, 11);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, 10, 0, 12);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, 10, 0, 13);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolBlue, 10, 0, 10);

        // Lines 149-171: Side tower decorations (wool + sandstone pattern)
        for (int n = 0; n <= sizeX - 1; n += sizeX - 1) {
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n, 2, 1);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n, 2, 2);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n, 2, 3);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n, 3, 1);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n, 3, 2);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n, 3, 3);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n, 4, 1);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 1, n, 4, 2);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n, 4, 3);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n, 5, 1);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n, 5, 2);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n, 5, 3);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n, 6, 1);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 1, n, 6, 2);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n, 6, 3);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n, 7, 1);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n, 7, 2);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n, 7, 3);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n, 8, 1);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n, 8, 2);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n, 8, 3);
        }

        // Lines 172-194: Front face decorations
        for (int n = 2; n <= sizeX - 3; n += sizeX - 3 - 2) {
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n-1, 2, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n, 2, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n+1, 2, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n-1, 3, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n, 3, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n+1, 3, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n-1, 4, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 1, n, 4, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n+1, 4, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n-1, 5, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n, 5, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n+1, 5, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n-1, 6, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 1, n, 6, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n+1, 6, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n-1, 7, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n, 7, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, n+1, 7, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n-1, 8, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n, 8, 0);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, n+1, 8, 0);
        }

        // Lines 195-200: Upper front facade
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 8, 4, 0, 12, 6, 0, SANDSTONE, 2, SANDSTONE, 2, false);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 8, 6, 0);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 12, 6, 0);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, 9, 5, 0);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 1, 10, 5, 0);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, WOOL, woolOrange, 11, 5, 0);

        // Lines 201-228: Underground trap room
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 8, -14, 8, 12, -11, 12, SANDSTONE, 2, SANDSTONE, 2, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 8, -10, 8, 12, -10, 12, SANDSTONE, 1, SANDSTONE, 1, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 8, -9, 8, 12, -9, 12, SANDSTONE, 2, SANDSTONE, 2, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 8, -8, 8, 12, -1, 12, SANDSTONE, SANDSTONE, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 9, -11, 9, 11, -1, 11, AIR, AIR, false);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_PRESSURE_PLATE, 0, 10, -11, 10);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 9, -13, 9, 11, -13, 11, TNT, AIR, false);

        // Chest alcoves with sandstone detail
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 8, -11, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 8, -10, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 1, 7, -10, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, 7, -11, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 12, -11, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 12, -10, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 1, 13, -10, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, 13, -11, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 10, -11, 8);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 10, -10, 8);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 1, 10, -10, 7);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, 10, -11, 7);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 10, -11, 12);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 10, -10, 12);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 1, 10, -10, 13);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SANDSTONE, 2, 10, -11, 13);

        // Lines 224-229: 4 chests using Direction.offsetX/Z
        // Direction offsets: {0: S(0,1), 1: W(-1,0), 2: N(0,-1), 3: E(1,0)}
        for (int i = 0; i < 4; ++i) {
            int ox = DIR_OFFSET_X[i] * 2;
            int oz = DIR_OFFSET_Z[i] * 2;
            int chestLX = 10 + ox;
            int chestLZ = 10 + oz;
            int wx = ctx.getXWithOffset(chestLX, chestLZ);
            int wy = ctx.getYWithOffset(-11);
            int wz = ctx.getZWithOffset(chestLX, chestLZ);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, CHEST, 0, chestLX, -11, chestLZ);
            Chunk::ChestInfo ci;
            ci.x = wx; ci.y = wy; ci.z = wz;
            ci.lootTable = "desert_pyramid";
            chests.push_back(std::move(ci));
        }
    } // end placeDesertPyramid

    // ════════════════════════════════════════════════════════════════════
    // Jungle Pyramid — Java: ComponentScatteredFeaturePieces$JunglePyramid
    // Size: 12 x 10 x 15
    // ════════════════════════════════════════════════════════════════════
    void placeJunglePyramid(int structCX, int structCZ, int structY, int coordBaseMode,
                            JavaRNG& rng, int worldMinX, int worldMinZ,
                            int32_t* blocks, uint8_t* meta,
                            std::vector<Chunk::ChestInfo>& chests) {
        constexpr int sizeX = 12, sizeZ = 15;
        int startX = structCX * 16 + 8;
        int startZ = structCZ * 16 + 8;
        StructureCtx ctx;
        ctx.coordBaseMode = coordBaseMode;
        int CBM = coordBaseMode;

        switch (CBM) {
            case 0: ctx.bb = {startX, structY, startZ, startX + sizeX - 1, structY + 9, startZ + sizeZ - 1}; break;
            case 1: ctx.bb = {startX - sizeZ + 1, structY, startZ, startX, structY + 9, startZ + sizeX - 1}; break;
            case 2: ctx.bb = {startX, structY, startZ - sizeZ + 1, startX + sizeX - 1, structY + 9, startZ}; break;
            case 3: ctx.bb = {startX, structY, startZ, startX + sizeZ - 1, structY + 9, startZ + sizeX - 1}; break;
            default: ctx.bb = {startX, structY, startZ, startX + sizeX - 1, structY + 9, startZ + sizeZ - 1}; break;
        }

        int stairN = getMetadataWithOffset(STONE_STAIRS, 3, CBM);
        int stairS = getMetadataWithOffset(STONE_STAIRS, 2, CBM);
        int stairW = getMetadataWithOffset(STONE_STAIRS, 0, CBM);
        int stairE = getMetadataWithOffset(STONE_STAIRS, 1, CBM);

        // Lines 62-74: Main structure shell (randomized stones)
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 0, -4, 0, sizeX-1, 0, sizeZ-1);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 2, 1, 2, 9, 2, 2);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 2, 1, 12, 9, 2, 12);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 2, 1, 3, 2, 2, 11);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 9, 1, 3, 9, 2, 11);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 1, 3, 1, 10, 6, 1);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 1, 3, 13, 10, 6, 13);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 1, 3, 2, 1, 6, 12);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 10, 3, 2, 10, 6, 12);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 2, 3, 2, 9, 3, 12);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 2, 6, 2, 9, 6, 12);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 3, 7, 3, 8, 7, 11);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 4, 8, 4, 7, 8, 10);

        // Lines 75-83: Interior air carving
        fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, 3, 1, 3, 8, 2, 11);
        fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, 4, 3, 6, 7, 3, 9);
        fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, 2, 4, 2, 9, 5, 12);
        fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, 4, 6, 5, 7, 6, 9);
        fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, 5, 7, 6, 6, 7, 8);
        fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, 5, 1, 2, 6, 2, 2);
        fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, 5, 2, 12, 6, 2, 12);
        fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, 5, 5, 1, 6, 5, 1);
        fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, 5, 5, 13, 6, 5, 13);

        // Lines 84-87: Window openings
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 1, 5, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 10, 5, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 1, 5, 9);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 10, 5, 9);

        // Lines 88-130: Pillars, decorations, and stairs
        for (int n = 0; n <= 14; n += 14) {
            fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 2, 4, n, 2, 5, n);
            fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 4, 4, n, 4, 5, n);
            fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 7, 4, n, 7, 5, n);
            fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 9, 4, n, 9, 5, n);
        }
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 5, 6, 0, 6, 6, 0);
        for (int n = 0; n <= 11; n += 11) {
            for (int i = 2; i <= 12; i += 2)
                fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, n, 4, i, n, 5, i);
            fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, n, 6, 5, n, 6, 5);
            fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, n, 6, 9, n, 6, 9);
        }
        // Corner pillars
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 2, 7, 2, 2, 9, 2);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 9, 7, 2, 9, 9, 2);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 2, 7, 12, 2, 9, 12);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 9, 7, 12, 9, 9, 12);
        // Roof corners
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 4, 9, 4, 4, 9, 4);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 7, 9, 4, 7, 9, 4);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 4, 9, 10, 4, 9, 10);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 7, 9, 10, 7, 9, 10);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 5, 9, 7, 6, 9, 7);
        // Roof stairs
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairN, 5, 9, 6);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairN, 6, 9, 6);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairS, 5, 9, 8);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairS, 6, 9, 8);
        // Entrance stairs
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairN, 4, 0, 0);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairN, 5, 0, 0);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairN, 6, 0, 0);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairN, 7, 0, 0);
        // Interior stairs
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairN, 4, 1, 8);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairN, 4, 2, 9);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairN, 4, 3, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairN, 7, 1, 8);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairN, 7, 2, 9);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairN, 7, 3, 10);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 4, 1, 9, 4, 1, 9);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 7, 1, 9, 7, 1, 9);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 4, 1, 10, 7, 2, 10);
        // Balcony
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 5, 4, 5, 6, 4, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairW, 4, 4, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairE, 7, 4, 5);
        // Descending stairs
        for (int n = 0; n < 4; ++n) {
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairS, 5, 0 - n, 6 + n);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_STAIRS, stairS, 6, 0 - n, 6 + n);
            fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, 5, 0 - n, 7 + n, 6, 0 - n, 9 + n);
        }

        // Lines 136-148: Underground passages
        fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, 1, -3, 12, 10, -1, 13);
        fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, 1, -3, 1, 3, -1, 13);
        fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, 1, -3, 1, 9, -1, 5);
        for (int n = 1; n <= 13; n += 2)
            fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 1, -3, n, 1, -2, n);
        for (int n = 2; n <= 12; n += 2)
            fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 1, -1, n, 3, -1, n);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 2, -2, 1, 5, -2, 1);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 7, -2, 1, 9, -2, 1);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 6, -3, 1, 6, -3, 1);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 6, -1, 1, 6, -1, 1);

        // Lines 149-164: First tripwire trap + dispenser
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, TRIPWIRE_HOOK,
                   getMetadataWithOffset(TRIPWIRE_HOOK, 3, CBM) | 4, 1, -3, 8);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, TRIPWIRE_HOOK,
                   getMetadataWithOffset(TRIPWIRE_HOOK, 1, CBM) | 4, 4, -3, 8);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, TRIPWIRE, 4, 2, -3, 8);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, TRIPWIRE, 4, 3, -3, 8);
        // Redstone wire
        for (int z = 7; z >= 1; --z)
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, REDSTONE_WIRE, 0, 5, -3, z);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, REDSTONE_WIRE, 0, 4, -3, 1);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, MOSSY_COBBLESTONE, 0, 3, -3, 1);
        // Dispenser (facing north = 2)
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, DISPENSER,
                   getMetadataWithOffset(DISPENSER, 2, CBM), 3, -2, 1);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, VINE, 15, 3, -2, 2);

        // Lines 166-178: Second tripwire trap + dispenser
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, TRIPWIRE_HOOK,
                   getMetadataWithOffset(TRIPWIRE_HOOK, 2, CBM) | 4, 7, -3, 1);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, TRIPWIRE_HOOK,
                   getMetadataWithOffset(TRIPWIRE_HOOK, 0, CBM) | 4, 7, -3, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, TRIPWIRE, 4, 7, -3, 2);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, TRIPWIRE, 4, 7, -3, 3);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, TRIPWIRE, 4, 7, -3, 4);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, REDSTONE_WIRE, 0, 8, -3, 6);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, REDSTONE_WIRE, 0, 9, -3, 6);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, REDSTONE_WIRE, 0, 9, -3, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, MOSSY_COBBLESTONE, 0, 9, -3, 4);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, REDSTONE_WIRE, 0, 9, -2, 4);
        // Dispenser (facing south = 4... wait, Java says direction 4 which is UP for dispenser)
        // Java line 177: generateStructureDispenserContents(..., 9, -2, 3, 4, ...)
        // The '4' is the facing direction passed to getMetadataWithOffset(Blocks.dispenser, 4)
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, DISPENSER,
                   getMetadataWithOffset(DISPENSER, 4, CBM), 9, -2, 3);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, VINE, 15, 8, -1, 3);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, VINE, 15, 8, -2, 3);

        // Lines 181-183: Main chest (behind vines)
        {
            int wx = ctx.getXWithOffset(8, 3);
            int wy = ctx.getYWithOffset(-3);
            int wz = ctx.getZWithOffset(8, 3);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, CHEST, 0, 8, -3, 3);
            Chunk::ChestInfo ci;
            ci.x = wx; ci.y = wy; ci.z = wz;
            ci.lootTable = "jungle_pyramid";
            chests.push_back(std::move(ci));
        }

        // Lines 184-192: Mossy cobblestone supports
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, MOSSY_COBBLESTONE, 0, 9, -3, 2);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, MOSSY_COBBLESTONE, 0, 8, -3, 1);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, MOSSY_COBBLESTONE, 0, 4, -3, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, MOSSY_COBBLESTONE, 0, 5, -2, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, MOSSY_COBBLESTONE, 0, 5, -1, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, MOSSY_COBBLESTONE, 0, 6, -3, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, MOSSY_COBBLESTONE, 0, 7, -2, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, MOSSY_COBBLESTONE, 0, 7, -1, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, MOSSY_COBBLESTONE, 0, 8, -3, 5);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 9, -1, 1, 9, -1, 5);

        // Lines 194-213: Hidden chest room with piston puzzle
        fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, 8, -3, 8, 10, -1, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 3, 8, -2, 11);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 3, 9, -2, 11);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 3, 10, -2, 11);
        // Levers with inverted metadata
        int leverMeta = invertLeverMetadata(getMetadataWithOffset(LEVER, 2, CBM));
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, LEVER, leverMeta, 8, -2, 12);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, LEVER, leverMeta, 9, -2, 12);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, LEVER, leverMeta, 10, -2, 12);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 8, -3, 8, 8, -3, 10);
        fillWithRandomizedStones(ctx, blocks, meta, worldMinX, worldMinZ, rng, 10, -3, 8, 10, -3, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, MOSSY_COBBLESTONE, 0, 10, -2, 9);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, REDSTONE_WIRE, 0, 8, -2, 9);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, REDSTONE_WIRE, 0, 8, -2, 10);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, REDSTONE_WIRE, 0, 10, -1, 9);
        // Sticky pistons
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STICKY_PISTON, 1, 9, -2, 8);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STICKY_PISTON,
                   getMetadataWithOffset(STICKY_PISTON, 4, CBM), 10, -2, 8);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STICKY_PISTON,
                   getMetadataWithOffset(STICKY_PISTON, 4, CBM), 10, -1, 8);
        // Repeater
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, UNPOWERED_REPEATER,
                   getMetadataWithOffset(UNPOWERED_REPEATER, 2, CBM), 10, -2, 10);

        // Hidden chest
        {
            int wx = ctx.getXWithOffset(9, 10);
            int wy = ctx.getYWithOffset(-3);
            int wz = ctx.getZWithOffset(9, 10);
            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, CHEST, 0, 9, -3, 10);
            Chunk::ChestInfo ci;
            ci.x = wx; ci.y = wy; ci.z = wz;
            ci.lootTable = "jungle_pyramid";
            chests.push_back(std::move(ci));
        }
    } // end placeJunglePyramid

    // ════════════════════════════════════════════════════════════════════
    // Swamp Hut — Java: ComponentScatteredFeaturePieces$SwampHut
    // Size: 7 x 5 x 9
    // ════════════════════════════════════════════════════════════════════
    void placeSwampHut(int structCX, int structCZ, int structY, int coordBaseMode,
                       JavaRNG& rng, int worldMinX, int worldMinZ,
                       int32_t* blocks, uint8_t* meta,
                       std::vector<Chunk::SpawnerInfo>& spawners) {
        constexpr int sizeX = 7, sizeZ = 9;
        int startX = structCX * 16 + 8;
        int startZ = structCZ * 16 + 8;
        StructureCtx ctx;
        ctx.coordBaseMode = coordBaseMode;
        int CBM = coordBaseMode;

        switch (CBM) {
            case 0: ctx.bb = {startX, structY, startZ, startX + sizeX - 1, structY + 4, startZ + sizeZ - 1}; break;
            case 1: ctx.bb = {startX - sizeZ + 1, structY, startZ, startX, structY + 4, startZ + sizeX - 1}; break;
            case 2: ctx.bb = {startX, structY, startZ - sizeZ + 1, startX + sizeX - 1, structY + 4, startZ}; break;
            case 3: ctx.bb = {startX, structY, startZ, startX + sizeZ - 1, structY + 4, startZ + sizeX - 1}; break;
            default: ctx.bb = {startX, structY, startZ, startX + sizeX - 1, structY + 4, startZ + sizeZ - 1}; break;
        }

        // Lines 45-55: Floor, walls, and pillars (spruce planks = metadata 1)
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 1, 1, 1, 5, 1, 7, OAK_PLANKS, 1, OAK_PLANKS, 1, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 1, 4, 2, 5, 4, 7, OAK_PLANKS, 1, OAK_PLANKS, 1, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 2, 1, 0, 4, 1, 0, OAK_PLANKS, 1, OAK_PLANKS, 1, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 2, 2, 2, 3, 3, 2, OAK_PLANKS, 1, OAK_PLANKS, 1, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 1, 2, 3, 1, 3, 6, OAK_PLANKS, 1, OAK_PLANKS, 1, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 5, 2, 3, 5, 3, 6, OAK_PLANKS, 1, OAK_PLANKS, 1, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 2, 2, 7, 4, 3, 7, OAK_PLANKS, 1, OAK_PLANKS, 1, false);
        // Log pillars
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 1, 0, 2, 1, 3, 2, OAK_LOG, OAK_LOG, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 5, 0, 2, 5, 3, 2, OAK_LOG, OAK_LOG, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 1, 0, 7, 1, 3, 7, OAK_LOG, OAK_LOG, false);
        fillWithBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 5, 0, 7, 5, 3, 7, OAK_LOG, OAK_LOG, false);

        // Lines 56-65: Interior details
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, FENCE, 0, 2, 3, 2);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, FENCE, 0, 3, 3, 7);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 1, 3, 4);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 5, 3, 4);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, 5, 3, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, FLOWER_POT, 7, 1, 3, 5);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, CRAFTING_TABLE, 0, 3, 2, 6);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, CAULDRON, 0, 4, 2, 6);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, FENCE, 0, 1, 2, 1);
        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, FENCE, 0, 5, 2, 1);

        // Lines 66-73: Roof (spruce stairs)
        int stairN = getMetadataWithOffset(OAK_STAIRS, 3, CBM);
        int stairE = getMetadataWithOffset(OAK_STAIRS, 1, CBM);
        int stairW = getMetadataWithOffset(OAK_STAIRS, 0, CBM);
        int stairS = getMetadataWithOffset(OAK_STAIRS, 2, CBM);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 0, 4, 1, 6, 4, 1, SPRUCE_STAIRS, stairN, SPRUCE_STAIRS, stairN, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 0, 4, 2, 0, 4, 7, SPRUCE_STAIRS, stairW, SPRUCE_STAIRS, stairW, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 6, 4, 2, 6, 4, 7, SPRUCE_STAIRS, stairE, SPRUCE_STAIRS, stairE, false);
        fillWithMetadataBlocks(ctx, blocks, meta, worldMinX, worldMinZ, 0, 4, 8, 6, 4, 8, SPRUCE_STAIRS, stairS, SPRUCE_STAIRS, stairS, false);

        // Lines 74-78: Leg supports
        for (int z = 2; z <= 7; z += 5) {
            for (int x = 1; x <= 5; x += 4) {
                fillDownwards(ctx, blocks, meta, worldMinX, worldMinZ, OAK_LOG, 0, x, -1, z);
            }
        }

        // Lines 79-85: Witch spawn
        {
            int wx = ctx.getXWithOffset(2, 5);
            int wy = ctx.getYWithOffset(2);
            int wz = ctx.getZWithOffset(2, 5);
            int lx = wx - worldMinX;
            int lz = wz - worldMinZ;
            if (lx >= 0 && lx <= 15 && lz >= 0 && lz <= 15 && wy >= 1 && wy <= 254) {
                Chunk::SpawnerInfo si;
                si.x = wx; si.y = wy; si.z = wz;
                si.entityId = "Witch";
                spawners.push_back(std::move(si));
            }
        }
    } // end placeSwampHut

}; // class MapGenScatteredFeature

} // namespace mccpp
