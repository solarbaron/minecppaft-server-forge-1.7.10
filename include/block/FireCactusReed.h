/**
 * FireCactusReed.h — Fire spreading, cactus, and sugar cane.
 *
 * Java references:
 *   - net.minecraft.block.BlockFire (260 lines)
 *   - net.minecraft.block.BlockCactus (104 lines)
 *   - net.minecraft.block.BlockReed (112 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FIRE (BlockFire)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: fire, setTickRandomly(true), render type 3
 * No collision (null), not opaque, not collidable
 * MapColor: tntColor
 *
 * Flammability system:
 *   field_149849_a[256]: encouragement (how readily block ignites)
 *   field_149848_b[256]: flammability (chance fire catches from spark)
 *   func_149842_a(blockId, encouragement, flammability) sets both
 *
 * Registered flammable blocks:
 *   planks/double_wooden_slab/wooden_slab/fence/oak_stairs/
 *   birch_stairs/spruce_stairs/jungle_stairs: 5, 20
 *   log/log2/coal_block: 5, 5
 *   leaves/leaves2/wool: 30, 60
 *   bookshelf: 30, 20
 *   tnt: 15, 100
 *   tallgrass/double_plant/yellow_flower/red_flower/vine: 60/15, 100
 *   hay_block/carpet: 60, 20
 *
 * Tick rate: 30 + rand(10) (variable)
 * Gamerule: "doFireTick" controls spreading
 *
 * Eternal fire: netherrack (all dims) + End bedrock
 *
 * Metadata: 0-15 = fire age
 *   Increases by rand(3)/2 per tick (max 15)
 *
 * Rain extinguishment: 5-point rain check (center + 4 adjacent)
 *
 * Fire spreading (updateTick):
 *   1. Cannot stay if no support + no neighbor burns → air
 *   2. Non-eternal: no burnable neighbor + no solid below + age>3 → air
 *   3. Non-eternal: not on burnable + age==15 + rand(4)==0 → air
 *   4. tryCatchFire: 6 faces, base chance 300 (sides) / 250 (up/down)
 *      Humidity: -50 to all chances
 *   5. 3×3×6 neighbor spread:
 *      Bonus per Y above fire: +100 per level
 *      chance = (encouragement + 40 + difficulty*7) / (age + 30)
 *      Humidity: chance /= 2
 *      New fire age: current age + rand(5)/4, capped at 15
 *
 * tryCatchFire:
 *   rand(baseChance) < flammability → fire or destroy
 *   TNT → special detonation
 *   rand(age+10) < 5 + not raining → spread fire
 *   else → destroy block
 *
 * Portal creation: on added in overworld/nether → tryToCreatePortal
 *
 * Block ID: fire(51)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * CACTUS (BlockCactus)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: cactus, setTickRandomly(true), render type 13
 * Not opaque, not normal
 *
 * Collision: 1/16 inset on all sides and top
 *   (x+1/16, y, z+1/16) to (x+15/16, y+15/16, z+15/16)
 *
 * Growth:
 *   Max height: 3 blocks tall
 *   Metadata: 0-15 = growth stage
 *   At meta 15 + air above + height < 3 → grow upward, reset to 0
 *   Else: meta + 1
 *
 * Placement:
 *   canBlockStay: no solid material on 4 horizontal sides
 *   Below must be cactus or sand
 *   Fails → breakBlock with drop
 *
 * Contact damage: 1 HP (DamageSource.cactus)
 *
 * Block ID: cactus(81)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SUGAR CANE (BlockReed)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: plants, setTickRandomly(true), render type 1
 * No collision (null), not opaque
 *
 * Bounds: 6/16 wide centered (0.125 to 0.875 XZ)
 *
 * Growth:
 *   Same as cactus: max height 3, meta 0-15
 *   At meta 15 + air above + height < 3 → grow upward, reset
 *   Else: meta + 1
 *   Extra check: bottom must be reed OR have valid base
 *
 * Placement:
 *   On self: always valid
 *   On grass/dirt/sand: requires adjacent water at Y-1
 *     4 horizontal neighbors of base block
 *
 * Drops: Items.reeds (sugar cane item, ID 338)
 *
 * Block ID: reeds(83)
 *
 * Thread safety: Fire updates, growth on server tick thread.
 * JNI readiness: Fire/growth events for Forge.
 */
