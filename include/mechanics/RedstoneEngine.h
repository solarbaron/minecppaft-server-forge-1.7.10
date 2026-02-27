#pragma once
// RedstoneEngine — redstone power propagation and components.
// Simplified port of vanilla redstone (alz.java / BlockRedstoneWire).
//
// Vanilla redstone model:
//   - Power level 0-15 (metadata for redstone dust)
//   - Power sources: lever (on=15), button (on=15), redstone torch (15),
//     pressure plate, daylight sensor, comparator
//   - Redstone dust: propagates power with -1 per block distance
//   - Redstone torch: inverts — outputs 15 when block below is unpowered
//   - Repeater: re-powers to 15 with configurable delay (1-4 ticks)
//
// Block IDs:
//   55  = Redstone wire (dust)
//   75  = Redstone torch (off)
//   76  = Redstone torch (on)
//   69  = Lever
//   77  = Stone button
//   143 = Wooden button
//   70  = Stone pressure plate
//   72  = Wooden pressure plate
//   93  = Repeater (off)
//   94  = Repeater (on)
//   73  = Redstone ore
//   74  = Lit redstone ore
//   152 = Redstone block (always power 15)

#include <cstdint>
#include <vector>
#include <queue>
#include <unordered_set>
#include <tuple>
#include <functional>

#include "world/Chunk.h"

namespace mc {

// Forward declare
class World;

// Block IDs for redstone components
namespace RedstoneBlockID {
    constexpr uint16_t WIRE          = 55;
    constexpr uint16_t TORCH_OFF     = 75;
    constexpr uint16_t TORCH_ON      = 76;
    constexpr uint16_t LEVER         = 69;
    constexpr uint16_t STONE_BUTTON  = 77;
    constexpr uint16_t WOOD_BUTTON   = 143;
    constexpr uint16_t STONE_PLATE   = 70;
    constexpr uint16_t WOOD_PLATE    = 72;
    constexpr uint16_t REPEATER_OFF  = 93;
    constexpr uint16_t REPEATER_ON   = 94;
    constexpr uint16_t REDSTONE_BLOCK = 152;
    constexpr uint16_t LAMP_OFF      = 123;
    constexpr uint16_t LAMP_ON       = 124;
}

// Redstone update entry
struct RedstoneUpdate {
    int x, y, z;
    int delay;  // Ticks until activation (for repeaters)
};

// Direction offsets for 6-directional adjacency
struct Dir {
    int dx, dy, dz;
};

static constexpr Dir DIRS[6] = {
    {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}
};

// Result of a redstone update — block changes to broadcast
struct RedstoneChange {
    int x, y, z;
    uint16_t blockId;
    uint8_t meta;
};

class RedstoneEngine {
public:
    // Process a block change that might affect redstone
    // Called when a block is placed/broken/activated
    // Returns list of resulting block changes
    std::vector<RedstoneChange> onBlockUpdate(
            int x, int y, int z, uint16_t blockId, uint8_t meta,
            std::function<uint16_t(int,int,int)> getBlock,
            std::function<uint8_t(int,int,int)> getMeta,
            std::function<void(int,int,int,uint16_t,uint8_t)> setBlock) {

        std::vector<RedstoneChange> changes;

        switch (blockId) {
            case RedstoneBlockID::LEVER:
                handleLever(x, y, z, meta, getBlock, getMeta, setBlock, changes);
                break;
            case RedstoneBlockID::STONE_BUTTON:
            case RedstoneBlockID::WOOD_BUTTON:
                handleButton(x, y, z, meta, getBlock, getMeta, setBlock, changes);
                break;
            case RedstoneBlockID::WIRE:
                propagateDust(x, y, z, getBlock, getMeta, setBlock, changes);
                break;
            case RedstoneBlockID::TORCH_ON:
            case RedstoneBlockID::TORCH_OFF:
                handleTorch(x, y, z, blockId, getBlock, getMeta, setBlock, changes);
                break;
            case RedstoneBlockID::REPEATER_OFF:
            case RedstoneBlockID::REPEATER_ON:
                handleRepeater(x, y, z, blockId, meta, getBlock, getMeta, setBlock, changes);
                break;
            case RedstoneBlockID::REDSTONE_BLOCK:
                // Always emits power 15 — update adjacent dust
                updateAdjacentDust(x, y, z, getBlock, getMeta, setBlock, changes);
                break;
        }

        return changes;
    }

