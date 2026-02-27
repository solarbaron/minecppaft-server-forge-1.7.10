#pragma once
// MobSpawning — mob entity definitions, Spawn Mob packet, natural spawning.
// Ported from vanilla 1.7.10 EntityList (vh.java), SpawnerAnimals (aef.java),
// S0FPacketSpawnMob (gp2.java).
//
// Protocol 5 (1.7.10):
//   S→C 0x0F: Spawn Mob (entityId, type, x, y, z, yaw, pitch, headPitch,
//              velocityXYZ, metadata)
//
// Natural spawning:
//   - Mob cap per category: hostile=70, passive=10, water=5, ambient=15
//   - Spawning checks light, block type, and player proximity
//   - Hostile: light ≤ 7, ≥24 blocks from player
//   - Passive: only at world gen or on grass in light ≥ 9

#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include "networking/PacketBuffer.h"
#include "entity/EntityMetadata.h"

namespace mc {

// ============================================================
// Mob type IDs — vh.java (EntityList)
// ============================================================
namespace MobType {
    constexpr uint8_t CREEPER       = 50;
    constexpr uint8_t SKELETON      = 51;
    constexpr uint8_t SPIDER        = 52;
    constexpr uint8_t GIANT         = 53;
    constexpr uint8_t ZOMBIE        = 54;
    constexpr uint8_t SLIME         = 55;
    constexpr uint8_t GHAST         = 56;
    constexpr uint8_t ZOMBIE_PIG    = 57;
    constexpr uint8_t ENDERMAN      = 58;
    constexpr uint8_t CAVE_SPIDER   = 59;
    constexpr uint8_t SILVERFISH    = 60;
    constexpr uint8_t BLAZE         = 61;
    constexpr uint8_t MAGMA_CUBE    = 62;
    constexpr uint8_t ENDER_DRAGON  = 63;
    constexpr uint8_t WITHER        = 64;
    constexpr uint8_t BAT           = 65;
    constexpr uint8_t WITCH         = 66;
    constexpr uint8_t PIG           = 90;
    constexpr uint8_t SHEEP         = 91;
    constexpr uint8_t COW           = 92;
    constexpr uint8_t CHICKEN       = 93;
    constexpr uint8_t SQUID         = 94;
    constexpr uint8_t WOLF          = 95;
    constexpr uint8_t MOOSHROOM     = 96;
    constexpr uint8_t SNOWMAN       = 97;
    constexpr uint8_t OCELOT        = 98;
    constexpr uint8_t IRON_GOLEM    = 99;
    constexpr uint8_t HORSE         = 100;
    constexpr uint8_t VILLAGER      = 120;
}

// ============================================================
// Mob category for spawn caps — aef.java
// ============================================================
enum class MobCategory : uint8_t {
    HOSTILE,
    PASSIVE,
    WATER,
    AMBIENT
};

// ============================================================
// Mob definition
// ============================================================
struct MobDef {
    uint8_t     typeId;
    std::string name;
    MobCategory category;
    float       maxHealth;
    float       movementSpeed;
    float       attackDamage;
    float       width, height;
    int         spawnWeight;   // Relative spawn weight
    int         minGroup, maxGroup; // Pack size
};

// All vanilla mob spawn entries
inline const std::vector<MobDef>& getMobDefs() {
    static const std::vector<MobDef> mobs = {
        // Hostile (spawn in dark, despawn far from player)
        {MobType::CREEPER,      "Creeper",      MobCategory::HOSTILE,  20, 0.25f, 0,   0.6f, 1.7f, 100, 4, 4},
        {MobType::SKELETON,     "Skeleton",     MobCategory::HOSTILE,  20, 0.25f, 2,   0.6f, 1.99f,100, 4, 4},
        {MobType::SPIDER,       "Spider",       MobCategory::HOSTILE,  16, 0.3f,  2,   1.4f, 0.9f, 100, 4, 4},
        {MobType::ZOMBIE,       "Zombie",       MobCategory::HOSTILE,  20, 0.23f, 3,   0.6f, 1.95f,100, 4, 4},
        {MobType::SLIME,        "Slime",        MobCategory::HOSTILE,  16, 0.25f, 4,   2.04f,2.04f, 10, 4, 4},
        {MobType::ENDERMAN,     "Enderman",     MobCategory::HOSTILE,  40, 0.3f,  7,   0.6f, 2.9f,  10, 1, 4},
        {MobType::CAVE_SPIDER,  "CaveSpider",   MobCategory::HOSTILE,  12, 0.3f,  2,   0.7f, 0.5f,  10, 4, 4},
        {MobType::WITCH,        "Witch",        MobCategory::HOSTILE,  26, 0.25f, 0,   0.6f, 1.95f,  5, 1, 1},
        {MobType::SILVERFISH,   "Silverfish",   MobCategory::HOSTILE,   8, 0.25f, 1,   0.4f, 0.3f,  10, 4, 4},

        // Nether hostile
        {MobType::GHAST,        "Ghast",        MobCategory::HOSTILE,  10, 0.03f, 6,   4.0f, 4.0f,  50, 4, 4},
        {MobType::ZOMBIE_PIG,   "PigZombie",    MobCategory::HOSTILE,  20, 0.23f, 5,   0.6f, 1.95f,100, 4, 4},
        {MobType::BLAZE,        "Blaze",        MobCategory::HOSTILE,  20, 0.23f, 6,   0.6f, 1.8f,  10, 4, 4},
        {MobType::MAGMA_CUBE,   "LavaSlime",    MobCategory::HOSTILE,  16, 0.25f, 6,   2.04f,2.04f,  2, 4, 4},

        // Passive (spawn on grass, persist forever)
        {MobType::PIG,          "Pig",          MobCategory::PASSIVE,  10, 0.25f, 0,   0.9f, 0.9f,  12, 4, 4},
        {MobType::SHEEP,        "Sheep",        MobCategory::PASSIVE,  8,  0.25f, 0,   0.9f, 1.3f,  12, 4, 4},
        {MobType::COW,          "Cow",          MobCategory::PASSIVE,  10, 0.2f,  0,   0.9f, 1.4f,   8, 4, 4},
        {MobType::CHICKEN,      "Chicken",      MobCategory::PASSIVE,  4,  0.25f, 0,   0.4f, 0.7f,  10, 4, 4},
        {MobType::WOLF,         "Wolf",         MobCategory::PASSIVE,  8,  0.3f,  3,   0.6f, 0.85f,  5, 4, 4},
        {MobType::OCELOT,       "Ozelot",       MobCategory::PASSIVE,  10, 0.3f,  3,   0.6f, 0.7f,   2, 1, 3},
        {MobType::HORSE,        "EntityHorse",  MobCategory::PASSIVE,  30, 0.225f,0,   1.4f, 1.6f,   5, 2, 6},

        // Water
        {MobType::SQUID,        "Squid",        MobCategory::WATER,    10, 0.2f,  0,   0.95f,0.95f,  10, 4, 4},

        // Ambient
        {MobType::BAT,          "Bat",          MobCategory::AMBIENT,   6, 0.1f,  0,   0.5f, 0.9f,  10, 8, 8},
    };
    return mobs;
}

// Spawn caps per category — aef.java
namespace SpawnCap {
    constexpr int HOSTILE = 70;
    constexpr int PASSIVE = 10;
    constexpr int WATER   = 5;
    constexpr int AMBIENT = 15;

