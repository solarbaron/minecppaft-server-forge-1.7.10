/**
 * BedCakeCauldron.h — Bed, Cake, and Cauldron.
 *
 * Java references:
 *   - net.minecraft.block.BlockBed (190 lines)
 *   - net.minecraft.block.BlockCake (111 lines)
 *   - net.minecraft.block.BlockCauldron (157 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BED (BlockBed)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: cloth, extends BlockDirectional
 * 2-block structure: foot + head
 * Height: 9/16 (0.5625), render type 14, not opaque
 * Mobility: 1 (destroyed by piston)
 *
 * Direction offsets (bedDirections):
 *   0(S): {0, +1}, 1(W): {-1, 0}, 2(N): {0, -1}, 3(E): {+1, 0}
 *
 * Metadata:
 *   bits 0-1: direction (0-3)
 *   bit 2: occupied flag
 *   bit 3: head flag (0=foot, 1=head)
 *
 * Activation (onBlockActivated):
 *   1. Redirect foot → head (using direction offsets)
 *   2. Nether/End: explode (power 5.0, fire=true, destroy=true)
 *      BiomeGenBase.hell also triggers explosion
 *   3. Occupied: find sleeping player, if none → clear flag
 *      If someone sleeping → chat "tile.bed.occupied"
 *   4. sleepInBedAt:
 *      OK → setBedOccupied(true)
 *      NOT_POSSIBLE_NOW → "tile.bed.noSleep"
 *      NOT_SAFE → "tile.bed.notSafe"
 *
 * Spawn point: getSafeExitLocation
 *   Scans 3×3 area around each half (foot first, then head)
 *   Requires: solid below + non-opaque + non-opaque above
 *   nth result parameter for multiple spawns
 *
 * Break: foot drops bed item, head drops nothing
 *   If one half removed → other half breaks (unlinks)
 *   Creative harvest on head → clear foot
 *
 * Block ID: bed(26)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * CAKE (BlockCake)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: cake, setTickRandomly(true)
 * Not opaque, 8/16 high
 *
 * Metadata: 0-5 = slices eaten (0 = full, 5 = one slice left)
 *   6 slices total
 *
 * Bounds: shrinks from left as eaten
 *   X min: (1 + meta * 2) / 16.0
 *   X max: 15/16
 *   Z: 1/16 to 15/16
 *   Y: 0 to 8/16
 *
 * Eating (right-click or left-click):
 *   Requires canEat(false) — not full hunger
 *   +2 food, +0.1 saturation
 *   Increment meta; if >= 6 → setBlockToAir
 *
 * Placement: requires solid material below
 * Drops: nothing (quantityDropped = 0, getItemDropped = null)
 *
 * Block ID: cake(92)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * CAULDRON (BlockCauldron)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: iron, render type 24, not opaque
 *
 * Collision: 5 boxes (bowl shape)
 *   1. Base: full XZ, 0 to 5/16 height
 *   2. West wall: 0 to 2/16 X, full height
 *   3. North wall: 0 to 2/16 Z, full height
 *   4. East wall: 14/16 to 1 X, full height
 *   5. South wall: 14/16 to 1 Z, full height
 *
 * Metadata: 0-3 = water level
 *   0 = empty, 3 = full
 *
 * Water surface Y: (6 + 3*level) / 16
 *   Level 0: 6/16, 1: 9/16, 2: 12/16, 3: 15/16
 *
 * Interactions:
 *   Water bucket + level < 3 → fill to 3, returns empty bucket
 *   Glass bottle + level > 0 → potion(0) water bottle, level -1
 *   Leather armor + level > 0 → removeColor, level -1
 *
 * Entity collision:
 *   Burning entity + level > 0 + entity.minY <= water surface
 *     → extinguish, level -1
 *
 * Rain: 1/20 chance per random tick → level +1 (if < 3)
 *
 * Comparator: signal = water level (0-3)
 * Drops: cauldron item (380)
 *
 * Block ID: cauldron(118)
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Bed/cauldron events for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Bed Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace BedConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 26;

    // ─── Item ───
    static constexpr int32_t BED_ITEM_ID = 355;

    // ─── Metadata ───
    static constexpr int32_t DIRECTION_MASK = 3;       // bits 0-1
    static constexpr int32_t OCCUPIED_FLAG = 4;        // bit 2
    static constexpr int32_t HEAD_FLAG = 8;            // bit 3

    inline bool isHead(int32_t meta) { return (meta & HEAD_FLAG) != 0; }
    inline bool isOccupied(int32_t meta) { return (meta & OCCUPIED_FLAG) != 0; }
    inline int32_t getDirection(int32_t meta) { return meta & DIRECTION_MASK; }

    // ─── Direction offsets ───
    // 0(S): {0, +1}, 1(W): {-1, 0}, 2(N): {0, -1}, 3(E): {+1, 0}
    static constexpr int32_t OFFSETS_X[4] = {0, -1, 0, 1};
    static constexpr int32_t OFFSETS_Z[4] = {1, 0, -1, 0};

    // ─── Bounds ───
    static constexpr float HEIGHT = 0.5625f;           // 9/16

    // ─── Explosion ───
    static constexpr float NETHER_EXPLOSION_POWER = 5.0f;

    // ─── Sleep status ───
    // OK=0, NOT_POSSIBLE_NOW=1, NOT_SAFE=2, OTHER_PROBLEM=3
    static constexpr int32_t SLEEP_OK = 0;
    static constexpr int32_t SLEEP_NOT_NOW = 1;
    static constexpr int32_t SLEEP_NOT_SAFE = 2;

    // ─── Mobility ───
    static constexpr int32_t MOBILITY = 1;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 14;
}

// ═══════════════════════════════════════════════════════════════════════════
// Cake Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CakeConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 92;

    // ─── Slices ───
    static constexpr int32_t TOTAL_SLICES = 6;
    // Meta 0 = full (6 slices), meta 5 = 1 slice left

    // ─── Bounds ───
    // X min = (1 + meta * 2) / 16.0
    // X max = 15/16, Z: 1/16 to 15/16, Y: 0 to 8/16
    static constexpr float INSET = 0.0625f;           // 1/16
    static constexpr float HEIGHT = 0.5f;             // 8/16

    inline float sliceXMin(int32_t meta) {
        return static_cast<float>(1 + meta * 2) / 16.0f;
    }

    // ─── Food ───
    static constexpr int32_t FOOD_POINTS = 2;
    static constexpr float SATURATION = 0.1f;

    // ─── Drops ───
    // Nothing (consumed, not dropped)
}

// ═══════════════════════════════════════════════════════════════════════════
// Cauldron Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CauldronConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 118;

    // ─── Item ───
    static constexpr int32_t CAULDRON_ITEM_ID = 380;

    // ─── Water level ───
    static constexpr int32_t MAX_LEVEL = 3;
    static constexpr int32_t MIN_LEVEL = 0;

    // Water surface Y: (6 + 3 * level) / 16
    inline float waterSurfaceY(int32_t blockY, int32_t level) {
        return static_cast<float>(blockY) + (6.0f + 3.0f * level) / 16.0f;
    }

    // ─── Collision ───
    static constexpr float BASE_HEIGHT = 0.3125f;     // 5/16
    static constexpr float WALL_THICKNESS = 0.125f;    // 2/16

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 24;

    // ─── Rain ───
    static constexpr int32_t RAIN_CHANCE = 20;   // 1 in 20

    // ─── Items ───
    static constexpr int32_t WATER_BUCKET_ID = 326;
    static constexpr int32_t BUCKET_ID = 325;
    static constexpr int32_t GLASS_BOTTLE_ID = 374;
    static constexpr int32_t POTION_ITEM_ID = 373;  // water bottle = damage 0

    // ─── Comparator ───
    // Signal = water level (0-3)
}

} // namespace mccpp
