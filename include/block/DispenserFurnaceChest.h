/**
 * DispenserFurnaceChest.h — Dispenser/dropper, furnace, and chest.
 *
 * Java references:
 *   - net.minecraft.block.BlockDispenser (202 lines)
 *   - net.minecraft.block.BlockDropper (69 lines)
 *   - net.minecraft.block.BlockFurnace (167 lines)
 *   - net.minecraft.block.BlockChest (383 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DISPENSER (BlockDispenser)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, material: rock
 * Tick rate: 4
 * TileEntity: TileEntityDispenser (9-slot inventory)
 *
 * Metadata:
 *   bits 0-2: facing direction (0-5, uses BlockPistonBase.determineOrientation)
 *   bit 3: triggered (1) / idle (0)
 *
 * Auto-facing on placement (func_149938_m):
 *   Same logic as furnace — checks which neighbor is full block
 *   Default: 3 (south)
 *
 * Redstone activation (onNeighborBlockChange):
 *   Power = isBlockIndirectlyGettingPowered(x,y,z) OR (x,y+1,z)
 *   Powered + not triggered → schedule tick, set bit 3
 *   Unpowered + triggered → clear bit 3
 *
 * Dispensing (func_149941_e):
 *   Get random filled slot from TE
 *   Empty → playAuxSFX(1001) "click"
 *   Has item → registry lookup for IBehaviorDispenseItem
 *     If found: dispense(source, item), update slot
 *     Default: BehaviorDefaultDispenseItem (shoots item)
 *
 * Dispense position: 0.7 offset from face center
 *   x + 0.7 * facing.frontOffsetX, etc.
 *
 * Break: standard inventory scatter (with NBT preservation)
 * Custom name: from placed itemStack
 * Comparator: calcRedstoneFromInventory
 *
 * Block ID: dispenser(23)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DROPPER (BlockDropper)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockDispenser
 * TileEntity: TileEntityDropper
 *
 * Always uses BehaviorDefaultDispenseItem (no special dispense)
 *
 * Dispense override (func_149941_e):
 *   Check for IInventory at target position (via TileEntityHopper)
 *   If inventory exists:
 *     Transfer 1 item via func_145889_a (insertStack)
 *     Success (null return) → decrement source stack
 *     Failure → no change (item stays)
 *   If no inventory:
 *     Fall back to default dispense behavior
 *     Zero-stack result → null
 *
 * Block ID: dropper(158)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FURNACE (BlockFurnace)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, material: rock
 * TileEntity: TileEntityFurnace (3-slot: input, fuel, output)
 * isBurning: false (furnace) / true (lit_furnace)
 *
 * Auto-facing: same as dispenser (full block neighbor logic)
 *
 * Yaw placement:
 *   yaw 0→dir 2, 1→dir 5, 2→dir 3, 3→dir 4
 *
 * State swap (updateFurnaceBlockState):
 *   Static field_149934_M prevents break during swap
 *   burning → lit_furnace, not burning → furnace
 *   Preserves metadata and TE (validate + setTileEntity)
 *
 * Drop item: always furnace (unlit), not lit_furnace
 * Break: scatter only if NOT mid-swap (field_149934_M == false)
 * Custom name: from placed itemStack
 * Comparator: calcRedstoneFromInventory
 *
 * Block IDs: furnace(61), lit_furnace(62)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * CHEST (BlockChest)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, material: wood
 * Render type 22, not opaque, not normal
 * TileEntity: TileEntityChest (27-slot)
 * chestType: 0 (normal), 1 (trapped)
 *
 * Bounds: 1/16 inset (0.0625-0.9375) XZ, 14/16 Y (0.875)
 *   Extends to edge (0 or 1) toward adjacent same-type chest
 *
 * Double chest formation:
 *   canPlaceBlockAt: max 1 adjacent same block
 *     No adjacent block can already be part of double chest
 *   initMetadata: complex facing logic for both halves
 *     N/S pair → faces E(5) or W(4)
 *     E/W pair → faces N(2) or S(3)
 *     Checks full blocks on both sides for facing preference
 *
 * Opening (getInventory):
 *   Blocked if:
 *     normalCube above (self or partner)
 *     Ocelot sitting on top (AABB y+1 to y+2)
 *   Double: wraps both TEs in InventoryLargeChest
 *     "container.chestDouble" name
 *     West/north TE first, east/south TE second
 *
 * Trapped chest (chestType == 1):
 *   canProvidePower: true
 *   Weak power: numPlayersUsing (clamped 0-15)
 *   Strong power: weak power on side 1 (up) only
 *
 * Break: standard inventory scatter with NBT preservation
 * Custom name: from placed itemStack
 * Comparator: calcRedstoneFromInventory (uses getInventory for double)
 *
 * Block IDs: chest(54), trapped_chest(146), ender_chest(130)
 *
 * Thread safety: Container operations on server thread.
 * JNI readiness: Container block events for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Dispenser Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DispenserConstants {
    // ─── Block IDs ───
    static constexpr int32_t DISPENSER_ID = 23;
    static constexpr int32_t DROPPER_ID = 158;

    // ─── Tick ───
    static constexpr int32_t TICK_RATE = 4;

    // ─── Metadata ───
    static constexpr int32_t FACING_MASK = 7;           // bits 0-2
    static constexpr int32_t TRIGGERED_FLAG = 8;        // bit 3

    inline int32_t getFacing(int32_t meta) { return meta & FACING_MASK; }
    inline bool isTriggered(int32_t meta) { return (meta & TRIGGERED_FLAG) != 0; }

    // ─── Dispense position offset ───
    static constexpr double FACE_OFFSET = 0.7;

    // ─── Empty click sound ───
    static constexpr int32_t EMPTY_CLICK_SOUND = 1001;

    // ─── Inventory ───
    static constexpr int32_t SLOT_COUNT = 9;
}

// ═══════════════════════════════════════════════════════════════════════════
// Furnace Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FurnaceConstants {
    // ─── Block IDs ───
    static constexpr int32_t FURNACE_ID = 61;
    static constexpr int32_t LIT_FURNACE_ID = 62;

    // ─── Yaw → direction mapping ───
    // yaw 0→2(north), 1→5(east), 2→3(south), 3→4(west)

    // ─── Inventory slots ───
    static constexpr int32_t INPUT_SLOT = 0;
    static constexpr int32_t FUEL_SLOT = 1;
    static constexpr int32_t OUTPUT_SLOT = 2;
    static constexpr int32_t SLOT_COUNT = 3;
}

// ═══════════════════════════════════════════════════════════════════════════
// Chest Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace ChestConstants {
    // ─── Block IDs ───
    static constexpr int32_t CHEST_ID = 54;
    static constexpr int32_t TRAPPED_CHEST_ID = 146;
    static constexpr int32_t ENDER_CHEST_ID = 130;

    // ─── Chest types ───
    static constexpr int32_t NORMAL = 0;
    static constexpr int32_t TRAPPED = 1;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 22;

    // ─── Bounds (1/16 inset) ───
    static constexpr float INSET = 0.0625f;             // 1/16
    static constexpr float HEIGHT = 0.875f;              // 14/16

    // ─── Inventory ───
    static constexpr int32_t SLOT_COUNT = 27;

    // ─── Double chest ───
    // Max 1 adjacent same-type chest
    // No neighbor can already be part of double
    // InventoryLargeChest: "container.chestDouble"

    // ─── Trapped redstone ───
    // Weak power = numPlayersUsing (clamped 0-15)
    // Strong power on side 1 (up) only
}

// ═══════════════════════════════════════════════════════════════════════════
// Common Inventory Scatter Pattern
// ═══════════════════════════════════════════════════════════════════════════

namespace InventoryScatter {
    static constexpr float OFFSET_BASE = 0.1f;
    static constexpr float OFFSET_RANGE = 0.8f;
    static constexpr int32_t STACK_MIN = 10;
    static constexpr int32_t STACK_RANGE = 21;          // nextInt(21) + 10
    static constexpr float VELOCITY = 0.05f;
    static constexpr float Y_BOOST = 0.2f;
}

} // namespace mccpp
