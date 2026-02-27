#pragma once
// BlockTickHandler — random block tick system.
// Ported from mt.java (WorldServer.func_147456_g) tick logic.
//
// Vanilla random tick mechanics:
//   - Each game tick, for each loaded chunk section with blocks:
//     - 3 random block positions are chosen (randomTickSpeed = 3)
//     - The block at that position gets a random tick
//   - Blocks that respond to random ticks:
//     - Crops (wheat 59, carrots 141, potatoes 142): grow through stages 0-7
//     - Saplings (6): grow into trees
//     - Farmland (60): check moisture, revert to dirt if dry
//     - Grass (2): spread to adjacent dirt
//     - Mycelium (110): spread to adjacent dirt
//     - Ice (79): melt in light
//     - Snow layer (78): melt in light
//     - Fire (51): spread / extinguish
//     - Leaves (18): decay check
//
// We implement crop growth with metadata stages and farmland moisture.

#include <cstdint>
#include <random>
#include <chrono>
#include <vector>
#include <functional>

#include "world/Chunk.h"
#include "world/Block.h"

namespace mc {

// Forward declare World to avoid circular include
class World;

// Block change notification — sent back to caller for broadcasting
struct BlockChange {
    int x, y, z;
    uint16_t blockId;
    uint8_t meta;
};

class BlockTickHandler {
public:
    BlockTickHandler() : rng_(static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count())) {}

    // Process random ticks for a chunk
    // Returns list of block changes that need to be broadcast
    std::vector<BlockChange> tickChunk(ChunkColumn& chunk, int randomTickSpeed = 3) {
        std::vector<BlockChange> changes;

        for (int s = 0; s < 16; ++s) {
            if (!chunk.sections[s] || chunk.sections[s]->isEmpty()) continue;

            for (int i = 0; i < randomTickSpeed; ++i) {
                // Pick random position within section
                int rx = rng_() & 0xF;
                int ry = rng_() & 0xF;
                int rz = rng_() & 0xF;

                int worldY = s * 16 + ry;
                uint16_t blockId = chunk.sections[s]->getBlockId(rx, ry, rz);
                uint8_t meta = chunk.sections[s]->getMetadata(rx, ry, rz);

                // Process tick for this block
                auto change = tickBlock(chunk, rx, worldY, rz, blockId, meta);
                if (change) {
                    change->x += chunk.chunkX * 16;
                    change->z += chunk.chunkZ * 16;
                    changes.push_back(*change);
                }
            }
        }

        return changes;
    }

private:
    std::mt19937 rng_;

    std::optional<BlockChange> tickBlock(ChunkColumn& chunk,
                                          int x, int y, int z,
                                          uint16_t blockId, uint8_t meta) {
        switch (blockId) {
            // ========================================
            // Crops: wheat (59), carrots (141), potatoes (142)
            // Metadata 0-7, grows through stages
            // Growth chance: ~25% per random tick with light >= 9
            // ========================================
            case 59:   // Wheat
            case 141:  // Carrots
            case 142:  // Potatoes
                return tickCrop(chunk, x, y, z, blockId, meta);

            // ========================================
            // Sapling (6): grows into tree
            // meta bit 3 = ready to grow (second stage)
            // ========================================
            case 6:
                return tickSapling(chunk, x, y, z, meta);

            // ========================================
            // Farmland (60): moisture check
            // meta 0 = dry, 1-7 = wet
            // ========================================
            case 60:
                return tickFarmland(chunk, x, y, z, meta);

            // ========================================
            // Grass (2): spread to adjacent dirt
            // ========================================
            case 2:
                return tickGrass(chunk, x, y, z);

            // ========================================
            // Leaves (18): simplified decay
            // ========================================
            case 18:
                return tickLeaves(chunk, x, y, z, meta);

            default:
                return std::nullopt;
        }
    }

    std::optional<BlockChange> tickCrop(ChunkColumn& chunk,
                                         int x, int y, int z,
                                         uint16_t blockId, uint8_t meta) {
        if (meta >= 7) return std::nullopt; // Fully grown

        // Growth chance: vanilla uses light level and growth rate formula
        // Simplified: ~25% chance per random tick
        if ((rng_() % 4) != 0) return std::nullopt;

        // Check if farmland below
        if (y > 0) {
            uint16_t below = chunk.getBlock(x, y - 1, z);
            if (below != 60) return std::nullopt; // Need farmland
        }

        // Grow one stage
        uint8_t newMeta = meta + 1;
        chunk.setBlock(x, y, z, blockId, newMeta);
        return BlockChange{x, y, z, blockId, newMeta};
    }

