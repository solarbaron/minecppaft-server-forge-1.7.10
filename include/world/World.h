#pragma once
// World — manages chunk storage, generation, and persistence.
// Simplified port of mt.java (WorldServer) with Anvil region file I/O.

#include <cstdint>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <functional>
#include <iostream>
#include "world/Chunk.h"
#include "world/RegionFile.h"
#include "world/ChunkSerializer.h"
#include "mechanics/FurnaceManager.h"

namespace mc {

// Hash function for chunk coordinate pairs
struct ChunkPosHash {
    size_t operator()(const std::pair<int32_t, int32_t>& p) const {
        auto h1 = std::hash<int32_t>{}(p.first);
        auto h2 = std::hash<int32_t>{}(p.second);
        return h1 ^ (h2 << 16) ^ (h2 >> 16);
    }
};

// Hash for 3D block positions (tile entities)
struct BlockPosHash {
    size_t operator()(const std::tuple<int,int,int>& p) const {
        auto h1 = std::hash<int>{}(std::get<0>(p));
        auto h2 = std::hash<int>{}(std::get<1>(p));
        auto h3 = std::hash<int>{}(std::get<2>(p));
        return h1 ^ (h2 << 11) ^ (h3 << 22);
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
    std::string worldDir = "world"; // Directory for region files

    // Get or generate a chunk at the given coordinates
    ChunkColumn& getChunk(int cx, int cz) {
        auto key = std::make_pair(cx, cz);
        auto it = chunks_.find(key);
        if (it != chunks_.end()) {
            return *it->second;
        }

        // Try loading from disk first
        auto loaded = loadChunkFromDisk(cx, cz);
        if (loaded) {
            auto& ref = *loaded;
            chunks_[key] = std::move(loaded);
            return ref;
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

        // Tick furnaces every tick
        for (auto& [pos, furnace] : furnaces_) {
            furnace.tick();
        }
    }

    size_t loadedChunkCount() const { return chunks_.size(); }

    // === Furnace tile entities ===
    FurnaceTileEntity* getFurnace(int x, int y, int z) {
        auto key = std::make_tuple(x, y, z);
        auto it = furnaces_.find(key);
        return it != furnaces_.end() ? &it->second : nullptr;
    }

    FurnaceTileEntity& getOrCreateFurnace(int x, int y, int z) {
        auto key = std::make_tuple(x, y, z);
        auto it = furnaces_.find(key);
        if (it != furnaces_.end()) return it->second;
        auto& f = furnaces_[key];
        f.x = x; f.y = y; f.z = z;
        return f;
    }

    void removeFurnace(int x, int y, int z) {
        furnaces_.erase(std::make_tuple(x, y, z));
    }

    const auto& furnaces() const { return furnaces_; }

    // Save a single chunk to its region file
    void saveChunk(int cx, int cz) {
        auto it = chunks_.find(std::make_pair(cx, cz));
        if (it == chunks_.end()) return;

        int rx = RegionFile::regionCoord(cx);
        int rz = RegionFile::regionCoord(cz);
        int lx = RegionFile::localCoord(cx);
        int lz = RegionFile::localCoord(cz);

        auto& region = getRegion(rx, rz);
        auto nbt = ChunkSerializer::serialize(*it->second);
        region.writeChunk(lx, lz, nbt);
    }

    // Save all loaded chunks to disk
    void saveAll() {
        int saved = 0;
        for (auto& [key, chunk] : chunks_) {
            int rx = RegionFile::regionCoord(key.first);
            int rz = RegionFile::regionCoord(key.second);
            int lx = RegionFile::localCoord(key.first);
            int lz = RegionFile::localCoord(key.second);

            auto& region = getRegion(rx, rz);
            auto nbt = ChunkSerializer::serialize(*chunk);
            region.writeChunk(lx, lz, nbt);
            ++saved;
        }
        std::cout << "[WORLD] Saved " << saved << " chunks to disk\n";
    }

private:
    std::unordered_map<std::pair<int32_t, int32_t>,
                       std::unique_ptr<ChunkColumn>,
                       ChunkPosHash> chunks_;

    // Furnace tile entities by position
    std::unordered_map<std::tuple<int,int,int>, FurnaceTileEntity,
                       BlockPosHash> furnaces_;

    // Region file cache
    std::unordered_map<std::pair<int32_t, int32_t>,
                       std::unique_ptr<RegionFile>,
                       ChunkPosHash> regions_;

    RegionFile& getRegion(int rx, int rz) {
        auto key = std::make_pair(rx, rz);
        auto it = regions_.find(key);
        if (it != regions_.end()) return *it->second;

        auto path = RegionFile::regionPath(worldDir, rx, rz);
        auto region = std::make_unique<RegionFile>(path);
        auto& ref = *region;
        regions_[key] = std::move(region);
        return ref;
    }

    std::unique_ptr<ChunkColumn> loadChunkFromDisk(int cx, int cz) {
        int rx = RegionFile::regionCoord(cx);
        int rz = RegionFile::regionCoord(cz);
        int lx = RegionFile::localCoord(cx);
        int lz = RegionFile::localCoord(cz);

        auto& region = getRegion(rx, rz);
        if (!region.hasChunk(lx, lz)) return nullptr;

        auto nbt = region.readChunk(lx, lz);
        if (!nbt) return nullptr;

        return ChunkSerializer::deserialize(nbt);
    }
};

} // namespace mc

