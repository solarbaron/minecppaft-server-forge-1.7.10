/**
 * RegionFile.h — Anvil region file (.mca) reader/writer.
 *
 * Java reference: net.minecraft.world.chunk.storage.RegionFile
 *
 * Format:
 *   - 32×32 chunks per region file
 *   - 4096-byte sectors
 *   - Header: 2 sectors (8192 bytes)
 *     - Sector 0: offsets[1024] — int per chunk (sectorStart << 8 | sectorCount)
 *     - Sector 1: timestamps[1024] — int per chunk (unix seconds)
 *   - Chunk data: int length + byte compressionType + compressed bytes
 *     - Type 1 = GZip, Type 2 = Zlib (standard)
 *   - Free sector tracking for allocation
 *
 * Thread safety: Methods are synchronized (mutex per file).
 *
 * JNI readiness: Simple file handle, no C++ mangling issues.
 */
#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>
#include <zlib.h>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// RegionFile — Single .mca file for 32×32 chunk region.
// Java reference: net.minecraft.world.chunk.storage.RegionFile
// ═══════════════════════════════════════════════════════════════════════════

class RegionFile {
public:
    static constexpr int32_t SECTOR_SIZE = 4096;
    static constexpr int32_t CHUNKS_PER_REGION = 32;
    static constexpr int32_t HEADER_SECTORS = 2;  // offsets + timestamps
    static constexpr int32_t MAX_SECTORS = 256;    // Max sectors per chunk
    static constexpr uint8_t COMPRESSION_GZIP = 1;
    static constexpr uint8_t COMPRESSION_ZLIB = 2;

    RegionFile() = default;

    // Open/create a region file
    bool open(const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        path_ = path;

        file_.open(path, std::ios::in | std::ios::out | std::ios::binary);
        if (!file_.is_open()) {
            // File doesn't exist — create it
            file_.open(path, std::ios::out | std::ios::binary);
            if (!file_.is_open()) return false;
            file_.close();
            file_.open(path, std::ios::in | std::ios::out | std::ios::binary);
            if (!file_.is_open()) return false;
        }

        // Check file size
        file_.seekg(0, std::ios::end);
        int64_t fileSize = file_.tellg();

        // Java: if file size < 4096, write empty header (2 sectors)
        if (fileSize < SECTOR_SIZE * HEADER_SECTORS) {
            file_.seekp(0);
            std::vector<uint8_t> empty(SECTOR_SIZE * HEADER_SECTORS, 0);
            file_.write(reinterpret_cast<const char*>(empty.data()), empty.size());
            file_.flush();
            fileSize = SECTOR_SIZE * HEADER_SECTORS;
        }

        // Java: pad to sector boundary
        if (fileSize % SECTOR_SIZE != 0) {
            file_.seekp(0, std::ios::end);
            int64_t padding = SECTOR_SIZE - (fileSize % SECTOR_SIZE);
            std::vector<uint8_t> zeros(static_cast<size_t>(padding), 0);
            file_.write(reinterpret_cast<const char*>(zeros.data()), zeros.size());
            file_.flush();
            fileSize += padding;
        }

        int32_t totalSectors = static_cast<int32_t>(fileSize / SECTOR_SIZE);

        // Initialize free sector list
        sectorFree_.resize(static_cast<size_t>(totalSectors), true);
        sectorFree_[0] = false; // Header sector 0 (offsets)
        sectorFree_[1] = false; // Header sector 1 (timestamps)

        // Read offset table (sector 0)
        file_.seekg(0);
        for (int32_t i = 0; i < 1024; ++i) {
            offsets_[i] = readInt();
            if (offsets_[i] != 0) {
                int32_t sectorStart = offsets_[i] >> 8;
                int32_t sectorCount = offsets_[i] & 0xFF;
                if (sectorStart + sectorCount <= totalSectors) {
                    for (int32_t s = 0; s < sectorCount; ++s) {
                        sectorFree_[static_cast<size_t>(sectorStart + s)] = false;
                    }
                }
            }
        }

        // Read timestamp table (sector 1)
        for (int32_t i = 0; i < 1024; ++i) {
            timestamps_[i] = readInt();
        }

        isOpen_ = true;
        return true;
    }

