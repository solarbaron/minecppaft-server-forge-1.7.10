/**
 * PacketBuilder.h — Packet serialization for Protocol v5 (1.7.10).
 *
 * Reference: wiki.vg Protocol page for 1.7.10
 * Java references:
 *   - net.minecraft.network.play.server.S01PacketJoinGame
 *   - net.minecraft.network.play.server.S05PacketSpawnPosition
 *   - net.minecraft.network.play.server.S08PacketPlayerPosLook
 *   - net.minecraft.network.play.server.S00PacketKeepAlive
 *   - net.minecraft.network.play.server.S02PacketChat
 *   - net.minecraft.network.play.server.S03PacketTimeUpdate
 *   - net.minecraft.network.play.server.S06PacketUpdateHealth
 *   - net.minecraft.network.play.server.S07PacketRespawn
 *   - net.minecraft.network.play.server.S2BPacketChangeGameState
 *   - net.minecraft.network.play.server.S40PacketDisconnect
 *   - net.minecraft.network.play.server.S38PacketPlayerListItem
 *   - net.minecraft.network.play.server.S39PacketPlayerAbilities
 *   - net.minecraft.network.play.server.S1FPacketSetExperience
 *   - net.minecraft.network.play.server.S09PacketHeldItemChange
 *
 * All methods write big-endian, VarInt-prefixed packets matching the
 * exact wire format of 1.7.10 protocol version 5.
 *
 * Thread safety: Stateless builders — each returns an independent buffer.
 */
#pragma once

#include "PlayPackets.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// PacketWriter — Low-level binary writer for building packet payloads.
// Writes big-endian, matching Java's DataOutputStream.
// ═══════════════════════════════════════════════════════════════════════════

class PacketWriter {
public:
    PacketWriter() { buf_.reserve(128); }
    explicit PacketWriter(int32_t packetId) { buf_.reserve(128); writeVarInt(packetId); }

    // ─── Primitive writes ───

    void writeByte(int8_t v) { buf_.push_back(static_cast<uint8_t>(v)); }
    void writeUByte(uint8_t v) { buf_.push_back(v); }

    void writeBool(bool v) { buf_.push_back(v ? 1 : 0); }

    void writeShort(int16_t v) {
        buf_.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
        buf_.push_back(static_cast<uint8_t>(v & 0xFF));
    }

    void writeInt(int32_t v) {
        buf_.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
        buf_.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
        buf_.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
        buf_.push_back(static_cast<uint8_t>(v & 0xFF));
    }

    void writeLong(int64_t v) {
        for (int i = 56; i >= 0; i -= 8) {
            buf_.push_back(static_cast<uint8_t>((v >> i) & 0xFF));
        }
    }

    void writeFloat(float v) {
        uint32_t bits;
        std::memcpy(&bits, &v, sizeof(bits));
        writeInt(static_cast<int32_t>(bits));
    }

    void writeDouble(double v) {
        uint64_t bits;
        std::memcpy(&bits, &v, sizeof(bits));
        writeLong(static_cast<int64_t>(bits));
    }

    void writeVarInt(int32_t value) {
        uint32_t uv = static_cast<uint32_t>(value);
        while (uv >= 0x80) {
            buf_.push_back(static_cast<uint8_t>(uv & 0x7F) | 0x80);
            uv >>= 7;
        }
        buf_.push_back(static_cast<uint8_t>(uv));
    }

    void writeString(const std::string& s) {
        writeVarInt(static_cast<int32_t>(s.size()));
        buf_.insert(buf_.end(), s.begin(), s.end());
    }

    void writeBytes(const uint8_t* data, size_t len) {
        buf_.insert(buf_.end(), data, data + len);
    }

    void writeBytes(const std::vector<uint8_t>& data) {
        buf_.insert(buf_.end(), data.begin(), data.end());
    }

    // ─── Angle (rotation in 256ths of a circle) ───
    void writeAngle(float degrees) {
        buf_.push_back(static_cast<uint8_t>(static_cast<int32_t>(degrees * 256.0f / 360.0f) & 0xFF));
    }

