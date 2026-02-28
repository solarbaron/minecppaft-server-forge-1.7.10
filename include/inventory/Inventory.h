/**
 * Inventory.h — Inventory system: ItemStack, Slot, IInventory, InventoryPlayer, Container.
 *
 * Java references:
 *   - net.minecraft.item.ItemStack
 *   - net.minecraft.inventory.Slot
 *   - net.minecraft.inventory.IInventory
 *   - net.minecraft.entity.player.InventoryPlayer
 *   - net.minecraft.inventory.Container
 *   - net.minecraft.inventory.ContainerPlayer
 *
 * Thread safety:
 *   - Inventory instances are single-owner (owned by their player entity).
 *   - Container operations happen on the server tick thread.
 *
 * JNI readiness: POD-like ItemStack, integer item IDs match Java's Item.getIdFromItem().
 */
#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// ItemStack — A stack of items.
// Java reference: net.minecraft.item.ItemStack
//
// Uses item ID (int) rather than Item pointer — matches Java's serialization
// format and is JNI-friendly.
// ═══════════════════════════════════════════════════════════════════════════

class ItemStack {
public:
    ItemStack() = default;

    // Java: ItemStack(Item item, int stackSize, int damage)
    explicit ItemStack(int32_t itemId, int32_t count = 1, int32_t damage = 0)
        : itemId_(itemId), stackSize_(count), damage_(damage) {}

    // ─── Getters ───────────────────────────────────────────────────────
    int32_t getItemId() const { return itemId_; }
    int32_t getStackSize() const { return stackSize_; }
    int32_t getDamage() const { return damage_; }
    bool isEmpty() const { return itemId_ == 0 || stackSize_ <= 0; }

    // ─── Setters ───────────────────────────────────────────────────────
    void setItemId(int32_t id) { itemId_ = id; }
    void setStackSize(int32_t count) { stackSize_ = count; }
    void setDamage(int32_t dmg) { damage_ = dmg < 0 ? 0 : dmg; }

    // ─── Stack operations ──────────────────────────────────────────────
    // Java: ItemStack.getMaxStackSize() — delegates to Item; default 64
    int32_t getMaxStackSize() const { return 64; }

    // Java: ItemStack.isStackable()
    bool isStackable() const { return getMaxStackSize() > 1; }

    // Java: ItemStack.splitStack(int)
    ItemStack splitStack(int32_t amount) {
        int32_t taken = std::min(amount, stackSize_);
        ItemStack result(itemId_, taken, damage_);
        stackSize_ -= taken;
        return result;
    }

    // Java: ItemStack.copy()
    ItemStack copy() const {
        return ItemStack(itemId_, stackSize_, damage_);
    }

    // ─── Comparison ────────────────────────────────────────────────────
    // Java: ItemStack.isItemEqual(ItemStack)
    bool isItemEqual(const ItemStack& other) const {
        return itemId_ == other.itemId_ && damage_ == other.damage_;
    }

    // Java: ItemStack.areItemStacksEqual(ItemStack, ItemStack)
    bool isStackEqual(const ItemStack& other) const {
        return itemId_ == other.itemId_ && damage_ == other.damage_ &&
               stackSize_ == other.stackSize_;
    }

    // ─── Animation ─────────────────────────────────────────────────────
    int32_t animationsToGo = 0;

private:
    int32_t itemId_ = 0;
    int32_t stackSize_ = 0;
    int32_t damage_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// Slot — A single slot in a Container, referencing an IInventory.
// Java reference: net.minecraft.inventory.Slot
// ═══════════════════════════════════════════════════════════════════════════

class IInventory; // forward decl

class Slot {
public:
    // Java: Slot(IInventory, int slotIndex, int xPos, int yPos)
    Slot(IInventory* inventory, int32_t slotIndex, int32_t xPos, int32_t yPos)
        : inventory_(inventory), slotIndex_(slotIndex), xPos_(xPos), yPos_(yPos) {}

    int32_t getSlotIndex() const { return slotIndex_; }
    int32_t getXPos() const { return xPos_; }
    int32_t getYPos() const { return yPos_; }
    IInventory* getInventory() const { return inventory_; }

    // Java: Slot.getStack()
    std::optional<ItemStack> getStack() const;

