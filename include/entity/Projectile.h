#pragma once
// Projectile — arrow, snowball, egg, ender pearl, potion entity management.
// Ported from vanilla 1.7.10 EntityArrow (yw.java), EntitySnowball (zo.java),
// EntityEgg (yh.java), EntityEnderPearl (yj.java), EntityPotion (zl.java).
//
// Protocol 5 packets:
//   S→C 0x0E: Spawn Object (entityId, type, x, y, z, velocityXYZ)
//   S→C 0x14: Entity (entityId) — keepalive for entity
//   S→C 0x15: Entity Relative Move (entityId, dx, dy, dz)
//   S→C 0x17: Entity Look + Relative Move
//   S→C 0x13: Destroy Entities (entityIds[])

#include <cstdint>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <random>
#include "networking/PacketBuffer.h"

namespace mc {

// ============================================================
// S→C 0x0E Spawn Object — gm.java
// ============================================================
struct SpawnObjectPacket {
    int32_t entityId;
    int8_t  type;         // 60=arrow, 61=snowball, 62=egg, 65=ender pearl, 73=potion
    int32_t x, y, z;      // Fixed-point (value * 32)
    int8_t  pitch, yaw;
    int32_t data;          // Object data (e.g. shooter entity ID for arrows)
    int16_t velocityX = 0, velocityY = 0, velocityZ = 0; // Only if data != 0

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x0E);
        buf.writeVarInt(entityId);
        buf.writeByte(static_cast<uint8_t>(type));
        buf.writeInt(x);
        buf.writeInt(y);
        buf.writeInt(z);
        buf.writeByte(static_cast<uint8_t>(pitch));
        buf.writeByte(static_cast<uint8_t>(yaw));
        buf.writeInt(data);
        if (data != 0) {
            buf.writeShort(velocityX);
            buf.writeShort(velocityY);
            buf.writeShort(velocityZ);
        }
        return buf;
    }

    // Object type IDs — from ee.java (EntityTracker)
    static constexpr int8_t BOAT          = 1;
    static constexpr int8_t MINECART      = 10;
    static constexpr int8_t TNT_PRIMED    = 50;
    static constexpr int8_t ENDER_CRYSTAL = 51;
    static constexpr int8_t ARROW         = 60;
    static constexpr int8_t SNOWBALL      = 61;
    static constexpr int8_t EGG           = 62;
    static constexpr int8_t FIREBALL      = 63;
    static constexpr int8_t FIRE_CHARGE   = 64;
    static constexpr int8_t ENDER_PEARL   = 65;
    static constexpr int8_t WITHER_SKULL  = 66;
    static constexpr int8_t FALLING_BLOCK = 70;
    static constexpr int8_t ITEM_FRAME    = 71;
    static constexpr int8_t EYE_OF_ENDER  = 72;
    static constexpr int8_t POTION        = 73;
    static constexpr int8_t EXP_BOTTLE    = 75;
    static constexpr int8_t FIREWORK      = 76;
    static constexpr int8_t FISHING_FLOAT = 90;

    // Factory from world coords
    static SpawnObjectPacket create(int32_t eid, int8_t objType,
                                     double wx, double wy, double wz,
                                     float pitchDeg, float yawDeg,
                                     int32_t objData,
                                     double vx = 0, double vy = 0, double vz = 0) {
        SpawnObjectPacket pkt;
        pkt.entityId = eid;
        pkt.type = objType;
        pkt.x = static_cast<int32_t>(wx * 32.0);
        pkt.y = static_cast<int32_t>(wy * 32.0);
        pkt.z = static_cast<int32_t>(wz * 32.0);
        pkt.pitch = static_cast<int8_t>(pitchDeg / 360.0f * 256.0f);
        pkt.yaw = static_cast<int8_t>(yawDeg / 360.0f * 256.0f);
        pkt.data = objData;
        if (objData != 0) {
            auto clamp = [](double v) -> int16_t {
                return static_cast<int16_t>(std::max(-3.9, std::min(3.9, v)) * 8000.0);
            };
            pkt.velocityX = clamp(vx);
            pkt.velocityY = clamp(vy);
            pkt.velocityZ = clamp(vz);
        }
        return pkt;
    }
};

