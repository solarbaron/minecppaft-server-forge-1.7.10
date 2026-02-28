/**
 * EntityList.cpp — All 57 vanilla entity type registrations.
 *
 * Java reference: net.minecraft.entity.EntityList static initializer
 *
 * Exact string IDs and integer IDs from vanilla 1.7.10.
 * 24 spawn eggs with exact primary/secondary RGB colors.
 */

#include "entity/EntityList.h"
#include <iostream>

namespace mccpp {

std::unordered_map<std::string, int32_t> EntityList::nameToId_;
std::unordered_map<int32_t, std::string> EntityList::idToName_;
std::vector<EntityListEntry> EntityList::entries_;
std::vector<EntityEggInfo> EntityList::eggs_;
std::unordered_map<int32_t, size_t> EntityList::eggIndex_;
bool EntityList::initialized_ = false;

void EntityList::init() {
    if (initialized_) return;

    // All 57 vanilla entity types — exact Java static initializer order
    // Format: {name, id, hasEgg, primaryColor, secondaryColor}
    static const EntityListEntry allEntries[] = {
        // Items / XP
        {"Item",                  1, false, 0, 0},
        {"XPOrb",                 2, false, 0, 0},

        // Misc entities
        {"LeashKnot",             8, false, 0, 0},
        {"Painting",              9, false, 0, 0},

        // Projectiles
        {"Arrow",                10, false, 0, 0},
        {"Snowball",             11, false, 0, 0},
        {"Fireball",             12, false, 0, 0},
        {"SmallFireball",        13, false, 0, 0},
        {"ThrownEnderpearl",     14, false, 0, 0},
        {"EyeOfEnderSignal",     15, false, 0, 0},
        {"ThrownPotion",         16, false, 0, 0},
        {"ThrownExpBottle",      17, false, 0, 0},
        {"ItemFrame",            18, false, 0, 0},
        {"WitherSkull",          19, false, 0, 0},

        // Block entities
        {"PrimedTnt",            20, false, 0, 0},
        {"FallingSand",          21, false, 0, 0},
        {"FireworksRocketEntity",22, false, 0, 0},

        // Vehicles
        {"Boat",                 41, false, 0, 0},
        {"MinecartRideable",     42, false, 0, 0},
        {"MinecartChest",        43, false, 0, 0},
        {"MinecartFurnace",      44, false, 0, 0},
        {"MinecartTNT",          45, false, 0, 0},
        {"MinecartHopper",       46, false, 0, 0},
        {"MinecartSpawner",      47, false, 0, 0},
        {"MinecartCommandBlock", 40, false, 0, 0},

        // Base living types (no egg)
        {"Mob",                  48, false, 0, 0},
        {"Monster",              49, false, 0, 0},

        // Hostile mobs (with eggs)
        {"Creeper",              50, true,  894731,   0},
        {"Skeleton",             51, true,  0xC1C1C1, 0x494949},
        {"Spider",               52, true,  3419431,  11013646},
        {"Giant",                53, false, 0, 0},
        {"Zombie",               54, true,  44975,    7969893},
        {"Slime",                55, true,  5349438,  8306542},
        {"Ghast",                56, true,  0xF9F9F9, 0xBCBCBC},
        {"PigZombie",            57, true,  15373203, 5009705},
        {"Enderman",             58, true,  0x161616, 0},
        {"CaveSpider",           59, true,  803406,   11013646},
        {"Silverfish",           60, true,  0x6E6E6E, 0x303030},
        {"Blaze",                61, true,  16167425, 16775294},
        {"LavaSlime",            62, true,  0x340000, 0xFCFC00},

        // Bosses (no egg)
        {"EnderDragon",          63, false, 0, 0},
        {"WitherBoss",           64, false, 0, 0},

        // Bat + Witch (with eggs)
        {"Bat",                  65, true,  4996656,  986895},
        {"Witch",                66, true,  0x340000, 5349438},

        // Passive mobs (with eggs)
        {"Pig",                  90, true,  15771042, 14377823},
        {"Sheep",                91, true,  0xE7E7E7, 0xFFB5B5},
        {"Cow",                  92, true,  4470310,  0xA1A1A1},
        {"Chicken",              93, true,  0xA1A1A1, 0xFF0000},
        {"Squid",                94, true,  2243405,  7375001},
        {"Wolf",                 95, true,  0xD7D3D3, 13545366},
        {"MushroomCow",          96, true,  10489616, 0xB7B7B7},

        // Utility mobs (no egg)
        {"SnowMan",              97, false, 0, 0},

        // More passive (with eggs)
        {"Ozelot",               98, true,  15720061, 5653556},
        {"VillagerGolem",        99, false, 0, 0},
        {"EntityHorse",         100, true,  12623485, 0xEEE500},
        {"Villager",            120, true,  5651507,  12422002},

        // Ender Crystal
        {"EnderCrystal",        200, false, 0, 0},
    };

    int32_t count = static_cast<int32_t>(sizeof(allEntries) / sizeof(allEntries[0]));

    for (int32_t i = 0; i < count; ++i) {
        const auto& e = allEntries[i];
        entries_.push_back(e);
        nameToId_[e.name] = e.id;
        idToName_[e.id] = e.name;

        if (e.hasEgg) {
            EntityEggInfo egg{e.id, e.eggPrimary, e.eggSecondary};
            eggIndex_[e.id] = eggs_.size();
            eggs_.push_back(egg);
        }
    }

    initialized_ = true;
    std::cout << "[EntityList] Registered " << entries_.size()
              << " entity types, " << eggs_.size() << " spawn eggs\n";
}

int32_t EntityList::getIdByName(const std::string& name) {
    auto it = nameToId_.find(name);
    return it != nameToId_.end() ? it->second : 0;
}

const std::string& EntityList::getNameById(int32_t id) {
    static const std::string empty;
    auto it = idToName_.find(id);
    return it != idToName_.end() ? it->second : empty;
}

const EntityEggInfo* EntityList::getEggInfo(int32_t id) {
    auto it = eggIndex_.find(id);
    return it != eggIndex_.end() ? &eggs_[it->second] : nullptr;
}

const std::vector<EntityListEntry>& EntityList::getAllEntries() {
    return entries_;
}

const std::vector<EntityEggInfo>& EntityList::getAllEggs() {
    return eggs_;
}

int32_t EntityList::getEntityCount() {
    return static_cast<int32_t>(entries_.size());
}

int32_t EntityList::getEggCount() {
    return static_cast<int32_t>(eggs_.size());
}

} // namespace mccpp
