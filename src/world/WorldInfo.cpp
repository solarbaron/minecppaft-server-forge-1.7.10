/**
 * WorldInfo.cpp — World metadata implementation.
 *
 * Java references:
 *   net.minecraft.world.WorldType — 5 types with IDs 0,1,2,3,8
 *   net.minecraft.world.storage.WorldInfo — All world metadata fields
 *
 * WorldType initialization matches Java's static initializer exactly.
 */

#include "world/WorldInfo.h"
#include <algorithm>
#include <iostream>
#include <mutex>

namespace mccpp {

// ═════════════════════════════════════════════════════════════════════════════
// WorldTypes
// ═════════════════════════════════════════════════════════════════════════════

WorldTypeInfo WorldTypes::types_[16] = {};
bool WorldTypes::initialized_ = false;

void WorldTypes::init() {
    if (initialized_) return;

    // Zero all slots
    for (int i = 0; i < 16; ++i) {
        types_[i] = {-1, "", 0, false, false, false};
    }

    // Java: WorldType.DEFAULT = new WorldType(0, "default", 1).setVersioned()
    types_[0] = {0, "default", 1, true, true, false};

    // Java: WorldType.FLAT = new WorldType(1, "flat")
    types_[1] = {1, "flat", 0, true, false, false};

    // Java: WorldType.LARGE_BIOMES = new WorldType(2, "largeBiomes")
    types_[2] = {2, "largeBiomes", 0, true, false, false};

    // Java: WorldType.AMPLIFIED = new WorldType(3, "amplified").setNotificationData()
    types_[3] = {3, "amplified", 0, true, false, true};

    // Java: WorldType.DEFAULT_1_1 = new WorldType(8, "default_1_1", 0).setCanBeCreated(false)
    types_[8] = {8, "default_1_1", 0, false, false, false};

    initialized_ = true;
    std::cout << "[WorldType] Registered 5 world types\n";
}

const WorldTypeInfo* WorldTypes::getById(int32_t id) {
    if (id < 0 || id >= 16 || types_[id].id < 0) return nullptr;
    return &types_[id];
}

const WorldTypeInfo* WorldTypes::getByName(const std::string& name) {
    for (int i = 0; i < 16; ++i) {
        if (types_[i].id >= 0) {
            // Java: equalsIgnoreCase
            std::string a = types_[i].name;
            std::string b = name;
            std::transform(a.begin(), a.end(), a.begin(), ::tolower);
            std::transform(b.begin(), b.end(), b.begin(), ::tolower);
            if (a == b) return &types_[i];
        }
    }
    return nullptr;
}

// ═════════════════════════════════════════════════════════════════════════════
// WorldInfo — All getters/setters with thread safety
// ═════════════════════════════════════════════════════════════════════════════

WorldInfo::WorldInfo() = default;

// Seed
int64_t WorldInfo::getSeed() const { std::shared_lock l(mutex_); return seed_; }
void WorldInfo::setSeed(int64_t s) { std::unique_lock l(mutex_); seed_ = s; }

// Spawn
int32_t WorldInfo::getSpawnX() const { std::shared_lock l(mutex_); return spawnX_; }
int32_t WorldInfo::getSpawnY() const { std::shared_lock l(mutex_); return spawnY_; }
int32_t WorldInfo::getSpawnZ() const { std::shared_lock l(mutex_); return spawnZ_; }
void WorldInfo::setSpawnPosition(int32_t x, int32_t y, int32_t z) {
    std::unique_lock l(mutex_); spawnX_ = x; spawnY_ = y; spawnZ_ = z;
}

// Time
int64_t WorldInfo::getWorldTotalTime() const { std::shared_lock l(mutex_); return totalTime_; }
void WorldInfo::setWorldTotalTime(int64_t t) { std::unique_lock l(mutex_); totalTime_ = t; }
int64_t WorldInfo::getWorldTime() const { std::shared_lock l(mutex_); return worldTime_; }
void WorldInfo::setWorldTime(int64_t t) { std::unique_lock l(mutex_); worldTime_ = t; }

// Weather
bool WorldInfo::isRaining() const { std::shared_lock l(mutex_); return raining_; }
void WorldInfo::setRaining(bool r) { std::unique_lock l(mutex_); raining_ = r; }
int32_t WorldInfo::getRainTime() const { std::shared_lock l(mutex_); return rainTime_; }
void WorldInfo::setRainTime(int32_t t) { std::unique_lock l(mutex_); rainTime_ = t; }
bool WorldInfo::isThundering() const { std::shared_lock l(mutex_); return thundering_; }
void WorldInfo::setThundering(bool t) { std::unique_lock l(mutex_); thundering_ = t; }
int32_t WorldInfo::getThunderTime() const { std::shared_lock l(mutex_); return thunderTime_; }
void WorldInfo::setThunderTime(int32_t t) { std::unique_lock l(mutex_); thunderTime_ = t; }

// Game settings
GameType WorldInfo::getGameType() const { std::shared_lock l(mutex_); return gameType_; }
void WorldInfo::setGameType(GameType g) { std::unique_lock l(mutex_); gameType_ = g; }
Difficulty WorldInfo::getDifficulty() const { std::shared_lock l(mutex_); return difficulty_; }
void WorldInfo::setDifficulty(Difficulty d) { std::unique_lock l(mutex_); difficulty_ = d; }
bool WorldInfo::isHardcore() const { std::shared_lock l(mutex_); return hardcore_; }
void WorldInfo::setHardcore(bool h) { std::unique_lock l(mutex_); hardcore_ = h; }
bool WorldInfo::areCommandsAllowed() const { std::shared_lock l(mutex_); return commandsAllowed_; }
void WorldInfo::setCommandsAllowed(bool c) { std::unique_lock l(mutex_); commandsAllowed_ = c; }
bool WorldInfo::isMapFeaturesEnabled() const { std::shared_lock l(mutex_); return mapFeatures_; }
void WorldInfo::setMapFeaturesEnabled(bool m) { std::unique_lock l(mutex_); mapFeatures_ = m; }

// World type
int32_t WorldInfo::getWorldTypeId() const { std::shared_lock l(mutex_); return worldTypeId_; }
void WorldInfo::setWorldTypeId(int32_t id) { std::unique_lock l(mutex_); worldTypeId_ = id; }
std::string WorldInfo::getGeneratorOptions() const { std::shared_lock l(mutex_); return generatorOptions_; }
void WorldInfo::setGeneratorOptions(const std::string& o) { std::unique_lock l(mutex_); generatorOptions_ = o; }

// World name
std::string WorldInfo::getWorldName() const { std::shared_lock l(mutex_); return worldName_; }
void WorldInfo::setWorldName(const std::string& n) { std::unique_lock l(mutex_); worldName_ = n; }

// Border
double WorldInfo::getBorderCenterX() const { std::shared_lock l(mutex_); return borderCenterX_; }
double WorldInfo::getBorderCenterZ() const { std::shared_lock l(mutex_); return borderCenterZ_; }
void WorldInfo::setBorderCenter(double x, double z) { std::unique_lock l(mutex_); borderCenterX_ = x; borderCenterZ_ = z; }
double WorldInfo::getBorderSize() const { std::shared_lock l(mutex_); return borderSize_; }
void WorldInfo::setBorderSize(double s) { std::unique_lock l(mutex_); borderSize_ = s; }

// Size on disk
int64_t WorldInfo::getSizeOnDisk() const { std::shared_lock l(mutex_); return sizeOnDisk_; }
void WorldInfo::setSizeOnDisk(int64_t s) { std::unique_lock l(mutex_); sizeOnDisk_ = s; }

// Last played
int64_t WorldInfo::getLastTimePlayed() const { std::shared_lock l(mutex_); return lastPlayed_; }
void WorldInfo::setLastTimePlayed(int64_t t) { std::unique_lock l(mutex_); lastPlayed_ = t; }

} // namespace mccpp