    // ─── Fixed-point position (1/32 of a block = multiply by 32) ───
    void writeFixedPoint(double v) {
        writeInt(static_cast<int32_t>(std::floor(v * 32.0)));
    }

    // ─── Access ───
    const std::vector<uint8_t>& data() const { return buf_; }
    std::vector<uint8_t>& data() { return buf_; }
    size_t size() const { return buf_.size(); }

    // ─── Frame with VarInt length prefix ───
    // Returns the complete framed packet: [VarInt length][payload]
    std::vector<uint8_t> toFramed() const {
        std::vector<uint8_t> framed;
        framed.reserve(buf_.size() + 5);
        // Write length as VarInt
        uint32_t len = static_cast<uint32_t>(buf_.size());
        while (len >= 0x80) {
            framed.push_back(static_cast<uint8_t>(len & 0x7F) | 0x80);
            len >>= 7;
        }
        framed.push_back(static_cast<uint8_t>(len));
        framed.insert(framed.end(), buf_.begin(), buf_.end());
        return framed;
    }

private:
    std::vector<uint8_t> buf_;
};

// ═══════════════════════════════════════════════════════════════════════════
// Clientbound Packet Builders — Static factory methods.
// Each returns a fully serialized, length-prefixed packet ready to send.
// ═══════════════════════════════════════════════════════════════════════════

namespace PacketBuilder {

    // ─── 0x00 Keep Alive ───
    // Java: S00PacketKeepAlive — VarInt keepAliveId
    inline std::vector<uint8_t> keepAlive(int32_t keepAliveId) {
        PacketWriter w(ClientboundPacket::KeepAlive);
        w.writeVarInt(keepAliveId);
        return w.toFramed();
    }

    // ─── 0x01 Join Game ───
    // Java: S01PacketJoinGame
    inline std::vector<uint8_t> joinGame(int32_t entityId, uint8_t gamemode, int8_t dimension,
                                          uint8_t difficulty, uint8_t maxPlayers,
                                          const std::string& levelType) {
        PacketWriter w(ClientboundPacket::JoinGame);
        w.writeInt(entityId);       // Entity ID
        w.writeUByte(gamemode);     // Gamemode (0=survival, 1=creative, 2=adventure, bit 3=hardcore)
        w.writeByte(dimension);     // Dimension (-1=nether, 0=overworld, 1=end)
        w.writeUByte(difficulty);   // Difficulty (0-3)
        w.writeUByte(maxPlayers);   // Max players (used for tab list)
        w.writeString(levelType);   // Level type ("default", "flat", "largeBiomes", "amplified")
        return w.toFramed();
    }

    // ─── 0x02 Chat Message ───
    // Java: S02PacketChat — JSON chat component
    inline std::vector<uint8_t> chatMessage(const std::string& jsonText) {
        PacketWriter w(ClientboundPacket::ChatMessage);
        w.writeString(jsonText);
        return w.toFramed();
    }

    // ─── 0x03 Time Update ───
    // Java: S03PacketTimeUpdate
    inline std::vector<uint8_t> timeUpdate(int64_t worldAge, int64_t timeOfDay) {
        PacketWriter w(ClientboundPacket::TimeUpdate);
        w.writeLong(worldAge);
        w.writeLong(timeOfDay);
        return w.toFramed();
    }

    // ─── 0x05 Spawn Position ───
    // Java: S05PacketSpawnPosition
    inline std::vector<uint8_t> spawnPosition(int32_t x, int32_t y, int32_t z) {
        PacketWriter w(ClientboundPacket::SpawnPosition);
        w.writeInt(x);
        w.writeInt(y);
        w.writeInt(z);
        return w.toFramed();
    }

    // ─── 0x06 Update Health ───
    // Java: S06PacketUpdateHealth
    inline std::vector<uint8_t> updateHealth(float health, int32_t food, float saturation) {
        PacketWriter w(ClientboundPacket::UpdateHealth);
        w.writeFloat(health);
        w.writeVarInt(food);
        w.writeFloat(saturation);
        return w.toFramed();
    }

