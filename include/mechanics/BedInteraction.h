#pragma once
// BedHandler — bed interaction, sleep cycle, and spawn point management.
// Ported from vanilla 1.7.10 bed mechanics (BlockBed / MCP: ahd.java).
//
// Protocol 5 packets used:
//   S→C 0x0A: Use Bed (entityId, x, y, z) — shows player sleeping
//   S→C 0x0B: Animation (entityId, animation) — wake up animation
//   S→C 0x1A: Entity Status (entityId, status) — entity events
//   C→S 0x0B: Entity Action (entityId, actionId) — leave bed action

#include <cstdint>
#include <string>
#include <unordered_map>
#include <cmath>
#include "networking/PacketBuffer.h"
#include "world/Block.h"

namespace mc {

// ============================================================
// S→C 0x0A Use Bed — gj.java
// ============================================================
struct UseBedPacket {
    int32_t entityId;
    int32_t x;
    int8_t  y;
    int32_t z;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x0A);
        buf.writeVarInt(entityId);
        buf.writeInt(x);
        buf.writeByte(static_cast<uint8_t>(y));
        buf.writeInt(z);
        return buf;
    }
};

// ============================================================
// S→C 0x0B Animation — gi.java
// ============================================================
struct AnimationPacket {
    int32_t entityId;
    uint8_t animation;  // 0=swing arm, 1=take damage, 2=leave bed, 3=eat, 4=crit, 5=magic crit

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x0B);
        buf.writeVarInt(entityId);
        buf.writeByte(animation);
        return buf;
    }

    // Animation IDs
    static constexpr uint8_t SWING_ARM    = 0;
    static constexpr uint8_t TAKE_DAMAGE  = 1;
    static constexpr uint8_t LEAVE_BED    = 2;
    static constexpr uint8_t EAT_FOOD     = 3;
    static constexpr uint8_t CRITICAL_HIT = 4;
    static constexpr uint8_t MAGIC_CRIT   = 5;
};

// ============================================================
// S→C 0x1A Entity Status — gr.java
// ============================================================
struct EntityStatusPacket {
    int32_t entityId;
    int8_t  status;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x1A);
        buf.writeInt(entityId);
        buf.writeByte(static_cast<uint8_t>(status));
        return buf;
    }

    // Status IDs from vanilla
    static constexpr int8_t HURT             = 2;
    static constexpr int8_t DEATH            = 3;
    static constexpr int8_t WOLF_TAMING      = 6;
    static constexpr int8_t WOLF_TAMED       = 7;
    static constexpr int8_t WOLF_SHAKE       = 8;
    static constexpr int8_t EATING_ACCEPTED  = 9;
    static constexpr int8_t SHEEP_EAT_GRASS  = 10;
    static constexpr int8_t IRON_GOLEM_ROSE  = 11;
    static constexpr int8_t VILLAGER_HEARTS  = 12;
    static constexpr int8_t VILLAGER_ANGRY   = 13;
    static constexpr int8_t VILLAGER_HAPPY   = 14;
    static constexpr int8_t WITCH_PARTICLES  = 15;
    static constexpr int8_t ZOMBIE_CONVERTING = 16;
    static constexpr int8_t FIREWORK_EXPLODE = 17;
};

// ============================================================
// S→C 0x12 Entity Velocity — gp.java
// ============================================================
struct EntityVelocityPacket {
    int32_t entityId;
    int16_t velocityX;  // velocity * 8000
    int16_t velocityY;
    int16_t velocityZ;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x12);
        buf.writeVarInt(entityId);
        buf.writeShort(velocityX);
        buf.writeShort(velocityY);
        buf.writeShort(velocityZ);
        return buf;
    }

    // Factory from world velocity (blocks/tick)
    static EntityVelocityPacket fromVelocity(int32_t eid, double vx, double vy, double vz) {
        // Clamp to ±3.9 blocks/tick to prevent overflow
        auto clamp = [](double v) -> int16_t {
            double clamped = std::max(-3.9, std::min(3.9, v));
            return static_cast<int16_t>(clamped * 8000.0);
        };
        return {eid, clamp(vx), clamp(vy), clamp(vz)};
    }
};

// ============================================================
// S→C 0x20 Entity Properties — hb.java
// ============================================================
struct EntityPropertiesPacket {
    int32_t entityId;