    // Java: Slot.putStack(ItemStack)
    void putStack(const std::optional<ItemStack>& stack);

    // Java: Slot.decrStackSize(int)
    std::optional<ItemStack> decrStackSize(int32_t amount);

    // Java: Slot.isItemValid(ItemStack)
    virtual bool isItemValid(const ItemStack& /*stack*/) const { return true; }

    // Java: Slot.getSlotStackLimit()
    virtual int32_t getSlotStackLimit() const { return 64; }

    // Java: Slot.onSlotChanged()
    void onSlotChanged();

    // Slot number within the Container (set by Container.addSlotToContainer)
    int32_t slotNumber = 0;

private:
    IInventory* inventory_;
    int32_t slotIndex_;
    int32_t xPos_;
    int32_t yPos_;
};

// ═══════════════════════════════════════════════════════════════════════════
// IInventory — Abstract inventory interface.
// Java reference: net.minecraft.inventory.IInventory
// ═══════════════════════════════════════════════════════════════════════════

class IInventory {
public:
    virtual ~IInventory() = default;

    virtual int32_t getSizeInventory() const = 0;
    virtual std::optional<ItemStack> getStackInSlot(int32_t index) const = 0;
    virtual std::optional<ItemStack> decrStackSize(int32_t index, int32_t count) = 0;
    virtual std::optional<ItemStack> getStackInSlotOnClosing(int32_t index) = 0;
    virtual void setInventorySlotContents(int32_t index, const std::optional<ItemStack>& stack) = 0;
    virtual std::string getInventoryName() const = 0;
    virtual bool isCustomInventoryName() const = 0;
    virtual int32_t getInventoryStackLimit() const = 0;
    virtual void markDirty() = 0;
    virtual bool isItemValidForSlot(int32_t index, const ItemStack& stack) const = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// InventoryPlayer — Player's inventory: 36 main + 4 armor slots.
// Java reference: net.minecraft.entity.player.InventoryPlayer
//
// Slots 0-8:   hotbar
// Slots 9-35:  main inventory
// Slots 36-39: armor (feet=36, legs=37, chest=38, head=39)
// ═══════════════════════════════════════════════════════════════════════════

class InventoryPlayer : public IInventory {
public:
    // Constants matching Java
    static constexpr int32_t MAIN_SIZE = 36;
    static constexpr int32_t ARMOR_SIZE = 4;
    static constexpr int32_t TOTAL_SIZE = MAIN_SIZE + ARMOR_SIZE; // 40
    static constexpr int32_t HOTBAR_SIZE = 9;

    InventoryPlayer() {
        mainInventory_.fill(std::nullopt);
        armorInventory_.fill(std::nullopt);
    }

    // ─── IInventory implementation ─────────────────────────────────────
    int32_t getSizeInventory() const override { return TOTAL_SIZE; }

    std::optional<ItemStack> getStackInSlot(int32_t index) const override {
        // Java: InventoryPlayer.getStackInSlot(int)
        // Slots 0-35: mainInventory, 36-39: armorInventory
        if (index >= 0 && index < MAIN_SIZE) {
            return mainInventory_[index];
        } else if (index >= MAIN_SIZE && index < TOTAL_SIZE) {
            return armorInventory_[index - MAIN_SIZE];
        }
        return std::nullopt;
    }

    std::optional<ItemStack> decrStackSize(int32_t index, int32_t count) override;
    std::optional<ItemStack> getStackInSlotOnClosing(int32_t index) override;
    void setInventorySlotContents(int32_t index, const std::optional<ItemStack>& stack) override;

    std::string getInventoryName() const override { return "container.inventory"; }
    bool isCustomInventoryName() const override { return false; }
    int32_t getInventoryStackLimit() const override { return 64; }
    void markDirty() override { inventoryChanged_ = true; }
    bool isItemValidForSlot(int32_t /*index*/, const ItemStack& /*stack*/) const override { return true; }

    // ─── Player-specific methods ───────────────────────────────────────

    // Java: InventoryPlayer.getCurrentItem()
    std::optional<ItemStack> getCurrentItem() const {
        if (currentItem_ >= 0 && currentItem_ < HOTBAR_SIZE) {
            return mainInventory_[currentItem_];
        }
        return std::nullopt;
    }

