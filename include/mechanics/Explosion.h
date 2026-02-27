#pragma once
// Explosion — vanilla 1.7.10 explosion mechanics.
// Ported from the obfuscated Explosion class (aht.java / MCP: Explosion).
//
// Algorithm:
//   Phase A (doExplosionA): Cast 1352 rays (16x16x16 cube surface)
//     from center outward, decrementing power by block resistance.
//     Collect affected blocks. Apply entity damage & knockback.
//   Phase B (doExplosionB): Destroy affected blocks, spawn particles,
//     play sound, apply effects.
//
// Reference constants from MCP:
//   - Ray step: 0.3 (per iteration along ray)
//   - Power decay per step: -(0.225 + random * 0.3)
//   - Blast resistance factor: (resistance + 0.3) * 0.3
//   - Entity exposure: fraction of AABB visible from center
//   - Damage: (1.0 - distance/(power*2)) * exposure * 7 + 1

#include <cstdint>
#include <cmath>
#include <vector>
#include <unordered_set>
#include <random>
#include <functional>
#include "world/Block.h"

namespace mc {

// Forward declarations — explosion callbacks
using BlockGetFunc     = std::function<uint16_t(int, int, int)>;
using BlockSetFunc     = std::function<void(int, int, int, uint16_t)>;
using BlockResistFunc  = std::function<float(uint16_t)>;

struct ExplosionResult {
    std::vector<std::tuple<int, int, int>> affectedBlocks;
    std::vector<std::tuple<int, float>> entityDamages; // entityId, damage
};

class Explosion {
public:
    double x, y, z;
    float  power;
    bool   causesFire;
    bool   destroysBlocks;

    Explosion(double px, double py, double pz, float pw, bool fire = false, bool destroy = true)
        : x(px), y(py), z(pz), power(pw), causesFire(fire), destroysBlocks(destroy),
          rng_(std::random_device{}()) {}

    // Phase A: Calculate affected blocks and entity damage
    // blockGet: returns blockId at (x,y,z)
    // blockResist: returns blast resistance for a block ID
    ExplosionResult doExplosionA(BlockGetFunc blockGet, BlockResistFunc blockResist) {
        ExplosionResult result;

        // Cast 1352 rays from the 6 faces of a 16x16x16 grid
        for (int i = 0; i < 16; ++i) {
            for (int j = 0; j < 16; ++j) {
                for (int k = 0; k < 16; ++k) {
                    // Only process surface cubes
                    if (i != 0 && i != 15 && j != 0 && j != 15 && k != 0 && k != 15) continue;

                    // Ray direction from center of grid to surface point
                    double dx = static_cast<double>(i) / 15.0 * 2.0 - 1.0;
                    double dy = static_cast<double>(j) / 15.0 * 2.0 - 1.0;
                    double dz = static_cast<double>(k) / 15.0 * 2.0 - 1.0;
                    double len = std::sqrt(dx * dx + dy * dy + dz * dz);
                    dx /= len;
                    dy /= len;
                    dz /= len;

                    // Start with full power, decay along ray
                    float intensity = power * (0.7f + dist_(rng_) * 0.6f);

                    double rx = x;
                    double ry = y;
                    double rz = z;

                    while (intensity > 0.0f) {
                        int bx = static_cast<int>(std::floor(rx));
                        int by = static_cast<int>(std::floor(ry));
                        int bz = static_cast<int>(std::floor(rz));

                        uint16_t blockId = blockGet(bx, by, bz);

                        if (blockId != BlockID::AIR) {
                            float resistance = blockResist(blockId);
                            // Resistance formula: (resistance + 0.3) * 0.3
                            intensity -= (resistance + 0.3f) * 0.3f;
                        }

                        if (intensity > 0.0f && by >= 0 && by < 256) {
                            auto key = packKey(bx, by, bz);
                            if (affectedSet_.insert(key).second) {
                                result.affectedBlocks.emplace_back(bx, by, bz);
                            }
                        }

                        rx += dx * 0.3;
                        ry += dy * 0.3;
                        rz += dz * 0.3;
                        intensity -= 0.22500001f; // Vanilla constant: 0.3 * 0.75
                    }
                }
            }
        }

        return result;
    }

    // Phase B: Destroy blocks (set to air)
    void doExplosionB(const ExplosionResult& result, BlockSetFunc blockSet) {
        if (!destroysBlocks) return;

        for (auto& [bx, by, bz] : result.affectedBlocks) {
            blockSet(bx, by, bz, BlockID::AIR);
        }

        // If causesFire, randomly place fire blocks
        if (causesFire) {
            std::uniform_int_distribution<int> fireDist(0, 2);
            for (auto& [bx, by, bz] : result.affectedBlocks) {
                if (fireDist(rng_) == 0) {
                    // Place fire if the block below is solid
                    // (simplified — vanilla checks getBlock(x,y-1,z).isOpaqueCube())
                    blockSet(bx, by, bz, BlockID::FIRE);
                }
            }
        }
    }