    // Tick scheduled redstone updates (repeaters, button deactivation)
    std::vector<RedstoneChange> tickScheduled(
            std::function<uint16_t(int,int,int)> getBlock,
            std::function<uint8_t(int,int,int)> getMeta,
            std::function<void(int,int,int,uint16_t,uint8_t)> setBlock) {

        std::vector<RedstoneChange> changes;
        std::vector<RedstoneUpdate> remaining;

        for (auto& update : scheduledUpdates_) {
            if (--update.delay <= 0) {
                // Process the delayed update
                uint16_t bid = getBlock(update.x, update.y, update.z);
                uint8_t bm = getMeta(update.x, update.y, update.z);

                if (bid == RedstoneBlockID::REPEATER_OFF) {
                    // Turn on
                    setBlock(update.x, update.y, update.z,
                             RedstoneBlockID::REPEATER_ON, bm);
                    changes.push_back({update.x, update.y, update.z,
                                       RedstoneBlockID::REPEATER_ON, bm});
                    updateAdjacentDust(update.x, update.y, update.z,
                                      getBlock, getMeta, setBlock, changes);
                } else if (bid == RedstoneBlockID::REPEATER_ON) {
                    // Turn off
                    setBlock(update.x, update.y, update.z,
                             RedstoneBlockID::REPEATER_OFF, bm);
                    changes.push_back({update.x, update.y, update.z,
                                       RedstoneBlockID::REPEATER_OFF, bm});
                    updateAdjacentDust(update.x, update.y, update.z,
                                      getBlock, getMeta, setBlock, changes);
                } else if (bid == RedstoneBlockID::STONE_BUTTON ||
                           bid == RedstoneBlockID::WOOD_BUTTON) {
                    // Button deactivation
                    uint8_t newMeta = bm & 0x7; // Clear activated bit
                    setBlock(update.x, update.y, update.z, bid, newMeta);
                    changes.push_back({update.x, update.y, update.z, bid, newMeta});
                    updateAdjacentDust(update.x, update.y, update.z,
                                      getBlock, getMeta, setBlock, changes);
                }
            } else {
                remaining.push_back(update);
            }
        }

        scheduledUpdates_ = std::move(remaining);
        return changes;
    }

private:
    std::vector<RedstoneUpdate> scheduledUpdates_;

    // Handle lever toggle
    void handleLever(int x, int y, int z, uint8_t meta,
                     std::function<uint16_t(int,int,int)> getBlock,
                     std::function<uint8_t(int,int,int)> getMeta,
                     std::function<void(int,int,int,uint16_t,uint8_t)> setBlock,
                     std::vector<RedstoneChange>& changes) {
        // Toggle lever state (bit 3 = powered)
        uint8_t newMeta = meta ^ 0x8;
        setBlock(x, y, z, RedstoneBlockID::LEVER, newMeta);
        changes.push_back({x, y, z, RedstoneBlockID::LEVER, newMeta});

        // Update adjacent redstone
        updateAdjacentDust(x, y, z, getBlock, getMeta, setBlock, changes);
    }

    // Handle button press
    void handleButton(int x, int y, int z, uint8_t meta,
                      std::function<uint16_t(int,int,int)> getBlock,
                      std::function<uint8_t(int,int,int)> getMeta,
                      std::function<void(int,int,int,uint16_t,uint8_t)> setBlock,
                      std::vector<RedstoneChange>& changes) {
        uint16_t bid = getBlock(x, y, z);

        // Activate (set bit 3)
        uint8_t newMeta = meta | 0x8;
        setBlock(x, y, z, bid, newMeta);
        changes.push_back({x, y, z, bid, newMeta});

        // Schedule deactivation
        int delay = (bid == RedstoneBlockID::WOOD_BUTTON) ? 30 : 20; // ticks
        scheduledUpdates_.push_back({x, y, z, delay});

        updateAdjacentDust(x, y, z, getBlock, getMeta, setBlock, changes);
    }

