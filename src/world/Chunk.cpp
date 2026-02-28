/**
 * Chunk.cpp — Chunk format implementation.
 *
 * Java references:
 *   net.minecraft.world.chunk.NibbleArray
 *   net.minecraft.world.chunk.storage.ExtendedBlockStorage
 *   net.minecraft.world.chunk.Chunk
 *   net.minecraft.world.chunk.storage.RegionFile
 *   net.minecraft.world.chunk.storage.AnvilChunkLoader
 *
 * Implements:
 *   - ChunkSection: 16x16x16 block storage with nibble arrays
 *   - Chunk: 16 sections + biomes + NBT serialize/deserialize
 *   - RegionFile: Anvil .mca file reader/writer with zlib compression
 */

#include "world/Chunk.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <stdexcept>

// zlib for region file compression
#include <zlib.h>

namespace mccpp {

// ═════════════════════════════════════════════════════════════════════════════
// ChunkSection (ExtendedBlockStorage)
// ═════════════════════════════════════════════════════════════════════════════

ChunkSection::ChunkSection(int yBase, bool hasSkylight)
    : yBase_(yBase)
    , metadata_(4096, 4)
    , blocklight_(4096, 4)
{
    blockLSB_.fill(0);
    if (hasSkylight) {
        skylight_ = std::make_unique<NibbleArray>(4096, 4);
    }
}

Block* ChunkSection::getBlock(int x, int y, int z) const {
    // Java: ExtendedBlockStorage.getBlockByExtId(x, y, z)
    // Index: y << 8 | z << 4 | x
    int id = blockLSB_[y << 8 | z << 4 | x] & 0xFF;
    if (blockMSB_) {
        id |= (blockMSB_->get(x, y, z) << 8);
    }
    return Block::getBlockById(id);
}

void ChunkSection::setBlock(int x, int y, int z, Block* block) {
    // Java: ExtendedBlockStorage.setExtBlockID(x, y, z, block)
    int idx = y << 8 | z << 4 | x;

    // Decrement counts for old block
    int oldId = blockLSB_[idx] & 0xFF;
    if (blockMSB_) {
        oldId |= (blockMSB_->get(x, y, z) << 8);
    }
    Block* oldBlock = Block::getBlockById(oldId);
    if (oldBlock && oldBlock->getMaterial() != Material::Air) {
        --blockRefCount_;
        if (oldBlock->getTickRandomly()) {
            --tickRefCount_;
        }
    }

    // Increment counts for new block
    if (block && block->getMaterial() != Material::Air) {
        ++blockRefCount_;
        if (block->getTickRandomly()) {
            ++tickRefCount_;
        }
    }

    int newId = block ? Block::getIdFromBlock(block) : 0;
    blockLSB_[idx] = static_cast<uint8_t>(newId & 0xFF);

    if (newId > 255) {
        if (!blockMSB_) {
            blockMSB_ = std::make_unique<NibbleArray>(4096, 4);
        }
        blockMSB_->set(x, y, z, (newId & 0xF00) >> 8);
    } else if (blockMSB_) {
        blockMSB_->set(x, y, z, 0);
    }
}

int ChunkSection::getBlockMetadata(int x, int y, int z) const {
    return metadata_.get(x, y, z);
}

void ChunkSection::setBlockMetadata(int x, int y, int z, int meta) {
    const_cast<NibbleArray&>(metadata_).set(x, y, z, meta);
}

int ChunkSection::getBlockLight(int x, int y, int z) const {
    return blocklight_.get(x, y, z);
}

void ChunkSection::setBlockLight(int x, int y, int z, int val) {
    const_cast<NibbleArray&>(blocklight_).set(x, y, z, val);
}

int ChunkSection::getSkyLight(int x, int y, int z) const {
    return skylight_ ? skylight_->get(x, y, z) : 0;
}

void ChunkSection::setSkyLight(int x, int y, int z, int val) {
    if (skylight_) skylight_->set(x, y, z, val);
}

void ChunkSection::recalcRefCounts() {
    // Java: ExtendedBlockStorage.removeInvalidBlocks()
    blockRefCount_ = 0;
    tickRefCount_ = 0;
    for (int y = 0; y < 16; ++y) {
        for (int z = 0; z < 16; ++z) {
            for (int x = 0; x < 16; ++x) {
                Block* block = getBlock(x, y, z);
                if (block && block->getMaterial() != Material::Air) {
                    ++blockRefCount_;
                    if (block->getTickRandomly()) {
                        ++tickRefCount_;
                    }
                }
            }
        }
    }
}

void ChunkSection::setBlockLSBArray(const std::vector<uint8_t>& arr) {
    size_t copyLen = std::min(arr.size(), blockLSB_.size());
    std::memcpy(blockLSB_.data(), arr.data(), copyLen);
}

// ═════════════════════════════════════════════════════════════════════════════
// Chunk
// ═════════════════════════════════════════════════════════════════════════════

Block* Chunk::getBlock(int x, int y, int z) const {
    int sectionIdx = y >> 4;
    if (sectionIdx < 0 || sectionIdx >= SECTION_COUNT || !sections[sectionIdx]) {
        return Block::getBlockById(0); // air
    }
    return sections[sectionIdx]->getBlock(x, y & 0xF, z);
}

void Chunk::setBlock(int x, int y, int z, Block* block) {
    int sectionIdx = y >> 4;
    if (sectionIdx < 0 || sectionIdx >= SECTION_COUNT) return;
    if (!sections[sectionIdx]) {
        if (!block || block->getMaterial() == Material::Air) return;
        sections[sectionIdx] = std::make_unique<ChunkSection>(sectionIdx << 4, true);
    }
    sections[sectionIdx]->setBlock(x, y & 0xF, z, block);
}

int Chunk::getBlockMetadata(int x, int y, int z) const {
    int sectionIdx = y >> 4;
    if (sectionIdx < 0 || sectionIdx >= SECTION_COUNT || !sections[sectionIdx]) return 0;
    return sections[sectionIdx]->getBlockMetadata(x, y & 0xF, z);
}

void Chunk::setBlockMetadata(int x, int y, int z, int meta) {
    int sectionIdx = y >> 4;
    if (sectionIdx < 0 || sectionIdx >= SECTION_COUNT || !sections[sectionIdx]) return;
    sections[sectionIdx]->setBlockMetadata(x, y & 0xF, z, meta);
}

std::shared_ptr<nbt::NBTTagCompound> Chunk::writeToNBT() const {
    // Java reference: AnvilChunkLoader.writeChunkToNBT()
    auto level = std::make_shared<nbt::NBTTagCompound>();

    level->setByte("V", 1);
    level->setInteger("xPos", xPosition);
    level->setInteger("zPos", zPosition);
    level->setLong("LastUpdate", 0); // world time not available yet
    level->setIntArray("HeightMap", std::vector<int32_t>(heightMap.begin(), heightMap.end()));
    level->setBoolean("TerrainPopulated", isTerrainPopulated);
    level->setBoolean("LightPopulated", isLightPopulated);
    level->setLong("InhabitedTime", inhabitedTime);

    // Sections
    auto sectionList = std::make_unique<nbt::NBTTagList>();
    for (int i = 0; i < SECTION_COUNT; ++i) {
        if (!sections[i] || sections[i]->isEmpty()) continue;
        auto sectionTag = std::make_unique<nbt::NBTTagCompound>();

        sectionTag->setByte("Y", static_cast<int8_t>((sections[i]->getYBase() >> 4) & 0xFF));

        // Block LSB
        const auto& lsb = sections[i]->getBlockLSBArray();
        sectionTag->setByteArray("Blocks", std::vector<int8_t>(lsb.begin(), lsb.end()));

        // Block MSB (Add) — optional
        if (sections[i]->getBlockMSBArray()) {
            const auto& msbData = sections[i]->getBlockMSBArray()->data;
            sectionTag->setByteArray("Add", std::vector<int8_t>(msbData.begin(), msbData.end()));
        }

        // Metadata
        const auto& metaData = sections[i]->getMetadataArray().data;
        sectionTag->setByteArray("Data", std::vector<int8_t>(metaData.begin(), metaData.end()));

        // Block light
        const auto& blData = sections[i]->getBlocklightArray().data;
        sectionTag->setByteArray("BlockLight", std::vector<int8_t>(blData.begin(), blData.end()));

        // Sky light
        if (sections[i]->getSkylightArray()) {
            const auto& slData = sections[i]->getSkylightArray()->data;
            sectionTag->setByteArray("SkyLight", std::vector<int8_t>(slData.begin(), slData.end()));
        } else {
            sectionTag->setByteArray("SkyLight", std::vector<int8_t>(2048, 0));
        }

        sectionList->appendTag(std::move(sectionTag));
    }
    level->setTag("Sections", std::move(sectionList));

    // Biomes
    level->setByteArray("Biomes", std::vector<int8_t>(biomes.begin(), biomes.end()));

    // Entities and TileEntities — empty lists for now (entity system not yet implemented)
    level->setTag("Entities", std::make_unique<nbt::NBTTagList>());
    level->setTag("TileEntities", std::make_unique<nbt::NBTTagList>());

    return level;
}

std::unique_ptr<Chunk> Chunk::readFromNBT(const nbt::NBTTagCompound& levelTag) {
    // Java reference: AnvilChunkLoader.readChunkFromNBT()
    int x = levelTag.getInteger("xPos");
    int z = levelTag.getInteger("zPos");

    auto chunk = std::make_unique<Chunk>(x, z);

    // Height map
    auto hm = levelTag.getIntArray("HeightMap");
    for (size_t i = 0; i < std::min(hm.size(), chunk->heightMap.size()); ++i) {
        chunk->heightMap[i] = hm[i];
    }

    chunk->isTerrainPopulated = levelTag.getBoolean("TerrainPopulated");
    chunk->isLightPopulated = levelTag.getBoolean("LightPopulated");
    chunk->inhabitedTime = levelTag.getLong("InhabitedTime");

    // Sections
    auto sectionsTag = levelTag.getTagList("Sections", 10);
    if (sectionsTag) {
        for (int i = 0; i < sectionsTag->tagCount(); ++i) {
            auto sectionTag = sectionsTag->getCompoundTagAt(i);
            if (!sectionTag) continue;

            int yIdx = sectionTag->getByte("Y") & 0xFF;
            bool hasSkylight = sectionTag->hasKey("SkyLight", 7);

            auto section = std::make_unique<ChunkSection>(yIdx << 4, hasSkylight);

            // Block LSB
            auto blocks = sectionTag->getByteArray("Blocks");
            section->setBlockLSBArray(std::vector<uint8_t>(blocks.begin(), blocks.end()));

            // Block MSB (Add)
            if (sectionTag->hasKey("Add", 7)) {
                auto add = sectionTag->getByteArray("Add");
                section->setBlockMSBArray(std::make_unique<NibbleArray>(
                    std::vector<uint8_t>(add.begin(), add.end()), 4));
            }

            // Metadata
            auto data = sectionTag->getByteArray("Data");
            section->setMetadataArray(NibbleArray(std::vector<uint8_t>(data.begin(), data.end()), 4));

            // Block light
            auto bl = sectionTag->getByteArray("BlockLight");
            section->setBlocklightArray(NibbleArray(std::vector<uint8_t>(bl.begin(), bl.end()), 4));

            // Sky light
            if (hasSkylight) {
                auto sl = sectionTag->getByteArray("SkyLight");
                section->setSkylightArray(std::make_unique<NibbleArray>(
                    std::vector<uint8_t>(sl.begin(), sl.end()), 4));
            }

            section->recalcRefCounts();

            if (yIdx >= 0 && yIdx < SECTION_COUNT) {
                chunk->sections[yIdx] = std::move(section);
            }
        }
    }

    // Biomes
    if (levelTag.hasKey("Biomes", 7)) {
        auto biomeArr = levelTag.getByteArray("Biomes");
        for (size_t i = 0; i < std::min(biomeArr.size(), chunk->biomes.size()); ++i) {
            chunk->biomes[i] = static_cast<uint8_t>(biomeArr[i]);
        }
    }

    return chunk;
}

// ═════════════════════════════════════════════════════════════════════════════
// RegionFile — Anvil .mca I/O
// ═════════════════════════════════════════════════════════════════════════════

// Helper: read big-endian int32 from buffer
static int32_t readBE32(const uint8_t* p) {
    return (static_cast<int32_t>(p[0]) << 24) |
           (static_cast<int32_t>(p[1]) << 16) |
           (static_cast<int32_t>(p[2]) << 8) |
            static_cast<int32_t>(p[3]);
}

// Helper: write big-endian int32 to buffer
static void writeBE32(uint8_t* p, int32_t v) {
    p[0] = static_cast<uint8_t>((v >> 24) & 0xFF);
    p[1] = static_cast<uint8_t>((v >> 16) & 0xFF);
    p[2] = static_cast<uint8_t>((v >> 8) & 0xFF);
    p[3] = static_cast<uint8_t>(v & 0xFF);
}

RegionFile::RegionFile(const std::string& path) : path_(path) {
    // Java reference: RegionFile constructor
    file_.open(path, std::ios::in | std::ios::out | std::ios::binary);

    if (!file_.is_open()) {
        // Create new file
        file_.open(path, std::ios::out | std::ios::binary);
        if (!file_.is_open()) {
            std::cerr << "[RegionFile] Failed to create " << path << "\n";
            return;
        }
        // Write empty header (8192 bytes = offsets + timestamps)
        std::vector<uint8_t> header(8192, 0);
        file_.write(reinterpret_cast<const char*>(header.data()), header.size());
        file_.close();
        file_.open(path, std::ios::in | std::ios::out | std::ios::binary);
    }

    // Read header
    file_.seekg(0, std::ios::end);
    auto fileLen = file_.tellg();

    if (fileLen < 8192) {
        // Pad to minimum size
        file_.seekp(0, std::ios::end);
        std::vector<uint8_t> pad(8192 - static_cast<int>(fileLen), 0);
        file_.write(reinterpret_cast<const char*>(pad.data()), pad.size());
        file_.flush();
        fileLen = 8192;
    }

    // Pad to sector boundary
    auto remainder = static_cast<int64_t>(fileLen) & 0xFFF;
    if (remainder != 0) {
        file_.seekp(0, std::ios::end);
        std::vector<uint8_t> pad(4096 - static_cast<int>(remainder), 0);
        file_.write(reinterpret_cast<const char*>(pad.data()), pad.size());
        file_.flush();
        fileLen = file_.tellp();
    }

    int totalSectors = static_cast<int>(static_cast<int64_t>(fileLen) / 4096);
    sectorFree_.resize(totalSectors, true);
    sectorFree_[0] = false; // offsets header
    sectorFree_[1] = false; // timestamps header

    // Read offsets
    file_.seekg(0);
    uint8_t buf[4];
    for (int i = 0; i < 1024; ++i) {
        file_.read(reinterpret_cast<char*>(buf), 4);
        offsets_[i] = readBE32(buf);

        int offset = offsets_[i];
        if (offset != 0) {
            int sectorStart = offset >> 8;
            int sectorCount = offset & 0xFF;
            if (sectorStart + sectorCount <= totalSectors) {
                for (int s = 0; s < sectorCount; ++s) {
                    sectorFree_[sectorStart + s] = false;
                }
            }
        }
    }

    // Read timestamps
    for (int i = 0; i < 1024; ++i) {
        file_.read(reinterpret_cast<char*>(buf), 4);
        timestamps_[i] = readBE32(buf);
    }
}

RegionFile::~RegionFile() {
    close();
}

void RegionFile::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) {
        file_.close();
    }
}

