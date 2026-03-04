/**
 * MapGenMineshaft.h — Abandoned mineshaft structure generation.
 *
 * Java reference: net.minecraft.world.gen.structure.MapGenMineshaft
 *               + StructureMineshaftPieces (Room, Corridor, Cross, Stairs)
 *
 * Generates branching underground corridors with:
 *   - Fence post + plank beam supports every 5 blocks
 *   - Rail tracks (1/3 chance per corridor)
 *   - Cobweb clusters in spider corridors (1/23 chance when no rails)
 *   - Cave spider mob spawner in spider corridors
 *   - Torch placement near supports (5% chance)
 *   - Planks floor under corridors/crosses where air would be below
 *
 * Cross-chunk approach:
 *   For each chunk being generated, scan a radius of surrounding chunks
 *   for potential mineshaft start positions. For each start found, generate
 *   the full recursive piece tree, then only place blocks falling within
 *   the target chunk's 16×16 column.
 *
 * Thread safety: Each OverworldGenerator instance owns its own MapGenMineshaft.
 */
#pragma once

#include "world/Chunk.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <vector>

namespace mccpp {

class MapGenMineshaft {
public:
    explicit MapGenMineshaft(int64_t seed) : worldSeed_(seed) {}

    /**
     * Generate mineshaft structures that overlap with chunk (chunkX, chunkZ).
     *
     * @param seed      World seed
     * @param chunkX    Chunk X coordinate
     * @param chunkZ    Chunk Z coordinate
     * @param blocks    Block ID array (XZY ordering: (x*16+z)*256+y), chunk-local coords
     * @param meta      Block metadata array (same ordering)
     * @param spawners  Output vector for mob spawner tile entities (cave spiders)
     */
    void generate(int64_t seed, int chunkX, int chunkZ,
                  int32_t* blocks, uint8_t* meta,
                  std::vector<Chunk::SpawnerInfo>& spawners) {
        // Java: MapGenStructure.generateStructuresInChunk
        // Scan surrounding chunks for potential starts
        constexpr int SCAN_RADIUS = 4;
        int worldMinX = chunkX * 16;
        int worldMinZ = chunkZ * 16;

        for (int cx = chunkX - SCAN_RADIUS; cx <= chunkX + SCAN_RADIUS; ++cx) {
            for (int cz = chunkZ - SCAN_RADIUS; cz <= chunkZ + SCAN_RADIUS; ++cz) {
                // Deterministic RNG for this potential start chunk
                JavaRNG rng(getChunkSeed(cx, cz));

                // Java: MapGenMineshaft.canSpawnStructureAtCoords
                if (rng.nextDouble() >= SPAWN_CHANCE) continue;
                if (rng.nextInt(80) >= std::max(std::abs(cx), std::abs(cz))) continue;

                // This chunk has a mineshaft start — generate piece tree
                pieces_.clear();
                JavaRNG structRng(getChunkSeed(cx, cz) ^ 0x4D696E65LL); // "Mine" salt

                // Java: StructureMineshaftStart constructor
                int startX = (cx << 4) + 2;
                int startZ = (cz << 4) + 2;
                int roomW = 7 + structRng.nextInt(6);   // 7-12
                int roomH = 4 + structRng.nextInt(6);   // 4-9 (becomes minY+roomH = maxY)
                int roomD = 7 + structRng.nextInt(6);   // 7-12

                BBox roomBB = { startX, 50, startZ,
                                startX + roomW, 54 + structRng.nextInt(6), startZ + roomD };

                // markAvailableHeight: shift to Y range 10..maxY
                int yRange = roomBB.maxY - roomBB.minY + 1;
                int newMinY = 10 + structRng.nextInt(std::max(1, 50 - yRange));
                roomBB.maxY = newMinY + yRange - 1;
                roomBB.minY = newMinY;

                addRoom(structRng, roomBB, 0);

                // Place all pieces that overlap with the target chunk
                for (const auto& piece : pieces_) {
                    placePiece(piece, worldMinX, worldMinZ, blocks, meta, spawners, chunkX, chunkZ);
                }
            }
        }
    }

private:
    int64_t worldSeed_;

    static constexpr double SPAWN_CHANCE = 0.004;  // Java: field_82673_e
    static constexpr int MAX_DEPTH = 8;

    // Block IDs
    static constexpr int32_t AIR       = 0;
    static constexpr int32_t STONE     = 1;
    static constexpr int32_t DIRT      = 3;
    static constexpr int32_t PLANKS    = 5;
    static constexpr int32_t RAIL      = 66;
    static constexpr int32_t COBWEB    = 30;
    static constexpr int32_t TORCH     = 50;
    static constexpr int32_t FENCE     = 85;
    static constexpr int32_t SPAWNER   = 52;