#pragma once

#include <cstdint>
#include <array>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Fire Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FireConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 51;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 3;

    // ─── Tick ───
    static constexpr int32_t BASE_TICK_RATE = 30;
    static constexpr int32_t TICK_VARIANCE = 10;

    // ─── Metadata ───
    static constexpr int32_t MAX_AGE = 15;

    // ─── Flammability table size ───
    static constexpr int32_t TABLE_SIZE = 256;

    // ─── Spreading chances ───
    static constexpr int32_t SIDE_CATCH_CHANCE = 300;
    static constexpr int32_t UP_DOWN_CATCH_CHANCE = 250;
    static constexpr int32_t HUMIDITY_MODIFIER = -50;

    // ─── Neighbor spread ───
    static constexpr int32_t SPREAD_RADIUS_XZ = 1;     // 3×3
    static constexpr int32_t SPREAD_Y_MIN = -1;
    static constexpr int32_t SPREAD_Y_MAX = 4;         // Y-1 to Y+4
    static constexpr int32_t BASE_SPREAD_CHANCE = 100;
    static constexpr int32_t Y_BONUS_PER_LEVEL = 100;
    static constexpr int32_t ENCOURAGEMENT_BONUS = 40;
    static constexpr int32_t DIFFICULTY_MULTIPLIER = 7;
    static constexpr int32_t AGE_DIVISOR_BASE = 30;

    // ─── Eternal fire ───
    static constexpr int32_t NETHERRACK_ID = 87;
    static constexpr int32_t BEDROCK_ID = 7;

    // ─── Registered flammable blocks ───
    // encouragement, flammability pairs (selected):
    //   Wood family: 5, 20
    //   Logs/coal: 5, 5
    //   Leaves/wool: 30, 60
    //   TNT: 15, 100
    //   Plants: 60, 100
    //   Hay/carpet: 60, 20
    //   Vine: 15, 100
    //   Bookshelf: 30, 20

    // ─── tryCatchFire ───
    static constexpr int32_t FIRE_SURVIVE_CHANCE = 5;   // rand(age+10) < 5
    static constexpr int32_t TNT_ID = 46;
}

// ═══════════════════════════════════════════════════════════════════════════
// Cactus Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CactusConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 81;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 13;

    // ─── Collision ───
    static constexpr float INSET = 0.0625f;            // 1/16

    // ─── Growth ───
    static constexpr int32_t MAX_HEIGHT = 3;
    static constexpr int32_t MAX_GROWTH_META = 15;

    // ─── Placement ───
    static constexpr int32_t SAND_ID = 12;

    // ─── Damage ───
    static constexpr float CONTACT_DAMAGE = 1.0f;
    // DamageSource.cactus
}

// ═══════════════════════════════════════════════════════════════════════════
// Reed (Sugar Cane) Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace ReedConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 83;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 1;

    // ─── Bounds ───
    static constexpr float HALF_WIDTH = 0.375f;         // 6/16
    // Centered: 0.125 to 0.875 XZ

    // ─── Growth ───
    static constexpr int32_t MAX_HEIGHT = 3;
    static constexpr int32_t MAX_GROWTH_META = 15;

    // ─── Placement base blocks ───
    static constexpr int32_t GRASS_ID = 2;
    static constexpr int32_t DIRT_ID = 3;
    static constexpr int32_t SAND_ID = 12;
    // + requires adjacent water at Y-1 (4 horizontal neighbors)

    // ─── Drops ───
    static constexpr int32_t REEDS_ITEM_ID = 338;
}

} // namespace mccpp
