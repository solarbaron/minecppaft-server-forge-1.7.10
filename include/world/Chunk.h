/**
 * Chunk.h — Chunk data structures: NibbleArray, ChunkSection, Chunk, RegionFile.
 *
 * Java references:
 *   - net.minecraft.world.chunk.NibbleArray
 *   - net.minecraft.world.chunk.storage.ExtendedBlockStorage
 *   - net.minecraft.world.chunk.Chunk (data portion)
 *   - net.minecraft.world.chunk.storage.RegionFile
 *
 * This implements the Anvil chunk format for block storage, serialization,
 * and region file I/O (reading/writing .mca files).
 *
 * Thread safety: ChunkSection and Chunk are intended for single-owner access
 * (one thread owns a chunk at a time, via the chunk provider). RegionFile
 * uses a mutex for concurrent read/write access.
 */
#pragma once

#include "block/Block.h"
#include "nbt/NBT.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// NibbleArray — 4-bit-per-element packed array (half-byte storage).
// Java reference: net.minecraft.world.chunk.NibbleArray
// ═══════════════════════════════════════════════════════════════════════════

class NibbleArray {
public:
    std::vector<uint8_t> data;

    /**
     * Create a NibbleArray for `length` elements.
     * Java: NibbleArray(int n, int n2) — n = element count, n2 = depthBits (always 4 for blocks)
     */
    explicit NibbleArray(int elementCount, int depthBits = 4)
        : data(elementCount >> 1, 0)
        , depthBits_(depthBits)
        , depthBitsPlusFour_(depthBits + 4) {}

    /**
     * Create from existing data buffer.
     * Java: NibbleArray(byte[], int)
     */
    NibbleArray(std::vector<uint8_t> buf, int depthBits = 4)
        : data(std::move(buf))
        , depthBits_(depthBits)
        , depthBitsPlusFour_(depthBits + 4) {}

    /**
     * Get nibble value at (x, y, z).
     * Java: NibbleArray.get(x, y, z)
     * Index: y << (depthBits+4) | z << depthBits | x
     */
    int get(int x, int y, int z) const {
        int idx = (y << depthBitsPlusFour_) | (z << depthBits_) | x;
        int half = idx >> 1;
        int odd = idx & 1;
        if (odd == 0) {
            return data[half] & 0x0F;
        }
        return (data[half] >> 4) & 0x0F;
    }

    /**
     * Set nibble value at (x, y, z).
     * Java: NibbleArray.set(x, y, z, val)
     */
    void set(int x, int y, int z, int val) {
        int idx = (y << depthBitsPlusFour_) | (z << depthBits_) | x;
        int half = idx >> 1;
        int odd = idx & 1;
        if (odd == 0) {
            data[half] = static_cast<uint8_t>((data[half] & 0xF0) | (val & 0x0F));
        } else {
            data[half] = static_cast<uint8_t>((data[half] & 0x0F) | ((val & 0x0F) << 4));
        }
    }

private:
    int depthBits_;
    int depthBitsPlusFour_;
};

// ═══════════════════════════════════════════════════════════════════════════
// ChunkSection — 16x16x16 block storage (ExtendedBlockStorage in Java).
// Java reference: net.minecraft.world.chunk.storage.ExtendedBlockStorage
// ═══════════════════════════════════════════════════════════════════════════

class ChunkSection {
public:
    /**
     * Java: ExtendedBlockStorage(int yBase, boolean hasSkylight)
     */
    explicit ChunkSection(int yBase, bool hasSkylight = true);

    // Block access
    Block* getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, Block* block);
    int getBlockMetadata(int x, int y, int z) const;
    void setBlockMetadata(int x, int y, int z, int meta);

    // Light access
    int getBlockLight(int x, int y, int z) const;
    void setBlockLight(int x, int y, int z, int val);
    int getSkyLight(int x, int y, int z) const;
    void setSkyLight(int x, int y, int z, int val);

    // Status
    bool isEmpty() const { return blockRefCount_ == 0; }
    bool needsRandomTick() const { return tickRefCount_ > 0; }
    int getYBase() const { return yBase_; }

    /**
     * Recalculates blockRefCount and tickRefCount by scanning all blocks.
     * Java: ExtendedBlockStorage.removeInvalidBlocks()
     */
    void recalcRefCounts();

    // Raw data access (for NBT serialization)
    const std::array<uint8_t, 4096>& getBlockLSBArray() const { return blockLSB_; }
    void setBlockLSBArray(const std::vector<uint8_t>& arr);
    NibbleArray* getBlockMSBArray() { return blockMSB_.get(); }
    void setBlockMSBArray(std::unique_ptr<NibbleArray> arr) { blockMSB_ = std::move(arr); }
    NibbleArray& getMetadataArray() { return metadata_; }
    void setMetadataArray(NibbleArray arr) { metadata_ = std::move(arr); }
    NibbleArray& getBlocklightArray() { return blocklight_; }
    void setBlocklightArray(NibbleArray arr) { blocklight_ = std::move(arr); }
    NibbleArray* getSkylightArray() { return skylight_.get(); }
    void setSkylightArray(std::unique_ptr<NibbleArray> arr) { skylight_ = std::move(arr); }

