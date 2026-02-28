/**
 * NBT.h — Named Binary Tag implementation with byte-for-byte Java parity.
 *
 * Reference: net.minecraft.nbt.NBTBase and all subclasses.
 *
 * Wire format uses big-endian (network byte order) for all multi-byte values.
 * Strings use Java's modified UTF-8: 2-byte big-endian length + UTF-8 bytes.
 *
 * Tag IDs (from NBTBase.createNewByType):
 *   0  = TAG_End
 *   1  = TAG_Byte
 *   2  = TAG_Short
 *   3  = TAG_Int
 *   4  = TAG_Long
 *   5  = TAG_Float
 *   6  = TAG_Double
 *   7  = TAG_Byte_Array
 *   8  = TAG_String
 *   9  = TAG_List
 *   10 = TAG_Compound
 *   11 = TAG_Int_Array
 *
 * Thread safety: individual NBT tags are NOT thread-safe (same as Java).
 * The caller must synchronize if sharing NBT data across threads.
 */
#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {
namespace nbt {

// Forward declarations
class NBTTagCompound;
class NBTTagList;

/**
 * Tag type IDs — mirrors NBTBase.getId() values.
 */
enum class TagType : uint8_t {
    End       = 0,
    Byte      = 1,
    Short     = 2,
    Int       = 3,
    Long      = 4,
    Float     = 5,
    Double    = 6,
    ByteArray = 7,
    String    = 8,
    List      = 9,
    Compound  = 10,
    IntArray  = 11,
};

// Constant for "any numeric type" checks — matches Java's magic number 99
constexpr int TAG_ANY_NUMERIC = 99;

/**
 * DataOutput — big-endian binary writer (mirrors java.io.DataOutput).
 */
class DataOutput {
public:
    explicit DataOutput(std::vector<uint8_t>& buf) : buf_(buf) {}

    void writeByte(int8_t v)   { buf_.push_back(static_cast<uint8_t>(v)); }
    void writeShort(int16_t v) {
        buf_.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
        buf_.push_back(static_cast<uint8_t>(v & 0xFF));
    }
    void writeInt(int32_t v) {
        buf_.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
        buf_.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
        buf_.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
        buf_.push_back(static_cast<uint8_t>(v & 0xFF));
    }
    void writeLong(int64_t v) {
        for (int i = 56; i >= 0; i -= 8)
            buf_.push_back(static_cast<uint8_t>((v >> i) & 0xFF));
    }
    void writeFloat(float v) {
        uint32_t bits;
        std::memcpy(&bits, &v, sizeof(bits));
        writeInt(static_cast<int32_t>(bits));
    }
    void writeDouble(double v) {
        uint64_t bits;
        std::memcpy(&bits, &v, sizeof(bits));
        writeLong(static_cast<int64_t>(bits));
    }
    /**
     * Java's DataOutput.writeUTF(): 2-byte big-endian length + modified UTF-8 bytes.
     */
    void writeUTF(const std::string& s) {
        writeShort(static_cast<int16_t>(s.size()));
        buf_.insert(buf_.end(), s.begin(), s.end());
    }
    void write(const uint8_t* data, size_t len) {
        buf_.insert(buf_.end(), data, data + len);
    }

private:
    std::vector<uint8_t>& buf_;
};

/**
 * DataInput — big-endian binary reader (mirrors java.io.DataInput).
 */
class DataInput {
public:
    DataInput(const uint8_t* data, size_t length)
        : data_(data), length_(length), pos_(0) {}