    // ─── 0x07 Respawn ───
    // Java: S07PacketRespawn
    inline std::vector<uint8_t> respawn(int32_t dimension, uint8_t difficulty,
                                         uint8_t gamemode, const std::string& levelType) {
        PacketWriter w(ClientboundPacket::Respawn);
        w.writeInt(dimension);
        w.writeUByte(difficulty);
        w.writeUByte(gamemode);
        w.writeString(levelType);
        return w.toFramed();
    }

    // ─── 0x08 Player Position And Look ───
    // Java: S08PacketPlayerPosLook
    inline std::vector<uint8_t> playerPosAndLook(double x, double y, double z,
                                                   float yaw, float pitch, bool onGround) {
        PacketWriter w(ClientboundPacket::PlayerPosAndLook);
        w.writeDouble(x);
        w.writeDouble(y);
        w.writeDouble(z);
        w.writeFloat(yaw);
        w.writeFloat(pitch);
        w.writeBool(onGround);
        return w.toFramed();
    }

    // ─── 0x09 Held Item Change ───
    // Java: S09PacketHeldItemChange
    inline std::vector<uint8_t> heldItemChange(int8_t slot) {
        PacketWriter w(ClientboundPacket::HeldItemChange);
        w.writeByte(slot);
        return w.toFramed();
    }

    // ─── 0x1F Set Experience ───
    // Java: S1FPacketSetExperience
    inline std::vector<uint8_t> setExperience(float experienceBar, int32_t level,
                                                int32_t totalExperience) {
        PacketWriter w(ClientboundPacket::SetExperience);
        w.writeFloat(experienceBar);
        w.writeVarInt(level);
        w.writeVarInt(totalExperience);
        return w.toFramed();
    }

    // ─── 0x2B Change Game State ───
    // Java: S2BPacketChangeGameState
    // reason: 1=rain_start, 2=rain_end, 3=gamemode, 4=enter_credits, etc
    inline std::vector<uint8_t> changeGameState(uint8_t reason, float value) {
        PacketWriter w(ClientboundPacket::ChangeGameState);
        w.writeUByte(reason);
        w.writeFloat(value);
        return w.toFramed();
    }

    // ─── 0x38 Player List Item ───
    // Java: S38PacketPlayerListItem
    // 1.7.10: string playerName, bool online, short ping
    inline std::vector<uint8_t> playerListItem(const std::string& playerName,
                                                 bool online, int16_t ping) {
        PacketWriter w(ClientboundPacket::PlayerListItem);
        w.writeString(playerName);
        w.writeBool(online);
        w.writeShort(ping);
        return w.toFramed();
    }

    // ─── 0x39 Player Abilities ───
    // Java: S39PacketPlayerAbilities
    // flags: bit 0=invulnerable, 1=flying, 2=allowFlying, 3=creativeMode
    inline std::vector<uint8_t> playerAbilities(uint8_t flags, float flySpeed,
                                                  float walkSpeed) {
        PacketWriter w(ClientboundPacket::PlayerAbilities);
        w.writeUByte(flags);
        w.writeFloat(flySpeed);
        w.writeFloat(walkSpeed);
        return w.toFramed();
    }

    // ─── 0x40 Disconnect ───
    // Java: S40PacketDisconnect — JSON reason
    inline std::vector<uint8_t> disconnect(const std::string& jsonReason) {
        PacketWriter w(ClientboundPacket::Disconnect);
        w.writeString(jsonReason);
        return w.toFramed();
    }

    // ─── 0x13 Destroy Entities ───
    // Java: S13PacketDestroyEntities
    inline std::vector<uint8_t> destroyEntities(const std::vector<int32_t>& entityIds) {
        PacketWriter w(ClientboundPacket::DestroyEntities);
        w.writeVarInt(static_cast<int32_t>(entityIds.size()));
        for (int32_t id : entityIds) {
            w.writeVarInt(id);
        }
        return w.toFramed();
    }

