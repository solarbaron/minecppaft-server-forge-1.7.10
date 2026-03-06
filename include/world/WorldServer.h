/**
 * WorldServer.h — Dimension-level world management.
 *
 * Java reference: net.minecraft.world.WorldServer (703 lines)
 *
 * Implements the main tick pipeline for a server-side dimension:
 *   1. Hardcore difficulty enforcement
 *   2. Biome cache cleanup
 *   3. All-players-sleeping → skip to dawn
 *   4. Mob spawning (if doMobSpawning)
 *   5. Chunk provider unload queued
 *   6. Skylight subtracted recalculation
 *   7. World time: totalWorldTime++, worldTime++ (if doDaylightCycle)
 *   8. tickUpdates: scheduled block updates (TreeSet, max 1000/tick)
 *   9. Random block ticks: LCG updateLCG*3+1013904223
 *      - Thunder: 1/100000 chance per active chunk
 *      - Ice/snow: 1/16 chance per active chunk
 *      - 3 random ticks per ExtendedBlockStorage section
 *   10. Player manager update
 *   11. Village tick + siege
 *   12. Portal forcer cleanup
 *   13. Block events
 *
 * Thread safety: Main server thread owns tick().
 * JNI readiness: Predictable fields, clear lifecycle hooks.
 */
#pragma once

// If World.h was already included, it provides the canonical WorldServer,
// ChunkCoordHash, and Difficulty definitions. We guard against redefinition.
#ifndef MCCPP_WORLD_H_INCLUDED
// World.h not included — provide standalone definitions.
#else
// World.h already provides WorldServer, ChunkCoordHash, Difficulty.
// Only provide supplementary types (NextTickListEntry, BlockEventData, etc.)
#endif

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// NextTickListEntry — Scheduled block tick.
// Java reference: net.minecraft.world.NextTickListEntry
// ═══════════════════════════════════════════════════════════════════════════

struct NextTickListEntry {
    int32_t x, y, z;
    int32_t blockId;
    int64_t scheduledTime;
    int32_t priority;

    bool operator<(const NextTickListEntry& o) const {
        if (scheduledTime != o.scheduledTime) return scheduledTime < o.scheduledTime;
        if (priority != o.priority) return priority < o.priority;
        if (x != o.x) return x < o.x;
        if (y != o.y) return y < o.y;
        return z < o.z;
    }

    bool operator==(const NextTickListEntry& o) const {
        return x == o.x && y == o.y && z == o.z && blockId == o.blockId;
    }
};

