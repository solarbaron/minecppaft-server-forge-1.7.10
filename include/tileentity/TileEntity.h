/**
 * TileEntity.h — Block entity base class and registry.
 *
 * Java reference: net.minecraft.tileentity.TileEntity
 *
 * 20 registered tile entity types with string ID mapping.
 * Base class provides position, metadata caching, lifecycle
 * (invalidate/validate), NBT serialization, and factory creation.
 *
 * Thread safety: TileEntities are per-chunk, accessed from server thread.
 *   Registry is static/const after init.
 *
 * JNI readiness: String-based type IDs for easy JVM mapping.
 */
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// TileEntityType — Enum of all vanilla tile entity types.
// Java reference: TileEntity static initializer
// ═══════════════════════════════════════════════════════════════════════════

enum class TileEntityType : int32_t {
    FURNACE       = 0,
    CHEST         = 1,
    ENDER_CHEST   = 2,
    JUKEBOX       = 3,
    DISPENSER     = 4,
    DROPPER       = 5,
    SIGN          = 6,
    MOB_SPAWNER   = 7,
    NOTE_BLOCK    = 8,
    PISTON        = 9,
    BREWING_STAND = 10,
    ENCHANT_TABLE = 11,
    END_PORTAL    = 12,
    COMMAND_BLOCK = 13,
    BEACON        = 14,
    SKULL         = 15,
    DAYLIGHT_DET  = 16,
    HOPPER        = 17,
    COMPARATOR    = 18,
    FLOWER_POT    = 19,
    UNKNOWN       = -1
};

// ═══════════════════════════════════════════════════════════════════════════
// TileEntity — Block entity base class.
// Java reference: net.minecraft.tileentity.TileEntity
// ═══════════════════════════════════════════════════════════════════════════

struct TileEntity {
    TileEntityType type = TileEntityType::UNKNOWN;
    std::string typeId;  // NBT "id" string (e.g., "Furnace", "Chest")

    // Java: xCoord, yCoord, zCoord
    int32_t xCoord = 0;
    int32_t yCoord = 0;
    int32_t zCoord = 0;

    // Java: blockMetadata — cached, -1 = uncached
    int32_t blockMetadata = -1;

    // Java: blockType — cached block ID at position
    int32_t blockTypeId = 0;

    // Java: tileEntityInvalid
    bool isInvalid = false;

    // ─── Type-specific data ───

    // Furnace: burn time, cook time, items
    int16_t furnaceBurnTime = 0;
    int16_t furnaceCookTime = 0;

    // Note block: note (0-24)
    int8_t noteBlockNote = 0;

    // Sign: 4 lines
    std::string signLine1, signLine2, signLine3, signLine4;

    // Command block
    std::string commandBlockCommand;
    std::string commandBlockName = "@";
    int32_t commandBlockSuccessCount = 0;
    bool commandBlockTrackOutput = true;

    // Beacon: levels, primary/secondary effects
    int32_t beaconLevels = 0;
    int32_t beaconPrimary = 0;
    int32_t beaconSecondary = 0;

    // Skull: type, rotation, extra type (player name)
    int8_t skullType = 0;
    int8_t skullRotation = 0;
    std::string skullExtraType;

    // Flower pot: item ID, metadata
    int32_t flowerPotItemId = 0;
    int32_t flowerPotMetadata = 0;

    // Comparator: output signal
    int32_t comparatorOutputSignal = 0;

    // Brewing stand: brew time
    int32_t brewTime = 0;

    // Mob spawner: entity ID, delays
    std::string spawnerEntityId = "Pig";
    int16_t spawnerDelay = 20;
    int16_t spawnerMinDelay = 200;
    int16_t spawnerMaxDelay = 800;
    int16_t spawnerSpawnCount = 4;
    int16_t spawnerMaxNearby = 6;
    int16_t spawnerSpawnRange = 4;

    // Piston: extending block
    int32_t pistonStoredBlockId = 0;
    int32_t pistonStoredMeta = 0;
    int32_t pistonFacing = 0;
    bool pistonExtending = false;

    // Hopper: transfer cooldown
    int32_t hopperTransferCooldown = -1;

    // ─── Lifecycle methods ───

    void invalidate() { isInvalid = true; }
    void validate() { isInvalid = false; }

    void updateContainingBlockInfo() {
        blockTypeId = 0;
        blockMetadata = -1;
    }

    // Java: getDeathMessage key-style type identification
    std::string getNBTId() const { return typeId; }
};

// ═══════════════════════════════════════════════════════════════════════════
// TileEntityRegistry — Factory and type mapping.
// Java reference: TileEntity.nameToClassMap / classToNameMap
// ═══════════════════════════════════════════════════════════════════════════

class TileEntityRegistry {
public:
    static void init();

    // Java: createAndLoadEntity — create by string ID
    static std::unique_ptr<TileEntity> create(const std::string& typeId);

    // Lookup type enum from string ID
    static TileEntityType getType(const std::string& typeId);

    // Lookup string ID from type enum
    static const std::string& getTypeId(TileEntityType type);

    // Get all registered type IDs
    static const std::vector<std::string>& getAllTypeIds();

    static int32_t getCount();

private:
    static std::unordered_map<std::string, TileEntityType> nameToType_;
    static std::unordered_map<int32_t, std::string> typeToName_;
    static std::vector<std::string> allTypeIds_;
    static bool initialized_;
};

} // namespace mccpp
