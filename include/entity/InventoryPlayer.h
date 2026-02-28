/**
 * InventoryPlayer.h — Player inventory system.
 *
 * Java reference: net.minecraft.entity.player.InventoryPlayer
 *
 * Layout:
 *   mainInventory[36]: slots 0-8 hotbar, 9-35 main inventory
 *   armorInventory[4]: boots(0), leggings(1), chestplate(2), helmet(3)
 *   currentItem: hotbar selection index (0-8)
 *   itemStack: cursor held item (drag item in GUI)
 *
 * NBT format:
 *   Slot byte 0-35  → mainInventory
 *   Slot byte 100-103 → armorInventory
 *
 * Thread safety: Per-player, accessed from server thread only.
 *
 * JNI readiness: Fixed array sizes, simple struct layout.
 */
#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// InventorySlot — Represents one item stack in inventory.
// Java reference: net.minecraft.item.ItemStack (simplified for inventory use)
// ═══════════════════════════════════════════════════════════════════════════

struct InvSlot {
    int32_t itemId = 0;       // Item ID (0 = empty)
    int32_t stackSize = 0;    // Stack count
    int32_t metadata = 0;     // Damage/metadata value
    int32_t animationsToGo = 0; // Java: animationsToGo (pickup animation timer)

    bool isEmpty() const { return itemId == 0 || stackSize <= 0; }
    void clear() { itemId = 0; stackSize = 0; metadata = 0; animationsToGo = 0; }

