#pragma once
// MobAI — behavior system for non-player entities.
// Simplified port of the vanilla AI task system from uf.java (EntityAITasks).
//
// Vanilla AI uses a priority-based task system where entities pick tasks
// to execute each tick. We simplify this to a state machine per mob type:
//   - IDLE: standing still, occasional look changes
//   - WANDER: move to random nearby position
//   - CHASE: move toward target player (hostile only)
//   - ATTACK: deal damage when close enough (hostile only)
//   - PANIC: flee from last damage source (passive only)
//
// Pathfinding uses a simplified step-toward approach with ground detection.
// Full A* is deferred to a later iteration for performance.

#include <cstdint>
#include <cmath>
#include <random>
#include <chrono>
#include <vector>
#include <optional>

#include "entity/MobEntity.h"

namespace mc {

// Forward declare
struct Player;

// AI behavior state
enum class AIState {
    IDLE,
    WANDER,
    CHASE,
    ATTACK,
    PANIC
};

// Movement speed by mob type (blocks per tick)
namespace MobSpeed {
    constexpr float ZOMBIE    = 0.046f;  // ~0.23 b/s at 20tps (vanilla: 0.23)
    constexpr float SKELETON  = 0.05f;   // 0.25 b/s
    constexpr float SPIDER    = 0.06f;   // 0.30 b/s
    constexpr float CREEPER   = 0.05f;   // 0.25 b/s
    constexpr float ENDERMAN  = 0.06f;
    constexpr float PIG       = 0.05f;
    constexpr float SHEEP     = 0.046f;
    constexpr float COW       = 0.04f;
    constexpr float CHICKEN   = 0.05f;
    constexpr float DEFAULT   = 0.046f;

    inline float forType(uint8_t typeId) {
        switch (typeId) {
            case EntityTypeID::ZOMBIE:    return ZOMBIE;
            case EntityTypeID::SKELETON:  return SKELETON;
            case EntityTypeID::SPIDER:    return SPIDER;
            case EntityTypeID::CREEPER:   return CREEPER;
            case EntityTypeID::ENDERMAN:  return ENDERMAN;
            case EntityTypeID::PIG:       return PIG;
            case EntityTypeID::SHEEP:     return SHEEP;
            case EntityTypeID::COW:       return COW;
            case EntityTypeID::CHICKEN:   return CHICKEN;
            default: return DEFAULT;
        }
    }
}

// Attack range and damage by mob type
namespace MobAttack {
    constexpr float ATTACK_RANGE = 2.0f;  // blocks
    constexpr int   ATTACK_COOLDOWN = 20; // ticks (1 second)

    inline float damageForType(uint8_t typeId) {
        switch (typeId) {
            case EntityTypeID::ZOMBIE:    return 3.0f;  // 1.5 hearts
            case EntityTypeID::SKELETON:  return 2.0f;  // Arrow damage varies
            case EntityTypeID::SPIDER:    return 2.0f;
            case EntityTypeID::CREEPER:   return 0.0f;  // Explodes instead
            case EntityTypeID::ENDERMAN:  return 7.0f;  // 3.5 hearts
            default: return 2.0f;
        }
    }
}

// Detection range for hostile mobs
constexpr float HOSTILE_DETECT_RANGE = 16.0f;  // blocks
constexpr float PASSIVE_WANDER_RANGE = 8.0f;   // blocks
constexpr float PANIC_RANGE = 16.0f;           // blocks

// Per-mob AI state stored alongside MobEntity
struct MobAIState {
    AIState state = AIState::IDLE;

    // Target position for movement
    double targetX = 0, targetY = 0, targetZ = 0;
    bool hasTarget = false;

    // Target player entity ID for hostile chase
    int32_t targetPlayerId = -1;

    // Timers
    int idleTimer = 0;        // Ticks remaining in idle state
    int attackCooldown = 0;   // Ticks until next attack
    int panicTimer = 0;       // Ticks remaining in panic state
    int wanderTimer = 0;      // Ticks remaining in wander state

    // RNG for this mob
    std::mt19937 rng{static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count())};
};

