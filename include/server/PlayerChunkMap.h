/**
 * PlayerChunkMap.h — Per-world chunk-to-player tracking system.
 *
 * Java reference: net.minecraft.server.management.PlayerManager (246 lines)
 *
 * Tracks which chunks each player is subscribed to based on their position
 * and the server's view distance. When a player moves, chunks enter/leave
 * their view radius, triggering chunk load/unload packets.
 *
 * Key algorithms:
 *
 * 1. addPlayer (line 86-98):
 *    - Subscribe to all chunks in square: [x-radius, x+radius] × [z-radius, z+radius]
 *    - Store managedPosX/Z for later delta comparison
 *    - filterChunkLoadQueue: reorder in spiral (center outward)
 *
 * 2. removePlayer (line 132-143):
 *    - Unsubscribe from all chunks in managedPos radius
 *
 * 3. updateMountedMovingPlayer (line 154-184):
 *    - Movement threshold: 64.0 squared distance
 *    - For each chunk in new view: if not in old view → subscribe
 *    - For each chunk in old view: if not in new view → unsubscribe
 *    - Uses overlaps() for efficient containment check
 *
 * 4. filterChunkLoadQueue (line 100-130):
 *    - Spiral pattern from center outward
 *    - Direction constants: {{1,0},{0,1},{-1,0},{0,-1}}
 *    - Spiral: for n=1 to radius*2, 2 sides of length n, rotating
 *    - Final side: radius*2 in last direction
 *
 * 5. func_152622_a — view distance resize (line 191-219):
 *    - Clamp to [3, 20]
 *    - If growing: add new chunks for all players
 *    - If shrinking: remove excess chunks for all players
 *
 * 6. updatePlayerInstances (line 38-59):
 *    - Every 8000 ticks: full update of all instances
 *    - Otherwise: only process dirty instances
 *    - Empty dimension with no respawn: unload all chunks
 *
 * Thread safety: Per-dimension manager, ticked on server thread.
 * JNI readiness: Simple data structures with clear lifecycle.
 */
#pragma once

#include <cstdint>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// PlayerChunkMap constants
// ═══════════════════════════════════════════════════════════════════════════

namespace ChunkMapConstants {
    // ─── View distance clamp ───
    // Java: MathHelper.clamp_int(n, 3, 20)
    static constexpr int32_t MIN_VIEW_DISTANCE = 3;
    static constexpr int32_t MAX_VIEW_DISTANCE = 20;

    // ─── Movement threshold ───
    // Java: d * d + d2 * d2 >= 64.0 → recalculate chunks
    // This is 8 blocks squared (8*8 = 64)
    static constexpr double MOVEMENT_THRESHOLD_SQ = 64.0;

    // ─── Full update interval ───
    // Java: if (l - previousTotalWorldTime > 8000L)
    // Every 8000 ticks (6 min 40 sec) do a full pass
    static constexpr int64_t FULL_UPDATE_INTERVAL = 8000;

    // ─── Spiral direction constants ───
    // Java: xzDirectionsConst = {{1,0},{0,1},{-1,0},{0,-1}}
    // Used for center-outward spiral chunk loading order
    static constexpr std::array<std::array<int32_t, 2>, 4> SPIRAL_DIRS = {{
        {{1, 0}}, {{0, 1}}, {{-1, 0}}, {{0, -1}}
    }};

    // ─── Furthest viewable block ───
    // Java: getFurthestViewableBlock(n) = n * 16 - 16
    inline int32_t getFurthestViewableBlock(int32_t viewDistance) {
        return viewDistance * 16 - 16;
    }