private:
    int yBase_;
    int blockRefCount_ = 0;
    int tickRefCount_ = 0;

    // Block IDs: LSB is mandatory, MSB is optional (only for IDs > 255)
    std::array<uint8_t, 4096> blockLSB_;
    std::unique_ptr<NibbleArray> blockMSB_;  // nullable, like Java

    NibbleArray metadata_;     // 4096 elements
    NibbleArray blocklight_;   // 4096 elements
    std::unique_ptr<NibbleArray> skylight_;  // nullable (nether has no skylight)
};

// ═══════════════════════════════════════════════════════════════════════════
// Chunk — 16x256x16 column of block data.
// Java reference: net.minecraft.world.chunk.Chunk (data portion only)
// ═══════════════════════════════════════════════════════════════════════════

class Chunk {
public:
    static constexpr int SECTION_COUNT = 16;
    static constexpr int BIOME_ARRAY_SIZE = 256; // 16x16

    int xPosition = 0;
    int zPosition = 0;

    // Height map — one int per column (16x16 = 256 entries)
    std::array<int32_t, 256> heightMap{};

    // Biome data — one byte per column
    std::array<uint8_t, BIOME_ARRAY_SIZE> biomes{};

    // Chunk sections (16 vertical slices, nullable like Java)
    std::array<std::unique_ptr<ChunkSection>, SECTION_COUNT> sections;

    // Flags matching Java's Chunk fields
    bool isTerrainPopulated = false;
    bool isLightPopulated = false;
    bool hasEntities = false;
    int64_t inhabitedTime = 0;
    bool isModified = false;  // Java: Chunk.isModified — set when block changes

    // Spawner placements from world generation — consumed on chunk load
    // to populate server spawnerStorage_ with correct mob types.
    struct SpawnerInfo {
        int32_t x, y, z;        // World coordinates
        std::string entityId;   // e.g. "Zombie", "Skeleton", "Spider"
    };
    std::vector<SpawnerInfo> pendingSpawners;

    // Chest placements from world generation — consumed on chunk load
    // to populate server chestStorage_ with correct loot tables.
    struct ChestInfo {
        int32_t x, y, z;        // World coordinates
        std::string lootTable;  // e.g. "desert_pyramid", "jungle_pyramid"
    };
    std::vector<ChestInfo> pendingChests;

    // Villager placements from village generation — consumed on chunk load
    // to spawn villagers via summonMob with correct profession types.
    // Java reference: StructureVillagePieces$Village.spawnVillagers()
    struct VillagerInfo {
        int32_t x, y, z;        // World coordinates
        int32_t profession;     // 0=farmer, 1=librarian, 2=priest, 3=blacksmith, 4=butcher
    };
    std::vector<VillagerInfo> pendingVillagers;

    Chunk() = default;
    Chunk(int x, int z) : xPosition(x), zPosition(z) {
        biomes.fill(0);
    }