// ============================================================
// S→C 0x13 Destroy Entities — ha.java
// ============================================================
struct DestroyEntitiesPacket {
    std::vector<int32_t> entityIds;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x13);
        buf.writeByte(static_cast<uint8_t>(entityIds.size()));
        for (auto eid : entityIds) {
            buf.writeVarInt(eid);
        }
        return buf;
    }
};

// ============================================================
// S→C 0x15 Entity Relative Move — gs.java
// ============================================================
struct EntityRelativeMovePacket {
    int32_t entityId;
    int8_t  dx, dy, dz;  // Fixed-point delta (value * 32)
    bool    onGround;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x15);
        buf.writeVarInt(entityId);
        buf.writeByte(static_cast<uint8_t>(dx));
        buf.writeByte(static_cast<uint8_t>(dy));
        buf.writeByte(static_cast<uint8_t>(dz));
        buf.writeBoolean(onGround);
        return buf;
    }
};

// ============================================================
// S→C 0x18 Entity Teleport — gw.java
// ============================================================
struct EntityTeleportPacket {
    int32_t entityId;
    int32_t x, y, z;   // Fixed-point (value * 32)
    int8_t  yaw, pitch;
    bool    onGround;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x18);
        buf.writeVarInt(entityId);
        buf.writeInt(x);
        buf.writeInt(y);
        buf.writeInt(z);
        buf.writeByte(static_cast<uint8_t>(yaw));
        buf.writeByte(static_cast<uint8_t>(pitch));
        buf.writeBoolean(onGround);
        return buf;
    }

    static EntityTeleportPacket fromPos(int32_t eid, double wx, double wy, double wz,
                                         float yawDeg, float pitchDeg) {
        return {
            eid,
            static_cast<int32_t>(wx * 32.0),
            static_cast<int32_t>(wy * 32.0),
            static_cast<int32_t>(wz * 32.0),
            static_cast<int8_t>(yawDeg / 360.0f * 256.0f),
            static_cast<int8_t>(pitchDeg / 360.0f * 256.0f),
            false
        };
    }
};

// ============================================================
// Projectile entity (runtime data)
// ============================================================
struct Projectile {
    int32_t entityId;
    int8_t  type;        // SpawnObjectPacket type constant
    int32_t shooterId;   // Entity that fired it
    double  posX, posY, posZ;
    double  motionX, motionY, motionZ;
    float   yaw, pitch;
    int     ticksAlive = 0;
    int     ticksInGround = 0;
    bool    inGround = false;
    bool    dead = false;

    // Arrow-specific
    int     damage = 2;     // Base arrow damage (vanilla: 2)
    int     knockback = 0;  // Arrow knockback level
    bool    isCritical = false;

    // Tick projectile physics — vanilla gravity + drag
    void tick() {
        if (dead) return;
        ticksAlive++;

        if (inGround) {
            ticksInGround++;
            // Arrows despawn after 1200 ticks (60 seconds) in ground
            if (type == SpawnObjectPacket::ARROW && ticksInGround >= 1200) {
                dead = true;
            }
            return;
        }

        // Apply motion
        posX += motionX;
        posY += motionY;
        posZ += motionZ;

        // Gravity — vanilla: 0.05 blocks/tick² for arrows, 0.03 for throwables
        double gravity = (type == SpawnObjectPacket::ARROW) ? 0.05 : 0.03;
        motionY -= gravity;

        // Air drag — vanilla: 0.99 for arrows, 0.99 for throwables
        double drag = 0.99;
        motionX *= drag;
        motionY *= drag;
        motionZ *= drag;

        // Update angles from motion vector
        double horizSpeed = std::sqrt(motionX * motionX + motionZ * motionZ);
        yaw = static_cast<float>(std::atan2(motionX, motionZ) * 180.0 / M_PI);
        pitch = static_cast<float>(std::atan2(motionY, horizSpeed) * 180.0 / M_PI);

        // Non-arrow projectiles die after 200 ticks (10 seconds airtime)
        if (type != SpawnObjectPacket::ARROW && ticksAlive >= 200) {
            dead = true;
        }
    }

    // Check if projectile hit a block at (bx, by, bz)
    bool checkBlockCollision(int bx, int by, int bz) const {
        int px = static_cast<int>(std::floor(posX));
        int py = static_cast<int>(std::floor(posY));
        int pz = static_cast<int>(std::floor(posZ));
        return px == bx && py == by && pz == bz;
    }

