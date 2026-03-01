/**
 * NoteJukeboxComparator.h — Note block, jukebox, and redstone comparator mechanics.
 *
 * Java references:
 *   - net.minecraft.tileentity.TileEntityNote (62 lines)
 *   - net.minecraft.block.BlockJukebox (104 lines)
 *   - net.minecraft.block.BlockJukebox$TileEntityJukebox (inner class)
 *   - net.minecraft.block.BlockRedstoneComparator (191 lines)
 *   - net.minecraft.tileentity.TileEntityComparator (24 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * NOTE BLOCK (TileEntityNote + BlockNote)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Fields:
 *   note (byte): 0-24, wraps via (note + 1) % 25
 *   previousRedstoneState (bool): edge detection for redstone
 *
 * changePitch: note = (note + 1) % 25, markDirty
 *
 * triggerNote(world, x, y, z):
 *   1. Block above must be air material — if not, no sound
 *   2. Instrument from material of block BELOW:
 *      - air/default → 0 (harp/piano)
 *      - rock → 1 (bass drum)
 *      - sand → 2 (snare drum)
 *      - glass → 3 (clicks/sticks)
 *      - wood → 4 (bass guitar)
 *   3. addBlockEvent(x, y, z, noteblock, instrument, note)
 *
 * NBT: "note" (byte, clamped 0-24)
 *
 * Block behavior (BlockNote):
 *   - Right click: changePitch + triggerNote
 *   - Redstone edge (powered → !previousRedstoneState): triggerNote
 *
 * ═══════════════════════════════════════════════════════════════════════
 * JUKEBOX (BlockJukebox + TileEntityJukebox)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * TileEntityJukebox:
 *   - 1-slot ItemStack holder (func_145856_a/func_145857_a)
 *   - NBT: "RecordItem" compound
 *
 * BlockJukebox:
 *   - Material: wood, creative tab: decorations
 *   - Metadata: 0 = empty, 1 = has disc
 *
 * Insert disc (func_149926_b):
 *   - Copy itemStack into tile entity
 *   - Set metadata to 1
 *
 * Eject disc (func_149925_e — right click when meta=1 or breakBlock):
 *   1. playAuxSFX(1005, x, y, z, 0) — stop record
 *   2. playRecord(null, x, y, z) — clear playing record
 *   3. Clear tile entity itemStack
 *   4. Set metadata to 0
 *   5. Spawn EntityItem with disc:
 *      - Position: block + rand*0.7 + 0.15 (XZ), block + 0.6+rand*0.14 (Y)
 *      - delayBeforeCanPickup = 10
 *
 * Comparator output:
 *   - Empty → 0
 *   - Has disc → Item.getIdFromItem(disc) + 1 - Item.getIdFromItem(record_13)
 *   - record_13 (ID 2256) = signal 1
 *   - record_cat (ID 2257) = signal 2
 *   - ... up to record_wait (ID 2267) = signal 12
 *
 * ═══════════════════════════════════════════════════════════════════════
 * REDSTONE COMPARATOR (BlockRedstoneComparator + TileEntityComparator)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Two modes (metadata bit 2):
 *   bit 2 = 0: Compare mode — output = input if input >= side input
 *   bit 2 = 1: Subtract mode — output = max(input - side_input, 0)
 *
 * Metadata layout:
 *   bits 0-1: direction (0-3)
 *   bit 2: subtract mode flag
 *   bit 3: powered flag
 *
 * Tick delay: 2 game ticks (func_149901_b returns 2)
 *
 * Input strength (getInputStrength):
 *   1. Standard redstone input from rear (via BlockRedstoneDiode)
 *   2. Check block directly behind (in facing direction):
 *      - If hasComparatorInputOverride: use that signal
 *      - Else if input < 15 AND block isNormalCube:
 *        Check block TWO behind (through solid block):
 *        If hasComparatorInputOverride: use that signal
 *   3. Examples: chests, furnaces, brewing stands, hoppers, jukeboxes
 *
 * Output calculation (getOutputStrength):
 *   - Compare mode: input strength directly
 *   - Subtract mode: max(input - side_input, 0)
 *
 * Activation check (isGettingInput):
 *   - input >= 15: always on
 *   - input == 0: always off
 *   - side == 0: on (no opposition)
 *   - else: input >= side (compare behavior)
 *
 * Update scheduling (func_149897_b):
 *   - Skip if already scheduled this tick
 *   - If output or powered state changed:
 *     - If connected to repeater chain (func_149912_i): priority -1 (faster)
 *     - Else: priority 0 (normal)
 *
 * Right click: toggle compare/subtract mode
 *   - Sound: random.click, 0.3f volume, 0.55f (subtract) or 0.5f (compare)
 *   - Toggle bit 2, recalculate immediately
 *
 * TileEntityComparator:
 *   - outputSignal (int): stored output level (0-15)
 *   - NBT: "OutputSignal" (int)
 *
 * Thread safety: Block updates on server thread.
 * JNI readiness: Simple constants and metadata parsing.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Note Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace NoteBlockConstants {
    // ─── Block ID ───
    static constexpr int32_t NOTE_BLOCK_ID = 25;

    // ─── Pitch range ───
    // Java: 0-24, wraps via (note + 1) % 25
    static constexpr int32_t MIN_NOTE = 0;
    static constexpr int32_t MAX_NOTE = 24;
    static constexpr int32_t NUM_NOTES = 25;

    // ─── Instruments ───
    // Java: determined by material of block below the note block
    static constexpr int32_t INSTRUMENT_HARP = 0;       // air/default
    static constexpr int32_t INSTRUMENT_BASS_DRUM = 1;   // rock material
    static constexpr int32_t INSTRUMENT_SNARE = 2;       // sand material
    static constexpr int32_t INSTRUMENT_CLICKS = 3;      // glass material
    static constexpr int32_t INSTRUMENT_BASS_GUITAR = 4; // wood material
    static constexpr int32_t NUM_INSTRUMENTS = 5;

    // ─── Instrument names (for debug/logging) ───
    static constexpr const char* INSTRUMENT_NAMES[] = {
        "harp",         // 0
        "bd",           // 1 (bass drum)
        "snare",        // 2
        "hat",          // 3 (hi-hat / clicks)
        "bassattack",   // 4
    };

    // ─── Sound name format ───
    // Java: "note." + instrumentName  (e.g. "note.harp", "note.bd")
    static constexpr const char* SOUND_PREFIX = "note.";

    // ─── NBT ───
    static constexpr const char* TAG_NOTE = "note";
}

// ═══════════════════════════════════════════════════════════════════════════
// Jukebox Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace JukeboxConstants {
    // ─── Block ID ───
    static constexpr int32_t JUKEBOX_ID = 84;

    // ─── Metadata ───
    static constexpr int32_t META_EMPTY = 0;
    static constexpr int32_t META_HAS_DISC = 1;

    // ─── Sound effects ───
    // Java: playAuxSFX(1005, ...) — stop record
    static constexpr int32_t SFX_STOP_RECORD = 1005;

    // ─── ItemEntity spawn offsets ───
    // Java: rand * 0.7 + (1-0.7)*0.5 = rand * 0.7 + 0.15 (XZ)
    // Java: rand * 0.7 + (1-0.7)*0.2 + 0.6 = rand * 0.7 + 0.26 (Y capped)
    static constexpr float SPAWN_SCALE = 0.7f;
    static constexpr float SPAWN_XZ_OFFSET = 0.15f;
    static constexpr float SPAWN_Y_BASE = 0.6f;
    static constexpr float SPAWN_Y_OFFSET = 0.06f;        // (1-0.7)*0.2

    // ─── Pickup delay ───
    static constexpr int32_t PICKUP_DELAY = 10;

    // ─── Comparator output ───
    // Java: Item.getIdFromItem(disc) + 1 - Item.getIdFromItem(Items.record_13)
    // record_13 = ID 2256
    static constexpr int32_t RECORD_13_ID = 2256;
    // record_cat = 2257, record_blocks = 2258, etc.
    // record_wait = 2267 (last disc in 1.7.10)

    // Disc comparator values:
    // record_13 → 1, record_cat → 2, ... record_wait → 12
    inline int32_t getComparatorOutput(int32_t discItemId) {
        if (discItemId == 0) return 0;
        return discItemId + 1 - RECORD_13_ID;
    }

    // ─── NBT ───
    static constexpr const char* TAG_RECORD_ITEM = "RecordItem";
}

// ═══════════════════════════════════════════════════════════════════════════
// Disc Item IDs
// ═══════════════════════════════════════════════════════════════════════════

namespace DiscIds {
    static constexpr int32_t RECORD_13 = 2256;     // C418 - 13
    static constexpr int32_t RECORD_CAT = 2257;    // C418 - cat
    static constexpr int32_t RECORD_BLOCKS = 2258; // C418 - blocks
    static constexpr int32_t RECORD_CHIRP = 2259;  // C418 - chirp
    static constexpr int32_t RECORD_FAR = 2260;    // C418 - far
    static constexpr int32_t RECORD_MALL = 2261;   // C418 - mall
    static constexpr int32_t RECORD_MELLOHI = 2262;// C418 - mellohi
    static constexpr int32_t RECORD_STAL = 2263;    // C418 - stal
    static constexpr int32_t RECORD_STRAD = 2264;  // C418 - strad
    static constexpr int32_t RECORD_WARD = 2265;   // C418 - ward
    static constexpr int32_t RECORD_11 = 2266;     // C418 - 11
    static constexpr int32_t RECORD_WAIT = 2267;   // C418 - wait
    static constexpr int32_t FIRST_RECORD = 2256;
    static constexpr int32_t LAST_RECORD = 2267;
    static constexpr int32_t NUM_RECORDS = 12;
}

// ═══════════════════════════════════════════════════════════════════════════
// Redstone Comparator Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace ComparatorConstants {
    // ─── Block IDs ───
    static constexpr int32_t UNPOWERED_COMPARATOR_ID = 149;
    static constexpr int32_t POWERED_COMPARATOR_ID = 150;

    // ─── Tick delay ───
    // Java: func_149901_b returns 2 (2 redstone ticks = 4 game ticks)
    static constexpr int32_t TICK_DELAY = 2;

    // ─── Metadata layout ───
    // bits 0-1: direction (0-3, player facing based)
    static constexpr int32_t DIRECTION_MASK = 3;
    // bit 2: subtract mode (1 = subtract, 0 = compare)
    static constexpr int32_t SUBTRACT_FLAG = 4;
    // bit 3: powered flag
    static constexpr int32_t POWERED_FLAG = 8;

    inline int32_t getDirection(int32_t metadata) {
        return metadata & DIRECTION_MASK;
    }

    inline bool isSubtractMode(int32_t metadata) {
        return (metadata & SUBTRACT_FLAG) != 0;
    }

    inline bool isPowered(int32_t metadata) {
        return (metadata & POWERED_FLAG) != 0;
    }

    // ─── Output calculation ───

    // Compare mode: output = input if input >= sideInput, else 0
    inline int32_t calculateCompareOutput(int32_t input, int32_t sideInput) {
        if (input >= sideInput) return input;
        return 0;
    }

    // Subtract mode: output = max(input - sideInput, 0)
    inline int32_t calculateSubtractOutput(int32_t input, int32_t sideInput) {
        int32_t result = input - sideInput;
        return result > 0 ? result : 0;
    }

    // Combined output based on mode
    inline int32_t calculateOutput(int32_t metadata, int32_t input, int32_t sideInput) {
        if (isSubtractMode(metadata)) {
            return calculateSubtractOutput(input, sideInput);
        }
        return input;  // Compare mode: pass through input
    }

    // ─── Activation check ───
    // Java: isGettingInput
    inline bool isActivated(int32_t input, int32_t sideInput) {
        if (input >= 15) return true;
        if (input == 0) return false;
        if (sideInput == 0) return true;
        return input >= sideInput;
    }

    // ─── Scheduling priority ───
    // Java: scheduleBlockUpdateWithPriority priority -1 (chain) or 0 (normal)
    static constexpr int32_t PRIORITY_FAST = -1;
    static constexpr int32_t PRIORITY_NORMAL = 0;

    // ─── Click sounds ───
    static constexpr const char* CLICK_SOUND = "random.click";
    static constexpr float CLICK_VOLUME = 0.3f;
    static constexpr float CLICK_PITCH_SUBTRACT = 0.55f;
    static constexpr float CLICK_PITCH_COMPARE = 0.5f;

    // ─── NBT ───
    static constexpr const char* TAG_OUTPUT_SIGNAL = "OutputSignal";
}

// ═══════════════════════════════════════════════════════════════════════════
// Blocks with Comparator Input Override
// Java: block.hasComparatorInputOverride() — blocks that feed comparator
// ═══════════════════════════════════════════════════════════════════════════

namespace ComparatorSources {
    // Blocks that provide comparator signals:
    // Chest (54, 146): Container.calcRedstoneFromInventory
    // Trapped Chest (146): same as chest
    // Furnace (61, 62): inventory fill level
    // Brewing Stand (117): inventory fill level
    // Hopper (154): inventory fill level
    // Dispenser (23): inventory fill level
    // Dropper (158): inventory fill level
    // Jukebox (84): disc ID based signal
    // Cauldron (118): water level (0-3)
    // End Portal Frame (120): eye of ender (0 or 15)
    // Cake (92): slices remaining

    // ─── Container.calcRedstoneFromInventory algorithm ───
    // Sum (stackSize / maxStackSize) across all slots
    // signal = floor(average * 14) + (total > 0 ? 1 : 0)
    // Range: 0 (empty) to 15 (full)
    static constexpr int32_t MAX_SIGNAL = 15;
    static constexpr int32_t SIGNAL_SCALE = 14;

    // ─── Special block signals ───
    // Cauldron: meta 0→0, 1→1, 2→2, 3→3
    // End portal frame: (meta & 4) ? 15 : 0  (has eye of ender)
    // Cake: (7 - meta) * 2  (0-7 bites, fresh = 14)
    static constexpr int32_t END_FRAME_EYE_META = 4;
    static constexpr int32_t END_FRAME_EYE_SIGNAL = 15;
    static constexpr int32_t CAKE_SIGNAL_PER_SLICE = 2;
    static constexpr int32_t CAKE_MAX_BITES = 7;
}

} // namespace mccpp
