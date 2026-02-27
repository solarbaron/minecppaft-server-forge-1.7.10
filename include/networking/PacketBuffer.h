#pragma once
// PacketBuffer — ported from et.java
// Wraps a byte vector with read/write cursor and Minecraft-specific types.

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <stdexcept>
#include "networking/VarInt.h"

namespace mc {

class PacketBuffer {
public:
    PacketBuffer() = default;

    explicit PacketBuffer(std::vector<uint8_t> data)
        : data_(std::move(data)) {}

    // --- Write operations ---

    void writeByte(uint8_t val) {
        data_.push_back(val);
    }

    void writeBoolean(bool val) {
        writeByte(val ? 1 : 0);
    }

    // Big-endian short — Java's writeShort
    void writeShort(int16_t val) {
        data_.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
        data_.push_back(static_cast<uint8_t>(val & 0xFF));
    }

    // Big-endian int — Java's writeInt
    void writeInt(int32_t val) {
        data_.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
        data_.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
        data_.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
        data_.push_back(static_cast<uint8_t>(val & 0xFF));
    }

    // Big-endian long — Java's writeLong
    void writeLong(int64_t val) {
        for (int i = 7; i >= 0; --i) {
            data_.push_back(static_cast<uint8_t>((val >> (i * 8)) & 0xFF));
        }
    }

    void writeFloat(float val) {
        int32_t bits;
        std::memcpy(&bits, &val, 4);
        writeInt(bits);
    }

    void writeDouble(double val) {
        int64_t bits;
        std::memcpy(&bits, &val, 8);
        writeLong(bits);
    }

    // et.b(int) — writeVarInt
    void writeVarInt(int32_t val) {
        VarInt::write(data_, val);
    }

    // et.a(String) — writeString: VarInt length prefix + UTF-8 bytes
    void writeString(const std::string& str) {
        if (str.size() > 32767) {
            throw std::runtime_error("String too big (was " + std::to_string(str.size()) +
                                     " bytes encoded, max 32767)");
        }
        writeVarInt(static_cast<int32_t>(str.size()));
        data_.insert(data_.end(), str.begin(), str.end());
    }

    void writeBytes(const uint8_t* src, size_t len) {
        data_.insert(data_.end(), src, src + len);
    }

    void writeBytes(const std::vector<uint8_t>& src) {
        data_.insert(data_.end(), src.begin(), src.end());
    }

    // --- Read operations ---

    uint8_t readByte() {
        ensureReadable(1);
        return data_[readPos_++];
    }

    bool readBoolean() {
        return readByte() != 0;
    }

    int16_t readShort() {
        ensureReadable(2);
        int16_t val = static_cast<int16_t>(
            (static_cast<uint16_t>(data_[readPos_]) << 8) |
            static_cast<uint16_t>(data_[readPos_ + 1])
        );
        readPos_ += 2;
        return val;
    }

    uint16_t readUnsignedShort() {
        ensureReadable(2);
        uint16_t val = (static_cast<uint16_t>(data_[readPos_]) << 8) |
                       static_cast<uint16_t>(data_[readPos_ + 1]);
        readPos_ += 2;
        return val;
    }

    int32_t readInt() {
        ensureReadable(4);
        int32_t val = (static_cast<int32_t>(data_[readPos_]) << 24) |
                      (static_cast<int32_t>(data_[readPos_ + 1]) << 16) |
                      (static_cast<int32_t>(data_[readPos_ + 2]) << 8) |
                      static_cast<int32_t>(data_[readPos_ + 3]);
        readPos_ += 4;
        return val;
    }

    int64_t readLong() {
        ensureReadable(8);
        int64_t val = 0;
        for (int i = 0; i < 8; ++i) {
            val = (val << 8) | static_cast<int64_t>(data_[readPos_++]);
        }
        return val;
    }

    float readFloat() {
        int32_t bits = readInt();
        float val;
        std::memcpy(&val, &bits, 4);
        return val;
    }

    double readDouble() {
        int64_t bits = readLong();
        double val;
        std::memcpy(&val, &bits, 8);
        return val;
    }

    // et.a() — readVarInt
    int32_t readVarInt() {
        return VarInt::read(data_.data(), data_.size(), readPos_);
    }

    // et.c(int) — readString with max length
    std::string readString(int maxLength = 32767) {
        int32_t len = readVarInt();
        if (len > maxLength * 4) {
            throw std::runtime_error("Encoded string too long (" + std::to_string(len) +
                                     " > " + std::to_string(maxLength * 4) + ")");
        }
        if (len < 0) {
            throw std::runtime_error("Negative string length");
        }
        ensureReadable(static_cast<size_t>(len));
        std::string str(reinterpret_cast<const char*>(data_.data() + readPos_),
                        static_cast<size_t>(len));
        readPos_ += static_cast<size_t>(len);
        if (static_cast<int>(str.size()) > maxLength) {
            throw std::runtime_error("String too long (" + std::to_string(str.size()) +
                                     " > " + std::to_string(maxLength) + ")");
        }
        return str;
    }

    std::vector<uint8_t> readBytes(size_t count) {
        ensureReadable(count);
        std::vector<uint8_t> out(data_.begin() + readPos_,
                                  data_.begin() + readPos_ + count);
        readPos_ += count;
        return out;
    }

    // --- Accessors ---

    size_t readableBytes() const {
        return data_.size() - readPos_;
    }

    size_t readerIndex() const { return readPos_; }
    void setReaderIndex(size_t idx) { readPos_ = idx; }

    const std::vector<uint8_t>& data() const { return data_; }
    std::vector<uint8_t>& data() { return data_; }

    void clear() {
        data_.clear();
        readPos_ = 0;
    }

private:
    void ensureReadable(size_t count) const {
        if (readPos_ + count > data_.size()) {
            throw std::runtime_error("PacketBuffer underflow: need " +
                std::to_string(count) + " bytes, have " +
                std::to_string(data_.size() - readPos_));
        }
    }

    std::vector<uint8_t> data_;
    size_t readPos_ = 0;
};

} // namespace mc
