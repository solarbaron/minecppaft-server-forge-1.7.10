/**
 * PotionEffect.h — Potion/status effect system.
 *
 * Java references:
 *   - net.minecraft.potion.Potion — Effect definitions, performEffect, isReady
 *   - net.minecraft.potion.PotionEffect — Duration, amplifier, ambient tracking
 *   - net.minecraft.potion.PotionHealth — Instant heal/harm
 *   - net.minecraft.potion.PotionAttackDamage — Strength/weakness modifiers
 *   - net.minecraft.potion.PotionAbsoption — Absorption hearts
 *   - net.minecraft.potion.PotionHealthBoost — Max health increase
 *
 * All 23 vanilla 1.7.10 potion effects with exact IDs, colors, tick intervals,
 * and attribute modifier values from the decompiled source.
 *
 * Thread safety:
 *   - Potion registry is static/const after initialization.
 *   - PotionEffect instances are per-entity (owned by entity, no sharing).
 *   - Attribute modifier application is synchronized per-entity.
 *
 * JNI readiness: Integer potion IDs and simple structs for JVM mapping.
 */
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// AttributeModifier — Simplified attribute modifier for potion effects.
// Java reference: net.minecraft.entity.ai.attributes.AttributeModifier
// ═══════════════════════════════════════════════════════════════════════════

struct AttributeModifier {
    std::string uuid;
    std::string name;
    double amount;
    int32_t operation; // 0=add, 1=multiply_base, 2=multiply_total
};

// ═══════════════════════════════════════════════════════════════════════════
// Potion — Potion effect definition.
// Java reference: net.minecraft.potion.Potion
//
// Each potion has:
//   - id: unique ID (1-23)
//   - isBadEffect: negative effects (slowness, poison, etc.)
//   - liquidColor: RGB color for particles/display
//   - effectiveness: controls tick rate scaling (0.25 for fast-tick effects)
//   - isInstant: true for heal/harm (single application, no duration)
//   - attributeModifiers: passive stat changes while active
// ═══════════════════════════════════════════════════════════════════════════

struct Potion {
    int32_t id;
    std::string name;
    bool isBadEffect;
    int32_t liquidColor;    // RGB packed color
    double effectiveness;   // Tick rate modifier (lower = more frequent)
    bool isInstant;         // Heal/Harm: applied once, no duration

    // Attribute modifiers applied while effect is active
    // Key: attribute name (e.g. "generic.movementSpeed")
    struct AttrMod {
        std::string attribute;
        AttributeModifier modifier;
    };
    std::vector<AttrMod> attributeModifiers;

    // Java: Potion.isReady(duration, amplifier)
    // Returns true when the effect should tick at this duration
    bool isReady(int32_t duration, int32_t amplifier) const;

    // Java: Potion.func_111183_a(amplifier, modifier)
    // Scale modifier amount by amplifier: amount * (amplifier + 1)
    double getScaledAmount(int32_t amplifier, const AttributeModifier& mod) const {
        return mod.amount * static_cast<double>(amplifier + 1);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// PotionEffect — Active effect instance on an entity.
// Java reference: net.minecraft.potion.PotionEffect
//
// Tracks duration countdown, amplifier level, and ambient state.
// Ticked every server tick; calls Potion.isReady to determine when to apply.
// ═══════════════════════════════════════════════════════════════════════════

struct PotionEffect {
    int32_t potionId;
    int32_t duration;     // Remaining ticks (0 = expired)
    int32_t amplifier;    // Level 0 = I, 1 = II, etc.
    bool ambient;         // From beacon (no particles, doesn't show in HUD)
    bool showParticles;   // Show swirl particles

    PotionEffect() : potionId(0), duration(0), amplifier(0),
                     ambient(false), showParticles(true) {}

    PotionEffect(int32_t id, int32_t dur, int32_t amp, bool amb = false)
        : potionId(id), duration(dur), amplifier(amp),
          ambient(amb), showParticles(true) {}

    // Java: PotionEffect.onUpdate()
    // Returns true if effect is still active
    bool onUpdate();

    // Java: PotionEffect.getIsPotionDurationMax()
    bool isMaxDuration() const { return duration == 32767; }

    // Java: PotionEffect.combine(PotionEffect)
    // Merge with another effect: keep higher amplifier, longer duration
    void combine(const PotionEffect& other);

    // Java: PotionEffect.deincrementDuration()
    int32_t decrementDuration() {
        if (duration > 0) --duration;
        return duration;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// PotionRegistry — Static registry of all vanilla potions.
// Java reference: net.minecraft.potion.Potion static fields
//
// Thread safety: initialized once at startup, read-only after.
// ═══════════════════════════════════════════════════════════════════════════

class PotionRegistry {
public:
    static void init();
    static const Potion* getById(int32_t id);
    static const std::vector<Potion>& getAll();
    static int32_t getCount();

    // Potion IDs — matching Java's Potion static fields
    static constexpr int32_t SPEED          = 1;
    static constexpr int32_t SLOWNESS       = 2;
    static constexpr int32_t HASTE          = 3;
    static constexpr int32_t MINING_FATIGUE = 4;
    static constexpr int32_t STRENGTH       = 5;
    static constexpr int32_t INSTANT_HEALTH = 6;
    static constexpr int32_t INSTANT_DAMAGE = 7;
    static constexpr int32_t JUMP_BOOST     = 8;
    static constexpr int32_t NAUSEA         = 9;
    static constexpr int32_t REGENERATION   = 10;
    static constexpr int32_t RESISTANCE     = 11;
    static constexpr int32_t FIRE_RESISTANCE = 12;
    static constexpr int32_t WATER_BREATHING = 13;
    static constexpr int32_t INVISIBILITY   = 14;
    static constexpr int32_t BLINDNESS      = 15;
    static constexpr int32_t NIGHT_VISION   = 16;
    static constexpr int32_t HUNGER         = 17;
    static constexpr int32_t WEAKNESS       = 18;
    static constexpr int32_t POISON         = 19;
    static constexpr int32_t WITHER         = 20;
    static constexpr int32_t HEALTH_BOOST   = 21;
    static constexpr int32_t ABSORPTION     = 22;
    static constexpr int32_t SATURATION     = 23;

private:
    static std::vector<Potion> potions_;
    static bool initialized_;
};

// ═══════════════════════════════════════════════════════════════════════════
// PotionHelper — Potion brewing and damage calculation utilities.
// ═══════════════════════════════════════════════════════════════════════════

class PotionHelper {
public:
    // Calculate instant heal/harm amount
    // Java: Potion.performEffect — heal: 4 << amplifier, harm: 6 << amplifier
    static int32_t calcHealAmount(int32_t amplifier) {
        return std::max(4 << amplifier, 0);
    }
    static int32_t calcHarmAmount(int32_t amplifier) {
        return 6 << amplifier;
    }

    // Calculate splash potion heal/harm with distance falloff
    // Java: Potion.affectEntity — amount * distance_modifier + 0.5
    static int32_t calcSplashHeal(int32_t amplifier, double distanceMod) {
        return static_cast<int32_t>(distanceMod * static_cast<double>(4 << amplifier) + 0.5);
    }
    static int32_t calcSplashHarm(int32_t amplifier, double distanceMod) {
        return static_cast<int32_t>(distanceMod * static_cast<double>(6 << amplifier) + 0.5);
    }

    // Hunger effect exhaustion per tick
    // Java: Potion.performEffect hunger case: 0.025 * (amplifier + 1)
    static float calcHungerExhaustion(int32_t amplifier) {
        return 0.025f * static_cast<float>(amplifier + 1);
    }
};

} // namespace mccpp
