#pragma once
// Play state packets (S→C) for the join sequence.
// Implements the minimum packets a MC 1.7.10 client needs to fully connect.
// 
// Reference: hd.java (Join Game), protocol wiki for others.
// Protocol version: 5 (Minecraft 1.7.10)

#include <cstdint>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <zlib.h>
#include "networking/PacketBuffer.h"
#include "world/Chunk.h"
#include "inventory/ItemStack.h"

namespace mc {

// ============================================================
// S→C 0x01 Join Game — hd.java
// ============================================================
// hd.b(et): writeInt(entityId), writeByte(gameMode|hardcore),
//           writeByte(dimension), writeByte(difficulty),
//           writeByte(maxPlayers), writeString(levelType)
struct JoinGamePacket {
    int32_t entityId;
    uint8_t gameMode;     // 0=survival,1=creative,2=adventure
    bool    hardcore;
    int8_t  dimension;    // -1=nether,0=overworld,1=end
    uint8_t difficulty;   // 0=peaceful,1=easy,2=normal,3=hard
    uint8_t maxPlayers;
    std::string levelType; // "default","flat","largeBiomes","amplified"

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x01); // Packet ID
        buf.writeInt(entityId);
        uint8_t modeByte = gameMode;
        if (hardcore) modeByte |= 0x08;
        buf.writeByte(modeByte);
        buf.writeByte(static_cast<uint8_t>(dimension));
        buf.writeByte(difficulty);
        buf.writeByte(maxPlayers);
        buf.writeString(levelType);
        return buf;
    }
};

// ============================================================
// S→C 0x05 Spawn Position
// ============================================================
// Sends the world spawn position (compass points here)
struct SpawnPositionPacket {
    int32_t x, y, z;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x05); // Packet ID
        buf.writeInt(x);
        buf.writeInt(y);
        buf.writeInt(z);
        return buf;
    }
};

// ============================================================
// S→C 0x08 Player Position And Look
// ============================================================
// Teleports the player and sets look direction
struct PlayerPositionAndLookPacket {
    double x, y, z;
    float  yaw, pitch;
    bool   onGround;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x08); // Packet ID
        buf.writeDouble(x);
        buf.writeDouble(y); // feet Y (client uses this + 1.62 for eyes)
        buf.writeDouble(z);
        buf.writeFloat(yaw);
        buf.writeFloat(pitch);
        buf.writeBoolean(onGround);
        return buf;
    }
};

// ============================================================
// S→C 0x00 Keep Alive
// ============================================================
struct KeepAlivePacket {
    int32_t keepAliveId;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x00); // Packet ID
        buf.writeInt(keepAliveId);
        return buf;
    }
};

// ============================================================
// S→C 0x21 Chunk Data
// ============================================================
// Sends a single chunk column. For initial join we send an empty
// "unload" chunk (groundUp=true, primaryBitmap=0, data=empty)
// to satisfy the client's requirement for spawn area chunks.
struct ChunkDataPacket {
    int32_t  chunkX, chunkZ;
    bool     groundUpContinuous;
    uint16_t primaryBitmap;
    uint16_t addBitmap;
    std::vector<uint8_t> compressedData; // zlib-compressed chunk data

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x21); // Packet ID
        buf.writeInt(chunkX);
        buf.writeInt(chunkZ);
        buf.writeBoolean(groundUpContinuous);
        buf.writeShort(static_cast<int16_t>(primaryBitmap));
        buf.writeShort(static_cast<int16_t>(addBitmap));
        buf.writeInt(static_cast<int32_t>(compressedData.size()));
        buf.writeBytes(compressedData);
        return buf;
    }

    // Create an "unload" chunk packet — tells the client this chunk has no data
    static ChunkDataPacket makeUnload(int32_t cx, int32_t cz) {
        ChunkDataPacket pkt;
        pkt.chunkX = cx;
        pkt.chunkZ = cz;
        pkt.groundUpContinuous = true;
        pkt.primaryBitmap = 0;
        pkt.addBitmap = 0;
        // Empty zlib stream for empty chunk
        pkt.compressedData = {0x78, 0x9C, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01};
        return pkt;
    }

    // Create from a ChunkColumn with real block data, zlib-compressed
    static ChunkDataPacket fromChunkColumn(const ChunkColumn& col, bool groundUp) {
        auto serialized = col.serialize(groundUp);

        // Zlib compress the raw chunk data
        uLongf compBound = compressBound(static_cast<uLong>(serialized.data.size()));
        std::vector<uint8_t> compressed(compBound);
        int ret = compress2(compressed.data(), &compBound,
                            serialized.data.data(),
                            static_cast<uLong>(serialized.data.size()),
                            Z_DEFAULT_COMPRESSION);
        if (ret != Z_OK) {
            throw std::runtime_error("zlib compress failed for chunk data");
        }
        compressed.resize(compBound);

        ChunkDataPacket pkt;
        pkt.chunkX = col.chunkX;
        pkt.chunkZ = col.chunkZ;
        pkt.groundUpContinuous = groundUp;
        pkt.primaryBitmap = serialized.primaryBitmap;
        pkt.addBitmap = serialized.addBitmap;
        pkt.compressedData = std::move(compressed);
        return pkt;
    }
};

