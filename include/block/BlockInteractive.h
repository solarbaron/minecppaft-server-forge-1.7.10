/**
 * BlockInteractive.h — Hopper, Bed, and Door mechanics.
 *
 * Java references:
 *   - net.minecraft.block.BlockHopper (183 lines)
 *   - net.minecraft.block.BlockBed (190 lines)
 *   - net.minecraft.block.BlockDoor (251 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * HOPPER (BlockHopper)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Collision: 5 AABB boxes forming funnel shape
 *   - Bottom slab: full width, 0.625 height
 *   - 4 walls: 0.125 thick, full height
 *
 * Metadata: bits 0-2 = output direction, bit 3 = disabled (redstone)
 *   - Direction from placement: oppositeSide[clickedFace]
 *   - If opposite is UP (1): use DOWN (0) instead
 *   - Redstone power: inverts bit 3 (powered = disabled)
 *
 * Placement: custom name transfers to TileEntityHopper
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BED (BlockBed)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Two-block structure: foot + head
 * Direction table: {{0,1}, {-1,0}, {0,-1}, {1,0}}
 *   Index maps to: South, West, North, East
 *
 * Metadata:
 *   - bits 0-1: direction (0-3)
 *   - bit 2 (0x4): occupied
 *   - bit 3 (0x8): is head block
 *
 * Activation:
 *   1. Navigate to head block if foot clicked
 *   2. Nether/End: explode (power 5.0, fire=true, destroy=true)
 *      - Center explosion between both blocks
 *   3. If occupied: "tile.bed.occupied" message
 *   4. Sleep attempt → EnumStatus: OK, NOT_POSSIBLE_NOW, NOT_SAFE
 *      - OK: set occupied bit
 *      - NOT_POSSIBLE_NOW: "tile.bed.noSleep" (daytime)
 *      - NOT_SAFE: "tile.bed.notSafe" (monsters nearby)
 *
 * Block height: 9/16 (0.5625)
 * Item drop: only from foot block (Items.bed)
 * Mobility: 1 (movable by piston)
 *
 * Safe exit (getSafeExitLocation):
 *   - Search foot side first, then head side
 *   - 3×3 area around each half
 *   - Need: solid below + non-opaque at Y and Y+1
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DOOR (BlockDoor)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Two-block structure: lower + upper half
 *
 * Full metadata (combining both halves):
 *   - bits 0-1: direction (0-3)
 *   - bit 2 (0x4): open state
 *   - bit 3 (0x8): is upper half
 *   - bit 4 (0x10): hinge on right (from upper)
 *
 * Direction-to-bounds mapping (3/16 = 0.1875 thick):
 *   dir=0: open → Z thin; closed → X thin (west side)
 *   dir=1: open → X east; closed → Z thin (north side)
 *   dir=2: open → Z south; closed → X east side
 *   dir=3: open → X west; closed → Z south
 *   + hinge flips the open bounds
 *
 * Activation:
 *   - Iron doors: do not respond to right-click
 *   - Wooden doors: toggle bit 2, update lower half metadata
 *   - Sound event: 1003
 *
 * Redstone (onNeighborBlockChange):
 *   - Lower half: validate upper exists, floor support
 *   - Check power at Y and Y+1
 *   - Open/close via func_150014_a
 *   - Upper half: validate lower exists, delegate
 *
 * Placement: needs solid floor, Y ≤ 254 (room for upper half)
 * Passability: isPassable when open (bit 2 set)
 * Mobility: 1 (movable by piston)
 *
 * Thread safety: Block ticks happen on the server thread.
 * JNI readiness: Simple numeric constants.
 */
#pragma once