    int8_t readByte() {
        checkAvailable(1);
        return static_cast<int8_t>(data_[pos_++]);
    }
    int16_t readShort() {
        checkAvailable(2);
        int16_t v = static_cast<int16_t>((data_[pos_] << 8) | data_[pos_ + 1]);
        pos_ += 2;
        return v;
    }
    int32_t readInt() {
        checkAvailable(4);
        int32_t v = (static_cast<int32_t>(data_[pos_]) << 24) |
                    (static_cast<int32_t>(data_[pos_ + 1]) << 16) |
                    (static_cast<int32_t>(data_[pos_ + 2]) << 8) |
                    static_cast<int32_t>(data_[pos_ + 3]);
        pos_ += 4;
        return v;
    }
    int64_t readLong() {
        checkAvailable(8);
        int64_t v = 0;
        for (int i = 0; i < 8; ++i)
            v = (v << 8) | data_[pos_ + i];
        pos_ += 8;
        return v;
    }
    float readFloat() {
        int32_t bits = readInt();
        float v;
        std::memcpy(&v, &bits, sizeof(v));
        return v;
    }
    double readDouble() {
        int64_t bits = readLong();
        double v;
        std::memcpy(&v, &bits, sizeof(v));
        return v;
    }
    std::string readUTF() {
        uint16_t len = static_cast<uint16_t>(readShort());
        checkAvailable(len);
        std::string s(reinterpret_cast<const char*>(data_ + pos_), len);
        pos_ += len;
        return s;
    }
    void readFully(uint8_t* buf, size_t len) {
        checkAvailable(len);
        std::memcpy(buf, data_ + pos_, len);
        pos_ += len;
    }

    size_t remaining() const { return length_ - pos_; }
    size_t position() const { return pos_; }

private:
    void checkAvailable(size_t n) const {
        if (pos_ + n > length_)
            throw std::runtime_error("NBT DataInput: unexpected end of data");
    }
    const uint8_t* data_;
    size_t length_;
    size_t pos_;
};

/**
 * NBTBase — abstract base for all NBT tags.
 * Java reference: net.minecraft.nbt.NBTBase
 */
class NBTBase {
public:
    virtual ~NBTBase() = default;

    virtual TagType getId() const = 0;
    virtual void write(DataOutput& out) const = 0;
    virtual void read(DataInput& in, int depth) = 0;
    virtual std::unique_ptr<NBTBase> copy() const = 0;
    virtual std::string toString() const = 0;

    /**
     * Factory method — mirrors NBTBase.createNewByType(byte).
     */
    static std::unique_ptr<NBTBase> createNewByType(TagType type);

    static const char* getTypeName(TagType type);
};

// ─── Primitive Tags ─────────────────────────────────────────────────────────

/**
 * TAG_Byte (id=1). Java reference: net.minecraft.nbt.NBTTagByte
 */
class NBTTagByte : public NBTBase {
public:
    NBTTagByte() : data_(0) {}
    explicit NBTTagByte(int8_t v) : data_(v) {}

    TagType getId() const override { return TagType::Byte; }
    void write(DataOutput& out) const override { out.writeByte(data_); }
    void read(DataInput& in, int) override { data_ = in.readByte(); }
    std::unique_ptr<NBTBase> copy() const override { return std::make_unique<NBTTagByte>(data_); }
    std::string toString() const override { return std::to_string(data_) + "b"; }

    int8_t  getByte()   const { return data_; }
    int16_t getShort()  const { return data_; }
    int32_t getInt()    const { return data_; }
    int64_t getLong()   const { return data_; }
    float   getFloat()  const { return static_cast<float>(data_); }
    double  getDouble() const { return static_cast<double>(data_); }

private:
    int8_t data_;
};

/**
 * TAG_Short (id=2). Java reference: net.minecraft.nbt.NBTTagShort
 */
class NBTTagShort : public NBTBase {
public:
    NBTTagShort() : data_(0) {}
    explicit NBTTagShort(int16_t v) : data_(v) {}

    TagType getId() const override { return TagType::Short; }
    void write(DataOutput& out) const override { out.writeShort(data_); }
    void read(DataInput& in, int) override { data_ = in.readShort(); }
    std::unique_ptr<NBTBase> copy() const override { return std::make_unique<NBTTagShort>(data_); }
    std::string toString() const override { return std::to_string(data_) + "s"; }

