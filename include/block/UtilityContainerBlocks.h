/**
 * UtilityContainerBlocks.h — Anvil, brewing stand, enchanting table,
 *                             beacon, and hopper.
 *
 * Java references:
 *   - net.minecraft.block.BlockAnvil (97 lines)
 *   - net.minecraft.block.BlockBrewingStand (130 lines)
 *   - net.minecraft.block.BlockEnchantmentTable (59 lines)
 *   - net.minecraft.block.BlockBeacon (65 lines)
 *   - net.minecraft.block.BlockHopper (183 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ANVIL (BlockAnvil)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockFalling, material: anvil
 * Render type 35, not opaque, not normal
 * Light opacity: 0
 *
 * 3 damage states: "intact", "slightlyDamaged", "veryDamaged"
 *
 * Metadata:
 *   bits 0-1: direction (0-3, yaw-based with +1 offset)
 *   bits 2-3: damage level (0-2)
 *
 * Placement (onBlockPlacedBy):
 *   yaw = floor(rotationYaw * 4/360 + 0.5) & 3
 *   direction = (yaw + 1) % 4
 *   yaw mapping: 0→2, 1→3, 2→0, 3→1
 *   preserves damage bits from existing meta
 *
 * Bounds: based on direction
 *   N/S (dir 1 or 3): X 0-1, Z 2/16 inset (0.125-0.875)
 *   E/W (dir 0 or 2): Z 0-1, X 2/16 inset (0.125-0.875)
 *   Full Y
 *
 * Falling: onStartFalling → setHurtEntities(true)
 * Landing sound: playAuxSFX(1022)
 *
 * damageDropped: meta >> 2 (preserves damage tier)
 *
 * Block ID: anvil(145)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BREWING STAND (BlockBrewingStand)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, material: iron
 * Render type 25, not opaque, not normal
 * TileEntity: TileEntityBrewingStand
 *
 * Collision: 2-part
 *   Central post: 0.4375-0.5625 XZ (2/16 wide), 0-0.875 Y (14/16)
 *   Base plate: full XZ, 0-0.125 Y (2/16)
 *
 * Break behavior: standard inventory scatter pattern
 *   Random offset: rand*0.8+0.1 per axis
 *   Stack split: rand(21)+10 per entity
 *   Velocity: gaussian*0.05, Y offset +0.2
 *
 * Custom name: from placed itemStack
 * Comparator: calcRedstoneFromInventory
 * Drop item: Items.brewing_stand (not the block)
 *
 * Block ID: brewing_stand(117)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ENCHANTMENT TABLE (BlockEnchantmentTable)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, material: rock
 * Not opaque, not normal
 * Light opacity: 0
 * TileEntity: TileEntityEnchantmentTable
 *
 * Bounds: full XZ, 0-0.75 Y (12/16)
 *
 * Activation: displayGUIEnchantment(x,y,z, name or null)
 *   Uses TE custom name if present
 *
 * Custom name: from placed itemStack
 *
 * Block ID: enchanting_table(116)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BEACON (BlockBeacon)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, material: glass
 * Hardness: 3.0
 * Render type 34, not opaque, not normal
 * TileEntity: TileEntityBeacon
 *
 * Activation: opens beacon GUI (func_146104_a)
 * Custom name: from placed itemStack
 *
 * Block ID: beacon(138)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * HOPPER (BlockHopper)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, material: iron
 * Render type 38, not opaque, not normal
 * TileEntity: TileEntityHopper
 *
 * Metadata:
 *   bits 0-2: output direction (0=down, 2=north, 3=south, 4=west, 5=east)
 *   bit 3: disabled (1) / active (0)
 *   Direction: Facing.oppositeSide of placed face (1→0, cannot face up)
 *
 * Collision: 5-part (funnel shape)
 *   Base: full XZ, 0-0.625 Y (10/16)
 *   North wall: full XZ, 0-2/16 Z, full Y
 *   South wall: full XZ, 14/16-1 Z, full Y
 *   West wall: 0-2/16 X, full YZ
 *   East wall: 14/16-1 X, full YZ
 *
 * Redstone: powered → disabled (set bit 3)
 *   updateBlockData: checks isBlockIndirectlyGettingPowered
 *   Active when NOT powered (bit 3 = 0)
 *
 * Break: inventory scatter (same pattern as brewing stand)
 *   Preserves NBT tag compound on dropped items
 *
 * Custom name: from placed itemStack
 * Comparator: calcRedstoneFromInventory
 *
 * Block ID: hopper(154)
 *
 * Thread safety: TE operations on server thread.
 * JNI readiness: Container block events for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Anvil Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace AnvilConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 145;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 35;

    // ─── Metadata ───
    static constexpr int32_t DIRECTION_MASK = 3;        // bits 0-1
    static constexpr int32_t DAMAGE_SHIFT = 2;          // bits 2-3
    static constexpr int32_t DAMAGE_MASK = 0x0C;        // bits 2-3

    // ─── Damage states ───
    static constexpr int32_t INTACT = 0;
    static constexpr int32_t SLIGHTLY_DAMAGED = 1;
    static constexpr int32_t VERY_DAMAGED = 2;

    // ─── Bounds inset ───
    static constexpr float INSET = 0.125f;              // 2/16

    // ─── Falling ───
    static constexpr int32_t LANDING_SOUND_ID = 1022;

    // ─── Yaw mapping (with +1 offset) ───
    // yaw 0→dir 2, 1→dir 3, 2→dir 0, 3→dir 1
}

// ═══════════════════════════════════════════════════════════════════════════
// Brewing Stand Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace BrewingStandConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 117;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 25;

    // ─── Collision ───
    // Central post
    static constexpr float POST_MIN = 0.4375f;          // 7/16
    static constexpr float POST_MAX = 0.5625f;          // 9/16
    static constexpr float POST_HEIGHT = 0.875f;        // 14/16
    // Base plate
    static constexpr float BASE_HEIGHT = 0.125f;        // 2/16

    // ─── Item scatter on break ───
    static constexpr float SCATTER_OFFSET = 0.1f;
    static constexpr float SCATTER_RANGE = 0.8f;
    static constexpr int32_t SCATTER_MIN_STACK = 10;
    static constexpr int32_t SCATTER_MAX_STACK = 31;    // 21 + 10
    static constexpr float SCATTER_VELOCITY = 0.05f;
    static constexpr float SCATTER_Y_BOOST = 0.2f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Enchantment Table Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EnchantingTableConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 116;

    // ─── Bounds ───
    static constexpr float HEIGHT = 0.75f;              // 12/16
}

// ═══════════════════════════════════════════════════════════════════════════
// Beacon Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace BeaconConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 138;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 34;

    // ─── Properties ───
    static constexpr float HARDNESS = 3.0f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Hopper Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace HopperConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 154;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 38;

    // ─── Metadata ───
    static constexpr int32_t DIRECTION_MASK = 7;        // bits 0-2
    static constexpr int32_t DISABLED_FLAG = 8;          // bit 3

    inline int32_t getDirection(int32_t meta) { return meta & DIRECTION_MASK; }
    inline bool isActive(int32_t meta) { return (meta & DISABLED_FLAG) == 0; }

    // ─── Output directions ───
    static constexpr int32_t DOWN = 0;
    static constexpr int32_t NORTH = 2;
    static constexpr int32_t SOUTH = 3;
    static constexpr int32_t WEST = 4;
    static constexpr int32_t EAST = 5;

    // ─── Collision: 5-part funnel ───
    static constexpr float BASE_HEIGHT = 0.625f;        // 10/16
    static constexpr float WALL_THICKNESS = 0.125f;     // 2/16

    // ─── Redstone ───
    // Powered → disabled (bit 3 set)
    // Active when NOT indirectly powered
}

} // namespace mccpp
