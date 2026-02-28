/**
 * Redstone.h — Redstone signal propagation, power sources, and pistons.
 *
 * Java references:
 *   - net.minecraft.block.BlockRedstoneWire — Signal propagation (0-15 power)
 *   - net.minecraft.block.BlockRedstoneTorch — Invertible power source
 *   - net.minecraft.block.BlockRedstoneRepeater — Signal delay/amplification
 *   - net.minecraft.block.BlockRedstoneComparator — Compare/subtract modes
 *   - net.minecraft.block.BlockPistonBase — Piston extend/retract
 *   - net.minecraft.block.BlockLever — Toggle power source
 *   - net.minecraft.block.BlockButton — Momentary power source
 *   - net.minecraft.block.BlockPressurePlate — Weight-based power
 *
 * Thread safety:
 *   - Redstone signal updates execute on the main tick thread.
 *   - Scheduled tick updates use the world's tick queue.
 *   - Piston events are synchronous within the tick loop.
 *
 * JNI readiness: Simple enums and method signatures for JVM binding.
 */
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// RedstoneDirection — Facing constants matching Java's Direction class
// Java reference: net.minecraft.util.Direction
// ═══════════════════════════════════════════════════════════════════════════

enum class RedstoneFace : int32_t {
    DOWN  = 0,
    UP    = 1,
    NORTH = 2,
    SOUTH = 3,
    WEST  = 4,
    EAST  = 5
};

// Java: net.minecraft.util.Facing — offsets for each face
struct FacingOffset {
    int32_t dx, dy, dz;
};

inline constexpr FacingOffset FACING_OFFSETS[6] = {
    { 0, -1,  0}, // DOWN
    { 0,  1,  0}, // UP
    { 0,  0, -1}, // NORTH
    { 0,  0,  1}, // SOUTH
    {-1,  0,  0}, // WEST
    { 1,  0,  0}, // EAST
};

// Java: net.minecraft.util.Direction.offsetX/Y/Z arrays
inline constexpr int32_t OPPOSITE_FACE[6] = {1, 0, 3, 2, 5, 4};

// ═══════════════════════════════════════════════════════════════════════════
// RedstoneSignal — Core signal propagation engine.
// Java reference: net.minecraft.block.BlockRedstoneWire.func_150175_a
//
// Implements recursive signal strength calculation with decay per block.
// Signal strength: 0 (no power) to 15 (max power)
// Each wire block reduces signal by 1.
// ═══════════════════════════════════════════════════════════════════════════

class RedstoneSignal {
public:
    // Maximum signal strength (redstone levels)
    static constexpr int32_t MAX_POWER = 15;
    // No signal
    static constexpr int32_t MIN_POWER = 0;

    // Block position key for visited sets
    struct BlockPos {
        int32_t x, y, z;
        bool operator==(const BlockPos& o) const { return x == o.x && y == o.y && z == o.z; }
    };

    struct BlockPosHash {
        std::size_t operator()(const BlockPos& p) const {
            return std::hash<int64_t>{}(
                (static_cast<int64_t>(p.x) * 0x1F1F1F1FL) ^
                (static_cast<int64_t>(p.y) * 0x7F7F7FL) ^
                static_cast<int64_t>(p.z));
        }
    };

    // Callback: get block ID at position
    using GetBlockFn = std::function<int32_t(int32_t x, int32_t y, int32_t z)>;
    // Callback: get block metadata at position
    using GetMetaFn = std::function<int32_t(int32_t x, int32_t y, int32_t z)>;
    // Callback: set block metadata (power level for wire)
    using SetMetaFn = std::function<void(int32_t x, int32_t y, int32_t z, int32_t meta)>;
    // Callback: notify neighbors of block change
    using NotifyFn = std::function<void(int32_t x, int32_t y, int32_t z)>;
    // Callback: check if block is solid (opaque cube)
    using IsSolidFn = std::function<bool(int32_t x, int32_t y, int32_t z)>;

    // Java: BlockRedstoneWire.func_150175_a — Calculate and propagate signal
    // Returns the computed power level at this position
    static int32_t calculateSignal(int32_t x, int32_t y, int32_t z,
                                     int32_t currentPower,
                                     GetBlockFn getBlock,
                                     GetMetaFn getMeta,
                                     IsSolidFn isSolid);

