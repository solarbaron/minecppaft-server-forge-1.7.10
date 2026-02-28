/**
 * EntityAttributes.h — Entity attribute system with modifiers.
 *
 * Java references:
 *   - net.minecraft.entity.SharedMonsterAttributes — 5 vanilla attributes
 *   - net.minecraft.entity.ai.attributes.IAttribute — Attribute definition
 *   - net.minecraft.entity.ai.attributes.RangedAttribute — Min/max clamped
 *   - net.minecraft.entity.ai.attributes.AttributeModifier — UUID-keyed modifier
 *   - net.minecraft.entity.ai.attributes.ModifiableAttributeInstance — Instance with modifiers
 *
 * Modifier operations:
 *   0: Add — base += amount
 *   1: Multiply base — result += base * amount (additive multiply)
 *   2: Multiply total — result *= (1.0 + amount)
 *
 * Thread safety: Per-entity, accessed from server thread.
 *
 * JNI readiness: String-based attribute names, UUID as two int64s.
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// AttributeModifier — UUID-identified value modifier.
// Java reference: net.minecraft.entity.ai.attributes.AttributeModifier
// ═══════════════════════════════════════════════════════════════════════════

struct AttributeModifier {
    int64_t uuidMost = 0;
    int64_t uuidLeast = 0;
    std::string name;
    double amount = 0.0;
    int32_t operation = 0; // 0=add, 1=multiply_base, 2=multiply_total
    bool saved = true;     // Java: isSaved

    bool operator==(const AttributeModifier& other) const {
        return uuidMost == other.uuidMost && uuidLeast == other.uuidLeast;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// AttributeDefinition — Defines an attribute's identity and range.
// Java reference: net.minecraft.entity.ai.attributes.RangedAttribute
// ═══════════════════════════════════════════════════════════════════════════

struct AttributeDefinition {
    std::string unlocalizedName; // e.g., "generic.maxHealth"
    std::string description;     // e.g., "Max Health"
    double defaultValue = 0.0;
    double minValue = 0.0;
    double maxValue = std::numeric_limits<double>::max();
    bool shouldWatch = false;    // Java: setShouldWatch — sync to client

    double clamp(double value) const {
        return std::clamp(value, minValue, maxValue);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// AttributeInstance — Live attribute value with applied modifiers.
// Java reference: net.minecraft.entity.ai.attributes.ModifiableAttributeInstance
//
// Computation order (Java exact):
//   1. Start with baseValue
//   2. Add all operation=0 modifiers
//   3. Multiply: result += base * sum(operation=1 amounts)
//   4. Multiply: result *= product(1.0 + operation=2 amounts)
//   5. Clamp to [min, max]
// ═══════════════════════════════════════════════════════════════════════════

struct AttributeInstance {
    const AttributeDefinition* definition = nullptr;
    double baseValue = 0.0;
    std::vector<AttributeModifier> modifiers;
    mutable bool dirty = true;
    mutable double cachedValue = 0.0;

    AttributeInstance() = default;
    explicit AttributeInstance(const AttributeDefinition* def)
        : definition(def), baseValue(def ? def->defaultValue : 0.0) {}

    double getBaseValue() const { return baseValue; }

    void setBaseValue(double value) {
        baseValue = value;
        dirty = true;
    }

    // Java: applyModifier
    void applyModifier(const AttributeModifier& mod) {
        // Remove existing with same UUID
        removeModifierByUUID(mod.uuidMost, mod.uuidLeast);
        modifiers.push_back(mod);
        dirty = true;
    }

    // Java: removeModifier
    void removeModifier(const AttributeModifier& mod) {
        removeModifierByUUID(mod.uuidMost, mod.uuidLeast);
        dirty = true;
    }

    // Java: getModifier
    const AttributeModifier* getModifier(int64_t uuidMost, int64_t uuidLeast) const {
        for (const auto& m : modifiers) {
            if (m.uuidMost == uuidMost && m.uuidLeast == uuidLeast) return &m;
        }
        return nullptr;
    }

    // Java: getAttributeValue — compute final value with all modifiers
    double getAttributeValue() const {
        if (!dirty) return cachedValue;

        double value = baseValue;

        // Operation 0: additive
        for (const auto& m : modifiers) {
            if (m.operation == 0) value += m.amount;
        }

        double base = value;

        // Operation 1: multiply base (additive with base)
        for (const auto& m : modifiers) {
            if (m.operation == 1) value += base * m.amount;
        }

        // Operation 2: multiply total
        for (const auto& m : modifiers) {
            if (m.operation == 2) value *= (1.0 + m.amount);
        }

        // Clamp
        if (definition) {
            value = definition->clamp(value);
        }

        cachedValue = value;
        dirty = false;
        return value;
    }

    // Java: func_111122_c — get all modifiers (for NBT saving)
    const std::vector<AttributeModifier>& getAllModifiers() const { return modifiers; }

private:
    void removeModifierByUUID(int64_t most, int64_t least) {
        modifiers.erase(
            std::remove_if(modifiers.begin(), modifiers.end(),
                [most, least](const AttributeModifier& m) {
                    return m.uuidMost == most && m.uuidLeast == least;
                }),
            modifiers.end()
        );
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// AttributeMap — Collection of attribute instances for one entity.
// Java reference: net.minecraft.entity.ai.attributes.BaseAttributeMap
// ═══════════════════════════════════════════════════════════════════════════

class AttributeMap {
public:
    void registerAttribute(const AttributeDefinition* def) {
        instances_[def->unlocalizedName] = AttributeInstance(def);
    }

    AttributeInstance* getInstance(const std::string& name) {
        auto it = instances_.find(name);
        return it != instances_.end() ? &it->second : nullptr;
    }

    const AttributeInstance* getInstance(const std::string& name) const {
        auto it = instances_.find(name);
        return it != instances_.end() ? &it->second : nullptr;
    }

    const std::unordered_map<std::string, AttributeInstance>& getAll() const {
        return instances_;
    }

private:
    std::unordered_map<std::string, AttributeInstance> instances_;
};

// ═══════════════════════════════════════════════════════════════════════════
// SharedMonsterAttributes — 5 vanilla attribute definitions.
// Java reference: net.minecraft.entity.SharedMonsterAttributes
// ═══════════════════════════════════════════════════════════════════════════

namespace SharedMonsterAttributes {

    // Java: generic.maxHealth — default 20.0, range [0, MAX]
    inline const AttributeDefinition maxHealth = {
        "generic.maxHealth", "Max Health", 20.0, 0.0,
        std::numeric_limits<double>::max(), true
    };

    // Java: generic.followRange — default 32.0, range [0, 2048]
    inline const AttributeDefinition followRange = {
        "generic.followRange", "Follow Range", 32.0, 0.0, 2048.0, false
    };

    // Java: generic.knockbackResistance — default 0.0, range [0, 1]
    inline const AttributeDefinition knockbackResistance = {
        "generic.knockbackResistance", "Knockback Resistance", 0.0, 0.0, 1.0, false
    };

    // Java: generic.movementSpeed — default 0.7, range [0, MAX]
    inline const AttributeDefinition movementSpeed = {
        "generic.movementSpeed", "Movement Speed", 0.699999988079071, 0.0,
        std::numeric_limits<double>::max(), true
    };

    // Java: generic.attackDamage — default 2.0, range [0, MAX]
    inline const AttributeDefinition attackDamage = {
        "generic.attackDamage", "Attack Damage", 2.0, 0.0,
        std::numeric_limits<double>::max(), false
    };

    // Register all 5 attributes on an entity
    inline void registerDefaults(AttributeMap& map) {
        map.registerAttribute(&maxHealth);
        map.registerAttribute(&followRange);
        map.registerAttribute(&knockbackResistance);
        map.registerAttribute(&movementSpeed);
        map.registerAttribute(&attackDamage);
    }

    // NBT field names
    constexpr const char* NBT_NAME = "Name";
    constexpr const char* NBT_BASE = "Base";
    constexpr const char* NBT_MODIFIERS = "Modifiers";
    constexpr const char* NBT_MOD_NAME = "Name";
    constexpr const char* NBT_MOD_AMOUNT = "Amount";
    constexpr const char* NBT_MOD_OPERATION = "Operation";
    constexpr const char* NBT_MOD_UUID_MOST = "UUIDMost";
    constexpr const char* NBT_MOD_UUID_LEAST = "UUIDLeast";
}

// ═══════════════════════════════════════════════════════════════════════════
// Common modifier UUIDs used in vanilla
// Java reference: Various entity classes
// ═══════════════════════════════════════════════════════════════════════════

namespace AttributeUUIDs {
    // Sprint speed boost: EntityLivingBase
    constexpr int64_t SPRINT_SPEED_MOST  = -5765578904828771824LL;  // 662A6B8D-DA3E-4C1C-...
    constexpr int64_t SPRINT_SPEED_LEAST = -8859020046780227981LL;

    // Baby zombie speed boost
    constexpr int64_t BABY_SPEED_MOST    = -4897891277128498952LL;
    constexpr int64_t BABY_SPEED_LEAST   = -7443276725661046067LL;

    // Potion effects
    constexpr int64_t HEALTH_BOOST_MOST  = 0x5D6F0BA2LL;
    constexpr int64_t HEALTH_BOOST_LEAST = 0LL;
}

} // namespace mccpp
