/**
 * World.h — WorldServer and ChunkProviderServer.
 *
 * Java references:
 *   - net.minecraft.world.WorldServer
 *   - net.minecraft.world.gen.ChunkProviderServer
 *   - net.minecraft.world.gen.ChunkProviderFlat
 *
 * The WorldServer owns the chunk provider, manages the tick loop for
 * world-level events (weather, time, scheduled ticks), and provides
 * block access across all loaded chunks.
 *
 * ChunkProviderServer is the chunk cache: it loads from disk (RegionFile),
 * generates new chunks (via IChunkGenerator), and unloads stale ones.
 *
 * Thread safety:
 *   - WorldServer tick runs on the main server thread.
 *   - Chunk loading can be triggered from any thread via loadChunk().
 *   - The chunk map is protected by a shared_mutex for concurrent reads.
 *   - Block access is safe through the chunk's owning section.
 *
 * JNI readiness: flat property layout, int position types for fast lookup.
 */
#pragma once

#include "world/Chunk.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// ChunkCoordIntPair — hash key for chunk coordinates.
// Java reference: net.minecraft.world.ChunkCoordIntPair
// ═══════════════════════════════════════════════════════════════════════════

struct ChunkCoordIntPair {
    int32_t chunkX;
    int32_t chunkZ;

    /**
     * Java: ChunkCoordIntPair.chunkXZ2Int(x, z) = (long)x & 0xFFFFFFFFL | ((long)z & 0xFFFFFFFFL) << 32
     */
    static int64_t chunkXZ2Int(int32_t x, int32_t z) {
        return (static_cast<int64_t>(x) & 0xFFFFFFFFL) |
               ((static_cast<int64_t>(z) & 0xFFFFFFFFL) << 32);
    }

    bool operator==(const ChunkCoordIntPair& o) const {
        return chunkX == o.chunkX && chunkZ == o.chunkZ;
    }
};