    // ──────────────── Simple Java-like RNG ────────────────
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
            if ((bound & (bound - 1)) == 0) {
                return static_cast<int32_t>((static_cast<int64_t>(bound) * static_cast<int64_t>(next(31))) >> 31);
            }
            int32_t bits, val;
            do {
                bits = next(31);
                val = bits % bound;
            } while (bits - val + (bound - 1) < 0);
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
                          + static_cast<int64_t>(cz) * 132897987541LL + 10387313LL;
    }

    // ──────────────── Bounding Box ────────────────
    struct BBox {
        int minX, minY, minZ, maxX, maxY, maxZ;
        int getXSize() const { return maxX - minX + 1; }
        int getYSize() const { return maxY - minY + 1; }
        int getZSize() const { return maxZ - minZ + 1; }
        bool intersects(const BBox& o) const {
            return maxX >= o.minX && minX <= o.maxX &&
                   maxY >= o.minY && minY <= o.maxY &&
                   maxZ >= o.minZ && minZ <= o.maxZ;
        }
    };

    // ──────────────── Piece Types ────────────────
    enum class PieceType { Room, Corridor, Cross, Stairs };

    struct Piece {
        PieceType type;
        BBox bb;
        int direction;      // 0=south(+Z), 1=west(-X), 2=north(-Z), 3=east(+X)
        int depth;
        bool hasRails;
        bool hasSpiders;
        int sectionCount;   // Corridor only: number of 5-block sections
        bool isMultiFloor;  // Cross only
    };

    std::vector<Piece> pieces_;

    // ──────────────── Intersection check ────────────────
    const Piece* findIntersecting(const BBox& bb) const {
        for (const auto& p : pieces_) {
            if (p.bb.intersects(bb)) return &p;
        }
        return nullptr;
    }

    // ──────────────── Piece builders ────────────────

    void addRoom(JavaRNG& rng, const BBox& roomBB, int depth) {
        Piece room;
        room.type = PieceType::Room;
        room.bb = roomBB;
        room.direction = 0;
        room.depth = depth;
        room.hasRails = false;
        room.hasSpiders = false;
        room.sectionCount = 0;
        room.isMultiFloor = false;
        pieces_.push_back(room);

        // Java: Room.buildComponent — spawn corridors from all 4 walls
        int yRange = roomBB.getYSize() - 3 - 1;
        if (yRange <= 0) yRange = 1;

        // North wall (-Z)
        for (int n = 0; n < roomBB.getXSize(); ) {
            n += rng.nextInt(roomBB.getXSize());
            if (n + 3 > roomBB.getXSize()) break;
            tryAddPiece(rng, roomBB.minX + n, roomBB.minY + rng.nextInt(yRange) + 1,
                        roomBB.minZ - 1, 2, depth);
            n += 4;
        }
        // South wall (+Z)
        for (int n = 0; n < roomBB.getXSize(); ) {
            n += rng.nextInt(roomBB.getXSize());
            if (n + 3 > roomBB.getXSize()) break;
            tryAddPiece(rng, roomBB.minX + n, roomBB.minY + rng.nextInt(yRange) + 1,
                        roomBB.maxZ + 1, 0, depth);
            n += 4;
        }
        // West wall (-X)
        for (int n = 0; n < roomBB.getZSize(); ) {
            n += rng.nextInt(roomBB.getZSize());
            if (n + 3 > roomBB.getZSize()) break;
            tryAddPiece(rng, roomBB.minX - 1, roomBB.minY + rng.nextInt(yRange) + 1,
                        roomBB.minZ + n, 1, depth);
            n += 4;
        }
        // East wall (+X)
        for (int n = 0; n < roomBB.getZSize(); ) {
            n += rng.nextInt(roomBB.getZSize());
            if (n + 3 > roomBB.getZSize()) break;
            tryAddPiece(rng, roomBB.maxX + 1, roomBB.minY + rng.nextInt(yRange) + 1,
                        roomBB.minZ + n, 3, depth);
            n += 4;
        }
    }

    void tryAddPiece(JavaRNG& rng, int x, int y, int z, int direction, int depth) {
        if (depth > MAX_DEPTH) return;
        // Distance check: don't go more than 80 blocks from any existing piece
        if (!pieces_.empty()) {
            const auto& root = pieces_[0];
            if (std::abs(x - root.bb.minX) > 80 || std::abs(z - root.bb.minZ) > 80) return;
        }

        // Java: getRandomComponent — weighted random selection
        int roll = rng.nextInt(100);
        if (roll >= 80) {
            addCross(rng, x, y, z, direction, depth + 1);
        } else if (roll >= 70) {
            addStairs(rng, x, y, z, direction, depth + 1);
        } else {
            addCorridor(rng, x, y, z, direction, depth + 1);
        }
    }