    void close() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (file_.is_open()) {
            file_.flush();
            file_.close();
        }
        isOpen_ = false;
    }

    bool isOpen() const { return isOpen_; }

    // ─── Read chunk data ───

    // Java: getChunkDataInputStream — returns decompressed chunk data
    // Returns empty vector if chunk doesn't exist.
    std::vector<uint8_t> readChunkData(int32_t localX, int32_t localZ) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!isOpen_ || outOfBounds(localX, localZ)) return {};

        int32_t offset = getOffset(localX, localZ);
        if (offset == 0) return {}; // Chunk not saved

        int32_t sectorStart = offset >> 8;
        int32_t sectorCount = offset & 0xFF;

        if (sectorStart + sectorCount > static_cast<int32_t>(sectorFree_.size())) {
            return {}; // Invalid offset
        }

        // Seek to chunk data
        file_.seekg(static_cast<int64_t>(sectorStart) * SECTOR_SIZE);

        // Read length and compression type
        int32_t length = readInt();
        if (length <= 0 || length > SECTOR_SIZE * sectorCount) return {};

        uint8_t compressionType = 0;
        file_.read(reinterpret_cast<char*>(&compressionType), 1);

        // Read compressed data
        std::vector<uint8_t> compressed(static_cast<size_t>(length - 1));
        file_.read(reinterpret_cast<char*>(compressed.data()),
                   static_cast<std::streamsize>(compressed.size()));

        // Decompress
        if (compressionType == COMPRESSION_ZLIB) {
            return decompressZlib(compressed);
        } else if (compressionType == COMPRESSION_GZIP) {
            return decompressGzip(compressed);
        }

        return {}; // Unknown compression
    }

    // ─── Write chunk data ───

    // Java: write(int, int, byte[], int) — write compressed chunk data
    bool writeChunkData(int32_t localX, int32_t localZ,
                         const std::vector<uint8_t>& uncompressedData) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!isOpen_ || outOfBounds(localX, localZ)) return false;

        // Compress with zlib (type 2)
        auto compressed = compressZlib(uncompressedData);
        if (compressed.empty()) return false;

        return writeCompressedData(localX, localZ, compressed);
    }

    // ─── Query ───

    // Java: isChunkSaved
    bool isChunkSaved(int32_t localX, int32_t localZ) const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (outOfBounds(localX, localZ)) return false;
        return offsets_[localX + localZ * 32] != 0;
    }

    int32_t getChunkTimestamp(int32_t localX, int32_t localZ) const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (outOfBounds(localX, localZ)) return 0;
        return timestamps_[localX + localZ * 32];
    }

    // ─── Static helpers ───

    // Get region coords from chunk coords
    static int32_t chunkToRegion(int32_t chunkCoord) {
        return chunkCoord >> 5; // floor(chunkCoord / 32)
    }

    // Get local coords within region
    static int32_t chunkToLocal(int32_t chunkCoord) {
        return chunkCoord & 31; // chunkCoord % 32
    }

    // Build region filename: r.X.Z.mca
    static std::string getFileName(int32_t regionX, int32_t regionZ) {
        return "r." + std::to_string(regionX) + "." + std::to_string(regionZ) + ".mca";
    }

