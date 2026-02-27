#pragma once
// StructureGenerator — dungeon rooms and mineshaft corridors.
// Ported from aby.java (MapGenDungeons/WorldGenDungeons) and
// abp.java (MapGenMineshaft/StructureMineshaftPieces).
//
// Vanilla structures (simplified):
//   - Dungeons: 7x7x5 cobblestone rooms with mob spawner + 2 loot chests
//   - Mineshafts: 3-wide corridors with fence supports, planks, rails, torches
//
// Block IDs:
//   4 = Cobblestone, 48 = Mossy Cobblestone
//   52 = Mob Spawner, 54 = Chest
//   85 = Fence, 5 = Planks, 66 = Rail, 50 = Torch

#include <cstdint>
#include <random>
#include <vector>
#include <functional>
#include <algorithm>

namespace mc {

// Loot table for dungeon chests — from aby.java
struct LootEntry {
    int16_t itemId;
    int8_t minCount;
    int8_t maxCount;
    int weight;
};

static const LootEntry DUNGEON_LOOT[] = {
    {264, 1, 3, 3},   // Diamond
    {265, 1, 4, 10},  // Iron Ingot
    {297, 1, 1, 10},  // Bread
    {296, 1, 4, 10},  // Wheat
    {289, 1, 4, 10},  // Gunpowder
    {287, 1, 4, 10},  // String
    {39,  1, 2, 10},  // Red mushroom (bucket placeholder)
    {262, 1, 8, 10},  // Arrow -> technically saddle in vanilla but simplified
    {263, 1, 4, 10},  // Coal
    {331, 1, 4, 10},  // Redstone
    {344, 1, 1, 10},  // Egg (placeholder for music disc)
    {266, 1, 3, 5},   // Gold Ingot
    {388, 1, 2, 2},   // Emerald
};

// Spawner mob types for dungeons
static const char* DUNGEON_MOBS[] = {
    "Zombie", "Skeleton", "Spider"
};

class StructureGenerator {
public:
    using SetBlockFn = std::function<void(int,int,int,uint16_t,uint8_t)>;
    using GetBlockFn = std::function<uint16_t(int,int,int)>;

    // Generate dungeons in a chunk (called during terrain gen)
    static void generateDungeons(int cx, int cz, uint64_t seed,
                                  GetBlockFn getBlock, SetBlockFn setBlock) {
        std::mt19937 rng(seed ^ (cx * 395728347LL + cz * 793456289LL));

        // Vanilla: 8 attempts per chunk
        int attempts = 8;
        for (int a = 0; a < attempts; ++a) {
            int bx = cx * 16 + (rng() % 16);
            int by = 10 + (rng() % 40); // y 10-50
            int bz = cz * 16 + (rng() % 16);

            // Check if location is suitable (must be in stone/air boundary)
            if (getBlock(bx, by, bz) != 0) continue; // Needs air at center
            if (getBlock(bx, by - 1, bz) == 0) continue; // Needs floor

            // Count solid walls around
            int solidWalls = 0;
            if (getBlock(bx - 4, by, bz) != 0) solidWalls++;
            if (getBlock(bx + 4, by, bz) != 0) solidWalls++;
            if (getBlock(bx, by, bz - 4) != 0) solidWalls++;
            if (getBlock(bx, by, bz + 4) != 0) solidWalls++;
            if (solidWalls < 3) continue;

            // Place dungeon room (7x5x7, centered on bx,by,bz)
            placeDungeonRoom(bx, by, bz, rng, getBlock, setBlock);
        }
    }