    // Java: BlockRedstoneWire.func_150177_e — Called on wire placement/update
    // Propagates signal through network
    static void updateSurroundingRedstone(int32_t x, int32_t y, int32_t z,
                                            GetBlockFn getBlock,
                                            GetMetaFn getMeta,
                                            SetMetaFn setMeta,
                                            NotifyFn notify,
                                            IsSolidFn isSolid);

    // Java: BlockRedstoneWire.func_150172_m — Update connected neighbors
    static void notifyWireNeighbors(int32_t x, int32_t y, int32_t z,
                                      NotifyFn notify);

    // Get maximum incoming power from adjacent blocks
    static int32_t getMaxCurrentStrength(int32_t x, int32_t y, int32_t z,
                                           int32_t currentMax,
                                           GetBlockFn getBlock,
                                           GetMetaFn getMeta,
                                           IsSolidFn isSolid);
};

// Block IDs for redstone components (matching vanilla 1.7.10)
namespace RedstoneBlocks {
    constexpr int32_t REDSTONE_WIRE           = 55;
    constexpr int32_t REDSTONE_TORCH_ON       = 76;
    constexpr int32_t REDSTONE_TORCH_OFF      = 75;
    constexpr int32_t REDSTONE_REPEATER_OFF   = 93;
    constexpr int32_t REDSTONE_REPEATER_ON    = 94;
    constexpr int32_t REDSTONE_COMPARATOR_OFF = 149;
    constexpr int32_t REDSTONE_COMPARATOR_ON  = 150;
    constexpr int32_t REDSTONE_BLOCK          = 152;
    constexpr int32_t LEVER                   = 69;
    constexpr int32_t STONE_BUTTON            = 77;
    constexpr int32_t WOODEN_BUTTON           = 143;
    constexpr int32_t STONE_PRESSURE_PLATE    = 70;
    constexpr int32_t WOODEN_PRESSURE_PLATE   = 72;
    constexpr int32_t LIGHT_WEIGHTED_PLATE    = 147;
    constexpr int32_t HEAVY_WEIGHTED_PLATE    = 148;
    constexpr int32_t DAYLIGHT_SENSOR         = 151;
    constexpr int32_t TRAPPED_CHEST           = 146;
    constexpr int32_t REDSTONE_LAMP_OFF       = 123;
    constexpr int32_t REDSTONE_LAMP_ON        = 124;
    constexpr int32_t PISTON                  = 33;
    constexpr int32_t STICKY_PISTON           = 29;
    constexpr int32_t PISTON_HEAD             = 34;
    constexpr int32_t PISTON_EXTENSION        = 36;
    constexpr int32_t DISPENSER               = 23;
    constexpr int32_t DROPPER                 = 158;
    constexpr int32_t TNT                     = 46;
    constexpr int32_t NOTE_BLOCK              = 25;
}

// ═══════════════════════════════════════════════════════════════════════════
// RedstoneTorch — Redstone torch logic.
// Java reference: net.minecraft.block.BlockRedstoneTorch
//
// Features:
//   - Inverts input signal (powered block → torch OFF)
//   - Burnout protection: max 8 toggles within 60 ticks
//   - Tick rate: 2 (1/10 second)
//   - Outputs power 15 in all directions except attached face
// ═══════════════════════════════════════════════════════════════════════════

class RedstoneTorch {
public:
    // Java: BlockRedstoneTorch tick rate = 2
    static constexpr int32_t TICK_RATE = 2;
    // Java: burnout protection — max 8 toggles in 60 ticks
    static constexpr int32_t MAX_TOGGLES = 8;
    static constexpr int64_t TOGGLE_WINDOW = 60;

    struct Toggle {
        int32_t x, y, z;
        int64_t worldTime;
    };

    // Check if torch is receiving power from attached block
    // Java: BlockRedstoneTorch.func_150110_m
    static bool isReceivingPower(int32_t x, int32_t y, int32_t z,
                                  int32_t metadata,
                                  RedstoneSignal::GetBlockFn getBlock,
                                  RedstoneSignal::GetMetaFn getMeta);

    // Check burnout protection
    // Java: BlockRedstoneTorch.func_150111_a
    static bool checkBurnout(int32_t x, int32_t y, int32_t z,
                              int64_t worldTime,
                              std::vector<Toggle>& toggleList,
                              bool addToggle);