    // ── Corridor ──
    void addCorridor(JavaRNG& rng, int x, int y, int z, int dir, int depth) {
        int sections = rng.nextInt(3) + 2;  // 2-4 sections
        while (sections > 0) {
            int len = sections * 5;
            BBox bb = {x, y, z, x, y + 2, z};
            switch (dir) {
                case 0: bb.maxX = x + 2; bb.maxZ = z + len - 1; break;
                case 1: bb.minX = x - len + 1; bb.maxZ = z + 2; break;
                case 2: bb.maxX = x + 2; bb.minZ = z - len + 1; break;
                case 3: bb.maxX = x + len - 1; bb.maxZ = z + 2; break;
            }
            if (!findIntersecting(bb)) break;
            --sections;
        }
        if (sections <= 0) return;

        int len = sections * 5;
        BBox bb = {x, y, z, x, y + 2, z};
        switch (dir) {
            case 0: bb.maxX = x + 2; bb.maxZ = z + len - 1; break;
            case 1: bb.minX = x - len + 1; bb.maxZ = z + 2; break;
            case 2: bb.maxX = x + 2; bb.minZ = z - len + 1; break;
            case 3: bb.maxX = x + len - 1; bb.maxZ = z + 2; break;
        }

        Piece corridor;
        corridor.type = PieceType::Corridor;
        corridor.bb = bb;
        corridor.direction = dir;
        corridor.depth = depth;
        corridor.hasRails = rng.nextInt(3) == 0;
        corridor.hasSpiders = !corridor.hasRails && rng.nextInt(23) == 0;
        corridor.sectionCount = sections;
        corridor.isMultiFloor = false;
        pieces_.push_back(corridor);

        // Continue branching from corridor end
        buildCorridorChildren(rng, corridor, depth);
    }

    void buildCorridorChildren(JavaRNG& rng, const Piece& corr, int depth) {
        int n = depth;
        int dir = corr.direction;
        int r = rng.nextInt(4);

        switch (dir) {
            case 2: // North
                if (r <= 1) tryAddPiece(rng, corr.bb.minX, corr.bb.minY - 1 + rng.nextInt(3),
                                        corr.bb.minZ - 1, dir, n);
                else if (r == 2) tryAddPiece(rng, corr.bb.minX - 1, corr.bb.minY - 1 + rng.nextInt(3),
                                             corr.bb.minZ, 1, n);
                else tryAddPiece(rng, corr.bb.maxX + 1, corr.bb.minY - 1 + rng.nextInt(3),
                                 corr.bb.minZ, 3, n);
                break;
            case 0: // South
                if (r <= 1) tryAddPiece(rng, corr.bb.minX, corr.bb.minY - 1 + rng.nextInt(3),
                                        corr.bb.maxZ + 1, dir, n);
                else if (r == 2) tryAddPiece(rng, corr.bb.minX - 1, corr.bb.minY - 1 + rng.nextInt(3),
                                             corr.bb.maxZ - 3, 1, n);
                else tryAddPiece(rng, corr.bb.maxX + 1, corr.bb.minY - 1 + rng.nextInt(3),
                                 corr.bb.maxZ - 3, 3, n);
                break;
            case 1: // West
                if (r <= 1) tryAddPiece(rng, corr.bb.minX - 1, corr.bb.minY - 1 + rng.nextInt(3),
                                        corr.bb.minZ, dir, n);
                else if (r == 2) tryAddPiece(rng, corr.bb.minX, corr.bb.minY - 1 + rng.nextInt(3),
                                             corr.bb.minZ - 1, 2, n);
                else tryAddPiece(rng, corr.bb.minX, corr.bb.minY - 1 + rng.nextInt(3),
                                 corr.bb.maxZ + 1, 0, n);
                break;
            case 3: // East
                if (r <= 1) tryAddPiece(rng, corr.bb.maxX + 1, corr.bb.minY - 1 + rng.nextInt(3),
                                        corr.bb.minZ, dir, n);
                else if (r == 2) tryAddPiece(rng, corr.bb.maxX - 3, corr.bb.minY - 1 + rng.nextInt(3),
                                             corr.bb.minZ - 1, 2, n);
                else tryAddPiece(rng, corr.bb.maxX - 3, corr.bb.minY - 1 + rng.nextInt(3),
                                 corr.bb.maxZ + 1, 0, n);
                break;
        }

        // Side branches along corridor length (Java parity)
        if (depth < MAX_DEPTH) {
            if (dir == 2 || dir == 0) {
                for (int z = corr.bb.minZ + 3; z + 3 <= corr.bb.maxZ; z += 5) {
                    int roll = rng.nextInt(5);
                    if (roll == 0) tryAddPiece(rng, corr.bb.minX - 1, corr.bb.minY, z, 1, depth + 1);
                    else if (roll == 1) tryAddPiece(rng, corr.bb.maxX + 1, corr.bb.minY, z, 3, depth + 1);
                }
            } else {
                for (int x = corr.bb.minX + 3; x + 3 <= corr.bb.maxX; x += 5) {
                    int roll = rng.nextInt(5);
                    if (roll == 0) tryAddPiece(rng, x, corr.bb.minY, corr.bb.minZ - 1, 2, depth + 1);
                    else if (roll == 1) tryAddPiece(rng, x, corr.bb.minY, corr.bb.maxZ + 1, 0, depth + 1);
                }
            }
        }
    }