#include <cstdint>
#include <array>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Hopper Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace HopperConstants {
    // ─── Block ID ───
    static constexpr int32_t HOPPER_ID = 154;

    // ─── Metadata ───
    static constexpr int32_t DIRECTION_MASK = 7;      // bits 0-2
    static constexpr int32_t DISABLED_BIT = 8;          // bit 3
    static constexpr int32_t DISABLED_CLEAR = ~DISABLED_BIT;

    // Get output direction from metadata
    inline int32_t getDirection(int32_t metadata) {
        return metadata & DIRECTION_MASK;
    }

    // Is hopper active (not disabled by redstone)
    // Java: getActiveStateFromMetadata — (n & 8) != 8
    inline bool isActive(int32_t metadata) {
        return (metadata & DISABLED_BIT) != DISABLED_BIT;
    }

    // ─── Collision boxes (funnel shape) ───
    // Java: addCollisionBoxesToList — 5 AABBs
    struct AABB {
        float minX, minY, minZ, maxX, maxY, maxZ;
    };

    // Bottom slab
    static constexpr AABB BOTTOM_SLAB = {0.0f, 0.0f, 0.0f, 1.0f, 0.625f, 1.0f};

    // Wall thickness
    static constexpr float WALL_THICKNESS = 0.125f;

    // 4 walls (west, north, east, south)
    static constexpr AABB WALLS[] = {
        {0.0f, 0.0f, 0.0f, WALL_THICKNESS, 1.0f, 1.0f},     // west wall
        {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, WALL_THICKNESS},     // north wall
        {1.0f - WALL_THICKNESS, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f}, // east wall
        {0.0f, 0.0f, 1.0f - WALL_THICKNESS, 1.0f, 1.0f, 1.0f}, // south wall
    };

    // ─── Placement ───
    // Java: onBlockPlaced — Facing.oppositeSide[face], UP(1)→DOWN(0)
    static constexpr int32_t OPPOSITE_SIDES[] = {1, 0, 3, 2, 5, 4};

    inline int32_t getPlacementDirection(int32_t clickedFace) {
        int32_t dir = (clickedFace >= 0 && clickedFace < 6) ? OPPOSITE_SIDES[clickedFace] : 0;
        // If output would be UP, use DOWN instead
        if (dir == 1) dir = 0;
        return dir;
    }

    // ─── Inventory ───
    static constexpr int32_t INVENTORY_SIZE = 5;

    // ─── Transfer cooldown ───
    // Java: TileEntityHopper — 8 tick transfer cooldown
    static constexpr int32_t TRANSFER_COOLDOWN = 8;
}

// ═══════════════════════════════════════════════════════════════════════════
// Bed Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace BedConstants {
    // ─── Block ID ───
    static constexpr int32_t BED_ID = 26;

    // ─── Block height ───
    // Java: setBedBounds → 0.5625f (9/16)
    static constexpr float BLOCK_HEIGHT = 0.5625f;

    // ─── Metadata bits ───
    static constexpr int32_t DIRECTION_MASK = 3;    // bits 0-1
    static constexpr int32_t OCCUPIED_BIT = 4;       // bit 2
    static constexpr int32_t HEAD_BIT = 8;           // bit 3
    static constexpr int32_t OCCUPIED_CLEAR = ~OCCUPIED_BIT;

    // ─── Direction table ───
    // Java: bedDirections = {{0,1}, {-1,0}, {0,-1}, {1,0}}
    // Maps direction index (0-3) to (deltaX, deltaZ) from foot → head
    struct DirectionOffset {
        int32_t dx, dz;
    };
    static constexpr DirectionOffset DIRECTIONS[] = {
        { 0,  1},  // 0: South
        {-1,  0},  // 1: West
        { 0, -1},  // 2: North
        { 1,  0},  // 3: East
    };

    // ─── Nether/End bed explosion ───
    // Java: world.newExplosion(null, x+0.5f, y+0.5f, z+0.5f, 5.0f, true, true)
    static constexpr float EXPLOSION_POWER = 5.0f;
    static constexpr bool EXPLOSION_FIRE = true;
    static constexpr bool EXPLOSION_DESTROY = true;

    // ─── Sleep status messages ───
    static constexpr const char* MSG_OCCUPIED = "tile.bed.occupied";
    static constexpr const char* MSG_NO_SLEEP = "tile.bed.noSleep";
    static constexpr const char* MSG_NOT_SAFE = "tile.bed.notSafe";

    // ─── Sleep status enum ───
    // Java: EntityPlayer.EnumStatus
    enum class SleepStatus {
        OK,
        NOT_POSSIBLE_NOW,
        NOT_SAFE,
        OTHER_PROBLEM,
        TOO_FAR_AWAY
    };

    // ─── Mobility ───
    static constexpr int32_t MOBILITY_FLAG = 1;  // movable by piston

    // Query helpers
    inline int32_t getDirection(int32_t metadata) {
        return metadata & DIRECTION_MASK;
    }

    inline bool isHead(int32_t metadata) {
        return (metadata & HEAD_BIT) != 0;
    }

    inline bool isOccupied(int32_t metadata) {
        return (metadata & OCCUPIED_BIT) != 0;
    }

    inline int32_t setOccupied(int32_t metadata, bool occupied) {
        return occupied ? (metadata | OCCUPIED_BIT) : (metadata & OCCUPIED_CLEAR);
    }

    // ─── Safe exit search ───
    // Java: getSafeExitLocation — search 3×3 around each half
    // Need: solid below + non-opaque at Y + non-opaque at Y+1
    static constexpr int32_t SAFE_EXIT_RADIUS = 1;
    static constexpr int32_t SAFE_EXIT_PHASES = 2;  // foot side, then head side
}

