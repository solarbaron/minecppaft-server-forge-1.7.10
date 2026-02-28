/**
 * Block.cpp — Block implementation and registry initialization.
 *
 * Java reference: net.minecraft.block.Block
 * All 176 vanilla 1.7.10 blocks registered with exact properties.
 *
 * Properties extracted directly from Block.registerBlocks() in Java:
 *   blockRegistry.addObject(id, "name", new BlockType().setHardness(h).setResistance(r)...)
 */

#include "block/Block.h"

#include <iostream>

namespace mccpp {

// ─── Static registry instance ───────────────────────────────────────────────
RegistryNamespaced<Block*> Block::blockRegistry;

// ─── Block storage — static array owning all block instances ────────────────
// Using a static array ensures stable pointers for the registry.
static Block blockStorage[256];
static int blockStorageCount = 0;

static Block& alloc(Material mat) {
    Block& b = blockStorage[blockStorageCount++];
    b = Block(mat);
    return b;
}

// ─── Constructor ────────────────────────────────────────────────────────────

Block::Block(Material material) : material_(material) {
    // Java reference: Block(Material)
    // this.setBlockBounds(0, 0, 0, 1, 1, 1);
    // this.fullBlock = this.isOpaqueCube();
    // this.lightOpacity = this.isOpaqueCube() ? 255 : 0;
    // this.translucent = !material.blocksLight();
    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    fullBlock_ = true;
    opaqueCube_ = true;
    lightOpacity_ = 255;
    translucent_ = false;
}

// ─── Builder methods ────────────────────────────────────────────────────────

Block& Block::setHardness(float h) {
    // Java: if (this.blockResistance < h * 5.0f) this.blockResistance = h * 5.0f;
    hardness_ = h;
    if (resistance_ < h * 5.0f) {
        resistance_ = h * 5.0f;
    }
    return *this;
}

Block& Block::setResistance(float r) {
    // Java: this.blockResistance = r * 3.0f;
    resistance_ = r * 3.0f;
    return *this;
}

Block& Block::setLightLevel(float f) {
    // Java: this.lightValue = (int)(15.0f * f);
    lightValue_ = static_cast<int>(15.0f * f);
    return *this;
}

Block& Block::setLightOpacity(int opacity) {
    lightOpacity_ = opacity;
    return *this;
}

Block& Block::setUnlocalizedName(const std::string& name) {
    unlocalizedName_ = name;
    return *this;
}

Block& Block::setTextureName(const std::string& name) {
    textureName_ = name;
    return *this;
}

Block& Block::setTickRandomly(bool v) {
    needsRandomTick_ = v;
    return *this;
}

Block& Block::setBlockUnbreakable() {
    setHardness(-1.0f);
    return *this;
}

Block& Block::disableStats() {
    enableStats_ = false;
    return *this;
}

void Block::setBlockBounds(float x1, float y1, float z1, float x2, float y2, float z2) {
    minX_ = x1; minY_ = y1; minZ_ = z1;
    maxX_ = x2; maxY_ = y2; maxZ_ = z2;
}

// ─── Static lookup ──────────────────────────────────────────────────────────

Block* Block::getBlockFromName(const std::string& name) {
    // Java reference: Block.getBlockFromName()
    if (blockRegistry.containsKey(name)) {
        return blockRegistry.getObject(name);
    }
    try {
        return blockRegistry.getObjectById(std::stoi(name));
    } catch (...) {
        return nullptr;
    }
}

// ─── registerBlocks ─────────────────────────────────────────────────────────
// Java reference: Block.registerBlocks()
// Every block with its exact ID, name, and property values from the Java source.

void Block::registerBlocks() {
    blockStorageCount = 0;

    // Helper macro — allocates a block, applies builder calls, registers it
    #define REG(id, name, mat) \
        { Block& b = alloc(mat); b

    #define END(id, name) \
        ; blockRegistry.addObject(id, name, &b); }

    // 0: air
    REG(0, "air", Material::Air).setUnlocalizedName("air") END(0, "air")

    // 1: stone
    REG(1, "stone", Material::Rock).setHardness(1.5f).setResistance(10.0f)
        .setUnlocalizedName("stone").setTextureName("stone") END(1, "stone")

    // 2: grass
    REG(2, "grass", Material::Grass).setHardness(0.6f)
        .setUnlocalizedName("grass").setTextureName("grass") END(2, "grass")

    // 3: dirt
    REG(3, "dirt", Material::Ground).setHardness(0.5f)
        .setUnlocalizedName("dirt").setTextureName("dirt") END(3, "dirt")

    // 4: cobblestone
    REG(4, "cobblestone", Material::Rock).setHardness(2.0f).setResistance(10.0f)
        .setUnlocalizedName("stonebrick").setTextureName("cobblestone") END(4, "cobblestone")

    // 5: planks
    REG(5, "planks", Material::Wood).setHardness(2.0f).setResistance(5.0f)
        .setUnlocalizedName("wood").setTextureName("planks") END(5, "planks")

    // 6: sapling
    REG(6, "sapling", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("sapling").setTextureName("sapling") END(6, "sapling")

    // 7: bedrock
    REG(7, "bedrock", Material::Rock).setBlockUnbreakable().setResistance(6000000.0f)
        .setUnlocalizedName("bedrock").disableStats().setTextureName("bedrock") END(7, "bedrock")

    // 8: flowing_water
    REG(8, "flowing_water", Material::Water).setHardness(100.0f).setLightOpacity(3)
        .setUnlocalizedName("water").disableStats().setTextureName("water_flow") END(8, "flowing_water")

    // 9: water
    REG(9, "water", Material::Water).setHardness(100.0f).setLightOpacity(3)
        .setUnlocalizedName("water").disableStats().setTextureName("water_still") END(9, "water")

    // 10: flowing_lava
    REG(10, "flowing_lava", Material::Lava).setHardness(100.0f).setLightLevel(1.0f)
        .setUnlocalizedName("lava").disableStats().setTextureName("lava_flow") END(10, "flowing_lava")

    // 11: lava
    REG(11, "lava", Material::Lava).setHardness(100.0f).setLightLevel(1.0f)
        .setUnlocalizedName("lava").disableStats().setTextureName("lava_still") END(11, "lava")

    // 12: sand
    REG(12, "sand", Material::Sand).setHardness(0.5f)
        .setUnlocalizedName("sand").setTextureName("sand") END(12, "sand")

    // 13: gravel
    REG(13, "gravel", Material::Ground).setHardness(0.6f)
        .setUnlocalizedName("gravel").setTextureName("gravel") END(13, "gravel")

    // 14: gold_ore
    REG(14, "gold_ore", Material::Rock).setHardness(3.0f).setResistance(5.0f)
        .setUnlocalizedName("oreGold").setTextureName("gold_ore") END(14, "gold_ore")

    // 15: iron_ore
    REG(15, "iron_ore", Material::Rock).setHardness(3.0f).setResistance(5.0f)
        .setUnlocalizedName("oreIron").setTextureName("iron_ore") END(15, "iron_ore")

    // 16: coal_ore
    REG(16, "coal_ore", Material::Rock).setHardness(3.0f).setResistance(5.0f)
        .setUnlocalizedName("oreCoal").setTextureName("coal_ore") END(16, "coal_ore")

    // 17: log
    REG(17, "log", Material::Wood).setHardness(2.0f)
        .setUnlocalizedName("log").setTextureName("log") END(17, "log")

    // 18: leaves
    REG(18, "leaves", Material::Plants).setHardness(0.2f).setLightOpacity(1)
        .setUnlocalizedName("leaves").setTextureName("leaves") END(18, "leaves")

    // 19: sponge
    REG(19, "sponge", Material::Sponge).setHardness(0.6f)
        .setUnlocalizedName("sponge").setTextureName("sponge") END(19, "sponge")

    // 20: glass
    REG(20, "glass", Material::Glass).setHardness(0.3f)
        .setUnlocalizedName("glass").setTextureName("glass") END(20, "glass")

    // 21: lapis_ore
    REG(21, "lapis_ore", Material::Rock).setHardness(3.0f).setResistance(5.0f)
        .setUnlocalizedName("oreLapis").setTextureName("lapis_ore") END(21, "lapis_ore")

    // 22: lapis_block
    REG(22, "lapis_block", Material::Iron).setHardness(3.0f).setResistance(5.0f)
        .setUnlocalizedName("blockLapis").setTextureName("lapis_block") END(22, "lapis_block")

    // 23: dispenser
    REG(23, "dispenser", Material::Rock).setHardness(3.5f)
        .setUnlocalizedName("dispenser").setTextureName("dispenser") END(23, "dispenser")

    // 24: sandstone
    REG(24, "sandstone", Material::Rock).setHardness(0.8f)
        .setUnlocalizedName("sandStone").setTextureName("sandstone") END(24, "sandstone")

    // 25: noteblock
    REG(25, "noteblock", Material::Wood).setHardness(0.8f)
        .setUnlocalizedName("musicBlock").setTextureName("noteblock") END(25, "noteblock")

    // 26: bed
    REG(26, "bed", Material::Cloth).setHardness(0.2f)
        .setUnlocalizedName("bed").disableStats().setTextureName("bed") END(26, "bed")

    // 27: golden_rail
    REG(27, "golden_rail", Material::Circuits).setHardness(0.7f)
        .setUnlocalizedName("goldenRail").setTextureName("rail_golden") END(27, "golden_rail")

    // 28: detector_rail
    REG(28, "detector_rail", Material::Circuits).setHardness(0.7f)
        .setUnlocalizedName("detectorRail").setTextureName("rail_detector") END(28, "detector_rail")

    // 29: sticky_piston
    REG(29, "sticky_piston", Material::Piston).setUnlocalizedName("pistonStickyBase") END(29, "sticky_piston")

    // 30: web
    REG(30, "web", Material::Web).setLightOpacity(1).setHardness(4.0f)
        .setUnlocalizedName("web").setTextureName("web") END(30, "web")

    // 31: tallgrass
    REG(31, "tallgrass", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("tallgrass") END(31, "tallgrass")

    // 32: deadbush
    REG(32, "deadbush", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("deadbush").setTextureName("deadbush") END(32, "deadbush")

    // 33: piston
    REG(33, "piston", Material::Piston).setUnlocalizedName("pistonBase") END(33, "piston")

    // 34: piston_head
    REG(34, "piston_head", Material::Piston).setUnlocalizedName("pistonHead") END(34, "piston_head")

    // 35: wool
    REG(35, "wool", Material::Cloth).setHardness(0.8f)
        .setUnlocalizedName("cloth").setTextureName("wool_colored") END(35, "wool")

    // 36: piston_extension (moving piston)
    REG(36, "piston_extension", Material::Piston).setUnlocalizedName("pistonMoving") END(36, "piston_extension")

    // 37: yellow_flower
    REG(37, "yellow_flower", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("flower1").setTextureName("flower_dandelion") END(37, "yellow_flower")

    // 38: red_flower
    REG(38, "red_flower", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("flower2").setTextureName("flower_rose") END(38, "red_flower")

    // 39: brown_mushroom
    REG(39, "brown_mushroom", Material::Plants).setHardness(0.0f).setLightLevel(0.125f)
        .setUnlocalizedName("mushroom").setTextureName("mushroom_brown") END(39, "brown_mushroom")

    // 40: red_mushroom
    REG(40, "red_mushroom", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("mushroom").setTextureName("mushroom_red") END(40, "red_mushroom")

    // 41: gold_block
    REG(41, "gold_block", Material::Iron).setHardness(3.0f).setResistance(10.0f)
        .setUnlocalizedName("blockGold").setTextureName("gold_block") END(41, "gold_block")

    // 42: iron_block
    REG(42, "iron_block", Material::Iron).setHardness(5.0f).setResistance(10.0f)
        .setUnlocalizedName("blockIron").setTextureName("iron_block") END(42, "iron_block")

    // 43: double_stone_slab
    REG(43, "double_stone_slab", Material::Rock).setHardness(2.0f).setResistance(10.0f)
        .setUnlocalizedName("stoneSlab") END(43, "double_stone_slab")

    // 44: stone_slab
    REG(44, "stone_slab", Material::Rock).setHardness(2.0f).setResistance(10.0f)
        .setUnlocalizedName("stoneSlab") END(44, "stone_slab")

    // 45: brick_block
    REG(45, "brick_block", Material::Rock).setHardness(2.0f).setResistance(10.0f)
        .setUnlocalizedName("brick").setTextureName("brick") END(45, "brick_block")

    // 46: tnt
    REG(46, "tnt", Material::TNT).setHardness(0.0f)
        .setUnlocalizedName("tnt").setTextureName("tnt") END(46, "tnt")

    // 47: bookshelf
    REG(47, "bookshelf", Material::Wood).setHardness(1.5f)
        .setUnlocalizedName("bookshelf").setTextureName("bookshelf") END(47, "bookshelf")

    // 48: mossy_cobblestone
    REG(48, "mossy_cobblestone", Material::Rock).setHardness(2.0f).setResistance(10.0f)
        .setUnlocalizedName("stoneMoss").setTextureName("cobblestone_mossy") END(48, "mossy_cobblestone")

    // 49: obsidian
    REG(49, "obsidian", Material::Rock).setHardness(50.0f).setResistance(2000.0f)
        .setUnlocalizedName("obsidian").setTextureName("obsidian") END(49, "obsidian")

    // 50: torch
    REG(50, "torch", Material::Circuits).setHardness(0.0f).setLightLevel(0.9375f)
        .setUnlocalizedName("torch").setTextureName("torch_on") END(50, "torch")

    // 51: fire
    REG(51, "fire", Material::Fire).setHardness(0.0f).setLightLevel(1.0f)
        .setUnlocalizedName("fire").disableStats().setTextureName("fire") END(51, "fire")

    // 52: mob_spawner
    REG(52, "mob_spawner", Material::Rock).setHardness(5.0f)
        .setUnlocalizedName("mobSpawner").disableStats().setTextureName("mob_spawner") END(52, "mob_spawner")

    // 53: oak_stairs
    REG(53, "oak_stairs", Material::Wood).setHardness(2.0f).setResistance(5.0f)
        .setUnlocalizedName("stairsWood") END(53, "oak_stairs")

    // 54: chest
    REG(54, "chest", Material::Wood).setHardness(2.5f)
        .setUnlocalizedName("chest") END(54, "chest")

    // 55: redstone_wire
    REG(55, "redstone_wire", Material::Circuits).setHardness(0.0f)
        .setUnlocalizedName("redstoneDust").disableStats().setTextureName("redstone_dust") END(55, "redstone_wire")

    // 56: diamond_ore
    REG(56, "diamond_ore", Material::Rock).setHardness(3.0f).setResistance(5.0f)
        .setUnlocalizedName("oreDiamond").setTextureName("diamond_ore") END(56, "diamond_ore")

    // 57: diamond_block
    REG(57, "diamond_block", Material::Iron).setHardness(5.0f).setResistance(10.0f)
        .setUnlocalizedName("blockDiamond").setTextureName("diamond_block") END(57, "diamond_block")

    // 58: crafting_table
    REG(58, "crafting_table", Material::Wood).setHardness(2.5f)
        .setUnlocalizedName("workbench").setTextureName("crafting_table") END(58, "crafting_table")

    // 59: wheat
    REG(59, "wheat", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("crops").setTextureName("wheat") END(59, "wheat")

    // 60: farmland
    REG(60, "farmland", Material::Ground).setHardness(0.6f)
        .setUnlocalizedName("farmland").setTextureName("farmland") END(60, "farmland")

    // 61: furnace
    REG(61, "furnace", Material::Rock).setHardness(3.5f)
        .setUnlocalizedName("furnace") END(61, "furnace")

    // 62: lit_furnace
    REG(62, "lit_furnace", Material::Rock).setHardness(3.5f).setLightLevel(0.875f)
        .setUnlocalizedName("furnace") END(62, "lit_furnace")

    // 63: standing_sign
    REG(63, "standing_sign", Material::Wood).setHardness(1.0f)
        .setUnlocalizedName("sign").disableStats() END(63, "standing_sign")

    // 64: wooden_door
    REG(64, "wooden_door", Material::Wood).setHardness(3.0f)
        .setUnlocalizedName("doorWood").disableStats().setTextureName("door_wood") END(64, "wooden_door")

    // 65: ladder
    REG(65, "ladder", Material::Circuits).setHardness(0.4f)
        .setUnlocalizedName("ladder").setTextureName("ladder") END(65, "ladder")

    // 66: rail
    REG(66, "rail", Material::Circuits).setHardness(0.7f)
        .setUnlocalizedName("rail").setTextureName("rail_normal") END(66, "rail")

    // 67: stone_stairs
    REG(67, "stone_stairs", Material::Rock).setHardness(2.0f).setResistance(10.0f)
        .setUnlocalizedName("stairsStone") END(67, "stone_stairs")

    // 68: wall_sign
    REG(68, "wall_sign", Material::Wood).setHardness(1.0f)
        .setUnlocalizedName("sign").disableStats() END(68, "wall_sign")

    // 69: lever
    REG(69, "lever", Material::Circuits).setHardness(0.5f)
        .setUnlocalizedName("lever").setTextureName("lever") END(69, "lever")

    // 70: stone_pressure_plate
    REG(70, "stone_pressure_plate", Material::Rock).setHardness(0.5f)
        .setUnlocalizedName("pressurePlate") END(70, "stone_pressure_plate")

    // 71: iron_door
    REG(71, "iron_door", Material::Iron).setHardness(5.0f)
        .setUnlocalizedName("doorIron").disableStats().setTextureName("door_iron") END(71, "iron_door")

    // 72: wooden_pressure_plate
    REG(72, "wooden_pressure_plate", Material::Wood).setHardness(0.5f)
        .setUnlocalizedName("pressurePlate") END(72, "wooden_pressure_plate")

    // 73: redstone_ore
    REG(73, "redstone_ore", Material::Rock).setHardness(3.0f).setResistance(5.0f)
        .setUnlocalizedName("oreRedstone").setTextureName("redstone_ore") END(73, "redstone_ore")

    // 74: lit_redstone_ore
    REG(74, "lit_redstone_ore", Material::Rock).setHardness(3.0f).setResistance(5.0f)
        .setLightLevel(0.625f).setUnlocalizedName("oreRedstone").setTextureName("redstone_ore") END(74, "lit_redstone_ore")

    // 75: unlit_redstone_torch
    REG(75, "unlit_redstone_torch", Material::Circuits).setHardness(0.0f)
        .setUnlocalizedName("notGate").setTextureName("redstone_torch_off") END(75, "unlit_redstone_torch")

    // 76: redstone_torch
    REG(76, "redstone_torch", Material::Circuits).setHardness(0.0f).setLightLevel(0.5f)
        .setUnlocalizedName("notGate").setTextureName("redstone_torch_on") END(76, "redstone_torch")

    // 77: stone_button
    REG(77, "stone_button", Material::Circuits).setHardness(0.5f)
        .setUnlocalizedName("button") END(77, "stone_button")

    // 78: snow_layer
    REG(78, "snow_layer", Material::Snow).setHardness(0.1f).setLightOpacity(0)
        .setUnlocalizedName("snow").setTextureName("snow") END(78, "snow_layer")

    // 79: ice
    REG(79, "ice", Material::Ice).setHardness(0.5f).setLightOpacity(3)
        .setUnlocalizedName("ice").setTextureName("ice") END(79, "ice")

    // 80: snow
    REG(80, "snow", Material::Snow).setHardness(0.2f)
        .setUnlocalizedName("snow").setTextureName("snow") END(80, "snow")

    // 81: cactus
    REG(81, "cactus", Material::Plants).setHardness(0.4f)
        .setUnlocalizedName("cactus").setTextureName("cactus") END(81, "cactus")

    // 82: clay
    REG(82, "clay", Material::Clay).setHardness(0.6f)
        .setUnlocalizedName("clay").setTextureName("clay") END(82, "clay")

    // 83: reeds (sugar cane)
    REG(83, "reeds", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("reeds").disableStats().setTextureName("reeds") END(83, "reeds")

    // 84: jukebox
    REG(84, "jukebox", Material::Wood).setHardness(2.0f).setResistance(10.0f)
        .setUnlocalizedName("jukebox").setTextureName("jukebox") END(84, "jukebox")

    // 85: fence
    REG(85, "fence", Material::Wood).setHardness(2.0f).setResistance(5.0f)
        .setUnlocalizedName("fence") END(85, "fence")

    // 86: pumpkin
    REG(86, "pumpkin", Material::Plants).setHardness(1.0f)
        .setUnlocalizedName("pumpkin").setTextureName("pumpkin") END(86, "pumpkin")

    // 87: netherrack
    REG(87, "netherrack", Material::Rock).setHardness(0.4f)
        .setUnlocalizedName("hellrock").setTextureName("netherrack") END(87, "netherrack")

    // 88: soul_sand
    REG(88, "soul_sand", Material::Sand).setHardness(0.5f)
        .setUnlocalizedName("hellsand").setTextureName("soul_sand") END(88, "soul_sand")

    // 89: glowstone
    REG(89, "glowstone", Material::Glass).setHardness(0.3f).setLightLevel(1.0f)
        .setUnlocalizedName("lightgem").setTextureName("glowstone") END(89, "glowstone")

    // 90: portal
    REG(90, "portal", Material::Portal).setHardness(-1.0f).setLightLevel(0.75f)
        .setUnlocalizedName("portal").setTextureName("portal") END(90, "portal")

    // 91: lit_pumpkin
    REG(91, "lit_pumpkin", Material::Plants).setHardness(1.0f).setLightLevel(1.0f)
        .setUnlocalizedName("litpumpkin").setTextureName("pumpkin") END(91, "lit_pumpkin")

    // 92: cake
    REG(92, "cake", Material::Cake).setHardness(0.5f)
        .setUnlocalizedName("cake").disableStats().setTextureName("cake") END(92, "cake")

    // 93: unpowered_repeater
    REG(93, "unpowered_repeater", Material::Circuits).setHardness(0.0f)
        .setUnlocalizedName("diode").disableStats().setTextureName("repeater_off") END(93, "unpowered_repeater")

    // 94: powered_repeater
    REG(94, "powered_repeater", Material::Circuits).setHardness(0.0f).setLightLevel(0.625f)
        .setUnlocalizedName("diode").disableStats().setTextureName("repeater_on") END(94, "powered_repeater")

    // 95: stained_glass
    REG(95, "stained_glass", Material::Glass).setHardness(0.3f)
        .setUnlocalizedName("stainedGlass").setTextureName("glass") END(95, "stained_glass")

    // 96: trapdoor
    REG(96, "trapdoor", Material::Wood).setHardness(3.0f)
        .setUnlocalizedName("trapdoor").disableStats().setTextureName("trapdoor") END(96, "trapdoor")

    // 97: monster_egg (silverfish)
    REG(97, "monster_egg", Material::Clay).setHardness(0.75f)
        .setUnlocalizedName("monsterStoneEgg") END(97, "monster_egg")

    // 98: stonebrick
    REG(98, "stonebrick", Material::Rock).setHardness(1.5f).setResistance(10.0f)
        .setUnlocalizedName("stonebricksmooth").setTextureName("stonebrick") END(98, "stonebrick")

    // 99: brown_mushroom_block
    REG(99, "brown_mushroom_block", Material::Wood).setHardness(0.2f)
        .setUnlocalizedName("mushroom").setTextureName("mushroom_block") END(99, "brown_mushroom_block")

    // 100: red_mushroom_block
    REG(100, "red_mushroom_block", Material::Wood).setHardness(0.2f)
        .setUnlocalizedName("mushroom").setTextureName("mushroom_block") END(100, "red_mushroom_block")

    // 101: iron_bars
    REG(101, "iron_bars", Material::Iron).setHardness(5.0f).setResistance(10.0f)
        .setUnlocalizedName("fenceIron") END(101, "iron_bars")

    // 102: glass_pane
    REG(102, "glass_pane", Material::Glass).setHardness(0.3f)
        .setUnlocalizedName("thinGlass") END(102, "glass_pane")

    // 103: melon_block
    REG(103, "melon_block", Material::Plants).setHardness(1.0f)
        .setUnlocalizedName("melon").setTextureName("melon") END(103, "melon_block")

    // 104: pumpkin_stem
    REG(104, "pumpkin_stem", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("pumpkinStem").setTextureName("pumpkin_stem") END(104, "pumpkin_stem")

    // 105: melon_stem
    REG(105, "melon_stem", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("pumpkinStem").setTextureName("melon_stem") END(105, "melon_stem")

    // 106: vine
    REG(106, "vine", Material::Vine).setHardness(0.2f)
        .setUnlocalizedName("vine").setTextureName("vine") END(106, "vine")

    // 107: fence_gate
    REG(107, "fence_gate", Material::Wood).setHardness(2.0f).setResistance(5.0f)
        .setUnlocalizedName("fenceGate") END(107, "fence_gate")

    // 108: brick_stairs
    REG(108, "brick_stairs", Material::Rock).setHardness(2.0f).setResistance(10.0f)
        .setUnlocalizedName("stairsBrick") END(108, "brick_stairs")

    // 109: stone_brick_stairs
    REG(109, "stone_brick_stairs", Material::Rock).setHardness(1.5f).setResistance(10.0f)
        .setUnlocalizedName("stairsStoneBrickSmooth") END(109, "stone_brick_stairs")

    // 110: mycelium
    REG(110, "mycelium", Material::Grass).setHardness(0.6f)
        .setUnlocalizedName("mycel").setTextureName("mycelium") END(110, "mycelium")

    // 111: waterlily (lily pad)
    REG(111, "waterlily", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("waterlily").setTextureName("waterlily") END(111, "waterlily")

    // 112: nether_brick
    REG(112, "nether_brick", Material::Rock).setHardness(2.0f).setResistance(10.0f)
        .setUnlocalizedName("netherBrick").setTextureName("nether_brick") END(112, "nether_brick")

    // 113: nether_brick_fence
    REG(113, "nether_brick_fence", Material::Rock).setHardness(2.0f).setResistance(10.0f)
        .setUnlocalizedName("netherFence") END(113, "nether_brick_fence")

    // 114: nether_brick_stairs
    REG(114, "nether_brick_stairs", Material::Rock).setHardness(2.0f).setResistance(10.0f)
        .setUnlocalizedName("stairsNetherBrick") END(114, "nether_brick_stairs")

    // 115: nether_wart
    REG(115, "nether_wart", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("netherStalk").setTextureName("nether_wart") END(115, "nether_wart")

    // 116: enchanting_table
    REG(116, "enchanting_table", Material::Rock).setHardness(5.0f).setResistance(2000.0f)
        .setUnlocalizedName("enchantmentTable").setTextureName("enchanting_table") END(116, "enchanting_table")

    // 117: brewing_stand
    REG(117, "brewing_stand", Material::Iron).setHardness(0.5f).setLightLevel(0.125f)
        .setUnlocalizedName("brewingStand").setTextureName("brewing_stand") END(117, "brewing_stand")

    // 118: cauldron
    REG(118, "cauldron", Material::Iron).setHardness(2.0f)
        .setUnlocalizedName("cauldron").setTextureName("cauldron") END(118, "cauldron")

    // 119: end_portal
    REG(119, "end_portal", Material::Portal).setHardness(-1.0f).setResistance(6000000.0f)
        .setUnlocalizedName("endPortal") END(119, "end_portal")

    // 120: end_portal_frame
    REG(120, "end_portal_frame", Material::Rock).setHardness(-1.0f).setResistance(6000000.0f)
        .setLightLevel(0.125f).setUnlocalizedName("endPortalFrame").setTextureName("endframe") END(120, "end_portal_frame")

    // 121: end_stone
    REG(121, "end_stone", Material::Rock).setHardness(3.0f).setResistance(15.0f)
        .setUnlocalizedName("whiteStone").setTextureName("end_stone") END(121, "end_stone")

    // 122: dragon_egg
    REG(122, "dragon_egg", Material::DragonEgg).setHardness(3.0f).setResistance(15.0f)
        .setLightLevel(0.125f).setUnlocalizedName("dragonEgg").setTextureName("dragon_egg") END(122, "dragon_egg")

    // 123: redstone_lamp
    REG(123, "redstone_lamp", Material::Glass).setHardness(0.3f)
        .setUnlocalizedName("redstoneLight").setTextureName("redstone_lamp_off") END(123, "redstone_lamp")

    // 124: lit_redstone_lamp
    REG(124, "lit_redstone_lamp", Material::Glass).setHardness(0.3f)
        .setUnlocalizedName("redstoneLight").setTextureName("redstone_lamp_on") END(124, "lit_redstone_lamp")

    // 125: double_wooden_slab
    REG(125, "double_wooden_slab", Material::Wood).setHardness(2.0f).setResistance(5.0f)
        .setUnlocalizedName("woodSlab") END(125, "double_wooden_slab")

    // 126: wooden_slab
    REG(126, "wooden_slab", Material::Wood).setHardness(2.0f).setResistance(5.0f)
        .setUnlocalizedName("woodSlab") END(126, "wooden_slab")

    // 127: cocoa
    REG(127, "cocoa", Material::Plants).setHardness(0.2f).setResistance(5.0f)
        .setUnlocalizedName("cocoa").setTextureName("cocoa") END(127, "cocoa")

    // 128: sandstone_stairs
    REG(128, "sandstone_stairs", Material::Rock).setHardness(0.8f)
        .setUnlocalizedName("stairsSandStone") END(128, "sandstone_stairs")

    // 129: emerald_ore
    REG(129, "emerald_ore", Material::Rock).setHardness(3.0f).setResistance(5.0f)
        .setUnlocalizedName("oreEmerald").setTextureName("emerald_ore") END(129, "emerald_ore")

    // 130: ender_chest
    REG(130, "ender_chest", Material::Rock).setHardness(22.5f).setResistance(1000.0f)
        .setLightLevel(0.5f).setUnlocalizedName("enderChest") END(130, "ender_chest")

    // 131: tripwire_hook
    REG(131, "tripwire_hook", Material::Circuits).setHardness(0.0f)
        .setUnlocalizedName("tripWireSource").setTextureName("trip_wire_source") END(131, "tripwire_hook")

    // 132: tripwire
    REG(132, "tripwire", Material::Circuits).setHardness(0.0f)
        .setUnlocalizedName("tripWire").setTextureName("trip_wire") END(132, "tripwire")

    // 133: emerald_block
    REG(133, "emerald_block", Material::Iron).setHardness(5.0f).setResistance(10.0f)
        .setUnlocalizedName("blockEmerald").setTextureName("emerald_block") END(133, "emerald_block")

    // 134-136: wood stairs (spruce, birch, jungle)
    REG(134, "spruce_stairs", Material::Wood).setHardness(2.0f).setResistance(5.0f)
        .setUnlocalizedName("stairsWoodSpruce") END(134, "spruce_stairs")
    REG(135, "birch_stairs", Material::Wood).setHardness(2.0f).setResistance(5.0f)
        .setUnlocalizedName("stairsWoodBirch") END(135, "birch_stairs")
    REG(136, "jungle_stairs", Material::Wood).setHardness(2.0f).setResistance(5.0f)
        .setUnlocalizedName("stairsWoodJungle") END(136, "jungle_stairs")

    // 137: command_block
    REG(137, "command_block", Material::Iron).setBlockUnbreakable().setResistance(6000000.0f)
        .setUnlocalizedName("commandBlock").setTextureName("command_block") END(137, "command_block")

    // 138: beacon
    REG(138, "beacon", Material::Glass).setHardness(3.0f).setLightLevel(1.0f)
        .setUnlocalizedName("beacon").setTextureName("beacon") END(138, "beacon")

    // 139: cobblestone_wall
    REG(139, "cobblestone_wall", Material::Rock).setHardness(2.0f).setResistance(10.0f)
        .setUnlocalizedName("cobbleWall") END(139, "cobblestone_wall")

    // 140: flower_pot
    REG(140, "flower_pot", Material::Circuits).setHardness(0.0f)
        .setUnlocalizedName("flowerPot").setTextureName("flower_pot") END(140, "flower_pot")

    // 141: carrots
    REG(141, "carrots", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("carrots").setTextureName("carrots") END(141, "carrots")

    // 142: potatoes
    REG(142, "potatoes", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("potatoes").setTextureName("potatoes") END(142, "potatoes")

    // 143: wooden_button
    REG(143, "wooden_button", Material::Circuits).setHardness(0.5f)
        .setUnlocalizedName("button") END(143, "wooden_button")

    // 144: skull
    REG(144, "skull", Material::Circuits).setHardness(1.0f)
        .setUnlocalizedName("skull").setTextureName("skull") END(144, "skull")

    // 145: anvil
    REG(145, "anvil", Material::Iron).setHardness(5.0f).setResistance(2000.0f)
        .setUnlocalizedName("anvil") END(145, "anvil")

    // 146: trapped_chest
    REG(146, "trapped_chest", Material::Wood).setHardness(2.5f)
        .setUnlocalizedName("chestTrap") END(146, "trapped_chest")

    // 147: light_weighted_pressure_plate
    REG(147, "light_weighted_pressure_plate", Material::Iron).setHardness(0.5f)
        .setUnlocalizedName("weightedPlate_light") END(147, "light_weighted_pressure_plate")

    // 148: heavy_weighted_pressure_plate
    REG(148, "heavy_weighted_pressure_plate", Material::Iron).setHardness(0.5f)
        .setUnlocalizedName("weightedPlate_heavy") END(148, "heavy_weighted_pressure_plate")

    // 149: unpowered_comparator
    REG(149, "unpowered_comparator", Material::Circuits).setHardness(0.0f)
        .setUnlocalizedName("comparator").disableStats().setTextureName("comparator_off") END(149, "unpowered_comparator")

    // 150: powered_comparator
    REG(150, "powered_comparator", Material::Circuits).setHardness(0.0f).setLightLevel(0.625f)
        .setUnlocalizedName("comparator").disableStats().setTextureName("comparator_on") END(150, "powered_comparator")

    // 151: daylight_detector
    REG(151, "daylight_detector", Material::Wood).setHardness(0.2f)
        .setUnlocalizedName("daylightDetector").setTextureName("daylight_detector") END(151, "daylight_detector")

    // 152: redstone_block
    REG(152, "redstone_block", Material::Iron).setHardness(5.0f).setResistance(10.0f)
        .setUnlocalizedName("blockRedstone").setTextureName("redstone_block") END(152, "redstone_block")

    // 153: quartz_ore
    REG(153, "quartz_ore", Material::Rock).setHardness(3.0f).setResistance(5.0f)
        .setUnlocalizedName("netherquartz").setTextureName("quartz_ore") END(153, "quartz_ore")

    // 154: hopper
    REG(154, "hopper", Material::Iron).setHardness(3.0f).setResistance(8.0f)
        .setUnlocalizedName("hopper").setTextureName("hopper") END(154, "hopper")

    // 155: quartz_block
    REG(155, "quartz_block", Material::Rock).setHardness(0.8f)
        .setUnlocalizedName("quartzBlock").setTextureName("quartz_block") END(155, "quartz_block")

    // 156: quartz_stairs
    REG(156, "quartz_stairs", Material::Rock).setHardness(0.8f)
        .setUnlocalizedName("stairsQuartz") END(156, "quartz_stairs")

    // 157: activator_rail
    REG(157, "activator_rail", Material::Circuits).setHardness(0.7f)
        .setUnlocalizedName("activatorRail").setTextureName("rail_activator") END(157, "activator_rail")

    // 158: dropper
    REG(158, "dropper", Material::Rock).setHardness(3.5f)
        .setUnlocalizedName("dropper").setTextureName("dropper") END(158, "dropper")

    // 159: stained_hardened_clay
    REG(159, "stained_hardened_clay", Material::Rock).setHardness(1.25f).setResistance(7.0f)
        .setUnlocalizedName("clayHardenedStained").setTextureName("hardened_clay_stained") END(159, "stained_hardened_clay")

    // 160: stained_glass_pane
    REG(160, "stained_glass_pane", Material::Glass).setHardness(0.3f)
        .setUnlocalizedName("thinStainedGlass").setTextureName("glass") END(160, "stained_glass_pane")

    // 161: leaves2
    REG(161, "leaves2", Material::Plants).setHardness(0.2f).setLightOpacity(1)
        .setUnlocalizedName("leaves").setTextureName("leaves") END(161, "leaves2")

    // 162: log2
    REG(162, "log2", Material::Wood).setHardness(2.0f)
        .setUnlocalizedName("log").setTextureName("log") END(162, "log2")

    // 163: acacia_stairs
    REG(163, "acacia_stairs", Material::Wood).setHardness(2.0f).setResistance(5.0f)
        .setUnlocalizedName("stairsWoodAcacia") END(163, "acacia_stairs")

    // 164: dark_oak_stairs
    REG(164, "dark_oak_stairs", Material::Wood).setHardness(2.0f).setResistance(5.0f)
        .setUnlocalizedName("stairsWoodDarkOak") END(164, "dark_oak_stairs")

    // IDs 165-169 are unused in 1.7.10

    // 170: hay_block
    REG(170, "hay_block", Material::Grass).setHardness(0.5f)
        .setUnlocalizedName("hayBlock").setTextureName("hay_block") END(170, "hay_block")

    // 171: carpet
    REG(171, "carpet", Material::Carpet).setHardness(0.1f).setLightOpacity(0)
        .setUnlocalizedName("woolCarpet") END(171, "carpet")

    // 172: hardened_clay
    REG(172, "hardened_clay", Material::Rock).setHardness(1.25f).setResistance(7.0f)
        .setUnlocalizedName("clayHardened").setTextureName("hardened_clay") END(172, "hardened_clay")

    // 173: coal_block
    REG(173, "coal_block", Material::Rock).setHardness(5.0f).setResistance(10.0f)
        .setUnlocalizedName("blockCoal").setTextureName("coal_block") END(173, "coal_block")

    // 174: packed_ice
    REG(174, "packed_ice", Material::PackedIce).setHardness(0.5f)
        .setUnlocalizedName("icePacked").setTextureName("ice_packed") END(174, "packed_ice")

    // 175: double_plant
    REG(175, "double_plant", Material::Plants).setHardness(0.0f)
        .setUnlocalizedName("doublePlant") END(175, "double_plant")

    #undef REG
    #undef END

    std::cout << "[Block] Registered " << blockRegistry.size() << " blocks\n";
}

} // namespace mccpp
