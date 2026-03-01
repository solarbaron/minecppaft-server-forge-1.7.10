/**
 * BrewingBeacon.h — Brewing Stand and Beacon tile entity mechanics.
 *
 * Java references:
 *   - net.minecraft.tileentity.TileEntityBrewingStand (270 lines)
 *   - net.minecraft.tileentity.TileEntityBeacon (240 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BREWING STAND (TileEntityBrewingStand)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Inventory: 4 slots
 *   - Slots 0-2: potion bottles (output)
 *   - Slot 3: ingredient (input)
 *   - ISidedInventory: face 1 (top) → slot 3, all other faces → slots 0-2
 *
 * Brew cycle:
 *   1. If brewTime == 0 && canBrew(): start (brewTime = 400, save ingredient)
 *   2. Each tick: brewTime-- (if > 0)
 *   3. If brewTime reaches 0: call brewPotions()
 *   4. If !canBrew() or ingredient changed: cancel (brewTime = 0)
 *
 * canBrew() validation:
 *   - Slot 3 must have a potion ingredient (isPotionIngredient)
 *   - At least one bottle in slots 0-2 must be Items.potionitem
 *   - The ingredient must actually change the potion's metadata
 *   - Different effects list, OR non-splash → splash conversion
 *
 * brewPotions():
 *   - For each bottle in slots 0-2 that is potionitem:
 *     - Compute new metadata: PotionHelper.applyIngredient(old, ingredientEffect)
 *     - If potion changes (effects differ or splash conversion): update metadata
 *   - Consume ingredient: if hasContainerItem → replace, else decrement/null
 *
 * Metadata (block): filledSlots bitmask (bit 0=slot0, bit 1=slot1, bit 2=slot2)
 *
 * NBT: Items (TagList), BrewTime (short), CustomName (string)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BEACON (TileEntityBeacon)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Update cycle: every 80 ticks (4 seconds)
 *   1. func_146003_y(): Check pyramid structure
 *   2. func_146000_x(): Apply potion effects to nearby players
 *
 * Pyramid structure (func_146003_y):
 *   - Need sky access at Y+1
 *   - Check layers 1-4 below beacon:
 *     Layer N: (2N+1)² blocks centered under beacon
 *     Valid blocks: iron_block, gold_block, diamond_block, emerald_block
 *   - levels = highest complete layer (0-4)
 *   - Level 4 first time: trigger fullBeacon achievement
 *
 * Effect application (func_146000_x):
 *   - Requires: isComplete && levels > 0 && primaryEffect > 0
 *   - Range: levels * 10 + 10 blocks (AABB, Y extends to world height)
 *   - Primary effect: PotionEffect(primaryId, 180 ticks, amplifier, ambient)
 *   - Amplifier: 0 normally, 1 if levels >= 4 AND primary == secondary
 *   - If levels >= 4 AND primary != secondary AND secondary > 0:
 *     Additional effect: PotionEffect(secondaryId, 180 ticks, 0, ambient)
 *
 * Effect tiers:
 *   Level 1: Speed(1), Haste(3)
 *   Level 2: Resistance(11), Jump Boost(8)
 *   Level 3: Strength(5)
 *   Level 4: Regeneration(10) — as secondary only
 *
 * setPrimaryEffect validation:
 *   - Must be in effectsList[0..min(levels-1, 2)]
 *
 * setSecondaryEffect validation:
 *   - Requires levels >= 4
 *   - Must be in effectsList[0..3]
 *
 * Inventory: 1 slot (payment)
 *   - Valid items: emerald, diamond, gold_ingot, iron_ingot
 *   - Stack limit: 1
 *
 * NBT: Primary (int), Secondary (int), Levels (int)
 * Description packet: S35PacketUpdateTileEntity (type 3)
 *
 * Thread safety: Tile entity updates happen on the server thread.
 * JNI readiness: Simple POD data and potion ID constants.
 */
#pragma once