    inline int getCapForCategory(MobCategory cat) {
        switch (cat) {
            case MobCategory::HOSTILE: return HOSTILE;
            case MobCategory::PASSIVE: return PASSIVE;
            case MobCategory::WATER:   return WATER;
            case MobCategory::AMBIENT: return AMBIENT;
        }
        return 70;
    }
}

// ============================================================
// S→C 0x0F Spawn Mob — gp2.java
// ============================================================
struct SpawnMobPacket {
    int32_t entityId;
    uint8_t type;
    int32_t x, y, z;     // Fixed-point (value * 32)
    int8_t  yaw, pitch, headPitch;
    int16_t velocityX, velocityY, velocityZ;
    std::vector<MetadataEntry> metadata;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x0F);
        buf.writeVarInt(entityId);
        buf.writeByte(type);
        buf.writeInt(x);
        buf.writeInt(y);
        buf.writeInt(z);
        buf.writeByte(static_cast<uint8_t>(yaw));
        buf.writeByte(static_cast<uint8_t>(pitch));
        buf.writeByte(static_cast<uint8_t>(headPitch));
        buf.writeShort(velocityX);
        buf.writeShort(velocityY);
        buf.writeShort(velocityZ);
        DataWatcher::writeToBuffer(buf, metadata);
        return buf;
    }

    static SpawnMobPacket create(int32_t eid, uint8_t mobType,
                                  double wx, double wy, double wz,
                                  float yawDeg = 0, float pitchDeg = 0) {
        SpawnMobPacket pkt;
        pkt.entityId = eid;
        pkt.type = mobType;
        pkt.x = static_cast<int32_t>(wx * 32.0);
        pkt.y = static_cast<int32_t>(wy * 32.0);
        pkt.z = static_cast<int32_t>(wz * 32.0);
        pkt.yaw = static_cast<int8_t>(yawDeg / 360.0f * 256.0f);
        pkt.pitch = static_cast<int8_t>(pitchDeg / 360.0f * 256.0f);
        pkt.headPitch = pkt.yaw;
        pkt.velocityX = 0;
        pkt.velocityY = 0;
        pkt.velocityZ = 0;

        // Default mob metadata
        DataWatcher dw;
        dw.initMob();
        // Set mob-specific health
        for (auto& mob : getMobDefs()) {
            if (mob.typeId == mobType) {
                dw.setFloat(DataWatcher::IDX_HEALTH, mob.maxHealth);
                break;
            }
        }
        pkt.metadata = dw.getAll();
        return pkt;
    }
};

