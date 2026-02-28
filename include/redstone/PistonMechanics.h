/**
 * PistonMechanics.h — Piston push/pull logic.
 *
 * Java references:
 *   - net.minecraft.block.BlockPistonBase — Core piston behavior
 *   - net.minecraft.util.Facing — Direction offset tables
 *
 * Key mechanics:
 *   1. Orientation: determined from player look direction
 *      - Within ±2 XZ blocks: check eye height vs block Y
 *      - Otherwise: yaw → face direction (0→south, 1→west, 2→north, 3→east)
 *   2. Power check: 12 redstone input positions around piston
 *      - 6 faces (skip own face) + 6 above-offset positions
 *   3. Push check (canPushBlock):
 *      - Obsidian: never pushable
 *      - Extended pistons: never pushable
 *      - Hardness == -1: never (bedrock, etc.)
 *      - Mobility 2: never (leaves, etc. when pushed)
 *      - Mobility 1: only destroyable (flowers, grass, etc.)
 *      - Tile entities: never pushable
 *   4. Extension check: max 12 blocks push limit
 *   5. Push chain: walk backward from end, place piston_extension
 *      tiles, notify neighbors
 *   6. Retraction: sticky pulls 1 block, normal clears head
 *
 * Thread safety: Called from main server thread.
 * JNI readiness: Simple data, predictable layout.
 */
#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Facing — Direction offset tables.
// Java reference: net.minecraft.util.Facing
// ═══════════════════════════════════════════════════════════════════════════

namespace Facing {
    // Direction indices: 0=down, 1=up, 2=north(-Z), 3=south(+Z), 4=west(-X), 5=east(+X)
    constexpr int32_t offsetsXForSide[6] = {0, 0, 0, 0, -1, 1};
    constexpr int32_t offsetsYForSide[6] = {-1, 1, 0, 0, 0, 0};
    constexpr int32_t offsetsZForSide[6] = {0, 0, -1, 1, 0, 0};

    // Opposite direction
    constexpr int32_t oppositeSide[6] = {1, 0, 3, 2, 5, 4};
}

// ═══════════════════════════════════════════════════════════════════════════
// PistonMechanics — Core piston push/pull engine.
// Java reference: net.minecraft.block.BlockPistonBase
// ═══════════════════════════════════════════════════════════════════════════

class PistonMechanics {
public:
    static constexpr int32_t MAX_PUSH = 12;

    // Block IDs
    static constexpr int32_t AIR = 0;
    static constexpr int32_t PISTON = 33;
    static constexpr int32_t STICKY_PISTON = 29;
    static constexpr int32_t PISTON_HEAD = 34;
    static constexpr int32_t PISTON_EXTENSION = 36;
    static constexpr int32_t OBSIDIAN = 49;

    // Block info for push checks
    struct BlockInfo {
        int32_t blockId;
        int32_t metadata;
        float hardness;
        int32_t mobilityFlag;  // 0=normal, 1=destroy, 2=immovable
        bool hasTileEntity;
        bool isAir;
    };

    using GetBlockInfoFn = std::function<BlockInfo(int32_t, int32_t, int32_t)>;
    using GetIndirectPowerFn = std::function<bool(int32_t, int32_t, int32_t, int32_t)>;

    // ─── Metadata encoding ───

    // Java: getPistonOrientation — bottom 3 bits
    static int32_t getOrientation(int32_t meta) { return meta & 7; }

    // Java: isExtended — bit 3
    static bool isExtended(int32_t meta) { return (meta & 8) != 0; }

    // ─── Orientation from player position ───

    // Java: determineOrientation
    static int32_t determineOrientation(double playerX, double playerY, double playerZ,
                                          float yaw, float yOffset,
                                          int32_t blockX, int32_t blockY, int32_t blockZ) {
        if (std::abs(static_cast<float>(playerX) - static_cast<float>(blockX)) < 2.0f &&
            std::abs(static_cast<float>(playerZ) - static_cast<float>(blockZ)) < 2.0f) {
            double eyeY = playerY + 1.82 - static_cast<double>(yOffset);
            if (eyeY - static_cast<double>(blockY) > 2.0) return 1;  // up
            if (static_cast<double>(blockY) - eyeY > 0.0) return 0;  // down
        }

        int32_t facing = static_cast<int32_t>(std::floor(static_cast<double>(yaw * 4.0f / 360.0f) + 0.5)) & 3;
        switch (facing) {
            case 0: return 2;   // north (-Z)
            case 1: return 5;   // east (+X)
            case 2: return 3;   // south (+Z)
            case 3: return 4;   // west (-X)
            default: return 0;
        }
    }

    // ─── Power check ───