    int8_t  getByte()   const { return static_cast<int8_t>(data_ & 0xFF); }
    int16_t getShort()  const { return data_; }
    int32_t getInt()    const { return data_; }
    int64_t getLong()   const { return data_; }
    float   getFloat()  const { return static_cast<float>(data_); }
    double  getDouble() const { return static_cast<double>(data_); }

private:
    int16_t data_;
};

/**
 * TAG_Int (id=3). Java reference: net.minecraft.nbt.NBTTagInt
 */
class NBTTagInt : public NBTBase {
public:
    NBTTagInt() : data_(0) {}
    explicit NBTTagInt(int32_t v) : data_(v) {}

    TagType getId() const override { return TagType::Int; }
    void write(DataOutput& out) const override { out.writeInt(data_); }
    void read(DataInput& in, int) override { data_ = in.readInt(); }
    std::unique_ptr<NBTBase> copy() const override { return std::make_unique<NBTTagInt>(data_); }
    std::string toString() const override { return std::to_string(data_); }

    // Java reference: NBTTagInt — exact casting behavior
    int8_t  getByte()   const { return static_cast<int8_t>(data_ & 0xFF); }
    int16_t getShort()  const { return static_cast<int16_t>(data_ & 0xFFFF); }
    int32_t getInt()    const { return data_; }
    int64_t getLong()   const { return data_; }
    float   getFloat()  const { return static_cast<float>(data_); }
    double  getDouble() const { return static_cast<double>(data_); }

private:
    int32_t data_;
};

/**
 * TAG_Long (id=4). Java reference: net.minecraft.nbt.NBTTagLong
 */
class NBTTagLong : public NBTBase {
public:
    NBTTagLong() : data_(0) {}
    explicit NBTTagLong(int64_t v) : data_(v) {}

    TagType getId() const override { return TagType::Long; }
    void write(DataOutput& out) const override { out.writeLong(data_); }
    void read(DataInput& in, int) override { data_ = in.readLong(); }
    std::unique_ptr<NBTBase> copy() const override { return std::make_unique<NBTTagLong>(data_); }
    std::string toString() const override { return std::to_string(data_) + "L"; }

    int8_t  getByte()   const { return static_cast<int8_t>(data_ & 0xFF); }
    int16_t getShort()  const { return static_cast<int16_t>(data_ & 0xFFFF); }
    int32_t getInt()    const { return static_cast<int32_t>(data_); }
    int64_t getLong()   const { return data_; }
    float   getFloat()  const { return static_cast<float>(data_); }
    double  getDouble() const { return static_cast<double>(data_); }

private:
    int64_t data_;
};

/**
 * TAG_Float (id=5). Java reference: net.minecraft.nbt.NBTTagFloat
 */
class NBTTagFloat : public NBTBase {
public:
    NBTTagFloat() : data_(0.0f) {}
    explicit NBTTagFloat(float v) : data_(v) {}

    TagType getId() const override { return TagType::Float; }
    void write(DataOutput& out) const override { out.writeFloat(data_); }
    void read(DataInput& in, int) override { data_ = in.readFloat(); }
    std::unique_ptr<NBTBase> copy() const override { return std::make_unique<NBTTagFloat>(data_); }
    std::string toString() const override { return std::to_string(data_) + "f"; }

    int8_t  getByte()   const { return static_cast<int8_t>(static_cast<int32_t>(data_) & 0xFF); }
    int16_t getShort()  const { return static_cast<int16_t>(static_cast<int32_t>(data_) & 0xFFFF); }
    int32_t getInt()    const { return static_cast<int32_t>(data_); }
    int64_t getLong()   const { return static_cast<int64_t>(data_); }
    float   getFloat()  const { return data_; }
    double  getDouble() const { return static_cast<double>(data_); }

private:
    float data_;
};

/**
 * TAG_Double (id=6). Java reference: net.minecraft.nbt.NBTTagDouble
 */
class NBTTagDouble : public NBTBase {
public:
    NBTTagDouble() : data_(0.0) {}
    explicit NBTTagDouble(double v) : data_(v) {}

    TagType getId() const override { return TagType::Double; }
    void write(DataOutput& out) const override { out.writeDouble(data_); }
    void read(DataInput& in, int) override { data_ = in.readDouble(); }
    std::unique_ptr<NBTBase> copy() const override { return std::make_unique<NBTTagDouble>(data_); }
    std::string toString() const override { return std::to_string(data_) + "d"; }