// ═══════════════════════════════════════════════════════════════════════════
// Door Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DoorConstants {
    // ─── Block IDs ───
    static constexpr int32_t WOODEN_DOOR_ID = 64;
    static constexpr int32_t IRON_DOOR_ID = 71;

    // ─── Full metadata bits ───
    // Combined from lower half (bits 0-2) + upper half (bit 0 = hinge)
    static constexpr int32_t DIRECTION_MASK = 3;    // bits 0-1
    static constexpr int32_t OPEN_BIT = 4;           // bit 2
    static constexpr int32_t UPPER_BIT = 8;          // bit 3
    static constexpr int32_t HINGE_BIT = 16;         // bit 4 (from upper half)

    // ─── Door thickness ───
    // Java: func_150011_b — 3/16 = 0.1875f
    static constexpr float THICKNESS = 0.1875f;

    // ─── Sound ───
    // Java: world.playAuxSFXAtEntity(player, 1003, ...)
    static constexpr int32_t DOOR_SOUND_EVENT = 1003;

    // ─── Max placement height ───
    // Java: if (n2 >= 255) return false
    static constexpr int32_t MAX_PLACEMENT_Y = 254;  // needs room for upper half

    // ─── Mobility ───
    static constexpr int32_t MOBILITY_FLAG = 1;

    // Query helpers
    inline int32_t getDirection(int32_t fullMeta) {
        return fullMeta & DIRECTION_MASK;
    }

    inline bool isOpen(int32_t fullMeta) {
        return (fullMeta & OPEN_BIT) != 0;
    }

    inline bool isUpperHalf(int32_t metadata) {
        return (metadata & UPPER_BIT) != 0;
    }

    inline bool isHingeRight(int32_t fullMeta) {
        return (fullMeta & HINGE_BIT) != 0;
    }

    // ─── Compute full metadata from both halves ───
    // Java: getFullMetadata — merges lower bits 0-2 + upper bit 0 into 5-bit value
    inline int32_t computeFullMetadata(int32_t lowerMeta, int32_t upperMeta, bool isUpper) {
        int32_t lower, upper;
        if (isUpper) {
            lower = lowerMeta;  // from Y-1
            upper = upperMeta;  // from current Y (this is the upper)
        } else {
            lower = upperMeta;  // current Y (this is the lower)
            upper = lowerMeta;  // from Y+1
        }
        bool hinge = (upper & 1) != 0;
        return (lower & 7) | (isUpper ? UPPER_BIT : 0) | (hinge ? HINGE_BIT : 0);
    }

    // ─── Door bounds for each direction/open/hinge state ───
    // Java: func_150011_b — 16 cases (4 dirs × open/closed × hinge)
    struct DoorBounds {
        float minX, minY, minZ, maxX, maxY, maxZ;
    };

    // Closed bounds per direction (thin along one axis)
    inline DoorBounds getClosedBounds(int32_t direction) {
        switch (direction) {
            case 0: return {0.0f, 0.0f, 0.0f, THICKNESS, 1.0f, 1.0f};     // thin west
            case 1: return {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, THICKNESS};     // thin north
            case 2: return {1.0f - THICKNESS, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f}; // thin east
            case 3: return {0.0f, 0.0f, 1.0f - THICKNESS, 1.0f, 1.0f, 1.0f}; // thin south
            default: return {0.0f, 0.0f, 0.0f, THICKNESS, 1.0f, 1.0f};
        }
    }

    // Open bounds per direction (rotated 90°, hinge-dependent)
    inline DoorBounds getOpenBounds(int32_t direction, bool hinge) {
        switch (direction) {
            case 0:
                return hinge ?
                    DoorBounds{0.0f, 0.0f, 1.0f - THICKNESS, 1.0f, 1.0f, 1.0f} :
                    DoorBounds{0.0f, 0.0f, 0.0f, 1.0f, 1.0f, THICKNESS};
            case 1:
                return hinge ?
                    DoorBounds{0.0f, 0.0f, 0.0f, THICKNESS, 1.0f, 1.0f} :
                    DoorBounds{1.0f - THICKNESS, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
            case 2:
                return hinge ?
                    DoorBounds{0.0f, 0.0f, 0.0f, 1.0f, 1.0f, THICKNESS} :
                    DoorBounds{0.0f, 0.0f, 1.0f - THICKNESS, 1.0f, 1.0f, 1.0f};
            case 3:
                return hinge ?
                    DoorBounds{1.0f - THICKNESS, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f} :
                    DoorBounds{0.0f, 0.0f, 0.0f, THICKNESS, 1.0f, 1.0f};
            default:
                return {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, THICKNESS};
        }
    }

    // Get bounds based on full metadata
    inline DoorBounds getBounds(int32_t fullMeta) {
        int32_t dir = getDirection(fullMeta);
        if (isOpen(fullMeta)) {
            return getOpenBounds(dir, isHingeRight(fullMeta));
        }
        return getClosedBounds(dir);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Chest Constants
// Java: net.minecraft.block.BlockChest — referenced for completeness
// ═══════════════════════════════════════════════════════════════════════════

namespace ChestConstants {
    // ─── Block IDs ───
    static constexpr int32_t CHEST_ID = 54;
    static constexpr int32_t ENDER_CHEST_ID = 130;
    static constexpr int32_t TRAPPED_CHEST_ID = 146;

    // ─── Inventory size ───
    static constexpr int32_t SINGLE_SIZE = 27;
    static constexpr int32_t DOUBLE_SIZE = 54;

    // ─── Metadata: facing direction ───
    // 2=north, 3=south, 4=west, 5=east
    static constexpr int32_t FACING_NORTH = 2;
    static constexpr int32_t FACING_SOUTH = 3;
    static constexpr int32_t FACING_WEST = 4;
    static constexpr int32_t FACING_EAST = 5;

    // ─── Max adjacent chests ───
    // Chests can be at most 2 wide (double chest)
    // Java: isOcelotBlockingChest — cats on top prevent opening
    static constexpr int32_t MAX_WIDTH = 2;

    // ─── Trapped chest comparator output ───
    // Java: output = numPlayersUsing (0-15)
    static constexpr int32_t MAX_COMPARATOR_OUTPUT = 15;
}

// ═══════════════════════════════════════════════════════════════════════════
// Facing utility
// Java: net.minecraft.util.Facing — opposite sides
// ═══════════════════════════════════════════════════════════════════════════

namespace FacingUtil {
    // Java: Facing.oppositeSide = {1, 0, 3, 2, 5, 4}
    static constexpr int32_t OPPOSITE[] = {1, 0, 3, 2, 5, 4};

    inline int32_t getOpposite(int32_t side) {
        return (side >= 0 && side < 6) ? OPPOSITE[side] : 0;
    }

    // Java: Facing.offsetsXForSide, etc.
    static constexpr int32_t OFFSETS_X[] = { 0,  0,  0,  0, -1,  1};
    static constexpr int32_t OFFSETS_Y[] = {-1,  1,  0,  0,  0,  0};
    static constexpr int32_t OFFSETS_Z[] = { 0,  0, -1,  1,  0,  0};
}

} // namespace mccpp
