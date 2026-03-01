/**
 * TileEntities.h — Concrete tile entity types.
 *
 * Java references:
 *   - net.minecraft.tileentity.TileEntityChest (327 lines)
 *   - net.minecraft.tileentity.TileEntityFurnace (318 lines)
 *   - net.minecraft.tileentity.TileEntitySign (59 lines)
 *   - net.minecraft.tileentity.TileEntityNote (62 lines)
 *   - net.minecraft.tileentity.TileEntityBeacon (240 lines)
 *   - net.minecraft.tileentity.TileEntityBrewingStand (270 lines)
 *   - net.minecraft.tileentity.TileEntityDispenser (162 lines)
 *
 * Thread safety: Tile entities tick on world thread.
 */
#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <optional>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// TileEntityChest — Single/double chest with lid animation.
// Java: net.minecraft.tileentity.TileEntityChest (327 lines)
//
//   Inventory: 27 slots (chestContents[36] array but getSizeInventory=27)
//   Stack limit: 64
//   Adjacent chest detection: XNEG, XPOS, ZNEG, ZPOS
//   Double chest: checks for same BlockChest type
//   Lid animation: 0.1/tick, range [0,1]
//     Open sound at lidAngle==0 when players>0
//     Close sound when lidAngle crosses 0.5 downward
//   Player count: recount every 200 ticks via 5-block AABB EntityPlayer scan
//     Formula: (ticksSinceSync + x + y + z) % 200 == 0
//   Block events: id=1, data=numPlayersUsing
//   Notify: self + below on open/close
//   NBT: "Items" tag list with "Slot" byte, "CustomName" string
//   Player range: distSq ≤ 64.0
// ═══════════════════════════════════════════════════════════════════════════

class TileEntityChest {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;

    static constexpr int32_t INVENTORY_SIZE = 27;
    static constexpr int32_t STACK_LIMIT = 64;
    static constexpr float LID_SPEED = 0.1f;
    static constexpr float LID_MIN = 0.0f;
    static constexpr float LID_MAX = 1.0f;
    static constexpr float CLOSE_SOUND_THRESHOLD = 0.5f;
    static constexpr int32_t PLAYER_RECOUNT_INTERVAL = 200;
    static constexpr float PLAYER_SEARCH_RANGE = 5.0f;
    static constexpr double USE_RANGE_SQ = 64.0;
    static constexpr float SOUND_VOLUME = 0.5f;
    static constexpr int32_t BLOCK_EVENT_ID = 1;

    // Adjacent chest state
    bool adjacentChestChecked = false;
    bool hasAdjacentXNeg = false, hasAdjacentXPos = false;
    bool hasAdjacentZNeg = false, hasAdjacentZPos = false;

    // Lid animation
    float lidAngle = 0;
    float prevLidAngle = 0;
    int32_t numPlayersUsing = 0;
    int32_t ticksSinceSync = 0;
    int32_t cachedChestType = -1;
    std::string customName;

    void tickLid() {
        prevLidAngle = lidAngle;
        if (numPlayersUsing > 0 && lidAngle < LID_MAX) {
            lidAngle += LID_SPEED;
            if (lidAngle > LID_MAX) lidAngle = LID_MAX;
        } else if (numPlayersUsing == 0 && lidAngle > LID_MIN) {
            lidAngle -= LID_SPEED;
            if (lidAngle < LID_MIN) lidAngle = LID_MIN;
        }
    }

    bool shouldPlayOpenSound() const {
        return numPlayersUsing > 0 && lidAngle == 0 && !hasAdjacentZNeg && !hasAdjacentXNeg;
    }

    bool shouldPlayCloseSound() const {
        return lidAngle < CLOSE_SOUND_THRESHOLD && prevLidAngle >= CLOSE_SOUND_THRESHOLD
               && !hasAdjacentZNeg && !hasAdjacentXNeg;
    }

    bool shouldRecountPlayers() const {
        return numPlayersUsing != 0 && (ticksSinceSync + xCoord + yCoord + zCoord) % PLAYER_RECOUNT_INTERVAL == 0;
    }

    // Sound center offset for double chests
    double getSoundCenterX() const { return xCoord + 0.5 + (hasAdjacentXPos ? 0.5 : 0.0); }
    double getSoundCenterZ() const { return zCoord + 0.5 + (hasAdjacentZPos ? 0.5 : 0.0); }

