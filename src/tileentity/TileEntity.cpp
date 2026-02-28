/**
 * TileEntity.cpp — TileEntity registry implementation.
 *
 * Java reference: net.minecraft.tileentity.TileEntity static initializer
 *
 * All 20 vanilla tile entity types registered with exact string IDs
 * matching Java's addMapping calls.
 */

#include "tileentity/TileEntity.h"
#include <iostream>

namespace mccpp {

std::unordered_map<std::string, TileEntityType> TileEntityRegistry::nameToType_;
std::unordered_map<int32_t, std::string> TileEntityRegistry::typeToName_;
std::vector<std::string> TileEntityRegistry::allTypeIds_;
bool TileEntityRegistry::initialized_ = false;

void TileEntityRegistry::init() {
    if (initialized_) return;

    // Java: TileEntity static initializer — exact string IDs
    // addMapping(TileEntityFurnace.class, "Furnace")
    // addMapping(TileEntityChest.class, "Chest")
    // etc.

    struct Mapping {
        TileEntityType type;
        const char* id;
    };

    static const Mapping mappings[] = {
        {TileEntityType::FURNACE,       "Furnace"},
        {TileEntityType::CHEST,         "Chest"},
        {TileEntityType::ENDER_CHEST,   "EnderChest"},
        {TileEntityType::JUKEBOX,       "RecordPlayer"},
        {TileEntityType::DISPENSER,     "Trap"},
        {TileEntityType::DROPPER,       "Dropper"},
        {TileEntityType::SIGN,          "Sign"},
        {TileEntityType::MOB_SPAWNER,   "MobSpawner"},
        {TileEntityType::NOTE_BLOCK,    "Music"},
        {TileEntityType::PISTON,        "Piston"},
        {TileEntityType::BREWING_STAND, "Cauldron"},
        {TileEntityType::ENCHANT_TABLE, "EnchantTable"},
        {TileEntityType::END_PORTAL,    "Airportal"},
        {TileEntityType::COMMAND_BLOCK, "Control"},
        {TileEntityType::BEACON,        "Beacon"},
        {TileEntityType::SKULL,         "Skull"},
        {TileEntityType::DAYLIGHT_DET,  "DLDetector"},
        {TileEntityType::HOPPER,        "Hopper"},
        {TileEntityType::COMPARATOR,    "Comparator"},
        {TileEntityType::FLOWER_POT,    "FlowerPot"},
    };

    for (const auto& m : mappings) {
        nameToType_[m.id] = m.type;
        typeToName_[static_cast<int32_t>(m.type)] = m.id;
        allTypeIds_.push_back(m.id);
    }

    initialized_ = true;
    std::cout << "[TileEntity] Registered " << allTypeIds_.size() << " tile entity types\n";
}

std::unique_ptr<TileEntity> TileEntityRegistry::create(const std::string& typeId) {
    auto it = nameToType_.find(typeId);
    if (it == nameToType_.end()) {
        std::cerr << "[TileEntity] Unknown type: " << typeId << "\n";
        return nullptr;
    }

    auto te = std::make_unique<TileEntity>();
    te->type = it->second;
    te->typeId = typeId;

    // Set type-specific defaults
    switch (te->type) {
        case TileEntityType::MOB_SPAWNER:
            te->spawnerEntityId = "Pig";
            te->spawnerDelay = 20;
            te->spawnerMinDelay = 200;
            te->spawnerMaxDelay = 800;
            te->spawnerSpawnCount = 4;
            te->spawnerMaxNearby = 6;
            te->spawnerSpawnRange = 4;
            break;
        case TileEntityType::COMMAND_BLOCK:
            te->commandBlockName = "@";
            te->commandBlockTrackOutput = true;
            break;
        case TileEntityType::HOPPER:
            te->hopperTransferCooldown = -1;
            break;
        default:
            break;
    }

    return te;
}

TileEntityType TileEntityRegistry::getType(const std::string& typeId) {
    auto it = nameToType_.find(typeId);
    return it != nameToType_.end() ? it->second : TileEntityType::UNKNOWN;
}

const std::string& TileEntityRegistry::getTypeId(TileEntityType type) {
    static const std::string empty;
    auto it = typeToName_.find(static_cast<int32_t>(type));
    return it != typeToName_.end() ? it->second : empty;
}

const std::vector<std::string>& TileEntityRegistry::getAllTypeIds() {
    return allTypeIds_;
}

int32_t TileEntityRegistry::getCount() {
    return static_cast<int32_t>(allTypeIds_.size());
}

} // namespace mccpp