    // ── Cross (intersection) ──
    void addCross(JavaRNG& rng, int x, int y, int z, int dir, int depth) {
        BBox bb = {x, y, z, x, y + 2, z};
        bool multiFloor = rng.nextInt(4) == 0;
        if (multiFloor) bb.maxY += 4;
        switch (dir) {
            case 2: bb.minX = x - 1; bb.maxX = x + 3; bb.minZ = z - 4; break;
            case 0: bb.minX = x - 1; bb.maxX = x + 3; bb.maxZ = z + 4; break;
            case 1: bb.minX = x - 4; bb.minZ = z - 1; bb.maxZ = z + 3; break;
            case 3: bb.maxX = x + 4; bb.minZ = z - 1; bb.maxZ = z + 3; break;
        }
        if (findIntersecting(bb)) return;

        Piece cross;
        cross.type = PieceType::Cross;
        cross.bb = bb;
        cross.direction = dir;
        cross.depth = depth;
        cross.hasRails = false;
        cross.hasSpiders = false;
        cross.sectionCount = 0;
        cross.isMultiFloor = multiFloor;
        pieces_.push_back(cross);

        // Spawn children in 3 directions (not back the way we came)
        // Java: Cross.buildComponent
        switch (dir) {
            case 2:
                tryAddPiece(rng, bb.minX + 1, bb.minY, bb.minZ - 1, 2, depth);
                tryAddPiece(rng, bb.minX - 1, bb.minY, bb.minZ + 1, 1, depth);
                tryAddPiece(rng, bb.maxX + 1, bb.minY, bb.minZ + 1, 3, depth);
                break;
            case 0:
                tryAddPiece(rng, bb.minX + 1, bb.minY, bb.maxZ + 1, 0, depth);
                tryAddPiece(rng, bb.minX - 1, bb.minY, bb.minZ + 1, 1, depth);
                tryAddPiece(rng, bb.maxX + 1, bb.minY, bb.minZ + 1, 3, depth);
                break;
            case 1:
                tryAddPiece(rng, bb.minX + 1, bb.minY, bb.minZ - 1, 2, depth);
                tryAddPiece(rng, bb.minX + 1, bb.minY, bb.maxZ + 1, 0, depth);
                tryAddPiece(rng, bb.minX - 1, bb.minY, bb.minZ + 1, 1, depth);
                break;
            case 3:
                tryAddPiece(rng, bb.minX + 1, bb.minY, bb.minZ - 1, 2, depth);
                tryAddPiece(rng, bb.minX + 1, bb.minY, bb.maxZ + 1, 0, depth);
                tryAddPiece(rng, bb.maxX + 1, bb.minY, bb.minZ + 1, 3, depth);
                break;
        }
        // Multi-floor: additional upper-level branches
        if (multiFloor) {
            if (rng.nextBoolean()) tryAddPiece(rng, bb.minX + 1, bb.minY + 4, bb.minZ - 1, 2, depth);
            if (rng.nextBoolean()) tryAddPiece(rng, bb.minX - 1, bb.minY + 4, bb.minZ + 1, 1, depth);
            if (rng.nextBoolean()) tryAddPiece(rng, bb.maxX + 1, bb.minY + 4, bb.minZ + 1, 3, depth);
            if (rng.nextBoolean()) tryAddPiece(rng, bb.minX + 1, bb.minY + 4, bb.maxZ + 1, 0, depth);
        }
    }

    // ── Stairs ──
    void addStairs(JavaRNG& rng, int x, int y, int z, int dir, int depth) {
        BBox bb = {x, y - 5, z, x, y + 2, z};
        switch (dir) {
            case 2: bb.maxX = x + 2; bb.minZ = z - 8; break;
            case 0: bb.maxX = x + 2; bb.maxZ = z + 8; break;
            case 1: bb.minX = x - 8; bb.maxZ = z + 2; break;
            case 3: bb.maxX = x + 8; bb.maxZ = z + 2; break;
        }
        if (findIntersecting(bb)) return;

        Piece stairs;
        stairs.type = PieceType::Stairs;
        stairs.bb = bb;
        stairs.direction = dir;
        stairs.depth = depth;
        stairs.hasRails = false;
        stairs.hasSpiders = false;
        stairs.sectionCount = 0;
        stairs.isMultiFloor = false;
        pieces_.push_back(stairs);

        // Continue in same direction from the bottom
        switch (dir) {
            case 2: tryAddPiece(rng, bb.minX, bb.minY, bb.minZ - 1, 2, depth); break;
            case 0: tryAddPiece(rng, bb.minX, bb.minY, bb.maxZ + 1, 0, depth); break;
            case 1: tryAddPiece(rng, bb.minX - 1, bb.minY, bb.minZ, 1, depth); break;
            case 3: tryAddPiece(rng, bb.maxX + 1, bb.minY, bb.minZ, 3, depth); break;
        }
    }