    static constexpr const char* OPEN_SOUND = "random.chestopen";
    static constexpr const char* CLOSE_SOUND = "random.chestclosed";
    static constexpr const char* CONTAINER_NAME = "container.chest";
};

// ═══════════════════════════════════════════════════════════════════════════
// TileEntityFurnace — Smelting furnace with burn time tracking.
// Java: net.minecraft.tileentity.TileEntityFurnace (318 lines)
//
//   Inventory: 3 slots (0=input, 1=fuel, 2=output)
//   ISidedInventory: top→[0], bottom→[2,1], sides→[1]
//   Cook time: 200 ticks per item
//   Burn decrement: 1/tick when burning
//   Smelt: output slot checks item match + stack size limits
//   Container item support: fuel slot returns container item
//   Block state update: lit/unlit via BlockFurnace.updateFurnaceBlockState
//
//   ─── Burn times ───
//   wooden_slab: 150, wood material: 300, coal_block: 16000
//   wood tool/sword/hoe: 200, stick: 100, coal: 1600
//   lava_bucket: 20000, sapling: 100, blaze_rod: 2400
//
//   NBT: "Items" taglist, "BurnTime" short, "CookTime" short, "CustomName"
//   Slot validation: 0=any, 1=fuel only, 2=no insert
//   Extract: bottom only, fuel slot only outputs buckets
// ═══════════════════════════════════════════════════════════════════════════

class TileEntityFurnace {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;

    static constexpr int32_t INVENTORY_SIZE = 3;
    static constexpr int32_t SLOT_INPUT = 0;
    static constexpr int32_t SLOT_FUEL = 1;
    static constexpr int32_t SLOT_OUTPUT = 2;
    static constexpr int32_t STACK_LIMIT = 64;
    static constexpr int32_t COOK_TIME = 200;
    static constexpr double USE_RANGE_SQ = 64.0;

    // ISidedInventory face routing
    static constexpr std::array<int32_t, 1> SLOTS_TOP = {0};
    static constexpr std::array<int32_t, 2> SLOTS_BOTTOM = {2, 1};
    static constexpr std::array<int32_t, 1> SLOTS_SIDES = {1};

    int32_t furnaceBurnTime = 0;
    int32_t currentItemBurnTime = 0;
    int32_t furnaceCookTime = 0;
    std::string customName;

    bool isBurning() const { return furnaceBurnTime > 0; }

    void tickBurn() { if (furnaceBurnTime > 0) --furnaceBurnTime; }

    // ─── Fuel burn times ───
    struct FuelEntry { int32_t itemId; int32_t burnTime; };
    static constexpr int32_t NUM_FUEL_ENTRIES = 9;
    static constexpr std::array<FuelEntry, NUM_FUEL_ENTRIES> FUEL_TABLE = {{
        {126, 150},   // wooden_slab
        {280, 100},   // stick
        {263, 1600},  // coal
        {327, 20000}, // lava_bucket
        {6,   100},   // sapling
        {369, 2400},  // blaze_rod
        {173, 16000}, // coal_block
        // Wood material items: 300 ticks (generic)
        // Wood tools/swords/hoes: 200 ticks (generic)
        {0, 0},       // sentinel
        {0, 0},       // sentinel
    }};

    // Wood material burn time
    static constexpr int32_t WOOD_MATERIAL_BURN = 300;
    static constexpr int32_t WOOD_TOOL_BURN = 200;

    static constexpr const char* CONTAINER_NAME = "container.furnace";
};

// ═══════════════════════════════════════════════════════════════════════════
// TileEntitySign — Wall/standing sign with 4 text lines.
// Java: net.minecraft.tileentity.TileEntitySign (59 lines)
//
//   4 lines, max 15 chars each (truncated on load)
//   lineBeingEdited: -1 (not editing), 0-3 (line index)
//   isEditable: true initially, false after first NBT load
//   NBT: "Text1".."Text4" strings
//   Description packet: S33PacketUpdateSign
// ═══════════════════════════════════════════════════════════════════════════

class TileEntitySign {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;

    static constexpr int32_t NUM_LINES = 4;
    static constexpr int32_t MAX_LINE_LENGTH = 15;

    std::array<std::string, NUM_LINES> signText = {"", "", "", ""};
    int32_t lineBeingEdited = -1;
    bool isEditable = true;
    int32_t editorEntityId = -1;

