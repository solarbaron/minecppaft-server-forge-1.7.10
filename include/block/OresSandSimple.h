/**
 * OresSandSimple.h — Ore drops/XP, redstone ore glow, sand/gravel, simple material blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockOre (92 lines)
 *   - net.minecraft.block.BlockRedstoneOre (126 lines)
 *   - net.minecraft.block.BlockSand (27 lines)
 *   - Various simple blocks referenced below
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ORE BLOCKS (BlockOre)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: rock
 *
 * Drop items (getItemDropped):
 *   coal_ore → Items.coal
 *   diamond_ore → Items.diamond
 *   lapis_ore → Items.dye (damage 4 = lapis lazuli)
 *   emerald_ore → Items.emerald
 *   quartz_ore → Items.quartz
 *   iron_ore, gold_ore → drop self (smelt to ingot)
 *
 * Drop quantity (quantityDropped):
 *   lapis_ore → 4 + rand(5) = 4-8
 *   all others → 1
 *
 * Fortune bonus (quantityDroppedWithBonus):
 *   Only for ores that drop items (not self):
 *   multiplier = max(0, rand(fortune + 2) - 1) + 1
 *   result = quantityDropped * multiplier
 *
 * XP drops (dropBlockAsItemWithChance):
 *   coal_ore: 0-2
 *   diamond_ore: 3-7
 *   emerald_ore: 3-7
 *   lapis_ore: 2-5
 *   quartz_ore: 2-5
 *   iron_ore, gold_ore: 0 (drop self, smelt for XP)
 *
 * Block IDs:
 *   coal_ore=16, iron_ore=15, gold_ore=14, diamond_ore=56,
 *   lapis_ore=21, emerald_ore=129, quartz_ore=153
 *
 * ═══════════════════════════════════════════════════════════════════════
 * REDSTONE ORE (BlockRedstoneOre)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: rock
 * Two block variants: redstone_ore (73) and lit_redstone_ore (74)
 * field_150187_a: true = glowing, false = dark
 *
 * Activation triggers (→ switch to lit version):
 *   - onBlockClicked (player hits)
 *   - onEntityWalking (entity walks on top)
 *   - onBlockActivated (player right-clicks)
 *
 * Deactivation (updateTick):
 *   tick rate = 30 → reverts lit → dark
 *
 * Drops:
 *   Item: Items.redstone
 *   Quantity: 4 + rand(2) = 4-5
 *   Fortune: base + rand(fortune + 1)
 *   XP: 1 + rand(5) = 1-5
 *
 * Particles (func_150186_m):
 *   6 reddust particles on exposed faces
 *   1/16 offset from block surface
 *
 * Silk touch: always gives Blocks.redstone_ore (dark version)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SAND (BlockSand)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockFalling (gravity affected)
 * 2 variants: default (meta 0, sand color) and red (meta 1, dirt color)
 * damageDropped = meta
 *
 * Block IDs: sand=12, red_sand=12:1
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SIMPLE MATERIAL BLOCKS (no special logic, included for completeness)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * BlockStone (1): rock, drops cobblestone (no silk = cobblestone)
 * BlockDirt (3): 3 variants — dirt(0), coarse_dirt(1), podzol(2)
 * BlockGravel (13): falls, 10% flint drop (+ fortune boost)
 * BlockClay (82): drops 4 clay balls
 * BlockGlass (20): drops nothing, silk harvest
 * BlockGlowstone (89): drops 2-4 glowstone dust (max 4 even with fortune)
 * BlockBookshelf (47): drops 3 books
 * BlockObsidian (49): rock, hardness 50, resistance 2000
 * BlockNetherrack (87): rock, hardness 0.4
 * BlockSoulSand (88): sand material, 0.875 height (slows entities)
 * BlockStoneBrick (98): 4 variants (normal, mossy, cracked, chiseled)
 * BlockWeb (30): drops string, 4× break with sword, slows entities
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Simple metadata and IDs.
 */
#pragma once

#include <cstdint>
#include <algorithm>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Ore Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace OreConstants {
    // ─── Block IDs ───
    static constexpr int32_t COAL_ORE_ID = 16;
    static constexpr int32_t IRON_ORE_ID = 15;
    static constexpr int32_t GOLD_ORE_ID = 14;
    static constexpr int32_t DIAMOND_ORE_ID = 56;
    static constexpr int32_t LAPIS_ORE_ID = 21;
    static constexpr int32_t EMERALD_ORE_ID = 129;
    static constexpr int32_t QUARTZ_ORE_ID = 153;

    // ─── Drop items (Item IDs) ───
    static constexpr int32_t COAL_ITEM_ID = 263;
    static constexpr int32_t DIAMOND_ITEM_ID = 264;
    static constexpr int32_t DYE_ITEM_ID = 351;       // lapis = damage 4
    static constexpr int32_t EMERALD_ITEM_ID = 388;
    static constexpr int32_t QUARTZ_ITEM_ID = 406;
    static constexpr int32_t LAPIS_DYE_DAMAGE = 4;

    // ─── Lapis drop quantity ───
    static constexpr int32_t LAPIS_DROP_BASE = 4;
    static constexpr int32_t LAPIS_DROP_RANDOM = 5;  // 4-8

    // ─── XP ranges ───
    struct XpRange {
        int32_t min, max;
    };
    static constexpr XpRange COAL_XP = {0, 2};
    static constexpr XpRange DIAMOND_XP = {3, 7};
    static constexpr XpRange EMERALD_XP = {3, 7};
    static constexpr XpRange LAPIS_XP = {2, 5};
    static constexpr XpRange QUARTZ_XP = {2, 5};

    // ─── Fortune formula ───
    // multiplier = max(0, rand(fortune + 2) - 1) + 1
    // result = quantityDropped * multiplier
}

