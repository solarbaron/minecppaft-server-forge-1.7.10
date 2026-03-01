/**
 * DyeBookItems.h — Dye item with bonemeal/cocoa/sheep, writable and signed books.
 *
 * Java references:
 *   - net.minecraft.item.ItemDye (115 lines)
 *   - net.minecraft.item.ItemWritableBook (50 lines)
 *   - net.minecraft.item.ItemEditableBook (56 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DYE ITEM (ItemDye)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Has subtypes, max durability 0, 16 color variants by metadata
 *
 * Color names (metadata 0-15):
 *   0=black, 1=red, 2=green, 3=brown, 4=blue, 5=purple,
 *   6=cyan, 7=silver, 8=gray, 9=pink, 10=lime, 11=yellow,
 *   12=lightBlue, 13=magenta, 14=orange, 15=white
 *
 * Color hex values (dyeColors[]):
 *   {0x1E1B1B, 11743532, 3887386, 5320730, 2437522, 8073150,
 *    2651799, 0xABABAB, 0x434343, 14188952, 4312372, 14602026,
 *    6719955, 12801229, 15435844, 0xF0F0F0}
 *
 * Special actions:
 *
 * 1. BONEMEAL (meta 15 = white):
 *    Block must implement IGrowable
 *    canFertilize → shouldFertilize → fertilize
 *    Decrements stack, plays SFX 2005 (green sparkle particles)
 *
 * 2. COCOA BEAN (meta 3 = brown):
 *    Right-click on jungle log (BlockLog type 3):
 *    Places cocoa block on the clicked face
 *    Cannot place on top (face 0) or bottom (face 1)
 *    Side faces offset by face direction, air check
 *    Cocoa uses onBlockPlaced for metadata
 *
 * 3. SHEEP DYE (itemInteractionForEntity):
 *    Right-click on EntitySheep
 *    Uses BlockColored.func_150032_b(meta) = ~meta & 0xF inversion
 *    Only if: not sheared AND current fleece color != new color
 *    Sets fleece color, decrements stack
 *
 * Item ID: 351 (dye)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * WRITABLE BOOK (ItemWritableBook)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Max stack: 1
 * Right-click: opens book GUI (displayGUIBook)
 * getShareTag: true (NBT sent to client)
 *
 * Validation (validBookPageTagContents):
 *   - NBT must exist
 *   - Must have "pages" key of type 9 (NBTTagList)
 *   - Each page is type 8 (String)
 *   - Each page ≤ 256 characters
 *
 * Item ID: 386 (writable_book)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * SIGNED BOOK (ItemEditableBook)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Max stack: 1
 * Right-click: opens book GUI
 * getShareTag: true
 * Display name: NBT "title" tag (if present), else default
 *
 * Validation (validBookTagContents):
 *   - Passes validBookPageTagContents (pages check)
 *   - Must have "title" key of type 8 (String)
 *   - Title ≤ 16 characters
 *   - Must have "author" key of type 8 (String)
 *
 * Item ID: 387 (written_book)
 *
 * Thread safety: Item use on server thread.
 * JNI readiness: NBT validation accessible for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Dye Item Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DyeConstants {
    // ─── Item ID ───
    static constexpr int32_t ITEM_ID = 351;

    // ─── Color count ───
    static constexpr int32_t NUM_COLORS = 16;

    // ─── Color names ───
    static constexpr const char* COLOR_NAMES[16] = {
        "black", "red", "green", "brown", "blue", "purple",
        "cyan", "silver", "gray", "pink", "lime", "yellow",
        "lightBlue", "magenta", "orange", "white"
    };

    // ─── Color hex values (for particles, rendering) ───
    static constexpr int32_t DYE_COLORS[16] = {
        0x1E1B1B,   // 0  black
        11743532,   // 1  red
        3887386,    // 2  green
        5320730,    // 3  brown
        2437522,    // 4  blue
        8073150,    // 5  purple
        2651799,    // 6  cyan
        0xABABAB,   // 7  silver
        0x434343,   // 8  gray
        14188952,   // 9  pink
        4312372,    // 10 lime
        14602026,   // 11 yellow
        6719955,    // 12 light blue
        12801229,   // 13 magenta
        15435844,   // 14 orange
        0xF0F0F0    // 15 white
    };

    // ─── Special meta values ───
    static constexpr int32_t META_BONEMEAL = 15;   // white dye = bonemeal
    static constexpr int32_t META_COCOA_BEAN = 3;  // brown dye = cocoa bean
    static constexpr int32_t META_LAPIS = 4;       // blue dye = lapis lazuli

    // ─── Bonemeal ───
    static constexpr int32_t SFX_BONEMEAL = 2005;  // green sparkle particles

    // ─── Cocoa placement ───
    static constexpr int32_t JUNGLE_LOG_TYPE = 3;  // BlockLog type for jungle
    // Cannot place on face 0 (bottom) or 1 (top)

    // ─── Sheep dye ───
    // Color inversion: ~meta & 0xF (BlockColored.func_150032_b)
    inline int32_t invertColor(int32_t dyeMeta) {
        return (~dyeMeta) & 0xF;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Book Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace BookConstants {
    // ─── Item IDs ───
    static constexpr int32_t WRITABLE_BOOK_ID = 386;
    static constexpr int32_t WRITTEN_BOOK_ID = 387;

    // ─── Stack ───
    static constexpr int32_t MAX_STACK = 1;

    // ─── Page validation ───
    static constexpr int32_t MAX_PAGE_LENGTH = 256;      // characters per page
    static constexpr int32_t PAGES_TAG_TYPE = 9;          // NBTTagList
    static constexpr int32_t STRING_TAG_TYPE = 8;         // NBTTagString

    // ─── Title validation ───
    static constexpr int32_t MAX_TITLE_LENGTH = 16;

    // ─── NBT keys ───
    static constexpr const char* NBT_PAGES = "pages";
    static constexpr const char* NBT_TITLE = "title";
    static constexpr const char* NBT_AUTHOR = "author";
    static constexpr const char* NBT_GENERATION = "generation";
}

} // namespace mccpp
