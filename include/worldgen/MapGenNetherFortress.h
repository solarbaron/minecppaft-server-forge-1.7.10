/**
 * MapGenNetherFortress.h — Nether Fortress (Nether Bridge) structure generation.
 *
 * Java reference: net.minecraft.world.gen.structure.MapGenNetherBridge
 *               + StructureNetherBridgePieces (all inner classes)
 *               + MapGenNetherBridge$Start
 *
 * Generates nether fortresses with 14 piece types:
 *   Primary:   Straight, Crossing3, Crossing, Stairs, Throne, Entrance
 *   Secondary: Corridor5, Crossing2, Corridor2, Corridor, Corridor3, Corridor4, NetherStalkRoom
 *   Filler:    End (dead-end cap)
 *
 * Grid-based spawning: coordinates >> 4, 1/3 chance, deterministic offset.
 * Recursive piece assembly with depth limit 30, distance limit 112.
 * Height range: Y 48-70.
 *
 * Thread safety: Each NetherGenerator instance owns its own MapGenNetherFortress.
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

class MapGenNetherFortress {
public:
    explicit MapGenNetherFortress() = default;

    /**
     * Generate fortress structures in a Nether chunk.
     * blockArray: int32_t[32768] indexed as (x*16+z)*128+y (128-height Nether format)
     */
    void generate(int64_t worldSeed, int32_t chunkX, int32_t chunkZ,
                  int32_t* blockArray, uint8_t* metaArray,
                  std::vector<Chunk::SpawnerInfo>& spawners,
                  std::vector<Chunk::ChestInfo>& chests) {
        worldSeed_ = worldSeed;
        int worldMinX = chunkX * 16;
        int worldMinZ = chunkZ * 16;

        // Scan 8-chunk radius for potential fortress origins
        for (int32_t cx = chunkX - 8; cx <= chunkX + 8; ++cx) {
            for (int32_t cz = chunkZ - 8; cz <= chunkZ + 8; ++cz) {
                if (!canSpawnStructureAtCoords(cx, cz)) continue;

                // Generate fortress pieces deterministically
                JavaRNG rng(getFortressSeed(cx, cz));
                std::vector<FortressPiece> pieces;
                generateFortress(cx, cz, rng, pieces);

                // Place overlapping pieces into this chunk
                for (auto& piece : pieces) {
                    if (piece.bb.maxX < worldMinX || piece.bb.minX > worldMinX + 15) continue;
                    if (piece.bb.maxZ < worldMinZ || piece.bb.minZ > worldMinZ + 15) continue;
                    placePiece(piece, worldMinX, worldMinZ, blockArray, metaArray, spawners, chests);
                }
            }
        }
    }

