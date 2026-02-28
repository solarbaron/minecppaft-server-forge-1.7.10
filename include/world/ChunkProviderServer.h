/**
 * ChunkProviderServer.h — Server-side chunk loading/generation manager.
 *
 * Java reference: net.minecraft.world.gen.ChunkProviderServer (262 lines)
 *
 * Architecture:
 *   - LongHashMap: chunkXZ2Int(x,z) → Chunk*, O(1) lookup
 *   - droppedChunksSet: ConcurrentHashMap-backed set of chunks to unload
 *   - loadChunk flow: cache → loadFromFile → generate → map → onChunkLoad → populate
 *   - provideChunk: cache hit → return, else loadChunk if chunkLoadOverride
 *   - unloadQueuedChunks: max 100/tick, save+unload, skip spawn area (±128 blocks)
 *   - saveChunks: max 24 per call (unless forced)
 *   - populate: guard with isTerrainPopulated flag
 *
 * Thread safety:
 *   - shared_mutex on id2ChunkMap for concurrent reads, exclusive writes
 *   - ConcurrentHashSet via atomic + mutex for droppedChunksSet
 *   - Chunk load/generate may be called from async worker threads
 *
 * JNI readiness: Chunk pointers accessible via predictable API.
 */
#pragma once

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// ChunkData — Minimal chunk representation for the provider.
// ═══════════════════════════════════════════════════════════════════════════

struct ChunkData {
    int32_t xPosition;
    int32_t zPosition;
    int64_t lastSaveTime = 0;
    bool isModified = false;
    bool isTerrainPopulated = false;
    bool isLoaded = false;

    // Terrain data (16×256×16 block IDs + metadata)
    // Stored as raw arrays for JNI compatibility
    uint8_t blockIds[16 * 256 * 16] = {};
    uint8_t blockMeta[16 * 256 * 16 / 2] = {};  // Nibble array

    ChunkData() : xPosition(0), zPosition(0) {}
    ChunkData(int32_t x, int32_t z) : xPosition(x), zPosition(z) {}

    bool needsSaving(bool forced) const {
        return forced || isModified;
    }

    void setModified() { isModified = true; }
};

// ═══════════════════════════════════════════════════════════════════════════
// ChunkProviderServer — Server-side chunk loading and caching.
// Java reference: net.minecraft.world.gen.ChunkProviderServer
// ═══════════════════════════════════════════════════════════════════════════

class ChunkProviderServer {
public:
    bool chunkLoadOverride = true;

    // ─── Callbacks for world integration ───
    using GenerateChunkFn = std::function<ChunkData(int32_t chunkX, int32_t chunkZ)>;
    using LoadChunkFn = std::function<bool(int32_t chunkX, int32_t chunkZ, ChunkData& out)>;
    using SaveChunkFn = std::function<void(const ChunkData& chunk)>;
    using PopulateFn = std::function<void(int32_t chunkX, int32_t chunkZ)>;

    GenerateChunkFn generateChunk;
    LoadChunkFn loadChunkFromFile;
    SaveChunkFn saveChunkToFile;
    PopulateFn populateChunk;

    // ─── Spawn area protection ───
    bool canRespawnHere = true;
    int32_t spawnX = 0, spawnZ = 0;
    static constexpr int32_t SPAWN_PROTECTION_RADIUS = 128;  // blocks

    // ─── World state ───
    std::atomic<int64_t> totalWorldTime{0};
    std::atomic<bool> disableLevelSaving{false};

    // ═══════════════════════════════════════════════════════════════════════
    // Chunk key helper
    // Java: ChunkCoordIntPair.chunkXZ2Int
    // ═══════════════════════════════════════════════════════════════════════

