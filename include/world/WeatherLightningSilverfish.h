/**
 * WeatherLightningSilverfish.h — Weather system, lightning bolts, and silverfish blocks.
 *
 * Java references:
 *   - net.minecraft.entity.effect.EntityLightningBolt (95 lines)
 *   - net.minecraft.entity.effect.EntityWeatherEffect (16 lines)
 *   - net.minecraft.block.BlockSilverfish (129 lines)
 *   - WorldServer.updateWeather() / tick() (weather tick from WorldServer)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * WEATHER CYCLE (from WorldServer)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Rain cycle:
 *   - worldInfo.rainTime: ticks until rain state toggles
 *   - When rain starts: rainTime = rand(12000) + 12000 [12k-24k ticks]
 *   - When rain stops: rainTime = rand(168000) + 12000 [12k-180k ticks]
 *   - Rainfall strength: ramps 0→1 at 0.01/tick, 1→0 at 0.01/tick
 *
 * Thunder cycle (only active during rain):
 *   - worldInfo.thunderTime: ticks until thunder state toggles
 *   - When thunder starts: thunderTime = rand(12000) + 3600 [3.6k-15.6k]
 *   - When thunder stops: thunderTime = rand(168000) + 12000 [12k-180k]
 *
 * Lightning spawning (during thunder):
 *   - Per-chunk check: rand(100000) == 0
 *   - Strike position: random XZ in chunk, highest non-air Y
 *   - Spawn EntityLightningBolt at that position
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LIGHTNING BOLT (EntityLightningBolt)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends EntityWeatherEffect (which extends Entity)
 *
 * Construction:
 *   - lightningState = 2 (initial flash count)
 *   - boltVertex = rand.nextLong() (visual randomization)
 *   - boltLivingTime = rand(3) + 1 (1-3 re-strikes)
 *   - Fire placement (if doFireTick AND difficulty >= Normal):
 *     - Place fire at impact position (if air + canPlaceBlockAt)
 *     - 4 additional random positions: ±1 on each axis
 *
 * onUpdate tick:
 *   lightningState == 2: play sounds
 *     - "ambient.weather.thunder" vol=10000 pitch=0.8+rand*0.2
 *     - "random.explode" vol=2.0 pitch=0.5+rand*0.2
 *
 *   lightningState decrements each tick:
 *     state < 0:
 *       - boltLivingTime == 0 → setDead (entity removed)
 *       - state < -rand(10): re-strike
 *         - boltLivingTime--
 *         - lightningState = 1 (new flash)
 *         - boltVertex = rand.nextLong()
 *         - Try placing fire again at impact
 *
 *   state >= 0 (visible):
 *     - Client: lastLightningBolt = 2 (sky flash)
 *     - Server: damage entities in 3-block radius, +6Y upward
 *       entity.onStruckByLightning(this)
 *
 * No NBT persistence (not saved to disk)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SILVERFISH / INFESTED BLOCKS (BlockSilverfish)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: clay, hardness 0.0 (instant break)
 *
 * 6 variants (metadata 0-5):
 *   0 = stone       (Blocks.stone:0)
 *   1 = cobblestone  (Blocks.cobblestone:0)
 *   2 = stone brick  (Blocks.stonebrick:0)
 *   3 = mossy brick  (Blocks.stonebrick:1)
 *   4 = cracked brick (Blocks.stonebrick:2)
 *   5 = chiseled brick (Blocks.stonebrick:3)
 *
 * Behavior:
 *   - On break OR dropBlockAsItemWithChance: spawn EntitySilverfish
 *   - Silverfish at (x+0.5, y, z+0.5) with 0 rotation
 *   - quantityDropped = 0 (no item drops)
 *   - createStackedBlock returns the disguised block
 *
 * Infestation check (func_150196_a):
 *   - Only stone, cobblestone, or stonebrick can be infested
 *
 * Convert: block+meta → silverfish meta (func_150195_a)
 * Reverse: silverfish meta → (block, meta) pair (func_150197_b)
 *
 * Thread safety: Entity spawning on server thread.
 * JNI readiness: Simple constants and metadata mapping.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Weather Cycle Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace WeatherConstants {
    // ─── Rain timing ───
    // When rain starts: duration = rand(12000) + 12000
    static constexpr int32_t RAIN_DURATION_BASE = 12000;
    static constexpr int32_t RAIN_DURATION_RAND = 12000;
    // Total: 12000-24000 ticks (10-20 minutes)

    // When rain stops: clear duration = rand(168000) + 12000
    static constexpr int32_t CLEAR_DURATION_BASE = 12000;
    static constexpr int32_t CLEAR_DURATION_RAND = 168000;
    // Total: 12000-180000 ticks (10-150 minutes)

    // ─── Thunder timing ───
    // When thunder starts: duration = rand(12000) + 3600
    static constexpr int32_t THUNDER_DURATION_BASE = 3600;
    static constexpr int32_t THUNDER_DURATION_RAND = 12000;
    // Total: 3600-15600 ticks (3-13 minutes)

    // When thunder stops: clear = same as rain clear
    static constexpr int32_t THUNDER_CLEAR_BASE = 12000;
    static constexpr int32_t THUNDER_CLEAR_RAND = 168000;

    // ─── Rainfall strength ramp ───
    // Java: prevRainStr + 0.01f or prevRainStr - 0.01f
    static constexpr float RAIN_RAMP_RATE = 0.01f;
    static constexpr float RAIN_MIN = 0.0f;
    static constexpr float RAIN_MAX = 1.0f;

    // ─── Thunder strength ramp ───
    static constexpr float THUNDER_RAMP_RATE = 0.01f;
    static constexpr float THUNDER_MIN = 0.0f;
    static constexpr float THUNDER_MAX = 1.0f;

    // ─── Lightning spawn chance ───
    // Java: rand(100000) == 0, per loaded chunk during thunder
    static constexpr int32_t LIGHTNING_CHANCE = 100000;
}

// ═══════════════════════════════════════════════════════════════════════════
// Lightning Bolt Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LightningConstants {
    // ─── Initial state ───
    static constexpr int32_t INITIAL_STATE = 2;      // flash count
    static constexpr int32_t MIN_LIVING_TIME = 1;
    static constexpr int32_t MAX_LIVING_TIME_RAND = 3; // rand(3) + 1 = 1-3

    // ─── Sounds ───
    static constexpr const char* SOUND_THUNDER = "ambient.weather.thunder";
    static constexpr float THUNDER_VOLUME = 10000.0f;
    static constexpr float THUNDER_PITCH_BASE = 0.8f;
    static constexpr float THUNDER_PITCH_RAND = 0.2f;

    static constexpr const char* SOUND_EXPLODE = "random.explode";
    static constexpr float EXPLODE_VOLUME = 2.0f;
    static constexpr float EXPLODE_PITCH_BASE = 0.5f;
    static constexpr float EXPLODE_PITCH_RAND = 0.2f;

    // ─── Fire placement ───
    // Only on Normal or Hard difficulty
    // Requires doFireTick gamerule = true
    // Requires chunks loaded within 10 blocks
    static constexpr int32_t FIRE_CHECK_RADIUS = 10;
    static constexpr int32_t FIRE_SPREAD_ATTEMPTS = 4;
    // Spread range: ±1 on each axis from impact
    static constexpr int32_t FIRE_SPREAD_RANGE = 3;  // rand(3) - 1

    // ─── Entity strike ───
    // Radius: 3 blocks XZ, 3 below + 6 above
    static constexpr double STRIKE_RADIUS = 3.0;
    static constexpr double STRIKE_Y_ABOVE = 6.0;

    // ─── Re-strike ───
    // When state < -rand(10): regenerate bolt
    static constexpr int32_t RESTRIKE_RAND = 10;

    // ─── Sky flash ───
    static constexpr int32_t SKY_FLASH_DURATION = 2;  // lastLightningBolt = 2

    // ─── Gamerules ───
    static constexpr const char* DO_FIRE_TICK = "doFireTick";
}

// ═══════════════════════════════════════════════════════════════════════════
// Silverfish / Infested Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SilverfishBlockConstants {
    // ─── Block ID ───
    static constexpr int32_t MONSTER_EGG_ID = 97;

    // ─── Hardness ───
    static constexpr float HARDNESS = 0.0f;  // instant break (any tool)

    // ─── Block names ───
    static constexpr const char* VARIANT_NAMES[] = {
        "stone", "cobble", "brick", "mossybrick", "crackedbrick", "chiseledbrick"
    };
    static constexpr int32_t NUM_VARIANTS = 6;

    // ─── Disguise mapping (meta → block+meta pair) ───
    // 0 = stone:0
    // 1 = cobblestone:0
    // 2 = stonebrick:0
    // 3 = stonebrick:1 (mossy)
    // 4 = stonebrick:2 (cracked)
    // 5 = stonebrick:3 (chiseled)

    struct DisguisedBlock {
        int32_t blockId;
        int32_t metadata;
    };

    static constexpr int32_t STONE_ID = 1;
    static constexpr int32_t COBBLESTONE_ID = 4;
    static constexpr int32_t STONEBRICK_ID = 98;

    static constexpr DisguisedBlock DISGUISE_MAP[] = {
        {STONE_ID,       0},  // meta 0 → stone
        {COBBLESTONE_ID, 0},  // meta 1 → cobblestone
        {STONEBRICK_ID,  0},  // meta 2 → stone brick
        {STONEBRICK_ID,  1},  // meta 3 → mossy stone brick
        {STONEBRICK_ID,  2},  // meta 4 → cracked stone brick
        {STONEBRICK_ID,  3},  // meta 5 → chiseled stone brick
    };

    // ─── Reverse mapping (block+meta → silverfish meta) ───
    // Java: func_150195_a(Block, metadata)
    inline int32_t getInfestedMeta(int32_t blockId, int32_t blockMeta) {
        if (blockMeta == 0) {
            if (blockId == COBBLESTONE_ID) return 1;
            if (blockId == STONEBRICK_ID) return 2;
            return 0;  // stone
        }
        if (blockId == STONEBRICK_ID) {
            switch (blockMeta) {
                case 1: return 3;  // mossy
                case 2: return 4;  // cracked
                case 3: return 5;  // chiseled
                default: break;
            }
        }
        return 0;
    }

    // ─── Infestable blocks check ───
    // Java: func_150196_a — only stone, cobblestone, or stonebrick
    inline bool canInfest(int32_t blockId) {
        return blockId == STONE_ID || blockId == COBBLESTONE_ID || blockId == STONEBRICK_ID;
    }

    // ─── Silverfish spawn position ───
    // Java: (x + 0.5, y, z + 0.5), rotation (0, 0)
    static constexpr double SPAWN_OFFSET_XZ = 0.5;
}

} // namespace mccpp
