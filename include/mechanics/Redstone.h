#pragma once
// RedstoneEngine — redstone signal propagation, component logic.
// Ported from vanilla 1.7.10 BlockRedstoneWire (ajf.java),
// BlockRedstoneTorch (ajg.java), BlockRedstoneRepeater (ajd.java),
// BlockRedstoneComparator (ajb.java), BlockPistonBase (aji.java).
//
// Redstone mechanics:
//   - Wire: power 0-15, decrements by 1 per block traveled
//   - Torch: strong power source (15), inverted from input
//   - Repeater: delay 1-4 ticks, direction-specific, lockable
//   - Comparator: compare/subtract modes
//   - Piston: push/pull up to 12 blocks

#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include "world/Block.h"

namespace mc {

// ============================================================
// Redstone direction — 6 faces
// ============================================================
enum class RedstoneFace : uint8_t {
    DOWN = 0, UP = 1,
    NORTH = 2, SOUTH = 3,
    WEST = 4, EAST = 5
};

// ============================================================
// Redstone component types
// ============================================================
enum class RedstoneComponent : uint8_t {
    NONE,
    WIRE,           // Block 55
    TORCH,          // Block 75 (off), 76 (on)
    REPEATER,       // Block 93 (off), 94 (on)
    COMPARATOR,     // Block 149 (off), 150 (on)
    LEVER,          // Block 69
    BUTTON,         // Block 77 (stone), 143 (wood)
    PRESSURE_PLATE, // Block 70 (stone), 72 (wood)
    PISTON,         // Block 33, 29 (sticky)
    LAMP,           // Block 123 (off), 124 (on)
    TRAPDOOR,       // Block 96
    DOOR,           // Block 64 (iron), 71 (wood)
    TNT,            // Block 46
    DROPPER,        // Block 158
    DISPENSER,      // Block 23
    NOTEBLOCK,      // Block 25
    HOPPER          // Block 154
};

// Identify redstone component from block ID
inline RedstoneComponent getRedstoneComponent(uint16_t blockId) {
    switch (blockId) {
        case 55:  return RedstoneComponent::WIRE;
        case 75: case 76: return RedstoneComponent::TORCH;
        case 93: case 94: return RedstoneComponent::REPEATER;
        case 149: case 150: return RedstoneComponent::COMPARATOR;
        case 69:  return RedstoneComponent::LEVER;
        case 77: case 143: return RedstoneComponent::BUTTON;
        case 70: case 72: return RedstoneComponent::PRESSURE_PLATE;
        case 33: case 29: return RedstoneComponent::PISTON;
        case 123: case 124: return RedstoneComponent::LAMP;
        case 96:  return RedstoneComponent::TRAPDOOR;
        case 64: case 71: return RedstoneComponent::DOOR;
        case 46:  return RedstoneComponent::TNT;
        case 158: return RedstoneComponent::DROPPER;
        case 23:  return RedstoneComponent::DISPENSER;
        case 25:  return RedstoneComponent::NOTEBLOCK;
        case 154: return RedstoneComponent::HOPPER;
        default:  return RedstoneComponent::NONE;
    }
}

// ============================================================
// Redstone signal state at a block position
// ============================================================
struct RedstoneState {
    int x, y, z;
    uint8_t powerLevel = 0;  // 0-15
    bool    stronglyPowered = false; // Direct from source
};

// ============================================================
// Repeater/Comparator state
// ============================================================
struct RepeaterState {
    int x, y, z;
    uint8_t delay;       // 1-4 ticks
    uint8_t direction;   // 0-3 (NESW)
    bool    powered;
    bool    locked;      // Locked by adjacent repeater
    int     tickCount;   // Countdown to toggle
};

// ============================================================
// RedstoneEngine — signal propagation
// ============================================================
class RedstoneEngine {
public:
    using BlockGetFunc = std::function<uint16_t(int, int, int)>;
    using BlockMetaFunc = std::function<uint8_t(int, int, int)>;
    using BlockSetFunc = std::function<void(int, int, int, uint16_t, uint8_t)>;

