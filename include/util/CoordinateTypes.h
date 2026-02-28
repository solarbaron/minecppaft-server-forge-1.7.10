/**
 * CoordinateTypes.h — Minecraft coordinate types and ray trace results.
 *
 * Java references:
 *   - net.minecraft.util.MovingObjectPosition — Ray trace hit result
 *   - net.minecraft.util.ChunkCoordinates — Block position (int x,y,z)
 *   - net.minecraft.world.ChunkCoordIntPair — Chunk column (int x,z)
 *   - net.minecraft.world.ChunkPosition — Block position (alternative)
 *
 * These are fundamental coordinate primitives used throughout the codebase.
 *
 * Thread safety: Value types, no shared state.
 *
 * JNI readiness: Simple POD-like structs, trivially copyable.
 */
#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// BlockPos — Integer block position (x, y, z).
// Java reference: net.minecraft.util.ChunkCoordinates
// ═══════════════════════════════════════════════════════════════════════════

struct BlockPos {
    int32_t x = 0, y = 0, z = 0;

    BlockPos() = default;
    BlockPos(int32_t x, int32_t y, int32_t z) : x(x), y(y), z(z) {}

    void set(int32_t nx, int32_t ny, int32_t nz) { x = nx; y = ny; z = nz; }

    bool operator==(const BlockPos& o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const BlockPos& o) const { return !(*this == o); }

    // Java: compareTo — Y first, then Z, then X
    bool operator<(const BlockPos& o) const {
        if (y != o.y) return y < o.y;
        if (z != o.z) return z < o.z;
        return x < o.x;
    }

    // Java: hashCode — x + z<<8 + y<<16
    std::size_t hash() const {
        return static_cast<std::size_t>(x + (z << 8) + (y << 16));
    }

    // Java: getDistanceSquared
    float getDistanceSquared(int32_t ox, int32_t oy, int32_t oz) const {
        float dx = static_cast<float>(x - ox);
        float dy = static_cast<float>(y - oy);
        float dz = static_cast<float>(z - oz);
        return dx * dx + dy * dy + dz * dz;
    }

    float getDistanceSquaredTo(const BlockPos& o) const {
        return getDistanceSquared(o.x, o.y, o.z);
    }

    std::string toString() const {
        return "Pos{x=" + std::to_string(x) + ", y=" + std::to_string(y) +
               ", z=" + std::to_string(z) + "}";
    }
};

} // namespace mccpp

// Hash specialization for use in unordered_map
namespace std {
    template<> struct hash<mccpp::BlockPos> {
        std::size_t operator()(const mccpp::BlockPos& p) const { return p.hash(); }
    };
}

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// ChunkPos — Chunk column position (chunkX, chunkZ).
// Java reference: net.minecraft.world.ChunkCoordIntPair
// ═══════════════════════════════════════════════════════════════════════════

struct ChunkPos {
    int32_t chunkX = 0, chunkZ = 0;

    ChunkPos() = default;
    ChunkPos(int32_t cx, int32_t cz) : chunkX(cx), chunkZ(cz) {}

    // Java: static chunkXZ2Int — pack two chunk coords into one long
    static int64_t chunkXZ2Int(int32_t cx, int32_t cz) {
        return (static_cast<int64_t>(cx) & 0xFFFFFFFFL) |
               ((static_cast<int64_t>(cz) & 0xFFFFFFFFL) << 32);
    }

    int64_t toLong() const { return chunkXZ2Int(chunkX, chunkZ); }

    // Java: getCenter — block position at center of chunk
    int32_t getCenterXPosition() const { return (chunkX << 4) + 8; }
    int32_t getCenterZPosition() const { return (chunkZ << 4) + 8; }

    // Java: getChunkCoordIntPair — get min block position
    int32_t getXStart() const { return chunkX << 4; }
    int32_t getZStart() const { return chunkZ << 4; }
    int32_t getXEnd() const { return (chunkX << 4) + 15; }
    int32_t getZEnd() const { return (chunkZ << 4) + 15; }

    // Java: func_151350_a — get block position within chunk
    BlockPos getBlock(int32_t localX, int32_t y, int32_t localZ) const {
        return {getXStart() + localX, y, getZStart() + localZ};
    }

    bool operator==(const ChunkPos& o) const { return chunkX == o.chunkX && chunkZ == o.chunkZ; }
    bool operator!=(const ChunkPos& o) const { return !(*this == o); }

