#pragma once
// PlayerDataIO — save/load player data as gzip-compressed NBT files.
// Ported from nt.java (SaveHandler) and du.java (CompressedStreamTools).
// Format: worlddir/playerdata/<uuid>.dat (gzip-compressed NBT compound)

#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include <zlib.h>

#include "nbt/NBT.h"
#include "entity/Player.h"

namespace mc {

namespace fs = std::filesystem;

class PlayerDataIO {
public:
    explicit PlayerDataIO(const std::string& worldDir = "world")
        : worldDir_(worldDir) {
        // Create playerdata directory if it doesn't exist
        fs::create_directories(fs::path(worldDir_) / "playerdata");
    }

    // Save player data to worlddir/playerdata/<uuid>.dat
    // Matches nt.java savePlayerData() -> du.a(dh, File) (gzip write)
    bool savePlayer(const Player& player) {
        auto nbt = player.saveToNBT();
        if (!nbt) return false;

        // Serialize NBT to bytes
        auto rawData = nbt::serializeNBT(*nbt);

        // Gzip compress — matches du.a(dh, OutputStream) using GZIPOutputStream
        auto compressed = gzipCompress(rawData);
        if (compressed.empty()) {
            std::cerr << "[SAVE] Failed to gzip compress data for " << player.name << "\n";
            return false;
        }

        // Write to file: first to .dat_new, then rename (atomic swap)
        // This matches Java's write-to-tmp-then-rename pattern
        std::string uuid = sanitizeUuid(player.uuid);
        fs::path datFile = fs::path(worldDir_) / "playerdata" / (uuid + ".dat");
        fs::path tmpFile = fs::path(worldDir_) / "playerdata" / (uuid + ".dat_new");

        // Write compressed data
        std::ofstream ofs(tmpFile, std::ios::binary);
        if (!ofs) {
            std::cerr << "[SAVE] Failed to open " << tmpFile << "\n";
            return false;
        }
        ofs.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());
        ofs.close();

        // Rename tmp -> final (atomic on most filesystems)
        std::error_code ec;
        fs::rename(tmpFile, datFile, ec);
        if (ec) {
            std::cerr << "[SAVE] Failed to rename " << tmpFile << " -> " << datFile << "\n";
            return false;
        }

        std::cout << "[SAVE] Saved " << player.name << " (" << uuid << ")\n";
        return true;
    }

    // Load player data from worlddir/playerdata/<uuid>.dat
    // Matches nt.java loadPlayerData() -> du.a(File) (gzip read)
    bool loadPlayer(Player& player) {
        std::string uuid = sanitizeUuid(player.uuid);
        fs::path datFile = fs::path(worldDir_) / "playerdata" / (uuid + ".dat");

        if (!fs::exists(datFile)) {
            return false; // New player, no data to load
        }

        // Read file
        std::ifstream ifs(datFile, std::ios::binary);
        if (!ifs) {
            std::cerr << "[LOAD] Failed to open " << datFile << "\n";
            return false;
        }

        std::vector<uint8_t> compressed(
            (std::istreambuf_iterator<char>(ifs)),
            std::istreambuf_iterator<char>());
        ifs.close();

        // Gzip decompress
        auto rawData = gzipDecompress(compressed);
        if (rawData.empty()) {
            std::cerr << "[LOAD] Failed to gzip decompress " << datFile << "\n";
            return false;
        }

        // Deserialize NBT
        try {
            auto nbt = nbt::deserializeNBT(rawData);
            player.loadFromNBT(*nbt);
            std::cout << "[LOAD] Loaded " << player.name << " (" << uuid << ")\n";
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[LOAD] NBT parse error for " << datFile << ": " << e.what() << "\n";
            return false;
        }
    }

    // Check if player data exists
    bool hasPlayerData(const std::string& uuid) const {
        fs::path datFile = fs::path(worldDir_) / "playerdata" / (sanitizeUuid(uuid) + ".dat");
        return fs::exists(datFile);
    }

private:
    std::string worldDir_;

    // Remove hyphens from UUID for filename
    static std::string sanitizeUuid(const std::string& uuid) {
        std::string clean;
        for (char c : uuid) {
            if (c != '-') clean += c;
        }
        return clean;
    }

    // Gzip compress — matches Java's GZIPOutputStream
    static std::vector<uint8_t> gzipCompress(const std::vector<uint8_t>& data) {
        z_stream strm{};
        // windowBits = 15 + 16 for gzip format
        if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                         15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            return {};
        }

        strm.avail_in = static_cast<uInt>(data.size());
        strm.next_in = const_cast<Bytef*>(data.data());

        std::vector<uint8_t> result;
        uint8_t buf[16384];

        do {
            strm.avail_out = sizeof(buf);
            strm.next_out = buf;
            deflate(&strm, Z_FINISH);
            size_t have = sizeof(buf) - strm.avail_out;
            result.insert(result.end(), buf, buf + have);
        } while (strm.avail_out == 0);

        deflateEnd(&strm);
        return result;
    }

    // Gzip decompress — matches Java's GZIPInputStream
    static std::vector<uint8_t> gzipDecompress(const std::vector<uint8_t>& data) {
        z_stream strm{};
        // windowBits = 15 + 16 for gzip auto-detect
        if (inflateInit2(&strm, 15 + 16) != Z_OK) {
            return {};
        }

        strm.avail_in = static_cast<uInt>(data.size());
        strm.next_in = const_cast<Bytef*>(data.data());

        std::vector<uint8_t> result;
        uint8_t buf[16384];

        int ret;
        do {
            strm.avail_out = sizeof(buf);
            strm.next_out = buf;
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                inflateEnd(&strm);
                return {};
            }
            size_t have = sizeof(buf) - strm.avail_out;
            result.insert(result.end(), buf, buf + have);
        } while (ret != Z_STREAM_END);

        inflateEnd(&strm);
        return result;
    }
};

} // namespace mc
