/**
 * VillageManager.h — Village detection, iron golem spawning, and reputation.
 *
 * Java references:
 *   - net.minecraft.village.VillageCollection (223 lines)
 *   - net.minecraft.village.Village (370 lines)
 *   - net.minecraft.village.VillageDoorInfo (door data)
 *   - net.minecraft.village.Village$VillageAgressor (aggressor tracking)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * VILLAGE COLLECTION (VillageCollection)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * World-level manager for all villages. Stored as WorldSavedData "villages".
 *
 * Tick cycle:
 *   1. Tick each village
 *   2. Remove annihilated villages (no doors left)
 *   3. Pop oldest villager position from queue
 *   4. Scan 16×4×16 around position for wooden doors
 *   5. For each door: check if already tracked, or add to newDoors list
 *   6. Assign newDoors to nearest village (within 32+radius) or create new
 *   7. Every 400 ticks: markDirty (trigger save)
 *
 * Villager position queue: max 64 entries
 * findNearestVillage: distance² < range² considering village radius
 *
 * Door interior detection (addDoorToNewListIfAppropriate):
 *   - Door facing (dir 0,2 = X-axis, dir 1,3 = Z-axis)
 *   - Count sky-visible blocks in -5..0 and 1..5 along axis
 *   - If imbalance: inside points toward less sky (-2 or +2 offset)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * VILLAGE (Village)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Center = average of all door positions (incremental via centerHelper)
 * Radius = max(32, sqrt(maxDoorDistSquared) + 1)
 *
 * Tick(n):
 *   - removeDeadAndOutOfRangeDoors (every tick)
 *     - 1/50 chance: reset door opening counters
 *     - Remove doors that: aren't wooden_door blocks OR timestamp > 1200 old
 *   - removeDeadAndOldAgressors (> 300 ticks old or dead)
 *   - Every 20 ticks: count villagers in AABB (radius × 4 Y)
 *   - Every 30 ticks: count iron golems in same AABB
 *   - Iron golem spawn check:
 *     - Need: numIronGolems < numVillagers/10
 *     - Need: 20+ doors
 *     - Chance: random(7000) == 0
 *     - 10 attempts: random(16)-8 XZ, random(6)-3 Y from center
 *     - Validation: solid below, 2×4×2 clear space (no normalCube)
 *
 * Player reputation:
 *   - Range: -30 to +10 (clamped via MathHelper.clamp_int)
 *   - "Too low" threshold: <= -15 (iron golems become hostile)
 *   - Cleared when numVillagers reaches 0
 *
 * Breeding:
 *   - isMatingSeason: noBreedTicks == 0 || tickCounter - noBreedTicks >= 3600
 *   - endMatingSeason: noBreedTicks = tickCounter
 *
 * Door nearest search:
 *   - findNearestDoor: simple distance²
 *   - findNearestDoorUnrestricted: distance > 256 → multiply by 1000,
 *     else use getDoorOpeningRestrictionCounter
 *
 * Aggressor system:
 *   - addOrRenewAgressor: add or update timestamp
 *   - findNearestVillageAggressor: nearest aggressor to entity
 *   - Cleanup: remove dead or > 300 ticks old
 *
 * NBT format:
 *   Village: PopSize, Radius, Golems, Stable, Tick, MTick,
 *            CX/CY/CZ (center), ACX/ACY/ACZ (centerHelper),
 *            Doors list [X,Y,Z, IDX,IDZ, TS],
 *            Players list [Name, S(score)]
 *   Collection: Tick, Villages list
 *
 * Thread safety: Village ticking happens on the server thread.
 * JNI readiness: Simple POD structs, string-keyed reputation map.
 */
#pragma once

#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <map>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Village Door Info
// ═══════════════════════════════════════════════════════════════════════════

struct VillageDoorInfo {
    int32_t posX = 0, posY = 0, posZ = 0;
    // Inside direction offset (determines which side of door is "inside")
    // Java: insideDirectionX, insideDirectionZ — typically -2 or +2
    int32_t insideDirectionX = 0;
    int32_t insideDirectionZ = 0;
    // Last tick this door was validated as existing
    int32_t lastActivityTimestamp = 0;
    // Has this door been removed from village
    bool isDetachedFromVillageFlag = false;
    // Door opening restriction counter (for AI pathfinding)
    int32_t doorOpeningRestrictionCounter = 0;

    VillageDoorInfo() = default;
    VillageDoorInfo(int32_t x, int32_t y, int32_t z,
                    int32_t idx, int32_t idz, int32_t ts)
        : posX(x), posY(y), posZ(z),
          insideDirectionX(idx), insideDirectionZ(idz),
          lastActivityTimestamp(ts) {}

