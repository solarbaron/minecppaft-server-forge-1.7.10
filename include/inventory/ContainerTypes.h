/**
 * ContainerTypes.h — All concrete container types for player-inventory interaction.
 *
 * Java references:
 *   - net.minecraft.inventory.ContainerPlayer (108 lines)
 *   - net.minecraft.inventory.ContainerWorkbench (112 lines)
 *   - net.minecraft.inventory.ContainerChest (74 lines)
 *   - net.minecraft.inventory.ContainerFurnace (100 lines)
 *   - net.minecraft.inventory.ContainerRepair (329 lines)
 *   - net.minecraft.inventory.ContainerBrewingStand (96 lines)
 *   - net.minecraft.inventory.ContainerDispenser (73 lines)
 *   - net.minecraft.inventory.ContainerHopper (62 lines)
 *   - net.minecraft.inventory.ContainerBeacon (116 lines)
 *   - net.minecraft.inventory.ContainerEnchantment (126 lines)
 *   - net.minecraft.inventory.ContainerMerchant (95 lines)
 *   - net.minecraft.inventory.ContainerHorseInventory (54 lines)
 *
 * Thread safety: Containers are per-player, accessed on single player thread.
 */
#pragma once

#include <cstdint>
#include <array>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Slot layout — GUI coordinates for slot positioning (pixel positions).
// Java: Each container defines slots with (inventory, slotIndex, xPos, yPos)
//   Standard player inventory layout:
//     Main 3x9: x = 8+col*18, y = 84+row*18 (rows 0-2)
//     Hotbar 9: x = 8+col*18, y = 142
// ═══════════════════════════════════════════════════════════════════════════

struct SlotPos { int32_t x; int32_t y; };
static constexpr int32_t SLOT_SPACING = 18;
static constexpr int32_t PLAYER_INV_X = 8;
static constexpr int32_t PLAYER_INV_Y = 84;
static constexpr int32_t PLAYER_HOTBAR_Y = 142;

// Player inventory slot indices in container:
//   Main: 9..35 (3 rows of 9)
//   Hotbar: 0..8
// Shift-click standard: main↔hotbar, special→main+hotbar

// ═══════════════════════════════════════════════════════════════════════════
// ContainerPlayer — Player inventory + 2x2 crafting.
// Java: net.minecraft.inventory.ContainerPlayer (108 lines)
//
//   Total: 45 slots
//     0: craft output (144, 36) — SlotCrafting
//     1-4: craft matrix 2x2 (88+col*18, 26+row*18)
//     5-8: armor slots (8, 8+row*18)
//     9-35: main inventory (8+col*18, 84+row*18)
//     36-44: hotbar (8+col*18, 142)
//
//   Shift-click (transferStackInSlot):
//     0 (output): → 9..45 (reverse)
//     1-4 (craft): → 9..45
//     5-8 (armor): → 9..45
//     9-35 (main): armor check first, then → 36..45
//     36-44 (hotbar): → 9..36
//
//   On close: drop craft matrix items (slots 1-4)
//   canInteractWith: always true
// ═══════════════════════════════════════════════════════════════════════════

class ContainerPlayer {
public:
    static constexpr int32_t NUM_SLOTS = 45;
    static constexpr int32_t CRAFT_OUTPUT = 0;
    static constexpr int32_t CRAFT_START = 1;
    static constexpr int32_t CRAFT_END = 4;
    static constexpr int32_t ARMOR_START = 5;
    static constexpr int32_t ARMOR_END = 8;
    static constexpr int32_t MAIN_START = 9;
    static constexpr int32_t MAIN_END = 35;
    static constexpr int32_t HOTBAR_START = 36;
    static constexpr int32_t HOTBAR_END = 44;

    // Craft output slot pos
    static constexpr SlotPos CRAFT_OUTPUT_POS = {144, 36};

    // Craft matrix 2x2
    static SlotPos craftSlotPos(int32_t col, int32_t row) {
        return {88 + col * SLOT_SPACING, 26 + row * SLOT_SPACING};
    }

    // Armor slot
    static SlotPos armorSlotPos(int32_t row) {
        return {8, 8 + row * SLOT_SPACING};
    }

    // Shift-click ranges
    static constexpr int32_t SHIFT_TARGET_START = 9;
    static constexpr int32_t SHIFT_TARGET_END = 45;
    static constexpr int32_t SHIFT_MAIN_TO_HOTBAR_START = 36;
    static constexpr int32_t SHIFT_HOTBAR_TO_MAIN_END = 36;
};

