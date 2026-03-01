/**
 * PistonSystem.h — Complete piston base, extension head, and moving block.
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
 * Material: piston, hardness 0.5, soundTypePiston
 * isSticky: false (piston) or true (sticky_piston)
 * Render type 16, not opaque, not normal
 *
 * Metadata:
 *   bits 0-2: orientation (0=down,1=up,2=north,3=south,4=west,5=east)
 *   bit 3: extended (1) / retracted (0)
 *   7 = invalid orientation (skip update)
 *
 * Placement (determineOrientation):
 *   If player close (|dx|<2, |dz|<2):
 *     eyeY - blockY > 2 → up(1)
 *     blockY - eyeY > 0 → down(0)
 *   Else: yaw 0→south(2), 1→west(5), 2→north(3), 3→east(4)
 *   eyeY = posY + 1.82 - yOffset
 *
 * Power detection (isIndirectlyPowered):
 *   6 direct neighbors (skipping own face direction)
 *   + 6 positions one block above with side power
 *   = 12 total checks (BUD behavior)
 *
 * Extend (canExtend + tryExtend):
 *   13 positions checked forward, max 12 blocks pushed
 *   canPushBlock rules:
 *     Obsidian: NEVER
 *     Piston/sticky if extended: NEVER
 *     Hardness -1 (bedrock): NEVER
 *     Mobility 2 (liquids): NEVER
 *     Mobility 1 (plants): only when destroying
 *     ITileEntityProvider: NEVER
 *   tryExtend: push chain → replace with piston_extension TEs
 *   Mobility 1 blocks: drop items then air
 *
 * Retract (block event 1):
 *   Normal piston: clear head to air
 *   Sticky piston: pull block from 2 ahead
 *     Check piston_extension TE → use stored block
 *     Check pushable mobility 0 or piston → pull
 *
 * Sounds: extend "tile.piston.out" vol=0.5 pitch=rand*0.25+0.6
 *         retract "tile.piston.in" vol=0.5 pitch=rand*0.15+0.6
 *
 * Extended bounds: 3/4 block (inset 4/16 from head face)
 *
 * Block IDs: piston(33), sticky_piston(29)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PISTON HEAD (BlockPistonExtension)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: piston, hardness 0.5
 * Render type 17, cannot be placed manually, drops 0
 *
 * Metadata:
 *   bits 0-2: direction (clamped 0-5)
 *   bit 3: sticky(1) / normal(0)
 *
 * 2-part collision per direction:
 *   Head plate: 4/16 thick, full face coverage
 *   Arm: 4/16 centered (0.375-0.625) on perpendicular axes
 *
 * Break: creative removes base piston; orphan detection clears self
 * Neighbor: if no piston behind → air; else forwards neighbor change
 *
 * Block ID: piston_head(34)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PISTON MOVING (BlockPistonMoving)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, hardness -1 (unbreakable)
 * Render type -1 (invisible, TE renders)
 * Cannot be placed, no drops
 *
 * TileEntityPiston stores:
 *   storedBlock, storedMetadata, pistonOrientation,
 *   extending, shouldHeadBeRendered,
 *   progress (0.0-1.0)
 *
 * Collision: animated offset = progress × facing
 *   If extending: offset = 1.0 - progress
 *   Adjusts min/max of stored block's AABB by -facing*offset
 *
 * Break: clears TE via clearPistonTileEntity
 * Drop: delegates to stored block's drop
 *
 * Block ID: piston_extension(36)
 *
 * Thread safety: Piston operations via block events on server thread.
 */
#pragma once

#include <cstdint>
#include <array>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Facing Offset Tables (used by all piston blocks)
// ═══════════════════════════════════════════════════════════════════════════

namespace PistonFacing {
    // Facing.offsetsXForSide/Y/Z
    static constexpr int32_t X[6] = { 0,  0,  0, 0, -1, 1};
    static constexpr int32_t Y[6] = {-1,  1,  0, 0,  0, 0};
    static constexpr int32_t Z[6] = { 0,  0, -1, 1,  0, 0};

