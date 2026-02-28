/**
 * ScheduledTick.h — Scheduled block update system and random tick engine.
 *
 * Java references:
 *   - net.minecraft.world.NextTickListEntry — Single scheduled tick entry
 *   - net.minecraft.world.WorldServer.tickUpdates — Tick processing
 *   - net.minecraft.world.WorldServer.scheduleBlockUpdate — Schedule a tick
 *   - net.minecraft.world.WorldServer.func_147456_g — Random tick, weather
 *   - net.minecraft.block.BlockEventData — Block event data
 *
 * Architecture:
 *   - Sorted set (by time→priority→insertionOrder) + hash set for O(1) dedup
 *   - Max 1000 ticks processed per game tick
 *   - Block events use double-buffered lists with ping-pong index
 *   - Random ticks: 3 per chunk section via LCG (updateLCG * 3 + 1013904223)
 *
 * Thread safety: Called from world tick thread.
 * JNI readiness: Simple data, standard containers.
 */
#pragma once

#include <cstdint>
#include <functional>
#include <set>
#include <unordered_set>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// NextTickListEntry — A single scheduled block update.
// Java reference: net.minecraft.world.NextTickListEntry
// ═══════════════════════════════════════════════════════════════════════════

struct NextTickListEntry {
    int32_t x, y, z;
    int32_t blockId;
    int64_t scheduledTime;
    int32_t priority;
    int64_t tickEntryId;

    // Java: compareTo — time, then priority, then insertion order
    bool operator<(const NextTickListEntry& other) const {
        if (scheduledTime != other.scheduledTime) return scheduledTime < other.scheduledTime;
        if (priority != other.priority) return priority < other.priority;
        return tickEntryId < other.tickEntryId;
    }

    // Java: equals — position + block (NOT time or priority)
    bool operator==(const NextTickListEntry& other) const {
        return x == other.x && y == other.y && z == other.z && blockId == other.blockId;
    }
};

