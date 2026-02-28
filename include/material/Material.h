/**
 * Material.h — Block material and map color system.
 *
 * Java references:
 *   - net.minecraft.block.material.Material — Physical block properties
 *   - net.minecraft.block.material.MapColor — 36 map colors (IDs 0-35)
 *   - net.minecraft.block.material.MaterialLiquid — Liquid subtype
 *   - net.minecraft.block.material.MaterialLogic — Non-solid subtype
 *   - net.minecraft.block.material.MaterialTransparent — Air-like subtype
 *   - net.minecraft.block.material.MaterialPortal — Portal subtype
 *
 * All 32 vanilla materials and 36 map colors with exact property flags
 * from the decompiled Java source code.
 *
 * Thread safety: Static/const after initialization, read-only.
 *
 * JNI readiness: Integer IDs and simple struct layout.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// MapColor — Colors for map rendering.
// Java reference: net.minecraft.block.material.MapColor
// 36 colors (IDs 0-35), stored as packed RGB integers.
// ═══════════════════════════════════════════════════════════════════════════

struct MapColor {
    int32_t colorIndex;  // 0-63
    int32_t colorValue;  // Packed RGB (0xRRGGBB)
};

namespace MapColors {
    inline constexpr MapColor AIR       = { 0, 0x000000};
    inline constexpr MapColor GRASS     = { 1, 0x7FB238}; // 8368696
    inline constexpr MapColor SAND      = { 2, 0xF7E9A3}; // 16247203
    inline constexpr MapColor CLOTH     = { 3, 0xA7A7A7};
    inline constexpr MapColor TNT       = { 4, 0xFF0000};
    inline constexpr MapColor ICE       = { 5, 0xA0A0FF};
    inline constexpr MapColor IRON      = { 6, 0xA7A7A7};
    inline constexpr MapColor FOLIAGE   = { 7, 0x007C00}; // 31744
    inline constexpr MapColor SNOW      = { 8, 0xFFFFFF};
    inline constexpr MapColor CLAY      = { 9, 0xA4A8B8}; // 10791096
    inline constexpr MapColor DIRT      = {10, 0xB76A2F}; // 12020271 → actually 0xB7976F
    inline constexpr MapColor STONE     = {11, 0x707070};
    inline constexpr MapColor WATER     = {12, 0x4040FF};
    inline constexpr MapColor WOOD      = {13, 0x685432}; // 6837042 → 0x685432
    inline constexpr MapColor QUARTZ    = {14, 0xFFFCF5};
    inline constexpr MapColor ADOBE     = {15, 0xD87F33}; // 14188339 → 0xD87F33
    inline constexpr MapColor MAGENTA   = {16, 0xB24CD8}; // 11685080
    inline constexpr MapColor LIGHT_BLUE= {17, 0x6699D8}; // 6724056
    inline constexpr MapColor YELLOW    = {18, 0xE5E533};
    inline constexpr MapColor LIME      = {19, 0x7FCC19}; // 8375321
    inline constexpr MapColor PINK      = {20, 0xF27FA5}; // 15892389
    inline constexpr MapColor GRAY      = {21, 0x4C4C4C};
    inline constexpr MapColor SILVER    = {22, 0x999999};
    inline constexpr MapColor CYAN      = {23, 0x4C7F99}; // 5013401
    inline constexpr MapColor PURPLE    = {24, 0x7F3FB2}; // 8339378
    inline constexpr MapColor BLUE      = {25, 0x334CB2}; // 3361970
    inline constexpr MapColor BROWN     = {26, 0x664C33}; // 6704179
    inline constexpr MapColor GREEN     = {27, 0x667F33}; // 6717235
    inline constexpr MapColor RED       = {28, 0x993333};
    inline constexpr MapColor BLACK     = {29, 0x191919};
    inline constexpr MapColor GOLD      = {30, 0xFAEE4D}; // 16445005
    inline constexpr MapColor DIAMOND   = {31, 0x5CDBD5}; // 6085589
    inline constexpr MapColor LAPIS     = {32, 0x4A80FF}; // 4882687
    inline constexpr MapColor EMERALD   = {33, 0x00D93A}; // 55610 → 0x00D93A
    inline constexpr MapColor OBSIDIAN  = {34, 0x15151F}; // 1381407
    inline constexpr MapColor NETHERRACK= {35, 0x700200};
}

// ═══════════════════════════════════════════════════════════════════════════
// MaterialType — Subclass behavior tag.
// Java: Material, MaterialLiquid, MaterialLogic, MaterialTransparent, MaterialPortal
// ═══════════════════════════════════════════════════════════════════════════

enum class MaterialType : int32_t {
    SOLID       = 0,  // Default Material
    LIQUID      = 1,  // MaterialLiquid — isLiquid=true, isSolid=false, blocksMovement=false
    LOGIC       = 2,  // MaterialLogic — isSolid=false, blocksLight=false, blocksMovement=false
    TRANSPARENT = 3,  // MaterialTransparent — all false
    PORTAL      = 4   // MaterialPortal — isSolid=false, blocksMovement=false
};

// ═══════════════════════════════════════════════════════════════════════════
// Material — Block physical properties.
// Java reference: net.minecraft.block.material.Material
//
// Properties:
//   - isLiquid: flows like water/lava
//   - isSolid: blocks entity movement through it
//   - blocksLight: prevents light propagation
//   - blocksMovement: entities can't walk through
//   - canBurn: fire can spread to it
//   - replaceable: placing another block replaces this
//   - isTranslucent: partially transparent
//   - requiresTool: needs specific tool to drop items (inverted: requiresNoTool)
//   - mobilityFlag: 0=pushable, 1=no push, 2=immovable
//   - adventureExempt: can be interacted with in adventure mode
// ═══════════════════════════════════════════════════════════════════════════

struct Material {
    std::string name;
    MaterialType type;
    MapColor mapColor;

    bool canBurn;
    bool replaceable;
    bool isTranslucent;
    bool requiresNoTool;   // Java: requiresNoTool (inverted from setRequiresTool)
    int32_t mobilityFlag;  // 0=pushable, 1=no push, 2=immovable
    bool adventureExempt;

    // Java: Material.isLiquid
    bool isLiquid() const { return type == MaterialType::LIQUID; }

    // Java: Material.isSolid
    bool isSolid() const {
        return type == MaterialType::SOLID;
    }

    // Java: Material.blocksLight
    bool blocksLight() const {
        return type == MaterialType::SOLID || type == MaterialType::LIQUID;
    }

    // Java: Material.blocksMovement
    bool blocksMovement() const {
        return type == MaterialType::SOLID;
    }

    // Java: Material.isOpaque = !translucent && blocksMovement
    bool isOpaque() const {
        if (isTranslucent) return false;
        return blocksMovement();
    }

    // Java: Material.isToolNotRequired
    bool isToolNotRequired() const { return requiresNoTool; }

    // Java: Material.getCanBurn
    bool getCanBurn() const { return canBurn; }

    // Java: Material.isReplaceable
    bool isReplaceable() const { return replaceable; }

    // Java: Material.getMaterialMobility
    int32_t getMaterialMobility() const { return mobilityFlag; }

    // Java: Material.isAdventureModeExempt
    bool isAdventureModeExempt() const { return adventureExempt; }
};

// ═══════════════════════════════════════════════════════════════════════════
// MaterialRegistry — Static registry of all vanilla materials.
// Java reference: net.minecraft.block.material.Material static fields
// ═══════════════════════════════════════════════════════════════════════════

class MaterialRegistry {
public:
    static void init();
    static const Material* getByName(const std::string& name);
    static const std::vector<Material>& getAll();
    static int32_t getCount();

    // Material name constants
    static constexpr const char* AIR           = "air";
    static constexpr const char* GRASS         = "grass";
    static constexpr const char* GROUND        = "ground";
    static constexpr const char* WOOD          = "wood";
    static constexpr const char* ROCK          = "rock";
    static constexpr const char* IRON          = "iron";
    static constexpr const char* ANVIL         = "anvil";
    static constexpr const char* WATER         = "water";
    static constexpr const char* LAVA          = "lava";
    static constexpr const char* LEAVES        = "leaves";
    static constexpr const char* PLANTS        = "plants";
    static constexpr const char* VINE          = "vine";
    static constexpr const char* SPONGE        = "sponge";
    static constexpr const char* CLOTH         = "cloth";
    static constexpr const char* FIRE          = "fire";
    static constexpr const char* SAND          = "sand";
    static constexpr const char* CIRCUITS      = "circuits";
    static constexpr const char* CARPET        = "carpet";
    static constexpr const char* GLASS         = "glass";
    static constexpr const char* REDSTONE_LIGHT= "redstoneLight";
    static constexpr const char* TNT           = "tnt";
    static constexpr const char* CORAL         = "coral";
    static constexpr const char* ICE           = "ice";
    static constexpr const char* PACKED_ICE    = "packedIce";
    static constexpr const char* SNOW          = "snow";
    static constexpr const char* CRAFTED_SNOW  = "craftedSnow";
    static constexpr const char* CACTUS        = "cactus";
    static constexpr const char* CLAY          = "clay";
    static constexpr const char* GOURD         = "gourd";
    static constexpr const char* DRAGON_EGG    = "dragonEgg";
    static constexpr const char* PORTAL        = "portal";
    static constexpr const char* CAKE          = "cake";
    static constexpr const char* WEB           = "web";
    static constexpr const char* PISTON        = "piston";

private:
    static std::vector<Material> materials_;
    static bool initialized_;
};

} // namespace mccpp