    int8_t  getByte()   const { return static_cast<int8_t>(static_cast<int64_t>(data_) & 0xFF); }
    int16_t getShort()  const { return static_cast<int16_t>(static_cast<int64_t>(data_) & 0xFFFF); }
    int32_t getInt()    const { return static_cast<int32_t>(data_); }
    int64_t getLong()   const { return static_cast<int64_t>(data_); }
    float   getFloat()  const { return static_cast<float>(data_); }
    double  getDouble() const { return data_; }

private:
    double data_;
};

// ─── Array / String Tags ────────────────────────────────────────────────────

/**
 * TAG_Byte_Array (id=7). Java reference: net.minecraft.nbt.NBTTagByteArray
 */
class NBTTagByteArray : public NBTBase {
public:
    NBTTagByteArray() = default;
    explicit NBTTagByteArray(std::vector<int8_t> data) : data_(std::move(data)) {}

    TagType getId() const override { return TagType::ByteArray; }
    void write(DataOutput& out) const override {
        out.writeInt(static_cast<int32_t>(data_.size()));
        out.write(reinterpret_cast<const uint8_t*>(data_.data()), data_.size());
    }
    void read(DataInput& in, int) override {
        int32_t len = in.readInt();
        data_.resize(static_cast<size_t>(len));
        in.readFully(reinterpret_cast<uint8_t*>(data_.data()), static_cast<size_t>(len));
    }
    std::unique_ptr<NBTBase> copy() const override { return std::make_unique<NBTTagByteArray>(data_); }
    std::string toString() const override { return "[" + std::to_string(data_.size()) + " bytes]"; }

    const std::vector<int8_t>& getByteArray() const { return data_; }
    std::vector<int8_t>& getByteArray() { return data_; }

private:
    std::vector<int8_t> data_;
};

/**
 * TAG_Int_Array (id=11). Java reference: net.minecraft.nbt.NBTTagIntArray
 */
class NBTTagIntArray : public NBTBase {
public:
    NBTTagIntArray() = default;
    explicit NBTTagIntArray(std::vector<int32_t> data) : data_(std::move(data)) {}

    TagType getId() const override { return TagType::IntArray; }
    void write(DataOutput& out) const override {
        out.writeInt(static_cast<int32_t>(data_.size()));
        for (int32_t v : data_) out.writeInt(v);
    }
    void read(DataInput& in, int) override {
        int32_t len = in.readInt();
        data_.resize(static_cast<size_t>(len));
        for (int32_t i = 0; i < len; ++i) data_[i] = in.readInt();
    }
    std::unique_ptr<NBTBase> copy() const override { return std::make_unique<NBTTagIntArray>(data_); }
    std::string toString() const override { return "[" + std::to_string(data_.size()) + " ints]"; }

    const std::vector<int32_t>& getIntArray() const { return data_; }
    std::vector<int32_t>& getIntArray() { return data_; }

private:
    std::vector<int32_t> data_;
};

/**
 * TAG_String (id=8). Java reference: net.minecraft.nbt.NBTTagString
 */
class NBTTagString : public NBTBase {
public:
    NBTTagString() = default;
    explicit NBTTagString(std::string data) : data_(std::move(data)) {}

    TagType getId() const override { return TagType::String; }
    void write(DataOutput& out) const override { out.writeUTF(data_); }
    void read(DataInput& in, int) override { data_ = in.readUTF(); }
    std::unique_ptr<NBTBase> copy() const override { return std::make_unique<NBTTagString>(data_); }
    std::string toString() const override { return "\"" + data_ + "\""; }