    static int64_t chunkKey(int32_t x, int32_t z) {
        return static_cast<int64_t>(x) & 0xFFFFFFFFL |
               (static_cast<int64_t>(z) & 0xFFFFFFFFL) << 32;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // chunkExists — Check if chunk is in cache.
    // Java: chunkExists
    // ═══════════════════════════════════════════════════════════════════════

    bool chunkExists(int32_t x, int32_t z) {
        std::shared_lock lock(chunkMapMutex_);
        return id2ChunkMap_.count(chunkKey(x, z)) > 0;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // provideChunk — Get chunk, loading/generating if needed.
    // Java: provideChunk
    // ═══════════════════════════════════════════════════════════════════════

    ChunkData* provideChunk(int32_t x, int32_t z) {
        int64_t key = chunkKey(x, z);
        {
            std::shared_lock lock(chunkMapMutex_);
            auto it = id2ChunkMap_.find(key);
            if (it != id2ChunkMap_.end()) return &it->second;
        }

        if (chunkLoadOverride) {
            return loadChunk(x, z);
        }

        return nullptr;  // Dummy chunk equivalent
    }

    // ═══════════════════════════════════════════════════════════════════════
    // loadChunk — Load from file or generate, cache, and populate.
    // Java: loadChunk
    // Thread safety: exclusive lock on chunk map during insertion.
    // ═══════════════════════════════════════════════════════════════════════

    ChunkData* loadChunk(int32_t x, int32_t z) {
        int64_t key = chunkKey(x, z);

        // Remove from unload queue
        {
            std::lock_guard lock(dropSetMutex_);
            droppedChunksSet_.erase(key);
        }

        // Check cache first (with read lock)
        {
            std::shared_lock lock(chunkMapMutex_);
            auto it = id2ChunkMap_.find(key);
            if (it != id2ChunkMap_.end()) return &it->second;
        }

        // Try load from file
        ChunkData chunk(x, z);
        bool loaded = false;
        if (loadChunkFromFile) {
            loaded = loadChunkFromFile(x, z, chunk);
            if (loaded) {
                chunk.lastSaveTime = totalWorldTime.load();
            }
        }

        // Generate if not loaded
        if (!loaded && generateChunk) {
            chunk = generateChunk(x, z);
            chunk.xPosition = x;
            chunk.zPosition = z;
        }

        chunk.isLoaded = true;

        // Insert into cache (exclusive lock)
        ChunkData* result;
        {
            std::unique_lock lock(chunkMapMutex_);
            auto [it, inserted] = id2ChunkMap_.emplace(key, std::move(chunk));
            result = &it->second;
            if (inserted) {
                loadedChunkCoords_.push_back({x, z});
            }
        }

        // Populate if needed
        if (!result->isTerrainPopulated && populateChunk) {
            populateChunk(x, z);
            result->isTerrainPopulated = true;
            result->setModified();
        }

        return result;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // dropChunk — Mark chunk for unloading.
    // Java: dropChunk — skip spawn area protection (±128 blocks)
    // ═══════════════════════════════════════════════════════════════════════

    void dropChunk(int32_t x, int32_t z) {
        if (canRespawnHere) {
            int32_t dx = x * 16 + 8 - spawnX;
            int32_t dz = z * 16 + 8 - spawnZ;
            if (dx >= -SPAWN_PROTECTION_RADIUS && dx <= SPAWN_PROTECTION_RADIUS &&
                dz >= -SPAWN_PROTECTION_RADIUS && dz <= SPAWN_PROTECTION_RADIUS) {
                return;  // Keep spawn chunks loaded
            }
        }
        std::lock_guard lock(dropSetMutex_);
        droppedChunksSet_.insert(chunkKey(x, z));
    }

    // ═══════════════════════════════════════════════════════════════════════
    // unloadAllChunks — Mark all loaded chunks for unloading.
    // Java: unloadAllChunks
    // ═══════════════════════════════════════════════════════════════════════

    void unloadAllChunks() {
        std::shared_lock lock(chunkMapMutex_);
        for (auto& [key, chunk] : id2ChunkMap_) {
            dropChunk(chunk.xPosition, chunk.zPosition);
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // unloadQueuedChunks — Process up to 100 pending unloads per tick.
    // Java: unloadQueuedChunks
    // ═══════════════════════════════════════════════════════════════════════

    void unloadQueuedChunks() {
        if (disableLevelSaving.load()) return;

        std::vector<int64_t> toUnload;
        {
            std::lock_guard lock(dropSetMutex_);
            int32_t count = 0;
            for (auto it = droppedChunksSet_.begin();
                 it != droppedChunksSet_.end() && count < 100; ++count) {
                toUnload.push_back(*it);
                it = droppedChunksSet_.erase(it);
            }
        }

        for (int64_t key : toUnload) {
            std::unique_lock lock(chunkMapMutex_);
            auto it = id2ChunkMap_.find(key);
            if (it != id2ChunkMap_.end()) {
                // Save before unloading
                if (saveChunkToFile) {
                    it->second.lastSaveTime = totalWorldTime.load();
                    saveChunkToFile(it->second);
                }

                // Remove from loaded list
                auto& coords = loadedChunkCoords_;
                coords.erase(
                    std::remove_if(coords.begin(), coords.end(),
                        [&](auto& c) { return c.first == it->second.xPosition &&
                                               c.second == it->second.zPosition; }),
                    coords.end());

                id2ChunkMap_.erase(it);
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // saveChunks — Save all (or up to 24) modified chunks.
    // Java: saveChunks(saveAll, progressUpdate)
    // ═══════════════════════════════════════════════════════════════════════

    bool saveChunks(bool saveAll) {
        if (!saveChunkToFile) return true;

        int32_t saved = 0;
        std::shared_lock lock(chunkMapMutex_);
        for (auto& [key, chunk] : id2ChunkMap_) {
            if (!chunk.needsSaving(saveAll)) continue;

            chunk.lastSaveTime = totalWorldTime.load();
            saveChunkToFile(chunk);
            chunk.isModified = false;

            if (++saved >= 24 && !saveAll) return false;
        }
        return true;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Accessors
    // ═══════════════════════════════════════════════════════════════════════

    int32_t getLoadedChunkCount() const {
        std::shared_lock lock(chunkMapMutex_);
        return static_cast<int32_t>(id2ChunkMap_.size());
    }

    std::string makeString() const {
        std::shared_lock lock(chunkMapMutex_);
        std::lock_guard dropLock(dropSetMutex_);
        return "ServerChunkCache: " + std::to_string(id2ChunkMap_.size()) +
               " Drop: " + std::to_string(droppedChunksSet_.size());
    }

    // Get chunk from cache only (no load/generate)
    ChunkData* getChunkIfLoaded(int32_t x, int32_t z) {
        std::shared_lock lock(chunkMapMutex_);
        auto it = id2ChunkMap_.find(chunkKey(x, z));
        return it != id2ChunkMap_.end() ? &it->second : nullptr;
    }

    // Iterate over all loaded chunks (read-only)
    template<typename Func>
    void forEachLoadedChunk(Func&& func) {
        std::shared_lock lock(chunkMapMutex_);
        for (auto& [key, chunk] : id2ChunkMap_) {
            func(chunk);
        }
    }

private:
    mutable std::shared_mutex chunkMapMutex_;
    std::unordered_map<int64_t, ChunkData> id2ChunkMap_;
    std::vector<std::pair<int32_t, int32_t>> loadedChunkCoords_;

    mutable std::mutex dropSetMutex_;
    std::unordered_set<int64_t> droppedChunksSet_;
};

} // namespace mccpp
