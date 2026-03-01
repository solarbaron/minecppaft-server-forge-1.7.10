/**
 * ChestFurnaceAnvil.h — Core container and utility blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockChest (383 lines)
 *   - net.minecraft.block.BlockFurnace (167 lines)
 *   - net.minecraft.block.BlockAnvil (97 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * CHEST (BlockChest)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood, extends BlockContainer
 * TileEntityChest: 27-slot inventory
 * Render type: 22, not opaque
 *
 * Two types: chestType field
 *   0: normal chest
 *   1: trapped chest (provides redstone, creative tab: redstone)
 *
 * Bounds: 1/16 inset XZ (0.0625 to 0.9375), 14/16 high (0.875)
 *   Extended toward adjacent chest for double-chest visual
 *
 * Double chest:
 *   Max 2 chests adjacent. No triple chests allowed.
 *   canPlaceBlockAt: count adjacent chests, reject if >1
 *     Also reject if any adjacent chest is already doubled
 *   InventoryLargeChest: "container.chestDouble", 54 slots
 *   Left/right ordering: negative direction = left
 *
 * Opening blocked by:
 *   1. Normal block above
 *   2. Ocelot sitting on top (EntityOcelot.isSitting in AABB Y+1 to Y+2)
 *   3. Either half of double blocked → both blocked
 *
 * Metadata: facing direction (2=N, 3=S, 4=W, 5=E)
 * Placement: yaw → (0→2, 1→5, 2→3, 3→4)
 * initMetadata: auto-orient from adjacent fullBlocks
 *   Double NS pair → faces 4/5, Double EW pair → faces 2/3
 *
 * Trapped chest:
 *   canProvidePower = true (type 1 only)
 *   Weak power: numPlayersUsing clamped 0-15
 *   Strong power: only face 1 (below)
 *
 * Break: scatter all 27 items
 * hasComparatorInputOverride = true
 * Custom name: transfers to TileEntityChest
 *
 * Block IDs: chest(54), trapped_chest(146), ender_chest(130)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FURNACE (BlockFurnace)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: rock, extends BlockContainer
 * TileEntityFurnace: 3-slot inventory (input, fuel, output)
 *
 * Two block variants: furnace(61) and lit_furnace(62)
 *   isBurning field: true for lit variant
 *   Drops: always furnace item (not lit)
 *
 * State swap (updateFurnaceBlockState):
 *   field_149934_M = true (suppress breakBlock scatter)
 *   setBlock to furnace or lit_furnace
 *   field_149934_M = false
 *   Restore metadata and TileEntity
 *   TileEntity is preserved across state change
 *
 * Metadata: facing direction (2=N, 3=S, 4=W, 5=E)
 * Placement: same yaw formula as chest (0→2, 1→5, 2→3, 3→4)
 * Auto-orient from adjacent fullBlocks (func_149930_e)
 *
 * Break: scatter items only if !field_149934_M
 * hasComparatorInputOverride = true
 * Custom name: setCustomInventoryName
 *
 * TileEntityFurnace behavior:
 *   Smelt time: 200 ticks (10 seconds)
 *   Fuel: various items with burn times
 *   Coal: 1600 ticks, wood: 300, lava bucket: 20000
 *
 * Block IDs: furnace(61), lit_furnace(62)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ANVIL (BlockAnvil)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: anvil, extends BlockFalling
 * lightOpacity: 0, not opaque, render type 35
 *
 * 3 damage stages: bits 2-3 of metadata
 *   0: intact, 1: slightlyDamaged, 2: veryDamaged
 *   damageDropped: meta >> 2
 *
 * Rotation: bits 0-1 of metadata (facing direction)
 *   Placement: yaw+1 then mod 4 → (0→2, 1→3, 2→0, 3→1)
 *   Bounds: 14/16 wide on short axis (2/16 inset), full on long axis
 *     Direction 1/3: Z-bound inset (0.125 to 0.875)
 *     Direction 0/2: X-bound inset (0.125 to 0.875)
 *
 * Falling (extends BlockFalling):
 *   onStartFalling: setHurtEntities(true)
 *     Deals damage on landing to entities below
 *     Damage: min(fallDistance * 2, 40)
 *   playSoundWhenFallen: SFX 1022 (anvil land)
 *
 * Interaction: displayGUIAnvil(x, y, z)
 *   Repair + name + enchanting combine
 *   Experience cost visible in GUI
 *   12% chance to degrade damage per use
 *
 * Block ID: anvil (145)
 *
 * Thread safety: TileEntity inventory on server thread.
 * JNI readiness: Container interfaces for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Chest Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace ChestConstants {
    // ─── Block IDs ───
    static constexpr int32_t CHEST_ID = 54;
    static constexpr int32_t TRAPPED_CHEST_ID = 146;
    static constexpr int32_t ENDER_CHEST_ID = 130;

    // ─── Types ───
    static constexpr int32_t TYPE_NORMAL = 0;
    static constexpr int32_t TYPE_TRAPPED = 1;

    // ─── Inventory ───
    static constexpr int32_t SINGLE_SIZE = 27;
    static constexpr int32_t DOUBLE_SIZE = 54;

    // ─── Bounds ───
    static constexpr float INSET = 0.0625f;          // 1/16
    static constexpr float HEIGHT = 0.875f;            // 14/16

    // ─── Facing (metadata) ───
    // 2=N, 3=S, 4=W, 5=E
    // Yaw mapping: 0→2, 1→5, 2→3, 3→4
    inline int32_t yawToFacing(int32_t yawQuadrant) {
        switch (yawQuadrant) {
            case 0: return 2;
            case 1: return 5;
            case 2: return 3;
            case 3: return 4;
            default: return 3;
        }
    }

    // ─── Trapped chest ───
    // Redstone: weak = numPlayersUsing (0-15)
    // Strong: only face 1 (below)
    static constexpr int32_t MAX_PLAYERS_SIGNAL = 15;
    static constexpr int32_t STRONG_FACE = 1;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 22;
}

// ═══════════════════════════════════════════════════════════════════════════
// Furnace Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FurnaceConstants {
    // ─── Block IDs ───
    static constexpr int32_t FURNACE_ID = 61;
    static constexpr int32_t LIT_FURNACE_ID = 62;

    // ─── Inventory ───
    static constexpr int32_t SLOT_INPUT = 0;
    static constexpr int32_t SLOT_FUEL = 1;
    static constexpr int32_t SLOT_OUTPUT = 2;
    static constexpr int32_t INVENTORY_SIZE = 3;

    // ─── Smelting ───
    static constexpr int32_t SMELT_TIME = 200;        // 10 seconds

    // ─── Fuel burn times (common) ───
    static constexpr int32_t COAL_BURN = 1600;
    static constexpr int32_t WOOD_BURN = 300;
    static constexpr int32_t STICK_BURN = 100;
    static constexpr int32_t LAVA_BURN = 20000;
    static constexpr int32_t BLAZE_ROD_BURN = 2400;

    // ─── Facing ───
    // Same as chest: 2=N, 3=S, 4=W, 5=E
    // Yaw: 0→2, 1→5, 2→3, 3→4

    // ─── Light ───
    static constexpr int32_t LIT_LIGHT_LEVEL = 13;
}

// ═══════════════════════════════════════════════════════════════════════════
// Anvil Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace AnvilConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 145;

    // ─── Metadata ───
    static constexpr int32_t FACING_MASK = 3;         // bits 0-1
    static constexpr int32_t DAMAGE_SHIFT = 2;        // bits 2-3
    static constexpr int32_t DAMAGE_MASK = 0xC;

    // ─── Damage stages ───
    static constexpr int32_t DAMAGE_INTACT = 0;
    static constexpr int32_t DAMAGE_SLIGHTLY = 1;
    static constexpr int32_t DAMAGE_VERY = 2;
    static constexpr float DEGRADE_CHANCE = 0.12f;     // 12% per use

    // ─── Bounds ───
    static constexpr float SHORT_AXIS_MIN = 0.125f;    // 2/16
    static constexpr float SHORT_AXIS_MAX = 0.875f;    // 14/16

    // ─── Falling damage ───
    // Damage = min(fallDistance * 2, 40)
    static constexpr float DAMAGE_PER_BLOCK = 2.0f;
    static constexpr int32_t MAX_FALL_DAMAGE = 40;

    // ─── Sound ───
    static constexpr int32_t LAND_SFX = 1022;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 35;
}

} // namespace mccpp
