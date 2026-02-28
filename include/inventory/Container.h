/**
 * Container.h — Server-side inventory window system.
 *
 * Java references:
 *   - net.minecraft.inventory.Container — Base GUI container
 *   - net.minecraft.inventory.Slot — Single inventory slot
 *   - net.minecraft.inventory.ContainerPlayer — Player inventory (2x2 crafting)
 *   - net.minecraft.inventory.ContainerChest — Chest/double chest
 *   - net.minecraft.inventory.ContainerWorkbench — 3x3 crafting table
 *   - net.minecraft.inventory.ContainerFurnace — Furnace
 *
 * Protocol: Server tracks window ID, slot contents, and broadcasts changes.
 * Click modes: 0=normal, 1=shift-click, 2=number key, 3=creative middle,
 *              4=drop, 5=drag, 6=double-click collect.
 *
 * Thread safety: Each container is owned by one player connection.
 * No shared state — containers are not accessed across threads.
 *
 * JNI readiness: Simple struct layout, virtual methods for override.
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// ItemStack — Simplified item stack (mirrors InvSlot but with copy semantics).
// Java reference: net.minecraft.item.ItemStack
// ═══════════════════════════════════════════════════════════════════════════

struct ItemStack {
    int32_t itemId = 0;
    int32_t stackSize = 0;
    int32_t metadata = 0;
    bool hasSubtypes = false;

    bool isEmpty() const { return itemId <= 0 || stackSize <= 0; }
    bool isStackable() const { return getMaxStackSize() > 1; }

    int32_t getMaxStackSize() const {
        // Java: Item.getItemStackLimit — default 64, some items 1 or 16
        return 64; // Simplified; actual limit from item registry
    }

    bool isItemEqual(const ItemStack& other) const {
        return itemId == other.itemId &&
               (!hasSubtypes || metadata == other.metadata);
    }

    // Java: areItemStackTagsEqual — simplified (no NBT comparison yet)
    static bool areTagsEqual(const ItemStack& a, const ItemStack& b) {
        return true; // NBT comparison placeholder
    }

    ItemStack copy() const { return *this; }

    ItemStack splitStack(int32_t amount) {
        int32_t taken = std::min(amount, stackSize);
        ItemStack result = *this;
        result.stackSize = taken;
        stackSize -= taken;
        return result;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Slot — Single slot in a container.
// Java reference: net.minecraft.inventory.Slot
// ═══════════════════════════════════════════════════════════════════════════

struct Slot {
    int32_t slotNumber = 0;     // Index within the container
    int32_t invIndex = 0;       // Index within the backing inventory
    std::optional<ItemStack> stack;

    // Java: getStack
    ItemStack* getStack() {
        return stack.has_value() && !stack->isEmpty() ? &stack.value() : nullptr;
    }

    const ItemStack* getStack() const {
        return stack.has_value() && !stack->isEmpty() ? &stack.value() : nullptr;
    }

    bool getHasStack() const {
        return stack.has_value() && !stack->isEmpty();
    }

    // Java: putStack
    void putStack(const ItemStack* item) {
        if (item && !item->isEmpty()) {
            stack = *item;
        } else {
            stack = std::nullopt;
        }
        onSlotChanged();
    }

    void putStack(const ItemStack& item) {
        if (!item.isEmpty()) {
            stack = item;
        } else {
            stack = std::nullopt;
        }
        onSlotChanged();
    }

    // Java: decrStackSize
    ItemStack decrStackSize(int32_t amount) {
        if (!getHasStack()) return {};
        ItemStack result;
        if (stack->stackSize <= amount) {
            result = *stack;
            stack = std::nullopt;
        } else {
            result = stack->splitStack(amount);
            if (stack->stackSize <= 0) stack = std::nullopt;
        }
        onSlotChanged();
        return result;
    }

    // Java: getSlotStackLimit — most slots allow 64
    virtual int32_t getSlotStackLimit() const { return 64; }

    // Java: isItemValid — can this item be placed here?
    virtual bool isItemValid(const ItemStack& /*item*/) const { return true; }

    // Java: canTakeStack
    virtual bool canTakeStack() const { return true; }

    // Java: onSlotChanged
    virtual void onSlotChanged() {}

    // Java: onPickupFromSlot
    virtual void onPickupFromSlot(const ItemStack& /*stack*/) {}

    virtual ~Slot() = default;
};