    /**
     * Get block at world-relative coordinates within this chunk.
     * x, z are 0-15 (chunk-local), y is 0-255.
     */
    Block* getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, Block* block);
    int getBlockMetadata(int x, int y, int z) const;
    void setBlockMetadata(int x, int y, int z, int meta);

    /**
     * Mark chunk as modified (needs saving).
     * Java reference: Chunk.setChunkModified()
     */
    void setChunkModified() { isModified = true; }

    /**
     * Check if chunk needs saving.
     * Java reference: Chunk.needsSaving()
     */
    bool needsSaving() const { return isModified; }

    /**
     * Serialize chunk data to NBT (Level compound).
     * Java reference: AnvilChunkLoader.writeChunkToNBT()
     * Note: Entities and TileEntities are not yet serialized (requires entity system).
     */
    std::shared_ptr<nbt::NBTTagCompound> writeToNBT() const;

    /**
     * Deserialize chunk data from NBT (Level compound).
     * Java reference: AnvilChunkLoader.readChunkFromNBT()
     */
    static std::unique_ptr<Chunk> readFromNBT(const nbt::NBTTagCompound& levelTag);

    /**
     * Generate sky light map — initialize skylight values for all columns.
     * Java reference: Chunk.generateSkylightMap()
     *
     * For each (x,z) column, scans from Y=255 downward:
     *   - Sets skylight=15 for all blocks with lightOpacity=0 (air, glass, etc.)
     *   - Stops at the first block with lightOpacity>0 (opaque block)
     *   - Updates heightMap[] with the Y+1 of the highest opaque block
     *
     * Also initializes block light from light-emitting blocks.
     */
    void generateSkylightMap() {
        // Compute height map + set sky light
        for (int x = 0; x < 16; ++x) {
            for (int z = 0; z < 16; ++z) {
                int topY = 0;

                // Scan from top down to find highest opaque block
                for (int y = 255; y >= 0; --y) {
                    int blockId = 0;
                    int sectionIdx = y >> 4;
                    if (sections[sectionIdx]) {
                        auto* block = sections[sectionIdx]->getBlock(x, y & 15, z);
                        if (block) blockId = Block::getIdFromBlock(block);
                    }

                    auto* blockDef = Block::getBlockById(blockId);
                    int opacity = blockDef ? blockDef->getLightOpacity() : 0;

                    if (opacity > 0) {
                        topY = y + 1;
                        break;
                    }
                }

                heightMap[z * 16 + x] = topY;

                // Set skylight in existing sections only
                // (client fills skylight=15 for missing sections in overworld)
                for (int y = 255; y >= 0; --y) {
                    int sectionIdx = y >> 4;
                    if (!sections[sectionIdx]) continue;
                    int skyVal = (y >= topY) ? 15 : 0;
                    sections[sectionIdx]->setSkyLight(x, y & 15, z, skyVal);
                }
            }
        }

        // Initialize block light from light-emitting blocks
        for (int sIdx = 0; sIdx < SECTION_COUNT; ++sIdx) {
            if (!sections[sIdx]) continue;
            for (int x = 0; x < 16; ++x) {
                for (int y = 0; y < 16; ++y) {
                    for (int z = 0; z < 16; ++z) {
                        auto* block = sections[sIdx]->getBlock(x, y, z);
                        if (block) {
                            int emission = block->getLightValue();
                            if (emission > 0) {
                                sections[sIdx]->setBlockLight(x, y, z, emission);
                            }
                        }
                    }
                }
            }
        }

        isLightPopulated = true;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// RegionFile — Raw .mca file I/O for Anvil format.
// Java reference: net.minecraft.world.chunk.storage.RegionFile
//
// Region files store 32x32 chunks in 4096-byte sectors.
// Header: 4096 bytes of chunk offsets + 4096 bytes of timestamps.
// Each chunk is zlib-compressed NBT data.
//
// Thread safety: mutex-protected for concurrent access.
// ═══════════════════════════════════════════════════════════════════════════

class RegionFile {
public:
    explicit RegionFile(const std::string& path);
    ~RegionFile();

    // Non-copyable
    RegionFile(const RegionFile&) = delete;
    RegionFile& operator=(const RegionFile&) = delete;

    /**
     * Read chunk NBT data (decompressed) from the region file.
     * Java reference: RegionFile.getChunkDataInputStream()
     * Returns nullopt if chunk is not saved.
     */
    std::optional<std::vector<uint8_t>> readChunkData(int localX, int localZ);

    /**
     * Write chunk NBT data (will be zlib-compressed) to the region file.
     * Java reference: RegionFile.write(x, z, data, length)
     */
    bool writeChunkData(int localX, int localZ, const std::vector<uint8_t>& data);

    /**
     * Check if a chunk exists in this region.
     * Java reference: RegionFile.isChunkSaved()
     */
    bool isChunkSaved(int localX, int localZ) const;

    void close();

private:
    static bool outOfBounds(int x, int z) { return x < 0 || x >= 32 || z < 0 || z >= 32; }
    int getOffset(int x, int z) const { return offsets_[x + z * 32]; }
    void setOffset(int x, int z, int val);
    void setTimestamp(int x, int z, int val);

    void writeSector(int sectorNum, const uint8_t* data, int length);

    std::string path_;
    std::fstream file_;
    mutable std::mutex mutex_;
    std::array<int32_t, 1024> offsets_{};
    std::array<int32_t, 1024> timestamps_{};
    std::vector<bool> sectorFree_;
};

} // namespace mccpp