struct NextTickHash {
    // Java: hashCode = (x * 1024 * 1024 + z * 1024 + y) * 256
    size_t operator()(const NextTickListEntry& e) const {
        return static_cast<size_t>(
            (e.x * 1024 * 1024 + e.z * 1024 + e.y) * 256
        );
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// BlockEventData — Block event (pistons, note blocks, etc.)
// Java reference: net.minecraft.block.BlockEventData
// ═══════════════════════════════════════════════════════════════════════════

struct BlockEventData {
    int32_t x, y, z;
    int32_t blockId;
    int32_t eventId;
    int32_t eventParam;

    bool operator==(const BlockEventData& other) const {
        return x == other.x && y == other.y && z == other.z &&
               blockId == other.blockId && eventId == other.eventId &&
               eventParam == other.eventParam;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// RandomTickResult — Result of a random tick selection.
// ═══════════════════════════════════════════════════════════════════════════

struct RandomTickResult {
    int32_t x, y, z;
    int32_t blockId;
};

// ═══════════════════════════════════════════════════════════════════════════
// ScheduledTickManager — Manages the tick priority queue.
// Java reference: net.minecraft.world.WorldServer (tick-related fields)
// ═══════════════════════════════════════════════════════════════════════════

class ScheduledTickManager {
public:
    ScheduledTickManager() = default;

    // ─── Scheduling ───

    // Java: scheduleBlockUpdateWithPriority
    void scheduleUpdate(int32_t x, int32_t y, int32_t z, int32_t blockId,
                          int32_t delay, int32_t priority, int64_t worldTime) {
        NextTickListEntry entry;
        entry.x = x;
        entry.y = y;
        entry.z = z;
        entry.blockId = blockId;
        entry.scheduledTime = static_cast<int64_t>(delay) + worldTime;
        entry.priority = priority;
        entry.tickEntryId = nextId_++;

        if (hashSet_.find(entry) == hashSet_.end()) {
            hashSet_.insert(entry);
            sortedSet_.insert(entry);
        }
    }

    // Java: scheduleBlockUpdate (priority = 0)
    void scheduleUpdate(int32_t x, int32_t y, int32_t z, int32_t blockId,
                          int32_t delay, int64_t worldTime) {
        scheduleUpdate(x, y, z, blockId, delay, 0, worldTime);
    }

    // Java: isBlockTickScheduledThisTick
    bool isScheduledThisTick(int32_t x, int32_t y, int32_t z, int32_t blockId) const {
        NextTickListEntry query;
        query.x = x; query.y = y; query.z = z; query.blockId = blockId;
        for (const auto& e : pendingThisTick_) {
            if (e == query) return true;
        }
        return false;
    }

    // ─── Tick processing ───

    struct TickAction {
        int32_t x, y, z;
        int32_t blockId;
        bool reschedule;  // If chunk didn't exist, reschedule
    };

    // Java: tickUpdates
    std::vector<TickAction> processTicks(int64_t worldTime, bool forceAll) {
        std::vector<TickAction> results;

        int32_t count = static_cast<int32_t>(sortedSet_.size());
        // Java: consistency check
        // if (count != hashSet_.size()) → error (we maintain invariant)

        if (count > 1000) count = 1000;

        pendingThisTick_.clear();

        // Phase 1: Collect due ticks
        auto it = sortedSet_.begin();
        for (int32_t i = 0; i < count && it != sortedSet_.end(); ++i) {
            if (!forceAll && it->scheduledTime > worldTime) break;
            NextTickListEntry entry = *it;
            it = sortedSet_.erase(it);
            hashSet_.erase(entry);
            pendingThisTick_.push_back(entry);
        }

        // Phase 2: Execute ticks
        for (auto& entry : pendingThisTick_) {
            TickAction action;
            action.x = entry.x;
            action.y = entry.y;
            action.z = entry.z;
            action.blockId = entry.blockId;
            action.reschedule = false;
            results.push_back(action);
        }

        pendingThisTick_.clear();
        return results;
    }

    // ─── Chunk save/load ───

    // Java: getPendingBlockUpdates — get ticks within a chunk
    std::vector<NextTickListEntry> getTicksInChunk(int32_t chunkX, int32_t chunkZ,
                                                     bool removeFound) {
        std::vector<NextTickListEntry> result;
        int32_t minX = (chunkX << 4) - 2;
        int32_t maxX = minX + 16 + 2;
        int32_t minZ = (chunkZ << 4) - 2;
        int32_t maxZ = minZ + 16 + 2;

        // Check sorted set
        auto it = sortedSet_.begin();
        while (it != sortedSet_.end()) {
            if (it->x >= minX && it->x < maxX && it->z >= minZ && it->z < maxZ) {
                result.push_back(*it);
                if (removeFound) {
                    hashSet_.erase(*it);
                    it = sortedSet_.erase(it);
                    continue;
                }
            }
            ++it;
        }

        // Also check pendingThisTick
        auto pt = pendingThisTick_.begin();
        while (pt != pendingThisTick_.end()) {
            if (pt->x >= minX && pt->x < maxX && pt->z >= minZ && pt->z < maxZ) {
                result.push_back(*pt);
                if (removeFound) {
                    pt = pendingThisTick_.erase(pt);
                    continue;
                }
            }
            ++pt;
        }

        return result;
    }

    bool hasPendingTicks() const { return !sortedSet_.empty(); }
    int32_t getPendingCount() const { return static_cast<int32_t>(sortedSet_.size()); }

private:
    std::set<NextTickListEntry> sortedSet_;     // Java: pendingTickListEntriesTreeSet
    std::unordered_set<NextTickListEntry, NextTickHash> hashSet_;  // Java: pendingTickListEntriesHashSet
    std::vector<NextTickListEntry> pendingThisTick_;  // Java: pendingTickListEntriesThisTick
    int64_t nextId_ = 0;  // Java: NextTickListEntry.nextTickEntryID (static)
};

// ═══════════════════════════════════════════════════════════════════════════
// BlockEventManager — Double-buffered block event system.
// Java reference: net.minecraft.world.WorldServer.field_147490_S
// ═══════════════════════════════════════════════════════════════════════════

class BlockEventManager {
public:
    BlockEventManager() = default;

    // Java: addBlockEvent
    void addEvent(int32_t x, int32_t y, int32_t z, int32_t blockId,
                    int32_t eventId, int32_t eventParam) {
        BlockEventData event{x, y, z, blockId, eventId, eventParam};
        // Check for duplicate
        for (const auto& existing : buffers_[activeIndex_]) {
            if (existing == event) return;
        }
        buffers_[activeIndex_].push_back(event);
    }

    // Java: func_147488_Z — process all pending events
    // Returns events to send to clients
    using EventHandler = std::function<bool(const BlockEventData&)>;

    std::vector<BlockEventData> processEvents(EventHandler handler) {
        std::vector<BlockEventData> clientUpdates;

        while (!buffers_[activeIndex_].empty()) {
            int32_t currentIdx = activeIndex_;
            activeIndex_ ^= 1;  // Ping-pong

            for (const auto& event : buffers_[currentIdx]) {
                if (handler(event)) {
                    clientUpdates.push_back(event);
                }
            }
            buffers_[currentIdx].clear();
        }

        return clientUpdates;
    }

private:
    std::vector<BlockEventData> buffers_[2];
    int32_t activeIndex_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// RandomTickEngine — LCG-based random tick selection.
// Java reference: net.minecraft.world.WorldServer.func_147456_g
// ═══════════════════════════════════════════════════════════════════════════

class RandomTickEngine {
public:
    RandomTickEngine() = default;
    explicit RandomTickEngine(int32_t seed) : updateLCG_(seed) {}

    // Java: LCG update: updateLCG = updateLCG * 3 + 1013904223
    void advance() {
        updateLCG_ = updateLCG_ * 3 + 1013904223;
    }

    int32_t getValue() const { return updateLCG_; }

    // Extract random tick position within a 16x16x16 section
    // Java: n9 = updateLCG >> 2; x = n9 & 0xF; z = (n9 >> 8) & 0xF; y = (n9 >> 16) & 0xF
    struct TickPos {
        int32_t x, y, z;  // 0-15 within section
    };

    TickPos getRandomPosition() {
        advance();
        int32_t val = updateLCG_ >> 2;
        return {
            val & 0xF,
            (val >> 16) & 0xF,
            (val >> 8) & 0xF
        };
    }

    // Get 3 random tick positions per section (Java: for i = 0..2)
    std::array<TickPos, 3> getRandomTicks() {
        return {getRandomPosition(), getRandomPosition(), getRandomPosition()};
    }

    // Lightning position selection within chunk
    struct ChunkPos { int32_t x, z; };

    ChunkPos getLightningPos(int32_t chunkBaseX, int32_t chunkBaseZ) {
        advance();
        int32_t val = updateLCG_ >> 2;
        return {
            chunkBaseX + (val & 0xF),
            chunkBaseZ + ((val >> 8) & 0xF)
        };
    }

    // Ice/snow position selection
    ChunkPos getIceSnowPos(int32_t chunkBaseX, int32_t chunkBaseZ) {
        advance();
        int32_t val = updateLCG_ >> 2;
        return {
            (val & 0xF) + chunkBaseX,
            ((val >> 8) & 0xF) + chunkBaseZ
        };
    }

private:
    int32_t updateLCG_ = 0;
};

} // namespace mccpp
