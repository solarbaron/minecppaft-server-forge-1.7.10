/**
 * Redstone.cpp — Redstone system implementation.
 *
 * Java references:
 *   net.minecraft.block.BlockRedstoneWire — Signal propagation
 *   net.minecraft.block.BlockRedstoneTorch — Burnout protection
 *   net.minecraft.block.BlockPistonBase — Push/pull mechanics
 *
 * Key behaviors preserved from Java:
 *   - Wire signal decays by 1 per block
 *   - Torch burnout: 8 toggles in 60 ticks = burnout
 *   - Piston push limit: 12 blocks
 *   - Repeater always outputs 15 (full refresh)
 */

#include "redstone/Redstone.h"
#include <algorithm>
#include <cmath>

namespace mccpp {

// ═════════════════════════════════════════════════════════════════════════════
// RedstoneSignal
// ═════════════════════════════════════════════════════════════════════════════

// Java: BlockRedstoneWire.func_150175_a
// Calculate signal strength at a wire position by examining neighbors.
int32_t RedstoneSignal::calculateSignal(int32_t x, int32_t y, int32_t z,
                                          int32_t currentPower,
                                          GetBlockFn getBlock,
                                          GetMetaFn getMeta,
                                          IsSolidFn isSolid) {
    int32_t maxPower = MIN_POWER;

    // Check all 4 horizontal neighbors for redstone wire
    // Java: checks N/S/E/W plus up/down connections through solid blocks
    constexpr int32_t dx[] = {-1, 1, 0, 0};
    constexpr int32_t dz[] = {0, 0, -1, 1};

    for (int i = 0; i < 4; ++i) {
        int32_t nx = x + dx[i];
        int32_t nz = z + dz[i];
        int32_t blockId = getBlock(nx, y, nz);

        // Direct horizontal wire connection
        if (blockId == RedstoneBlocks::REDSTONE_WIRE) {
            int32_t neighborPower = getMeta(nx, y, nz);
            maxPower = std::max(maxPower, neighborPower - 1);
        }

        // Power sources (torch, repeater, block, etc.)
        if (blockId == RedstoneBlocks::REDSTONE_TORCH_ON) {
            maxPower = MAX_POWER;
        }
        if (blockId == RedstoneBlocks::REDSTONE_BLOCK) {
            maxPower = MAX_POWER;
        }
        if (blockId == RedstoneBlocks::REDSTONE_REPEATER_ON) {
            // Check if repeater is facing toward this wire
            int32_t repMeta = getMeta(nx, y, nz);
            int32_t facing = repMeta & 0x3;
            // Facing: 0=S(+z), 1=W(-x), 2=N(-z), 3=E(+x)
            bool facingToward = false;
            if (facing == 0 && dz[i] == -1) facingToward = true;  // facing south, we're north
            if (facing == 1 && dx[i] == 1) facingToward = true;   // facing west, we're east
            if (facing == 2 && dz[i] == 1) facingToward = true;   // facing north, we're south
            if (facing == 3 && dx[i] == -1) facingToward = true;  // facing east, we're west
            if (facingToward) maxPower = MAX_POWER;
        }

        // Wire going up through adjacent block
        if (isSolid(nx, y, nz)) {
            // If adjacent block is solid, check for wire on top
            int32_t aboveId = getBlock(nx, y + 1, nz);
            if (aboveId == RedstoneBlocks::REDSTONE_WIRE) {
                int32_t abovePower = getMeta(nx, y + 1, nz);
                maxPower = std::max(maxPower, abovePower - 1);
            }
        } else {
            // If not solid, check for wire below
            int32_t belowId = getBlock(nx, y - 1, nz);
            if (belowId == RedstoneBlocks::REDSTONE_WIRE) {
                int32_t belowPower = getMeta(nx, y - 1, nz);
                maxPower = std::max(maxPower, belowPower - 1);
            }
        }
    }

    return std::max(maxPower, MIN_POWER);
}

// Java: BlockRedstoneWire.func_150177_e — Full network update
void RedstoneSignal::updateSurroundingRedstone(int32_t x, int32_t y, int32_t z,
                                                 GetBlockFn getBlock,
                                                 GetMetaFn getMeta,
                                                 SetMetaFn setMeta,
                                                 NotifyFn notify,
                                                 IsSolidFn isSolid) {
    int32_t currentMeta = getMeta(x, y, z);
    int32_t newPower = calculateSignal(x, y, z, currentMeta, getBlock, getMeta, isSolid);

    if (newPower != currentMeta) {
        // Update wire power level
        setMeta(x, y, z, newPower);

        // Notify all neighbors
        notifyWireNeighbors(x, y, z, notify);

        // Recursively update connected wires
        constexpr int32_t dx[] = {-1, 1, 0, 0};
        constexpr int32_t dz[] = {0, 0, -1, 1};

        for (int i = 0; i < 4; ++i) {
            int32_t nx = x + dx[i];
            int32_t nz = z + dz[i];
            if (getBlock(nx, y, nz) == RedstoneBlocks::REDSTONE_WIRE) {
                updateSurroundingRedstone(nx, y, nz, getBlock, getMeta, setMeta, notify, isSolid);
            }
            // Check up/down connections
            if (getBlock(nx, y + 1, nz) == RedstoneBlocks::REDSTONE_WIRE) {
                updateSurroundingRedstone(nx, y + 1, nz, getBlock, getMeta, setMeta, notify, isSolid);
            }
            if (getBlock(nx, y - 1, nz) == RedstoneBlocks::REDSTONE_WIRE) {
                updateSurroundingRedstone(nx, y - 1, nz, getBlock, getMeta, setMeta, notify, isSolid);
            }
        }
    }
}

// Java: BlockRedstoneWire.func_150172_m — Notify blocks around wire
void RedstoneSignal::notifyWireNeighbors(int32_t x, int32_t y, int32_t z, NotifyFn notify) {
    notify(x, y, z);
    // Notify all 6 faces
    for (int face = 0; face < 6; ++face) {
        notify(x + FACING_OFFSETS[face].dx,
               y + FACING_OFFSETS[face].dy,
               z + FACING_OFFSETS[face].dz);
    }
}

int32_t RedstoneSignal::getMaxCurrentStrength(int32_t x, int32_t y, int32_t z,
                                                int32_t currentMax,
                                                GetBlockFn getBlock,
                                                GetMetaFn getMeta,
                                                IsSolidFn /*isSolid*/) {
    if (getBlock(x, y, z) == RedstoneBlocks::REDSTONE_WIRE) {
        int32_t power = getMeta(x, y, z);
        return std::max(currentMax, power);
    }
    return currentMax;
}

// ═════════════════════════════════════════════════════════════════════════════
// RedstoneTorch
// ═════════════════════════════════════════════════════════════════════════════

// Java: BlockRedstoneTorch.func_150110_m — Check if torch receives power
bool RedstoneTorch::isReceivingPower(int32_t x, int32_t y, int32_t z,
                                      int32_t metadata,
                                      RedstoneSignal::GetBlockFn getBlock,
                                      RedstoneSignal::GetMetaFn getMeta) {
    // Metadata encodes which face the torch is attached to:
    // 1=east wall, 2=west wall, 3=south wall, 4=north wall, 5=floor
    // Check the block the torch is attached to for power
    (void)getBlock; (void)getMeta;

    // Java maps metadata to the attached direction and checks
    // getIndirectPowerOutput on the attached block
    switch (metadata) {
        case 5: // floor-mounted: check block below
            // Would check y-1 for indirect power
            return false;
        case 3: // south wall
            return false;
        case 4: // north wall
            return false;
        case 1: // east wall
            return false;
        case 2: // west wall
            return false;
        default:
            return false;
    }
}

// Java: BlockRedstoneTorch.func_150111_a — Burnout protection
bool RedstoneTorch::checkBurnout(int32_t x, int32_t y, int32_t z,
                                   int64_t worldTime,
                                   std::vector<Toggle>& toggleList,
                                   bool addToggle) {
    if (addToggle) {
        toggleList.push_back({x, y, z, worldTime});
    }

    // Clean up old toggles (older than TOGGLE_WINDOW ticks)
    while (!toggleList.empty() &&
           worldTime - toggleList.front().worldTime > TOGGLE_WINDOW) {
        toggleList.erase(toggleList.begin());
    }

    // Count recent toggles at this position
    int32_t count = 0;
    for (const auto& toggle : toggleList) {
        if (toggle.x == x && toggle.y == y && toggle.z == z) {
            if (++count >= MAX_TOGGLES) {
                return true; // Burned out!
            }
        }
    }
    return false;
}

// Java: BlockRedstoneTorch.isProvidingWeakPower
int32_t RedstoneTorch::getWeakPower(bool isLit, int32_t metadata, int32_t side) {
    if (!isLit) return 0;

    // Don't provide power to the face the torch is attached to
    // Java's exact checks per metadata value
    if (metadata == 5 && side == 1) return 0; // floor: no power up
    if (metadata == 3 && side == 3) return 0; // south wall
    if (metadata == 4 && side == 2) return 0; // north wall
    if (metadata == 1 && side == 5) return 0; // east wall
    if (metadata == 2 && side == 4) return 0; // west wall

    return 15;
}

// Java: BlockRedstoneTorch.isProvidingStrongPower
int32_t RedstoneTorch::getStrongPower(bool isLit, int32_t /*metadata*/, int32_t side) {
    // Strong power only downward (side 0)
    if (side == 0) {
        return getWeakPower(isLit, 5, side);
    }
    return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// RedstoneRepeater — Locking check
// ═══════════════════════════════════════════════════════════════════════════

bool RedstoneRepeater::isLocked(int32_t x, int32_t y, int32_t z,
                                  int32_t metadata,
                                  RedstoneSignal::GetBlockFn getBlock,
                                  RedstoneSignal::GetMetaFn getMeta) {
    // A repeater is locked when an adjacent powered repeater
    // is pointing into its side.
    int32_t facing = getFacing(metadata);

    // Check perpendicular sides for powered repeaters
    // Facing 0(S) or 2(N): check East and West
    // Facing 1(W) or 3(E): check North and South
    int32_t side1x = x, side1z = z, side2x = x, side2z = z;
    if (facing == 0 || facing == 2) {
        side1x = x - 1; // west
        side2x = x + 1; // east
    } else {
        side1z = z - 1; // north
        side2z = z + 1; // south
    }

    // Check if side1 has a powered repeater facing toward us
    int32_t block1 = getBlock(side1x, y, side1z);
    if (block1 == RedstoneBlocks::REDSTONE_REPEATER_ON) {
        int32_t meta1 = getMeta(side1x, y, side1z);
        int32_t f1 = getFacing(meta1);
        // Check if that repeater faces toward this one
        if ((facing == 0 || facing == 2) && (f1 == 3)) return true;
        if ((facing == 1 || facing == 3) && (f1 == 0)) return true;
    }

    int32_t block2 = getBlock(side2x, y, side2z);
    if (block2 == RedstoneBlocks::REDSTONE_REPEATER_ON) {
        int32_t meta2 = getMeta(side2x, y, side2z);
        int32_t f2 = getFacing(meta2);
        if ((facing == 0 || facing == 2) && (f2 == 1)) return true;
        if ((facing == 1 || facing == 3) && (f2 == 2)) return true;
    }

    return false;
}

// ═══════════════════════════════════════════════════════════════════════════
// PistonMechanics
// ═══════════════════════════════════════════════════════════════════════════

// Java: BlockPistonBase.canExtend — Check if piston can push blocks
bool PistonMechanics::canExtend(int32_t x, int32_t y, int32_t z,
                                  int32_t direction,
                                  RedstoneSignal::GetBlockFn getBlock) {
    // Walk along push direction up to MAX_PUSH_DISTANCE
    int32_t dx = FACING_OFFSETS[direction].dx;
    int32_t dy = FACING_OFFSETS[direction].dy;
    int32_t dz = FACING_OFFSETS[direction].dz;

    for (int32_t i = 1; i <= MAX_PUSH_DISTANCE + 1; ++i) {
        int32_t bx = x + dx * i;
        int32_t by = y + dy * i;
        int32_t bz = z + dz * i;

        int32_t blockId = getBlock(bx, by, bz);

        // Air = can extend (nothing to push further)
        if (blockId == 0) return true;

        // Unpushable block = cannot extend
        if (isUnpushable(blockId)) return false;

        // Check if this block can be pushed
        if (!canPushBlock(blockId, false)) return false;

        // If we've reached max distance and still have blocks, fail
        if (i == MAX_PUSH_DISTANCE + 1) return false;
    }

    return true;
}

// Java: BlockPistonBase.tryExtend
bool PistonMechanics::tryExtend(int32_t x, int32_t y, int32_t z,
                                  int32_t direction, bool isSticky,
                                  RedstoneSignal::GetBlockFn getBlock,
                                  RedstoneSignal::SetMetaFn setMeta,
                                  RedstoneSignal::NotifyFn notify) {
    (void)isSticky; // Used for retract (separate path)

    if (!canExtend(x, y, z, direction, getBlock)) return false;

    int32_t dx = FACING_OFFSETS[direction].dx;
    int32_t dy = FACING_OFFSETS[direction].dy;
    int32_t dz = FACING_OFFSETS[direction].dz;

    // Collect blocks to push
    std::vector<RedstoneSignal::BlockPos> toMove;
    for (int32_t i = 1; i <= MAX_PUSH_DISTANCE; ++i) {
        int32_t bx = x + dx * i;
        int32_t by = y + dy * i;
        int32_t bz = z + dz * i;

        int32_t blockId = getBlock(bx, by, bz);
        if (blockId == 0) break; // Air, done
        toMove.push_back({bx, by, bz});
    }

    // Move blocks from farthest to nearest (to avoid overwriting)
    for (auto it = toMove.rbegin(); it != toMove.rend(); ++it) {
        int32_t destX = it->x + dx;
        int32_t destY = it->y + dy;
        int32_t destZ = it->z + dz;
        // Would call world.setBlock to move each block
        // Simplified: just notify
        notify(destX, destY, destZ);
    }

    // Place piston head at extend position
    int32_t headX = x + dx;
    int32_t headY = y + dy;
    int32_t headZ = z + dz;
    // setMeta for piston head
    setMeta(headX, headY, headZ, direction | 0x8);

    // Mark piston base as extended
    setMeta(x, y, z, direction | 0x8);

    notify(x, y, z);
    return true;
}

// Java: BlockPistonBase.isIndirectlyPowered
bool PistonMechanics::isIndirectlyPowered(int32_t x, int32_t y, int32_t z,
                                            int32_t direction,
                                            RedstoneSignal::GetBlockFn getBlock,
                                            RedstoneSignal::GetMetaFn getMeta) {
    // Check all 6 sides except the piston face direction
    for (int face = 0; face < 6; ++face) {
        if (face == direction) continue; // Skip piston face

        int32_t nx = x + FACING_OFFSETS[face].dx;
        int32_t ny = y + FACING_OFFSETS[face].dy;
        int32_t nz = z + FACING_OFFSETS[face].dz;

        int32_t blockId = getBlock(nx, ny, nz);

        // Direct power sources
        if (blockId == RedstoneBlocks::REDSTONE_TORCH_ON) return true;
        if (blockId == RedstoneBlocks::REDSTONE_BLOCK) return true;

        // Redstone wire (only powers if signal > 0)
        if (blockId == RedstoneBlocks::REDSTONE_WIRE) {
            int32_t wirePower = getMeta(nx, ny, nz);
            if (wirePower > 0) return true;
        }

        // Powered repeater facing this piston
        if (blockId == RedstoneBlocks::REDSTONE_REPEATER_ON) {
            int32_t repMeta = getMeta(nx, ny, nz);
            int32_t repFacing = repMeta & 0x3;
            // Check if repeater output faces piston
            int32_t oppFace = OPPOSITE_FACE[face];
            // Map repeater facing (0=S,1=W,2=N,3=E) to face
            int32_t repOutputFace = -1;
            switch (repFacing) {
                case 0: repOutputFace = 3; break; // South
                case 1: repOutputFace = 4; break; // West
                case 2: repOutputFace = 2; break; // North
                case 3: repOutputFace = 5; break; // East
            }
            if (repOutputFace == oppFace) return true;
        }
    }

    // Special case: check block below for redstone (Java BUD-style)
    // Java: world.getIndirectPowerOutput checks down(-Y) for all pistons
    if (direction != 0) { // Not facing down
        int32_t belowBlock = getBlock(x, y - 1, z);
        if (belowBlock == RedstoneBlocks::REDSTONE_TORCH_ON) return true;
        if (belowBlock == RedstoneBlocks::REDSTONE_WIRE &&
            getMeta(x, y - 1, z) > 0) return true;
    }

    return false;
}

// Java: BlockPistonBase.determineOrientation
int32_t PistonMechanics::determineOrientation(double playerX, double playerY, double playerZ,
                                                int32_t blockX, int32_t blockY, int32_t blockZ,
                                                float playerPitch) {
    // Java: MathHelper.abs(playerX - blockX), etc.
    // If player is looking up/down enough, piston faces up/down
    if (std::abs(playerPitch) > 45.0f) {
        // Looking up → piston faces down (0), looking down → faces up (1)
        double dy = playerY - static_cast<double>(blockY);
        if (playerPitch > 0 && dy < 2.0) return 0; // down
        return 1; // up
    }

    // Otherwise determine horizontal facing from player position
    double dx = playerX - static_cast<double>(blockX) - 0.5;
    double dz = playerZ - static_cast<double>(blockZ) - 0.5;

    float angle = static_cast<float>(std::atan2(dz, dx) * 180.0 / M_PI);
    // Normalize to 0-360
    if (angle < 0) angle += 360.0f;

    // 0° = East(+X), 90° = South(+Z), 180° = West(-X), 270° = North(-Z)
    if (angle >= 315.0f || angle < 45.0f) return 5;  // East
    if (angle >= 45.0f && angle < 135.0f) return 3;   // South
    if (angle >= 135.0f && angle < 225.0f) return 4;  // West
    return 2; // North
}

} // namespace mccpp