    // Handle redstone torch
    void handleTorch(int x, int y, int z, uint16_t blockId,
                     std::function<uint16_t(int,int,int)> getBlock,
                     std::function<uint8_t(int,int,int)> getMeta,
                     std::function<void(int,int,int,uint16_t,uint8_t)> setBlock,
                     std::vector<RedstoneChange>& changes) {
        // Check if block below is powered
        bool belowPowered = isPowered(x, y - 1, z, getBlock, getMeta);

        uint16_t newId = belowPowered ?
            RedstoneBlockID::TORCH_OFF : RedstoneBlockID::TORCH_ON;

        if (newId != blockId) {
            uint8_t meta = getMeta(x, y, z);
            setBlock(x, y, z, newId, meta);
            changes.push_back({x, y, z, newId, meta});
            updateAdjacentDust(x, y, z, getBlock, getMeta, setBlock, changes);
        }
    }

    // Handle repeater
    void handleRepeater(int x, int y, int z, uint16_t blockId, uint8_t meta,
                        std::function<uint16_t(int,int,int)> getBlock,
                        std::function<uint8_t(int,int,int)> getMeta,
                        std::function<void(int,int,int,uint16_t,uint8_t)> setBlock,
                        std::vector<RedstoneChange>& changes) {
        // Repeater delay: bits 2-3 of meta = delay (0-3 = 1-4 ticks)
        int delay = ((meta >> 2) & 0x3) + 1;
        // Direction: bits 0-1 of meta

        // Check input side
        bool inputPowered = isRepeaterInputPowered(x, y, z, meta, getBlock, getMeta);

        if (inputPowered && blockId == RedstoneBlockID::REPEATER_OFF) {
            // Schedule turning on
            scheduledUpdates_.push_back({x, y, z, delay * 2}); // *2 for game ticks
        } else if (!inputPowered && blockId == RedstoneBlockID::REPEATER_ON) {
            // Schedule turning off
            scheduledUpdates_.push_back({x, y, z, delay * 2});
        }
    }

    // Propagate redstone dust power levels from a position
    void propagateDust(int x, int y, int z,
                       std::function<uint16_t(int,int,int)> getBlock,
                       std::function<uint8_t(int,int,int)> getMeta,
                       std::function<void(int,int,int,uint16_t,uint8_t)> setBlock,
                       std::vector<RedstoneChange>& changes) {
        // Calculate new power level for this wire
        int power = calculateWirePower(x, y, z, getBlock, getMeta);

        uint8_t currentMeta = getMeta(x, y, z);
        if (power == currentMeta) return; // No change

        // Update this wire
        setBlock(x, y, z, RedstoneBlockID::WIRE, static_cast<uint8_t>(power));
        changes.push_back({x, y, z, RedstoneBlockID::WIRE,
                           static_cast<uint8_t>(power)});

        // Propagate to adjacent wires
        for (auto& d : DIRS) {
            int nx = x + d.dx, ny = y + d.dy, nz = z + d.dz;
            if (getBlock(nx, ny, nz) == RedstoneBlockID::WIRE) {
                propagateDust(nx, ny, nz, getBlock, getMeta, setBlock, changes);
            }
        }

        // Update redstone powered devices (lamps, pistons, etc.)
        updateAdjacentDevices(x, y, z, power, getBlock, getMeta, setBlock, changes);
    }

    // Calculate power level for a wire at position
    int calculateWirePower(int x, int y, int z,
                           std::function<uint16_t(int,int,int)> getBlock,
                           std::function<uint8_t(int,int,int)> getMeta) {
        int maxPower = 0;

        // Check all 4 horizontal neighbors + up/down
        for (auto& d : DIRS) {
            int nx = x + d.dx, ny = y + d.dy, nz = z + d.dz;
            uint16_t bid = getBlock(nx, ny, nz);

            if (bid == RedstoneBlockID::WIRE) {
                int neighborPower = getMeta(nx, ny, nz);
                if (neighborPower - 1 > maxPower) {
                    maxPower = neighborPower - 1;
                }
            } else if (bid == RedstoneBlockID::TORCH_ON) {
                maxPower = 15;
            } else if (bid == RedstoneBlockID::REPEATER_ON) {
                // Check if repeater is facing toward this wire
                maxPower = 15;
            } else if (bid == RedstoneBlockID::REDSTONE_BLOCK) {
                maxPower = 15;
            } else if (bid == RedstoneBlockID::LEVER) {
                if (getMeta(nx, ny, nz) & 0x8) maxPower = 15;
            } else if (bid == RedstoneBlockID::STONE_BUTTON ||
                       bid == RedstoneBlockID::WOOD_BUTTON) {
                if (getMeta(nx, ny, nz) & 0x8) maxPower = 15;
            }
        }

        return maxPower;
    }