// ═══════════════════════════════════════════════════════════════════════════
// ContainerWorkbench — 3x3 crafting table.
// Java: net.minecraft.inventory.ContainerWorkbench (112 lines)
//
//   Total: 46 slots
//     0: craft output (124, 35) — SlotCrafting
//     1-9: craft matrix 3x3 (30+col*18, 17+row*18)
//     10-36: main inventory
//     37-45: hotbar
//
//   canInteractWith: block is crafting_table AND distSq ≤ 64
//   On close: drop craft matrix items (slots 0-8 of matrix)
//   Shift-click:
//     0 (output): → 10..46 (reverse)
//     10-36 (main): → 37..46
//     37-45 (hotbar): → 10..37
//     else: → 10..46
// ═══════════════════════════════════════════════════════════════════════════

class ContainerWorkbench {
public:
    static constexpr int32_t NUM_SLOTS = 46;
    static constexpr int32_t CRAFT_OUTPUT = 0;
    static constexpr int32_t GRID_START = 1;
    static constexpr int32_t GRID_END = 9;
    static constexpr int32_t GRID_SIZE = 3; // 3x3
    static constexpr int32_t MAIN_START = 10;
    static constexpr int32_t HOTBAR_START = 37;
    static constexpr double INTERACT_RANGE_SQ = 64.0;

    static constexpr SlotPos CRAFT_OUTPUT_POS = {124, 35};

    static SlotPos gridSlotPos(int32_t col, int32_t row) {
        return {30 + col * SLOT_SPACING, 17 + row * SLOT_SPACING};
    }

    int32_t posX = 0, posY = 0, posZ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// ContainerChest — Generic chest container (single/double).
// Java: net.minecraft.inventory.ContainerChest (74 lines)
//
//   numRows = inventorySize / 9  (3 for single, 6 for double)
//   Total: numRows*9 + 36 slots
//     0..numRows*9-1: chest inventory (8+col*18, 18+row*18)
//     numRows*9..+27: main inventory (8+col*18, 103+row*18 + offset)
//     +27..+36: hotbar (8+col*18, 161+offset)
//     where offset = (numRows-4)*18
//
//   canInteractWith: delegates to lowerChestInventory.isUseableByPlayer
//   On open: lowerChestInventory.openChest()
//   On close: lowerChestInventory.closeChest()
//   Shift-click:
//     < numRows*9: → numRows*9..inventorySlots.size() (reverse)
//     ≥ numRows*9: → 0..numRows*9
// ═══════════════════════════════════════════════════════════════════════════

class ContainerChest {
public:
    int32_t numRows = 3;  // 3 for single, 6 for double

    int32_t getChestSlotCount() const { return numRows * 9; }
    int32_t getTotalSlotCount() const { return numRows * 9 + 36; }

    // Chest slot y offset
    int32_t getOffset() const { return (numRows - 4) * SLOT_SPACING; }

    SlotPos chestSlotPos(int32_t col, int32_t row) const {
        return {8 + col * SLOT_SPACING, 18 + row * SLOT_SPACING};
    }

    SlotPos playerMainPos(int32_t col, int32_t row) const {
        return {8 + col * SLOT_SPACING, 103 + row * SLOT_SPACING + getOffset()};
    }

