#pragma once
// TileEntitySync — tile entity state sync and block action packets.
// Ported from vanilla 1.7.10 S35PacketUpdateTileEntity (jc2.java),
// S24PacketBlockAction (ge.java), TileEntity (apr.java).
//
// Protocol 5 (1.7.10):
//   S→C 0x35: Update Tile Entity (x, y, z, action, nbtData)
//   S→C 0x24: Block Action (x, y, z, byte1, byte2, blockType)
//   S→C 0x23: Block Change (x, y, z, blockId, blockMeta)
//   S→C 0x22: Multi Block Change (chunkX, chunkZ, count, data)

#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>
#include "networking/PacketBuffer.h"
#include "nbt/NBT.h"

namespace mc {

// ============================================================
// S→C 0x35 Update Tile Entity — jc2.java
// ============================================================
struct UpdateTileEntityPacket {
    int32_t x;
    int16_t y;
    int32_t z;
    uint8_t action;     // 1=mob spawner, 2=command block, 3=beacon, 4=skull, 5=flower pot, 6=banner
    NBTCompound nbtData;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x35);
        buf.writeInt(x);
        buf.writeShort(y);
        buf.writeInt(z);
        buf.writeByte(action);
        // Write NBT compound
        nbtData.write(buf);
        return buf;
    }

    // Action type constants
    static constexpr uint8_t ACTION_MOB_SPAWNER   = 1;
    static constexpr uint8_t ACTION_COMMAND_BLOCK  = 2;
    static constexpr uint8_t ACTION_BEACON         = 3;
    static constexpr uint8_t ACTION_SKULL           = 4;
    static constexpr uint8_t ACTION_FLOWER_POT      = 5;
    static constexpr uint8_t ACTION_BANNER          = 6;

    // Factory for mob spawner
    static UpdateTileEntityPacket mobSpawner(int bx, int by, int bz,
                                              const std::string& entityId, int16_t delay) {
        UpdateTileEntityPacket pkt;
        pkt.x = bx;
        pkt.y = static_cast<int16_t>(by);
        pkt.z = bz;
        pkt.action = ACTION_MOB_SPAWNER;
        pkt.nbtData.setString("EntityId", entityId);
        pkt.nbtData.setShort("Delay", delay);
        pkt.nbtData.setShort("MinSpawnDelay", 200);
        pkt.nbtData.setShort("MaxSpawnDelay", 800);
        pkt.nbtData.setShort("SpawnCount", 4);
        pkt.nbtData.setShort("SpawnRange", 4);
        pkt.nbtData.setShort("MaxNearbyEntities", 6);
        pkt.nbtData.setShort("RequiredPlayerRange", 16);
        return pkt;
    }

    // Factory for skull
    static UpdateTileEntityPacket skull(int bx, int by, int bz,
                                         uint8_t skullType, uint8_t rotation,
                                         const std::string& ownerName = "") {
        UpdateTileEntityPacket pkt;
        pkt.x = bx;
        pkt.y = static_cast<int16_t>(by);
        pkt.z = bz;
        pkt.action = ACTION_SKULL;
        pkt.nbtData.setByte("SkullType", skullType);
        pkt.nbtData.setByte("Rot", rotation);
        if (!ownerName.empty()) {
            pkt.nbtData.setString("ExtraType", ownerName);
        }
        return pkt;
    }
};

// ============================================================
// S→C 0x24 Block Action — ge.java
// ============================================================
struct BlockActionPacket {
    int32_t x;
    int16_t y;
    int32_t z;
    uint8_t byte1;     // Action ID
    uint8_t byte2;     // Action parameter
    int32_t blockType; // VarInt block ID

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x24);
        buf.writeInt(x);
        buf.writeShort(y);
        buf.writeInt(z);
        buf.writeByte(byte1);
        buf.writeByte(byte2);
        buf.writeVarInt(blockType);
        return buf;
    }

    // Block actions — note block
    static BlockActionPacket noteBlock(int bx, int by, int bz, uint8_t instrument, uint8_t pitch) {
        return {bx, static_cast<int16_t>(by), bz, instrument, pitch, 25}; // Block 25 = noteblock
    }

    // Block actions — piston extend/retract
    static BlockActionPacket pistonExtend(int bx, int by, int bz, uint8_t direction) {
        return {bx, static_cast<int16_t>(by), bz, 0, direction, 33}; // Block 33 = piston
    }

    static BlockActionPacket pistonRetract(int bx, int by, int bz, uint8_t direction) {
        return {bx, static_cast<int16_t>(by), bz, 1, direction, 33};
    }

    // Block actions — chest open/close animation
    static BlockActionPacket chestOpen(int bx, int by, int bz, uint8_t viewers) {
        return {bx, static_cast<int16_t>(by), bz, 1, viewers, 54}; // Block 54 = chest
    }

    static BlockActionPacket enderChestOpen(int bx, int by, int bz, uint8_t viewers) {
        return {bx, static_cast<int16_t>(by), bz, 1, viewers, 130}; // Block 130 = ender chest
    }
};

// ============================================================
// S→C 0x23 Block Change — gd.java
// ============================================================
struct BlockChangePacket {
    int32_t  x;
    uint8_t  y;
    int32_t  z;
    int32_t  blockId;   // VarInt
    uint8_t  blockMeta;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x23);
        buf.writeInt(x);
        buf.writeByte(y);
        buf.writeInt(z);
        buf.writeVarInt(blockId);
        buf.writeByte(blockMeta);
        return buf;
    }
};