#include <cstdint>
#include <array>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Brewing Stand Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace BrewingConstants {
    // ─── Inventory layout ───
    // Java: brewingItemStacks = new ItemStack[4]
    static constexpr int32_t TOTAL_SLOTS = 4;
    static constexpr int32_t BOTTLE_SLOTS = 3;       // slots 0-2
    static constexpr int32_t INGREDIENT_SLOT = 3;     // slot 3

    // ─── ISidedInventory ───
    // Java: inputSlots = {3}, outputSlots = {0, 1, 2}
    // Face 1 (top) → ingredient slot
    // All other faces → bottle slots
    static constexpr int32_t INPUT_FACE = 1;  // top face

    // ─── Brew time ───
    // Java: this.brewTime = 400
    static constexpr int32_t BREW_DURATION = 400;

    // ─── Stack limit ───
    static constexpr int32_t STACK_LIMIT = 64;

    // ─── Useable distance ───
    // Java: getDistanceSq > 64.0
    static constexpr double USE_DISTANCE_SQ = 64.0;

    // ─── Block ID ───
    static constexpr int32_t BREWING_STAND_ID = 117;

    // ─── Potion item ID ───
    static constexpr int32_t POTION_ITEM_ID = 373;
    static constexpr int32_t GLASS_BOTTLE_ID = 374;

    // ─── Splash potion detection ───
    // Java: ItemPotion.isSplash(metadata) — check bit 14 (0x4000)
    static constexpr int32_t SPLASH_BIT = 0x4000;

    inline bool isSplash(int32_t potionMeta) {
        return (potionMeta & SPLASH_BIT) != 0;
    }

    // ─── Filled slots bitmask ───
    // Java: getFilledSlots — bit 0=slot0, bit 1=slot1, bit 2=slot2
    inline int32_t computeFilledSlots(bool slot0, bool slot1, bool slot2) {
        int32_t mask = 0;
        if (slot0) mask |= 1;
        if (slot1) mask |= 2;
        if (slot2) mask |= 4;
        return mask;
    }

    // ─── NBT tags ───
    static constexpr const char* TAG_ITEMS = "Items";
    static constexpr const char* TAG_BREW_TIME = "BrewTime";
    static constexpr const char* TAG_CUSTOM_NAME = "CustomName";
    static constexpr const char* TAG_SLOT = "Slot";

    // ─── Default container name ───
    static constexpr const char* DEFAULT_NAME = "container.brewing";
}

// ═══════════════════════════════════════════════════════════════════════════
// Potion Helper Constants
// Java: net.minecraft.potion.PotionHelper — bit manipulation for potion data
// ═══════════════════════════════════════════════════════════════════════════

namespace PotionHelperConstants {
    // ─── Potion data bits ───
    // Bits 0-3: effect ID (0-15 base)
    // Bit 5: extended duration
    // Bit 6: amplified
    // Bits 8-13: ingredient modifier
    // Bit 14: splash

    // ─── applyIngredient ───
    // Java: PotionHelper.applyIngredient(int potionData, String ingredient)
    // Complex bit manipulation — the ingredient string encodes operations:
    //   +/- operations on specific bit ranges
    //   & operations for masking
    //   | operations for setting

    // ─── Base potion data values ───
    static constexpr int32_t AWKWARD_POTION = 16;     // Nether wart base
    static constexpr int32_t MUNDANE_POTION = 0;
    static constexpr int32_t THICK_POTION = 32;

    // ─── Ingredient item IDs ───
    static constexpr int32_t NETHER_WART_ID = 372;
    static constexpr int32_t GLOWSTONE_DUST_ID = 348;
    static constexpr int32_t REDSTONE_ID = 331;
    static constexpr int32_t FERMENTED_SPIDER_EYE_ID = 376;
    static constexpr int32_t GUNPOWDER_ID = 289;
    static constexpr int32_t BLAZE_POWDER_ID = 377;
    static constexpr int32_t SUGAR_ID = 353;
    static constexpr int32_t SPIDER_EYE_ID = 375;
    static constexpr int32_t GHAST_TEAR_ID = 370;
    static constexpr int32_t MAGMA_CREAM_ID = 378;
    static constexpr int32_t GOLDEN_CARROT_ID = 396;
    static constexpr int32_t PUFFERFISH_ID = 349;  // meta 3
}

