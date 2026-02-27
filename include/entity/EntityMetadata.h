#pragma once
// EntityMetadata — DataWatcher entity metadata system.
// Ported from vanilla 1.7.10 DataWatcher (te.java) and
// S1CPacketEntityMetadata (gy.java).
//
// Protocol 5 (1.7.10) metadata format:
//   Each entry: (type << 5 | index) as byte, then value.
//   Types: 0=byte, 1=short, 2=int, 3=float, 4=string, 5=ItemStack, 6=xyz ints
//   Terminated by 0x7F byte.
//
// S→C 0x1C Entity Metadata — sends watchable object changes

#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <unordered_map>
#include "networking/PacketBuffer.h"

namespace mc {

// ============================================================
// Metadata value types — te.java (DataWatcher)
// ============================================================
struct MetadataItemStack {
    int16_t itemId;
    int8_t  count;
    int16_t damage;
};

struct MetadataBlockPos {
    int32_t x, y, z;
};

using MetadataValue = std::variant<
    int8_t,           // type 0: Byte
    int16_t,          // type 1: Short
    int32_t,          // type 2: Int
    float,            // type 3: Float
    std::string,      // type 4: String
    MetadataItemStack, // type 5: ItemStack (Slot)
    MetadataBlockPos   // type 6: Block Position (x,y,z ints)
>;

struct MetadataEntry {
    uint8_t       index;
    MetadataValue value;
};

// ============================================================
// DataWatcher — tracks entity metadata entries
// ============================================================
class DataWatcher {
public:
    // Add/update entries
    void setByte(uint8_t index, int8_t val) {
        entries_[index] = MetadataEntry{index, val};
        dirty_.insert(index);
    }

    void setShort(uint8_t index, int16_t val) {
        entries_[index] = MetadataEntry{index, val};
        dirty_.insert(index);
    }

    void setInt(uint8_t index, int32_t val) {
        entries_[index] = MetadataEntry{index, val};
        dirty_.insert(index);
    }

    void setFloat(uint8_t index, float val) {
        entries_[index] = MetadataEntry{index, val};
        dirty_.insert(index);
    }

    void setString(uint8_t index, const std::string& val) {
        entries_[index] = MetadataEntry{index, val};
        dirty_.insert(index);
    }

    void setItemStack(uint8_t index, int16_t itemId, int8_t count, int16_t damage) {
        entries_[index] = MetadataEntry{index, MetadataItemStack{itemId, count, damage}};
        dirty_.insert(index);
    }

    void setBlockPos(uint8_t index, int32_t x, int32_t y, int32_t z) {
        entries_[index] = MetadataEntry{index, MetadataBlockPos{x, y, z}};
        dirty_.insert(index);
    }

    // Check if any values changed since last sync
    bool isDirty() const { return !dirty_.empty(); }

    // Get changed entries and clear dirty flags
    std::vector<MetadataEntry> getDirtyAndClear() {
        std::vector<MetadataEntry> result;
        for (auto idx : dirty_) {
            auto it = entries_.find(idx);
            if (it != entries_.end()) {
                result.push_back(it->second);
            }
        }
        dirty_.clear();
        return result;
    }

    // Get all entries (for initial spawn)
    std::vector<MetadataEntry> getAll() const {
        std::vector<MetadataEntry> result;
        for (auto& [idx, entry] : entries_) {
            result.push_back(entry);
        }
        return result;
    }

    // Serialize metadata entries to buffer (protocol format)
    static void writeToBuffer(PacketBuffer& buf, const std::vector<MetadataEntry>& entries) {
        for (auto& entry : entries) {
            uint8_t typeId = static_cast<uint8_t>(entry.value.index());
            uint8_t header = (typeId << 5) | (entry.index & 0x1F);
            buf.writeByte(header);

            std::visit([&buf](auto&& val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, int8_t>) {
                    buf.writeByte(static_cast<uint8_t>(val));
                } else if constexpr (std::is_same_v<T, int16_t>) {
                    buf.writeShort(val);
                } else if constexpr (std::is_same_v<T, int32_t>) {
                    buf.writeInt(val);
                } else if constexpr (std::is_same_v<T, float>) {
                    buf.writeFloat(val);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    buf.writeString(val);
                } else if constexpr (std::is_same_v<T, MetadataItemStack>) {
                    buf.writeShort(val.itemId);
                    if (val.itemId >= 0) {
                        buf.writeByte(static_cast<uint8_t>(val.count));
                        buf.writeShort(val.damage);
                    }
                } else if constexpr (std::is_same_v<T, MetadataBlockPos>) {
                    buf.writeInt(val.x);
                    buf.writeInt(val.y);
                    buf.writeInt(val.z);
                }
            }, entry.value);
        }
        buf.writeByte(0x7F); // Terminator
    }

    // Common metadata indices — Entity base class (sa.java)
    // Index 0: Byte — entity flags (on fire, crouching, sprinting, invisible, etc.)
    static constexpr uint8_t IDX_FLAGS       = 0;
    static constexpr uint8_t IDX_AIR         = 1;  // Short — air ticks remaining
    static constexpr uint8_t IDX_NAME_TAG    = 2;  // String — custom name
    static constexpr uint8_t IDX_SHOW_NAME   = 3;  // Byte — show custom name (1/0)
    static constexpr uint8_t IDX_SILENT      = 4;  // Byte — silent (1/0)