    // Java: isIndirectlyPowered — checks 12 positions
    static bool isIndirectlyPowered(int32_t x, int32_t y, int32_t z, int32_t direction,
                                       GetIndirectPowerFn getPower) {
        // Check 6 faces (skip own face direction)
        if (direction != 0 && getPower(x, y - 1, z, 0)) return true;
        if (direction != 1 && getPower(x, y + 1, z, 1)) return true;
        if (direction != 2 && getPower(x, y, z - 1, 2)) return true;
        if (direction != 3 && getPower(x, y, z + 1, 3)) return true;
        if (direction != 5 && getPower(x + 1, y, z, 5)) return true;
        if (direction != 4 && getPower(x - 1, y, z, 4)) return true;

        // Check positions offset by +1 in Y (quasi-connectivity)
        if (getPower(x, y, z, 0)) return true;            // below self
        if (getPower(x, y + 2, z, 1)) return true;        // above+1
        if (getPower(x, y + 1, z - 1, 2)) return true;    // north+up
        if (getPower(x, y + 1, z + 1, 3)) return true;    // south+up
        if (getPower(x - 1, y + 1, z, 4)) return true;    // west+up
        if (getPower(x + 1, y + 1, z, 5)) return true;    // east+up

        return false;
    }

    // ─── Push checks ───

    // Java: canPushBlock
    static bool canPushBlock(const BlockInfo& info, bool canDestroy) {
        // Obsidian: never
        if (info.blockId == OBSIDIAN) return false;

        // Pistons: only if not extended
        if (info.blockId == PISTON || info.blockId == STICKY_PISTON) {
            return !isExtended(info.metadata);
        }

        // Indestructible: never
        if (info.hardness == -1.0f) return false;

        // Mobility checks
        if (info.mobilityFlag == 2) return false;  // immovable
        if (info.mobilityFlag == 1) return canDestroy;  // destroy only

        // Tile entities: never pushable
        if (info.hasTileEntity) return false;

        return true;
    }

    // Java: canExtend — check if push chain is valid
    static bool canExtend(int32_t x, int32_t y, int32_t z, int32_t direction,
                            GetBlockInfoFn getInfo) {
        int32_t cx = x + Facing::offsetsXForSide[direction];
        int32_t cy = y + Facing::offsetsYForSide[direction];
        int32_t cz = z + Facing::offsetsZForSide[direction];

        for (int32_t i = 0; i < 13; ++i) {
            if (cy <= 0 || cy >= 255) return false;

            BlockInfo info = getInfo(cx, cy, cz);
            if (info.isAir) break;
            if (!canPushBlock(info, true)) return false;
            if (info.mobilityFlag == 1) break;  // will be destroyed
            if (i == 12) return false;  // too many blocks

            cx += Facing::offsetsXForSide[direction];
            cy += Facing::offsetsYForSide[direction];
            cz += Facing::offsetsZForSide[direction];
        }
        return true;
    }

    // ─── Push chain execution ───

    struct PushAction {
        enum Type {
            SET_BLOCK,        // Place a block
            SET_AIR,          // Remove a block
            SET_EXTENSION,    // Place piston extension tile
            NOTIFY_NEIGHBORS, // Notify neighbor blocks
            DROP_ITEM         // Drop item (mobility 1)
        };
        Type type;
        int32_t x, y, z;
        int32_t blockId;
        int32_t metadata;
        int32_t direction;
    };

    struct PushResult {
        std::vector<PushAction> actions;
        bool success;
    };

    // Java: tryExtend — execute push chain
    static PushResult tryExtend(int32_t x, int32_t y, int32_t z, int32_t direction,
                                   bool isSticky, int32_t pistonBlockId,
                                   GetBlockInfoFn getInfo) {
        PushResult result;
        result.success = false;

        int32_t cx = x + Facing::offsetsXForSide[direction];
        int32_t cy = y + Facing::offsetsYForSide[direction];
        int32_t cz = z + Facing::offsetsZForSide[direction];

        // Walk forward to find end of push chain
        int32_t count;
        for (count = 0; count < 13; ++count) {
            if (cy <= 0 || cy >= 255) return result;

            BlockInfo info = getInfo(cx, cy, cz);
            if (info.isAir) break;
            if (!canPushBlock(info, true)) return result;

            // Destroyable block (mobility 1)
            if (info.mobilityFlag == 1) {
                result.actions.push_back({PushAction::DROP_ITEM, cx, cy, cz,
                                          info.blockId, info.metadata, direction});
                result.actions.push_back({PushAction::SET_AIR, cx, cy, cz, AIR, 0, 0});
                break;
            }

            if (count == 12) return result;

            cx += Facing::offsetsXForSide[direction];
            cy += Facing::offsetsYForSide[direction];
            cz += Facing::offsetsZForSide[direction];
        }

        // End of chain position
        int32_t endX = cx, endY = cy, endZ = cz;

        // Walk backward: push each block forward one position
        struct PushedBlock {
            int32_t blockId, metadata;
        };
        std::vector<PushedBlock> pushedBlocks;

        while (cx != x || cy != y || cz != z) {
            int32_t prevX = cx - Facing::offsetsXForSide[direction];
            int32_t prevY = cy - Facing::offsetsYForSide[direction];
            int32_t prevZ = cz - Facing::offsetsZForSide[direction];
            BlockInfo prev = getInfo(prevX, prevY, prevZ);

            if (prev.blockId == pistonBlockId && prevX == x && prevY == y && prevZ == z) {
                // Piston head
                int32_t headMeta = direction | (isSticky ? 8 : 0);
                result.actions.push_back({PushAction::SET_EXTENSION, cx, cy, cz,
                                          PISTON_HEAD, headMeta, direction});
            } else {
                result.actions.push_back({PushAction::SET_EXTENSION, cx, cy, cz,
                                          prev.blockId, prev.metadata, direction});
            }
            pushedBlocks.push_back({prev.blockId, prev.metadata});

            cx = prevX;
            cy = prevY;
            cz = prevZ;
        }

        // Notify neighbors (walk from end backward)
        cx = endX; cy = endY; cz = endZ;
        int32_t blockIdx = 0;
        while (cx != x || cy != y || cz != z) {
            int32_t prevX = cx - Facing::offsetsXForSide[direction];
            int32_t prevY = cy - Facing::offsetsYForSide[direction];
            int32_t prevZ = cz - Facing::offsetsZForSide[direction];
            if (blockIdx < static_cast<int32_t>(pushedBlocks.size())) {
                result.actions.push_back({PushAction::NOTIFY_NEIGHBORS, prevX, prevY, prevZ,
                                          pushedBlocks[blockIdx].blockId, 0, 0});
            }
            ++blockIdx;
            cx = prevX;
            cy = prevY;
            cz = prevZ;
        }

        result.success = true;
        return result;
    }

