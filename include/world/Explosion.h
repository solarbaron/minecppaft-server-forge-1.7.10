/**
 * Explosion.h — Explosion mechanics: block destruction, entity damage, knockback.
 *
 * Java reference: net.minecraft.world.Explosion
 *
 * The explosion algorithm:
 *   1. doExplosionA() — Ray-cast from center along 16³ cube surface normals.
 *      Each ray steps 0.3 blocks, reducing strength by block resistance.
 *      Affected blocks are collected. Entities within 2× radius take
 *      distance-based damage with block-density occlusion.
 *
 *   2. doExplosionB() — Destroy blocks (drop items with 1/size chance),
 *      place fire if flaming, spawn particles.
 *
 * Block access and entity queries are abstracted via callbacks for
 * thread-safe access from any thread.
 *
 * JNI readiness: Simple structs, predictable layout.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <unordered_set>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// BlockPos — Hashable 3D block position for affected block set.
// ═══════════════════════════════════════════════════════════════════════════

struct ExplosionBlockPos {
    int32_t x, y, z;
    bool operator==(const ExplosionBlockPos& o) const { return x == o.x && y == o.y && z == o.z; }
};

struct ExplosionBlockPosHash {
    size_t operator()(const ExplosionBlockPos& p) const {
        // Simple hash combining
        size_t h = static_cast<size_t>(p.x) * 73856093u;
        h ^= static_cast<size_t>(p.y) * 19349663u;
        h ^= static_cast<size_t>(p.z) * 83492791u;
        return h;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityHit — Entity affected by the explosion.
// ═══════════════════════════════════════════════════════════════════════════

struct ExplosionEntityHit {
    int32_t entityId;
    float damage;
    double knockbackX, knockbackY, knockbackZ;
    bool isPlayer;
    // Player-specific push for S12PacketEntityVelocity
    double playerPushX, playerPushY, playerPushZ;
};

// ═══════════════════════════════════════════════════════════════════════════
// Explosion — Complete explosion calculation.
// Java reference: net.minecraft.world.Explosion
// ═══════════════════════════════════════════════════════════════════════════

class Explosion {
public:
    // Callback: (x, y, z) → block explosion resistance (0 for air)
    using BlockResistanceFn = std::function<float(int32_t, int32_t, int32_t)>;

    // Callback: (x, y, z) → is the block solid (not air)?
    using BlockSolidFn = std::function<bool(int32_t, int32_t, int32_t)>;

    // Entity info for damage calculation
    struct EntityInfo {
        int32_t entityId;
        double posX, posY, posZ;
        float eyeHeight;
        double bbMinX, bbMinY, bbMinZ;
        double bbMaxX, bbMaxY, bbMaxZ;
        bool isPlayer;
        float blastProtectionFactor;  // From enchantment (0.0 = none)
    };

    // Callback: returns all entities within the given AABB
    using EntityQueryFn = std::function<std::vector<EntityInfo>(
        double minX, double minY, double minZ,
        double maxX, double maxY, double maxZ)>;

    // Callback: (origin, bbMin, bbMax) → block density [0..1] for occlusion
    using BlockDensityFn = std::function<double(
        double ox, double oy, double oz,
        double minX, double minY, double minZ,
        double maxX, double maxY, double maxZ)>;

    Explosion(double x, double y, double z, float size,
              bool isFlaming, bool isSmoking)
        : explosionX_(x), explosionY_(y), explosionZ_(z),
          explosionSize_(size),
          isFlaming_(isFlaming), isSmoking_(isSmoking) {}

    // ─── Phase A: Calculate affected blocks and entity damage ───

    // Java: doExplosionA
    void calculateExplosion(BlockResistanceFn blockResistance,
                             EntityQueryFn entityQuery,
                             BlockDensityFn blockDensity,
                             uint64_t randomSeed) {
        affectedBlocks_.clear();
        entityHits_.clear();
        rng_ = randomSeed;

        float originalSize = explosionSize_;

        // 1. Ray-cast from center along 16³ cube surface normals
        std::unordered_set<ExplosionBlockPos, ExplosionBlockPosHash> blockSet;
        constexpr int32_t RAYS = 16;

        for (int32_t i = 0; i < RAYS; ++i) {
            for (int32_t j = 0; j < RAYS; ++j) {
                for (int32_t k = 0; k < RAYS; ++k) {
                    // Only surface of the cube
                    if (i != 0 && i != RAYS - 1 &&
                        j != 0 && j != RAYS - 1 &&
                        k != 0 && k != RAYS - 1) continue;

                    // Direction vector (from center of unit cube)
                    double dx = static_cast<double>(i) / (static_cast<double>(RAYS) - 1.0) * 2.0 - 1.0;
                    double dy = static_cast<double>(j) / (static_cast<double>(RAYS) - 1.0) * 2.0 - 1.0;
                    double dz = static_cast<double>(k) / (static_cast<double>(RAYS) - 1.0) * 2.0 - 1.0;
                    double len = std::sqrt(dx * dx + dy * dy + dz * dz);
                    dx /= len;
                    dy /= len;
                    dz /= len;

                    // Ray-march
                    double rayX = explosionX_;
                    double rayY = explosionY_;
                    double rayZ = explosionZ_;
                    constexpr float STEP = 0.3f;

                    // Java: explosionSize * (0.7f + rand * 0.6f)
                    float strength = explosionSize_ * (0.7f + nextFloat() * 0.6f);

                    while (strength > 0.0f) {
                        int32_t bx = floorDouble(rayX);
                        int32_t by = floorDouble(rayY);
                        int32_t bz = floorDouble(rayZ);

                        float resistance = blockResistance(bx, by, bz);
                        if (resistance > 0.0f) {
                            // Java: f3 -= (f4 + 0.3f) * f2
                            strength -= (resistance + 0.3f) * STEP;
                        }

                        if (strength > 0.0f) {
                            blockSet.insert({bx, by, bz});
                        }

                        rayX += dx * static_cast<double>(STEP);
                        rayY += dy * static_cast<double>(STEP);
                        rayZ += dz * static_cast<double>(STEP);

                        // Java: f3 -= f2 * 0.75f
                        strength -= STEP * 0.75f;
                    }
                }
            }
        }

        affectedBlocks_.assign(blockSet.begin(), blockSet.end());

        // 2. Entity damage — doubled radius for entity search
        explosionSize_ *= 2.0f;

        double searchMinX = explosionX_ - static_cast<double>(explosionSize_) - 1.0;
        double searchMaxX = explosionX_ + static_cast<double>(explosionSize_) + 1.0;
        double searchMinY = explosionY_ - static_cast<double>(explosionSize_) - 1.0;
        double searchMaxY = explosionY_ + static_cast<double>(explosionSize_) + 1.0;
        double searchMinZ = explosionZ_ - static_cast<double>(explosionSize_) - 1.0;
        double searchMaxZ = explosionZ_ + static_cast<double>(explosionSize_) + 1.0;

        auto entities = entityQuery(searchMinX, searchMinY, searchMinZ,
                                      searchMaxX, searchMaxY, searchMaxZ);

        for (const auto& entity : entities) {
            // Distance ratio
            double distX = entity.posX - explosionX_;
            double distY = entity.posY + static_cast<double>(entity.eyeHeight) - explosionY_;
            double distZ = entity.posZ - explosionZ_;
            double dist = std::sqrt(distX * distX + distY * distY + distZ * distZ);
            double distRatio = dist / static_cast<double>(explosionSize_);

            if (distRatio > 1.0) continue;
            if (dist == 0.0) continue;

            // Normalize direction
            distX /= dist;
            distY /= dist;
            distZ /= dist;

            // Block density occlusion
            double density = blockDensity(explosionX_, explosionY_, explosionZ_,
                                            entity.bbMinX, entity.bbMinY, entity.bbMinZ,
                                            entity.bbMaxX, entity.bbMaxY, entity.bbMaxZ);

            double impact = (1.0 - distRatio) * density;

            // Java: ((impact² + impact) / 2.0 * 8.0 * size + 1.0)
            float damage = static_cast<float>(
                (impact * impact + impact) / 2.0 * 8.0 *
                static_cast<double>(explosionSize_) + 1.0);

            // Knockback with enchantment blast protection
            // Java: EnchantmentProtection.func_92092_a(entity, impact)
            double knockbackFactor = impact;
            if (entity.blastProtectionFactor > 0.0f) {
                knockbackFactor -= knockbackFactor * static_cast<double>(entity.blastProtectionFactor);
            }

            ExplosionEntityHit hit;
            hit.entityId = entity.entityId;
            hit.damage = damage;
            hit.knockbackX = distX * knockbackFactor;
            hit.knockbackY = distY * knockbackFactor;
            hit.knockbackZ = distZ * knockbackFactor;
            hit.isPlayer = entity.isPlayer;
            hit.playerPushX = distX * impact;
            hit.playerPushY = distY * impact;
            hit.playerPushZ = distZ * impact;
            entityHits_.push_back(hit);
        }

        explosionSize_ = originalSize;
    }

    // ─── Phase B results access ───

    const std::vector<ExplosionBlockPos>& getAffectedBlocks() const { return affectedBlocks_; }
    const std::vector<ExplosionEntityHit>& getEntityHits() const { return entityHits_; }

    // ─── Phase B: Block destruction decisions ───

    // Java: doExplosionB — returns blocks to destroy + fire placements
    struct DestructionResult {
        std::vector<ExplosionBlockPos> blocksToDestroy;    // Set to air
        std::vector<ExplosionBlockPos> blocksToDropItems;  // Drop items before destroying
        std::vector<ExplosionBlockPos> firePositions;      // Place fire blocks
    };

    DestructionResult calculateDestruction(BlockSolidFn blockSolid) {
        DestructionResult result;

        if (isSmoking_) {
            for (const auto& pos : affectedBlocks_) {
                bool solid = blockSolid(pos.x, pos.y, pos.z);
                if (solid) {
                    // Java: 1.0f / explosionSize drop chance
                    if (nextFloat() <= (1.0f / explosionSize_)) {
                        result.blocksToDropItems.push_back(pos);
                    }
                    result.blocksToDestroy.push_back(pos);
                }
            }
        }

        if (isFlaming_) {
            for (const auto& pos : affectedBlocks_) {
                bool isAir = !blockSolid(pos.x, pos.y, pos.z);
                bool belowSolid = blockSolid(pos.x, pos.y - 1, pos.z);
                // Java: air && belowSolid && rand(3) == 0
                if (isAir && belowSolid && (nextInt(3) == 0)) {
                    result.firePositions.push_back(pos);
                }
            }
        }

        return result;
    }

    // ─── Properties ───

    double getX() const { return explosionX_; }
    double getY() const { return explosionY_; }
    double getZ() const { return explosionZ_; }
    float getSize() const { return explosionSize_; }
    bool isFlaming() const { return isFlaming_; }
    bool isSmoking() const { return isSmoking_; }

    // Java: particle type based on size
    bool isLargeExplosion() const { return explosionSize_ >= 2.0f && isSmoking_; }

    // Java: sound parameters
    static constexpr float SOUND_VOLUME = 4.0f;
    float getSoundPitch() const {
        // (1.0f + (rand - rand) * 0.2f) * 0.7f
        return (1.0f + (nextFloatConst() - nextFloatConst()) * 0.2f) * 0.7f;
    }

private:
    // Java: MathHelper.floor_double
    static int32_t floorDouble(double d) {
        int32_t i = static_cast<int32_t>(d);
        return d < static_cast<double>(i) ? i - 1 : i;
    }

    // Simple LCG random for deterministic explosion shapes
    float nextFloat() {
        rng_ = rng_ * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<float>((rng_ >> 33) & 0x7FFFFF) / static_cast<float>(0x800000);
    }

    int32_t nextInt(int32_t bound) {
        rng_ = rng_ * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<int32_t>((rng_ >> 33) % static_cast<uint64_t>(bound));
    }

    float nextFloatConst() const {
        // For const methods, just return a deterministic value
        return 0.5f;
    }

    // ─── State ───
    double explosionX_, explosionY_, explosionZ_;
    float explosionSize_;
    bool isFlaming_;
    bool isSmoking_;

    std::vector<ExplosionBlockPos> affectedBlocks_;
    std::vector<ExplosionEntityHit> entityHits_;
    uint64_t rng_ = 0;
};

} // namespace mccpp