struct ChunkCoordHash {
    size_t operator()(int64_t key) const {
        return std::hash<int64_t>{}(key);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// IChunkGenerator — interface for world generators.
// Java reference: net.minecraft.world.chunk.IChunkProvider (generator subset)
// ═══════════════════════════════════════════════════════════════════════════

class IChunkGenerator {
public:
    virtual ~IChunkGenerator() = default;

    /**
     * Generate a new chunk at the given chunk coordinates.
     * Java reference: IChunkProvider.provideChunk(int, int)
     */
    virtual std::unique_ptr<Chunk> provideChunk(int chunkX, int chunkZ) = 0;

    /**
     * Get generator description string.
     * Java reference: IChunkProvider.makeString()
     */
    virtual std::string makeString() const = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// ChunkProviderFlat — Superflat world generator.
// Java reference: net.minecraft.world.gen.ChunkProviderFlat
//
// Default superflat preset: bedrock(1) + dirt(2) + grass(1) at y=0..3
// ═══════════════════════════════════════════════════════════════════════════

class ChunkProviderFlat : public IChunkGenerator {
public:
    ChunkProviderFlat();

    std::unique_ptr<Chunk> provideChunk(int chunkX, int chunkZ) override;
    std::string makeString() const override { return "FlatLevelSource"; }

private:
    // Block layers: index = y level, value = block ID
    // Default: bedrock at 0, dirt at 1-2, grass at 3
    std::array<int32_t, 256> blockLayers_{};
    std::array<uint8_t, 256> blockMeta_{};
};

// ═══════════════════════════════════════════════════════════════════════════
// ChunkProviderServer — Chunk cache with load/generate/unload.
// Java reference: net.minecraft.world.gen.ChunkProviderServer
//
// Thread safety: chunk map protected by shared_mutex.
// ═══════════════════════════════════════════════════════════════════════════

class WorldServer; // forward declaration

class ChunkProviderServer {
public:
    ChunkProviderServer(WorldServer* world, std::unique_ptr<IChunkGenerator> generator);

    /**
     * Get or load/generate a chunk.
     * Java reference: ChunkProviderServer.loadChunk(int, int)
     * Thread-safe: acquires write lock if chunk needs to be added.
     */
    Chunk* loadChunk(int chunkX, int chunkZ);

    /**
     * Get chunk if loaded, nullptr otherwise.
     * Java reference: ChunkProviderServer.provideChunk(int, int) when already loaded
     * Thread-safe: acquires read lock.
     */
    Chunk* getChunkIfLoaded(int chunkX, int chunkZ) const;

    /**
     * Check if chunk exists in cache.
     * Java reference: ChunkProviderServer.chunkExists(int, int)
     */
    bool chunkExists(int chunkX, int chunkZ) const;

    /**
     * Mark a chunk for unloading.
     * Java reference: ChunkProviderServer.dropChunk(int, int)
     */
    void dropChunk(int chunkX, int chunkZ);

    /**
     * Process unload queue (called from tick loop).
     * Java reference: ChunkProviderServer.unloadQueuedChunks()
     */
    bool unloadQueuedChunks();

    /**
     * Get number of loaded chunks.
     */
    int getLoadedChunkCount() const;

    /**
     * Get list of all loaded chunks (for saving/iteration).
     */
    std::vector<Chunk*> getLoadedChunks() const;

private:
    WorldServer* world_;
    std::unique_ptr<IChunkGenerator> generator_;

    // Chunk storage: key = ChunkCoordIntPair hash, value = owned Chunk
    mutable std::shared_mutex chunkMapMutex_;
    std::unordered_map<int64_t, std::unique_ptr<Chunk>, ChunkCoordHash> chunkMap_;

    // Unload queue
    mutable std::mutex unloadMutex_;
    std::vector<int64_t> unloadQueue_;
};

// ═══════════════════════════════════════════════════════════════════════════
// WorldServer — Server-side world instance.
// Java reference: net.minecraft.world.WorldServer
//
// Each dimension (overworld, nether, end) has its own WorldServer.
// The WorldServer owns the chunk provider, manages world time,
// weather, and provides block access.
// ═══════════════════════════════════════════════════════════════════════════

class WorldServer {
public:
    /**
     * Java: WorldServer(MinecraftServer, ISaveHandler, String, int, WorldSettings, Profiler)
     * Simplified for now — dimension ID + world name.
     */
    WorldServer(int dimensionId, const std::string& worldName);
    ~WorldServer();

    /**
     * Initialize the world — set up spawn position, generate initial chunks.
     * Java reference: WorldServer.initialize(WorldSettings)
     */
    void initialize();

    /**
     * Tick the world — time, weather, scheduled ticks, entity ticks.
     * Java reference: WorldServer.tick()
     * Called once per server tick from the main loop.
     */
    void tick();

    // ─── Block access ──────────────────────────────────────────────────
    Block* getBlock(int x, int y, int z);
    void setBlock(int x, int y, int z, Block* block);
    int getBlockMetadata(int x, int y, int z);
    void setBlockMetadata(int x, int y, int z, int meta);

    // ─── Chunk access ──────────────────────────────────────────────────
    Chunk* getChunkFromChunkCoords(int chunkX, int chunkZ);
    Chunk* getChunkFromBlockCoords(int blockX, int blockZ);
    ChunkProviderServer* getChunkProvider() { return chunkProvider_.get(); }

    // ─── World properties ──────────────────────────────────────────────
    int getDimensionId() const { return dimensionId_; }
    const std::string& getWorldName() const { return worldName_; }
    int64_t getTotalWorldTime() const { return totalWorldTime_; }
    int64_t getWorldTime() const { return worldTime_; }
    void setWorldTime(int64_t time) { worldTime_ = time; }
    bool hasNoSky() const { return dimensionId_ == -1; } // Nether

    // Spawn position
    int getSpawnX() const { return spawnX_; }
    int getSpawnY() const { return spawnY_; }
    int getSpawnZ() const { return spawnZ_; }
    void setSpawnPoint(int x, int y, int z) {
        spawnX_ = x; spawnY_ = y; spawnZ_ = z;
    }

    // Seed
    int64_t getSeed() const { return seed_; }
    void setSeed(int64_t seed) { seed_ = seed; }

private:
    int dimensionId_;
    std::string worldName_;

    std::unique_ptr<ChunkProviderServer> chunkProvider_;

    // World time (in ticks)
    int64_t totalWorldTime_ = 0;
    int64_t worldTime_ = 0;

    // Spawn position
    int spawnX_ = 0;
    int spawnY_ = 64;
    int spawnZ_ = 0;

    // Seed
    int64_t seed_ = 0;
};

} // namespace mccpp
