/**
 * SignFlowerPotSkull.h — Sign, flower pot, and skull tile entity blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockSign (128 lines)
 *   - net.minecraft.block.BlockFlowerPot (209 lines)
 *   - net.minecraft.block.BlockSkull (232 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SIGN (BlockSign)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood, extends BlockContainer (has TileEntitySign)
 * No collision box (null), passable, not opaque, render type -1 (TESR)
 *
 * Two variants:
 *   1. Standing sign (field_149967_b = true):
 *      - Needs solid material below
 *      - Metadata 0-15: rotation (16 compass directions)
 *      - Default bounds: (0.25, 0, 0.25) → (0.75, 1, 0.75)
 *
 *   2. Wall sign (field_149967_b = false):
 *      - Metadata 2-5: wall facing (same as ladder)
 *      - Needs solid material at support wall
 *      - Wall bounds (2/16 thick panel):
 *        meta 2 (south): (0, 0.28125, 0.875) → (1, 0.78125, 1)
 *        meta 3 (north): (0, 0.28125, 0) → (1, 0.78125, 0.125)
 *        meta 4 (east):  (0.875, 0.28125, 0) → (1, 0.78125, 1)
 *        meta 5 (west):  (0, 0.28125, 0) → (0.125, 0.78125, 1)
 *
 * Drops: Items.sign (always sign item, not wall/standing block)
 * breakBlock neighbor validation checks specific support
 *
 * Block IDs: standing_sign (63), wall_sign (68)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FLOWER POT (BlockFlowerPot)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, extends BlockContainer (TileEntityFlowerPot)
 * Not opaque, not normal, render type 33
 * Needs solidTopSurface below
 *
 * Dimensions: 6/16 wide, 6/16 tall, centered
 *   Bounds: (0.3125, 0, 0.3125) → (0.6875, 0.375, 0.6875)
 *
 * Plant insertion (onBlockActivated):
 *   - Right-click with valid plant ItemBlock
 *   - Stores item+meta in TileEntityFlowerPot
 *   - Consumes 1 item (not in creative)
 *
 * Valid plants:
 *   yellow_flower, red_flower, cactus, brown_mushroom,
 *   red_mushroom, sapling, deadbush, tallgrass (fern, meta 2)
 *
 * Metadata → plant mapping (createNewTileEntity):
 *    0 = empty
 *    1 = red_flower:0
 *    2 = yellow_flower:0
 *    3 = sapling:0 (oak)
 *    4 = sapling:1 (spruce)
 *    5 = sapling:2 (birch)
 *    6 = sapling:3 (jungle)
 *    7 = red_mushroom
 *    8 = brown_mushroom
 *    9 = cactus
 *   10 = deadbush
 *   11 = tallgrass:2 (fern)
 *   12 = sapling:4 (acacia)
 *   13 = sapling:5 (dark oak)
 *
 * breakBlock: drops both pot (Items.flower_pot) + plant contents
 * Creative harvest: clears tile entity plant
 *
 * Block ID: 140
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SKULL (BlockSkull)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, extends BlockContainer (TileEntitySkull)
 * Not opaque, not normal, render type -1 (TESR)
 *
 * Skull types (stored in TileEntitySkull):
 *   0 = skeleton skull
 *   1 = wither skeleton skull
 *   2 = zombie head
 *   3 = player head (with GameProfile)
 *   4 = creeper head
 *
 * Metadata (bits 0-2, facing):
 *   1 = floor (looking up)
 *   2 = on south wall
 *   3 = on north wall
 *   4 = on east wall
 *   5 = on west wall
 *   bit 3: creative-broken flag (suppress drops)
 *
 * Bounds:
 *   Floor (default): (0.25, 0, 0.25) → (0.75, 0.5, 0.75) — 8/16 cube
 *   Wall south: (0.25, 0.25, 0.5) → (0.75, 0.75, 1)
 *   Wall north: (0.25, 0.25, 0) → (0.75, 0.75, 0.5)
 *   Wall east: (0.5, 0.25, 0.25) → (1, 0.75, 0.75)
 *   Wall west: (0, 0.25, 0.25) → (0.5, 0.75, 0.75)
 *
 * Placement: facing from yaw with +2.5 offset:
 *   (floor(yaw * 4 / 360 + 2.5) & 3)
 *
 * Drops: Items.skull with type from TileEntity
 *   Player head (type 3): includes SkullOwner NBT GameProfile
 *   Creative: sets bit 3, suppresses drops
 *
 * Wither spawn (makeWither):
 *   Requires 3 wither skeleton skulls in a row + T-shape soul sand:
 *
 *   Pattern (Z-axis or X-axis):
 *     [W] [W] [W]      ← 3 wither skulls (top row)
 *     [S] [S] [S]      ← 3 soul sand (middle row)
 *         [S]           ← 1 soul sand (center bottom)
 *
 *   Checks both horizontal axes (Z then X)
 *   Scans offset -2 to 0 for each axis
 *   Must not be peaceful difficulty
 *   Y >= 2 required
 *
 *   On match: clears T-shape blocks, spawns EntityWither
 *   Wither spawns at center, 1.45 below skull row
 *   120 snowballpoof particles, achievement trigger in 50-block radius
 *
 * Block ID: 144
 *
 * Thread safety: Block + tile entity on server thread.
 * JNI readiness: TileEntity stored data accessible.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Sign Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SignConstants {
    // ─── Block IDs ───
    static constexpr int32_t STANDING_SIGN_ID = 63;
    static constexpr int32_t WALL_SIGN_ID = 68;

    // ─── Standing sign ───
    // Metadata 0-15: 16 rotation values (22.5° each)
    static constexpr int32_t STANDING_ROTATIONS = 16;
    // Default bounds: (0.25, 0, 0.25) → (0.75, 1, 0.75)

    // ─── Wall sign ───
    // Metadata 2-5: facing (same as ladder)
    static constexpr float WALL_THICKNESS = 0.125f;  // 2/16
    static constexpr float WALL_Y_MIN = 0.28125f;     // 4.5/16
    static constexpr float WALL_Y_MAX = 0.78125f;     // 12.5/16

    struct WallBounds {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
    };

    // Indexed by (meta - 2) for metas 2-5
    static constexpr WallBounds WALL_SIGN_BOUNDS[] = {
        // meta 2 (south wall)
        {0.0f, WALL_Y_MIN, 1.0f - WALL_THICKNESS, 1.0f, WALL_Y_MAX, 1.0f},
        // meta 3 (north wall)
        {0.0f, WALL_Y_MIN, 0.0f, 1.0f, WALL_Y_MAX, WALL_THICKNESS},
        // meta 4 (east wall)
        {1.0f - WALL_THICKNESS, WALL_Y_MIN, 0.0f, 1.0f, WALL_Y_MAX, 1.0f},
        // meta 5 (west wall)
        {0.0f, WALL_Y_MIN, 0.0f, WALL_THICKNESS, WALL_Y_MAX, 1.0f},
    };

    // Support offset (same as sign facing)
    struct SupportOffset {
        int32_t dx, dz;
    };
    static constexpr SupportOffset WALL_SUPPORTS[] = {
        { 0, +1},  // meta 2
        { 0, -1},  // meta 3
        {+1,  0},  // meta 4
        {-1,  0},  // meta 5
    };
}

// ═══════════════════════════════════════════════════════════════════════════
// Flower Pot Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FlowerPotConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 140;

    // ─── Dimensions ───
    // 6/16 wide (0.375), centered, 6/16 tall
    static constexpr float POT_SIZE = 0.375f;
    static constexpr float POT_HALF = POT_SIZE / 2.0f;  // 0.1875
    // Bounds: (0.3125, 0, 0.3125) → (0.6875, 0.375, 0.6875)

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 33;

    // ─── Plant metadata mapping ───
    static constexpr int32_t POT_EMPTY = 0;
    static constexpr int32_t POT_RED_FLOWER = 1;
    static constexpr int32_t POT_YELLOW_FLOWER = 2;
    static constexpr int32_t POT_OAK_SAPLING = 3;
    static constexpr int32_t POT_SPRUCE_SAPLING = 4;
    static constexpr int32_t POT_BIRCH_SAPLING = 5;
    static constexpr int32_t POT_JUNGLE_SAPLING = 6;
    static constexpr int32_t POT_RED_MUSHROOM = 7;
    static constexpr int32_t POT_BROWN_MUSHROOM = 8;
    static constexpr int32_t POT_CACTUS = 9;
    static constexpr int32_t POT_DEADBUSH = 10;
    static constexpr int32_t POT_FERN = 11;
    static constexpr int32_t POT_ACACIA_SAPLING = 12;
    static constexpr int32_t POT_DARK_OAK_SAPLING = 13;
}

// ═══════════════════════════════════════════════════════════════════════════
// Skull Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SkullConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 144;

    // ─── Skull types ───
    static constexpr int32_t SKELETON = 0;
    static constexpr int32_t WITHER_SKELETON = 1;
    static constexpr int32_t ZOMBIE = 2;
    static constexpr int32_t PLAYER = 3;
    static constexpr int32_t CREEPER = 4;

    // ─── Metadata ───
    static constexpr int32_t FACING_MASK = 7;       // bits 0-2
    static constexpr int32_t CREATIVE_FLAG = 8;      // bit 3 (suppress drops)

    static constexpr int32_t FACING_FLOOR = 1;
    static constexpr int32_t FACING_SOUTH = 2;
    static constexpr int32_t FACING_NORTH = 3;
    static constexpr int32_t FACING_EAST = 4;
    static constexpr int32_t FACING_WEST = 5;

    // ─── Bounds ───
    struct SkullBounds {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
    };

    static constexpr SkullBounds FLOOR_BOUNDS = {0.25f, 0.0f, 0.25f, 0.75f, 0.5f, 0.75f};

    // Indexed by (meta - 2) for wall metas 2-5
    static constexpr SkullBounds WALL_BOUNDS[] = {
        {0.25f, 0.25f, 0.5f, 0.75f, 0.75f, 1.0f},   // meta 2 (south)
        {0.25f, 0.25f, 0.0f, 0.75f, 0.75f, 0.5f},   // meta 3 (north)
        {0.5f,  0.25f, 0.25f, 1.0f, 0.75f, 0.75f},   // meta 4 (east)
        {0.0f,  0.25f, 0.25f, 0.5f, 0.75f, 0.75f},   // meta 5 (west)
    };

    // ─── Placement yaw ───
    // Java: (floor(yaw * 4 / 360 + 2.5) & 3)
    inline int32_t getFacingFromYaw(float yaw) {
        return static_cast<int32_t>(std::floor(yaw * 4.0f / 360.0f + 2.5f)) & 3;
    }

    // ─── Wither spawn ───
    static constexpr int32_t WITHER_SKULL_TYPE = 1;
    static constexpr int32_t SOUL_SAND_ID = 88;
    static constexpr int32_t WITHER_HEIGHT_MIN = 2;
    static constexpr double WITHER_Y_OFFSET = -1.45;
    static constexpr int32_t WITHER_PARTICLES = 120;
    static constexpr double WITHER_ACHIEVEMENT_RADIUS = 50.0;
}

} // namespace mccpp