    // ─── 0x12 Entity Velocity ───
    // Java: S12PacketEntityVelocity
    // velocity = clamped to [-3.9, 3.9], sent as short = (int)(v * 8000)
    inline std::vector<uint8_t> entityVelocity(int32_t entityId, double vx, double vy, double vz) {
        PacketWriter w(ClientboundPacket::EntityVelocity);
        w.writeInt(entityId);
        auto clamp = [](double v) -> int16_t {
            double c = std::max(-3.9, std::min(3.9, v));
            return static_cast<int16_t>(c * 8000.0);
        };
        w.writeShort(clamp(vx));
        w.writeShort(clamp(vy));
        w.writeShort(clamp(vz));
        return w.toFramed();
    }

    // ─── 0x18 Entity Teleport ───
    // Java: S18PacketEntityTeleport
    inline std::vector<uint8_t> entityTeleport(int32_t entityId, double x, double y, double z,
                                                 float yaw, float pitch) {
        PacketWriter w(ClientboundPacket::EntityTeleport);
        w.writeVarInt(entityId);
        w.writeFixedPoint(x);
        w.writeFixedPoint(y);
        w.writeFixedPoint(z);
        w.writeAngle(yaw);
        w.writeAngle(pitch);
        return w.toFramed();
    }

    // ─── 0x19 Entity Head Look ───
    // Java: S19PacketEntityHeadLook
    inline std::vector<uint8_t> entityHeadLook(int32_t entityId, float yaw) {
        PacketWriter w(ClientboundPacket::EntityHeadLook);
        w.writeVarInt(entityId);
        w.writeAngle(yaw);
        return w.toFramed();
    }

    // ─── 0x1A Entity Status ───
    // Java: S19PacketEntityStatus
    inline std::vector<uint8_t> entityStatus(int32_t entityId, int8_t status) {
        PacketWriter w(ClientboundPacket::EntityStatus);
        w.writeInt(entityId);
        w.writeByte(status);
        return w.toFramed();
    }

    // ─── 0x23 Block Change ───
    // Java: S23PacketBlockChange
    inline std::vector<uint8_t> blockChange(int32_t x, uint8_t y, int32_t z,
                                              int32_t blockId, uint8_t metadata) {
        PacketWriter w(ClientboundPacket::BlockChange);
        w.writeInt(x);
        w.writeUByte(y);
        w.writeInt(z);
        w.writeVarInt(blockId);
        w.writeUByte(metadata);
        return w.toFramed();
    }

    // ─── 0x28 Effect (world event) ───
    // Java: S28PacketEffect
    inline std::vector<uint8_t> effect(int32_t effectId, int32_t x, uint8_t y, int32_t z,
                                        int32_t data, bool disableRelativeVolume) {
        PacketWriter w(ClientboundPacket::Effect);
        w.writeInt(effectId);
        w.writeInt(x);
        w.writeUByte(y);
        w.writeInt(z);
        w.writeInt(data);
        w.writeBool(disableRelativeVolume);
        return w.toFramed();
    }

    // ─── 0x29 Sound Effect ───
    // Java: S29PacketSoundEffect
    // position = fixed-point * 8
    inline std::vector<uint8_t> soundEffect(const std::string& soundName,
                                              double x, double y, double z,
                                              float volume, float pitch) {
        PacketWriter w(ClientboundPacket::SoundEffect);
        w.writeString(soundName);
        w.writeInt(static_cast<int32_t>(x * 8.0));
        w.writeInt(static_cast<int32_t>(y * 8.0));
        w.writeInt(static_cast<int32_t>(z * 8.0));
        w.writeFloat(volume);
        // Pitch is sent as unsigned byte * 63 in some implementations,
        // but 1.7.10 uses float. Clamped to 0.5-2.0, sent as ubyte = pitch*63
        // Actually in 1.7.10 protocol, pitch is sent as float for sound effect
        w.writeFloat(pitch);
        return w.toFramed();
    }

} // namespace PacketBuilder

} // namespace mccpp
