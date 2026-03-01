/**
 * DispenserDropperHopper.h — Container mechanism blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockDispenser (202 lines)
 *   - net.minecraft.block.BlockDropper (69 lines)
 *   - net.minecraft.block.BlockHopper (183 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DISPENSER (BlockDispenser)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: rock, extends BlockContainer
 * TileEntityDispenser: 9-slot inventory
 * Tick rate: 4
 *
 * Metadata:
 *   bits 0-2: facing direction (0-5, piston-style from determineOrientation)
 *   bit 3: triggered flag
 *
 * Orientation (same as piston):
 *   Uses BlockPistonBase.determineOrientation from entity look direction
 *   Initial placement: auto-orient from adjacent full blocks
 *   func_149938_m: checks Z-/Z+/X-/X+ for fullBlock preference
 *
 * Dispensing (func_149941_e):
 *   1. Get random slot from TileEntityDispenser.func_146017_i
 *   2. If no item (-1): play SFX 1001 (click)
 *   3. Look up IBehaviorDispenseItem from dispenseBehaviorRegistry
 *   4. Call dispense(blockSource, itemStack)
 *   5. Update slot with result
 *
 * Dispense position: 0.7 * facingOffset from block center
 *   getIPositionFromBlockSource: x + 0.7*frontOffsetX etc.
 *
 * Redstone (onNeighborBlockChange):
 *   Powered (self or above) + not triggered → schedule tick + set bit 3
 *   Unpowered + triggered → clear bit 3
 *
 * Break: scatter all inventory items with:
 *   Position: random 0.8+0.1 offset per axis
 *   Stack splitting: random 10-31 per entity
 *   Velocity: gaussian * 0.05, +0.2 upward
 *   Copy NBT tag compound
 *
 * Named: custom name transfers to TileEntity
 * hasComparatorInputOverride = true (calcRedstoneFromInventory)
 *
 * Block ID: dispenser (23)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DROPPER (BlockDropper)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockDispenser. Uses TileEntityDropper.
 *
 * Key difference: overrides func_149941_e
 *   1. Get random slot
 *   2. Check for adjacent inventory on output face
 *      Uses TileEntityHopper.func_145893_b to find inventory
 *   3. If inventory found:
 *      Push 1 item via TileEntityHopper.func_145889_a
 *      If successful: decrement slot, null if empty
 *      If failed: no change
 *   4. If no inventory: use BehaviorDefaultDispenseItem (drop as entity)
 *
 * Always uses default behavior (no special behaviors)
 *   func_149940_a → field_149947_P (BehaviorDefaultDispenseItem)
 *
 * Block ID: dropper (158)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * HOPPER (BlockHopper)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: iron, extends BlockContainer
 * TileEntityHopper: 5-slot inventory
 * Not opaque, not normal, render type 38
 *
 * Collision: 5 boxes (funnel shape)
 *   1. Base: full XZ, 0 to 10/16 height
 *   2. West wall: 0 to 2/16 X, full height
 *   3. North wall: 0 to 2/16 Z, full height
 *   4. East wall: (1-2/16) to 1 X, full height
 *   5. South wall: (1-2/16) to 1 Z, full height
 *
 * Metadata:
 *   bits 0-2: output direction (0-5, where items go)
 *     Placement: oppositeSide of clicked face
 *     If opposite = 1 (up) → force 0 (down) — can only output down or sides
 *   bit 3: disabled flag (powered = disabled)
 *
 * Active state: NOT powered → active (sucks/pushes items)
 *   Power inversion: powered disables the hopper
 *
 * updateBlockData (onNeighborBlockChange + onBlockAdded):
 *   active = !isBlockIndirectlyGettingPowered
 *   Update bit 3 if changed
 *
 * Break: scatter inventory (same algorithm as dispenser)
 * Named: custom name transfers to TileEntityHopper
 * hasComparatorInputOverride = true
 *
 * getDirectionFromMetadata: meta & 7
 * getActiveStateFromMetadata: (meta & 8) != 8
 *
 * Block ID: hopper (154)
 *
 * Thread safety: TileEntity inventory access on server thread.
 * JNI readiness: IInventory interface for Forge container events.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Dispenser Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DispenserConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 23;

    // ─── Properties ───
    static constexpr int32_t TICK_RATE = 4;
    static constexpr int32_t INVENTORY_SIZE = 9;

    // ─── Metadata ───
    static constexpr int32_t FACING_MASK = 7;        // bits 0-2
    static constexpr int32_t TRIGGERED_FLAG = 8;     // bit 3
    static constexpr int32_t TRIGGERED_CLEAR = ~8;   // 0xFFFFFFF7

    // ─── Dispense position ───
    static constexpr double DISPENSE_OFFSET = 0.7;  // 0.7 * facing offset

    // ─── Item scatter (on break) ───
    static constexpr float SCATTER_OFFSET_MIN = 0.1f;
    static constexpr float SCATTER_OFFSET_RANGE = 0.8f;
    static constexpr int32_t SCATTER_MIN_STACK = 10;
    static constexpr int32_t SCATTER_MAX_EXTRA = 21;
    static constexpr float SCATTER_VELOCITY = 0.05f;
    static constexpr float SCATTER_UP_BIAS = 0.2f;

    // ─── SFX ───
    static constexpr int32_t SFX_EMPTY = 1001;    // click when empty

    // ─── Facing (EnumFacing.getFront) ───
    // 0=down, 1=up, 2=north, 3=south, 4=west, 5=east
    static constexpr int32_t FACING_X[6] = {0, 0, 0, 0, -1, 1};
    static constexpr int32_t FACING_Y[6] = {-1, 1, 0, 0, 0, 0};
    static constexpr int32_t FACING_Z[6] = {0, 0, -1, 1, 0, 0};
}

// ═══════════════════════════════════════════════════════════════════════════
// Dropper Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DropperConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 158;

    // ─── Properties ───
    // Inherits all from Dispenser
    // Key difference: always uses BehaviorDefaultDispenseItem
    // Pushes into adjacent inventories via TileEntityHopper logic
}

// ═══════════════════════════════════════════════════════════════════════════
// Hopper Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace HopperConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 154;

    // ─── Properties ───
    static constexpr int32_t INVENTORY_SIZE = 5;

    // ─── Metadata ───
    static constexpr int32_t DIRECTION_MASK = 7;     // bits 0-2
    static constexpr int32_t DISABLED_FLAG = 8;       // bit 3

    inline int32_t getDirection(int32_t meta) { return meta & DIRECTION_MASK; }
    inline bool isActive(int32_t meta) { return (meta & DISABLED_FLAG) != DISABLED_FLAG; }

    // ─── Output direction ───
    // oppositeSide of placement face, 1 forced to 0
    // Can output: 0(down), 2(north), 3(south), 4(west), 5(east)
    // Cannot output: 1(up)

    // ─── Collision (funnel shape) ───
    static constexpr float BASE_HEIGHT = 0.625f;     // 10/16
    static constexpr float WALL_THICKNESS = 0.125f;  // 2/16

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 38;

    // ─── Transfer rate ───
    // TileEntityHopper:
    //   cooldown: 8 ticks between transfers
    //   Pulls from above, pushes to output direction
    //   1 item per transfer
    static constexpr int32_t TRANSFER_COOLDOWN = 8;

    // ─── Item scatter (same as Dispenser) ───
    static constexpr float SCATTER_OFFSET_MIN = 0.1f;
    static constexpr float SCATTER_OFFSET_RANGE = 0.8f;
    static constexpr int32_t SCATTER_MIN_STACK = 10;
    static constexpr int32_t SCATTER_MAX_EXTRA = 21;
    static constexpr float SCATTER_VELOCITY = 0.05f;
    static constexpr float SCATTER_UP_BIAS = 0.2f;
}

} // namespace mccpp