    // Calculate arrow damage (vanilla formula)
    float calculateDamage() const {
        double speed = std::sqrt(motionX * motionX + motionY * motionY + motionZ * motionZ);
        float dmg = static_cast<float>(speed * damage);
        if (isCritical) {
            // Random bonus: +1 to +damage/2+1
            dmg += static_cast<float>(damage) / 2.0f + 1.0f;
        }
        return dmg;
    }
};

// ============================================================
// ProjectileManager — tracks all active projectiles
// ============================================================
class ProjectileManager {
public:
    // Spawn an arrow from a player
    Projectile& spawnArrow(int32_t entityId, int32_t shooterId,
                            double x, double y, double z,
                            float yaw, float pitch, float force) {
        Projectile proj;
        proj.entityId = entityId;
        proj.type = SpawnObjectPacket::ARROW;
        proj.shooterId = shooterId;
        proj.posX = x;
        proj.posY = y;
        proj.posZ = z;
        proj.yaw = yaw;
        proj.pitch = pitch;
        proj.isCritical = (force >= 1.0f);

        // Convert yaw/pitch to velocity — vanilla arrow launch
        double radPitch = -pitch * M_PI / 180.0;
        double radYaw = -yaw * M_PI / 180.0;
        double speed = force * 3.0; // Max 3.0 blocks/tick at full charge
        proj.motionX = -std::sin(radYaw) * std::cos(radPitch) * speed;
        proj.motionY = std::sin(radPitch) * speed;
        proj.motionZ = std::cos(radYaw) * std::cos(radPitch) * speed;

        projectiles_[entityId] = proj;
        return projectiles_[entityId];
    }

    // Spawn a generic throwable (snowball, egg, ender pearl)
    Projectile& spawnThrowable(int32_t entityId, int8_t type, int32_t shooterId,
                                double x, double y, double z,
                                float yaw, float pitch) {
        Projectile proj;
        proj.entityId = entityId;
        proj.type = type;
        proj.shooterId = shooterId;
        proj.posX = x;
        proj.posY = y;
        proj.posZ = z;
        proj.yaw = yaw;
        proj.pitch = pitch;

        double radPitch = -pitch * M_PI / 180.0;
        double radYaw = -yaw * M_PI / 180.0;
        double speed = 1.5; // Throwable speed
        proj.motionX = -std::sin(radYaw) * std::cos(radPitch) * speed;
        proj.motionY = std::sin(radPitch) * speed;
        proj.motionZ = std::cos(radYaw) * std::cos(radPitch) * speed;

        projectiles_[entityId] = proj;
        return projectiles_[entityId];
    }

    // Tick all projectiles
    void tickAll() {
        std::vector<int32_t> toRemove;
        for (auto& [id, proj] : projectiles_) {
            proj.tick();
            if (proj.dead) {
                toRemove.push_back(id);
            }
        }
        for (auto id : toRemove) {
            projectiles_.erase(id);
        }
    }

    // Get a projectile by ID
    Projectile* get(int32_t entityId) {
        auto it = projectiles_.find(entityId);
        return it != projectiles_.end() ? &it->second : nullptr;
    }

    // Remove a projectile
    void remove(int32_t entityId) {
        projectiles_.erase(entityId);
    }

    // Get all active projectiles
    const std::unordered_map<int32_t, Projectile>& all() const {
        return projectiles_;
    }

    // Get dead projectile IDs for destroy packet
    std::vector<int32_t> collectDead() {
        std::vector<int32_t> dead;
        for (auto it = projectiles_.begin(); it != projectiles_.end(); ) {
            if (it->second.dead) {
                dead.push_back(it->first);
                it = projectiles_.erase(it);
            } else {
                ++it;
            }
        }
        return dead;
    }

    // Create spawn packet for a projectile
    static SpawnObjectPacket makeSpawnPacket(const Projectile& proj) {
        return SpawnObjectPacket::create(
            proj.entityId, proj.type,
            proj.posX, proj.posY, proj.posZ,
            proj.pitch, proj.yaw,
            proj.shooterId,
            proj.motionX, proj.motionY, proj.motionZ
        );
    }

private:
    std::unordered_map<int32_t, Projectile> projectiles_;
};

} // namespace mc