    // ──────────────── Block placement ────────────────

    /**
     * Place piece blocks that fall within the target chunk.
     * All coordinates in the piece are world-space; we clip to [worldMinX..worldMinX+15] etc.
     */
    void placePiece(const Piece& piece, int worldMinX, int worldMinZ,
                    int32_t* blocks, uint8_t* meta,
                    std::vector<Chunk::SpawnerInfo>& spawners,
                    int chunkX, int chunkZ) {
        // Use a separate RNG for block placement per piece (deterministic from position)
        JavaRNG placeRng(static_cast<int64_t>(piece.bb.minX) * 7919LL +
                         static_cast<int64_t>(piece.bb.minZ) * 4813LL +
                         static_cast<int64_t>(piece.bb.minY) * 2381LL +
                         worldSeed_);

        switch (piece.type) {
            case PieceType::Room:     placeRoom(piece, placeRng, worldMinX, worldMinZ, blocks, meta); break;
            case PieceType::Corridor: placeCorridor(piece, placeRng, worldMinX, worldMinZ, blocks, meta, spawners, chunkX, chunkZ); break;
            case PieceType::Cross:    placeCross(piece, placeRng, worldMinX, worldMinZ, blocks, meta); break;
            case PieceType::Stairs:   placeStairs(piece, placeRng, worldMinX, worldMinZ, blocks, meta); break;
        }
    }

    // ── Helper: set block in chunk-local coords if world pos is within chunk ──
    static inline void setBlockAt(int32_t* blocks, uint8_t* meta,
                                  int worldMinX, int worldMinZ,
                                  int wx, int wy, int wz,
                                  int32_t blockId, uint8_t blockMeta = 0) {
        int lx = wx - worldMinX;
        int lz = wz - worldMinZ;
        if (lx < 0 || lx > 15 || lz < 0 || lz > 15 || wy < 1 || wy > 254) return;
        blocks[(lx * 16 + lz) * 256 + wy] = blockId;
        if (blockMeta != 0) meta[(lx * 16 + lz) * 256 + wy] = blockMeta;
    }

    static inline int32_t getBlockAt(const int32_t* blocks,
                                     int worldMinX, int worldMinZ,
                                     int wx, int wy, int wz) {
        int lx = wx - worldMinX;
        int lz = wz - worldMinZ;
        if (lx < 0 || lx > 15 || lz < 0 || lz > 15 || wy < 0 || wy > 255) return STONE;
        return blocks[(lx * 16 + lz) * 256 + wy];
    }

    // ── Helper: check if block is liquid ──
    static inline bool isLiquid(int32_t id) {
        return id == 8 || id == 9 || id == 10 || id == 11;  // water, lava
    }

    // ── Helper: check if bounding box overlaps with chunk ──
    static inline bool overlapsChunk(const BBox& bb, int worldMinX, int worldMinZ) {
        return bb.maxX >= worldMinX && bb.minX <= worldMinX + 15 &&
               bb.maxZ >= worldMinZ && bb.minZ <= worldMinZ + 15;
    }

    // ── Helper: check for liquid in structure BB within chunk ──
    bool isLiquidInBB(const BBox& bb, const int32_t* blocks, int worldMinX, int worldMinZ) {
        for (int x = std::max(bb.minX, worldMinX); x <= std::min(bb.maxX, worldMinX + 15); ++x) {
            for (int z = std::max(bb.minZ, worldMinZ); z <= std::min(bb.maxZ, worldMinZ + 15); ++z) {
                for (int y = bb.minY; y <= bb.maxY; ++y) {
                    if (isLiquid(getBlockAt(blocks, worldMinX, worldMinZ, x, y, z))) return true;
                }
            }
        }
        return false;
    }

    // ── Helper: fill a box with a block, only replacing non-air ──
    void fillBox(int32_t* blocks, uint8_t* meta, int worldMinX, int worldMinZ,
                 int x1, int y1, int z1, int x2, int y2, int z2,
                 int32_t blockId, bool replaceAirOnly = false) {
        for (int x = x1; x <= x2; ++x) {
            for (int z = z1; z <= z2; ++z) {
                for (int y = y1; y <= y2; ++y) {
                    if (replaceAirOnly) {
                        int32_t cur = getBlockAt(blocks, worldMinX, worldMinZ, x, y, z);
                        if (cur != AIR) continue;
                    }
                    setBlockAt(blocks, meta, worldMinX, worldMinZ, x, y, z, blockId);
                }
            }
        }
    }

