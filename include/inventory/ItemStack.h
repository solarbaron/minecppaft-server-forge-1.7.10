#pragma once
// ItemStack — ported from add.java
// Fields: b = stackSize, e = Item (adb), f = damage/metadata, d = NBT tag
//
// NBT format (add.b(dh)):
//   id: short (item ID)
//   Count: byte (stack size)
//   Damage: short (damage/metadata)
//   tag: compound (optional, enchantments, display, etc.)

#include <cstdint>
#include <memory>
#include <optional>
#include "nbt/NBT.h"
#include "inventory/Item.h"

namespace mc {

struct ItemStack {
    int16_t itemId = 0;      // add.e → adb.b(item) = numeric ID
    int8_t  count = 0;       // add.b = stackSize
    int16_t damage = 0;      // add.f = damage/metadata
    std::shared_ptr<nbt::NBTTagCompound> nbt; // add.d = tag compound

    ItemStack() = default;

    ItemStack(int16_t id, int8_t cnt = 1, int16_t dmg = 0)
        : itemId(id), count(cnt), damage(dmg) {}

    bool isEmpty() const { return itemId <= 0 || count <= 0; }

    // NBT serialization — add.b(dh)
    std::shared_ptr<nbt::NBTTagCompound> saveToNBT() const {
        auto tag = std::make_shared<nbt::NBTTagCompound>();
        tag->setShort("id", itemId);
        tag->setByte("Count", count);
        tag->setShort("Damage", damage);
        if (nbt) {
            tag->setTag("tag", nbt);
        }
        return tag;
    }

    // NBT deserialization — add.c(dh)
    static ItemStack loadFromNBT(const nbt::NBTTagCompound& tag) {
        ItemStack stack;
        stack.itemId = tag.getShort("id");
        stack.count = tag.getByte("Count");
        stack.damage = tag.getShort("Damage");
        if (stack.damage < 0) stack.damage = 0;
        if (tag.hasKey("tag")) {
            stack.nbt = tag.getCompound("tag");
        }
        return stack;
    }

    // Protocol 47 slot serialization (for Set Slot / Window Items)
    // Format: short itemId (-1 if empty), byte count, short damage, [NBT length + NBT data]
    void writeToPacket(PacketBuffer& buf) const {
        if (isEmpty()) {
            buf.writeShort(-1);
            return;
        }
        buf.writeShort(itemId);
        buf.writeByte(static_cast<uint8_t>(count));
        buf.writeShort(damage);

        if (nbt && !nbt->isEmpty()) {
            // Serialize NBT to bytes
            auto nbtData = nbt::serializeNBT(*nbt);
            buf.writeShort(static_cast<int16_t>(nbtData.size()));
            buf.writeBytes(nbtData);
        } else {
            buf.writeShort(-1); // No NBT
        }
    }

    // Read from packet buffer
    static ItemStack readFromPacket(PacketBuffer& buf) {
        int16_t id = buf.readShort();
        if (id < 0) return ItemStack(); // empty

        ItemStack stack;
        stack.itemId = id;
        stack.count = static_cast<int8_t>(buf.readByte());
        stack.damage = buf.readShort();

        int16_t nbtLen = buf.readShort();
        if (nbtLen > 0) {
            auto nbtBytes = buf.readBytes(static_cast<size_t>(nbtLen));
            stack.nbt = nbt::deserializeNBT(nbtBytes);
        }

        return stack;
    }
};

} // namespace mc