    // Java: getMaxStackSize — depends on item, default 64
    int32_t getMaxStackSize() const {
        // Tools/weapons/armor max at 1, ender pearls at 16, etc.
        // Simplified: return 64 for most, specific items override
        return 64;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// InventoryPlayer — Per-player inventory.
// Java reference: net.minecraft.entity.player.InventoryPlayer
// ═══════════════════════════════════════════════════════════════════════════

class InventoryPlayer {
public:
    // Java: mainInventory = new ItemStack[36]
    static constexpr int32_t MAIN_SIZE = 36;
    // Java: armorInventory = new ItemStack[4]
    static constexpr int32_t ARMOR_SIZE = 4;
    // Java: getHotbarSize() = 9
    static constexpr int32_t HOTBAR_SIZE = 9;
    // Java: getInventoryStackLimit() = 64
    static constexpr int32_t STACK_LIMIT = 64;
    // Java: getSizeInventory() = mainInventory.length + 4
    static constexpr int32_t TOTAL_SIZE = MAIN_SIZE + ARMOR_SIZE;

    InvSlot mainInventory[MAIN_SIZE] = {};
    InvSlot armorInventory[ARMOR_SIZE] = {};

    // Java: currentItem — hotbar selection (0-8)
    int32_t currentItem = 0;

    // Java: itemStack — cursor/drag item
    InvSlot cursorItem;

    // Java: inventoryChanged
    bool inventoryChanged = false;

    // ─── Getters ───

    // Java: getCurrentItem
    const InvSlot* getCurrentItem() const {
        if (currentItem >= 0 && currentItem < HOTBAR_SIZE) {
            return mainInventory[currentItem].isEmpty() ? nullptr : &mainInventory[currentItem];
        }
        return nullptr;
    }

    // Java: getStackInSlot — unified view (0-35 main, 36-39 armor)
    const InvSlot* getStackInSlot(int32_t slot) const {
        if (slot < MAIN_SIZE) {
            return mainInventory[slot].isEmpty() ? nullptr : &mainInventory[slot];
        }
        int32_t armorSlot = slot - MAIN_SIZE;
        if (armorSlot >= 0 && armorSlot < ARMOR_SIZE) {
            return armorInventory[armorSlot].isEmpty() ? nullptr : &armorInventory[armorSlot];
        }
        return nullptr;
    }

    // Java: setInventorySlotContents
    void setInventorySlotContents(int32_t slot, const InvSlot& item) {
        if (slot < MAIN_SIZE) {
            mainInventory[slot] = item;
        } else {
            int32_t armorSlot = slot - MAIN_SIZE;
            if (armorSlot >= 0 && armorSlot < ARMOR_SIZE) {
                armorInventory[armorSlot] = item;
            }
        }
    }

    // Java: armorItemInSlot
    const InvSlot* armorItemInSlot(int32_t slot) const {
        if (slot >= 0 && slot < ARMOR_SIZE) {
            return armorInventory[slot].isEmpty() ? nullptr : &armorInventory[slot];
        }
        return nullptr;
    }

    // ─── Slot search ───

    // Java: getFirstEmptyStack
    int32_t getFirstEmptyStack() const {
        for (int32_t i = 0; i < MAIN_SIZE; ++i) {
            if (mainInventory[i].isEmpty()) return i;
        }
        return -1;
    }

    // Java: getInventorySlotContainItem
    int32_t findItemSlot(int32_t itemId) const {
        for (int32_t i = 0; i < MAIN_SIZE; ++i) {
            if (!mainInventory[i].isEmpty() && mainInventory[i].itemId == itemId) return i;
        }
        return -1;
    }

    // Java: storeItemStack — find stackable slot
    int32_t findStackableSlot(const InvSlot& item) const {
        for (int32_t i = 0; i < MAIN_SIZE; ++i) {
            if (!mainInventory[i].isEmpty() &&
                mainInventory[i].itemId == item.itemId &&
                mainInventory[i].metadata == item.metadata &&
                mainInventory[i].stackSize < mainInventory[i].getMaxStackSize() &&
                mainInventory[i].stackSize < STACK_LIMIT) {
                return i;
            }
        }
        return -1;
    }

    // ─── Item operations ───

    // Java: addItemStackToInventory — returns true if any items were stored
    bool addItemStackToInventory(InvSlot& item) {
        if (item.isEmpty()) return false;

        // Try to stack with existing
        while (item.stackSize > 0) {
            int32_t slot = findStackableSlot(item);
            if (slot < 0) {
                slot = getFirstEmptyStack();
            }
            if (slot < 0) return false;  // Full

            if (mainInventory[slot].isEmpty()) {
                mainInventory[slot] = item;
                mainInventory[slot].stackSize = 0;
            }

            int32_t space = mainInventory[slot].getMaxStackSize() - mainInventory[slot].stackSize;
            if (space > STACK_LIMIT - mainInventory[slot].stackSize) {
                space = STACK_LIMIT - mainInventory[slot].stackSize;
            }
            int32_t toMove = std::min(item.stackSize, space);
            if (toMove <= 0) return false;

            mainInventory[slot].stackSize += toMove;
            mainInventory[slot].animationsToGo = 5;
            item.stackSize -= toMove;
        }
        inventoryChanged = true;
        return true;
    }

    // Java: consumeInventoryItem
    bool consumeInventoryItem(int32_t itemId) {
        int32_t slot = findItemSlot(itemId);
        if (slot < 0) return false;
        if (--mainInventory[slot].stackSize <= 0) {
            mainInventory[slot].clear();
        }
        inventoryChanged = true;
        return true;
    }

    // Java: hasItem
    bool hasItem(int32_t itemId) const {
        return findItemSlot(itemId) >= 0;
    }

    // ─── Armor ───

    // Java: getTotalArmorValue — sum of damageReduceAmount for armor items
    // Armor values: leather=1/1/3/1, chain/iron=1/2/5/2, gold=1/3/5/2, diamond=3/6/8/3
    int32_t getTotalArmorValue() const;

    // Java: damageArmor(float damage)
    // damage /= 4, min 1
    void damageArmor(float damage);

    // ─── Bulk operations ───

    // Java: clearInventory
    int32_t clearInventory(int32_t itemId = -1, int32_t metadata = -1);

    // Java: dropAllItems — clears all slots (actual dropping handled by caller)
    void dropAllItems();

    // Java: copyInventory
    void copyInventory(const InventoryPlayer& source);

    // Java: markDirty
    void markDirty() { inventoryChanged = true; }

    // ─── NBT ───

    static constexpr int32_t NBT_ARMOR_OFFSET = 100;  // Armor slot byte offset in NBT
    static constexpr const char* INVENTORY_NAME = "container.inventory";
};

} // namespace mccpp
