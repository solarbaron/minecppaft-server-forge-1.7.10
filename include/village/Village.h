/**
 * Village.h — Village mechanics: doors, reputation, iron golems, sieges.
 *
 * Java references:
 *   - net.minecraft.village.Village — Village state and tick logic
 *   - net.minecraft.village.VillageDoorInfo — Door tracking
 *   - net.minecraft.village.Village$VillageAgressor — Aggressor tracking
 *   - net.minecraft.village.VillageSiege — Zombie siege event
 *
 * Villages track doors, compute center/radius, manage player
 * reputation, spawn iron golems, track aggressors, and persist
 * to NBT.
 *
 * Not thread-safe: villages tick on the world thread.
 * JNI readiness: Simple structs, NBT-compatible fields.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// VillageDoorInfo — Tracks a single door in the village.
// Java reference: net.minecraft.village.VillageDoorInfo
// ═══════════════════════════════════════════════════════════════════════════

struct VillageDoorInfo {
    int32_t posX, posY, posZ;          // Door block position
    int32_t insideDirectionX;           // Direction to "inside" from door
    int32_t insideDirectionZ;
    int32_t lastActivityTimestamp;      // Last tick this door was validated
    bool isDetachedFromVillageFlag = false;
    int32_t doorOpeningRestrictionCounter = 0;

    VillageDoorInfo() = default;

    VillageDoorInfo(int32_t x, int32_t y, int32_t z, int32_t dx, int32_t dz, int32_t timestamp)
        : posX(x), posY(y), posZ(z),
          insideDirectionX(dx), insideDirectionZ(dz),
          lastActivityTimestamp(timestamp) {}

    // Java: getDistanceSquared
    int32_t getDistanceSquared(int32_t x, int32_t y, int32_t z) const {
        int32_t dx = posX - x;
        int32_t dy = posY - y;
        int32_t dz = posZ - z;
        return dx * dx + dy * dy + dz * dz;
    }

    // Java: getInsidePosX/Z — center of "inside" area
    int32_t getInsidePosX() const { return posX + insideDirectionX; }
    int32_t getInsidePosZ() const { return posZ + insideDirectionZ; }

    // Java: getDoorOpeningRestrictionCounter
    int32_t getDoorOpeningRestrictionCounter() const { return doorOpeningRestrictionCounter; }

    void incrementDoorOpeningRestrictionCounter() { ++doorOpeningRestrictionCounter; }
    void resetDoorOpeningRestrictionCounter() { doorOpeningRestrictionCounter = 0; }
};

// ═══════════════════════════════════════════════════════════════════════════
// VillageAggressor — Tracks an entity that attacked villagers.
// Java reference: net.minecraft.village.Village$VillageAgressor
// ═══════════════════════════════════════════════════════════════════════════

struct VillageAggressor {
    int32_t aggressorEntityId;
    int32_t aggressionTime;

    VillageAggressor(int32_t entityId, int32_t time)
        : aggressorEntityId(entityId), aggressionTime(time) {}
};

// ═══════════════════════════════════════════════════════════════════════════
// Village — Complete village state and mechanics.
// Java reference: net.minecraft.village.Village
// ═══════════════════════════════════════════════════════════════════════════

class Village {
public:
    Village() = default;

    // ─── Door management ───

    // Java: addVillageDoorInfo
    void addDoor(const VillageDoorInfo& door) {
        doors_.push_back(door);
        centerHelperX_ += door.posX;
        centerHelperY_ += door.posY;
        centerHelperZ_ += door.posZ;
        updateRadiusAndCenter();
        lastAddDoorTimestamp_ = door.lastActivityTimestamp;
    }

    // Java: getVillageDoorAt
    VillageDoorInfo* getDoorAt(int32_t x, int32_t y, int32_t z) {
        if (getDistanceSqToCenter(x, y, z) > static_cast<float>(radius_ * radius_)) {
            return nullptr;
        }
        for (auto& door : doors_) {
            if (door.posX == x && door.posZ == z && std::abs(door.posY - y) <= 1) {
                return &door;
            }
        }
        return nullptr;
    }

    // Java: findNearestDoor
    VillageDoorInfo* findNearestDoor(int32_t x, int32_t y, int32_t z) {
        VillageDoorInfo* nearest = nullptr;
        int32_t minDist = INT32_MAX;
        for (auto& door : doors_) {
            int32_t dist = door.getDistanceSquared(x, y, z);
            if (dist < minDist) {
                nearest = &door;
                minDist = dist;
            }
        }
        return nearest;
    }

    // Java: findNearestDoorUnrestricted — prefers less-restricted doors
    VillageDoorInfo* findNearestDoorUnrestricted(int32_t x, int32_t y, int32_t z) {
        VillageDoorInfo* nearest = nullptr;
        int32_t minScore = INT32_MAX;
        for (auto& door : doors_) {
            int32_t dist = door.getDistanceSquared(x, y, z);
            int32_t score = dist > 256 ? (dist * 1000) : door.getDoorOpeningRestrictionCounter();
            if (score < minScore) {
                nearest = &door;
                minScore = score;
            }
        }
        return nearest;
    }

    // ─── Tick ───

    // Java: tick
    struct TickResult {
        bool shouldSpawnGolem = false;
        double golemX = 0.0, golemY = 0.0, golemZ = 0.0;
    };

    TickResult tick(int32_t worldTick, bool randomChance7000, int32_t random50,
                     int32_t randX, int32_t randY, int32_t randZ) {
        TickResult result;
        tickCounter_ = worldTick;

        // Remove invalid doors
        removeDeadDoors(random50 == 0);

        // Remove old aggressors
        removeOldAggressors();

        // Count villagers every 20 ticks (done externally, just track the tick)
        // Count iron golems every 30 ticks (done externally)

        // Iron golem spawning: 1 per 10 villagers, >20 doors, 1/7000 chance
        int32_t desiredGolems = numVillagers_ / 10;
        if (numIronGolems_ < desiredGolems &&
            static_cast<int32_t>(doors_.size()) > 20 &&
            randomChance7000) {

            // Try to find spawn location (simplified — caller provides random offsets)
            int32_t spawnX = centerX_ + randX;
            int32_t spawnY = centerY_ + randY;
            int32_t spawnZ = centerZ_ + randZ;

            if (isInRange(spawnX, spawnY, spawnZ)) {
                result.shouldSpawnGolem = true;
                result.golemX = static_cast<double>(spawnX);
                result.golemY = static_cast<double>(spawnY);
                result.golemZ = static_cast<double>(spawnZ);
                ++numIronGolems_;
            }
        }
        return result;
    }

    // ─── Position queries ───

    int32_t getCenterX() const { return centerX_; }
    int32_t getCenterY() const { return centerY_; }
    int32_t getCenterZ() const { return centerZ_; }
    int32_t getRadius() const { return radius_; }

    bool isInRange(int32_t x, int32_t y, int32_t z) const {
        return getDistanceSqToCenter(x, y, z) < static_cast<float>(radius_ * radius_);
    }

    float getDistanceSqToCenter(int32_t x, int32_t y, int32_t z) const {
        float dx = static_cast<float>(x - centerX_);
        float dy = static_cast<float>(y - centerY_);
        float dz = static_cast<float>(z - centerZ_);
        return dx * dx + dy * dy + dz * dz;
    }

    // ─── Village status ───

    int32_t getNumDoors() const { return static_cast<int32_t>(doors_.size()); }
    int32_t getNumVillagers() const { return numVillagers_; }
    void setNumVillagers(int32_t n) { numVillagers_ = n; }
    int32_t getNumIronGolems() const { return numIronGolems_; }
    void setNumIronGolems(int32_t n) { numIronGolems_ = n; }
    bool isAnnihilated() const { return doors_.empty(); }

    int32_t getTicksSinceLastDoorAdding() const {
        return tickCounter_ - lastAddDoorTimestamp_;
    }

    // ─── Mating season ───

    // Java: isMatingSeason — 3600 tick cooldown
    bool isMatingSeason() const {
        return noBreedTicks_ == 0 || tickCounter_ - noBreedTicks_ >= 3600;
    }

    // Java: endMatingSeason
    void endMatingSeason() {
        noBreedTicks_ = tickCounter_;
    }

    // ─── Player reputation ───

    // Java: getReputationForPlayer
    int32_t getReputation(const std::string& player) const {
        auto it = playerReputation_.find(player);
        return it != playerReputation_.end() ? it->second : 0;
    }

    // Java: setReputationForPlayer — delta, clamped to [-30, 10]
    int32_t modifyReputation(const std::string& player, int32_t delta) {
        int32_t current = getReputation(player);
        int32_t newRep = std::clamp(current + delta, -30, 10);
        playerReputation_[player] = newRep;
        return newRep;
    }

    // Java: isPlayerReputationTooLow — threshold: -15
    bool isPlayerReputationTooLow(const std::string& player) const {
        return getReputation(player) <= -15;
    }

    // Java: setDefaultPlayerReputation
    void setAllReputations(int32_t delta) {
        for (auto& [name, rep] : playerReputation_) {
            modifyReputation(name, delta);
        }
    }

    void clearReputations() { playerReputation_.clear(); }

    // ─── Aggressor tracking ───

    // Java: addOrRenewAgressor
    void addOrRenewAggressor(int32_t entityId) {
        for (auto& agg : aggressors_) {
            if (agg.aggressorEntityId == entityId) {
                agg.aggressionTime = tickCounter_;
                return;
            }
        }
        aggressors_.emplace_back(entityId, tickCounter_);
    }

    const std::vector<VillageAggressor>& getAggressors() const { return aggressors_; }

    // ─── NBT serialization ───

    struct NBTData {
        int32_t popSize, radius, golems, stable, tick, mTick;
        int32_t cx, cy, cz, acx, acy, acz;
        std::vector<VillageDoorInfo> doors;
        std::map<std::string, int32_t> players;
    };

    NBTData toNBT() const {
        NBTData data;
        data.popSize = numVillagers_;
        data.radius = radius_;
        data.golems = numIronGolems_;
        data.stable = lastAddDoorTimestamp_;
        data.tick = tickCounter_;
        data.mTick = noBreedTicks_;
        data.cx = centerX_;
        data.cy = centerY_;
        data.cz = centerZ_;
        data.acx = centerHelperX_;
        data.acy = centerHelperY_;
        data.acz = centerHelperZ_;
        data.doors = doors_;
        data.players = playerReputation_;
        return data;
    }

    void fromNBT(const NBTData& data) {
        numVillagers_ = data.popSize;
        radius_ = data.radius;
        numIronGolems_ = data.golems;
        lastAddDoorTimestamp_ = data.stable;
        tickCounter_ = data.tick;
        noBreedTicks_ = data.mTick;
        centerX_ = data.cx;
        centerY_ = data.cy;
        centerZ_ = data.cz;
        centerHelperX_ = data.acx;
        centerHelperY_ = data.acy;
        centerHelperZ_ = data.acz;
        doors_ = data.doors;
        playerReputation_ = data.players;
    }

    // ─── Access to doors for iteration ───
    const std::vector<VillageDoorInfo>& getDoors() const { return doors_; }

private:
    // Java: removeDeadAndOutOfRangeDoors
    void removeDeadDoors(bool resetCounters) {
        bool changed = false;
        for (auto it = doors_.begin(); it != doors_.end(); ) {
            if (resetCounters) {
                it->resetDoorOpeningRestrictionCounter();
            }
            // Doors that haven't been validated in 1200 ticks are removed
            // (actual door block check is done externally)
            if (std::abs(tickCounter_ - it->lastActivityTimestamp) > 1200) {
                centerHelperX_ -= it->posX;
                centerHelperY_ -= it->posY;
                centerHelperZ_ -= it->posZ;
                it->isDetachedFromVillageFlag = true;
                it = doors_.erase(it);
                changed = true;
            } else {
                ++it;
            }
        }
        if (changed) updateRadiusAndCenter();
    }

    // Java: removeDeadAndOldAgressors
    void removeOldAggressors() {
        aggressors_.erase(
            std::remove_if(aggressors_.begin(), aggressors_.end(),
                [this](const VillageAggressor& agg) {
                    return std::abs(tickCounter_ - agg.aggressionTime) > 300;
                }),
            aggressors_.end());
    }

    // Java: updateVillageRadiusAndCenter
    void updateRadiusAndCenter() {
        int32_t n = static_cast<int32_t>(doors_.size());
        if (n == 0) {
            centerX_ = centerY_ = centerZ_ = 0;
            radius_ = 0;
            return;
        }
        centerX_ = centerHelperX_ / n;
        centerY_ = centerHelperY_ / n;
        centerZ_ = centerHelperZ_ / n;

        int32_t maxDistSq = 0;
        for (const auto& door : doors_) {
            int32_t d = door.getDistanceSquared(centerX_, centerY_, centerZ_);
            maxDistSq = std::max(maxDistSq, d);
        }
        radius_ = std::max(32, static_cast<int32_t>(std::sqrt(static_cast<double>(maxDistSq))) + 1);
    }

    // ─── State ───
    std::vector<VillageDoorInfo> doors_;
    std::vector<VillageAggressor> aggressors_;
    std::map<std::string, int32_t> playerReputation_;

    int32_t centerX_ = 0, centerY_ = 0, centerZ_ = 0;
    int32_t centerHelperX_ = 0, centerHelperY_ = 0, centerHelperZ_ = 0;
    int32_t radius_ = 0;
    int32_t lastAddDoorTimestamp_ = 0;
    int32_t tickCounter_ = 0;
    int32_t numVillagers_ = 0;
    int32_t numIronGolems_ = 0;
    int32_t noBreedTicks_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// VillageSiege — Zombie siege event during night.
// Java reference: net.minecraft.village.VillageSiege
//
// Checks every tick during night if any player is near a village with
// >=10 doors and >=20 villagers. If so, spawns ~20 zombies near the
// village border over several ticks.
// ═══════════════════════════════════════════════════════════════════════════

class VillageSiege {
public:
    VillageSiege() = default;

    // Java: tick — called every world tick
    struct SiegeSpawnResult {
        bool shouldSpawnZombie = false;
        double spawnX = 0.0, spawnY = 0.0, spawnZ = 0.0;
    };

    SiegeSpawnResult tick(int64_t worldTime, bool hasNearbyVillage,
                           int32_t villageDoorCount, int32_t villagerCount,
                           double villageX, double villageY, double villageZ,
                           int32_t villageRadius, int32_t randAngle, int32_t randDist) {
        SiegeSpawnResult result;

        // Java: Only active at night (13000-22000)
        long timeOfDay = worldTime % 24000L;
        bool isNight = timeOfDay >= 13000 && timeOfDay < 22000;

        if (!isNight) {
            siegeState_ = 0;
            return result;
        }

        switch (siegeState_) {
            case 0: // Looking for village to siege
                if (!hasNearbyVillage) return result;
                if (villageDoorCount < 10 || villagerCount < 20) return result;
                // Start siege
                siegeState_ = 1;
                nextSpawnTime_ = 0;
                spawnCount_ = 0;
                siegeCenterX_ = villageX;
                siegeCenterY_ = villageY;
                siegeCenterZ_ = villageZ;
                break;

            case 1: // Spawning zombies
                if (spawnCount_ >= 20) {
                    siegeState_ = 0; // Siege complete
                    return result;
                }
                if (nextSpawnTime_ > 0) {
                    --nextSpawnTime_;
                    return result;
                }
                nextSpawnTime_ = 2; // Spawn every 2 ticks

                // Compute spawn position on village border
                {
                    double angle = static_cast<double>(randAngle) * 3.14159265358979 / 180.0;
                    double dist = static_cast<double>(villageRadius) * 0.9;
                    result.spawnX = siegeCenterX_ + std::cos(angle) * dist;
                    result.spawnZ = siegeCenterZ_ + std::sin(angle) * dist;
                    result.spawnY = siegeCenterY_;
                    result.shouldSpawnZombie = true;
                    ++spawnCount_;
                }
                break;
        }
        return result;
    }

    bool isSiegeActive() const { return siegeState_ == 1; }
    int32_t getSpawnCount() const { return spawnCount_; }

private:
    int32_t siegeState_ = 0;       // 0=inactive, 1=spawning
    int32_t nextSpawnTime_ = 0;
    int32_t spawnCount_ = 0;
    double siegeCenterX_ = 0.0, siegeCenterY_ = 0.0, siegeCenterZ_ = 0.0;
};

} // namespace mccpp
