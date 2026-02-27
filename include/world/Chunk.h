#pragma once
// Chunk data structures for Minecraft 1.7.10.
// A chunk column is 16 sections (0-15), each 16x16x16 blocks.
//
// Protocol 47 chunk format:
//   For each present section (indicated by primaryBitmap):
//     - 4096 block IDs as LSB bytes (lower 8 bits of 12-bit ID)
//     - 2048 bytes metadata nibbles (4-bit, 2 per byte)
//     - 2048 bytes block light nibbles
//     - 2048 bytes sky light nibbles
//     - [optional] 2048 bytes add array (upper 4 bits of 12-bit ID)
//   After all sections:
//     - 256 bytes biome data (if groundUpContinuous)
//
// This matches the vanilla chunk serialization used by the Java server.

#include <cstdint>
#include <cstring>
#include <vector>
#include <array>
#include <memory>
#include "world/Block.h"

namespace mc {

// ============================================================
// ChunkSection — 16x16x16 block storage
// ============================================================
class ChunkSection {
public:
    ChunkSection() {
        std::memset(blockIds_, 0, sizeof(blockIds_));
        std::memset(metadata_, 0, sizeof(metadata_));
        std::memset(blockLight_, 0, sizeof(blockLight_));
        // Sky light defaults to 15 (full sunlight) — matches Java default
        std::memset(skyLight_, 0xFF, sizeof(skyLight_));
    }

    // Y-index within column (0-15)
    int yIndex = 0;

    // Block access — index = y*256 + z*16 + x (matches MCP ExtendedBlockStorage)
    uint16_t getBlockId(int x, int y, int z) const {
        int idx = y * 256 + z * 16 + x;
        uint16_t base = blockIds_[idx];
        // Add upper 4 bits from add array
        uint8_t add = getNibble(addData_, idx);
        return base | (static_cast<uint16_t>(add) << 8);
    }

    void setBlockId(int x, int y, int z, uint16_t id) {
        int idx = y * 256 + z * 16 + x;
        blockIds_[idx] = static_cast<uint8_t>(id & 0xFF);
        uint8_t upper = static_cast<uint8_t>((id >> 8) & 0xF);
        setNibble(addData_, idx, upper);
        if (upper != 0) hasAddData_ = true;
    }

    uint8_t getMetadata(int x, int y, int z) const {
        return getNibble(metadata_, y * 256 + z * 16 + x);
    }

    void setMetadata(int x, int y, int z, uint8_t meta) {
        setNibble(metadata_, y * 256 + z * 16 + x, meta & 0xF);
    }

    void setBlockLight(int x, int y, int z, uint8_t level) {
        setNibble(blockLight_, y * 256 + z * 16 + x, level & 0xF);
    }

    void setSkyLight(int x, int y, int z, uint8_t level) {
        setNibble(skyLight_, y * 256 + z * 16 + x, level & 0xF);
    }

    // Check if section has any non-air blocks
    bool isEmpty() const {
        for (int i = 0; i < 4096; ++i) {
            if (blockIds_[i] != 0 || getNibble(addData_, i) != 0) return false;
        }
        return true;
    }

    bool hasAdd() const { return hasAddData_; }

    // Raw data access for serialization
    const uint8_t* blockIdArray() const { return blockIds_; }
    const uint8_t* metadataArray() const { return metadata_; }
    const uint8_t* blockLightArray() const { return blockLight_; }
    const uint8_t* skyLightArray() const { return skyLight_; }
    const uint8_t* addArray() const { return addData_; }

private:
    static uint8_t getNibble(const uint8_t* arr, int idx) {
        uint8_t byte = arr[idx >> 1];
        return (idx & 1) ? ((byte >> 4) & 0xF) : (byte & 0xF);
    }

    static void setNibble(uint8_t* arr, int idx, uint8_t val) {
        int byteIdx = idx >> 1;
        if (idx & 1) {
            arr[byteIdx] = (arr[byteIdx] & 0x0F) | ((val & 0xF) << 4);
        } else {
            arr[byteIdx] = (arr[byteIdx] & 0xF0) | (val & 0xF);
        }
    }

    uint8_t blockIds_[4096];    // Lower 8 bits of block ID
    uint8_t metadata_[2048];    // Block metadata nibbles
    uint8_t blockLight_[2048];  // Block light nibbles
    uint8_t skyLight_[2048];    // Sky light nibbles
    uint8_t addData_[2048] = {};// Upper 4 bits of block ID (add array)
    bool hasAddData_ = false;
};

// ============================================================
// ChunkColumn — 16 sections + biome data
// ============================================================
class ChunkColumn {
public:
    int chunkX = 0, chunkZ = 0;
    std::array<std::unique_ptr<ChunkSection>, 16> sections;
    std::array<uint8_t, 256> biomes; // 16x16 biome IDs (XZ order)

