#pragma once
// Item registry — simplified port of adb.java (Item class)
// In 1.7.10, items are identified by numeric IDs (0-~400).
// Block items share IDs 0-255 with their block counterparts.

#include <cstdint>
#include <string>
#include <unordered_map>

namespace mc {

// Item IDs — matches vanilla 1.7.10 exactly
// Block items (0-255) share IDs with blocks
namespace ItemID {
    // Block items (same as BlockID)
    constexpr int16_t AIR           = 0;
    constexpr int16_t STONE         = 1;
    constexpr int16_t GRASS         = 2;
    constexpr int16_t DIRT          = 3;
    constexpr int16_t COBBLESTONE   = 4;
    constexpr int16_t PLANKS        = 5;
    constexpr int16_t SAPLING       = 6;
    constexpr int16_t BEDROCK       = 7;
    constexpr int16_t SAND          = 12;
    constexpr int16_t GRAVEL        = 13;
    constexpr int16_t LOG           = 17;
    constexpr int16_t LEAVES        = 18;
    constexpr int16_t GLASS         = 20;
    constexpr int16_t WOOL          = 35;
    constexpr int16_t GOLD_BLOCK    = 41;
    constexpr int16_t IRON_BLOCK    = 42;
    constexpr int16_t STONE_SLAB    = 44;
    constexpr int16_t BRICK         = 45;
    constexpr int16_t BOOKSHELF     = 47;
    constexpr int16_t OBSIDIAN      = 49;
    constexpr int16_t TORCH         = 50;
    constexpr int16_t OAK_STAIRS    = 53;
    constexpr int16_t CHEST         = 54;
    constexpr int16_t CRAFTING_TABLE = 58;
    constexpr int16_t FURNACE       = 61;
    constexpr int16_t LADDER        = 65;
    constexpr int16_t GLOWSTONE     = 89;
    constexpr int16_t FENCE         = 85;

    // Non-block items (256+)
    constexpr int16_t IRON_SHOVEL   = 256;
    constexpr int16_t IRON_PICKAXE  = 257;
    constexpr int16_t IRON_AXE      = 258;
    constexpr int16_t FLINT_STEEL   = 259;
    constexpr int16_t APPLE         = 260;
    constexpr int16_t BOW           = 261;
    constexpr int16_t ARROW         = 262;
    constexpr int16_t COAL          = 263;
    constexpr int16_t DIAMOND       = 264;
    constexpr int16_t IRON_INGOT    = 265;
    constexpr int16_t GOLD_INGOT    = 266;
    constexpr int16_t IRON_SWORD    = 267;
    constexpr int16_t WOODEN_SWORD  = 268;
    constexpr int16_t WOODEN_SHOVEL = 269;
    constexpr int16_t WOODEN_PICKAXE = 270;
    constexpr int16_t WOODEN_AXE    = 271;
    constexpr int16_t STONE_SWORD   = 272;
    constexpr int16_t STONE_SHOVEL  = 273;
    constexpr int16_t STONE_PICKAXE = 274;
    constexpr int16_t STONE_AXE     = 275;
    constexpr int16_t DIAMOND_SWORD = 276;
    constexpr int16_t DIAMOND_SHOVEL = 277;
    constexpr int16_t DIAMOND_PICKAXE = 278;
    constexpr int16_t DIAMOND_AXE   = 279;
    constexpr int16_t STICK         = 280;
    constexpr int16_t BOWL          = 281;
    constexpr int16_t STRING        = 287;
    constexpr int16_t FEATHER       = 288;
    constexpr int16_t GUNPOWDER     = 289;
    constexpr int16_t WHEAT_SEEDS   = 295;
    constexpr int16_t WHEAT         = 296;
    constexpr int16_t BREAD         = 297;
    constexpr int16_t LEATHER_HELMET = 298;
    constexpr int16_t LEATHER_CHESTPLATE = 299;
    constexpr int16_t LEATHER_LEGGINGS = 300;
    constexpr int16_t LEATHER_BOOTS = 301;
    constexpr int16_t CHAIN_HELMET  = 302;
    constexpr int16_t CHAIN_CHESTPLATE = 303;
    constexpr int16_t CHAIN_LEGGINGS = 304;
    constexpr int16_t CHAIN_BOOTS   = 305;
    constexpr int16_t IRON_HELMET   = 306;
    constexpr int16_t IRON_CHESTPLATE = 307;
    constexpr int16_t IRON_LEGGINGS = 308;
    constexpr int16_t IRON_BOOTS    = 309;
    constexpr int16_t DIAMOND_HELMET = 310;
    constexpr int16_t DIAMOND_CHESTPLATE = 311;
    constexpr int16_t DIAMOND_LEGGINGS = 312;
    constexpr int16_t DIAMOND_BOOTS = 313;
    constexpr int16_t GOLD_HELMET   = 314;
    constexpr int16_t GOLD_CHESTPLATE = 315;
    constexpr int16_t GOLD_LEGGINGS = 316;
    constexpr int16_t GOLD_BOOTS    = 317;
    constexpr int16_t FLINT         = 318;
    constexpr int16_t PORK_RAW      = 319;
    constexpr int16_t PORK_COOKED   = 320;
    constexpr int16_t GOLDEN_APPLE  = 322;
    constexpr int16_t BUCKET        = 325;
    constexpr int16_t WATER_BUCKET  = 326;
    constexpr int16_t LAVA_BUCKET   = 327;
    constexpr int16_t COMPASS       = 345;
    constexpr int16_t CLOCK         = 347;
    constexpr int16_t BONE          = 352;
    constexpr int16_t ENDER_PEARL   = 368;
    constexpr int16_t BLAZE_ROD     = 369;
    constexpr int16_t BEEF_RAW      = 363;
    constexpr int16_t BEEF_COOKED   = 364;
    constexpr int16_t ENDER_EYE     = 381;
    constexpr int16_t EXPERIENCE_BOTTLE = 384;
    constexpr int16_t EMERALD       = 388;
    constexpr int16_t BOOK_QUILL    = 386;
    constexpr int16_t WRITTEN_BOOK  = 387;
    constexpr int16_t NETHER_STAR   = 399;
    constexpr int16_t NAME_TAG      = 421;
}

// Max stack size for common items (default 64)
inline int maxStackSize(int16_t itemId) {
    // Tools, weapons, armor: stack to 1
    if ((itemId >= 256 && itemId <= 279) || // tools
        (itemId >= 298 && itemId <= 317) || // armor
        itemId == ItemID::BOW ||
        itemId == ItemID::FLINT_STEEL ||
        itemId == ItemID::BUCKET ||
        itemId == ItemID::WATER_BUCKET ||
        itemId == ItemID::LAVA_BUCKET) {
        return 1;
    }
    // Ender pearls, eggs: stack to 16
    if (itemId == ItemID::ENDER_PEARL ||
        itemId == ItemID::ENDER_EYE) {
        return 16;
    }
    return 64;
}

} // namespace mc
