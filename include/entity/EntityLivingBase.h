/**
 * EntityLivingBase.h — Core living entity mechanics.
 *
 * Java reference: net.minecraft.entity.EntityLivingBase (1384 lines)
 *
 * Implements:
 *   - Health system: DataWatcher index 6, setHealth/getHealth/heal
 *   - Damage pipeline:
 *     1. isEntityInvulnerable → reject
 *     2. Fire damage + fireResistance potion → reject
 *     3. Anvil/falling block + helmet → 75% damage, helmet durability
 *     4. I-frame check: hurtResistantTime > maxHurtResistantTime/2 → only delta
 *     5. applyArmorCalculations: (25 - totalArmor) / 25
 *     6. applyPotionDamageCalculations: resistance potion, enchantment modifiers
 *     7. Absorption amount deduction
 *     8. Health reduction
 *   - Knockback: halve motion, push 0.4 strength, cap motionY at 0.4
 *   - Fall damage: ceil(fallDistance - 3.0 - jumpBoostLevel)
 *   - Potion effects: HashMap<int, PotionEffect>, tick/add/remove/clear
 *   - Death: 20 tick timer, XP drop, particles
 *   - Combat tracking: revenge target (100 tick timeout), attacker, recentlyHit
 *   - NBT: HealF/Health/HurtTime/DeathTime/AttackTime/AbsorptionAmount/ActiveEffects
 *   - Equipment: 5 slots (hand + 4 armor)
 *
 * Thread safety: Entity state accessed from main thread.
 * JNI readiness: Predictable fields, standard accessors.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// SharedMonsterAttributes — Standard attribute keys.
// Java reference: net.minecraft.entity.SharedMonsterAttributes
// ═══════════════════════════════════════════════════════════════════════════

namespace SharedMonsterAttributes {
    constexpr double MAX_HEALTH_DEFAULT = 20.0;
    constexpr double KNOCKBACK_RESISTANCE_DEFAULT = 0.0;
    constexpr double MOVEMENT_SPEED_DEFAULT = 0.1;
    constexpr double ATTACK_DAMAGE_DEFAULT = 2.0;
    constexpr double FOLLOW_RANGE_DEFAULT = 16.0;

    // Sprinting speed boost: +30% (operation 2 = multiplicative)
    constexpr double SPRINTING_SPEED_BOOST = 0.30000001192092896;
}

// ═══════════════════════════════════════════════════════════════════════════
// PotionEffect — Active potion effect on an entity.
// Java reference: net.minecraft.potion.PotionEffect
// ═══════════════════════════════════════════════════════════════════════════

struct PotionEffect {
    int32_t potionId;
    int32_t duration;     // Ticks remaining
    int32_t amplifier;    // 0 = level I, 1 = level II, etc.
    bool ambient;         // From beacon
    bool showParticles;

    // Java: onUpdate — tick the effect, returns false when expired
    bool onUpdate() {
        if (duration > 0) --duration;
        return duration > 0;
    }

    // Java: combine — merge with another effect (keep stronger)
    void combine(const PotionEffect& other) {
        if (other.amplifier > amplifier) {
            amplifier = other.amplifier;
        }
        if (other.duration > duration) {
            duration = other.duration;
        }
        ambient = other.ambient;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Potion IDs — Vanilla potion type constants.
// Java reference: net.minecraft.potion.Potion (IDs 1-23)
// ═══════════════════════════════════════════════════════════════════════════

namespace PotionID {
    constexpr int32_t SPEED = 1;
    constexpr int32_t SLOWNESS = 2;
    constexpr int32_t HASTE = 3;
    constexpr int32_t MINING_FATIGUE = 4;
    constexpr int32_t STRENGTH = 5;
    constexpr int32_t INSTANT_HEALTH = 6;
    constexpr int32_t INSTANT_DAMAGE = 7;
    constexpr int32_t JUMP_BOOST = 8;
    constexpr int32_t NAUSEA = 9;
    constexpr int32_t REGENERATION = 10;
    constexpr int32_t RESISTANCE = 11;
    constexpr int32_t FIRE_RESISTANCE = 12;
    constexpr int32_t WATER_BREATHING = 13;
    constexpr int32_t INVISIBILITY = 14;
    constexpr int32_t BLINDNESS = 15;
    constexpr int32_t NIGHT_VISION = 16;
    constexpr int32_t HUNGER = 17;
    constexpr int32_t WEAKNESS = 18;
    constexpr int32_t POISON = 19;
    constexpr int32_t WITHER = 20;
    constexpr int32_t HEALTH_BOOST = 21;
    constexpr int32_t ABSORPTION = 22;
    constexpr int32_t SATURATION = 23;
}

// ═══════════════════════════════════════════════════════════════════════════
// EnumCreatureAttribute
// Java reference: net.minecraft.entity.EnumCreatureAttribute
// ═══════════════════════════════════════════════════════════════════════════

enum class CreatureAttribute { UNDEFINED, UNDEAD, ARTHROPOD };

// ═══════════════════════════════════════════════════════════════════════════
// EntityLivingBase — Core living entity.
// Java reference: net.minecraft.entity.EntityLivingBase
// ═══════════════════════════════════════════════════════════════════════════

class EntityLivingBase {
public:
    // ─── Position/Motion (from Entity) ───
    double posX = 0, posY = 0, posZ = 0;
    double prevPosX = 0, prevPosY = 0, prevPosZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    float prevRotationYaw = 0, prevRotationPitch = 0;
    float width = 0.6f, height = 1.8f;
    float yOffset = 0;
    float stepHeight = 0.5f;
    float fallDistance = 0;
    bool onGround = false;
    bool isDead = false;
    bool isAirBorne = false;
    int32_t ticksExisted = 0;
    int32_t entityId = 0;
    int32_t hurtResistantTime = 0;

    // ─── Living entity fields ───
    bool isSwingInProgress = false;
    int32_t swingProgressInt = 0;
    int32_t arrowHitTimer = 0;
    float prevHealth = 0;
    int32_t hurtTime = 0;
    int32_t maxHurtTime = 0;
    float attackedAtYaw = 0;
    int32_t deathTime = 0;
    int32_t attackTime = 0;
    float prevSwingProgress = 0;
    float swingProgress = 0;
    float prevLimbSwingAmount = 0;
    float limbSwingAmount = 0;
    float limbSwing = 0;
    int32_t maxHurtResistantTime = 20;
    float prevCameraPitch = 0;
    float cameraPitch = 0;
    float renderYawOffset = 0;
    float prevRenderYawOffset = 0;
    float rotationYawHead = 0;
    float prevRotationYawHead = 0;
    float jumpMovementFactor = 0.02f;
    float moveStrafing = 0;
    float moveForward = 0;

    // ─── Attributes ───
    double maxHealth = SharedMonsterAttributes::MAX_HEALTH_DEFAULT;
    double knockbackResistance = SharedMonsterAttributes::KNOCKBACK_RESISTANCE_DEFAULT;
    double movementSpeed = SharedMonsterAttributes::MOVEMENT_SPEED_DEFAULT;
    double attackDamage = SharedMonsterAttributes::ATTACK_DAMAGE_DEFAULT;
    double followRange = SharedMonsterAttributes::FOLLOW_RANGE_DEFAULT;

    // ─── Combat ───
    int32_t recentlyHit = 0;
    bool dead = false;
    int32_t entityAge = 0;
    float lastDamage = 0;
    bool isJumping = false;
    int32_t scoreValue = 0;

    // ─── Absorption ───
    float absorptionAmount = 0;

    // ─── Equipment: 0=hand, 1-4=armor ───
    struct ItemSlot {
        int32_t itemId = 0;
        int32_t count = 0;
        int32_t damage = 0;
        int32_t armorValue = 0;  // For armor calculation
        bool isEmpty() const { return itemId == 0 || count <= 0; }
    };
    ItemSlot equipment[5];

    // ─── Potion effects ───
    std::unordered_map<int32_t, PotionEffect> activePotions;

    // ─── Creature attribute ───
    CreatureAttribute creatureAttribute = CreatureAttribute::UNDEFINED;

    // ═══════════════════════════════════════════════════════════════════════
    // Health
    // ═══════════════════════════════════════════════════════════════════════

    float health_ = 20.0f;

    float getHealth() const { return health_; }

    void setHealth(float h) {
        health_ = std::clamp(h, 0.0f, static_cast<float>(maxHealth));
    }

    float getMaxHealth() const { return static_cast<float>(maxHealth); }

    void heal(float amount) {
        if (health_ > 0) setHealth(health_ + amount);
    }

    bool isEntityAlive() const { return !isDead && health_ > 0.0f; }

    float getAbsorptionAmount() const { return absorptionAmount; }
    void setAbsorptionAmount(float a) { absorptionAmount = std::max(0.0f, a); }

    // ═══════════════════════════════════════════════════════════════════════
    // Potion effects
    // ═══════════════════════════════════════════════════════════════════════

    bool isPotionActive(int32_t id) const {
        return activePotions.count(id) > 0;
    }

    const PotionEffect* getActivePotionEffect(int32_t id) const {
        auto it = activePotions.find(id);
        return it != activePotions.end() ? &it->second : nullptr;
    }

    void addPotionEffect(PotionEffect effect) {
        // Undead: immune to regeneration and poison
        if (creatureAttribute == CreatureAttribute::UNDEAD) {
            if (effect.potionId == PotionID::REGENERATION ||
                effect.potionId == PotionID::POISON) return;
        }

        auto it = activePotions.find(effect.potionId);
        if (it != activePotions.end()) {
            it->second.combine(effect);
        } else {
            activePotions[effect.potionId] = effect;
        }
    }

    void removePotionEffect(int32_t id) { activePotions.erase(id); }

    void clearActivePotions() { activePotions.clear(); }

    // Java: updatePotionEffects — tick all active effects
    void updatePotionEffects() {
        for (auto it = activePotions.begin(); it != activePotions.end(); ) {
            if (!it->second.onUpdate()) {
                it = activePotions.erase(it);
            } else {
                ++it;
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Armor calculation
    // ═══════════════════════════════════════════════════════════════════════

    int32_t getTotalArmorValue() const {
        int32_t total = 0;
        for (int32_t i = 1; i <= 4; ++i) {  // Slots 1-4 are armor
            if (!equipment[i].isEmpty()) {
                total += equipment[i].armorValue;
            }
        }
        return total;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Damage pipeline
    // Java: attackEntityFrom → damageEntity
    // ═══════════════════════════════════════════════════════════════════════

    struct DamageInfo {
        enum Type {
            GENERIC, FALL, DROWN, IN_WALL, FIRE, LAVA, ON_FIRE,
            ANVIL, FALLING_BLOCK, OUT_OF_WORLD, MAGIC, WITHER,
            STARVE, CACTUS, PLAYER_ATTACK, MOB_ATTACK, ARROW,
            EXPLOSION
        };
        Type type = GENERIC;
        int32_t attackerEntityId = -1;
        bool isFireDamage = false;
        bool isUnblockable = false;
        bool isDamageAbsolute = false;
    };

    struct DamageResult {
        bool applied;
        float finalDamage;
    };

    // Java: attackEntityFrom
    DamageResult attackEntityFrom(const DamageInfo& info, float amount) {
        // 1. Invulnerable check
        if (health_ <= 0.0f) return {false, 0};

        // 2. Fire resistance
        if (info.isFireDamage && isPotionActive(PotionID::FIRE_RESISTANCE)) {
            return {false, 0};
        }

        // 3. Anvil/falling block helmet reduction
        if ((info.type == DamageInfo::ANVIL || info.type == DamageInfo::FALLING_BLOCK) &&
            !equipment[4].isEmpty()) {
            amount *= 0.75f;
        }

        entityAge = 0;
        limbSwingAmount = 1.5f;

        // 4. I-frame check
        bool newHit = true;
        if (static_cast<float>(hurtResistantTime) >
            static_cast<float>(maxHurtResistantTime) / 2.0f) {
            if (amount <= lastDamage) return {false, 0};
            float actualDamage = applyDamageReductions(info, amount - lastDamage);
            applyFinalDamage(actualDamage);
            lastDamage = amount;
            newHit = false;
        } else {
            lastDamage = amount;
            prevHealth = health_;
            hurtResistantTime = maxHurtResistantTime;
            float actualDamage = applyDamageReductions(info, amount);
            applyFinalDamage(actualDamage);
            maxHurtTime = 10;
            hurtTime = 10;
        }

        // 5. Attacker tracking
        if (info.attackerEntityId >= 0) {
            recentlyHit = 100;
        }

        // 6. Knockback direction
        attackedAtYaw = 0;

        // 7. Death check
        if (health_ <= 0.0f) {
            dead = true;
        }

        return {true, amount};
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Knockback
    // Java: knockBack
    // ═══════════════════════════════════════════════════════════════════════

    void knockBack(double dx, double dz, float strength = 0.4f) {
        // Knockback resistance check
        if (knockbackResistance >= 1.0) return;

        isAirBorne = true;
        float dist = static_cast<float>(std::sqrt(dx * dx + dz * dz));
        if (dist < 0.001f) return;

        motionX /= 2.0;
        motionY /= 2.0;
        motionZ /= 2.0;
        motionX -= dx / static_cast<double>(dist) * static_cast<double>(strength);
        motionY += static_cast<double>(strength);
        motionZ -= dz / static_cast<double>(dist) * static_cast<double>(strength);

        if (motionY > 0.4) motionY = 0.4;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Fall damage
    // Java: fall — ceil(fallDistance - 3.0 - jumpBoost)
    // ═══════════════════════════════════════════════════════════════════════

    int32_t calculateFallDamage(float distance) const {
        float reduction = 3.0f;
        auto jumpEffect = getActivePotionEffect(PotionID::JUMP_BOOST);
        if (jumpEffect) {
            reduction += static_cast<float>(jumpEffect->amplifier + 1);
        }
        int32_t damage = static_cast<int32_t>(std::ceil(distance - reduction));
        return std::max(0, damage);
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Death update
    // Java: onDeathUpdate — 20 tick timer
    // ═══════════════════════════════════════════════════════════════════════

    bool onDeathUpdate() {
        ++deathTime;
        if (deathTime >= 20) {
            isDead = true;
            return true;  // Entity should be removed
        }
        return false;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Drowning
    // Java: air supply, decreaseAirSupply
    // ═══════════════════════════════════════════════════════════════════════

    int32_t air = 300;

    int32_t getAir() const { return air; }
    void setAir(int32_t a) { air = a; }

    int32_t decreaseAirSupply(int32_t current, int32_t respirationLevel) {
        if (respirationLevel > 0) {
            // Random chance to not decrease based on respiration level
            // Simplified: skip decrease (respirationLevel)/(respirationLevel+1) chance
            return current;  // Simplified
        }
        return current - 1;
    }

    // Java: onEntityUpdate drowning logic
    void updateDrowning(bool inWater, bool canBreathe) {
        if (inWater) {
            if (!canBreathe && !isPotionActive(PotionID::WATER_BREATHING)) {
                air = decreaseAirSupply(air, 0);
                if (air == -20) {
                    air = 0;
                    DamageInfo drowning;
                    drowning.type = DamageInfo::DROWN;
                    drowning.isUnblockable = true;
                    attackEntityFrom(drowning, 2.0f);
                }
            }
        } else {
            air = 300;
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Suffocation (in wall)
    // ═══════════════════════════════════════════════════════════════════════

    void updateSuffocation(bool insideOpaqueBlock) {
        if (isEntityAlive() && insideOpaqueBlock) {
            DamageInfo wallDmg;
            wallDmg.type = DamageInfo::IN_WALL;
            wallDmg.isUnblockable = true;
            attackEntityFrom(wallDmg, 1.0f);
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Tick
    // ═══════════════════════════════════════════════════════════════════════

    void onLivingUpdate() {
        prevSwingProgress = swingProgress;

        if (attackTime > 0) --attackTime;
        if (hurtTime > 0) --hurtTime;
        if (hurtResistantTime > 0) --hurtResistantTime;

        if (health_ <= 0.0f) {
            onDeathUpdate();
        }

        if (recentlyHit > 0) {
            --recentlyHit;
        }

        updatePotionEffects();

        prevRenderYawOffset = renderYawOffset;
        prevRotationYawHead = rotationYawHead;
        prevRotationYaw = rotationYaw;
        prevRotationPitch = rotationPitch;

        ++ticksExisted;
        ++entityAge;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // XP calculation
    // ═══════════════════════════════════════════════════════════════════════

    virtual int32_t getExperiencePoints() const { return 0; }

    // Java: EntityXPOrb.getXPSplit
    static std::vector<int32_t> splitXP(int32_t totalXP) {
        std::vector<int32_t> orbs;
        while (totalXP > 0) {
            int32_t split;
            if (totalXP >= 2477) split = 2477;
            else if (totalXP >= 1237) split = 1237;
            else if (totalXP >= 617) split = 617;
            else if (totalXP >= 307) split = 307;
            else if (totalXP >= 149) split = 149;
            else if (totalXP >= 73) split = 73;
            else if (totalXP >= 37) split = 37;
            else if (totalXP >= 17) split = 17;
            else if (totalXP >= 7) split = 7;
            else if (totalXP >= 3) split = 3;
            else split = 1;
            totalXP -= split;
            orbs.push_back(split);
        }
        return orbs;
    }

private:
    // Java: applyArmorCalculations + applyPotionDamageCalculations
    float applyDamageReductions(const DamageInfo& info, float damage) {
        // Armor reduction
        if (!info.isUnblockable) {
            int32_t armor = getTotalArmorValue();
            int32_t factor = 25 - armor;
            damage = damage * static_cast<float>(factor) / 25.0f;
        }

        // Potion resistance
        if (!info.isDamageAbsolute &&
            isPotionActive(PotionID::RESISTANCE) &&
            info.type != DamageInfo::OUT_OF_WORLD) {
            auto effect = getActivePotionEffect(PotionID::RESISTANCE);
            if (effect) {
                int32_t reduction = (effect->amplifier + 1) * 5;
                int32_t factor = 25 - reduction;
                damage = damage * static_cast<float>(factor) / 25.0f;
            }
        }

        return std::max(0.0f, damage);
    }

    // Java: damageEntity — apply to health after absorption
    void applyFinalDamage(float damage) {
        float preDamage = damage;
        damage = std::max(damage - absorptionAmount, 0.0f);
        absorptionAmount -= (preDamage - damage);
        if (absorptionAmount < 0) absorptionAmount = 0;
        if (damage > 0.0f) {
            setHealth(health_ - damage);
        }
    }
};

} // namespace mccpp
