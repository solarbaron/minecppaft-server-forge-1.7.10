/**
 * World.cpp — WorldServer, ChunkProviderServer, ChunkProviderFlat implementation.
 *
 * Java references:
 *   net.minecraft.world.WorldServer
 *   net.minecraft.world.gen.ChunkProviderServer
 *   net.minecraft.world.gen.ChunkProviderFlat
 *
 * Implements:
 *   - ChunkProviderFlat: default superflat (bedrock + 2 dirt + grass)
 *   - ChunkProviderServer: chunk cache with load/generate, unload queue
 *   - WorldServer: tick loop, block access, spawn initialization
 *
 * Multi-threading adaptations:
 *   - ChunkProviderServer uses shared_mutex for concurrent read access
 *   - Unload queue protected by separate mutex
 */

#include "world/World.h"

#include <algorithm>
#include <iostream>

namespace mccpp {

// ═════════════════════════════════════════════════════════════════════════════
// ChunkProviderFlat — Superflat world generator
// Java reference: net.minecraft.world.gen.ChunkProviderFlat
// ═════════════════════════════════════════════════════════════════════════════

ChunkProviderFlat::ChunkProviderFlat() {
    blockLayers_.fill(0);
    blockMeta_.fill(0);

    // Default superflat preset: bedrock(y=0), dirt(y=1,2), grass(y=3)
    // Java: FlatGeneratorInfo default layers
    // Block IDs: bedrock=7, dirt=3, grass=2
    blockLayers_[0] = 7;   // bedrock
    blockLayers_[1] = 3;   // dirt
    blockLayers_[2] = 3;   // dirt
    blockLayers_[3] = 2;   // grass_block
}

std::unique_ptr<Chunk> ChunkProviderFlat::provideChunk(int chunkX, int chunkZ) {
    // Java reference: ChunkProviderFlat.provideChunk(int, int)
    auto chunk = std::make_unique<Chunk>(chunkX, chunkZ);

    for (int y = 0; y < 256; ++y) {
        int blockId = blockLayers_[y];
        if (blockId == 0) continue;

        int sectionIdx = y >> 4;
        if (!chunk->sections[sectionIdx]) {
            chunk->sections[sectionIdx] = std::make_unique<ChunkSection>(sectionIdx << 4, true);
        }

        Block* block = Block::getBlockById(blockId);
        if (!block) continue;

        for (int x = 0; x < 16; ++x) {
            for (int z = 0; z < 16; ++z) {
                chunk->sections[sectionIdx]->setBlock(x, y & 0xF, z, block);
                if (blockMeta_[y] != 0) {
                    chunk->sections[sectionIdx]->setBlockMetadata(x, y & 0xF, z, blockMeta_[y]);
                }
            }
        }
    }

    // Set biome to plains (1) for all columns
    chunk->biomes.fill(1);

    // Calculate height map
    for (int x = 0; x < 16; ++x) {
        for (int z = 0; z < 16; ++z) {
            // Find highest non-air block
            for (int y = 255; y >= 0; --y) {
                if (blockLayers_[y] != 0) {
                    chunk->heightMap[z * 16 + x] = y + 1;
                    break;
                }
            }
        }
    }

    chunk->isTerrainPopulated = true;
    chunk->isLightPopulated = true;

    return chunk;
}

// ═════════════════════════════════════════════════════════════════════════════
// ChunkProviderServer — Chunk cache
// Java reference: net.minecraft.world.gen.ChunkProviderServer
// ═════════════════════════════════════════════════════════════════════════════

ChunkProviderServer::ChunkProviderServer(WorldServer* world, std::unique_ptr<IChunkGenerator> generator)
    : world_(world)
    , generator_(std::move(generator))
{}

Chunk* ChunkProviderServer::loadChunk(int chunkX, int chunkZ) {
    // Java reference: ChunkProviderServer.loadChunk(int, int)
    int64_t key = ChunkCoordIntPair::chunkXZ2Int(chunkX, chunkZ);

    // Remove from unload queue if present
    {
        std::lock_guard<std::mutex> lock(unloadMutex_);
        auto it = std::find(unloadQueue_.begin(), unloadQueue_.end(), key);
        if (it != unloadQueue_.end()) {
            unloadQueue_.erase(it);
        }
    }

    // Check if already loaded (read lock)
    {
        std::shared_lock<std::shared_mutex> rlock(chunkMapMutex_);
        auto it = chunkMap_.find(key);
        if (it != chunkMap_.end()) {
            return it->second.get();
        }
    }

    // Not loaded — generate new chunk
    std::unique_ptr<Chunk> chunk;
    if (generator_) {
        chunk = generator_->provideChunk(chunkX, chunkZ);
    } else {
        // Empty chunk fallback
        chunk = std::make_unique<Chunk>(chunkX, chunkZ);
    }

    // Insert (write lock)
    {
        std::unique_lock<std::shared_mutex> wlock(chunkMapMutex_);
        auto [it, inserted] = chunkMap_.emplace(key, std::move(chunk));
        return it->second.get();
    }
}

Chunk* ChunkProviderServer::getChunkIfLoaded(int chunkX, int chunkZ) const {
    int64_t key = ChunkCoordIntPair::chunkXZ2Int(chunkX, chunkZ);
    std::shared_lock<std::shared_mutex> rlock(chunkMapMutex_);
    auto it = chunkMap_.find(key);
    return (it != chunkMap_.end()) ? it->second.get() : nullptr;
}

bool ChunkProviderServer::chunkExists(int chunkX, int chunkZ) const {
    int64_t key = ChunkCoordIntPair::chunkXZ2Int(chunkX, chunkZ);
    std::shared_lock<std::shared_mutex> rlock(chunkMapMutex_);
    return chunkMap_.find(key) != chunkMap_.end();
}

void ChunkProviderServer::dropChunk(int chunkX, int chunkZ) {
    // Java reference: ChunkProviderServer.dropChunk(int, int)
    // Don't unload spawn chunks (within 192 blocks of spawn)
    // Simplified: don't unload within 12 chunks of spawn
    int spawnCX = world_->getSpawnX() >> 4;
    int spawnCZ = world_->getSpawnZ() >> 4;
    if (std::abs(chunkX - spawnCX) <= 12 && std::abs(chunkZ - spawnCZ) <= 12) {
        return;
    }

    int64_t key = ChunkCoordIntPair::chunkXZ2Int(chunkX, chunkZ);
    std::lock_guard<std::mutex> lock(unloadMutex_);
    unloadQueue_.push_back(key);
}

bool ChunkProviderServer::unloadQueuedChunks() {
    // Java reference: ChunkProviderServer.unloadQueuedChunks()
    // Process up to 100 chunks per tick
    std::vector<int64_t> toUnload;
    {
        std::lock_guard<std::mutex> lock(unloadMutex_);
        int count = std::min(static_cast<int>(unloadQueue_.size()), 100);
        if (count > 0) {
            toUnload.assign(unloadQueue_.begin(), unloadQueue_.begin() + count);
            unloadQueue_.erase(unloadQueue_.begin(), unloadQueue_.begin() + count);
        }
    }

    if (toUnload.empty()) return false;

    std::unique_lock<std::shared_mutex> wlock(chunkMapMutex_);
    for (int64_t key : toUnload) {
        chunkMap_.erase(key);
    }

    return true;
}

int ChunkProviderServer::getLoadedChunkCount() const {
    std::shared_lock<std::shared_mutex> rlock(chunkMapMutex_);
    return static_cast<int>(chunkMap_.size());
}

std::vector<Chunk*> ChunkProviderServer::getLoadedChunks() const {
    std::shared_lock<std::shared_mutex> rlock(chunkMapMutex_);
    std::vector<Chunk*> result;
    result.reserve(chunkMap_.size());
    for (const auto& [key, chunk] : chunkMap_) {
        result.push_back(chunk.get());
    }
    return result;
}

// ═════════════════════════════════════════════════════════════════════════════
// WorldServer
// Java reference: net.minecraft.world.WorldServer
// ═════════════════════════════════════════════════════════════════════════════

WorldServer::WorldServer(int dimensionId, const std::string& worldName)
    : dimensionId_(dimensionId)
    , worldName_(worldName)
{
    // Create chunk provider with flat generator
    auto generator = std::make_unique<ChunkProviderFlat>();
    chunkProvider_ = std::make_unique<ChunkProviderServer>(this, std::move(generator));
}

WorldServer::~WorldServer() = default;

void WorldServer::initialize() {
    // Java reference: WorldServer.initialize(WorldSettings)

    // Set default spawn point
    spawnX_ = 0;
    spawnY_ = 4;  // Above the superflat surface (bedrock=0, dirt=1-2, grass=3)
    spawnZ_ = 0;

    // Pre-generate spawn area chunks
    // Java: server generates chunks in a 25x25 area (±12 chunks) around spawn
    std::cout << "[World] Preparing spawn area for dimension " << dimensionId_ << "...\n";

    int spawnCX = spawnX_ >> 4;
    int spawnCZ = spawnZ_ >> 4;
    int radius = 12; // 12 chunk radius = 25x25 area

    int totalChunks = (radius * 2 + 1) * (radius * 2 + 1);
    int loaded = 0;

    for (int cx = spawnCX - radius; cx <= spawnCX + radius; ++cx) {
        for (int cz = spawnCZ - radius; cz <= spawnCZ + radius; ++cz) {
            chunkProvider_->loadChunk(cx, cz);
            ++loaded;
        }
    }

    std::cout << "[World] Prepared " << loaded << " chunks for dimension " << dimensionId_ << "\n";
}

void WorldServer::tick() {
    // Java reference: WorldServer.tick()
    // Increment world time
    ++totalWorldTime_;
    worldTime_ = totalWorldTime_ % 24000; // Day cycle

    // Process chunk unloads
    chunkProvider_->unloadQueuedChunks();

    // TODO: Weather updates
    // TODO: Scheduled block ticks
    // TODO: Entity ticking
    // TODO: Random block ticks
}

// ─── Block access ────────────────────────────────────────────────────────

Block* WorldServer::getBlock(int x, int y, int z) {
    if (y < 0 || y >= 256) return Block::getBlockById(0);
    Chunk* chunk = getChunkFromBlockCoords(x, z);
    if (!chunk) return Block::getBlockById(0);
    return chunk->getBlock(x & 15, y, z & 15);
}

void WorldServer::setBlock(int x, int y, int z, Block* block) {
    if (y < 0 || y >= 256) return;
    Chunk* chunk = getChunkFromBlockCoords(x, z);
    if (!chunk) return;
    chunk->setBlock(x & 15, y, z & 15, block);
}

int WorldServer::getBlockMetadata(int x, int y, int z) {
    if (y < 0 || y >= 256) return 0;
    Chunk* chunk = getChunkFromBlockCoords(x, z);
    if (!chunk) return 0;
    return chunk->getBlockMetadata(x & 15, y, z & 15);
}

void WorldServer::setBlockMetadata(int x, int y, int z, int meta) {
    if (y < 0 || y >= 256) return;
    Chunk* chunk = getChunkFromBlockCoords(x, z);
    if (!chunk) return;
    chunk->setBlockMetadata(x & 15, y, z & 15, meta);
}

Chunk* WorldServer::getChunkFromChunkCoords(int chunkX, int chunkZ) {
    return chunkProvider_->loadChunk(chunkX, chunkZ);
}

Chunk* WorldServer::getChunkFromBlockCoords(int blockX, int blockZ) {
    return getChunkFromChunkCoords(blockX >> 4, blockZ >> 4);
}

} // namespace mccpp