private:
    static bool outOfBounds(int32_t x, int32_t z) {
        return x < 0 || x >= 32 || z < 0 || z >= 32;
    }

    int32_t getOffset(int32_t x, int32_t z) const {
        return offsets_[x + z * 32];
    }

    void setOffset(int32_t x, int32_t z, int32_t value) {
        int32_t idx = x + z * 32;
        offsets_[idx] = value;
        file_.seekp(static_cast<int64_t>(idx) * 4);
        writeInt(value);
    }

    void setTimestamp(int32_t x, int32_t z, int32_t value) {
        int32_t idx = x + z * 32;
        timestamps_[idx] = value;
        file_.seekp(SECTOR_SIZE + static_cast<int64_t>(idx) * 4);
        writeInt(value);
    }

    // Write compressed data with sector allocation
    bool writeCompressedData(int32_t x, int32_t z,
                              const std::vector<uint8_t>& compressed) {
        int32_t dataLen = static_cast<int32_t>(compressed.size());
        // +5 for length(4) + compression_type(1)
        int32_t sectorsNeeded = (dataLen + 5) / SECTOR_SIZE + 1;

        // Java: if >= 256, chunk too large
        if (sectorsNeeded >= MAX_SECTORS) return false;

        int32_t oldOffset = getOffset(x, z);
        int32_t oldStart = oldOffset >> 8;
        int32_t oldCount = oldOffset & 0xFF;

        // Can we reuse the same sectors?
        if (oldStart != 0 && oldCount == sectorsNeeded) {
            writeChunkAtSector(oldStart, compressed);
        } else {
            // Free old sectors
            for (int32_t s = 0; s < oldCount; ++s) {
                if (oldStart + s < static_cast<int32_t>(sectorFree_.size())) {
                    sectorFree_[static_cast<size_t>(oldStart + s)] = true;
                }
            }

            // Find contiguous free run
            int32_t startSector = -1;
            int32_t runLength = 0;

            for (size_t i = 0; i < sectorFree_.size(); ++i) {
                if (sectorFree_[i]) {
                    if (runLength == 0) startSector = static_cast<int32_t>(i);
                    ++runLength;
                    if (runLength >= sectorsNeeded) break;
                } else {
                    runLength = 0;
                }
            }

            if (runLength >= sectorsNeeded) {
                // Use found space
                for (int32_t s = 0; s < sectorsNeeded; ++s) {
                    sectorFree_[static_cast<size_t>(startSector + s)] = false;
                }
                setOffset(x, z, (startSector << 8) | sectorsNeeded);
                writeChunkAtSector(startSector, compressed);
            } else {
                // Append to end of file
                file_.seekp(0, std::ios::end);
                startSector = static_cast<int32_t>(sectorFree_.size());
                for (int32_t s = 0; s < sectorsNeeded; ++s) {
                    // Write empty sector to extend file
                    std::vector<uint8_t> empty(SECTOR_SIZE, 0);
                    file_.write(reinterpret_cast<const char*>(empty.data()), SECTOR_SIZE);
                    sectorFree_.push_back(false);
                }
                setOffset(x, z, (startSector << 8) | sectorsNeeded);
                writeChunkAtSector(startSector, compressed);
            }
        }

        // Update timestamp
        auto now = static_cast<int32_t>(std::time(nullptr));
        setTimestamp(x, z, now);
        file_.flush();
        return true;
    }

    // Java: write(int sector, byte[] data, int length)
    void writeChunkAtSector(int32_t sector, const std::vector<uint8_t>& compressed) {
        file_.seekp(static_cast<int64_t>(sector) * SECTOR_SIZE);
        int32_t dataLen = static_cast<int32_t>(compressed.size());
        writeInt(dataLen + 1); // length + compression type byte
        uint8_t type = COMPRESSION_ZLIB;
        file_.write(reinterpret_cast<const char*>(&type), 1);
        file_.write(reinterpret_cast<const char*>(compressed.data()),
                    static_cast<std::streamsize>(compressed.size()));
    }

    // ─── Big-endian I/O ───

    int32_t readInt() {
        uint8_t buf[4];
        file_.read(reinterpret_cast<char*>(buf), 4);
        return (static_cast<int32_t>(buf[0]) << 24) |
               (static_cast<int32_t>(buf[1]) << 16) |
               (static_cast<int32_t>(buf[2]) << 8) |
               static_cast<int32_t>(buf[3]);
    }

    void writeInt(int32_t v) {
        uint8_t buf[4] = {
            static_cast<uint8_t>((v >> 24) & 0xFF),
            static_cast<uint8_t>((v >> 16) & 0xFF),
            static_cast<uint8_t>((v >> 8) & 0xFF),
            static_cast<uint8_t>(v & 0xFF)
        };
        file_.write(reinterpret_cast<const char*>(buf), 4);
    }

    // ─── Zlib helpers ───

    static std::vector<uint8_t> decompressZlib(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> result;
        result.resize(data.size() * 4); // Initial guess

        z_stream strm = {};
        if (inflateInit(&strm) != Z_OK) return {};

        strm.avail_in = static_cast<uInt>(data.size());
        strm.next_in = const_cast<Bytef*>(data.data());

        int ret;
        do {
            strm.avail_out = static_cast<uInt>(result.size() - strm.total_out);
            strm.next_out = result.data() + strm.total_out;
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_BUF_ERROR || (ret == Z_OK && strm.avail_out == 0)) {
                result.resize(result.size() * 2);
            } else if (ret != Z_OK && ret != Z_STREAM_END) {
                inflateEnd(&strm);
                return {};
            }
        } while (ret != Z_STREAM_END);

        result.resize(strm.total_out);
        inflateEnd(&strm);
        return result;
    }

    static std::vector<uint8_t> decompressGzip(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> result;
        result.resize(data.size() * 4);

        z_stream strm = {};
        // 15 + 16 = gzip decoding
        if (inflateInit2(&strm, 15 + 16) != Z_OK) return {};

        strm.avail_in = static_cast<uInt>(data.size());
        strm.next_in = const_cast<Bytef*>(data.data());

        int ret;
        do {
            strm.avail_out = static_cast<uInt>(result.size() - strm.total_out);
            strm.next_out = result.data() + strm.total_out;
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_BUF_ERROR || (ret == Z_OK && strm.avail_out == 0)) {
                result.resize(result.size() * 2);
            } else if (ret != Z_OK && ret != Z_STREAM_END) {
                inflateEnd(&strm);
                return {};
            }
        } while (ret != Z_STREAM_END);

        result.resize(strm.total_out);
        inflateEnd(&strm);
        return result;
    }

    static std::vector<uint8_t> compressZlib(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> result(data.size() + 128);
        z_stream strm = {};
        if (deflateInit(&strm, Z_DEFAULT_COMPRESSION) != Z_OK) return {};

        strm.avail_in = static_cast<uInt>(data.size());
        strm.next_in = const_cast<Bytef*>(data.data());
        strm.avail_out = static_cast<uInt>(result.size());
        strm.next_out = result.data();

        int ret = deflate(&strm, Z_FINISH);
        if (ret == Z_STREAM_END) {
            result.resize(strm.total_out);
        } else {
            // Buffer too small — retry with larger
            result.resize(data.size() * 2);
            deflateReset(&strm);
            strm.avail_in = static_cast<uInt>(data.size());
            strm.next_in = const_cast<Bytef*>(data.data());
            strm.avail_out = static_cast<uInt>(result.size());
            strm.next_out = result.data();
            deflate(&strm, Z_FINISH);
            result.resize(strm.total_out);
        }
        deflateEnd(&strm);
        return result;
    }

    // ─── State ───
    mutable std::mutex mutex_;
    std::fstream file_;
    std::string path_;
    bool isOpen_ = false;

    int32_t offsets_[1024] = {};       // Sector offset + count packed
    int32_t timestamps_[1024] = {};    // Unix timestamps
    std::vector<bool> sectorFree_;     // Free sector tracking
};