    const std::string& getString() const { return data_; }

private:
    std::string data_;
};

/**
 * TAG_End (id=0). Java reference: net.minecraft.nbt.NBTTagEnd
 * Sentinel tag — marks end of a Compound tag. Never stored in containers.
 */
class NBTTagEnd : public NBTBase {
public:
    TagType getId() const override { return TagType::End; }
    void write(DataOutput&) const override {}
    void read(DataInput&, int) override {}
    std::unique_ptr<NBTBase> copy() const override { return std::make_unique<NBTTagEnd>(); }
    std::string toString() const override { return "END"; }
};

// ─── Container Tags ─────────────────────────────────────────────────────────

/**
 * TAG_List (id=9). Java reference: net.minecraft.nbt.NBTTagList
 *
 * A typed list: all elements must have the same tag type.
 * Wire format: byte tagType, int32 count, then count tags.
 */
class NBTTagList : public NBTBase {
public:
    NBTTagList() = default;

    TagType getId() const override { return TagType::List; }

    void write(DataOutput& out) const override {
        // Java: tagType = !tagList.isEmpty() ? tagList.get(0).getId() : 0
        TagType type = tags_.empty() ? TagType::End
                     : tags_[0]->getId();
        out.writeByte(static_cast<int8_t>(type));
        out.writeInt(static_cast<int32_t>(tags_.size()));
        for (auto& tag : tags_) tag->write(out);
    }

    void read(DataInput& in, int depth) override {
        // Java: depth > 512 throws RuntimeException
        if (depth > 512) throw std::runtime_error("NBT tag list too deeply nested (>512)");
        tagType_ = static_cast<TagType>(in.readByte());
        int32_t count = in.readInt();
        tags_.clear();
        tags_.reserve(static_cast<size_t>(count));
        for (int32_t i = 0; i < count; ++i) {
            auto tag = NBTBase::createNewByType(tagType_);
            if (!tag) throw std::runtime_error("Unknown NBT tag type in list");
            tag->read(in, depth + 1);
            tags_.push_back(std::move(tag));
        }
    }

    std::unique_ptr<NBTBase> copy() const override {
        auto list = std::make_unique<NBTTagList>();
        list->tagType_ = tagType_;
        for (auto& tag : tags_) list->tags_.push_back(tag->copy());
        return list;
    }

    std::string toString() const override {
        std::string s = "[";
        for (size_t i = 0; i < tags_.size(); ++i) {
            s += std::to_string(i) + ':' + tags_[i]->toString() + ',';
        }
        return s + "]";
    }

    // --- Accessors matching Java API ---

    void appendTag(std::unique_ptr<NBTBase> tag) {
        // Java: if tagType==0 set it; if mismatch, warn and return
        if (tagType_ == TagType::End) {
            tagType_ = tag->getId();
        } else if (tagType_ != tag->getId()) {
            return; // mismatching types
        }
        tags_.push_back(std::move(tag));
    }

    int32_t tagCount() const { return static_cast<int32_t>(tags_.size()); }
    TagType getTagType() const { return tagType_; }

    NBTBase* getTag(int32_t index) const {
        if (index < 0 || index >= static_cast<int32_t>(tags_.size())) return nullptr;
        return tags_[index].get();
    }

    NBTTagCompound* getCompoundTagAt(int32_t index) const;
    double getDoubleAt(int32_t index) const;
    float getFloatAt(int32_t index) const;
    const std::vector<int32_t>& getIntArrayAt(int32_t index) const;
    std::string getStringTagAt(int32_t index) const;

private:
    TagType tagType_ = TagType::End;
    std::vector<std::unique_ptr<NBTBase>> tags_;
};

/**
 * TAG_Compound (id=10). Java reference: net.minecraft.nbt.NBTTagCompound
 *
 * An unordered map of named tags.
 * Wire format: repeat [byte tagId][UTF-string key][tag data], terminated by TAG_End (0x00).
 */
class NBTTagCompound : public NBTBase {
public:
    NBTTagCompound() = default;

    TagType getId() const override { return TagType::Compound; }

    void write(DataOutput& out) const override {
        // Java: for each entry: writeByte(tag.getId()), writeUTF(key), tag.write()
        for (auto& [key, tag] : tagMap_) {
            out.writeByte(static_cast<int8_t>(tag->getId()));
            out.writeUTF(key);
            tag->write(out);
        }
        out.writeByte(0); // TAG_End
    }

