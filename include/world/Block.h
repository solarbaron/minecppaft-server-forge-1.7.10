#pragma once
// Block registry — simplified port of pp.java (Block class)
// In Minecraft 1.7.10, block IDs are 0-4095 (12-bit), stored in
// the static array pp.C (ph[] C = new ph[4096]).
//
// For the initial implementation we only need block IDs and names
// for the most common blocks used in flat world generation.

#include <cstdint>
#include <string>
#include <array>

namespace mc {

// Block IDs — matches MCP/vanilla 1.7.10 IDs exactly
// From pp.java static initializer and adb.java (Item→Block mapping)
namespace BlockID {
    constexpr uint16_t AIR        = 0;
    constexpr uint16_t STONE      = 1;
    constexpr uint16_t GRASS      = 2;
    constexpr uint16_t DIRT       = 3;
    constexpr uint16_t COBBLE     = 4;
    constexpr uint16_t PLANKS     = 5;
    constexpr uint16_t SAPLING    = 6;
    constexpr uint16_t BEDROCK    = 7;
    constexpr uint16_t WATER_FLOW = 8;
    constexpr uint16_t WATER      = 9;
    constexpr uint16_t LAVA_FLOW  = 10;
    constexpr uint16_t LAVA       = 11;
    constexpr uint16_t SAND       = 12;
    constexpr uint16_t GRAVEL     = 13;
    constexpr uint16_t GOLD_ORE   = 14;
    constexpr uint16_t IRON_ORE   = 15;
    constexpr uint16_t COAL_ORE   = 16;
    constexpr uint16_t LOG        = 17;
    constexpr uint16_t LEAVES     = 18;
    constexpr uint16_t GLASS      = 20;
    constexpr uint16_t SANDSTONE  = 24;
    constexpr uint16_t WOOL       = 35;
    constexpr uint16_t GOLD_BLOCK = 41;
    constexpr uint16_t IRON_BLOCK = 42;
    constexpr uint16_t BRICK      = 45;
    constexpr uint16_t BOOKSHELF  = 47;
    constexpr uint16_t OBSIDIAN   = 49;
    constexpr uint16_t TORCH      = 50;
    constexpr uint16_t OAK_STAIRS = 53;
    constexpr uint16_t CHEST      = 54;
    constexpr uint16_t DIAMOND_ORE = 56;
    constexpr uint16_t DIAMOND_BLK = 57;
    constexpr uint16_t CRAFTING   = 58;
    constexpr uint16_t FURNACE    = 61;
    constexpr uint16_t LADDER     = 65;
    constexpr uint16_t COBBLE_STAIR= 67;
    constexpr uint16_t SNOW_LAYER = 78;
    constexpr uint16_t ICE        = 79;
    constexpr uint16_t CLAY       = 82;
    constexpr uint16_t NETHERRACK = 87;
    constexpr uint16_t GLOWSTONE  = 89;
    constexpr uint16_t STONE_BRICK = 98;
    constexpr uint16_t ENDER_STONE = 121;
    constexpr uint16_t EMERALD_ORE = 129;
    constexpr uint16_t EMERALD_BLK = 133;
    constexpr uint16_t QUARTZ_BLOCK = 155;
    constexpr uint16_t HAY_BLOCK  = 170;
    constexpr uint16_t HARDENED_CLAY = 172;
}

// Block light opacity — used for lighting calculations
// 0 = fully transparent, 255 = fully opaque
// For now, a simple: air=0, everything else=255
inline uint8_t getBlockOpacity(uint16_t blockId) {
    if (blockId == BlockID::AIR) return 0;
    if (blockId == BlockID::GLASS) return 0;
    if (blockId == BlockID::TORCH) return 0;
    if (blockId == BlockID::SAPLING) return 0;
    if (blockId == BlockID::LEAVES) return 1;
    if (blockId == BlockID::ICE) return 3;
    if (blockId == BlockID::WATER || blockId == BlockID::WATER_FLOW) return 3;
    return 255; // Solid
}

// Block light emission
inline uint8_t getBlockLightEmission(uint16_t blockId) {
    if (blockId == BlockID::TORCH) return 14;
    if (blockId == BlockID::GLOWSTONE) return 15;
    if (blockId == BlockID::LAVA || blockId == BlockID::LAVA_FLOW) return 15;
    if (blockId == BlockID::FURNACE) return 13;
    return 0;
}

} // namespace mc