    // ──────────────── Room placement ────────────────
    // Java: Room.addComponentParts
    void placeRoom(const Piece& piece, JavaRNG& rng,
                   int worldMinX, int worldMinZ,
                   int32_t* blocks, uint8_t* meta) {
        if (!overlapsChunk(piece.bb, worldMinX, worldMinZ)) return;
        if (isLiquidInBB(piece.bb, blocks, worldMinX, worldMinZ)) return;

        const auto& bb = piece.bb;
        // Floor: dirt
        fillBox(blocks, meta, worldMinX, worldMinZ,
                bb.minX, bb.minY, bb.minZ, bb.maxX, bb.minY, bb.maxZ, DIRT);
        // Interior air: y+1 to y+3
        int topAir = std::min(bb.minY + 3, bb.maxY);
        fillBox(blocks, meta, worldMinX, worldMinZ,
                bb.minX, bb.minY + 1, bb.minZ, bb.maxX, topAir, bb.maxZ, AIR);
        // Upper air: randomize (80% chance per block above y+4)
        for (int x = bb.minX; x <= bb.maxX; ++x) {
            for (int z = bb.minZ; z <= bb.maxZ; ++z) {
                for (int y = bb.minY + 4; y <= bb.maxY; ++y) {
                    if (rng.nextFloat() < 0.8f) {
                        setBlockAt(blocks, meta, worldMinX, worldMinZ, x, y, z, AIR);
                    }
                }
            }
        }
    }