    void read(DataInput& in, int depth) override {
        if (depth > 512) throw std::runtime_error("NBT compound too deeply nested (>512)");
        tagMap_.clear();
        while (true) {
            TagType type = static_cast<TagType>(in.readByte());
            if (type == TagType::End) break;
            std::string key = in.readUTF();
            auto tag = NBTBase::createNewByType(type);
            if (!tag) throw std::runtime_error("Unknown NBT tag type: " + std::to_string(static_cast<int>(type)));
            tag->read(in, depth + 1);
            tagMap_[key] = std::move(tag);
        }
    }

    std::unique_ptr<NBTBase> copy() const override {
        auto compound = std::make_unique<NBTTagCompound>();
        for (auto& [key, tag] : tagMap_) {
            compound->tagMap_[key] = tag->copy();
        }
        return compound;
    }

    std::string toString() const override {
        std::string s = "{";
        for (auto& [key, tag] : tagMap_) {
            s += key + ':' + tag->toString() + ',';
        }
        return s + "}";
    }

    // --- Setters (matching Java NBTTagCompound API) ---

    void setTag(const std::string& key, std::unique_ptr<NBTBase> tag) {
        tagMap_[key] = std::move(tag);
    }
    void setByte(const std::string& key, int8_t v) {
        tagMap_[key] = std::make_unique<NBTTagByte>(v);
    }
    void setShort(const std::string& key, int16_t v) {
        tagMap_[key] = std::make_unique<NBTTagShort>(v);
    }
    void setInteger(const std::string& key, int32_t v) {
        tagMap_[key] = std::make_unique<NBTTagInt>(v);
    }
    void setLong(const std::string& key, int64_t v) {
        tagMap_[key] = std::make_unique<NBTTagLong>(v);
    }
    void setFloat(const std::string& key, float v) {
        tagMap_[key] = std::make_unique<NBTTagFloat>(v);
    }
    void setDouble(const std::string& key, double v) {
        tagMap_[key] = std::make_unique<NBTTagDouble>(v);
    }
    void setString(const std::string& key, const std::string& v) {
        tagMap_[key] = std::make_unique<NBTTagString>(v);
    }
    void setByteArray(const std::string& key, std::vector<int8_t> v) {
        tagMap_[key] = std::make_unique<NBTTagByteArray>(std::move(v));
    }
    void setIntArray(const std::string& key, std::vector<int32_t> v) {
        tagMap_[key] = std::make_unique<NBTTagIntArray>(std::move(v));
    }
    void setBoolean(const std::string& key, bool v) {
        setByte(key, v ? 1 : 0);
    }

    // --- Getters (matching Java NBTTagCompound API exactly) ---

    bool hasKey(const std::string& key) const {
        return tagMap_.find(key) != tagMap_.end();
    }

    /**
     * Java: hasKey(String, int) — checks tag type.
     * Special: type 99 matches any numeric type (1-6).
     */
    bool hasKey(const std::string& key, int type) const {
        auto it = tagMap_.find(key);
        if (it == tagMap_.end()) return false;
        int tagId = static_cast<int>(it->second->getId());
        if (tagId == type) return true;
        if (type == TAG_ANY_NUMERIC) {
            return tagId >= 1 && tagId <= 6;
        }
        return false;
    }

    TagType getTagId(const std::string& key) const {
        auto it = tagMap_.find(key);
        return (it != tagMap_.end()) ? it->second->getId() : TagType::End;
    }

    NBTBase* getTag(const std::string& key) const {
        auto it = tagMap_.find(key);
        return (it != tagMap_.end()) ? it->second.get() : nullptr;
    }

    int8_t getByte(const std::string& key) const {
        auto it = tagMap_.find(key);
        if (it == tagMap_.end()) return 0;
        auto* tag = it->second.get();
        switch (tag->getId()) {
            case TagType::Byte:   return static_cast<NBTTagByte*>(tag)->getByte();
            case TagType::Short:  return static_cast<NBTTagShort*>(tag)->getByte();
            case TagType::Int:    return static_cast<NBTTagInt*>(tag)->getByte();
            case TagType::Long:   return static_cast<NBTTagLong*>(tag)->getByte();
            case TagType::Float:  return static_cast<NBTTagFloat*>(tag)->getByte();
            case TagType::Double: return static_cast<NBTTagDouble*>(tag)->getByte();
            default: return 0;
        }
    }

