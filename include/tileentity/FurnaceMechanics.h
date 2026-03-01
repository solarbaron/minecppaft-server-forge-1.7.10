/**
 * FurnaceMechanics.h — Furnace smelting system with fuel burn times.
 *
 * Java reference:
 *   - net.minecraft.tileentity.TileEntityFurnace (318 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FURNACE TILE ENTITY (TileEntityFurnace)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Inventory: 3 slots
 *   - Slot 0: input (item to smelt)
 *   - Slot 1: fuel
 *   - Slot 2: output (smelted result)
 *
 * ISidedInventory:
 *   - Face 0 (bottom): {2, 1} — extract output and spent fuel
 *   - Face 1 (top): {0} — insert input
 *   - All other faces: {1} — insert fuel
 *   - canExtractItem: bottom face, fuel slot → only if item is bucket
 *     (allows lava bucket → bucket return)
 *
 * State variables:
 *   - furnaceBurnTime: remaining burn ticks for current fuel item
 *   - currentItemBurnTime: total burn time of current fuel (for GUI progress)
 *   - furnaceCookTime: progress toward smelting (0-199, completes at 200)
 *
 * Update tick logic:
 *   1. If furnaceBurnTime > 0: decrement
 *   2. If furnaceBurnTime == 0 && canSmelt(): consume fuel, start burning
 *      - Consume 1 fuel from slot 1
 *      - If fuel has container item (lava_bucket → bucket): replace slot 1
 *   3. If burning && canSmelt(): increment furnaceCookTime
 *      - At furnaceCookTime == 200: smeltItem(), reset to 0
 *   4. If !burning || !canSmelt(): reset furnaceCookTime to 0
 *   5. If burn state changed: update block to lit/unlit furnace
 *
 * canSmelt():
 *   - Input slot must have item
 *   - FurnaceRecipes must have result for input
 *   - Output slot must be:
 *     a) empty, OR
 *     b) same item as result AND stackSize < min(64, maxStackSize)
 *
 * smeltItem():
 *   - Get result from FurnaceRecipes
 *   - If output empty: copy result
 *   - If output same item: increment stackSize
 *   - Decrement input stackSize (remove if 0)
 *
 * isItemValidForSlot:
 *   - Slot 2 (output): never
 *   - Slot 1 (fuel): only if isItemFuel
 *   - Slot 0 (input): always
 *
 * NBT: Items (TagList + Slot byte), BurnTime (short), CookTime (short),
 *       CustomName (string)
 * Load: currentItemBurnTime recalculated from fuel slot
 *
 * Thread safety: Tile entity ticking on server thread.
 * JNI readiness: Simple POD constants.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Furnace Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FurnaceConstants {
    // ─── Block IDs ───
    static constexpr int32_t FURNACE_ID = 61;        // unlit
    static constexpr int32_t LIT_FURNACE_ID = 62;    // lit

    // ─── Inventory layout ───
    static constexpr int32_t TOTAL_SLOTS = 3;
    static constexpr int32_t INPUT_SLOT = 0;
    static constexpr int32_t FUEL_SLOT = 1;
    static constexpr int32_t OUTPUT_SLOT = 2;

    // ─── Smelt duration ───
    // Java: if (furnaceCookTime == 200) { smeltItem(); }
    static constexpr int32_t SMELT_DURATION = 200;  // 10 seconds at 20 TPS

    // ─── Stack limit ───
    static constexpr int32_t STACK_LIMIT = 64;

    // ─── Useable distance ───
    static constexpr double USE_DISTANCE_SQ = 64.0;

    // ─── ISidedInventory face mappings ───
    // Java: face 0 (bottom) → {2, 1}, face 1 (top) → {0}, others → {1}
    static constexpr int32_t FACE_BOTTOM = 0;
    static constexpr int32_t FACE_TOP = 1;

    // ─── canExtractItem exception ───
    // Java: n2 != 0 || n != 1 || itemStack.getItem() == Items.bucket
    // Only allow extracting from fuel slot via bottom if item is bucket
    static constexpr int32_t BUCKET_ITEM_ID = 325;

    // ─── Container window properties ───
    // Java: progress bar IDs — 0=cookTime, 1=burnTime, 2=currentItemBurnTime
    static constexpr int32_t PROGRESS_COOK_TIME = 0;
    static constexpr int32_t PROGRESS_BURN_TIME = 1;
    static constexpr int32_t PROGRESS_CURRENT_BURN = 2;

    // ─── NBT tags ───
    static constexpr const char* TAG_ITEMS = "Items";
    static constexpr const char* TAG_BURN_TIME = "BurnTime";
    static constexpr const char* TAG_COOK_TIME = "CookTime";
    static constexpr const char* TAG_CUSTOM_NAME = "CustomName";
    static constexpr const char* TAG_SLOT = "Slot";

    // ─── Default container name ───
    static constexpr const char* DEFAULT_NAME = "container.furnace";
}

// ═══════════════════════════════════════════════════════════════════════════
// Fuel Burn Times
// Java: TileEntityFurnace.getItemBurnTime(ItemStack)
// All values in ticks (20 ticks = 1 second)
// ═══════════════════════════════════════════════════════════════════════════

namespace FuelBurnTime {
    // ─── Block fuels ───
    // Java: wooden_slab → 150
    static constexpr int32_t WOODEN_SLAB = 150;
    // Java: block.getMaterial() == Material.wood → 300
    static constexpr int32_t WOOD_BLOCK = 300;        // planks, logs, fences, etc.
    // Java: coal_block → 16000
    static constexpr int32_t COAL_BLOCK = 16000;       // 80 items (10× coal)

    // ─── Wooden tool fuels ───
    // Java: ItemTool WOOD → 200, ItemSword WOOD → 200, ItemHoe WOOD → 200
    static constexpr int32_t WOODEN_TOOL = 200;
    static constexpr int32_t WOODEN_SWORD = 200;
    static constexpr int32_t WOODEN_HOE = 200;

    // ─── Item fuels ───
    // Java: Items.stick → 100
    static constexpr int32_t STICK = 100;
    // Java: Items.coal → 1600
    static constexpr int32_t COAL = 1600;              // 8 items
    // Java: Items.lava_bucket → 20000
    static constexpr int32_t LAVA_BUCKET = 20000;      // 100 items
    // Java: sapling → 100
    static constexpr int32_t SAPLING = 100;
    // Java: Items.blaze_rod → 2400
    static constexpr int32_t BLAZE_ROD = 2400;         // 12 items

    // ─── Item IDs for fuel detection ───
    static constexpr int32_t STICK_ID = 280;
    static constexpr int32_t COAL_ID = 263;
    static constexpr int32_t LAVA_BUCKET_ID = 327;
    static constexpr int32_t BLAZE_ROD_ID = 369;

    // ─── Block IDs for fuel detection ───
    static constexpr int32_t WOODEN_SLAB_ID = 126;     // wooden_slab
    static constexpr int32_t COAL_BLOCK_ID = 173;      // coal_block
    static constexpr int32_t SAPLING_ID = 6;            // sapling

    // ─── Wood material block IDs (getMaterial() == wood) ───
    // Includes: planks, logs, wooden stairs, fences, fence gates,
    //           bookshelves, chests, crafting table, jukeboxes,
    //           trapdoors, old/new wood slabs, daylight sensors,
    //           wooden buttons, trapped chests, note blocks
    static constexpr int32_t OAK_PLANKS_ID = 5;
    static constexpr int32_t OAK_LOG_ID = 17;
    static constexpr int32_t OAK_LOG2_ID = 162;
    static constexpr int32_t BOOKSHELF_ID = 47;
    static constexpr int32_t CHEST_ID = 54;
    static constexpr int32_t CRAFTING_TABLE_ID = 58;
    static constexpr int32_t JUKEBOX_ID = 84;
    static constexpr int32_t TRAPDOOR_ID = 96;
    static constexpr int32_t NOTE_BLOCK_ID = 25;
    static constexpr int32_t TRAPPED_CHEST_ID = 146;
    static constexpr int32_t DAYLIGHT_SENSOR_ID = 151;

    // ─── Container item return ───
    // Lava bucket → bucket (item 325)
    static constexpr int32_t BUCKET_RETURN_ID = 325;

    // ─── Summary table ───
    // Fuel                 | Burn Time | Items Smelted
    // --------------------|-----------|---------------
    // Lava Bucket          | 20000     | 100
    // Coal Block           | 16000     | 80
    // Blaze Rod            | 2400      | 12
    // Coal/Charcoal        | 1600      | 8
    // Wood Block (generic) | 300       | 1.5
    // Wooden Tool/Sword    | 200       | 1
    // Wooden Slab          | 150       | 0.75
    // Stick                | 100       | 0.5
    // Sapling              | 100       | 0.5

    // ─── Burn time lookup by item/block ID ───
    // This maps the prioritized check order from Java exactly
    inline int32_t getBurnTime(int32_t itemId, bool isBlock, int32_t blockMaterialId,
                                bool isWoodTool, bool isWoodSword, bool isWoodHoe) {
        // Block checks (order matters — specific blocks before material check)
        if (isBlock) {
            if (itemId == WOODEN_SLAB_ID) return WOODEN_SLAB;          // 150
            if (blockMaterialId == 1 /* Material.wood */) return WOOD_BLOCK; // 300
            if (itemId == COAL_BLOCK_ID) return COAL_BLOCK;             // 16000
        }
        // Wooden tool checks
        if (isWoodTool) return WOODEN_TOOL;                             // 200
        if (isWoodSword) return WOODEN_SWORD;                           // 200
        if (isWoodHoe) return WOODEN_HOE;                               // 200
        // Individual item checks
        if (itemId == STICK_ID) return STICK;                           // 100
        if (itemId == COAL_ID) return COAL;                             // 1600
        if (itemId == LAVA_BUCKET_ID) return LAVA_BUCKET;              // 20000
        if (itemId == SAPLING_ID) return SAPLING;                      // 100
        if (itemId == BLAZE_ROD_ID) return BLAZE_ROD;                  // 2400
        return 0;  // Not fuel
    }

    inline bool isFuel(int32_t burnTime) {
        return burnTime > 0;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Smelting XP rewards
// Java: FurnaceRecipes.getSmeltingExperience(ItemStack)
// ═══════════════════════════════════════════════════════════════════════════

namespace SmeltingXP {
    // ─── XP values for smelted items ───
    static constexpr float IRON_INGOT = 0.7f;
    static constexpr float GOLD_INGOT = 1.0f;
    static constexpr float DIAMOND = 1.0f;
    static constexpr float GLASS = 0.1f;
    static constexpr float STONE = 0.1f;
    static constexpr float COOKED_BEEF = 0.35f;
    static constexpr float COOKED_PORK = 0.35f;
    static constexpr float COOKED_CHICKEN = 0.35f;
    static constexpr float COOKED_FISH = 0.35f;
    static constexpr float BAKED_POTATO = 0.35f;
    static constexpr float BRICK = 0.3f;
    static constexpr float CACTUS_GREEN = 0.2f;     // dye from cactus
    static constexpr float NETHER_BRICK = 0.1f;
    static constexpr float COAL_FROM_ORE = 0.1f;
    static constexpr float EMERALD = 1.0f;
    static constexpr float REDSTONE = 0.7f;
    static constexpr float LAPIS = 0.2f;
    static constexpr float QUARTZ = 0.2f;
    static constexpr float DEFAULT_XP = 0.0f;

    // ─── XP orb spawning on extract ───
    // Java: When player extracts from output slot, accumulated XP is spawned
    // as experience orbs with random splitting
    // Formula: xpTotal = (int)accumulated + (random < fractional ? 1 : 0)
}

} // namespace mccpp
