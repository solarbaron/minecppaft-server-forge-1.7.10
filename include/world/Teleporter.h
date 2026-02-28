/**
 * Teleporter.h — Nether/End portal search, creation, and teleportation.
 *
 * Java reference: net.minecraft.world.Teleporter
 *
 * Algorithm:
 *   1. End dimension: Create flat 5×5 obsidian platform, place entity on top.
 *   2. placeInExistingPortal: Search 128-block XZ radius for portal blocks.
 *      Cache results in long-keyed hash map (chunk coord pair key).
 *      Detect portal orientation from adjacent portal blocks.
 *      Rotate entity motion based on entry→exit direction difference.
 *   3. makePortal: Two-pass search for valid portal placement:
 *      Pass 1: 3×4×4 clear area with solid floor, 4 rotations
 *      Pass 2: 1×4×4 fallback, 2 rotations only
 *      Build obsidian frame (4×5) with portal blocks inside.
 *   4. Cache management: Stale portal positions expire after 600 ticks.
 *      Cleanup runs every 100 ticks.
 *
 * Block IDs used: obsidian(49), portal(90), air(0)
 *
 * Thread safety: Called from world tick thread.
 * JNI readiness: Simple data, standard containers.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Direction table — Java net.minecraft.util.Direction constants.
// ═══════════════════════════════════════════════════════════════════════════

namespace Direction {
    // Java: Direction.offsetX / offsetZ — horizontal offsets for 4 directions
    // Dirs: 0=south(+Z), 1=west(-X), 2=north(-Z), 3=east(+X)
    constexpr int32_t offsetX[4] = { 0, -1,  0,  1};
    constexpr int32_t offsetZ[4] = { 1,  0, -1,  0};
    constexpr int32_t rotateLeft[4] = {1, 2, 3, 0};
    constexpr int32_t rotateOpposite[4] = {2, 3, 0, 1};
    constexpr int32_t enderEyeMetaToDirection[4] = {3, 0, 1, 2};
}

// ═══════════════════════════════════════════════════════════════════════════
// Portal position cache entry.
// Java reference: net.minecraft.world.Teleporter$PortalPosition
// ═══════════════════════════════════════════════════════════════════════════

struct PortalPosition {
    int32_t x, y, z;
    int64_t lastUpdateTime;
};

// ═══════════════════════════════════════════════════════════════════════════
// Teleporter — Portal search, creation, and entity placement.
// Java reference: net.minecraft.world.Teleporter
// ═══════════════════════════════════════════════════════════════════════════

class Teleporter {
public:
    // Block interaction callbacks
    using GetBlockFn = std::function<int32_t(int32_t, int32_t, int32_t)>;
    using SetBlockFn = std::function<void(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t)>;
    using IsAirFn = std::function<bool(int32_t, int32_t, int32_t)>;
    using IsSolidFn = std::function<bool(int32_t, int32_t, int32_t)>;

    static constexpr int32_t PORTAL_BLOCK = 90;
    static constexpr int32_t OBSIDIAN_BLOCK = 49;
    static constexpr int32_t AIR_BLOCK = 0;

    Teleporter() = default;

    // ─── Entity teleport result ───

    struct TeleportResult {
        double x, y, z;
        float yaw, pitch;
        double motionX, motionY, motionZ;
        bool success;
    };

    // ─── End dimension: flat obsidian platform ───

    // Java: placeInPortal — dimension 1 special case
    struct BlockPlacement {
        int32_t x, y, z;
        int32_t blockId;
    };

    static std::vector<BlockPlacement> createEndPlatform(int32_t entityX, int32_t entityY, int32_t entityZ) {
        std::vector<BlockPlacement> blocks;
        int32_t baseY = entityY - 1;
        for (int32_t dx = -2; dx <= 2; ++dx) {
            for (int32_t dz = -2; dz <= 2; ++dz) {
                for (int32_t dy = -1; dy < 3; ++dy) {
                    bool isFloor = (dy < 0);
                    blocks.push_back({
                        entityX + dz + dx * 0,  // Java: n + j*1 + i*0
                        baseY + dy,
                        entityZ + dz * 0 - dx,  // Java: n3 + j*0 - i*1
                        isFloor ? OBSIDIAN_BLOCK : AIR_BLOCK
                    });
                }
            }
        }
        return blocks;
    }

    // ─── Portal search ───

    // Java: placeInExistingPortal → searches 128-block radius
    struct SearchResult {
        bool found;
        int32_t portalX, portalY, portalZ;
        int32_t exitDirection;  // -1 = unknown, 0-3 = facing
    };

    SearchResult findNearestPortal(double entityX, double entityY, double entityZ,
                                     GetBlockFn getBlock, int32_t worldHeight,
                                     int64_t worldTime) {
        int32_t eX = floorD(entityX);
        int32_t eZ = floorD(entityZ);
        int64_t cacheKey = chunkKey(eX, eZ);

        // Check cache first
        auto cacheIt = portalCache_.find(cacheKey);
        if (cacheIt != portalCache_.end()) {
            cacheIt->second.lastUpdateTime = worldTime;
            return {true, cacheIt->second.x, cacheIt->second.y, cacheIt->second.z,
                    detectDirection(cacheIt->second.x, cacheIt->second.y, cacheIt->second.z, getBlock)};
        }

        // Search 128-block radius
        constexpr int32_t RANGE = 128;
        double bestDist = -1.0;
        int32_t bestX = 0, bestY = 0, bestZ = 0;

        for (int32_t x = eX - RANGE; x <= eX + RANGE; ++x) {
            double dx = static_cast<double>(x) + 0.5 - entityX;
            for (int32_t z = eZ - RANGE; z <= eZ + RANGE; ++z) {
                double dz = static_cast<double>(z) + 0.5 - entityZ;
                for (int32_t y = worldHeight - 1; y >= 0; --y) {
                    if (getBlock(x, y, z) != PORTAL_BLOCK) continue;
                    // Find bottom of portal column
                    while (y > 0 && getBlock(x, y - 1, z) == PORTAL_BLOCK) --y;
                    double dy = static_cast<double>(y) + 0.5 - entityY;
                    double dist = dx * dx + dy * dy + dz * dz;
                    if (bestDist < 0.0 || dist < bestDist) {
                        bestDist = dist;
                        bestX = x; bestY = y; bestZ = z;
                    }
                }
            }
        }

        if (bestDist >= 0.0) {
            // Cache result
            portalCache_[cacheKey] = {bestX, bestY, bestZ, worldTime};
            cacheKeys_.push_back(cacheKey);
            return {true, bestX, bestY, bestZ,
                    detectDirection(bestX, bestY, bestZ, getBlock)};
        }
        return {false, 0, 0, 0, -1};
    }

    // ─── Portal creation ───

    struct PortalBuild {
        std::vector<BlockPlacement> blocks;
        int32_t portalX, portalY, portalZ;
        int32_t direction;
    };

    // Java: makePortal — two-pass portal placement search
    PortalBuild buildPortal(double entityX, double entityY, double entityZ,
                              IsAirFn isAir, IsSolidFn isSolid,
                              int32_t worldHeight, int32_t randomDir) {
        PortalBuild result;
        constexpr int32_t RANGE = 16;
        int32_t eX = floorD(entityX);
        int32_t eY = floorD(entityY);
        int32_t eZ = floorD(entityZ);

        double bestDist = -1.0;
        int32_t bestX = eX, bestY = eY, bestZ = eZ;
        int32_t bestDir = 0;

        // Pass 1: Find 3×4×4 area with solid floor
        for (int32_t x = eX - RANGE; x <= eX + RANGE; ++x) {
            double dx = static_cast<double>(x) + 0.5 - entityX;
            for (int32_t z = eZ - RANGE; z <= eZ + RANGE; ++z) {
                double dz = static_cast<double>(z) + 0.5 - entityZ;
                for (int32_t y = worldHeight - 1; y >= 0; --y) {
                    if (!isAir(x, y, z)) continue;
                    while (y > 0 && isAir(x, y - 1, z)) --y;

                    for (int32_t r = randomDir; r < randomDir + 4; ++r) {
                        int32_t dirX = r % 2;
                        int32_t dirZ = 1 - dirX;
                        if (r % 4 >= 2) { dirX = -dirX; dirZ = -dirZ; }

                        bool valid = true;
                        for (int32_t depth = 0; depth < 3 && valid; ++depth) {
                            for (int32_t width = 0; width < 4 && valid; ++width) {
                                for (int32_t height = -1; height < 4 && valid; ++height) {
                                    int32_t bx = x + (width - 1) * dirX + depth * dirZ;
                                    int32_t by = y + height;
                                    int32_t bz = z + (width - 1) * dirZ - depth * dirX;
                                    if (height < 0) {
                                        if (!isSolid(bx, by, bz)) valid = false;
                                    } else {
                                        if (!isAir(bx, by, bz)) valid = false;
                                    }
                                }
                            }
                        }
                        if (!valid) continue;

                        double dy = static_cast<double>(y) + 0.5 - entityY;
                        double dist = dx * dx + dy * dy + dz * dz;
                        if (bestDist < 0.0 || dist < bestDist) {
                            bestDist = dist;
                            bestX = x; bestY = y; bestZ = z;
                            bestDir = r % 4;
                        }
                    }
                }
            }
        }

        // Pass 2: Fallback — 1×4×4 area
        if (bestDist < 0.0) {
            for (int32_t x = eX - RANGE; x <= eX + RANGE; ++x) {
                double dx = static_cast<double>(x) + 0.5 - entityX;
                for (int32_t z = eZ - RANGE; z <= eZ + RANGE; ++z) {
                    double dz = static_cast<double>(z) + 0.5 - entityZ;
                    for (int32_t y = worldHeight - 1; y >= 0; --y) {
                        if (!isAir(x, y, z)) continue;
                        while (y > 0 && isAir(x, y - 1, z)) --y;

                        for (int32_t r = randomDir; r < randomDir + 2; ++r) {
                            int32_t dirX = r % 2;
                            int32_t dirZ = 1 - dirX;

                            bool valid = true;
                            for (int32_t width = 0; width < 4 && valid; ++width) {
                                for (int32_t height = -1; height < 4 && valid; ++height) {
                                    int32_t bx = x + (width - 1) * dirX;
                                    int32_t by = y + height;
                                    int32_t bz = z + (width - 1) * dirZ;
                                    if (height < 0) {
                                        if (!isSolid(bx, by, bz)) valid = false;
                                    } else {
                                        if (!isAir(bx, by, bz)) valid = false;
                                    }
                                }
                            }
                            if (!valid) continue;

                            double dy = static_cast<double>(y) + 0.5 - entityY;
                            double dist = dx * dx + dy * dy + dz * dz;
                            if (bestDist < 0.0 || dist < bestDist) {
                                bestDist = dist;
                                bestX = x; bestY = y; bestZ = z;
                                bestDir = r % 2;
                            }
                        }
                    }
                }
            }
        }

        // Build the portal frame
        int32_t fX = bestDir % 2;
        int32_t fZ = 1 - fX;
        if (bestDir % 4 >= 2) { fX = -fX; fZ = -fZ; }

        // If no valid location found, create platform at y=70
        if (bestDist < 0.0) {
            bestY = std::max(70, std::min(worldHeight - 10, eY));
            for (int32_t depth = -1; depth <= 1; ++depth) {
                for (int32_t width = 1; width < 3; ++width) {
                    for (int32_t height = -1; height < 3; ++height) {
                        int32_t bx = bestX + (width - 1) * fX + depth * fZ;
                        int32_t by = bestY + height;
                        int32_t bz = bestZ + (width - 1) * fZ - depth * fX;
                        result.blocks.push_back({bx, by, bz, height < 0 ? OBSIDIAN_BLOCK : AIR_BLOCK});
                    }
                }
            }
        }

        // Build obsidian frame with portal fill (4 wide × 5 tall)
        for (int32_t width = 0; width < 4; ++width) {
            for (int32_t height = -1; height < 4; ++height) {
                int32_t bx = bestX + (width - 1) * fX;
                int32_t by = bestY + height;
                int32_t bz = bestZ + (width - 1) * fZ;
                bool isFrame = (width == 0 || width == 3 || height == -1 || height == 3);
                result.blocks.push_back({bx, by, bz, isFrame ? OBSIDIAN_BLOCK : PORTAL_BLOCK});
            }
        }

        result.portalX = bestX;
        result.portalY = bestY;
        result.portalZ = bestZ;
        result.direction = bestDir;
        return result;
    }

    // ─── Cache management ───

    // Java: removeStalePortalLocations — every 100 ticks, expire after 600
    void removeStalePortals(int64_t worldTime) {
        if (worldTime % 100 != 0) return;
        int64_t expireThreshold = worldTime - 600;

        auto it = cacheKeys_.begin();
        while (it != cacheKeys_.end()) {
            auto cached = portalCache_.find(*it);
            if (cached == portalCache_.end() || cached->second.lastUpdateTime < expireThreshold) {
                if (cached != portalCache_.end()) {
                    portalCache_.erase(cached);
                }
                it = cacheKeys_.erase(it);
            } else {
                ++it;
            }
        }
    }

    // ─── Motion rotation for portal direction change ───

    struct MotionResult {
        double motionX, motionZ;
        float yaw;
    };

    // Java: placeInExistingPortal motion rotation logic
    static MotionResult rotateMotion(double motionX, double motionZ, float yaw,
                                       int32_t entryDir, int32_t exitDir) {
        MotionResult result;
        float f4 = 0, f5 = 0, f6 = 0, f7 = 0;

        if (exitDir == entryDir) {
            f4 = 1.0f; f5 = 1.0f;
        } else if (exitDir == Direction::rotateOpposite[entryDir]) {
            f4 = -1.0f; f5 = -1.0f;
        } else if (exitDir == Direction::enderEyeMetaToDirection[entryDir]) {
            f6 = 1.0f; f7 = -1.0f;
        } else {
            f6 = -1.0f; f7 = 1.0f;
        }

        result.motionX = motionX * static_cast<double>(f4) + motionZ * static_cast<double>(f7);
        result.motionZ = motionX * static_cast<double>(f6) + motionZ * static_cast<double>(f5);
        result.yaw = yaw - static_cast<float>(entryDir * 90) + static_cast<float>(exitDir * 90);
        return result;
    }

private:
    static int32_t floorD(double d) {
        int32_t i = static_cast<int32_t>(d);
        return d < static_cast<double>(i) ? i - 1 : i;
    }

    static int64_t chunkKey(int32_t x, int32_t z) {
        // Java: ChunkCoordIntPair.chunkXZ2Int
        return (static_cast<int64_t>(x) & 0xFFFFFFFFL) |
               ((static_cast<int64_t>(z) & 0xFFFFFFFFL) << 32);
    }

    int32_t detectDirection(int32_t x, int32_t y, int32_t z, GetBlockFn& getBlock) {
        int32_t dir = -1;
        if (getBlock(x - 1, y, z) == PORTAL_BLOCK) dir = 2;
        if (getBlock(x + 1, y, z) == PORTAL_BLOCK) dir = 0;
        if (getBlock(x, y, z - 1) == PORTAL_BLOCK) dir = 3;
        if (getBlock(x, y, z + 1) == PORTAL_BLOCK) dir = 1;
        return dir;
    }

    std::unordered_map<int64_t, PortalPosition> portalCache_;
    std::vector<int64_t> cacheKeys_;
};

} // namespace mccpp
