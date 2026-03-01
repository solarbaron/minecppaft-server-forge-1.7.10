/**
 * DoublePlantCakeWorkbench.h — Two-block plants, edible cake, crafting table.
 *
 * Java references:
 *   - net.minecraft.block.BlockDoublePlant (193 lines)
 *   - net.minecraft.block.BlockCake (111 lines)
 *   - net.minecraft.block.BlockWorkbench (29 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DOUBLE PLANT (BlockDoublePlant)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: plants, extends BlockBush, implements IGrowable
 * Hardness: 0, step sound: grass, render type 40
 * Full block bounds (0,0,0)→(1,1,1)
 *
 * Types (6 variants):
 *   0 = sunflower
 *   1 = syringa (lilac)
 *   2 = double tall grass
 *   3 = large fern
 *   4 = rose bush
 *   5 = peony
 *
 * Metadata layout:
 *   bit 3 (0x8): upper half flag
 *   bits 0-2 (0x7): plant type (lower half) or yaw data (upper half)
 *   func_149887_c(meta): (meta & 8) != 0 → is upper half
 *   func_149890_d(meta): meta & 7 → plant type
 *
 * Two-block structure:
 *   Lower half: stores plant type in bits 0-2
 *   Upper half: bit 3 set, bits 0-2 = yaw data from placement
 *               Gets actual type from block below
 *
 * Placement:
 *   canPlaceBlockAt: standard plant + air above
 *   onBlockPlacedBy: sets upper half (meta = 8 | yaw_data)
 *   yaw formula: ((floor(yaw * 4 / 360 + 0.5) & 3) + 2) % 4
 *
 * canBlockStay:
 *   Upper half: block below must be this block
 *   Lower half: block above must be this block + standard plant check
 *
 * Drops:
 *   Upper half: nothing (null)
 *   Grass (type 2) or fern (type 3): nothing normally
 *   Others: drop block item with type
 *
 * Shears (harvestBlock):
 *   Grass (type 2): drops 2× tallgrass:1 (tall grass)
 *   Fern (type 3): drops 2× tallgrass:2 (fern)
 *
 * Bonemeal:
 *   canFertilize: type != 2 && type != 3 (not grass/fern)
 *   shouldFertilize: always true
 *   fertilize: drops item copy of self
 *
 * Breaking upper half:
 *   Creative: silently remove lower half
 *   Survival + grass/fern: shear check → break lower
 *   Survival + other: break lower (with drops)
 *
 * Block ID: 175
 *
 * ═══════════════════════════════════════════════════════════════════════
 * CAKE (BlockCake)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: cake, tick randomly
 * Not opaque, not normal
 * Drops: NOTHING (no item drops, eaten only)
 *
 * Metadata: 0-5 = number of slices eaten (6 total bites)
 *   Meta 0 = full cake (7 slices remaining)
 *   Meta 5 = 1 slice remaining
 *   Meta 6 = air (removed)
 *
 * Bounds (shrinking from west/left):
 *   minX = (1 + meta * 2) / 16.0  → 1/16 to 11/16
 *   maxX = 1 - 1/16 = 15/16
 *   minZ = 1/16, maxZ = 15/16
 *   maxY = 8/16 = 0.5
 *
 * Eating (right-click OR left-click):
 *   Requires: canEat(false) — not full
 *   Effect: +2 food, +0.1 saturation
 *   Then: meta += 1, if meta >= 6 → setBlockToAir
 *
 * Support: needs solid material below
 *
 * Block ID: 92
 *
 * ═══════════════════════════════════════════════════════════════════════
 * CRAFTING TABLE (BlockWorkbench)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood
 * Full block, opaque
 * Right-click opens 3×3 crafting GUI (displayGUIWorkbench)
 *
 * Block ID: 58
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Simple metadata layout.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Double Plant Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DoublePlantConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 175;

    // ─── Plant types ───
    static constexpr int32_t SUNFLOWER = 0;
    static constexpr int32_t SYRINGA = 1;    // lilac
    static constexpr int32_t GRASS = 2;      // double tall grass
    static constexpr int32_t FERN = 3;       // large fern
    static constexpr int32_t ROSE = 4;       // rose bush
    static constexpr int32_t PAEONIA = 5;    // peony
    static constexpr int32_t NUM_TYPES = 6;

    static constexpr const char* TYPE_NAMES[] = {
        "sunflower", "syringa", "grass", "fern", "rose", "paeonia"
    };

    // ─── Metadata ───
    static constexpr int32_t UPPER_HALF_FLAG = 8;  // bit 3
    static constexpr int32_t TYPE_MASK = 7;         // bits 0-2

    inline bool isUpperHalf(int32_t meta) { return (meta & UPPER_HALF_FLAG) != 0; }
    inline int32_t getType(int32_t meta) { return meta & TYPE_MASK; }

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 40;

    // ─── Shear drops ───
    // grass → 2× tallgrass:1, fern → 2× tallgrass:2
    static constexpr int32_t TALLGRASS_ID = 31;
    static constexpr int32_t TALLGRASS_GRASS_META = 1;
    static constexpr int32_t TALLGRASS_FERN_META = 2;
    static constexpr int32_t SHEAR_DROP_COUNT = 2;

    // ─── Placement yaw formula ───
    // Java: ((floor(yaw * 4 / 360 + 0.5) & 3) + 2) % 4
    inline int32_t getYawData(float yaw) {
        return (static_cast<int32_t>(std::floor(yaw * 4.0f / 360.0f + 0.5f)) & 3 + 2) % 4;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Cake Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CakeConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 92;

    // ─── Slices ───
    static constexpr int32_t MAX_BITES = 6;  // meta 0-5, at 6 → removed

    // ─── Food stats per bite ───
    static constexpr int32_t FOOD_POINTS = 2;
    static constexpr float SATURATION = 0.1f;

    // ─── Bounds formula ───
    // minX = (1 + meta * 2) / 16.0
    // maxX = 15/16 (0.9375)
    // minZ = 1/16 (0.0625), maxZ = 15/16 (0.9375)
    // maxY = 8/16 (0.5)
    static constexpr float EDGE = 0.0625f;       // 1/16
    static constexpr float MAX_Y = 0.5f;          // 8/16

    inline float getMinX(int32_t meta) {
        return static_cast<float>(1 + meta * 2) / 16.0f;
    }
    static constexpr float MAX_X = 0.9375f;       // 15/16

    // ─── Collision Y adjustment ───
    // Java: maxY = y + 0.5 - 0.0625 = y + 0.4375
    static constexpr float COLLISION_MAX_Y_OFFSET = 0.4375f;

    // ─── Drops ───
    static constexpr int32_t DROP_QUANTITY = 0;  // no drops
}

// ═══════════════════════════════════════════════════════════════════════════
// Crafting Table Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace WorkbenchConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 58;

    // Opens 3×3 crafting GUI via displayGUIWorkbench(x, y, z)
    // Full opaque wood block, no special metadata
}

} // namespace mccpp