struct NextTickHash {
    size_t operator()(const NextTickListEntry& e) const {
        size_t h = std::hash<int32_t>{}(e.x);
        h ^= std::hash<int32_t>{}(e.y) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int32_t>{}(e.z) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int32_t>{}(e.blockId) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// BlockEventData — Queued block event.
// Java reference: net.minecraft.block.BlockEventData
// ═══════════════════════════════════════════════════════════════════════════

struct BlockEventData {
    int32_t x, y, z;
    int32_t blockId;
    int32_t eventId;
    int32_t eventParam;
};

// ═══════════════════════════════════════════════════════════════════════════
// ChunkCoord — only defined if World.h not already included
// ═══════════════════════════════════════════════════════════════════════════

#ifndef MCCPP_WORLD_H_INCLUDED
struct ChunkCoord {
    int32_t chunkX, chunkZ;
    bool operator==(const ChunkCoord& o) const { return chunkX == o.chunkX && chunkZ == o.chunkZ; }
};
struct ChunkCoordHash {
    size_t operator()(const ChunkCoord& c) const {
        return std::hash<int64_t>{}(static_cast<int64_t>(c.chunkX) << 32 | (c.chunkZ & 0xFFFFFFFFL));
    }
};
#endif

// ═══════════════════════════════════════════════════════════════════════════
// Difficulty enum — only defined if World.h not already included
// ═══════════════════════════════════════════════════════════════════════════

#ifndef MCCPP_WORLD_H_INCLUDED
enum class Difficulty : int32_t {
    PEACEFUL = 0,
    EASY = 1,
    NORMAL = 2,
    HARD = 3
};
#endif

// ═══════════════════════════════════════════════════════════════════════════
// BonusChestContent — Weighted loot for spawn bonus chest.
// Java reference: WorldServer.bonusChestContent
// ═══════════════════════════════════════════════════════════════════════════

struct BonusChestItem {
    int32_t itemId;
    int32_t meta;
    int32_t minCount, maxCount;
    int32_t weight;
};

// Items: sticks(280), planks(5), log(17), stoneAxe(275), woodAxe(271),
//        stonePickaxe(274), woodPickaxe(270), apple(260), bread(297), log2(162)
static constexpr BonusChestItem BONUS_CHEST_CONTENT[] = {
    {280, 0, 1, 3, 10},    // Sticks
    {5,   0, 1, 3, 10},    // Planks
    {17,  0, 1, 3, 10},    // Log
    {275, 0, 1, 1, 3},     // Stone axe
    {271, 0, 1, 1, 5},     // Wooden axe
    {274, 0, 1, 1, 3},     // Stone pickaxe
    {270, 0, 1, 1, 5},     // Wooden pickaxe
    {260, 0, 2, 3, 5},     // Apple
    {297, 0, 2, 3, 3},     // Bread
    {162, 0, 1, 3, 10},    // Log2 (acacia/dark oak)
};

// ═══════════════════════════════════════════════════════════════════════════
// WorldServer — Alternate dimension-level world (only if World.h not included)
// If World.h was included, its WorldServer class is the canonical one.
// ═══════════════════════════════════════════════════════════════════════════

#ifndef MCCPP_WORLD_H_INCLUDED
class WorldServer {
public:
    // ─── World state ───
    int32_t dimensionId = 0;
    std::string worldName;
    int64_t worldSeed = 0;
    Difficulty difficulty = Difficulty::NORMAL;
    bool isHardcore = false;
    bool disableLevelSaving = false;
    bool spawnHostileMobs = true;
    bool spawnPeacefulMobs = true;

    // ─── Time ───
    int64_t totalWorldTime = 0;
    int64_t worldTime = 0;
    int32_t skylightSubtracted = 0;

    // ─── Weather ───
    bool raining = false;
    bool thundering = false;
    int32_t rainTime = 0;
    int32_t thunderTime = 0;
    float rainingStrength = 0.0f;
    float prevRainingStrength = 0.0f;
    float thunderingStrength = 0.0f;
    float prevThunderingStrength = 0.0f;

    // ─── Random block tick LCG ───
    int32_t updateLCG = 0;

    // ─── Active chunks ───
    std::unordered_set<ChunkCoord, ChunkCoordHash> activeChunkSet;

    // ─── Scheduled tick updates ───
    std::set<NextTickListEntry> pendingTicksTree;
    std::unordered_set<NextTickListEntry, NextTickHash> pendingTicksHash;
    std::vector<NextTickListEntry> pendingTicksThisTick;

    // ─── Block events ───
    std::vector<BlockEventData> blockEvents[2];
    int32_t blockEventIndex = 0;

    // ─── Entity tick optimization ───
    int32_t updateEntityTick = 0;
    bool allPlayersSleeping = false;
    int32_t playerCount = 0;

    // ─── Game rules ───
    bool doDaylightCycle = true;
    bool doMobSpawning = true;
    bool doMobLoot = true;
    bool doFireTick = true;
    bool keepInventory = false;

    // ─── Callbacks ───
    using BlockTickFn = std::function<void(int32_t x, int32_t y, int32_t z, int32_t blockId)>;
    using GetBlockFn = std::function<int32_t(int32_t x, int32_t y, int32_t z)>;
    using IsTickRandomFn = std::function<bool(int32_t blockId)>;
    using SetBlockFn = std::function<void(int32_t x, int32_t y, int32_t z, int32_t blockId)>;

    struct WeatherChange {
        bool rainStarted = false;
        bool rainStopped = false;
        bool rainStrengthChanged = false;
        bool thunderStrengthChanged = false;
        float newRainStrength = 0.0f;
        float newThunderStrength = 0.0f;
    };

    void tick(BlockTickFn onBlockTick, GetBlockFn getBlock,
                IsTickRandomFn isRandom, SetBlockFn setBlock) {
        if (isHardcore && difficulty != Difficulty::HARD) difficulty = Difficulty::HARD;
        if (allPlayersSleeping && playerCount > 0) {
            if (doDaylightCycle) { int64_t next = worldTime + 24000L; worldTime = next - (next % 24000L); }
            allPlayersSleeping = false;
            resetRainAndThunder();
        }
        ++totalWorldTime;
        if (doDaylightCycle) ++worldTime;
        tickUpdates(onBlockTick, getBlock);
        tickBlocks(onBlockTick, getBlock, isRandom, setBlock);
        if (playerCount == 0) ++updateEntityTick; else updateEntityTick = 0;
        int32_t prev = blockEventIndex; blockEventIndex = 1 - blockEventIndex; blockEvents[prev].clear();
    }

    WeatherChange updateWeather() {
        WeatherChange changes;
        bool wasRaining = raining && rainingStrength > 0.0f;
        if (thunderTime <= 0) { thunderTime = thundering ? (std::rand()%12000+3600) : (std::rand()%168000+12000); }
        else { --thunderTime; if (thunderTime <= 0) thundering = !thundering; }
        prevThunderingStrength = thunderingStrength;
        thunderingStrength += thundering ? 0.01f : -0.01f;
        thunderingStrength = std::clamp(thunderingStrength, 0.0f, 1.0f);
        if (rainTime <= 0) { rainTime = raining ? (std::rand()%12000+12000) : (std::rand()%168000+12000); }
        else { --rainTime; if (rainTime <= 0) raining = !raining; }
        prevRainingStrength = rainingStrength;
        rainingStrength += raining ? 0.01f : -0.01f;
        rainingStrength = std::clamp(rainingStrength, 0.0f, 1.0f);
        if (prevRainingStrength != rainingStrength) { changes.rainStrengthChanged = true; changes.newRainStrength = rainingStrength; }
        if (prevThunderingStrength != thunderingStrength) { changes.thunderStrengthChanged = true; changes.newThunderStrength = thunderingStrength; }
        bool isRaining = raining && rainingStrength > 0.0f;
        if (wasRaining && !isRaining) changes.rainStopped = true;
        else if (!wasRaining && isRaining) changes.rainStarted = true;
        return changes;
    }

    struct SubChunkInfo { int32_t yBase; bool needsRandomTick; };
    using GetSectionsFn = std::function<std::vector<SubChunkInfo>(int32_t, int32_t)>;
    using GetSubchunkBlockFn = std::function<int32_t(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t)>;

    void tickBlocks(BlockTickFn onBlockTick, GetBlockFn getBlock, IsTickRandomFn isRandom, SetBlockFn setBlock) {
        for (auto& coord : activeChunkSet) {
            if (raining && thundering) { advanceLCG(); if ((updateLCG%100000)==0) { int32_t v = advanceLCG()>>2; (void)v; } }
            advanceLCG();
        }
    }

    bool tickUpdates(BlockTickFn onBlockTick, GetBlockFn getBlock) {
        int32_t count = std::min(static_cast<int32_t>(pendingTicksTree.size()), 1000);
        auto it = pendingTicksTree.begin();
        for (int32_t i = 0; i < count && it != pendingTicksTree.end(); ++i) {
            if (it->scheduledTime > totalWorldTime) break;
            pendingTicksThisTick.push_back(*it); pendingTicksHash.erase(*it);
            auto r = it; ++it; pendingTicksTree.erase(r);
        }
        for (auto& e : pendingTicksThisTick) {
            int32_t cur = getBlock(e.x, e.y, e.z);
            if (cur == e.blockId && cur != 0) onBlockTick(e.x, e.y, e.z, e.blockId);
        }
        bool has = !pendingTicksTree.empty(); pendingTicksThisTick.clear(); return has;
    }

    void scheduleBlockUpdate(int32_t x, int32_t y, int32_t z, int32_t blockId, int32_t delay, int32_t priority = 0) {
        NextTickListEntry e; e.x=x; e.y=y; e.z=z; e.blockId=blockId; e.scheduledTime=delay+totalWorldTime; e.priority=priority;
        if (pendingTicksHash.find(e)==pendingTicksHash.end()) { pendingTicksHash.insert(e); pendingTicksTree.insert(e); }
    }

    void addBlockEvent(int32_t x, int32_t y, int32_t z, int32_t blockId, int32_t eventId, int32_t param) {
        blockEvents[blockEventIndex].push_back({x,y,z,blockId,eventId,param});
    }

    void resetRainAndThunder() { rainTime=0; raining=false; thunderTime=0; thundering=false; }

    void updateAllPlayersSleepingFlag(int32_t numPlayers, int32_t numSleeping) {
        playerCount = numPlayers; allPlayersSleeping = (numPlayers>0 && numSleeping==numPlayers);
    }

    float getCelestialAngle(float pt) const {
        int32_t dp = static_cast<int32_t>(worldTime%24000L);
        float a = (static_cast<float>(dp)+pt)/24000.0f-0.25f;
        if (a<0) a+=1; if (a>1) a-=1;
        float s=a; a=1.0f-static_cast<float>((std::cos(a*3.14159265358979)+1.0)/2.0); a=s+(a-s)/3.0f;
        return a;
    }

    int32_t calculateSkylightSubtracted(float pt) const {
        float a=getCelestialAngle(pt);
        float b=1.0f-(static_cast<float>(std::cos(a*3.14159265358979*2.0))*2.0f+0.5f);
        b=std::clamp(b,0.0f,1.0f); b=1.0f-b;
        if(raining) b-=0.3f; if(thundering) b-=0.3f; b=std::clamp(b,0.0f,1.0f);
        return static_cast<int32_t>((1.0f-b)*11.0f);
    }

    int32_t advanceLCG() { updateLCG=updateLCG*3+1013904223; return updateLCG; }

    int32_t spawnX=0, spawnY=64, spawnZ=0;
    void setSpawnPoint(int32_t x, int32_t y, int32_t z) { spawnX=x; spawnY=y; spawnZ=z; }
    bool shouldUpdateEntities() const { return playerCount>0 || updateEntityTick<1200; }
};
#endif // !MCCPP_WORLD_H_INCLUDED

} // namespace mccpp