    SlotPos playerHotbarPos(int32_t col) const {
        return {8 + col * SLOT_SPACING, 161 + getOffset()};
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// ContainerFurnace — Smelting furnace with progress syncing.
// Java: net.minecraft.inventory.ContainerFurnace (100 lines)
//
//   Total: 39 slots
//     0: input (56, 17)
//     1: fuel (56, 53)
//     2: output (116, 35) — SlotFurnace
//     3-29: main inventory
//     30-38: hotbar
//
//   Progress sync: 3 fields
//     0: furnaceCookTime
//     1: furnaceBurnTime
//     2: currentItemBurnTime
//
//   Shift-click:
//     2 (output): → 3..39 (reverse)
//     0-1: → 3..39
//     smeltable item: → 0..1
//     fuel item: → 1..2
//     3-29 (main): → 30..39
//     30-38 (hotbar): → 3..30
// ═══════════════════════════════════════════════════════════════════════════

class ContainerFurnace {
public:
    static constexpr int32_t NUM_SLOTS = 39;
    static constexpr int32_t SLOT_INPUT = 0;
    static constexpr int32_t SLOT_FUEL = 1;
    static constexpr int32_t SLOT_OUTPUT = 2;
    static constexpr int32_t MAIN_START = 3;
    static constexpr int32_t MAIN_END = 30;
    static constexpr int32_t HOTBAR_START = 30;
    static constexpr int32_t HOTBAR_END = 39;

    static constexpr SlotPos INPUT_POS = {56, 17};
    static constexpr SlotPos FUEL_POS = {56, 53};
    static constexpr SlotPos OUTPUT_POS = {116, 35};

    // Progress bar IDs
    static constexpr int32_t PROGRESS_COOK_TIME = 0;
    static constexpr int32_t PROGRESS_BURN_TIME = 1;
    static constexpr int32_t PROGRESS_ITEM_BURN_TIME = 2;

    int32_t lastCookTime = 0;
    int32_t lastBurnTime = 0;
    int32_t lastItemBurnTime = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// ContainerRepair — Anvil with enchantment merging and repair cost.
// Java: net.minecraft.inventory.ContainerRepair (329 lines)
//
//   Total: 39 slots
//     0: left input (27, 47)
//     1: right input (76, 47)
//     2: output (134, 47) — custom slot, consumes levels on pickup
//     3-29: main inventory
//     30-38: hotbar
//
//   Cost calculation:
//     Base repair cost: sum of both items' repairCost
//     Material repair: max(1, damage/100) + enchantCount per material unit
//     Enchantment merge: weight-based cost (10→1, 5→2, 2→4, 1→8)
//       Books halve cost
//     Max level combine: higher level, same→+1
//     Rename: 7 (damageable) or stackSize*5
//     Max cost: 40 (Creative bypasses)
//     Result repairCost: max(left,right) - 9*(hasName) + 2
//
//   canInteractWith: block is anvil AND distSq ≤ 64
//   On close: drop input items
// ═══════════════════════════════════════════════════════════════════════════

class ContainerRepair {
public:
    static constexpr int32_t NUM_SLOTS = 39;
    static constexpr int32_t SLOT_LEFT = 0;
    static constexpr int32_t SLOT_RIGHT = 1;
    static constexpr int32_t SLOT_OUTPUT = 2;
    static constexpr int32_t MAIN_START = 3;
    static constexpr int32_t MAIN_END = 39;
    static constexpr double INTERACT_RANGE_SQ = 64.0;
    static constexpr int32_t MAX_COST = 40;
    static constexpr int32_t MAX_COST_CREATIVE_BYPASS = 39;

    static constexpr SlotPos LEFT_POS = {27, 47};
    static constexpr SlotPos RIGHT_POS = {76, 47};
    static constexpr SlotPos OUTPUT_POS = {134, 47};

    // Enchantment weight → repair cost
    static constexpr int32_t weightToCost(int32_t weight) {
        switch (weight) {
            case 10: return 1;
            case 5: return 2;
            case 2: return 4;
            case 1: return 8;
            default: return 1;
        }
    }

    // Rename costs
    static constexpr int32_t RENAME_COST_DAMAGEABLE = 7;
    static int32_t renameCostStackable(int32_t stackSize) { return stackSize * 5; }

    int32_t maximumCost = 0;
    int32_t materialCost = 0;
    std::string repairedItemName;
    int32_t posX = 0, posY = 0, posZ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// ContainerBrewingStand — Brewing stand with brew time sync.
// Java: ContainerBrewingStand (96 lines)
//
//   Total: 40 slots
//     0-2: potion bottles (56/79/102, 46)
//     3: ingredient (79, 17) — SlotBrewingStandIngredient
//     4-30: main inventory
//     31-39: hotbar
//
//   Progress: brewTime (id=0)
//   Shift-click:
//     0-2 (bottles): → 4..40
//     3 (ingredient): → 4..40
//     isPotionIngredient: → 3..4
//     isPotion/glassBottle: → 0..3
//     4-30: → 31..40
//     31-39: → 4..31
// ═══════════════════════════════════════════════════════════════════════════

class ContainerBrewingStand {
public:
    static constexpr int32_t NUM_SLOTS = 40;
    static constexpr int32_t SLOT_BOTTLE_0 = 0;
    static constexpr int32_t SLOT_BOTTLE_2 = 2;
    static constexpr int32_t SLOT_INGREDIENT = 3;
    static constexpr int32_t MAIN_START = 4;

    static constexpr SlotPos BOTTLE_POS[3] = {{56, 46}, {79, 46}, {102, 46}};
    static constexpr SlotPos INGREDIENT_POS = {79, 17};

    int32_t lastBrewTime = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// ContainerDispenser — 3x3 dispenser/dropper grid.
// Java: ContainerDispenser (73 lines)
//
//   Total: 45 slots
//     0-8: dispenser grid 3x3 (62+col*18, 17+row*18)
//     9-35: main inventory
//     36-44: hotbar
// ═══════════════════════════════════════════════════════════════════════════

class ContainerDispenser {
public:
    static constexpr int32_t NUM_SLOTS = 45;
    static constexpr int32_t DISPENSER_START = 0;
    static constexpr int32_t DISPENSER_END = 8;
    static constexpr int32_t MAIN_START = 9;

    static SlotPos dispenserSlotPos(int32_t col, int32_t row) {
        return {62 + col * SLOT_SPACING, 17 + row * SLOT_SPACING};
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// ContainerHopper — 5-slot hopper.
// Java: ContainerHopper (62 lines)
//
//   Total: 41 slots
//     0-4: hopper inventory (44+col*18, 20)
//     5-31: main inventory
//     32-40: hotbar
// ═══════════════════════════════════════════════════════════════════════════

class ContainerHopper {
public:
    static constexpr int32_t NUM_SLOTS = 41;
    static constexpr int32_t HOPPER_START = 0;
    static constexpr int32_t HOPPER_END = 4;
    static constexpr int32_t MAIN_START = 5;

    static SlotPos hopperSlotPos(int32_t col) {
        return {44 + col * SLOT_SPACING, 20};
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// ContainerBeacon — Beacon with payment slot.
// Java: ContainerBeacon (116 lines)
//
//   Total: 37 slots
//     0: payment (136, 110) — emerald/diamond/gold_ingot/iron_ingot
//     1-27: main inventory
//     28-36: hotbar
// ═══════════════════════════════════════════════════════════════════════════

class ContainerBeacon {
public:
    static constexpr int32_t NUM_SLOTS = 37;
    static constexpr int32_t SLOT_PAYMENT = 0;
    static constexpr int32_t MAIN_START = 1;

    static constexpr SlotPos PAYMENT_POS = {136, 110};
};

// ═══════════════════════════════════════════════════════════════════════════
// ContainerEnchantment — Enchantment table with 3 level display.
// Java: ContainerEnchantment (126 lines)
//
//   Total: 37 slots
//     0: item to enchant (15, 47) — only 1 item, no stacks
//     1-27: main inventory
//     28-36: hotbar
//   3 enchant levels stored in enchantLevels[3]
//   Requires bookshelves within 2-block radius
// ═══════════════════════════════════════════════════════════════════════════

class ContainerEnchantment {
public:
    static constexpr int32_t NUM_SLOTS = 37;
    static constexpr int32_t SLOT_ITEM = 0;
    static constexpr int32_t MAIN_START = 1;
    static constexpr int32_t NUM_LEVELS = 3;
    static constexpr int32_t BOOKSHELF_RADIUS = 2;

    static constexpr SlotPos ITEM_POS = {15, 47};

    std::array<int32_t, NUM_LEVELS> enchantLevels = {0, 0, 0};
    int32_t posX = 0, posY = 0, posZ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// ContainerMerchant — Villager trading.
// Java: ContainerMerchant (95 lines)
//
//   Total: 39 slots
//     0: buy input 1 (36, 53)
//     1: buy input 2 (62, 53)
//     2: merchant result (120, 53) — SlotMerchantResult
//     3-29: main inventory
//     30-38: hotbar
// ═══════════════════════════════════════════════════════════════════════════

class ContainerMerchant {
public:
    static constexpr int32_t NUM_SLOTS = 39;
    static constexpr int32_t SLOT_BUY_1 = 0;
    static constexpr int32_t SLOT_BUY_2 = 1;
    static constexpr int32_t SLOT_RESULT = 2;
    static constexpr int32_t MAIN_START = 3;

    static constexpr SlotPos BUY_1_POS = {36, 53};
    static constexpr SlotPos BUY_2_POS = {62, 53};
    static constexpr SlotPos RESULT_POS = {120, 53};
};

// ═══════════════════════════════════════════════════════════════════════════
// ContainerHorseInventory — Horse inventory with saddle + armor + chest.
// Java: ContainerHorseInventory (54 lines)
//
//   Slot 0: saddle (8, 18) — only accepts saddle
//   Slot 1: horse armor (8, 36) — only for non-chested horses
//   Slots 2+: chest grid (if horse has chest, up to 15 slots)
//   Standard player inventory after
// ═══════════════════════════════════════════════════════════════════════════

class ContainerHorseInventory {
public:
    static constexpr int32_t SLOT_SADDLE = 0;
    static constexpr int32_t SLOT_ARMOR = 1;
    static constexpr int32_t CHEST_START = 2;
    static constexpr int32_t MAX_CHEST_SLOTS = 15;

    static constexpr SlotPos SADDLE_POS = {8, 18};
    static constexpr SlotPos ARMOR_POS = {8, 36};

    static SlotPos chestSlotPos(int32_t col, int32_t row) {
        return {80 + col * SLOT_SPACING, 18 + row * SLOT_SPACING};
    }
};

} // namespace mccpp
