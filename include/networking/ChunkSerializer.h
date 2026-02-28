/**
 * ChunkSerializer.h — Chunk data serialization for S21 (Chunk Data) packet.
 *
 * Java references:
 *   - net.minecraft.network.play.server.S21PacketChunkData
 *   - net.minecraft.network.play.server.S21PacketChunkData$Extracted
 *   - net.minecraft.world.chunk.storage.ExtendedBlockStorage
 *   - net.minecraft.network.play.server.S26PacketMapChunkBulk
 *
 * Protocol v5 chunk data layout per section (16×16×16):
 *   1. Block ID LSB array:  4096 bytes (one byte per block)
 *   2. Metadata nibble:     2048 bytes (half-byte per block)
 *   3. Block light nibble:  2048 bytes
 *   4. Sky light nibble:    2048 bytes (overworld/end only)
 *   [Optional] Block ID MSB: 2048 bytes (for IDs > 255)
 *   Total per section: 10240 (no sky) or 12288 (with sky) + optional 2048 MSB
 *
 * Full chunk also includes 256 bytes of biome data.
 *
 * The uncompressed data is zlib deflated before sending.
 *
 * Thread safety: Stateless — each call produces an independent buffer.
 */
#pragma once

#include "PacketBuilder.h"

#include <cstdint>
#include <cstring>
#include <vector>
#include <zlib.h>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// ChunkSection — 16×16×16 block storage.
// Java reference: net.minecraft.world.chunk.storage.ExtendedBlockStorage
// ═══════════════════════════════════════════════════════════════════════════

struct ChunkSection {
    static constexpr int32_t BLOCKS = 4096;     // 16*16*16
    static constexpr int32_t NIBBLE = 2048;     // 4096/2

    uint8_t blockLSB[BLOCKS] = {};     // Block ID low 8 bits
    uint8_t metadata[NIBBLE] = {};     // 4-bit metadata (nibble array)
    uint8_t blockLight[NIBBLE] = {};   // 4-bit block light
    uint8_t skyLight[NIBBLE] = {};     // 4-bit sky light
    uint8_t blockMSB[NIBBLE] = {};     // Block ID high 4 bits (for ID > 255)
    bool hasMSB = false;               // Whether MSB array is populated
    bool isEmpty_ = true;              // Java: ExtendedBlockStorage.isEmpty()

    // Get block ID at local coordinates
    int32_t getBlockId(int32_t x, int32_t y, int32_t z) const {
        int32_t idx = y * 256 + z * 16 + x;
        int32_t lsb = blockLSB[idx];
        if (!hasMSB) return lsb;
        int32_t nibIdx = idx >> 1;
        int32_t msb = (idx & 1) ? (blockMSB[nibIdx] >> 4) : (blockMSB[nibIdx] & 0x0F);
        return (msb << 8) | lsb;
    }

    // Set block ID at local coordinates
    void setBlockId(int32_t x, int32_t y, int32_t z, int32_t id) {
        int32_t idx = y * 256 + z * 16 + x;
        blockLSB[idx] = static_cast<uint8_t>(id & 0xFF);
        if (id > 255) {
            hasMSB = true;
            int32_t nibIdx = idx >> 1;
            uint8_t msb4 = static_cast<uint8_t>((id >> 8) & 0x0F);
            if (idx & 1) {
                blockMSB[nibIdx] = (blockMSB[nibIdx] & 0x0F) | (msb4 << 4);
            } else {
                blockMSB[nibIdx] = (blockMSB[nibIdx] & 0xF0) | msb4;
            }
        }
        isEmpty_ = false;
    }

    // Get 4-bit metadata
    uint8_t getMetadata(int32_t x, int32_t y, int32_t z) const {
        int32_t idx = y * 256 + z * 16 + x;
        int32_t nibIdx = idx >> 1;
        return (idx & 1) ? (metadata[nibIdx] >> 4) : (metadata[nibIdx] & 0x0F);
    }

    void setMetadata(int32_t x, int32_t y, int32_t z, uint8_t val) {
        int32_t idx = y * 256 + z * 16 + x;
        int32_t nibIdx = idx >> 1;
        if (idx & 1) {
            metadata[nibIdx] = (metadata[nibIdx] & 0x0F) | ((val & 0x0F) << 4);
        } else {
            metadata[nibIdx] = (metadata[nibIdx] & 0xF0) | (val & 0x0F);
        }
    }

    bool isEmpty() const { return isEmpty_; }
};

// ═══════════════════════════════════════════════════════════════════════════
// ChunkData — Full chunk column (16 sections + biome).
// ═══════════════════════════════════════════════════════════════════════════

