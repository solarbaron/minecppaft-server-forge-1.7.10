#pragma once
// NBT (Named Binary Tag) — full implementation ported from:
//   dy.java  (NBTBase)
//   dl.java  (NBTTagEnd, type 0)
//   dg.java  (NBTTagByte, type 1)
//   dw.java  (NBTTagShort, type 2)
//   dp.java  (NBTTagInt, type 3)
//   dr.java  (NBTTagLong, type 4)
//   dm.java  (NBTTagFloat, type 5)
//   dk.java  (NBTTagDouble, type 6)
//   df.java  (NBTTagByteArray, type 7)
//   dx.java  (NBTTagString, type 8)
//   dq.java  (NBTTagList, type 9)
//   dh.java  (NBTTagCompound, type 10)
//   dn.java  (NBTTagIntArray, type 11)
//   du.java  (CompressedStreamTools) — top-level read/write
//
// Java's DataOutput uses big-endian; we replicate that exactly.
// Java's writeUTF uses modified UTF-8 with 2-byte length prefix.

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <cassert>

namespace mc {
namespace nbt {

// Forward declarations
class NBTBase;
class NBTTagCompound;
class NBTTagList;

using NBTPtr = std::shared_ptr<NBTBase>;

// ============================================================
// Tag type IDs — dy.a[] in dy.java
// ============================================================
enum TagType : uint8_t {
    TAG_End       = 0,
    TAG_Byte      = 1,
    TAG_Short     = 2,
    TAG_Int       = 3,
    TAG_Long      = 4,
    TAG_Float     = 5,
    TAG_Double    = 6,
    TAG_ByteArray = 7,
    TAG_String    = 8,
    TAG_List      = 9,
    TAG_Compound  = 10,
    TAG_IntArray  = 11
};

// ============================================================
// DataStream — helper for big-endian binary I/O
// Mirrors Java's DataInputStream/DataOutputStream behavior.
// ============================================================
class DataStream {
public:
    DataStream() = default;
    explicit DataStream(const std::vector<uint8_t>& data) : data_(data) {}
    explicit DataStream(std::vector<uint8_t>&& data) : data_(std::move(data)) {}

    // --- Read (DataInput) ---
    uint8_t readByte() {
        ensure(1);
        return data_[pos_++];
    }

    int8_t readSignedByte() {
        return static_cast<int8_t>(readByte());
    }

    int16_t readShort() {
        ensure(2);
        int16_t val = static_cast<int16_t>(
            (static_cast<uint16_t>(data_[pos_]) << 8) |
            static_cast<uint16_t>(data_[pos_ + 1]));
        pos_ += 2;
        return val;
    }

    int32_t readInt() {
        ensure(4);
        int32_t val = (static_cast<int32_t>(data_[pos_]) << 24) |
                      (static_cast<int32_t>(data_[pos_ + 1]) << 16) |
                      (static_cast<int32_t>(data_[pos_ + 2]) << 8) |
                      static_cast<int32_t>(data_[pos_ + 3]);
        pos_ += 4;
        return val;
    }