// ============================================================
// S→C 0x39 Player Abilities
// ============================================================
// Sets the player's abilities (flying, invulnerable, etc.)
struct PlayerAbilitiesPacket {
    bool invulnerable;
    bool flying;
    bool allowFlying;
    bool creativeMode;
    float flySpeed;    // Default: 0.05
    float walkSpeed;   // Default: 0.1

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x39); // Packet ID
        uint8_t flags = 0;
        if (invulnerable) flags |= 0x01;
        if (flying)       flags |= 0x02;
        if (allowFlying)  flags |= 0x04;
        if (creativeMode) flags |= 0x08;
        buf.writeByte(flags);
        buf.writeFloat(flySpeed);
        buf.writeFloat(walkSpeed);
        return buf;
    }
};

// ============================================================
// S→C 0x3F Plugin Message 
// ============================================================
// Used for brand channel "MC|Brand"
struct PluginMessagePacket {
    std::string channel;
    std::vector<uint8_t> data;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x3F); // Packet ID
        buf.writeString(channel);
        buf.writeShort(static_cast<int16_t>(data.size()));
        buf.writeBytes(data);
        return buf;
    }

    static PluginMessagePacket makeBrand(const std::string& brand) {
        PluginMessagePacket pkt;
        pkt.channel = "MC|Brand";
        pkt.data.assign(brand.begin(), brand.end());
        return pkt;
    }
};

// ============================================================
// S→C 0x0C Spawn Player
// ============================================================
// Spawns another player entity for the client
struct SpawnPlayerPacket {
    int32_t entityId;
    std::string uuid;     // Player UUID (with dashes)
    std::string name;     // Player name
    int32_t dataCount = 0;// Number of data entries (simplified: 0)
    double x, y, z;
    float yaw, pitch;
    int16_t currentItem;  // Item in hand (0 = empty)

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x0C); // Packet ID
        buf.writeVarInt(entityId);

        // In protocol 5, UUID is sent as a string
        buf.writeString(uuid);
        buf.writeString(name);

        // Data count (VarInt) — properties like skin etc, 0 for now
        buf.writeVarInt(0);

        // Fixed-point position (absolute int = value * 32)
        buf.writeInt(static_cast<int32_t>(x * 32.0));
        buf.writeInt(static_cast<int32_t>(y * 32.0));
        buf.writeInt(static_cast<int32_t>(z * 32.0));

        // Rotation (angle = value * 256 / 360)
        buf.writeByte(static_cast<uint8_t>(static_cast<int>(yaw * 256.0f / 360.0f) & 0xFF));
        buf.writeByte(static_cast<uint8_t>(static_cast<int>(pitch * 256.0f / 360.0f) & 0xFF));

        buf.writeShort(currentItem);

        // Entity metadata terminator (0x7F)
        buf.writeByte(0x7F);

        return buf;
    }
};

// ============================================================
// S→C 0x13 Destroy Entities
// ============================================================
struct DestroyEntitiesPacket {
    std::vector<int32_t> entityIds;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x13); // Packet ID
        buf.writeByte(static_cast<uint8_t>(entityIds.size()));
        for (int32_t id : entityIds) {
            buf.writeInt(id);
        }
        return buf;
    }
};

// ============================================================
// S→C 0x03 Time Update
// ============================================================
// Sends world age and time of day
struct TimeUpdatePacket {
    int64_t worldAge;   // Total ticks since world creation
    int64_t timeOfDay;  // Time of day (0-24000), negative = fixed time

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x03); // Packet ID
        buf.writeLong(worldAge);
        buf.writeLong(timeOfDay);
        return buf;
    }
};