    // Generate mineshaft corridors in a chunk
    static void generateMineshafts(int cx, int cz, uint64_t seed,
                                     GetBlockFn getBlock, SetBlockFn setBlock) {
        std::mt19937 rng(seed ^ (cx * 246813579LL + cz * 135792468LL));

        // 1 in 100 chunks gets a mineshaft start
        if (rng() % 100 != 0) return;

        int startX = cx * 16 + 8;
        int startY = 20 + (rng() % 20); // y 20-40
        int startZ = cz * 16 + 8;

        // Generate 3-8 corridor segments
        int numCorridors = 3 + (rng() % 6);
        int curX = startX, curY = startY, curZ = startZ;

        for (int c = 0; c < numCorridors; ++c) {
            int dir = rng() % 4; // 0=N, 1=S, 2=E, 3=W
            int length = 8 + (rng() % 12); // 8-20 blocks

            int dx = 0, dz = 0;
            switch (dir) {
                case 0: dz = -1; break;
                case 1: dz =  1; break;
                case 2: dx =  1; break;
                case 3: dx = -1; break;
            }

            for (int step = 0; step < length; ++step) {
                int px = curX + dx * step;
                int pz = curZ + dz * step;

                // Only place within our chunk
                if (px < cx * 16 || px >= (cx + 1) * 16) continue;
                if (pz < cz * 16 || pz >= (cz + 1) * 16) continue;

                // 3-wide, 3-tall corridor
                for (int w = -1; w <= 1; ++w) {
                    int wx = px + (dz != 0 ? w : 0);
                    int wz = pz + (dx != 0 ? w : 0);

                    // Carve air
                    for (int h = 0; h < 3; ++h) {
                        setBlock(wx, curY + h, wz, 0, 0); // Air
                    }

                    // Floor planks
                    if (getBlock(wx, curY - 1, wz) != 0) {
                        setBlock(wx, curY - 1, wz, 5, 0); // Oak planks
                    }
                }

                // Fence supports every 4 blocks
                if (step % 4 == 0) {
                    int fx1 = px + (dz != 0 ? -1 : 0);
                    int fz1 = pz + (dx != 0 ? -1 : 0);
                    int fx2 = px + (dz != 0 ?  1 : 0);
                    int fz2 = pz + (dx != 0 ?  1 : 0);

                    setBlock(fx1, curY, fz1, 85, 0);     // Fence
                    setBlock(fx1, curY + 1, fz1, 85, 0); // Fence
                    setBlock(fx2, curY, fz2, 85, 0);     // Fence
                    setBlock(fx2, curY + 1, fz2, 85, 0); // Fence

                    // Plank beam across top
                    for (int w = -1; w <= 1; ++w) {
                        int bx2 = px + (dz != 0 ? w : 0);
                        int bz2 = pz + (dx != 0 ? w : 0);
                        setBlock(bx2, curY + 2, bz2, 5, 0); // Planks
                    }
                }

                // Rail on center
                setBlock(px, curY, pz, 66, 0); // Rail

                // Torch every 8 blocks
                if (step % 8 == 0) {
                    setBlock(px + (dz != 0 ? -1 : 0), curY + 1,
                             pz + (dx != 0 ? -1 : 0), 50, 0); // Torch
                }
            }

            curX += dx * length;
            curZ += dz * length;
            // Slight Y variation
            curY += static_cast<int>(rng() % 3) - 1;
            curY = std::clamp(curY, 10, 50);
        }
    }

private:
    static void placeDungeonRoom(int cx, int cy, int cz, std::mt19937& rng,
                                  GetBlockFn getBlock, SetBlockFn setBlock) {
        int halfW = 3, halfL = 3, height = 4;

        // Build walls, floor, ceiling
        for (int dx = -halfW; dx <= halfW; ++dx) {
            for (int dz = -halfL; dz <= halfL; ++dz) {
                // Floor
                uint16_t floorBlock = (rng() % 4 == 0) ? 48 : 4; // Mossy or cobble
                setBlock(cx + dx, cy - 1, cz + dz, floorBlock, 0);

                // Ceiling
                setBlock(cx + dx, cy + height, cz + dz, 4, 0);

                // Interior air
                for (int dy = 0; dy < height; ++dy) {
                    if (std::abs(dx) == halfW || std::abs(dz) == halfL) {
                        // Walls — cobblestone with holes
                        if (rng() % 4 != 0) {
                            uint16_t wallBlock = (rng() % 3 == 0) ? 48 : 4;
                            setBlock(cx + dx, cy + dy, cz + dz, wallBlock, 0);
                        }
                    } else {
                        setBlock(cx + dx, cy + dy, cz + dz, 0, 0); // Air
                    }
                }
            }
        }

        // Mob spawner at center
        setBlock(cx, cy, cz, 52, 0);

        // Two loot chests
        int chest1Dir = rng() % 4;
        static constexpr int chestDx[] = {-2, 2, 0, 0};
        static constexpr int chestDz[] = {0, 0, -2, 2};

        setBlock(cx + chestDx[chest1Dir], cy, cz + chestDz[chest1Dir], 54, 0);

        int chest2Dir = (chest1Dir + 1 + rng() % 3) % 4;
        setBlock(cx + chestDx[chest2Dir], cy, cz + chestDz[chest2Dir], 54, 0);
    }
};

} // namespace mc
