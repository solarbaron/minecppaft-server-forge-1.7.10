/**
 * PistonBaseHeadMoving.h — Piston base, extension head, and moving block.
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
 * Material: piston, hardness 0.5, render type 16
 * isSticky: normal(false) or sticky(true)
 * Not opaque, not normal
 *
 * Metadata:
 *   bits 0-2: orientation (0-5, 6 faces)
 *   bit 3: extended (1) / retracted (0)
 *
 * Orientation (determineOrientation):
 *   Close range (abs(dX) < 2, abs(dZ) < 2):
 *     eyeHeight = posY + 1.82 - yOffset
 *     eye - blockY > 2.0 → face up (1)
 *     blockY - eye > 0.0 → face down (0)
 *   Far range: yaw → 0→2, 1→5, 2→3, 3→4
 *
 * Power detection (isIndirectlyPowered):
 *   6 direct checks: each face except own orientation
 *   6 quasi-connectivity checks: Y+0→side 0, Y+2→side 1,
 *     (Y+1, Z-1)→2, (Y+1, Z+1)→3, (X-1, Y+1)→4, (X+1, Y+1)→5
 *
 * Push logic:
 *   Max push: 12 blocks, scan 13 ahead
 *   canPushBlock exclusions:
 *     Obsidian → never
 *     Extended piston → never
 *     Hardness -1 → never
 *     Mobility 2 → never
 *     Mobility 1 → only on extend (destroy)
 *     ITileEntityProvider → never
 *
 *   tryExtend: walk blocks, back-propagate to piston_extension TEs
 *   Notify neighbors in reverse order after placement
 *
 * Block events: 0=extend, 1=retract
 *   Extend: metadata |= 8, play "tile.piston.out"
 *   Retract: sticky pulls 2-ahead if mobility 0 or piston
 *   Sounds: vol=0.5, extend pitch=rand*0.25+0.6, retract=rand*0.15+0.6
 *
 * Extended bounds: 12/16 body (4/16 removed on arm side)
 *
 * Block IDs: piston(33), sticky_piston(29)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PISTON HEAD (BlockPistonExtension)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: piston, hardness 0.5, render type 17
 * Not opaque, not normal, cannot be placed, drops 0
 *
 * Meta: bits 0-2 direction (clamped 0..5), bit 3 sticky
 *
 * Compound collision per direction:
 *   Plate (4/16): 0→Y 0-0.25, 1→Y 0.75-1.0, 2→Z 0-0.25,
 *     3→Z 0.75-1.0, 4→X 0-0.25, 5→X 0.75-1.0
 *   Arm rod: vert(0/1) XZ 0.375-0.625, horiz(2-5) 0.25-0.75 cross
 *
 * breakBlock/neighborChange: orphan→removes base/self
 *
 * Block ID: piston_head(34)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PISTON MOVING (BlockPistonMoving)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * BlockContainer, material: piston, hardness -1, render -1
 * TileEntity: TileEntityPiston(block, meta, dir, extending, source)
 *
 * Animated collision: stored block bounds offset by Facing.offsets × progress
 * Right-click + no TE → air. dropBlock → delegates to stored block
 *
 * Block ID: piston_extension(36)
 *
 * Thread safety: Block events and TE access on server thread.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Piston Base Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PistonBaseDef {
    static constexpr int32_t NORMAL_ID = 33;
    static constexpr int32_t STICKY_ID = 29;
    static constexpr int32_t RENDER_TYPE = 16;
    static constexpr float HARDNESS = 0.5f;

    // Meta
    static constexpr int32_t DIR_MASK = 7;               // bits 0-2
    static constexpr int32_t EXTENDED_FLAG = 8;          // bit 3
    static constexpr int32_t INVALID_DIR = 7;

    // Push
    static constexpr int32_t MAX_PUSH = 12;
    static constexpr int32_t SCAN_LIMIT = 13;

    // Orientation
    static constexpr float CLOSE_RANGE = 2.0f;
    static constexpr double EYE_HEIGHT = 1.82;

    // Events
    static constexpr int32_t EVENT_EXTEND = 0;
    static constexpr int32_t EVENT_RETRACT = 1;

    // Sounds
    static constexpr float SOUND_VOL = 0.5f;
    static constexpr float EXTEND_PITCH_BASE = 0.6f;
    static constexpr float EXTEND_PITCH_RNG = 0.25f;
    static constexpr float RETRACT_PITCH_RNG = 0.15f;

    // Extended body
    static constexpr float BODY_INSET = 0.25f;           // 4/16 removed
}

// ═══════════════════════════════════════════════════════════════════════════
// Piston Head Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PistonHeadDef {
    static constexpr int32_t BLOCK_ID = 34;
    static constexpr int32_t RENDER_TYPE = 17;
    static constexpr int32_t DIR_MASK = 7;
    static constexpr int32_t STICKY_FLAG = 8;

    // Plate: 4/16 thick
    static constexpr float PLATE = 0.25f;

    // Arm: narrow 6/16-10/16, wide 4/16-12/16
    static constexpr float ARM_N_MIN = 0.375f;
    static constexpr float ARM_N_MAX = 0.625f;
    static constexpr float ARM_W_MIN = 0.25f;
    static constexpr float ARM_W_MAX = 0.75f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Piston Moving Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PistonMovingDef {
    static constexpr int32_t BLOCK_ID = 36;
    static constexpr int32_t RENDER_TYPE = -1;
    static constexpr float HARDNESS = -1.0f;
}

} // namespace mccpp
