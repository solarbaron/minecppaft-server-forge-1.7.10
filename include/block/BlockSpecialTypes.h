/**
 * BlockSpecialTypes.h — TNT, Falling Blocks, and Dispenser mechanics.
 *
 * Java references:
 *   - net.minecraft.block.BlockTNT (100 lines)
 *   - net.minecraft.block.BlockFalling (93 lines)
 *   - net.minecraft.block.BlockDispenser (202 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TNT (BlockTNT)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Ignition triggers:
 *   1. Placed next to redstone power → immediate ignition
 *   2. Neighbor block change with redstone power → ignition
 *   3. Player right-click with flint_and_steel → ignite + damage item
 *   4. Burning arrow collision → ignite (shooter = entityLivingBase)
 *   5. Destroyed by explosion → ignite with shorter random fuse
 *
 * Ignition process (func_150114_a):
 *   - If metadata bit 0 is set (activated):
 *     - Spawn EntityTNTPrimed at block center (x+0.5, y+0.5, z+0.5)
 *     - Play "game.tnt.primed" sound (volume=1.0, pitch=1.0)
 *
 * Explosion chain (onBlockDestroyedByExplosion):
 *   - Spawn TNT entity with shortened fuse:
 *     fuse = random(fuse/4) + fuse/8
 *   - Default fuse = 80 ticks (4 seconds)
 *   - Chain fuse range: 20-40 ticks (1-2 seconds)
 *
 * canDropFromExplosion: false (TNT always detonates, never drops)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FALLING BLOCKS (BlockFalling)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Blocks that obey gravity: sand, gravel, dragon egg, anvil, concrete powder
 *
 * Tick rate: 2 ticks
 * Static field: fallInstantly (used during world generation)
 *
 * Fall check (func_149830_m):
 *   1. Check if block below allows falling (canFallBelow)
 *   2. If fallInstantly OR chunks not loaded (32-block radius):
 *      - Instantly teleport: remove block, scan down, place at landing
 *   3. If chunks loaded (normal gameplay):
 *      - Spawn EntityFallingBlock at center (x+0.5, y+0.5, z+0.5)
 *      - Entity handles physics, landing, and item drops
 *
 * canFallBelow materials: air, fire, water, lava
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DISPENSER (BlockDispenser)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Tick rate: 4 ticks
 * Metadata: bits 0-2 = facing direction, bit 3 = powered state
 *
 * Direction auto-detection (func_149938_m):
 *   - Compare adjacent block fullness to determine facing
 *   - Default facing: north (3)
 *   - Player placement: uses BlockPistonBase.determineOrientation()
 *
 * Dispensing (func_149941_e):
 *   1. Get random slot from dispenser tile entity (func_146017_i)
 *   2. If slot -1 (empty): play "click" sound (1001)
 *   3. Look up behavior from dispenseBehaviorRegistry
 *   4. Execute behavior.dispense(source, stack)
 *   5. Update slot with result stack
 *
 * Dispensing position:
 *   - Offset = 0.7 blocks along facing direction
 *   - x + 0.7*frontX, y + 0.7*frontY, z + 0.7*frontZ
 *
 * Redstone power (onNeighborBlockChange):
 *   - Check power at position AND at Y+1 (for upper-half sensitivity)
 *   - Rising edge (power && !was_powered): schedule tick + set bit 3
 *   - Falling edge (!power && was_powered): clear bit 3
 *
 * Block break (breakBlock):
 *   - Drop all inventory contents as EntityItem
 *   - Item scatter: position = random(0.8) + 0.1 in each axis
 *   - Split stacks: random(21)+10 per drop entity
 *   - Item velocity: gaussian(0.05), Y+0.2 base upward
 *   - Copy NBT tag compound if present
 *
 * Comparator: outputs standard inventory fill level
 *
 * Thread safety: Block ticks happen on the server thread.
 * JNI readiness: Simple numeric constants and struct layouts.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// TNT Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TNTConstants {
    // ─── Fuse ───
    // Java: EntityTNTPrimed default fuse = 80 ticks
    static constexpr int32_t DEFAULT_FUSE = 80;

    // ─── Chain explosion fuse ───
    // Java: fuse = random(fuse/4) + fuse/8
    // Default: random(20) + 10 = 10-30 ticks
    static constexpr int32_t CHAIN_FUSE_RANDOM_DIVISOR = 4;
    static constexpr int32_t CHAIN_FUSE_BASE_DIVISOR = 8;

    // ─── Spawn offset ───
    // Java: EntityTNTPrimed(world, n+0.5, n2+0.5, n3+0.5, ...)
    static constexpr float SPAWN_OFFSET = 0.5f;

    // ─── Activation metadata bit ───
    // Java: (n4 & 1) == 1
    static constexpr int32_t ACTIVATED_BIT = 1;

    // ─── Sound ───
    static constexpr const char* PRIMED_SOUND = "game.tnt.primed";
    static constexpr float SOUND_VOLUME = 1.0f;
    static constexpr float SOUND_PITCH = 1.0f;

    // ─── Block ID ───
    static constexpr int32_t TNT_ID = 46;

    // ─── Explosion power ───
    // Java: EntityTNTPrimed — explosion strength = 4.0f
    static constexpr float EXPLOSION_POWER = 4.0f;

    // ─── Flint and steel item ID ───
    static constexpr int32_t FLINT_AND_STEEL_ID = 259;
}

// ═══════════════════════════════════════════════════════════════════════════
// Falling Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FallingBlockConstants {
    // ─── Tick rate ───
    // Java: tickRate = 2
    static constexpr int32_t TICK_RATE = 2;

    // ─── Chunk check radius ───
    // Java: checkChunksExist(n-32, n2-32, n3-32, n+32, n2+32, n3+32)
    static constexpr int32_t CHUNK_CHECK_RADIUS = 32;

    // ─── Spawn offset ───
    // Java: EntityFallingBlock(world, n+0.5, n2+0.5, n3+0.5, ...)
    static constexpr float SPAWN_OFFSET = 0.5f;

    // ─── Can fall below material IDs ───
    // Java: canFallBelow — air, fire, water, lava
    static constexpr int32_t AIR_MATERIAL = 0;
    static constexpr int32_t FIRE_ID = 51;

    // ─── Falling block IDs ───
    static constexpr int32_t SAND_ID = 12;
    static constexpr int32_t GRAVEL_ID = 13;
    static constexpr int32_t DRAGON_EGG_ID = 122;
    static constexpr int32_t ANVIL_ID = 145;

    // Check if a material allows falling through
    inline bool canFallThrough(int32_t blockId, bool isAir, bool isWater, bool isLava) {
        if (isAir) return true;
        if (blockId == FIRE_ID) return true;
        if (isWater) return true;
        if (isLava) return true;
        return false;
    }

    // ─── EntityFallingBlock constants ───
    // Java: EntityFallingBlock.java
    static constexpr int32_t MAX_FALL_TILE_TICKS = 600;  // 30 seconds
    static constexpr float FALL_HURT_AMOUNT = 2.0f;      // damage per block fallen
    static constexpr int32_t FALL_HURT_MAX = 40;          // max damage
    static constexpr int32_t MIN_FALL_HURT_HEIGHT = 1;    // min blocks to deal damage
}

// ═══════════════════════════════════════════════════════════════════════════
// Dispenser Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DispenserConstants {
    // ─── Tick rate ───
    // Java: tickRate = 4
    static constexpr int32_t TICK_RATE = 4;

    // ─── Metadata encoding ───
    // Java: bits 0-2 = facing, bit 3 = powered
    static constexpr int32_t FACING_MASK = 7;
    static constexpr int32_t POWERED_BIT = 8;
    static constexpr int32_t POWERED_CLEAR = ~POWERED_BIT;  // 0xFFFFFFF7

    // ─── Default facing ───
    // Java: n4 = 3 (north) default in func_149938_m
    static constexpr int32_t DEFAULT_FACING = 3;

    // ─── Facing directions ───
    // 0=down, 1=up, 2=north, 3=south, 4=west, 5=east
    static constexpr int32_t FACING_DOWN = 0;
    static constexpr int32_t FACING_UP = 1;
    static constexpr int32_t FACING_NORTH = 2;
    static constexpr int32_t FACING_SOUTH = 3;
    static constexpr int32_t FACING_WEST = 4;
    static constexpr int32_t FACING_EAST = 5;

    // ─── Facing direction offsets ───
    struct FacingOffset {
        int32_t x, y, z;
    };
    static constexpr FacingOffset FACING_OFFSETS[] = {
        { 0, -1,  0},  // 0: down
        { 0,  1,  0},  // 1: up
        { 0,  0, -1},  // 2: north
        { 0,  0,  1},  // 3: south
        {-1,  0,  0},  // 4: west
        { 1,  0,  0},  // 5: east
    };

    // ─── Dispensing position offset ───
    // Java: getIPositionFromBlockSource — 0.7 along facing
    static constexpr double DISPENSE_OFFSET = 0.7;

    // ─── Empty dispenser sound ───
    // Java: world.playAuxSFX(1001, ...)
    static constexpr int32_t EMPTY_SOUND_EVENT = 1001;

    // ─── Item scatter on break ───
    // Java: breakBlock — random(0.8) + 0.1 position
    static constexpr float SCATTER_RANGE = 0.8f;
    static constexpr float SCATTER_OFFSET = 0.1f;
    // Java: random(21) + 10 stack split
    static constexpr int32_t SPLIT_RANDOM_RANGE = 21;
    static constexpr int32_t SPLIT_MIN = 10;
    // Java: gaussian(0.05) + 0.2 Y
    static constexpr float VELOCITY_SPREAD = 0.05f;
    static constexpr float VELOCITY_Y_BASE = 0.2f;

    // ─── Block IDs ───
    static constexpr int32_t DISPENSER_ID = 23;
    static constexpr int32_t DROPPER_ID = 158;

    // ─── Inventory size ───
    static constexpr int32_t INVENTORY_SIZE = 9;

    // Get facing from metadata
    inline int32_t getFacing(int32_t metadata) {
        return metadata & FACING_MASK;
    }

    // Check if powered from metadata
    inline bool isPowered(int32_t metadata) {
        return (metadata & POWERED_BIT) != 0;
    }

    // Set powered bit in metadata
    inline int32_t setPowered(int32_t metadata, bool powered) {
        return powered ? (metadata | POWERED_BIT) : (metadata & POWERED_CLEAR);
    }

    // Get dispense position for a facing direction
    inline void getDispensePosition(double blockX, double blockY, double blockZ,
                                     int32_t facing,
                                     double& outX, double& outY, double& outZ) {
        const auto& offset = FACING_OFFSETS[facing < 6 ? facing : 0];
        outX = blockX + DISPENSE_OFFSET * offset.x;
        outY = blockY + DISPENSE_OFFSET * offset.y;
        outZ = blockZ + DISPENSE_OFFSET * offset.z;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// EnumFacing utility
// Java: net.minecraft.util.EnumFacing — 6 directions
// Used by dispenser and other directional blocks
// ═══════════════════════════════════════════════════════════════════════════

namespace EnumFacing {
    static constexpr int32_t DOWN = 0;
    static constexpr int32_t UP = 1;
    static constexpr int32_t NORTH = 2;
    static constexpr int32_t SOUTH = 3;
    static constexpr int32_t WEST = 4;
    static constexpr int32_t EAST = 5;
    static constexpr int32_t COUNT = 6;

    // Java: EnumFacing.getFront(n & 7) — get facing from metadata
    inline int32_t getFront(int32_t index) {
        return (index >= 0 && index < COUNT) ? index : 0;
    }

    // Get opposite facing
    inline int32_t getOpposite(int32_t facing) {
        switch (facing) {
            case DOWN:  return UP;
            case UP:    return DOWN;
            case NORTH: return SOUTH;
            case SOUTH: return NORTH;
            case WEST:  return EAST;
            case EAST:  return WEST;
            default:    return DOWN;
        }
    }

    // Front offsets (X, Y, Z)
    inline void getFrontOffset(int32_t facing, int32_t& ox, int32_t& oy, int32_t& oz) {
        ox = oy = oz = 0;
        switch (facing) {
            case DOWN:  oy = -1; break;
            case UP:    oy =  1; break;
            case NORTH: oz = -1; break;
            case SOUTH: oz =  1; break;
            case WEST:  ox = -1; break;
            case EAST:  ox =  1; break;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Placement orientation
// Java: BlockPistonBase.determineOrientation — player-relative facing
// Used by dispensers, droppers, pistons, hoppers
// ═══════════════════════════════════════════════════════════════════════════

namespace PlacementOrientation {
    // ─── Determine facing direction based on player pitch/yaw ───
    // Java: BlockPistonBase.determineOrientation(world, x, y, z, placer)
    //
    // Algorithm:
    //   1. Compute dx = placer.x - block.x, dz = placer.z - block.z
    //   2. If abs(dx) < 2 && abs(dz) < 2:
    //      - If placer.y + 1.82 - placer.yOffset > block.y + 2: facing = UP (1)
    //      - If placer.y + 1.82 - placer.yOffset < block.y:     facing = DOWN (0)
    //   3. Else: use placer horizontal facing (from yaw):
    //      - Yaw 0   = South (3)
    //      - Yaw 90  = West (4)
    //      - Yaw 180 = North (2)
    //      - Yaw 270 = East (5)

    // ─── Player eye height for orientation check ───
    static constexpr double PLAYER_EYE_HEIGHT = 1.82;

    // ─── Distance threshold for vertical orientation ───
    static constexpr double DISTANCE_THRESHOLD = 2.0;

    // ─── Yaw to horizontal facing ───
    // Java: MathHelper.floor_double(yaw/360 * 4 + 0.5) & 3
    // 0=south, 1=west, 2=north, 3=east
    static constexpr int32_t YAW_TO_FACING[] = {3, 4, 2, 5};  // south, west, north, east

    inline int32_t getHorizontalFacing(float yaw) {
        int32_t index = static_cast<int32_t>(std::floor(yaw / 360.0f * 4.0f + 0.5f)) & 3;
        return YAW_TO_FACING[index];
    }
}

} // namespace mccpp