    // Java: hashCode — LCG hash with DEADBEEF
    std::size_t hash() const {
        int32_t h1 = 1664525 * chunkX + 1013904223;
        int32_t h2 = 1664525 * (chunkZ ^ static_cast<int32_t>(0xDEADBEEF)) + 1013904223;
        return static_cast<std::size_t>(h1 ^ h2);
    }

    std::string toString() const {
        return "[" + std::to_string(chunkX) + ", " + std::to_string(chunkZ) + "]";
    }
};

} // namespace mccpp

namespace std {
    template<> struct hash<mccpp::ChunkPos> {
        std::size_t operator()(const mccpp::ChunkPos& p) const { return p.hash(); }
    };
}

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// MovingObjectType — What the ray trace hit.
// Java reference: net.minecraft.util.MovingObjectPosition$MovingObjectType
// ═══════════════════════════════════════════════════════════════════════════

enum class MovingObjectType : int32_t {
    MISS   = 0,
    BLOCK  = 1,
    ENTITY = 2
};

// ═══════════════════════════════════════════════════════════════════════════
// MovingObjectPosition — Full ray trace result.
// Java reference: net.minecraft.util.MovingObjectPosition
// ═══════════════════════════════════════════════════════════════════════════

struct MovingObjectPosition {
    MovingObjectType typeOfHit = MovingObjectType::MISS;
    int32_t blockX = 0, blockY = 0, blockZ = 0;
    int32_t sideHit = -1;       // 0=bottom, 1=top, 2=north, 3=south, 4=west, 5=east
    double hitVecX = 0.0, hitVecY = 0.0, hitVecZ = 0.0;
    int32_t entityHitId = -1;   // Entity ID if type == ENTITY

    // Java: block constructor
    static MovingObjectPosition blockHit(int32_t bx, int32_t by, int32_t bz,
                                          int32_t side, double hx, double hy, double hz) {
        MovingObjectPosition mop;
        mop.typeOfHit = MovingObjectType::BLOCK;
        mop.blockX = bx; mop.blockY = by; mop.blockZ = bz;
        mop.sideHit = side;
        mop.hitVecX = hx; mop.hitVecY = hy; mop.hitVecZ = hz;
        return mop;
    }

    // Java: entity constructor
    static MovingObjectPosition entityHit(int32_t entityId, double hx, double hy, double hz) {
        MovingObjectPosition mop;
        mop.typeOfHit = MovingObjectType::ENTITY;
        mop.entityHitId = entityId;
        mop.hitVecX = hx; mop.hitVecY = hy; mop.hitVecZ = hz;
        return mop;
    }

    // Java: miss (constructor with false)
    static MovingObjectPosition miss(int32_t bx, int32_t by, int32_t bz,
                                      int32_t side, double hx, double hy, double hz) {
        MovingObjectPosition mop;
        mop.typeOfHit = MovingObjectType::MISS;
        mop.blockX = bx; mop.blockY = by; mop.blockZ = bz;
        mop.sideHit = side;
        mop.hitVecX = hx; mop.hitVecY = hy; mop.hitVecZ = hz;
        return mop;
    }

    bool isBlock() const { return typeOfHit == MovingObjectType::BLOCK; }
    bool isEntity() const { return typeOfHit == MovingObjectType::ENTITY; }
    bool isMiss() const { return typeOfHit == MovingObjectType::MISS; }

    std::string toString() const {
        return "HitResult{type=" + std::to_string(static_cast<int>(typeOfHit)) +
               ", x=" + std::to_string(blockX) + ", y=" + std::to_string(blockY) +
               ", z=" + std::to_string(blockZ) + ", f=" + std::to_string(sideHit) + "}";
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Side/Face constants
// Java: Various places reference these as int values 0-5
// ═══════════════════════════════════════════════════════════════════════════

namespace ForgeDirection {
    constexpr int32_t DOWN  = 0; // -Y
    constexpr int32_t UP    = 1; // +Y
    constexpr int32_t NORTH = 2; // -Z
    constexpr int32_t SOUTH = 3; // +Z
    constexpr int32_t WEST  = 4; // -X
    constexpr int32_t EAST  = 5; // +X

    // Opposite face
    constexpr int32_t getOpposite(int32_t side) {
        return side ^ 1; // 0↔1, 2↔3, 4↔5
    }

    // Axis offset for each direction
    constexpr int32_t OFFSET_X[6] = { 0,  0,  0,  0, -1,  1};
    constexpr int32_t OFFSET_Y[6] = {-1,  1,  0,  0,  0,  0};
    constexpr int32_t OFFSET_Z[6] = { 0,  0, -1,  1,  0,  0};
}

} // namespace mccpp
