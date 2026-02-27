#pragma once
// SpawnPlayer — S→C 0x0C Named Entity Spawn + player visibility tracking.
// Ported from vanilla 1.7.10 S0CPacketSpawnPlayer (go2.java / MCP).
//
// Protocol 5 (1.7.10):
//   S→C 0x0C: Spawn Player (entityId, playerUUID, playerName,
//              dataCount, [propertyName, value, signature],
//              x, y, z, yaw, pitch, currentItem, metadata)
//
// This packet spawns another player entity in the client's world.
// The player must already be in the tab list (0x38) before this.

#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "networking/PacketBuffer.h"
#include "entity/EntityMetadata.h"

namespace mc {

// ============================================================
// S→C 0x0C Spawn Player — go2.java
// ============================================================
struct SpawnPlayerPacket {
    int32_t     entityId;
    std::string playerUUID;  // Without dashes
    std::string playerName;
    // Skin properties (for 1.7.10 protocol 5, sent inline)
    struct Property {
        std::string name;
        std::string value;
        std::string signature; // Empty if unsigned
    };
    std::vector<Property> properties;
    int32_t x, y, z;     // Fixed-point (value * 32)
    int8_t  yaw, pitch;
    int16_t currentItem;  // Item ID held, 0 for empty
    std::vector<MetadataEntry> metadata;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x0C);
        buf.writeVarInt(entityId);

        // In protocol 5, UUID is sent as a string (hex, no dashes)
        buf.writeString(playerUUID);
        buf.writeString(playerName);

        // Property count and data
        buf.writeVarInt(static_cast<int32_t>(properties.size()));
        for (auto& prop : properties) {
            buf.writeString(prop.name);
            buf.writeString(prop.value);
            buf.writeString(prop.signature);
        }

        buf.writeInt(x);
        buf.writeInt(y);
        buf.writeInt(z);
        buf.writeByte(static_cast<uint8_t>(yaw));
        buf.writeByte(static_cast<uint8_t>(pitch));
        buf.writeShort(currentItem);

        // Entity metadata
        DataWatcher::writeToBuffer(buf, metadata);

        return buf;
    }

    // Factory from player data
    static SpawnPlayerPacket fromPlayer(int32_t eid, const std::string& uuid,
                                         const std::string& name,
                                         double px, double py, double pz,
                                         float yawDeg, float pitchDeg,
                                         int16_t heldItem = 0) {
        SpawnPlayerPacket pkt;
        pkt.entityId = eid;
        pkt.playerUUID = uuid;
        pkt.playerName = name;
        pkt.x = static_cast<int32_t>(px * 32.0);
        pkt.y = static_cast<int32_t>(py * 32.0);
        pkt.z = static_cast<int32_t>(pz * 32.0);
        pkt.yaw = static_cast<int8_t>(yawDeg / 360.0f * 256.0f);
        pkt.pitch = static_cast<int8_t>(pitchDeg / 360.0f * 256.0f);
        pkt.currentItem = heldItem;

        // Default player metadata
        DataWatcher dw;
        dw.initPlayer();
        pkt.metadata = dw.getAll();

        return pkt;
    }
};

// ============================================================
// S→C 0x16 Entity Look — gu.java
// ============================================================
struct EntityLookPacket {
    int32_t entityId;
    int8_t  yaw, pitch;
    bool    onGround;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x16);
        buf.writeVarInt(entityId);
        buf.writeByte(static_cast<uint8_t>(yaw));
        buf.writeByte(static_cast<uint8_t>(pitch));
        buf.writeBoolean(onGround);
        return buf;
    }

    static EntityLookPacket fromDegrees(int32_t eid, float yawDeg, float pitchDeg, bool ground) {
        return {
            eid,
            static_cast<int8_t>(yawDeg / 360.0f * 256.0f),
            static_cast<int8_t>(pitchDeg / 360.0f * 256.0f),
            ground
        };
    }
};

// ============================================================
// S→C 0x17 Entity Look + Relative Move — gv.java
// ============================================================
struct EntityLookRelativeMovePacket {
    int32_t entityId;
    int8_t  dx, dy, dz;
    int8_t  yaw, pitch;
    bool    onGround;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x17);
        buf.writeVarInt(entityId);
        buf.writeByte(static_cast<uint8_t>(dx));
        buf.writeByte(static_cast<uint8_t>(dy));
        buf.writeByte(static_cast<uint8_t>(dz));
        buf.writeByte(static_cast<uint8_t>(yaw));
        buf.writeByte(static_cast<uint8_t>(pitch));
        buf.writeBoolean(onGround);
        return buf;
    }
};

// ============================================================
// Player position tracking for entity updates
// ============================================================
struct TrackedPosition {
    double posX, posY, posZ;
    float  yaw, pitch;

