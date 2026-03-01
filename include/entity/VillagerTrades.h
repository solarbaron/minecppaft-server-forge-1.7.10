/**
 * VillagerTrades.h — Villager profession trade tables and recipe generation.
 *
 * Java reference:
 *   - net.minecraft.entity.passive.EntityVillager (581 lines)
 *   - addDefaultEquipmentAndRecipies, villagersSellingList, blacksmithSellingList
 *
 * ═══════════════════════════════════════════════════════════════════════
 * VILLAGER PROFESSIONS
 * ═══════════════════════════════════════════════════════════════════════
 *
 * 5 professions (DataWatcher key 16):
 *   0 = Farmer  (brown robe)
 *   1 = Librarian (white robe)
 *   2 = Priest/Cleric (purple robe)
 *   3 = Blacksmith (black apron)
 *   4 = Butcher (white apron)
 *
 * On spawn: random profession 0-4 via rand.nextInt(5)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TRADE GENERATION
 * ═══════════════════════════════════════════════════════════════════════
 *
 * addDefaultEquipmentAndRecipies(count):
 *   1. Compute probability adjustment:
 *      field_82191_bN = sqrt(existingRecipesSize) * 0.2f
 *   2. adjustProbability(base): result = base + field_82191_bN
 *      if > 0.9 → 0.9 - (result - 0.9)  [reflects back]
 *   3. Per-profession recipe generation (prob-gated)
 *   4. If no recipes generated: add gold_ingot buy (prob 1.0)
 *   5. Shuffle recipes randomly
 *   6. Add 'count' recipes to buyingList (addToListWithCheck)
 *
 * Two recipe types:
 *   func_146091_a — BUY: player sells items TO villager for emeralds
 *     Stack size from villagersSellingList range
 *   func_146089_b — SELL: villager sells items TO player for emeralds
 *     Price from blacksmithSellingList range
 *     Negative price → villager gives items AND emeralds (food etc.)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TRADE REFRESH
 * ═══════════════════════════════════════════════════════════════════════
 *
 * When buying player purchases the LAST recipe in the list:
 *   timeUntilReset = 40 (2 seconds)
 *   needsInitilization = true
 *
 * On timer expiry:
 *   - Disabled recipes: add rand(6)+rand(6)+2 max uses
 *   - Call addDefaultEquipmentAndRecipies(1) — add 1 new recipe
 *   - Apply Regeneration I for 200 ticks (10 seconds)
 *   - If village & lastBuyer: +1 reputation
 *
 * ═══════════════════════════════════════════════════════════════════════
 * REPUTATION SYSTEM
 * ═══════════════════════════════════════════════════════════════════════
 *
 * setRevengeTarget → -1 rep (or -3 if child)
 * onDeath by player → -2 rep
 * onDeath by mob → endMatingSeason
 * Successful trade → +1 rep
 * New villager arrives → +5 rep (isLookingForHome)
 *
 * Thread safety: Entity update on server thread.
 * JNI readiness: Simple price tables and constants.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Villager Profession IDs
// ═══════════════════════════════════════════════════════════════════════════

namespace VillagerProfession {
    static constexpr int32_t FARMER = 0;
    static constexpr int32_t LIBRARIAN = 1;
    static constexpr int32_t PRIEST = 2;     // Cleric
    static constexpr int32_t BLACKSMITH = 3;
    static constexpr int32_t BUTCHER = 4;
    static constexpr int32_t NUM_PROFESSIONS = 5;

    // DataWatcher key for profession
    static constexpr int32_t DATAWATCHER_KEY = 16;
}

// ═══════════════════════════════════════════════════════════════════════════
// Item IDs for trade references
// ═══════════════════════════════════════════════════════════════════════════

namespace TradeItemIds {
    static constexpr int32_t EMERALD = 388;
    static constexpr int32_t WHEAT = 296;
    static constexpr int32_t WOOL_BLOCK = 35;
    static constexpr int32_t CHICKEN_RAW = 365;
    static constexpr int32_t COOKED_FISH = 350;
    static constexpr int32_t BREAD = 297;
    static constexpr int32_t MELON = 360;
    static constexpr int32_t APPLE = 260;
    static constexpr int32_t COOKIE = 357;
    static constexpr int32_t SHEARS = 359;
    static constexpr int32_t FLINT_AND_STEEL = 259;
    static constexpr int32_t COOKED_CHICKEN = 366;
    static constexpr int32_t ARROW = 262;
    static constexpr int32_t GRAVEL_BLOCK = 13;
    static constexpr int32_t FLINT = 318;
    static constexpr int32_t COAL = 263;
    static constexpr int32_t IRON_INGOT = 265;
    static constexpr int32_t GOLD_INGOT = 266;
    static constexpr int32_t DIAMOND = 264;
    static constexpr int32_t PAPER = 339;
    static constexpr int32_t BOOK = 340;
    static constexpr int32_t WRITTEN_BOOK = 387;
    static constexpr int32_t BOOKSHELF_BLOCK = 47;
    static constexpr int32_t GLASS_BLOCK = 20;
    static constexpr int32_t COMPASS = 345;
    static constexpr int32_t CLOCK = 347;
    static constexpr int32_t ENCHANTED_BOOK = 403;
    static constexpr int32_t ENDER_EYE = 381;
    static constexpr int32_t ENDER_PEARL = 368;
    static constexpr int32_t XP_BOTTLE = 384;
    static constexpr int32_t REDSTONE = 331;
    static constexpr int32_t GLOWSTONE_BLOCK = 89;
    static constexpr int32_t IRON_SWORD = 267;
    static constexpr int32_t DIAMOND_SWORD = 276;
    static constexpr int32_t IRON_AXE = 258;
    static constexpr int32_t DIAMOND_AXE = 279;
    static constexpr int32_t IRON_PICKAXE = 257;
    static constexpr int32_t DIAMOND_PICKAXE = 278;
    static constexpr int32_t IRON_SHOVEL = 256;
    static constexpr int32_t DIAMOND_SHOVEL = 277;
    static constexpr int32_t IRON_HOE = 292;
    static constexpr int32_t DIAMOND_HOE = 293;
    static constexpr int32_t IRON_BOOTS = 309;
    static constexpr int32_t DIAMOND_BOOTS = 313;
    static constexpr int32_t IRON_HELMET = 306;
    static constexpr int32_t DIAMOND_HELMET = 310;
    static constexpr int32_t IRON_CHESTPLATE = 307;
    static constexpr int32_t DIAMOND_CHESTPLATE = 311;
    static constexpr int32_t IRON_LEGGINGS = 308;
    static constexpr int32_t DIAMOND_LEGGINGS = 312;
    static constexpr int32_t CHAINMAIL_BOOTS = 305;
    static constexpr int32_t CHAINMAIL_HELMET = 302;
    static constexpr int32_t CHAINMAIL_CHESTPLATE = 303;
    static constexpr int32_t CHAINMAIL_LEGGINGS = 304;
    static constexpr int32_t PORKCHOP = 319;
    static constexpr int32_t BEEF = 363;
    static constexpr int32_t SADDLE = 329;
    static constexpr int32_t LEATHER_CHESTPLATE = 299;
    static constexpr int32_t LEATHER_BOOTS = 301;
    static constexpr int32_t LEATHER_HELMET = 298;
    static constexpr int32_t LEATHER_LEGGINGS = 300;
    static constexpr int32_t COOKED_PORKCHOP = 320;
    static constexpr int32_t COOKED_BEEF = 364;
}

// ═══════════════════════════════════════════════════════════════════════════
// Buy Price Table (villagersSellingList)
// Player sells X items → gets 1 emerald
// Values: {min, max} stack size the player must provide
// ═══════════════════════════════════════════════════════════════════════════

struct PriceRange {
    int32_t itemId;
    int32_t min;
    int32_t max;
};

namespace BuyPrices {
    // player sells these to villager for emeralds
    static constexpr PriceRange TABLE[] = {
        {TradeItemIds::COAL,          16, 24},
        {TradeItemIds::IRON_INGOT,     8, 10},
        {TradeItemIds::GOLD_INGOT,     8, 10},
        {TradeItemIds::DIAMOND,        4,  6},
        {TradeItemIds::PAPER,         24, 36},
        {TradeItemIds::BOOK,          11, 13},
        {TradeItemIds::WRITTEN_BOOK,   1,  1},
        {TradeItemIds::ENDER_PEARL,    3,  4},
        {TradeItemIds::ENDER_EYE,      2,  3},
        {TradeItemIds::PORKCHOP,      14, 18},
        {TradeItemIds::BEEF,          14, 18},
        {TradeItemIds::CHICKEN_RAW,   14, 18},
        {TradeItemIds::COOKED_FISH,    9, 13},
        // wheat_seeds = 34-48 (item ID 295)
        // melon_seeds = 30-38 (item ID 362)
        // pumpkin_seeds = 30-38 (item ID 361)
        {TradeItemIds::WHEAT,         18, 22},
        {TradeItemIds::WOOL_BLOCK,    14, 22},
        // rotten_flesh = 36-64 (item ID 367)
    };
    static constexpr int32_t TABLE_SIZE = 16;
}

// ═══════════════════════════════════════════════════════════════════════════
// Sell Price Table (blacksmithSellingList)
// Villager sells 1 item for X emeralds
// Negative values = villager sells X items for 1 emerald (food/cheap items)
// ═══════════════════════════════════════════════════════════════════════════

namespace SellPrices {
    static constexpr PriceRange TABLE[] = {
        // Tools
        {TradeItemIds::FLINT_AND_STEEL,    3,   4},
        {TradeItemIds::SHEARS,             3,   4},
        {TradeItemIds::IRON_SWORD,         7,  11},
        {TradeItemIds::DIAMOND_SWORD,     12,  14},
        {TradeItemIds::IRON_AXE,           6,   8},
        {TradeItemIds::DIAMOND_AXE,        9,  12},
        {TradeItemIds::IRON_PICKAXE,       7,   9},
        {TradeItemIds::DIAMOND_PICKAXE,   10,  12},
        {TradeItemIds::IRON_SHOVEL,        4,   6},
        {TradeItemIds::DIAMOND_SHOVEL,     7,   8},
        {TradeItemIds::IRON_HOE,           4,   6},
        {TradeItemIds::DIAMOND_HOE,        7,   8},
        // Armor
        {TradeItemIds::IRON_BOOTS,         4,   6},
        {TradeItemIds::DIAMOND_BOOTS,      7,   8},
        {TradeItemIds::IRON_HELMET,        4,   6},
        {TradeItemIds::DIAMOND_HELMET,     7,   8},
        {TradeItemIds::IRON_CHESTPLATE,   10,  14},
        {TradeItemIds::DIAMOND_CHESTPLATE, 16,  19},
        {TradeItemIds::IRON_LEGGINGS,      8,  10},
        {TradeItemIds::DIAMOND_LEGGINGS,  11,  14},
        {TradeItemIds::CHAINMAIL_BOOTS,    5,   7},
        {TradeItemIds::CHAINMAIL_HELMET,   5,   7},
        {TradeItemIds::CHAINMAIL_CHESTPLATE, 11, 15},
        {TradeItemIds::CHAINMAIL_LEGGINGS, 9,  11},
        // Food (negative = villager gives items for 1 emerald)
        {TradeItemIds::BREAD,             -4,  -2},
        {TradeItemIds::MELON,             -8,  -4},
        {TradeItemIds::APPLE,             -8,  -4},
        {TradeItemIds::COOKIE,           -10,  -7},
        // Blocks
        {TradeItemIds::GLASS_BLOCK,       -5,  -3},
        {TradeItemIds::BOOKSHELF_BLOCK,    3,   4},
        // Leather armor
        {TradeItemIds::LEATHER_CHESTPLATE, 4,   5},
        {TradeItemIds::LEATHER_BOOTS,      2,   4},
        {TradeItemIds::LEATHER_HELMET,     2,   4},
        {TradeItemIds::LEATHER_LEGGINGS,   2,   4},
        // Other
        {TradeItemIds::SADDLE,             6,   8},
        {TradeItemIds::XP_BOTTLE,         -4,  -1},
        {TradeItemIds::REDSTONE,          -4,  -1},
        {TradeItemIds::COMPASS,           10,  12},
        {TradeItemIds::CLOCK,             10,  12},
        {TradeItemIds::GLOWSTONE_BLOCK,   -3,  -1},
        // Cooked meats (negative)
        {TradeItemIds::COOKED_PORKCHOP,   -7,  -5},
        {TradeItemIds::COOKED_BEEF,       -7,  -5},
        {TradeItemIds::COOKED_CHICKEN,    -8,  -6},
        // Eye of Ender (positive = sell for emeralds)
        {TradeItemIds::ENDER_EYE,          7,  11},
        // Arrow (negative = give arrows for 1 emerald)
        {TradeItemIds::ARROW,            -12,  -8},
    };
    static constexpr int32_t TABLE_SIZE = 45;
}

// ═══════════════════════════════════════════════════════════════════════════
// Per-Profession Trade Recipes
// ═══════════════════════════════════════════════════════════════════════════

// Trade entry: item ID, whether it's a buy or sell, base probability
struct TradeRecipeEntry {
    int32_t itemId;
    bool isBuyRecipe;  // true = player sells to villager, false = villager sells
    float baseProbability;
};

namespace FarmerTrades {
    // Profession 0: Brown robe
    static constexpr TradeRecipeEntry RECIPES[] = {
        // BUY (player → villager)
        {TradeItemIds::WHEAT,          true,  0.9f},
        {TradeItemIds::WOOL_BLOCK,     true,  0.5f},
        {TradeItemIds::CHICKEN_RAW,    true,  0.5f},
        {TradeItemIds::COOKED_FISH,    true,  0.4f},
        // SELL (villager → player)
        {TradeItemIds::BREAD,          false, 0.9f},
        {TradeItemIds::MELON,          false, 0.3f},
        {TradeItemIds::APPLE,          false, 0.3f},
        {TradeItemIds::COOKIE,         false, 0.3f},
        {TradeItemIds::SHEARS,         false, 0.3f},
        {TradeItemIds::FLINT_AND_STEEL,false, 0.3f},
        {TradeItemIds::COOKED_CHICKEN, false, 0.3f},
        {TradeItemIds::ARROW,          false, 0.5f},
    };
    static constexpr int32_t RECIPE_COUNT = 12;
    // Special: gravel → flint, prob 0.5
    // Java: 10 gravel → 4-5 flint (+ emerald as input)
    static constexpr float GRAVEL_FLINT_PROB = 0.5f;
    static constexpr int32_t GRAVEL_COUNT = 10;
    static constexpr int32_t FLINT_MIN = 4;
    static constexpr int32_t FLINT_RANDOM = 2;  // rand(2)
}

namespace LibrarianTrades {
    // Profession 1: White robe
    static constexpr TradeRecipeEntry RECIPES[] = {
        // BUY
        {TradeItemIds::PAPER,          true,  0.8f},
        {TradeItemIds::BOOK,           true,  0.8f},
        {TradeItemIds::WRITTEN_BOOK,   true,  0.3f},
        // SELL
        {TradeItemIds::BOOKSHELF_BLOCK,false, 0.8f},
        {TradeItemIds::GLASS_BLOCK,    false, 0.2f},
        {TradeItemIds::COMPASS,        false, 0.2f},
        {TradeItemIds::CLOCK,          false, 0.2f},
    };
    static constexpr int32_t RECIPE_COUNT = 7;
    // Special: enchanted book trade, prob 0.07
    // Java: random enchantment from enchantmentsBookList
    //   price = 2 + rand(5 + level*10) + 3*level emeralds
    //   input: book + emeralds
    //   output: enchanted book
    static constexpr float ENCH_BOOK_PROB = 0.07f;
    static constexpr int32_t ENCH_BOOK_BASE_PRICE = 2;
    static constexpr int32_t ENCH_BOOK_RAND_BASE = 5;
    static constexpr int32_t ENCH_BOOK_RAND_PER_LVL = 10;
    static constexpr int32_t ENCH_BOOK_PRICE_PER_LVL = 3;
}

namespace PriestTrades {
    // Profession 2: Purple robe (Cleric)
    static constexpr TradeRecipeEntry RECIPES[] = {
        // SELL only
        {TradeItemIds::ENDER_EYE,      false, 0.3f},
        {TradeItemIds::XP_BOTTLE,      false, 0.2f},
        {TradeItemIds::REDSTONE,       false, 0.4f},
        {TradeItemIds::GLOWSTONE_BLOCK,false, 0.3f},
    };
    static constexpr int32_t RECIPE_COUNT = 4;
    // Special: enchanted equipment, prob 0.05 each
    // Java: 8 items (iron/diamond sword, chestplate, axe, pickaxe)
    //   price = 2 + rand(3) emeralds
    //   output: item with random enchantment level 5 + rand(15)
    static constexpr float ENCH_EQUIPMENT_PROB = 0.05f;
    static constexpr int32_t ENCH_EQUIP_BASE_PRICE = 2;
    static constexpr int32_t ENCH_EQUIP_PRICE_RAND = 3;
    static constexpr int32_t ENCH_EQUIP_MIN_LEVEL = 5;
    static constexpr int32_t ENCH_EQUIP_LEVEL_RAND = 15;
    // Items that can be enchanted:
    static constexpr int32_t ENCHANTABLE_ITEMS[] = {
        TradeItemIds::IRON_SWORD,
        TradeItemIds::DIAMOND_SWORD,
        TradeItemIds::IRON_CHESTPLATE,
        TradeItemIds::DIAMOND_CHESTPLATE,
        TradeItemIds::IRON_AXE,
        TradeItemIds::DIAMOND_AXE,
        TradeItemIds::IRON_PICKAXE,
        TradeItemIds::DIAMOND_PICKAXE,
    };
    static constexpr int32_t NUM_ENCHANTABLE = 8;
}

namespace BlacksmithTrades {
    // Profession 3: Black apron
    static constexpr TradeRecipeEntry RECIPES[] = {
        // BUY
        {TradeItemIds::COAL,            true,  0.7f},
        {TradeItemIds::IRON_INGOT,      true,  0.5f},
        {TradeItemIds::GOLD_INGOT,      true,  0.5f},
        {TradeItemIds::DIAMOND,         true,  0.5f},
        // SELL
        {TradeItemIds::IRON_SWORD,      false, 0.5f},
        {TradeItemIds::DIAMOND_SWORD,   false, 0.5f},
        {TradeItemIds::IRON_AXE,        false, 0.3f},
        {TradeItemIds::DIAMOND_AXE,     false, 0.3f},
        {TradeItemIds::IRON_PICKAXE,    false, 0.5f},
        {TradeItemIds::DIAMOND_PICKAXE, false, 0.5f},
        {TradeItemIds::IRON_SHOVEL,     false, 0.2f},
        {TradeItemIds::DIAMOND_SHOVEL,  false, 0.2f},
        {TradeItemIds::IRON_HOE,        false, 0.2f},
        {TradeItemIds::DIAMOND_HOE,     false, 0.2f},
        {TradeItemIds::IRON_BOOTS,      false, 0.2f},
        {TradeItemIds::DIAMOND_BOOTS,   false, 0.2f},
        {TradeItemIds::IRON_HELMET,     false, 0.2f},
        {TradeItemIds::DIAMOND_HELMET,  false, 0.2f},
        {TradeItemIds::IRON_CHESTPLATE, false, 0.2f},
        {TradeItemIds::DIAMOND_CHESTPLATE, false, 0.2f},
        {TradeItemIds::IRON_LEGGINGS,   false, 0.2f},
        {TradeItemIds::DIAMOND_LEGGINGS,false, 0.2f},
        {TradeItemIds::CHAINMAIL_BOOTS, false, 0.1f},
        {TradeItemIds::CHAINMAIL_HELMET,false, 0.1f},
        {TradeItemIds::CHAINMAIL_CHESTPLATE, false, 0.1f},
        {TradeItemIds::CHAINMAIL_LEGGINGS,   false, 0.1f},
    };
    static constexpr int32_t RECIPE_COUNT = 26;
}

namespace ButcherTrades {
    // Profession 4: White apron
    static constexpr TradeRecipeEntry RECIPES[] = {
        // BUY
        {TradeItemIds::COAL,            true,  0.7f},
        {TradeItemIds::PORKCHOP,        true,  0.5f},
        {TradeItemIds::BEEF,            true,  0.5f},
        // SELL
        {TradeItemIds::SADDLE,          false, 0.1f},
        {TradeItemIds::LEATHER_CHESTPLATE, false, 0.3f},
        {TradeItemIds::LEATHER_BOOTS,   false, 0.3f},
        {TradeItemIds::LEATHER_HELMET,  false, 0.3f},
        {TradeItemIds::LEATHER_LEGGINGS,false, 0.3f},
        {TradeItemIds::COOKED_PORKCHOP, false, 0.3f},
        {TradeItemIds::COOKED_BEEF,     false, 0.3f},
    };
    static constexpr int32_t RECIPE_COUNT = 10;
}

// ═══════════════════════════════════════════════════════════════════════════
// Trade Generation Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TradeGenConstants {
    // ─── Probability adjustment ───
    // Java: field_82191_bN = sqrt(existingRecipeCount) * 0.2f
    static constexpr float PROB_SCALING = 0.2f;
    // Probability cap
    static constexpr float MAX_PROBABILITY = 0.9f;

    // ─── Fallback trade ───
    // Java: if no recipes generated, add gold_ingot buy at prob 1.0
    static constexpr int32_t FALLBACK_ITEM = TradeItemIds::GOLD_INGOT;
    static constexpr float FALLBACK_PROB = 1.0f;

    // ─── Trade refresh ───
    // Java: timeUntilReset = 40 (2 seconds)
    static constexpr int32_t REFRESH_COOLDOWN = 40;
    // Java: disabled recipe restoration: rand(6) + rand(6) + 2 uses
    static constexpr int32_t RESTORE_USES_MIN = 2;
    static constexpr int32_t RESTORE_USES_RAND1 = 6;
    static constexpr int32_t RESTORE_USES_RAND2 = 6;

    // ─── Regeneration on trade refresh ───
    // Java: Regeneration I for 200 ticks (10 seconds)
    static constexpr int32_t REGEN_POTION_ID = 10;  // Potion.regeneration
    static constexpr int32_t REGEN_DURATION = 200;
    static constexpr int32_t REGEN_AMPLIFIER = 0;

    // ─── AI tick ───
    // Java: randomTickDivider = 70 + rand(50) = 70-119 ticks
    static constexpr int32_t AI_TICK_MIN = 70;
    static constexpr int32_t AI_TICK_RAND = 50;

    // ─── Village search radius ───
    static constexpr int32_t VILLAGE_SEARCH_RADIUS = 32;
    // Home area radius = villageRadius * 0.6
    static constexpr float HOME_RADIUS_SCALE = 0.6f;

    // ─── Reputation changes ───
    static constexpr int32_t REP_TRADE_SUCCESS = 1;
    static constexpr int32_t REP_ATTACK_ADULT = -1;
    static constexpr int32_t REP_ATTACK_CHILD = -3;
    static constexpr int32_t REP_KILL = -2;
    static constexpr int32_t REP_NEW_ARRIVAL = 5;
}

// ═══════════════════════════════════════════════════════════════════════════
// Villager Entity Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace VillagerEntityConstants {
    // Java: setSize(0.6f, 1.8f)
    static constexpr float WIDTH = 0.6f;
    static constexpr float HEIGHT = 1.8f;

    // Java: movementSpeed = 0.5
    static constexpr double MOVEMENT_SPEED = 0.5;

    // Java: maxHealth = 20
    static constexpr double MAX_HEALTH = 20.0;

    // Java: canDespawn = false (villagers never despawn)
    static constexpr bool CAN_DESPAWN = false;

    // Java: allowLeashing = false
    static constexpr bool ALLOW_LEASHING = false;

    // ─── Sounds ───
    static constexpr const char* SOUND_IDLE = "mob.villager.idle";
    static constexpr const char* SOUND_HAGGLE = "mob.villager.haggle";
    static constexpr const char* SOUND_HIT = "mob.villager.hit";
    static constexpr const char* SOUND_DEATH = "mob.villager.death";
    static constexpr const char* SOUND_YES = "mob.villager.yes";
    static constexpr const char* SOUND_NO = "mob.villager.no";

    // ─── Entity status bytes ───
    static constexpr int8_t STATUS_ANGRY = 13;      // attacked
    static constexpr int8_t STATUS_HAPPY = 14;      // trade completed

    // ─── NBT tags ───
    static constexpr const char* TAG_PROFESSION = "Profession";
    static constexpr const char* TAG_RICHES = "Riches";
    static constexpr const char* TAG_OFFERS = "Offers";

    // ─── AI task priorities ───
    // 0: Swim, 1: Avoid zombie (8 blocks, 0.6 speed) + Trade + LookAtTrader
    // 2: MoveIndoors, 3: RestrictOpenDoor, 4: OpenDoor(true)
    // 5: MoveTowardsRestriction(0.6), 6: VillagerMate
    // 7: FollowGolem, 8: Play(0.32)
    // 9: WatchClosest2(player 3f/1f, villager 5f/0.02f)
    // 9: Wander(0.6), 10: WatchClosest(EntityLiving 8f)
    static constexpr float AVOID_ZOMBIE_RANGE = 8.0f;
    static constexpr double AVOID_SPEED = 0.6;
    static constexpr double MOVE_SPEED = 0.6;
    static constexpr double PLAY_SPEED = 0.32;
}

} // namespace mccpp