    // ─── Retraction ───

    struct RetractResult {
        std::vector<PushAction> actions;
    };

    static RetractResult tryRetract(int32_t x, int32_t y, int32_t z, int32_t direction,
                                       bool isSticky, int32_t pistonBlockId,
                                       GetBlockInfoFn getInfo) {
        RetractResult result;

        // Set piston to extension state temporarily
        result.actions.push_back({PushAction::SET_EXTENSION, x, y, z,
                                  pistonBlockId, direction, direction});

        if (isSticky) {
            // Check 2 blocks in front for pullable block
            int32_t pullX = x + Facing::offsetsXForSide[direction] * 2;
            int32_t pullY = y + Facing::offsetsYForSide[direction] * 2;
            int32_t pullZ = z + Facing::offsetsZForSide[direction] * 2;
            BlockInfo pullInfo = getInfo(pullX, pullY, pullZ);

            if (!pullInfo.isAir && canPushBlock(pullInfo, false) &&
                (pullInfo.mobilityFlag == 0 ||
                 pullInfo.blockId == PISTON || pullInfo.blockId == STICKY_PISTON)) {
                // Pull block back
                int32_t destX = x + Facing::offsetsXForSide[direction];
                int32_t destY = y + Facing::offsetsYForSide[direction];
                int32_t destZ = z + Facing::offsetsZForSide[direction];
                result.actions.push_back({PushAction::SET_EXTENSION, destX, destY, destZ,
                                          pullInfo.blockId, pullInfo.metadata, direction});
                result.actions.push_back({PushAction::SET_AIR, pullX, pullY, pullZ, AIR, 0, 0});
            } else {
                // Nothing to pull — clear head position
                int32_t headX = x + Facing::offsetsXForSide[direction];
                int32_t headY = y + Facing::offsetsYForSide[direction];
                int32_t headZ = z + Facing::offsetsZForSide[direction];
                result.actions.push_back({PushAction::SET_AIR, headX, headY, headZ, AIR, 0, 0});
            }
        } else {
            // Normal piston: just clear head
            int32_t headX = x + Facing::offsetsXForSide[direction];
            int32_t headY = y + Facing::offsetsYForSide[direction];
            int32_t headZ = z + Facing::offsetsZForSide[direction];
            result.actions.push_back({PushAction::SET_AIR, headX, headY, headZ, AIR, 0, 0});
        }

        return result;
    }

    // ─── Block bounds when extended ───

    struct AABB {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
    };

    // Java: setBlockBoundsBasedOnState
    static AABB getExtendedBounds(int32_t direction) {
        switch (direction) {
            case 0: return {0.0f, 0.25f, 0.0f, 1.0f, 1.0f, 1.0f};  // down
            case 1: return {0.0f, 0.0f, 0.0f, 1.0f, 0.75f, 1.0f};  // up
            case 2: return {0.0f, 0.0f, 0.25f, 1.0f, 1.0f, 1.0f};  // north
            case 3: return {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.75f};  // south
            case 4: return {0.25f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};  // west
            case 5: return {0.0f, 0.0f, 0.0f, 0.75f, 1.0f, 1.0f};  // east
            default: return {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
        }
    }

    static AABB getRetractedBounds() {
        return {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    }
};

} // namespace mccpp
