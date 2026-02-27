#pragma once
// Physics — entity physics simulation.
// Ported from sv.java (Entity) and wq.java (EntityLivingBase).
// Implements gravity, AABB collision detection, ground detection, fall damage.
//
// Key constants from vanilla:
//   gravity = 0.08 blocks/tick² (sv.java line ~981)
//   drag    = 0.98 (sv.java motionY *= 0.9800000190734863)
//   player width  = 0.6 blocks (mw.java)
//   player height = 1.8 blocks (mw.java)
//   terminal velocity ~= 3.92 b/t (78.4 m/s)

#include <cstdint>
#include <cmath>
#include <algorithm>

#include "entity/Player.h"
#include "world/World.h"
#include "world/Block.h"

namespace mc {

// ============================================================
// AABB — sv.java AxisAlignedBB (ph.java)
// ============================================================
struct AABB {
    double minX, minY, minZ;
    double maxX, maxY, maxZ;

    AABB() : minX(0), minY(0), minZ(0), maxX(0), maxY(0), maxZ(0) {}
    AABB(double x1, double y1, double z1, double x2, double y2, double z2)
        : minX(x1), minY(y1), minZ(z1), maxX(x2), maxY(y2), maxZ(z2) {}

    // Expand box by delta in each direction
    AABB expand(double dx, double dy, double dz) const {
        double nx1 = dx < 0 ? minX + dx : minX;
        double ny1 = dy < 0 ? minY + dy : minY;
        double nz1 = dz < 0 ? minZ + dz : minZ;
        double nx2 = dx > 0 ? maxX + dx : maxX;
        double ny2 = dy > 0 ? maxY + dy : maxY;
        double nz2 = dz > 0 ? maxZ + dz : maxZ;
        return AABB(nx1, ny1, nz1, nx2, ny2, nz2);
    }

    // Offset box by delta
    AABB offset(double dx, double dy, double dz) const {
        return AABB(minX + dx, minY + dy, minZ + dz,
                    maxX + dx, maxY + dy, maxZ + dz);
    }

    // Check if two AABBs intersect
    bool intersects(const AABB& other) const {
        return maxX > other.minX && minX < other.maxX &&
               maxY > other.minY && minY < other.maxY &&
               maxZ > other.minZ && minZ < other.maxZ;
    }

    // Calculate Y offset that would prevent collision
    double calculateYOffset(const AABB& other, double dy) const {
        if (other.maxX > minX && other.minX < maxX &&
            other.maxZ > minZ && other.minZ < maxZ) {
            if (dy > 0.0 && other.maxY <= minY) {
                double d = minY - other.maxY;
                if (d < dy) dy = d;
            }
            if (dy < 0.0 && other.minY >= maxY) {
                double d = maxY - other.minY;
                if (d > dy) dy = d;
            }
        }
        return dy;
    }

    // Player bounding box from center position (mw.java)
    static AABB fromPlayer(double x, double y, double z) {
        constexpr double HALF_WIDTH = 0.3;  // width/2 = 0.6/2
        constexpr double HEIGHT = 1.8;
        return AABB(x - HALF_WIDTH, y, z - HALF_WIDTH,
                    x + HALF_WIDTH, y + HEIGHT, z + HALF_WIDTH);
    }
};

// ============================================================
// Physics Engine
// ============================================================
class Physics {
public:
    // Vanilla gravity constant — sv.java
    static constexpr double GRAVITY = 0.08;
    // Vanilla drag — sv.java: motionY *= 0.9800000190734863
    static constexpr double DRAG_Y = 0.9800000190734863;
    // Block drag (air) — sv.java
    static constexpr double DRAG_XZ = 0.91;
    // Fall damage threshold (3 blocks) — wq.java
    static constexpr float FALL_DAMAGE_THRESHOLD = 3.0f;