    // Facing.oppositeSide
    static constexpr int32_t OPPOSITE[6] = {1, 0, 3, 2, 5, 4};
}

// ═══════════════════════════════════════════════════════════════════════════
// Piston Base Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PistonBase {
    // ─── Block IDs ───
    static constexpr int32_t PISTON_ID = 33;
    static constexpr int32_t STICKY_PISTON_ID = 29;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 16;

    // ─── Properties ───
    static constexpr float HARDNESS = 0.5f;

    // ─── Metadata ───
    static constexpr int32_t ORIENTATION_MASK = 7;      // bits 0-2
    static constexpr int32_t EXTENDED_FLAG = 8;          // bit 3
    static constexpr int32_t INVALID_ORIENTATION = 7;

    inline int32_t getOrientation(int32_t meta) { return meta & ORIENTATION_MASK; }
    inline bool isExtended(int32_t meta) { return (meta & EXTENDED_FLAG) != 0; }

    // ─── Directions ───
    static constexpr int32_t DOWN = 0;
    static constexpr int32_t UP = 1;
    static constexpr int32_t NORTH = 2;
    static constexpr int32_t SOUTH = 3;
    static constexpr int32_t WEST = 4;
    static constexpr int32_t EAST = 5;

    // ─── Push limit ───
    static constexpr int32_t MAX_PUSH_BLOCKS = 12;
    static constexpr int32_t MAX_CHECK_POSITIONS = 13;

    // ─── Block events ───
    static constexpr int32_t EVENT_EXTEND = 0;
    static constexpr int32_t EVENT_RETRACT = 1;

    // ─── Sounds ───
    static constexpr float SOUND_VOLUME = 0.5f;
    static constexpr float EXTEND_PITCH_BASE = 0.6f;
    static constexpr float EXTEND_PITCH_RANGE = 0.25f;
    static constexpr float RETRACT_PITCH_BASE = 0.6f;
    static constexpr float RETRACT_PITCH_RANGE = 0.15f;

    // ─── Extended bounds ───
    static constexpr float EXTENDED_INSET = 0.25f;      // 4/16

    // ─── Unpushable ───
    static constexpr int32_t OBSIDIAN_ID = 49;
    //   hardness -1 = unbreakable (bedrock, end_portal_frame, etc.)
    //   mobility 2 = not pushable
    //   mobility 1 = destroyable (plants, etc.)
    //   ITileEntityProvider = not pushable
}

// ═══════════════════════════════════════════════════════════════════════════
// Piston Head Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PistonHead {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 34;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 17;

    // ─── Properties ───
    static constexpr float HARDNESS = 0.5f;

    // ─── Metadata ───
    static constexpr int32_t DIRECTION_MASK = 7;        // bits 0-2
    static constexpr int32_t STICKY_FLAG = 8;           // bit 3

    inline int32_t getDirection(int32_t meta) {
        int32_t d = meta & DIRECTION_MASK;
        return (d > 5) ? 5 : d;                         // clamp 0-5
    }

    // ─── Collision ───
    static constexpr float HEAD_THICKNESS = 0.25f;      // 4/16
    static constexpr float ARM_MIN_PERP = 0.375f;       // 6/16
    static constexpr float ARM_MAX_PERP = 0.625f;       // 10/16
    static constexpr float ARM_MIN_HORIZ = 0.25f;       // 4/16
    static constexpr float ARM_MAX_HORIZ = 0.75f;       // 12/16
}

// ═══════════════════════════════════════════════════════════════════════════
// Piston Moving Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PistonMoving {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 36;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = -1;          // invisible

    // ─── Properties ───
    static constexpr float HARDNESS = -1.0f;            // unbreakable
}

} // namespace mccpp
