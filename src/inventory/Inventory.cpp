/**
 * Inventory.cpp — Inventory system implementation.
 *
 * Java references:
 *   net.minecraft.item.ItemStack
 *   net.minecraft.inventory.Slot
 *   net.minecraft.entity.player.InventoryPlayer
 *   net.minecraft.inventory.Container
 *   net.minecraft.inventory.ContainerPlayer
 *
 * Key behaviors preserved from Java:
 *   - InventoryPlayer slot layout: 0-8 hotbar, 9-35 main, 36-39 armor
 *   - addItemStackToInventory: tries existing stacks first, then empty slots
 *   - ContainerPlayer slot numbering: 0 craft output, 1-4 craft grid,
 *     5-8 armor, 9-35 main, 36-44 hotbar
 */

#include "inventory/Inventory.h"

#include <algorithm>

namespace mccpp {

// ═════════════════════════════════════════════════════════════════════════════
// Slot
// ═════════════════════════════════════════════════════════════════════════════

std::optional<ItemStack> Slot::getStack() const {
    return inventory_->getStackInSlot(slotIndex_);
}

void Slot::putStack(const std::optional<ItemStack>& stack) {
    inventory_->setInventorySlotContents(slotIndex_, stack);
    onSlotChanged();
}

std::optional<ItemStack> Slot::decrStackSize(int32_t amount) {
    return inventory_->decrStackSize(slotIndex_, amount);
}

void Slot::onSlotChanged() {
    inventory_->markDirty();
}

// ═════════════════════════════════════════════════════════════════════════════
// InventoryPlayer
// ═════════════════════════════════════════════════════════════════════════════

std::optional<ItemStack> InventoryPlayer::decrStackSize(int32_t index, int32_t count) {
    // Java: InventoryPlayer.decrStackSize(int, int)
    std::optional<ItemStack>* target = nullptr;

    if (index >= 0 && index < MAIN_SIZE) {
        target = &mainInventory_[index];
    } else if (index >= MAIN_SIZE && index < TOTAL_SIZE) {
        target = &armorInventory_[index - MAIN_SIZE];
    }

    if (!target || !target->has_value()) return std::nullopt;

    auto& stack = target->value();
    if (stack.getStackSize() <= count) {
        auto result = *target;
        *target = std::nullopt;
        return result;
    } else {
        auto split = stack.splitStack(count);
        if (stack.getStackSize() <= 0) {
            *target = std::nullopt;
        }
        return split;
    }
}

std::optional<ItemStack> InventoryPlayer::getStackInSlotOnClosing(int32_t index) {
    // Java: InventoryPlayer.getStackInSlotOnClosing(int)
    std::optional<ItemStack>* target = nullptr;

    if (index >= 0 && index < MAIN_SIZE) {
        target = &mainInventory_[index];
    } else if (index >= MAIN_SIZE && index < TOTAL_SIZE) {
        target = &armorInventory_[index - MAIN_SIZE];
    }

    if (!target || !target->has_value()) return std::nullopt;

    auto result = *target;
    *target = std::nullopt;
    return result;
}

void InventoryPlayer::setInventorySlotContents(int32_t index, const std::optional<ItemStack>& stack) {
    // Java: InventoryPlayer.setInventorySlotContents(int, ItemStack)
    if (index >= 0 && index < MAIN_SIZE) {
        mainInventory_[index] = stack;
    } else if (index >= MAIN_SIZE && index < TOTAL_SIZE) {
        armorInventory_[index - MAIN_SIZE] = stack;
    }
}

int32_t InventoryPlayer::getFirstEmptyStack() const {
    // Java: InventoryPlayer.getFirstEmptyStack()
    for (int32_t i = 0; i < MAIN_SIZE; ++i) {
        if (!mainInventory_[i].has_value()) {
            return i;
        }
    }
    return -1;
}

bool InventoryPlayer::addItemStackToInventory(ItemStack& stack) {
    // Java: InventoryPlayer.addItemStackToInventory(ItemStack)

    if (stack.isEmpty()) return false;

    // Try to merge with existing stacks first
    // Java: storePartialItemStack() logic
    if (stack.isStackable()) {
        for (int32_t i = 0; i < MAIN_SIZE && stack.getStackSize() > 0; ++i) {
            auto& slot = mainInventory_[i];
            if (slot.has_value() && slot->isItemEqual(stack)) {
                int32_t maxStack = slot->getMaxStackSize();
                int32_t canFit = maxStack - slot->getStackSize();
                if (canFit > 0) {
                    int32_t toMove = std::min(canFit, stack.getStackSize());
                    slot->setStackSize(slot->getStackSize() + toMove);
                    stack.setStackSize(stack.getStackSize() - toMove);
                }
            }
        }
    }

    // Try to place remaining in empty slot
    if (stack.getStackSize() > 0) {
        int32_t emptySlot = getFirstEmptyStack();
        if (emptySlot >= 0) {
            mainInventory_[emptySlot] = stack;
            stack.setStackSize(0);
        }
    }

    return stack.getStackSize() <= 0;
}

// ═════════════════════════════════════════════════════════════════════════════
// Container
// ═════════════════════════════════════════════════════════════════════════════

void Container::detectAndSendChanges() {
    // Java: Container.detectAndSendChanges()
    // Compares current slot contents with tracked stacks, sends updates
    for (int32_t i = 0; i < static_cast<int32_t>(slots_.size()); ++i) {
        auto current = slots_[i]->getStack();
        auto& tracked = trackedStacks_[i];

        bool changed = false;
        if (current.has_value() != tracked.has_value()) {
            changed = true;
        } else if (current.has_value() && tracked.has_value()) {
            changed = !current->isStackEqual(tracked.value());
        }

        if (changed) {
            tracked = current.has_value()
                ? std::optional<ItemStack>(current->copy())
                : std::nullopt;
            // In full implementation, would send S2FPacketSetSlot to crafters
        }
    }
}

void Container::putStackInSlot(int32_t index, const std::optional<ItemStack>& stack) {
    // Java: Container.putStackInSlot(int, ItemStack)
    if (auto* slot = getSlot(index)) {
        slot->putStack(stack);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// ContainerPlayer
// ═════════════════════════════════════════════════════════════════════════════

// Helper: Armor slot that only accepts armor items in the correct slot
class ArmorSlot : public Slot {
public:
    ArmorSlot(IInventory* inv, int32_t index, int32_t x, int32_t y, int32_t armorSlot)
        : Slot(inv, index, x, y), armorSlot_(armorSlot) {}

    // Java: SlotArmor.isItemValid(ItemStack) — only accepts armor for this slot
    bool isItemValid(const ItemStack& /*stack*/) const override {
        // Simplified: allow any item in armor slots for now
        // Full implementation would check ItemArmor.armorType == armorSlot_
        return true;
    }

    int32_t getSlotStackLimit() const override { return 1; }

private:
    int32_t armorSlot_;
};

ContainerPlayer::ContainerPlayer(InventoryPlayer& playerInventory) {
    // Java: ContainerPlayer constructor
    // Reference: net.minecraft.inventory.ContainerPlayer

    // Slot 0: Crafting output (not backed by a real inventory yet)
    // For now, use playerInventory slot 0 as placeholder
    // In full implementation, this would be InventoryCraftResult

    // Slots 1-4: Crafting grid (2x2)
    // For now, use dummy — will be InventoryCrafting later

    // Slots 5-8: Armor slots
    // Java: for (int i = 0; i < 4; ++i) addSlotToContainer(new SlotArmor(...))
    // Armor slots reference playerInventory armor array (indices 36-39)
    for (int32_t i = 0; i < 4; ++i) {
        addSlotToContainer(std::make_unique<ArmorSlot>(
            &playerInventory,
            InventoryPlayer::MAIN_SIZE + (3 - i),  // Head=39, Chest=38, Legs=37, Feet=36
            8, 8 + i * 18,
            i
        ));
    }

    // Slots 9-35: Main inventory (3 rows of 9)
    // Java: slot indices 9-35 of mainInventory
    for (int32_t row = 0; row < 3; ++row) {
        for (int32_t col = 0; col < 9; ++col) {
            addSlotToContainer(std::make_unique<Slot>(
                &playerInventory,
                col + (row + 1) * 9,  // indices 9-35
                8 + col * 18,
                84 + row * 18
            ));
        }
    }

    // Slots 36-44: Hotbar (1 row of 9)
    // Java: slot indices 0-8 of mainInventory
    for (int32_t col = 0; col < 9; ++col) {
        addSlotToContainer(std::make_unique<Slot>(
            &playerInventory,
            col,  // indices 0-8
            8 + col * 18,
            142
        ));
    }
}

} // namespace mccpp
