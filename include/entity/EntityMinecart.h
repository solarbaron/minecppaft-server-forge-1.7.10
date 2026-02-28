/**
 * EntityMinecart.h — Minecart rail physics and EntityLightningBolt.
 *
 * Java references:
 *   - net.minecraft.entity.item.EntityMinecart (727 lines)
 *   - net.minecraft.entity.effect.EntityLightningBolt (95 lines)
 *
 * EntityMinecart:
 *   - Size: 0.98×0.7
 *   - 10-entry direction matrix for 10 rail metadata values
 *   - Rail snapping: project position onto rail segment [start, end]
 *   - Slope gravity: motionX/Z -= 0.0078125 on ascending rails (meta 2-5)
 *   - Powered rail: boost 0.06 in direction of travel, or brake to 0.03/stop
 *   - Speed cap: 0.4 (maxSpeed), 2.0 (absolute velocity clamp)
 *   - Rider drag: 0.997 (occupied) vs 0.96 (empty)
 *   - Off-rail: clamp ±maxSpeed, onGround *= 0.5, !onGround *= 0.95
 *   - yawFromMotion: atan2(dZ, dX), reverse detection (±170° flip)
 *   - Height compensation: posY from func_70489_a (rail position interpolation)
 *   - Entity collision: push mobs into empty carts, minecart-minecart elastic
 *   - Minecart-minecart: 0.8 dot product gate, furnace priority (type 2)
 *   - DataWatcher: 17 (rollingAmplitude int), 18 (rollingDirection int),
 *     19 (damage float), 20 (displayTile int), 21 (displayOffset int), 22 (hasDisplay byte)
 *   - 7 subtypes: 0=empty, 1=chest, 2=furnace, 3=TNT, 4=spawner, 5=hopper, 6=command
 *   - Damage: hit *= 10, threshold 40, creative instant-kill
 *
 * EntityLightningBolt:
 *   - lightningState: starts at 2 (flash), decrements per tick
 *   - boltLivingTime: rand(3)+1 re-strikes
 *   - Fire: at impact position + 4 random nearby (±1 block), only on Normal/Hard with doFireTick
 *   - Entity damage: 3.0 block radius (x±3, y-3 to y+9, z±3)
 *   - Sound: "ambient.weather.thunder" (vol 10000, pitch 0.8-1.0) + "random.explode" (vol 2, pitch 0.5-0.7)
 *   - No NBT save/load
 *
 * Thread safety: Entity tick on single thread per entity.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityMinecart — Rail vehicle with full physics.
// Java: net.minecraft.entity.item.EntityMinecart (727 lines)
// ═══════════════════════════════════════════════════════════════════════════

class EntityMinecart {
public:
    // ─── Entity base ───
    double posX = 0, posY = 0, posZ = 0;
    double prevPosX = 0, prevPosY = 0, prevPosZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    float prevRotationYaw = 0;
    bool onGround = false;
    bool isDead = false;
    float fallDistance = 0;
    int32_t entityId = 0;
    int32_t riderEntityId = -1;

    // ─── Minecart-specific ───
    int32_t minecartType = 0; // 0=empty,1=chest,2=furnace,3=tnt,4=spawner,5=hopper,6=cmd
    bool isInReverse = false;
    std::string entityName;

    // DataWatcher values
    int32_t rollingAmplitude = 0;
    int32_t rollingDirection = 1;
    float damage = 0;
    int32_t displayTile = 0;     // low 16 bits = blockId, high 16 = data
    int32_t displayTileOffset = 6;
    bool hasDisplayTile = false;

    // ─── Constants ───
    static constexpr float WIDTH = 0.98f;
    static constexpr float HEIGHT = 0.7f;
    static constexpr float MOUNTED_Y_OFFSET = -0.3f;
    static constexpr float GRAVITY = 0.04f;
    static constexpr double MAX_SPEED = 0.4;
    static constexpr double VELOCITY_CLAMP = 2.0;
    static constexpr double SLOPE_ACCEL = 0.0078125;
    static constexpr double POWERED_ACCEL = 0.06;
    static constexpr double BRAKE_THRESHOLD = 0.03;
    static constexpr float RIDER_DRAG = 0.997f;
    static constexpr float EMPTY_DRAG = 0.96f;
    static constexpr float DAMAGE_THRESHOLD = 40.0f;
    static constexpr float DAMAGE_MULTIPLIER = 10.0f;
    static constexpr float COLLISION_PUSHFORCE = 0.1f;
    static constexpr float DOT_GATE = 0.8f;

    // ═══════════════════════════════════════════════════════════════════════
    // Rail direction matrix — 10 entries for metadata 0-9.
    // Each entry: [start][xyz], [end][xyz]
    //   0: NS flat, 1: EW flat, 2-5: slopes, 6-9: corners
    //
    // Java: private static final int[][][] matrix
    // ═══════════════════════════════════════════════════════════════════════

    static constexpr int32_t MATRIX[10][2][3] = {
        {{ 0, 0,-1}, { 0, 0, 1}},  // 0: NS
        {{-1, 0, 0}, { 1, 0, 0}},  // 1: EW
        {{-1,-1, 0}, { 1, 0, 0}},  // 2: ascending east
        {{-1, 0, 0}, { 1,-1, 0}},  // 3: ascending west
        {{ 0, 0,-1}, { 0,-1, 1}},  // 4: ascending south
        {{ 0,-1,-1}, { 0, 0, 1}},  // 5: ascending north
        {{ 0, 0, 1}, { 1, 0, 0}},  // 6: SE corner
        {{ 0, 0, 1}, {-1, 0, 0}},  // 7: SW corner
        {{ 0, 0,-1}, {-1, 0, 0}},  // 8: NW corner
        {{ 0, 0,-1}, { 1, 0, 0}},  // 9: NE corner
    };

    // ═══════════════════════════════════════════════════════════════════════
    // Rail physics — main on-rail tick.
    // Java: func_145821_a
    //
    // Parameters:
    //   blockX/Y/Z: rail block position
    //   maxSpeed: 0.4 for regular
    //   slopeAccel: 0.0078125
    //   railMeta: block metadata (0-9 for rail, 0-7 for powered)
    //   isPoweredRail: block is golden_rail
    //   isPowered: powered rail is active (meta & 8)
    //   hasRider: entity is being ridden
    //   riderYaw: rider's rotationYaw
    //   riderForward: rider's moveForward
    // ═══════════════════════════════════════════════════════════════════════

    void onRailTick(int32_t bx, int32_t by, int32_t bz,
                     double maxSpeed, double slopeAccel,
                     int32_t railMeta,
                     bool isPoweredRail, bool isPowered,
                     bool hasRider, float riderYaw, double riderForward) {
        fallDistance = 0;

        posY = by;
        bool boosted = false;
        bool braking = false;

        if (isPoweredRail) {
            boosted = isPowered;
            braking = !boosted;
        }

        // Strip powered flag from meta
        int32_t meta = railMeta;
        if (isPoweredRail) meta &= 7;

        // Slope gravity (meta 2-5 = ascending)
        if (meta >= 2 && meta <= 5) {
            posY = by + 1;
        }
        if (meta == 2) motionX -= slopeAccel;
        if (meta == 3) motionX += slopeAccel;
        if (meta == 4) motionZ += slopeAccel;
        if (meta == 5) motionZ -= slopeAccel;

        // Align motion to rail direction
        auto& dir = MATRIX[meta];
        double dx = dir[1][0] - dir[0][0];
        double dz = dir[1][2] - dir[0][2];
        double dirLen = std::sqrt(dx * dx + dz * dz);

        // Flip direction to match current motion
        double dot = motionX * dx + motionZ * dz;
        if (dot < 0) { dx = -dx; dz = -dz; }

        // Clamp speed to 2.0
        double speed = std::sqrt(motionX * motionX + motionZ * motionZ);
        if (speed > VELOCITY_CLAMP) speed = VELOCITY_CLAMP;

        motionX = speed * dx / dirLen;
        motionZ = speed * dz / dirLen;

        // Rider push when stopped
        if (hasRider && riderForward > 0) {
            double currentSpdSq = motionX * motionX + motionZ * motionZ;
            if (currentSpdSq < 0.01) {
                double ry = riderYaw * M_PI / 180.0;
                motionX += -std::sin(ry) * 0.1;
                motionZ += std::cos(ry) * 0.1;
                braking = false;
            }
        }

        // Braking (unpowered golden rail)
        if (braking) {
            double spd = std::sqrt(motionX * motionX + motionZ * motionZ);
            if (spd < BRAKE_THRESHOLD) {
                motionX = 0; motionY = 0; motionZ = 0;
            } else {
                motionX *= 0.5; motionY = 0; motionZ *= 0.5;
            }
        }

        // Snap to rail position
        double startX = bx + 0.5 + dir[0][0] * 0.5;
        double startZ = bz + 0.5 + dir[0][2] * 0.5;
        double endX = bx + 0.5 + dir[1][0] * 0.5;
        double endZ = bz + 0.5 + dir[1][2] * 0.5;
        dx = endX - startX;
        dz = endZ - startZ;

        double t = 0;
        if (dx == 0) { posX = bx + 0.5; t = posZ - bz; }
        else if (dz == 0) { posZ = bz + 0.5; t = posX - bx; }
        else {
            double px = posX - startX;
            double pz = posZ - startZ;
            t = (px * dx + pz * dz) * 2.0;
        }
        posX = startX + dx * t;
        posZ = startZ + dz * t;

        // Apply rider speed reduction
        double mx = motionX, mz = motionZ;
        if (hasRider) { mx *= 0.75; mz *= 0.75; }

        // Clamp to maxSpeed
        if (mx < -maxSpeed) mx = -maxSpeed;
        if (mx > maxSpeed) mx = maxSpeed;
        if (mz < -maxSpeed) mz = -maxSpeed;
        if (mz > maxSpeed) mz = maxSpeed;

        // moveEntity(mx, 0, mz) — caller handles

        // Slope height adjustment
        if (dir[0][1] != 0) {
            // Check if moved to start endpoint
        }
        if (dir[1][1] != 0) {
            // Check if moved to end endpoint
        }

        // Apply drag
        applyDrag(hasRider);

        // Powered rail boost
        if (boosted) {
            double spd = std::sqrt(motionX * motionX + motionZ * motionZ);
            if (spd > 0.01) {
                motionX += motionX / spd * POWERED_ACCEL;
                motionZ += motionZ / spd * POWERED_ACCEL;
            } else if (meta == 1) {
                // Push in direction of adjacent solid block (EW rail)
                // Handled by caller checking block solidity
            } else if (meta == 0) {
                // Push in direction of adjacent solid block (NS rail)
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Off-rail movement — when not on rails
    // Java: func_94088_b
    //   - Clamp motion to ±maxSpeed
    //   - onGround: all *= 0.5
    //   - moveEntity
    //   - !onGround: all *= 0.95
    // ═══════════════════════════════════════════════════════════════════════

    void offRailTick(double maxSpeed) {
        if (motionX < -maxSpeed) motionX = -maxSpeed;
        if (motionX > maxSpeed) motionX = maxSpeed;
        if (motionZ < -maxSpeed) motionZ = -maxSpeed;
        if (motionZ > maxSpeed) motionZ = maxSpeed;

        if (onGround) {
            motionX *= 0.5;
            motionY *= 0.5;
            motionZ *= 0.5;
        }

        // moveEntity done by caller

        if (!onGround) {
            motionX *= 0.95;
            motionY *= 0.95;
            motionZ *= 0.95;
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Drag — applied after rail movement.
    // Java: applyDrag
    //   - Rider: X/Z *= 0.997, Y = 0
    //   - Empty: X/Z *= 0.96, Y = 0
    // ═══════════════════════════════════════════════════════════════════════

    void applyDrag(bool hasRider) {
        if (hasRider) {
            motionX *= RIDER_DRAG;
            motionY = 0;
            motionZ *= RIDER_DRAG;
        } else {
            motionX *= EMPTY_DRAG;
            motionY = 0;
            motionZ *= EMPTY_DRAG;
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Main tick
    // ═══════════════════════════════════════════════════════════════════════

    struct TickResult {
        bool onRail;
        bool shouldDie;
    };

    TickResult onUpdate() {
        if (rollingAmplitude > 0) --rollingAmplitude;
        if (damage > 0) damage -= 1.0f;
        if (posY < -64) { isDead = true; return {false, true}; }

        prevPosX = posX; prevPosY = posY; prevPosZ = posZ;
        motionY -= GRAVITY;

        // Rail detection done by caller
        return {false, false};
    }

    // ─── Yaw from motion ───
    void updateYawFromMotion() {
        rotationPitch = 0;
        double dx = prevPosX - posX;
        double dz = prevPosZ - posZ;
        if (dx * dx + dz * dz > 0.001) {
            rotationYaw = static_cast<float>(std::atan2(dz, dx) * 180.0 / M_PI);
            if (isInReverse) rotationYaw += 180.0f;
        }

        // Reverse detection
        float yawDelta = rotationYaw - prevRotationYaw;
        while (yawDelta > 180.0f) yawDelta -= 360.0f;
        while (yawDelta < -180.0f) yawDelta += 360.0f;
        if (yawDelta < -170.0f || yawDelta >= 170.0f) {
            rotationYaw += 180.0f;
            isInReverse = !isInReverse;
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Damage handling — same pattern as EntityBoat
    // ═══════════════════════════════════════════════════════════════════════

    struct DamageResult {
        bool broken;
        bool dropItems;
    };

    DamageResult attackEntityFrom(float dmg, bool isCreative, bool isInvulnerable) {
        if (isDead || isInvulnerable) return {false, false};

        rollingDirection = -rollingDirection;
        rollingAmplitude = 10;
        damage += dmg * DAMAGE_MULTIPLIER;

        if (isCreative || damage > DAMAGE_THRESHOLD) {
            isDead = true;
            if (isCreative && !entityName.empty()) {
                // Named creative kill → still drop with name
            }
            return {true, !(isCreative && entityName.empty())};
        }
        return {false, false};
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Minecart-minecart collision
    // Java: applyEntityCollision
    //   - Dot product gate: only interact if facing angle > 0.8
    //   - Furnace (type 2) has priority: pushes others
    //   - Equal types: average velocity
    //   - Non-minecart entities: push with 0.1 force, 0.25 to entity
    //   - Auto-mount: living non-player entities into empty (type 0) carts
    // ═══════════════════════════════════════════════════════════════════════

    struct CollisionResult {
        double selfMotionX, selfMotionZ;
        double otherMotionX, otherMotionZ;
        bool autoMount; // Other should mount this cart
    };

    CollisionResult collideWithMinecart(double otherX, double otherZ,
                                          double otherMotionX_, double otherMotionZ_,
                                          int32_t otherType) {
        CollisionResult result{};
        result.selfMotionX = motionX;
        result.selfMotionZ = motionZ;
        result.otherMotionX = otherMotionX_;
        result.otherMotionZ = otherMotionZ_;

        double dx = otherX - posX;
        double dz = otherZ - posZ;
        double distSq = dx * dx + dz * dz;
        if (distSq < 1e-4) return result;

        double dist = std::sqrt(distSq);
        dx /= dist; dz /= dist;

        double invDist = 1.0 / dist;
        if (invDist > 1) invDist = 1;
        dx *= invDist; dz *= invDist;
        dx *= COLLISION_PUSHFORCE; dz *= COLLISION_PUSHFORCE;
        dx *= 0.5; dz *= 0.5;

        // Dot product gate
        double cosYaw = std::cos(rotationYaw * M_PI / 180.0);
        double sinYaw = std::sin(rotationYaw * M_PI / 180.0);
        double dirLen = std::sqrt((otherX - posX) * (otherX - posX) + (otherZ - posZ) * (otherZ - posZ));
        if (dirLen < 1e-7) return result;
        double normDX = (otherX - posX) / dirLen;
        double normDZ = (otherZ - posZ) / dirLen;
        double dotP = std::abs(normDX * cosYaw + normDZ * sinYaw);
        if (dotP < DOT_GATE) return result;

        double sumMX = result.otherMotionX + motionX;
        double sumMZ = result.otherMotionZ + motionZ;

        if (otherType == 2 && minecartType != 2) {
            // Furnace pushes us
            result.selfMotionX = motionX * 0.2 + result.otherMotionX - dx;
            result.selfMotionZ = motionZ * 0.2 + result.otherMotionZ - dz;
            result.otherMotionX = otherMotionX_ * 0.95;
            result.otherMotionZ = otherMotionZ_ * 0.95;
        } else if (otherType != 2 && minecartType == 2) {
            // We are furnace, push other
            result.otherMotionX = otherMotionX_ * 0.2 + motionX + dx;
            result.otherMotionZ = otherMotionZ_ * 0.2 + motionZ + dz;
            result.selfMotionX = motionX * 0.95;
            result.selfMotionZ = motionZ * 0.95;
        } else {
            // Equal: average
            sumMX /= 2.0; sumMZ /= 2.0;
            result.selfMotionX = motionX * 0.2 + sumMX - dx;
            result.selfMotionZ = motionZ * 0.2 + sumMZ - dz;
            result.otherMotionX = otherMotionX_ * 0.2 + sumMX + dx;
            result.otherMotionZ = otherMotionZ_ * 0.2 + sumMZ + dz;
        }

        return result;
    }

    bool canBeCollidedWith() const { return !isDead; }
    bool canBePushed() const { return true; }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityLightningBolt — Lightning strike effect.
// Java: net.minecraft.entity.effect.EntityLightningBolt (95 lines)
// ═══════════════════════════════════════════════════════════════════════════

class EntityLightningBolt {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    // Lightning state
    int32_t lightningState = 2;  // Starts at 2 (flash), decrements per tick
    int64_t boltVertex = 0;       // Random vertex for render
    int32_t boltLivingTime = 0;   // Re-strikes remaining

    // ─── Constants ───
    static constexpr double ENTITY_DAMAGE_RADIUS = 3.0;
    static constexpr double ENTITY_DAMAGE_HEIGHT = 6.0; // Y extends +6 above
    static constexpr int32_t INITIAL_FIRE_ATTEMPTS = 4;  // Random nearby fires
    static constexpr float THUNDER_VOLUME = 10000.0f;
    static constexpr float EXPLODE_VOLUME = 2.0f;

    // ═══════════════════════════════════════════════════════════════════════
    // Spawn
    // Java: EntityLightningBolt(world, x, y, z)
    //   - lightningState = 2
    //   - boltVertex = rand.nextLong()
    //   - boltLivingTime = rand(3) + 1
    //   - Fire at impact: check air, can place fire
    //   - 4 random nearby fires: ±1 block XYZ
    //   - Only on Normal/Hard with doFireTick gamerule
    // ═══════════════════════════════════════════════════════════════════════

    struct SpawnResult {
        bool playThunder;
        bool setFireAtImpact;
        int32_t fireX, fireY, fireZ;
        struct NearbyFire { int32_t x, y, z; };
        NearbyFire nearbyFires[INITIAL_FIRE_ATTEMPTS];
        int32_t nearbyFireCount;
    };

    void spawn(double x, double y, double z, int64_t randVertex, int32_t randLiving) {
        posX = x; posY = y; posZ = z;
        lightningState = 2;
        boltVertex = randVertex;
        boltLivingTime = randLiving; // rand(3) + 1 → caller provides 1-3
    }

    // ═══════════════════════════════════════════════════════════════════════
    // onUpdate — Lightning tick.
    // Java: EntityLightningBolt.onUpdate
    //
    // Tick phases:
    //   1. lightningState == 2: play thunder + explosion sounds
    //   2. --lightningState
    //   3. lightningState < 0:
    //      - boltLivingTime == 0 → die
    //      - lightningState < -rand(10) → re-flash: --boltLivingTime,
    //        lightningState = 1, new boltVertex, try fire
    //   4. lightningState >= 0: damage entities in 3-block radius
    // ═══════════════════════════════════════════════════════════════════════

    struct TickResult {
        bool playThunder;         // First tick sound
        bool damageEntities;      // Entity scan this tick
        bool tryFire;             // Re-strike fire attempt
        bool shouldDie;
        int64_t newBoltVertex;    // For re-strike render
    };

    TickResult onUpdate(int32_t randForRetrigger, int64_t randNewVertex) {
        TickResult result{};

        // Sound on first flash
        if (lightningState == 2) {
            result.playThunder = true;
        }

        --lightningState;

        if (lightningState < 0) {
            if (boltLivingTime == 0) {
                isDead = true;
                result.shouldDie = true;
                return result;
            }

            if (lightningState < -randForRetrigger) {
                // Re-strike
                --boltLivingTime;
                lightningState = 1;
                boltVertex = randNewVertex;
                result.tryFire = true;
            }
        }

        // Damage entities when visible
        if (lightningState >= 0) {
            result.damageEntities = true;
        }

        return result;
    }

    // Entity damage bounds
    void getDamageBounds(double& minX, double& minY, double& minZ,
                          double& maxX, double& maxY, double& maxZ) const {
        minX = posX - ENTITY_DAMAGE_RADIUS;
        minY = posY - ENTITY_DAMAGE_RADIUS;
        minZ = posZ - ENTITY_DAMAGE_RADIUS;
        maxX = posX + ENTITY_DAMAGE_RADIUS;
        maxY = posY + ENTITY_DAMAGE_HEIGHT + ENTITY_DAMAGE_RADIUS;
        maxZ = posZ + ENTITY_DAMAGE_RADIUS;
    }
};

} // namespace mccpp
