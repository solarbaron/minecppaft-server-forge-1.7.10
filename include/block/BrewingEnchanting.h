/**
 * BrewingEnchanting.h — Brewing stand and enchantment table blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockBrewingStand (130 lines)
 *   - net.minecraft.block.BlockEnchantmentTable (59 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BREWING STAND (BlockBrewingStand)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: iron, extends BlockContainer (TileEntityBrewingStand)
 * Not opaque, not normal, render type 25
 * Drops: Items.brewing_stand
 *
 * Compound collision (2 boxes):
 *   1. Central rod: (0.4375, 0, 0.4375) → (0.5625, 0.875, 0.5625)
 *      2/16 wide, 14/16 tall centered
 *   2. Base plate:  (0, 0, 0) → (1, 0.125, 1)
 *      Full XZ, 2/16 tall
 *
 * Inventory: 4 slots (3 bottles + 1 ingredient)
 * breakBlock: scatters all inventory items as EntityItem
 *   Spawn offset: random 0.1-0.9 on each axis
 *   Stack splitting: random 10-30 per entity
 *   Motion: gaussian * 0.05, +0.2 upward
 *
 * Placement: custom name from item display name
 * Activation: opens brewing GUI (func_146098_a)
 * Comparator output: Container.calcRedstoneFromInventory
 *
 * Block ID: 117
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ENCHANTMENT TABLE (BlockEnchantmentTable)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: rock, extends BlockContainer (TileEntityEnchantmentTable)
 * Not opaque, not normal
 * Height: 12/16 (0.75)
 * Light opacity: 0 (transparent to light despite rock material)
 *
 * Activation:
 *   Opens enchantment GUI (displayGUIEnchantment)
 *   Passes custom name if tile entity has one
 *
 * Enchanting power (from surrounding bookshelves):
 *   Checks 5×5 ring, 2 blocks out from table, at Y and Y+1
 *   Air gap required between bookshelf and table
 *   Max 15 bookshelves contribute (checked in ContainerEnchantment)
 *
 * Placement: custom name from item display name
 *
 * Block ID: 116
 *
 * Thread safety: Block + tile entity on server thread.
 * JNI readiness: Inventory accessible through TileEntity.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Brewing Stand Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace BrewingStandConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 117;

    // ─── Collision boxes ───
    struct CollisionBox {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
    };

    // Central rod: 2/16 wide, 14/16 tall
    static constexpr CollisionBox ROD = {
        0.4375f, 0.0f, 0.4375f,  // 7/16, 0, 7/16
        0.5625f, 0.875f, 0.5625f  // 9/16, 14/16, 9/16
    };

    // Base plate: full XZ, 2/16 tall
    static constexpr CollisionBox BASE = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.125f, 1.0f  // 1, 2/16, 1
    };

    // ─── Inventory ───
    static constexpr int32_t INVENTORY_SIZE = 4;  // 3 bottles + 1 ingredient
    static constexpr int32_t BOTTLE_SLOT_START = 0;
    static constexpr int32_t BOTTLE_SLOT_END = 2;
    static constexpr int32_t INGREDIENT_SLOT = 3;

    // ─── Item scatter on break ───
    static constexpr float SCATTER_OFFSET_MIN = 0.1f;
    static constexpr float SCATTER_OFFSET_RANGE = 0.8f;  // 0.1 to 0.9
    static constexpr int32_t SCATTER_STACK_MIN = 10;
    static constexpr int32_t SCATTER_STACK_RANGE = 21;    // 10 to 30
    static constexpr float SCATTER_MOTION = 0.05f;
    static constexpr float SCATTER_UPWARD = 0.2f;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 25;

    // ─── Brewing mechanics (from TileEntityBrewingStand) ───
    static constexpr int32_t BREW_TIME_MAX = 400;  // 20 seconds
    // Bottles are filtered: must be glass bottle or potion
    // Ingredient checked against PotionHelper for valid recipes
}

// ═══════════════════════════════════════════════════════════════════════════
// Enchantment Table Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EnchantmentTableConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 116;

    // ─── Dimensions ───
    static constexpr float HEIGHT = 0.75f;  // 12/16

    // ─── Light ───
    static constexpr int32_t LIGHT_OPACITY = 0;

    // ─── Bookshelf detection (ContainerEnchantment) ───
    // Scan radius: 2 blocks out in each cardinal and diagonal direction
    // Pattern: 5×5 ring at Y and Y+1 relative to table
    // Air gap: block at offset/2 must be air
    // Each bookshelf adds enchanting power
    static constexpr int32_t BOOKSHELF_SCAN_RADIUS = 2;
    static constexpr int32_t MAX_BOOKSHELVES = 15;
    static constexpr int32_t BOOKSHELF_ID = 47;

    // ─── Enchanting level formula ───
    // Base enchantability from item material
    // Modified by: random(0, b/4+1) + random(0, b/4+1) + 1
    //   where b = bookshelves (0-15)
    // Then multiplied by random factor (0.85 to 1.15)
    // Slot modifiers: slot 0 = max(base/3, 1), slot 1 = base*2/3, slot 2 = base
}

} // namespace mccpp