// ============================================================
// S→C 0x02 Chat Message
// ============================================================
struct ChatMessagePacket {
    std::string jsonText;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x02); // Packet ID
        buf.writeString(jsonText);
        return buf;
    }

    // Helper: create a simple text chat message
    static ChatMessagePacket makeText(const std::string& text) {
        ChatMessagePacket pkt;
        pkt.jsonText = R"({"text":")" + text + R"("})";
        return pkt;
    }

    // Helper: create a player chat message
    static ChatMessagePacket makeChat(const std::string& player, const std::string& message) {
        ChatMessagePacket pkt;
        pkt.jsonText = R"({"translate":"chat.type.text","with":[")" + player + R"(",")" + message + R"("]})";
        return pkt;
    }
};

// ============================================================
// S→C 0x2F Set Slot
// ============================================================
// Updates a single slot in a window
struct SetSlotPacket {
    int8_t windowId;   // 0 = player inventory
    int16_t slotIndex;
    ItemStack item;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x2F); // Packet ID
        buf.writeByte(static_cast<uint8_t>(windowId));
        buf.writeShort(slotIndex);
        item.writeToPacket(buf);
        return buf;
    }
};

// ============================================================
// S→C 0x30 Window Items
// ============================================================
// Sends entire window contents
struct WindowItemsPacket {
    int8_t windowId;
    std::vector<ItemStack> items;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x30); // Packet ID
        buf.writeByte(static_cast<uint8_t>(windowId));
        buf.writeShort(static_cast<int16_t>(items.size()));
        for (auto& item : items) {
            item.writeToPacket(buf);
        }
        return buf;
    }
};

// ============================================================
// S→C 0x09 Held Item Change
// ============================================================
struct HeldItemChangePacket {
    int8_t slot; // 0-8

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x09); // Packet ID
        buf.writeByte(static_cast<uint8_t>(slot));
        return buf;
    }
};

// ============================================================
// S→C 0x23 Block Change
// ============================================================
struct BlockChangePacket {
    int32_t x, y, z;
    int32_t blockId;   // VarInt
    uint8_t metadata;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x23); // Packet ID
        buf.writeInt(x);
        buf.writeByte(static_cast<uint8_t>(y));
        buf.writeInt(z);
        buf.writeVarInt(blockId);
        buf.writeByte(metadata);
        return buf;
    }
};

// ============================================================
// S→C 0x06 Update Health — ib.java
// ============================================================
// Sends health, food, and saturation to the client
struct UpdateHealthPacket {
    float health;       // 0.0 = dead, 20.0 = full
    int32_t food;       // 0-20
    float saturation;   // 0.0-5.0

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x06); // Packet ID
        buf.writeFloat(health);
        buf.writeShort(static_cast<int16_t>(food)); // Protocol 5: Short, not VarInt
        buf.writeFloat(saturation);
        return buf;
    }
};

// ============================================================
// S→C 0x1F Set Experience — ia.java
// ============================================================
struct SetExperiencePacket {
    float barProgress;   // 0.0-1.0
    int16_t level;
    int16_t totalExp;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x1F); // Packet ID
        buf.writeFloat(barProgress);
        buf.writeShort(level);     // Protocol 5: Short, not VarInt
        buf.writeShort(totalExp);  // Protocol 5: Short, not VarInt
        return buf;
    }
};

// ============================================================
// S→C 0x38 Player List Item — id.java
// ============================================================
// Adds/removes a player from the tab list
struct PlayerListItemPacket {
    std::string playerName;
    bool online;         // true=add, false=remove
    int16_t ping;        // Latency in ms

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x38); // Packet ID
        buf.writeString(playerName);
        buf.writeBoolean(online);
        buf.writeShort(ping);
        return buf;
    }
};

// ============================================================
// S→C 0x07 Respawn — gv.java
// ============================================================
struct RespawnPacket {
    int32_t dimension;   // -1/0/1
    uint8_t difficulty;  // 0-3
    uint8_t gameMode;    // 0/1/2
    std::string levelType; // "flat", "default", etc.

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x07); // Packet ID
        buf.writeInt(dimension);
        buf.writeByte(difficulty);
        buf.writeByte(gameMode);
        buf.writeString(levelType);
        return buf;
    }
};

// ============================================================
// S→C 0x2B Change Game State — gw.java
// ============================================================
// Used for rain, game mode changes, etc.
struct ChangeGameStatePacket {
    uint8_t reason;   // 1=rain start, 2=rain stop, 3=gamemode change, 4=credits
    float value;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x2B); // Packet ID
        buf.writeByte(reason);
        buf.writeFloat(value);
        return buf;
    }
};

// ============================================================
// S→C 0x1A Entity Status — gc.java
// ============================================================
struct EntityStatusPacket {
    int32_t entityId;
    int8_t status;    // 2=hurt, 3=dead, 9=eat complete

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x1A); // Packet ID
        buf.writeInt(entityId);
        buf.writeByte(static_cast<uint8_t>(status));
        return buf;
    }
};