    // Entity flags (index 0 bitmask)
    static constexpr int8_t FLAG_ON_FIRE     = 0x01;
    static constexpr int8_t FLAG_CROUCHING   = 0x02;
    static constexpr int8_t FLAG_SPRINTING   = 0x08;
    static constexpr int8_t FLAG_EATING      = 0x10; // Also: blocking with sword
    static constexpr int8_t FLAG_INVISIBLE   = 0x20;

    // LivingEntity indices (sh.java)
    static constexpr uint8_t IDX_HEALTH      = 6;  // Float — health
    static constexpr uint8_t IDX_POTION_COLOR = 7; // Int — potion effect color
    static constexpr uint8_t IDX_POTION_AMBIENT = 8; // Byte — potion ambient

    // Player-specific indices (yz.java / EntityPlayer)
    static constexpr uint8_t IDX_ABSORPTION  = 17; // Float — absorption hearts
    static constexpr uint8_t IDX_SCORE       = 18; // Int — player score

    // Mob-specific
    static constexpr uint8_t IDX_MOB_FLAGS   = 15; // Byte — mob AI flags
    static constexpr uint8_t IDX_BABY        = 12; // Byte — is baby (1/0)

    // Item entity
    static constexpr uint8_t IDX_ITEM        = 10; // Slot — item stack

    // Arrow
    static constexpr uint8_t IDX_ARROW_CRITICAL = 16; // Byte — is critical

    // Setup default player metadata
    void initPlayer() {
        setByte(IDX_FLAGS, 0);
        setShort(IDX_AIR, 300);
        setString(IDX_NAME_TAG, "");
        setByte(IDX_SHOW_NAME, 1);
        setByte(IDX_SILENT, 0);
        setFloat(IDX_HEALTH, 20.0f);
        setInt(IDX_POTION_COLOR, 0);
        setByte(IDX_POTION_AMBIENT, 0);
        setFloat(IDX_ABSORPTION, 0.0f);
        setInt(IDX_SCORE, 0);
        dirty_.clear(); // Init is not "dirty"
    }

    // Setup default mob metadata
    void initMob() {
        setByte(IDX_FLAGS, 0);
        setShort(IDX_AIR, 300);
        setString(IDX_NAME_TAG, "");
        setByte(IDX_SHOW_NAME, 0);
        setByte(IDX_SILENT, 0);
        setFloat(IDX_HEALTH, 20.0f);
        setInt(IDX_POTION_COLOR, 0);
        setByte(IDX_POTION_AMBIENT, 0);
        setByte(IDX_MOB_FLAGS, 0);
        dirty_.clear();
    }

private:
    std::unordered_map<uint8_t, MetadataEntry> entries_;
    std::unordered_set<uint8_t> dirty_;
};

// ============================================================
// S→C 0x1C Entity Metadata — gy.java
// ============================================================
struct EntityMetadataPacket {
    int32_t entityId;
    std::vector<MetadataEntry> entries;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x1C);
        buf.writeVarInt(entityId);
        DataWatcher::writeToBuffer(buf, entries);
        return buf;
    }
};

// ============================================================
// S→C 0x19 Entity Head Look — gx.java
// ============================================================
struct EntityHeadLookPacket {
    int32_t entityId;
    int8_t  headYaw;  // Angle in steps of 1/256 of a turn

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x19);
        buf.writeVarInt(entityId);
        buf.writeByte(static_cast<uint8_t>(headYaw));
        return buf;
    }

    static EntityHeadLookPacket fromDegrees(int32_t eid, float yaw) {
        return {eid, static_cast<int8_t>(yaw / 360.0f * 256.0f)};
    }
};

// ============================================================
// S→C 0x1B Attach Entity — gt.java
// ============================================================
struct AttachEntityPacket {
    int32_t entityId;
    int32_t vehicleId;  // -1 to detach
    bool    leash;       // true = leash, false = riding

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x1B);
        buf.writeInt(entityId);
        buf.writeInt(vehicleId);
        buf.writeBoolean(leash);
        return buf;
    }
};

// ============================================================
// S→C 0x04 Entity Equipment — gl.java
// ============================================================
struct EntityEquipmentPacket {
    int32_t entityId;
    int16_t slot;       // 0=held, 1=boots, 2=leggings, 3=chestplate, 4=helmet
    // Item slot data
    int16_t itemId = -1;
    int8_t  count = 0;
    int16_t damage = 0;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x04);
        buf.writeVarInt(entityId);
        buf.writeShort(slot);
        if (itemId < 0) {
            buf.writeShort(-1); // Empty slot
        } else {
            buf.writeShort(itemId);
            buf.writeByte(static_cast<uint8_t>(count));
            buf.writeShort(damage);
            buf.writeByte(0); // No NBT
        }
        return buf;
    }

    // Slot constants
    static constexpr int16_t SLOT_HELD       = 0;
    static constexpr int16_t SLOT_BOOTS      = 1;
    static constexpr int16_t SLOT_LEGGINGS   = 2;
    static constexpr int16_t SLOT_CHESTPLATE = 3;
    static constexpr int16_t SLOT_HELMET     = 4;
};

} // namespace mc
