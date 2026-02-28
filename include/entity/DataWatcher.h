/**
 * DataWatcher.h — Entity metadata synchronization.
 *
 * Java reference: net.minecraft.entity.DataWatcher
 *
 * Protocol format (1.7.10):
 *   Each entry: 1 byte header = (typeId << 5) | (dataId & 0x1F)
 *   Then type-specific payload:
 *     0 = byte, 1 = short, 2 = int, 3 = float,
 *     4 = string (varint-prefixed UTF-8),
 *     5 = slot (item stack), 6 = int,int,int (block pos)
 *   Terminated by 0x7F (127)
 *
 * Max 32 entries (IDs 0-31).
 * Used by every entity for syncing flags, health, name, etc.
 *
 * Thread safety: shared_mutex for concurrent reads.
 *
 * JNI readiness: std::variant for type safety, simple ID-based lookup.
 */
#pragma once

#include <cstdint>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// DataWatcher type IDs — match Java exactly.
// Java: static { dataTypes.put(Byte.class, 0); ... }
// ═══════════════════════════════════════════════════════════════════════════

namespace DataType {
    constexpr int32_t BYTE    = 0;
    constexpr int32_t SHORT   = 1;
    constexpr int32_t INT     = 2;
    constexpr int32_t FLOAT   = 3;
    constexpr int32_t STRING  = 4;
    constexpr int32_t SLOT    = 5; // ItemStack — serialized separately
    constexpr int32_t BLOCK_POS = 6; // int x, int y, int z
}

// ═══════════════════════════════════════════════════════════════════════════
// BlockPosData — Inline block position for DataWatcher type 6.
// ═══════════════════════════════════════════════════════════════════════════

struct BlockPosData {
    int32_t x = 0, y = 0, z = 0;
    bool operator==(const BlockPosData& o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const BlockPosData& o) const { return !(*this == o); }
};

// ═══════════════════════════════════════════════════════════════════════════
// SlotData — Simplified item stack for DataWatcher type 5.
// ═══════════════════════════════════════════════════════════════════════════

struct SlotData {
    int16_t itemId = -1;   // -1 = empty
    int8_t count = 0;
    int16_t damage = 0;
    // Note: full NBT data handled during serialization

    bool operator==(const SlotData& o) const {
        return itemId == o.itemId && count == o.count && damage == o.damage;
    }
    bool operator!=(const SlotData& o) const { return !(*this == o); }
    bool isEmpty() const { return itemId < 0; }
};

// ═══════════════════════════════════════════════════════════════════════════
// WatchableValue — Type-safe union for all DataWatcher types.
// ═══════════════════════════════════════════════════════════════════════════

using WatchableValue = std::variant<
    int8_t,       // type 0: byte
    int16_t,      // type 1: short
    int32_t,      // type 2: int
    float,        // type 3: float
    std::string,  // type 4: string
    SlotData,     // type 5: item stack
    BlockPosData  // type 6: block position
>;

// ═══════════════════════════════════════════════════════════════════════════
// WatchableObject — Single watched entry.
// Java reference: net.minecraft.entity.DataWatcher$WatchableObject
// ═══════════════════════════════════════════════════════════════════════════

struct WatchableObject {
    int32_t typeId;    // 0-6
    int32_t dataId;    // 0-31
    WatchableValue value;
    bool dirty = false;

    WatchableObject() : typeId(0), dataId(0), value(int8_t(0)) {}
    WatchableObject(int32_t type, int32_t id, WatchableValue val)
        : typeId(type), dataId(id), value(std::move(val)) {}