    // Java: InventoryPlayer.currentItem
    int32_t getCurrentSlot() const { return currentItem_; }
    void setCurrentSlot(int32_t slot) { currentItem_ = slot % HOTBAR_SIZE; }

    // Java: InventoryPlayer.getHotbarSize()
    static int32_t getHotbarSize() { return HOTBAR_SIZE; }

    // Java: InventoryPlayer.getFirstEmptyStack()
    int32_t getFirstEmptyStack() const;

    // Java: InventoryPlayer.addItemStackToInventory(ItemStack)
    bool addItemStackToInventory(ItemStack& stack);

    // ─── Direct array access (for serialization) ───────────────────────
    const std::array<std::optional<ItemStack>, MAIN_SIZE>& getMainInventory() const { return mainInventory_; }
    const std::array<std::optional<ItemStack>, ARMOR_SIZE>& getArmorInventory() const { return armorInventory_; }
    std::array<std::optional<ItemStack>, MAIN_SIZE>& getMainInventory() { return mainInventory_; }
    std::array<std::optional<ItemStack>, ARMOR_SIZE>& getArmorInventory() { return armorInventory_; }

private:
    // Java: public ItemStack[] mainInventory = new ItemStack[36]
    std::array<std::optional<ItemStack>, MAIN_SIZE> mainInventory_;
    // Java: public ItemStack[] armorInventory = new ItemStack[4]
    std::array<std::optional<ItemStack>, ARMOR_SIZE> armorInventory_;
    // Java: public int currentItem
    int32_t currentItem_ = 0;
    bool inventoryChanged_ = false;
};

// ═══════════════════════════════════════════════════════════════════════════
// Container — Base class for all inventory containers (GUIs).
// Java reference: net.minecraft.inventory.Container
//
// A Container maps inventory slots to a linear slot list with consistent
// numbering, enabling packet-based slot operations.
// ═══════════════════════════════════════════════════════════════════════════

class Container {
public:
    virtual ~Container() = default;

    // Java: Container.windowId
    int32_t windowId = 0;

    // ─── Slot management ───────────────────────────────────────────────

    // Java: Container.addSlotToContainer(Slot)
    Slot& addSlotToContainer(std::unique_ptr<Slot> slot) {
        slot->slotNumber = static_cast<int32_t>(slots_.size());
        slots_.push_back(std::move(slot));
        trackedStacks_.push_back(std::nullopt);
        return *slots_.back();
    }

    // Java: Container.getSlot(int)
    Slot* getSlot(int32_t index) {
        if (index >= 0 && index < static_cast<int32_t>(slots_.size()))
            return slots_[index].get();
        return nullptr;
    }

    int32_t getSlotCount() const { return static_cast<int32_t>(slots_.size()); }

    // Java: Container.detectAndSendChanges()
    virtual void detectAndSendChanges();

    // Java: Container.canInteractWith(EntityPlayer)
    virtual bool canInteractWith() const = 0;

    // Java: Container.onContainerClosed(EntityPlayer)
    virtual void onContainerClosed() {}

    // Java: Container.putStackInSlot(int, ItemStack)
    void putStackInSlot(int32_t index, const std::optional<ItemStack>& stack);

protected:
    std::vector<std::unique_ptr<Slot>> slots_;
    std::vector<std::optional<ItemStack>> trackedStacks_;
};

// ═══════════════════════════════════════════════════════════════════════════
// ContainerPlayer — The player's default inventory container.
// Java reference: net.minecraft.inventory.ContainerPlayer
//
// Layout (45 slots total):
//   0:      Crafting output
//   1-4:    Crafting grid (2x2)
//   5-8:    Armor (head=5, chest=6, legs=7, feet=8)
//   9-35:   Main inventory
//   36-44:  Hotbar
// ═══════════════════════════════════════════════════════════════════════════

class ContainerPlayer : public Container {
public:
    // Java: ContainerPlayer(InventoryPlayer, boolean, EntityPlayer)
    explicit ContainerPlayer(InventoryPlayer& playerInventory);

    bool canInteractWith() const override { return true; }
};

} // namespace mccpp