    // Get power output for a given face
    // Java: BlockRedstoneTorch.isProvidingWeakPower
    static int32_t getWeakPower(bool isLit, int32_t metadata, int32_t side);

    // Strong power: only to block below (side 0 = down)
    // Java: BlockRedstoneTorch.isProvidingStrongPower
    static int32_t getStrongPower(bool isLit, int32_t metadata, int32_t side);
};

// ═══════════════════════════════════════════════════════════════════════════
// RedstoneRepeater — Signal delay and amplification.
// Java reference: net.minecraft.block.BlockRedstoneRepeater
//
// Features:
//   - 1-4 tick delay (metadata bits 2-3)
//   - Full signal refresh (outputs 15 regardless of input)
//   - Lockable by adjacent powered repeater
//   - Directional (metadata bits 0-1 = facing NSWE)
// ═══════════════════════════════════════════════════════════════════════════

class RedstoneRepeater {
public:
    // Get delay in ticks from metadata
    // Java: BlockRedstoneRepeater.func_149831_b — delay = (meta >> 2) + 1
    static int32_t getDelay(int32_t metadata) {
        return ((metadata >> 2) & 0x3) + 1;
    }

    // Get facing direction from metadata (0=S, 1=W, 2=N, 3=E)
    static int32_t getFacing(int32_t metadata) {
        return metadata & 0x3;
    }

    // Check if repeater is locked by adjacent powered repeater
    static bool isLocked(int32_t x, int32_t y, int32_t z,
                          int32_t metadata,
                          RedstoneSignal::GetBlockFn getBlock,
                          RedstoneSignal::GetMetaFn getMeta);

    // Output power (always 15 when powered)
    static constexpr int32_t OUTPUT_POWER = 15;
};

// ═══════════════════════════════════════════════════════════════════════════
// RedstoneComparator — Compare or subtract mode.
// Java reference: net.minecraft.block.BlockRedstoneComparator
//
// Features:
//   - Compare mode (default): output = rear if rear >= strongest side
//   - Subtract mode (torch lit): output = max(0, rear - strongest side)
//   - Reads container inventory strength (15 * fullness)
//   - Directional (metadata bits 0-1 = facing)
// ═══════════════════════════════════════════════════════════════════════════

class RedstoneComparator {
public:
    // Mode from metadata bit 2
    static bool isSubtractMode(int32_t metadata) {
        return (metadata & 0x4) != 0;
    }

    // Get facing from metadata
    static int32_t getFacing(int32_t metadata) {
        return metadata & 0x3;
    }

    // Calculate output signal
    // Java: BlockRedstoneComparator.func_149968_f
    static int32_t calculateOutput(int32_t rearSignal, int32_t sideSignal, bool subtractMode) {
        if (subtractMode) {
            return std::max(0, rearSignal - sideSignal);
        }
        // Compare mode: output rear signal only if >= side signal
        return (rearSignal >= sideSignal) ? rearSignal : 0;
    }

