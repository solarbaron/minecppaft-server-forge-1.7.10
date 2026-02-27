#pragma once
// MobEntity — non-player entity with type, position, health.
// Entity type IDs from sg.java (EntityList).
// MobSpawner handles spawning passive/hostile mobs around players.

#include <cstdint>
#include <string>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>
#include <unordered_map>
#include <algorithm>

#include "nbt/NBT.h"

namespace mc {

// Entity type IDs — sg.java static initializer
namespace EntityTypeID {
    // Hostile
    constexpr uint8_t CREEPER     = 50;
    constexpr uint8_t SKELETON    = 51;
    constexpr uint8_t SPIDER      = 52;
    constexpr uint8_t GIANT       = 53;
    constexpr uint8_t ZOMBIE      = 54;
    constexpr uint8_t SLIME       = 55;
    constexpr uint8_t GHAST       = 56;
    constexpr uint8_t PIG_ZOMBIE  = 57;
    constexpr uint8_t ENDERMAN    = 58;
    constexpr uint8_t CAVE_SPIDER = 59;
    constexpr uint8_t SILVERFISH  = 60;
    constexpr uint8_t BLAZE       = 61;
    constexpr uint8_t MAGMA_CUBE  = 62;
    constexpr uint8_t ENDER_DRAGON= 63;
    constexpr uint8_t WITHER      = 64;
    constexpr uint8_t BAT         = 65;
    constexpr uint8_t WITCH       = 66;

    // Passive
    constexpr uint8_t PIG         = 90;
    constexpr uint8_t SHEEP       = 91;
    constexpr uint8_t COW         = 92;
    constexpr uint8_t CHICKEN     = 93;
    constexpr uint8_t SQUID       = 94;
    constexpr uint8_t WOLF        = 95;
    constexpr uint8_t MOOSHROOM   = 96;
    constexpr uint8_t SNOW_GOLEM  = 97;
    constexpr uint8_t OCELOT      = 98;
    constexpr uint8_t IRON_GOLEM  = 99;
    constexpr uint8_t HORSE       = 100;
    constexpr uint8_t VILLAGER    = 120;

    // Other
    constexpr uint8_t ENDER_CRYSTAL = 200;
}

// Non-player entity
struct MobEntity {
    int32_t entityId = 0;
    uint8_t typeId = 0;        // EntityTypeID

    // Position (fixed-point for protocol: val * 32)
    double posX = 0.0, posY = 0.0, posZ = 0.0;

    // Rotation (angle = val * 256/360)
    float yaw = 0.0f, pitch = 0.0f, headYaw = 0.0f;

    // Velocity (1/8000 b/t)
    int16_t velX = 0, velY = 0, velZ = 0;

    // Health
    float health = 20.0f;
    float maxHealth = 20.0f;

    // Flags
    bool onFire = false;
    bool crouching = false;
    bool noAI = false;

    // Metadata for WatchedObject protocol encoding
    // Returns entity metadata bytes for spawn packet
    std::vector<uint8_t> serializeMetadata() const {
        std::vector<uint8_t> data;

        // Index 0: flags byte (bit 0=onFire, bit 1=crouching, etc.)
        uint8_t flags = 0;
        if (onFire) flags |= 0x01;
        if (crouching) flags |= 0x02;
        data.push_back(0x00); // type=Byte (0<<5 | index=0)
        data.push_back(flags);

        // Index 6: health (float)
        data.push_back(0x06 | (3 << 5)); // type=Float (3<<5 | index=6)
        union { float f; uint8_t b[4]; } u;
        u.f = health;
        // Big endian
        data.push_back(u.b[3]);
        data.push_back(u.b[2]);
        data.push_back(u.b[1]);
        data.push_back(u.b[0]);

        // End marker
        data.push_back(0x7F);

        return data;
    }

    // Get the entity name for this type
    static const char* nameForType(uint8_t typeId) {
        switch (typeId) {
            case EntityTypeID::CREEPER:     return "Creeper";
            case EntityTypeID::SKELETON:    return "Skeleton";
            case EntityTypeID::SPIDER:      return "Spider";
            case EntityTypeID::ZOMBIE:      return "Zombie";
            case EntityTypeID::SLIME:       return "Slime";
            case EntityTypeID::ENDERMAN:    return "Enderman";
            case EntityTypeID::CAVE_SPIDER: return "CaveSpider";
            case EntityTypeID::WITCH:       return "Witch";
            case EntityTypeID::BAT:         return "Bat";
            case EntityTypeID::PIG:         return "Pig";
            case EntityTypeID::SHEEP:       return "Sheep";
            case EntityTypeID::COW:         return "Cow";
            case EntityTypeID::CHICKEN:     return "Chicken";
            case EntityTypeID::WOLF:        return "Wolf";
            case EntityTypeID::VILLAGER:    return "Villager";
            case EntityTypeID::HORSE:       return "Horse";
            default: return "Unknown";
        }
    }

