/**
 * BlockCrops.h — Crop growth, farmland hydration, and Nether portal mechanics.
 *
 * Java references:
 *   - net.minecraft.block.BlockCrops (144 lines)
 *   - net.minecraft.block.BlockFarmland (103 lines)
 *   - net.minecraft.block.BlockPortal (116 lines)
 *   - net.minecraft.block.BlockPortal$Size (portal frame validation)
 *
 * Three critical game mechanics blocks in one header.
 *
 * ═══════════════════════════════════════════════════════════════════════
 * CROP GROWTH (BlockCrops)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Growth stages: metadata 0-7 (0=planted, 7=fully mature)
 * Can only be placed on farmland block
 *
 * Growth tick (updateTick):
 *   1. Light level at Y+1 must be >= 9
 *   2. If stage < 7: random(25/growthRate + 1) == 0 → advance stage
 *   3. growthRate = sum of farmland bonuses from 3×3 area below
 *      - No farmland: +0
 *      - Dry farmland (meta=0): +1.0 (center) or +0.25 (corner/edge)
 *      - Wet farmland (meta>0): +3.0 (center) or +0.75 (corner/edge)
 *      - Adjacent same crop in both X and Z: growthRate /= 2
 *      - Diagonal same crop: growthRate /= 2
 *
 * Bone meal (fertilize):
 *   - Adds 2-5 growth stages (randomIntInRange(2,5))
 *   - Capped at 7
 *
 * Drops:
 *   - Stage 7: drop crop item (wheat) + 0-3 extra seeds
 *   - Stage < 7: drop seed item
 *   - Extra seed chance: random(15) <= stage
 *   - Fortune increases attempts by fortune level
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FARMLAND (BlockFarmland)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Moisture level: metadata 0-7 (0=dry, 7=fully hydrated)
 * Block bounds: full-width but 15/16 height (0.9375)
 *
 * Hydration tick (updateTick):
 *   1. If water within 4 blocks (±4 X, ±0..1 Y, ±4 Z) OR rain: set to 7
 *   2. Else if moisture > 0: decrement by 1
 *   3. Else if no crop above: revert to dirt
 *
 * Trampling (onFallenUpon):
 *   - Fall distance > 0.5: random(fallDist - 0.5) → turn to dirt
 *   - Non-player mobs: only if mobGriefing gamerule is true
 *
 * Neighbor change:
 *   - Solid block placed above farmland → revert to dirt
 *
 * Supported crops: wheat, melon_stem, pumpkin_stem, potatoes, carrots
 *
 * ═══════════════════════════════════════════════════════════════════════
 * NETHER PORTAL (BlockPortal)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Metadata: axis bits (& 3) — 0=unset, 1=X-axis, 2=Z-axis
 * No collision box (entities pass through)
 *
 * Portal creation (tryToCreatePortal):
 *   - Try both axis 1 (X) and axis 2 (Z)
 *   - Validate frame using BlockPortal$Size
 *   - Frame must be obsidian, interior must be air/fire
 *   - Minimum size: 3 wide × 3 tall, Maximum: 21 wide × 21 tall
 *   - If frame valid and interior has 0 portals: fill with portal blocks
 *
 * Zombie pigman spawning (updateTick):
 *   - Only in surface world (overworld)
 *   - If doMobSpawning rule: random(2000) < difficulty
 *   - Spawn entity 57 (zombie pigman) at solid surface below portal
 *   - Set portal cooldown on spawned entity
 *
 * Entity collision (onEntityCollidedWithBlock):
 *   - Not riding and not ridden: entity.setInPortal()
 *
 * Neighbor change:
 *   - Validate portal frame still intact
 *   - If frame broken: replace with air
 *
 * Thread safety: Block ticks happen on the server thread.
 * JNI readiness: Simple numeric constants.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Crop Growth Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CropConstants {
    // ─── Growth stages ───
    static constexpr int32_t MIN_STAGE = 0;
    static constexpr int32_t MAX_STAGE = 7;

    // ─── Light requirement ───
    // Java: world.getBlockLightValue(n, n2+1, n3) >= 9
    static constexpr int32_t MIN_LIGHT_LEVEL = 9;

    // ─── Growth rate formula ───
    // Java: random.nextInt((int)(25.0f / growthRate) + 1) == 0
    static constexpr float BASE_GROWTH_DIVISOR = 25.0f;

    // ─── Farmland growth bonuses ───
    // Java: func_149864_n
    static constexpr float DRY_FARMLAND_CENTER = 1.0f;
    static constexpr float WET_FARMLAND_CENTER = 3.0f;
    static constexpr float FARMLAND_EDGE_DIVISOR = 4.0f;
    static constexpr float ADJACENT_CROP_PENALTY = 2.0f;  // divide by 2
    static constexpr float BASE_GROWTH_RATE = 1.0f;

    // ─── Bone meal ───
    // Java: MathHelper.getRandomIntegerInRange(world.rand, 2, 5)
    static constexpr int32_t FERTILIZE_MIN = 2;
    static constexpr int32_t FERTILIZE_MAX = 5;

    // ─── Seed drops ───
    // Java: if (n4 >= 7) { n6 = 3 + n5(fortune); for ... random(15) > n4 }
    static constexpr int32_t SEED_DROP_BASE_ATTEMPTS = 3;
    static constexpr int32_t SEED_DROP_CHANCE = 15;

    // ─── Block IDs ───
    static constexpr int32_t FARMLAND_ID = 60;
    static constexpr int32_t WHEAT_ID = 59;  // crops block

    // ─── Crop height bounds ───
    // Java: setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.25f, 1.0f)
    static constexpr float CROP_HEIGHT = 0.25f;

    // ─── Growth rate computation ───
    // Java: func_149864_n — 3×3 farmland check below crop
    // Returns the growth rate modifier based on surrounding farmland
    inline float computeGrowthRate(bool farmlandBelow[9], bool farmlandWet[9],
                                     bool sameX, bool sameZ,
                                     bool sameDiagonal) {
        float rate = BASE_GROWTH_RATE;

        for (int32_t i = 0; i < 9; ++i) {
            if (!farmlandBelow[i]) continue;
            float bonus = farmlandWet[i] ? WET_FARMLAND_CENTER : DRY_FARMLAND_CENTER;
            // Index 4 = center position (directly below crop)
            if (i != 4) {
                bonus /= FARMLAND_EDGE_DIVISOR;
            }
            rate += bonus;
        }

        // Adjacent crop penalty
        // Java: if (bl3 || bl && bl2) f /= 2.0f
        // bl = same crop in X neighbors, bl2 = same crop in Z neighbors
        // bl3 = same crop in any diagonal
        if (sameDiagonal || (sameX && sameZ)) {
            rate /= ADJACENT_CROP_PENALTY;
        }

        return rate;
    }

    // ─── Should advance growth ───
    inline bool shouldGrow(float growthRate, int32_t randomValue) {
        int32_t threshold = static_cast<int32_t>(BASE_GROWTH_DIVISOR / growthRate) + 1;
        return (randomValue % threshold) == 0;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Farmland Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FarmlandConstants {
    // ─── Moisture levels ───
    static constexpr int32_t MIN_MOISTURE = 0;
    static constexpr int32_t MAX_MOISTURE = 7;

    // ─── Block height ───
    // Java: setBlockBounds(0, 0, 0, 1, 0.9375, 1)
    static constexpr float BLOCK_HEIGHT = 0.9375f;
    // Light opacity: 255 (fully opaque)
    static constexpr int32_t LIGHT_OPACITY = 255;

    // ─── Water search radius ───
    // Java: func_149821_m — search ±4 X, 0..+1 Y, ±4 Z for water
    static constexpr int32_t WATER_SEARCH_RADIUS_XZ = 4;
    static constexpr int32_t WATER_SEARCH_Y_MIN = 0;
    static constexpr int32_t WATER_SEARCH_Y_MAX = 1;

    // ─── Trampling ───
    // Java: onFallenUpon — world.rand.nextFloat() < f - 0.5f
    static constexpr float TRAMPLE_THRESHOLD = 0.5f;

    // ─── Supported crop block IDs ───
    // Java: func_149822_e — checks if crop is above farmland
    static constexpr int32_t WHEAT_ID = 59;
    static constexpr int32_t MELON_STEM_ID = 105;
    static constexpr int32_t PUMPKIN_STEM_ID = 104;
    static constexpr int32_t POTATOES_ID = 142;
    static constexpr int32_t CARROTS_ID = 141;

    // ─── Dirt block ID (conversion target) ───
    static constexpr int32_t DIRT_ID = 3;

    // Check if block is a supported crop
    inline bool isCropBlock(int32_t blockId) {
        return blockId == WHEAT_ID ||
               blockId == MELON_STEM_ID ||
               blockId == PUMPKIN_STEM_ID ||
               blockId == POTATOES_ID ||
               blockId == CARROTS_ID;
    }

    // Check if entity should trample farmland
    inline bool shouldTrample(float fallDistance, float randomFloat) {
        return randomFloat < (fallDistance - TRAMPLE_THRESHOLD);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Nether Portal Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PortalConstants {
    // ─── Metadata axis encoding ───
    // Java: func_149999_b(n) = n & 3
    static constexpr int32_t AXIS_MASK = 3;
    static constexpr int32_t AXIS_UNSET = 0;
    static constexpr int32_t AXIS_X = 1;
    static constexpr int32_t AXIS_Z = 2;

    // ─── Portal frame size limits ───
    // Java: BlockPortal$Size — validated in constructor
    static constexpr int32_t MIN_WIDTH = 2;   // interior width
    static constexpr int32_t MAX_WIDTH = 21;
    static constexpr int32_t MIN_HEIGHT = 3;  // interior height
    static constexpr int32_t MAX_HEIGHT = 21;

    // ─── Frame block ───
    static constexpr int32_t OBSIDIAN_ID = 49;
    static constexpr int32_t PORTAL_ID = 90;
    static constexpr int32_t FIRE_ID = 51;
    static constexpr int32_t AIR_ID = 0;

    // ─── Zombie pigman spawning ───
    // Java: random.nextInt(2000) < difficulty
    static constexpr int32_t PIGMAN_SPAWN_CHANCE = 2000;
    // Java: ItemMonsterPlacer.spawnCreature(world, 57, ...)
    static constexpr int32_t ZOMBIE_PIGMAN_ID = 57;
    // Spawn offset: +0.5 X, +1.1 Y, +0.5 Z
    static constexpr double SPAWN_OFFSET_X = 0.5;
    static constexpr double SPAWN_OFFSET_Y = 1.1;
    static constexpr double SPAWN_OFFSET_Z = 0.5;

    // ─── Block bounds for portal with axis ───
    // Java: setBlockBoundsBasedOnState
    static constexpr float THIN_HALF = 0.125f;  // 1/8 block
    static constexpr float FULL_HALF = 0.5f;

    // ─── Portal axis direction offsets ───
    // Java: field_150001_a = {{}, {3,1}, {2,0}}
    // Maps axis → {widthDir, lengthDir} for portal size validation
    // widthDir/lengthDir are direction indices:
    //   0=-Z, 1=+Z, 2=-X, 3=+X
    struct AxisDirections {
        int32_t dir1, dir2;
    };
    static constexpr AxisDirections AXIS_DIRS[] = {
        {0, 0},  // axis 0: unused
        {3, 1},  // axis 1 (X): width along Z, length along X
        {2, 0},  // axis 2 (Z): width along X, length along Z
    };

    // Extract portal axis from metadata
    inline int32_t getAxis(int32_t metadata) {
        return metadata & AXIS_MASK;
    }

    // Check if block can be inside portal frame
    inline bool isValidInterior(int32_t blockId) {
        return blockId == AIR_ID || blockId == FIRE_ID || blockId == PORTAL_ID;
    }

    // Check if block is part of portal frame
    inline bool isFrameBlock(int32_t blockId) {
        return blockId == OBSIDIAN_ID;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Additional crop types (block IDs for stem/melon/pumpkin growth)
// Java: BlockStem, BlockMelon
// ═══════════════════════════════════════════════════════════════════════════

namespace StemConstants {
    // ─── Stem growth stages ───
    static constexpr int32_t MAX_STAGE = 7;

    // ─── Stem block IDs ───
    static constexpr int32_t MELON_STEM_ID = 105;
    static constexpr int32_t PUMPKIN_STEM_ID = 104;

    // ─── Fruit block IDs ───
    static constexpr int32_t MELON_BLOCK_ID = 103;
    static constexpr int32_t PUMPKIN_BLOCK_ID = 86;

    // ─── Stem growth rate ───
    // Same formula as crops: random((int)(25/growthRate)+1) == 0
    // But stem has additional check: if stage == 7, try to place fruit
    // Fruit placement: random direction (±X, ±Z), target must be air,
    // block below target must be farmland, dirt, or grass

    // ─── Fruit placement requirements ───
    static constexpr int32_t DIRT_ID = 3;
    static constexpr int32_t GRASS_ID = 2;
    static constexpr int32_t FARMLAND_ID = 60;

    inline bool canSupportFruit(int32_t blockId) {
        return blockId == FARMLAND_ID || blockId == DIRT_ID || blockId == GRASS_ID;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Sapling growth (BlockSapling)
// Java: net.minecraft.block.BlockSapling (85 lines)
// ═══════════════════════════════════════════════════════════════════════════

namespace SaplingConstants {
    // ─── Sapling types (metadata bits 0-2) ───
    static constexpr int32_t OAK = 0;
    static constexpr int32_t SPRUCE = 1;
    static constexpr int32_t BIRCH = 2;
    static constexpr int32_t JUNGLE = 3;
    static constexpr int32_t ACACIA = 4;
    static constexpr int32_t DARK_OAK = 5;
    static constexpr int32_t TYPE_MASK = 7;

    // ─── Growth stage bit ───
    // Java: metadata bit 3 = ready to grow
    static constexpr int32_t STAGE_BIT = 8;

    // ─── Block ID ───
    static constexpr int32_t SAPLING_ID = 6;

    // ─── Light requirement ───
    static constexpr int32_t MIN_LIGHT = 9;

    // ─── Growth check ───
    // Java: if (random.nextInt(7) == 0) → try grow
    static constexpr int32_t GROWTH_CHANCE = 7;

    // ─── 2x2 mega tree check ───
    // Jungle and Dark Oak saplings need 2×2 arrangement for mega tree
    inline bool canGrowMega(int32_t type) {
        return type == JUNGLE || type == DARK_OAK;
    }
}

} // namespace mccpp
