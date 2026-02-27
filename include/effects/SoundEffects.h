#pragma once
// SoundEffects — particle and sound effect packets + world events.
// Ported from ir.java (S2APacketParticles), gk.java (S28PacketEffect),
// and go.java (S29PacketSoundEffect).
//
// Protocol 5 (1.7.10):
//   0x2A — Particle (particleName, x, y, z, offsetXYZ, speed, count)
//   0x28 — Effect (effectId, x, y, z, data, disableRelativeVolume)
//   0x29 — Sound Effect (soundName, x*8, y*8, z*8, volume, pitch)

#include <cstdint>
#include <string>
#include <vector>
#include "networking/PacketBuffer.h"

namespace mc {

// ============================================================
// S→C 0x2A Particle — ir.java
// ============================================================
struct ParticlePacket {
    std::string particleName;  // e.g. "explode", "flame", "heart"
    float x, y, z;
    float offsetX, offsetY, offsetZ;
    float particleSpeed;
    int32_t numberOfParticles;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x2A);
        buf.writeString(particleName);
        buf.writeFloat(x);
        buf.writeFloat(y);
        buf.writeFloat(z);
        buf.writeFloat(offsetX);
        buf.writeFloat(offsetY);
        buf.writeFloat(offsetZ);
        buf.writeFloat(particleSpeed);
        buf.writeInt(numberOfParticles);
        return buf;
    }

    // Common factory methods
    static ParticlePacket explosion(float px, float py, float pz) {
        return {"explode", px, py, pz, 0.3f, 0.3f, 0.3f, 0.0f, 10};
    }

    static ParticlePacket flame(float px, float py, float pz) {
        return {"flame", px, py, pz, 0.1f, 0.1f, 0.1f, 0.02f, 5};
    }

    static ParticlePacket heart(float px, float py, float pz) {
        return {"heart", px, py, pz, 0.5f, 0.5f, 0.5f, 0.0f, 3};
    }

    static ParticlePacket smoke(float px, float py, float pz) {
        return {"smoke", px, py, pz, 0.2f, 0.2f, 0.2f, 0.01f, 8};
    }

    static ParticlePacket blockBreak(float px, float py, float pz, int blockId) {
        return {"blockcrack_" + std::to_string(blockId) + "_0",
                px, py, pz, 0.3f, 0.3f, 0.3f, 0.05f, 30};
    }

    static ParticlePacket criticalHit(float px, float py, float pz) {
        return {"crit", px, py, pz, 0.5f, 0.5f, 0.5f, 0.1f, 10};
    }

    static ParticlePacket enchantHit(float px, float py, float pz) {
        return {"magicCrit", px, py, pz, 0.5f, 0.5f, 0.5f, 0.1f, 10};
    }

    static ParticlePacket portal(float px, float py, float pz) {
        return {"portal", px, py, pz, 0.5f, 1.0f, 0.5f, 1.0f, 20};
    }

    static ParticlePacket splash(float px, float py, float pz) {
        return {"splash", px, py, pz, 0.3f, 0.1f, 0.3f, 0.0f, 15};
    }

    static ParticlePacket note(float px, float py, float pz) {
        return {"note", px, py, pz, 0.0f, 0.0f, 0.0f, 1.0f, 1};
    }
};

// ============================================================
// S→C 0x28 Effect — gk.java
// ============================================================
// World effects (sounds + particles combined, position-based)
struct EffectPacket {
    int32_t effectId;
    int32_t x;
    int8_t  y;
    int32_t z;
    int32_t data;
    bool    disableRelativeVolume;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x28);
        buf.writeInt(effectId);
        buf.writeInt(x);
        buf.writeByte(static_cast<uint8_t>(y));
        buf.writeInt(z);
        buf.writeInt(data);
        buf.writeBoolean(disableRelativeVolume);
        return buf;
    }

    // Effect IDs — from aho.java (World)
    // Sound effects (1000-1013)
    static constexpr int32_t CLICK            = 1000;
    static constexpr int32_t CLICK_FAIL       = 1001;
    static constexpr int32_t SHOOT_ARROW      = 1002;
    static constexpr int32_t DOOR_TOGGLE      = 1003;
    static constexpr int32_t EXTINGUISH       = 1004;
    static constexpr int32_t PLAY_RECORD      = 1005;
    static constexpr int32_t GHAST_CHARGE     = 1007;
    static constexpr int32_t GHAST_SHOOT      = 1008;
    static constexpr int32_t BLAZE_SHOOT      = 1009;
    static constexpr int32_t ZOMBIE_DOOR      = 1010;
    static constexpr int32_t ZOMBIE_IRON_DOOR = 1011;
    static constexpr int32_t ZOMBIE_BREAK     = 1012;
    static constexpr int32_t WITHER_SHOOT     = 1013;

    // Particle effects (2000-2006)
    static constexpr int32_t SMOKE            = 2000; // data = direction 0-8
    static constexpr int32_t BLOCK_BREAK      = 2001; // data = block ID
    static constexpr int32_t SPLASH_POTION    = 2002; // data = potion ID
    static constexpr int32_t ENDER_EYE        = 2003;
    static constexpr int32_t MOB_SPAWN        = 2004; // Spawner flames
    static constexpr int32_t BONE_MEAL        = 2005; // data = count
    static constexpr int32_t DRAGON_BREATH    = 2006;

    // Factory methods
    static EffectPacket blockBreak(int bx, int by, int bz, int blockId) {
        return {BLOCK_BREAK, bx, static_cast<int8_t>(by), bz, blockId, false};
    }

    static EffectPacket smokeEffect(int bx, int by, int bz, int direction) {
        return {SMOKE, bx, static_cast<int8_t>(by), bz, direction, false};
    }

    static EffectPacket boneMeal(int bx, int by, int bz) {
        return {BONE_MEAL, bx, static_cast<int8_t>(by), bz, 0, false};
    }
};

