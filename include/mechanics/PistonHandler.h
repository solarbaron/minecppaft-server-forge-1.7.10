#pragma once
// PistonHandler — piston extension/retraction with block pushing.
// Ported from aov.java (BlockPistonBase) and aow.java (BlockPistonExtension).
//
// Vanilla piston mechanics:
//   - Piston (33) and Sticky Piston (29)
//   - Push up to 12 blocks in a line
//   - Cannot push: obsidian, bedrock, extended pistons, certain tile entities
//   - Sticky pistons pull the block when retracting
//   - Direction stored in metadata bits 0-2 (0=down,1=up,2=N,3=S,4=W,5=E)
//   - Metadata bit 3 = extended
//   - Piston extension block (34): head block when extended
//   - Block action packet (0x24) for push/pull animation
//
// Block IDs:
//   29 = Sticky piston
//   33 = Piston
//   34 = Piston extension (head)
//   36 = Piston moving (tile entity - simplified)

#include <cstdint>
#include <vector>
#include <functional>

namespace mc {

namespace PistonBlockID {
    constexpr uint16_t PISTON        = 33;
    constexpr uint16_t STICKY_PISTON = 29;
    constexpr uint16_t PISTON_HEAD   = 34;
    constexpr uint16_t PISTON_MOVING = 36;
}

// Blocks that cannot be pushed by pistons
inline bool isImmovable(uint16_t blockId) {
    switch (blockId) {
        case 7:   // Bedrock
        case 49:  // Obsidian
        case 90:  // Nether portal
        case 119: // End portal
        case 120: // End portal frame
        case 130: // Ender chest
        case 137: // Command block
        case 138: // Beacon
        case PistonBlockID::PISTON_HEAD:
        case PistonBlockID::PISTON_MOVING:
            return true;
        default:
            return false;
    }
}

// Blocks that are destroyed when pushed (not moved)
inline bool isBreakableByPiston(uint16_t blockId) {
    switch (blockId) {
        case 30:  // Cobweb
        case 31:  // Tall grass
        case 32:  // Dead bush
        case 37:  // Dandelion
        case 38:  // Poppy
        case 39:  // Brown mushroom
        case 40:  // Red mushroom
        case 50:  // Torch
        case 51:  // Fire
        case 55:  // Redstone wire
        case 59:  // Wheat crops
        case 63:  // Sign (standing)
        case 64:  // Wooden door
        case 65:  // Ladder
        case 66:  // Rail
        case 68:  // Sign (wall)
        case 69:  // Lever
        case 70:  // Stone pressure plate
        case 71:  // Iron door
        case 72:  // Wooden pressure plate
        case 75:  // Redstone torch (off)
        case 76:  // Redstone torch (on)
        case 77:  // Stone button
        case 78:  // Snow layer
        case 83:  // Sugar cane
        case 93:  // Repeater (off)
        case 94:  // Repeater (on)
        case 104: // Pumpkin stem
        case 105: // Melon stem
        case 106: // Vines
        case 115: // Nether wart
        case 131: // Tripwire hook
        case 132: // Tripwire
        case 141: // Carrots
        case 142: // Potatoes
        case 143: // Wooden button
        case 171: // Carpet
            return true;
        default:
            return false;
    }
}

// Direction offsets for piston facing
struct PistonDir {
    int dx, dy, dz;
};

inline PistonDir getPistonDirection(uint8_t facing) {
    switch (facing & 0x7) {
        case 0: return {0, -1, 0}; // Down
        case 1: return {0,  1, 0}; // Up
        case 2: return {0, 0, -1}; // North
        case 3: return {0, 0,  1}; // South
        case 4: return {-1, 0, 0}; // West
        case 5: return { 1, 0, 0}; // East
        default: return {0, 0, 0};
    }
}

// Block change from piston action
struct PistonBlockChange {
    int x, y, z;
    uint16_t blockId;
    uint8_t meta;
};

class PistonHandler {
public:
    static constexpr int MAX_PUSH = 12; // Vanilla push limit

