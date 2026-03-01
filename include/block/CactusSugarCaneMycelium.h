/**
 * CactusSugarCaneMycelium.h — Cactus, sugar cane, and mycelium blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockCactus (104 lines)
 *   - net.minecraft.block.BlockReed (112 lines)
 *   - net.minecraft.block.BlockMycelium (47 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * CACTUS (BlockCactus)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: cactus, tick randomly, render type 13
 *
 * Growth:
 *   Max height: 3 blocks (counts below, grows if < 3)
 *   Metadata 0-15: growth counter
 *   Meta == 15: place cactus above, reset meta to 0
 *   Meta < 15: increment by 1
 *   Requires air above to grow
 *
 * Collision: 1/16 inset from each side
 *   (x+1/16, y, z+1/16) → (x+15/16, y+15/16, z+15/16)
 *
 * Placement rules (canBlockStay):
 *   No solid material on any of the 4 horizontal neighbors
 *   Below must be cactus OR sand
 *
 * Contact damage (onEntityCollidedWithBlock):
 *   DamageSource.cactus, 1.0 damage
 *
 * Block ID: 81
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SUGAR CANE / REED (BlockReed)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: plants, tick randomly, no collision, render type 1
 * Width: 6/16 centered (0.125→0.875), full height
 * Not opaque, not normal
 *
 * Growth:
 *   Same 3-high limit as cactus
 *   Meta 0-15 counter, meta 15 → place above + reset
 *   Requires air above
 *   Extra condition: skips if bottom block is not reeds AND
 *     canBlockStay fails (validates chain)
 *
 * Placement (canPlaceBlockAt / canBlockStay):
 *   On another reed: always OK
 *   Otherwise: must be on grass/dirt/sand
 *     AND must have water adjacent at same level as base block
 *     (checks 4 cardinal neighbors at y-1 for Material.water)
 *
 * Drops: Items.reeds (sugar cane item, 338)
 *
 * Block ID: 83
 *
 * ═══════════════════════════════════════════════════════════════════════
 * MYCELIUM (BlockMycelium)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: grass, tick randomly
 *
 * Spread algorithm (identical to BlockGrass):
 *   DEATH: light above < 4 AND opacity above > 2 → dirt
 *   SPREAD: light above >= 9
 *     4 attempts: target ±1x ±3y ±1z
 *     Requirements: target = dirt meta 0, light ≥ 4 above target,
 *       opacity ≤ 2 above target → set to mycelium
 *
 * Drops: dirt
 *
 * Block ID: 110
 *
 * Thread safety: Block tick on server thread.
 * JNI readiness: Simple growth mechanics.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Cactus Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CactusConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 81;

    // ─── Growth ───
    static constexpr int32_t MAX_HEIGHT = 3;
    static constexpr int32_t MAX_META = 15;  // growth counter 0-15

    // ─── Collision (1/16 inset) ───
    static constexpr float COLLISION_INSET = 0.0625f;   // 1/16
    // (x+1/16, y, z+1/16) → (x+15/16, y+15/16, z+15/16)

    // ─── Placement ───
    static constexpr int32_t SAND_ID = 12;
    // Below must be cactus or sand
    // No solid material on 4 horizontal neighbors

    // ─── Contact damage ───
    static constexpr float CONTACT_DAMAGE = 1.0f;
    // DamageSource.cactus

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 13;
}

// ═══════════════════════════════════════════════════════════════════════════
// Sugar Cane (Reed) Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SugarCaneConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 83;

    // ─── Growth ───
    static constexpr int32_t MAX_HEIGHT = 3;
    static constexpr int32_t MAX_META = 15;  // growth counter 0-15

    // ─── Bounds ───
    static constexpr float HALF_WIDTH = 0.375f;  // 6/16 centered
    static constexpr float HEIGHT = 1.0f;          // full height
    // No collision box (null)

    // ─── Placement ───
    static constexpr int32_t GRASS_ID = 2;
    static constexpr int32_t DIRT_ID = 3;
    static constexpr int32_t SAND_ID = 12;
    // Below must be grass/dirt/sand with adjacent water at same level
    // Or below is another reed

    // ─── Drops ───
    static constexpr int32_t REEDS_ITEM_ID = 338;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 1;
}

// ═══════════════════════════════════════════════════════════════════════════
// Mycelium Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace MyceliumConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 110;

    // ─── Spread ───
    // Identical to grass spread algorithm:
    // Death: light < 4 + opacity > 2 → dirt
    // Spread: light >= 9, 4 attempts ±1x ±3y ±1z to dirt meta 0
    static constexpr int32_t DEATH_LIGHT_THRESHOLD = 4;
    static constexpr int32_t DEATH_OPACITY_THRESHOLD = 2;
    static constexpr int32_t SPREAD_LIGHT_MIN = 9;
    static constexpr int32_t SPREAD_ATTEMPTS = 4;
    static constexpr int32_t SPREAD_RANGE_XZ = 3;    // ±1
    static constexpr int32_t SPREAD_RANGE_Y = 5;     // ±2 approx
    static constexpr int32_t SPREAD_TARGET_META = 0;  // dirt meta 0 only

    // ─── Drops ───
    // Drops dirt (same as grass)
    static constexpr int32_t DIRT_ID = 3;
}

} // namespace mccpp
