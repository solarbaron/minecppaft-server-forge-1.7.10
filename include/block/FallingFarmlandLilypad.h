/**
 * FallingFarmlandLilypad.h — Gravity, agriculture, and aquatic blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockFalling (93 lines)
 *   - net.minecraft.block.BlockSand (27 lines)
 *   - net.minecraft.block.BlockGravel (25 lines)
 *   - net.minecraft.block.BlockFarmland (103 lines)
 *   - net.minecraft.block.BlockLilyPad (57 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FALLING BLOCK BASE (BlockFalling)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: sand (default), extends Block
 * Tick rate: 2
 * Static: fallInstantly (for world gen, instant placement)
 *
 * Fall mechanics:
 *   onBlockAdded + onNeighborBlockChange → scheduleBlockUpdate
 *   updateTick → func_149830_m:
 *     1. canFallBelow(below) && y >= 0
 *     2. If fallInstantly OR chunks not loaded in 32-block radius:
 *        Instant teleport to lowest valid Y
 *     3. Else: spawn EntityFallingBlock at (x+0.5, y+0.5, z+0.5)
 *        with metadata preserved
 *
 * canFallBelow: air, fire, water, or lava → true
 *
 * Hooks:
 *   onStartFalling(EntityFallingBlock) — override for sand anvil damage
 *   playSoundWhenFallen(world, x, y, z, meta) — override for landing
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SAND (BlockSand)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockFalling
 * 2 variants: 0="default" (sandColor), 1="red" (dirtColor)
 * damageDropped = meta (preserves variant)
 *
 * Block ID: sand(12)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * GRAVEL (BlockGravel)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockFalling
 * Flint drop chance:
 *   fortune capped at 3
 *   chance = 1 / (10 - fortune * 3)
 *   fortune 0: 10%, fortune 1: 14.3%, fortune 2: 25%, fortune 3: 100%
 *   If flint: drops Items.flint, else: drops self
 *
 * Block ID: gravel(13)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FARMLAND (BlockFarmland)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: ground, setTickRandomly(true)
 * Height: 15/16 (0.9375) visual, full 1.0 collision
 * Not opaque, lightOpacity=255 (full)
 *
 * Metadata: 0-7 = moisture level (7 = fully hydrated)
 *
 * Hydration (updateTick):
 *   1. Water scan (func_149821_m): 9×2×9 area (±4 XZ, Y to Y+1)
 *      Any water block → meta = 7
 *   2. Rain at Y+1 → meta = 7
 *   3. Otherwise: moisture decreases by 1 each tick
 *   4. At 0: if no crop above → revert to dirt
 *
 * Crop check (func_149822_e):
 *   wheat, melon_stem, pumpkin_stem, potatoes, carrots
 *   at same X,Z at Y+1
 *
 * Trampling (onFallenUpon):
 *   Chance: rand < (fallDistance - 0.5)
 *   Required: player OR mobGriefing gamerule
 *   Result: revert to dirt
 *
 * Solid block above → revert to dirt (onNeighborBlockChange)
 * Drops: dirt item
 *
 * Block ID: farmland(60)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LILY PAD (BlockLilyPad)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockBush (plant base class)
 * Material: plants (from BlockBush)
 * Render type: 23
 *
 * Bounds: full XZ (0-1), 0 to 1/64 Y (0.015625)
 * Has collision (unlike most plants)
 *
 * Boat interaction: boats pass through (no collision added)
 *   addCollisionBoxesToList skips if entity instanceof EntityBoat
 *
 * Placement:
 *   canPlaceBlockOn: only Blocks.water
 *   canBlockStay: requires still water below (meta 0)
 *     Y must be in [0, 256)
 *
 * Block ID: waterlily(111)
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Agriculture events for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Falling Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FallingBlockConstants {
    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 2;

    // ─── Chunk check radius ───
    static constexpr int32_t CHECK_RADIUS = 32;

    // ─── Entity spawn offset ───
    static constexpr float SPAWN_OFFSET = 0.5f;

    // ─── canFallBelow materials ───
    // air, fire, water, lava → can fall through
}

// ═══════════════════════════════════════════════════════════════════════════
// Sand Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SandConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 12;

    // ─── Variants ───
    static constexpr int32_t DEFAULT = 0;
    static constexpr int32_t RED = 1;
    static constexpr int32_t VARIANT_COUNT = 2;
}

// ═══════════════════════════════════════════════════════════════════════════
// Gravel Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace GravelConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 13;

    // ─── Flint drop ───
    static constexpr int32_t FLINT_ITEM_ID = 318;
    static constexpr int32_t MAX_FORTUNE = 3;

    // Flint chance: 1 / (10 - fortune * 3)
    // fortune 0: 1/10, 1: 1/7, 2: 1/4, 3: 1/1 (100%)
    inline bool dropsFlint(int32_t fortune, int32_t randValue) {
        int32_t capped = fortune > MAX_FORTUNE ? MAX_FORTUNE : fortune;
        return randValue % (10 - capped * 3) == 0;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Farmland Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FarmlandConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 60;

    // ─── Properties ───
    static constexpr float VISUAL_HEIGHT = 0.9375f;    // 15/16
    static constexpr int32_t LIGHT_OPACITY = 255;

    // ─── Metadata ───
    static constexpr int32_t MAX_MOISTURE = 7;
    static constexpr int32_t MIN_MOISTURE = 0;

    // ─── Water scan ───
    static constexpr int32_t WATER_SCAN_RADIUS_XZ = 4;
    static constexpr int32_t WATER_SCAN_RADIUS_Y = 1;   // Y to Y+1

    // ─── Crop blocks (for revert protection) ───
    static constexpr int32_t WHEAT_ID = 59;
    static constexpr int32_t MELON_STEM_ID = 105;
    static constexpr int32_t PUMPKIN_STEM_ID = 104;
    static constexpr int32_t POTATOES_ID = 142;
    static constexpr int32_t CARROTS_ID = 141;

    // ─── Trampling ───
    // Chance: random < (fallDistance - 0.5)
    static constexpr float TRAMPLE_THRESHOLD = 0.5f;

    // ─── Reverts to ───
    static constexpr int32_t DIRT_ID = 3;
}

// ═══════════════════════════════════════════════════════════════════════════
// Lily Pad Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LilyPadConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 111;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 23;

    // ─── Bounds ───
    static constexpr float HEIGHT = 0.015625f;          // 1/64

    // ─── Placement ───
    static constexpr int32_t WATER_ID = 9;              // still water
    // canBlockStay: material==water AND meta==0 (still)
    // Y valid range: [0, 256)

    // ─── Boat ───
    // Boats pass through (no collision for EntityBoat)
}

} // namespace mccpp