// ============================================================
// S→C 0x15 Entity Relative Move — gj.java
// ============================================================
// Delta position in fixed-point (value * 32)
struct EntityRelativeMovePacket {
    int32_t entityId;
    int8_t dx, dy, dz;  // Fixed-point delta (pixels = blocks * 32)

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x15); // Packet ID
        buf.writeVarInt(entityId);
        buf.writeByte(static_cast<uint8_t>(dx));
        buf.writeByte(static_cast<uint8_t>(dy));
        buf.writeByte(static_cast<uint8_t>(dz));
        return buf;
    }
};

// ============================================================
// S→C 0x16 Entity Look — gi.java
// ============================================================
struct EntityLookPacket {
    int32_t entityId;
    uint8_t yaw;    // Angle = degrees * 256 / 360
    uint8_t pitch;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x16); // Packet ID
        buf.writeVarInt(entityId);
        buf.writeByte(yaw);
        buf.writeByte(pitch);
        return buf;
    }

    static uint8_t toAngle(float degrees) {
        return static_cast<uint8_t>(static_cast<int>(degrees * 256.0f / 360.0f) & 0xFF);
    }
};

// ============================================================
// S→C 0x17 Entity Look and Relative Move — gk.java
// ============================================================
struct EntityLookAndRelativeMovePacket {
    int32_t entityId;
    int8_t dx, dy, dz;
    uint8_t yaw, pitch;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x17); // Packet ID
        buf.writeVarInt(entityId);
        buf.writeByte(static_cast<uint8_t>(dx));
        buf.writeByte(static_cast<uint8_t>(dy));
        buf.writeByte(static_cast<uint8_t>(dz));
        buf.writeByte(yaw);
        buf.writeByte(pitch);
        return buf;
    }
};

// ============================================================
// S→C 0x18 Entity Teleport — gn.java
// ============================================================
// Absolute position in fixed-point (value * 32)
struct EntityTeleportPacket {
    int32_t entityId;
    int32_t x, y, z;  // Fixed-point absolute (blocks * 32)
    uint8_t yaw, pitch;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x18); // Packet ID
        buf.writeVarInt(entityId);
        buf.writeInt(x);
        buf.writeInt(y);
        buf.writeInt(z);
        buf.writeByte(yaw);
        buf.writeByte(pitch);
        return buf;
    }

    static EntityTeleportPacket fromPlayer(int32_t eid, double px, double py, double pz,
                                            float yawDeg, float pitchDeg) {
        EntityTeleportPacket pkt;
        pkt.entityId = eid;
        pkt.x = static_cast<int32_t>(px * 32.0);
        pkt.y = static_cast<int32_t>(py * 32.0);
        pkt.z = static_cast<int32_t>(pz * 32.0);
        pkt.yaw = EntityLookPacket::toAngle(yawDeg);
        pkt.pitch = EntityLookPacket::toAngle(pitchDeg);
        return pkt;
    }
};

// ============================================================
// S→C 0x19 Entity Head Look — gl.java
// ============================================================
struct EntityHeadLookPacket {
    int32_t entityId;
    uint8_t headYaw;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x19); // Packet ID
        buf.writeVarInt(entityId);
        buf.writeByte(headYaw);
        return buf;
    }
};

// ============================================================
// S→C 0x0B Animation — ge.java
// ============================================================
struct AnimationPacket {
    int32_t entityId;
    uint8_t animation; // 0=swing arm, 1=take damage, 2=leave bed, 3=eat, 4=crit, 5=magic crit

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x0B); // Packet ID
        buf.writeVarInt(entityId);
        buf.writeByte(animation);
        return buf;
    }
};

// ============================================================
// S→C 0x1C Entity Metadata — gg.java
// ============================================================
// Simplified entity metadata for player entities.
// Full implementation would use the DataWatcher system (dn.java),
// but for now we send the minimum: entity flags + health.
struct EntityMetadataPacket {
    int32_t entityId;
    // Simplified: we'll build the metadata bytes inline

