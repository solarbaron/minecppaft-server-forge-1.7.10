#pragma once
// ChunkSerializer — converts ChunkColumn to/from NBT for Anvil persistence.
// Matches vanilla's chunk NBT format used by aqp.java (AnvilChunkLoader).

#include <memory>
#include <cstring>
#include "world/Chunk.h"
#include "nbt/NBT.h"

namespace mc {

class ChunkSerializer {
public:
    // Serialize ChunkColumn to NBT compound
    static std::shared_ptr<nbt::NBTTagCompound> serialize(const ChunkColumn& chunk) {
        auto root = std::make_shared<nbt::NBTTagCompound>();
        auto level = std::make_shared<nbt::NBTTagCompound>();

        level->setInt("xPos", chunk.chunkX);
        level->setInt("zPos", chunk.chunkZ);

        // Sections
        auto sections = std::make_shared<nbt::NBTTagList>();
        for (int i = 0; i < 16; ++i) {
            if (!chunk.sections[i] || chunk.sections[i]->isEmpty()) continue;

            auto section = std::make_shared<nbt::NBTTagCompound>();
            section->setByte("Y", static_cast<int8_t>(i));

            // Blocks (4096 bytes) — copy uint8_t* to vector<int8_t>
            auto blocks = std::make_shared<nbt::NBTTagByteArray>();
            blocks->value.resize(4096);
            std::memcpy(blocks->value.data(),
                        chunk.sections[i]->blockIdArray(), 4096);
            section->tags["Blocks"] = blocks;

            // Data (2048 bytes)
            auto data = std::make_shared<nbt::NBTTagByteArray>();
            data->value.resize(2048);
            std::memcpy(data->value.data(),
                        chunk.sections[i]->metadataArray(), 2048);
            section->tags["Data"] = data;

            // BlockLight (2048 bytes)
            auto blockLight = std::make_shared<nbt::NBTTagByteArray>();
            blockLight->value.resize(2048);
            std::memcpy(blockLight->value.data(),
                        chunk.sections[i]->blockLightArray(), 2048);
            section->tags["BlockLight"] = blockLight;

            // SkyLight (2048 bytes)
            auto skyLight = std::make_shared<nbt::NBTTagByteArray>();
            skyLight->value.resize(2048);
            std::memcpy(skyLight->value.data(),
                        chunk.sections[i]->skyLightArray(), 2048);
            section->tags["SkyLight"] = skyLight;

            // Add array (optional)
            if (chunk.sections[i]->hasAdd()) {
                auto add = std::make_shared<nbt::NBTTagByteArray>();
                add->value.resize(2048);
                std::memcpy(add->value.data(),
                            chunk.sections[i]->addArray(), 2048);
                section->tags["Add"] = add;
            }

            sections->add(section);
        }
        level->setList("Sections", sections);

        // Biomes (256 bytes)
        auto biomes = std::make_shared<nbt::NBTTagByteArray>();
        biomes->value.resize(256);
        std::memcpy(biomes->value.data(), chunk.biomes.data(), 256);
        level->tags["Biomes"] = biomes;

        root->tags["Level"] = level;
        return root;
    }

    // Deserialize NBT compound to ChunkColumn
    static std::unique_ptr<ChunkColumn> deserialize(
            const std::shared_ptr<nbt::NBTTagCompound>& root) {
        if (!root || !root->hasKey("Level")) return nullptr;

        auto level = std::dynamic_pointer_cast<nbt::NBTTagCompound>(
            root->tags.at("Level"));
        if (!level) return nullptr;

        int cx = level->getInt("xPos");
        int cz = level->getInt("zPos");
        auto chunk = std::make_unique<ChunkColumn>(cx, cz);

        // Sections
        if (level->hasKey("Sections")) {
            auto sections = level->getList("Sections", nbt::TAG_Compound);
            for (auto& tag : sections->tags) {
                auto sec = std::dynamic_pointer_cast<nbt::NBTTagCompound>(tag);
                if (!sec) continue;

                int y = sec->getByte("Y");
                if (y < 0 || y >= 16) continue;

                auto section = std::make_unique<ChunkSection>();
                section->yIndex = y;

                // Load blocks + add data, then set via setBlockId
                std::shared_ptr<nbt::NBTTagByteArray> blocksArr, addArr;
                if (sec->hasKey("Blocks"))
                    blocksArr = std::dynamic_pointer_cast<nbt::NBTTagByteArray>(
                        sec->tags.at("Blocks"));
                if (sec->hasKey("Add"))
                    addArr = std::dynamic_pointer_cast<nbt::NBTTagByteArray>(
                        sec->tags.at("Add"));

                if (blocksArr && static_cast<int>(blocksArr->value.size()) == 4096) {
                    for (int idx = 0; idx < 4096; ++idx) {
                        int lx = idx & 0xF;
                        int lz = (idx >> 4) & 0xF;
                        int ly = (idx >> 8) & 0xF;

                        uint16_t id = static_cast<uint8_t>(blocksArr->value[idx]);
                        if (addArr && static_cast<int>(addArr->value.size()) == 2048) {
                            uint8_t addByte = static_cast<uint8_t>(addArr->value[idx >> 1]);
                            uint8_t addVal = (idx & 1) ?
                                ((addByte >> 4) & 0xF) : (addByte & 0xF);
                            id |= (static_cast<uint16_t>(addVal) << 8);
                        }
                        section->setBlockId(lx, ly, lz, id);
                    }
                }

                // Load metadata
                if (sec->hasKey("Data")) {
                    auto data = std::dynamic_pointer_cast<nbt::NBTTagByteArray>(
                        sec->tags.at("Data"));
                    if (data && static_cast<int>(data->value.size()) == 2048) {
                        for (int idx = 0; idx < 4096; ++idx) {
                            int lx = idx & 0xF;
                            int lz = (idx >> 4) & 0xF;
                            int ly = (idx >> 8) & 0xF;
                            uint8_t byte = static_cast<uint8_t>(data->value[idx >> 1]);
                            uint8_t val = (idx & 1) ?
                                ((byte >> 4) & 0xF) : (byte & 0xF);
                            section->setMetadata(lx, ly, lz, val);
                        }
                    }
                }

                chunk->sections[y] = std::move(section);
            }
        }

        // Biomes
        if (level->hasKey("Biomes")) {
            auto biomes = std::dynamic_pointer_cast<nbt::NBTTagByteArray>(
                level->tags.at("Biomes"));
            if (biomes && static_cast<int>(biomes->value.size()) == 256) {
                std::memcpy(chunk->biomes.data(), biomes->value.data(), 256);
            }
        }

        return chunk;
    }
};

} // namespace mc
