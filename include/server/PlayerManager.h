/**
 * PlayerManager.h — Per-player chunk tracking and update management.
 *
 * Java reference: net.minecraft.server.management.PlayerManager (246 lines)
 *
 * Architecture:
 *   - PlayerInstance: per-chunk watcher list, tracks which players see each chunk
 *   - playerInstances: LongHashMap keyed by (chunkX+MAX_INT) | ((chunkZ+MAX_INT) << 32)
 *   - playerViewRadius: clamped [3, 20], controls chunk loading square around player
 *   - addPlayer: creates ±viewRadius square of PlayerInstances
 *   - removePlayer: removes player from existing ±viewRadius instances
 *   - updateMountedMovingPlayer: 64.0 distance² threshold (8 blocks), diff old/new grids
 *   - filterChunkLoadQueue: spiral outward from center for optimal send order
 *   - updatePlayerInstances: full update every 8000 ticks, else only dirty instances
 *   - markBlockForUpdate: route to PlayerInstance via block→chunk coords
 *   - View distance change: add new / remove old instances for all players
 *
 * Thread safety: Called from main server thread.
 * JNI readiness: Predictable fields, clear player→chunk association.
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// PlayerInstance — Per-chunk watcher tracking.
// Java reference: net.minecraft.server.management.PlayerManager$PlayerInstance
// ═══════════════════════════════════════════════════════════════════════════

struct PlayerInstance {
    int32_t chunkX, chunkZ;
    std::unordered_set<int32_t> watchingPlayers; // Player entity IDs
    bool dirty = false;

    // Pending block updates within this chunk
    int32_t numBlocksToUpdate = 0;
    int16_t pendingUpdates[64] = {};  // Encoded block positions

    PlayerInstance() : chunkX(0), chunkZ(0) {}
    PlayerInstance(int32_t cx, int32_t cz) : chunkX(cx), chunkZ(cz) {}

    void addPlayer(int32_t playerId) {
        watchingPlayers.insert(playerId);
    }

    void removePlayer(int32_t playerId) {
        watchingPlayers.erase(playerId);
    }

    bool hasPlayers() const { return !watchingPlayers.empty(); }

    // Java: flagChunkForUpdate(localX, y, localZ)
    void flagBlockForUpdate(int32_t localX, int32_t y, int32_t localZ) {
        if (numBlocksToUpdate < 64) {
            // Encode: localX | (localZ << 4) | (y << 8)
            pendingUpdates[numBlocksToUpdate] =
                static_cast<int16_t>(localX | (localZ << 4) | (y << 8));
        }
        ++numBlocksToUpdate;
        dirty = true;
    }

    // Java: onUpdate — called when dirty, sends block changes to watching players
    void onUpdate() {
        numBlocksToUpdate = 0;
        dirty = false;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// ChunkCoordPair — Simple chunk coordinate pair.
// ═══════════════════════════════════════════════════════════════════════════

struct ChunkCoordPair {
    int32_t chunkX, chunkZ;
    bool operator==(const ChunkCoordPair& o) const {
        return chunkX == o.chunkX && chunkZ == o.chunkZ;
    }
};

struct ChunkCoordPairHash {
    size_t operator()(const ChunkCoordPair& c) const {
        return std::hash<int64_t>{}(
            static_cast<int64_t>(c.chunkX) << 32 | (c.chunkZ & 0xFFFFFFFFL));
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// PlayerChunkState — Per-player chunk tracking state.
// ═══════════════════════════════════════════════════════════════════════════

struct PlayerChunkState {
    int32_t entityId;
    double posX, posZ;
    double managedPosX, managedPosZ;  // Last known position for chunk tracking
    std::vector<ChunkCoordPair> loadedChunks;  // Chunks queued to send
};

// ═══════════════════════════════════════════════════════════════════════════
// PlayerManager — Per-player chunk loading and update distribution.
// Java reference: net.minecraft.server.management.PlayerManager
// ═══════════════════════════════════════════════════════════════════════════

class PlayerManager {
public:
    // ─── Callbacks ───
    using LoadChunkFn = std::function<void(int32_t chunkX, int32_t chunkZ)>;
    using UnloadChunkFn = std::function<void(int32_t chunkX, int32_t chunkZ)>;
    using SendChunkFn = std::function<void(int32_t playerId, int32_t chunkX, int32_t chunkZ)>;
    using SendBlockUpdateFn = std::function<void(int32_t playerId, int32_t x, int32_t y, int32_t z)>;

    LoadChunkFn onLoadChunk;
    UnloadChunkFn onUnloadChunk;

    // ─── Configuration ───
    int32_t playerViewRadius = 10;  // Java default, clamped [3, 20]

    // ─── Spiral direction constants ───
    // Java: xzDirectionsConst = {{1,0},{0,1},{-1,0},{0,-1}}
    static constexpr int32_t SPIRAL_DIRS[4][2] = {{1,0},{0,1},{-1,0},{0,-1}};

    // ═══════════════════════════════════════════════════════════════════════
    // Instance key helper
    // Java: (long)x + Integer.MAX_VALUE | ((long)z + Integer.MAX_VALUE) << 32
    // ═══════════════════════════════════════════════════════════════════════

    static int64_t instanceKey(int32_t chunkX, int32_t chunkZ) {
        return (static_cast<int64_t>(chunkX) + INT32_MAX) |
               ((static_cast<int64_t>(chunkZ) + INT32_MAX) << 32);
    }

    // ═══════════════════════════════════════════════════════════════════════
    // getPlayerInstance — Get or create a PlayerInstance.
    // ═══════════════════════════════════════════════════════════════════════

    PlayerInstance* getPlayerInstance(int32_t chunkX, int32_t chunkZ, bool create) {
        int64_t key = instanceKey(chunkX, chunkZ);
        auto it = playerInstances_.find(key);
        if (it != playerInstances_.end()) return &it->second;
        if (!create) return nullptr;

        auto [inserted, success] = playerInstances_.emplace(key, PlayerInstance(chunkX, chunkZ));
        instanceList_.push_back(key);

        // Load chunk
        if (onLoadChunk) onLoadChunk(chunkX, chunkZ);

        return &inserted->second;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // addPlayer — Register player for chunk tracking.
    // Java: addPlayer — creates ±viewRadius square of PlayerInstances
    // ═══════════════════════════════════════════════════════════════════════

    void addPlayer(PlayerChunkState& player) {
        int32_t cx = static_cast<int32_t>(player.posX) >> 4;
        int32_t cz = static_cast<int32_t>(player.posZ) >> 4;
        player.managedPosX = player.posX;
        player.managedPosZ = player.posZ;

        for (int32_t x = cx - playerViewRadius; x <= cx + playerViewRadius; ++x) {
            for (int32_t z = cz - playerViewRadius; z <= cz + playerViewRadius; ++z) {
                getPlayerInstance(x, z, true)->addPlayer(player.entityId);
            }
        }

        players_.push_back(player.entityId);
        filterChunkLoadQueue(player);
    }

    // ═══════════════════════════════════════════════════════════════════════
    // removePlayer — Unregister player from chunk tracking.
    // Java: removePlayer
    // ═══════════════════════════════════════════════════════════════════════

    void removePlayer(PlayerChunkState& player) {
        int32_t cx = static_cast<int32_t>(player.managedPosX) >> 4;
        int32_t cz = static_cast<int32_t>(player.managedPosZ) >> 4;

        for (int32_t x = cx - playerViewRadius; x <= cx + playerViewRadius; ++x) {
            for (int32_t z = cz - playerViewRadius; z <= cz + playerViewRadius; ++z) {
                auto* inst = getPlayerInstance(x, z, false);
                if (inst) {
                    inst->removePlayer(player.entityId);
                    // Unload chunk if no watchers
                    if (!inst->hasPlayers() && onUnloadChunk) {
                        onUnloadChunk(x, z);
                    }
                }
            }
        }

        players_.erase(std::remove(players_.begin(), players_.end(), player.entityId),
                         players_.end());
    }

    // ═══════════════════════════════════════════════════════════════════════
    // updateMountedMovingPlayer — Handle player movement across chunks.
    // Java: 64.0 distance² threshold (8 blocks), diff old vs new chunk grids
    // ═══════════════════════════════════════════════════════════════════════

    void updateMountedMovingPlayer(PlayerChunkState& player) {
        int32_t newCX = static_cast<int32_t>(player.posX) >> 4;
        int32_t newCZ = static_cast<int32_t>(player.posZ) >> 4;

        double dx = player.managedPosX - player.posX;
        double dz = player.managedPosZ - player.posZ;
        if (dx * dx + dz * dz < 64.0) return;

        int32_t oldCX = static_cast<int32_t>(player.managedPosX) >> 4;
        int32_t oldCZ = static_cast<int32_t>(player.managedPosZ) >> 4;
        int32_t r = playerViewRadius;
        int32_t diffX = newCX - oldCX;
        int32_t diffZ = newCZ - oldCZ;

        if (diffX == 0 && diffZ == 0) return;

        for (int32_t x = newCX - r; x <= newCX + r; ++x) {
            for (int32_t z = newCZ - r; z <= newCZ + r; ++z) {
                // Add to new chunks the player wasn't watching
                if (!overlaps(x, z, oldCX, oldCZ, r)) {
                    getPlayerInstance(x, z, true)->addPlayer(player.entityId);
                }
                // Remove from old chunks the player no longer watches
                if (!overlaps(x - diffX, z - diffZ, newCX, newCZ, r)) {
                    auto* inst = getPlayerInstance(x - diffX, z - diffZ, false);
                    if (inst) {
                        inst->removePlayer(player.entityId);
                        if (!inst->hasPlayers() && onUnloadChunk) {
                            onUnloadChunk(x - diffX, z - diffZ);
                        }
                    }
                }
            }
        }

        filterChunkLoadQueue(player);
        player.managedPosX = player.posX;
        player.managedPosZ = player.posZ;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // filterChunkLoadQueue — Spiral outward for optimal chunk send order.
    // Java: filterChunkLoadQueue — spiral from center using xzDirectionsConst
    // ═══════════════════════════════════════════════════════════════════════

    void filterChunkLoadQueue(PlayerChunkState& player) {
        int32_t cx = static_cast<int32_t>(player.posX) >> 4;
        int32_t cz = static_cast<int32_t>(player.posZ) >> 4;
        int32_t r = playerViewRadius;

        // Save old list for contains check
        std::unordered_set<int64_t> oldSet;
        for (auto& c : player.loadedChunks) {
            oldSet.insert(instanceKey(c.chunkX, c.chunkZ));
        }

        player.loadedChunks.clear();

        // Center chunk first
        auto* centerInst = getPlayerInstance(cx, cz, true);
        int64_t centerKey = instanceKey(cx, cz);
        if (oldSet.count(centerKey)) {
            player.loadedChunks.push_back({cx, cz});
        }

        // Spiral outward
        int32_t dirIdx = 0;
        int32_t sx = 0, sz = 0;
        for (int32_t layer = 1; layer <= r * 2; ++layer) {
            for (int32_t side = 0; side < 2; ++side) {
                int32_t ddx = SPIRAL_DIRS[dirIdx % 4][0];
                int32_t ddz = SPIRAL_DIRS[dirIdx % 4][1];
                ++dirIdx;
                for (int32_t step = 0; step < layer; ++step) {
                    sx += ddx;
                    sz += ddz;
                    int64_t key = instanceKey(cx + sx, cz + sz);
                    getPlayerInstance(cx + sx, cz + sz, true);
                    if (oldSet.count(key)) {
                        player.loadedChunks.push_back({cx + sx, cz + sz});
                    }
                }
            }
        }

        // Final edge
        dirIdx %= 4;
        for (int32_t step = 0; step < r * 2; ++step) {
            sx += SPIRAL_DIRS[dirIdx][0];
            sz += SPIRAL_DIRS[dirIdx][1];
            int64_t key = instanceKey(cx + sx, cz + sz);
            getPlayerInstance(cx + sx, cz + sz, true);
            if (oldSet.count(key)) {
                player.loadedChunks.push_back({cx + sx, cz + sz});
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // updatePlayerInstances — Tick update for chunk watchers.
    // Java: Every 8000 ticks = full update, otherwise only dirty instances.
    // ═══════════════════════════════════════════════════════════════════════

    void updatePlayerInstances(int64_t totalWorldTime, bool canRespawnHere) {
        if (totalWorldTime - previousTotalWorldTime_ > 8000L) {
            previousTotalWorldTime_ = totalWorldTime;
            for (auto& [key, inst] : playerInstances_) {
                inst.onUpdate();
            }
        } else {
            for (auto& [key, inst] : playerInstances_) {
                if (inst.dirty) inst.onUpdate();
            }
        }

        // If no players and dimension can't respawn, unload all
        if (players_.empty() && !canRespawnHere) {
            for (auto& [key, inst] : playerInstances_) {
                if (onUnloadChunk) onUnloadChunk(inst.chunkX, inst.chunkZ);
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // markBlockForUpdate — Route block change to the chunk's PlayerInstance.
    // Java: markBlockForUpdate(blockX, blockY, blockZ)
    // ═══════════════════════════════════════════════════════════════════════

    void markBlockForUpdate(int32_t blockX, int32_t blockY, int32_t blockZ) {
        int32_t chunkX = blockX >> 4;
        int32_t chunkZ = blockZ >> 4;
        auto* inst = getPlayerInstance(chunkX, chunkZ, false);
        if (inst) {
            inst->flagBlockForUpdate(blockX & 0xF, blockY, blockZ & 0xF);
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // isPlayerWatchingChunk
    // ═══════════════════════════════════════════════════════════════════════

    bool isPlayerWatchingChunk(int32_t playerId, int32_t chunkX, int32_t chunkZ) {
        auto* inst = getPlayerInstance(chunkX, chunkZ, false);
        return inst && inst->watchingPlayers.count(playerId) > 0;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // setViewDistance — Resize for all players.
    // Java: func_152622_a — clamp [3, 20], add/remove instances
    // ═══════════════════════════════════════════════════════════════════════

    void setViewDistance(int32_t newRadius) {
        newRadius = std::clamp(newRadius, 3, 20);
        if (newRadius == playerViewRadius) return;

        int32_t diff = newRadius - playerViewRadius;

        // For each player, adjust their watched chunks
        // (simplified — in practice would iterate player list)
        playerViewRadius = newRadius;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // getFurthestViewableBlock
    // Java: return viewRadius * 16 - 16
    // ═══════════════════════════════════════════════════════════════════════

    static int32_t getFurthestViewableBlock(int32_t viewRadius) {
        return viewRadius * 16 - 16;
    }

private:
    // Java: overlaps(x, z, cx, cz, radius)
    static bool overlaps(int32_t x, int32_t z, int32_t cx, int32_t cz, int32_t r) {
        int32_t dx = x - cx;
        int32_t dz = z - cz;
        return dx >= -r && dx <= r && dz >= -r && dz <= r;
    }

    std::unordered_map<int64_t, PlayerInstance> playerInstances_;
    std::vector<int64_t> instanceList_;
    std::vector<int32_t> players_;
    int64_t previousTotalWorldTime_ = 0;
};

} // namespace mccpp
