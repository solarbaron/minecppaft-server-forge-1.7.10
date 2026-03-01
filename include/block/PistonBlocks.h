/**
 * PistonBlocks.h — Piston base, extension head, and moving block.
 *
 * Java references:
 *   - net.minecraft.block.BlockPistonBase (394 lines)
 *   - net.minecraft.block.BlockPistonExtension (180 lines)
 *   - net.minecraft.block.BlockPistonMoving (178 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PISTON BASE (BlockPistonBase)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: piston, hardness 0.5, sticky or normal variant
 * Not opaque, render type 16
 *
 * Metadata:
 *   bits 0-2: orientation (0-5 = D/U/N/S/W/E)
 *   bit 3: extended flag
 *
 * Orientation (determineOrientation):
 *   If player within 2 blocks XZ:
 *     eyeHeight = posY + 1.82 - yOffset
 *     Looking up (eye - blockY > 2) → up (1)
 *     Looking down (blockY - eye > 0) → down (0)
 *   Else: yaw-based: 0→2(N), 1→5(E), 2→3(S), 3→4(W)
 *
 * Power detection (isIndirectlyPowered):
 *   12 positions checked:
 *   6 direct faces (skip own direction)
 *   + 6 positions one block above (BUD mechanics)
 *
 * canPushBlock rules:
 *   Cannot push: obsidian, hardness -1, mobility 2, TileEntities
 *   Pistons: can push if not extended
 *   Mobility 1: only when extending (not during canExtend check)
 *
 * canExtend: scan up to 13 blocks in push direction
 *   Stop at: air, mobility 1 (destroyable), or fail at 13
 *   Fail at: world bounds, unpushable block
 *
 * tryExtend:
 *   Scans forward, destroys mobility-1 blocks
 *   Creates piston_extension chain from far end back to base
 *   Each block → piston_extension with TileEntityPiston
 *   Base position → piston_head extension
 *   Then notifies all pushed positions
 *
 * Retraction (event type 1):
 *   Clear extension tile entity
 *   Place piston_extension at base with own orientation
 *   Sticky: check 2 blocks ahead
 *     If extending TileEntityPiston → grab stored block
 *     If pushable block (mobility 0 or piston) → pull back
 *   Non-sticky: clear extension space
 *
 * Extended bounds: 0.25 inset on push face (3/4 block body)
 *
 * Sounds:
 *   Extend: "tile.piston.out", 0.5, rand*0.25+0.6
 *   Retract: "tile.piston.in", 0.5, rand*0.15+0.6
 *
 * Block IDs: piston(33), sticky_piston(29)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PISTON EXTENSION / HEAD (BlockPistonExtension)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: piston, hardness 0.5, render type 17
 * Not opaque, not normal, cannot be placed by player
 * No drops (quantityDropped = 0)
 *
 * Collision: 2-part per face:
 *   Down(0): plate 0-0.25 Y + arm 0.375-0.625 XZ center up
 *   Up(1): plate 0.75-1.0 Y + arm center down
 *   North(2): plate 0-0.25 Z + arm center south
 *   South(3): plate 0.75-1.0 Z + arm center north
 *   West(4): plate 0-0.25 X + arm center east
 *   East(5): plate 0.75-1.0 X + arm center west
 *
 * Harvest: creative mode removes piston base behind
 * Break: removes extended piston base behind (drops it)
 * Neighbor: if no piston/sticky_piston behind → remove self
 *
 * getDirectionMeta: (meta & 7) clamped 0-5
 *
 * Block ID: piston_head (34)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PISTON MOVING (BlockPistonMoving)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: piston, extends BlockContainer (has TileEntity)
 * Hardness: -1.0 (unbreakable, but not literally — special handling)
 * Render type: -1 (invisible, rendered by TileEntityPiston)
 *
 * Cannot be placed by player
 * No drops (getItemDropped = null)
 *
 * getTileEntity static factory:
 *   Creates TileEntityPiston(block, meta, orientation, extending, isSource)
 *
 * Collision: animated based on TileEntityPiston progress
 *   progress = func_145860_a(0) — interpolated position
 *   Extending: 1.0 - progress (approaching final position)
 *   Collision AABB offset by facing * progress along push axis
 *
 * breakBlock: if TileEntityPiston → clearPistonTileEntity
 * dropBlockAsItemWithChance: drops stored block from TileEntity
 * onBlockActivated: if no TE → remove (cleanup)
 *
 * Block ID: piston_extension (36)
 *
 * Thread safety: Piston events via block event queue (server thread).
 * JNI readiness: TileEntityPiston for Forge piston events.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Piston Base Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PistonBaseConstants {
    // ─── Block IDs ───
    static constexpr int32_t PISTON_ID = 33;
    static constexpr int32_t STICKY_PISTON_ID = 29;

    // ─── Properties ───
    static constexpr float HARDNESS = 0.5f;

    // ─── Metadata ───
    static constexpr int32_t ORIENTATION_MASK = 7;   // bits 0-2
    static constexpr int32_t EXTENDED_FLAG = 8;       // bit 3

    // Orientation values:
    static constexpr int32_t DIR_DOWN = 0;
    static constexpr int32_t DIR_UP = 1;
    static constexpr int32_t DIR_NORTH = 2;
    static constexpr int32_t DIR_SOUTH = 3;
    static constexpr int32_t DIR_WEST = 4;
    static constexpr int32_t DIR_EAST = 5;
    static constexpr int32_t DIR_INVALID = 7;

    inline int32_t getOrientation(int32_t meta) { return meta & ORIENTATION_MASK; }
    inline bool isExtended(int32_t meta) { return (meta & EXTENDED_FLAG) != 0; }

    // ─── Push limits ───
    static constexpr int32_t MAX_PUSH_BLOCKS = 12;   // 0-12 = 13 iterations
    static constexpr int32_t MAX_SCAN_BLOCKS = 13;    // scan limit

    // ─── Orientation determination ───
    static constexpr float CLOSE_RANGE = 2.0f;    // XZ distance threshold
    static constexpr double EYE_HEIGHT = 1.82;
    static constexpr double UP_THRESHOLD = 2.0;    // eye - blockY > 2 → up
    // blockY - eye > 0 → down

    // Yaw → orientation: 0→2(N), 1→5(E), 2→3(S), 3→4(W)
    static constexpr int32_t YAW_TO_ORIENTATION[4] = {2, 5, 3, 4};

    // ─── Power check positions ───
    // 6 direct faces (skip own direction) + 6 BUD positions (one block above)

    // ─── canPushBlock ───
    static constexpr int32_t OBSIDIAN_ID = 49;
    // Reject: obsidian, hardness -1, mobility 2, TileEntity
    // Pistons: pushable only if not extended
    // Mobility 1: destroyable (push only, not canExtend)

    // ─── Extended bounds ───
    static constexpr float RETRACTED_INSET = 0.25f;  // 1/4 block inset per face

    // ─── Sounds ───
    // Extend: "tile.piston.out", vol=0.5, pitch=rand*0.25+0.6
    // Retract: "tile.piston.in", vol=0.5, pitch=rand*0.15+0.6
    static constexpr float SOUND_VOLUME = 0.5f;
    static constexpr float EXTEND_PITCH_BASE = 0.6f;
    static constexpr float EXTEND_PITCH_RANGE = 0.25f;
    static constexpr float RETRACT_PITCH_BASE = 0.6f;
    static constexpr float RETRACT_PITCH_RANGE = 0.15f;

    // ─── Facing offsets ───
    // Facing.offsetsXForSide[6] = {0, 0, 0, 0, -1, 1}
    // Facing.offsetsYForSide[6] = {-1, 1, 0, 0, 0, 0}
    // Facing.offsetsZForSide[6] = {0, 0, -1, 1, 0, 0}
    static constexpr int32_t FACING_X[6] = {0, 0, 0, 0, -1, 1};
    static constexpr int32_t FACING_Y[6] = {-1, 1, 0, 0, 0, 0};
    static constexpr int32_t FACING_Z[6] = {0, 0, -1, 1, 0, 0};
    static constexpr int32_t OPPOSITE[6] = {1, 0, 3, 2, 5, 4};
}

// ═══════════════════════════════════════════════════════════════════════════
// Piston Extension (Head) Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PistonHeadConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 34;   // piston_head

    // ─── Properties ───
    static constexpr float HARDNESS = 0.5f;
    static constexpr int32_t RENDER_TYPE = 17;

    // ─── Collision plate dimensions ───
    static constexpr float PLATE_THICKNESS = 0.25f;   // 4/16
    static constexpr float ARM_MIN = 0.375f;           // 6/16
    static constexpr float ARM_MAX = 0.625f;           // 10/16

    // ─── Meta ───
    // bits 0-2: direction (clamped 0-5)
    // bit 3: sticky flag (sticky piston head)
    static constexpr int32_t STICKY_FLAG = 8;

    inline int32_t getDirection(int32_t meta) {
        int32_t dir = meta & 7;
        return dir > 5 ? 5 : dir;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Piston Moving (Animated Block) Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PistonMovingConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 36;   // piston_extension (moving block)

    // ─── Properties ───
    static constexpr float HARDNESS = -1.0f;  // unbreakable (special)
    static constexpr int32_t RENDER_TYPE = -1; // invisible (TE renders)

    // ─── TileEntityPiston ───
    // Stores: storedBlock, storedMeta, orientation, extending, isSource
    // Progress: 0.0→1.0 over 2 ticks (0.5 per tick)
    static constexpr float PROGRESS_PER_TICK = 0.5f;
    static constexpr float PROGRESS_MAX = 1.0f;
}

} // namespace mccpp
