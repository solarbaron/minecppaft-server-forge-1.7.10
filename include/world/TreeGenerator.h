#pragma once
// TreeGenerator — tree placement as world decoration.
// Ported from abz.java (WorldGenTrees), aca.java (WorldGenBigTree),
// acb.java (WorldGenForest/Birch), acf.java (WorldGenTaiga2/Spruce).
//
// Vanilla tree types:
//   - Oak: 4-6 trunk, 3-wide sphere canopy (log=17:0, leaves=18:0)
//   - Birch: 5-7 trunk, 2-wide canopy (log=17:2, leaves=18:2)
//   - Spruce: 6-10 trunk, cone-shaped canopy (log=17:1, leaves=18:1)
//   - Jungle: 4-8 trunk with vines (log=17:3, leaves=18:3)
//   - Big oak: 8-14 trunk, large sphere canopy
//
// Block IDs:
//   17 = Log (meta: 0=oak, 1=spruce, 2=birch, 3=jungle)
//   18 = Leaves (meta: 0=oak, 1=spruce, 2=birch, 3=jungle)

#include <cstdint>
#include <random>
#include <functional>
#include <algorithm>

namespace mc {

enum class TreeType : uint8_t {
    OAK       = 0,
    SPRUCE    = 1,
    BIRCH     = 2,
    JUNGLE    = 3,
    BIG_OAK   = 4,
    ACACIA    = 5,
};

class TreeGenerator {
public:
    using SetBlockFn = std::function<void(int,int,int,uint16_t,uint8_t)>;
    using GetBlockFn = std::function<uint16_t(int,int,int)>;

    // Generate a tree at world position (x, baseY, z)
    // baseY should be the surface block Y (the block on top of which the tree grows)
    static bool generate(TreeType type, int x, int baseY, int z,
                         std::mt19937& rng, GetBlockFn getBlock, SetBlockFn setBlock) {
        switch (type) {
            case TreeType::OAK:     return generateOak(x, baseY, z, rng, getBlock, setBlock);
            case TreeType::BIRCH:   return generateBirch(x, baseY, z, rng, getBlock, setBlock);
            case TreeType::SPRUCE:  return generateSpruce(x, baseY, z, rng, getBlock, setBlock);
            case TreeType::JUNGLE:  return generateJungle(x, baseY, z, rng, getBlock, setBlock);
            case TreeType::BIG_OAK: return generateBigOak(x, baseY, z, rng, getBlock, setBlock);
            default: return false;
        }
    }

    // Select tree type based on biome
    static TreeType treeForBiome(uint8_t biomeId) {
        switch (biomeId) {
            case 5:  // Taiga
            case 12: // Ice Plains
            case 13: // Ice Mountains
                return TreeType::SPRUCE;
            case 27: // Birch Forest
                return TreeType::BIRCH;
            case 21: // Jungle
                return TreeType::JUNGLE;
            case 29: // Roofed Forest
                return TreeType::BIG_OAK;
            default:
                return TreeType::OAK;
        }
    }

private:
    // ---- Oak tree: 4-6 trunk, sphere canopy ----
    // Reference: abz.java (WorldGenTrees)
    static bool generateOak(int x, int y, int z, std::mt19937& rng,
                            GetBlockFn getBlock, SetBlockFn setBlock) {
        int height = 4 + (rng() % 3); // 4-6

        // Check space
        if (y + height + 2 > 255) return false;
        uint16_t below = getBlock(x, y, z);
        if (below != 2 && below != 3) return false; // Needs grass or dirt

        // Place dirt under trunk
        setBlock(x, y, z, 3, 0);

        // Trunk
        for (int dy = 1; dy <= height; ++dy) {
            setBlock(x, y + dy, z, 17, 0); // Oak log
        }

        // Canopy: sphere at top
        int canopyBase = y + height - 2;
        for (int dy = canopyBase; dy <= y + height + 1; ++dy) {
            int radius = (dy <= y + height - 1) ? 2 : 1;
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    if (dx == 0 && dz == 0 && dy <= y + height) continue; // trunk
                    if (std::abs(dx) == radius && std::abs(dz) == radius &&
                        (rng() % 2) == 0) continue; // Corner cutoff
                    uint16_t existing = getBlock(x + dx, dy, z + dz);
                    if (existing == 0 || existing == 18) {
                        setBlock(x + dx, dy, z + dz, 18, 0); // Oak leaves
                    }
                }
            }
        }