    int16_t getShort(const std::string& key) const {
        auto it = tagMap_.find(key);
        if (it == tagMap_.end()) return 0;
        auto* tag = it->second.get();
        switch (tag->getId()) {
            case TagType::Byte:   return static_cast<NBTTagByte*>(tag)->getShort();
            case TagType::Short:  return static_cast<NBTTagShort*>(tag)->getShort();
            case TagType::Int:    return static_cast<NBTTagInt*>(tag)->getShort();
            case TagType::Long:   return static_cast<NBTTagLong*>(tag)->getShort();
            case TagType::Float:  return static_cast<NBTTagFloat*>(tag)->getShort();
            case TagType::Double: return static_cast<NBTTagDouble*>(tag)->getShort();
            default: return 0;
        }
    }

    int32_t getInteger(const std::string& key) const {
        auto it = tagMap_.find(key);
        if (it == tagMap_.end()) return 0;
        auto* tag = it->second.get();
        switch (tag->getId()) {
            case TagType::Byte:   return static_cast<NBTTagByte*>(tag)->getInt();
            case TagType::Short:  return static_cast<NBTTagShort*>(tag)->getInt();
            case TagType::Int:    return static_cast<NBTTagInt*>(tag)->getInt();
            case TagType::Long:   return static_cast<NBTTagLong*>(tag)->getInt();
            case TagType::Float:  return static_cast<NBTTagFloat*>(tag)->getInt();
            case TagType::Double: return static_cast<NBTTagDouble*>(tag)->getInt();
            default: return 0;
        }
    }

    int64_t getLong(const std::string& key) const {
        auto it = tagMap_.find(key);
        if (it == tagMap_.end()) return 0;
        auto* tag = it->second.get();
        switch (tag->getId()) {
            case TagType::Byte:   return static_cast<NBTTagByte*>(tag)->getLong();
            case TagType::Short:  return static_cast<NBTTagShort*>(tag)->getLong();
            case TagType::Int:    return static_cast<NBTTagInt*>(tag)->getLong();
            case TagType::Long:   return static_cast<NBTTagLong*>(tag)->getLong();
            case TagType::Float:  return static_cast<NBTTagFloat*>(tag)->getLong();
            case TagType::Double: return static_cast<NBTTagDouble*>(tag)->getLong();
            default: return 0;
        }
    }

    float getFloat(const std::string& key) const {
        auto it = tagMap_.find(key);
        if (it == tagMap_.end()) return 0.0f;
        auto* tag = it->second.get();
        switch (tag->getId()) {
            case TagType::Byte:   return static_cast<NBTTagByte*>(tag)->getFloat();
            case TagType::Short:  return static_cast<NBTTagShort*>(tag)->getFloat();
            case TagType::Int:    return static_cast<NBTTagInt*>(tag)->getFloat();
            case TagType::Long:   return static_cast<NBTTagLong*>(tag)->getFloat();
            case TagType::Float:  return static_cast<NBTTagFloat*>(tag)->getFloat();
            case TagType::Double: return static_cast<NBTTagDouble*>(tag)->getFloat();
            default: return 0.0f;
        }
    }

    double getDouble(const std::string& key) const {
        auto it = tagMap_.find(key);
        if (it == tagMap_.end()) return 0.0;
        auto* tag = it->second.get();
        switch (tag->getId()) {
            case TagType::Byte:   return static_cast<NBTTagByte*>(tag)->getDouble();
            case TagType::Short:  return static_cast<NBTTagShort*>(tag)->getDouble();
            case TagType::Int:    return static_cast<NBTTagInt*>(tag)->getDouble();
            case TagType::Long:   return static_cast<NBTTagLong*>(tag)->getDouble();
            case TagType::Float:  return static_cast<NBTTagFloat*>(tag)->getDouble();
            case TagType::Double: return static_cast<NBTTagDouble*>(tag)->getDouble();
            default: return 0.0;
        }
    }

