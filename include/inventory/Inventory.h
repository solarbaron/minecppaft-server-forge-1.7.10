#pragma once
// Inventory — ported from yz.java (EntityPlayer) inventory and aai.java (InventoryPlayer)
//
// MC 1.7.10 player inventory layout (45 slots):
//   Slots 0-8:   Hotbar (crafting result excluded in this model)
//   Slots 9-35:  Main inventory
//   Slots 36-39: Armor (boots=36, legs=37, chest=38, helm=39)
//   Slots 40-44: Crafting grid (2x2) + output
//
// Protocol window 0 (player inventory) slot mapping:
//   0:          Crafting output
//   1-4:        Crafting input (2x2)
//   5-8:        Armor (helm=5, chest=6, legs=7, boots=8)
//   9-35:       Main inventory
//   36-44:      Hotbar
//
// We store internally as:
//   mainInventory[36] = slots 0-35 (hotbar 0-8, main 9-35)
//   armorInventory[4] = armor slots
//   currentSlot = selected hotbar slot (0-8)

#include <cstdint>
#include <array>
#include <optional>
#include "inventory/ItemStack.h"

namespace mc {

class Inventory {
public:
    // Main inventory (36 slots: 0-8 hotbar, 9-35 main)
    std::array<std::optional<ItemStack>, 36> mainSlots;

    // Armor (4 slots: 0=boots, 1=legs, 2=chest, 3=helm)
    std::array<std::optional<ItemStack>, 4> armorSlots;

    // Currently selected hotbar slot (0-8)
    int8_t currentSlot = 0;

    // Get the item currently in hand
    std::optional<ItemStack> getHeldItem() const {
        if (currentSlot >= 0 && currentSlot < 9) {
            return mainSlots[currentSlot];
        }
        return std::nullopt;
    }

    // Clear all slots
    void clear() {
        mainSlots.fill(std::nullopt);
        armorSlots.fill(std::nullopt);
    }

    // Set a slot by protocol window-0 slot index
    void setWindowSlot(int16_t windowSlot, const std::optional<ItemStack>& stack) {
        if (windowSlot >= 36 && windowSlot <= 44) {
            // Hotbar: window 36-44 → internal 0-8
            mainSlots[windowSlot - 36] = stack;
        }
        else if (windowSlot >= 9 && windowSlot <= 35) {
            // Main inventory: window 9-35 → internal 9-35
            mainSlots[windowSlot] = stack;
        }
        else if (windowSlot >= 5 && windowSlot <= 8) {
            // Armor: window 5-8 → armor 3,2,1,0 (helm=5→3, chest=6→2, legs=7→1, boots=8→0)
            armorSlots[8 - windowSlot] = stack;
        }
        // Crafting slots (0-4) are not persisted
    }

    // Get a slot by protocol window-0 slot index
    std::optional<ItemStack> getWindowSlot(int16_t windowSlot) const {
        if (windowSlot >= 36 && windowSlot <= 44) {
            return mainSlots[windowSlot - 36];
        }
        else if (windowSlot >= 9 && windowSlot <= 35) {
            return mainSlots[windowSlot];
        }
        else if (windowSlot >= 5 && windowSlot <= 8) {
            return armorSlots[8 - windowSlot];
        }
        return std::nullopt;
    }

    // NBT save — matches yz.b(dh) inventory writing
    void saveToNBT(nbt::NBTTagCompound& tag) const {
        auto invList = std::make_shared<nbt::NBTTagList>();
        for (int i = 0; i < 36; ++i) {
            if (mainSlots[i] && !mainSlots[i]->isEmpty()) {
                auto slotTag = mainSlots[i]->saveToNBT();
                slotTag->setByte("Slot", static_cast<int8_t>(i));
                invList->add(slotTag);
            }
        }
        for (int i = 0; i < 4; ++i) {
            if (armorSlots[i] && !armorSlots[i]->isEmpty()) {
                auto slotTag = armorSlots[i]->saveToNBT();
                slotTag->setByte("Slot", static_cast<int8_t>(100 + i));
                invList->add(slotTag);
            }
        }
        tag.setList("Inventory", invList);
        tag.setInt("SelectedItemSlot", currentSlot);
    }

    // NBT load — matches yz.a(dh) inventory reading
    void loadFromNBT(const nbt::NBTTagCompound& tag) {
        clear();
        if (tag.hasKey("Inventory")) {
            auto invList = tag.getList("Inventory", nbt::TAG_Compound);
            for (size_t i = 0; i < invList->tags.size(); ++i) {
                auto slotTag = std::dynamic_pointer_cast<nbt::NBTTagCompound>(invList->tags[i]);
                if (!slotTag) continue;
                int8_t slot = slotTag->getByte("Slot");
                auto stack = ItemStack::loadFromNBT(*slotTag);
                if (slot >= 0 && slot < 36) {
                    mainSlots[slot] = stack;
                } else if (slot >= 100 && slot < 104) {
                    armorSlots[slot - 100] = stack;
                }
            }
        }
        if (tag.hasKey("SelectedItemSlot")) {
            currentSlot = static_cast<int8_t>(tag.getInt("SelectedItemSlot"));
        }
    }

    // Write all 45 window slots for Window Items packet
    void writeAllSlots(PacketBuffer& buf) const {
        // Slot 0: crafting output (empty)
        ItemStack().writeToPacket(buf);

        // Slots 1-4: crafting grid (empty)
        for (int i = 0; i < 4; ++i) {
            ItemStack().writeToPacket(buf);
        }

        // Slots 5-8: armor (helm, chest, legs, boots)
        for (int i = 3; i >= 0; --i) {
            if (armorSlots[i]) {
                armorSlots[i]->writeToPacket(buf);
            } else {
                ItemStack().writeToPacket(buf);
            }
        }

        // Slots 9-35: main inventory
        for (int i = 9; i < 36; ++i) {
            if (mainSlots[i]) {
                mainSlots[i]->writeToPacket(buf);
            } else {
                ItemStack().writeToPacket(buf);
            }
        }

        // Slots 36-44: hotbar
        for (int i = 0; i < 9; ++i) {
            if (mainSlots[i]) {
                mainSlots[i]->writeToPacket(buf);
            } else {
                ItemStack().writeToPacket(buf);
            }
        }
    }
};

} // namespace mc
