#pragma once
// RegionFile — Anvil/McRegion .mca file reader/writer.
// Implements the vanilla region file format from aqo.java (RegionFile).
//
// Format:
//   - File divided into 4KB sectors
//   - Header: 2 sectors (8KB total)
//     - Sector 0 (bytes 0-4095): 1024 location entries (4 bytes each)
//       - Bits 31-8: sector offset, Bits 7-0: sector count
//     - Sector 1 (bytes 4096-8191): 1024 timestamp entries (4 bytes each, epoch seconds)
//   - Chunk data: compressed NBT with 5-byte header
//     - Bytes 0-3: exact length (big-endian int)
//     - Byte 4: compression type (1=gzip, 2=zlib)
//     - Remaining bytes: compressed data
//
// Each region file holds a 32x32 grid of chunks.
// File naming: r.X.Z.mca where X,Z are region coordinates (chunk >> 5)

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <zlib.h>

#include "nbt/NBT.h"

namespace mc {

class RegionFile {
public:
    static constexpr int SECTOR_SIZE = 4096;
    static constexpr int SECTOR_INTS = SECTOR_SIZE / 4; // 1024 entries per header sector
    static constexpr int REGION_SIZE = 32; // 32x32 chunks per region

    // Constructor: opens or creates a region file
    explicit RegionFile(const std::string& path) : path_(path) {
        // Ensure parent directory exists
        auto parent = std::filesystem::path(path).parent_path();
        if (!parent.empty()) {
            std::filesystem::create_directories(parent);
        }

        if (std::filesystem::exists(path)) {
            loadHeader();
        } else {
            // Initialize empty header
            std::memset(locations_, 0, sizeof(locations_));
            std::memset(timestamps_, 0, sizeof(timestamps_));
        }
    }

    // Check if a chunk exists in this region
    bool hasChunk(int localX, int localZ) const {
        if (localX < 0 || localX >= 32 || localZ < 0 || localZ >= 32) return false;
        return locations_[localX + localZ * 32] != 0;
    }

    // Read chunk NBT data from region file
    // localX/localZ are 0-31 within the region
    std::shared_ptr<nbt::NBTTagCompound> readChunk(int localX, int localZ) {
        if (!hasChunk(localX, localZ)) return nullptr;

        uint32_t loc = locations_[localX + localZ * 32];
        uint32_t sectorOffset = (loc >> 8) & 0xFFFFFF;
        uint32_t sectorCount  = loc & 0xFF;

        if (sectorOffset == 0 || sectorCount == 0) return nullptr;

        std::ifstream file(path_, std::ios::binary);
        if (!file) return nullptr;

        // Seek to chunk data
        file.seekg(sectorOffset * SECTOR_SIZE);

        // Read length + compression type
        uint8_t header[5];
        file.read(reinterpret_cast<char*>(header), 5);
        if (!file) return nullptr;

        uint32_t length = (header[0] << 24) | (header[1] << 16) |
                          (header[2] << 8)  | header[3];
        uint8_t compressionType = header[4];

        if (length <= 1) return nullptr;

        // Read compressed data
        std::vector<uint8_t> compressed(length - 1);
        file.read(reinterpret_cast<char*>(compressed.data()), compressed.size());
        if (!file) return nullptr;

        // Decompress
        std::vector<uint8_t> decompressed;
        if (compressionType == 2) {
            // Zlib
            decompressed = zlibDecompress(compressed);
        } else if (compressionType == 1) {
            // Gzip
            decompressed = gzipDecompress(compressed);
        } else {
            std::cerr << "[RegionFile] Unknown compression type: "
                      << (int)compressionType << "\n";
            return nullptr;
        }

        if (decompressed.empty()) return nullptr;

        // Parse NBT
        try {
            return nbt::deserializeNBT(decompressed);
        } catch (...) {
            return nullptr;
        }
    }

    // Write chunk NBT data to region file
    void writeChunk(int localX, int localZ,
                    const std::shared_ptr<nbt::NBTTagCompound>& tag) {
        if (localX < 0 || localX >= 32 || localZ < 0 || localZ >= 32) return;

        // Serialize NBT
        auto nbtData = nbt::serializeNBT(*tag);

        // Compress with zlib (type 2)
        std::vector<uint8_t> compressed = zlibCompress(nbtData);

        // Total chunk data: 4 bytes length + 1 byte type + compressed data
        uint32_t totalLen = static_cast<uint32_t>(compressed.size()) + 1;
        uint32_t sectorsNeeded = (totalLen + 4 + SECTOR_SIZE - 1) / SECTOR_SIZE;

        // Find free sectors
        uint32_t sectorOffset = findFreeSectors(sectorsNeeded, localX, localZ);

        // Write chunk data
        std::fstream file(path_, std::ios::in | std::ios::out | std::ios::binary);
        if (!file) {
            // Create new file
            file.open(path_, std::ios::out | std::ios::binary);
            if (!file) {
                std::cerr << "[RegionFile] Cannot open " << path_ << "\n";
                return;
            }
            // Write empty header (8KB)
            std::vector<uint8_t> emptyHeader(SECTOR_SIZE * 2, 0);
            file.write(reinterpret_cast<char*>(emptyHeader.data()), emptyHeader.size());
            file.close();
            file.open(path_, std::ios::in | std::ios::out | std::ios::binary);
        }

        file.seekp(sectorOffset * SECTOR_SIZE);

        // Write: length (4 bytes, big-endian) + compression type (1 byte) + data
        uint8_t header[5];
        header[0] = (totalLen >> 24) & 0xFF;
        header[1] = (totalLen >> 16) & 0xFF;
        header[2] = (totalLen >> 8)  & 0xFF;
        header[3] = totalLen & 0xFF;
        header[4] = 2; // Zlib compression
        file.write(reinterpret_cast<char*>(header), 5);
        file.write(reinterpret_cast<char*>(compressed.data()), compressed.size());

        // Pad to sector boundary
        size_t written = 5 + compressed.size();
        size_t padding = (sectorsNeeded * SECTOR_SIZE) - written;
        if (padding > 0) {
            std::vector<uint8_t> pad(padding, 0);
            file.write(reinterpret_cast<char*>(pad.data()), pad.size());
        }

        // Update location table
        int idx = localX + localZ * 32;
        locations_[idx] = (sectorOffset << 8) | (sectorsNeeded & 0xFF);
        timestamps_[idx] = static_cast<uint32_t>(std::time(nullptr));

        // Write header
        file.seekp(0);
        for (int i = 0; i < 1024; ++i) {
            uint8_t buf[4];
            buf[0] = (locations_[i] >> 24) & 0xFF;
            buf[1] = (locations_[i] >> 16) & 0xFF;
            buf[2] = (locations_[i] >> 8)  & 0xFF;
            buf[3] = locations_[i] & 0xFF;
            file.write(reinterpret_cast<char*>(buf), 4);
        }
        for (int i = 0; i < 1024; ++i) {
            uint8_t buf[4];
            buf[0] = (timestamps_[i] >> 24) & 0xFF;
            buf[1] = (timestamps_[i] >> 16) & 0xFF;
            buf[2] = (timestamps_[i] >> 8)  & 0xFF;
            buf[3] = timestamps_[i] & 0xFF;
            file.write(reinterpret_cast<char*>(buf), 4);
        }
    }