    void loadLine(int32_t index, const std::string& text) {
        if (index >= 0 && index < NUM_LINES) {
            signText[index] = text.length() <= MAX_LINE_LENGTH ? text : text.substr(0, MAX_LINE_LENGTH);
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// TileEntityNote — Note block with pitch and instrument.
// Java: net.minecraft.tileentity.TileEntityNote (62 lines)
//
//   Note: 0-24 (mod 25 on increment)
//   Clamp on load: 0-24
//   Instrument: based on material of block below
//     0=harp(default), 1=rock(bassdrum), 2=sand(snare),
//     3=glass(clicks), 4=wood(bass)
//   Requires air above to play
//   Block event: noteblock, instrument, note
// ═══════════════════════════════════════════════════════════════════════════

class TileEntityNote {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;

    static constexpr int32_t MAX_NOTE = 24;
    static constexpr int32_t NUM_NOTES = 25;

    int8_t note = 0;
    bool previousRedstoneState = false;

    enum Instrument : int32_t {
        HARP = 0,
        BASS_DRUM = 1,
        SNARE = 2,
        CLICKS = 3,
        BASS = 4
    };

    void changePitch() {
        note = static_cast<int8_t>((note + 1) % NUM_NOTES);
    }

    void clampNote() {
        if (note < 0) note = 0;
        if (note > MAX_NOTE) note = MAX_NOTE;
    }

    // Material → instrument mapping
    // Java: rock=1, sand=2, glass=3, wood=4, else=0
    static Instrument getInstrument(int32_t materialId) {
        switch (materialId) {
            case 1: return BASS_DRUM;  // rock
            case 2: return SNARE;      // sand
            case 3: return CLICKS;     // glass
            case 4: return BASS;       // wood
            default: return HARP;
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// TileEntityBeacon — Beacon with pyramid level detection and effects.
// Java: net.minecraft.tileentity.TileEntityBeacon (240 lines)
//
//   Inventory: 1 slot (payment: emerald/diamond/gold_ingot/iron_ingot)
//   Stack limit: 1
//   Update every 80 ticks (worldTime % 80 == 0)
//   Level detection:
//     Must see sky at y+1
//     Pyramid layers 1-4 below: (2n+1)² blocks
//     Valid blocks: emerald_block, gold_block, diamond_block, iron_block
//   Effects:
//     Level 1-3: effectsList[0..2] (speed/haste, resistance/jump, strength)
//     Level 4: effectsList[3] (regeneration), secondary effect
//     If primary==secondary at level≥4: amplifier 1
//     Else secondary applied separately at amplifier 0
//   Effect range: levels*10+10 blocks, AABB up to world height
//   Effect duration: 180 ticks (9 seconds)
//   Achievement: fullBeacon at level 4
//   NBT: "Primary" int, "Secondary" int, "Levels" int
// ═══════════════════════════════════════════════════════════════════════════

class TileEntityBeacon {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;

    static constexpr int32_t INVENTORY_SIZE = 1;
    static constexpr int32_t STACK_LIMIT = 1;
    static constexpr int32_t UPDATE_INTERVAL = 80;
    static constexpr int32_t MAX_LEVELS = 4;
    static constexpr int32_t EFFECT_DURATION = 180; // ticks
    static constexpr double USE_RANGE_SQ = 64.0;
    static constexpr int32_t ACH_SEARCH_XZ = 10;
    static constexpr int32_t ACH_SEARCH_Y = 5;

    // Effects per tier
    // Java: effectsList[0]={speed,haste}, [1]={resistance,jump}, [2]={strength}, [3]={regen}
    struct EffectTier {
        int32_t effects[2];
        int32_t count;
    };
    static constexpr std::array<EffectTier, 4> EFFECTS = {{
        {{1, 3}, 2},   // speed(1), haste(3)
        {{11, 8}, 2},  // resistance(11), jump_boost(8)
        {{5, 0}, 1},   // strength(5)
        {{10, 0}, 1},  // regeneration(10)
    }};

    // Valid pyramid blocks
    static constexpr int32_t BLOCK_EMERALD = 133;
    static constexpr int32_t BLOCK_GOLD = 41;
    static constexpr int32_t BLOCK_DIAMOND = 57;
    static constexpr int32_t BLOCK_IRON = 42;

    // Valid payment items
    static constexpr int32_t ITEM_EMERALD = 388;
    static constexpr int32_t ITEM_DIAMOND = 264;
    static constexpr int32_t ITEM_GOLD_INGOT = 266;
    static constexpr int32_t ITEM_IRON_INGOT = 265;

    bool isComplete = false;
    int32_t levels = -1;
    int32_t primaryEffect = 0;
    int32_t secondaryEffect = 0;
    std::string customName;

    // Effect range
    double getEffectRange() const { return levels * 10 + 10; }

    // Amplifier: 1 if primary==secondary at level≥4, else 0
    int32_t getPrimaryAmplifier() const {
        return (levels >= MAX_LEVELS && primaryEffect == secondaryEffect) ? 1 : 0;
    }

    bool hasSecondaryEffect() const {
        return levels >= MAX_LEVELS && primaryEffect != secondaryEffect && secondaryEffect > 0;
    }

    static constexpr const char* CONTAINER_NAME = "container.beacon";
};

// ═══════════════════════════════════════════════════════════════════════════
// TileEntityBrewingStand — Potion brewing with ingredient checking.
// Java: net.minecraft.tileentity.TileEntityBrewingStand (270 lines)
//
//   Inventory: 4 slots (0-2=potion bottles, 3=ingredient)
//   ISidedInventory: top→[3](ingredient input), not-top→[0,1,2](bottles out)
//   Brew time: 400 ticks
//   canBrew: ingredient is potion ingredient, at least 1 bottle, result differs
//   Brewing: applies PotionHelper.applyIngredient to metadata
//   Container item support for ingredients
//   Filled slots: bitmask 0-2 for metadata update
//   Slot validation: 3=potionIngredient, 0-2=potionitem or glass_bottle
//   NBT: "Items" taglist, "BrewTime" short, "CustomName"
// ═══════════════════════════════════════════════════════════════════════════

class TileEntityBrewingStand {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;

    static constexpr int32_t INVENTORY_SIZE = 4;
    static constexpr int32_t SLOT_BOTTLE_0 = 0;
    static constexpr int32_t SLOT_BOTTLE_1 = 1;
    static constexpr int32_t SLOT_BOTTLE_2 = 2;
    static constexpr int32_t SLOT_INGREDIENT = 3;
    static constexpr int32_t STACK_LIMIT = 64;
    static constexpr int32_t BREW_TIME = 400;
    static constexpr double USE_RANGE_SQ = 64.0;

    // ISidedInventory
    static constexpr std::array<int32_t, 1> INPUT_SLOTS = {3};
    static constexpr std::array<int32_t, 3> OUTPUT_SLOTS = {0, 1, 2};

    int32_t brewTime = 0;
    int32_t filledSlots = 0;
    int32_t ingredientItemId = 0;
    std::string customName;

    bool isBrewing() const { return brewTime > 0; }

    // Filled slots bitmask (for block metadata)
    static int32_t calculateFilledSlots(bool slot0, bool slot1, bool slot2) {
        int32_t r = 0;
        if (slot0) r |= 1;
        if (slot1) r |= 2;
        if (slot2) r |= 4;
        return r;
    }

    static constexpr const char* CONTAINER_NAME = "container.brewing";
};

// ═══════════════════════════════════════════════════════════════════════════
// TileEntityDispenser — 9-slot random-output dispenser.
// Java: net.minecraft.tileentity.TileEntityDispenser (162 lines)
//
//   Inventory: 9 slots
//   Stack limit: 64
//   Random slot selection: reservoir sampling (rand(count)==0)
//   addItem: finds first empty slot, returns index or -1
//   Any item valid for any slot
//   NBT: "Items" taglist, "CustomName"
// ═══════════════════════════════════════════════════════════════════════════

class TileEntityDispenser {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;

    static constexpr int32_t INVENTORY_SIZE = 9;
    static constexpr int32_t STACK_LIMIT = 64;
    static constexpr double USE_RANGE_SQ = 64.0;

    std::string customName;

    // Java reservoir sampling: for each occupied slot, 1/count chance to select
    // Returns -1 if empty
    static int32_t selectRandomSlot(const bool occupied[9], int32_t randValues[9]) {
        int32_t selected = -1;
        int32_t count = 1;
        for (int32_t i = 0; i < INVENTORY_SIZE; ++i) {
            if (!occupied[i]) continue;
            if (randValues[i] % count == 0) {
                selected = i;
            }
            ++count;
        }
        return selected;
    }

    static constexpr const char* CONTAINER_NAME = "container.dispenser";
};

// ═══════════════════════════════════════════════════════════════════════════
// TileEntityDropper — Extends TileEntityDispenser.
// Java: net.minecraft.tileentity.TileEntityDropper (19 lines)
//   Identical to dispenser but named "container.dropper"
// ═══════════════════════════════════════════════════════════════════════════

class TileEntityDropper : public TileEntityDispenser {
public:
    static constexpr const char* CONTAINER_NAME_DROPPER = "container.dropper";
};

// ═══════════════════════════════════════════════════════════════════════════
// TileEntityEnchantmentTable — Enchanting table with book animation.
// Java: net.minecraft.tileentity.TileEntityEnchantmentTable (117 lines)
//   Book rotation animation, player tracking for page flip
//   No inventory persistence (slot handled by container)
// ═══════════════════════════════════════════════════════════════════════════

class TileEntityEnchantmentTable {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;

    int32_t tickCount = 0;
    float bookSpread = 0;         // 0-1
    float bookSpreadPrev = 0;
    float bookRotation = 0;       // radians
    float bookRotationPrev = 0;
    float pageFlip = 0;
    float pageFlipPrev = 0;
    float bookSpreadTarget = 0;
    float bookRotationTarget = 0;

    static constexpr const char* CONTAINER_NAME = "container.enchant";
};

// ═══════════════════════════════════════════════════════════════════════════
// TileEntityHopper — Item transfer hopper.
// Java: TileEntityHopper ~300 lines
//   5 slots, 8 tick transfer cooldown
//   ISidedInventory: all slots from all faces
//   Pull from above, push below, hopper cart support
// ═══════════════════════════════════════════════════════════════════════════

class TileEntityHopper {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;

    static constexpr int32_t INVENTORY_SIZE = 5;
    static constexpr int32_t STACK_LIMIT = 64;
    static constexpr int32_t TRANSFER_COOLDOWN = 8;
    static constexpr double USE_RANGE_SQ = 64.0;

    int32_t transferCooldown = -1;
    std::string customName;

    bool isOnCooldown() const { return transferCooldown > 0; }
    void setCooldown(int32_t ticks) { transferCooldown = ticks; }

    static constexpr const char* CONTAINER_NAME = "container.hopper";
};

// ═══════════════════════════════════════════════════════════════════════════
// Simple tile entities (minimal state)
// ═══════════════════════════════════════════════════════════════════════════

class TileEntityComparator {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;
    int32_t outputSignal = 0;
};

class TileEntityDaylightDetector {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;
    // Updates light level from sky every tick
};

class TileEntityEndPortal {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;
    // No additional state
};

class TileEntityEnderChest {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;
    // Player-specific inventory managed by EntityPlayerMP
    float lidAngle = 0;
    float prevLidAngle = 0;
    int32_t numPlayersUsing = 0;
    int32_t ticksSinceSync = 0;
    static constexpr float LID_SPEED = 0.1f;
};

class TileEntityFlowerPot {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;
    int32_t flowerId = 0;
    int32_t flowerMetadata = 0;
    // NBT: "Item" int, "Data" int
};

class TileEntityPiston {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;
    int32_t storedBlockId = 0;
    int32_t storedMetadata = 0;
    int32_t pistonDirection = 0;
    float progress = 0;
    float lastProgress = 0;
    bool extending = false;
};

class TileEntitySkull {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;
    int32_t skullType = 0;
    int32_t skullRotation = 0;
    std::string extraType; // player name
};

class TileEntityMobSpawner {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;
    std::string entityType = "Pig";
    int32_t spawnDelay = 20;
    int32_t minSpawnDelay = 200;
    int32_t maxSpawnDelay = 800;
    int32_t spawnCount = 4;
    int32_t maxNearbyEntities = 6;
    int32_t activatingRangeFromPlayer = 16;
    int32_t spawnRange = 4;

    static constexpr const char* DEFAULT_ENTITY = "Pig";
};

class TileEntityCommandBlock {
public:
    int32_t xCoord = 0, yCoord = 0, zCoord = 0;
    std::string command;
    int32_t successCount = 0;
    std::string lastOutput;
    bool trackOutput = true;
    std::string customName = "@";
};

} // namespace mccpp
