/**
 * VarInt.h — Minecraft protocol VarInt/VarLong encoding & decoding.
 *
 * Reference: Protocol wiki §Data types; Java reference: PacketBuffer (obfuscated)
 * https://web.archive.org/web/20241129034727/https://wiki.vg/index.php?title=Protocol&oldid=7368
 *
 * Header-only. All functions are constexpr-friendly where possible.
 */
#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace mccpp {

/**
 * Result of a VarInt/VarLong decode: the decoded value and how many bytes
 * were consumed from the buffer.
 */
template <typename T>
struct VarResult {
    T value;
    int bytesRead;
};

// ─── VarInt (up to 5 bytes, encodes int32_t) ────────────────────────────────

/**
 * Encode a 32-bit integer as a VarInt and append the bytes to `out`.
 */
inline void writeVarInt(std::vector<uint8_t>& out, int32_t value) {
    auto uval = static_cast<uint32_t>(value);
    do {
        uint8_t byte = uval & 0x7F;
        uval >>= 7;
        if (uval != 0) {
            byte |= 0x80;
        }
        out.push_back(byte);
    } while (uval != 0);
}

/**
 * Encode a VarInt into a fixed buffer. Returns the number of bytes written.
 * Buffer must be at least 5 bytes.
 */
inline int writeVarInt(uint8_t* buf, int32_t value) {
    auto uval = static_cast<uint32_t>(value);
    int written = 0;
    do {
        uint8_t byte = uval & 0x7F;
        uval >>= 7;
        if (uval != 0) {
            byte |= 0x80;
        }
        buf[written++] = byte;
    } while (uval != 0);
    return written;
}

/**
 * Decode a VarInt from a byte buffer.
 * Throws std::runtime_error if the VarInt is too long (>5 bytes).
 */
inline VarResult<int32_t> readVarInt(const uint8_t* data, size_t length) {
    int32_t result = 0;
    int shift = 0;
    int bytesRead = 0;

    for (;;) {
        if (static_cast<size_t>(bytesRead) >= length) {
            throw std::runtime_error("VarInt: unexpected end of buffer");
        }
        uint8_t byte = data[bytesRead++];
        result |= static_cast<int32_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) {
            break;
        }
        shift += 7;
        if (shift >= 35) {
            throw std::runtime_error("VarInt: too many bytes (>5)");
        }
    }

    return {result, bytesRead};
}

/**
 * Get the encoded byte length of a VarInt value.
 */
inline int varIntSize(int32_t value) {
    auto uval = static_cast<uint32_t>(value);
    int size = 0;
    do {
        uval >>= 7;
        ++size;
    } while (uval != 0);
    return size;
}

// ─── VarLong (up to 10 bytes, encodes int64_t) ─────────────────────────────

/**
 * Encode a 64-bit integer as a VarLong and append the bytes to `out`.
 */
inline void writeVarLong(std::vector<uint8_t>& out, int64_t value) {
    auto uval = static_cast<uint64_t>(value);
    do {
        uint8_t byte = uval & 0x7F;
        uval >>= 7;
        if (uval != 0) {
            byte |= 0x80;
        }
        out.push_back(byte);
    } while (uval != 0);
}

/**
 * Decode a VarLong from a byte buffer.
 * Throws std::runtime_error if the VarLong is too long (>10 bytes).
 */
inline VarResult<int64_t> readVarLong(const uint8_t* data, size_t length) {
    int64_t result = 0;
    int shift = 0;
    int bytesRead = 0;

    for (;;) {
        if (static_cast<size_t>(bytesRead) >= length) {
            throw std::runtime_error("VarLong: unexpected end of buffer");
        }
        uint8_t byte = data[bytesRead++];
        result |= static_cast<int64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) {
            break;
        }
        shift += 7;
        if (shift >= 70) {
            throw std::runtime_error("VarLong: too many bytes (>10)");
        }
    }

    return {result, bytesRead};
}

// ─── Protocol string helpers ───────────────────────────────────────────────

/**
 * Encode a UTF-8 protocol string: VarInt length prefix + UTF-8 bytes.
 */
inline void writeString(std::vector<uint8_t>& out, const std::string& str) {
    writeVarInt(out, static_cast<int32_t>(str.size()));
    out.insert(out.end(), str.begin(), str.end());
}

/**
 * Decode a UTF-8 protocol string from a buffer.
 */
inline VarResult<std::string> readString(const uint8_t* data, size_t length) {
    auto lenResult = readVarInt(data, length);
    int32_t strLen = lenResult.value;
    int totalRead = lenResult.bytesRead;

    if (strLen < 0 || static_cast<size_t>(totalRead + strLen) > length) {
        throw std::runtime_error("String: length exceeds buffer");
    }

    std::string str(reinterpret_cast<const char*>(data + totalRead), static_cast<size_t>(strLen));
    totalRead += strLen;

    return {std::move(str), totalRead};
}

} // namespace mccpp