    // Build a basic player metadata with just entity flags
    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x1C); // Packet ID
        buf.writeVarInt(entityId);
        // Metadata terminator (0x7F = end)
        buf.writeByte(0x7F);
        return buf;
    }

    // Build metadata with entity flags and health
    static PacketBuffer serializePlayerMetadata(int32_t eid, float health, bool onFire = false,
                                                 bool crouching = false, bool sprinting = false) {
        PacketBuffer buf;
        buf.writeVarInt(0x1C); // Packet ID
        buf.writeVarInt(eid);

        // Index 0, Type Byte (0 << 5 | 0): Entity flags
        uint8_t flags = 0;
        if (onFire) flags |= 0x01;
        if (crouching) flags |= 0x02;
        if (sprinting) flags |= 0x08;
        buf.writeByte(0x00); // index 0, type byte
        buf.writeByte(flags);

        // Index 6, Type Float (3 << 5 | 6): Health
        buf.writeByte((3 << 5) | 6); // index 6, type float
        buf.writeFloat(health);

        // Terminator
        buf.writeByte(0x7F);
        return buf;
    }
};

// ============================================================
// S→C 0x04 Entity Equipment — gf.java
// ============================================================
// Shows held item or armor on an entity
struct EntityEquipmentPacket {
    int32_t entityId;
    int16_t slot;      // 0=held, 1=boots, 2=legs, 3=chest, 4=helm
    // Slot data (item)
    int16_t itemId = -1;   // -1 = empty
    int8_t  count = 0;
    int16_t damage = 0;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x04); // Packet ID
        buf.writeVarInt(entityId);
        buf.writeShort(slot);
        // Slot data
        buf.writeShort(itemId);
        if (itemId >= 0) {
            buf.writeByte(static_cast<uint8_t>(count));
            buf.writeShort(damage);
            buf.writeShort(-1); // No NBT
        }
        return buf;
    }
};

// ============================================================
// S→C 0x3A Tab Complete (response) — hu.java
// ============================================================
struct TabCompletePacket {
    std::vector<std::string> matches;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x3A); // Packet ID
        buf.writeVarInt(static_cast<int32_t>(matches.size()));
        for (auto& m : matches) {
            buf.writeString(m);
        }
        return buf;
    }
};

// ============================================================
// S→C 0x3B Scoreboard Objective — hs.java
// ============================================================
struct ScoreboardObjectivePacket {
    std::string name;       // Objective name (max 16)
    std::string displayName; // Display text
    uint8_t mode;           // 0=create, 1=remove, 2=update display

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x3B); // Packet ID
        buf.writeString(name);
        buf.writeString(displayName);
        buf.writeByte(mode);
        return buf;
    }
};

// ============================================================
// S→C 0x3C Update Score — ht.java
// ============================================================
struct UpdateScorePacket {
    std::string itemName;    // Score holder (player name)
    uint8_t action;          // 0=update, 1=remove
    std::string objectiveName;
    int32_t value = 0;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x3C); // Packet ID
        buf.writeString(itemName);
        buf.writeByte(action);
        if (action != 1) {
            buf.writeString(objectiveName);
            buf.writeVarInt(value);
        }
        return buf;
    }
};

// ============================================================
// S→C 0x3D Display Scoreboard — hp.java
// ============================================================
struct DisplayScoreboardPacket {
    uint8_t position;     // 0=list, 1=sidebar, 2=below name
    std::string scoreName; // Objective name

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x3D); // Packet ID
        buf.writeByte(position);
        buf.writeString(scoreName);
        return buf;
    }
};

// ============================================================
// S→C 0x12 Entity Velocity — gm.java
// ============================================================
struct EntityVelocityPacket {
    int32_t entityId;
    int16_t vx, vy, vz;  // Velocity in 1/8000 blocks per tick

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x12); // Packet ID
        buf.writeVarInt(entityId);
        buf.writeShort(vx);
        buf.writeShort(vy);
        buf.writeShort(vz);
        return buf;
    }

    // Helper: convert blocks/tick to protocol units (1/8000)
    static int16_t toProtocol(double blocksPerTick) {
        double clamped = std::max(-3.9, std::min(3.9, blocksPerTick));
        return static_cast<int16_t>(clamped * 8000.0);
    }
};

// ============================================================
// S→C 0x29 Sound Effect (named) — gq.java
// ============================================================
struct NamedSoundEffectPacket {
    std::string soundName;  // e.g. "random.click", "mob.zombie.say"
    int32_t x, y, z;       // Fixed-point (value * 8)
    float volume;
    uint8_t pitch;         // 63 = normal

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x29); // Packet ID
        buf.writeString(soundName);
        buf.writeInt(x);
        buf.writeInt(y);
        buf.writeInt(z);
        buf.writeFloat(volume);
        buf.writeByte(pitch);
        return buf;
    }

    static NamedSoundEffectPacket at(const std::string& sound,
                                      double bx, double by, double bz,
                                      float vol = 1.0f, uint8_t p = 63) {
        NamedSoundEffectPacket pkt;
        pkt.soundName = sound;
        pkt.x = static_cast<int32_t>(bx * 8.0);
        pkt.y = static_cast<int32_t>(by * 8.0);
        pkt.z = static_cast<int32_t>(bz * 8.0);
        pkt.volume = vol;
        pkt.pitch = p;
        return pkt;
    }
};

