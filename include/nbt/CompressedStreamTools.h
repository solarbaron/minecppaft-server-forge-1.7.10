/**
 * CompressedStreamTools.h — GZIP-compressed NBT serialization/deserialization.
 *
 * Java reference: net.minecraft.nbt.CompressedStreamTools (130 lines)
 *
 * The standard Minecraft NBT file format:
 *
 * Binary format (written by writeTag):
 *   1. byte: tag type ID (0=END, 1=BYTE, ..., 10=COMPOUND, 11=INT_ARRAY)
 *   2. if typeId != 0: Modified UTF-8 string (tag name) — always "" for root
 *   3. tag payload (format depends on type)
 *
 * File format (level.dat, region data, player data):
 *   - GZIP compressed
 *   - Root must be a TAG_Compound (type 10)
 *   - Root name is always empty string ""
 *
 * Methods:
 *   readCompressed(InputStream)    → GZIP → read root compound
 *   writeCompressed(NBT, OutputStream) → GZIP → write root compound
 *   compress(NBTTagCompound)       → byte[] (GZIP compressed)
 *   decompress(byte[], tracker)    → NBTTagCompound
 *   read(DataInputStream)          → NBTTagCompound (uncompressed)
 *   write(NBTTagCompound, DataOutput) → write (uncompressed)
 *
 * NBT Size Tracker (Java: NBTSizeTracker):
 *   - Limits total bytes read to prevent NBT bombs
 *   - INFINITE tracker: Long.MAX_VALUE limit
 *   - Chunk packet: 2MB limit (2097152L)
 *
 * Dependencies: zlib for GZIP compression
 * Thread safety: Stateless codec — thread-safe.
 * JNI readiness: Simple serialization API.
 */
#pragma once

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// NBT Tag Type IDs
// Java: NBTBase.createNewByType(byte)
// ═══════════════════════════════════════════════════════════════════════════

namespace NBTTypeId {
    static constexpr uint8_t END        = 0;
    static constexpr uint8_t BYTE       = 1;
    static constexpr uint8_t SHORT      = 2;
    static constexpr uint8_t INT        = 3;
    static constexpr uint8_t LONG       = 4;
    static constexpr uint8_t FLOAT      = 5;
    static constexpr uint8_t DOUBLE     = 6;
    static constexpr uint8_t BYTE_ARRAY = 7;
    static constexpr uint8_t STRING     = 8;
    static constexpr uint8_t LIST       = 9;
    static constexpr uint8_t COMPOUND   = 10;
    static constexpr uint8_t INT_ARRAY  = 11;
    static constexpr uint8_t COUNT      = 12;  // Total number of types

