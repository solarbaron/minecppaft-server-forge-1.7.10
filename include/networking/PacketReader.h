/**
 * PacketReader.h — Serverbound packet deserialization for Protocol v5 (1.7.10).
 *
 * Reference: wiki.vg Protocol page for 1.7.10
 * Java references:
 *   - net.minecraft.network.PacketBuffer — readVarIntFromBuffer, readStringFromBuffer
 *   - net.minecraft.network.play.client.* — All C→S play packets
 *
 * PacketReader wraps a raw byte buffer and provides big-endian read methods
 * matching Java's DataInputStream. All serverbound play packets are parsed
 * into typed structs.
 *
 * Thread safety: PacketReader is not thread-safe (single-reader per instance).
 * Each connection's read thread creates its own reader.
 */
#pragma once

#include "PlayPackets.h"

#include <cstdint>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// PacketReader — Big-endian binary reader over a byte buffer.
// ═══════════════════════════════════════════════════════════════════════════

class PacketReader {
public:
    PacketReader(const uint8_t* data, size_t size)
        : data_(data), size_(size), pos_(0) {}

    explicit PacketReader(const std::vector<uint8_t>& buf)
        : data_(buf.data()), size_(buf.size()), pos_(0) {}

    // ─── Primitive reads ───

    uint8_t readUByte() {
        checkRemaining(1);
        return data_[pos_++];
    }

    int8_t readByte() {
        return static_cast<int8_t>(readUByte());
    }

    bool readBool() {
        return readUByte() != 0;
    }

    int16_t readShort() {
        checkRemaining(2);
        int16_t v = static_cast<int16_t>(
            (static_cast<uint16_t>(data_[pos_]) << 8) |
            static_cast<uint16_t>(data_[pos_ + 1]));
        pos_ += 2;
        return v;
    }

    uint16_t readUShort() {
        return static_cast<uint16_t>(readShort());
    }

    int32_t readInt() {
        checkRemaining(4);
        int32_t v = (static_cast<int32_t>(data_[pos_]) << 24) |
                    (static_cast<int32_t>(data_[pos_ + 1]) << 16) |
                    (static_cast<int32_t>(data_[pos_ + 2]) << 8) |
                    static_cast<int32_t>(data_[pos_ + 3]);
        pos_ += 4;
        return v;
    }