    // Get region coordinates from chunk coordinates
    static int regionCoord(int chunkCoord) { return chunkCoord >> 5; }
    static int localCoord(int chunkCoord) { return chunkCoord & 31; }

    // Build region file path
    static std::string regionPath(const std::string& worldDir, int regionX, int regionZ) {
        return worldDir + "/region/r." + std::to_string(regionX) + "."
               + std::to_string(regionZ) + ".mca";
    }

private:
    std::string path_;
    uint32_t locations_[1024];
    uint32_t timestamps_[1024];

    void loadHeader() {
        std::ifstream file(path_, std::ios::binary);
        if (!file) return;

        // Read locations (sector 0)
        for (int i = 0; i < 1024; ++i) {
            uint8_t buf[4];
            file.read(reinterpret_cast<char*>(buf), 4);
            locations_[i] = (buf[0] << 24) | (buf[1] << 16) |
                            (buf[2] << 8)  | buf[3];
        }

        // Read timestamps (sector 1)
        for (int i = 0; i < 1024; ++i) {
            uint8_t buf[4];
            file.read(reinterpret_cast<char*>(buf), 4);
            timestamps_[i] = (buf[0] << 24) | (buf[1] << 16) |
                             (buf[2] << 8)  | buf[3];
        }
    }

    uint32_t findFreeSectors(uint32_t count, int localX, int localZ) {
        // Check if existing allocation is large enough
        int idx = localX + localZ * 32;
        uint32_t existing = locations_[idx];
        if (existing != 0) {
            uint32_t existCount = existing & 0xFF;
            if (existCount >= count) {
                return (existing >> 8) & 0xFFFFFF;
            }
        }

        // Find file size and allocate at the end
        if (!std::filesystem::exists(path_)) {
            return 2; // After header (2 sectors)
        }

        auto fileSize = std::filesystem::file_size(path_);
        uint32_t totalSectors = static_cast<uint32_t>(
            (fileSize + SECTOR_SIZE - 1) / SECTOR_SIZE);
        if (totalSectors < 2) totalSectors = 2; // At least header

        return totalSectors;
    }

    // Zlib compress
    static std::vector<uint8_t> zlibCompress(const std::vector<uint8_t>& input) {
        uLongf compLen = compressBound(input.size());
        std::vector<uint8_t> output(compLen);
        if (compress2(output.data(), &compLen, input.data(), input.size(),
                      Z_DEFAULT_COMPRESSION) != Z_OK) {
            return {};
        }
        output.resize(compLen);
        return output;
    }

    // Zlib decompress
    static std::vector<uint8_t> zlibDecompress(const std::vector<uint8_t>& input) {
        std::vector<uint8_t> output(input.size() * 8);
        for (int attempt = 0; attempt < 5; ++attempt) {
            uLongf outLen = output.size();
            int ret = uncompress(output.data(), &outLen, input.data(), input.size());
            if (ret == Z_OK) {
                output.resize(outLen);
                return output;
            }
            if (ret == Z_BUF_ERROR) {
                output.resize(output.size() * 2);
                continue;
            }
            return {};
        }
        return {};
    }

    // Gzip decompress
    static std::vector<uint8_t> gzipDecompress(const std::vector<uint8_t>& input) {
        z_stream strm{};
        strm.next_in = const_cast<uint8_t*>(input.data());
        strm.avail_in = input.size();

        if (inflateInit2(&strm, 15 + 16) != Z_OK) return {};

        std::vector<uint8_t> output(input.size() * 4);
        strm.next_out = output.data();
        strm.avail_out = output.size();

        while (true) {
            int ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_END) break;
            if (ret != Z_OK) {
                inflateEnd(&strm);
                return {};
            }
            if (strm.avail_out == 0) {
                size_t used = output.size();
                output.resize(output.size() * 2);
                strm.next_out = output.data() + used;
                strm.avail_out = output.size() - used;
            }
        }

        output.resize(strm.total_out);
        inflateEnd(&strm);
        return output;
    }
};

} // namespace mc