    std::string getString(const std::string& key) const {
        auto it = tagMap_.find(key);
        if (it == tagMap_.end()) return "";
        if (auto* s = dynamic_cast<NBTTagString*>(it->second.get())) return s->getString();
        return it->second->toString();
    }

    const std::vector<int8_t>& getByteArray(const std::string& key) const {
        static const std::vector<int8_t> empty;
        auto it = tagMap_.find(key);
        if (it == tagMap_.end()) return empty;
        if (auto* a = dynamic_cast<NBTTagByteArray*>(it->second.get())) return a->getByteArray();
        return empty;
    }

    const std::vector<int32_t>& getIntArray(const std::string& key) const {
        static const std::vector<int32_t> empty;
        auto it = tagMap_.find(key);
        if (it == tagMap_.end()) return empty;
        if (auto* a = dynamic_cast<NBTTagIntArray*>(it->second.get())) return a->getIntArray();
        return empty;
    }

    NBTTagCompound* getCompoundTag(const std::string& key) const {
        auto it = tagMap_.find(key);
        if (it == tagMap_.end()) return nullptr;
        return dynamic_cast<NBTTagCompound*>(it->second.get());
    }

    NBTTagList* getTagList(const std::string& key, int expectedType) const {
        auto it = tagMap_.find(key);
        if (it == tagMap_.end()) return nullptr;
        if (it->second->getId() != TagType::List) return nullptr;
        auto* list = static_cast<NBTTagList*>(it->second.get());
        if (list->tagCount() > 0 && static_cast<int>(list->getTagType()) != expectedType) return nullptr;
        return list;
    }

    bool getBoolean(const std::string& key) const {
        return getByte(key) != 0;
    }

    void removeTag(const std::string& key) {
        tagMap_.erase(key);
    }

    bool hasNoTags() const {
        return tagMap_.empty();
    }

    const std::unordered_map<std::string, std::unique_ptr<NBTBase>>& getTagMap() const {
        return tagMap_;
    }

private:
    std::unordered_map<std::string, std::unique_ptr<NBTBase>> tagMap_;
};

// ─── Top-level read/write ───────────────────────────────────────────────────

/**
 * Write a named root compound tag (the standard NBT file format).
 * Wire format: byte TAG_Compound (10), UTF-string name, compound data.
 *
 * Java reference: net.minecraft.nbt.CompressedStreamTools.write()
 */
inline void writeNamedTag(DataOutput& out, const std::string& name, const NBTTagCompound& root) {
    out.writeByte(static_cast<int8_t>(TagType::Compound));
    out.writeUTF(name);
    root.write(out);
}

/**
 * Read a named root compound tag.
 * Returns the root tag and its name.
 *
 * Java reference: net.minecraft.nbt.CompressedStreamTools.read()
 */
inline std::pair<std::unique_ptr<NBTTagCompound>, std::string> readNamedTag(DataInput& in) {
    TagType type = static_cast<TagType>(in.readByte());
    if (type != TagType::Compound)
        throw std::runtime_error("Root NBT tag must be Compound, got " + std::to_string(static_cast<int>(type)));
    std::string name = in.readUTF();
    auto compound = std::make_unique<NBTTagCompound>();
    compound->read(in, 0);
    return {std::move(compound), name};
}

/**
 * Serialize NBT to a byte vector (uncompressed).
 */
inline std::vector<uint8_t> serializeNBT(const NBTTagCompound& root, const std::string& name = "") {
    std::vector<uint8_t> buf;
    DataOutput out(buf);
    writeNamedTag(out, name, root);
    return buf;
}

/**
 * Deserialize NBT from raw bytes (uncompressed).
 */
inline std::unique_ptr<NBTTagCompound> deserializeNBT(const uint8_t* data, size_t length) {
    DataInput in(data, length);
    auto result = readNamedTag(in);
    return std::move(result.first);
}

} // namespace nbt
} // namespace mccpp