// ============================================================
// S→C 0x2A Particle — go.java
// ============================================================
struct ParticlePacket {
    std::string particleName; // e.g. "flame", "smoke", "heart"
    float x, y, z;
    float offsetX, offsetY, offsetZ;
    float speed;
    int32_t count;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x2A); // Packet ID
        buf.writeString(particleName);
        buf.writeFloat(x);
        buf.writeFloat(y);
        buf.writeFloat(z);
        buf.writeFloat(offsetX);
        buf.writeFloat(offsetY);
        buf.writeFloat(offsetZ);
        buf.writeFloat(speed);
        buf.writeInt(count);
        return buf;
    }
};

// ============================================================
// S→C 0x40 Disconnect (Play) — gx.java
// ============================================================
struct DisconnectPacket {
    std::string reason; // JSON text

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x40); // Packet ID
        buf.writeString(reason);
        return buf;
    }

    static DisconnectPacket withMessage(const std::string& text) {
        DisconnectPacket pkt;
        pkt.reason = R"({"text":")" + text + R"("})";
        return pkt;
    }
};

// ============================================================
// S→C 0x0D Collect Item — gb.java
// ============================================================
struct CollectItemPacket {
    int32_t collectedEntityId;
    int32_t collectorEntityId;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x0D); // Packet ID
        buf.writeVarInt(collectedEntityId);
        buf.writeVarInt(collectorEntityId);
        return buf;
    }
};

// ============================================================
// S→C 0x27 Explosion — gr.java
// ============================================================
struct ExplosionPacket {
    float x, y, z;
    float radius;
    std::vector<std::array<int8_t, 3>> affectedBlocks;
    float playerMotionX, playerMotionY, playerMotionZ;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x27); // Packet ID
        buf.writeFloat(x);
        buf.writeFloat(y);
        buf.writeFloat(z);
        buf.writeFloat(radius);
        buf.writeInt(static_cast<int32_t>(affectedBlocks.size()));
        for (auto& block : affectedBlocks) {
            buf.writeByte(static_cast<uint8_t>(block[0]));
            buf.writeByte(static_cast<uint8_t>(block[1]));
            buf.writeByte(static_cast<uint8_t>(block[2]));
        }
        buf.writeFloat(playerMotionX);
        buf.writeFloat(playerMotionY);
        buf.writeFloat(playerMotionZ);
        return buf;
    }
};

// ============================================================
// S→C 0x28 Effect — gp.java
// ============================================================
// World effects (door sounds, block break particles, etc.)
struct EffectPacket {
    int32_t effectId;
    int32_t x;
    uint8_t y;
    int32_t z;
    int32_t data;
    bool disableRelativeVolume = false;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x28); // Packet ID
        buf.writeInt(effectId);
        buf.writeInt(x);
        buf.writeByte(y);
        buf.writeInt(z);
        buf.writeInt(data);
        buf.writeBoolean(disableRelativeVolume);
        return buf;
    }
};

// ============================================================
// S→C 0x2D Open Window — gs.java
// ============================================================
struct OpenWindowPacket {
    uint8_t windowId;
    uint8_t inventoryType;  // 1=crafting, 0=chest, 2=furnace, etc.
    std::string windowTitle;
    uint8_t slotCount;
    bool useProvidedTitle = true;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x2D); // Packet ID
        buf.writeByte(windowId);
        buf.writeByte(inventoryType);
        buf.writeString(windowTitle);
        buf.writeByte(slotCount);
        buf.writeBoolean(useProvidedTitle);
        return buf;
    }
};

// ============================================================
// S→C 0x2E Close Window — gu.java
// ============================================================
struct CloseWindowPacket {
    uint8_t windowId;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x2E); // Packet ID
        buf.writeByte(windowId);
        return buf;
    }
};

// ============================================================
// S→C 0x32 Confirm Transaction — gw.java
// ============================================================
struct ConfirmTransactionPacket {
    uint8_t windowId;
    int16_t actionNumber;
    bool accepted;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x32); // Packet ID
        buf.writeByte(windowId);
        buf.writeShort(actionNumber);
        buf.writeBoolean(accepted);
        return buf;
    }
};