private:
    int64_t worldSeed_ = 0;

    // Block IDs
    static constexpr int32_t AIR = 0;
    static constexpr int32_t LAVA = 11;
    static constexpr int32_t FLOWING_LAVA = 10;
    static constexpr int32_t SOUL_SAND = 88;
    static constexpr int32_t NETHER_BRICK = 112;
    static constexpr int32_t NETHER_BRICK_FENCE = 113;
    static constexpr int32_t NETHER_BRICK_STAIRS = 114;
    static constexpr int32_t NETHER_WART = 115;
    static constexpr int32_t SPAWNER = 52;
    static constexpr int32_t CHEST = 54;

    // ── Java-like RNG ──
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
        int32_t nextInt() { return next(32); }
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
    };

    // ── Bounding Box ──
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
        bool isVecInside(int x, int y, int z) const {
            return x >= minX && x <= maxX && y >= minY && y <= maxY && z >= minZ && z <= maxZ;
        }
    };

    // Java: StructureBoundingBox.getComponentToAddBoundingBox
    static BBox getComponentBB(int x, int y, int z, int offX, int offY, int offZ,
                                int sizeX, int sizeY, int sizeZ, int facing) {
        switch (facing) {
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

    // Java: isAboveGround — bb.minY > 10
    static bool isAboveGround(const BBox& bb) {
        return bb.minY > 10;
    }

    // ── Coordinate transforms ──
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

    // ── Block helpers (128-height Nether indexing) ──
    static inline void setBlockAt128(int32_t* blocks, uint8_t* meta,
                                     int worldMinX, int worldMinZ,
                                     int wx, int wy, int wz,
                                     int32_t blockId, uint8_t blockMeta = 0) {
        int lx = wx - worldMinX;
        int lz = wz - worldMinZ;
        if (lx < 0 || lx > 15 || lz < 0 || lz > 15 || wy < 1 || wy > 126) return;
        int idx = (lx * 16 + lz) * 128 + wy;
        blocks[idx] = blockId;
        if (meta) meta[idx] = blockMeta;
    }

    static inline int32_t getBlockAt128(const int32_t* blocks,
                                        int worldMinX, int worldMinZ,
                                        int wx, int wy, int wz) {
        int lx = wx - worldMinX;
        int lz = wz - worldMinZ;
        if (lx < 0 || lx > 15 || lz < 0 || lz > 15 || wy < 0 || wy > 127) return 87; // netherrack
        return blocks[(lx * 16 + lz) * 128 + wy];
    }

    // ── Metadata rotation for nether brick stairs ──
    static int getMetadataWithOffset(int32_t blockId, int meta, int coordBaseMode) {
        if (blockId == NETHER_BRICK_STAIRS) {
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
        return meta;
    }

    // ── Placement helpers ──
    void placeBlock(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                    int worldMinX, int worldMinZ,
                    int32_t blockId, uint8_t blockMeta, int lx, int ly, int lz) {
        int wx = ctx.getXWithOffset(lx, lz);
        int wy = ctx.getYWithOffset(ly);
        int wz = ctx.getZWithOffset(lx, lz);
        setBlockAt128(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, blockId, blockMeta);
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
                        if (getBlockAt128(blocks, worldMinX, worldMinZ, wx, wy, wz) == AIR) continue;
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
        while (wy > 1) {
            int32_t existing = getBlockAt128(blocks, worldMinX, worldMinZ, wx, wy, wz);
            if (existing != AIR && existing != LAVA && existing != FLOWING_LAVA) break;
            setBlockAt128(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, blockId, blockMeta);
            --wy;
        }
    }

    // ── Piece types ──
    enum class PieceType {
        // Primary
        Straight, Crossing3, Crossing, Stairs, Throne, Entrance,
        // Secondary
        Corridor5, Crossing2, Corridor2, Corridor, Corridor3, Corridor4, NetherStalkRoom,
        // Filler
        End
    };

    struct FortressPiece {
        PieceType type;
        BBox bb;
        int coordBaseMode;
        int componentType; // depth
        // Type-specific data
        bool hasChest = false;   // Corridor, Corridor2
        bool hasSpawner = false; // Throne
        int fillSeed = 0;       // End
    };

    // ── PieceWeight ──
    struct PieceWeight {
        PieceType type;
        int weight;
        int instancesSpawned = 0;
        int instancesLimit;
        bool allowInRow; // field_78825_e — can be selected consecutively

        PieceWeight(PieceType t, int w, int limit, bool inRow = false)
            : type(t), weight(w), instancesLimit(limit), allowInRow(inRow) {}
        bool canSpawnMore() const { return instancesLimit == 0 || instancesSpawned < instancesLimit; }
        bool canSpawnAtDepth(int /*depth*/) const { return canSpawnMore(); }
    };

    // ── Fortress position check ──
    // Java: MapGenNetherBridge.canSpawnStructureAtCoords
    bool canSpawnStructureAtCoords(int chunkX, int chunkZ) const {
        int n3 = chunkX >> 4;  // divide by 16
        int n4 = chunkZ >> 4;
        JavaRNG rng(static_cast<int64_t>(n3 ^ (n4 << 4)) ^ worldSeed_);
        rng.nextInt(); // consume one
        if (rng.nextInt(3) != 0) return false;
        if (chunkX != (n3 << 4) + 4 + rng.nextInt(8)) return false;
        return chunkZ == (n4 << 4) + 4 + rng.nextInt(8);
    }

    int64_t getFortressSeed(int cx, int cz) const {
        return worldSeed_ + static_cast<int64_t>(cx) * 341873128712LL
                          + static_cast<int64_t>(cz) * 132897987541LL;
    }

    // ── Check intersection ──
    static bool findIntersecting(const std::vector<FortressPiece>& pieces, const BBox& bb) {
        for (auto& p : pieces) {
            if (p.bb.intersects(bb)) return true;
        }
        return false;
    }

    // ── Navigation: getNextComponentNormal/X/Z ──
    // Java: StructureNetherBridgePieces$Piece
    void getNextComponentNormal(const FortressPiece& from, int offX, int offY,
                                int& outX, int& outY, int& outZ, int& outFacing) {
        outFacing = from.coordBaseMode;
        switch (from.coordBaseMode) {
            case 2: outX = from.bb.minX + offX; outY = from.bb.minY + offY; outZ = from.bb.minZ - 1; break;
            case 0: outX = from.bb.minX + offX; outY = from.bb.minY + offY; outZ = from.bb.maxZ + 1; break;
            case 1: outX = from.bb.minX - 1;    outY = from.bb.minY + offY; outZ = from.bb.minZ + offX; break;
            case 3: outX = from.bb.maxX + 1;    outY = from.bb.minY + offY; outZ = from.bb.minZ + offX; break;
            default: outX = outY = outZ = 0; break;
        }
    }

    void getNextComponentX(const FortressPiece& from, int offY, int offX,
                           int& outX, int& outY, int& outZ, int& outFacing) {
        switch (from.coordBaseMode) {
            case 2: case 0:
                outFacing = 1;
                outX = from.bb.minX - 1;
                outY = from.bb.minY + offY;
                outZ = from.bb.minZ + offX;
                break;
            case 1: case 3:
                outFacing = 2;
                outX = from.bb.minX + offX;
                outY = from.bb.minY + offY;
                outZ = from.bb.minZ - 1;
                break;
            default: outFacing = 0; outX = outY = outZ = 0; break;
        }
    }

    void getNextComponentZ(const FortressPiece& from, int offY, int offX,
                           int& outX, int& outY, int& outZ, int& outFacing) {
        switch (from.coordBaseMode) {
            case 2: case 0:
                outFacing = 3;
                outX = from.bb.maxX + 1;
                outY = from.bb.minY + offY;
                outZ = from.bb.minZ + offX;
                break;
            case 1: case 3:
                outFacing = 0;
                outX = from.bb.minX + offX;
                outY = from.bb.minY + offY;
                outZ = from.bb.maxZ + 1;
                break;
            default: outFacing = 0; outX = outY = outZ = 0; break;
        }
    }
    // ── Try to create a piece of given type at position ──
    bool tryCreatePiece(PieceType type, int depth, JavaRNG& rng,
                        int x, int y, int z, int facing,
                        const std::vector<FortressPiece>& pieces,
                        FortressPiece& out) {
        BBox bb;
        out = FortressPiece{};
        switch (type) {
            case PieceType::Straight:
                bb = getComponentBB(x, y, z, -1, -3, 0, 5, 10, 19, facing); break;
            case PieceType::Crossing3:
                bb = getComponentBB(x, y, z, -8, -3, 0, 19, 10, 19, facing); break;
            case PieceType::Crossing:
                bb = getComponentBB(x, y, z, -2, 0, 0, 7, 9, 7, facing); break;
            case PieceType::Stairs:
                bb = getComponentBB(x, y, z, -2, 0, 0, 7, 11, 7, facing); break;
            case PieceType::Throne:
                bb = getComponentBB(x, y, z, -2, 0, 0, 7, 8, 9, facing); break;
            case PieceType::Entrance:
                bb = getComponentBB(x, y, z, -5, -3, 0, 13, 14, 13, facing); break;
            case PieceType::Corridor5:
                bb = getComponentBB(x, y, z, -1, 0, 0, 5, 7, 5, facing); break;
            case PieceType::Crossing2:
                bb = getComponentBB(x, y, z, -1, 0, 0, 5, 7, 5, facing); break;
            case PieceType::Corridor2:
                bb = getComponentBB(x, y, z, -1, 0, 0, 5, 7, 5, facing); break;
            case PieceType::Corridor:
                bb = getComponentBB(x, y, z, -1, 0, 0, 5, 7, 5, facing); break;
            case PieceType::Corridor3:
                bb = getComponentBB(x, y, z, -1, -7, 0, 5, 14, 10, facing); break;
            case PieceType::Corridor4:
                bb = getComponentBB(x, y, z, -3, 0, 0, 9, 7, 9, facing); break;
            case PieceType::NetherStalkRoom:
                bb = getComponentBB(x, y, z, -5, -3, 0, 13, 14, 13, facing); break;
            case PieceType::End:
                bb = getComponentBB(x, y, z, -1, -3, 0, 5, 10, 8, facing); break;
        }
        if (!isAboveGround(bb) || findIntersecting(pieces, bb)) return false;
        out.type = type;
        out.bb = bb;
        out.coordBaseMode = facing;
        out.componentType = depth;
        // Type-specific init
        if (type == PieceType::Corridor || type == PieceType::Corridor2) {
            out.hasChest = rng.nextInt(3) == 0;
        }
        if (type == PieceType::End) {
            out.fillSeed = rng.nextInt();
        }
        return true;
    }

    // ── Get exits for buildComponent ──
    void getExits(const FortressPiece& piece, const FortressPiece& start,
                  std::vector<std::tuple<int,int,int,int,bool>>& exits) {
        int ox, oy, oz, of;
        switch (piece.type) {
            case PieceType::Straight:
                getNextComponentNormal(piece, 1, 3, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, false);
                break;
            case PieceType::Crossing3:
                getNextComponentNormal(piece, 8, 3, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, false);
                getNextComponentX(piece, 3, 8, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, false);
                getNextComponentZ(piece, 3, 8, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, false);
                break;
            case PieceType::Crossing:
                getNextComponentNormal(piece, 2, 0, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, false);
                getNextComponentX(piece, 0, 2, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, false);
                getNextComponentZ(piece, 0, 2, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, false);
                break;
            case PieceType::Stairs:
                getNextComponentZ(piece, 6, 2, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, false);
                break;
            case PieceType::Throne:
                // No exits (dead end with blaze spawner)
                break;
            case PieceType::Entrance:
                getNextComponentNormal(piece, 5, 3, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, true);
                break;
            case PieceType::Corridor5:
                getNextComponentNormal(piece, 1, 0, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, true);
                break;
            case PieceType::Crossing2:
                getNextComponentNormal(piece, 1, 0, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, true);
                getNextComponentX(piece, 0, 1, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, true);
                getNextComponentZ(piece, 0, 1, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, true);
                break;
            case PieceType::Corridor2:
                getNextComponentZ(piece, 0, 1, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, true);
                break;
            case PieceType::Corridor:
                getNextComponentX(piece, 0, 1, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, true);
                break;
            case PieceType::Corridor3:
                getNextComponentNormal(piece, 1, 0, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, true);
                break;
            case PieceType::Corridor4: {
                int n = 1;
                if (piece.coordBaseMode == 1 || piece.coordBaseMode == 2) n = 5;
                getNextComponentX(piece, 0, n, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, true);
                getNextComponentZ(piece, 0, n, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, true);
                break;
            }
            case PieceType::NetherStalkRoom:
                getNextComponentNormal(piece, 5, 3, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, true);
                getNextComponentNormal(piece, 5, 11, ox, oy, oz, of);
                exits.emplace_back(ox, oy, oz, of, true);
                break;
            case PieceType::End:
                break; // No exits
        }
    }

    // ── Recursive fortress generation ──
    // Java: MapGenNetherBridge$Start constructor + buildComponent loop
    void generateFortress(int startCX, int startCZ, JavaRNG& rng,
                          std::vector<FortressPiece>& pieces) {
        // Primary and secondary piece weights (Java: StructureNetherBridgePieces)
        std::vector<PieceWeight> primaryWeights = {
            {PieceType::Straight,   30, 0, true},
            {PieceType::Crossing3,  10, 4},
            {PieceType::Crossing,   10, 4},
            {PieceType::Stairs,     10, 3},
            {PieceType::Throne,      5, 2},
            {PieceType::Entrance,    5, 1},
        };
        std::vector<PieceWeight> secondaryWeights = {
            {PieceType::Corridor5,       25, 0, true},
            {PieceType::Crossing2,       15, 5},
            {PieceType::Corridor2,        5, 10},
            {PieceType::Corridor,         5, 10},
            {PieceType::Corridor3,       10, 3, true},
            {PieceType::Corridor4,        7, 2},
            {PieceType::NetherStalkRoom,  5, 2},
        };

        // Start piece: Crossing3 at (chunkX*16+2, 64, chunkZ*16+2)
        int startX = (startCX << 4) + 2;
        int startZ = (startCZ << 4) + 2;
        int facing = rng.nextInt(4);

        FortressPiece startPiece;
        startPiece.type = PieceType::Crossing3;
        startPiece.coordBaseMode = facing;
        startPiece.componentType = 0;
        switch (facing) {
            case 0: case 2:
                startPiece.bb = BBox(startX, 64, startZ, startX + 18, 73, startZ + 18); break;
            default:
                startPiece.bb = BBox(startX, 64, startZ, startX + 18, 73, startZ + 18); break;
        }
        pieces.push_back(startPiece);

        // Track the last-selected piece weight for consecutive selection prevention
        int lastPrimaryIdx = -1;
        int lastSecondaryIdx = -1;

        // Pending list (indices into pieces that need buildComponent called)
        std::vector<int> pending;
        pending.push_back(0);

        // Also track the "field_74967_d" list — pieces whose buildComponent needs calling
        std::vector<int> delayedPending;

        // First buildComponent for the start piece
        {
            std::vector<std::tuple<int,int,int,int,bool>> exits;
            getExits(pieces[0], pieces[0], exits);
            for (auto& [ex, ey, ez, ef, isSecondary] : exits) {
                if (std::abs(ex - pieces[0].bb.minX) > 112 ||
                    std::abs(ez - pieces[0].bb.minZ) > 112) continue;
                auto& weights = isSecondary ? secondaryWeights : primaryWeights;
                int& lastIdx = isSecondary ? lastSecondaryIdx : lastPrimaryIdx;
                FortressPiece newPiece;
                if (selectAndCreatePiece(weights, lastIdx, pieces, rng, ex, ey, ez, ef, 1, newPiece)) {
                    int idx = static_cast<int>(pieces.size());
                    pieces.push_back(newPiece);
                    delayedPending.push_back(idx);
                }
            }
        }

        // Process delayed pending (Java: while (!field_74967_d.isEmpty()))
        while (!delayedPending.empty()) {
            int idx = rng.nextInt(static_cast<int>(delayedPending.size()));
            int pieceIdx = delayedPending[idx];
            delayedPending.erase(delayedPending.begin() + idx);

            auto& piece = pieces[pieceIdx];
            int depth = piece.componentType + 1;

            std::vector<std::tuple<int,int,int,int,bool>> exits;
            getExits(piece, pieces[0], exits);
            for (auto& [ex, ey, ez, ef, isSecondary] : exits) {
                if (std::abs(ex - pieces[0].bb.minX) > 112 ||
                    std::abs(ez - pieces[0].bb.minZ) > 112) continue;
                auto& weights = isSecondary ? secondaryWeights : primaryWeights;
                int& lastIdx = isSecondary ? lastSecondaryIdx : lastPrimaryIdx;
                FortressPiece newPiece;
                if (selectAndCreatePiece(weights, lastIdx, pieces, rng, ex, ey, ez, ef, depth, newPiece)) {
                    int newIdx = static_cast<int>(pieces.size());
                    pieces.push_back(newPiece);
                    delayedPending.push_back(newIdx);
                }
            }
        }

        // Update bounding box and set random height (Java: setRandomHeight 48-70)
        if (!pieces.empty()) {
            // Find overall bounding box
            BBox total = pieces[0].bb;
            for (size_t i = 1; i < pieces.size(); ++i) {
                total.minX = std::min(total.minX, pieces[i].bb.minX);
                total.minY = std::min(total.minY, pieces[i].bb.minY);
                total.minZ = std::min(total.minZ, pieces[i].bb.minZ);
                total.maxX = std::max(total.maxX, pieces[i].bb.maxX);
                total.maxY = std::max(total.maxY, pieces[i].bb.maxY);
                total.maxZ = std::max(total.maxZ, pieces[i].bb.maxZ);
            }
            // Java: setRandomHeight(world, random, 48, 70)
            int height = total.maxY - total.minY;
            int targetY = 48 + rng.nextInt(std::max(1, 70 - 48 - height));
            int shift = targetY - total.minY;
            for (auto& p : pieces) {
                p.bb.minY += shift;
                p.bb.maxY += shift;
            }
        }
    }

    // ── Weighted piece selection ──
    bool selectAndCreatePiece(std::vector<PieceWeight>& weights, int& lastIdx,
                              const std::vector<FortressPiece>& pieces, JavaRNG& rng,
                              int x, int y, int z, int facing, int depth,
                              FortressPiece& out) {
        if (depth > 30) {
            // Create End piece as fallback
            return tryCreatePiece(PieceType::End, depth, rng, x, y, z, facing, pieces, out);
        }

        // Calculate total weight
        int totalWeight = 0;
        bool hasLimited = false;
        for (size_t i = 0; i < weights.size(); ++i) {
            if (weights[i].instancesLimit > 0 && weights[i].instancesSpawned < weights[i].instancesLimit)
                hasLimited = true;
            totalWeight += weights[i].weight;
        }
        if (!hasLimited) totalWeight = -1;
        if (totalWeight <= 0) {
            return tryCreatePiece(PieceType::End, depth, rng, x, y, z, facing, pieces, out);
        }

        // Try up to 5 times
        for (int attempt = 0; attempt < 5; ++attempt) {
            int r = rng.nextInt(totalWeight);
            for (size_t i = 0; i < weights.size(); ++i) {
                r -= weights[i].weight;
                if (r >= 0) continue;
                // Check depth/instance limit
                if (!weights[i].canSpawnAtDepth(depth)) break;
                // Prevent consecutive same piece (unless allowInRow)
                if (static_cast<int>(i) == lastIdx && !weights[i].allowInRow) break;
                FortressPiece newPiece;
                if (tryCreatePiece(weights[i].type, depth, rng, x, y, z, facing, pieces, newPiece)) {
                    weights[i].instancesSpawned++;
                    lastIdx = static_cast<int>(i);
                    if (!weights[i].canSpawnMore()) {
                        weights.erase(weights.begin() + i);
                        if (lastIdx >= static_cast<int>(weights.size())) lastIdx = -1;
                    }
                    out = newPiece;
                    return true;
                }
                break;
            }
        }
        // Fallback: End piece
        return tryCreatePiece(PieceType::End, depth, rng, x, y, z, facing, pieces, out);
    }
    // ══════════════════════════════════════════════════════════════════════
    // placePiece — render all 14 piece types into the block array
    // ══════════════════════════════════════════════════════════════════════
    void placePiece(const FortressPiece& piece, int worldMinX, int worldMinZ,
                    int32_t* blocks, uint8_t* meta,
                    std::vector<Chunk::SpawnerInfo>& spawners,
                    std::vector<Chunk::ChestInfo>& chests) {
        StructureCtx ctx{piece.bb, piece.coordBaseMode};
        int CBM = piece.coordBaseMode;
        switch (piece.type) {
            case PieceType::Straight:    placeStraight(ctx, blocks, meta, worldMinX, worldMinZ); break;
            case PieceType::Crossing3:   placeCrossing3(ctx, blocks, meta, worldMinX, worldMinZ); break;
            case PieceType::Crossing:    placeCrossing(ctx, blocks, meta, worldMinX, worldMinZ); break;
            case PieceType::Stairs:      placeStairs(ctx, blocks, meta, worldMinX, worldMinZ); break;
            case PieceType::Throne:      placeThrone(ctx, blocks, meta, worldMinX, worldMinZ, spawners); break;
            case PieceType::Entrance:    placeEntrance(ctx, blocks, meta, worldMinX, worldMinZ); break;
            case PieceType::Corridor5:   placeCorridor5(ctx, blocks, meta, worldMinX, worldMinZ); break;
            case PieceType::Crossing2:   placeCrossing2(ctx, blocks, meta, worldMinX, worldMinZ); break;
            case PieceType::Corridor2:   placeCorridor2(ctx, blocks, meta, worldMinX, worldMinZ, piece.hasChest, chests); break;
            case PieceType::Corridor:    placeCorridor(ctx, blocks, meta, worldMinX, worldMinZ, piece.hasChest, chests); break;
            case PieceType::Corridor3:   placeCorridor3(ctx, blocks, meta, worldMinX, worldMinZ); break;
            case PieceType::Corridor4:   placeCorridor4(ctx, blocks, meta, worldMinX, worldMinZ); break;
            case PieceType::NetherStalkRoom: placeNetherStalkRoom(ctx, blocks, meta, worldMinX, worldMinZ); break;
            case PieceType::End:         placeEnd(ctx, blocks, meta, worldMinX, worldMinZ, piece.fillSeed); break;
        }
    }

    // ── Straight: 5×10×19 bridge corridor (Java: StructureNetherBridgePieces$Straight) ──
    void placeStraight(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                       int wmX, int wmZ) {
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 3, 0, 4, 4, 18, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 5, 0, 3, 7, 18, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 5, 0, 0, 5, 18, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 5, 0, 4, 5, 18, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 4, 2, 5, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 13, 4, 2, 18, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 0, 0, 4, 1, 3, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 0, 15, 4, 1, 18, NETHER_BRICK, NETHER_BRICK, false);
        for (int i = 0; i <= 4; ++i) {
            for (int j = 0; j <= 2; ++j) {
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, j);
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, 18-j);
            }
        }
        // Fence along sides
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 8, 0, 4, 8, 18, NETHER_BRICK, NETHER_BRICK, false);
        for (int i = 0; i <= 18; i += 2) {
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 0, 6, i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 0, 7, i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 4, 6, i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 4, 7, i);
        }
    }

    // ── Crossing3: 19×10×19 large 4-way (Java: StructureNetherBridgePieces$Crossing3) ──
    void placeCrossing3(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                        int wmX, int wmZ) {
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 3, 0, 18, 4, 18, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 5, 0, 18, 9, 18, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 5, 0, 5, 8, 0, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 13, 5, 0, 18, 8, 0, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 5, 18, 5, 8, 18, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 13, 5, 18, 18, 8, 18, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 5, 0, 0, 8, 5, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 5, 13, 0, 8, 18, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 18, 5, 0, 18, 8, 5, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 18, 5, 13, 18, 8, 18, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 5, 1, 17, 5, 17, NETHER_BRICK, NETHER_BRICK, false);

        // Fill underside
        for (int i = 0; i <= 18; ++i)
            for (int j = 0; j <= 18; ++j)
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, j);
    }

    // ── Crossing: 7×9×7 (Java: StructureNetherBridgePieces$Crossing) ──
    void placeCrossing(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                       int wmX, int wmZ) {
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 0, 0, 6, 1, 6, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 6, 7, 6, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 1, 6, 0, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 6, 1, 6, 6, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 5, 2, 0, 6, 6, 0, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 5, 2, 6, 6, 6, 6, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 0, 6, 1, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 5, 0, 6, 6, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 6, 2, 0, 6, 6, 1, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 6, 2, 5, 6, 6, 6, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 2, 6, 0, 4, 6, 0, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 2, 5, 0, 4, 5, 0, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 2, 6, 6, 4, 6, 6, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 2, 5, 6, 4, 5, 6, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 6, 2, 0, 6, 4, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 5, 2, 0, 5, 4, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 6, 6, 2, 6, 6, 4, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 6, 5, 2, 6, 5, 4, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 8, 0, 6, 8, 6, NETHER_BRICK, NETHER_BRICK, false);
        for (int i = 0; i <= 6; ++i)
            for (int j = 0; j <= 6; ++j)
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, j);
    }

    // ── Stairs: 7×11×7 (Java: StructureNetherBridgePieces$Stairs) ──
    void placeStairs(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                     int wmX, int wmZ) {
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 0, 0, 6, 1, 6, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 6, 10, 6, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 1, 8, 0, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 5, 2, 0, 6, 8, 0, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 1, 0, 8, 6, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 6, 2, 1, 6, 8, 6, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 2, 6, 5, 8, 6, NETHER_BRICK, NETHER_BRICK, false);
        // Fence on top
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 0, 3, 2);
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 0, 3, 4);
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 6, 3, 2);
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 6, 3, 4);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 9, 0, 6, 9, 6, NETHER_BRICK, NETHER_BRICK, false);
        // Stairs
        int stairMeta = getMetadataWithOffset(NETHER_BRICK_STAIRS, 3, ctx.coordBaseMode);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 2, 2, 0, 4, 2, 0, NETHER_BRICK, NETHER_BRICK, false);
        for (int ix = 2; ix <= 4; ++ix) placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(stairMeta), ix, 2, 0);
        for (int i = 0; i <= 6; ++i)
            for (int j = 0; j <= 6; ++j)
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, j);
    }

    // ── Throne: 7×8×9, has Blaze spawner (Java: StructureNetherBridgePieces$Throne) ──
    void placeThrone(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                     int wmX, int wmZ,
                     std::vector<Chunk::SpawnerInfo>& spawners) {
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 6, 7, 7, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 0, 0, 5, 1, 7, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 2, 1, 5, 2, 7, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 3, 2, 5, 3, 7, NETHER_BRICK, NETHER_BRICK, false);
        // Stairs
        int sm2 = getMetadataWithOffset(NETHER_BRICK_STAIRS, 2, ctx.coordBaseMode);
        int sm3 = getMetadataWithOffset(NETHER_BRICK_STAIRS, 3, ctx.coordBaseMode);
        for (int ix = 1; ix <= 5; ++ix) {
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(sm2), ix, 2, 0);
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(sm3), ix, 3, 1);
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(sm3), ix, 4, 2);
        }
        // Side walls
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 0, 4, 7, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 6, 2, 0, 6, 4, 7, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 5, 0, 6, 5, 7, NETHER_BRICK, NETHER_BRICK, false);
        // Fence pillars
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 1, 5, 0);
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 5, 5, 0);
        // Back wall
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 5, 7, 5, 5, 7, NETHER_BRICK, NETHER_BRICK, false);
        // Underside
        for (int i = 0; i <= 6; ++i)
            for (int j = 0; j <= 8; ++j)
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, j);
        // Blaze spawner at (3, 5, 5) local
        {
            int wx = ctx.getXWithOffset(3, 5);
            int wy = ctx.getYWithOffset(5);
            int wz = ctx.getZWithOffset(3, 5);
            if (wx >= wmX && wx <= wmX + 15 && wz >= wmZ && wz <= wmZ + 15) {
                setBlockAt128(blocks, meta, wmX, wmZ, wx, wy, wz, SPAWNER, 0);
                Chunk::SpawnerInfo si;
                si.x = wx; si.y = wy; si.z = wz;
                si.entityId = "Blaze";
                spawners.push_back(si);
            }
        }
    }

    // ── Entrance: 13×14×13 with lava (Java: StructureNetherBridgePieces$Entrance) ──
    void placeEntrance(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                       int wmX, int wmZ) {
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 3, 0, 12, 4, 12, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 5, 0, 12, 13, 12, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 5, 0, 1, 12, 12, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 11, 5, 0, 12, 12, 12, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 2, 5, 11, 4, 12, 12, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 8, 5, 11, 10, 12, 12, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 5, 9, 11, 7, 12, 12, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 2, 5, 0, 4, 12, 1, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 8, 5, 0, 10, 12, 1, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 5, 9, 0, 7, 12, 1, NETHER_BRICK, NETHER_BRICK, false);
        // Floor cross
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 2, 0, 8, 2, 12, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 4, 12, 2, 8, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 0, 0, 8, 1, 3, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 0, 9, 8, 1, 12, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 0, 4, 3, 1, 8, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 9, 0, 4, 12, 1, 8, NETHER_BRICK, NETHER_BRICK, false);
        // Lava pool center
        for (int i = 4; i <= 8; ++i)
            for (int j = 0; j <= 2; ++j) {
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, j);
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, 12-j);
            }
        for (int i = 0; i <= 2; ++i)
            for (int j = 4; j <= 8; ++j) {
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, j);
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, 12-i, -1, j);
            }
        // Lava in center (open area at floor level)
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 5, 5, 5, 7, 5, 7, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 5, 3, 5, 7, 4, 7, LAVA, LAVA, false);
    }

    // ── Corridor5: 5×7×5 basic corridor (Java: StructureNetherBridgePieces$Corridor5) ──
    void placeCorridor5(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                        int wmX, int wmZ) {
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 0, 0, 4, 1, 4, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 4, 5, 4, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 0, 5, 4, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 2, 0, 4, 5, 4, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 3, 1, 0, 4, 1, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 3, 3, 0, 4, 3, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 3, 1, 4, 4, 1, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 3, 3, 4, 4, 3, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 6, 0, 4, 6, 4, NETHER_BRICK, NETHER_BRICK, false);
        for (int i = 0; i <= 4; ++i)
            for (int j = 0; j <= 4; ++j)
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, j);
    }

    // ── Crossing2: 5×7×5 small 3-way (Java: StructureNetherBridgePieces$Crossing2) ──
    void placeCrossing2(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                        int wmX, int wmZ) {
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 0, 0, 4, 1, 4, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 4, 5, 4, AIR, AIR, false);
        //corner pillars
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 0, 5, 0, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 2, 0, 4, 5, 0, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 4, 0, 5, 4, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 2, 4, 4, 5, 4, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 6, 0, 4, 6, 4, NETHER_BRICK, NETHER_BRICK, false);
        for (int i = 0; i <= 4; ++i)
            for (int j = 0; j <= 4; ++j)
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, j);
    }

    // ── Corridor2: 5×7×5 may have chest (Java: StructureNetherBridgePieces$Corridor2) ──
    void placeCorridor2(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                        int wmX, int wmZ, bool hasChest,
                        std::vector<Chunk::ChestInfo>& chests) {
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 0, 0, 4, 1, 4, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 4, 5, 4, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 2, 0, 4, 5, 4, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 3, 1, 4, 4, 1, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 3, 3, 4, 4, 3, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 0, 5, 0, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 4, 3, 5, 4, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 3, 4, 1, 4, 4, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 6, 0, 4, 6, 4, NETHER_BRICK, NETHER_BRICK, false);
        for (int i = 0; i <= 4; ++i)
            for (int j = 0; j <= 4; ++j)
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, j);
        if (hasChest) {
            // Place chest at (1, 2, 3) local
            int wx = ctx.getXWithOffset(1, 3);
            int wy = ctx.getYWithOffset(2);
            int wz = ctx.getZWithOffset(1, 3);
            if (wx >= wmX && wx <= wmX + 15 && wz >= wmZ && wz <= wmZ + 15) {
                setBlockAt128(blocks, meta, wmX, wmZ, wx, wy, wz, CHEST, 0);
                Chunk::ChestInfo ci;
                ci.x = wx; ci.y = wy; ci.z = wz;
                ci.lootTable = "nether_bridge";
                chests.push_back(ci);
            }
        }
    }

    // ── Corridor: 5×7×5 may have chest (Java: StructureNetherBridgePieces$Corridor) ──
    void placeCorridor(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                       int wmX, int wmZ, bool hasChest,
                       std::vector<Chunk::ChestInfo>& chests) {
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 0, 0, 4, 1, 4, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 4, 5, 4, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 0, 5, 4, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 3, 1, 0, 4, 1, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 3, 3, 0, 4, 3, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 2, 0, 4, 5, 0, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 2, 4, 4, 5, 4, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 3, 4, 1, 4, 4, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 3, 3, 4, 3, 4, 4, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 6, 0, 4, 6, 4, NETHER_BRICK, NETHER_BRICK, false);
        for (int i = 0; i <= 4; ++i)
            for (int j = 0; j <= 4; ++j)
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, j);
        if (hasChest) {
            int wx = ctx.getXWithOffset(3, 1);
            int wy = ctx.getYWithOffset(2);
            int wz = ctx.getZWithOffset(3, 1);
            if (wx >= wmX && wx <= wmX + 15 && wz >= wmZ && wz <= wmZ + 15) {
                setBlockAt128(blocks, meta, wmX, wmZ, wx, wy, wz, CHEST, 0);
                Chunk::ChestInfo ci;
                ci.x = wx; ci.y = wy; ci.z = wz;
                ci.lootTable = "nether_bridge";
                chests.push_back(ci);
            }
        }
    }

    // ── Corridor3: 5×14×10 stairway descent (Java: StructureNetherBridgePieces$Corridor3) ──
    void placeCorridor3(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                        int wmX, int wmZ) {
        int sm2 = getMetadataWithOffset(NETHER_BRICK_STAIRS, 2, ctx.coordBaseMode);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 0, 0, 4, 1, 9, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 4, 13, 9, AIR, AIR, false);
        // Walls
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 0, 13, 9, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 2, 0, 4, 13, 9, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 9, 4, 13, 9, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 14, 0, 4, 14, 9, NETHER_BRICK, NETHER_BRICK, false);
        // Steps
        for (int step = 0; step < 7; ++step) {
            int z = step + 2;
            fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 2 + step*2, z, 3, 2 + step*2, z, NETHER_BRICK, NETHER_BRICK, false);
            for (int ix = 1; ix <= 3; ++ix)
                placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(sm2), ix, 2 + step*2, z);
        }
        for (int i = 0; i <= 4; ++i)
            for (int j = 0; j <= 9; ++j)
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, j);
    }

    // ── Corridor4: 9×7×9 T-junction (Java: StructureNetherBridgePieces$Corridor4) ──
    void placeCorridor4(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                        int wmX, int wmZ) {
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 0, 0, 8, 1, 8, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 8, 5, 8, AIR, AIR, false);
        // Side walls
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 0, 2, 5, 0, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 6, 2, 0, 8, 5, 0, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 3, 0, 1, 4, 0, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 7, 3, 0, 7, 4, 0, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 2, 4, 8, 2, 8, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 2, 3, 7, 2, 3, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 3, 3, 7, 5, 3, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 6, 0, 8, 6, 8, NETHER_BRICK, NETHER_BRICK, false);
        for (int i = 0; i <= 8; ++i)
            for (int j = 0; j <= 8; ++j)
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, j);
    }

    // ── NetherStalkRoom: 13×14×13 soul sand + wart farm ──
    void placeNetherStalkRoom(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                              int wmX, int wmZ) {
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 3, 0, 12, 4, 12, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 5, 0, 12, 13, 12, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 5, 0, 1, 12, 12, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 11, 5, 0, 12, 12, 12, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 2, 5, 11, 4, 12, 12, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 8, 5, 11, 10, 12, 12, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 5, 9, 11, 7, 12, 12, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 2, 5, 0, 4, 12, 1, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 8, 5, 0, 10, 12, 1, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 5, 9, 0, 7, 12, 1, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 2, 11, 2, 10, 12, 10, NETHER_BRICK, NETHER_BRICK, false);
        // Fence along edges
        for (int n = 1; n <= 11; n += 2) {
            fillWithBlocks(ctx, blocks, meta, wmX, wmZ, n, 10, 0, n, 11, 0, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
            fillWithBlocks(ctx, blocks, meta, wmX, wmZ, n, 10, 12, n, 11, 12, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
            fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 10, n, 0, 11, n, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
            fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 12, 10, n, 12, 11, n, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, n, 13, 0);
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, n, 13, 12);
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, 0, 13, n);
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, 12, 13, n);
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, n+1, 13, 0);
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, n+1, 13, 12);
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 0, 13, n+1);
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 12, 13, n+1);
        }
        // Corner fence posts
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 0, 13, 0);
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 0, 13, 12);
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_FENCE, 0, 12, 13, 0);
        // Side fence pillars
        for (int n = 3; n <= 9; n += 2) {
            fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 7, n, 1, 8, n, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
            fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 11, 7, n, 11, 8, n, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        }
        // Central staircase
        int sm3 = getMetadataWithOffset(NETHER_BRICK_STAIRS, 3, ctx.coordBaseMode);
        for (int n4 = 0; n4 <= 6; ++n4) {
            int n3 = n4 + 4;
            for (int n2 = 5; n2 <= 7; ++n2)
                placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(sm3), n2, 5+n4, n3);
            if (n3 >= 5 && n3 <= 8)
                fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 5, 5, n3, 7, n4+4, n3, NETHER_BRICK, NETHER_BRICK, false);
            else if (n3 >= 9 && n3 <= 10)
                fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 5, 8, n3, 7, n4+4, n3, NETHER_BRICK, NETHER_BRICK, false);
            if (n4 < 1) continue;
            fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 5, 6+n4, n3, 7, 9+n4, n3, AIR, AIR, false);
        }
        for (int n4 = 5; n4 <= 7; ++n4)
            placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(sm3), n4, 12, 11);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 5, 6, 7, 5, 7, 7, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 7, 6, 7, 7, 7, 7, NETHER_BRICK_FENCE, NETHER_BRICK_FENCE, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 5, 13, 12, 7, 13, 12, AIR, AIR, false);
        // Soul sand + nether wart platforms
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 2, 5, 2, 3, 5, 3, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 2, 5, 9, 3, 5, 10, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 2, 5, 4, 2, 5, 8, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 9, 5, 2, 10, 5, 3, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 9, 5, 9, 10, 5, 10, NETHER_BRICK, NETHER_BRICK, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 10, 5, 4, 10, 5, 8, NETHER_BRICK, NETHER_BRICK, false);
        // Stair edges
        int sm0 = getMetadataWithOffset(NETHER_BRICK_STAIRS, 0, ctx.coordBaseMode);
        int sm1 = getMetadataWithOffset(NETHER_BRICK_STAIRS, 1, ctx.coordBaseMode);
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(sm1), 4, 5, 2);
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(sm1), 4, 5, 3);
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(sm1), 4, 5, 9);
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(sm1), 4, 5, 10);
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(sm0), 8, 5, 2);
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(sm0), 8, 5, 3);
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(sm0), 8, 5, 9);
        placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK_STAIRS, static_cast<uint8_t>(sm0), 8, 5, 10);
        // Soul sand + nether wart
        for (int x = 3; x <= 4; ++x)
            for (int z = 4; z <= 8; ++z)
                placeBlock(ctx, blocks, meta, wmX, wmZ, SOUL_SAND, 0, x, 4, z);
        for (int x = 8; x <= 9; ++x)
            for (int z = 4; z <= 8; ++z)
                placeBlock(ctx, blocks, meta, wmX, wmZ, SOUL_SAND, 0, x, 4, z);
        for (int x = 3; x <= 4; ++x)
            for (int z = 4; z <= 8; ++z)
                placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_WART, 0, x, 5, z);
        for (int x = 8; x <= 9; ++x)
            for (int z = 4; z <= 8; ++z)
                placeBlock(ctx, blocks, meta, wmX, wmZ, NETHER_WART, 0, x, 5, z);
        // Underside
        for (int n2 = 4; n2 <= 8; ++n2)
            for (int n = 0; n <= 2; ++n) {
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, n2, -1, n);
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, n2, -1, 12-n);
            }
        for (int n2 = 0; n2 <= 2; ++n2)
            for (int n = 4; n <= 8; ++n) {
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, n2, -1, n);
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, 12-n2, -1, n);
            }
    }

    // ── End: 5×10×8 dead-end cap (Java: StructureNetherBridgePieces$End) ──
    void placeEnd(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                  int wmX, int wmZ, int fillSeed) {
        JavaRNG rng(static_cast<int64_t>(fillSeed));
        for (int y = 3; y >= 0; --y)
            for (int x = 0; x <= 4; ++x)
                for (int z = 0; z <= 7; ++z) {
                    // Java: 1 + rng.nextInt(3) rows filled from bottom
                    if (rng.nextInt(y == 3 ? 3 : 1) == 0) {
                        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, x, y*3 + 3, z, x, y*3 + 3, z,
                                       NETHER_BRICK, NETHER_BRICK, false);
                    }
                }
        // Base
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 0, 0, 4, 2, 7, NETHER_BRICK, NETHER_BRICK, false);
        for (int i = 0; i <= 4; ++i)
            for (int j = 0; j <= 7; ++j)
                fillDownwards(ctx, blocks, meta, wmX, wmZ, NETHER_BRICK, 0, i, -1, j);
    }
}; // class MapGenNetherFortress

} // namespace mccpp