    // Attempt to extend a piston at (x,y,z)
    // Returns list of block changes, empty if extension fails
    static std::vector<PistonBlockChange> tryExtend(
            int x, int y, int z, uint8_t meta,
            std::function<uint16_t(int,int,int)> getBlock,
            std::function<uint8_t(int,int,int)> getMeta,
            std::function<void(int,int,int,uint16_t,uint8_t)> setBlock) {

        std::vector<PistonBlockChange> changes;
        uint8_t facing = meta & 0x7;
        auto dir = getPistonDirection(facing);

        // Check if already extended
        if (meta & 0x8) return changes;

        // Collect blocks in push line
        std::vector<std::tuple<int,int,int,uint16_t,uint8_t>> pushLine;
        int cx = x + dir.dx, cy = y + dir.dy, cz = z + dir.dz;

        for (int i = 0; i < MAX_PUSH + 1; ++i) {
            uint16_t bid = getBlock(cx, cy, cz);

            if (bid == 0) break; // Air — end of line

            if (isImmovable(bid)) return {}; // Can't push

            if (isBreakableByPiston(bid)) {
                // Break this block (drop items in vanilla, we just remove)
                pushLine.emplace_back(cx, cy, cz, bid, getMeta(cx, cy, cz));
                break;
            }

            pushLine.emplace_back(cx, cy, cz, bid, getMeta(cx, cy, cz));

            if (static_cast<int>(pushLine.size()) > MAX_PUSH) return {}; // Too many

            cx += dir.dx;
            cy += dir.dy;
            cz += dir.dz;
        }

        // Move blocks from far end to near
        for (int i = static_cast<int>(pushLine.size()) - 1; i >= 0; --i) {
            auto& [bx, by, bz, bid, bm] = pushLine[i];
            int nx = bx + dir.dx, ny = by + dir.dy, nz = bz + dir.dz;

            if (isBreakableByPiston(bid)) {
                // Destroy the block
                setBlock(bx, by, bz, 0, 0);
                changes.push_back({bx, by, bz, 0, 0});
            } else {
                // Move to next position
                setBlock(nx, ny, nz, bid, bm);
                changes.push_back({nx, ny, nz, bid, bm});
                setBlock(bx, by, bz, 0, 0);
                changes.push_back({bx, by, bz, 0, 0});
            }
        }

        // Place piston extension head
        int hx = x + dir.dx, hy = y + dir.dy, hz = z + dir.dz;
        uint8_t headMeta = facing; // Head meta = facing direction
        setBlock(hx, hy, hz, PistonBlockID::PISTON_HEAD, headMeta);
        changes.push_back({hx, hy, hz, PistonBlockID::PISTON_HEAD, headMeta});

        // Mark piston as extended
        uint16_t pistonId = getBlock(x, y, z);
        setBlock(x, y, z, pistonId, meta | 0x8);
        changes.push_back({x, y, z, pistonId, static_cast<uint8_t>(meta | 0x8)});

        return changes;
    }

    // Attempt to retract a piston at (x,y,z)
    static std::vector<PistonBlockChange> tryRetract(
            int x, int y, int z, uint8_t meta, bool isSticky,
            std::function<uint16_t(int,int,int)> getBlock,
            std::function<uint8_t(int,int,int)> getMeta,
            std::function<void(int,int,int,uint16_t,uint8_t)> setBlock) {

        std::vector<PistonBlockChange> changes;
        uint8_t facing = meta & 0x7;
        auto dir = getPistonDirection(facing);

        // Check if extended
        if (!(meta & 0x8)) return changes;

        // Remove piston head
        int hx = x + dir.dx, hy = y + dir.dy, hz = z + dir.dz;
        uint16_t headBlock = getBlock(hx, hy, hz);
        if (headBlock == PistonBlockID::PISTON_HEAD) {
            setBlock(hx, hy, hz, 0, 0);
            changes.push_back({hx, hy, hz, 0, 0});
        }

        // Sticky piston: pull block behind head
        if (isSticky) {
            int px = hx + dir.dx, py = hy + dir.dy, pz = hz + dir.dz;
            uint16_t pullBlock = getBlock(px, py, pz);

            if (pullBlock != 0 && !isImmovable(pullBlock) &&
                !isBreakableByPiston(pullBlock)) {
                uint8_t pullMeta = getMeta(px, py, pz);
                setBlock(hx, hy, hz, pullBlock, pullMeta);
                changes.push_back({hx, hy, hz, pullBlock, pullMeta});
                setBlock(px, py, pz, 0, 0);
                changes.push_back({px, py, pz, 0, 0});
            }
        }

        // Mark piston as retracted
        uint16_t pistonId = getBlock(x, y, z);
        setBlock(x, y, z, pistonId, meta & 0x7); // Clear extended bit
        changes.push_back({x, y, z, pistonId, static_cast<uint8_t>(meta & 0x7)});

        return changes;
    }

    // Check if a piston should be powered (has adjacent redstone power)
    static bool isPowered(int x, int y, int z,
                          std::function<uint16_t(int,int,int)> getBlock,
                          std::function<uint8_t(int,int,int)> getMeta) {
        // Check all 6 neighbors for redstone power sources
        static constexpr int dx[] = {1,-1,0,0,0,0};
        static constexpr int dy[] = {0,0,1,-1,0,0};
        static constexpr int dz[] = {0,0,0,0,1,-1};

        for (int i = 0; i < 6; ++i) {
            int nx = x + dx[i], ny = y + dy[i], nz = z + dz[i];
            uint16_t bid = getBlock(nx, ny, nz);

            // Direct power sources
            if (bid == 55 && getMeta(nx, ny, nz) > 0) return true; // Dust
            if (bid == 76) return true;  // Redstone torch on
            if (bid == 94) return true;  // Repeater on
            if (bid == 152) return true; // Redstone block
            if (bid == 69 && (getMeta(nx, ny, nz) & 0x8)) return true; // Lever
            if ((bid == 77 || bid == 143) && (getMeta(nx, ny, nz) & 0x8)) return true; // Button
        }
        return false;
    }
};

} // namespace mc