// ═══════════════════════════════════════════════════════════════════════════
// Container — Base class for all inventory GUIs.
// Java reference: net.minecraft.inventory.Container
// ═══════════════════════════════════════════════════════════════════════════

class Container {
public:
    int32_t windowId = 0;
    std::vector<Slot> slots;
    std::vector<std::optional<ItemStack>> prevStacks; // For change detection

    virtual ~Container() = default;

    // ─── Slot management ───

    // Java: addSlotToContainer
    Slot& addSlot(Slot slot) {
        slot.slotNumber = static_cast<int32_t>(slots.size());
        slots.push_back(std::move(slot));
        prevStacks.push_back(std::nullopt);
        return slots.back();
    }

    Slot* getSlot(int32_t index) {
        if (index >= 0 && index < static_cast<int32_t>(slots.size())) {
            return &slots[static_cast<size_t>(index)];
        }
        return nullptr;
    }

    // Java: getInventory — snapshot of all slot contents
    std::vector<std::optional<ItemStack>> getInventory() const {
        std::vector<std::optional<ItemStack>> inv;
        inv.reserve(slots.size());
        for (const auto& slot : slots) {
            if (slot.getHasStack()) {
                inv.push_back(*slot.getStack());
            } else {
                inv.push_back(std::nullopt);
            }
        }
        return inv;
    }

    // ─── Change detection ───

    // Java: detectAndSendChanges
    // Returns indices of changed slots.
    std::vector<int32_t> detectChanges() {
        std::vector<int32_t> changed;
        for (size_t i = 0; i < slots.size(); ++i) {
            const ItemStack* current = slots[i].getStack();
            bool prevEmpty = !prevStacks[i].has_value() || prevStacks[i]->isEmpty();
            bool currEmpty = (current == nullptr);

            bool differ = false;
            if (prevEmpty != currEmpty) {
                differ = true;
            } else if (!prevEmpty && !currEmpty) {
                // Both non-empty — compare
                differ = (prevStacks[i]->itemId != current->itemId ||
                          prevStacks[i]->stackSize != current->stackSize ||
                          prevStacks[i]->metadata != current->metadata);
            }

            if (differ) {
                prevStacks[i] = currEmpty ? std::nullopt : std::optional<ItemStack>(*current);
                changed.push_back(static_cast<int32_t>(i));
            }
        }
        return changed;
    }

    // ─── Click handling ───

    // Cursor item — the item being held by the mouse cursor
    std::optional<ItemStack> cursorItem;

    // Drag state (mode 5)
    int32_t dragMode_ = -1;
    int32_t dragEvent_ = 0;
    std::unordered_set<int32_t> dragSlots_;