    inline const char* getName(uint8_t id) {
        switch (id) {
            case END:        return "TAG_End";
            case BYTE:       return "TAG_Byte";
            case SHORT:      return "TAG_Short";
            case INT:        return "TAG_Int";
            case LONG:       return "TAG_Long";
            case FLOAT:      return "TAG_Float";
            case DOUBLE:     return "TAG_Double";
            case BYTE_ARRAY: return "TAG_Byte_Array";
            case STRING:     return "TAG_String";
            case LIST:       return "TAG_List";
            case COMPOUND:   return "TAG_Compound";
            case INT_ARRAY:  return "TAG_Int_Array";
            default:         return "UNKNOWN";
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// NBTSizeTracker — Limits total bytes read during NBT deserialization.
//
// Java reference: net.minecraft.nbt.NBTSizeTracker
//   INFINITE = new NBTSizeTracker(Long.MAX_VALUE)
//   Chunk packet tracker: new NBTSizeTracker(2097152L)
// ═══════════════════════════════════════════════════════════════════════════

class NBTSizeTracker {
public:
    int64_t maxBytes;
    int64_t bytesRead = 0;

    explicit NBTSizeTracker(int64_t max) : maxBytes(max) {}

    // Java: read(long bits) — tracks bits read (in bits, divide by 8 internally)
    bool read(int64_t bits) {
        bytesRead += bits / 8;
        return bytesRead <= maxBytes;
    }

    // ─── Predefined trackers ───
    // Java: NBTSizeTracker.INFINITE
    static NBTSizeTracker infinite() {
        return NBTSizeTracker(std::numeric_limits<int64_t>::max());
    }

    // Java: Chunk data NBT limit — 2MB
    static constexpr int64_t CHUNK_NBT_LIMIT = 2097152;

    static NBTSizeTracker chunkTracker() {
        return NBTSizeTracker(CHUNK_NBT_LIMIT);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// CompressedStreamTools — GZIP NBT codec.
//
// All serialization uses big-endian byte order (Java DataOutput standard).
// ═══════════════════════════════════════════════════════════════════════════

namespace CompressedStreamTools {

    // ─── Modified UTF-8 encoding ───
    // Java DataOutput.writeUTF(String):
    //   2 bytes: length (unsigned short)
    //   N bytes: modified UTF-8 data
    //
    // Modified UTF-8 differences from standard UTF-8:
    //   - '\0' is encoded as 0xC0 0x80 (2 bytes, not 1)
    //   - Supplementary characters use surrogate pairs in CESU-8
    static constexpr int32_t MAX_UTF_LENGTH = 65535;

    // ─── Tag header format ───
    // writeTag(NBTBase, DataOutput):
    //   1. writeByte(tag.getId())     — 1 byte
    //   2. writeUTF("")              — 2 bytes (length=0) + 0 data bytes
    //   3. tag.write(dataOutput)     — variable
    //
    // Root compound total overhead: 1 (type) + 2 (name length) = 3 bytes

    // ─── Byte order ───
    // All multi-byte values are big-endian (network byte order)
    // This matches Java's DataOutputStream behavior

    // ─── GZIP magic number ───
    // First two bytes of a GZIP stream
    static constexpr uint8_t GZIP_MAGIC_1 = 0x1F;
    static constexpr uint8_t GZIP_MAGIC_2 = 0x8B;

    // ─── Check if data is GZIP compressed ───
    inline bool isGzipCompressed(const uint8_t* data, size_t len) {
        return len >= 2 && data[0] == GZIP_MAGIC_1 && data[1] == GZIP_MAGIC_2;
    }

    // ─── Big-endian read helpers ───
    inline int16_t readShortBE(const uint8_t* data) {
        return static_cast<int16_t>((data[0] << 8) | data[1]);
    }

    inline int32_t readIntBE(const uint8_t* data) {
        return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    }

    inline int64_t readLongBE(const uint8_t* data) {
        return (static_cast<int64_t>(readIntBE(data)) << 32) |
               (static_cast<int64_t>(readIntBE(data + 4)) & 0xFFFFFFFFL);
    }

    inline float readFloatBE(const uint8_t* data) {
        int32_t i = readIntBE(data);
        float f;
        std::memcpy(&f, &i, 4);
        return f;
    }

    inline double readDoubleBE(const uint8_t* data) {
        int64_t l = readLongBE(data);
        double d;
        std::memcpy(&d, &l, 8);
        return d;
    }

    // ─── Big-endian write helpers ───
    inline void writeShortBE(uint8_t* data, int16_t val) {
        data[0] = static_cast<uint8_t>((val >> 8) & 0xFF);
        data[1] = static_cast<uint8_t>(val & 0xFF);
    }

    inline void writeIntBE(uint8_t* data, int32_t val) {
        data[0] = static_cast<uint8_t>((val >> 24) & 0xFF);
        data[1] = static_cast<uint8_t>((val >> 16) & 0xFF);
        data[2] = static_cast<uint8_t>((val >> 8) & 0xFF);
        data[3] = static_cast<uint8_t>(val & 0xFF);
    }

    inline void writeLongBE(uint8_t* data, int64_t val) {
        writeIntBE(data, static_cast<int32_t>(val >> 32));
        writeIntBE(data + 4, static_cast<int32_t>(val & 0xFFFFFFFFL));
    }

    inline void writeFloatBE(uint8_t* data, float val) {
        int32_t i;
        std::memcpy(&i, &val, 4);
        writeIntBE(data, i);
    }

    inline void writeDoubleBE(uint8_t* data, double val) {
        int64_t l;
        std::memcpy(&l, &val, 8);
        writeLongBE(data, l);
    }

    // ─── Modified UTF-8 string read ───
    // Java: DataInputStream.readUTF()
    //   Read 2-byte length prefix, then that many bytes of modified UTF-8
    inline std::string readUTF(const uint8_t* data, size_t& offset) {
        int16_t len = readShortBE(data + offset);
        offset += 2;
        if (len < 0) len = 0;
        std::string result(reinterpret_cast<const char*>(data + offset),
                          static_cast<size_t>(len));
        offset += static_cast<size_t>(len);
        return result;
    }

    // ─── Modified UTF-8 string write ───
    inline void writeUTF(std::vector<uint8_t>& buf, const std::string& str) {
        int16_t len = static_cast<int16_t>(str.size());
        buf.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        buf.push_back(static_cast<uint8_t>(len & 0xFF));
        buf.insert(buf.end(), str.begin(), str.end());
    }
}

} // namespace mccpp