    // ─── Chunk coord hash key ───
    // Java: (long)n + Integer.MAX_VALUE | (long)n2 + Integer.MAX_VALUE << 32
    inline int64_t chunkHash(int32_t x, int32_t z) {
        // Offset by INT_MAX to make all values positive before combining
        int64_t lx = static_cast<int64_t>(x) + static_cast<int64_t>(INT32_MAX);
        int64_t lz = static_cast<int64_t>(z) + static_cast<int64_t>(INT32_MAX);
        return lx | (lz << 32);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// ChunkCoordIntPair — packed chunk coordinates
// Java: net.minecraft.world.ChunkCoordIntPair
// ═══════════════════════════════════════════════════════════════════════════

struct ChunkCoordIntPair {
    int32_t chunkXPos;
    int32_t chunkZPos;

    ChunkCoordIntPair() : chunkXPos(0), chunkZPos(0) {}
    ChunkCoordIntPair(int32_t x, int32_t z) : chunkXPos(x), chunkZPos(z) {}

    // Java: getCenterXPos / getCenterZPosition
    int32_t getCenterX() const { return (chunkXPos << 4) + 8; }
    int32_t getCenterZ() const { return (chunkZPos << 4) + 8; }

    // Java: getXStart / getZStart
    int32_t getXStart() const { return chunkXPos << 4; }
    int32_t getZStart() const { return chunkZPos << 4; }

    // Java: getXEnd / getZEnd
    int32_t getXEnd() const { return (chunkXPos << 4) + 15; }
    int32_t getZEnd() const { return (chunkZPos << 4) + 15; }

    bool operator==(const ChunkCoordIntPair& o) const {
        return chunkXPos == o.chunkXPos && chunkZPos == o.chunkZPos;
    }

    // Hash for unordered containers
    struct Hash {
        size_t operator()(const ChunkCoordIntPair& p) const {
            return std::hash<int64_t>()(ChunkMapConstants::chunkHash(p.chunkXPos, p.chunkZPos));
        }
    };
};

// ═══════════════════════════════════════════════════════════════════════════
// PlayerChunkMap — Manages chunk subscriptions per player.
// ═══════════════════════════════════════════════════════════════════════════

class PlayerChunkMap {
public:
    int32_t playerViewRadius = 10;  // Default view distance

    // ─── overlaps ───
    // Java: PlayerManager.overlaps(cx, cz, px, pz, radius)
    // Returns true if chunk (cx, cz) is within radius of (px, pz)
    static bool overlaps(int32_t cx, int32_t cz, int32_t px, int32_t pz, int32_t radius) {
        int32_t dx = cx - px;
        int32_t dz = cz - pz;
        return dx >= -radius && dx <= radius && dz >= -radius && dz <= radius;
    }

    // ─── View distance management ───
    // Java: func_152622_a(n) — set and resize view distance
    int32_t clampViewDistance(int32_t distance) const {
        if (distance < ChunkMapConstants::MIN_VIEW_DISTANCE)
            return ChunkMapConstants::MIN_VIEW_DISTANCE;
        if (distance > ChunkMapConstants::MAX_VIEW_DISTANCE)
            return ChunkMapConstants::MAX_VIEW_DISTANCE;
        return distance;
    }

    // ─── Movement threshold check ───
    // Java: d * d + d2 * d2 >= 64.0
    static bool hasMovedEnough(double oldX, double oldZ, double newX, double newZ) {
        double dx = oldX - newX;
        double dz = oldZ - newZ;
        return (dx * dx + dz * dz) >= ChunkMapConstants::MOVEMENT_THRESHOLD_SQ;
    }

    // ─── Spiral chunk load ordering ───
    // Java: filterChunkLoadQueue — spiral from center outward
    // Generates chunk coords in center-outward spiral order within view radius
    static std::vector<ChunkCoordIntPair> generateSpiral(int32_t centerX, int32_t centerZ,
                                                          int32_t radius)
    {
        std::vector<ChunkCoordIntPair> result;
        int32_t totalChunks = (2 * radius + 1) * (2 * radius + 1);
        result.reserve(totalChunks);

        // Start at center
        result.emplace_back(centerX, centerZ);

        int32_t x = 0, z = 0;
        int32_t dirIdx = 0;

        // Java spiral pattern: for n=1 to radius*2, two sides of length n
        for (int32_t n = 1; n <= radius * 2; ++n) {
            for (int32_t side = 0; side < 2; ++side) {
                const auto& dir = ChunkMapConstants::SPIRAL_DIRS[dirIdx % 4];
                for (int32_t step = 0; step < n; ++step) {
                    x += dir[0];
                    z += dir[1];
                    result.emplace_back(centerX + x, centerZ + z);
                }
                ++dirIdx;
            }
        }

        // Final side: radius*2 in the last direction
        dirIdx %= 4;
        for (int32_t step = 0; step < radius * 2; ++step) {
            x += ChunkMapConstants::SPIRAL_DIRS[dirIdx][0];
            z += ChunkMapConstants::SPIRAL_DIRS[dirIdx][1];
            result.emplace_back(centerX + x, centerZ + z);
        }

        return result;
    }

    // ─── Compute chunk delta ───
    // Returns chunks that are in newView but not in oldView
    static std::vector<ChunkCoordIntPair> computeNewChunks(
        int32_t oldCX, int32_t oldCZ,
        int32_t newCX, int32_t newCZ,
        int32_t radius)
    {
        std::vector<ChunkCoordIntPair> result;
        for (int32_t x = newCX - radius; x <= newCX + radius; ++x) {
            for (int32_t z = newCZ - radius; z <= newCZ + radius; ++z) {
                if (!overlaps(x, z, oldCX, oldCZ, radius)) {
                    result.emplace_back(x, z);
                }
            }
        }
        return result;
    }

    // Returns chunks that are in oldView but not in newView
    static std::vector<ChunkCoordIntPair> computeRemovedChunks(
        int32_t oldCX, int32_t oldCZ,
        int32_t newCX, int32_t newCZ,
        int32_t radius)
    {
        std::vector<ChunkCoordIntPair> result;
        for (int32_t x = oldCX - radius; x <= oldCX + radius; ++x) {
            for (int32_t z = oldCZ - radius; z <= oldCZ + radius; ++z) {
                if (!overlaps(x, z, newCX, newCZ, radius)) {
                    result.emplace_back(x, z);
                }
            }
        }
        return result;
    }

    // ─── Furthest viewable block ───
    int32_t getFurthestViewableBlock() const {
        return ChunkMapConstants::getFurthestViewableBlock(playerViewRadius);
    }
};

} // namespace mccpp
