/**
 * MapGenStronghold.h - Stronghold structure generation.
 *
 * Java reference: net.minecraft.world.gen.structure.MapGenStronghold
 *               + StructureStrongholdPieces (all inner classes)
 *               + MapGenStronghold$Start
 *
 * Generates strongholds with 11 piece types + corridor fallback:
 *   Stairs, Crossing, Straight, LeftTurn, RightTurn, Prison,
 *   ChestCorridor, RoomCrossing, StairsStraight, Library, PortalRoom, Corridor
 *
 * 3 strongholds per world in concentric rings around spawn.
 * Recursive piece assembly with depth limit 50, distance limit 112.
 * Stones block selector randomizes stonebrick/mossy/cracked/silverfish.
 *
 * Thread safety: Each OverworldGenerator instance owns its own MapGenStronghold.
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

class MapGenStronghold {
public:
    using BiomeQueryFn = std::function<int(int, int)>;

    explicit MapGenStronghold(int64_t seed, BiomeQueryFn biomeQuery = nullptr)
        : worldSeed_(seed), biomeQuery_(std::move(biomeQuery)) {}

    void setBiomeQuery(BiomeQueryFn fn) { biomeQuery_ = std::move(fn); }

    // ──── Generate: scan chunks for strongholds, place pieces ────
    void generate(int64_t seed, int chunkX, int chunkZ,
                  int32_t* blocks, uint8_t* meta,
                  std::vector<Chunk::SpawnerInfo>& spawners,
                  std::vector<Chunk::ChestInfo>& chests) {
        if (!initialized_) {
            initStrongholdPositions();
            initialized_ = true;
        }
        int worldMinX = chunkX * 16;
        int worldMinZ = chunkZ * 16;

        for (auto& pos : strongholdPositions_) {
            // Check if this chunk is within range of a stronghold
            int shCX = pos.first;
            int shCZ = pos.second;
            int dx = (shCX * 16 + 8) - worldMinX;
            int dz = (shCZ * 16 + 8) - worldMinZ;
            // Scan radius: strongholds can span ~7 chunks
            if (std::abs(dx) > 128 || std::abs(dz) > 128) continue;

            // Generate the stronghold once, deterministically
            JavaRNG rng(getChunkSeed(shCX, shCZ));
            std::vector<StrongholdPiece> pieces;
            generateStronghold(shCX, shCZ, rng, pieces);

            // Place all pieces that overlap this chunk
            for (auto& piece : pieces) {
                if (piece.bb.maxX < worldMinX || piece.bb.minX > worldMinX + 15) continue;
                if (piece.bb.maxZ < worldMinZ || piece.bb.minZ > worldMinZ + 15) continue;
                placePiece(piece, worldMinX, worldMinZ, blocks, meta, spawners, chests);
            }
        }
    }

private:
    int64_t worldSeed_;
    BiomeQueryFn biomeQuery_;
    bool initialized_ = false;
    std::vector<std::pair<int,int>> strongholdPositions_;

    // Block IDs
    static constexpr int32_t AIR = 0;
    static constexpr int32_t STONE = 1;
    static constexpr int32_t COBBLESTONE = 4;
    static constexpr int32_t OAK_PLANKS = 5;
    static constexpr int32_t WATER = 9;
    static constexpr int32_t LAVA = 11;
    static constexpr int32_t OAK_LOG = 17;
    static constexpr int32_t OAK_SLAB = 126;
    static constexpr int32_t BOOKSHELF = 47;
    static constexpr int32_t TORCH = 50;
    static constexpr int32_t SPAWNER = 52;
    static constexpr int32_t CHEST = 54;
    static constexpr int32_t LADDER = 65;
    static constexpr int32_t COBBLESTONE_STAIRS = 67;
    static constexpr int32_t STONE_SLAB = 44;
    static constexpr int32_t IRON_DOOR = 71;
    static constexpr int32_t STONE_BUTTON = 77;
    static constexpr int32_t STONEBRICK = 98;
    static constexpr int32_t STONE_BRICK_STAIRS = 109;
    static constexpr int32_t IRON_BARS = 101;
    static constexpr int32_t END_PORTAL_FRAME = 120;
    static constexpr int32_t END_PORTAL = 119;
    static constexpr int32_t MOSSY_STONEBRICK = 98;  // meta 1
    static constexpr int32_t CRACKED_STONEBRICK = 98; // meta 2
    static constexpr int32_t SILVERFISH_STONE = 97;
    static constexpr int32_t FLOWING_WATER = 8;

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
    };

    int64_t getChunkSeed(int cx, int cz) const {
        return worldSeed_ + static_cast<int64_t>(cx) * 341873128712LL
                          + static_cast<int64_t>(cz) * 132897987541LL;
    }

    int getBiome(int x, int z) const {
        if (biomeQuery_) return biomeQuery_(x, z);
        return 1;
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
        int xSize() const { return maxX - minX + 1; }
        int ySize() const { return maxY - minY + 1; }
        int zSize() const { return maxZ - minZ + 1; }
    };

    // Java: StructureBoundingBox.getComponentToAddBoundingBox
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

    static bool canStrongholdGoDeeper(const BBox& bb) {
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

    static inline bool isLiquid(int32_t id) {
        return id == 8 || id == 9 || id == 10 || id == 11;
    }

    // ──── Metadata rotation ────
    static int getMetadataWithOffset(int32_t blockId, int meta, int coordBaseMode) {
        // Stairs
        if (blockId == STONE_BRICK_STAIRS || blockId == COBBLESTONE_STAIRS) {
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
        // Doors (lower half)
        else if (blockId == IRON_DOOR) {
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
        // Button
        else if (blockId == STONE_BUTTON) {
            if (coordBaseMode == 0) {
                if (meta == 3) return 4; if (meta == 4) return 3;
            } else if (coordBaseMode == 1) {
                if (meta == 1) return 3; if (meta == 2) return 4;
                if (meta == 3) return 2; if (meta == 4) return 1;
            } else if (coordBaseMode == 3) {
                if (meta == 1) return 4; if (meta == 2) return 3;
                if (meta == 3) return 2; if (meta == 4) return 1;
            }
        }
        // End portal frame
        else if (blockId == END_PORTAL_FRAME) {
            if (coordBaseMode == 0) {
                if (meta == 0 || meta == 2) return meta ^ 2;
            } else if (coordBaseMode == 1) {
                return (meta + 1) & 3;
            } else if (coordBaseMode == 3) {
                return (meta + 3) & 3;
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

    // Java: fillWithRandomizedBlocks — Stones block selector
    // StructureStrongholdPieces$Stones: stonebrick(0), mossy(1), cracked(2), silverfish(5)
    void fillWithRandomizedBlocks(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                                  int worldMinX, int worldMinZ, JavaRNG& rng,
                                  int x1, int y1, int z1, int x2, int y2, int z2,
                                  bool skipAir) {
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
                    if (!isBorder) {
                        placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, x, y, z);
                    } else {
                        // Stones block selector
                        float f = rng.nextFloat();
                        if (f < 0.2f) {
                            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 2, x, y, z); // cracked
                        } else if (f < 0.5f) {
                            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 1, x, y, z); // mossy
                        } else if (f < 0.55f) {
                            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, SILVERFISH_STONE, 2, x, y, z); // silverfish
                        } else {
                            placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 0, x, y, z); // normal
                        }
                    }
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

    // ──── Piece types ────
    enum class PieceType {
        Stairs, Crossing, Straight, LeftTurn, RightTurn, Prison,
        ChestCorridor, RoomCrossing, StairsStraight, Library,
        PortalRoom, Corridor
    };

    // Java: StructureStrongholdPieces$Stronghold.Door
    enum class DoorType { OPENING, WOOD_DOOR, GRATES, IRON_DOOR };

    struct StrongholdPiece {
        PieceType type;
        BBox bb;
        int coordBaseMode;
        int componentType; // depth
        DoorType doorType = DoorType::OPENING;
        // Type-specific data
        bool expandsX = false, expandsZ = false; // Straight
        bool leftLow = false, leftHigh = false, rightLow = false, rightHigh = false; // Crossing
        int roomType = 0; // RoomCrossing
        bool isLargeRoom = false; // Library
        int corridorLength = 0; // Corridor
        bool hasPortal = false; // PortalRoom
    };

    // ──── PieceWeight ────
    struct PieceWeight {
        PieceType type;
        int weight;
        int instancesSpawned = 0;
        int instancesLimit;

        PieceWeight(PieceType t, int w, int limit) : type(t), weight(w), instancesLimit(limit) {}
        bool canSpawnMore() const { return instancesLimit == 0 || instancesSpawned < instancesLimit; }
    };

    // ──── Stronghold position initialization (ring-based) ────
    // Java: MapGenStronghold.generatePositions
    void initStrongholdPositions() {
        strongholdPositions_.clear();
        JavaRNG rng(worldSeed_);

        double angle = rng.nextFloat() * 3.141592653589793 * 2.0;
        int count = 0;
        int ring = 0;
        int countInRing = 3; // first ring has 3 strongholds
        int totalPlaced = 0;
        constexpr int TOTAL_STRONGHOLDS = 128; // 1.7.10 has 128

        for (totalPlaced = 0; totalPlaced < TOTAL_STRONGHOLDS; ++totalPlaced) {
            double dist = (4.0 * 32.0) + (6.0 * ring * 32.0) + (rng.nextInt(static_cast<int>(32 - 32 * 0.0)) - 0.0) * 32.0 * 0.0 + (rng.nextFloat() - 0.5) * 32.0 * 2.5;
            // Simplified: Java uses distance formula from spawn
            // dist = 32 * (4 + 6*ring) + random offset
            double d2 = (1.25 * 32.0 + rng.nextFloat() * 32.0 * 2.5) + ring * 6.0 * 32.0;
            int cx = static_cast<int>(std::round(std::cos(angle) * d2 / 16.0));
            int cz = static_cast<int>(std::round(std::sin(angle) * d2 / 16.0));
            strongholdPositions_.emplace_back(cx, cz);

            angle += 2.0 * 3.141592653589793 / countInRing;
            ++count;
            if (count >= countInRing) {
                ++ring;
                count = 0;
                countInRing = countInRing + 2 * countInRing / (ring + 1);
                if (countInRing > 128 - totalPlaced) countInRing = 128 - totalPlaced;
            }
        }
    }

    // ──── Check intersection with existing pieces ────
    static bool findIntersecting(const std::vector<StrongholdPiece>& pieces, const BBox& bb) {
        for (auto& p : pieces) {
            if (p.bb.intersects(bb)) return true;
        }
        return false;
    }

    // ──── Get random door type ────
    static DoorType getRandomDoor(JavaRNG& rng) {
        int r = rng.nextInt(5);
        if (r == 0) return DoorType::OPENING;
        if (r == 1) return DoorType::WOOD_DOOR;
        if (r == 2) return DoorType::GRATES;
        return DoorType::IRON_DOOR;
    }

    // ──── Try to create a piece at given position ────
    bool tryCreatePiece(PieceType type, int depth, JavaRNG& rng,
                        int x, int y, int z, int facing,
                        std::vector<StrongholdPiece>& pieces,
                        StrongholdPiece& out) {
        BBox bb;
        switch (type) {
            case PieceType::Stairs:
                bb = getComponentBB(x, y, z, -1, -7, 0, 5, 11, 5, facing); break;
            case PieceType::Crossing:
                bb = getComponentBB(x, y, z, -4, -3, 0, 10, 9, 11, facing); break;
            case PieceType::Straight:
                bb = getComponentBB(x, y, z, -1, -1, 0, 5, 5, 7, facing); break;
            case PieceType::LeftTurn:
            case PieceType::RightTurn:
                bb = getComponentBB(x, y, z, -1, -1, 0, 5, 5, 5, facing); break;
            case PieceType::Prison:
                bb = getComponentBB(x, y, z, -1, -1, 0, 9, 5, 11, facing); break;
            case PieceType::ChestCorridor:
                bb = getComponentBB(x, y, z, -1, -1, 0, 5, 5, 7, facing); break;
            case PieceType::RoomCrossing:
                bb = getComponentBB(x, y, z, -4, -1, 0, 11, 7, 11, facing); break;
            case PieceType::StairsStraight:
                bb = getComponentBB(x, y, z, -1, -7, 0, 5, 11, 8, facing); break;
            case PieceType::Library:
                bb = getComponentBB(x, y, z, -4, -1, 0, 14, 11, 15, facing);
                if (!canStrongholdGoDeeper(bb) || findIntersecting(pieces, bb)) {
                    bb = getComponentBB(x, y, z, -4, -1, 0, 14, 6, 15, facing);
                    out.isLargeRoom = false;
                } else {
                    out.isLargeRoom = true;
                }
                break;
            case PieceType::PortalRoom:
                bb = getComponentBB(x, y, z, -4, -1, 0, 11, 8, 16, facing); break;
            case PieceType::Corridor:
                bb = getComponentBB(x, y, z, -1, -1, 0, 5, 5, 4 + rng.nextInt(6), facing);
                out.corridorLength = bb.zSize() - 1;
                break;
        }

        if (!canStrongholdGoDeeper(bb) || findIntersecting(pieces, bb)) return false;

        out.type = type;
        out.bb = bb;
        out.coordBaseMode = facing;
        out.componentType = depth;
        if (type != PieceType::Corridor && type != PieceType::PortalRoom) {
            out.doorType = getRandomDoor(rng);
        }
        // Type-specific init
        if (type == PieceType::Straight) {
            out.expandsX = rng.nextInt(2) == 0;
            out.expandsZ = rng.nextInt(2) == 0;
        } else if (type == PieceType::Crossing) {
            out.leftLow = rng.nextBoolean();
            out.leftHigh = rng.nextBoolean();
            out.rightLow = rng.nextBoolean();
            out.rightHigh = rng.nextInt(3) > 0;
        } else if (type == PieceType::RoomCrossing) {
            out.roomType = rng.nextInt(5);
        }
        return true;
    }

    // ──── Get next component position ────
    void getNextComponentNormal(const StrongholdPiece& from, int offX, int offZ,
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

    void getNextComponentX(const StrongholdPiece& from, int offX, int offZ,
                           int& outX, int& outY, int& outZ, int& outFacing) {
        switch (from.coordBaseMode) {
            case 0: outFacing = 1; outX = from.bb.minX - 1; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ + offX; break;
            case 1: outFacing = 2; outX = from.bb.minX + offX; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ - 1; break;
            case 2: outFacing = 1; outX = from.bb.minX - 1; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ + offX; break;
            case 3: outFacing = 0; outX = from.bb.minX + offX; outY = from.bb.minY + offZ - 1; outZ = from.bb.maxZ + 1; break;
            default: outFacing = 0; outX = outY = outZ = 0; break;
        }
    }

    void getNextComponentZ(const StrongholdPiece& from, int offX, int offZ,
                           int& outX, int& outY, int& outZ, int& outFacing) {
        switch (from.coordBaseMode) {
            case 0: outFacing = 3; outX = from.bb.maxX + 1; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ + offX; break;
            case 1: outFacing = 0; outX = from.bb.minX + offX; outY = from.bb.minY + offZ - 1; outZ = from.bb.maxZ + 1; break;
            case 2: outFacing = 3; outX = from.bb.maxX + 1; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ + offX; break;
            case 3: outFacing = 2; outX = from.bb.minX + offX; outY = from.bb.minY + offZ - 1; outZ = from.bb.minZ - 1; break;
            default: outFacing = 0; outX = outY = outZ = 0; break;
        }
    }

    // ──── Recursive piece generation ────
    // Java: StructureStrongholdPieces.getNextComponent + buildComponent
    void generateStronghold(int startCX, int startCZ, JavaRNG& rng,
                            std::vector<StrongholdPiece>& pieces) {
        // Initialize piece weights
        std::vector<PieceWeight> weights = {
            {PieceType::Straight, 40, 0},
            {PieceType::Prison, 5, 5},
            {PieceType::LeftTurn, 20, 0},
            {PieceType::RightTurn, 20, 0},
            {PieceType::RoomCrossing, 10, 6},
            {PieceType::StairsStraight, 5, 5},
            {PieceType::Stairs, 5, 5},
            {PieceType::Crossing, 5, 4},
            {PieceType::ChestCorridor, 5, 4},
            {PieceType::Library, 10, 2},
            {PieceType::PortalRoom, 20, 1},
        };

        // Start piece: Stairs2 (extends Stairs)
        int startX = (startCX << 4) + 2;
        int startZ = (startCZ << 4) + 2;
        int facing = rng.nextInt(4);

        StrongholdPiece startPiece;
        startPiece.type = PieceType::Stairs;
        startPiece.coordBaseMode = facing;
        startPiece.componentType = 0;
        startPiece.doorType = DoorType::OPENING;
        startPiece.bb = getComponentBB(startX, 64, startZ, -1, -7, 0, 5, 11, 5, facing);
        pieces.push_back(startPiece);

        // Pending list for recursive generation
        std::vector<int> pending;
        pending.push_back(0);

        int maxDepth = 50;
        int maxPieces = 75;

        while (!pending.empty() && pieces.size() < static_cast<size_t>(maxPieces)) {
            int idx = rng.nextInt(static_cast<int>(pending.size()));
            int pieceIdx = pending[idx];
            pending.erase(pending.begin() + idx);

            auto& piece = pieces[pieceIdx];
            int depth = piece.componentType + 1;
            if (depth > maxDepth) continue;

            // Get exit positions based on piece type
            std::vector<std::tuple<int,int,int,int>> exits; // x,y,z,facing
            int ox, oy, oz, of;

            switch (piece.type) {
                case PieceType::Stairs:
                case PieceType::StairsStraight:
                    getNextComponentNormal(piece, 1, 1, ox, oy, oz, of);
                    exits.emplace_back(ox, oy, oz, of);
                    break;
                case PieceType::Straight:
                    getNextComponentNormal(piece, 1, 1, ox, oy, oz, of);
                    exits.emplace_back(ox, oy, oz, of);
                    if (piece.expandsX) {
                        getNextComponentX(piece, 1, 1, ox, oy, oz, of);
                        exits.emplace_back(ox, oy, oz, of);
                    }
                    if (piece.expandsZ) {
                        getNextComponentZ(piece, 1, 1, ox, oy, oz, of);
                        exits.emplace_back(ox, oy, oz, of);
                    }
                    break;
                case PieceType::LeftTurn:
                    if (piece.coordBaseMode == 2 || piece.coordBaseMode == 3) {
                        getNextComponentX(piece, 1, 1, ox, oy, oz, of);
                    } else {
                        getNextComponentZ(piece, 1, 1, ox, oy, oz, of);
                    }
                    exits.emplace_back(ox, oy, oz, of);
                    break;
                case PieceType::RightTurn:
                    if (piece.coordBaseMode == 2 || piece.coordBaseMode == 3) {
                        getNextComponentZ(piece, 1, 1, ox, oy, oz, of);
                    } else {
                        getNextComponentX(piece, 1, 1, ox, oy, oz, of);
                    }
                    exits.emplace_back(ox, oy, oz, of);
                    break;
                case PieceType::Prison:
                    getNextComponentNormal(piece, 1, 1, ox, oy, oz, of);
                    exits.emplace_back(ox, oy, oz, of);
                    break;
                case PieceType::ChestCorridor:
                    getNextComponentNormal(piece, 1, 1, ox, oy, oz, of);
                    exits.emplace_back(ox, oy, oz, of);
                    break;
                case PieceType::RoomCrossing:
                    getNextComponentNormal(piece, 4, 1, ox, oy, oz, of);
                    exits.emplace_back(ox, oy, oz, of);
                    getNextComponentX(piece, 1, 4, ox, oy, oz, of);
                    exits.emplace_back(ox, oy, oz, of);
                    getNextComponentZ(piece, 1, 4, ox, oy, oz, of);
                    exits.emplace_back(ox, oy, oz, of);
                    break;
                case PieceType::Crossing:
                    getNextComponentNormal(piece, 4, 1, ox, oy, oz, of);
                    exits.emplace_back(ox, oy, oz, of);
                    if (piece.leftLow) {
                        getNextComponentX(piece, 1, 1, ox, oy, oz, of);
                        exits.emplace_back(ox, oy, oz, of);
                    }
                    if (piece.rightLow) {
                        getNextComponentZ(piece, 1, 1, ox, oy, oz, of);
                        exits.emplace_back(ox, oy, oz, of);
                    }
                    if (piece.leftHigh) {
                        getNextComponentX(piece, 1, 5, ox, oy, oz, of);
                        exits.emplace_back(ox, oy, oz, of);
                    }
                    if (piece.rightHigh) {
                        getNextComponentZ(piece, 1, 5, ox, oy, oz, of);
                        exits.emplace_back(ox, oy, oz, of);
                    }
                    break;
                default:
                    break; // Library, PortalRoom, Corridor have no exits
            }

            // Try to place a piece at each exit
            for (auto& [ex, ey, ez, ef] : exits) {
                // Weighted random selection
                PieceType selectedType = PieceType::Corridor;
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

                StrongholdPiece newPiece;
                if (tryCreatePiece(selectedType, depth, rng, ex, ey, ez, ef, pieces, newPiece)) {
                    if (selectedIdx >= 0) weights[selectedIdx].instancesSpawned++;
                    int newIdx = static_cast<int>(pieces.size());
                    pieces.push_back(newPiece);
                    pending.push_back(newIdx);
                } else {
                    // Fallback: try corridor
                    StrongholdPiece corr;
                    if (tryCreatePiece(PieceType::Corridor, depth, rng, ex, ey, ez, ef, pieces, corr)) {
                        int newIdx = static_cast<int>(pieces.size());
                        pieces.push_back(corr);
                        pending.push_back(newIdx);
                    }
                }
            }
        }

        // Shift entire stronghold down to ~Y=30-40
        if (!pieces.empty()) {
            int minY = 999;
            for (auto& p : pieces) minY = std::min(minY, p.bb.minY);
            int targetY = 30 + static_cast<int>(rng.nextFloat() * 10);
            int shift = targetY - minY;
            for (auto& p : pieces) {
                p.bb.minY += shift;
                p.bb.maxY += shift;
            }
        }
    }

    // ──── Door placement (Java: Stronghold.placeDoor) ────
    void placeDoor(const StructureCtx& ctx, int32_t* blocks, uint8_t* meta,
                   int worldMinX, int worldMinZ, JavaRNG& rng,
                   DoorType doorType, int x, int y, int z) {
        int CBM = ctx.coordBaseMode;
        switch (doorType) {
            case DoorType::OPENING:
                fillWithAir(ctx, blocks, meta, worldMinX, worldMinZ, x, y, z, x + 2, y + 2, z);
                break;
            case DoorType::WOOD_DOOR:
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 0, x, y, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 0, x, y + 1, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 0, x, y + 2, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 0, x + 2, y, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 0, x + 2, y + 1, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 0, x + 2, y + 2, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, x + 1, y, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, x + 1, y + 1, z);
                // Door block omitted (complex) - use opening
                break;
            case DoorType::GRATES:
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, x + 1, y, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, x + 1, y + 1, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, IRON_BARS, 0, x, y, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, IRON_BARS, 0, x, y + 1, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, IRON_BARS, 0, x, y + 2, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, IRON_BARS, 0, x + 1, y + 2, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, IRON_BARS, 0, x + 2, y + 2, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, IRON_BARS, 0, x + 2, y + 1, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, IRON_BARS, 0, x + 2, y, z);
                break;
            case DoorType::IRON_DOOR:
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 0, x, y, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 0, x, y + 1, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 0, x, y + 2, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 0, x + 2, y, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 0, x + 2, y + 1, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONEBRICK, 0, x + 2, y + 2, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, x + 1, y, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, AIR, 0, x + 1, y + 1, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, IRON_DOOR,
                           static_cast<uint8_t>(getMetadataWithOffset(IRON_DOOR, 1, CBM)), x + 1, y, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, IRON_DOOR, 8, x + 1, y + 1, z);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_BUTTON,
                           static_cast<uint8_t>(getMetadataWithOffset(STONE_BUTTON, 4, CBM)), x + 2, y + 1, z + 1);
                placeBlock(ctx, blocks, meta, worldMinX, worldMinZ, STONE_BUTTON,
                           static_cast<uint8_t>(getMetadataWithOffset(STONE_BUTTON, 3, CBM)), x + 2, y + 1, z - 1);
                break;
        }
    }

    // ──── Master piece placer ────
    void placePiece(const StrongholdPiece& piece, int worldMinX, int worldMinZ,
                    int32_t* blocks, uint8_t* meta,
                    std::vector<Chunk::SpawnerInfo>& spawners,
                    std::vector<Chunk::ChestInfo>& chests) {
        // Construct deterministic RNG for this piece
        JavaRNG rng(static_cast<int64_t>(piece.bb.minX) * 48271LL +
                    static_cast<int64_t>(piece.bb.minZ) * 16807LL + worldSeed_);

        StructureCtx ctx;
        ctx.bb = piece.bb;
        ctx.coordBaseMode = piece.coordBaseMode;

        switch (piece.type) {
            case PieceType::Stairs: placeStairs(ctx, piece, blocks, meta, worldMinX, worldMinZ, rng); break;
            case PieceType::Crossing: placeCrossing(ctx, piece, blocks, meta, worldMinX, worldMinZ, rng); break;
            case PieceType::Straight: placeStraight(ctx, piece, blocks, meta, worldMinX, worldMinZ, rng); break;
            case PieceType::LeftTurn: placeLeftTurn(ctx, piece, blocks, meta, worldMinX, worldMinZ, rng); break;
            case PieceType::RightTurn: placeRightTurn(ctx, piece, blocks, meta, worldMinX, worldMinZ, rng); break;
            case PieceType::Prison: placePrison(ctx, piece, blocks, meta, worldMinX, worldMinZ, rng); break;
            case PieceType::ChestCorridor: placeChestCorridor(ctx, piece, blocks, meta, worldMinX, worldMinZ, rng, chests); break;
            case PieceType::RoomCrossing: placeRoomCrossing(ctx, piece, blocks, meta, worldMinX, worldMinZ, rng, chests); break;
            case PieceType::StairsStraight: placeStairsStraight(ctx, piece, blocks, meta, worldMinX, worldMinZ, rng); break;
            case PieceType::Library: placeLibrary(ctx, piece, blocks, meta, worldMinX, worldMinZ, rng, chests); break;
            case PieceType::PortalRoom: placePortalRoom(ctx, piece, blocks, meta, worldMinX, worldMinZ, rng, spawners); break;
            case PieceType::Corridor: placeCorridor(ctx, piece, blocks, meta, worldMinX, worldMinZ, rng); break;
        }
    }

    // ═══════ Individual piece renderers ═══════

    // Java: StructureStrongholdPieces$Stairs.addComponentParts
    void placeStairs(StructureCtx& ctx, const StrongholdPiece& piece,
                     int32_t* blocks, uint8_t* meta, int wmX, int wmZ, JavaRNG& rng) {
        int CBM = ctx.coordBaseMode;
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 0, 0, 0, 4, 10, 4, true);
        placeDoor(ctx, blocks, meta, wmX, wmZ, rng, piece.doorType, 1, 7, 0);
        placeDoor(ctx, blocks, meta, wmX, wmZ, rng, DoorType::OPENING, 1, 1, 4);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 0, 1, 1, 0, 3, 3, AIR, AIR, false);
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 1, 1, 4, 3, 3, AIR, AIR, false);
        // Stairs descending
        int stairMeta = getMetadataWithOffset(STONE_BRICK_STAIRS, 2, CBM);
        for (int i = 0; i < 6; ++i) {
            placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_BRICK_STAIRS, static_cast<uint8_t>(stairMeta), 1, 6 - i, 1 + i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_BRICK_STAIRS, static_cast<uint8_t>(stairMeta), 2, 6 - i, 1 + i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_BRICK_STAIRS, static_cast<uint8_t>(stairMeta), 3, 6 - i, 1 + i);
            if (i < 5) {
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 1, 5 - i, 1 + i);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 2, 5 - i, 1 + i);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 3, 5 - i, 1 + i);
            }
        }
    }

    // Java: StructureStrongholdPieces$Crossing.addComponentParts
    void placeCrossing(StructureCtx& ctx, const StrongholdPiece& piece,
                       int32_t* blocks, uint8_t* meta, int wmX, int wmZ, JavaRNG& rng) {
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 0, 0, 0, 9, 8, 10, true);
        // Main passage
        fillWithAir(ctx, blocks, meta, wmX, wmZ, 4, 3, 0, 5, 3, 10);
        fillWithAir(ctx, blocks, meta, wmX, wmZ, 4, 4, 0, 5, 4, 10);
        fillWithAir(ctx, blocks, meta, wmX, wmZ, 0, 5, 0, 9, 5, 10);
        // Vertical passages
        fillWithAir(ctx, blocks, meta, wmX, wmZ, 4, 1, 4, 5, 1, 6);
        // Pillars
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 0, 1, 1, 0, 1, 1, false);
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 0, 1, 3, 0, 1, 3, false);
        if (piece.leftLow) {
            fillWithAir(ctx, blocks, meta, wmX, wmZ, 0, 1, 7, 0, 3, 8);
        }
        if (piece.rightLow) {
            fillWithAir(ctx, blocks, meta, wmX, wmZ, 9, 1, 7, 9, 3, 8);
        }
        if (piece.leftHigh) {
            fillWithAir(ctx, blocks, meta, wmX, wmZ, 0, 5, 7, 0, 7, 8);
        }
        if (piece.rightHigh) {
            fillWithAir(ctx, blocks, meta, wmX, wmZ, 9, 5, 7, 9, 7, 8);
        }
        // Forward exit
        fillWithAir(ctx, blocks, meta, wmX, wmZ, 4, 1, 10, 5, 3, 10);
    }

    // Java: StructureStrongholdPieces$Straight.addComponentParts
    void placeStraight(StructureCtx& ctx, const StrongholdPiece& piece,
                       int32_t* blocks, uint8_t* meta, int wmX, int wmZ, JavaRNG& rng) {
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 0, 0, 0, 4, 4, 6, true);
        placeDoor(ctx, blocks, meta, wmX, wmZ, rng, piece.doorType, 1, 1, 0);
        placeDoor(ctx, blocks, meta, wmX, wmZ, rng, DoorType::OPENING, 1, 1, 6);
        // Torches
        int CBM = ctx.coordBaseMode;
        int tMeta1 = getMetadataWithOffset(TORCH, 3, CBM);
        placeBlock(ctx, blocks, meta, wmX, wmZ, TORCH, static_cast<uint8_t>(tMeta1), 1, 2, 1);
        placeBlock(ctx, blocks, meta, wmX, wmZ, TORCH, static_cast<uint8_t>(tMeta1), 3, 2, 1);
        if (piece.expandsX) {
            fillWithAir(ctx, blocks, meta, wmX, wmZ, 0, 1, 2, 0, 3, 4);
        }
        if (piece.expandsZ) {
            fillWithAir(ctx, blocks, meta, wmX, wmZ, 4, 1, 2, 4, 3, 4);
        }
    }

    // Java: StructureStrongholdPieces$LeftTurn.addComponentParts
    void placeLeftTurn(StructureCtx& ctx, const StrongholdPiece& piece,
                       int32_t* blocks, uint8_t* meta, int wmX, int wmZ, JavaRNG& rng) {
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 0, 0, 0, 4, 4, 4, true);
        placeDoor(ctx, blocks, meta, wmX, wmZ, rng, piece.doorType, 1, 1, 0);
        int CBM = ctx.coordBaseMode;
        if (CBM == 2 || CBM == 3) {
            fillWithAir(ctx, blocks, meta, wmX, wmZ, 0, 1, 1, 0, 3, 3);
        } else {
            fillWithAir(ctx, blocks, meta, wmX, wmZ, 4, 1, 1, 4, 3, 3);
        }
    }

    // Java: StructureStrongholdPieces$RightTurn.addComponentParts
    void placeRightTurn(StructureCtx& ctx, const StrongholdPiece& piece,
                        int32_t* blocks, uint8_t* meta, int wmX, int wmZ, JavaRNG& rng) {
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 0, 0, 0, 4, 4, 4, true);
        placeDoor(ctx, blocks, meta, wmX, wmZ, rng, piece.doorType, 1, 1, 0);
        int CBM = ctx.coordBaseMode;
        if (CBM == 2 || CBM == 3) {
            fillWithAir(ctx, blocks, meta, wmX, wmZ, 4, 1, 1, 4, 3, 3);
        } else {
            fillWithAir(ctx, blocks, meta, wmX, wmZ, 0, 1, 1, 0, 3, 3);
        }
    }

    // Java: StructureStrongholdPieces$Prison.addComponentParts
    void placePrison(StructureCtx& ctx, const StrongholdPiece& piece,
                     int32_t* blocks, uint8_t* meta, int wmX, int wmZ, JavaRNG& rng) {
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 0, 0, 0, 8, 4, 10, true);
        placeDoor(ctx, blocks, meta, wmX, wmZ, rng, piece.doorType, 1, 1, 0);
        fillWithAir(ctx, blocks, meta, wmX, wmZ, 1, 1, 10, 3, 3, 10);
        // Iron bars and cells
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 4, 1, 1, 4, 3, 1, false);
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 4, 1, 3, 4, 3, 3, false);
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 4, 1, 7, 4, 3, 7, false);
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 4, 1, 9, 4, 3, 9, false);
        // Iron bars
        for (int i = 1; i <= 3; ++i) {
            placeBlock(ctx, blocks, meta, wmX, wmZ, IRON_BARS, 0, 4, i, 4);
            placeBlock(ctx, blocks, meta, wmX, wmZ, IRON_BARS, 0, 4, i, 5);
            placeBlock(ctx, blocks, meta, wmX, wmZ, IRON_BARS, 0, 4, i, 6);
            placeBlock(ctx, blocks, meta, wmX, wmZ, IRON_BARS, 0, 0, i, 4);
            placeBlock(ctx, blocks, meta, wmX, wmZ, IRON_BARS, 0, 0, i, 5);
            placeBlock(ctx, blocks, meta, wmX, wmZ, IRON_BARS, 0, 0, i, 6);
        }
        // Door opening in cell
        int CBM = ctx.coordBaseMode;
        placeBlock(ctx, blocks, meta, wmX, wmZ, IRON_DOOR,
                   static_cast<uint8_t>(getMetadataWithOffset(IRON_DOOR, 3, CBM)), 4, 1, 2);
        placeBlock(ctx, blocks, meta, wmX, wmZ, IRON_DOOR, 8, 4, 2, 2);
        placeBlock(ctx, blocks, meta, wmX, wmZ, IRON_DOOR,
                   static_cast<uint8_t>(getMetadataWithOffset(IRON_DOOR, 3, CBM)), 4, 1, 8);
        placeBlock(ctx, blocks, meta, wmX, wmZ, IRON_DOOR, 8, 4, 2, 8);
    }

    // Java: StructureStrongholdPieces$ChestCorridor.addComponentParts
    void placeChestCorridor(StructureCtx& ctx, const StrongholdPiece& piece,
                            int32_t* blocks, uint8_t* meta, int wmX, int wmZ, JavaRNG& rng,
                            std::vector<Chunk::ChestInfo>& chests) {
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 0, 0, 0, 4, 4, 6, true);
        placeDoor(ctx, blocks, meta, wmX, wmZ, rng, piece.doorType, 1, 1, 0);
        placeDoor(ctx, blocks, meta, wmX, wmZ, rng, DoorType::OPENING, 1, 1, 6);
        // Chest at position (3,1,2)
        int wx = ctx.getXWithOffset(3, 2);
        int wy = ctx.getYWithOffset(1);
        int wz = ctx.getZWithOffset(3, 2);
        placeBlock(ctx, blocks, meta, wmX, wmZ, CHEST, 0, 3, 1, 2);
        Chunk::ChestInfo ci;
        ci.x = wx; ci.y = wy; ci.z = wz;
        ci.lootTable = "stronghold_corridor";
        chests.push_back(std::move(ci));
        // Slabs on wall
        int CBM = ctx.coordBaseMode;
        for (int i = 1; i <= 5; ++i) {
            placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_SLAB, 5, 0, 1, i); // stonebrick slab
        }
    }

    // Java: StructureStrongholdPieces$StairsStraight.addComponentParts
    void placeStairsStraight(StructureCtx& ctx, const StrongholdPiece& piece,
                             int32_t* blocks, uint8_t* meta, int wmX, int wmZ, JavaRNG& rng) {
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 0, 0, 0, 4, 10, 7, true);
        placeDoor(ctx, blocks, meta, wmX, wmZ, rng, piece.doorType, 1, 7, 0);
        placeDoor(ctx, blocks, meta, wmX, wmZ, rng, DoorType::OPENING, 1, 1, 7);
        int CBM = ctx.coordBaseMode;
        int stairMeta = getMetadataWithOffset(STONE_BRICK_STAIRS, 2, CBM);
        for (int i = 0; i < 6; ++i) {
            placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_BRICK_STAIRS, static_cast<uint8_t>(stairMeta), 1, 6 - i, 1 + i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_BRICK_STAIRS, static_cast<uint8_t>(stairMeta), 2, 6 - i, 1 + i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_BRICK_STAIRS, static_cast<uint8_t>(stairMeta), 3, 6 - i, 1 + i);
            if (i < 5) {
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 1, 5 - i, 1 + i);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 2, 5 - i, 1 + i);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 3, 5 - i, 1 + i);
            }
        }
    }

    // Java: StructureStrongholdPieces$RoomCrossing.addComponentParts
    void placeRoomCrossing(StructureCtx& ctx, const StrongholdPiece& piece,
                           int32_t* blocks, uint8_t* meta, int wmX, int wmZ, JavaRNG& rng,
                           std::vector<Chunk::ChestInfo>& chests) {
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 0, 0, 0, 10, 6, 10, true);
        placeDoor(ctx, blocks, meta, wmX, wmZ, rng, piece.doorType, 4, 1, 0);
        // Exits
        fillWithAir(ctx, blocks, meta, wmX, wmZ, 4, 1, 10, 6, 3, 10);
        fillWithAir(ctx, blocks, meta, wmX, wmZ, 0, 1, 4, 0, 3, 6);
        fillWithAir(ctx, blocks, meta, wmX, wmZ, 10, 1, 4, 10, 3, 6);

        int CBM = ctx.coordBaseMode;
        switch (piece.roomType) {
            case 0: // Empty
                break;
            case 1: // Pillar with torches
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 5, 1, 5);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 5, 2, 5);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 5, 3, 5);
                placeBlock(ctx, blocks, meta, wmX, wmZ, TORCH, static_cast<uint8_t>(getMetadataWithOffset(TORCH, 1, CBM)), 4, 3, 5);
                placeBlock(ctx, blocks, meta, wmX, wmZ, TORCH, static_cast<uint8_t>(getMetadataWithOffset(TORCH, 2, CBM)), 6, 3, 5);
                placeBlock(ctx, blocks, meta, wmX, wmZ, TORCH, static_cast<uint8_t>(getMetadataWithOffset(TORCH, 3, CBM)), 5, 3, 4);
                placeBlock(ctx, blocks, meta, wmX, wmZ, TORCH, static_cast<uint8_t>(getMetadataWithOffset(TORCH, 4, CBM)), 5, 3, 6);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_SLAB, 5, 4, 1, 4);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_SLAB, 5, 4, 1, 5);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_SLAB, 5, 4, 1, 6);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_SLAB, 5, 6, 1, 4);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_SLAB, 5, 6, 1, 5);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_SLAB, 5, 6, 1, 6);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_SLAB, 5, 5, 1, 4);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_SLAB, 5, 5, 1, 6);
                break;
            case 2: { // Water pool
                for (int i = 1; i <= 9; ++i) {
                    placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 1, 3, i);
                    placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 9, 3, i);
                }
                for (int i = 1; i <= 9; ++i) {
                    placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, i, 3, 1);
                    placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, i, 3, 9);
                }
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 5, 1, 4);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 5, 1, 6);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 5, 3, 4);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 5, 3, 6);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 4, 1, 5);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 6, 1, 5);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 4, 3, 5);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 6, 3, 5);
                for (int i = 1; i <= 3; ++i) {
                    placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 4, i, 4);
                    placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 6, i, 4);
                    placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 4, i, 6);
                    placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 6, i, 6);
                }
                placeBlock(ctx, blocks, meta, wmX, wmZ, FLOWING_WATER, 0, 5, 1, 5);
                break;
            }
            case 3: { // Chest room
                placeBlock(ctx, blocks, meta, wmX, wmZ, CHEST, 0, 3, 1, 5);
                {
                    int wx = ctx.getXWithOffset(3, 5);
                    int wy = ctx.getYWithOffset(1);
                    int wz = ctx.getZWithOffset(3, 5);
                    Chunk::ChestInfo ci;
                    ci.x = wx; ci.y = wy; ci.z = wz;
                    ci.lootTable = "stronghold_crossing";
                    chests.push_back(std::move(ci));
                }
                break;
            }
            case 4: // Cobblestone pillars
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 4, 1, 4);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 4, 2, 4);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 4, 3, 4);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 6, 1, 4);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 6, 2, 4);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 6, 3, 4);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 4, 1, 6);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 4, 2, 6);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 4, 3, 6);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 6, 1, 6);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 6, 2, 6);
                placeBlock(ctx, blocks, meta, wmX, wmZ, STONEBRICK, 0, 6, 3, 6);
                fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 4, 4, 6, 4, 6, STONEBRICK, STONEBRICK, false);
                break;
        }
    }

    // Java: StructureStrongholdPieces$Library.addComponentParts
    void placeLibrary(StructureCtx& ctx, const StrongholdPiece& piece,
                      int32_t* blocks, uint8_t* meta, int wmX, int wmZ, JavaRNG& rng,
                      std::vector<Chunk::ChestInfo>& chests) {
        int yMax = piece.isLargeRoom ? 10 : 5;
        int zMax = piece.isLargeRoom ? 14 : 14;
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 0, 0, 0, 13, yMax, zMax, true);
        placeDoor(ctx, blocks, meta, wmX, wmZ, rng, piece.doorType, 4, 1, 0);
        int CBM = ctx.coordBaseMode;
        // Floor decoration with bookshelves
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 1, 1, 1, 4, 13, BOOKSHELF, AIR, false);
        for (int z = 1; z <= 13; ++z) {
            placeBlock(ctx, blocks, meta, wmX, wmZ, BOOKSHELF, 0, 1, 1, z);
            if (z <= 12) placeBlock(ctx, blocks, meta, wmX, wmZ, BOOKSHELF, 0, 1, 2, z);
        }
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 12, 1, 1, 12, 4, 13, BOOKSHELF, AIR, false);
        for (int z = 1; z <= 13; ++z) {
            placeBlock(ctx, blocks, meta, wmX, wmZ, BOOKSHELF, 0, 12, 1, z);
            if (z <= 12) placeBlock(ctx, blocks, meta, wmX, wmZ, BOOKSHELF, 0, 12, 2, z);
        }
        // Torches
        placeBlock(ctx, blocks, meta, wmX, wmZ, TORCH, static_cast<uint8_t>(getMetadataWithOffset(TORCH, 2, CBM)), 2, 3, 1);
        placeBlock(ctx, blocks, meta, wmX, wmZ, TORCH, static_cast<uint8_t>(getMetadataWithOffset(TORCH, 1, CBM)), 11, 3, 1);
        // Chest
        {
            int wx = ctx.getXWithOffset(7, 2);
            int wy = ctx.getYWithOffset(1);
            int wz = ctx.getZWithOffset(7, 2);
            placeBlock(ctx, blocks, meta, wmX, wmZ, CHEST, 0, 7, 1, 2);
            Chunk::ChestInfo ci;
            ci.x = wx; ci.y = wy; ci.z = wz;
            ci.lootTable = "stronghold_library";
            chests.push_back(std::move(ci));
        }

        if (piece.isLargeRoom) {
            // Upper level bookshelves
            fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 1, 6, 1, 1, 9, 13, BOOKSHELF, AIR, false);
            fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 12, 6, 1, 12, 9, 13, BOOKSHELF, AIR, false);
            // Fence rail
            fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 3, 6, 3, 10, 6, 11, OAK_PLANKS, OAK_PLANKS, false);
            fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 6, 4, 9, 6, 10, AIR, AIR, false);
            // Upper chest
            {
                int wx = ctx.getXWithOffset(7, 7);
                int wy = ctx.getYWithOffset(6);
                int wz = ctx.getZWithOffset(7, 7);
                placeBlock(ctx, blocks, meta, wmX, wmZ, CHEST, 0, 7, 6, 7);
                Chunk::ChestInfo ci;
                ci.x = wx; ci.y = wy; ci.z = wz;
                ci.lootTable = "stronghold_library";
                chests.push_back(std::move(ci));
            }
        }
    }

    // Java: StructureStrongholdPieces$PortalRoom.addComponentParts
    void placePortalRoom(StructureCtx& ctx, const StrongholdPiece& piece,
                         int32_t* blocks, uint8_t* meta, int wmX, int wmZ, JavaRNG& rng,
                         std::vector<Chunk::SpawnerInfo>& spawners) {
        int CBM = ctx.coordBaseMode;
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 0, 0, 0, 10, 7, 15, false);
        // Entrance
        placeDoor(ctx, blocks, meta, wmX, wmZ, rng, DoorType::GRATES, 4, 1, 0);
        // 4 iron bars pillars
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 1, 6, 1, 1, 6, 14, false);
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 9, 6, 1, 9, 6, 14, false);
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 2, 6, 1, 8, 6, 1, false);
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 2, 6, 14, 8, 6, 14, false);
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 1, 1, 1, 2, 5, 4, false);
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 8, 1, 1, 9, 5, 4, false);
        fillWithAir(ctx, blocks, meta, wmX, wmZ, 1, 1, 1, 1, 3, 4);
        fillWithAir(ctx, blocks, meta, wmX, wmZ, 9, 1, 1, 9, 3, 4);
        // Iron bars across room
        for (int i = 3; i <= 7; ++i) {
            placeBlock(ctx, blocks, meta, wmX, wmZ, IRON_BARS, 0, 3, 1, i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, IRON_BARS, 0, 7, 1, i);
        }
        for (int i = 3; i <= 7; ++i) {
            placeBlock(ctx, blocks, meta, wmX, wmZ, IRON_BARS, 0, i, 1, 3);
            placeBlock(ctx, blocks, meta, wmX, wmZ, IRON_BARS, 0, i, 1, 7);
        }
        // Lava pool
        fillWithBlocks(ctx, blocks, meta, wmX, wmZ, 4, 1, 4, 6, 1, 6, LAVA, LAVA, false);
        // Stairs up to portal
        int stairN = getMetadataWithOffset(STONE_BRICK_STAIRS, 3, CBM);
        for (int z = 8; z <= 10; ++z) {
            placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_BRICK_STAIRS, static_cast<uint8_t>(stairN), 3, z - 7, z);
            placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_BRICK_STAIRS, static_cast<uint8_t>(stairN), 4, z - 7, z);
            placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_BRICK_STAIRS, static_cast<uint8_t>(stairN), 5, z - 7, z);
            placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_BRICK_STAIRS, static_cast<uint8_t>(stairN), 6, z - 7, z);
            placeBlock(ctx, blocks, meta, wmX, wmZ, STONE_BRICK_STAIRS, static_cast<uint8_t>(stairN), 7, z - 7, z);
        }
        // End portal frame (3x3 ring at Y=3, Z=11-13)
        // South side (z=11)
        for (int x = 4; x <= 6; ++x) {
            int frameMeta = getMetadataWithOffset(END_PORTAL_FRAME, 0, CBM);
            if (rng.nextFloat() > 0.9f) frameMeta |= 4; // 10% chance eye of ender
            placeBlock(ctx, blocks, meta, wmX, wmZ, END_PORTAL_FRAME, static_cast<uint8_t>(frameMeta), x, 3, 11);
        }
        // North side (z=13)
        for (int x = 4; x <= 6; ++x) {
            int frameMeta = getMetadataWithOffset(END_PORTAL_FRAME, 2, CBM);
            if (rng.nextFloat() > 0.9f) frameMeta |= 4;
            placeBlock(ctx, blocks, meta, wmX, wmZ, END_PORTAL_FRAME, static_cast<uint8_t>(frameMeta), x, 3, 13);
        }
        // West side (x=3)
        for (int z = 11; z <= 13; ++z) {
            int frameMeta = getMetadataWithOffset(END_PORTAL_FRAME, 3, CBM);
            if (rng.nextFloat() > 0.9f) frameMeta |= 4;
            placeBlock(ctx, blocks, meta, wmX, wmZ, END_PORTAL_FRAME, static_cast<uint8_t>(frameMeta), 3, 3, z);
        }
        // East side (x=7)
        for (int z = 11; z <= 13; ++z) {
            int frameMeta = getMetadataWithOffset(END_PORTAL_FRAME, 1, CBM);
            if (rng.nextFloat() > 0.9f) frameMeta |= 4;
            placeBlock(ctx, blocks, meta, wmX, wmZ, END_PORTAL_FRAME, static_cast<uint8_t>(frameMeta), 7, 3, z);
        }
        // Silverfish spawner at center
        int wx = ctx.getXWithOffset(5, 6);
        int wy = ctx.getYWithOffset(3);
        int wz = ctx.getZWithOffset(5, 6);
        placeBlock(ctx, blocks, meta, wmX, wmZ, SPAWNER, 0, 5, 3, 6);
        Chunk::SpawnerInfo si;
        si.x = wx; si.y = wy; si.z = wz;
        si.entityId = "Silverfish";
        spawners.push_back(std::move(si));
    }

    // Java: StructureStrongholdPieces$Corridor.addComponentParts
    void placeCorridor(StructureCtx& ctx, const StrongholdPiece& piece,
                       int32_t* blocks, uint8_t* meta, int wmX, int wmZ, JavaRNG& rng) {
        int len = piece.corridorLength;
        if (len <= 0) len = 3;
        fillWithRandomizedBlocks(ctx, blocks, meta, wmX, wmZ, rng, 0, 0, 0, 4, 4, len, true);
        for (int i = 0; i <= len; ++i) {
            placeBlock(ctx, blocks, meta, wmX, wmZ, AIR, 0, 1, 1, i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, AIR, 0, 2, 1, i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, AIR, 0, 3, 1, i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, AIR, 0, 1, 2, i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, AIR, 0, 2, 2, i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, AIR, 0, 3, 2, i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, AIR, 0, 1, 3, i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, AIR, 0, 2, 3, i);
            placeBlock(ctx, blocks, meta, wmX, wmZ, AIR, 0, 3, 3, i);
        }
    }

}; // class MapGenStronghold

} // namespace mccpp
