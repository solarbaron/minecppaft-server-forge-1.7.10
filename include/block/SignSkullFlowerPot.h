/**
 * SignSkullFlowerPot.h — Signs, skulls, and flower pots.
 *
 * Java references:
 *   - net.minecraft.block.BlockSign (128 lines)
 *   - net.minecraft.block.BlockSkull (232 lines)
 *   - net.minecraft.block.BlockFlowerPot (209 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SIGN (BlockSign)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood, extends BlockContainer
 * TileEntitySign: stores 4 lines of text (IChatComponent[4])
 * No collision, not opaque, render type -1 (TileEntityRenderer)
 *
 * Two variants via field_149967_b:
 *   Standing sign (true): placed on ground, 16 rotation values
 *     Bounds: 4/16 square centered, full height
 *     Needs solid block below
 *   Wall sign (false): attached to wall, meta 2-5 = facing
 *     Bounds: 2/16 thick slab per wall face
 *     2(N→Z+): z 0.875-1.0, y 0.28125-0.78125
 *     3(S→Z-): z 0.0-0.125
 *     4(W→X+): x 0.875-1.0
 *     5(E→X-): x 0.0-0.125
 *     Needs solid block on attached face
 *
 * Drops: Items.sign (323)
 * isPassable = true (entities walk through)
 *
 * Support validation (onNeighborBlockChange):
 *   Standing: requires solid material below
 *   Wall: requires solid material on attached face
 *   If unsupported: drop item + setBlockToAir
 *
 * Block IDs: standing_sign(63), wall_sign(68)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SKULL (BlockSkull)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, extends BlockContainer
 * TileEntitySkull: stores skullType + rotation + GameProfile
 * No collision from pool uses bounds-based, render type -1
 *
 * 5 skull types (TileEntity skullType field):
 *   0: skeleton, 1: wither_skeleton, 2: zombie
 *   3: player (stores GameProfile → NBT SkullOwner)
 *   4: creeper
 *
 * Metadata: bits 0-2 = wall facing direction
 *   1(floor): 8/16 centered, 0-8/16 Y
 *   2(N wall): 4/16-12/16 XZ, 4/16-12/16 Y, 8/16-16/16 Z
 *   3(S wall): z 0-0.5
 *   4(W wall): x 0.5-1.0
 *   5(E wall): x 0-0.5
 *   bit 3: suppress drop (creative harvest)
 *
 * Rotation: TileEntitySkull stores fine rotation 0-15
 * Placement: yaw formula (yaw*4/360 + 2.5) & 3
 *
 * Drop: Items.skull with damage = skullType
 *   Player skull: NBT tag "SkullOwner" with GameProfile
 *   Creative harvest (bit 3): suppress drop entirely
 *
 * Wither summoning (makeWither):
 *   Requires 3 wither skeleton skulls in a row + T-shape soul sand
 *   Pattern (two orientations):
 *     Z-axis: 3 skulls at (x, y, z+0..2), soul sand T below
 *     X-axis: 3 skulls at (x+0..2, y, z), soul sand T below
 *   Soul sand layout:
 *     Row of 3 at Y-1, center column at Y-2
 *   Requirements:
 *     Not peaceful difficulty
 *     Y >= 2
 *     Not client side
 *   Result:
 *     Clear all 7 blocks (3 skulls + 4 soul sand)
 *     Spawn EntityWither at center Y-1.45
 *     120 "snowballpoof" particles
 *     Achievement: spawnWither (50 block radius)
 *
 * Block ID: skull(144)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FLOWER POT (BlockFlowerPot)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, extends BlockContainer
 * TileEntityFlowerPot: stores item + metadata of planted flower
 * Render type 33, not opaque
 *
 * Bounds: 6/16 centered (0.3125 to 0.6875), 6/16 tall
 *
 * Accepted plants (func_149928_a):
 *   yellow_flower, red_flower, cactus
 *   brown_mushroom, red_mushroom, sapling, deadbush
 *   tallgrass (only fern, damage 2)
 *
 * Metadata→plant mapping (createNewTileEntity):
 *   0: empty, 1: red_flower, 2: yellow_flower
 *   3-6: sapling(0-3), 12: sapling(4), 13: sapling(5)
 *   7: red_mushroom, 8: brown_mushroom
 *   9: cactus, 10: deadbush
 *   11: tallgrass(fern)
 *
 * Interaction (right-click):
 *   Must hold ItemBlock
 *   Must be empty pot (getFlowerPotItem == null)
 *   Must be accepted plant
 *   Sets TileEntity item+data, updates meta, consumes item
 *
 * Break: drops flower_pot item + contained plant as separate drop
 * Creative harvest: clears TileEntity (no plant drop)
 * Placement: requires solid top surface below
 *
 * Block ID: flower_pot(140)
 *
 * Thread safety: TileEntity access on server thread.
 * JNI readiness: Sign/skull/pot for Forge display.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Sign Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SignConstants {
    // ─── Block IDs ───
    static constexpr int32_t STANDING_SIGN_ID = 63;
    static constexpr int32_t WALL_SIGN_ID = 68;

    // ─── Item ───
    static constexpr int32_t SIGN_ITEM_ID = 323;

    // ─── Standing rotation ───
    static constexpr int32_t ROTATION_COUNT = 16;    // 22.5° per step

    // ─── Wall bounds ───
    static constexpr float WALL_THICKNESS = 0.125f;   // 2/16
    static constexpr float WALL_Y_MIN = 0.28125f;     // 4.5/16
    static constexpr float WALL_Y_MAX = 0.78125f;     // 12.5/16

    // ─── Standing bounds ───
    static constexpr float STAND_HALF_WIDTH = 0.25f;   // 4/16
}

// ═══════════════════════════════════════════════════════════════════════════
// Skull Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SkullConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 144;

    // ─── Item ───
    static constexpr int32_t SKULL_ITEM_ID = 397;

    // ─── Skull types ───
    static constexpr int32_t TYPE_SKELETON = 0;
    static constexpr int32_t TYPE_WITHER_SKELETON = 1;
    static constexpr int32_t TYPE_ZOMBIE = 2;
    static constexpr int32_t TYPE_PLAYER = 3;
    static constexpr int32_t TYPE_CREEPER = 4;

    // ─── Metadata ───
    static constexpr int32_t FACING_MASK = 7;         // bits 0-2
    static constexpr int32_t CREATIVE_FLAG = 8;       // bit 3 suppress drop

    // ─── Facing ───
    static constexpr int32_t FACE_FLOOR = 1;
    static constexpr int32_t FACE_NORTH = 2;
    static constexpr int32_t FACE_SOUTH = 3;
    static constexpr int32_t FACE_WEST = 4;
    static constexpr int32_t FACE_EAST = 5;

    // ─── Bounds ───
    static constexpr float SKULL_SIZE = 0.25f;         // 4/16 half-width
    static constexpr float FLOOR_HEIGHT = 0.5f;       // 8/16
    static constexpr float WALL_HEIGHT_MIN = 0.25f;
    static constexpr float WALL_HEIGHT_MAX = 0.75f;

    // ─── Fine rotation ───
    static constexpr int32_t ROTATION_COUNT = 16;      // stored in TileEntity

    // ─── Wither summoning ───
    static constexpr int32_t WITHER_SKULL_TYPE = 1;
    static constexpr int32_t SOUL_SAND_ID = 88;
    static constexpr int32_t WITHER_PARTICLES = 120;
    static constexpr double WITHER_Y_OFFSET = -1.45;
    static constexpr float WITHER_ACHIEVE_RADIUS = 50.0f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Flower Pot Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FlowerPotConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 140;

    // ─── Item ───
    static constexpr int32_t FLOWER_POT_ITEM_ID = 390;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 33;

    // ─── Bounds ───
    static constexpr float POT_SIZE = 0.375f;          // 6/16
    static constexpr float POT_HALF = 0.1875f;         // 3/16
    // Center: (0.5-0.1875) to (0.5+0.1875) = 0.3125 to 0.6875

    // ─── Accepted plant block IDs ───
    static constexpr int32_t YELLOW_FLOWER_ID = 37;
    static constexpr int32_t RED_FLOWER_ID = 38;
    static constexpr int32_t CACTUS_ID = 81;
    static constexpr int32_t BROWN_MUSHROOM_ID = 39;
    static constexpr int32_t RED_MUSHROOM_ID = 40;
    static constexpr int32_t SAPLING_ID = 6;
    static constexpr int32_t DEADBUSH_ID = 32;
    static constexpr int32_t TALLGRASS_ID = 31;
    static constexpr int32_t FERN_DAMAGE = 2;

    // ─── Metadata → plant mapping ───
    // 0=empty, 1=red_flower(0), 2=yellow_flower
    // 3=sapling(0), 4=sapling(1), 5=sapling(2), 6=sapling(3)
    // 7=red_mushroom, 8=brown_mushroom
    // 9=cactus, 10=deadbush, 11=tallgrass(2)
    // 12=sapling(4), 13=sapling(5)
}

} // namespace mccpp
