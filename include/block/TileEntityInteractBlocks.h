/**
 * TileEntityInteractBlocks.h — Sign, bed, flower pot, and skull.
 *
 * Java references:
 *   - net.minecraft.block.BlockSign (128 lines)
 *   - net.minecraft.block.BlockBed (190 lines)
 *   - net.minecraft.block.BlockFlowerPot (209 lines)
 *   - net.minecraft.block.BlockSkull (232 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SIGN (BlockSign)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, material: wood
 * TileEntity: TileEntitySign
 * Render type: -1 (TESR only)
 * No collision AABB, passable, not opaque
 *
 * Two variants:
 *   Standing (field_149967_b=true): 16 rotations via meta 0-15
 *     Bounds: 0.25-0.75 XZ, 0-1.0 Y (full)
 *     Support: solid material below
 *
 *   Wall (field_149967_b=false): attached to wall
 *     Meta 2-5 → direction
 *     Bounds: 2/16 deep panel, Y 0.28125-0.78125
 *       meta 2: Z 0.875-1.0 (south face)
 *       meta 3: Z 0.0-0.125 (north face)
 *       meta 4: X 0.875-1.0 (east face)
 *       meta 5: X 0.0-0.125 (west face)
 *     Support: solid material on attached face
 *
 * Drop: Items.sign
 *
 * Block IDs: standing_sign(63), wall_sign(68)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BED (BlockBed)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockDirectional, material: cloth
 * Render type: 14
 * Not opaque, not normal
 * Mobility: 1
 *
 * 2-block structure with direction offsets:
 *   bedDirections[4][2] = {{0,1}, {-1,0}, {0,-1}, {1,0}}
 *
 * Metadata:
 *   bits 0-1: direction (0-3)
 *   bit 2: occupied (1) / empty (0)
 *   bit 3: head (1) / foot (0)
 *
 * Bounds: 0-1 XZ, 0-0.5625 Y (9/16)
 *
 * Right-click behavior:
 *   1. Redirect foot→head if needed
 *   2. Nether/End: explode (power 5.0, fire=true, destroy=true)
 *   3. Occupied: "tile.bed.occupied" message
 *   4. sleepInBedAt → OK/NOT_POSSIBLE_NOW/NOT_SAFE
 *   5. Set occupied on success
 *
 * Sleep status enum: OK, NOT_POSSIBLE_NOW, NOT_SAFE, OTHER_PROBLEM
 *
 * getSafeExitLocation:
 *   Scans both halves, 3×3 area per half
 *   Requires: solid below + non-opaque at Y and Y+1
 *   Skips n4 valid positions before returning
 *
 * neighbor → orphan detection (head without foot, foot without head)
 * Drop: foot only → Items.bed
 * Creative harvest from head: removes foot
 *
 * Block ID: bed(26)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FLOWER POT (BlockFlowerPot)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, material: circuits
 * TileEntity: TileEntityFlowerPot
 * Render type: 33
 * Not opaque, not normal
 *
 * Bounds: centered 6/16 wide, 6/16 tall (0.1875-0.8125 XZ, 0-0.375 Y)
 *
 * Accepts plants (right-click):
 *   yellow_flower, red_flower, cactus, brown_mushroom, red_mushroom,
 *   sapling, deadbush, tallgrass(meta 2 only = fern)
 *   Must be ItemBlock, TE must be empty
 *   Consumes 1 item (not in creative)
 *
 * Meta→plant mapping (createNewTileEntity):
 *   1=red_flower, 2=yellow_flower
 *   3-6=sapling(0-3), 12=sapling(4), 13=sapling(5)
 *   7=red_mushroom, 8=brown_mushroom
 *   9=cactus, 10=deadbush, 11=tallgrass(2)
 *
 * Break: drops pot (Items.flower_pot) + plant from TE
 * Creative harvest: clears TE content
 * Requires solid top surface below
 *
 * Block ID: flower_pot(140)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SKULL (BlockSkull)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, material: circuits
 * TileEntity: TileEntitySkull
 * Render type: -1 (TESR only)
 * Not opaque, not normal
 *
 * 5 skull types: 0=skeleton, 1=wither_skeleton, 2=zombie, 3=player, 4=creeper
 *
 * Placement meta (bits 0-2):
 *   1=floor, 2=north wall, 3=south wall, 4=east wall, 5=west wall
 *   Floor: yaw rotation (floor(yaw*4/360+2.5) & 3)
 *
 * Bounds: 8/16 × 8/16 head
 *   Floor (default): XZ 0.25-0.75, Y 0-0.5
 *   Wall 2: Z 0.5-1.0, Y 0.25-0.75
 *   Wall 3: Z 0.0-0.5, Y 0.25-0.75
 *   Wall 4: X 0.5-1.0, Y 0.25-0.75
 *   Wall 5: X 0.0-0.5, Y 0.25-0.75
 *
 * Wither spawn check (makeWither):
 *   Requires 3 wither skulls in a row (Z or X axis)
 *   T-shape soul sand pattern below:
 *     3 soul sand at Y-1, 1 soul sand at Y-2 center
 *   Not on PEACEFUL difficulty
 *   Clears 7 blocks, spawns EntityWither
 *   Position: center of T at Y-1.45
 *   Achievement: spawnWither (50-block radius)
 *   120 snowballpoof particles
 *
 * Drop:
 *   Items.skull with TE skull type
 *   Player skull (type 3): preserves SkullOwner NBT
 *   Creative: sets bit 3 to suppress drop
 *
 * Block ID: skull(144)
 *
 * Thread safety: TE access on server thread.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Sign Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SignConstants {
    // ─── Block IDs ───
    static constexpr int32_t STANDING_ID = 63;
    static constexpr int32_t WALL_ID = 68;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = -1;

    // ─── Standing: 16 rotation angles ───
    static constexpr int32_t MAX_ROTATION = 15;

    // ─── Wall bounds ───
    static constexpr float WALL_DEPTH = 0.125f;          // 2/16
    static constexpr float WALL_Y_MIN = 0.28125f;       // 4.5/16
    static constexpr float WALL_Y_MAX = 0.78125f;       // 12.5/16
}

// ═══════════════════════════════════════════════════════════════════════════
// Bed Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace BedConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 26;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 14;

    // ─── Direction offsets ───
    static constexpr int32_t BED_DIR[4][2] = {{0,1}, {-1,0}, {0,-1}, {1,0}};

    // ─── Metadata ───
    static constexpr int32_t DIR_MASK = 3;               // bits 0-1
    static constexpr int32_t OCCUPIED_FLAG = 4;          // bit 2
    static constexpr int32_t HEAD_FLAG = 8;              // bit 3

    // ─── Bounds ───
    static constexpr float HEIGHT = 0.5625f;             // 9/16

    // ─── Nether explosion ───
    static constexpr float EXPLOSION_POWER = 5.0f;

    // ─── Mobility ───
    static constexpr int32_t MOBILITY = 1;
}

// ═══════════════════════════════════════════════════════════════════════════
// Flower Pot Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FlowerPotConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 140;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 33;

    // ─── Bounds ───
    static constexpr float SIZE = 0.375f;                // 6/16
    static constexpr float HALF_SIZE = 0.1875f;          // 3/16

    // ─── Meta→plant table ───
    // 1=red_flower(0), 2=yellow_flower, 3-6=sapling(0-3)
    // 7=red_mushroom, 8=brown_mushroom, 9=cactus
    // 10=deadbush, 11=tallgrass(2), 12=sapling(4), 13=sapling(5)
}

// ═══════════════════════════════════════════════════════════════════════════
// Skull Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SkullConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 144;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = -1;

    // ─── Skull types ───
    static constexpr int32_t SKELETON = 0;
    static constexpr int32_t WITHER_SKELETON = 1;
    static constexpr int32_t ZOMBIE = 2;
    static constexpr int32_t PLAYER = 3;
    static constexpr int32_t CREEPER = 4;

    // ─── Placement meta ───
    static constexpr int32_t DIR_MASK = 7;               // bits 0-2
    static constexpr int32_t CREATIVE_BIT = 8;           // bit 3 (suppress drop)

    // ─── Head bounds ───
    static constexpr float HEAD_MIN = 0.25f;             // 4/16
    static constexpr float HEAD_MAX = 0.75f;             // 12/16
    static constexpr float FLOOR_HEIGHT = 0.5f;          // 8/16
    static constexpr float WALL_Y_MIN = 0.25f;
    static constexpr float WALL_Y_MAX = 0.75f;

    // ─── Wither spawn ───
    static constexpr int32_t WITHER_SKULLS_NEEDED = 3;
    static constexpr double WITHER_Y_OFFSET = -1.45;
    static constexpr int32_t WITHER_PARTICLES = 120;
    static constexpr double ACHIEVEMENT_RADIUS = 50.0;
}

} // namespace mccpp
