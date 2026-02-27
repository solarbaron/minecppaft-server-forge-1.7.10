#pragma once
// VarInt encoding/decoding — ported from et.java (PacketBuffer)
// Minecraft protocol 47 uses VarInt for packet length and packet ID framing.

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <string>

namespace mc {

class VarInt {
public:
    // et.a(int) — compute encoded size of a VarInt
    static int size(int32_t value) {
        if ((value & 0xFFFFFF80) == 0) return 1;
        if ((value & 0xFFFFC000) == 0) return 2;
        if ((value & 0xFFE00000) == 0) return 3;
        if ((value & 0xF0000000) == 0) return 4;
        return 5;
    }

    // et.a() — read a VarInt from a byte buffer at offset, advances offset
    static int32_t read(const uint8_t* data, size_t length, size_t& offset) {
        int32_t result = 0;
        int shift = 0;
        uint8_t byte;
        do {
            if (offset >= length) {
                throw std::runtime_error("VarInt: unexpected end of buffer");
            }
            byte = data[offset++];
            result |= static_cast<int32_t>(byte & 0x7F) << (shift * 7);
            ++shift;
            if (shift > 5) {
                throw std::runtime_error("VarInt too big");
            }
        } while ((byte & 0x80) == 0x80);
        return result;
    }

    // et.b(int) — write a VarInt into a byte buffer
    static void write(std::vector<uint8_t>& buf, int32_t value) {
        uint32_t uval = static_cast<uint32_t>(value);
        while (true) {
            if ((uval & 0xFFFFFF80) == 0) {
                buf.push_back(static_cast<uint8_t>(uval));
                return;
            }
            buf.push_back(static_cast<uint8_t>((uval & 0x7F) | 0x80));
            uval >>= 7;
        }
    }
};

} // namespace mc