    // Calculate container signal strength
    // Java: Container.calcRedstoneFromInventory
    static int32_t calcContainerSignal(int32_t totalItems, int32_t maxItems, int32_t slotCount) {
        if (slotCount == 0) return 0;
        float fillPercent = static_cast<float>(totalItems) / static_cast<float>(maxItems * slotCount);
        return static_cast<int32_t>(fillPercent * 14.0f) + (totalItems > 0 ? 1 : 0);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// PistonMechanics — Piston extension and retraction.
// Java reference: net.minecraft.block.BlockPistonBase
//
// Features:
//   - Normal and sticky variants
//   - Push limit: 12 blocks
//   - Cannot push obsidian, bedrock, extended pistons
//   - Sticky pistons pull blocks on retract
//   - Determines push direction from metadata
// ═══════════════════════════════════════════════════════════════════════════

class PistonMechanics {
public:
    // Java: max 12 blocks can be pushed by a piston
    static constexpr int32_t MAX_PUSH_DISTANCE = 12;

    // Java: BlockPistonBase.getPistonOrientation(int meta) — direction extract
    static int32_t getOrientation(int32_t metadata) {
        return metadata & 0x7;
    }

    // Java: BlockPistonBase.isExtended(int meta)
    static bool isExtended(int32_t metadata) {
        return (metadata & 0x8) != 0;
    }

    // Unpushable block IDs (obsidian, bedrock, barriers, etc.)
    static bool isUnpushable(int32_t blockId) {
        return blockId == 49   // obsidian
            || blockId == 7    // bedrock
            || blockId == 36   // piston extension (moving)
            || blockId == 34   // piston head
            || blockId == 119  // end portal
            || blockId == 120  // end portal frame
            || blockId == 137  // command block
            || blockId == 0;   // air (nothing to push)
    }

    // Java: BlockPistonBase.canPushBlock
    // Returns whether a block can be pushed/pulled by a piston
    static bool canPushBlock(int32_t blockId, bool isDestroying) {
        if (isUnpushable(blockId)) return false;
        // Tile entities cannot be pushed (chests, furnaces, etc.)
        // In Java: block.hasTileEntity(meta)
        // Simplified: common tile entities
        if (blockId == 54 || blockId == 61 || blockId == 62 || // chest, furnace
            blockId == 52 || blockId == 25 || blockId == 84 || // spawner, noteblock, jukebox
            blockId == 116 || blockId == 117 || blockId == 118 || // enchant table, brewery, cauldron
            blockId == 130 || blockId == 138 || blockId == 144 || // ender chest, beacon, skull
            blockId == 154 || blockId == 146) { // hopper, trapped chest
            return false;
        }
        return true;
    }

    // Java: BlockPistonBase.canExtend
    // Check if piston can extend (up to 12 blocks, no unpushable blocks)
    static bool canExtend(int32_t x, int32_t y, int32_t z,
                           int32_t direction,
                           RedstoneSignal::GetBlockFn getBlock);

    // Java: BlockPistonBase.tryExtend
    // Attempt to extend piston, pushing blocks in front
    static bool tryExtend(int32_t x, int32_t y, int32_t z,
                           int32_t direction, bool isSticky,
                           RedstoneSignal::GetBlockFn getBlock,
                           RedstoneSignal::SetMetaFn setMeta,
                           RedstoneSignal::NotifyFn notify);

    // Java: BlockPistonBase.isIndirectlyPowered
    // Check if piston is receiving power from any side except its face
    static bool isIndirectlyPowered(int32_t x, int32_t y, int32_t z,
                                     int32_t direction,
                                     RedstoneSignal::GetBlockFn getBlock,
                                     RedstoneSignal::GetMetaFn getMeta);

    // Java: BlockPistonBase.determineOrientation
    // Determine piston facing based on player placement
    static int32_t determineOrientation(double playerX, double playerY, double playerZ,
                                         int32_t blockX, int32_t blockY, int32_t blockZ,
                                         float playerPitch);
};

// ═══════════════════════════════════════════════════════════════════════════
// PowerSource — Lever, button, and pressure plate logic.
// ═══════════════════════════════════════════════════════════════════════════

class PowerSource {
public:
    // Lever: toggle power permanently
    // Java: BlockLever — metadata bit 3 = powered state
    static bool isLeverPowered(int32_t metadata) {
        return (metadata & 0x8) != 0;
    }

    // Button: momentary power, tick rate varies
    // Java: BlockButton — stone=20 ticks, wood=30 ticks
    static int32_t getButtonDuration(bool isWooden) {
        return isWooden ? 30 : 20;
    }

    // Pressure plate: powered when entity is on it
    // Java: BlockPressurePlate
    static int32_t getPressurePlateSignal(bool hasEntity) {
        return hasEntity ? 15 : 0;
    }

    // Weighted pressure plate: signal based on entity count
    // Java: BlockPressurePlateWeighted
    static int32_t getWeightedSignal(int32_t entityCount, int32_t maxWeight) {
        if (entityCount <= 0) return 0;
        float ratio = static_cast<float>(entityCount) / static_cast<float>(maxWeight);
        int32_t signal = static_cast<int32_t>(ratio * 15.0f) + 1;
        return std::min(signal, 15);
    }

    // Light weighted plate: max weight 15
    static constexpr int32_t LIGHT_MAX_WEIGHT = 15;
    // Heavy weighted plate: max weight 150
    static constexpr int32_t HEAVY_MAX_WEIGHT = 150;
};

} // namespace mccpp