    std::optional<BlockChange> tickSapling(ChunkColumn& chunk,
                                            int x, int y, int z, uint8_t meta) {
        uint8_t stage = (meta >> 3) & 1;
        uint8_t type = meta & 0x7; // Sapling type (oak, spruce, birch, jungle)

        if (stage == 0) {
            // First tick: set ready-to-grow flag
            if ((rng_() % 7) != 0) return std::nullopt; // ~14% chance
            uint8_t newMeta = type | 0x8; // Set bit 3
            chunk.setBlock(x, y, z, 6, newMeta);
            return BlockChange{x, y, z, 6, newMeta};
        } else {
            // Second tick: grow into tree (simplified: just place a log)
            if ((rng_() % 7) != 0) return std::nullopt;

            // Simplified tree: 4-6 blocks of log + leaves around top
            int height = 4 + (rng_() % 3);

            // Place logs
            for (int ly = 0; ly < height && y + ly < 256; ++ly) {
                chunk.setBlock(x, y + ly, z, 17, type & 0x3); // Log
            }

            // Place leaves around top
            int topY = y + height;
            for (int lx = -2; lx <= 2; ++lx) {
                for (int lz = -2; lz <= 2; ++lz) {
                    for (int ly = -2; ly <= 0; ++ly) {
                        int bx = x + lx, by = topY + ly, bz = z + lz;
                        if (bx < 0 || bx > 15 || bz < 0 || bz > 15 || by < 0 || by > 255)
                            continue;
                        if (std::abs(lx) + std::abs(lz) > 3) continue;
                        if (chunk.getBlock(bx, by, bz) == 0) {
                            chunk.setBlock(bx, by, bz, 18, type & 0x3); // Leaves
                        }
                    }
                }
            }

            return BlockChange{x, y, z, 17, static_cast<uint8_t>(type & 0x3)};
        }
    }

    std::optional<BlockChange> tickFarmland(ChunkColumn& chunk,
                                             int x, int y, int z, uint8_t meta) {
        // Check for water nearby (simplified: check if any water in 4-block radius at same Y)
        bool hasWater = false;
        for (int dx = -4; dx <= 4 && !hasWater; ++dx) {
            for (int dz = -4; dz <= 4 && !hasWater; ++dz) {
                int bx = x + dx, bz = z + dz;
                if (bx >= 0 && bx < 16 && bz >= 0 && bz < 16) {
                    uint16_t block = chunk.getBlock(bx, y, bz);
                    if (block == 8 || block == 9) hasWater = true; // Water / flowing water
                }
            }
        }

        if (hasWater) {
            if (meta < 7) {
                chunk.setBlock(x, y, z, 60, 7); // Max moisture
                return BlockChange{x, y, z, 60, 7};
            }
        } else {
            if (meta > 0) {
                // Dry out
                uint8_t newMeta = meta - 1;
                chunk.setBlock(x, y, z, 60, newMeta);
                return BlockChange{x, y, z, 60, newMeta};
            } else {
                // Check if crop above — if not, revert to dirt
                uint16_t above = (y < 255) ? chunk.getBlock(x, y + 1, z) : 0;
                if (above != 59 && above != 141 && above != 142) {
                    chunk.setBlock(x, y, z, 3, 0); // Dirt
                    return BlockChange{x, y, z, 3, 0};
                }
            }
        }
        return std::nullopt;
    }

    std::optional<BlockChange> tickGrass(ChunkColumn& chunk,
                                          int x, int y, int z) {
        // Spread to adjacent dirt blocks (simplified)
        if ((rng_() % 4) != 0) return std::nullopt;

        // Pick random adjacent position
        int dx = (rng_() % 3) - 1;
        int dz = (rng_() % 3) - 1;
        int dy = (rng_() % 3) - 1;

        int bx = x + dx, by = y + dy, bz = z + dz;
        if (bx < 0 || bx > 15 || bz < 0 || bz > 15 || by < 0 || by > 255)
            return std::nullopt;

        if (chunk.getBlock(bx, by, bz) == 3) { // Dirt
            // Check if block above is transparent (air)
            if (by + 1 <= 255 && chunk.getBlock(bx, by + 1, bz) == 0) {
                chunk.setBlock(bx, by, bz, 2, 0); // Grass
                return BlockChange{bx + chunk.chunkX * 16, by,
                                   bz + chunk.chunkZ * 16, 2, 0};
            }
        }
        return std::nullopt;
    }

    std::optional<BlockChange> tickLeaves(ChunkColumn& chunk,
                                           int x, int y, int z, uint8_t meta) {
        // Simplified leaf decay: if no log nearby, decay
        // Check bit 2 (player placed flag) — don't decay player-placed leaves
        if (meta & 0x4) return std::nullopt;

        // Only decay occasionally
        if ((rng_() % 20) != 0) return std::nullopt;

        // Check for nearby logs in 4-block radius
        bool hasLog = false;
        for (int dx = -4; dx <= 4 && !hasLog; ++dx) {
            for (int dy = -4; dy <= 4 && !hasLog; ++dy) {
                for (int dz = -4; dz <= 4 && !hasLog; ++dz) {
                    int bx = x + dx, by = y + dy, bz = z + dz;
                    if (bx < 0 || bx > 15 || bz < 0 || bz > 15 || by < 0 || by > 255)
                        continue;
                    uint16_t block = chunk.getBlock(bx, by, bz);
                    if (block == 17 || block == 162) hasLog = true; // Log / Acacia log
                }
            }
        }

        if (!hasLog) {
            chunk.setBlock(x, y, z, 0, 0); // Air (decayed)
            return BlockChange{x, y, z, 0, 0};
        }
        return std::nullopt;
    }
};

} // namespace mc