    // Java: slotClick — handle all click modes
    // mode: 0=normal, 1=shift, 2=numkey, 3=creative_middle, 4=drop, 5=drag, 6=dblclick
    // button: 0=left, 1=right (for modes 0,4); hotbar slot (for mode 2)
    std::optional<ItemStack> slotClick(int32_t slotIdx, int32_t button,
                                        int32_t mode, bool isCreative) {
        std::optional<ItemStack> result;

        // Mode 5: Drag
        if (mode == 5) {
            handleDrag(slotIdx, button, isCreative);
            return result;
        }

        if (dragEvent_ != 0) {
            resetDrag();
        }

        // Mode 0/1: Normal click / Shift-click
        if ((mode == 0 || mode == 1) && (button == 0 || button == 1)) {
            if (slotIdx == -999) {
                // Click outside — drop cursor item
                if (cursorItem.has_value()) {
                    if (button == 0) {
                        // Drop all
                        cursorItem = std::nullopt;
                    } else {
                        // Drop one
                        cursorItem->stackSize -= 1;
                        if (cursorItem->stackSize <= 0) cursorItem = std::nullopt;
                    }
                }
                return result;
            }

            if (mode == 1) {
                // Shift-click: transferStackInSlot
                if (slotIdx < 0) return std::nullopt;
                Slot* slot = getSlot(slotIdx);
                if (slot && slot->canTakeStack() && slot->getHasStack()) {
                    result = slot->getStack()->copy();
                    transferStackInSlot(slotIdx);
                }
                return result;
            }

            // Normal click (mode 0)
            if (slotIdx < 0) return std::nullopt;
            Slot* slot = getSlot(slotIdx);
            if (!slot) return std::nullopt;

            ItemStack* slotStack = slot->getStack();

            if (slotStack) result = slotStack->copy();

            if (!slotStack) {
                // Empty slot — place cursor item
                if (cursorItem.has_value() && slot->isItemValid(*cursorItem)) {
                    int32_t placeCount = (button == 0) ? cursorItem->stackSize : 1;
                    placeCount = std::min(placeCount, slot->getSlotStackLimit());
                    ItemStack placed = cursorItem->splitStack(placeCount);
                    slot->putStack(placed);
                    if (cursorItem->stackSize <= 0) cursorItem = std::nullopt;
                }
            } else if (slot->canTakeStack()) {
                if (!cursorItem.has_value()) {
                    // Pick up from slot
                    int32_t pickCount = (button == 0) ? slotStack->stackSize
                                                       : (slotStack->stackSize + 1) / 2;
                    ItemStack picked = slot->decrStackSize(pickCount);
                    cursorItem = picked;
                    if (!slot->getHasStack()) slot->putStack(nullptr);
                } else if (slot->isItemValid(*cursorItem)) {
                    // Same item — stack
                    if (slotStack->isItemEqual(*cursorItem) &&
                        ItemStack::areTagsEqual(*slotStack, *cursorItem)) {
                        int32_t addCount = (button == 0) ? cursorItem->stackSize : 1;
                        addCount = std::min(addCount, slot->getSlotStackLimit() - slotStack->stackSize);
                        addCount = std::min(addCount, cursorItem->getMaxStackSize() - slotStack->stackSize);
                        cursorItem->stackSize -= addCount;
                        if (cursorItem->stackSize <= 0) cursorItem = std::nullopt;
                        slotStack->stackSize += addCount;
                    } else if (cursorItem->stackSize <= slot->getSlotStackLimit()) {
                        // Swap
                        ItemStack temp = *slotStack;
                        slot->putStack(*cursorItem);
                        cursorItem = temp;
                    }
                }
            }
            slot->onSlotChanged();
            return result;
        }

        // Mode 2: Number key swap (button = hotbar slot 0-8)
        if (mode == 2 && button >= 0 && button < 9) {
            Slot* slot = getSlot(slotIdx);
            if (slot && slot->canTakeStack()) {
                // Swap slot with hotbar[button]
                // Simplified: just pick up and swap
                numberKeySwap(slotIdx, button);
            }
            return result;
        }

        // Mode 3: Creative middle-click — full stack copy
        if (mode == 3 && isCreative && !cursorItem.has_value() && slotIdx >= 0) {
            Slot* slot = getSlot(slotIdx);
            if (slot && slot->getHasStack()) {
                ItemStack copy = slot->getStack()->copy();
                copy.stackSize = copy.getMaxStackSize();
                cursorItem = copy;
            }
            return result;
        }

        // Mode 4: Drop
        if (mode == 4 && !cursorItem.has_value() && slotIdx >= 0) {
            Slot* slot = getSlot(slotIdx);
            if (slot && slot->getHasStack() && slot->canTakeStack()) {
                int32_t dropCount = (button == 0) ? 1 : slot->getStack()->stackSize;
                slot->decrStackSize(dropCount);
                // Dropped item would be spawned in world
            }
            return result;
        }

        // Mode 6: Double-click collect
        if (mode == 6 && slotIdx >= 0 && cursorItem.has_value()) {
            ItemStack& cursor = *cursorItem;
            for (size_t i = 0; i < slots.size() && cursor.stackSize < cursor.getMaxStackSize(); ++i) {
                Slot& s = slots[i];
                if (!s.getHasStack() || !s.canTakeStack()) continue;
                if (!cursor.isItemEqual(*s.getStack())) continue;
                if (!ItemStack::areTagsEqual(cursor, *s.getStack())) continue;

                int32_t take = std::min(cursor.getMaxStackSize() - cursor.stackSize,
                                         s.getStack()->stackSize);
                s.decrStackSize(take);
                cursor.stackSize += take;
                if (!s.getHasStack()) s.putStack(nullptr);
            }
            return result;
        }

        return result;
    }

