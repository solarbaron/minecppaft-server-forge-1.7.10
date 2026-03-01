/**
 * FireBlock.h — Fire spread, flammability, and extinguishing.
 *
 * Java reference:
 *   - net.minecraft.block.BlockFire (260 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FIRE (BlockFire)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: fire, tick randomly, no collision, not collidable
 * Map color: TNT color
 * Render type: 3
 *
 * Tick rate: 30 + rand(10) ticks
 *
 * Flammability system (two arrays, 256 entries each):
 *   field_149849_a[blockId] = encouragement (catch chance)
 *   field_149848_b[blockId] = flammability (burn chance)
 *
 * Registered flammable blocks:
 *   planks, double_wooden_slab, wooden_slab, fence,
 *     oak/birch/spruce/jungle_stairs: encourage=5, flammability=20
 *   log, log2, coal_block: encourage=5, flammability=5
 *   leaves, leaves2, wool: encourage=30, flammability=60
 *   bookshelf: encourage=30, flammability=20
 *   tnt, tallgrass, double_plant, yellow_flower, red_flower,
 *     vine: encourage varies, flammability=100
 *   hay_block, carpet: encourage=60, flammability=20
 *
 * updateTick algorithm:
 *   1. Check doFireTick gamerule — skip if false
 *   2. Eternal fire: netherrack below, or bedrock below in End
 *   3. Rain extinguish: if raining and any of 5 positions gets rain → remove
 *   4. Age increment: meta += rand(3)/2, capped at 15
 *   5. Reschedule: tickRate + rand(10)
 *   6. No neighbors burn + no solid below or age > 3 → extinguish
 *   7. Not eternal + no fuel below + age 15 + 1/4 chance → extinguish
 *
 * tryCatchFire (6 adjacent blocks):
 *   Checks flammability vs rand(chance):
 *     Horizontal: 300 (+humidity -50)
 *     Vertical: 250 (+humidity -50)
 *   If flammability rolls:
 *     rand(age+10) < 5 AND not raining → replace with fire (new age)
 *     Else → destroy block (setBlockToAir)
 *     TNT special: onBlockDestroyedByPlayer with meta 1 (ignited)
 *
 * Spread to new blocks (3×3, y-1 to y+4):
 *   Target must be air
 *   encouragement = max of 6 neighbors' encouragement values
 *   chance = (encouragement + 40 + difficulty*7) / (age + 30)
 *   humidity: chance /= 2
 *   threshold: 100 at y+0/y+1, +100 per level above y+1
 *     (y+2=200, y+3=300, y+4=400)
 *   if rand(threshold) < chance AND not raining → place fire
 *   new age = age + rand(5)/4, capped at 15
 *
 * canPlaceBlockAt: solid top surface below OR any neighbor can burn
 * onBlockAdded: portal creation attempt for dimension <= 0
 *
 * Block ID: 51
 *
 * Thread safety: Fire spread on server thread (doFireTick).
 * JNI readiness: Flammability arrays per-block ID for Forge access.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Fire Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FireConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 51;

    // ─── Tick ───
    static constexpr int32_t TICK_RATE = 30;
    static constexpr int32_t TICK_RANDOM = 10;  // + rand(10)

    // ─── Age ───
    static constexpr int32_t MAX_AGE = 15;
    // Age increment: += rand(3) / 2 (0 or 1)

    // ─── Eternal fire ───
    static constexpr int32_t NETHERRACK_ID = 87;
    static constexpr int32_t BEDROCK_ID = 7;
    // Netherrack below = eternal, OR bedrock below in End dimension

    // ─── Rain extinguish ───
    // Checks: (x,y,z), (x-1,y,z), (x+1,y,z), (x,y,z-1), (x,y,z+1)

    // ─── tryCatchFire chances ───
    static constexpr int32_t CHANCE_HORIZONTAL = 300;
    static constexpr int32_t CHANCE_VERTICAL = 250;
    static constexpr int32_t HUMIDITY_PENALTY = -50;

    // ─── Spread ───
    // Range: x±1, z±1, y-1 to y+4
    static constexpr int32_t SPREAD_Y_MIN = -1;
    static constexpr int32_t SPREAD_Y_MAX = 4;

    // Spread threshold: base 100 + (y_offset - 1) * 100 when above y+1
    static constexpr int32_t SPREAD_BASE_THRESHOLD = 100;

    // Chance formula: (encouragement + 40 + difficulty*7) / (age + 30)
    static constexpr int32_t ENCOURAGE_BONUS = 40;
    static constexpr int32_t DIFFICULTY_FACTOR = 7;
    static constexpr int32_t AGE_DIVISOR_BASE = 30;

    // ─── Self-extinguish ───
    static constexpr int32_t EXTINGUISH_AGE = 3;      // no solid below + age > 3
    static constexpr int32_t EXTINGUISH_CHANCE = 4;     // 1/4 at age 15 no fuel

    // ─── tryCatchFire ───
    // rand(age+10) < 5 → replace with fire, else destroy
    static constexpr int32_t CATCH_THRESHOLD = 5;
    static constexpr int32_t CATCH_AGE_BASE = 10;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 3;

    // ─── Flammability table ───
    // field_149849_a[blockId] = encouragement (catch fire chance)
    // field_149848_b[blockId] = flammability (burn away chance)
    static constexpr int32_t TABLE_SIZE = 256;

    // Registered block flammability values:
    // {blockId, encouragement, flammability}
    struct FlammableEntry {
        int32_t blockId;
        int32_t encouragement;
        int32_t flammability;
    };

    static constexpr FlammableEntry FLAMMABLE_BLOCKS[] = {
        // Wood (encourage 5, flammability 20)
        {5, 5, 20},     // planks
        {125, 5, 20},   // double_wooden_slab
        {126, 5, 20},   // wooden_slab
        {85, 5, 20},    // fence
        {53, 5, 20},    // oak_stairs
        {135, 5, 20},   // birch_stairs
        {134, 5, 20},   // spruce_stairs
        {136, 5, 20},   // jungle_stairs

        // Logs (encourage 5, flammability 5)
        {17, 5, 5},     // log
        {162, 5, 5},    // log2
        {173, 5, 5},    // coal_block

        // Leaves, wool (encourage 30, flammability 60)
        {18, 30, 60},   // leaves
        {161, 30, 60},  // leaves2
        {35, 30, 60},   // wool

        // Bookshelf (encourage 30, flammability 20)
        {47, 30, 20},   // bookshelf

        // Highly flammable (flammability 100)
        {46, 15, 100},  // tnt (encourage 15)
        {31, 60, 100},  // tallgrass
        {175, 60, 100}, // double_plant
        {37, 60, 100},  // yellow_flower
        {38, 60, 100},  // red_flower
        {106, 15, 100}, // vine (encourage 15)

        // Hay, carpet (encourage 60, flammability 20)
        {170, 60, 20},  // hay_block
        {171, 60, 20},  // carpet
    };

    static constexpr int32_t NUM_FLAMMABLE = sizeof(FLAMMABLE_BLOCKS) / sizeof(FlammableEntry);
}

} // namespace mccpp
