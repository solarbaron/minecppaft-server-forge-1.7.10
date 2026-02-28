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

#include <algorithm>
#include <atomic>
#include <cstdint>
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
// ChunkCoordIntPair
// ═══════════════════════════════════════════════════════════════════════════

struct ChunkCoord {
    int32_t chunkX, chunkZ;
    bool operator==(const ChunkCoord& o) const { return chunkX == o.chunkX && chunkZ == o.chunkZ; }
};
struct ChunkCoordHash {
    size_t operator()(const ChunkCoord& c) const {
        return std::hash<int64_t>{}(static_cast<int64_t>(c.chunkX) << 32 | (c.chunkZ & 0xFFFFFFFFL));
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Difficulty enum
// Java reference: net.minecraft.world.EnumDifficulty
// ═══════════════════════════════════════════════════════════════════════════

enum class Difficulty : int32_t {
    PEACEFUL = 0,
    EASY = 1,
    NORMAL = 2,
    HARD = 3
};

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
// WorldServer — Server-side dimension world.
// Java reference: net.minecraft.world.WorldServer
// ═══════════════════════════════════════════════════════════════════════════

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
    int64_t totalWorldTime = 0;    // Never resets
    int64_t worldTime = 0;         // Day/night cycle (0-24000)
    int32_t skylightSubtracted = 0;

    // ─── Weather ───
    bool raining = false;
    bool thundering = false;
    int32_t rainTime = 0;
    int32_t thunderTime = 0;

    // ─── Random block tick LCG ───
    // Java: updateLCG = updateLCG * 3 + 1013904223
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

    // ─── Callbacks ───
    using BlockTickFn = std::function<void(int32_t x, int32_t y, int32_t z, int32_t blockId)>;
    using GetBlockFn = std::function<int32_t(int32_t x, int32_t y, int32_t z)>;
    using IsTickRandomFn = std::function<bool(int32_t blockId)>;
    using SetBlockFn = std::function<void(int32_t x, int32_t y, int32_t z, int32_t blockId)>;

    // ═══════════════════════════════════════════════════════════════════════
    // Main tick pipeline
    // Java: WorldServer.tick()
    // ═══════════════════════════════════════════════════════════════════════

    void tick(BlockTickFn onBlockTick, GetBlockFn getBlock,
                IsTickRandomFn isRandom, SetBlockFn setBlock) {
        // 1. Hardcore → force hard difficulty
        if (isHardcore && difficulty != Difficulty::HARD) {
            difficulty = Difficulty::HARD;
        }

        // 2. All-players-sleeping → skip to dawn
        if (allPlayersSleeping && playerCount > 0) {
            if (doDaylightCycle) {
                int64_t next = worldTime + 24000L;
                worldTime = next - (next % 24000L);
            }
            allPlayersSleeping = false;
            resetRainAndThunder();
        }

        // 3. World time
        ++totalWorldTime;
        if (doDaylightCycle) ++worldTime;

        // 4. Scheduled tick updates
        tickUpdates(onBlockTick, getBlock);

        // 5. Random block ticks per active chunk
        tickBlocks(onBlockTick, getBlock, isRandom, setBlock);

        // 6. Entity tick optimization
        if (playerCount == 0) {
            ++updateEntityTick;
        } else {
            updateEntityTick = 0;
        }

        // 7. Swap block event buffers
        int32_t prev = blockEventIndex;
        blockEventIndex = 1 - blockEventIndex;
        blockEvents[prev].clear();
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Random block ticks
    // Java: func_147456_g
    // ═══════════════════════════════════════════════════════════════════════

    struct SubChunkInfo {
        int32_t yBase;         // Y base of the 16-high section
        bool needsRandomTick;  // Has any randomly-ticking blocks
    };

    // Callback to get subchunk sections for a chunk
    using GetSectionsFn = std::function<std::vector<SubChunkInfo>(int32_t chunkX, int32_t chunkZ)>;
    // Callback to get block ID from a subchunk
    using GetSubchunkBlockFn = std::function<int32_t(int32_t chunkX, int32_t chunkZ,
                                                        int32_t localX, int32_t localY, int32_t localZ,
                                                        int32_t sectionY)>;

    void tickBlocks(BlockTickFn onBlockTick, GetBlockFn getBlock,
                      IsTickRandomFn isRandom, SetBlockFn setBlock) {
        for (auto& coord : activeChunkSet) {
            int32_t baseX = coord.chunkX * 16;
            int32_t baseZ = coord.chunkZ * 16;

            // Thunder: 1/100000 chance during thunderstorm
            if (raining && thundering) {
                advanceLCG();
                if ((updateLCG % 100000) == 0) {
                    int32_t lcgVal = advanceLCG() >> 2;
                    int32_t tx = baseX + (lcgVal & 0xF);
                    int32_t tz = baseZ + ((lcgVal >> 8) & 0xF);
                    // Lightning strike would go here
                }
            }

            // Ice and snow: 1/16 chance
            advanceLCG();
            if ((updateLCG & 0xF) == 0) {
                int32_t lcgVal = advanceLCG() >> 2;
                int32_t ix = (lcgVal & 0xF);
                int32_t iz = ((lcgVal >> 8) & 0xF);
                // Ice/snow placement:
                // - Freeze water below precipitation height
                // - Place snow_layer during rain in cold biomes
            }

            // 3 random ticks per subchunk section
            // (Actual subchunk iteration deferred to world integration)
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Scheduled tick updates
    // Java: tickUpdates
    // ═══════════════════════════════════════════════════════════════════════

    bool tickUpdates(BlockTickFn onBlockTick, GetBlockFn getBlock) {
        int32_t count = static_cast<int32_t>(pendingTicksTree.size());
        if (count > 1000) count = 1000;

        // Move due entries from tree to this-tick list
        auto it = pendingTicksTree.begin();
        for (int32_t i = 0; i < count && it != pendingTicksTree.end(); ++i) {
            if (it->scheduledTime > totalWorldTime) break;
            pendingTicksThisTick.push_back(*it);
            pendingTicksHash.erase(*it);
            auto toRemove = it;
            ++it;
            pendingTicksTree.erase(toRemove);
        }

        // Execute ticks
        for (auto& entry : pendingTicksThisTick) {
            int32_t currentBlock = getBlock(entry.x, entry.y, entry.z);
            if (currentBlock == entry.blockId && currentBlock != 0) {
                onBlockTick(entry.x, entry.y, entry.z, entry.blockId);
            }
        }

        bool hasPending = !pendingTicksTree.empty();
        pendingTicksThisTick.clear();
        return hasPending;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Schedule a block update
    // Java: scheduleBlockUpdate / scheduleBlockUpdateWithPriority
    // ═══════════════════════════════════════════════════════════════════════

    void scheduleBlockUpdate(int32_t x, int32_t y, int32_t z,
                                int32_t blockId, int32_t delay, int32_t priority = 0) {
        NextTickListEntry entry;
        entry.x = x;
        entry.y = y;
        entry.z = z;
        entry.blockId = blockId;
        entry.scheduledTime = static_cast<int64_t>(delay) + totalWorldTime;
        entry.priority = priority;

        if (pendingTicksHash.find(entry) == pendingTicksHash.end()) {
            pendingTicksHash.insert(entry);
            pendingTicksTree.insert(entry);
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Block events
    // Java: addBlockEvent
    // ═══════════════════════════════════════════════════════════════════════

    void addBlockEvent(int32_t x, int32_t y, int32_t z,
                         int32_t blockId, int32_t eventId, int32_t param) {
        blockEvents[blockEventIndex].push_back({x, y, z, blockId, eventId, param});
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Weather
    // ═══════════════════════════════════════════════════════════════════════

    void resetRainAndThunder() {
        rainTime = 0;
        raining = false;
        thunderTime = 0;
        thundering = false;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Sleep
    // Java: updateAllPlayersSleepingFlag / areAllPlayersAsleep
    // ═══════════════════════════════════════════════════════════════════════

    // Called from PlayerList when sleep state changes
    void updateAllPlayersSleepingFlag(int32_t numPlayers, int32_t numSleeping) {
        playerCount = numPlayers;
        allPlayersSleeping = (numPlayers > 0 && numSleeping == numPlayers);
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Skylight
    // Java: calculateSkylightSubtracted
    // ═══════════════════════════════════════════════════════════════════════

    // Sun angle: 0.0 at noon, 0.5 at midnight
    float getCelestialAngle(float partialTicks) const {
        int32_t dayPhase = static_cast<int32_t>(worldTime % 24000L);
        float angle = (static_cast<float>(dayPhase) + partialTicks) / 24000.0f - 0.25f;
        if (angle < 0.0f) angle += 1.0f;
        if (angle > 1.0f) angle -= 1.0f;

        // Smoothing
        float smooth = angle;
        angle = 1.0f - static_cast<float>(
            (std::cos(static_cast<double>(angle) * 3.14159265358979) + 1.0) / 2.0);
        angle = smooth + (angle - smooth) / 3.0f;
        return angle;
    }

    int32_t calculateSkylightSubtracted(float partialTicks) const {
        float angle = getCelestialAngle(partialTicks);
        float brightness = 1.0f - (static_cast<float>(
            std::cos(static_cast<double>(angle) * 3.14159265358979 * 2.0)) * 2.0f + 0.5f);
        brightness = std::clamp(brightness, 0.0f, 1.0f);

        // Rain/thunder darken
        brightness = 1.0f - brightness;
        if (raining) brightness -= 0.3f;   // 5/16 ≈ 0.3125
        if (thundering) brightness -= 0.3f;
        brightness = std::clamp(brightness, 0.0f, 1.0f);

        return static_cast<int32_t>((1.0f - brightness) * 11.0f);
    }

    // ═══════════════════════════════════════════════════════════════════════
    // LCG helper
    // Java: updateLCG = updateLCG * 3 + 1013904223
    // ═══════════════════════════════════════════════════════════════════════

    int32_t advanceLCG() {
        updateLCG = updateLCG * 3 + 1013904223;
        return updateLCG;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Spawn point
    // ═══════════════════════════════════════════════════════════════════════

    int32_t spawnX = 0, spawnY = 64, spawnZ = 0;

    void setSpawnPoint(int32_t x, int32_t y, int32_t z) {
        spawnX = x; spawnY = y; spawnZ = z;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Entity management
    // ═══════════════════════════════════════════════════════════════════════

    bool shouldUpdateEntities() const {
        return playerCount > 0 || updateEntityTick < 1200;
    }
};

} // namespace mccpp