// ============================================================
// S→C 0x10 Spawn Experience Orb — gq2.java
// ============================================================
struct SpawnExpOrbPacket {
    int32_t entityId;
    int32_t x, y, z;  // Fixed-point
    int16_t count;     // XP amount

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x11);
        buf.writeVarInt(entityId);
        buf.writeInt(x);
        buf.writeInt(y);
        buf.writeInt(z);
        buf.writeShort(count);
        return buf;
    }

    static SpawnExpOrbPacket create(int32_t eid, double wx, double wy, double wz, int16_t xp) {
        return {eid,
                static_cast<int32_t>(wx * 32.0),
                static_cast<int32_t>(wy * 32.0),
                static_cast<int32_t>(wz * 32.0),
                xp};
    }
};

// ============================================================
// MobSpawnManager — natural spawning logic
// ============================================================
class MobSpawnManager {
public:
    struct SpawnedMob {
        int32_t entityId;
        uint8_t typeId;
        double  posX, posY, posZ;
        float   yaw;
        float   health;
        bool    persistent = false; // Named or player-interacted mobs
        bool    dead = false;
        int     ticksAlive = 0;
        int     despawnTimer = 0;
    };

    explicit MobSpawnManager(int64_t seed) : rng_(static_cast<uint32_t>(seed)) {}

    // Attempt natural spawning for a chunk
    // Returns mobs that should be spawned, respecting caps
    std::vector<SpawnedMob> attemptNaturalSpawns(
        int chunkX, int chunkZ,
        MobCategory category,
        int currentMobCount,
        int loadedChunkCount,
        int nextEntityId) {

        std::vector<SpawnedMob> spawned;
        int cap = SpawnCap::getCapForCategory(category) * loadedChunkCount / 289;
        if (currentMobCount >= cap) return spawned;

        // Pick random spawn position within chunk
        std::uniform_int_distribution<int> xzDist(0, 15);
        std::uniform_int_distribution<int> yDist(1, 255);

        int localX = xzDist(rng_);
        int localZ = xzDist(rng_);
        int worldX = chunkX * 16 + localX;
        int worldZ = chunkZ * 16 + localZ;
        int spawnY = yDist(rng_);

        // Get valid mobs for this category
        std::vector<const MobDef*> valid;
        for (auto& mob : getMobDefs()) {
            if (mob.category == category) {
                valid.push_back(&mob);
            }
        }
        if (valid.empty()) return spawned;

        // Weight-based selection
        int totalWeight = 0;
        for (auto* m : valid) totalWeight += m->spawnWeight;
        std::uniform_int_distribution<int> wDist(0, totalWeight - 1);
        int roll = wDist(rng_);
        const MobDef* selected = valid[0];
        int acc = 0;
        for (auto* m : valid) {
            acc += m->spawnWeight;
            if (roll < acc) { selected = m; break; }
        }

        // Spawn pack
        std::uniform_int_distribution<int> packDist(selected->minGroup, selected->maxGroup);
        int packSize = packDist(rng_);
        std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);

        for (int i = 0; i < packSize && currentMobCount + static_cast<int>(spawned.size()) < cap; ++i) {
            // Scatter within 6 blocks of pack center
            std::uniform_real_distribution<double> scatterDist(-6.0, 6.0);
            double sx = worldX + 0.5 + scatterDist(rng_);
            double sz = worldZ + 0.5 + scatterDist(rng_);

            SpawnedMob mob;
            mob.entityId = nextEntityId + static_cast<int32_t>(spawned.size());
            mob.typeId = selected->typeId;
            mob.posX = sx;
            mob.posY = static_cast<double>(spawnY) + 0.5;
            mob.posZ = sz;
            mob.yaw = angleDist(rng_);
            mob.health = selected->maxHealth;
            spawned.push_back(mob);
        }

        return spawned;
    }

    // Tick mob despawning — vanilla: despawn if >128 blocks from any player
    void tickDespawn(std::vector<SpawnedMob>& mobs,
                      const std::vector<std::pair<double, double>>& playerPositions) {
        for (auto& mob : mobs) {
            if (mob.persistent || mob.dead) continue;
            mob.ticksAlive++;

            double minDist = 1e9;
            for (auto& [px, pz] : playerPositions) {
                double dx = mob.posX - px;
                double dz = mob.posZ - pz;
                double dist = std::sqrt(dx * dx + dz * dz);
                if (dist < minDist) minDist = dist;
            }

            // >128 blocks from all players: instant despawn
            if (minDist > 128.0) {
                mob.dead = true;
                continue;
            }

            // >32 blocks: random despawn (1 in 800 per tick)
            if (minDist > 32.0) {
                mob.despawnTimer++;
                std::uniform_int_distribution<int> despawnDist(0, 799);
                if (despawnDist(rng_) == 0) {
                    mob.dead = true;
                }
            }
        }
    }

    // Get the mob definition for a type
    static const MobDef* getMobDef(uint8_t typeId) {
        for (auto& mob : getMobDefs()) {
            if (mob.typeId == typeId) return &mob;
        }
        return nullptr;
    }

private:
    std::mt19937 rng_;
};

} // namespace mc