    // Max health for each mob type
    static float maxHealthForType(uint8_t typeId) {
        switch (typeId) {
            case EntityTypeID::CREEPER:     return 20.0f;
            case EntityTypeID::SKELETON:    return 20.0f;
            case EntityTypeID::SPIDER:      return 16.0f;
            case EntityTypeID::ZOMBIE:      return 20.0f;
            case EntityTypeID::SLIME:       return 16.0f; // Large
            case EntityTypeID::ENDERMAN:    return 40.0f;
            case EntityTypeID::CAVE_SPIDER: return 12.0f;
            case EntityTypeID::WITCH:       return 26.0f;
            case EntityTypeID::BAT:         return 6.0f;
            case EntityTypeID::PIG:         return 10.0f;
            case EntityTypeID::SHEEP:       return 8.0f;
            case EntityTypeID::COW:         return 10.0f;
            case EntityTypeID::CHICKEN:     return 4.0f;
            case EntityTypeID::WOLF:        return 8.0f;
            case EntityTypeID::VILLAGER:    return 20.0f;
            case EntityTypeID::HORSE:       return 30.0f; // Varies
            case EntityTypeID::IRON_GOLEM:  return 100.0f;
            default: return 20.0f;
        }
    }
};

// Spawn Mob packet — 0x0F (S→C)
struct SpawnMobPacket {
    int32_t entityId;
    uint8_t type;
    int32_t x, y, z;     // Fixed-point (absolute * 32)
    int8_t yaw, pitch, headYaw;
    int16_t velX, velY, velZ;
    std::vector<uint8_t> metadata;

    static SpawnMobPacket fromMob(const MobEntity& mob) {
        SpawnMobPacket pkt;
        pkt.entityId = mob.entityId;
        pkt.type = mob.typeId;
        pkt.x = static_cast<int32_t>(mob.posX * 32.0);
        pkt.y = static_cast<int32_t>(mob.posY * 32.0);
        pkt.z = static_cast<int32_t>(mob.posZ * 32.0);
        pkt.yaw = static_cast<int8_t>(mob.yaw * 256.0f / 360.0f);
        pkt.pitch = static_cast<int8_t>(mob.pitch * 256.0f / 360.0f);
        pkt.headYaw = static_cast<int8_t>(mob.headYaw * 256.0f / 360.0f);
        pkt.velX = mob.velX;
        pkt.velY = mob.velY;
        pkt.velZ = mob.velZ;
        pkt.metadata = mob.serializeMetadata();
        return pkt;
    }

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x0F); // Packet ID
        buf.writeVarInt(entityId);
        buf.writeByte(type);
        buf.writeInt(x);
        buf.writeInt(y);
        buf.writeInt(z);
        buf.writeByte(static_cast<uint8_t>(yaw));
        buf.writeByte(static_cast<uint8_t>(pitch));
        buf.writeByte(static_cast<uint8_t>(headYaw));
        buf.writeShort(velX);
        buf.writeShort(velY);
        buf.writeShort(velZ);
        // Append raw metadata
        for (uint8_t b : metadata) {
            buf.writeByte(b);
        }
        return buf;
    }
};

// MobSpawner — handles natural mob spawning around players
class MobSpawner {
public:
    // Hostile mobs that can spawn at night / in dark
    static constexpr uint8_t HOSTILE_MOBS[] = {
        EntityTypeID::ZOMBIE,
        EntityTypeID::SKELETON,
        EntityTypeID::SPIDER,
        EntityTypeID::CREEPER,
        EntityTypeID::ENDERMAN,
        EntityTypeID::WITCH,
    };

    // Passive mobs that spawn in light
    static constexpr uint8_t PASSIVE_MOBS[] = {
        EntityTypeID::PIG,
        EntityTypeID::SHEEP,
        EntityTypeID::COW,
        EntityTypeID::CHICKEN,
    };