// ============================================================
// S→C 0x1D Entity Effect — apply a potion effect to an entity
// Reference: gq.java (S1DPacketEntityEffect)
// ============================================================
struct EntityEffectPacket {
    int32_t entityId;
    int8_t effectId;
    int8_t amplifier;
    int32_t duration;
    bool hideParticles;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x1D); // Packet ID
        buf.writeVarInt(entityId);
        buf.writeByte(effectId);
        buf.writeByte(amplifier);
        buf.writeVarInt(duration);
        return buf;
    }
};

// ============================================================
// S→C 0x1E Remove Entity Effect — remove a potion effect
// Reference: gr.java (S1EPacketRemoveEntityEffect)
// ============================================================
struct RemoveEntityEffectPacket {
    int32_t entityId;
    int8_t effectId;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x1E); // Packet ID
        buf.writeVarInt(entityId);
        buf.writeByte(effectId);
        return buf;
    }
};

// ============================================================
// S→C 0x2C Spawn Global Entity — lightning bolt
// Reference: fx.java (S2CPacketSpawnGlobalEntity)
// ============================================================
struct SpawnGlobalEntityPacket {
    int32_t entityId;
    int8_t type;       // 1 = lightning bolt
    int32_t x, y, z;   // Fixed-point (1/32 blocks)

    static SpawnGlobalEntityPacket lightning(int32_t eid, double px, double py, double pz) {
        SpawnGlobalEntityPacket pkt;
        pkt.entityId = eid;
        pkt.type = 1;
        pkt.x = static_cast<int32_t>(px * 32.0);
        pkt.y = static_cast<int32_t>(py * 32.0);
        pkt.z = static_cast<int32_t>(pz * 32.0);
        return pkt;
    }

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x2C); // Packet ID
        buf.writeVarInt(entityId);
        buf.writeByte(type);
        buf.writeInt(x);
        buf.writeInt(y);
        buf.writeInt(z);
        return buf;
    }
};

// ============================================================
// S→C 0x31 Window Property — iw.java
// ============================================================
struct WindowPropertyPacket {
    uint8_t windowId;
    int16_t property;
    int16_t value;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x31);
        buf.writeByte(windowId);
        buf.writeShort(property);
        buf.writeShort(value);
        return buf;
    }

    // Furnace properties
    static constexpr int16_t FURNACE_COOK_PROGRESS = 0;
    static constexpr int16_t FURNACE_FUEL_LEFT     = 1;
    static constexpr int16_t FURNACE_MAX_FUEL      = 2;

    // Enchantment table
    static constexpr int16_t ENCHANT_SLOT_1 = 0;
    static constexpr int16_t ENCHANT_SLOT_2 = 1;
    static constexpr int16_t ENCHANT_SLOT_3 = 2;

    // Beacon
    static constexpr int16_t BEACON_POWER   = 0;
    static constexpr int16_t BEACON_EFFECT1 = 1;
    static constexpr int16_t BEACON_EFFECT2 = 2;
};

// ============================================================
// S→C 0x34 Map Data — ix.java (S34PacketMaps)
// ============================================================
struct MapDataPacket {
    int32_t mapId;      // VarInt — map item damage value
    uint8_t scale;      // 0-4, world blocks per pixel (2^scale)

    // Map icons (player markers, etc.)
    struct Icon {
        uint8_t directionAndType; // Upper 4 = direction (0-15), lower 4 = type (0-15)
        int8_t  x;    // -128 to 127
        int8_t  z;
    };
    std::vector<Icon> icons;