    ChunkColumn() {
        biomes.fill(1); // Plains biome by default
    }

    ChunkColumn(int cx, int cz) : chunkX(cx), chunkZ(cz) {
        biomes.fill(1);
    }

    // Get/set block at world-relative position within chunk
    uint16_t getBlock(int x, int y, int z) const {
        int sectionY = y >> 4;
        if (sectionY < 0 || sectionY >= 16) return BlockID::AIR;
        if (!sections[sectionY]) return BlockID::AIR;
        return sections[sectionY]->getBlockId(x, y & 0xF, z);
    }

    void setBlock(int x, int y, int z, uint16_t blockId, uint8_t meta = 0) {
        int sectionY = y >> 4;
        if (sectionY < 0 || sectionY >= 16) return;
        if (!sections[sectionY]) {
            sections[sectionY] = std::make_unique<ChunkSection>();
            sections[sectionY]->yIndex = sectionY;
        }
        sections[sectionY]->setBlockId(x, y & 0xF, z, blockId);
        if (meta != 0) {
            sections[sectionY]->setMetadata(x, y & 0xF, z, meta);
        }
    }

    // Serialize chunk data for protocol 47 Chunk Data packet.
    // Returns: primaryBitmap, addBitmap, and the uncompressed data buffer.
    struct SerializedChunk {
        uint16_t primaryBitmap;
        uint16_t addBitmap;
        std::vector<uint8_t> data;
    };

    SerializedChunk serialize(bool includeBiomes) const {
        SerializedChunk result;
        result.primaryBitmap = 0;
        result.addBitmap = 0;

        // Compute bitmaps
        for (int i = 0; i < 16; ++i) {
            if (sections[i] && !sections[i]->isEmpty()) {
                result.primaryBitmap |= (1 << i);
                if (sections[i]->hasAdd()) {
                    result.addBitmap |= (1 << i);
                }
            }
        }

        // Calculate total size
        int sectionCount = __builtin_popcount(result.primaryBitmap);
        int addCount = __builtin_popcount(result.addBitmap);
        size_t dataSize = sectionCount * (4096 + 2048 + 2048 + 2048) // blocks + meta + blockLight + skyLight
                        + addCount * 2048                              // add array
                        + (includeBiomes ? 256 : 0);                   // biomes

        result.data.reserve(dataSize);

        // Write block IDs (4096 bytes per section)
        for (int i = 0; i < 16; ++i) {
            if (result.primaryBitmap & (1 << i)) {
                auto* arr = sections[i]->blockIdArray();
                result.data.insert(result.data.end(), arr, arr + 4096);
            }
        }

        // Write metadata nibbles (2048 bytes per section)
        for (int i = 0; i < 16; ++i) {
            if (result.primaryBitmap & (1 << i)) {
                auto* arr = sections[i]->metadataArray();
                result.data.insert(result.data.end(), arr, arr + 2048);
            }
        }

        // Write block light (2048 bytes per section)
        for (int i = 0; i < 16; ++i) {
            if (result.primaryBitmap & (1 << i)) {
                auto* arr = sections[i]->blockLightArray();
                result.data.insert(result.data.end(), arr, arr + 2048);
            }
        }

        // Write sky light (2048 bytes per section)
        for (int i = 0; i < 16; ++i) {
            if (result.primaryBitmap & (1 << i)) {
                auto* arr = sections[i]->skyLightArray();
                result.data.insert(result.data.end(), arr, arr + 2048);
            }
        }

        // Write add arrays (2048 bytes per section, only if needed)
        for (int i = 0; i < 16; ++i) {
            if (result.addBitmap & (1 << i)) {
                auto* arr = sections[i]->addArray();
                result.data.insert(result.data.end(), arr, arr + 2048);
            }
        }

        // Write biome data
        if (includeBiomes) {
            result.data.insert(result.data.end(), biomes.begin(), biomes.end());
        }

        return result;
    }
};

// ============================================================
// Flat world chunk generator
// ============================================================
// Generates a classic superflat chunk: 1 layer bedrock, 2 layers dirt, 1 layer grass
inline std::unique_ptr<ChunkColumn> generateFlatChunk(int cx, int cz) {
    auto chunk = std::make_unique<ChunkColumn>(cx, cz);

    for (int x = 0; x < 16; ++x) {
        for (int z = 0; z < 16; ++z) {
            chunk->setBlock(x, 0, z, BlockID::BEDROCK);
            chunk->setBlock(x, 1, z, BlockID::DIRT);
            chunk->setBlock(x, 2, z, BlockID::DIRT);
            chunk->setBlock(x, 3, z, BlockID::GRASS);
        }
    }

    return chunk;
}

} // namespace mc