    // Calculate fixed-point delta for relative move
    bool calculateDelta(double newX, double newY, double newZ,
                         int8_t& dx, int8_t& dy, int8_t& dz) const {
        int32_t diffX = static_cast<int32_t>((newX * 32.0) - (posX * 32.0));
        int32_t diffY = static_cast<int32_t>((newY * 32.0) - (posY * 32.0));
        int32_t diffZ = static_cast<int32_t>((newZ * 32.0) - (posZ * 32.0));

        // Check if delta fits in int8_t range (-128 to 127)
        if (diffX < -128 || diffX > 127 || diffY < -128 || diffY > 127 ||
            diffZ < -128 || diffZ > 127) {
            return false; // Need teleport instead
        }

        dx = static_cast<int8_t>(diffX);
        dy = static_cast<int8_t>(diffY);
        dz = static_cast<int8_t>(diffZ);
        return true;
    }
};

// ============================================================
// Player visibility manager — tracks which players see each other
// ============================================================
class PlayerVisibilityManager {
public:
    static constexpr double VIEW_DISTANCE = 256.0; // ~16 chunks

    // Register a player entering the world
    void addPlayer(int fd, int32_t entityId, double x, double y, double z,
                    float yaw, float pitch) {
        TrackedPlayer tp;
        tp.fd = fd;
        tp.entityId = entityId;
        tp.tracked = {x, y, z, yaw, pitch};
        tp.lastSent = tp.tracked;
        players_[fd] = tp;
    }

    // Update a player's position
    void updatePosition(int fd, double x, double y, double z, float yaw, float pitch) {
        auto it = players_.find(fd);
        if (it != players_.end()) {
            it->second.tracked = {x, y, z, yaw, pitch};
        }
    }

    // Remove a player
    void removePlayer(int fd) {
        int32_t eid = -1;
        auto it = players_.find(fd);
        if (it != players_.end()) {
            eid = it->second.entityId;
            players_.erase(it);
        }
        // Remove from all visibility sets
        if (eid >= 0) {
            for (auto& [otherFd, tp] : players_) {
                tp.visibleEntities.erase(eid);
            }
        }
    }

    // Get players that need spawn/despawn/movement updates for a given observer
    struct EntityUpdate {
        enum Type { SPAWN, DESPAWN, MOVE, TELEPORT, LOOK };
        Type    type;
        int     targetFd;
        int32_t entityId;
    };

    std::vector<EntityUpdate> getUpdates(int observerFd) {
        std::vector<EntityUpdate> updates;
        auto obsIt = players_.find(observerFd);
        if (obsIt == players_.end()) return updates;

        auto& observer = obsIt->second;
        double ox = observer.tracked.posX;
        double oz = observer.tracked.posZ;

        for (auto& [otherFd, other] : players_) {
            if (otherFd == observerFd) continue;

            double dist = std::sqrt(
                (other.tracked.posX - ox) * (other.tracked.posX - ox) +
                (other.tracked.posZ - oz) * (other.tracked.posZ - oz)
            );

            bool inRange = dist <= VIEW_DISTANCE;
            bool wasVisible = observer.visibleEntities.count(other.entityId) > 0;

            if (inRange && !wasVisible) {
                // Spawn
                observer.visibleEntities.insert(other.entityId);
                updates.push_back({EntityUpdate::SPAWN, otherFd, other.entityId});
            } else if (!inRange && wasVisible) {
                // Despawn
                observer.visibleEntities.erase(other.entityId);
                updates.push_back({EntityUpdate::DESPAWN, otherFd, other.entityId});
            } else if (inRange && wasVisible) {
                // Check for movement
                auto& last = other.lastSent;
                auto& curr = other.tracked;
                bool moved = (last.posX != curr.posX || last.posY != curr.posY || last.posZ != curr.posZ);
                bool looked = (last.yaw != curr.yaw || last.pitch != curr.pitch);

                if (moved) {
                    int8_t dx, dy, dz;
                    if (last.calculateDelta(curr.posX, curr.posY, curr.posZ, dx, dy, dz)) {
                        updates.push_back({EntityUpdate::MOVE, otherFd, other.entityId});
                    } else {
                        updates.push_back({EntityUpdate::TELEPORT, otherFd, other.entityId});
                    }
                } else if (looked) {
                    updates.push_back({EntityUpdate::LOOK, otherFd, other.entityId});
                }
            }
        }

        return updates;
    }

    // Mark all last-sent positions as current (call after sending updates)
    void commitPositions() {
        for (auto& [fd, tp] : players_) {
            tp.lastSent = tp.tracked;
        }
    }

    // Get a tracked player by fd
    const TrackedPosition* getTracked(int fd) const {
        auto it = players_.find(fd);
        return it != players_.end() ? &it->second.tracked : nullptr;
    }

    const TrackedPosition* getLastSent(int fd) const {
        auto it = players_.find(fd);
        return it != players_.end() ? &it->second.lastSent : nullptr;
    }

    int32_t getEntityId(int fd) const {
        auto it = players_.find(fd);
        return it != players_.end() ? it->second.entityId : -1;
    }

private:
    struct TrackedPlayer {
        int     fd;
        int32_t entityId;
        TrackedPosition tracked;
        TrackedPosition lastSent;
        std::unordered_set<int32_t> visibleEntities;
    };

    std::unordered_map<int, TrackedPlayer> players_;
};

} // namespace mc