    // ─── Merging ───

    // Java: mergeItemStack — try to merge item into slot range
    bool mergeItemStack(ItemStack& item, int32_t startSlot, int32_t endSlot, bool reverse) {
        bool merged = false;
        int32_t idx = reverse ? endSlot - 1 : startSlot;

        // First pass: merge into existing stacks
        if (item.isStackable()) {
            while (item.stackSize > 0 && (reverse ? idx >= startSlot : idx < endSlot)) {
                Slot* slot = getSlot(idx);
                ItemStack* existing = slot ? slot->getStack() : nullptr;
                if (existing && existing->isItemEqual(item) &&
                    ItemStack::areTagsEqual(*existing, item)) {
                    int32_t total = existing->stackSize + item.stackSize;
                    if (total <= item.getMaxStackSize()) {
                        item.stackSize = 0;
                        existing->stackSize = total;
                        slot->onSlotChanged();
                        merged = true;
                    } else if (existing->stackSize < item.getMaxStackSize()) {
                        item.stackSize -= item.getMaxStackSize() - existing->stackSize;
                        existing->stackSize = item.getMaxStackSize();
                        slot->onSlotChanged();
                        merged = true;
                    }
                }
                idx += reverse ? -1 : 1;
            }
        }

        // Second pass: place into empty slots
        if (item.stackSize > 0) {
            idx = reverse ? endSlot - 1 : startSlot;
            while (reverse ? idx >= startSlot : idx < endSlot) {
                Slot* slot = getSlot(idx);
                if (slot && !slot->getHasStack()) {
                    slot->putStack(item.copy());
                    slot->onSlotChanged();
                    item.stackSize = 0;
                    merged = true;
                    break;
                }
                idx += reverse ? -1 : 1;
            }
        }

        return merged;
    }

    // ─── Virtual overrides ───

    // Java: transferStackInSlot — shift-click behavior (subclass overrides)
    virtual void transferStackInSlot(int32_t /*slotIdx*/) {}

    // Java: canInteractWith
    virtual bool canInteractWith() const { return true; }

    // Java: onContainerClosed
    virtual void onContainerClosed() {
        if (cursorItem.has_value()) {
            // Drop cursor item
            cursorItem = std::nullopt;
        }
    }

    // Java: numberKeySwap — swap slot with hotbar
    virtual void numberKeySwap(int32_t /*slotIdx*/, int32_t /*hotbar*/) {}

    // ─── Static helpers ───

    // Java: extractDragMode — (button >> 2) & 3
    static int32_t extractDragMode(int32_t button) { return (button >> 2) & 3; }

    // Java: getDragEvent — button & 3
    static int32_t getDragEvent(int32_t button) { return button & 3; }

    // Java: isValidDragMode
    static bool isValidDragMode(int32_t mode) { return mode == 0 || mode == 1; }