    int64_t readLong() {
        ensure(8);
        int64_t val = 0;
        for (int i = 0; i < 8; ++i)
            val = (val << 8) | data_[pos_++];
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

    // Java's readUTF: 2-byte big-endian length prefix + modified UTF-8
    std::string readUTF() {
        uint16_t len = static_cast<uint16_t>(readShort());
        ensure(len);
        std::string str(reinterpret_cast<const char*>(data_.data() + pos_), len);
        pos_ += len;
        return str;
    }

    void readBytes(uint8_t* dst, size_t count) {
        ensure(count);
        std::memcpy(dst, data_.data() + pos_, count);
        pos_ += count;
    }

    // --- Write (DataOutput) ---
    void writeByte(uint8_t val) {
        data_.push_back(val);
    }

    void writeShort(int16_t val) {
        data_.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
        data_.push_back(static_cast<uint8_t>(val & 0xFF));
    }

    void writeInt(int32_t val) {
        data_.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
        data_.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
        data_.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
        data_.push_back(static_cast<uint8_t>(val & 0xFF));
    }

    void writeLong(int64_t val) {
        for (int i = 7; i >= 0; --i)
            data_.push_back(static_cast<uint8_t>((val >> (i * 8)) & 0xFF));
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

    // Java's writeUTF: 2-byte big-endian length prefix + bytes
    void writeUTF(const std::string& str) {
        writeShort(static_cast<int16_t>(str.size()));
        data_.insert(data_.end(), str.begin(), str.end());
    }

    void writeBytes(const uint8_t* src, size_t count) {
        data_.insert(data_.end(), src, src + count);
    }

    size_t remaining() const { return data_.size() - pos_; }
    size_t position() const { return pos_; }
    const std::vector<uint8_t>& buffer() const { return data_; }
    std::vector<uint8_t>& buffer() { return data_; }

private:
    void ensure(size_t count) const {
        if (pos_ + count > data_.size())
            throw std::runtime_error("NBT DataStream underflow");
    }

    std::vector<uint8_t> data_;
    size_t pos_ = 0;
};

// ============================================================
// NBTBase — dy.java
// ============================================================
class NBTBase {
public:
    virtual ~NBTBase() = default;
    virtual uint8_t getId() const = 0; // dy.a()
    virtual void write(DataStream& out) const = 0; // dy.a(DataOutput)
    virtual void read(DataStream& in, int depth) = 0; // dy.a(DataInput, int, ds)
    virtual NBTPtr clone() const = 0; // dy.b()
    virtual std::string toString() const = 0;

    // dy.a(byte) — factory
    static NBTPtr createTag(uint8_t typeId);
};

// ============================================================
// NBTTagEnd — dl.java (type 0)
// ============================================================
class NBTTagEnd : public NBTBase {
public:
    uint8_t getId() const override { return TAG_End; }
    void write(DataStream&) const override {}
    void read(DataStream&, int) override {}
    NBTPtr clone() const override { return std::make_shared<NBTTagEnd>(); }
    std::string toString() const override { return "END"; }
};

// ============================================================
// NBTTagByte — dg.java (type 1)
// ============================================================
class NBTTagByte : public NBTBase {
public:
    int8_t value = 0;
    NBTTagByte() = default;
    explicit NBTTagByte(int8_t v) : value(v) {}

    uint8_t getId() const override { return TAG_Byte; }

    void write(DataStream& out) const override {
        out.writeByte(static_cast<uint8_t>(value));
    }

    void read(DataStream& in, int) override {
        value = in.readSignedByte();
    }

    NBTPtr clone() const override { return std::make_shared<NBTTagByte>(value); }
    std::string toString() const override { return std::to_string(value) + "b"; }
};

// ============================================================
// NBTTagShort — dw.java (type 2)
// ============================================================
class NBTTagShort : public NBTBase {
public:
    int16_t value = 0;
    NBTTagShort() = default;
    explicit NBTTagShort(int16_t v) : value(v) {}

    uint8_t getId() const override { return TAG_Short; }

    void write(DataStream& out) const override {
        out.writeShort(value);
    }

    void read(DataStream& in, int) override {
        value = in.readShort();
    }

    NBTPtr clone() const override { return std::make_shared<NBTTagShort>(value); }
    std::string toString() const override { return std::to_string(value) + "s"; }
};

// ============================================================
// NBTTagInt — dp.java (type 3)
// ============================================================
class NBTTagInt : public NBTBase {
public:
    int32_t value = 0;
    NBTTagInt() = default;
    explicit NBTTagInt(int32_t v) : value(v) {}

    uint8_t getId() const override { return TAG_Int; }

    void write(DataStream& out) const override {
        out.writeInt(value);
    }

    void read(DataStream& in, int) override {
        value = in.readInt();
    }

    NBTPtr clone() const override { return std::make_shared<NBTTagInt>(value); }
    std::string toString() const override { return std::to_string(value); }
};

// ============================================================
// NBTTagLong — dr.java (type 4)
// ============================================================
class NBTTagLong : public NBTBase {
public:
    int64_t value = 0;
    NBTTagLong() = default;
    explicit NBTTagLong(int64_t v) : value(v) {}

    uint8_t getId() const override { return TAG_Long; }

    void write(DataStream& out) const override {
        out.writeLong(value);
    }

    void read(DataStream& in, int) override {
        value = in.readLong();
    }

    NBTPtr clone() const override { return std::make_shared<NBTTagLong>(value); }
    std::string toString() const override { return std::to_string(value) + "L"; }
};

// ============================================================
// NBTTagFloat — dm.java (type 5)
// ============================================================
class NBTTagFloat : public NBTBase {
public:
    float value = 0.0f;
    NBTTagFloat() = default;
    explicit NBTTagFloat(float v) : value(v) {}

    uint8_t getId() const override { return TAG_Float; }

    void write(DataStream& out) const override {
        out.writeFloat(value);
    }

    void read(DataStream& in, int) override {
        value = in.readFloat();
    }

    NBTPtr clone() const override { return std::make_shared<NBTTagFloat>(value); }
    std::string toString() const override { return std::to_string(value) + "f"; }
};

// ============================================================
// NBTTagDouble — dk.java (type 6)
// ============================================================
class NBTTagDouble : public NBTBase {
public:
    double value = 0.0;
    NBTTagDouble() = default;
    explicit NBTTagDouble(double v) : value(v) {}

    uint8_t getId() const override { return TAG_Double; }

    void write(DataStream& out) const override {
        out.writeDouble(value);
    }

    void read(DataStream& in, int) override {
        value = in.readDouble();
    }

    NBTPtr clone() const override { return std::make_shared<NBTTagDouble>(value); }
    std::string toString() const override { return std::to_string(value) + "d"; }
};

// ============================================================
// NBTTagByteArray — df.java (type 7)
// ============================================================
class NBTTagByteArray : public NBTBase {
public:
    std::vector<int8_t> value;
    NBTTagByteArray() = default;
    explicit NBTTagByteArray(std::vector<int8_t> v) : value(std::move(v)) {}

    uint8_t getId() const override { return TAG_ByteArray; }

    // df.a(DataOutput): writeInt(length), write(byte[])
    void write(DataStream& out) const override {
        out.writeInt(static_cast<int32_t>(value.size()));
        out.writeBytes(reinterpret_cast<const uint8_t*>(value.data()), value.size());
    }

    void read(DataStream& in, int) override {
        int32_t len = in.readInt();
        value.resize(len);
        in.readBytes(reinterpret_cast<uint8_t*>(value.data()), len);
    }

    NBTPtr clone() const override { return std::make_shared<NBTTagByteArray>(value); }
    std::string toString() const override {
        return "[" + std::to_string(value.size()) + " bytes]";
    }
};

// ============================================================
// NBTTagString — dx.java (type 8)
// ============================================================
class NBTTagString : public NBTBase {
public:
    std::string value;
    NBTTagString() = default;
    explicit NBTTagString(std::string v) : value(std::move(v)) {}

    uint8_t getId() const override { return TAG_String; }

    // dx.a(DataOutput): writeUTF(value)
    void write(DataStream& out) const override {
        out.writeUTF(value);
    }

    void read(DataStream& in, int) override {
        value = in.readUTF();
    }

    NBTPtr clone() const override { return std::make_shared<NBTTagString>(value); }
    std::string toString() const override { return "\"" + value + "\""; }
};

// ============================================================
// NBTTagIntArray — dn.java (type 11)
// ============================================================
class NBTTagIntArray : public NBTBase {
public:
    std::vector<int32_t> value;
    NBTTagIntArray() = default;
    explicit NBTTagIntArray(std::vector<int32_t> v) : value(std::move(v)) {}

    uint8_t getId() const override { return TAG_IntArray; }

    // dn.a(DataOutput): writeInt(length), for each: writeInt(element)
    void write(DataStream& out) const override {
        out.writeInt(static_cast<int32_t>(value.size()));
        for (int32_t v : value) {
            out.writeInt(v);
        }
    }

    void read(DataStream& in, int) override {
        int32_t len = in.readInt();
        value.resize(len);
        for (int32_t i = 0; i < len; ++i) {
            value[i] = in.readInt();
        }
    }

    NBTPtr clone() const override { return std::make_shared<NBTTagIntArray>(value); }
    std::string toString() const override {
        return "[" + std::to_string(value.size()) + " ints]";
    }
};

// ============================================================
// NBTTagList — dq.java (type 9)
// ============================================================
class NBTTagList : public NBTBase {
public:
    uint8_t elementType = TAG_End;
    std::vector<NBTPtr> tags;

    NBTTagList() = default;

    uint8_t getId() const override { return TAG_List; }

    // dq.a(DataOutput): writeByte(elementType), writeInt(count), each tag.write()
    void write(DataStream& out) const override {
        uint8_t et = tags.empty() ? TAG_End : tags[0]->getId();
        out.writeByte(et);
        out.writeInt(static_cast<int32_t>(tags.size()));
        for (auto& tag : tags) {
            tag->write(out);
        }
    }

    // dq.a(DataInput, int, ds): readByte, readInt, then read each element
    void read(DataStream& in, int depth) override {
        if (depth > 512) throw std::runtime_error("NBT tag too complex (depth > 512)");
        elementType = in.readByte();
        int32_t count = in.readInt();
        tags.clear();
        tags.reserve(count);
        for (int32_t i = 0; i < count; ++i) {
            auto tag = NBTBase::createTag(elementType);
            tag->read(in, depth + 1);
            tags.push_back(std::move(tag));
        }
    }

    // dq.a(dy) — add tag with type checking
    void add(NBTPtr tag) {
        if (elementType == TAG_End) {
            elementType = tag->getId();
        }
        tags.push_back(std::move(tag));
    }

    int size() const { return static_cast<int>(tags.size()); }

    NBTPtr clone() const override {
        auto copy = std::make_shared<NBTTagList>();
        copy->elementType = elementType;
        for (auto& tag : tags) {
            copy->tags.push_back(tag->clone());
        }
        return copy;
    }

    std::string toString() const override {
        std::string s = "[";
        for (size_t i = 0; i < tags.size(); ++i) {
            if (i > 0) s += ",";
            s += tags[i]->toString();
        }
        return s + "]";
    }
};

// ============================================================
// NBTTagCompound — dh.java (type 10)
// ============================================================
class NBTTagCompound : public NBTBase {
public:
    // Using ordered map to maintain consistent write order
    std::map<std::string, NBTPtr> tags;

    NBTTagCompound() = default;

    uint8_t getId() const override { return TAG_Compound; }

    // dh.a(DataOutput): for each entry: writeByte(type), writeUTF(name), tag.write()
    // Then writeByte(0) as END marker
    void write(DataStream& out) const override {
        for (auto& [name, tag] : tags) {
            out.writeByte(tag->getId());
            out.writeUTF(name);
            tag->write(out);
        }
        out.writeByte(TAG_End);
    }

    // dh.a(DataInput, int, ds): read type, name, value until type==0
    void read(DataStream& in, int depth) override {
        if (depth > 512) throw std::runtime_error("NBT tag too complex (depth > 512)");
        tags.clear();
        uint8_t typeId;
        while ((typeId = in.readByte()) != TAG_End) {
            std::string name = in.readUTF();
            auto tag = NBTBase::createTag(typeId);
            tag->read(in, depth + 1);
            tags[name] = std::move(tag);
        }
    }

    // --- Convenience setters (mirror dh.a(String, type)) ---
    void setByte(const std::string& key, int8_t val) {
        tags[key] = std::make_shared<NBTTagByte>(val);
    }
    void setShort(const std::string& key, int16_t val) {
        tags[key] = std::make_shared<NBTTagShort>(val);
    }
    void setInt(const std::string& key, int32_t val) {
        tags[key] = std::make_shared<NBTTagInt>(val);
    }
    void setLong(const std::string& key, int64_t val) {
        tags[key] = std::make_shared<NBTTagLong>(val);
    }
    void setFloat(const std::string& key, float val) {
        tags[key] = std::make_shared<NBTTagFloat>(val);
    }
    void setDouble(const std::string& key, double val) {
        tags[key] = std::make_shared<NBTTagDouble>(val);
    }
    void setString(const std::string& key, const std::string& val) {
        tags[key] = std::make_shared<NBTTagString>(val);
    }
    void setByteArray(const std::string& key, std::vector<int8_t> val) {
        tags[key] = std::make_shared<NBTTagByteArray>(std::move(val));
    }
    void setIntArray(const std::string& key, std::vector<int32_t> val) {
        tags[key] = std::make_shared<NBTTagIntArray>(std::move(val));
    }
    void setBoolean(const std::string& key, bool val) {
        setByte(key, val ? 1 : 0);
    }
    void setTag(const std::string& key, NBTPtr tag) {
        tags[key] = std::move(tag);
    }
    void setCompound(const std::string& key, std::shared_ptr<NBTTagCompound> comp) {
        tags[key] = std::move(comp);
    }
    void setList(const std::string& key, std::shared_ptr<NBTTagList> list) {
        tags[key] = std::move(list);
    }

    // --- Convenience getters (mirror dh.d/e/f/g/h/i/j/k/l/m/n) ---
    bool hasKey(const std::string& key) const {
        return tags.count(key) > 0;
    }

    uint8_t getTagType(const std::string& key) const {
        auto it = tags.find(key);
        return it != tags.end() ? it->second->getId() : TAG_End;
    }

    // dh.b(String, int) — check if key exists with given type (99 = any number)
    bool hasKey(const std::string& key, int typeId) const {
        uint8_t actual = getTagType(key);
        if (actual == typeId) return true;
        if (typeId == 99) {
            return actual >= TAG_Byte && actual <= TAG_Double;
        }
        return false;
    }

    int8_t getByte(const std::string& key) const {
        auto it = tags.find(key);
        if (it == tags.end()) return 0;
        auto* p = dynamic_cast<NBTTagByte*>(it->second.get());
        return p ? p->value : 0;
    }

    int16_t getShort(const std::string& key) const {
        auto it = tags.find(key);
        if (it == tags.end()) return 0;
        auto* p = dynamic_cast<NBTTagShort*>(it->second.get());
        return p ? p->value : 0;
    }

    int32_t getInt(const std::string& key) const {
        auto it = tags.find(key);
        if (it == tags.end()) return 0;
        auto* p = dynamic_cast<NBTTagInt*>(it->second.get());
        return p ? p->value : 0;
    }

    int64_t getLong(const std::string& key) const {
        auto it = tags.find(key);
        if (it == tags.end()) return 0;
        auto* p = dynamic_cast<NBTTagLong*>(it->second.get());
        return p ? p->value : 0;
    }

    float getFloat(const std::string& key) const {
        auto it = tags.find(key);
        if (it == tags.end()) return 0.0f;
        auto* p = dynamic_cast<NBTTagFloat*>(it->second.get());
        return p ? p->value : 0.0f;
    }

    double getDouble(const std::string& key) const {
        auto it = tags.find(key);
        if (it == tags.end()) return 0.0;
        auto* p = dynamic_cast<NBTTagDouble*>(it->second.get());
        return p ? p->value : 0.0;
    }

    std::string getString(const std::string& key) const {
        auto it = tags.find(key);
        if (it == tags.end()) return "";
        auto* p = dynamic_cast<NBTTagString*>(it->second.get());
        return p ? p->value : "";
    }

    std::vector<int8_t> getByteArray(const std::string& key) const {
        auto it = tags.find(key);
        if (it == tags.end()) return {};
        auto* p = dynamic_cast<NBTTagByteArray*>(it->second.get());
        return p ? p->value : std::vector<int8_t>{};
    }

    std::vector<int32_t> getIntArray(const std::string& key) const {
        auto it = tags.find(key);
        if (it == tags.end()) return {};
        auto* p = dynamic_cast<NBTTagIntArray*>(it->second.get());
        return p ? p->value : std::vector<int32_t>{};
    }

    bool getBoolean(const std::string& key) const {
        return getByte(key) != 0;
    }

    std::shared_ptr<NBTTagCompound> getCompound(const std::string& key) const {
        auto it = tags.find(key);
        if (it == tags.end()) return std::make_shared<NBTTagCompound>();
        auto p = std::dynamic_pointer_cast<NBTTagCompound>(it->second);
        return p ? p : std::make_shared<NBTTagCompound>();
    }

    std::shared_ptr<NBTTagList> getList(const std::string& key, uint8_t expectedType) const {
        auto it = tags.find(key);
        if (it == tags.end()) return std::make_shared<NBTTagList>();
        auto p = std::dynamic_pointer_cast<NBTTagList>(it->second);
        if (!p) return std::make_shared<NBTTagList>();
        if (p->size() > 0 && p->elementType != expectedType) {
            return std::make_shared<NBTTagList>();
        }
        return p;
    }

    bool isEmpty() const { return tags.empty(); }
    void remove(const std::string& key) { tags.erase(key); }

    NBTPtr clone() const override {
        auto copy = std::make_shared<NBTTagCompound>();
        for (auto& [k, v] : tags) {
            copy->tags[k] = v->clone();
        }
        return copy;
    }

    std::string toString() const override {
        std::string s = "{";
        bool first = true;
        for (auto& [k, v] : tags) {
            if (!first) s += ",";
            s += k + ":" + v->toString();
            first = false;
        }
        return s + "}";
    }
};

// ============================================================
// NBTBase::createTag — dy.a(byte) factory
// ============================================================
inline NBTPtr NBTBase::createTag(uint8_t typeId) {
    switch (typeId) {
        case TAG_End:       return std::make_shared<NBTTagEnd>();
        case TAG_Byte:      return std::make_shared<NBTTagByte>();
        case TAG_Short:     return std::make_shared<NBTTagShort>();
        case TAG_Int:       return std::make_shared<NBTTagInt>();
        case TAG_Long:      return std::make_shared<NBTTagLong>();
        case TAG_Float:     return std::make_shared<NBTTagFloat>();
        case TAG_Double:    return std::make_shared<NBTTagDouble>();
        case TAG_ByteArray: return std::make_shared<NBTTagByteArray>();
        case TAG_String:    return std::make_shared<NBTTagString>();
        case TAG_List:      return std::make_shared<NBTTagList>();
        case TAG_Compound:  return std::make_shared<NBTTagCompound>();
        case TAG_IntArray:  return std::make_shared<NBTTagIntArray>();
        default:
            throw std::runtime_error("Unknown NBT tag type: " + std::to_string(typeId));
    }
}

// ============================================================
// Top-level read/write — du.java (CompressedStreamTools)
// ============================================================

// du.a(dy, DataOutput): writeByte(type), writeUTF(""), tag.write()
inline void writeRootTag(const NBTTagCompound& root, DataStream& out) {
    out.writeByte(root.getId()); // 10
    out.writeUTF("");            // empty root name
    root.write(out);
}

// du.a(DataInput, int, ds): readByte(type), readUTF(name), tag.read()
inline std::shared_ptr<NBTTagCompound> readRootTag(DataStream& in) {
    uint8_t typeId = in.readByte();
    if (typeId == TAG_End) {
        return std::make_shared<NBTTagCompound>();
    }
    if (typeId != TAG_Compound) {
        throw std::runtime_error("Root tag must be a compound tag, got type " +
                                  std::to_string(typeId));
    }
    in.readUTF(); // root name (usually empty)
    auto root = std::make_shared<NBTTagCompound>();
    root->read(in, 0);
    return root;
}

// Serialize compound to byte vector (uncompressed)
inline std::vector<uint8_t> serializeNBT(const NBTTagCompound& root) {
    DataStream out;
    writeRootTag(root, out);
    return std::move(out.buffer());
}

// Deserialize compound from byte vector (uncompressed)
inline std::shared_ptr<NBTTagCompound> deserializeNBT(const std::vector<uint8_t>& data) {
    DataStream in(data);
    return readRootTag(in);
}

} // namespace nbt
} // namespace mc