    int32_t getDistanceSquared(int32_t x, int32_t y, int32_t z) const {
        int32_t dx = posX - x, dy = posY - y, dz = posZ - z;
        return dx*dx + dy*dy + dz*dz;
    }

    int32_t getInsideX() const { return posX + insideDirectionX; }
    int32_t getInsideZ() const { return posZ + insideDirectionZ; }

    int32_t getDoorOpeningRestrictionCounter() const {
        return doorOpeningRestrictionCounter;
    }

    void resetDoorOpeningRestrictionCounter() {
        doorOpeningRestrictionCounter = 0;
    }

    void incrementRestrictionCounter() {
        ++doorOpeningRestrictionCounter;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Village Aggressor
// ═══════════════════════════════════════════════════════════════════════════

struct VillageAggressor {
    int32_t aggressorEntityId = -1;
    int32_t aggressionTime = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// Village Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace VillageConstants {
    // ─── Village radius ───
    // Java: villageRadius = max(32, sqrt(maxDist) + 1)
    static constexpr int32_t MIN_VILLAGE_RADIUS = 32;

    // ─── Iron golem spawning ───
    // Java: numIronGolems < numVillagers / 10
    static constexpr int32_t VILLAGERS_PER_GOLEM = 10;
    // Java: villageDoorInfoList.size() > 20
    static constexpr int32_t MIN_DOORS_FOR_GOLEM = 20;
    // Java: random.nextInt(7000) == 0
    static constexpr int32_t GOLEM_SPAWN_CHANCE = 7000;
    // Java: 10 attempts
    static constexpr int32_t GOLEM_SPAWN_ATTEMPTS = 10;
    // Java: random(16)-8 XZ, random(6)-3 Y
    static constexpr int32_t GOLEM_XZ_RANGE = 16;
    static constexpr int32_t GOLEM_XZ_OFFSET = 8;
    static constexpr int32_t GOLEM_Y_RANGE = 6;
    static constexpr int32_t GOLEM_Y_OFFSET = 3;
    // Java: isValidIronGolemSpawningLocation — spawn volume 2×4×2
    static constexpr int32_t GOLEM_WIDTH = 2;
    static constexpr int32_t GOLEM_HEIGHT = 4;
    static constexpr int32_t GOLEM_DEPTH = 2;

    // ─── Population update intervals ───
    // Java: if (n % 20 == 0) updateNumVillagers
    static constexpr int32_t VILLAGER_COUNT_INTERVAL = 20;
    // Java: if (n % 30 == 0) updateNumIronGolems
    static constexpr int32_t GOLEM_COUNT_INTERVAL = 30;
    // Java: AABB Y range ±4 from center
    static constexpr int32_t COUNT_Y_RANGE = 4;

    // ─── Door maintenance ───
    // Java: 1/50 chance reset door counters
    static constexpr int32_t DOOR_RESET_CHANCE = 50;
    // Java: abs(tickCounter - lastActivityTimestamp) <= 1200
    static constexpr int32_t DOOR_EXPIRE_TICKS = 1200;

    // ─── Aggressor memory ───
    // Java: abs(tickCounter - agressionTime) <= 300
    static constexpr int32_t AGGRESSOR_MEMORY_TICKS = 300;

    // ─── Player reputation ───
    // Java: MathHelper.clamp_int(rep + change, -30, 10)
    static constexpr int32_t MIN_REPUTATION = -30;
    static constexpr int32_t MAX_REPUTATION = 10;
    // Java: isPlayerReputationTooLow → <= -15
    static constexpr int32_t HOSTILE_THRESHOLD = -15;

    // ─── Breeding ───
    // Java: isMatingSeason → noBreedTicks == 0 || tick - noBreed >= 3600
    static constexpr int32_t BREEDING_COOLDOWN = 3600;

    // ─── Door nearest search ───
    // Java: findNearestDoorUnrestricted — if distance > 256, multiply by 1000
    static constexpr int32_t DOOR_DISTANCE_THRESHOLD = 256;
    static constexpr int32_t DOOR_DISTANCE_MULTIPLIER = 1000;

    // ─── Reputation clamping ───
    inline int32_t clampReputation(int32_t value) {
        if (value < MIN_REPUTATION) return MIN_REPUTATION;
        if (value > MAX_REPUTATION) return MAX_REPUTATION;
        return value;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Village Collection Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace VillageCollectionConstants {
    // ─── Data name ───
    static constexpr const char* SAVE_NAME = "villages";

    // ─── Villager position queue ───
    // Java: if (villagerPositionsList.size() > 64) return
    static constexpr int32_t MAX_VILLAGER_POSITIONS = 64;

    // ─── Door scan radius ───
    // Java: for (i = x-16; i < x+16; ...) for (j = y-4; j < y+4; ...)
    static constexpr int32_t SCAN_RADIUS_XZ = 16;
    static constexpr int32_t SCAN_RADIUS_Y = 4;

    // ─── Door-to-village assignment distance ───
    // Java: n = 32 + village.getVillageRadius(); if dist² > n*n → skip
    static constexpr int32_t VILLAGE_MERGE_BASE = 32;

    // ─── Save interval ───
    // Java: if (tickCounter % 400 == 0) markDirty
    static constexpr int32_t SAVE_INTERVAL = 400;

    // ─── Sky access counting ───
    // Java: for (n5 = -5..0) and (n5 = 1..5) canBlockSeeTheSky
    static constexpr int32_t SKY_CHECK_RANGE = 5;
    // Java: insideDirection = n6 > 0 ? -2 : 2
    static constexpr int32_t INSIDE_OFFSET_POSITIVE = -2;
    static constexpr int32_t INSIDE_OFFSET_NEGATIVE = 2;
}

// ═══════════════════════════════════════════════════════════════════════════
// Village NBT Tags
// ═══════════════════════════════════════════════════════════════════════════

namespace VillageNBT {
    // ─── Collection level ───
    static constexpr const char* TAG_TICK = "Tick";
    static constexpr const char* TAG_VILLAGES = "Villages";

    // ─── Per-village ───
    static constexpr const char* TAG_POP_SIZE = "PopSize";
    static constexpr const char* TAG_RADIUS = "Radius";
    static constexpr const char* TAG_GOLEMS = "Golems";
    static constexpr const char* TAG_STABLE = "Stable";
    static constexpr const char* TAG_MTICK = "MTick";
    static constexpr const char* TAG_CX = "CX";
    static constexpr const char* TAG_CY = "CY";
    static constexpr const char* TAG_CZ = "CZ";
    static constexpr const char* TAG_ACX = "ACX";
    static constexpr const char* TAG_ACY = "ACY";
    static constexpr const char* TAG_ACZ = "ACZ";
    static constexpr const char* TAG_DOORS = "Doors";
    static constexpr const char* TAG_PLAYERS = "Players";

    // ─── Per-door ───
    static constexpr const char* TAG_X = "X";
    static constexpr const char* TAG_Y = "Y";
    static constexpr const char* TAG_Z = "Z";
    static constexpr const char* TAG_IDX = "IDX";
    static constexpr const char* TAG_IDZ = "IDZ";
    static constexpr const char* TAG_TS = "TS";

    // ─── Per-player reputation ───
    static constexpr const char* TAG_NAME = "Name";
    static constexpr const char* TAG_SCORE = "S";
}

// ═══════════════════════════════════════════════════════════════════════════
// Village helper: compute radius from door list
// ═══════════════════════════════════════════════════════════════════════════

inline int32_t computeVillageRadius(const std::vector<VillageDoorInfo>& doors,
                                     int32_t centerX, int32_t centerY, int32_t centerZ) {
    if (doors.empty()) return 0;

    int32_t maxDist = 0;
    for (const auto& door : doors) {
        int32_t dist = door.getDistanceSquared(centerX, centerY, centerZ);
        if (dist > maxDist) maxDist = dist;
    }
    int32_t radius = static_cast<int32_t>(std::sqrt(static_cast<double>(maxDist))) + 1;
    return std::max(VillageConstants::MIN_VILLAGE_RADIUS, radius);
}

// ═══════════════════════════════════════════════════════════════════════════
// Village helper: compute center from door positions
// Uses the incremental centerHelper approach from Java
// ═══════════════════════════════════════════════════════════════════════════

struct VillageCenter {
    int32_t x = 0, y = 0, z = 0;

    void set(int32_t cx, int32_t cy, int32_t cz) {
        x = cx; y = cy; z = cz;
    }

    float getDistanceSquared(int32_t ox, int32_t oy, int32_t oz) const {
        int32_t dx = x - ox, dy = y - oy, dz = z - oz;
        return static_cast<float>(dx*dx + dy*dy + dz*dz);
    }
};

inline VillageCenter computeVillageCenter(int32_t sumX, int32_t sumY, int32_t sumZ,
                                           int32_t numDoors) {
    VillageCenter center;
    if (numDoors > 0) {
        center.x = sumX / numDoors;
        center.y = sumY / numDoors;
        center.z = sumZ / numDoors;
    }
    return center;
}

} // namespace mccpp