    // Calculate entity damage from explosion
    // Returns (damage, knockback_x, knockback_y, knockback_z)
    struct EntityExplosionEffect {
        float damage;
        double knockX, knockY, knockZ;
    };

    EntityExplosionEffect calculateEntityEffect(double ex, double ey, double ez,
                                                 double entityWidth, double entityHeight) const {
        double dx = ex - x;
        double dy = (ey + entityHeight / 2.0) - y; // Center of entity
        double dz = ez - z;
        double distance = std::sqrt(dx * dx + dy * dy + dz * dz);

        double maxRange = power * 2.0;
        if (distance >= maxRange || distance == 0.0) {
            return {0.0f, 0.0, 0.0, 0.0};
        }

        // Normalize
        dx /= distance;
        dy /= distance;
        dz /= distance;

        // Exposure (simplified: 1.0 means fully exposed)
        double exposure = 1.0;

        // Damage formula from vanilla
        double impact = (1.0 - distance / maxRange) * exposure;
        float damage = static_cast<float>((impact * impact + impact) / 2.0 * 8.0 * maxRange + 1.0);

        // Knockback
        double knockX = dx * impact;
        double knockY = dy * impact;
        double knockZ = dz * impact;

        return {damage, knockX, knockY, knockZ};
    }

    // Get blast resistance for common blocks — vanilla values from Block.java
    static float getBlastResistance(uint16_t blockId) {
        switch (blockId) {
            case BlockID::AIR:            return 0.0f;
            case BlockID::STONE:          return 30.0f;
            case BlockID::GRASS:          return 3.0f;
            case BlockID::DIRT:           return 2.5f;
            case BlockID::COBBLESTONE:    return 30.0f;
            case BlockID::OAK_PLANKS:     return 15.0f;
            case BlockID::BEDROCK:        return 18000000.0f; // Indestructible
            case BlockID::SAND:           return 2.5f;
            case BlockID::GRAVEL:         return 3.0f;
            case BlockID::GOLD_ORE:       return 15.0f;
            case BlockID::IRON_ORE:       return 15.0f;
            case BlockID::COAL_ORE:       return 15.0f;
            case BlockID::OAK_LOG:        return 10.0f;
            case BlockID::OAK_LEAVES:     return 1.0f;
            case BlockID::GLASS:          return 1.5f;
            case BlockID::LAPIS_ORE:      return 15.0f;
            case BlockID::LAPIS_BLOCK:    return 15.0f;
            case BlockID::DISPENSER:      return 17.5f;
            case BlockID::SANDSTONE:      return 4.0f;
            case BlockID::NOTEBLOCK:      return 4.0f;
            case BlockID::OBSIDIAN:       return 6000.0f; // Very high
            case BlockID::DIAMOND_ORE:    return 15.0f;
            case BlockID::DIAMOND_BLOCK:  return 30.0f;
            case BlockID::CRAFTING_TABLE: return 12.5f;
            case BlockID::FURNACE:        return 17.5f;
            case BlockID::IRON_BLOCK:     return 30.0f;
            case BlockID::GOLD_BLOCK:     return 30.0f;
            case BlockID::BRICK_BLOCK:    return 30.0f;
            case BlockID::TNT:            return 0.0f; // TNT has no resistance
            case BlockID::BOOKSHELF:      return 7.5f;
            case BlockID::MOSSY_COBBLE:   return 30.0f;
            case BlockID::CHEST:          return 12.5f;
            case BlockID::REDSTONE_ORE:   return 15.0f;
            case BlockID::EMERALD_ORE:    return 15.0f;
            case BlockID::WATER:          return 500.0f; // Water is blast-resistant
            case BlockID::LAVA:           return 500.0f;
            default:                      return 5.0f; // Default for unknown blocks
        }
    }

private:
    static int64_t packKey(int x, int y, int z) {
        return (static_cast<int64_t>(x) & 0x3FFFFFF) |
               ((static_cast<int64_t>(z) & 0x3FFFFFF) << 26) |
               ((static_cast<int64_t>(y) & 0xFFF) << 52);
    }

    std::unordered_set<int64_t> affectedSet_;
    std::mt19937 rng_;
    std::uniform_real_distribution<float> dist_{0.0f, 1.0f};
};

} // namespace mc