    // Calculate power level at a position
    // Vanilla algorithm: check all 6 neighbors + wire connections
    uint8_t calculatePowerAt(int x, int y, int z,
                              BlockGetFunc getBlock, BlockMetaFunc getMeta) {
        uint8_t maxPower = 0;

        // Check direct strong power sources (torch, repeater output)
        static const int dx[] = {0, 0, -1, 1, 0, 0};
        static const int dy[] = {-1, 1, 0, 0, 0, 0};
        static const int dz[] = {0, 0, 0, 0, -1, 1};

        for (int i = 0; i < 6; ++i) {
            int nx = x + dx[i], ny = y + dy[i], nz = z + dz[i];
            uint16_t nBlock = getBlock(nx, ny, nz);
            auto comp = getRedstoneComponent(nBlock);

            switch (comp) {
                case RedstoneComponent::TORCH:
                    if (nBlock == 76) { // On torch
                        maxPower = 15;
                    }
                    break;

                case RedstoneComponent::REPEATER:
                    if (nBlock == 94) { // On repeater
                        uint8_t meta = getMeta(nx, ny, nz);
                        uint8_t dir = meta & 3;
                        // Check if output faces toward (x,y,z)
                        if (isOutputFacing(dir, nx - x, nz - z)) {
                            maxPower = 15;
                        }
                    }
                    break;

                case RedstoneComponent::LEVER:
                    if ((getMeta(nx, ny, nz) & 8) != 0) { // On
                        maxPower = 15;
                    }
                    break;

                case RedstoneComponent::BUTTON:
                    if ((getMeta(nx, ny, nz) & 8) != 0) { // Pressed
                        maxPower = 15;
                    }
                    break;

                case RedstoneComponent::PRESSURE_PLATE:
                    if (getMeta(nx, ny, nz) > 0) { // Active
                        maxPower = 15;
                    }
                    break;

                case RedstoneComponent::WIRE: {
                    uint8_t wirePower = getMeta(nx, ny, nz);
                    if (wirePower > 0) {
                        maxPower = std::max(maxPower, static_cast<uint8_t>(wirePower - 1));
                    }
                    break;
                }

                default: break;
            }
        }

        return maxPower;
    }

    // Propagate wire power changes from a source
    // BFS-based (vanilla uses recursive updates, we use iterative)
    struct BlockUpdate {
        int x, y, z;
        uint8_t newPower;
    };

    std::vector<BlockUpdate> propagateWire(int sourceX, int sourceY, int sourceZ,
                                            uint8_t sourcePower,
                                            BlockGetFunc getBlock, BlockMetaFunc getMeta) {
        std::vector<BlockUpdate> updates;
        std::queue<std::tuple<int, int, int, uint8_t>> queue;
        std::unordered_set<int64_t> visited;

        queue.push({sourceX, sourceY, sourceZ, sourcePower});

        while (!queue.empty()) {
            auto [x, y, z, power] = queue.front();
            queue.pop();

            auto key = packKey(x, y, z);
            if (visited.count(key)) continue;
            visited.insert(key);

            uint16_t block = getBlock(x, y, z);
            if (getRedstoneComponent(block) != RedstoneComponent::WIRE &&
                !(x == sourceX && y == sourceY && z == sourceZ)) continue;

            uint8_t currentPower = getMeta(x, y, z);
            if (power != currentPower) {
                updates.push_back({x, y, z, power});
            }

            if (power <= 0) continue;

            // Propagate to adjacent wires (power - 1)
            uint8_t nextPower = power > 0 ? power - 1 : 0;

            // Horizontal neighbors
            static const int hdx[] = {-1, 1, 0, 0};
            static const int hdz[] = {0, 0, -1, 1};

            for (int i = 0; i < 4; ++i) {
                int nx = x + hdx[i], nz = z + hdz[i];

                // Same level
                if (getRedstoneComponent(getBlock(nx, y, nz)) == RedstoneComponent::WIRE) {
                    queue.push({nx, y, nz, nextPower});
                }
                // Up slope (wire on block above)
                if (getRedstoneComponent(getBlock(nx, y + 1, nz)) == RedstoneComponent::WIRE &&
                    getBlock(x, y + 1, z) == BlockID::AIR) {
                    queue.push({nx, y + 1, nz, nextPower});
                }
                // Down slope (wire below, with air above it)
                if (getRedstoneComponent(getBlock(nx, y - 1, nz)) == RedstoneComponent::WIRE &&
                    getBlock(nx, y, nz) == BlockID::AIR) {
                    queue.push({nx, y - 1, nz, nextPower});
                }
            }
        }

        return updates;
    }

