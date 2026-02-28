/**
 * EntityItem.h — Dropped item entity and experience orb entity.
 *
 * Java references:
 *   - net.minecraft.entity.item.EntityItem (298 lines)
 *   - net.minecraft.entity.item.EntityXPOrb (199 lines)
 *
 * EntityItem:
 *   - Size: 0.25×0.25, yOffset = height/2
 *   - Gravity: 0.04 per tick
 *   - Ground friction: block.slipperiness * 0.98
 *   - Air friction: 0.98 (Y always)
 *   - Ground bounce: motionY *= -0.5
 *   - Despawn: age >= 6000 ticks (5 minutes)
 *   - Merging: combineItems (same item+meta+NBT, within 0.5 block radius)
 *   - Pickup: delay check + owner/thrower, achievements (log, leather, diamond, blaze_rod)
 *   - Lava: bob up with random XZ motion
 *   - Nether star: immune to explosion damage
 *   - Health: 5, takes damage from fire/attacks
 *   - NBT: Health (short→byte), Age (short), Item (compound), Owner, Thrower
 *   - DataWatcher slot 10: ItemStack (type 5)
 *   - Creative despawn: age = 4800
 *
 * EntityXPOrb:
 *   - Size: 0.5×0.5 (spawn), 0.25×0.25 (default)
 *   - Gravity: 0.03 per tick
 *   - Ground bounce: motionY *= -0.9
 *   - Despawn: xpOrbAge >= 6000
 *   - Player attraction: 8 block search radius, inverse distance force 0.1
 *   - Pickup: xpCooldown = 2, sound "random.orb"
 *   - XP split thresholds: 2477, 1237, 617, 307, 149, 73, 37, 17, 7, 3, 1
 *   - Health: 5
 *   - NBT: Health, Age, Value
 *
 * Thread safety: Entity tick on single thread per entity.
 * JNI readiness: Simple structs, DataWatcher slot 10 for ItemStack.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <optional>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityItem — Dropped item entity.
// Java reference: net.minecraft.entity.item.EntityItem (298 lines)
// ═══════════════════════════════════════════════════════════════════════════

class EntityItem {
public:
    // ─── Entity base fields ───
    double posX = 0, posY = 0, posZ = 0;
    double prevPosX = 0, prevPosY = 0, prevPosZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0;
    bool onGround = false;
    bool isDead = false;
    bool noClip = false;
    int32_t ticksExisted = 0;
    int32_t entityId = 0;

    // ─── EntityItem-specific ───
    int32_t age = 0;
    int32_t delayBeforeCanPickup = 0;
    int32_t health = 5;
    float hoverStart = 0;
    std::string owner;    // field_145802_g
    std::string thrower;  // field_145801_f

    // Item data (simplified — in full impl, references DataWatcher slot 10)
    int32_t itemId = 0;
    int32_t itemMeta = 0;
    int32_t stackSize = 1;
    int32_t maxStackSize = 64;
    bool hasSubtypes = false;

    // ─── Constants ───
    static constexpr float WIDTH = 0.25f;
    static constexpr float HEIGHT = 0.25f;
    static constexpr float GRAVITY = 0.04f;
    static constexpr float AIR_FRICTION = 0.98f;
    static constexpr float GROUND_BOUNCE = -0.5f;
    static constexpr int32_t DESPAWN_AGE = 6000;       // 5 minutes
    static constexpr int32_t CREATIVE_DESPAWN_AGE = 4800; // 4 minutes

    // Item IDs for special handling
    static constexpr int32_t NETHER_STAR_ID = 399;

    // ═══════════════════════════════════════════════════════════════════════
    // Construction
    // Java: EntityItem(world, x, y, z) — random initial motion
    // ═══════════════════════════════════════════════════════════════════════

    EntityItem() {
        hoverStart = static_cast<float>(fmod(static_cast<double>(rand()) / RAND_MAX * M_PI * 2.0, M_PI * 2.0));
    }

    void spawn(double x, double y, double z) {
        posX = x; posY = y; posZ = z;
        rotationYaw = static_cast<float>(static_cast<double>(rand()) / RAND_MAX * 360.0);
        motionX = static_cast<double>(rand()) / RAND_MAX * 0.2 - 0.1;
        motionY = 0.2;
        motionZ = static_cast<double>(rand()) / RAND_MAX * 0.2 - 0.1;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // onUpdate — Physics and logic tick.
    // Java: EntityItem.onUpdate
    //   - Gravity: motionY -= 0.04
    //   - Push out of blocks
    //   - Move entity
    //   - Lava check: bob up with random XZ
    //   - Merge nearby items (every tick if moved, or every 25 ticks)
    //   - Ground friction: block.slipperiness * 0.98
    //   - Air Y friction: 0.98
    //   - Ground bounce: motionY *= -0.5
    //   - Despawn at age 6000
    // ═══════════════════════════════════════════════════════════════════════

    struct TickResult {
        bool shouldDie;
        bool searchForMerge;
    };

    TickResult onUpdate(float groundSlipperiness, bool inLava) {
        if (itemId == 0) return {true, false};

        ++ticksExisted;

        if (delayBeforeCanPickup > 0) --delayBeforeCanPickup;

        prevPosX = posX; prevPosY = posY; prevPosZ = posZ;

        // Gravity
        motionY -= GRAVITY;

        // Lava bob
        if (inLava) {
            motionY = 0.2;
            // motionX/Z randomized by caller
        }

        // Check if moved blocks
        bool moved = (static_cast<int32_t>(prevPosX) != static_cast<int32_t>(posX)) ||
                     (static_cast<int32_t>(prevPosY) != static_cast<int32_t>(posY)) ||
                     (static_cast<int32_t>(prevPosZ) != static_cast<int32_t>(posZ));

        bool doMerge = moved || (ticksExisted % 25 == 0);

        // Friction
        float friction = AIR_FRICTION;
        if (onGround) {
            friction = groundSlipperiness * 0.98f;
        }

        motionX *= friction;
        motionY *= AIR_FRICTION;
        motionZ *= friction;

        if (onGround) {
            motionY *= GROUND_BOUNCE;
        }

        ++age;
        if (age >= DESPAWN_AGE) {
            return {true, false};
        }

        return {false, doMerge};
    }

    // ═══════════════════════════════════════════════════════════════════════
    // combineItems — Merge two item entities.
    // Java: EntityItem.combineItems
    //   - Same item ID, same metadata (if hasSubtypes), same NBT tag
    //   - Smaller stack merges into larger
    //   - Combined size must not exceed maxStackSize
    //   - Surviving entity gets min(age), max(delayBeforeCanPickup)
    // ═══════════════════════════════════════════════════════════════════════

    bool canMergeWith(const EntityItem& other) const {
        if (&other == this) return false;
        if (isDead || other.isDead) return false;
        if (itemId != other.itemId) return false;
        if (hasSubtypes && itemMeta != other.itemMeta) return false;
        // NBT comparison would go here
        if (stackSize + other.stackSize > maxStackSize) return false;
        return true;
    }

    // Returns true if this entity should die (merged into other)
    bool combineItems(EntityItem& other) {
        if (!canMergeWith(other)) return false;

        // Smaller merges into larger
        if (other.stackSize < stackSize) {
            return other.combineItems(*this);
        }

        if (other.stackSize + stackSize > maxStackSize) return false;

        other.stackSize += stackSize;
        other.delayBeforeCanPickup = std::max(other.delayBeforeCanPickup, delayBeforeCanPickup);
        other.age = std::min(other.age, age);
        isDead = true;
        return true;
    }

    void setAgeToCreativeDespawnTime() { age = CREATIVE_DESPAWN_AGE; }

    // ═══════════════════════════════════════════════════════════════════════
    // Attack/Damage
    // Java: EntityItem.attackEntityFrom
    //   - Nether star immune to explosions
    //   - health -= damage, die if <= 0
    // ═══════════════════════════════════════════════════════════════════════

    bool attackEntityFrom(float damage, bool isExplosion, bool isInvulnerable) {
        if (isInvulnerable) return false;
        if (itemId == NETHER_STAR_ID && isExplosion) return false;
        health -= static_cast<int32_t>(damage);
        if (health <= 0) isDead = true;
        return false;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Pickup
    // Java: EntityItem.onCollideWithPlayer
    //   - Check delayBeforeCanPickup == 0
    //   - Owner check: null, or within 200 ticks of despawn, or matches player
    //   - Add to inventory, trigger achievements, play "random.pop"
    //   - Achievement triggers: log→mineWood, leather→killCow,
    //     diamond→diamonds, blaze_rod→blazeRod
    // ═══════════════════════════════════════════════════════════════════════

    struct PickupResult {
        bool pickedUp;
        int32_t amount;
        bool shouldDie;
    };

    PickupResult tryPickup(const std::string& playerName, bool canAddToInventory) {
        if (delayBeforeCanPickup != 0) return {false, 0, false};
        if (!owner.empty() && (DESPAWN_AGE - age > 200) && owner != playerName) {
            return {false, 0, false};
        }

        if (canAddToInventory) {
            int32_t picked = stackSize;
            isDead = true;
            return {true, picked, true};
        }
        return {false, 0, false};
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityXPOrb — Experience orb entity.
// Java reference: net.minecraft.entity.item.EntityXPOrb (199 lines)
// ═══════════════════════════════════════════════════════════════════════════

class EntityXPOrb {
public:
    // ─── Entity base fields ───
    double posX = 0, posY = 0, posZ = 0;
    double prevPosX = 0, prevPosY = 0, prevPosZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0;
    bool onGround = false;
    bool isDead = false;
    int32_t entityId = 0;

    // ─── XP-specific ───
    int32_t xpColor = 0;    // Animation timer
    int32_t xpOrbAge = 0;
    int32_t pickupDelay = 0; // field_70532_c
    int32_t xpOrbHealth = 5;
    int32_t xpValue = 0;
    int32_t xpTargetColor = 0;

    // ─── Constants ───
    static constexpr float WIDTH_SPAWN = 0.5f;
    static constexpr float HEIGHT_SPAWN = 0.5f;
    static constexpr float WIDTH_DEFAULT = 0.25f;
    static constexpr float HEIGHT_DEFAULT = 0.25f;
    static constexpr float GRAVITY = 0.03f;
    static constexpr float AIR_FRICTION = 0.98f;
    static constexpr float GROUND_BOUNCE = -0.9f;
    static constexpr double ATTRACTION_RANGE = 8.0;
    static constexpr double ATTRACTION_FORCE = 0.1;
    static constexpr int32_t PICKUP_COOLDOWN = 2;
    static constexpr int32_t DESPAWN_AGE = 6000;

    // ═══════════════════════════════════════════════════════════════════════
    // Construction
    // Java: EntityXPOrb(world, x, y, z, xpValue)
    //   - Motion: random ±0.1 * 2.0 (XZ), random 0-0.2 * 2.0 (Y)
    // ═══════════════════════════════════════════════════════════════════════

    void spawn(double x, double y, double z, int32_t xp) {
        posX = x; posY = y; posZ = z;
        xpValue = xp;
        rotationYaw = static_cast<float>(static_cast<double>(rand()) / RAND_MAX * 360.0);
        motionX = (static_cast<double>(rand()) / RAND_MAX * 0.2 - 0.1) * 2.0;
        motionY = (static_cast<double>(rand()) / RAND_MAX * 0.2) * 2.0;
        motionZ = (static_cast<double>(rand()) / RAND_MAX * 0.2 - 0.1) * 2.0;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // onUpdate — Physics with player attraction.
    // Java: EntityXPOrb.onUpdate
    //   - Gravity: 0.03
    //   - Lava: bob up
    //   - Player search: every (xpColor - 20 + entityId%100) ticks
    //   - Attraction: inverse distance within 8 blocks, force 0.1
    //   - Ground friction: slipperiness * 0.98
    //   - Ground bounce: -0.9
    //   - Despawn at 6000 ticks
    // ═══════════════════════════════════════════════════════════════════════

    struct PlayerInfo {
        double playerX, playerY, playerZ;
        float eyeHeight;
        double distSq;
        bool valid;
    };

    void onUpdate(float groundSlipperiness, bool inLava, const PlayerInfo& nearestPlayer) {
        if (pickupDelay > 0) --pickupDelay;

        prevPosX = posX; prevPosY = posY; prevPosZ = posZ;

        // Gravity
        motionY -= GRAVITY;

        // Lava bob
        if (inLava) {
            motionY = 0.2;
            // XZ randomized by caller
        }

        // Player attraction
        if (nearestPlayer.valid &&
            xpTargetColor < xpColor - 20 + (entityId % 100)) {
            xpTargetColor = xpColor;

            if (nearestPlayer.distSq <= ATTRACTION_RANGE * ATTRACTION_RANGE) {
                double range = ATTRACTION_RANGE;
                double dx = (nearestPlayer.playerX - posX) / range;
                double dy = (nearestPlayer.playerY + nearestPlayer.eyeHeight - posY) / range;
                double dz = (nearestPlayer.playerZ - posZ) / range;
                double dist = std::sqrt(dx * dx + dy * dy + dz * dz);
                double factor = 1.0 - dist;
                if (factor > 0) {
                    factor *= factor;
                    motionX += (dx / dist) * factor * ATTRACTION_FORCE;
                    motionY += (dy / dist) * factor * ATTRACTION_FORCE;
                    motionZ += (dz / dist) * factor * ATTRACTION_FORCE;
                }
            }
        }

        // Friction
        float friction = AIR_FRICTION;
        if (onGround) {
            friction = groundSlipperiness * 0.98f;
        }
        motionX *= friction;
        motionY *= AIR_FRICTION;
        motionZ *= friction;

        if (onGround) {
            motionY *= GROUND_BOUNCE;
        }

        ++xpColor;
        ++xpOrbAge;
        if (xpOrbAge >= DESPAWN_AGE) {
            isDead = true;
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Damage
    // ═══════════════════════════════════════════════════════════════════════

    bool attackEntityFrom(float damage, bool isInvulnerable) {
        if (isInvulnerable) return false;
        xpOrbHealth -= static_cast<int32_t>(damage);
        if (xpOrbHealth <= 0) isDead = true;
        return false;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Pickup
    // Java: EntityXPOrb.onCollideWithPlayer
    //   - pickupDelay == 0 && player.xpCooldown == 0
    //   - Sets player.xpCooldown = 2
    //   - Plays "random.orb"
    //   - Adds xpValue experience
    // ═══════════════════════════════════════════════════════════════════════

    struct PickupResult {
        bool pickedUp;
        int32_t xpAmount;
    };

    PickupResult tryPickup(int32_t playerXpCooldown) {
        if (pickupDelay != 0 || playerXpCooldown != 0) return {false, 0};
        isDead = true;
        return {true, xpValue};
    }

    // ═══════════════════════════════════════════════════════════════════════
    // XP split — determines orb size when splitting large XP amounts.
    // Java: EntityXPOrb.getXPSplit
    // Thresholds: 2477, 1237, 617, 307, 149, 73, 37, 17, 7, 3, 1
    //   These are based on the formula: floor(level_cost(n)) where level_cost
    //   follows the 1.7.10 experience table.
    // ═══════════════════════════════════════════════════════════════════════

    static int32_t getXPSplit(int32_t totalXP) {
        if (totalXP >= 2477) return 2477;
        if (totalXP >= 1237) return 1237;
        if (totalXP >= 617) return 617;
        if (totalXP >= 307) return 307;
        if (totalXP >= 149) return 149;
        if (totalXP >= 73) return 73;
        if (totalXP >= 37) return 37;
        if (totalXP >= 17) return 17;
        if (totalXP >= 7) return 7;
        if (totalXP >= 3) return 3;
        return 1;
    }
};

} // namespace mccpp