// Result of an AI tick — what happened
struct AITickResult {
    bool moved = false;           // Position changed
    bool attacked = false;        // Attacked a player
    int32_t attackedPlayerId = -1;
    float attackDamage = 0.0f;
};

// The main AI tick function for a mob
class MobAI {
public:
    // Tick a single mob's AI given nearby player positions
    // playerPositions: vector of (entityId, x, y, z)
    static AITickResult tick(MobEntity& mob, MobAIState& ai,
                             const std::vector<std::tuple<int32_t,double,double,double>>& players) {
        AITickResult result;

        bool isHostile = (mob.typeId >= 50 && mob.typeId < 70);

        // Decrease cooldowns
        if (ai.attackCooldown > 0) --ai.attackCooldown;

        switch (ai.state) {
            case AIState::IDLE:
                tickIdle(mob, ai, players, isHostile);
                break;
            case AIState::WANDER:
                result.moved = tickWander(mob, ai);
                break;
            case AIState::CHASE:
                result.moved = tickChase(mob, ai, players);
                // Check attack range
                if (result.moved) {
                    auto target = findPlayerById(players, ai.targetPlayerId);
                    if (target) {
                        double dist = distance(mob.posX, mob.posY, mob.posZ,
                            std::get<1>(*target), std::get<2>(*target), std::get<3>(*target));
                        if (dist <= MobAttack::ATTACK_RANGE) {
                            ai.state = AIState::ATTACK;
                        }
                    }
                }
                break;
            case AIState::ATTACK:
                result = tickAttack(mob, ai, players);
                break;
            case AIState::PANIC:
                result.moved = tickPanic(mob, ai);
                break;
        }

        return result;
    }

private:
    static void tickIdle(MobEntity& mob, MobAIState& ai,
                         const std::vector<std::tuple<int32_t,double,double,double>>& players,
                         bool isHostile) {
        if (ai.idleTimer > 0) {
            --ai.idleTimer;
            // Randomly change look direction
            if (ai.rng() % 40 == 0) {
                mob.yaw = static_cast<float>(ai.rng() % 360);
            }
            return;
        }

        // Hostile: look for nearby player
        if (isHostile) {
            auto nearest = findNearestPlayer(mob, players);
            if (nearest && std::get<0>(*nearest) <= HOSTILE_DETECT_RANGE) {
                ai.state = AIState::CHASE;
                ai.targetPlayerId = std::get<1>(*nearest);
                return;
            }
        }

        // Start wandering
        std::uniform_real_distribution<double> dist(-PASSIVE_WANDER_RANGE, PASSIVE_WANDER_RANGE);
        ai.targetX = mob.posX + dist(ai.rng);
        ai.targetZ = mob.posZ + dist(ai.rng);
        ai.targetY = mob.posY;
        ai.hasTarget = true;
        ai.wanderTimer = 60 + (ai.rng() % 120); // 3-9 seconds
        ai.state = AIState::WANDER;
    }

    static bool tickWander(MobEntity& mob, MobAIState& ai) {
        if (ai.wanderTimer <= 0 || !ai.hasTarget) {
            ai.state = AIState::IDLE;
            ai.idleTimer = 40 + (ai.rng() % 80); // 2-6 seconds
            ai.hasTarget = false;
            return false;
        }
        --ai.wanderTimer;

        // Move toward target
        return moveToward(mob, ai.targetX, ai.targetY, ai.targetZ,
                          MobSpeed::forType(mob.typeId));
    }

    static bool tickChase(MobEntity& mob, MobAIState& ai,
                          const std::vector<std::tuple<int32_t,double,double,double>>& players) {
        auto target = findPlayerById(players, ai.targetPlayerId);
        if (!target) {
            // Lost target
            ai.state = AIState::IDLE;
            ai.idleTimer = 20;
            ai.targetPlayerId = -1;
            return false;
        }

        double tx = std::get<1>(*target);
        double ty = std::get<2>(*target);
        double tz = std::get<3>(*target);

        // Check if out of range
        double dist = distance(mob.posX, mob.posY, mob.posZ, tx, ty, tz);
        if (dist > HOSTILE_DETECT_RANGE * 2) {
            ai.state = AIState::IDLE;
            ai.idleTimer = 40;
            ai.targetPlayerId = -1;
            return false;
        }

        // Chase speed is 1.3x normal (vanilla hostile sprint)
        return moveToward(mob, tx, ty, tz, MobSpeed::forType(mob.typeId) * 1.3f);
    }