    // Tick repeaters/comparators — returns list of state changes
    std::vector<BlockUpdate> tickScheduled() {
        std::vector<BlockUpdate> updates;
        for (auto it = repeaters_.begin(); it != repeaters_.end(); ) {
            auto& rep = it->second;
            if (rep.tickCount > 0) {
                rep.tickCount--;
                if (rep.tickCount <= 0) {
                    rep.powered = !rep.powered;
                    updates.push_back({rep.x, rep.y, rep.z,
                                        static_cast<uint8_t>(rep.powered ? 15 : 0)});
                }
                ++it;
            } else {
                ++it;
            }
        }
        return updates;
    }

    // Schedule a repeater toggle
    void scheduleRepeater(int x, int y, int z, uint8_t delay, uint8_t direction, bool currentState) {
        auto key = packKey(x, y, z);
        repeaters_[key] = {x, y, z, delay, direction, currentState, false, delay * 2};
    }

    // Check if a block position is powered
    bool isPowered(int x, int y, int z, BlockGetFunc getBlock, BlockMetaFunc getMeta) {
        return calculatePowerAt(x, y, z, getBlock, getMeta) > 0;
    }

    // Piston push check — can push up to 12 blocks
    bool canPistonPush(int x, int y, int z, int dx, int dy, int dz,
                        BlockGetFunc getBlock) {
        for (int i = 1; i <= 13; ++i) {
            int nx = x + dx * i, ny = y + dy * i, nz = z + dz * i;
            uint16_t block = getBlock(nx, ny, nz);

            if (block == BlockID::AIR) return true; // Space to push
            if (block == BlockID::BEDROCK || block == BlockID::OBSIDIAN) return false;
            if (i > 12) return false; // Max push distance
        }
        return false;
    }

    // Get blocks that a piston will push
    std::vector<std::tuple<int, int, int>> getPistonPushList(
        int x, int y, int z, int dx, int dy, int dz,
        BlockGetFunc getBlock) {
        std::vector<std::tuple<int, int, int>> blocks;
        for (int i = 1; i <= 12; ++i) {
            int nx = x + dx * i, ny = y + dy * i, nz = z + dz * i;
            uint16_t block = getBlock(nx, ny, nz);
            if (block == BlockID::AIR) break;
            if (block == BlockID::BEDROCK || block == BlockID::OBSIDIAN) {
                blocks.clear();
                return blocks; // Can't push
            }
            blocks.push_back({nx, ny, nz});
        }
        return blocks;
    }

private:
    static bool isOutputFacing(uint8_t direction, int dx, int dz) {
        switch (direction) {
            case 0: return dz == -1; // North
            case 1: return dx == 1;  // East
            case 2: return dz == 1;  // South
            case 3: return dx == -1; // West
        }
        return false;
    }

    static int64_t packKey(int x, int y, int z) {
        return (static_cast<int64_t>(x) & 0x3FFFFFF) |
               ((static_cast<int64_t>(z) & 0x3FFFFFF) << 26) |
               ((static_cast<int64_t>(y) & 0xFFF) << 52);
    }

    std::unordered_map<int64_t, RepeaterState> repeaters_;
};

} // namespace mc