// ═══════════════════════════════════════════════════════════════════════════
// Beacon Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace BeaconConstants {
    // ─── Pyramid levels ───
    static constexpr int32_t MAX_LEVELS = 4;
    static constexpr int32_t MIN_ACTIVE_LEVELS = 1;

    // ─── Update interval ───
    // Java: worldObj.getTotalWorldTime() % 80L == 0
    static constexpr int64_t UPDATE_INTERVAL = 80;

    // ─── Range formula ───
    // Java: double d = this.levels * 10 + 10
    static constexpr int32_t RANGE_PER_LEVEL = 10;
    static constexpr int32_t RANGE_BASE = 10;

    inline double getRange(int32_t levels) {
        return static_cast<double>(levels * RANGE_PER_LEVEL + RANGE_BASE);
    }

    // ─── Effect duration ───
    // Java: PotionEffect(id, 180, amplifier, true)
    static constexpr int32_t EFFECT_DURATION = 180;  // 9 seconds
    static constexpr bool EFFECT_AMBIENT = true;

    // ─── Amplifier ───
    // Java: if levels >= 4 && primary == secondary: amplifier = 1
    static constexpr int32_t AMPLIFIED_LEVEL_REQ = 4;
    static constexpr int32_t AMPLIFIED_VALUE = 1;
    static constexpr int32_t NORMAL_AMPLIFIER = 0;

    // ─── Effect tiers ───
    // Java: effectsList = {{Speed,Haste}, {Resistance,Jump}, {Strength}, {Regen}}
    // Potion IDs from Potion class:
    static constexpr int32_t SPEED_ID = 1;
    static constexpr int32_t HASTE_ID = 3;
    static constexpr int32_t RESISTANCE_ID = 11;
    static constexpr int32_t JUMP_BOOST_ID = 8;
    static constexpr int32_t STRENGTH_ID = 5;
    static constexpr int32_t REGENERATION_ID = 10;

    // Effect tier layout: [level_index] = {effect_ids...}
    struct EffectTier {
        int32_t effectIds[2];
        int32_t count;
    };

    static constexpr EffectTier EFFECT_TIERS[] = {
        {{SPEED_ID, HASTE_ID}, 2},          // Level 1
        {{RESISTANCE_ID, JUMP_BOOST_ID}, 2}, // Level 2
        {{STRENGTH_ID, 0}, 1},               // Level 3
        {{REGENERATION_ID, 0}, 1},            // Level 4
    };

    // ─── Pyramid block IDs ───
    // Java: iron_block, gold_block, diamond_block, emerald_block
    static constexpr int32_t IRON_BLOCK_ID = 42;
    static constexpr int32_t GOLD_BLOCK_ID = 41;
    static constexpr int32_t DIAMOND_BLOCK_ID = 57;
    static constexpr int32_t EMERALD_BLOCK_ID = 133;

    inline bool isPyramidBlock(int32_t blockId) {
        return blockId == IRON_BLOCK_ID ||
               blockId == GOLD_BLOCK_ID ||
               blockId == DIAMOND_BLOCK_ID ||
               blockId == EMERALD_BLOCK_ID;
    }

    // ─── Pyramid layer sizes ───
    // Layer N: (2N+1)² blocks centered under beacon
    // Layer 1: 3×3 (9 blocks)
    // Layer 2: 5×5 (25 blocks)
    // Layer 3: 7×7 (49 blocks)
    // Layer 4: 9×9 (81 blocks)
    inline int32_t getLayerSize(int32_t level) {
        return 2 * level + 1;
    }

    // ─── Payment items ───
    // Java: emerald, diamond, gold_ingot, iron_ingot
    static constexpr int32_t EMERALD_ITEM_ID = 388;
    static constexpr int32_t DIAMOND_ITEM_ID = 264;
    static constexpr int32_t GOLD_INGOT_ID = 266;
    static constexpr int32_t IRON_INGOT_ID = 265;

    inline bool isValidPayment(int32_t itemId) {
        return itemId == EMERALD_ITEM_ID ||
               itemId == DIAMOND_ITEM_ID ||
               itemId == GOLD_INGOT_ID ||
               itemId == IRON_INGOT_ID;
    }

    // ─── Inventory ───
    static constexpr int32_t INVENTORY_SIZE = 1;
    static constexpr int32_t STACK_LIMIT = 1;

    // ─── Useable distance ───
    static constexpr double USE_DISTANCE_SQ = 64.0;

    // ─── Block ID ───
    static constexpr int32_t BEACON_ID = 138;

    // ─── Description packet type ───
    // Java: S35PacketUpdateTileEntity type 3
    static constexpr int32_t PACKET_TYPE = 3;

    // ─── NBT tags ───
    static constexpr const char* TAG_PRIMARY = "Primary";
    static constexpr const char* TAG_SECONDARY = "Secondary";
    static constexpr const char* TAG_LEVELS = "Levels";

    // ─── Default container name ───
    static constexpr const char* DEFAULT_NAME = "container.beacon";

    // ─── Achievement check ───
    // Java: if levels == 4 && prevLevels < levels → fullBeacon
    static constexpr int32_t ACHIEVEMENT_LEVEL = 4;
    // Achievement check AABB: expand(10, 5, 10) centered on beacon
    static constexpr double ACHIEVEMENT_RANGE_XZ = 10.0;
    static constexpr double ACHIEVEMENT_RANGE_Y = 5.0;

    // ─── Validate primary effect for given level ───
    // Java: for (i = 0; i < levels && i < 3; i++) check effectsList[i]
    inline bool isValidPrimary(int32_t effectId, int32_t levels) {
        int32_t maxTier = (levels < 3) ? levels : 3;
        for (int32_t i = 0; i < maxTier; ++i) {
            for (int32_t j = 0; j < EFFECT_TIERS[i].count; ++j) {
                if (EFFECT_TIERS[i].effectIds[j] == effectId) return true;
            }
        }
        return false;
    }

    // ─── Validate secondary effect ───
    // Java: requires levels >= 4, check all 4 tiers
    inline bool isValidSecondary(int32_t effectId, int32_t levels) {
        if (levels < 4) return false;
        for (int32_t i = 0; i < 4; ++i) {
            for (int32_t j = 0; j < EFFECT_TIERS[i].count; ++j) {
                if (EFFECT_TIERS[i].effectIds[j] == effectId) return true;
            }
        }
        return false;
    }
}

} // namespace mccpp