    // Maximum mobs in loaded chunks (vanilla: 70 hostile, 10 passive)
    static constexpr int MAX_HOSTILE = 70;
    static constexpr int MAX_PASSIVE = 10;

    // Spawn radius around players (blocks)
    static constexpr int SPAWN_RADIUS = 64;
    static constexpr int DESPAWN_RADIUS = 128;
    static constexpr int MIN_SPAWN_DIST = 24;

    // Get all currently alive mobs
    const std::unordered_map<int32_t, MobEntity>& mobs() const { return mobs_; }
    std::unordered_map<int32_t, MobEntity>& mobs() { return mobs_; }

    // Allocate next entity ID (shared with player ID counter)
    void setNextEntityId(std::atomic<int32_t>* nextId) { nextEntityId_ = nextId; }

    // Try to spawn mobs around a player position
    // Returns newly spawned mobs
    std::vector<MobEntity> trySpawnAround(double px, double py, double pz,
                                           int64_t worldTime) {
        std::vector<MobEntity> spawned;

        // Count existing mobs
        int hostileCount = 0, passiveCount = 0;
        for (auto& [id, mob] : mobs_) {
            if (mob.typeId >= 50 && mob.typeId < 70) ++hostileCount;
            else if (mob.typeId >= 90) ++passiveCount;
        }

        // Try spawning hostile (at night: worldTime 13000-23000) or dark areas
        bool isNight = (worldTime % 24000) >= 13000 && (worldTime % 24000) <= 23000;

        if (isNight && hostileCount < MAX_HOSTILE) {
            // Try a few spawn attempts
            for (int attempt = 0; attempt < 3; ++attempt) {
                auto mob = trySpawnMob(px, py, pz, true);
                if (mob) {
                    spawned.push_back(*mob);
                }
            }
        }

        // Passive mobs (always, but rare)
        if (passiveCount < MAX_PASSIVE && rng_() % 400 == 0) {
            auto mob = trySpawnMob(px, py, pz, false);
            if (mob) {
                spawned.push_back(*mob);
            }
        }

        return spawned;
    }

    // Despawn mobs too far from any player
    std::vector<int32_t> despawnFarFrom(double px, double py, double pz) {
        std::vector<int32_t> removed;
        for (auto it = mobs_.begin(); it != mobs_.end(); ) {
            auto& mob = it->second;
            double dx = mob.posX - px;
            double dy = mob.posY - py;
            double dz = mob.posZ - pz;
            double dist = std::sqrt(dx*dx + dy*dy + dz*dz);

            if (dist > DESPAWN_RADIUS) {
                removed.push_back(mob.entityId);
                it = mobs_.erase(it);
            } else {
                ++it;
            }
        }
        return removed;
    }

    // Register a spawned mob
    void registerMob(const MobEntity& mob) {
        mobs_[mob.entityId] = mob;
    }

private:
    std::unordered_map<int32_t, MobEntity> mobs_;
    std::atomic<int32_t>* nextEntityId_ = nullptr;
    std::mt19937 rng_{static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count())};

    std::optional<MobEntity> trySpawnMob(double px, double py, double pz,
                                         bool hostile) {
        if (!nextEntityId_) return std::nullopt;

        // Random offset from player
        std::uniform_real_distribution<double> offsetDist(MIN_SPAWN_DIST, SPAWN_RADIUS);
        std::uniform_real_distribution<double> angleDist(0, 2.0 * 3.14159265358979);

        double angle = angleDist(rng_);
        double distance = offsetDist(rng_);

        double spawnX = px + std::cos(angle) * distance;
        double spawnZ = pz + std::sin(angle) * distance;
        double spawnY = py; // Simplified: spawn at same Y as player

        // Pick random mob type
        const uint8_t* pool = hostile ? HOSTILE_MOBS : PASSIVE_MOBS;
        int poolSize = hostile ? 6 : 4;
        uint8_t typeId = pool[rng_() % poolSize];

        MobEntity mob;
        mob.entityId = (*nextEntityId_)++;
        mob.typeId = typeId;
        mob.posX = spawnX;
        mob.posY = spawnY;
        mob.posZ = spawnZ;
        mob.yaw = static_cast<float>(angleDist(rng_) * 360.0 / (2.0 * 3.14159265358979));
        mob.health = MobEntity::maxHealthForType(typeId);
        mob.maxHealth = mob.health;

        mobs_[mob.entityId] = mob;
        return mob;
    }
};

} // namespace mc