// ═══════════════════════════════════════════════════════════════════════════
// Redstone Ore Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RedstoneOreConstants {
    // ─── Block IDs ───
    static constexpr int32_t REDSTONE_ORE_ID = 73;
    static constexpr int32_t LIT_REDSTONE_ORE_ID = 74;

    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 30;  // ticks until revert lit → dark

    // ─── Drops ───
    static constexpr int32_t REDSTONE_ITEM_ID = 331;
    static constexpr int32_t DROP_BASE = 4;
    static constexpr int32_t DROP_RANDOM = 2;  // 4-5
    // Fortune: base + rand(fortune + 1)

    // ─── XP ───
    static constexpr int32_t XP_MIN = 1;
    static constexpr int32_t XP_RANDOM = 5;  // 1-5

    // ─── Particles ───
    static constexpr int32_t PARTICLE_COUNT = 6;  // one per face
    static constexpr double PARTICLE_OFFSET = 0.0625;  // 1/16
}

// ═══════════════════════════════════════════════════════════════════════════
// Sand / Gravel Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SandConstants {
    // ─── Block IDs ───
    static constexpr int32_t SAND_ID = 12;
    static constexpr int32_t GRAVEL_ID = 13;

    // ─── Sand variants ───
    static constexpr int32_t SAND_DEFAULT = 0;
    static constexpr int32_t SAND_RED = 1;

    // ─── Gravel drops ───
    static constexpr int32_t FLINT_ITEM_ID = 318;
    static constexpr int32_t FLINT_CHANCE = 10;  // 1/10 = 10%
    // Fortune: (fortune == 1) → 7/10, (fortune == 2) → 4/10, (fortune >= 3) → guaranteed
}

// ═══════════════════════════════════════════════════════════════════════════
// Simple Material Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SimpleBlockConstants {
    // ─── Stone ───
    static constexpr int32_t STONE_ID = 1;
    static constexpr int32_t COBBLESTONE_ID = 4;
    // Stone drops cobblestone, silk touch drops stone

    // ─── Dirt ───
    static constexpr int32_t DIRT_ID = 3;
    static constexpr int32_t DIRT_DEFAULT = 0;
    static constexpr int32_t DIRT_COARSE = 1;
    static constexpr int32_t DIRT_PODZOL = 2;

    // ─── Clay ───
    static constexpr int32_t CLAY_ID = 82;
    static constexpr int32_t CLAY_BALL_ID = 337;
    static constexpr int32_t CLAY_DROP_COUNT = 4;

    // ─── Glass ───
    static constexpr int32_t GLASS_ID = 20;
    // Drops nothing, silk harvest only

    // ─── Glowstone ───
    static constexpr int32_t GLOWSTONE_ID = 89;
    static constexpr int32_t GLOWSTONE_DUST_ID = 348;
    static constexpr int32_t GLOWSTONE_DROP_MIN = 2;
    static constexpr int32_t GLOWSTONE_DROP_RANDOM = 3;  // 2-4
    static constexpr int32_t GLOWSTONE_DROP_MAX = 4;
    // Fortune: min(max, base + rand(fortune + 1))

    // ─── Bookshelf ───
    static constexpr int32_t BOOKSHELF_ID = 47;
    static constexpr int32_t BOOK_ITEM_ID = 340;
    static constexpr int32_t BOOKSHELF_DROP_COUNT = 3;

    // ─── Obsidian ───
    static constexpr int32_t OBSIDIAN_ID = 49;
    static constexpr float OBSIDIAN_HARDNESS = 50.0f;
    static constexpr float OBSIDIAN_RESISTANCE = 2000.0f;

    // ─── Netherrack ───
    static constexpr int32_t NETHERRACK_ID = 87;
    static constexpr float NETHERRACK_HARDNESS = 0.4f;

    // ─── Soul Sand ───
    static constexpr int32_t SOUL_SAND_ID = 88;
    static constexpr float SOUL_SAND_HEIGHT = 0.875f;  // 14/16 — slows entities

    // ─── Stone Brick ───
    static constexpr int32_t STONE_BRICK_ID = 98;
    static constexpr int32_t STONE_BRICK_NORMAL = 0;
    static constexpr int32_t STONE_BRICK_MOSSY = 1;
    static constexpr int32_t STONE_BRICK_CRACKED = 2;
    static constexpr int32_t STONE_BRICK_CHISELED = 3;

    // ─── Web ───
    static constexpr int32_t WEB_ID = 30;
    static constexpr int32_t STRING_ITEM_ID = 287;
    // 4× faster break with sword
    // Slows entities to 1/25 horizontal speed

    // ─── Sandstone ───
    static constexpr int32_t SANDSTONE_ID = 24;
    static constexpr int32_t SANDSTONE_DEFAULT = 0;
    static constexpr int32_t SANDSTONE_CHISELED = 1;
    static constexpr int32_t SANDSTONE_SMOOTH = 2;
}

} // namespace mccpp
