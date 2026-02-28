/**
 * Material.cpp — Material and MapColor registry implementation.
 *
 * Java reference: net.minecraft.block.material.Material static initializer
 *
 * All 34 vanilla materials with exact property flags from Java source.
 */

#include "material/Material.h"
#include <iostream>

namespace mccpp {

std::vector<Material> MaterialRegistry::materials_;
bool MaterialRegistry::initialized_ = false;

void MaterialRegistry::init() {
    if (initialized_) return;

    // Format: {name, type, mapColor, canBurn, replaceable, translucent, requiresNoTool, mobility, adventureExempt}
    materials_.clear();
    materials_.reserve(34);

    // Java: Material.air = new MaterialTransparent(MapColor.airColor)
    materials_.push_back({AIR, MaterialType::TRANSPARENT, MapColors::AIR, false, false, false, true, 0, false});

    // Java: Material.grass = new Material(MapColor.grassColor)
    materials_.push_back({GRASS, MaterialType::SOLID, MapColors::GRASS, false, false, false, true, 0, false});

    // Java: Material.ground = new Material(MapColor.dirtColor)
    materials_.push_back({GROUND, MaterialType::SOLID, MapColors::DIRT, false, false, false, true, 0, false});

    // Java: Material.wood = new Material(MapColor.woodColor).setBurning()
    materials_.push_back({WOOD, MaterialType::SOLID, MapColors::WOOD, true, false, false, true, 0, false});

    // Java: Material.rock = new Material(MapColor.stoneColor).setRequiresTool()
    materials_.push_back({ROCK, MaterialType::SOLID, MapColors::STONE, false, false, false, false, 0, false});

    // Java: Material.iron = new Material(MapColor.ironColor).setRequiresTool()
    materials_.push_back({IRON, MaterialType::SOLID, MapColors::IRON, false, false, false, false, 0, false});

    // Java: Material.anvil = new Material(MapColor.ironColor).setRequiresTool().setImmovableMobility()
    materials_.push_back({ANVIL, MaterialType::SOLID, MapColors::IRON, false, false, false, false, 2, false});

    // Java: Material.water = new MaterialLiquid(MapColor.waterColor).setNoPushMobility()
    materials_.push_back({WATER, MaterialType::LIQUID, MapColors::WATER, false, false, false, true, 1, false});

    // Java: Material.lava = new MaterialLiquid(MapColor.tntColor).setNoPushMobility()
    materials_.push_back({LAVA, MaterialType::LIQUID, MapColors::TNT, false, false, false, true, 1, false});

    // Java: Material.leaves = new Material(MapColor.foliageColor).setBurning().setTranslucent().setNoPushMobility()
    materials_.push_back({LEAVES, MaterialType::SOLID, MapColors::FOLIAGE, true, false, true, true, 1, false});

    // Java: Material.plants = new MaterialLogic(MapColor.foliageColor).setNoPushMobility()
    materials_.push_back({PLANTS, MaterialType::LOGIC, MapColors::FOLIAGE, false, false, false, true, 1, false});

    // Java: Material.vine = new MaterialLogic(MapColor.foliageColor).setBurning().setNoPushMobility().setReplaceable()
    materials_.push_back({VINE, MaterialType::LOGIC, MapColors::FOLIAGE, true, true, false, true, 1, false});

    // Java: Material.sponge = new Material(MapColor.clothColor)
    materials_.push_back({SPONGE, MaterialType::SOLID, MapColors::CLOTH, false, false, false, true, 0, false});

    // Java: Material.cloth = new Material(MapColor.clothColor).setBurning()
    materials_.push_back({CLOTH, MaterialType::SOLID, MapColors::CLOTH, true, false, false, true, 0, false});

    // Java: Material.fire = new MaterialTransparent(MapColor.airColor).setNoPushMobility()
    materials_.push_back({FIRE, MaterialType::TRANSPARENT, MapColors::AIR, false, false, false, true, 1, false});

    // Java: Material.sand = new Material(MapColor.sandColor)
    materials_.push_back({SAND, MaterialType::SOLID, MapColors::SAND, false, false, false, true, 0, false});

    // Java: Material.circuits = new MaterialLogic(MapColor.airColor).setNoPushMobility()
    materials_.push_back({CIRCUITS, MaterialType::LOGIC, MapColors::AIR, false, false, false, true, 1, false});

    // Java: Material.carpet = new MaterialLogic(MapColor.clothColor).setBurning()
    materials_.push_back({CARPET, MaterialType::LOGIC, MapColors::CLOTH, true, false, false, true, 0, false});

    // Java: Material.glass = new Material(MapColor.airColor).setTranslucent().setAdventureModeExempt()
    materials_.push_back({GLASS, MaterialType::SOLID, MapColors::AIR, false, false, true, true, 0, true});

    // Java: Material.redstoneLight = new Material(MapColor.airColor).setAdventureModeExempt()
    materials_.push_back({REDSTONE_LIGHT, MaterialType::SOLID, MapColors::AIR, false, false, false, true, 0, true});

    // Java: Material.tnt = new Material(MapColor.tntColor).setBurning().setTranslucent()
    materials_.push_back({TNT, MaterialType::SOLID, MapColors::TNT, true, false, true, true, 0, false});

    // Java: Material.coral = new Material(MapColor.foliageColor).setNoPushMobility()
    materials_.push_back({CORAL, MaterialType::SOLID, MapColors::FOLIAGE, false, false, false, true, 1, false});

    // Java: Material.ice = new Material(MapColor.iceColor).setTranslucent().setAdventureModeExempt()
    materials_.push_back({ICE, MaterialType::SOLID, MapColors::ICE, false, false, true, true, 0, true});

    // Java: Material.packedIce = new Material(MapColor.iceColor).setAdventureModeExempt()
    materials_.push_back({PACKED_ICE, MaterialType::SOLID, MapColors::ICE, false, false, false, true, 0, true});

    // Java: Material.snow = new MaterialLogic(MapColor.snowColor).setReplaceable().setTranslucent().setRequiresTool().setNoPushMobility()
    materials_.push_back({SNOW, MaterialType::LOGIC, MapColors::SNOW, false, true, true, false, 1, false});

    // Java: Material.craftedSnow = new Material(MapColor.snowColor).setRequiresTool()
    materials_.push_back({CRAFTED_SNOW, MaterialType::SOLID, MapColors::SNOW, false, false, false, false, 0, false});

    // Java: Material.cactus = new Material(MapColor.foliageColor).setTranslucent().setNoPushMobility()
    materials_.push_back({CACTUS, MaterialType::SOLID, MapColors::FOLIAGE, false, false, true, true, 1, false});

    // Java: Material.clay = new Material(MapColor.clayColor)
    materials_.push_back({CLAY, MaterialType::SOLID, MapColors::CLAY, false, false, false, true, 0, false});

    // Java: Material.gourd = new Material(MapColor.foliageColor).setNoPushMobility()
    materials_.push_back({GOURD, MaterialType::SOLID, MapColors::FOLIAGE, false, false, false, true, 1, false});

    // Java: Material.dragonEgg = new Material(MapColor.foliageColor).setNoPushMobility()
    materials_.push_back({DRAGON_EGG, MaterialType::SOLID, MapColors::FOLIAGE, false, false, false, true, 1, false});

    // Java: Material.portal = new MaterialPortal(MapColor.airColor).setImmovableMobility()
    materials_.push_back({PORTAL, MaterialType::PORTAL, MapColors::AIR, false, false, false, true, 2, false});

    // Java: Material.cake = new Material(MapColor.airColor).setNoPushMobility()
    materials_.push_back({CAKE, MaterialType::SOLID, MapColors::AIR, false, false, false, true, 1, false});

    // Java: Material.web = new Material$1(MapColor.clothColor).setRequiresTool().setNoPushMobility()
    materials_.push_back({WEB, MaterialType::SOLID, MapColors::CLOTH, false, false, false, false, 1, false});

    // Java: Material.piston = new Material(MapColor.stoneColor).setImmovableMobility()
    materials_.push_back({PISTON, MaterialType::SOLID, MapColors::STONE, false, false, false, true, 2, false});

    initialized_ = true;
    std::cout << "[Material] Registered " << materials_.size() << " materials\n";
}

const Material* MaterialRegistry::getByName(const std::string& name) {
    for (const auto& mat : materials_) {
        if (mat.name == name) return &mat;
    }
    return nullptr;
}

const std::vector<Material>& MaterialRegistry::getAll() {
    return materials_;
}

int32_t MaterialRegistry::getCount() {
    return static_cast<int32_t>(materials_.size());
}

} // namespace mccpp
