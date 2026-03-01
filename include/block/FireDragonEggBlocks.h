/**
 * FireDragonEggBlocks.h — Fire, sponge, dragon egg.
 *
 * Java references:
 *   - net.minecraft.block.BlockFire (260 lines)
 *   - net.minecraft.block.BlockSponge (18 lines)
 *   - net.minecraft.block.BlockDragonEgg (115 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FIRE (BlockFire)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: fire, render type 3, map color: tnt
 * Tick randomly: true, tick rate: 30 + rand(10)
 * No collision, not opaque, not normal, not collidable
 * Drops: 0
 *
 * Flammability system:
 *   encouragement[256]: how easily fire spreads TO block (catch fire)
 *   flammability[256]: how easily block is destroyed BY fire
 *
 *   Registered blocks (func_149843_e):
 *     planks/double_wooden_slab/wooden_slab/fence/wood_stairs: enc=5, flam=20
 *     log/log2/coal_block: enc=5, flam=5
 *     leaves/leaves2/wool: enc=30, flam=60
 *     bookshelf: enc=30, flam=20
 *     tnt: enc=15, flam=100
 *     tallgrass/double_plant/yellow_flower/red_flower: enc=60, flam=100
 *     vine: enc=15, flam=100
 *     hay_block/carpet: enc=60, flam=20
 *
 * Fire tick behavior (doFireTick gamerule):
 *   1. Eternal: netherrack below, or bedrock below in End
 *   2. Rain extinguish: 5-position check (self + 4 horizontal)
 *   3. Age: meta 0-15, increments by rand(3)/2 per tick
 *   4. No neighbor burn + no solid below + age>3 → die
 *   5. Non-flammable below + age 15 + rand(4)==0 → die
 *   6. Humidity modifier: isBlockHighHumidity → -50
 *
 *   Direct spread (6 neighbors):
 *     tryCatchFire(chance, flammability):
 *       rand(chance) < flammability → set fire (age from parent)
 *       rand(age+10) < 5 + not raining → fire, else air
 *       TNT → ignite
 *     Horizontal: chance=300+humid, Vertical: chance=250+humid
 *
 *   Area spread (3×6×3 from -1,+1 to +1,+4):
 *     Only to air blocks
 *     encouragingChance = max(6 neighbor encouragements)
 *     spreadChance = (encourage+40+difficulty*7) / (age+30)
 *     Higher blocks: +100 per Y above center+1
 *     Humidity halves chance
 *     Rain blocks spread
 *
 * Portal: onBlockAdded in dim<=0 → tryToCreatePortal
 *
 * Block ID: fire(51)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SPONGE (BlockSponge)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: sponge, creative tab: building blocks
 * No special behavior in 1.7.10 (water absorption added in 1.8)
 *
 * Block ID: sponge(19)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DRAGON EGG (BlockDragonEgg)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: dragonEgg, render type 27
 * Not opaque, not normal
 * Bounds: 1/16 inset XZ (0.0625-0.9375), full Y
 * Tick rate: 5
 *
 * Gravity: same as sand/gravel (BlockFalling)
 *   canFallBelow check, fallInstantly or 32-block chunk check
 *   Spawns EntityFallingBlock at center
 *
 * Teleport (click or punch, func_150019_m):
 *   1000 attempts to find air block:
 *     X: n + rand(16) - rand(16) = ±15
 *     Y: n2 + rand(8) - rand(8) = ±7
 *     Z: n3 + rand(16) - rand(16) = ±15
 *   Server: setBlock(target) + setBlockToAir(source)
 *   Client: 128 portal particles along path
 *
 * Block ID: dragon_egg(122)
 *
 * Thread safety: Fire spread and dragon egg teleport on server thread.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Fire Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FireConst {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 51;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 3;

    // ─── Tick ───
    static constexpr int32_t TICK_RATE = 30;
    static constexpr int32_t TICK_RANDOM = 10;

    // ─── Array sizes ───
    static constexpr int32_t REGISTRY_SIZE = 256;

    // ─── Meta ───
    static constexpr int32_t MAX_AGE = 15;

    // ─── Spread chances ───
    static constexpr int32_t HORIZONTAL_CHANCE = 300;
    static constexpr int32_t VERTICAL_CHANCE = 250;
    static constexpr int32_t HUMIDITY_MODIFIER = -50;

    // ─── Area spread ───
    static constexpr int32_t SPREAD_BASE_CHANCE = 100;
    static constexpr int32_t SPREAD_Y_EXTRA = 100;    // per Y above center+1
    static constexpr int32_t ENCOURAGE_ADD = 40;
    static constexpr int32_t DIFFICULTY_MULT = 7;
    static constexpr int32_t AGE_DIVISOR_ADD = 30;

    // ─── Flammability entries (block_id, encouragement, flammability) ───
    // planks: 5,20 | log: 5,5 | leaves: 30,60
    // bookshelf: 30,20 | tnt: 15,100 | tallgrass: 60,100
    // wool: 30,60 | vine: 15,100 | coal_block: 5,5
    // hay_block: 60,20 | carpet: 60,20
}

// ═══════════════════════════════════════════════════════════════════════════
// Sponge Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SpongeConst {
    static constexpr int32_t BLOCK_ID = 19;
    // No special mechanics in 1.7.10
}

// ═══════════════════════════════════════════════════════════════════════════
// Dragon Egg Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DragonEggConst {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 122;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 27;

    // ─── Tick ───
    static constexpr int32_t TICK_RATE = 5;

    // ─── Bounds ───
    static constexpr float INSET = 0.0625f;              // 1/16

    // ─── Falling ───
    static constexpr int32_t FALL_CHECK_RADIUS = 32;

    // ─── Teleport ───
    static constexpr int32_t TELEPORT_ATTEMPTS = 1000;
    static constexpr int32_t TELEPORT_X_RANGE = 16;      // ±15
    static constexpr int32_t TELEPORT_Y_RANGE = 8;       // ±7
    static constexpr int32_t TELEPORT_Z_RANGE = 16;
    static constexpr int32_t TELEPORT_PARTICLES = 128;
}

} // namespace mccpp
