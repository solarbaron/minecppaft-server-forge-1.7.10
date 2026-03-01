/**
 * ItemInWorldManager.h — Server-side block interaction manager.
 *
 * Java reference: net.minecraft.server.management.ItemInWorldManager (240 lines)
 *
 * Handles all server-side block breaking, placing, and item usage logic.
 * This is the critical bridge between NetHandlerPlayServer packet processing
 * and actual world state modification.
 *
 * Block breaking state machine:
 *   1. onBlockClicked (status 0): Start breaking
 *      - Creative: instant break (tryHarvestBlock), unless sword
 *      - Survival: compute relativeHardness, if >= 1.0 instant break
 *                  else start progressive breaking (isDestroyingBlock = true)
 *   2. updateBlockRemoving (each tick):
 *      - Compute progress = relativeHardness * (ticks + 1)
 *      - Send break animation stage = (int)(progress * 10)
 *      - If progress >= 1.0: harvest block
 *   3. blockRemoving (status 2): Finish breaking
 *      - Progress >= 0.7: instant complete
 *      - Else: queue for delayed completion (receivedFinishDiggingPacket)
 *   4. cancelDestroyingBlock (status 1): Cancel
 *
 * Block interaction priority (activateBlockOrUseItem):
 *   1. If NOT sneaking (or no held item): try block.onBlockActivated
 *   2. If that fails: try item.tryPlaceItemIntoWorld
 *   3. Creative mode preserves item stack size and metadata
 *
 * Thread safety: One manager per player, ticked on server thread.
 * JNI readiness: Simple state machine with clear lifecycle.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// GameType — Mirrors WorldSettings.GameType
// Java: net.minecraft.world.WorldSettings$GameType
// ═══════════════════════════════════════════════════════════════════════════

enum class GameType : int32_t {
    NOT_SET = -1,
    SURVIVAL = 0,
    CREATIVE = 1,
    ADVENTURE = 2,
    SPECTATOR = 3  // Not in 1.7.10 but reserved
};

namespace GameTypeUtil {
    inline bool isCreative(GameType gt) { return gt == GameType::CREATIVE; }
    inline bool isAdventure(GameType gt) { return gt == GameType::ADVENTURE; }
    inline bool isSurvival(GameType gt) { return gt == GameType::SURVIVAL; }

    // Java: GameType.configurePlayerCapabilities(PlayerCapabilities)
    //   CREATIVE: allowFlying=true, isCreativeMode=true, disableDamage=true
    //   ADVENTURE: allowFlying=false, isCreativeMode=false, disableDamage=false
    //   SURVIVAL: allowFlying=false, isCreativeMode=false, disableDamage=false
    inline uint8_t toAbilitiesFlags(GameType gt) {
        switch (gt) {
            case GameType::CREATIVE:
                return 0x01 | 0x04 | 0x08;  // invulnerable | allowFlying | creative
            default:
                return 0x00;
        }
    }

    // Java: GameType.getID() / GameType.getByID()
    inline int32_t toId(GameType gt) { return static_cast<int32_t>(gt); }
    inline GameType fromId(int32_t id) {
        switch (id) {
            case 0: return GameType::SURVIVAL;
            case 1: return GameType::CREATIVE;
            case 2: return GameType::ADVENTURE;
            default: return GameType::NOT_SET;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// ItemInWorldManager — Per-player block interaction state.
// ═══════════════════════════════════════════════════════════════════════════

class ItemInWorldManager {
public:
    // ─── Game mode ───
    GameType gameType = GameType::NOT_SET;

    // ─── Block breaking state ───
    // Java: isDestroyingBlock — actively holding left click on a block
    bool isDestroyingBlock = false;

    // Java: initialDamage — tick count when break started
    int32_t initialDamage = 0;

    // Java: curBlockX/Y/Z — block being broken
    int32_t curBlockX = 0, curBlockY = 0, curBlockZ = 0;

    // Java: curblockDamage — running tick counter
    int32_t curblockDamage = 0;

    // Java: receivedFinishDiggingPacket — queued delayed finish
    bool receivedFinishDiggingPacket = false;

    // Java: posX/Y/Z — queued finish block position
    int32_t posX = 0, posY = 0, posZ = 0;

    // Java: initialBlockDamage — tick count for queued finish
    int32_t initialBlockDamage = 0;

    // Java: durabilityRemainingOnBlock — last sent break animation stage (0-9)
    int32_t durabilityRemainingOnBlock = -1;

    // ─── Game mode methods ───
    bool isCreative() const { return GameTypeUtil::isCreative(gameType); }
    bool isAdventure() const { return GameTypeUtil::isAdventure(gameType); }

    // ─── Block breaking constants ───
    // Java: break animation stages
    static constexpr int32_t MAX_BREAK_STAGE = 9;
    static constexpr float BREAK_PROGRESS_MULTIPLIER = 10.0f;

    // Java: blockRemoving threshold for instant completion
    // if (f >= 0.7f) → instant harvest
    static constexpr float FINISH_THRESHOLD = 0.7f;

    // Java: creative instant break effect
    // AuxSFX ID 2001 = block break particles + sound
    static constexpr int32_t BLOCK_BREAK_SFX = 2001;

    // ─── updateBlockRemoving ───
    // Java: Called every server tick per player
    // Computes break progress and sends animation updates
    //
    // Returns: BlockBreakResult indicating what happened
    enum class BreakTickResult {
        NOTHING,         // No active breaking
        PROGRESS,        // Updated break animation stage
        HARVESTED,       // Block was fully broken
        CANCELLED        // Block disappeared (air)
    };

    struct BreakTickInfo {
        BreakTickResult result = BreakTickResult::NOTHING;
        int32_t blockX = 0, blockY = 0, blockZ = 0;
        int32_t stage = -1;  // Break animation stage (0-9) or -1 for reset
    };

    BreakTickInfo updateBlockRemoving(float relativeHardness, bool isAir) {
        ++curblockDamage;
        BreakTickInfo info;

        if (receivedFinishDiggingPacket) {
            if (isAir) {
                receivedFinishDiggingPacket = false;
                info.result = BreakTickResult::CANCELLED;
                return info;
            }

            int32_t elapsed = curblockDamage - initialBlockDamage;
            float progress = relativeHardness * static_cast<float>(elapsed + 1);
            int32_t stage = static_cast<int32_t>(progress * BREAK_PROGRESS_MULTIPLIER);

            if (stage != durabilityRemainingOnBlock) {
                info.blockX = posX; info.blockY = posY; info.blockZ = posZ;
                info.stage = stage;
                durabilityRemainingOnBlock = stage;
                info.result = BreakTickResult::PROGRESS;
            }

            if (progress >= 1.0f) {
                receivedFinishDiggingPacket = false;
                info.blockX = posX; info.blockY = posY; info.blockZ = posZ;
                info.result = BreakTickResult::HARVESTED;
            }
        } else if (isDestroyingBlock) {
            if (isAir) {
                info.blockX = curBlockX; info.blockY = curBlockY; info.blockZ = curBlockZ;
                info.stage = -1;
                durabilityRemainingOnBlock = -1;
                isDestroyingBlock = false;
                info.result = BreakTickResult::CANCELLED;
                return info;
            }

            int32_t elapsed = curblockDamage - initialDamage;
            float progress = relativeHardness * static_cast<float>(elapsed + 1);
            int32_t stage = static_cast<int32_t>(progress * BREAK_PROGRESS_MULTIPLIER);

            if (stage != durabilityRemainingOnBlock) {
                info.blockX = curBlockX; info.blockY = curBlockY; info.blockZ = curBlockZ;
                info.stage = stage;
                durabilityRemainingOnBlock = stage;
                info.result = BreakTickResult::PROGRESS;
            }
        }

        return info;
    }

    // ─── onBlockClicked ───
    // Java: Called when player starts breaking (status 0)
    //
    // Returns: BlockClickResult
    enum class BlockClickResult {
        BLOCKED_ADVENTURE, // Adventure mode, wrong tool
        INSTANT_BREAK,     // Creative or hardness >= 1.0
        START_BREAKING,    // Started progressive break
        EXTINGUISHED       // Only extinguished fire (creative)
    };

    BlockClickResult onBlockClicked(int32_t x, int32_t y, int32_t z, int32_t face,
                                     float relativeHardness, bool isAirBlock)
    {
        if (isAdventure()) {
            return BlockClickResult::BLOCKED_ADVENTURE;
        }

        if (isCreative()) {
            // Creative: instant break (except extinguish fire first)
            // Caller should: extinguishFire, then tryHarvestBlock
            return BlockClickResult::INSTANT_BREAK;
        }

        // Survival: start breaking
        initialDamage = curblockDamage;

        if (!isAirBlock && relativeHardness >= 1.0f) {
            // Instant break (e.g., tall grass, flowers)
            return BlockClickResult::INSTANT_BREAK;
        }

        // Start progressive breaking
        isDestroyingBlock = true;
        curBlockX = x; curBlockY = y; curBlockZ = z;
        int32_t stage = static_cast<int32_t>(relativeHardness * BREAK_PROGRESS_MULTIPLIER);
        durabilityRemainingOnBlock = stage;

        return BlockClickResult::START_BREAKING;
    }

    // ─── blockRemoving ───
    // Java: Called when player finishes breaking (status 2)
    //
    // Returns: true if block should be harvested immediately
    bool blockRemoving(int32_t x, int32_t y, int32_t z, float relativeHardness) {
        if (x != curBlockX || y != curBlockY || z != curBlockZ) {
            return false;
        }

        int32_t elapsed = curblockDamage - initialDamage;
        float progress = relativeHardness * static_cast<float>(elapsed + 1);

        if (progress >= FINISH_THRESHOLD) {
            // Instant harvest
            isDestroyingBlock = false;
            durabilityRemainingOnBlock = -1;
            return true;  // Caller should: tryHarvestBlock
        }

        if (!receivedFinishDiggingPacket) {
            // Queue for delayed completion
            isDestroyingBlock = false;
            receivedFinishDiggingPacket = true;
            posX = x; posY = y; posZ = z;
            initialBlockDamage = initialDamage;
        }

        return false;
    }

    // ─── cancelDestroyingBlock ───
    // Java: Called when player cancels breaking (status 1)
    void cancelDestroyingBlock() {
        isDestroyingBlock = false;
        durabilityRemainingOnBlock = -1;
        // Caller should: send destroyBlockInWorldPartially(entityId, curBlockX/Y/Z, -1)
    }

    // ─── tryHarvestBlock ───
    // Java: The actual block harvesting logic
    //   1. Adventure mode: check tool exemption
    //   2. Creative: block sword on non-air blocks
    //   3. Play break particles/sound (SFX 2001, data=blockId+(meta<<12))
    //   4. removeBlock: onBlockHarvested → setBlockToAir → onBlockDestroyedByPlayer
    //   5. Survival: tool durability damage, drop items if harvestable
    //
    // Constants for SFX data encoding:
    static constexpr int32_t BLOCK_BREAK_SFX_META_SHIFT = 12;

    static int32_t encodeBreakSfxData(int32_t blockId, int32_t metadata) {
        return blockId + (metadata << BLOCK_BREAK_SFX_META_SHIFT);
    }

    // ─── activateBlockOrUseItem interaction priority ───
    // Java: activateBlockOrUseItem(player, world, item, x, y, z, face, hitX, hitY, hitZ)
    //   1. If (!sneaking || heldItem==null): block.onBlockActivated → return true
    //   2. If item != null: item.tryPlaceItemIntoWorld
    //      Creative: preserve stackSize and metadata
    //   3. Return whether item was used
    //
    // This is the logic that determines whether right-clicking opens a chest,
    // places a block, or uses an item.
    static constexpr float FACE_HIT_SCALE = 1.0f / 16.0f;

    // ─── tryUseItem ───
    // Java: tryUseItem(player, world, itemStack) — right-click in air
    //   Uses item.useItemRightClick
    //   Creative mode preserves stack size and metadata
    //   Returns true if item state changed

    // ─── Gamemode transition ───
    void setGameType(GameType type) {
        gameType = type;
    }

    void initializeGameType(GameType defaultType) {
        if (gameType == GameType::NOT_SET) {
            gameType = defaultType;
        }
    }
};

} // namespace mccpp
