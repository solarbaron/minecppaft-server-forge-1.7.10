/**
 * TeleporterPortal.h — Nether/End portal search, creation, and entity teleport.
 *
 * Java reference:
 *   - net.minecraft.world.Teleporter (354 lines)
 *   - net.minecraft.world.Teleporter$PortalPosition (cached coords)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TELEPORTER (Teleporter)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * placeInPortal(entity) — main entry point:
 *   - End (dimension 1): build 5×5 obsidian platform, teleport directly
 *   - Nether/Overworld:
 *     1. Try placeInExistingPortal (cache or 128-block search)
 *     2. If no portal found: makePortal, then placeInExistingPortal
 *
 * ═══════════════════════════════════════════════════════════════════════
 * placeInExistingPortal — find nearest portal block
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Cache: LongHashMap keyed by ChunkCoordIntPair.chunkXZ2Int(x, z)
 *   - Hit: use cached position, update lastUpdateTime
 *   - Miss: search 128-block XZ radius, all Y levels (top to bottom)
 *     - Find portal blocks, walk down to lowest portal Y
 *     - Track nearest by 3D distance² to entity
 *
 * After finding portal:
 *   1. Cache the position
 *   2. Determine portal direction by checking adjacent portal blocks:
 *      - x-1 has portal → direction 2
 *      - x+1 has portal → direction 0
 *      - z-1 has portal → direction 3
 *      - z+1 has portal → direction 1
 *   3. Check exit space (2 blocks tall on each side of portal)
 *      - If both sides blocked, flip direction by 180°
 *   4. Position entity with offset based on clear space:
 *      - One side clear → offset toward clear side
 *      - Both blocked → center perpendicular to portal
 *   5. Rotate entity velocity based on Direction tables
 *   6. Update entity yaw: f - getTeleportDirection*90 + portalDir*90
 *
 * Direction constants (Java: net.minecraft.util.Direction):
 *   rotateLeft[4], rotateOpposite[4], enderEyeMetaToDirection[4]
 *   offsetX[4], offsetZ[4]
 *
 * ═══════════════════════════════════════════════════════════════════════
 * makePortal — create a new nether portal
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Two-pass placement algorithm within 16-block radius:
 *
 * Pass 1 (wide portal):
 *   - Scan all air columns (walk down to bottom of air)
 *   - For each of 4 rotations (random start):
 *     Check 3×4×4 space: 3 wide, 4 deep, 4 tall
 *     - n3 < 0 (below portal): must be solid
 *     - n3 >= 0 (portal area): must be air
 *   - Track nearest valid position by distance² to entity
 *
 * Pass 2 (narrow portal, if pass 1 failed):
 *   - Same scan but only check 1×4 space (2 rotations only)
 *   - Relaxed validation
 *
 * Fallback (if both passes fail):
 *   - Clamp Y to 70..actualHeight-10
 *   - Build 3×2×3 obsidian platform with air above
 *
 * Portal construction:
 *   - 4×4 frame: outer ring = obsidian, inner = portal blocks
 *   - Notify neighbors of block changes
 *   - Orientation based on selected direction (n25, n26 multipliers)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * Cache cleanup
 * ═══════════════════════════════════════════════════════════════════════
 *
 * removeStalePortalLocations(totalWorldTime):
 *   - Every 100 ticks: iterate cache
 *   - Remove entries where lastUpdateTime < totalWorldTime - 600
 *
 * Thread safety: Teleporter operations on server thread.
 * JNI readiness: Simple coordinate structs, LongHashMap cache.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Teleporter Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TeleporterConstants {
    // ─── Dimension IDs ───
    static constexpr int32_t OVERWORLD = 0;
    static constexpr int32_t NETHER = -1;
    static constexpr int32_t END = 1;

    // ─── Portal search radius ───
    // Java: int n = 128; search entity.posX ± 128, entity.posZ ± 128
    static constexpr int32_t SEARCH_RADIUS = 128;

    // ─── Portal creation radius ───
    // Java: int n12 = 16; search within 16 blocks
    static constexpr int32_t CREATE_RADIUS = 16;

    // ─── Nether coordinate scale ───
    // Java: Overworld coords / 8 = Nether coords
    static constexpr double NETHER_SCALE = 8.0;

    // ─── End platform ───
    // Java: 5×5 obsidian at entity position, -2..+2 in XZ
    static constexpr int32_t END_PLATFORM_RADIUS = 2;
    // Java: air from Y to Y+2 (3 blocks tall clear space)
    static constexpr int32_t END_PLATFORM_HEIGHT = 3;

    // ─── Portal frame dimensions ───
    // Java: 4 columns wide × 4 blocks tall (including obsidian frame)
    // Inner portal area: 2 wide × 3 tall
    static constexpr int32_t PORTAL_WIDTH = 4;
    static constexpr int32_t PORTAL_HEIGHT = 4;

    // ─── makePortal pass 1: wide check ───
    // Java: for (n5 = 0; n5 < 3; ...) for (n4 = 0; n4 < 4; ...)
    //       for (n3 = -1; n3 < 4; ...)
    static constexpr int32_t PASS1_DEPTH = 3;
    static constexpr int32_t PASS1_WIDTH = 4;
    static constexpr int32_t PASS1_MIN_Y = -1;
    static constexpr int32_t PASS1_MAX_Y = 4;

    // ─── makePortal pass 2: narrow check ───
    // Java: for (i = 0; i < 4; ...) for (n4 = -1; n4 < 4; ...)
    static constexpr int32_t PASS2_WIDTH = 4;
    static constexpr int32_t PASS2_MIN_Y = -1;
    static constexpr int32_t PASS2_MAX_Y = 4;

    // ─── Fallback Y clamp ───
    // Java: if (n17 < 70) n17 = 70;
    //       if (n17 > actualHeight - 10) n17 = actualHeight - 10;
    static constexpr int32_t FALLBACK_MIN_Y = 70;
    static constexpr int32_t FALLBACK_Y_MARGIN = 10;

    // ─── Cache expiry ───
    // Java: removeStalePortalLocations — every 100 ticks, expire after 600
    static constexpr int64_t CACHE_CHECK_INTERVAL = 100;
    static constexpr int64_t CACHE_EXPIRY_TICKS = 600;

    // ─── Block IDs ───
    static constexpr int32_t OBSIDIAN_ID = 49;
    static constexpr int32_t PORTAL_ID = 90;
    static constexpr int32_t AIR_ID = 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Direction Constants
// Java: net.minecraft.util.Direction
// Used for portal orientation and velocity rotation
// ═══════════════════════════════════════════════════════════════════════════

namespace DirectionConstants {
    // Java: Direction.offsetX = {0, -1, 0, 1}
    static constexpr int32_t offsetX[] = {0, -1, 0, 1};
    // Java: Direction.offsetZ = {1, 0, -1, 0}
    static constexpr int32_t offsetZ[] = {1, 0, -1, 0};

    // Java: Direction.rotateLeft = {3, 0, 1, 2}
    // Turn left: 0→3, 1→0, 2→1, 3→2
    static constexpr int32_t rotateLeft[] = {3, 0, 1, 2};

    // Java: Direction.rotateRight = {1, 2, 3, 0}
    static constexpr int32_t rotateRight[] = {1, 2, 3, 0};

    // Java: Direction.rotateOpposite = {2, 3, 0, 1}
    // 180° turn: 0→2, 1→3, 2→0, 3→1
    static constexpr int32_t rotateOpposite[] = {2, 3, 0, 1};

    // Java: Direction.enderEyeMetaToDirection = {1, 2, 3, 0}
    static constexpr int32_t enderEyeMetaToDirection[] = {1, 2, 3, 0};

    // ─── Portal direction detection ───
    // Java: check portal blocks adjacent to found portal position
    // x-1 has portal → direction 2 (south)
    // x+1 has portal → direction 0 (north)
    // z-1 has portal → direction 3 (west)
    // z+1 has portal → direction 1 (east)
    static constexpr int32_t DIR_FROM_PORTAL_XMINUS = 2;
    static constexpr int32_t DIR_FROM_PORTAL_XPLUS = 0;
    static constexpr int32_t DIR_FROM_PORTAL_ZMINUS = 3;
    static constexpr int32_t DIR_FROM_PORTAL_ZPLUS = 1;
}

// ═══════════════════════════════════════════════════════════════════════════
// Portal Position Cache Entry
// Java: Teleporter$PortalPosition extends ChunkCoordinates
// ═══════════════════════════════════════════════════════════════════════════

struct PortalCacheEntry {
    int32_t posX = 0;
    int32_t posY = 0;
    int32_t posZ = 0;
    int64_t lastUpdateTime = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// Portal Placement Result
// ═══════════════════════════════════════════════════════════════════════════

struct PortalPlacementResult {
    double entityX = 0.0;
    double entityY = 0.0;
    double entityZ = 0.0;
    float entityYaw = 0.0f;
    float entityPitch = 0.0f;
    double motionX = 0.0;
    double motionY = 0.0;
    double motionZ = 0.0;
    bool success = false;
};

// ═══════════════════════════════════════════════════════════════════════════
// Portal frame: which blocks are obsidian vs portal
// Java: n3 = n8 == 0 || n8 == 3 || n7 == -1 || n7 == 3 ? obsidian : portal
// ═══════════════════════════════════════════════════════════════════════════

inline bool isFrameBlock(int32_t col, int32_t row) {
    // col 0..3, row -1..3
    // Frame (obsidian) on: first/last column OR bottom/top row
    return col == 0 || col == 3 || row == -1 || row == 3;
}

// ═══════════════════════════════════════════════════════════════════════════
// Velocity rotation for portal direction change
// Java: entity.motionX/Z transformed based on portal/teleport directions
// ═══════════════════════════════════════════════════════════════════════════

struct VelocityRotation {
    float forwardMultX = 0.0f;
    float forwardMultZ = 0.0f;
    float lateralMultX = 0.0f;
    float lateralMultZ = 0.0f;
};

inline VelocityRotation computeVelocityRotation(int32_t portalDir, int32_t teleportDir) {
    using namespace DirectionConstants;
    VelocityRotation result;

    if (portalDir == teleportDir) {
        // Same direction
        result.forwardMultX = 1.0f;
        result.forwardMultZ = 1.0f;
    } else if (portalDir == rotateOpposite[teleportDir]) {
        // Opposite direction
        result.forwardMultX = -1.0f;
        result.forwardMultZ = -1.0f;
    } else if (portalDir == enderEyeMetaToDirection[teleportDir]) {
        // 90° left
        result.lateralMultX = 1.0f;
        result.lateralMultZ = -1.0f;
    } else {
        // 90° right
        result.lateralMultX = -1.0f;
        result.lateralMultZ = 1.0f;
    }

    return result;
}

// ═══════════════════════════════════════════════════════════════════════════
// Yaw rotation for portal direction change
// Java: entity.rotationYaw = f - (float)(n11 * 90) + (float)(n10 * 90)
// ═══════════════════════════════════════════════════════════════════════════

inline float computePortalYaw(float originalYaw, int32_t teleportDir, int32_t portalDir) {
    return originalYaw - static_cast<float>(teleportDir * 90) + static_cast<float>(portalDir * 90);
}

// ═══════════════════════════════════════════════════════════════════════════
// ChunkCoordIntPair hash for portal cache
// Java: ChunkCoordIntPair.chunkXZ2Int(x, z)
// ═══════════════════════════════════════════════════════════════════════════

inline int64_t portalCacheKey(int32_t x, int32_t z) {
    return static_cast<int64_t>(x) & 0xFFFFFFFFL |
           (static_cast<int64_t>(z) & 0xFFFFFFFFL) << 32;
}

// ═══════════════════════════════════════════════════════════════════════════
// makePortal rotation helpers
// Java: n7 = n8 % 2; n6 = 1 - n7;
//       if (n8 % 4 >= 2) { n7 = -n7; n6 = -n6; }
// ═══════════════════════════════════════════════════════════════════════════

struct PortalOrientation {
    int32_t dx;  // primary axis modifier
    int32_t dz;  // secondary axis modifier
};

inline PortalOrientation getPortalOrientation(int32_t rotationIndex) {
    int32_t dx = rotationIndex % 2;
    int32_t dz = 1 - dx;
    if (rotationIndex % 4 >= 2) {
        dx = -dx;
        dz = -dz;
    }
    return {dx, dz};
}

} // namespace mccpp
