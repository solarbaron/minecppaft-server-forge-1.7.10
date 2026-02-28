/**
 * Item.cpp — Item implementation and registry initialization.
 *
 * Java reference: net.minecraft.item.Item
 * All vanilla 1.7.10 items registered with exact IDs and properties.
 *
 * Item IDs 0-255 are reserved for ItemBlocks (auto-registered from blocks).
 * Standalone items start at ID 256.
 */

#include "item/Item.h"

#include <iostream>

namespace mccpp {

// ─── Static registry instance ───────────────────────────────────────────────
RegistryNamespaced<Item*> Item::itemRegistry;

// ─── Item storage — static array owning all item instances ──────────────────
// Max registered ID is 2267 (record_wait), so 2300 is safe.
static Item itemStorage[2300];
static int itemStorageCount = 0;

static Item& alloc() {
    return itemStorage[itemStorageCount++];
}

// ─── Constructor ────────────────────────────────────────────────────────────

Item::Item() = default;

// ─── Builder methods ────────────────────────────────────────────────────────

Item& Item::setMaxStackSize(int n) { maxStackSize_ = n; return *this; }
Item& Item::setMaxDurability(int n) { maxDurability_ = n; maxStackSize_ = 1; return *this; }
Item& Item::setUnlocalizedName(const std::string& name) { unlocalizedName_ = name; return *this; }
Item& Item::setTextureName(const std::string& name) { textureName_ = name; return *this; }
Item& Item::setFull3D() { full3D_ = true; return *this; }
Item& Item::setHasSubtypes(bool v) { hasSubtypes_ = v; return *this; }
Item& Item::setContainerItem(Item* item) { containerItem_ = item; return *this; }
Item& Item::setCategory(ItemCategory cat) { category_ = cat; return *this; }
Item& Item::setToolMaterial(ToolMaterial mat) { toolMaterial_ = mat; return *this; }
Item& Item::setArmorMaterial(ArmorMaterial mat) { armorMaterial_ = mat; return *this; }
Item& Item::setArmorSlot(int slot) { armorSlot_ = slot; return *this; }
Item& Item::setFoodValues(int heal, float sat, bool wolf) {
    foodHealAmount_ = heal;
    foodSaturation_ = sat;
    isWolfFood_ = wolf;
    category_ = ItemCategory::Food;
    return *this;
}

// ─── registerItems ──────────────────────────────────────────────────────────
// Java reference: Item.registerItems()

void Item::registerItems() {
    itemStorageCount = 0;

    // Helper macros
    #define IREG(id, name) \
        { Item& it = alloc(); it

    #define IEND(id, name) \
        ; itemRegistry.addObject(id, name, &it); }

    // --- Tools ---
    // Java: Item$ToolMaterial durabilities: WOOD=59, STONE=131, IRON=250, EMERALD=1561, GOLD=32

    IREG(256, "iron_shovel").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::IRON)
        .setMaxDurability(250).setUnlocalizedName("shovelIron").setTextureName("iron_shovel") IEND(256, "iron_shovel")
    IREG(257, "iron_pickaxe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::IRON)
        .setMaxDurability(250).setUnlocalizedName("pickaxeIron").setTextureName("iron_pickaxe") IEND(257, "iron_pickaxe")
    IREG(258, "iron_axe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::IRON)
        .setMaxDurability(250).setUnlocalizedName("hatchetIron").setTextureName("iron_axe") IEND(258, "iron_axe")
    IREG(259, "flint_and_steel").setMaxDurability(64)
        .setUnlocalizedName("flintAndSteel").setTextureName("flint_and_steel") IEND(259, "flint_and_steel")
    IREG(260, "apple").setFoodValues(4, 0.3f, false)
        .setUnlocalizedName("apple").setTextureName("apple") IEND(260, "apple")
    IREG(261, "bow").setCategory(ItemCategory::Bow).setMaxDurability(384)
        .setUnlocalizedName("bow").setTextureName("bow") IEND(261, "bow")
    IREG(262, "arrow").setUnlocalizedName("arrow").setTextureName("arrow") IEND(262, "arrow")
    IREG(263, "coal").setHasSubtypes(true).setUnlocalizedName("coal").setTextureName("coal") IEND(263, "coal")
    IREG(264, "diamond").setUnlocalizedName("diamond").setTextureName("diamond") IEND(264, "diamond")
    IREG(265, "iron_ingot").setUnlocalizedName("ingotIron").setTextureName("iron_ingot") IEND(265, "iron_ingot")
    IREG(266, "gold_ingot").setUnlocalizedName("ingotGold").setTextureName("gold_ingot") IEND(266, "gold_ingot")

    // Swords
    IREG(267, "iron_sword").setCategory(ItemCategory::Sword).setToolMaterial(ToolMaterial::IRON)
        .setMaxDurability(250).setUnlocalizedName("swordIron").setTextureName("iron_sword") IEND(267, "iron_sword")
    IREG(268, "wooden_sword").setCategory(ItemCategory::Sword).setToolMaterial(ToolMaterial::WOOD)
        .setMaxDurability(59).setUnlocalizedName("swordWood").setTextureName("wood_sword") IEND(268, "wooden_sword")
    IREG(269, "wooden_shovel").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::WOOD)
        .setMaxDurability(59).setUnlocalizedName("shovelWood").setTextureName("wood_shovel") IEND(269, "wooden_shovel")
    IREG(270, "wooden_pickaxe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::WOOD)
        .setMaxDurability(59).setUnlocalizedName("pickaxeWood").setTextureName("wood_pickaxe") IEND(270, "wooden_pickaxe")
    IREG(271, "wooden_axe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::WOOD)
        .setMaxDurability(59).setUnlocalizedName("hatchetWood").setTextureName("wood_axe") IEND(271, "wooden_axe")
    IREG(272, "stone_sword").setCategory(ItemCategory::Sword).setToolMaterial(ToolMaterial::STONE)
        .setMaxDurability(131).setUnlocalizedName("swordStone").setTextureName("stone_sword") IEND(272, "stone_sword")
    IREG(273, "stone_shovel").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::STONE)
        .setMaxDurability(131).setUnlocalizedName("shovelStone").setTextureName("stone_shovel") IEND(273, "stone_shovel")
    IREG(274, "stone_pickaxe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::STONE)
        .setMaxDurability(131).setUnlocalizedName("pickaxeStone").setTextureName("stone_pickaxe") IEND(274, "stone_pickaxe")
    IREG(275, "stone_axe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::STONE)
        .setMaxDurability(131).setUnlocalizedName("hatchetStone").setTextureName("stone_axe") IEND(275, "stone_axe")
    IREG(276, "diamond_sword").setCategory(ItemCategory::Sword).setToolMaterial(ToolMaterial::EMERALD)
        .setMaxDurability(1561).setUnlocalizedName("swordDiamond").setTextureName("diamond_sword") IEND(276, "diamond_sword")
    IREG(277, "diamond_shovel").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::EMERALD)
        .setMaxDurability(1561).setUnlocalizedName("shovelDiamond").setTextureName("diamond_shovel") IEND(277, "diamond_shovel")
    IREG(278, "diamond_pickaxe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::EMERALD)
        .setMaxDurability(1561).setUnlocalizedName("pickaxeDiamond").setTextureName("diamond_pickaxe") IEND(278, "diamond_pickaxe")
    IREG(279, "diamond_axe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::EMERALD)
        .setMaxDurability(1561).setUnlocalizedName("hatchetDiamond").setTextureName("diamond_axe") IEND(279, "diamond_axe")

    IREG(280, "stick").setFull3D().setUnlocalizedName("stick").setTextureName("stick") IEND(280, "stick")
    IREG(281, "bowl").setUnlocalizedName("bowl").setTextureName("bowl") IEND(281, "bowl")
    IREG(282, "mushroom_stew").setFoodValues(6, 0.6f, false).setMaxStackSize(1)
        .setUnlocalizedName("mushroomStew").setTextureName("mushroom_stew") IEND(282, "mushroom_stew")

    // Gold tools
    IREG(283, "golden_sword").setCategory(ItemCategory::Sword).setToolMaterial(ToolMaterial::GOLD)
        .setMaxDurability(32).setUnlocalizedName("swordGold").setTextureName("gold_sword") IEND(283, "golden_sword")
    IREG(284, "golden_shovel").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::GOLD)
        .setMaxDurability(32).setUnlocalizedName("shovelGold").setTextureName("gold_shovel") IEND(284, "golden_shovel")
    IREG(285, "golden_pickaxe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::GOLD)
        .setMaxDurability(32).setUnlocalizedName("pickaxeGold").setTextureName("gold_pickaxe") IEND(285, "golden_pickaxe")
    IREG(286, "golden_axe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::GOLD)
        .setMaxDurability(32).setUnlocalizedName("hatchetGold").setTextureName("gold_axe") IEND(286, "golden_axe")

    IREG(287, "string").setUnlocalizedName("string").setTextureName("string") IEND(287, "string")
    IREG(288, "feather").setUnlocalizedName("feather").setTextureName("feather") IEND(288, "feather")
    IREG(289, "gunpowder").setUnlocalizedName("sulphur").setTextureName("gunpowder") IEND(289, "gunpowder")

    // Hoes
    IREG(290, "wooden_hoe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::WOOD)
        .setMaxDurability(59).setUnlocalizedName("hoeWood").setTextureName("wood_hoe") IEND(290, "wooden_hoe")
    IREG(291, "stone_hoe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::STONE)
        .setMaxDurability(131).setUnlocalizedName("hoeStone").setTextureName("stone_hoe") IEND(291, "stone_hoe")
    IREG(292, "iron_hoe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::IRON)
        .setMaxDurability(250).setUnlocalizedName("hoeIron").setTextureName("iron_hoe") IEND(292, "iron_hoe")
    IREG(293, "diamond_hoe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::EMERALD)
        .setMaxDurability(1561).setUnlocalizedName("hoeDiamond").setTextureName("diamond_hoe") IEND(293, "diamond_hoe")
    IREG(294, "golden_hoe").setCategory(ItemCategory::Tool).setToolMaterial(ToolMaterial::GOLD)
        .setMaxDurability(32).setUnlocalizedName("hoeGold").setTextureName("gold_hoe") IEND(294, "golden_hoe")

    IREG(295, "wheat_seeds").setUnlocalizedName("seeds").setTextureName("seeds_wheat") IEND(295, "wheat_seeds")
    IREG(296, "wheat").setUnlocalizedName("wheat").setTextureName("wheat") IEND(296, "wheat")
    IREG(297, "bread").setFoodValues(5, 0.6f, false)
        .setUnlocalizedName("bread").setTextureName("bread") IEND(297, "bread")

    // --- Armor ---
    // Java: ArmorMaterial durabilities: CLOTH=5*dmgFactor, CHAIN=15*dmgFactor, IRON=15*dmgFactor, DIAMOND=33*dmgFactor, GOLD=7*dmgFactor
    // dmgFactor[slot]: helmet=11, chest=16, legs=15, boots=13

    // Leather (CLOTH) - durability: 55/80/75/65
    IREG(298, "leather_helmet").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::CLOTH)
        .setArmorSlot(0).setMaxDurability(55).setUnlocalizedName("helmetCloth").setTextureName("leather_helmet") IEND(298, "leather_helmet")
    IREG(299, "leather_chestplate").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::CLOTH)
        .setArmorSlot(1).setMaxDurability(80).setUnlocalizedName("chestplateCloth").setTextureName("leather_chestplate") IEND(299, "leather_chestplate")
    IREG(300, "leather_leggings").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::CLOTH)
        .setArmorSlot(2).setMaxDurability(75).setUnlocalizedName("leggingsCloth").setTextureName("leather_leggings") IEND(300, "leather_leggings")
    IREG(301, "leather_boots").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::CLOTH)
        .setArmorSlot(3).setMaxDurability(65).setUnlocalizedName("bootsCloth").setTextureName("leather_boots") IEND(301, "leather_boots")

    // Chain - durability: 165/240/225/195
    IREG(302, "chainmail_helmet").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::CHAIN)
        .setArmorSlot(0).setMaxDurability(165).setUnlocalizedName("helmetChain").setTextureName("chainmail_helmet") IEND(302, "chainmail_helmet")
    IREG(303, "chainmail_chestplate").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::CHAIN)
        .setArmorSlot(1).setMaxDurability(240).setUnlocalizedName("chestplateChain").setTextureName("chainmail_chestplate") IEND(303, "chainmail_chestplate")
    IREG(304, "chainmail_leggings").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::CHAIN)
        .setArmorSlot(2).setMaxDurability(225).setUnlocalizedName("leggingsChain").setTextureName("chainmail_leggings") IEND(304, "chainmail_leggings")
    IREG(305, "chainmail_boots").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::CHAIN)
        .setArmorSlot(3).setMaxDurability(195).setUnlocalizedName("bootsChain").setTextureName("chainmail_boots") IEND(305, "chainmail_boots")

    // Iron - durability: 165/240/225/195
    IREG(306, "iron_helmet").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::IRON)
        .setArmorSlot(0).setMaxDurability(165).setUnlocalizedName("helmetIron").setTextureName("iron_helmet") IEND(306, "iron_helmet")
    IREG(307, "iron_chestplate").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::IRON)
        .setArmorSlot(1).setMaxDurability(240).setUnlocalizedName("chestplateIron").setTextureName("iron_chestplate") IEND(307, "iron_chestplate")
    IREG(308, "iron_leggings").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::IRON)
        .setArmorSlot(2).setMaxDurability(225).setUnlocalizedName("leggingsIron").setTextureName("iron_leggings") IEND(308, "iron_leggings")
    IREG(309, "iron_boots").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::IRON)
        .setArmorSlot(3).setMaxDurability(195).setUnlocalizedName("bootsIron").setTextureName("iron_boots") IEND(309, "iron_boots")

    // Diamond - durability: 363/528/495/429
    IREG(310, "diamond_helmet").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::DIAMOND)
        .setArmorSlot(0).setMaxDurability(363).setUnlocalizedName("helmetDiamond").setTextureName("diamond_helmet") IEND(310, "diamond_helmet")
    IREG(311, "diamond_chestplate").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::DIAMOND)
        .setArmorSlot(1).setMaxDurability(528).setUnlocalizedName("chestplateDiamond").setTextureName("diamond_chestplate") IEND(311, "diamond_chestplate")
    IREG(312, "diamond_leggings").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::DIAMOND)
        .setArmorSlot(2).setMaxDurability(495).setUnlocalizedName("leggingsDiamond").setTextureName("diamond_leggings") IEND(312, "diamond_leggings")
    IREG(313, "diamond_boots").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::DIAMOND)
        .setArmorSlot(3).setMaxDurability(429).setUnlocalizedName("bootsDiamond").setTextureName("diamond_boots") IEND(313, "diamond_boots")

    // Gold armor - durability: 77/112/105/91
    IREG(314, "golden_helmet").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::GOLD)
        .setArmorSlot(0).setMaxDurability(77).setUnlocalizedName("helmetGold").setTextureName("gold_helmet") IEND(314, "golden_helmet")
    IREG(315, "golden_chestplate").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::GOLD)
        .setArmorSlot(1).setMaxDurability(112).setUnlocalizedName("chestplateGold").setTextureName("gold_chestplate") IEND(315, "golden_chestplate")
    IREG(316, "golden_leggings").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::GOLD)
        .setArmorSlot(2).setMaxDurability(105).setUnlocalizedName("leggingsGold").setTextureName("gold_leggings") IEND(316, "golden_leggings")
    IREG(317, "golden_boots").setCategory(ItemCategory::Armor).setArmorMaterial(ArmorMaterial::GOLD)
        .setArmorSlot(3).setMaxDurability(91).setUnlocalizedName("bootsGold").setTextureName("gold_boots") IEND(317, "golden_boots")

    IREG(318, "flint").setUnlocalizedName("flint").setTextureName("flint") IEND(318, "flint")

    // Foods
    IREG(319, "porkchop").setFoodValues(3, 0.3f, true).setUnlocalizedName("porkchopRaw").setTextureName("porkchop_raw") IEND(319, "porkchop")
    IREG(320, "cooked_porkchop").setFoodValues(8, 0.8f, true).setUnlocalizedName("porkchopCooked").setTextureName("porkchop_cooked") IEND(320, "cooked_porkchop")
    IREG(321, "painting").setUnlocalizedName("painting").setTextureName("painting") IEND(321, "painting")
    IREG(322, "golden_apple").setFoodValues(4, 1.2f, false).setUnlocalizedName("appleGold").setTextureName("apple_golden") IEND(322, "golden_apple")
    IREG(323, "sign").setMaxStackSize(16).setUnlocalizedName("sign").setTextureName("sign") IEND(323, "sign")
    IREG(324, "wooden_door").setUnlocalizedName("doorWood").setTextureName("door_wood") IEND(324, "wooden_door")

    // Buckets
    Item* bucketEmpty = nullptr;
    IREG(325, "bucket").setMaxStackSize(16).setUnlocalizedName("bucket").setTextureName("bucket_empty") IEND(325, "bucket")
    bucketEmpty = &itemStorage[itemStorageCount - 1];
    IREG(326, "water_bucket").setMaxStackSize(1).setContainerItem(bucketEmpty)
        .setUnlocalizedName("bucketWater").setTextureName("bucket_water") IEND(326, "water_bucket")
    IREG(327, "lava_bucket").setMaxStackSize(1).setContainerItem(bucketEmpty)
        .setUnlocalizedName("bucketLava").setTextureName("bucket_lava") IEND(327, "lava_bucket")

    IREG(328, "minecart").setMaxStackSize(1).setUnlocalizedName("minecart").setTextureName("minecart_normal") IEND(328, "minecart")
    IREG(329, "saddle").setMaxStackSize(1).setUnlocalizedName("saddle").setTextureName("saddle") IEND(329, "saddle")
    IREG(330, "iron_door").setUnlocalizedName("doorIron").setTextureName("door_iron") IEND(330, "iron_door")
    IREG(331, "redstone").setUnlocalizedName("redstone").setTextureName("redstone_dust") IEND(331, "redstone")
    IREG(332, "snowball").setMaxStackSize(16).setUnlocalizedName("snowball").setTextureName("snowball") IEND(332, "snowball")
    IREG(333, "boat").setMaxStackSize(1).setUnlocalizedName("boat").setTextureName("boat") IEND(333, "boat")
    IREG(334, "leather").setUnlocalizedName("leather").setTextureName("leather") IEND(334, "leather")
    IREG(335, "milk_bucket").setMaxStackSize(1).setContainerItem(bucketEmpty)
        .setUnlocalizedName("milk").setTextureName("bucket_milk") IEND(335, "milk_bucket")
    IREG(336, "brick").setUnlocalizedName("brick").setTextureName("brick") IEND(336, "brick")
    IREG(337, "clay_ball").setUnlocalizedName("clay").setTextureName("clay_ball") IEND(337, "clay_ball")
    IREG(338, "reeds").setUnlocalizedName("reeds").setTextureName("reeds") IEND(338, "reeds")
    IREG(339, "paper").setUnlocalizedName("paper").setTextureName("paper") IEND(339, "paper")
    IREG(340, "book").setUnlocalizedName("book").setTextureName("book_normal") IEND(340, "book")
    IREG(341, "slime_ball").setUnlocalizedName("slimeball").setTextureName("slimeball") IEND(341, "slime_ball")
    IREG(342, "chest_minecart").setMaxStackSize(1).setUnlocalizedName("minecartChest").setTextureName("minecart_chest") IEND(342, "chest_minecart")
    IREG(343, "furnace_minecart").setMaxStackSize(1).setUnlocalizedName("minecartFurnace").setTextureName("minecart_furnace") IEND(343, "furnace_minecart")
    IREG(344, "egg").setMaxStackSize(16).setUnlocalizedName("egg").setTextureName("egg") IEND(344, "egg")
    IREG(345, "compass").setUnlocalizedName("compass").setTextureName("compass") IEND(345, "compass")
    IREG(346, "fishing_rod").setMaxDurability(64).setUnlocalizedName("fishingRod").setTextureName("fishing_rod") IEND(346, "fishing_rod")
    IREG(347, "clock").setUnlocalizedName("clock").setTextureName("clock") IEND(347, "clock")
    IREG(348, "glowstone_dust").setUnlocalizedName("yellowDust").setTextureName("glowstone_dust") IEND(348, "glowstone_dust")
    IREG(349, "fish").setHasSubtypes(true).setFoodValues(2, 0.3f, false)
        .setUnlocalizedName("fish").setTextureName("fish_raw") IEND(349, "fish")
    IREG(350, "cooked_fished").setHasSubtypes(true).setFoodValues(5, 0.6f, false)
        .setUnlocalizedName("fish").setTextureName("fish_cooked") IEND(350, "cooked_fished")
    IREG(351, "dye").setHasSubtypes(true).setUnlocalizedName("dyePowder").setTextureName("dye_powder") IEND(351, "dye")
    IREG(352, "bone").setFull3D().setUnlocalizedName("bone").setTextureName("bone") IEND(352, "bone")
    IREG(353, "sugar").setUnlocalizedName("sugar").setTextureName("sugar") IEND(353, "sugar")
    IREG(354, "cake").setMaxStackSize(1).setUnlocalizedName("cake").setTextureName("cake") IEND(354, "cake")
    IREG(355, "bed").setMaxStackSize(1).setUnlocalizedName("bed").setTextureName("bed") IEND(355, "bed")
    IREG(356, "repeater").setUnlocalizedName("diode").setTextureName("repeater") IEND(356, "repeater")
    IREG(357, "cookie").setFoodValues(2, 0.1f, false).setUnlocalizedName("cookie").setTextureName("cookie") IEND(357, "cookie")
    IREG(358, "filled_map").setUnlocalizedName("map").setTextureName("map_filled") IEND(358, "filled_map")
    IREG(359, "shears").setMaxDurability(238).setUnlocalizedName("shears").setTextureName("shears") IEND(359, "shears")
    IREG(360, "melon").setFoodValues(2, 0.3f, false).setUnlocalizedName("melon").setTextureName("melon") IEND(360, "melon")
    IREG(361, "pumpkin_seeds").setUnlocalizedName("seeds_pumpkin").setTextureName("seeds_pumpkin") IEND(361, "pumpkin_seeds")
    IREG(362, "melon_seeds").setUnlocalizedName("seeds_melon").setTextureName("seeds_melon") IEND(362, "melon_seeds")
    IREG(363, "beef").setFoodValues(3, 0.3f, true).setUnlocalizedName("beefRaw").setTextureName("beef_raw") IEND(363, "beef")
    IREG(364, "cooked_beef").setFoodValues(8, 0.8f, true).setUnlocalizedName("beefCooked").setTextureName("beef_cooked") IEND(364, "cooked_beef")
    IREG(365, "chicken").setFoodValues(2, 0.3f, true).setUnlocalizedName("chickenRaw").setTextureName("chicken_raw") IEND(365, "chicken")
    IREG(366, "cooked_chicken").setFoodValues(6, 0.6f, true).setUnlocalizedName("chickenCooked").setTextureName("chicken_cooked") IEND(366, "cooked_chicken")
    IREG(367, "rotten_flesh").setFoodValues(4, 0.1f, true).setUnlocalizedName("rottenFlesh").setTextureName("rotten_flesh") IEND(367, "rotten_flesh")
    IREG(368, "ender_pearl").setMaxStackSize(16).setUnlocalizedName("enderPearl").setTextureName("ender_pearl") IEND(368, "ender_pearl")
    IREG(369, "blaze_rod").setUnlocalizedName("blazeRod").setTextureName("blaze_rod") IEND(369, "blaze_rod")
    IREG(370, "ghast_tear").setUnlocalizedName("ghastTear").setTextureName("ghast_tear") IEND(370, "ghast_tear")
    IREG(371, "gold_nugget").setUnlocalizedName("goldNugget").setTextureName("gold_nugget") IEND(371, "gold_nugget")
    IREG(372, "nether_wart").setUnlocalizedName("netherStalkSeeds").setTextureName("nether_wart") IEND(372, "nether_wart")
    IREG(373, "potion").setCategory(ItemCategory::Potion).setMaxStackSize(1)
        .setHasSubtypes(true).setUnlocalizedName("potion").setTextureName("potion") IEND(373, "potion")
    IREG(374, "glass_bottle").setUnlocalizedName("glassBottle").setTextureName("potion_bottle_empty") IEND(374, "glass_bottle")
    IREG(375, "spider_eye").setFoodValues(2, 0.8f, false)
        .setUnlocalizedName("spiderEye").setTextureName("spider_eye") IEND(375, "spider_eye")
    IREG(376, "fermented_spider_eye").setUnlocalizedName("fermentedSpiderEye").setTextureName("spider_eye_fermented") IEND(376, "fermented_spider_eye")
    IREG(377, "blaze_powder").setUnlocalizedName("blazePowder").setTextureName("blaze_powder") IEND(377, "blaze_powder")
    IREG(378, "magma_cream").setUnlocalizedName("magmaCream").setTextureName("magma_cream") IEND(378, "magma_cream")
    IREG(379, "brewing_stand").setUnlocalizedName("brewingStand").setTextureName("brewing_stand") IEND(379, "brewing_stand")
    IREG(380, "cauldron").setUnlocalizedName("cauldron").setTextureName("cauldron") IEND(380, "cauldron")
    IREG(381, "ender_eye").setUnlocalizedName("eyeOfEnder").setTextureName("ender_eye") IEND(381, "ender_eye")
    IREG(382, "speckled_melon").setUnlocalizedName("speckledMelon").setTextureName("melon_speckled") IEND(382, "speckled_melon")
    IREG(383, "spawn_egg").setHasSubtypes(true).setUnlocalizedName("monsterPlacer").setTextureName("spawn_egg") IEND(383, "spawn_egg")
    IREG(384, "experience_bottle").setUnlocalizedName("expBottle").setTextureName("experience_bottle") IEND(384, "experience_bottle")
    IREG(385, "fire_charge").setUnlocalizedName("fireball").setTextureName("fireball") IEND(385, "fire_charge")
    IREG(386, "writable_book").setMaxStackSize(1).setUnlocalizedName("writingBook").setTextureName("book_writable") IEND(386, "writable_book")
    IREG(387, "written_book").setMaxStackSize(16).setUnlocalizedName("writtenBook").setTextureName("book_written") IEND(387, "written_book")
    IREG(388, "emerald").setUnlocalizedName("emerald").setTextureName("emerald") IEND(388, "emerald")
    IREG(389, "item_frame").setUnlocalizedName("frame").setTextureName("item_frame") IEND(389, "item_frame")
    IREG(390, "flower_pot").setUnlocalizedName("flowerPot").setTextureName("flower_pot") IEND(390, "flower_pot")
    IREG(391, "carrot").setFoodValues(4, 0.6f, false).setUnlocalizedName("carrots").setTextureName("carrot") IEND(391, "carrot")
    IREG(392, "potato").setFoodValues(1, 0.3f, false).setUnlocalizedName("potato").setTextureName("potato") IEND(392, "potato")
    IREG(393, "baked_potato").setFoodValues(6, 0.6f, false).setUnlocalizedName("potatoBaked").setTextureName("potato_baked") IEND(393, "baked_potato")
    IREG(394, "poisonous_potato").setFoodValues(2, 0.3f, false)
        .setUnlocalizedName("potatoPoisonous").setTextureName("potato_poisonous") IEND(394, "poisonous_potato")
    IREG(395, "map").setUnlocalizedName("emptyMap").setTextureName("map_empty") IEND(395, "map")
    IREG(396, "golden_carrot").setFoodValues(6, 1.2f, false)
        .setUnlocalizedName("carrotGolden").setTextureName("carrot_golden") IEND(396, "golden_carrot")
    IREG(397, "skull").setUnlocalizedName("skull").setTextureName("skull") IEND(397, "skull")
    IREG(398, "carrot_on_a_stick").setMaxDurability(25)
        .setUnlocalizedName("carrotOnAStick").setTextureName("carrot_on_a_stick") IEND(398, "carrot_on_a_stick")
    IREG(399, "nether_star").setUnlocalizedName("netherStar").setTextureName("nether_star") IEND(399, "nether_star")
    IREG(400, "pumpkin_pie").setFoodValues(8, 0.3f, false)
        .setUnlocalizedName("pumpkinPie").setTextureName("pumpkin_pie") IEND(400, "pumpkin_pie")
    IREG(401, "fireworks").setUnlocalizedName("fireworks").setTextureName("fireworks") IEND(401, "fireworks")
    IREG(402, "firework_charge").setUnlocalizedName("fireworksCharge").setTextureName("fireworks_charge") IEND(402, "firework_charge")
    IREG(403, "enchanted_book").setMaxStackSize(1).setUnlocalizedName("enchantedBook").setTextureName("book_enchanted") IEND(403, "enchanted_book")
    IREG(404, "comparator").setUnlocalizedName("comparator").setTextureName("comparator") IEND(404, "comparator")
    IREG(405, "netherbrick").setUnlocalizedName("netherbrick").setTextureName("netherbrick") IEND(405, "netherbrick")
    IREG(406, "quartz").setUnlocalizedName("netherquartz").setTextureName("quartz") IEND(406, "quartz")
    IREG(407, "tnt_minecart").setMaxStackSize(1).setUnlocalizedName("minecartTnt").setTextureName("minecart_tnt") IEND(407, "tnt_minecart")
    IREG(408, "hopper_minecart").setMaxStackSize(1).setUnlocalizedName("minecartHopper").setTextureName("minecart_hopper") IEND(408, "hopper_minecart")

    // IDs 409-416 unused

    IREG(417, "iron_horse_armor").setMaxStackSize(1).setUnlocalizedName("horsearmormetal").setTextureName("iron_horse_armor") IEND(417, "iron_horse_armor")
    IREG(418, "golden_horse_armor").setMaxStackSize(1).setUnlocalizedName("horsearmorgold").setTextureName("gold_horse_armor") IEND(418, "golden_horse_armor")
    IREG(419, "diamond_horse_armor").setMaxStackSize(1).setUnlocalizedName("horsearmordiamond").setTextureName("diamond_horse_armor") IEND(419, "diamond_horse_armor")
    IREG(420, "lead").setUnlocalizedName("leash").setTextureName("lead") IEND(420, "lead")
    IREG(421, "name_tag").setUnlocalizedName("nameTag").setTextureName("name_tag") IEND(421, "name_tag")
    IREG(422, "command_block_minecart").setMaxStackSize(1)
        .setUnlocalizedName("minecartCommandBlock").setTextureName("minecart_command_block") IEND(422, "command_block_minecart")

    // Music discs (IDs 2256-2267)
    IREG(2256, "record_13").setCategory(ItemCategory::Record).setMaxStackSize(1)
        .setUnlocalizedName("record").setTextureName("record_13") IEND(2256, "record_13")
    IREG(2257, "record_cat").setCategory(ItemCategory::Record).setMaxStackSize(1)
        .setUnlocalizedName("record").setTextureName("record_cat") IEND(2257, "record_cat")
    IREG(2258, "record_blocks").setCategory(ItemCategory::Record).setMaxStackSize(1)
        .setUnlocalizedName("record").setTextureName("record_blocks") IEND(2258, "record_blocks")
    IREG(2259, "record_chirp").setCategory(ItemCategory::Record).setMaxStackSize(1)
        .setUnlocalizedName("record").setTextureName("record_chirp") IEND(2259, "record_chirp")
    IREG(2260, "record_far").setCategory(ItemCategory::Record).setMaxStackSize(1)
        .setUnlocalizedName("record").setTextureName("record_far") IEND(2260, "record_far")
    IREG(2261, "record_mall").setCategory(ItemCategory::Record).setMaxStackSize(1)
        .setUnlocalizedName("record").setTextureName("record_mall") IEND(2261, "record_mall")
    IREG(2262, "record_mellohi").setCategory(ItemCategory::Record).setMaxStackSize(1)
        .setUnlocalizedName("record").setTextureName("record_mellohi") IEND(2262, "record_mellohi")
    IREG(2263, "record_stal").setCategory(ItemCategory::Record).setMaxStackSize(1)
        .setUnlocalizedName("record").setTextureName("record_stal") IEND(2263, "record_stal")
    IREG(2264, "record_strad").setCategory(ItemCategory::Record).setMaxStackSize(1)
        .setUnlocalizedName("record").setTextureName("record_strad") IEND(2264, "record_strad")
    IREG(2265, "record_ward").setCategory(ItemCategory::Record).setMaxStackSize(1)
        .setUnlocalizedName("record").setTextureName("record_ward") IEND(2265, "record_ward")
    IREG(2266, "record_11").setCategory(ItemCategory::Record).setMaxStackSize(1)
        .setUnlocalizedName("record").setTextureName("record_11") IEND(2266, "record_11")
    IREG(2267, "record_wait").setCategory(ItemCategory::Record).setMaxStackSize(1)
        .setUnlocalizedName("record").setTextureName("record_wait") IEND(2267, "record_wait")

    // --- Auto-register ItemBlock entries for blocks that don't have special item forms ---
    // Java: The registerItems() method also registers an ItemBlock for most blocks.
    // We use simplified ItemBlock items (category=Block) for blocks not in the skip set.
    // Skip set matches Java: air, bed, piston_head, piston_extension, flowing_water, water,
    //   flowing_lava, lava, double_stone_slab, fire, redstone_wire, wheat, standing_sign,
    //   wooden_door, wall_sign, iron_door, lit_redstone_ore, unlit_redstone_torch,
    //   reeds, sugar cane (reeds), cake, unpowered_repeater, powered_repeater,
    //   pumpkin_stem, melon_stem, nether_wart, brewing_stand (block), cauldron (block),
    //   lit_redstone_lamp, cocoa, tripwire, flower_pot (block), carrots, potatoes,
    //   skull (block), unpowered_comparator, powered_comparator, redstone_wire

    // Block IDs that should NOT get an auto-ItemBlock in the item registry
    static const int skipBlockIds[] = {
        0,   // air
        26,  // bed
        34,  // piston_head
        36,  // piston_extension
        51,  // fire
        55,  // redstone_wire
        59,  // wheat
        63,  // standing_sign
        64,  // wooden_door
        68,  // wall_sign
        71,  // iron_door
        74,  // lit_redstone_ore
        75,  // unlit_redstone_torch
        83,  // reeds
        92,  // cake
        93,  // unpowered_repeater
        94,  // powered_repeater
        104, // pumpkin_stem
        105, // melon_stem
        115, // nether_wart
        117, // brewing_stand
        118, // cauldron
        124, // lit_redstone_lamp
        127, // cocoa
        132, // tripwire
        140, // flower_pot
        141, // carrots
        142, // potatoes
        144, // skull
        149, // unpowered_comparator
        150, // powered_comparator
        -1   // sentinel
    };

    auto shouldSkip = [](int id) {
        for (int i = 0; skipBlockIds[i] != -1; ++i) {
            if (skipBlockIds[i] == id) return true;
        }
        return false;
    };

    // Register ItemBlock for each block
    for (int32_t blockId = 0; blockId < Block::blockRegistry.maxId(); ++blockId) {
        Block* block = Block::getBlockById(blockId);
        if (!block) continue;
        if (shouldSkip(blockId)) continue;
        // Don't overwrite standalone items that already have this ID
        if (itemRegistry.containsId(blockId)) continue;

        Item& blockItem = alloc();
        blockItem.setCategory(ItemCategory::Block);
        blockItem.setUnlocalizedName(block->getUnlocalizedName());
        itemRegistry.addObject(blockId, Block::blockRegistry.getNameForObject(block), &blockItem);
    }

    #undef IREG
    #undef IEND

    std::cout << "[Item] Registered " << itemRegistry.size() << " items\n";
}

} // namespace mccpp