    // Column update (partial map data)
    uint8_t columns;    // Number of columns to update (0 = icons only)
    uint8_t rows;
    uint8_t xOffset;
    uint8_t zOffset;
    std::vector<uint8_t> data; // Color indices (columns * rows bytes)

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x34);
        buf.writeVarInt(mapId);
        buf.writeShort(static_cast<int16_t>(data.size() + 3 + icons.size() * 3 + 1));

        // Scale
        buf.writeByte(scale);

        // Icons
        buf.writeVarInt(static_cast<int32_t>(icons.size()));
        for (auto& icon : icons) {
            buf.writeByte(icon.directionAndType);
            buf.writeByte(static_cast<uint8_t>(icon.x));
            buf.writeByte(static_cast<uint8_t>(icon.z));
        }

        // Column data
        buf.writeByte(columns);
        if (columns > 0) {
            buf.writeByte(rows);
            buf.writeByte(xOffset);
            buf.writeByte(zOffset);
            buf.writeVarInt(static_cast<int32_t>(data.size()));
            for (auto b : data) {
                buf.writeByte(b);
            }
        }

        return buf;
    }

    // Icon type constants
    static constexpr uint8_t ICON_WHITE_ARROW   = 0;
    static constexpr uint8_t ICON_GREEN_ARROW   = 1;
    static constexpr uint8_t ICON_RED_ARROW     = 2;
    static constexpr uint8_t ICON_BLUE_ARROW    = 3;
    static constexpr uint8_t ICON_WHITE_CROSS   = 4;
    static constexpr uint8_t ICON_RED_POINTER   = 5;
    static constexpr uint8_t ICON_WHITE_CIRCLE  = 6;

    // Factory for full map update
    static MapDataPacket fullUpdate(int32_t id, uint8_t mapScale,
                                     const std::vector<uint8_t>& pixels) {
        MapDataPacket pkt;
        pkt.mapId = id;
        pkt.scale = mapScale;
        pkt.columns = 128;
        pkt.rows = 128;
        pkt.xOffset = 0;
        pkt.zOffset = 0;
        pkt.data = pixels;
        return pkt;
    }

    // Factory for icons-only update
    static MapDataPacket iconsOnly(int32_t id, uint8_t mapScale,
                                    const std::vector<Icon>& mapIcons) {
        MapDataPacket pkt;
        pkt.mapId = id;
        pkt.scale = mapScale;
        pkt.icons = mapIcons;
        pkt.columns = 0;
        pkt.rows = 0;
        pkt.xOffset = 0;
        pkt.zOffset = 0;
        return pkt;
    }
};

// ============================================================
// Map color palette — vanilla 1.7.10 base colors (4 shades each)
// ============================================================
namespace MapColor {
    // Base color IDs (multiply by 4 to get first shade)
    constexpr uint8_t NONE       = 0;   // Transparent
    constexpr uint8_t GRASS      = 1;   // #7FB238
    constexpr uint8_t SAND       = 2;   // #F7E9A3
    constexpr uint8_t CLOTH      = 3;   // #C7C7C7 (wool)
    constexpr uint8_t TNT        = 4;   // #FF0000
    constexpr uint8_t ICE        = 5;   // #A0A0FF
    constexpr uint8_t IRON       = 6;   // #A7A7A7
    constexpr uint8_t FOLIAGE    = 7;   // #007C00
    constexpr uint8_t SNOW       = 8;   // #FFFFFF
    constexpr uint8_t CLAY       = 9;   // #A4A8B8
    constexpr uint8_t DIRT       = 10;  // #976D4D
    constexpr uint8_t STONE      = 11;  // #707070
    constexpr uint8_t WATER      = 12;  // #4040FF
    constexpr uint8_t WOOD       = 13;  // #8F7748
    constexpr uint8_t QUARTZ     = 14;  // #FFFCF5
    constexpr uint8_t ADOBE      = 15;  // #D87F33
    constexpr uint8_t MAGENTA    = 16;  // #B24CD8
    constexpr uint8_t LIGHT_BLUE = 17;  // #6699D8
    constexpr uint8_t YELLOW     = 18;  // #E5E533
    constexpr uint8_t LIME       = 19;  // #7FCC19
    constexpr uint8_t PINK       = 20;  // #F27FA5
    constexpr uint8_t GRAY       = 21;  // #4C4C4C
    constexpr uint8_t SILVER     = 22;  // #999999
    constexpr uint8_t CYAN       = 23;  // #4C7F99
    constexpr uint8_t PURPLE     = 24;  // #7F3FB2
    constexpr uint8_t BLUE       = 25;  // #334CB2
    constexpr uint8_t BROWN      = 26;  // #664C33
    constexpr uint8_t GREEN      = 27;  // #667F33
    constexpr uint8_t RED        = 28;  // #993333
    constexpr uint8_t BLACK      = 29;  // #191919
    constexpr uint8_t GOLD       = 30;  // #FAEE4D
    constexpr uint8_t DIAMOND    = 31;  // #5CDBD5
    constexpr uint8_t LAPIS      = 32;  // #4A80FF
    constexpr uint8_t EMERALD    = 33;  // #00D93A
    constexpr uint8_t PODZOL     = 34;  // #815631
    constexpr uint8_t NETHER     = 35;  // #700200

    // Shade multiplier: shade 0 = 180/255, shade 1 = 220/255, shade 2 = 255/255, shade 3 = 135/255
    inline uint8_t makeColor(uint8_t baseColor, uint8_t shade) {
        return static_cast<uint8_t>(baseColor * 4 + shade);
    }
}

} // namespace mc

