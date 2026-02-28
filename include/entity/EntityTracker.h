/**
 * EntityTracker.h — Server-side entity visibility management.
 *
 * Java references:
 *   - net.minecraft.entity.EntityTracker
 *   - net.minecraft.entity.EntityTrackerEntry
 *
 * Manages which entities are visible to which players. For each tracked
 * entity, maintains a set of players that can see it, and sends
 * spawn/despawn/movement packets as entities enter/leave tracking range.
 *
 * Entity type → tracking parameters (from Java):
 *   Player:        range=512, interval=2
 *   Projectile:    range=64,  interval=10-20
 *   Mob/Animal:    range=80,  interval=3
 *   Item:          range=64,  interval=20
 *   Minecart/Boat: range=80,  interval=3
 *   TNT/Falling:   range=160, interval=10-20
 *   Dragon:        range=160, interval=3
 *   XPOrb:         range=160, interval=20
 *   EnderCrystal:  range=256, interval=MAX
 *
 * Thread safety: Mutex-protected for add/remove/update.
 * Use shared_mutex for read-heavy operations (sending to trackers).
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityType — Classification for tracking parameters.
// ═══════════════════════════════════════════════════════════════════════════

enum class TrackedEntityType : uint8_t {
    Player,
    FishHook,
    Arrow,
    Fireball,
    SmallFireball,
    Snowball,
    EnderPearl,
    EnderEye,
    Egg,
    Potion,
    ExpBottle,
    FireworkRocket,
    Item,
    Minecart,
    Boat,
    Squid,
    Wither,
    Bat,
    Animal, // IAnimals
    Dragon,
    TNTPrimed,
    FallingBlock,
    Hanging, // Painting, ItemFrame
    XPOrb,
    EnderCrystal,
    Other
};

// ═══════════════════════════════════════════════════════════════════════════
// TrackingParams — Per-entity-type tracking configuration.
// Java reference: EntityTracker.trackEntity (giant if-else chain)
// ═══════════════════════════════════════════════════════════════════════════

struct TrackingParams {
    int32_t trackingRange;              // Blocks (before clamping to max threshold)
    int32_t updateInterval;             // Ticks between position updates
    bool sendVelocityUpdates;           // Whether to send motion packets

    // Java: getTrackingParams — maps entity type to parameters
    static TrackingParams forType(TrackedEntityType type) {
        switch (type) {
            case TrackedEntityType::Player:        return { 512, 2, false };
            case TrackedEntityType::FishHook:       return { 64, 5, true };
            case TrackedEntityType::Arrow:          return { 64, 20, false };
            case TrackedEntityType::SmallFireball:   return { 64, 10, false };
            case TrackedEntityType::Fireball:        return { 64, 10, false };
            case TrackedEntityType::Snowball:        return { 64, 10, true };
            case TrackedEntityType::EnderPearl:      return { 64, 10, true };
            case TrackedEntityType::EnderEye:        return { 64, 4, true };
            case TrackedEntityType::Egg:             return { 64, 10, true };
            case TrackedEntityType::Potion:          return { 64, 10, true };
            case TrackedEntityType::ExpBottle:       return { 64, 10, true };
            case TrackedEntityType::FireworkRocket:  return { 64, 10, true };
            case TrackedEntityType::Item:            return { 64, 20, true };
            case TrackedEntityType::Minecart:        return { 80, 3, true };
            case TrackedEntityType::Boat:            return { 80, 3, true };
            case TrackedEntityType::Squid:           return { 64, 3, true };
            case TrackedEntityType::Wither:          return { 80, 3, false };
            case TrackedEntityType::Bat:             return { 80, 3, false };
            case TrackedEntityType::Animal:          return { 80, 3, true };
            case TrackedEntityType::Dragon:          return { 160, 3, true };
            case TrackedEntityType::TNTPrimed:       return { 160, 10, true };
            case TrackedEntityType::FallingBlock:    return { 160, 20, true };
            case TrackedEntityType::Hanging:         return { 160, INT32_MAX, false };
            case TrackedEntityType::XPOrb:           return { 160, 20, true };
            case TrackedEntityType::EnderCrystal:    return { 256, INT32_MAX, false };
            default:                                return { 80, 3, true };
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// TrackedEntity — Position snapshot for a tracked entity.
// Simplified entity state needed for tracking decisions.
// ═══════════════════════════════════════════════════════════════════════════

struct TrackedEntityInfo {
    int32_t entityId = 0;
    TrackedEntityType type = TrackedEntityType::Other;
    double posX = 0.0, posY = 0.0, posZ = 0.0;
    float yaw = 0.0f, pitch = 0.0f, headYaw = 0.0f;
    double motionX = 0.0, motionY = 0.0, motionZ = 0.0;
    int32_t chunkX = 0, chunkZ = 0;   // Chunk coordinates
    bool isPlayer = false;
    bool isDead = false;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityTrackerEntry — Tracks one entity and its observing players.
// Java reference: net.minecraft.entity.EntityTrackerEntry
// ═══════════════════════════════════════════════════════════════════════════

class EntityTrackerEntry {
public:
    TrackedEntityInfo entity;
    TrackingParams params;

    // Last sent position/rotation (for delta computation)
    double lastPosX = 0.0, lastPosY = 0.0, lastPosZ = 0.0;
    float lastYaw = 0.0f, lastPitch = 0.0f, lastHeadYaw = 0.0f;

    int32_t updateCounter = 0;
    bool playerEntitiesUpdated = false;

    // Set of player entity IDs that are currently tracking this entity
    std::unordered_set<int32_t> trackingPlayers;

    EntityTrackerEntry() = default;

    EntityTrackerEntry(const TrackedEntityInfo& info, int32_t maxRange)
        : entity(info) {
        params = TrackingParams::forType(info.type);
        // Clamp to max threshold
        if (params.trackingRange > maxRange) {
            params.trackingRange = maxRange;
        }
        lastPosX = info.posX;
        lastPosY = info.posY;
        lastPosZ = info.posZ;
        lastYaw = info.yaw;
        lastPitch = info.pitch;
        lastHeadYaw = info.headYaw;
    }

    // Check if a player is within tracking range
    bool isPlayerInRange(double playerX, double playerZ) const {
        double dx = playerX - entity.posX;
        double dz = playerZ - entity.posZ;
        double range = static_cast<double>(params.trackingRange);
        return dx >= -range && dx <= range && dz >= -range && dz <= range;
    }

    // Compute position deltas (fixed-point * 32)
    bool hasPositionChanged() const {
        return entity.posX != lastPosX || entity.posY != lastPosY || entity.posZ != lastPosZ;
    }

    bool hasRotationChanged() const {
        return entity.yaw != lastYaw || entity.pitch != lastPitch;
    }

    // Update last-sent values
    void snapshotPosition() {
        lastPosX = entity.posX;
        lastPosY = entity.posY;
        lastPosZ = entity.posZ;
        lastYaw = entity.yaw;
        lastPitch = entity.pitch;
        lastHeadYaw = entity.headYaw;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityTracker — Manages all tracked entities for one world.
// Java reference: net.minecraft.entity.EntityTracker
// ═══════════════════════════════════════════════════════════════════════════

class EntityTracker {
public:
    explicit EntityTracker(int32_t maxTrackingDistance = 512)
        : maxTrackingDistance_(maxTrackingDistance) {}

    // ─── Track / Untrack ───

    // Java: trackEntity + addEntityToTracker
    bool trackEntity(const TrackedEntityInfo& info) {
        std::unique_lock<std::shared_mutex> lock(mutex_);

        if (entriesById_.count(info.entityId)) {
            return false; // Already tracked
        }

        auto entry = std::make_unique<EntityTrackerEntry>(info, maxTrackingDistance_);
        entriesById_[info.entityId] = std::move(entry);
        return true;
    }

    // Java: untrackEntity
    void untrackEntity(int32_t entityId) {
        std::unique_lock<std::shared_mutex> lock(mutex_);

        auto it = entriesById_.find(entityId);
        if (it != entriesById_.end()) {
            auto& entry = it->second;
            // Collect all players that were tracking this entity
            // so they can be told to destroy it
            lastUntracked_.clear();
            for (int32_t pid : entry->trackingPlayers) {
                lastUntracked_.push_back(pid);
            }
            entriesById_.erase(it);
        }
    }

    // Get players that need to receive S13PacketDestroyEntities after untrack
    std::vector<int32_t> getLastUntrackedPlayers() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return lastUntracked_;
    }

    // ─── Update ───

    // Java: updateTrackedEntities — called once per tick
    // For each entry, checks all players for enter/leave tracking range.
    // Returns a list of (playerEntityId, trackedEntityId, isEntering) events.
    struct TrackingEvent {
        int32_t playerEntityId;
        int32_t trackedEntityId;
        bool entering; // true=spawn, false=despawn
    };

    std::vector<TrackingEvent> updateTrackedEntities(
            const std::vector<TrackedEntityInfo>& allPlayers) {

        std::unique_lock<std::shared_mutex> lock(mutex_);
        std::vector<TrackingEvent> events;

        for (auto& [entityId, entry] : entriesById_) {
            entry->updateCounter++;

            // Skip if not time for update yet
            if (entry->updateCounter % entry->params.updateInterval != 0 &&
                entry->params.updateInterval != INT32_MAX) {
                // Still check player visibility even if not updating position
            }

            // Check each player
            for (const auto& player : allPlayers) {
                if (player.entityId == entityId) continue; // Don't track self

                bool inRange = entry->isPlayerInRange(player.posX, player.posZ);
                bool wasTracking = entry->trackingPlayers.count(player.entityId) > 0;

                if (inRange && !wasTracking) {
                    // Player entered range — spawn entity for them
                    entry->trackingPlayers.insert(player.entityId);
                    events.push_back({ player.entityId, entityId, true });
                } else if (!inRange && wasTracking) {
                    // Player left range — despawn entity for them
                    entry->trackingPlayers.erase(player.entityId);
                    events.push_back({ player.entityId, entityId, false });
                }
            }

            entry->playerEntitiesUpdated = false;
        }

        return events;
    }

    // ─── Position update ───

    // Update entity position in tracker (called when entity moves)
    void updateEntityPosition(int32_t entityId, double x, double y, double z,
                                float yaw, float pitch, float headYaw) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        auto it = entriesById_.find(entityId);
        if (it != entriesById_.end()) {
            auto& e = it->second->entity;
            e.posX = x; e.posY = y; e.posZ = z;
            e.yaw = yaw; e.pitch = pitch; e.headYaw = headYaw;
            e.chunkX = static_cast<int32_t>(std::floor(x)) >> 4;
            e.chunkZ = static_cast<int32_t>(std::floor(z)) >> 4;
        }
    }

    // Mark entity as dead
    void markDead(int32_t entityId) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        auto it = entriesById_.find(entityId);
        if (it != entriesById_.end()) {
            it->second->entity.isDead = true;
        }
    }

    // ─── Query ───

    // Java: sendToAllTrackingEntity — send packet to all players tracking entity
    std::vector<int32_t> getTrackingPlayers(int32_t entityId) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = entriesById_.find(entityId);
        if (it != entriesById_.end()) {
            return std::vector<int32_t>(
                it->second->trackingPlayers.begin(),
                it->second->trackingPlayers.end());
        }
        return {};
    }

    // Java: func_151248_b — send to all tracking + the entity itself if player
    std::vector<int32_t> getTrackingPlayersAndSelf(int32_t entityId) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = entriesById_.find(entityId);
        if (it != entriesById_.end()) {
            std::vector<int32_t> result(
                it->second->trackingPlayers.begin(),
                it->second->trackingPlayers.end());
            if (it->second->entity.isPlayer) {
                result.push_back(entityId); // Include self
            }
            return result;
        }
        return {};
    }

    // Java: removePlayerFromTrackers — remove player from all entries
    void removePlayerFromTrackers(int32_t playerEntityId) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        for (auto& [id, entry] : entriesById_) {
            entry->trackingPlayers.erase(playerEntityId);
        }
    }

    // Java: func_85172_a — update tracking for entities in a specific chunk
    std::vector<TrackingEvent> updatePlayerForChunk(int32_t playerEntityId,
                                                      double playerX, double playerZ,
                                                      int32_t chunkX, int32_t chunkZ) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        std::vector<TrackingEvent> events;

        for (auto& [id, entry] : entriesById_) {
            if (id == playerEntityId) continue;
            if (entry->entity.chunkX != chunkX || entry->entity.chunkZ != chunkZ) continue;

            bool inRange = entry->isPlayerInRange(playerX, playerZ);
            bool wasTracking = entry->trackingPlayers.count(playerEntityId) > 0;

            if (inRange && !wasTracking) {
                entry->trackingPlayers.insert(playerEntityId);
                events.push_back({ playerEntityId, id, true });
            }
        }
        return events;
    }

    // Get all tracked entity IDs
    std::vector<int32_t> getAllTrackedIds() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        std::vector<int32_t> ids;
        ids.reserve(entriesById_.size());
        for (const auto& [id, entry] : entriesById_) {
            ids.push_back(id);
        }
        return ids;
    }

    // Get entry count
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return entriesById_.size();
    }

    // Check if entity needs position/rotation update packets
    struct MovementUpdate {
        int32_t entityId;
        double dx, dy, dz;
        float yaw, pitch, headYaw;
        bool posChanged;
        bool rotChanged;
        bool sendVelocity;
        double motionX, motionY, motionZ;
        std::vector<int32_t> watchers;
    };

    std::vector<MovementUpdate> getMovementUpdates() {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        std::vector<MovementUpdate> updates;

        for (auto& [id, entry] : entriesById_) {
            if (entry->trackingPlayers.empty()) continue;
            if (entry->updateCounter % entry->params.updateInterval != 0 &&
                entry->params.updateInterval != INT32_MAX) continue;

            bool posChanged = entry->hasPositionChanged();
            bool rotChanged = entry->hasRotationChanged();

            if (!posChanged && !rotChanged) continue;

            MovementUpdate upd;
            upd.entityId = id;
            upd.dx = entry->entity.posX - entry->lastPosX;
            upd.dy = entry->entity.posY - entry->lastPosY;
            upd.dz = entry->entity.posZ - entry->lastPosZ;
            upd.yaw = entry->entity.yaw;
            upd.pitch = entry->entity.pitch;
            upd.headYaw = entry->entity.headYaw;
            upd.posChanged = posChanged;
            upd.rotChanged = rotChanged;
            upd.sendVelocity = entry->params.sendVelocityUpdates;
            upd.motionX = entry->entity.motionX;
            upd.motionY = entry->entity.motionY;
            upd.motionZ = entry->entity.motionZ;
            upd.watchers.assign(entry->trackingPlayers.begin(),
                                 entry->trackingPlayers.end());

            entry->snapshotPosition();
            updates.push_back(std::move(upd));
        }
        return updates;
    }

private:
    mutable std::shared_mutex mutex_;
    int32_t maxTrackingDistance_;
    std::unordered_map<int32_t, std::unique_ptr<EntityTrackerEntry>> entriesById_;
    std::vector<int32_t> lastUntracked_;
};

} // namespace mccpp