struct ChunkData {
    int32_t chunkX = 0;
    int32_t chunkZ = 0;
    ChunkSection sections[16];
    uint8_t biomes[256] = {};          // One byte per column (16×16)
    bool hasSkyLight = true;           // true for overworld/end

    // Check if a section exists (non-null equivalent)
    bool hasSection(int32_t y) const {
        return y >= 0 && y < 16 && !sections[y].isEmpty();
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// ChunkExtracted — Intermediate extracted data before compression.
// Java reference: S21PacketChunkData$Extracted
// ═══════════════════════════════════════════════════════════════════════════

struct ChunkExtracted {
    std::vector<uint8_t> data;
    uint16_t primaryBitmask = 0;   // Which sections are included
    uint16_t addBitmask = 0;       // Which sections have MSB data
};

// ═══════════════════════════════════════════════════════════════════════════
// ChunkSerializer — Extracts and compresses chunk data.
// Java reference: S21PacketChunkData.func_149269_a
// ═══════════════════════════════════════════════════════════════════════════

namespace ChunkSerializer {

    // Java: func_149269_a — Extract chunk section data in wire order.
    // sectionMask = which sections to include (0xFFFF = all)
    // fullChunk = include biome data
    inline ChunkExtracted extract(const ChunkData& chunk, bool fullChunk,
                                    uint16_t sectionMask = 0xFFFF) {
        ChunkExtracted result;

        // Pass 1: Determine which sections to include
        for (int32_t i = 0; i < 16; ++i) {
            if (!(sectionMask & (1 << i))) continue;
            if (fullChunk && chunk.sections[i].isEmpty()) continue;
            if (!chunk.hasSection(i) && !fullChunk) continue;

            result.primaryBitmask |= (1 << i);
            if (chunk.sections[i].hasMSB) {
                result.addBitmask |= (1 << i);
            }
        }

        // Calculate total size
        int32_t sectionCount = 0;
        int32_t msbCount = 0;
        for (int32_t i = 0; i < 16; ++i) {
            if (result.primaryBitmask & (1 << i)) {
                ++sectionCount;
                if (result.addBitmask & (1 << i)) ++msbCount;
            }
        }

        size_t totalSize = static_cast<size_t>(sectionCount) * ChunkSection::BLOCKS;  // blockLSB
        totalSize += static_cast<size_t>(sectionCount) * ChunkSection::NIBBLE;        // metadata
        totalSize += static_cast<size_t>(sectionCount) * ChunkSection::NIBBLE;        // blockLight
        if (chunk.hasSkyLight) {
            totalSize += static_cast<size_t>(sectionCount) * ChunkSection::NIBBLE;    // skyLight
        }
        totalSize += static_cast<size_t>(msbCount) * ChunkSection::NIBBLE;            // blockMSB
        if (fullChunk) totalSize += 256;                                              // biomes

        result.data.resize(totalSize);
        size_t offset = 0;

        // Pass 2: Block ID LSB arrays
        for (int32_t i = 0; i < 16; ++i) {
            if (!(result.primaryBitmask & (1 << i))) continue;
            std::memcpy(result.data.data() + offset, chunk.sections[i].blockLSB, ChunkSection::BLOCKS);
            offset += ChunkSection::BLOCKS;
        }

        // Pass 3: Metadata nibble arrays
        for (int32_t i = 0; i < 16; ++i) {
            if (!(result.primaryBitmask & (1 << i))) continue;
            std::memcpy(result.data.data() + offset, chunk.sections[i].metadata, ChunkSection::NIBBLE);
            offset += ChunkSection::NIBBLE;
        }

        // Pass 4: Block light nibble arrays
        for (int32_t i = 0; i < 16; ++i) {
            if (!(result.primaryBitmask & (1 << i))) continue;
            std::memcpy(result.data.data() + offset, chunk.sections[i].blockLight, ChunkSection::NIBBLE);
            offset += ChunkSection::NIBBLE;
        }

        // Pass 5: Sky light nibble arrays (overworld/end only)
        if (chunk.hasSkyLight) {
            for (int32_t i = 0; i < 16; ++i) {
                if (!(result.primaryBitmask & (1 << i))) continue;
                std::memcpy(result.data.data() + offset, chunk.sections[i].skyLight, ChunkSection::NIBBLE);
                offset += ChunkSection::NIBBLE;
            }
        }

        // Pass 6: Block ID MSB nibble arrays (for IDs > 255)
        for (int32_t i = 0; i < 16; ++i) {
            if (!(result.addBitmask & (1 << i))) continue;
            std::memcpy(result.data.data() + offset, chunk.sections[i].blockMSB, ChunkSection::NIBBLE);
            offset += ChunkSection::NIBBLE;
        }

        // Pass 7: Biome data (full chunk only)
        if (fullChunk) {
            std::memcpy(result.data.data() + offset, chunk.biomes, 256);
            offset += 256;
        }

        return result;
    }

    // Compress extracted data with zlib deflate.
    inline std::vector<uint8_t> compress(const std::vector<uint8_t>& raw) {
        std::vector<uint8_t> compressed(raw.size() + 128); // Generous initial size
        z_stream strm = {};
        deflateInit(&strm, Z_DEFAULT_COMPRESSION);
        strm.avail_in = static_cast<uInt>(raw.size());
        strm.next_in = const_cast<Bytef*>(raw.data());
        strm.avail_out = static_cast<uInt>(compressed.size());
        strm.next_out = compressed.data();
        deflate(&strm, Z_FINISH);
        compressed.resize(strm.total_out);
        deflateEnd(&strm);
        return compressed;
    }

    // ─── S21 Chunk Data — Single chunk packet ───
    // Java: S21PacketChunkData.writePacketData
    // Wire: int chunkX, int chunkZ, bool fullChunk, short primaryBitmask,
    //       short addBitmask, int compressedLen, byte[] compressed
    inline std::vector<uint8_t> buildChunkDataPacket(const ChunkData& chunk,
                                                       bool fullChunk,
                                                       uint16_t sectionMask = 0xFFFF) {
        auto extracted = extract(chunk, fullChunk, sectionMask);
        auto compressed = compress(extracted.data);

        PacketWriter w(ClientboundPacket::ChunkData);
        w.writeInt(chunk.chunkX);
        w.writeInt(chunk.chunkZ);
        w.writeBool(fullChunk);
        w.writeShort(static_cast<int16_t>(extracted.primaryBitmask));
        w.writeShort(static_cast<int16_t>(extracted.addBitmask));
        w.writeInt(static_cast<int32_t>(compressed.size()));
        w.writeBytes(compressed);
        return w.toFramed();
    }

    // ─── S26 Map Chunk Bulk — Multiple chunks in one packet ───
    // Java: S26PacketMapChunkBulk.writePacketData
    // Wire: short chunkCount, int compressedLen, bool hasSkyLight,
    //       byte[] compressed, then per-chunk: int chunkX, int chunkZ,
    //       short primaryBitmask, short addBitmask
    inline std::vector<uint8_t> buildBulkChunkPacket(
            const std::vector<const ChunkData*>& chunks,
            bool fullChunk) {

        // Extract all chunks
        struct BulkEntry {
            ChunkExtracted extracted;
            int32_t chunkX, chunkZ;
        };
        std::vector<BulkEntry> entries;
        entries.reserve(chunks.size());

        // Concatenate all uncompressed data
        std::vector<uint8_t> allRaw;
        bool hasSky = false;
        for (const auto* chunk : chunks) {
            BulkEntry e;
            e.chunkX = chunk->chunkX;
            e.chunkZ = chunk->chunkZ;
            e.extracted = extract(*chunk, fullChunk);
            allRaw.insert(allRaw.end(), e.extracted.data.begin(), e.extracted.data.end());
            if (chunk->hasSkyLight) hasSky = true;
            entries.push_back(std::move(e));
        }

        auto compressed = compress(allRaw);

        PacketWriter w(ClientboundPacket::MapChunkBulk);
        w.writeShort(static_cast<int16_t>(entries.size()));
        w.writeInt(static_cast<int32_t>(compressed.size()));
        w.writeBool(hasSky);
        w.writeBytes(compressed);

        // Per-chunk metadata
        for (const auto& e : entries) {
            w.writeInt(e.chunkX);
            w.writeInt(e.chunkZ);
            w.writeShort(static_cast<int16_t>(e.extracted.primaryBitmask));
            w.writeShort(static_cast<int16_t>(e.extracted.addBitmask));
        }

        return w.toFramed();
    }

    // ─── Unload chunk (send empty S21 with primaryBitmask=0) ───
    inline std::vector<uint8_t> buildUnloadChunkPacket(int32_t chunkX, int32_t chunkZ) {
        PacketWriter w(ClientboundPacket::ChunkData);
        w.writeInt(chunkX);
        w.writeInt(chunkZ);
        w.writeBool(true); // full chunk
        w.writeShort(0);   // no sections
        w.writeShort(0);   // no add data
        // Compressed empty data: just biome array (256 zeroes)
        std::vector<uint8_t> biomes(256, 0);
        auto compressed = compress(biomes);
        w.writeInt(static_cast<int32_t>(compressed.size()));
        w.writeBytes(compressed);
        return w.toFramed();
    }

} // namespace ChunkSerializer

} // namespace mccpp
