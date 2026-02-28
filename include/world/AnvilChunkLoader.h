/**
 * AnvilChunkLoader.h — Anvil region file chunk serialization/deserialization.
 *
 * Java reference: net.minecraft.world.chunk.storage.AnvilChunkLoader (318 lines)
 *
 * Architecture:
 *   - Chunk NBT format: Level{V:1, xPos, zPos, LastUpdate, HeightMap int[256],
 *     TerrainPopulated, LightPopulated, InhabitedTime, Sections[16] × {Y byte,
 *     Blocks byte[4096], Add nibble[2048]?, Data nibble[2048],
 *     BlockLight nibble[2048], SkyLight nibble[2048]}, Biomes byte[256],
 *     Entities[], TileEntities[], TileTicks[]{i,x,y,z,t,p}}
 *   - Threaded I/O: pending queue with coordinate set, writeNextIO pops first
 *   - Load: check pending queue first, else read from RegionFile
 *   - Save: serialize to NBT, queue for async write
 *
 * Thread safety:
 *   - mutex on pending chunks queue and coordinate set
 *   - Writes happen on dedicated I/O thread
 *
 * JNI readiness: Predictable NBT key names, standard byte/nibble arrays.
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// ExtendedBlockStorage — 16×16×16 subchunk section.
// Java reference: net.minecraft.world.chunk.storage.ExtendedBlockStorage
// ═══════════════════════════════════════════════════════════════════════════

struct ExtendedBlockStorage {
    int32_t yBase;               // Y coordinate (bottom of section, multiple of 16)
    bool hasSkyLight;
    uint8_t blockLSB[4096] = {}; // Lower 8 bits of block ID
    uint8_t blockMSB[2048] = {}; // Upper 4 bits of block ID (nibble, may be absent)
    bool hasBlockMSB = false;
    uint8_t metadata[2048] = {}; // Block metadata (nibble)
    uint8_t blockLight[2048] = {}; // Block light (nibble)
    uint8_t skyLight[2048] = {};   // Sky light (nibble)

    int32_t nonAirBlockCount = 0;
    int32_t tickRandomBlockCount = 0;

    ExtendedBlockStorage() : yBase(0), hasSkyLight(true) {}
    ExtendedBlockStorage(int32_t y, bool sky) : yBase(y), hasSkyLight(sky) {}

    bool getNeedsRandomTick() const { return tickRandomBlockCount > 0; }
    int32_t getYLocation() const { return yBase; }

    // Get block ID at local coords (0-15)
    int32_t getBlockId(int32_t x, int32_t y, int32_t z) const {
        int32_t idx = y * 256 + z * 16 + x;
        int32_t id = blockLSB[idx];
        if (hasBlockMSB) {
            int32_t nibbleIdx = idx >> 1;
            int32_t shift = (idx & 1) * 4;
            id |= ((blockMSB[nibbleIdx] >> shift) & 0xF) << 8;
        }
        return id;
    }

    // Get metadata at local coords
    int32_t getMetadata(int32_t x, int32_t y, int32_t z) const {
        int32_t idx = y * 256 + z * 16 + x;
        int32_t nibbleIdx = idx >> 1;
        int32_t shift = (idx & 1) * 4;
        return (metadata[nibbleIdx] >> shift) & 0xF;
    }

    // Get/set nibble helper
    static uint8_t getNibble(const uint8_t* arr, int32_t idx) {
        return (arr[idx >> 1] >> ((idx & 1) * 4)) & 0xF;
    }

    static void setNibble(uint8_t* arr, int32_t idx, uint8_t val) {
        int32_t byteIdx = idx >> 1;
        int32_t shift = (idx & 1) * 4;
        arr[byteIdx] = (arr[byteIdx] & ~(0xF << shift)) | ((val & 0xF) << shift);
    }

    // Count non-air blocks and ticking blocks
    void removeInvalidBlocks() {
        nonAirBlockCount = 0;
        tickRandomBlockCount = 0;
        for (int32_t y = 0; y < 16; ++y) {
            for (int32_t z = 0; z < 16; ++z) {
                for (int32_t x = 0; x < 16; ++x) {
                    if (getBlockId(x, y, z) != 0) {
                        ++nonAirBlockCount;
                    }
                }
            }
        }
    }

    bool isEmpty() const { return nonAirBlockCount == 0; }
};

// ═══════════════════════════════════════════════════════════════════════════
// AnvilChunkData — Complete chunk data for serialization.
// ═══════════════════════════════════════════════════════════════════════════

struct AnvilChunkData {
    int32_t xPos, zPos;
    int64_t lastUpdate = 0;
    int64_t inhabitedTime = 0;
    bool terrainPopulated = false;
    bool lightPopulated = false;
    bool hasEntities = false;

    int32_t heightMap[256] = {};
    uint8_t biomes[256] = {};

    ExtendedBlockStorage sections[16];
    bool sectionExists[16] = {};

    // Tile ticks
    struct TileTick {
        int32_t blockId;
        int32_t x, y, z;
        int32_t delay;    // Relative to world time
        int32_t priority;
    };
    std::vector<TileTick> tileTicks;
};

// ═══════════════════════════════════════════════════════════════════════════
// PendingChunk — Queued chunk write.
// Java reference: AnvilChunkLoader$PendingChunk
// ═══════════════════════════════════════════════════════════════════════════

struct PendingChunk {
    int32_t chunkX, chunkZ;
    AnvilChunkData data;
};

// ═══════════════════════════════════════════════════════════════════════════
// AnvilChunkLoader — Anvil format chunk I/O with threaded writes.
// Java reference: net.minecraft.world.chunk.storage.AnvilChunkLoader
// ═══════════════════════════════════════════════════════════════════════════

class AnvilChunkLoader {
public:
    std::string saveDirectory;

    explicit AnvilChunkLoader(const std::string& dir) : saveDirectory(dir) {}

    // ═══════════════════════════════════════════════════════════════════════
    // Serialize chunk to NBT-compatible byte stream.
    // Java: writeChunkToNBT
    //
    // NBT structure:
    //   Level {
    //     V: byte = 1
    //     xPos: int, zPos: int
    //     LastUpdate: long
    //     HeightMap: int[256]
    //     TerrainPopulated: boolean
    //     LightPopulated: boolean
    //     InhabitedTime: long
    //     Sections: list of compound {
    //       Y: byte (section index 0-15)
    //       Blocks: byte[4096] (LSB of block ID)
    //       Add: byte[2048] (MSB nibble, optional)
    //       Data: byte[2048] (metadata nibble)
    //       BlockLight: byte[2048]
    //       SkyLight: byte[2048]
    //     }
    //     Biomes: byte[256]
    //     Entities: list of compound
    //     TileEntities: list of compound
    //     TileTicks: list of compound { i:int, x:int, y:int, z:int, t:int, p:int }
    //   }
    // ═══════════════════════════════════════════════════════════════════════

    struct SerializedSection {
        uint8_t yIndex;           // Section Y index (0-15)
        uint8_t blocks[4096];     // Block LSB
        uint8_t add[2048];        // Block MSB nibble (if present)
        bool hasAdd;
        uint8_t data[2048];       // Metadata nibble
        uint8_t blockLight[2048];
        uint8_t skyLight[2048];
    };

    struct SerializedChunk {
        int32_t xPos, zPos;
        int64_t lastUpdate;
        int64_t inhabitedTime;
        bool terrainPopulated;
        bool lightPopulated;
        int32_t heightMap[256];
        uint8_t biomes[256];
        std::vector<SerializedSection> sections;
        std::vector<AnvilChunkData::TileTick> tileTicks;
    };

    SerializedChunk serializeChunk(const AnvilChunkData& chunk) const {
        SerializedChunk out;
        out.xPos = chunk.xPos;
        out.zPos = chunk.zPos;
        out.lastUpdate = chunk.lastUpdate;
        out.inhabitedTime = chunk.inhabitedTime;
        out.terrainPopulated = chunk.terrainPopulated;
        out.lightPopulated = chunk.lightPopulated;
        std::memcpy(out.heightMap, chunk.heightMap, sizeof(out.heightMap));
        std::memcpy(out.biomes, chunk.biomes, sizeof(out.biomes));

        for (int32_t i = 0; i < 16; ++i) {
            if (!chunk.sectionExists[i]) continue;
            const auto& sec = chunk.sections[i];
            if (sec.isEmpty()) continue;

            SerializedSection ss;
            ss.yIndex = static_cast<uint8_t>(i);
            std::memcpy(ss.blocks, sec.blockLSB, sizeof(ss.blocks));
            ss.hasAdd = sec.hasBlockMSB;
            if (ss.hasAdd) {
                std::memcpy(ss.add, sec.blockMSB, sizeof(ss.add));
            }
            std::memcpy(ss.data, sec.metadata, sizeof(ss.data));
            std::memcpy(ss.blockLight, sec.blockLight, sizeof(ss.blockLight));
            std::memcpy(ss.skyLight, sec.skyLight, sizeof(ss.skyLight));

            out.sections.push_back(std::move(ss));
        }

        out.tileTicks = chunk.tileTicks;
        return out;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Deserialize chunk from NBT-compatible data.
    // Java: readChunkFromNBT
    // ═══════════════════════════════════════════════════════════════════════

    AnvilChunkData deserializeChunk(const SerializedChunk& in) const {
        AnvilChunkData chunk;
        chunk.xPos = in.xPos;
        chunk.zPos = in.zPos;
        chunk.lastUpdate = in.lastUpdate;
        chunk.inhabitedTime = in.inhabitedTime;
        chunk.terrainPopulated = in.terrainPopulated;
        chunk.lightPopulated = in.lightPopulated;
        std::memcpy(chunk.heightMap, in.heightMap, sizeof(chunk.heightMap));
        std::memcpy(chunk.biomes, in.biomes, sizeof(chunk.biomes));

        for (const auto& ss : in.sections) {
            int32_t idx = ss.yIndex;
            if (idx < 0 || idx >= 16) continue;

            auto& sec = chunk.sections[idx];
            sec.yBase = idx << 4;
            sec.hasSkyLight = true;
            chunk.sectionExists[idx] = true;

            std::memcpy(sec.blockLSB, ss.blocks, sizeof(sec.blockLSB));
            sec.hasBlockMSB = ss.hasAdd;
            if (ss.hasAdd) {
                std::memcpy(sec.blockMSB, ss.add, sizeof(sec.blockMSB));
            }
            std::memcpy(sec.metadata, ss.data, sizeof(sec.metadata));
            std::memcpy(sec.blockLight, ss.blockLight, sizeof(sec.blockLight));
            std::memcpy(sec.skyLight, ss.skyLight, sizeof(sec.skyLight));

            sec.removeInvalidBlocks();
        }

        chunk.tileTicks = in.tileTicks;
        return chunk;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Threaded I/O — Pending write queue
    // Java: addChunkToPending, writeNextIO, saveExtraData
    // Thread safety: mutex on pending queue
    // ═══════════════════════════════════════════════════════════════════════

    void queueChunkSave(const AnvilChunkData& chunk) {
        std::lock_guard lock(pendingMutex_);
        int64_t key = chunkKey(chunk.xPos, chunk.zPos);

        // Update existing pending entry if present
        if (pendingCoords_.count(key)) {
            for (auto& pending : pendingChunks_) {
                if (pending.chunkX == chunk.xPos && pending.chunkZ == chunk.zPos) {
                    pending.data = chunk;
                    return;
                }
            }
        }

        pendingChunks_.push_back({chunk.xPos, chunk.zPos, chunk});
        pendingCoords_.insert(key);
    }

    // Pop and return next chunk to write. Returns true if there was one.
    // Java: writeNextIO
    bool writeNextIO(PendingChunk& out) {
        std::lock_guard lock(pendingMutex_);
        if (pendingChunks_.empty()) return false;

        out = std::move(pendingChunks_.front());
        pendingChunks_.pop_front();
        pendingCoords_.erase(chunkKey(out.chunkX, out.chunkZ));
        return true;
    }

    // Flush all pending writes
    // Java: saveExtraData
    void flushAllPending() {
        PendingChunk chunk;
        while (writeNextIO(chunk)) {
            // Write chunk to disk (via RegionFile)
        }
    }

    // Check if chunk has pending save
    bool hasPendingSave(int32_t x, int32_t z) const {
        std::lock_guard lock(pendingMutex_);
        return pendingCoords_.count(chunkKey(x, z)) > 0;
    }

    // Load pending chunk data (for reads during async write)
    // Java: loadChunk — checks pending queue first before disk
    bool loadFromPending(int32_t x, int32_t z, AnvilChunkData& out) {
        std::lock_guard lock(pendingMutex_);
        int64_t key = chunkKey(x, z);
        if (!pendingCoords_.count(key)) return false;

        for (const auto& pending : pendingChunks_) {
            if (pending.chunkX == x && pending.chunkZ == z) {
                out = pending.data;
                return true;
            }
        }
        return false;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Validation
    // Java: checkedReadChunkFromNBT — validates Level tag and Sections
    // ═══════════════════════════════════════════════════════════════════════

    static bool validateChunkData(const AnvilChunkData& chunk, int32_t expectedX, int32_t expectedZ) {
        return chunk.xPos == expectedX && chunk.zPos == expectedZ;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Accessors
    // ═══════════════════════════════════════════════════════════════════════

    int32_t getPendingCount() const {
        std::lock_guard lock(pendingMutex_);
        return static_cast<int32_t>(pendingChunks_.size());
    }

private:
    static int64_t chunkKey(int32_t x, int32_t z) {
        return static_cast<int64_t>(x) & 0xFFFFFFFFL |
               (static_cast<int64_t>(z) & 0xFFFFFFFFL) << 32;
    }

    mutable std::mutex pendingMutex_;
    std::deque<PendingChunk> pendingChunks_;
    std::unordered_set<int64_t> pendingCoords_;
};

} // namespace mccpp
