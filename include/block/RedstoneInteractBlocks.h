/**
 * RedstoneInteractBlocks.h — Note block, jukebox, TNT, lever, and button.
 *
 * Java references:
 *   - net.minecraft.block.BlockNote (85 lines)
 *   - net.minecraft.block.BlockJukebox (104 lines)
 *   - net.minecraft.block.BlockTNT (100 lines)
 *   - net.minecraft.block.BlockLever (315 lines)
 *   - net.minecraft.block.BlockButton (309 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * NOTE BLOCK (BlockNote)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, material: wood
 * TileEntity: TileEntityNote (stores note 0-24, previousRedstoneState)
 *
 * Redstone: edge-triggered (rising edge only)
 *   Tracks previousRedstoneState in TE
 *   Only triggers on false→true transition
 *
 * Interaction:
 *   Right-click: changePitch() then triggerNote()
 *   Left-click: triggerNote() only
 *
 * Sound (onBlockEventReceived):
 *   Pitch: 2^((note-12)/12.0)
 *   5 instruments by block event ID:
 *     0=harp, 1=bd, 2=snare, 3=hat, 4=bassattack
 *   Sound: "note." + instrument
 *   Volume: 3.0
 *   Position: block center (x+0.5, y+0.5, z+0.5)
 *
 * Particle: "note" at (x+0.5, y+1.2, z+0.5), param = note/24.0
 *
 * Block ID: noteblock(25)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * JUKEBOX (BlockJukebox)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, material: wood
 * TileEntity: TileEntityJukebox (stores 1 ItemStack)
 *
 * Metadata: 0=empty, 1=has disc
 *
 * Insert (func_149926_b): copy item to TE, set meta 1
 * Eject (func_149925_e):
 *   Play: playAuxSFX(1005) + playRecord(null)
 *   Clear TE disc, set meta 0
 *   Drop entity: scatter 0.7 XZ, Y base 0.6+0.2, pickup delay 10
 *
 * Right-click: if meta=1, eject; if meta=0, return false (use item)
 * Break: eject disc first
 *
 * Comparator: itemID + 1 - record_13 ID
 *   (outputs 1-12 for disc types)
 *
 * Block ID: jukebox(84)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TNT (BlockTNT)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: tnt
 *
 * 4 ignition sources:
 *   1. Redstone: onBlockAdded/onNeighborBlockChange + powered → ignite
 *   2. Explosion: fuse = rand(fuse/4) + fuse/8
 *   3. Flint & steel: right-click → ignite + damage item
 *   4. Burning arrow: entity collision check
 *
 * Ignition (func_150114_a):
 *   Only if meta bit 0 set
 *   Spawn EntityTNTPrimed at center (+0.5, +0.5, +0.5)
 *   Sound: "game.tnt.primed" vol=1.0 pitch=1.0
 *
 * canDropFromExplosion: false (chain reaction instead)
 *
 * Block ID: tnt(46)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LEVER (BlockLever)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, render type 12
 * No collision AABB
 * canProvidePower: true
 *
 * Metadata:
 *   bits 0-2: orientation (8 values with 2 ceiling + 2 floor variants)
 *   bit 3: powered (1) / off (0)
 *
 * 8 orientations:
 *   0: ceiling (NS axis), 7: ceiling (EW axis)
 *   5: floor (NS axis), 6: floor (EW axis)
 *   1: west wall, 2: east wall, 3: north wall, 4: south wall
 *
 * invertMetadata: 0→0, 1→5, 2→4, 3→3, 4→2, 5→1
 *
 * Placement: side → orientation, floor/ceiling adds yaw for axis
 *   Floor: yaw bit 0 → 5(NS) or 6(EW)
 *   Ceiling: yaw bit 0 → 7(EW) or 0(NS)
 *
 * Toggle (onBlockActivated):
 *   XOR bit 3
 *   Sound: "random.click" vol=0.3, pitch=0.6(on)/0.5(off)
 *   Notify self + attached block
 *
 * Strong power: 15 per orientation→side mapping:
 *   0,7→side 0(down), 5,6→side 1(up)
 *   4→side 2(north), 3→side 3(south)
 *   2→side 4(west), 1→side 5(east)
 *
 * Bounds per orientation:
 *   Wall: 3/16 thick, centered 6/16 wide, Y 0.2-0.8
 *   Floor: centered 8/16 wide, Y 0-0.6
 *   Ceiling: centered 8/16 wide, Y 0.4-1.0
 *
 * Block ID: lever(69)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BUTTON (BlockButton)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Abstract. Material: circuits
 * No collision AABB
 * canProvidePower: true
 * wooden: true/false (determines tick rate and arrow activation)
 *
 * Tick rate: wooden=30, stone=20
 *
 * Metadata:
 *   bits 0-2: orientation (1=west, 2=east, 3=north, 4=south, 5=up)
 *   bit 3: pressed (1) / released (0)
 *
 * 4 wall orientations only (no floor/ceiling):
 *   Wall placement requires normalCube behind
 *   findSolidSide: checks -X, +X, -Z, +Z (default 1)
 *
 * Press (onBlockActivated):
 *   Already pressed → true (no action)
 *   Set bit 3, schedule tick
 *   Sound: "random.click" vol=0.3, pitch=0.6
 *
 * Release (updateTick):
 *   Stone: immediate release, sound pitch=0.5
 *   Wooden: activateButton check for arrows
 *
 * Arrow activation (wooden only):
 *   Checks EntityArrow within button AABB
 *   Found + not pressed → press + schedule tick
 *   Not found + pressed → release
 *   Found → re-schedule tick
 *
 * Bounds (setBlockBoundsFromMeta):
 *   6/16 × 4/16 face, depth 2/16 (released) or 1/16 (pressed)
 *   Centered on attached face
 *
 * Strong power: 15 per direction mapping:
 *   5→side 1(up), 4→side 2(north), 3→side 3(south)
 *   2→side 4(west), 1→side 5(east)
 *
 * Block IDs: stone_button(77), wooden_button(143)
 *
 * Thread safety: Block events on server thread.
 * JNI readiness: Redstone events for Forge.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Note Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace NoteBlockConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 25;

    // ─── Pitch formula ───
    // pitch = 2.0 ^ ((note - 12) / 12.0)
    static constexpr int32_t MAX_NOTE = 24;
    static constexpr int32_t PITCH_CENTER = 12;

    inline float getPitch(int32_t note) {
        return static_cast<float>(std::pow(2.0, static_cast<double>(note - PITCH_CENTER) / 12.0));
    }

    // ─── Instruments ───
    static constexpr int32_t HARP = 0;
    static constexpr int32_t BASS_DRUM = 1;
    static constexpr int32_t SNARE = 2;
    static constexpr int32_t HAT = 3;
    static constexpr int32_t BASS_ATTACK = 4;

    // ─── Sound ───
    static constexpr float SOUND_VOLUME = 3.0f;

    // ─── Particle ───
    static constexpr double PARTICLE_Y_OFFSET = 1.2;
    static constexpr double NOTE_DIVISOR = 24.0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Jukebox Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace JukeboxConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 84;

    // ─── Metadata ───
    static constexpr int32_t EMPTY = 0;
    static constexpr int32_t HAS_DISC = 1;

    // ─── Eject ───
    static constexpr int32_t EJECT_SOUND = 1005;
    static constexpr int32_t PICKUP_DELAY = 10;
    static constexpr float SCATTER_RANGE = 0.7f;
    static constexpr double Y_BASE = 0.6;
    static constexpr double Y_OFFSET = 0.2;

    // ─── Comparator ───
    // output = Item.getIdFromItem(disc) + 1 - Item.getIdFromItem(record_13)
    static constexpr int32_t RECORD_13_ID = 2256;       // Items.record_13
}

// ═══════════════════════════════════════════════════════════════════════════
// TNT Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TNTConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 46;

    // ─── Ignition meta check ───
    static constexpr int32_t IGNITE_BIT = 1;             // (meta & 1) == 1

    // ─── Spawn offset ───
    static constexpr float SPAWN_OFFSET = 0.5f;

    // ─── Explosion chain fuse ───
    // fuse = rand(fuse/4) + fuse/8

    // ─── Sound ───
    // "game.tnt.primed" vol=1.0, pitch=1.0
}

// ═══════════════════════════════════════════════════════════════════════════
// Lever Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LeverConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 69;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 12;

    // ─── Metadata ───
    static constexpr int32_t ORIENTATION_MASK = 7;       // bits 0-2
    static constexpr int32_t POWERED_FLAG = 8;           // bit 3

    // ─── 8 orientations ───
    static constexpr int32_t CEILING_NS = 0;
    static constexpr int32_t WALL_WEST = 1;
    static constexpr int32_t WALL_EAST = 2;
    static constexpr int32_t WALL_NORTH = 3;
    static constexpr int32_t WALL_SOUTH = 4;
    static constexpr int32_t FLOOR_NS = 5;
    static constexpr int32_t FLOOR_EW = 6;
    static constexpr int32_t CEILING_EW = 7;

    // ─── invertMetadata ───
    static constexpr int32_t INVERT[6] = {0, 5, 4, 3, 2, 1};

    // ─── Strong power side mapping ───
    // 0,7→0(down), 5,6→1(up), 4→2, 3→3, 2→4, 1→5

    // ─── Bounds ───
    static constexpr float WALL_THICKNESS = 0.1875f;    // 3/16
    static constexpr float FLOOR_CEIL_SIZE = 0.25f;     // 4/16 half-width
    static constexpr float WALL_Y_MIN = 0.2f;
    static constexpr float WALL_Y_MAX = 0.8f;
    static constexpr float FLOOR_HEIGHT = 0.6f;
    static constexpr float CEIL_Y_MIN = 0.4f;

    // ─── Sound ───
    static constexpr float CLICK_VOLUME = 0.3f;
    static constexpr float CLICK_PITCH_ON = 0.6f;
    static constexpr float CLICK_PITCH_OFF = 0.5f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Button Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace ButtonConstants {
    // ─── Block IDs ───
    static constexpr int32_t STONE_BUTTON_ID = 77;
    static constexpr int32_t WOODEN_BUTTON_ID = 143;

    // ─── Tick rates ───
    static constexpr int32_t WOODEN_TICK = 30;
    static constexpr int32_t STONE_TICK = 20;

    // ─── Metadata ───
    static constexpr int32_t DIRECTION_MASK = 7;         // bits 0-2
    static constexpr int32_t PRESSED_FLAG = 8;           // bit 3

    // ─── 4 wall orientations ───
    static constexpr int32_t WEST = 1;
    static constexpr int32_t EAST = 2;
    static constexpr int32_t NORTH = 3;
    static constexpr int32_t SOUTH = 4;

    // ─── Strong power side mapping ───
    // 5→1(up), 4→2, 3→3, 2→4, 1→5

    // ─── Bounds ───
    static constexpr float FACE_WIDTH = 0.1875f;        // 6/16 (half = 3/16)
    static constexpr float FACE_HEIGHT_MIN = 0.375f;    // 6/16
    static constexpr float FACE_HEIGHT_MAX = 0.625f;    // 10/16
    static constexpr float DEPTH_RELEASED = 0.125f;     // 2/16
    static constexpr float DEPTH_PRESSED = 0.0625f;     // 1/16

    // ─── Sound ───
    static constexpr float CLICK_VOLUME = 0.3f;
    static constexpr float CLICK_PITCH_ON = 0.6f;
    static constexpr float CLICK_PITCH_OFF = 0.5f;
}

} // namespace mccpp