// ═══════════════════════════════════════════════════════════════════════════
// RegionFileCache — Cache of open region files.
// Java reference: net.minecraft.world.chunk.storage.RegionFileCache
// ═══════════════════════════════════════════════════════════════════════════

class RegionFileCache {
public:
    // Get or open region file for the given chunk coordinates
    RegionFile* getRegionFile(const std::string& worldDir, int32_t chunkX, int32_t chunkZ) {
        std::lock_guard<std::mutex> lock(mutex_);

        int32_t regionX = RegionFile::chunkToRegion(chunkX);
        int32_t regionZ = RegionFile::chunkToRegion(chunkZ);
        int64_t key = (static_cast<int64_t>(regionX) << 32) | (static_cast<uint32_t>(regionZ));

        auto it = cache_.find(key);
        if (it != cache_.end()) {
            return it->second.get();
        }

        // Open new region file
        std::string regionDir = worldDir + "/region";
        std::string path = regionDir + "/" + RegionFile::getFileName(regionX, regionZ);

        auto region = std::make_unique<RegionFile>();
        if (!region->open(path)) {
            return nullptr;
        }

        RegionFile* ptr = region.get();
        cache_[key] = std::move(region);
        return ptr;
    }

    // Read chunk NBT data from region file
    std::vector<uint8_t> readChunkData(const std::string& worldDir,
                                        int32_t chunkX, int32_t chunkZ) {
        RegionFile* region = getRegionFile(worldDir, chunkX, chunkZ);
        if (!region) return {};

        int32_t localX = RegionFile::chunkToLocal(chunkX);
        int32_t localZ = RegionFile::chunkToLocal(chunkZ);
        return region->readChunkData(localX, localZ);
    }

    // Write chunk NBT data to region file
    bool writeChunkData(const std::string& worldDir,
                         int32_t chunkX, int32_t chunkZ,
                         const std::vector<uint8_t>& data) {
        RegionFile* region = getRegionFile(worldDir, chunkX, chunkZ);
        if (!region) return false;

        int32_t localX = RegionFile::chunkToLocal(chunkX);
        int32_t localZ = RegionFile::chunkToLocal(chunkZ);
        return region->writeChunkData(localX, localZ, data);
    }

    // Close all cached region files
    void closeAll() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [key, region] : cache_) {
            region->close();
        }
        cache_.clear();
    }

private:
    std::mutex mutex_;
    std::unordered_map<int64_t, std::unique_ptr<RegionFile>> cache_;
};

} // namespace mccpp