// ============================================================
// S→C 0x29 Named Sound Effect — go.java
// ============================================================
struct SoundEffectPacket {
    std::string soundName;
    int32_t     x;      // Fixed point (x * 8)
    int32_t     y;      // Fixed point (y * 8)
    int32_t     z;      // Fixed point (z * 8)
    float       volume; // 1.0 = 100%
    uint8_t     pitch;  // 63 = 100%, range 0..255

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x29);
        buf.writeString(soundName);
        buf.writeInt(x);
        buf.writeInt(y);
        buf.writeInt(z);
        buf.writeFloat(volume);
        buf.writeByte(pitch);
        return buf;
    }

    // Factory: create from world coordinates
    static SoundEffectPacket at(const std::string& sound, double wx, double wy, double wz,
                                 float vol = 1.0f, float pitchMult = 1.0f) {
        SoundEffectPacket pkt;
        pkt.soundName = sound;
        pkt.x = static_cast<int32_t>(wx * 8.0);
        pkt.y = static_cast<int32_t>(wy * 8.0);
        pkt.z = static_cast<int32_t>(wz * 8.0);
        pkt.volume = vol;
        pkt.pitch = static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, pitchMult * 63.0f)));
        return pkt;
    }

    // Common sound names — from ahr.java (SoundEvents)
    static SoundEffectPacket blockDig(double x, double y, double z) {
        return at("random.break", x, y, z, 1.0f, 1.0f);
    }

    static SoundEffectPacket blockPlace(double x, double y, double z) {
        return at("random.wood_click", x, y, z, 1.0f, 0.8f);
    }

    static SoundEffectPacket playerHurt(double x, double y, double z) {
        return at("game.player.hurt", x, y, z, 1.0f, 1.0f);
    }

    static SoundEffectPacket entityExplode(double x, double y, double z) {
        return at("random.explode", x, y, z, 4.0f, 0.9f);
    }

    static SoundEffectPacket levelUp(double x, double y, double z) {
        return at("random.levelup", x, y, z, 0.75f, 1.0f);
    }

    static SoundEffectPacket playerBurp(double x, double y, double z) {
        return at("random.burp", x, y, z, 0.5f, 1.0f);
    }

    static SoundEffectPacket doorOpen(double x, double y, double z) {
        return at("random.door_open", x, y, z, 1.0f, 1.0f);
    }

    static SoundEffectPacket doorClose(double x, double y, double z) {
        return at("random.door_close", x, y, z, 1.0f, 1.0f);
    }

    static SoundEffectPacket chestOpen(double x, double y, double z) {
        return at("random.chestopen", x, y, z, 0.5f, 1.0f);
    }

    static SoundEffectPacket chestClose(double x, double y, double z) {
        return at("random.chestclosed", x, y, z, 0.5f, 1.0f);
    }

    static SoundEffectPacket anvilUse(double x, double y, double z) {
        return at("random.anvil_use", x, y, z, 0.5f, 1.0f);
    }

    static SoundEffectPacket anvilBreak(double x, double y, double z) {
        return at("random.anvil_break", x, y, z, 0.8f, 1.0f);
    }

    static SoundEffectPacket splash(double x, double y, double z) {
        return at("liquid.splash", x, y, z, 0.4f, 1.0f);
    }

    static SoundEffectPacket fizz(double x, double y, double z) {
        return at("random.fizz", x, y, z, 0.5f, 2.6f);
    }

    static SoundEffectPacket noteBlock(double x, double y, double z, float pitchMult) {
        return at("note.harp", x, y, z, 3.0f, pitchMult);
    }
};

} // namespace mc
