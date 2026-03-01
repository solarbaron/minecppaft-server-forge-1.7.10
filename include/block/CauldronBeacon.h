/**
 * CauldronBeacon.h — Cauldron water level and beacon pyramid block.
 *
 * Java references:
 *   - net.minecraft.block.BlockCauldron (157 lines)
 *   - net.minecraft.block.BlockBeacon (65 lines)
 *   - net.minecraft.tileentity.TileEntityBeacon (referenced)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * CAULDRON (BlockCauldron)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: iron
 * Not opaque, not normal, render type 24
 * Drops: Items.cauldron
 *
 * Metadata: 0-3 = water level (0=empty, 3=full)
 *   getPowerFromMeta(meta) = meta (identity function)
 *
 * Compound collision (5 boxes):
 *   1. Floor:       (0, 0, 0) → (1, 0.3125, 1) — 5/16 height
 *   2. West wall:   (0, 0, 0) → (0.125, 1, 1)
 *   3. North wall:  (0, 0, 0) → (1, 1, 0.125)
 *   4. East wall:   (0.875, 0, 0) → (1, 1, 1)
 *   5. South wall:  (0, 0, 0.875) → (1, 1, 1)
 *   Wall thickness: 2/16 (0.125)
 *
 * Water surface height:
 *   Java: y + (6 + 3 * waterLevel) / 16
 *   Level 0: y + 0.375 (6/16)
 *   Level 1: y + 0.5625 (9/16)
 *   Level 2: y + 0.75 (12/16)
 *   Level 3: y + 0.9375 (15/16)
 *
 * Interactions (onBlockActivated):
 *   - Water bucket + level < 3: fill to 3, return empty bucket
 *   - Glass bottle + level > 0: extract water bottle, level -= 1
 *     If inventory full, spawn item at (x+0.5, y+1.5, z+0.5)
 *   - Leather armor (CLOTH material) + level > 0: wash color, level -= 1
 *
 * Entity collision (onEntityCollidedWithBlock):
 *   If entity is burning AND level > 0 AND entity.minY <= water surface:
 *   → extinguish entity, level -= 1
 *
 * Rain:
 *   1/20 chance per tick to increase level by 1 (if < 3)
 *
 * Comparator output: water level (0-3)
 *
 * Block ID: 118
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BEACON (BlockBeacon)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: glass
 * Hardness: 3.0
 * Not opaque, not normal, render type 34
 * Extends BlockContainer (TileEntityBeacon)
 *
 * Placement:
 *   If item has display name → beacon gets custom name
 *
 * Activation:
 *   Right-click opens beacon GUI (func_146104_a)
 *
 * Pyramid structure (from TileEntityBeacon):
 *   Scans layers below beacon for valid mineral blocks:
 *     iron_block, gold_block, diamond_block, emerald_block
 *
 *   Layer sizes (centered on beacon x,z):
 *     Layer 1: 3×3 (1 layer = level 1)
 *     Layer 2: 5×5 (2 layers = level 2)
 *     Layer 3: 7×7 (3 layers = level 3)
 *     Layer 4: 9×9 (4 layers = level 4)
 *
 *   Beam requires unobstructed sky access
 *
 * Effects by level:
 *   Level 1: Speed OR Haste (primary)
 *   Level 2: Resistance OR Jump Boost (primary)
 *   Level 3: Strength (primary)
 *   Level 4: Regeneration (secondary) OR double primary
 *
 * Effect range: (level * 10 + 10) blocks
 *   Level 1: 20 blocks
 *   Level 2: 30 blocks
 *   Level 3: 40 blocks
 *   Level 4: 50 blocks
 *
 * Effect duration: (9 + level * 2) * 20 ticks
 *   Level 1: 220 ticks (11 seconds)
 *   Level 2: 260 ticks (13 seconds)
 *   Level 3: 300 ticks (15 seconds)
 *   Level 4: 340 ticks (17 seconds)
 *
 * Block ID: 138
 *
 * Thread safety: Block + tile entity on server thread.
 * JNI readiness: Water level is simple metadata.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Cauldron Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CauldronConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 118;

    // ─── Water level ───
    static constexpr int32_t LEVEL_EMPTY = 0;
    static constexpr int32_t LEVEL_MAX = 3;

    // getPowerFromMeta is identity: level = meta
    inline int32_t getWaterLevel(int32_t meta) { return meta; }

    // ─── Water surface height ───
    // Java: (6 + 3 * level) / 16.0f
    inline float getWaterSurfaceOffset(int32_t level) {
        return (6.0f + 3.0f * static_cast<float>(level)) / 16.0f;
    }
    // Level 0: 6/16  = 0.375
    // Level 1: 9/16  = 0.5625
    // Level 2: 12/16 = 0.75
    // Level 3: 15/16 = 0.9375

    // ─── Compound collision boxes ───
    struct CollisionBox {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
    };

    static constexpr CollisionBox COLLISION_BOXES[] = {
        // Floor: 5/16 height
        {0.0f, 0.0f, 0.0f, 1.0f, 0.3125f, 1.0f},
        // West wall: 2/16 thick
        {0.0f, 0.0f, 0.0f, 0.125f, 1.0f, 1.0f},
        // North wall
        {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.125f},
        // East wall
        {0.875f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f},
        // South wall
        {0.0f, 0.0f, 0.875f, 1.0f, 1.0f, 1.0f},
    };
    static constexpr int32_t NUM_COLLISION_BOXES = 5;

    // ─── Wall thickness ───
    static constexpr float WALL_THICKNESS = 0.125f;  // 2/16

    // ─── Rain fill ───
    static constexpr int32_t RAIN_CHANCE = 20;  // 1/20 probability

    // ─── Item spawn offset ───
    // Glass bottle overflow: (x+0.5, y+1.5, z+0.5)

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 24;

    // ─── Comparator ───
    // Output = water level (0-3)
}

// ═══════════════════════════════════════════════════════════════════════════
// Beacon Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace BeaconConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 138;

    // ─── Properties ───
    static constexpr float HARDNESS = 3.0f;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 34;

    // ─── Pyramid mineral block IDs ───
    static constexpr int32_t IRON_BLOCK_ID = 42;
    static constexpr int32_t GOLD_BLOCK_ID = 41;
    static constexpr int32_t DIAMOND_BLOCK_ID = 57;
    static constexpr int32_t EMERALD_BLOCK_ID = 133;

    inline bool isValidPyramidBlock(int32_t blockId) {
        return blockId == IRON_BLOCK_ID || blockId == GOLD_BLOCK_ID
            || blockId == DIAMOND_BLOCK_ID || blockId == EMERALD_BLOCK_ID;
    }

    // ─── Pyramid layer sizes ───
    static constexpr int32_t MAX_LEVEL = 4;
    // Layer N: (2N+1) × (2N+1) blocks centered below beacon
    // Level 1: 3×3, Level 2: 5×5, Level 3: 7×7, Level 4: 9×9

    // ─── Effect range ───
    // Java: (level * 10 + 10) blocks
    inline int32_t getEffectRange(int32_t level) {
        return level * 10 + 10;
    }

    // ─── Effect duration ───
    // Java: (9 + level * 2) * 20 ticks
    inline int32_t getEffectDuration(int32_t level) {
        return (9 + level * 2) * 20;
    }

    // ─── Potion effect IDs ───
    static constexpr int32_t SPEED_ID = 1;
    static constexpr int32_t HASTE_ID = 3;
    static constexpr int32_t RESISTANCE_ID = 11;
    static constexpr int32_t JUMP_BOOST_ID = 8;
    static constexpr int32_t STRENGTH_ID = 5;
    static constexpr int32_t REGENERATION_ID = 10;

    // Effects by level:
    // Level 1: Speed(1) or Haste(3)
    // Level 2: Resistance(11) or Jump Boost(8)
    // Level 3: Strength(5)
    // Level 4: Regeneration(10) as secondary, or boost primary to II
}

} // namespace mccpp