bool RegionFile::isChunkSaved(int localX, int localZ) const {
    if (outOfBounds(localX, localZ)) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return getOffset(localX, localZ) != 0;
}

std::optional<std::vector<uint8_t>> RegionFile::readChunkData(int localX, int localZ) {
    if (outOfBounds(localX, localZ)) return std::nullopt;

    std::lock_guard<std::mutex> lock(mutex_);
    if (!file_.is_open()) return std::nullopt;

    int offset = getOffset(localX, localZ);
    if (offset == 0) return std::nullopt;

    int sectorStart = offset >> 8;
    int sectorCount = offset & 0xFF;

    if (sectorStart + sectorCount > static_cast<int>(sectorFree_.size())) {
        return std::nullopt;
    }

    // Seek to sector
    file_.seekg(static_cast<int64_t>(sectorStart) * 4096);

    // Read length and compression type
    uint8_t header[5];
    file_.read(reinterpret_cast<char*>(header), 5);
    int32_t dataLength = readBE32(header);
    uint8_t compressionType = header[4];

    if (dataLength <= 0 || dataLength > 4096 * sectorCount) {
        return std::nullopt;
    }

    // Read compressed data
    std::vector<uint8_t> compressed(dataLength - 1);
    file_.read(reinterpret_cast<char*>(compressed.data()), compressed.size());

    // Decompress
    if (compressionType == 1) {
        // GZip — not commonly used but supported
        // TODO: gzip decompression if needed
        return std::nullopt;
    } else if (compressionType == 2) {
        // Zlib (inflate)
        std::vector<uint8_t> decompressed;
        decompressed.resize(1024 * 1024); // 1MB max

        z_stream strm{};
        strm.next_in = compressed.data();
        strm.avail_in = static_cast<uInt>(compressed.size());
        strm.next_out = decompressed.data();
        strm.avail_out = static_cast<uInt>(decompressed.size());

        if (inflateInit(&strm) != Z_OK) return std::nullopt;
        int ret = inflate(&strm, Z_FINISH);
        inflateEnd(&strm);

        if (ret != Z_STREAM_END) return std::nullopt;

        decompressed.resize(strm.total_out);
        return decompressed;
    }

    return std::nullopt;
}