    // Check if a position is powered (has redstone power)
    bool isPowered(int x, int y, int z,
                   std::function<uint16_t(int,int,int)> getBlock,
                   std::function<uint8_t(int,int,int)> getMeta) {
        // Check block itself
        uint16_t bid = getBlock(x, y, z);
        if (bid == RedstoneBlockID::WIRE && getMeta(x, y, z) > 0) return true;
        if (bid == RedstoneBlockID::REDSTONE_BLOCK) return true;
        if (bid == RedstoneBlockID::TORCH_ON) return true;
        if (bid == RedstoneBlockID::REPEATER_ON) return true;
        if (bid == RedstoneBlockID::LEVER && (getMeta(x, y, z) & 0x8)) return true;
        if ((bid == RedstoneBlockID::STONE_BUTTON || bid == RedstoneBlockID::WOOD_BUTTON)
            && (getMeta(x, y, z) & 0x8)) return true;

        // Check adjacent wires
        for (auto& d : DIRS) {
            int nx = x + d.dx, ny = y + d.dy, nz = z + d.dz;
            uint16_t nbid = getBlock(nx, ny, nz);
            if (nbid == RedstoneBlockID::WIRE && getMeta(nx, ny, nz) > 0) return true;
        }

        return false;
    }

    // Check repeater input
    bool isRepeaterInputPowered(int x, int y, int z, uint8_t meta,
                                std::function<uint16_t(int,int,int)> getBlock,
                                std::function<uint8_t(int,int,int)> getMeta) {
        // Direction from meta bits 0-1: 0=S, 1=W, 2=N, 3=E
        int dir = meta & 0x3;
        int dx = 0, dz = 0;
        switch (dir) {
            case 0: dz = -1; break; // Input from North
            case 1: dx = 1;  break; // Input from East
            case 2: dz = 1;  break; // Input from South
            case 3: dx = -1; break; // Input from West
        }
        return isPowered(x + dx, y, z + dz, getBlock, getMeta);
    }

    // Update redstone dust around a position
    void updateAdjacentDust(int x, int y, int z,
                            std::function<uint16_t(int,int,int)> getBlock,
                            std::function<uint8_t(int,int,int)> getMeta,
                            std::function<void(int,int,int,uint16_t,uint8_t)> setBlock,
                            std::vector<RedstoneChange>& changes) {
        for (auto& d : DIRS) {
            int nx = x + d.dx, ny = y + d.dy, nz = z + d.dz;
            if (getBlock(nx, ny, nz) == RedstoneBlockID::WIRE) {
                propagateDust(nx, ny, nz, getBlock, getMeta, setBlock, changes);
            }
        }
    }

    // Update powered devices (redstone lamp, etc.)
    void updateAdjacentDevices(int x, int y, int z, int power,
                               std::function<uint16_t(int,int,int)> getBlock,
                               std::function<uint8_t(int,int,int)> getMeta,
                               std::function<void(int,int,int,uint16_t,uint8_t)> setBlock,
                               std::vector<RedstoneChange>& changes) {
        for (auto& d : DIRS) {
            int nx = x + d.dx, ny = y + d.dy, nz = z + d.dz;
            uint16_t bid = getBlock(nx, ny, nz);

            // Redstone lamp
            if (bid == RedstoneBlockID::LAMP_OFF && power > 0) {
                setBlock(nx, ny, nz, RedstoneBlockID::LAMP_ON, 0);
                changes.push_back({nx, ny, nz, RedstoneBlockID::LAMP_ON, 0});
            } else if (bid == RedstoneBlockID::LAMP_ON && power == 0) {
                // Check if no other power source
                if (!isPowered(nx, ny, nz, getBlock, getMeta)) {
                    setBlock(nx, ny, nz, RedstoneBlockID::LAMP_OFF, 0);
                    changes.push_back({nx, ny, nz, RedstoneBlockID::LAMP_OFF, 0});
                }
            }

            // Redstone torch
            if (bid == RedstoneBlockID::TORCH_ON || bid == RedstoneBlockID::TORCH_OFF) {
                handleTorch(nx, ny, nz, bid, getBlock, getMeta, setBlock, changes);
            }
        }
    }
};

} // namespace mc