        return true;
    }

    // ---- Birch tree: 5-7 trunk, smaller canopy ----
    // Reference: acb.java (WorldGenForest)
    static bool generateBirch(int x, int y, int z, std::mt19937& rng,
                              GetBlockFn getBlock, SetBlockFn setBlock) {
        int height = 5 + (rng() % 3); // 5-7

        if (y + height + 2 > 255) return false;
        uint16_t below = getBlock(x, y, z);
        if (below != 2 && below != 3) return false;

        setBlock(x, y, z, 3, 0);

        for (int dy = 1; dy <= height; ++dy) {
            setBlock(x, y + dy, z, 17, 2); // Birch log
        }

        int canopyBase = y + height - 2;
        for (int dy = canopyBase; dy <= y + height + 1; ++dy) {
            int radius = (dy <= y + height - 1) ? 2 : 1;
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    if (dx == 0 && dz == 0 && dy <= y + height) continue;
                    if (std::abs(dx) == radius && std::abs(dz) == radius &&
                        (rng() % 2) == 0) continue;
                    if (getBlock(x + dx, dy, z + dz) == 0) {
                        setBlock(x + dx, dy, z + dz, 18, 2); // Birch leaves
                    }
                }
            }
        }

        return true;
    }

    // ---- Spruce tree: 6-10 trunk, cone canopy ----
    // Reference: acf.java (WorldGenTaiga2)
    static bool generateSpruce(int x, int y, int z, std::mt19937& rng,
                               GetBlockFn getBlock, SetBlockFn setBlock) {
        int height = 6 + (rng() % 5); // 6-10

        if (y + height + 2 > 255) return false;
        uint16_t below = getBlock(x, y, z);
        if (below != 2 && below != 3) return false;

        setBlock(x, y, z, 3, 0);

        for (int dy = 1; dy <= height; ++dy) {
            setBlock(x, y + dy, z, 17, 1); // Spruce log
        }

        // Cone-shaped canopy
        int canopyStart = y + 3;
        for (int dy = canopyStart; dy <= y + height + 1; ++dy) {
            int layerFromTop = y + height + 1 - dy;
            int radius = 1 + layerFromTop / 2;
            radius = std::min(radius, 3);

            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    if (dx == 0 && dz == 0 && dy <= y + height) continue;
                    // Diamond shape for spruce
                    if (std::abs(dx) + std::abs(dz) > radius + 1) continue;
                    if (getBlock(x + dx, dy, z + dz) == 0) {
                        setBlock(x + dx, dy, z + dz, 18, 1); // Spruce leaves
                    }
                }
            }
        }

        // Top spike
        setBlock(x, y + height + 1, z, 18, 1);

        return true;
    }

    // ---- Jungle tree: 4-8 trunk with vines ----
    // Reference: acy.java (WorldGenMegaJungle simplified)
    static bool generateJungle(int x, int y, int z, std::mt19937& rng,
                               GetBlockFn getBlock, SetBlockFn setBlock) {
        int height = 4 + (rng() % 5); // 4-8

        if (y + height + 2 > 255) return false;
        uint16_t below = getBlock(x, y, z);
        if (below != 2 && below != 3) return false;

        setBlock(x, y, z, 3, 0);

        for (int dy = 1; dy <= height; ++dy) {
            setBlock(x, y + dy, z, 17, 3); // Jungle log
        }

        // Wider canopy for jungle
        int canopyBase = y + height - 2;
        for (int dy = canopyBase; dy <= y + height + 1; ++dy) {
            int radius = (dy <= y + height - 1) ? 3 : 2;
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    if (dx == 0 && dz == 0 && dy <= y + height) continue;
                    if (std::abs(dx) == radius && std::abs(dz) == radius) continue;
                    if (getBlock(x + dx, dy, z + dz) == 0) {
                        setBlock(x + dx, dy, z + dz, 18, 3); // Jungle leaves
                    }
                }
            }
        }

        // Vines on sides (block 106)
        for (int dy = canopyBase; dy <= y + height; ++dy) {
            for (int side = 0; side < 4; ++side) {
                int vx = x + ((side == 0) ? -3 : (side == 1) ? 3 : 0);
                int vz = z + ((side == 2) ? -3 : (side == 3) ? 3 : 0);
                if (getBlock(vx, dy, vz) == 0 && (rng() % 3) == 0) {
                    setBlock(vx, dy, vz, 106, 0); // Vine
                }
            }
        }

        return true;
    }

    // ---- Big oak: taller with larger canopy ----
    // Reference: aca.java (WorldGenBigTree simplified)
    static bool generateBigOak(int x, int y, int z, std::mt19937& rng,
                               GetBlockFn getBlock, SetBlockFn setBlock) {
        int height = 8 + (rng() % 7); // 8-14

        if (y + height + 3 > 255) return false;
        uint16_t below = getBlock(x, y, z);
        if (below != 2 && below != 3) return false;

        setBlock(x, y, z, 3, 0);

        for (int dy = 1; dy <= height; ++dy) {
            setBlock(x, y + dy, z, 17, 0); // Oak log
        }

        // Large sphere canopy
        int canopyCenter = y + height - 1;
        int canopyRadius = 3 + (rng() % 2);
        for (int dy = canopyCenter - canopyRadius; dy <= canopyCenter + canopyRadius; ++dy) {
            int yDist = std::abs(dy - canopyCenter);
            int layerRadius = canopyRadius - yDist / 2;
            for (int dx = -layerRadius; dx <= layerRadius; ++dx) {
                for (int dz = -layerRadius; dz <= layerRadius; ++dz) {
                    if (dx == 0 && dz == 0 && dy <= y + height) continue;
                    double dist = std::sqrt(dx*dx + dz*dz + yDist*yDist * 0.5);
                    if (dist <= canopyRadius + 0.5) {
                        if (getBlock(x + dx, dy, z + dz) == 0) {
                            setBlock(x + dx, dy, z + dz, 18, 0);
                        }
                    }
                }
            }
        }

        return true;
    }
};

} // namespace mc
