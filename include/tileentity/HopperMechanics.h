/**
 * HopperMechanics.h — Hopper tile entity item transfer pipeline.
 *
 * Java reference:
 *   - net.minecraft.tileentity.TileEntityHopper (452 lines)
 *   - net.minecraft.tileentity.IHopper (interface)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * HOPPER TILE ENTITY (TileEntityHopper)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Inventory: 5 slots, stack limit 64
 * transferCooldown: starts at -1 (uninitialized), set to 8 after transfer
 *
 * updateEntity (every tick):
 *   1. Decrement transferCooldown
 *   2. If not on cooldown: attempt transfer via func_145887_i
 *
 * Main transfer logic (func_145887_i):
 *   - Requires: not on cooldown AND block metadata active state
 *   - Two independent operations (both can happen in one tick):
 *
 *   A. PUSH (func_145883_k — output transfer):
 *      - Skip if hopper is empty (func_152104_k)
 *      - Get destination inventory: block in metadata direction
 *        via Facing.offsetsXYZ[direction]
 *      - Skip if destination is full (func_152102_a)
 *      - For each non-empty slot in hopper:
 *        - Copy item, extract 1 from hopper
 *        - Insert into destination via func_145889_a (face = opposite side)
 *        - If accepted: markDirty on destination, return true
 *        - If rejected: restore hopper slot, try next slot
 *
 *   B. PULL (func_145891_a — input suction):
 *      - Skip if hopper is full (func_152105_l)
 *      - Get source inventory: block directly above (Y+1)
 *      - If source exists and not empty:
 *        - ISidedInventory: iterate getSlotsForFace(0) — bottom face
 *        - Regular: iterate all slots
 *        - For each slot: check canExtractItem, extract 1, insert into hopper
 *      - If no source inventory: check for EntityItem above
 *        - AABB 1×1×1 at hopper position Y+1
 *        - Pick up entire item stack, insert into hopper
 *        - Kill entity if fully consumed, else update stack
 *
 *   If either operation succeeded: set cooldown to 8, markDirty
 *
 * Item insertion algorithm (func_145889_a):
 *   - For each slot in target inventory (respecting ISidedInventory):
 *     - If target slot empty AND isItemValidForSlot AND canInsertItem:
 *       Place entire stack → return null (success)
 *     - If target slot has same item (func_145894_a — same id/meta/NBT,
 *       not overstacked):
 *       Transfer min(remaining, maxStackSize - existingSize)
 *   - If stack remains after all slots: return remainder (partial transfer)
 *   - Null return = fully consumed
 *
 * Stack matching (func_145894_a):
 *   - Same item type (getItem)
 *   - Same metadata
 *   - Source not overstacked (stackSize <= maxStackSize)
 *   - Same NBT tags (areItemStackTagsEqual)
 *
 * Destination discovery (func_145893_b):
 *   1. Get TileEntity at position → if IInventory, use it
 *      - If TileEntityChest: get merged double-chest inventory via BlockChest
 *   2. If no tile entity: check entity inventories in AABB
 *      - Select random entity if multiple
 *   3. Return null if nothing found
 *
 * Cooldown cascade:
 *   - When inserting into another TileEntityHopper: set target cooldown to 8
 *   - This prevents "chains" from transferring too fast
 *
 * Thread safety: Tile entity update on server thread.
 * JNI readiness: Simple POD constants and inventory interface.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Hopper Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace HopperConstants {
    // ─── Block ID ───
    static constexpr int32_t HOPPER_ID = 154;

    // ─── Inventory ───
    static constexpr int32_t INVENTORY_SIZE = 5;
    static constexpr int32_t STACK_LIMIT = 64;

    // ─── Transfer cooldown ───
    // Java: setTransferCooldown(8) after successful transfer
    static constexpr int32_t TRANSFER_COOLDOWN = 8;
    // Java: transferCooldown = -1 (uninitialized at creation)
    static constexpr int32_t COOLDOWN_UNINITIALIZED = -1;

    // ─── Transfer amount ───
    // Java: decrStackSize(i, 1) — always transfer 1 item at a time
    static constexpr int32_t TRANSFER_AMOUNT = 1;

    // ─── Source detection ───
    // Java: func_145884_b — source is at hopper Y + 1.0
    static constexpr double SOURCE_Y_OFFSET = 1.0;

    // ─── Source face ───
    // Java: pull from source face 0 (bottom of block above)
    static constexpr int32_t SOURCE_FACE = 0;

    // ─── EntityItem pickup AABB ───
    // Java: AABB(x, y+1, z, x+1, y+2, z+1) — 1×1×1 above hopper
    static constexpr double ENTITY_PICKUP_Y_OFFSET = 1.0;
    static constexpr double ENTITY_PICKUP_SIZE = 1.0;

    // ─── Useable distance ───
    static constexpr double USE_DISTANCE_SQ = 64.0;

    // ─── Container name ───
    static constexpr const char* DEFAULT_NAME = "container.hopper";

    // ─── NBT tags ───
    static constexpr const char* TAG_ITEMS = "Items";
    static constexpr const char* TAG_COOLDOWN = "TransferCooldown";
    static constexpr const char* TAG_CUSTOM_NAME = "CustomName";
    static constexpr const char* TAG_SLOT = "Slot";
}

// ═══════════════════════════════════════════════════════════════════════════
// Facing Offsets
// Java: net.minecraft.util.Facing — side-to-offset mappings
// Used by hopper to find destination inventory
// ═══════════════════════════════════════════════════════════════════════════

namespace FacingOffsets {
    // Java: Facing.offsetsXForSide = {0, 0, 0, 0, -1, 1}
    static constexpr int32_t offsetsX[] = {0, 0, 0, 0, -1, 1};
    // Java: Facing.offsetsYForSide = {-1, 1, 0, 0, 0, 0}
    static constexpr int32_t offsetsY[] = {-1, 1, 0, 0, 0, 0};
    // Java: Facing.offsetsZForSide = {0, 0, -1, 1, 0, 0}
    static constexpr int32_t offsetsZ[] = {0, 0, -1, 1, 0, 0};
    // Java: Facing.oppositeSide = {1, 0, 3, 2, 5, 4}
    static constexpr int32_t oppositeSide[] = {1, 0, 3, 2, 5, 4};

    // Side indices:
    // 0 = bottom (DOWN)
    // 1 = top (UP)
    // 2 = north (NORTH) -Z
    // 3 = south (SOUTH) +Z
    // 4 = west (WEST) -X
    // 5 = east (EAST) +X
}

// ═══════════════════════════════════════════════════════════════════════════
// Hopper Direction from Metadata
// Java: BlockHopper.getDirectionFromMetadata(meta)
// Java: BlockHopper.getActiveStateFromMetadata(meta)
// ═══════════════════════════════════════════════════════════════════════════

namespace HopperBlockMeta {
    // Java: direction = meta & 7 (bottom 3 bits = facing direction)
    static constexpr int32_t DIRECTION_MASK = 7;

    // Java: active = (meta & 8) != 8 (bit 3 = disabled flag, inverted)
    static constexpr int32_t DISABLED_FLAG = 8;

    inline int32_t getDirection(int32_t metadata) {
        return metadata & DIRECTION_MASK;
    }

    inline bool isActive(int32_t metadata) {
        return (metadata & DISABLED_FLAG) == 0;
    }

    // ─── Hopper pointing directions ───
    // 0 = down (default)
    // 2 = north (-Z)
    // 3 = south (+Z)
    // 4 = west (-X)
    // 5 = east (+X)
    // Note: hoppers cannot point up (1)
    static constexpr int32_t DIR_DOWN = 0;
    static constexpr int32_t DIR_NORTH = 2;
    static constexpr int32_t DIR_SOUTH = 3;
    static constexpr int32_t DIR_WEST = 4;
    static constexpr int32_t DIR_EAST = 5;
}

// ═══════════════════════════════════════════════════════════════════════════
// Transfer Pipeline Helpers
// ═══════════════════════════════════════════════════════════════════════════

namespace HopperTransfer {
    // ─── Check if two ItemStacks can be merged ───
    // Java: func_145894_a — same item, same meta, not overstacked, same NBT
    struct StackMatchResult {
        bool canMerge;
        int32_t transferable;  // how many can be transferred
    };

    inline StackMatchResult canMergeStacks(
        int32_t existingItemId, int32_t existingMeta, int32_t existingSize,
        int32_t existingMaxStack, bool existingHasTag,
        int32_t incomingItemId, int32_t incomingMeta, int32_t incomingSize,
        bool incomingHasTag, bool tagsEqual) {

        // Java: if (item != item2) return false
        if (existingItemId != incomingItemId) return {false, 0};
        // Java: if (meta != meta2) return false
        if (existingMeta != incomingMeta) return {false, 0};
        // Java: if (stackSize > maxStackSize) return false
        if (existingSize > existingMaxStack) return {false, 0};
        // Java: areItemStackTagsEqual
        if (!tagsEqual) return {false, 0};

        // Java: transferable = min(incoming, maxStack - existing)
        int32_t space = existingMaxStack - existingSize;
        int32_t canTransfer = incomingSize < space ? incomingSize : space;

        return {true, canTransfer};
    }

    // ─── Check if inventory slot can accept item ───
    // Java: isItemValidForSlot AND (not ISidedInventory OR canInsertItem)
    // Returns true if item can be placed in the slot

    // ─── Check if inventory slot can provide item ───
    // Java: not ISidedInventory OR canExtractItem
    // Returns true if item can be extracted from the slot
}

// ═══════════════════════════════════════════════════════════════════════════
// Container Window Properties
// ═══════════════════════════════════════════════════════════════════════════

namespace HopperContainer {
    // Java: ContainerHopper — 5 hopper slots + player inventory
    static constexpr int32_t HOPPER_SLOT_COUNT = 5;

    // ─── GUI slot positions ───
    // Java: Hopper slots at y=20, x = 44 + i*18 (i=0..4)
    static constexpr int32_t HOPPER_SLOT_Y = 20;
    static constexpr int32_t HOPPER_SLOT_X_START = 44;
    static constexpr int32_t SLOT_SPACING = 18;

    // ─── Player inventory offsets ───
    // Java: Player inventory rows at y=51 + row*18
    // Hotbar at y=109
    static constexpr int32_t PLAYER_INV_Y_START = 51;
    static constexpr int32_t PLAYER_HOTBAR_Y = 109;

    // ─── Shift-click ranges ───
    // Slots 0-4: hopper
    // Slots 5-31: player inventory
    // Slots 32-40: hotbar
    static constexpr int32_t HOPPER_START = 0;
    static constexpr int32_t HOPPER_END = 5;
    static constexpr int32_t PLAYER_INV_START = 5;
    static constexpr int32_t PLAYER_INV_END = 32;
    static constexpr int32_t HOTBAR_START = 32;
    static constexpr int32_t HOTBAR_END = 41;
}

} // namespace mccpp
