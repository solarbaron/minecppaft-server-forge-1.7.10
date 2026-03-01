/**
 * FarmlandCrops.h — Farmland moisture mechanics and crop growth system.
 *
 * Java references:
 *   - net.minecraft.block.BlockFarmland (103 lines)
 *   - net.minecraft.block.BlockCrops (144 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FARMLAND (BlockFarmland)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: ground, tick randomly, light opacity 255
 * Height: 15/16 (0.9375) visual, full block collision (0→1)
 * Not opaque, not normal (due to height)
 *
 * Moisture (metadata 0-7):
 *   7 = fully hydrated (wet texture)
 *   0 = dry, will revert to dirt if no crop above
 *
 * updateTick hydration logic:
 *   1. If water within range OR rain falling → set to 7 (fully wet)
 *   2. Else if meta > 0 → decrement by 1 (drying)
 *   3. Else if meta == 0 AND no crop above → revert to dirt
 *
 * Water search (func_149821_m):
 *   9×2×9 area: x±4, y to y+1, z±4
 *   Any block with Material.water → hydrated
 *
 * Crop check (func_149822_e):
 *   Block directly above must be: wheat, melon_stem, pumpkin_stem,
 *   potatoes, or carrots
 *   (With n4=0, only checks the single block above)
 *
 * Trampling (onFallenUpon):
 *   If fall distance > 0.5:
 *     Chance = rand.nextFloat() < (fallDist - 0.5)
 *     Players: always eligible
 *     Non-players: only if mobGriefing gamerule is true
 *     → revert to dirt
 *
 * Neighbor change: if solid block above → revert to dirt
 * Drops: dirt
 *
 * Block ID: 60
 *
 * ═══════════════════════════════════════════════════════════════════════
 * CROPS (BlockCrops) — Wheat, Potatoes, Carrots
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: plants, extends BlockBush, implements IGrowable
 * Hardness: 0.0, step sound grass, no creative tab
 * Placement: farmland only
 * Height: 0.25 (25%), render type 6 (cross)
 *
 * Growth stages: meta 0-7 (7 = mature)
 *
 * Growth rate formula (func_149864_n):
 *   Base: scan 3×3 farmland below crop:
 *     Center farmland: dry=1.0, hydrated(meta>0)=3.0
 *     Adjacent farmland: dry=0.25, hydrated=0.75
 *   Penalty: if diagonal same crop exists, OR both adjacent axes
 *     have same crop → rate /= 2
 *
 * Growth chance per tick:
 *   Requires light >= 9 above
 *   Chance = 1 in (floor(25/rate) + 1)
 *   Best case (hydrated, no neighbors): rate=13, chance=1/2
 *   Worst case (dry, crowded): rate=0.5, chance=1/51
 *
 * Bonemeal:
 *   canFertilize: meta != 7
 *   shouldFertilize: always true
 *   fertilize: meta += rand(2,5), capped at 7
 *
 * Drops:
 *   Mature (meta 7): getCrop() (wheat) + bonus seeds
 *   Immature: getSeed() (wheat_seeds)
 *   Bonus seeds: 3+fortune attempts, each has meta/15 chance
 *
 * Subclasses:
 *   BlockCrops = wheat (getSeed=wheat_seeds, getCrop=wheat)
 *   BlockPotato = potatoes (getSeed=potato, getCrop=potato, poison potato 1/50)
 *   BlockCarrot = carrots (getSeed=carrot, getCrop=carrot)
 *
 * Block IDs: wheat(59), potatoes(142), carrots(141)
 *
 * Thread safety: Block tick on server thread.
 * JNI readiness: Growth rate formula accessible for Forge crop events.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Farmland Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FarmlandConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 60;

    // ─── Dimensions ───
    static constexpr float VISUAL_HEIGHT = 0.9375f;   // 15/16
    // Collision: full block (0→1)

    // ─── Moisture ───
    static constexpr int32_t MAX_MOISTURE = 7;
    static constexpr int32_t DRY = 0;

    // ─── Water search ───
    // 9×2×9: x±4, y to y+1, z±4
    static constexpr int32_t WATER_SEARCH_XZ = 4;
    static constexpr int32_t WATER_SEARCH_Y_MIN = 0;    // same level
    static constexpr int32_t WATER_SEARCH_Y_MAX = 1;    // one above

    // ─── Crop preservation ───
    // Crops that prevent dry farmland from reverting:
    // wheat(59), melon_stem(105), pumpkin_stem(104), potatoes(142), carrots(141)
    static constexpr int32_t WHEAT_ID = 59;
    static constexpr int32_t MELON_STEM_ID = 105;
    static constexpr int32_t PUMPKIN_STEM_ID = 104;
    static constexpr int32_t POTATOES_ID = 142;
    static constexpr int32_t CARROTS_ID = 141;

    // ─── Trampling ───
    static constexpr float TRAMPLE_MIN_FALL = 0.5f;  // fallDist - 0.5 = chance

    // ─── Drops ───
    static constexpr int32_t DIRT_ID = 3;
}

// ═══════════════════════════════════════════════════════════════════════════
// Crop Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CropConstants {
    // ─── Block IDs ───
    static constexpr int32_t WHEAT_ID = 59;
    static constexpr int32_t POTATOES_ID = 142;
    static constexpr int32_t CARROTS_ID = 141;

    // ─── Growth ───
    static constexpr int32_t MAX_GROWTH = 7;           // meta 0-7
    static constexpr int32_t MIN_LIGHT = 9;             // light >= 9 above
    static constexpr float GROWTH_RATE_BASE = 25.0f;    // chance = floor(25/rate)+1

    // ─── Growth rate formula ───
    // Farmland bonus at each position in 3×3 below crop:
    static constexpr float FARMLAND_DRY = 1.0f;        // dry farmland
    static constexpr float FARMLAND_WET = 3.0f;         // hydrated (meta > 0)
    static constexpr float ADJACENT_DIVISOR = 4.0f;     // non-center / 4
    static constexpr float CROWDED_PENALTY = 2.0f;      // rate / 2

    // ─── Bonemeal ───
    static constexpr int32_t BONEMEAL_MIN = 2;
    static constexpr int32_t BONEMEAL_MAX = 5;         // += rand(2, 5)

    // ─── Drops ───
    // Mature: getCrop() + bonus seeds (3+fortune attempts)
    // Each attempt: rand(15) > meta → skip, else drop seed
    static constexpr int32_t SEED_BASE_ATTEMPTS = 3;
    static constexpr int32_t SEED_DROP_DENOMINATOR = 15;

    // ─── Wheat items ───
    static constexpr int32_t WHEAT_SEEDS_ITEM = 295;
    static constexpr int32_t WHEAT_ITEM = 296;

    // ─── Potato items ───
    static constexpr int32_t POTATO_ITEM = 392;
    static constexpr int32_t POISONOUS_POTATO = 394;
    static constexpr int32_t POISON_POTATO_CHANCE = 50;  // 1/50

    // ─── Carrot items ───
    static constexpr int32_t CARROT_ITEM = 391;

    // ─── Bounds ───
    static constexpr float HEIGHT = 0.25f;
    static constexpr int32_t RENDER_TYPE = 6;           // cross
}

} // namespace mccpp