bool RegionFile::writeChunkData(int localX, int localZ, const std::vector<uint8_t>& data) {
    if (outOfBounds(localX, localZ)) return false;

    std::lock_guard<std::mutex> lock(mutex_);
    if (!file_.is_open()) return false;

    // Compress with zlib (deflate)
    std::vector<uint8_t> compressed(compressBound(static_cast<uLong>(data.size())));
    uLongf compLen = static_cast<uLongf>(compressed.size());
    if (compress2(compressed.data(), &compLen, data.data(),
                  static_cast<uLong>(data.size()), Z_DEFAULT_COMPRESSION) != Z_OK) {
        return false;
    }
    compressed.resize(compLen);

    // Calculate sectors needed: 5 bytes header (4 len + 1 type) + data
    int totalBytes = 5 + static_cast<int>(compressed.size());
    int sectorsNeeded = (totalBytes + 4095) / 4096;

    if (sectorsNeeded >= 256) return false;

    int oldOffset = getOffset(localX, localZ);
    int oldSectorStart = oldOffset >> 8;
    int oldSectorCount = oldOffset & 0xFF;

    // Free old sectors
    if (oldSectorStart != 0) {
        for (int i = 0; i < oldSectorCount; ++i) {
            if (oldSectorStart + i < static_cast<int>(sectorFree_.size())) {
                sectorFree_[oldSectorStart + i] = true;
            }
        }
    }

    // Find free space or append
    int newSector = -1;

    // Can reuse if same size
    if (oldSectorStart != 0 && oldSectorCount == sectorsNeeded) {
        newSector = oldSectorStart;
    } else {
        // Search for contiguous free sectors
        int runStart = -1;
        int runLen = 0;
        for (int i = 2; i < static_cast<int>(sectorFree_.size()); ++i) {
            if (sectorFree_[i]) {
                if (runLen == 0) runStart = i;
                ++runLen;
                if (runLen >= sectorsNeeded) {
                    newSector = runStart;
                    break;
                }
            } else {
                runLen = 0;
            }
        }

        if (newSector == -1) {
            // Append at end
            newSector = static_cast<int>(sectorFree_.size());
            file_.seekp(0, std::ios::end);
            for (int i = 0; i < sectorsNeeded; ++i) {
                std::vector<uint8_t> empty(4096, 0);
                file_.write(reinterpret_cast<const char*>(empty.data()), empty.size());
                sectorFree_.push_back(false);
            }
        }
    }

    // Mark sectors as used
    for (int i = 0; i < sectorsNeeded; ++i) {
        sectorFree_[newSector + i] = false;
    }

    // Write data
    file_.seekp(static_cast<int64_t>(newSector) * 4096);
    uint8_t hdr[5];
    writeBE32(hdr, static_cast<int32_t>(compressed.size()) + 1);
    hdr[4] = 2; // zlib compression
    file_.write(reinterpret_cast<const char*>(hdr), 5);
    file_.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());

    // Pad to sector boundary
    int written = 5 + static_cast<int>(compressed.size());
    int padding = (sectorsNeeded * 4096) - written;
    if (padding > 0) {
        std::vector<uint8_t> pad(padding, 0);
        file_.write(reinterpret_cast<const char*>(pad.data()), pad.size());
    }

    // Update offset and timestamp
    setOffset(localX, localZ, (newSector << 8) | sectorsNeeded);

    auto now = std::chrono::system_clock::now();
    auto epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    setTimestamp(localX, localZ, static_cast<int32_t>(epoch));

    file_.flush();
    return true;
}

void RegionFile::setOffset(int x, int z, int val) {
    offsets_[x + z * 32] = val;
    file_.seekp(static_cast<int64_t>(x + z * 32) * 4);
    uint8_t buf[4];
    writeBE32(buf, val);
    file_.write(reinterpret_cast<const char*>(buf), 4);
}

void RegionFile::setTimestamp(int x, int z, int val) {
    timestamps_[x + z * 32] = val;
    file_.seekp(4096 + static_cast<int64_t>(x + z * 32) * 4);
    uint8_t buf[4];
    writeBE32(buf, val);
    file_.write(reinterpret_cast<const char*>(buf), 4);
}

void RegionFile::writeSector(int sectorNum, const uint8_t* data, int length) {
    file_.seekp(static_cast<int64_t>(sectorNum) * 4096);
    file_.write(reinterpret_cast<const char*>(data), length);
}

} // namespace mccpp
