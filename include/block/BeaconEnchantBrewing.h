/**
 * BeaconEnchantBrewing.h — Functional TileEntity blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockBeacon (65 lines)
 *   - net.minecraft.block.BlockEnchantmentTable (59 lines)
 *   - net.minecraft.block.BlockBrewingStand (130 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BEACON (BlockBeacon)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: glass, extends BlockContainer
 * Hardness: 3.0, creative tab: misc
 * TileEntityBeacon
 *
 * Not opaque, not normal, render type 34
 *
 * Interaction: opens beacon GUI (func_146104_a)
 * Custom name: transfers from ItemStack to TileEntity
 *
 * TileEntityBeacon behavior:
 *   Scans pyramid every 4 seconds (80 ticks)
 *   Pyramid levels 1-4, made of iron/gold/emerald/diamond blocks
 *   Level determines reach and available effects:
 *     Level 1: Speed/Haste, range 20
 *     Level 2: Resistance/Jump Boost, range 30
 *     Level 3: Strength, range 40
 *     Level 4: Regeneration (secondary), range 50
 *   Beam: requires clear sky above (no opaque blocks)
 *   Effect: applied every 4 seconds, duration = (9 + level*2) * 20 ticks
 *   Amplifier: 0 for primary, 1 for secondary (if same as primary)
 *
 * Block ID: beacon (138)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ENCHANTING TABLE (BlockEnchantmentTable)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: rock, extends BlockContainer
 * Height: 12/16 (0.75), lightOpacity: 0
 * Creative tab: decorations
 * TileEntityEnchantmentTable
 *
 * Not opaque, not normal
 *
 * Interaction: displayGUIEnchantment
 *   Passes custom name if TileEntity has one (func_145921_b → func_145919_a)
 *
 * Custom name: transfers from ItemStack to TileEntity
 *
 * Enchanting mechanics (ContainerEnchantment):
 *   Bookshelves: counted in 5×5×2 area around table
 *     Must have air between bookshelf and table
 *     Max 15 bookshelves contribute
 *   Enchantment levels: based on shelf count + random
 *     Level = rand(1..8) + floor(shelfCount/2) + rand(0..shelfCount)
 *     Then modified by enchantability of item
 *
 * Block ID: enchanting_table (116)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BREWING STAND (BlockBrewingStand)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: iron, extends BlockContainer
 * TileEntityBrewingStand: 4-slot inventory (3 bottles + 1 ingredient)
 * Render type 25
 *
 * Collision: 2 boxes
 *   1. Center rod: 7/16 to 9/16 XZ (2/16 wide), 0 to 14/16 Y
 *   2. Base pad: full XZ, 0 to 2/16 Y (0.125)
 *
 * Metadata: bits 0-2 = which bottle slots are occupied
 *   bit 0: slot 0, bit 1: slot 1, bit 2: slot 2
 *   (visual only, actual inventory in TileEntity)
 *
 * Break: scatters all items (same algorithm as dispenser)
 *   Random offset 0.8+0.1, gaussian velocity 0.05, +0.2 up
 *   Stack splitting 10-31 per entity, NBT copy
 *
 * Drops: brewing_stand item (379)
 * hasComparatorInputOverride = true (calcRedstoneFromInventory)
 * Custom name: transfers to TileEntity
 *
 * TileEntityBrewingStand behavior:
 *   Brewing time: 400 ticks (20 seconds)
 *   Needs blaze powder (fuel, 1.7.10 doesn't need it yet)
 *   Applies potion recipe to up to 3 bottles simultaneously
 *   Updates metadata bits for occupied bottle slots
 *
 * Block ID: brewing_stand (117)
 *
 * Thread safety: TileEntity inventory on server thread.
 * JNI readiness: Container interfaces for Forge GUI events.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Beacon Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace BeaconConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 138;

    // ─── Properties ───
    static constexpr float HARDNESS = 3.0f;
    static constexpr int32_t RENDER_TYPE = 34;

    // ─── Pyramid ───
    static constexpr int32_t MAX_LEVEL = 4;
    static constexpr int32_t SCAN_INTERVAL = 80;    // ticks (4 seconds)

    // Valid pyramid blocks:
    static constexpr int32_t IRON_BLOCK_ID = 42;
    static constexpr int32_t GOLD_BLOCK_ID = 41;
    static constexpr int32_t EMERALD_BLOCK_ID = 133;
    static constexpr int32_t DIAMOND_BLOCK_ID = 57;

    // ─── Range per level ───
    static constexpr int32_t RANGE[5] = {0, 20, 30, 40, 50};

    // ─── Effect duration ───
    // (9 + level * 2) * 20 ticks
    inline int32_t effectDuration(int32_t level) {
        return (9 + level * 2) * 20;
    }

    // ─── Effects available per level ───
    // Level 1: Speed (1), Haste (3)
    // Level 2: Resistance (11), Jump Boost (8)
    // Level 3: Strength (5)
    // Level 4: Regeneration (10) as secondary
}

// ═══════════════════════════════════════════════════════════════════════════
// Enchanting Table Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EnchantTableConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 116;

    // ─── Properties ───
    static constexpr float HEIGHT = 0.75f;          // 12/16
    static constexpr int32_t LIGHT_OPACITY = 0;

    // ─── Bookshelves ───
    static constexpr int32_t MAX_BOOKSHELVES = 15;
    static constexpr int32_t BOOKSHELF_ID = 47;

    // Bookshelf scan area: 5×5×2 around table at Y+0 and Y+1
    // Must have air between bookshelf and table
    // Positions: x ∈ [-2, +2], z ∈ [-2, +2], y ∈ [0, 1]
    // Excluding positions within 1 block of table (those must be air)

    // ─── Enchantment level calculation ───
    // enchantability = item stat
    // base = rand(1..8) + floor(shelves/2) + rand(0..shelves)
    // modified by enchantability
}

// ═══════════════════════════════════════════════════════════════════════════
// Brewing Stand Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace BrewingStandConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 117;

    // ─── Item ID ───
    static constexpr int32_t BREWING_STAND_ITEM_ID = 379;

    // ─── Properties ───
    static constexpr int32_t INVENTORY_SIZE = 4;    // 3 bottles + 1 ingredient
    static constexpr int32_t RENDER_TYPE = 25;

    // ─── Collision ───
    // Center rod
    static constexpr float ROD_X_MIN = 0.4375f;     // 7/16
    static constexpr float ROD_X_MAX = 0.5625f;     // 9/16
    static constexpr float ROD_Z_MIN = 0.4375f;
    static constexpr float ROD_Z_MAX = 0.5625f;
    static constexpr float ROD_Y_MAX = 0.875f;      // 14/16
    // Base pad
    static constexpr float BASE_HEIGHT = 0.125f;     // 2/16

    // ─── Metadata ───
    // bits 0-2: bottle slot occupancy (visual)
    static constexpr int32_t BOTTLE_0_FLAG = 1;
    static constexpr int32_t BOTTLE_1_FLAG = 2;
    static constexpr int32_t BOTTLE_2_FLAG = 4;

    // ─── Brewing ───
    static constexpr int32_t BREW_TIME = 400;        // 20 seconds

    // ─── Item scatter (same as dispenser) ───
    static constexpr float SCATTER_OFFSET_MIN = 0.1f;
    static constexpr float SCATTER_OFFSET_RANGE = 0.8f;
    static constexpr int32_t SCATTER_MIN_STACK = 10;
    static constexpr int32_t SCATTER_MAX_EXTRA = 21;
    static constexpr float SCATTER_VELOCITY = 0.05f;
    static constexpr float SCATTER_UP_BIAS = 0.2f;
}

} // namespace mccpp