    // ──────────────── Corridor placement ────────────────
    // Java: Corridor.addComponentParts
    // Corridors use local coordinates relative to direction. For simplicity,
    // I map the Java local (x, y, z) to world coords based on direction.
    void placeCorridor(const Piece& piece, JavaRNG& rng,
                       int worldMinX, int worldMinZ,
                       int32_t* blocks, uint8_t* meta,
                       std::vector<Chunk::SpawnerInfo>& spawners,
                       int chunkX, int chunkZ) {
        if (!overlapsChunk(piece.bb, worldMinX, worldMinZ)) return;
        if (isLiquidInBB(piece.bb, blocks, worldMinX, worldMinZ)) return;

        const auto& bb = piece.bb;
        int dir = piece.direction;
        int sections = piece.sectionCount;
        int corridorLen = sections * 5 - 1;

        // Java uses local coordinates with getXWithOffset/getZWithOffset
        // Local X is the cross axis (0-2), local Z is the corridor axis (0..corridorLen)
        // We need to map these to world coordinates based on direction:
        auto toWorld = [&](int localX, int localY, int localZ, int& wx, int& wy, int& wz) {
            wy = bb.minY + localY;
            switch (dir) {
                case 0: // South: +Z = corridor direction, +X = right
                    wx = bb.minX + localX;
                    wz = bb.minZ + localZ;
                    break;
                case 1: // West: -X = corridor direction, +Z = right
                    wx = bb.maxX - localZ;
                    wz = bb.minZ + localX;
                    break;
                case 2: // North: -Z = corridor direction, +X = right
                    wx = bb.minX + localX;
                    wz = bb.maxZ - localZ;
                    break;
                case 3: // East: +X = corridor direction, +Z = right
                    wx = bb.minX + localZ;
                    wz = bb.minZ + localX;
                    break;
            }
        };

        // Carve main corridor: 3 wide, 2 high, full length
        for (int lx = 0; lx <= 2; ++lx) {
            for (int lz = 0; lz <= corridorLen; ++lz) {
                for (int ly = 0; ly <= 1; ++ly) {
                    int wx, wy, wz;
                    toWorld(lx, ly, lz, wx, wy, wz);
                    setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, AIR);
                }
                // Ceiling: 80% chance of air at y+2
                if (rng.nextFloat() < 0.8f) {
                    int wx, wy, wz;
                    toWorld(lx, 2, lz, wx, wy, wz);
                    setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, AIR);
                }
            }
        }

        // Spider corridor: 60% cobwebs in lower half
        if (piece.hasSpiders) {
            for (int lx = 0; lx <= 2; ++lx) {
                for (int lz = 0; lz <= corridorLen; ++lz) {
                    for (int ly = 0; ly <= 1; ++ly) {
                        if (rng.nextFloat() < 0.6f) {
                            int wx, wy, wz;
                            toWorld(lx, ly, lz, wx, wy, wz);
                            setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, COBWEB);
                        }
                    }
                }
            }
        }

        bool spawnerPlaced = false;

        // Support beams every 5 blocks
        for (int sect = 0; sect < sections; ++sect) {
            int beamZ = 2 + sect * 5;

            // Fence posts on sides (local x=0 and x=2)
            for (int ly = 0; ly <= 1; ++ly) {
                int wx0, wy0, wz0, wx2, wy2, wz2;
                toWorld(0, ly, beamZ, wx0, wy0, wz0);
                toWorld(2, ly, beamZ, wx2, wy2, wz2);
                setBlockAt(blocks, meta, worldMinX, worldMinZ, wx0, wy0, wz0, FENCE);
                setBlockAt(blocks, meta, worldMinX, worldMinZ, wx2, wy2, wz2, FENCE);
            }

            // Crossbeam: planks at y+2
            if (rng.nextInt(4) == 0) {
                // Partial beam: only sides
                int wx, wy, wz;
                toWorld(0, 2, beamZ, wx, wy, wz);
                setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, PLANKS);
                toWorld(2, 2, beamZ, wx, wy, wz);
                setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, PLANKS);
            } else {
                // Full beam: entire width
                for (int lx = 0; lx <= 2; ++lx) {
                    int wx, wy, wz;
                    toWorld(lx, 2, beamZ, wx, wy, wz);
                    setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, PLANKS);
                }
            }

            // Cobweb near supports (10% at ±1, 5% at ±2)
            auto maybeWeb = [&](int lx, int ly, int lz, float chance) {
                if (rng.nextFloat() < chance) {
                    int wx, wy, wz;
                    toWorld(lx, ly, lz, wx, wy, wz);
                    if (lz >= 0 && lz <= corridorLen) {
                        setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, COBWEB);
                    }
                }
            };
            maybeWeb(0, 2, beamZ - 1, 0.1f);
            maybeWeb(2, 2, beamZ - 1, 0.1f);
            maybeWeb(0, 2, beamZ + 1, 0.1f);
            maybeWeb(2, 2, beamZ + 1, 0.1f);
            maybeWeb(0, 2, beamZ - 2, 0.05f);
            maybeWeb(2, 2, beamZ - 2, 0.05f);
            maybeWeb(0, 2, beamZ + 2, 0.05f);
            maybeWeb(2, 2, beamZ + 2, 0.05f);

            // Torch: 5% chance at center position near supports
            if (rng.nextFloat() < 0.05f) {
                int wx, wy, wz;
                toWorld(1, 2, beamZ - 1, wx, wy, wz);
                if (beamZ - 1 >= 0 && beamZ - 1 <= corridorLen) {
                    setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, TORCH);
                }
            }
            if (rng.nextFloat() < 0.05f) {
                int wx, wy, wz;
                toWorld(1, 2, beamZ + 1, wx, wy, wz);
                if (beamZ + 1 >= 0 && beamZ + 1 <= corridorLen) {
                    setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, TORCH);
                }
            }

            // Spider spawner (one per spider corridor)
            if (piece.hasSpiders && !spawnerPlaced) {
                int spawnZ = beamZ - 1 + rng.nextInt(3);
                int wx, wy, wz;
                toWorld(1, 0, spawnZ, wx, wy, wz);
                int lx = wx - worldMinX;
                int lz2 = wz - worldMinZ;
                if (lx >= 0 && lx <= 15 && lz2 >= 0 && lz2 <= 15 && wy >= 1 && wy <= 254) {
                    setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, SPAWNER);
                    Chunk::SpawnerInfo si;
                    si.x = wx;
                    si.y = wy;
                    si.z = wz;
                    si.entityId = "CaveSpider";
                    spawners.push_back(std::move(si));
                    spawnerPlaced = true;
                }
            }
        }

        // Floor: place planks where air would be below
        for (int lx = 0; lx <= 2; ++lx) {
            for (int lz = 0; lz <= corridorLen; ++lz) {
                int wx, wy, wz;
                toWorld(lx, -1, lz, wx, wy, wz);
                int32_t below = getBlockAt(blocks, worldMinX, worldMinZ, wx, wy, wz);
                if (below == AIR || isLiquid(below)) {
                    setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, PLANKS);
                }
            }
        }

        // Rails: 70% chance per tile on solid floor
        if (piece.hasRails) {
            for (int lz = 0; lz <= corridorLen; ++lz) {
                int wx, wy, wz;
                toWorld(1, 0, lz, wx, wy, wz);
                // Check floor below is solid
                int wxf, wyf, wzf;
                toWorld(1, -1, lz, wxf, wyf, wzf);
                int32_t floor = getBlockAt(blocks, worldMinX, worldMinZ, wxf, wyf, wzf);
                if (floor != AIR && !isLiquid(floor)) {
                    if (rng.nextFloat() < 0.7f) {
                        // Rail meta: 0 = north-south, 1 = east-west
                        uint8_t railMeta = (dir == 0 || dir == 2) ? 0 : 1;
                        setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, RAIL, railMeta);
                    }
                }
            }
        }
    }

    // ──────────────── Cross placement ────────────────
    // Java: Cross.addComponentParts
    void placeCross(const Piece& piece, JavaRNG& rng,
                    int worldMinX, int worldMinZ,
                    int32_t* blocks, uint8_t* meta) {
        if (!overlapsChunk(piece.bb, worldMinX, worldMinZ)) return;
        if (isLiquidInBB(piece.bb, blocks, worldMinX, worldMinZ)) return;

        const auto& bb = piece.bb;

        if (piece.isMultiFloor) {
            // Lower floor: cross-shaped air carving
            fillBox(blocks, meta, worldMinX, worldMinZ,
                    bb.minX + 1, bb.minY, bb.minZ, bb.maxX - 1, bb.minY + 2, bb.maxZ, AIR);
            fillBox(blocks, meta, worldMinX, worldMinZ,
                    bb.minX, bb.minY, bb.minZ + 1, bb.maxX, bb.minY + 2, bb.maxZ - 1, AIR);
            // Upper floor: cross-shaped air carving
            fillBox(blocks, meta, worldMinX, worldMinZ,
                    bb.minX + 1, bb.maxY - 2, bb.minZ, bb.maxX - 1, bb.maxY, bb.maxZ, AIR);
            fillBox(blocks, meta, worldMinX, worldMinZ,
                    bb.minX, bb.maxY - 2, bb.minZ + 1, bb.maxX, bb.maxY, bb.maxZ - 1, AIR);
            // Middle divider
            fillBox(blocks, meta, worldMinX, worldMinZ,
                    bb.minX + 1, bb.minY + 3, bb.minZ + 1, bb.maxX - 1, bb.minY + 3, bb.maxZ - 1, AIR);
        } else {
            // Single floor: cross-shaped air carving
            fillBox(blocks, meta, worldMinX, worldMinZ,
                    bb.minX + 1, bb.minY, bb.minZ, bb.maxX - 1, bb.maxY, bb.maxZ, AIR);
            fillBox(blocks, meta, worldMinX, worldMinZ,
                    bb.minX, bb.minY, bb.minZ + 1, bb.maxX, bb.maxY, bb.maxZ - 1, AIR);
        }

        // Corner pillars: planks columns
        for (int y = bb.minY; y <= bb.maxY; ++y) {
            setBlockAt(blocks, meta, worldMinX, worldMinZ, bb.minX + 1, y, bb.minZ + 1, PLANKS);
            setBlockAt(blocks, meta, worldMinX, worldMinZ, bb.minX + 1, y, bb.maxZ - 1, PLANKS);
            setBlockAt(blocks, meta, worldMinX, worldMinZ, bb.maxX - 1, y, bb.minZ + 1, PLANKS);
            setBlockAt(blocks, meta, worldMinX, worldMinZ, bb.maxX - 1, y, bb.maxZ - 1, PLANKS);
        }

        // Floor: planks where air below
        for (int x = bb.minX; x <= bb.maxX; ++x) {
            for (int z = bb.minZ; z <= bb.maxZ; ++z) {
                int32_t below = getBlockAt(blocks, worldMinX, worldMinZ, x, bb.minY - 1, z);
                if (below == AIR || isLiquid(below)) {
                    setBlockAt(blocks, meta, worldMinX, worldMinZ, x, bb.minY - 1, z, PLANKS);
                }
            }
        }
    }

    // ──────────────── Stairs placement ────────────────
    // Java: Stairs.addComponentParts
    void placeStairs(const Piece& piece, JavaRNG& rng,
                     int worldMinX, int worldMinZ,
                     int32_t* blocks, uint8_t* meta) {
        if (!overlapsChunk(piece.bb, worldMinX, worldMinZ)) return;
        if (isLiquidInBB(piece.bb, blocks, worldMinX, worldMinZ)) return;

        const auto& bb = piece.bb;
        int dir = piece.direction;

        // Java Corridor orientation: local coords where Z is along the corridor direction
        auto toWorld = [&](int localX, int localY, int localZ, int& wx, int& wy, int& wz) {
            wy = bb.minY + localY;
            switch (dir) {
                case 0: wx = bb.minX + localX; wz = bb.minZ + localZ; break;
                case 1: wx = bb.maxX - localZ; wz = bb.minZ + localX; break;
                case 2: wx = bb.minX + localX; wz = bb.maxZ - localZ; break;
                case 3: wx = bb.minX + localZ; wz = bb.minZ + localX; break;
            }
        };

        // Upper landing: 3×2 air at top
        for (int lx = 0; lx <= 2; ++lx) {
            for (int lz = 0; lz <= 1; ++lz) {
                for (int ly = 5; ly <= 7; ++ly) {
                    int wx, wy, wz;
                    toWorld(lx, ly, lz, wx, wy, wz);
                    setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, AIR);
                }
            }
        }
        // Lower landing: 3×2 air at bottom
        for (int lx = 0; lx <= 2; ++lx) {
            for (int lz = 7; lz <= 8; ++lz) {
                for (int ly = 0; ly <= 2; ++ly) {
                    int wx, wy, wz;
                    toWorld(lx, ly, lz, wx, wy, wz);
                    setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, AIR);
                }
            }
        }
        // Stair steps: diagonal carving
        for (int step = 0; step < 5; ++step) {
            int yBot = 5 - step - (step < 4 ? 1 : 0);
            int yTop = 7 - step;
            int lz = 2 + step;
            for (int lx = 0; lx <= 2; ++lx) {
                for (int ly = yBot; ly <= yTop; ++ly) {
                    int wx, wy, wz;
                    toWorld(lx, ly, lz, wx, wy, wz);
                    setBlockAt(blocks, meta, worldMinX, worldMinZ, wx, wy, wz, AIR);
                }
            }
        }
    }
};

} // namespace mccpp