    // Java: canAddItemToSlot
    static bool canAddItemToSlot(const Slot& slot, const ItemStack& item, bool ignoreStackSize) {
        bool canAdd = !slot.getHasStack();
        if (slot.getHasStack() && slot.getStack()->isItemEqual(item) &&
            ItemStack::areTagsEqual(*slot.getStack(), item)) {
            canAdd = canAdd || (slot.getStack()->stackSize +
                                (ignoreStackSize ? 0 : item.stackSize) <= item.getMaxStackSize());
        }
        return canAdd;
    }

    // Java: computeStackSize
    static void computeStackSize(size_t slotCount, int32_t dragMode,
                                   ItemStack& item, int32_t existingCount) {
        switch (dragMode) {
            case 0: // Split evenly
                item.stackSize = static_cast<int32_t>(
                    static_cast<float>(item.stackSize) / static_cast<float>(slotCount));
                break;
            case 1: // One each
                item.stackSize = 1;
                break;
        }
        item.stackSize += existingCount;
    }

    // Java: calcRedstoneFromInventory — comparator output
    static int32_t calcRedstoneOutput(const std::vector<Slot>& inv, int32_t stackLimit = 64) {
        if (inv.empty()) return 0;
        int32_t filledSlots = 0;
        float fillRatio = 0.0f;
        for (const auto& slot : inv) {
            if (!slot.getHasStack()) continue;
            float maxForSlot = static_cast<float>(std::min(stackLimit, slot.getStack()->getMaxStackSize()));
            fillRatio += static_cast<float>(slot.getStack()->stackSize) / maxForSlot;
            ++filledSlots;
        }
        fillRatio /= static_cast<float>(inv.size());
        return static_cast<int32_t>(fillRatio * 14.0f) + (filledSlots > 0 ? 1 : 0);
    }

private:
    void resetDrag() {
        dragEvent_ = 0;
        dragSlots_.clear();
    }