    // Java: (objectType << 5 | dataValueId & 0x1F) & 0xFF
    uint8_t getHeaderByte() const {
        return static_cast<uint8_t>((typeId << 5 | (dataId & 0x1F)) & 0xFF);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// DataWatcher — Entity metadata container with change tracking.
// Java reference: net.minecraft.entity.DataWatcher
//
// Thread safety: shared_mutex (multiple readers, exclusive writers).
// ═══════════════════════════════════════════════════════════════════════════

class DataWatcher {
public:
    DataWatcher() = default;

    // ─── Add new entries (during entity init) ───

    // Java: addObject — typed overloads
    void addByte(int32_t id, int8_t value) {
        addEntry(id, DataType::BYTE, WatchableValue(value));
    }

    void addShort(int32_t id, int16_t value) {
        addEntry(id, DataType::SHORT, WatchableValue(value));
    }

    void addInt(int32_t id, int32_t value) {
        addEntry(id, DataType::INT, WatchableValue(value));
    }

    void addFloat(int32_t id, float value) {
        addEntry(id, DataType::FLOAT, WatchableValue(value));
    }

    void addString(int32_t id, const std::string& value) {
        addEntry(id, DataType::STRING, WatchableValue(value));
    }

    void addSlot(int32_t id, const SlotData& value) {
        addEntry(id, DataType::SLOT, WatchableValue(value));
    }

    void addBlockPos(int32_t id, const BlockPosData& value) {
        addEntry(id, DataType::BLOCK_POS, WatchableValue(value));
    }

    // Java: addObjectByDataType — add with default value
    void addByType(int32_t id, int32_t typeId) {
        WatchableValue def;
        switch (typeId) {
            case DataType::BYTE:    def = int8_t(0); break;
            case DataType::SHORT:   def = int16_t(0); break;
            case DataType::INT:     def = int32_t(0); break;
            case DataType::FLOAT:   def = float(0.0f); break;
            case DataType::STRING:  def = std::string(); break;
            case DataType::SLOT:    def = SlotData{}; break;
            case DataType::BLOCK_POS: def = BlockPosData{}; break;
            default: return;
        }
        addEntry(id, typeId, std::move(def));
    }

    // ─── Typed getters ───

    // Java: getWatchableObjectByte
    int8_t getByte(int32_t id) const {
        std::shared_lock lock(mutex_);
        auto it = entries_.find(id);
        return (it != entries_.end()) ? std::get<int8_t>(it->second.value) : 0;
    }

    // Java: getWatchableObjectShort
    int16_t getShort(int32_t id) const {
        std::shared_lock lock(mutex_);
        auto it = entries_.find(id);
        return (it != entries_.end()) ? std::get<int16_t>(it->second.value) : 0;
    }

    // Java: getWatchableObjectInt
    int32_t getInt(int32_t id) const {
        std::shared_lock lock(mutex_);
        auto it = entries_.find(id);
        return (it != entries_.end()) ? std::get<int32_t>(it->second.value) : 0;
    }

    // Java: getWatchableObjectFloat
    float getFloat(int32_t id) const {
        std::shared_lock lock(mutex_);
        auto it = entries_.find(id);
        return (it != entries_.end()) ? std::get<float>(it->second.value) : 0.0f;
    }

    // Java: getWatchableObjectString
    std::string getString(int32_t id) const {
        std::shared_lock lock(mutex_);
        auto it = entries_.find(id);
        return (it != entries_.end()) ? std::get<std::string>(it->second.value) : "";
    }

    SlotData getSlot(int32_t id) const {
        std::shared_lock lock(mutex_);
        auto it = entries_.find(id);
        return (it != entries_.end()) ? std::get<SlotData>(it->second.value) : SlotData{};
    }

    BlockPosData getBlockPos(int32_t id) const {
        std::shared_lock lock(mutex_);
        auto it = entries_.find(id);
        return (it != entries_.end()) ? std::get<BlockPosData>(it->second.value) : BlockPosData{};
    }

    // ─── Update (with dirty tracking) ───

    // Java: updateObject — set new value, mark dirty if changed
    void updateByte(int32_t id, int8_t value) {
        std::unique_lock lock(mutex_);
        auto it = entries_.find(id);
        if (it != entries_.end() && std::get<int8_t>(it->second.value) != value) {
            it->second.value = value;
            it->second.dirty = true;
            hasChanges_ = true;
        }
    }

    void updateShort(int32_t id, int16_t value) {
        std::unique_lock lock(mutex_);
        auto it = entries_.find(id);
        if (it != entries_.end() && std::get<int16_t>(it->second.value) != value) {
            it->second.value = value;
            it->second.dirty = true;
            hasChanges_ = true;
        }
    }

    void updateInt(int32_t id, int32_t value) {
        std::unique_lock lock(mutex_);
        auto it = entries_.find(id);
        if (it != entries_.end() && std::get<int32_t>(it->second.value) != value) {
            it->second.value = value;
            it->second.dirty = true;
            hasChanges_ = true;
        }
    }

    void updateFloat(int32_t id, float value) {
        std::unique_lock lock(mutex_);
        auto it = entries_.find(id);
        if (it != entries_.end() && std::get<float>(it->second.value) != value) {
            it->second.value = value;
            it->second.dirty = true;
            hasChanges_ = true;
        }
    }

    void updateString(int32_t id, const std::string& value) {
        std::unique_lock lock(mutex_);
        auto it = entries_.find(id);
        if (it != entries_.end() && std::get<std::string>(it->second.value) != value) {
            it->second.value = value;
            it->second.dirty = true;
            hasChanges_ = true;
        }
    }

    void updateSlot(int32_t id, const SlotData& value) {
        std::unique_lock lock(mutex_);
        auto it = entries_.find(id);
        if (it != entries_.end() && std::get<SlotData>(it->second.value) != value) {
            it->second.value = value;
            it->second.dirty = true;
            hasChanges_ = true;
        }
    }

    void updateBlockPos(int32_t id, const BlockPosData& value) {
        std::unique_lock lock(mutex_);
        auto it = entries_.find(id);
        if (it != entries_.end() && std::get<BlockPosData>(it->second.value) != value) {
            it->second.value = value;
            it->second.dirty = true;
            hasChanges_ = true;
        }
    }

    // ─── Change tracking ───

    // Java: hasObjectChanged
    bool hasChanges() const {
        std::shared_lock lock(mutex_);
        return hasChanges_;
    }

    // Java: getChanged — returns dirty entries and clears dirty flags
    std::vector<WatchableObject> getChanged() {
        std::unique_lock lock(mutex_);
        std::vector<WatchableObject> result;
        if (!hasChanges_) return result;
        for (auto& [id, entry] : entries_) {
            if (entry.dirty) {
                entry.dirty = false;
                result.push_back(entry);
            }
        }
        hasChanges_ = false;
        return result;
    }

    // Java: getAllWatched — returns all entries
    std::vector<WatchableObject> getAll() const {
        std::shared_lock lock(mutex_);
        std::vector<WatchableObject> result;
        result.reserve(entries_.size());
        for (const auto& [id, entry] : entries_) {
            result.push_back(entry);
        }
        return result;
    }

    // Java: getIsBlank
    bool isBlank() const {
        std::shared_lock lock(mutex_);
        return entries_.empty();
    }

    // Java: func_111144_e — clear changed flag without returning entries
    void clearChanges() {
        std::unique_lock lock(mutex_);
        hasChanges_ = false;
    }

    // ─── Packet serialization helpers ───

    // Java: writeWatchableObjectToPacketBuffer
    // Writes a single entry to a byte buffer.
    // Format: header byte = (typeId << 5) | (dataId & 0x1F)
    //         Then type-specific data.
    static void writeEntry(const WatchableObject& entry, std::vector<uint8_t>& buf) {
        buf.push_back(entry.getHeaderByte());
        switch (entry.typeId) {
            case DataType::BYTE:
                buf.push_back(static_cast<uint8_t>(std::get<int8_t>(entry.value)));
                break;
            case DataType::SHORT: {
                int16_t v = std::get<int16_t>(entry.value);
                buf.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
                buf.push_back(static_cast<uint8_t>(v & 0xFF));
                break;
            }
            case DataType::INT: {
                int32_t v = std::get<int32_t>(entry.value);
                buf.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
                buf.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
                buf.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
                buf.push_back(static_cast<uint8_t>(v & 0xFF));
                break;
            }
            case DataType::FLOAT: {
                float fv = std::get<float>(entry.value);
                uint32_t iv;
                std::memcpy(&iv, &fv, sizeof(iv));
                buf.push_back(static_cast<uint8_t>((iv >> 24) & 0xFF));
                buf.push_back(static_cast<uint8_t>((iv >> 16) & 0xFF));
                buf.push_back(static_cast<uint8_t>((iv >> 8) & 0xFF));
                buf.push_back(static_cast<uint8_t>(iv & 0xFF));
                break;
            }
            case DataType::STRING: {
                // Varint-prefixed UTF-8 string
                const auto& s = std::get<std::string>(entry.value);
                writeVarInt(buf, static_cast<int32_t>(s.size()));
                buf.insert(buf.end(), s.begin(), s.end());
                break;
            }
            case DataType::SLOT: {
                // Simplified: just item ID as short
                const auto& slot = std::get<SlotData>(entry.value);
                int16_t id = slot.itemId;
                buf.push_back(static_cast<uint8_t>((id >> 8) & 0xFF));
                buf.push_back(static_cast<uint8_t>(id & 0xFF));
                if (id >= 0) {
                    buf.push_back(static_cast<uint8_t>(slot.count));
                    buf.push_back(static_cast<uint8_t>((slot.damage >> 8) & 0xFF));
                    buf.push_back(static_cast<uint8_t>(slot.damage & 0xFF));
                    buf.push_back(0); // No NBT (tag end)
                }
                break;
            }
            case DataType::BLOCK_POS: {
                const auto& bp = std::get<BlockPosData>(entry.value);
                writeInt(buf, bp.x);
                writeInt(buf, bp.y);
                writeInt(buf, bp.z);
                break;
            }
        }
    }

    // Java: writeWatchedListToPacketBuffer — write list + terminator
    static void writeList(const std::vector<WatchableObject>& entries, std::vector<uint8_t>& buf) {
        for (const auto& entry : entries) {
            writeEntry(entry, buf);
        }
        buf.push_back(0x7F); // Terminator = 127
    }

    // Write all entries to buffer
    void writeAllToBuffer(std::vector<uint8_t>& buf) const {
        auto all = getAll();
        writeList(all, buf);
    }

    // ─── Standard entity DataWatcher IDs ───
    // Java: Entity constructor registers these
    static constexpr int32_t ENTITY_FLAGS = 0;    // byte: bit 0=onFire, 1=sneaking, 2=riding, 3=sprinting, 4=eating/blocking, 5=invisible
    static constexpr int32_t AIR_TICKS = 1;       // short: 300 default

    // EntityLivingBase
    static constexpr int32_t HEALTH = 6;          // float
    static constexpr int32_t POTION_COLOR = 7;    // int
    static constexpr int32_t POTION_AMBIENT = 8;  // byte
    static constexpr int32_t ARROW_COUNT = 9;     // byte
    static constexpr int32_t HIDE_NAME = 10;      // byte (actually name tag visibility for 1.7.10)
    static constexpr int32_t ABSORPTION = 17;     // float (1.8+, but registered in 1.7.10)

private:
    void addEntry(int32_t id, int32_t typeId, WatchableValue&& val) {
        if (id > 31) return; // Java: max 31
        std::unique_lock lock(mutex_);
        if (entries_.count(id)) return; // Duplicate check
        entries_[id] = WatchableObject(typeId, id, std::move(val));
    }

    static void writeVarInt(std::vector<uint8_t>& buf, int32_t value) {
        uint32_t uv = static_cast<uint32_t>(value);
        while (uv >= 0x80) {
            buf.push_back(static_cast<uint8_t>(uv & 0x7F) | 0x80);
            uv >>= 7;
        }
        buf.push_back(static_cast<uint8_t>(uv));
    }

    static void writeInt(std::vector<uint8_t>& buf, int32_t v) {
        buf.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
        buf.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
        buf.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
        buf.push_back(static_cast<uint8_t>(v & 0xFF));
    }

    mutable std::shared_mutex mutex_;
    std::unordered_map<int32_t, WatchableObject> entries_;
    bool hasChanges_ = false;
};

} // namespace mccpp