    int64_t readLong() {
        checkRemaining(8);
        int64_t v = 0;
        for (int i = 0; i < 8; ++i) {
            v = (v << 8) | static_cast<int64_t>(data_[pos_++]);
        }
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

    int32_t readVarInt() {
        int32_t result = 0;
        int shift = 0;
        uint8_t b;
        do {
            checkRemaining(1);
            b = data_[pos_++];
            result |= (b & 0x7F) << shift;
            shift += 7;
            if (shift >= 32) {
                throw std::runtime_error("VarInt too big");
            }
        } while (b & 0x80);
        return result;
    }

    int64_t readVarLong() {
        int64_t result = 0;
        int shift = 0;
        uint8_t b;
        do {
            checkRemaining(1);
            b = data_[pos_++];
            result |= static_cast<int64_t>(b & 0x7F) << shift;
            shift += 7;
            if (shift >= 64) {
                throw std::runtime_error("VarLong too big");
            }
        } while (b & 0x80);
        return result;
    }

    // Java: readStringFromBuffer — VarInt length + UTF-8 bytes
    std::string readString(int32_t maxLen = 32767) {
        int32_t len = readVarInt();
        if (len < 0 || len > maxLen) {
            throw std::runtime_error("String too long: " + std::to_string(len));
        }
        checkRemaining(static_cast<size_t>(len));
        std::string s(reinterpret_cast<const char*>(data_ + pos_), static_cast<size_t>(len));
        pos_ += static_cast<size_t>(len);
        return s;
    }

    // Read raw bytes
    std::vector<uint8_t> readBytes(size_t count) {
        checkRemaining(count);
        std::vector<uint8_t> result(data_ + pos_, data_ + pos_ + count);
        pos_ += count;
        return result;
    }

    // ─── Position info ───
    size_t remaining() const { return size_ - pos_; }
    size_t position() const { return pos_; }
    bool hasMore() const { return pos_ < size_; }
    void skip(size_t n) { checkRemaining(n); pos_ += n; }

private:
    void checkRemaining(size_t n) const {
        if (pos_ + n > size_) {
            throw std::runtime_error("PacketReader: buffer underflow");
        }
    }

    const uint8_t* data_;
    size_t size_;
    size_t pos_;
};

// ═══════════════════════════════════════════════════════════════════════════
// Serverbound Packet Structures — Parsed from PacketReader.
// All 24 serverbound play packets for Protocol v5.
// ═══════════════════════════════════════════════════════════════════════════

// 0x00 Keep Alive
struct SB_KeepAlive {
    int32_t keepAliveId;
    static SB_KeepAlive read(PacketReader& r) {
        return { r.readVarInt() };
    }
};

// 0x01 Chat Message
struct SB_ChatMessage {
    std::string message; // max 100 chars
    static SB_ChatMessage read(PacketReader& r) {
        return { r.readString(100) };
    }
};

// 0x02 Use Entity
struct SB_UseEntity {
    int32_t targetId;
    int8_t type; // 0=interact, 1=attack
    static SB_UseEntity read(PacketReader& r) {
        return { r.readInt(), r.readByte() };
    }
};

// 0x03 Player (on ground only)
struct SB_Player {
    bool onGround;
    static SB_Player read(PacketReader& r) {
        return { r.readBool() };
    }
};

// 0x04 Player Position
struct SB_PlayerPosition {
    double x, feetY, headY, z;
    bool onGround;
    static SB_PlayerPosition read(PacketReader& r) {
        double x = r.readDouble();
        double feetY = r.readDouble();
        double headY = r.readDouble();
        double z = r.readDouble();
        bool onGround = r.readBool();
        return { x, feetY, headY, z, onGround };
    }
};

// 0x05 Player Look
struct SB_PlayerLook {
    float yaw, pitch;
    bool onGround;
    static SB_PlayerLook read(PacketReader& r) {
        float yaw = r.readFloat();
        float pitch = r.readFloat();
        bool onGround = r.readBool();
        return { yaw, pitch, onGround };
    }
};

// 0x06 Player Position And Look
struct SB_PlayerPosAndLook {
    double x, feetY, headY, z;
    float yaw, pitch;
    bool onGround;
    static SB_PlayerPosAndLook read(PacketReader& r) {
        double x = r.readDouble();
        double feetY = r.readDouble();
        double headY = r.readDouble();
        double z = r.readDouble();
        float yaw = r.readFloat();
        float pitch = r.readFloat();
        bool onGround = r.readBool();
        return { x, feetY, headY, z, yaw, pitch, onGround };
    }
};

// 0x07 Player Digging
struct SB_PlayerDigging {
    int8_t status; // 0=start, 1=cancel, 2=finish, 3=drop stack, 4=drop item, 5=shoot/eat
    int32_t x;
    uint8_t y;
    int32_t z;
    int8_t face; // 0-5
    static SB_PlayerDigging read(PacketReader& r) {
        int8_t status = r.readByte();
        int32_t x = r.readInt();
        uint8_t y = r.readUByte();
        int32_t z = r.readInt();
        int8_t face = r.readByte();
        return { status, x, y, z, face };
    }
};

// 0x08 Player Block Placement
struct SB_PlayerBlockPlace {
    int32_t x;
    uint8_t y;
    int32_t z;
    int8_t direction; // 0-5, or -1 for use item
    // Held item slot data follows (simplified: item id)
    int16_t heldItemId;
    int8_t cursorX, cursorY, cursorZ; // 0-16 within block face
    static SB_PlayerBlockPlace read(PacketReader& r) {
        int32_t x = r.readInt();
        uint8_t y = r.readUByte();
        int32_t z = r.readInt();
        int8_t dir = r.readByte();
        int16_t itemId = r.readShort();
        int8_t cx = 0, cy = 0, cz = 0;
        if (itemId >= 0) {
            r.readByte();   // count
            r.readShort();  // damage
            int8_t nbtTag = r.readByte(); // 0 = no NBT
            if (nbtTag != 0) {
                // Skip NBT — for now just consume remaining
                // Full NBT parsing will be done in a dedicated reader
            }
        }
        cx = r.readByte();
        cy = r.readByte();
        cz = r.readByte();
        return { x, y, z, dir, itemId, cx, cy, cz };
    }
};

// 0x09 Held Item Change
struct SB_HeldItemChange {
    int16_t slotId; // 0-8
    static SB_HeldItemChange read(PacketReader& r) {
        return { r.readShort() };
    }
};

// 0x0A Animation
struct SB_Animation {
    int32_t entityId;
    int8_t animation; // 1=swing arm
    static SB_Animation read(PacketReader& r) {
        return { r.readInt(), r.readByte() };
    }
};

// 0x0B Entity Action
struct SB_EntityAction {
    int32_t entityId;
    int8_t actionId; // 1=sneak, 2=unsneak, 3=bed, 4=sprint, 5=unsprint, 6=horseJump, 7=openInv
    int32_t jumpBoost;
    static SB_EntityAction read(PacketReader& r) {
        int32_t eid = r.readInt();
        int8_t action = r.readByte();
        int32_t boost = r.readInt();
        return { eid, action, boost };
    }
};

// 0x0C Steer Vehicle
struct SB_SteerVehicle {
    float sideways, forward;
    bool jump, unmount;
    static SB_SteerVehicle read(PacketReader& r) {
        float sw = r.readFloat();
        float fw = r.readFloat();
        bool j = r.readBool();
        bool u = r.readBool();
        return { sw, fw, j, u };
    }
};

// 0x0D Close Window
struct SB_CloseWindow {
    uint8_t windowId;
    static SB_CloseWindow read(PacketReader& r) {
        return { r.readUByte() };
    }
};

// 0x0E Click Window
struct SB_ClickWindow {
    uint8_t windowId;
    int16_t slot;
    int8_t button;
    int16_t actionNumber;
    int8_t mode;
    // Clicked item follows (simplified)
    int16_t itemId;
    static SB_ClickWindow read(PacketReader& r) {
        uint8_t wid = r.readUByte();
        int16_t slot = r.readShort();
        int8_t btn = r.readByte();
        int16_t action = r.readShort();
        int8_t mode = r.readByte();
        int16_t itemId = r.readShort();
        if (itemId >= 0) {
            r.readByte();   // count
            r.readShort();  // damage
            int8_t nbt = r.readByte();
            // Skip NBT if present
        }
        return { wid, slot, btn, action, mode, itemId };
    }
};

// 0x0F Confirm Transaction
struct SB_ConfirmTransaction {
    uint8_t windowId;
    int16_t actionNumber;
    bool accepted;
    static SB_ConfirmTransaction read(PacketReader& r) {
        return { r.readUByte(), r.readShort(), r.readBool() };
    }
};

// 0x10 Creative Inventory Action
struct SB_CreativeInventory {
    int16_t slot;
    int16_t itemId;
    static SB_CreativeInventory read(PacketReader& r) {
        int16_t slot = r.readShort();
        int16_t itemId = r.readShort();
        if (itemId >= 0) {
            r.readByte();   // count
            r.readShort();  // damage
            r.readByte();   // NBT tag (0=none)
        }
        return { slot, itemId };
    }
};

// 0x11 Enchant Item
struct SB_EnchantItem {
    uint8_t windowId;
    int8_t enchantment; // 0-2, slot in enchanting table
    static SB_EnchantItem read(PacketReader& r) {
        return { r.readUByte(), r.readByte() };
    }
};

// 0x12 Update Sign
struct SB_UpdateSign {
    int32_t x;
    int16_t y;
    int32_t z;
    std::string line1, line2, line3, line4;
    static SB_UpdateSign read(PacketReader& r) {
        int32_t x = r.readInt();
        int16_t y = r.readShort();
        int32_t z = r.readInt();
        std::string l1 = r.readString(15);
        std::string l2 = r.readString(15);
        std::string l3 = r.readString(15);
        std::string l4 = r.readString(15);
        return { x, y, z, std::move(l1), std::move(l2), std::move(l3), std::move(l4) };
    }
};

// 0x13 Player Abilities
struct SB_PlayerAbilities {
    uint8_t flags; // bit 0=invuln, 1=flying, 2=allowFly, 3=creative
    float flySpeed, walkSpeed;
    static SB_PlayerAbilities read(PacketReader& r) {
        uint8_t flags = r.readUByte();
        float fly = r.readFloat();
        float walk = r.readFloat();
        return { flags, fly, walk };
    }
};

// 0x14 Tab Complete
struct SB_TabComplete {
    std::string text;
    static SB_TabComplete read(PacketReader& r) {
        return { r.readString(32767) };
    }
};

// 0x15 Client Settings
struct SB_ClientSettings {
    std::string locale;
    int8_t viewDistance;
    int8_t chatFlags;
    bool chatColors;
    uint8_t skinParts; // displayed skin parts bitmask
    static SB_ClientSettings read(PacketReader& r) {
        std::string locale = r.readString(7);
        int8_t vd = r.readByte();
        int8_t cf = r.readByte();
        bool cc = r.readBool();
        // In 1.7.10, difficulty is sent here too but not used
        r.readByte(); // unused difficulty
        uint8_t sp = r.readUByte();
        return { std::move(locale), vd, cf, cc, sp };
    }
};

// 0x16 Client Status
struct SB_ClientStatus {
    int8_t actionId; // 0=respawn, 1=request stats, 2=open inventory achievement
    static SB_ClientStatus read(PacketReader& r) {
        return { r.readByte() };
    }
};

// 0x17 Plugin Message
struct SB_PluginMessage {
    std::string channel;
    std::vector<uint8_t> data;
    static SB_PluginMessage read(PacketReader& r) {
        std::string channel = r.readString(20);
        int16_t len = r.readShort();
        std::vector<uint8_t> data = r.readBytes(static_cast<size_t>(len));
        return { std::move(channel), std::move(data) };
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// VarInt framing helper — extract one packet from a stream buffer.
// Returns the packet data (without length prefix) or nullopt if incomplete.
// ═══════════════════════════════════════════════════════════════════════════

inline std::optional<std::vector<uint8_t>> extractPacket(const uint8_t* data, size_t size,
                                                          size_t& consumed) {
    consumed = 0;
    if (size == 0) return std::nullopt;

    // Try to read VarInt length
    int32_t length = 0;
    int shift = 0;
    size_t pos = 0;
    while (pos < size) {
        uint8_t b = data[pos++];
        length |= (b & 0x7F) << shift;
        shift += 7;
        if (!(b & 0x80)) {
            // Complete VarInt
            if (length < 0 || length > 2097152) { // 2 MiB max
                throw std::runtime_error("Packet too large: " + std::to_string(length));
            }
            size_t totalNeeded = pos + static_cast<size_t>(length);
            if (size < totalNeeded) return std::nullopt; // Need more data
            consumed = totalNeeded;
            return std::vector<uint8_t>(data + pos, data + totalNeeded);
        }
        if (shift >= 35) {
            throw std::runtime_error("VarInt too big in frame");
        }
    }
    return std::nullopt; // Incomplete VarInt
}

} // namespace mccpp