    void handleDrag(int32_t slotIdx, int32_t button, bool /*isCreative*/) {
        int32_t prevEvent = dragEvent_;
        dragEvent_ = getDragEvent(button);

        if ((prevEvent != 1 || dragEvent_ != 2) && prevEvent != dragEvent_) {
            resetDrag();
            return;
        }
        if (!cursorItem.has_value()) {
            resetDrag();
            return;
        }

        if (dragEvent_ == 0) {
            // Start drag
            dragMode_ = extractDragMode(button);
            if (isValidDragMode(dragMode_)) {
                dragEvent_ = 1;
                dragSlots_.clear();
            } else {
                resetDrag();
            }
        } else if (dragEvent_ == 1) {
            // Add slot to drag
            Slot* slot = getSlot(slotIdx);
            if (slot && canAddItemToSlot(*slot, *cursorItem, true) &&
                slot->isItemValid(*cursorItem) &&
                cursorItem->stackSize > static_cast<int32_t>(dragSlots_.size())) {
                dragSlots_.insert(slotIdx);
            }
        } else if (dragEvent_ == 2) {
            // Finalize drag
            if (!dragSlots_.empty()) {
                ItemStack source = cursorItem->copy();
                int32_t remaining = cursorItem->stackSize;

                for (int32_t idx : dragSlots_) {
                    Slot* slot = getSlot(idx);
                    if (!slot || !canAddItemToSlot(*slot, *cursorItem, true) ||
                        !slot->isItemValid(*cursorItem)) continue;

                    ItemStack placed = source.copy();
                    int32_t existing = slot->getHasStack() ? slot->getStack()->stackSize : 0;
                    computeStackSize(dragSlots_.size(), dragMode_, placed, existing);
                    placed.stackSize = std::min(placed.stackSize, placed.getMaxStackSize());
                    placed.stackSize = std::min(placed.stackSize, slot->getSlotStackLimit());
                    remaining -= (placed.stackSize - existing);
                    slot->putStack(placed);
                }

                if (remaining > 0) {
                    cursorItem->stackSize = remaining;
                } else {
                    cursorItem = std::nullopt;
                }
            }
            resetDrag();
        } else {
            resetDrag();
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// ContainerPlayer — Player inventory container (2×2 crafting).
// Java reference: net.minecraft.inventory.ContainerPlayer
// Slots: 0=craft output, 1-4=craft grid, 5-8=armor, 9-35=main, 36-44=hotbar
// ═══════════════════════════════════════════════════════════════════════════

class ContainerPlayer : public Container {
public:
    static constexpr int32_t CRAFT_OUTPUT = 0;
    static constexpr int32_t CRAFT_START = 1;
    static constexpr int32_t CRAFT_END = 4;
    static constexpr int32_t ARMOR_START = 5;
    static constexpr int32_t ARMOR_END = 8;
    static constexpr int32_t MAIN_START = 9;
    static constexpr int32_t MAIN_END = 35;
    static constexpr int32_t HOTBAR_START = 36;
    static constexpr int32_t HOTBAR_END = 44;
    static constexpr int32_t TOTAL_SLOTS = 45;

    ContainerPlayer() {
        windowId = 0; // Player inventory is always window 0
        // Add all 45 slots
        for (int32_t i = 0; i < TOTAL_SLOTS; ++i) {
            Slot s;
            s.invIndex = i;
            addSlot(std::move(s));
        }
    }

    void transferStackInSlot(int32_t slotIdx) override {
        Slot* slot = getSlot(slotIdx);
        if (!slot || !slot->getHasStack()) return;

        ItemStack item = *slot->getStack();

        if (slotIdx >= MAIN_START && slotIdx <= HOTBAR_END) {
            // Main/hotbar → armor (if armor item) or crafting grid
            // Simplified: just move between main and hotbar
            if (slotIdx >= HOTBAR_START) {
                mergeItemStack(item, MAIN_START, MAIN_END + 1, false);
            } else {
                mergeItemStack(item, HOTBAR_START, HOTBAR_END + 1, false);
            }
        }

        if (item.stackSize <= 0) {
            slot->putStack(nullptr);
        } else {
            slot->stack = item;
            slot->onSlotChanged();
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// ContainerChest — Chest container.
// Java reference: net.minecraft.inventory.ContainerChest
// Single chest: 27 slots + 36 player slots = 63 total
// Double chest: 54 slots + 36 player slots = 90 total
// ═══════════════════════════════════════════════════════════════════════════

class ContainerChest : public Container {
public:
    int32_t chestSize;

    explicit ContainerChest(int32_t numRows = 3) : chestSize(numRows * 9) {
        // Chest slots
        for (int32_t i = 0; i < chestSize; ++i) {
            Slot s;
            s.invIndex = i;
            addSlot(std::move(s));
        }
        // Player main inventory (9-35)
        for (int32_t i = 0; i < 27; ++i) {
            Slot s;
            s.invIndex = 9 + i;
            addSlot(std::move(s));
        }
        // Player hotbar (0-8)
        for (int32_t i = 0; i < 9; ++i) {
            Slot s;
            s.invIndex = i;
            addSlot(std::move(s));
        }
    }

    void transferStackInSlot(int32_t slotIdx) override {
        Slot* slot = getSlot(slotIdx);
        if (!slot || !slot->getHasStack()) return;

        ItemStack item = *slot->getStack();
        int32_t playerStart = chestSize;
        int32_t playerEnd = chestSize + 36;

        if (slotIdx < chestSize) {
            // Chest → player
            mergeItemStack(item, playerStart, playerEnd, true);
        } else {
            // Player → chest
            mergeItemStack(item, 0, chestSize, false);
        }

        if (item.stackSize <= 0) {
            slot->putStack(nullptr);
        } else {
            slot->stack = item;
            slot->onSlotChanged();
        }
    }
};

} // namespace mccpp
