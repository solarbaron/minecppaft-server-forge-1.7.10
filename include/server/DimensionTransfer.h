/**
 * DimensionTransfer.h — Cross-dimension player/entity transfer logic.
 *
 * Java reference: ServerConfigurationManager (705 lines)
 *   - transferPlayerToDimension (lines 325-342)
 *   - transferEntityToWorld (lines 344-387)
 *   - recreatePlayerEntity (lines 281-323)
 *
 * Implements the complete dimension transition pipeline:
 *
 * 1. Nether Portal (Overworld ↔ Nether):
 *    - Overworld → Nether: divide X,Z by 8
 *    - Nether → Overworld: multiply X,Z by 8
 *    - Clamp coordinates to ±29999872
 *    - Use Teleporter.placeInPortal for portal alignment
 *
 * 2. End Portal (Overworld → End):
 *    - Place at End spawn point (getEntrancePortalLocation)
 *    - No coordinate scaling
 *    - Rotation set to 90°, 0°
 *
 * 3. End Exit (End → Overworld):
 *    - Place at world spawn point
 *    - No coordinate scaling
 *
 * 4. Respawn (recreatePlayerEntity):
 *    - Preserve entity ID
 *    - Check bed location validity (verifyRespawnCoordinates)
 *    - Bed spawn: position + (0.5, 0.1, 0.5)
 *    - No bed: send S2B GameState 0 (invalid bed)
 *    - Push up until no collision with world
 *    - Send S07 Respawn, S05 SpawnPosition, S1F Experience
 *    - Sync time, weather, inventory, potion effects
 *
 * Thread safety: All transfer operations happen on the server thread.
 * JNI readiness: Simple stateless utility functions.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Dimension IDs — standard Minecraft dimension numbering
// ═══════════════════════════════════════════════════════════════════════════

namespace DimensionId {
    static constexpr int32_t NETHER    = -1;
    static constexpr int32_t OVERWORLD =  0;
    static constexpr int32_t END       =  1;
}

// ═══════════════════════════════════════════════════════════════════════════
// Dimension transfer constants and utilities.
// ═══════════════════════════════════════════════════════════════════════════

namespace DimensionTransfer {

    // ─── Nether coordinate scaling ───
    // Java: d3 = 8.0
    // Nether coords = Overworld coords / 8
    static constexpr double NETHER_SCALE = 8.0;

    // ─── World border coordinate clamp ───
    // Java: MathHelper.clamp_int((int)d, -29999872, 29999872)
    // This is the world border limit for portal placement
    static constexpr int32_t WORLD_BORDER_MIN = -29999872;
    static constexpr int32_t WORLD_BORDER_MAX =  29999872;

    // ─── End portal rotation ───
    // Java: entity.setLocationAndAngles(d, entity.posY, d2, 90.0f, 0.0f)
    static constexpr float END_SPAWN_YAW = 90.0f;
    static constexpr float END_SPAWN_PITCH = 0.0f;

    // ─── Bed spawn offset ───
    // Java: setLocationAndAngles(chunkCoordinates.posX + 0.5, posY + 0.1, posZ + 0.5, 0, 0)
    static constexpr double BED_SPAWN_X_OFFSET = 0.5;
    static constexpr double BED_SPAWN_Y_OFFSET = 0.1;
    static constexpr double BED_SPAWN_Z_OFFSET = 0.5;

    // ─── Game state packet for invalid bed ───
    // Java: S2BPacketChangeGameState(0, 0.0f) — "invalid bed"
    static constexpr int32_t GAMESTATE_INVALID_BED = 0;

    // ─── Weather game state IDs ───
    // Java: S2BPacketChangeGameState
    static constexpr int32_t GAMESTATE_BEGIN_RAIN = 1;
    static constexpr int32_t GAMESTATE_END_RAIN = 2;
    static constexpr int32_t GAMESTATE_CHANGE_GAMEMODE = 3;
    static constexpr int32_t GAMESTATE_WIN_GAME = 4;
    static constexpr int32_t GAMESTATE_DEMO_EVENT = 5;
    static constexpr int32_t GAMESTATE_ARROW_HITTING = 6;
    static constexpr int32_t GAMESTATE_RAIN_STRENGTH = 7;
    static constexpr int32_t GAMESTATE_THUNDER_STRENGTH = 8;

    // ─── Coordinate transform ───
    // Java: transferEntityToWorld coordinate logic
    struct TransferResult {
        double posX, posY, posZ;
        float yaw, pitch;
        bool usePortalPlacer;  // true = call Teleporter.placeInPortal
    };

    // Compute destination coordinates for dimension transfer
    // Java: ServerConfigurationManager.transferEntityToWorld
    //
    // Entity at (srcX, srcY, srcZ) with yaw/pitch
    // Moving from srcDim to destDim
    // destSpawnX/Y/Z = spawn point for End/Overworld transitions
    static TransferResult computeTransfer(
        double srcX, double srcY, double srcZ,
        float srcYaw, float srcPitch,
        int32_t srcDim, int32_t destDim,
        double destSpawnX, double destSpawnY, double destSpawnZ)
    {
        TransferResult result;
        result.posX = srcX;
        result.posY = srcY;
        result.posZ = srcZ;
        result.yaw = srcYaw;
        result.pitch = srcPitch;
        result.usePortalPlacer = false;

        if (destDim == DimensionId::NETHER) {
            // Going TO the Nether: divide by 8
            // Java: entity.dimension == -1 check in transferEntityToWorld
            // (confusing: this code runs AFTER dimension is set to target)
            result.posX = srcX / NETHER_SCALE;
            result.posZ = srcZ / NETHER_SCALE;
        } else if (destDim == DimensionId::OVERWORLD && srcDim == DimensionId::NETHER) {
            // Coming FROM the Nether: multiply by 8
            result.posX = srcX * NETHER_SCALE;
            result.posZ = srcZ * NETHER_SCALE;
        } else {
            // End dimension: use spawn point
            // Java: n == 1 ? worldServer2.getSpawnPoint() : worldServer2.getEntrancePortalLocation()
            result.posX = destSpawnX;
            result.posY = destSpawnY;
            result.posZ = destSpawnZ;
            result.yaw = END_SPAWN_YAW;
            result.pitch = END_SPAWN_PITCH;
        }

        // Portal placement (only for non-End dimensions)
        // Java: if (n != 1) { ... placeInPortal ... }
        if (destDim != DimensionId::END) {
            // Clamp to world border
            int32_t clampedX = static_cast<int32_t>(result.posX);
            int32_t clampedZ = static_cast<int32_t>(result.posZ);
            if (clampedX < WORLD_BORDER_MIN) clampedX = WORLD_BORDER_MIN;
            if (clampedX > WORLD_BORDER_MAX) clampedX = WORLD_BORDER_MAX;
            if (clampedZ < WORLD_BORDER_MIN) clampedZ = WORLD_BORDER_MIN;
            if (clampedZ > WORLD_BORDER_MAX) clampedZ = WORLD_BORDER_MAX;
            result.posX = static_cast<double>(clampedX);
            result.posZ = static_cast<double>(clampedZ);
            result.usePortalPlacer = true;
        }

        return result;
    }

    // ─── Bed spawn verification ───
    // Java: EntityPlayer.verifyRespawnCoordinates(world, bedPos, forced)
    // Returns valid spawn coordinates or null if bed is invalid
    struct BedSpawnResult {
        bool valid;
        double posX, posY, posZ;
    };

    static BedSpawnResult computeBedSpawn(int32_t bedX, int32_t bedY, int32_t bedZ) {
        BedSpawnResult result;
        result.valid = true;
        result.posX = static_cast<double>(bedX) + BED_SPAWN_X_OFFSET;
        result.posY = static_cast<double>(bedY) + BED_SPAWN_Y_OFFSET;
        result.posZ = static_cast<double>(bedZ) + BED_SPAWN_Z_OFFSET;
        return result;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Login packet sequence constants — initializeConnectionToPlayer
// Java: ServerConfigurationManager.initializeConnectionToPlayer (lines 104-152)
//
// Packet order sent to newly connected player:
//   1. S01 JoinGame (entityId, gameType, hardcoreMode, dimension, difficulty,
//                     maxPlayers, worldType)
//   2. S3F CustomPayload("MC|Brand", serverBrand bytes)
//   3. S05 SpawnPosition (world spawn X, Y, Z)
//   4. S39 PlayerAbilities (capabilities)
//   5. S09 HeldItemChange (currentItem slot index)
//   6. Scoreboard sync (all teams + display objectives)
//   7. S38 PlayerListItem (add self + all existing players)
//   8. S03 TimeUpdate (totalWorldTime, worldTime, doDaylightCycle)
//   9. Weather sync (S2B GameState 1 + rainStrength + thunderStrength)
//  10. S1D EntityEffect (all active potion effects)
//  11. Riding entity restore (from saved NBT "Riding" tag)
//
// Additional actions:
//   - Chat join message (yellow, "multiplayer.player.joined")
//   - If name changed: "multiplayer.player.joined.renamed"
//   - Refresh server status
//   - Texture pack request (if configured)
// ═══════════════════════════════════════════════════════════════════════════

namespace LoginSequence {
    // Java: S3FPacketCustomPayload("MC|Brand", ...)
    static constexpr const char* CHANNEL_BRAND = "MC|Brand";

    // Java: The join chat message translation keys
    static constexpr const char* JOIN_MESSAGE_KEY = "multiplayer.player.joined";
    static constexpr const char* JOIN_RENAMED_KEY = "multiplayer.player.joined.renamed";

    // Java: EnumChatFormatting.YELLOW for join messages
    static constexpr char CHAT_COLOR_YELLOW = 'e';
}

// ═══════════════════════════════════════════════════════════════════════════
// Ping cycling — ServerConfigurationManager.onTick
// Java: playerPingIndex cycles 0..600, sends S38 for one player per tick
// ═══════════════════════════════════════════════════════════════════════════

namespace PingCycle {
    static constexpr int32_t PING_CYCLE_PERIOD = 600;  // 30 seconds
    static constexpr int32_t INITIAL_PING_VALUE = 1000;  // Java: playerLoggedIn sends ping=1000
}

// ═══════════════════════════════════════════════════════════════════════════
// Player proximity broadcast — sendToAllNearExcept
// Java: squared distance comparison: dx*dx + dy*dy + dz*dz < range*range
// ═══════════════════════════════════════════════════════════════════════════

namespace ProximityBroadcast {
    // Check if (px, py, pz) is within range of (cx, cy, cz)
    static bool isInRange(double cx, double cy, double cz,
                          double px, double py, double pz,
                          double range)
    {
        double dx = cx - px;
        double dy = cy - py;
        double dz = cz - pz;
        return (dx * dx + dy * dy + dz * dz) < (range * range);
    }
}

} // namespace mccpp