    // Simple solid block check
    static bool isSolidBlock(uint16_t blockId) {
        return blockId != BlockID::AIR &&
               blockId != BlockID::TORCH &&
               blockId != BlockID::SAPLING &&
               blockId != BlockID::LADDER &&
               blockId != BlockID::WATER &&
               blockId != BlockID::WATER_FLOW &&
               blockId != BlockID::LAVA &&
               blockId != BlockID::LAVA_FLOW;
    }

    // Tick physics for a player — called from server tick
    // Returns true if the player took fall damage
    static bool tickPlayer(Player& player, World& world) {
        // Skip physics if:
        // - Player is flying (creative flight)
        // - Player is in creative/spectator mode (no fall damage)
        if (player.flying || player.allowFlying) {
            player.fallDistance = 0.0f;
            return false;
        }

        // Apply gravity — sv.java: this.w -= 0.08
        player.motionY -= GRAVITY;

        // Apply drag — sv.java: this.w *= 0.9800000190734863
        player.motionY *= DRAG_Y;

        // Predicted new Y after gravity
        double newY = player.posY + player.motionY;

        // Ground collision check
        bool wasOnGround = player.onGround;
        player.onGround = false;

        // Check blocks below the player's feet
        AABB playerBox = AABB::fromPlayer(player.posX, newY, player.posZ);

        // Check all block positions that the player's bounding box overlaps
        int minBX = static_cast<int>(std::floor(playerBox.minX));
        int maxBX = static_cast<int>(std::floor(playerBox.maxX));
        int minBY = static_cast<int>(std::floor(playerBox.minY));
        int maxBY = static_cast<int>(std::floor(playerBox.maxY));
        int minBZ = static_cast<int>(std::floor(playerBox.minZ));
        int maxBZ = static_cast<int>(std::floor(playerBox.maxZ));

        for (int bx = minBX; bx <= maxBX; ++bx) {
            for (int bz = minBZ; bz <= maxBZ; ++bz) {
                for (int by = minBY; by <= maxBY; ++by) {
                    if (by < 0 || by > 255) continue;

                    uint16_t blockId = world.getBlock(bx, by, bz);
                    if (isSolidBlock(blockId)) {
                        // Solid block — check AABB collision
                        AABB blockBox(bx, by, bz, bx + 1.0, by + 1.0, bz + 1.0);

                        if (playerBox.intersects(blockBox)) {
                            // Colliding with a solid block below
                            if (player.motionY < 0) {
                                // Landing on top of block
                                newY = by + 1.0;
                                player.motionY = 0.0;
                                player.onGround = true;
                            }
                        }
                    }
                }
            }
        }

        // Void check — kill player if below Y=-64
        if (newY < -64.0) {
            newY = -64.0;
            player.motionY = 0.0;
        }

        // Update position
        player.posY = newY;

        // Fall damage calculation — wq.java
        bool tookDamage = false;
        if (player.onGround) {
            if (player.fallDistance > FALL_DAMAGE_THRESHOLD) {
                // Fall damage = fallDistance - 3 — wq.java
                float damage = player.fallDistance - FALL_DAMAGE_THRESHOLD;
                if (player.gameMode == GameMode::Survival) {
                    player.health -= damage;
                    if (player.health < 0.0f) player.health = 0.0f;
                    tookDamage = true;
                }
            }
            player.fallDistance = 0.0f;
        } else if (player.motionY < 0) {
            // Accumulate fall distance — sv.java: this.P -= this.w
            player.fallDistance -= static_cast<float>(player.motionY);
        }

        return tookDamage;
    }

    // Check if a position is on solid ground
    static bool isOnGround(double x, double y, double z, World& world) {
        int bx = static_cast<int>(std::floor(x));
        int by = static_cast<int>(std::floor(y - 0.01));
        int bz = static_cast<int>(std::floor(z));

        if (by < 0 || by > 255) return false;

        return isSolidBlock(world.getBlock(bx, by, bz));
    }
};

} // namespace mc
