#pragma once
// World — manages chunk storage and generation.
// Simplified port of mt.java (WorldServer) for chunk management.
//
// For now: in-memory chunk cache with flat world generation.
// Anvil file I/O will be added in a later iteration.

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <functional>
#include "world/Chunk.h"

namespace mc {

// Hash function for chunk coordinate pairs
struct ChunkPosHash {
    size_t operator()(const std::pair<int32_t, int32_t>& p) const {
        // Cantor pairing function
        auto h1 = std::hash<int32_t>{}(p.first);
        auto h2 = std::hash<int32_t>{}(p.second);
        return h1 ^ (h2 << 16) ^ (h2 >> 16);
    }
};

class World {
public:
    std::string name = "world";
    int64_t worldTime = 6000;   // Ticks (6000 = noon)
    int64_t dayTime = 6000;     // Time of day
    int64_t seed = 0;
    bool raining = false;
    bool thundering = false;

    // Get or generate a chunk at the given coordinates
    ChunkColumn& getChunk(int cx, int cz) {
        auto key = std::make_pair(cx, cz);
        auto it = chunks_.find(key);
        if (it != chunks_.end()) {
            return *it->second;
        }
        // Generate flat chunk
        auto chunk = generateFlatChunk(cx, cz);
        auto& ref = *chunk;
        chunks_[key] = std::move(chunk);
        return ref;
    }

    bool hasChunk(int cx, int cz) const {
        return chunks_.count(std::make_pair(cx, cz)) > 0;
    }

    // Get block at world coordinates
    uint16_t getBlock(int x, int y, int z) const {
        int cx = x >> 4;
        int cz = z >> 4;
        auto it = chunks_.find(std::make_pair(cx, cz));
        if (it == chunks_.end()) return BlockID::AIR;
        return it->second->getBlock(x & 0xF, y, z & 0xF);
    }

    // Set block at world coordinates
    void setBlock(int x, int y, int z, uint16_t blockId, uint8_t meta = 0) {
        int cx = x >> 4;
        int cz = z >> 4;
        auto& chunk = getChunk(cx, cz);
        chunk.setBlock(x & 0xF, y, z & 0xF, blockId, meta);
    }

    // Tick the world (time advancement)
    void tick() {
        ++worldTime;
        ++dayTime;
        if (dayTime >= 24000) dayTime -= 24000;
    }

    size_t loadedChunkCount() const { return chunks_.size(); }

private:
    std::unordered_map<std::pair<int32_t, int32_t>,
                       std::unique_ptr<ChunkColumn>,
                       ChunkPosHash> chunks_;
};

} // namespace mc