    static AITickResult tickAttack(MobEntity& mob, MobAIState& ai,
                                   const std::vector<std::tuple<int32_t,double,double,double>>& players) {
        AITickResult result;

        auto target = findPlayerById(players, ai.targetPlayerId);
        if (!target) {
            ai.state = AIState::IDLE;
            ai.idleTimer = 20;
            return result;
        }

        double tx = std::get<1>(*target);
        double ty = std::get<2>(*target);
        double tz = std::get<3>(*target);
        double dist = distance(mob.posX, mob.posY, mob.posZ, tx, ty, tz);

        if (dist > MobAttack::ATTACK_RANGE * 1.5) {
            // Target moved away, chase again
            ai.state = AIState::CHASE;
            return result;
        }

        // Face target
        mob.yaw = static_cast<float>(
            std::atan2(tz - mob.posZ, tx - mob.posX) * 180.0 / 3.14159265358979 - 90.0);

        // Attack if cooldown ready
        if (ai.attackCooldown <= 0) {
            result.attacked = true;
            result.attackedPlayerId = ai.targetPlayerId;
            result.attackDamage = MobAttack::damageForType(mob.typeId);
            ai.attackCooldown = MobAttack::ATTACK_COOLDOWN;
        }

        return result;
    }

    static bool tickPanic(MobEntity& mob, MobAIState& ai) {
        if (ai.panicTimer <= 0 || !ai.hasTarget) {
            ai.state = AIState::IDLE;
            ai.idleTimer = 60;
            ai.hasTarget = false;
            return false;
        }
        --ai.panicTimer;

        // Run away faster (1.5x speed)
        return moveToward(mob, ai.targetX, ai.targetY, ai.targetZ,
                          MobSpeed::forType(mob.typeId) * 1.5f);
    }

    // Move mob toward a target position at given speed
    static bool moveToward(MobEntity& mob, double tx, double ty, double tz, float speed) {
        double dx = tx - mob.posX;
        double dz = tz - mob.posZ;
        double dist = std::sqrt(dx * dx + dz * dz);

        if (dist < 0.1) return false; // Close enough

        // Normalize and apply speed
        double nx = dx / dist;
        double nz = dz / dist;

        mob.posX += nx * speed;
        mob.posZ += nz * speed;

        // Face movement direction
        mob.yaw = static_cast<float>(
            std::atan2(nz, nx) * 180.0 / 3.14159265358979 - 90.0);
        mob.headYaw = mob.yaw;

        return true;
    }

    // Find nearest player to a mob, returns (distance, playerId)
    static std::optional<std::pair<double, int32_t>> findNearestPlayer(
            const MobEntity& mob,
            const std::vector<std::tuple<int32_t,double,double,double>>& players) {
        double bestDist = 1e9;
        int32_t bestId = -1;

        for (auto& [id, px, py, pz] : players) {
            double d = distance(mob.posX, mob.posY, mob.posZ, px, py, pz);
            if (d < bestDist) {
                bestDist = d;
                bestId = id;
            }
        }

        if (bestId == -1) return std::nullopt;
        return std::make_pair(bestDist, bestId);
    }

    // Find player by entity ID
    static std::optional<std::tuple<int32_t,double,double,double>> findPlayerById(
            const std::vector<std::tuple<int32_t,double,double,double>>& players,
            int32_t targetId) {
        for (auto& p : players) {
            if (std::get<0>(p) == targetId) return p;
        }
        return std::nullopt;
    }

    static double distance(double x1, double y1, double z1,
                           double x2, double y2, double z2) {
        double dx = x2 - x1, dy = y2 - y1, dz = z2 - z1;
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }
};

} // namespace mc
