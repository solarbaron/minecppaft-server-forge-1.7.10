#pragma once
// WindowInteraction — container window open/close/click + sign editing.
// Ported from iu.java (S2DPacketOpenWindow), iv.java (S2FPacketSetSlot),
// C→S jb.java (C0DPacketCloseWindow), jc.java (C0EPacketClickWindow),
// ji.java (C12PacketUpdateSign).
//
// Protocol 5 (1.7.10):
//   S→C 0x2D: Open Window (windowId, type, title, slotCount, useTitle)
//   S→C 0x2F: Set Slot (windowId, slot, slotData)
//   S→C 0x30: Window Items (already implemented)
//   S→C 0x32: Confirm Transaction (windowId, action, accepted)
//   C→S 0x0D: Close Window (windowId)
//   C→S 0x0E: Click Window (windowId, slot, button, action, mode, clicked)
//   C→S 0x12: Update Sign (x, y, z, line1-4)

#include <cstdint>
#include <string>
#include <array>
#include <unordered_map>
#include "networking/PacketBuffer.h"

namespace mc {

// OpenWindowPacket (0x2D), SetSlotPacket (0x2F), and
// ConfirmTransactionPacket (0x32) are already defined in PlayPackets.h

// ============================================================
// S→C 0x33 Update Sign — iy.java
// ============================================================
struct UpdateSignPacket {
    int32_t x;
    int16_t y;
    int32_t z;
    std::string line1, line2, line3, line4;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x33);
        buf.writeInt(x);
        buf.writeShort(y);
        buf.writeInt(z);
        buf.writeString(line1);
        buf.writeString(line2);
        buf.writeString(line3);
        buf.writeString(line4);
        return buf;
    }
};

// ============================================================
// C→S 0x0D Close Window — jb.java
// ============================================================
struct CloseWindowC2S {
    uint8_t windowId;

    static CloseWindowC2S read(PacketBuffer& buf) {
        CloseWindowC2S pkt;
        pkt.windowId = buf.readByte();
        return pkt;
    }
};

// ============================================================
// C→S 0x0E Click Window — jc.java
// ============================================================
struct ClickWindowC2S {
    int8_t  windowId;
    int16_t slotIndex;
    int8_t  button;       // 0=left, 1=right
    int16_t actionNumber;
    int8_t  mode;         // 0=click, 1=shift-click, 2=number key, 3=middle, 4=drop, 5=drag, 6=double
    // Clicked item data
    int16_t itemId = -1;
    int8_t  count = 0;
    int16_t meta = 0;

    static ClickWindowC2S read(PacketBuffer& buf) {
        ClickWindowC2S pkt;
        pkt.windowId = static_cast<int8_t>(buf.readByte());
        pkt.slotIndex = buf.readShort();
        pkt.button = static_cast<int8_t>(buf.readByte());
        pkt.actionNumber = buf.readShort();
        pkt.mode = static_cast<int8_t>(buf.readByte());
        // Read slot data
        pkt.itemId = buf.readShort();
        if (pkt.itemId >= 0) {
            pkt.count = static_cast<int8_t>(buf.readByte());
            pkt.meta = buf.readShort();
            // Skip NBT data
            int8_t nbtTag = static_cast<int8_t>(buf.readByte());
            if (nbtTag != 0) {
                // Non-empty NBT — skip it (simplified)
                // In a full implementation we'd parse the NBT compound
            }
        }
        return pkt;
    }
};

// ============================================================
// C→S 0x12 Update Sign — ji.java
// ============================================================
struct UpdateSignC2S {
    int32_t x;
    int16_t y;
    int32_t z;
    std::string line1, line2, line3, line4;

    static UpdateSignC2S read(PacketBuffer& buf) {
        UpdateSignC2S pkt;
        pkt.x = buf.readInt();
        pkt.y = buf.readShort();
        pkt.z = buf.readInt();
        pkt.line1 = buf.readString(15);
        pkt.line2 = buf.readString(15);
        pkt.line3 = buf.readString(15);
        pkt.line4 = buf.readString(15);
        return pkt;
    }
};

// ============================================================
// Sign tile entity storage
// ============================================================
struct SignData {
    int x, y, z;
    std::array<std::string, 4> lines;
};

// Manages sign data in the world
class SignManager {
public:
    void setSign(int x, int y, int z, const std::string& l1, const std::string& l2,
                 const std::string& l3, const std::string& l4) {
        auto key = packKey(x, y, z);
        signs_[key] = {{l1, l2, l3, l4}};
    }

    const std::array<std::string, 4>* getSign(int x, int y, int z) const {
        auto it = signs_.find(packKey(x, y, z));
        return it != signs_.end() ? &it->second : nullptr;
    }

    void removeSign(int x, int y, int z) {
        signs_.erase(packKey(x, y, z));
    }

    // Create an Update Sign packet for sending to clients
    UpdateSignPacket makePacket(int x, int y, int z) const {
        auto* lines = getSign(x, y, z);
        UpdateSignPacket pkt;
        pkt.x = x;
        pkt.y = static_cast<int16_t>(y);
        pkt.z = z;
        if (lines) {
            pkt.line1 = (*lines)[0];
            pkt.line2 = (*lines)[1];
            pkt.line3 = (*lines)[2];
            pkt.line4 = (*lines)[3];
        }
        return pkt;
    }

private:
    static int64_t packKey(int x, int y, int z) {
        return (static_cast<int64_t>(x) & 0x3FFFFFF) |
               ((static_cast<int64_t>(z) & 0x3FFFFFF) << 26) |
               ((static_cast<int64_t>(y) & 0xFFF) << 52);
    }

    std::unordered_map<int64_t, std::array<std::string, 4>> signs_;
};

} // namespace mc
