/**
 * TNTNoteJukebox.h — TNT, Note Block, and Jukebox.
 *
 * Java references:
 *   - net.minecraft.block.BlockTNT (100 lines)
 *   - net.minecraft.block.BlockNote (85 lines)
 *   - net.minecraft.block.BlockJukebox (104 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TNT (BlockTNT)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: tnt, creative tab: redstone
 * canDropFromExplosion = false
 *
 * Activation triggers (all set meta bit 0 = 1 for priming):
 *   1. Redstone power (onBlockAdded or onNeighborBlockChange)
 *      isBlockIndirectlyGettingPowered → prime + setBlockToAir
 *   2. Flint & steel (onBlockActivated)
 *      Damages item by 1, primes with player as placer
 *   3. Explosion (onBlockDestroyedByExplosion)
 *      Short fuse: rand(fuse/4) + fuse/8
 *      Inherits explosion placer as Entity attacker
 *   4. Flaming arrow (onEntityCollidedWithBlock)
 *      EntityArrow.isBurning → prime, shootingEntity as placer
 *
 * Priming (func_150114_a):
 *   If (meta & 1) == 1 AND server side:
 *     Spawn EntityTNTPrimed at (x+0.5, y+0.5, z+0.5)
 *     Sound: "game.tnt.primed" vol 1.0 pitch 1.0
 *
 * EntityTNTPrimed:
 *   Default fuse: 80 ticks (4 seconds)
 *   Explosion power: 4.0
 *
 * Block ID: tnt (46)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * NOTE BLOCK (BlockNote)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood, extends BlockContainer
 * TileEntityNote: stores note (0-24) and previousRedstoneState
 *
 * Redstone (onNeighborBlockChange):
 *   Edge detection: triggers only on rising edge
 *   previousRedstoneState tracks last power state
 *   Powered + was unpowered → triggerNote
 *
 * Interaction:
 *   Right-click → changePitch (increment note mod 25) + play
 *   Left-click → play current note
 *
 * Instruments (blockEvent n4):
 *   0: harp (default), 1: bd (bass drum), 2: snare
 *   3: hat (hi-hat), 4: bassattack (bass guitar)
 *   Determined by block below in TileEntityNote.triggerNote
 *
 * Pitch formula: pow(2.0, (note - 12) / 12.0)
 *   note 0 = F#3, note 12 = F#4, note 24 = F#5
 *
 * Sound: "note.{instrument}" volume 3.0, pitch from formula
 * Particle: "note" at (x+0.5, y+1.2, z+0.5), data = note/24.0
 *
 * Block ID: noteblock (25)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * JUKEBOX (BlockJukebox)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood, extends BlockContainer
 * TileEntityJukebox: stores one disc ItemStack
 *
 * Metadata: 0 = empty, 1 = has disc
 *
 * Insert disc (func_149926_b):
 *   Set TileEntity record to stack.copy()
 *   Set meta to 1
 *
 * Eject disc (func_149925_e):
 *   Get ItemStack from TileEntity
 *   If not null:
 *     SFX 1005 (stop record)
 *     playRecord(null) to stop music
 *     Clear TileEntity record
 *     Set meta to 0
 *     Spawn EntityItem:
 *       Position: random 0.7 range offset per axis
 *       Y offset: rand*0.7 + 0.15 + 0.6
 *       delayBeforeCanPickup = 10
 *
 * Interaction:
 *   Right-click on meta 0 → return false (item handler inserts)
 *   Right-click on meta 1 → eject disc + return true
 *
 * breakBlock: ejects disc before breaking
 *
 * Comparator override:
 *   Signal = itemId + 1 - record_13_id
 *   record_13 = 2256, so signal = itemId - 2255
 *   13→1, cat→2, blocks→3, chirp→4 ... wait→12
 *   Empty → 0
 *
 * Block ID: jukebox (84)
 *
 * Thread safety: TileEntity access on server thread.
 * JNI readiness: Record/note events for Forge sound system.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// TNT Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TNTConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 46;

    // ─── Priming ───
    static constexpr int32_t PRIMED_FLAG = 1;     // meta bit 0

    // ─── EntityTNTPrimed ───
    static constexpr int32_t DEFAULT_FUSE = 80;    // 4 seconds
    static constexpr float EXPLOSION_POWER = 4.0f;
    static constexpr float SPAWN_OFFSET = 0.5f;   // center of block

    // ─── Chain explosion fuse ───
    // rand(fuse/4) + fuse/8 = rand(20) + 10 = 10-29 ticks
    inline int32_t chainFuse(int32_t baseFuse, int32_t randVal) {
        return randVal % (baseFuse / 4) + baseFuse / 8;
    }

    // ─── Sound ───
    static constexpr const char* PRIME_SOUND = "game.tnt.primed";
    static constexpr float PRIME_VOLUME = 1.0f;
    static constexpr float PRIME_PITCH = 1.0f;

    // ─── Items ───
    static constexpr int32_t FLINT_AND_STEEL_ID = 259;
}

// ═══════════════════════════════════════════════════════════════════════════
// Note Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace NoteBlockConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 25;

    // ─── Note range ───
    static constexpr int32_t MIN_NOTE = 0;
    static constexpr int32_t MAX_NOTE = 24;
    static constexpr int32_t NOTE_COUNT = 25;

    // ─── Pitch formula ───
    // pitch = 2^((note - 12) / 12.0)
    inline float calculatePitch(int32_t note) {
        return static_cast<float>(std::pow(2.0, (static_cast<double>(note) - 12.0) / 12.0));
    }

    // ─── Instruments (blockEvent n4) ───
    static constexpr int32_t INSTRUMENT_HARP = 0;
    static constexpr int32_t INSTRUMENT_BASS_DRUM = 1;
    static constexpr int32_t INSTRUMENT_SNARE = 2;
    static constexpr int32_t INSTRUMENT_HAT = 3;
    static constexpr int32_t INSTRUMENT_BASS_ATTACK = 4;

    // Instrument names for sound
    inline const char* getInstrumentName(int32_t instrument) {
        switch (instrument) {
            case 1: return "bd";
            case 2: return "snare";
            case 3: return "hat";
            case 4: return "bassattack";
            default: return "harp";
        }
    }

    // ─── Sound ───
    static constexpr float NOTE_VOLUME = 3.0f;
    // Sound: "note.{instrument}"

    // ─── Particle ───
    static constexpr double PARTICLE_Y_OFFSET = 1.2;
    // data: note / 24.0
}

// ═══════════════════════════════════════════════════════════════════════════
// Jukebox Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace JukeboxConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 84;

    // ─── Metadata ───
    static constexpr int32_t META_EMPTY = 0;
    static constexpr int32_t META_HAS_DISC = 1;

    // ─── Record item IDs ───
    static constexpr int32_t RECORD_13_ID = 2256;    // Items.record_13
    static constexpr int32_t RECORD_WAIT_ID = 2267;  // Items.record_wait (last)

    // Comparator signal = itemId + 1 - RECORD_13_ID
    // 13→1, cat→2, blocks→3, chirp→4, far→5, mall→6,
    // mellohi→7, stal→8, strad→9, ward→10, 11→11, wait→12
    inline int32_t comparatorSignal(int32_t itemId) {
        return itemId + 1 - RECORD_13_ID;
    }

    // ─── Eject ───
    static constexpr int32_t SFX_STOP_RECORD = 1005;
    static constexpr int32_t PICKUP_DELAY = 10;
    static constexpr float EJECT_RANGE = 0.7f;
    static constexpr float EJECT_Y_BASE = 0.6f;
    static constexpr float EJECT_Y_OFFSET = 0.15f;  // (1-0.7)*0.5 ≈ 0.15
}

} // namespace mccpp