    struct Property {
        std::string key;
        double value;
    };
    std::vector<Property> properties;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x20);
        buf.writeVarInt(entityId);
        buf.writeInt(static_cast<int32_t>(properties.size()));
        for (auto& prop : properties) {
            buf.writeString(prop.key);
            buf.writeDouble(prop.value);
            buf.writeVarInt(0); // No modifiers
        }
        return buf;
    }

    // Common property keys
    static EntityPropertiesPacket playerDefaults(int32_t eid) {
        EntityPropertiesPacket pkt;
        pkt.entityId = eid;
        pkt.properties = {
            {"generic.maxHealth", 20.0},
            {"generic.movementSpeed", 0.10000000149011612},
            {"generic.knockbackResistance", 0.0},
            {"generic.attackDamage", 1.0},
        };
        return pkt;
    }
};

// ============================================================
// Bed handler — manages sleep state and spawn points
// ============================================================
class BedHandler {
public:
    struct SpawnPoint {
        int x, y, z;
    };

    // Check if a bed block is at the given position
    static bool isBed(uint16_t blockId) {
        return blockId == BlockID::BED;
    }

    // Try to put a player to sleep
    enum class SleepResult {
        OK,
        NOT_NIGHT,
        TOO_FAR,
        OCCUPIED,
        NOT_SAFE,
        NOT_BED
    };

    SleepResult trySleep(int entityId, int bedX, int bedY, int bedZ,
                          int64_t worldTime, uint16_t blockAtBed) {
        if (!isBed(blockAtBed)) return SleepResult::NOT_BED;

        // Check if it's night (12541 to 23458 ticks)
        int64_t timeOfDay = worldTime % 24000;
        if (timeOfDay < 12541 || timeOfDay > 23458) {
            return SleepResult::NOT_NIGHT;
        }

        // Check if bed is already occupied
        if (occupiedBeds_.count(packKey(bedX, bedY, bedZ))) {
            return SleepResult::OCCUPIED;
        }

        // Mark bed as occupied
        occupiedBeds_[packKey(bedX, bedY, bedZ)] = entityId;
        sleepingPlayers_[entityId] = {bedX, bedY, bedZ};

        return SleepResult::OK;
    }

    // Wake a player up
    void wakeUp(int entityId) {
        auto it = sleepingPlayers_.find(entityId);
        if (it != sleepingPlayers_.end()) {
            auto& sp = it->second;
            occupiedBeds_.erase(packKey(sp.x, sp.y, sp.z));
            sleepingPlayers_.erase(it);
        }
    }

    bool isSleeping(int entityId) const {
        return sleepingPlayers_.count(entityId) > 0;
    }

    // Set spawn point for a player
    void setSpawnPoint(int entityId, int x, int y, int z) {
        spawnPoints_[entityId] = {x, y, z};
    }

    const SpawnPoint* getSpawnPoint(int entityId) const {
        auto it = spawnPoints_.find(entityId);
        return it != spawnPoints_.end() ? &it->second : nullptr;
    }

    // Check all sleeping players — if all are sleeping, skip to dawn
    bool allPlayersSleeping(int totalPlayers) const {
        if (totalPlayers == 0) return false;
        return static_cast<int>(sleepingPlayers_.size()) >= totalPlayers;
    }

    // Get the wake-up position (1 block beside bed)
    static SpawnPoint getWakeUpPosition(int bedX, int bedY, int bedZ) {
        // Check adjacent blocks for a safe standing position
        // Vanilla checks the foot of bed + 1 block around
        return {bedX, bedY + 1, bedZ};
    }

private:
    static int64_t packKey(int x, int y, int z) {
        return (static_cast<int64_t>(x) & 0x3FFFFFF) |
               ((static_cast<int64_t>(z) & 0x3FFFFFF) << 26) |
               ((static_cast<int64_t>(y) & 0xFFF) << 52);
    }

    std::unordered_map<int64_t, int> occupiedBeds_;       // bedKey -> entityId
    std::unordered_map<int, SpawnPoint> sleepingPlayers_;  // entityId -> bed pos
    std::unordered_map<int, SpawnPoint> spawnPoints_;      // entityId -> spawn
};

} // namespace mc