// ============================================================
// S→C 0x22 Multi Block Change — gc.java
// ============================================================
struct MultiBlockChangePacket {
    int32_t chunkX, chunkZ;
    struct Record {
        uint8_t  xz;       // Upper 4 bits = x, lower 4 bits = z (within chunk)
        uint8_t  y;
        int16_t  blockIdMeta; // (blockId << 4) | meta
    };
    std::vector<Record> records;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x22);
        buf.writeInt(chunkX);
        buf.writeInt(chunkZ);
        buf.writeShort(static_cast<int16_t>(records.size()));
        buf.writeInt(static_cast<int32_t>(records.size() * 4)); // Data size in bytes
        for (auto& rec : records) {
            buf.writeShort(rec.blockIdMeta);
            buf.writeByte(rec.y);
            buf.writeByte(rec.xz);
        }
        return buf;
    }

    void addChange(int localX, int localZ, int y, int blockId, int meta) {
        Record rec;
        rec.xz = static_cast<uint8_t>(((localX & 0xF) << 4) | (localZ & 0xF));
        rec.y = static_cast<uint8_t>(y);
        rec.blockIdMeta = static_cast<int16_t>((blockId << 4) | (meta & 0xF));
        records.push_back(rec);
    }
};

// ============================================================
// Tile entity tracker — tracks tile entities needing sync
// ============================================================
class TileEntityTracker {
public:
    enum class Type {
        CHEST,
        FURNACE,
        MOB_SPAWNER,
        SIGN,
        SKULL,
        COMMAND_BLOCK,
        BEACON,
        FLOWER_POT,
        JUKEBOX,
        ENCHANT_TABLE,
        BREWING_STAND
    };

    struct TileEntity {
        int x, y, z;
        Type type;
        NBTCompound data;
        bool dirty = false;
    };

    void addTileEntity(int x, int y, int z, Type type) {
        auto key = packKey(x, y, z);
        tileEntities_[key] = {x, y, z, type, {}, false};
    }

    void removeTileEntity(int x, int y, int z) {
        tileEntities_.erase(packKey(x, y, z));
    }

    TileEntity* get(int x, int y, int z) {
        auto it = tileEntities_.find(packKey(x, y, z));
        return it != tileEntities_.end() ? &it->second : nullptr;
    }

    void markDirty(int x, int y, int z) {
        auto* te = get(x, y, z);
        if (te) te->dirty = true;
    }

    // Collect dirty tile entities and create update packets
    std::vector<UpdateTileEntityPacket> collectDirtyUpdates() {
        std::vector<UpdateTileEntityPacket> updates;
        for (auto& [key, te] : tileEntities_) {
            if (!te.dirty) continue;
            te.dirty = false;

            UpdateTileEntityPacket pkt;
            pkt.x = te.x;
            pkt.y = static_cast<int16_t>(te.y);
            pkt.z = te.z;
            pkt.nbtData = te.data;

            switch (te.type) {
                case Type::MOB_SPAWNER:   pkt.action = 1; break;
                case Type::COMMAND_BLOCK: pkt.action = 2; break;
                case Type::BEACON:        pkt.action = 3; break;
                case Type::SKULL:         pkt.action = 4; break;
                case Type::FLOWER_POT:    pkt.action = 5; break;
                default:                  pkt.action = 1; break; // Generic
            }

            updates.push_back(pkt);
        }
        return updates;
    }

    // Get tile entities in a chunk for initial sync
    std::vector<UpdateTileEntityPacket> getTileEntitiesInChunk(int chunkX, int chunkZ) {
        std::vector<UpdateTileEntityPacket> result;
        int minX = chunkX * 16, maxX = minX + 15;
        int minZ = chunkZ * 16, maxZ = minZ + 15;

        for (auto& [key, te] : tileEntities_) {
            if (te.x >= minX && te.x <= maxX && te.z >= minZ && te.z <= maxZ) {
                UpdateTileEntityPacket pkt;
                pkt.x = te.x;
                pkt.y = static_cast<int16_t>(te.y);
                pkt.z = te.z;
                pkt.nbtData = te.data;
                switch (te.type) {
                    case Type::MOB_SPAWNER:   pkt.action = 1; break;
                    case Type::COMMAND_BLOCK: pkt.action = 2; break;
                    case Type::BEACON:        pkt.action = 3; break;
                    case Type::SKULL:         pkt.action = 4; break;
                    case Type::FLOWER_POT:    pkt.action = 5; break;
                    default:                  pkt.action = 1; break;
                }
                result.push_back(pkt);
            }
        }
        return result;
    }

    // Track chest viewer counts for block action
    void setChestViewers(int x, int y, int z, int count) {
        chestViewers_[packKey(x, y, z)] = count;
    }

    int getChestViewers(int x, int y, int z) const {
        auto it = chestViewers_.find(packKey(x, y, z));
        return it != chestViewers_.end() ? it->second : 0;
    }

private:
    static int64_t packKey(int x, int y, int z) {
        return (static_cast<int64_t>(x) & 0x3FFFFFF) |
               ((static_cast<int64_t>(z) & 0x3FFFFFF) << 26) |
               ((static_cast<int64_t>(y) & 0xFFF) << 52);
    }

    std::unordered_map<int64_t, TileEntity> tileEntities_;
    std::unordered_map<int64_t, int> chestViewers_;
};

} // namespace mc
