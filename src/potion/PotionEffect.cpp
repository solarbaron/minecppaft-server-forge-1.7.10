/**
 * PotionEffect.cpp — Potion system implementation.
 *
 * Java references:
 *   net.minecraft.potion.Potion — static initializer, performEffect, isReady
 *   net.minecraft.potion.PotionEffect — duration tick, combine
 *
 * Key behaviors preserved from Java:
 *   - Regeneration: heals every 50 >> amplifier ticks
 *   - Poison: damages every 25 >> amplifier ticks (won't kill, stops at 1HP)
 *   - Wither: damages every 40 >> amplifier ticks (CAN kill)
 *   - Hunger: adds exhaustion every tick
 *   - Speed: +20% per level (operation 2 = multiply total)
 *   - Slowness: -15% per level
 *   - Strength: +3 per level (operation 2)
 *   - Weakness: +2 per level added (operation 0, but negative effect)
 *   - Health Boost: +4 max HP per level (operation 0)
 */

#include "potion/PotionEffect.h"
#include <algorithm>
#include <iostream>

namespace mccpp {

// ═════════════════════════════════════════════════════════════════════════════
// Potion::isReady — Determine if effect should tick at this duration
// ═════════════════════════════════════════════════════════════════════════════

bool Potion::isReady(int32_t duration, int32_t amplifier) const {
    // Java: Potion.isReady — exact tick intervals per effect type
    if (id == PotionRegistry::REGENERATION) {
        // Regen: 50 >> amplifier ticks between heals
        // Level I: every 50 ticks (2.5s), II: 25 (1.25s), III: 12, IV: 6
        int32_t interval = 50 >> amplifier;
        return interval > 0 ? (duration % interval == 0) : true;
    }
    if (id == PotionRegistry::POISON) {
        // Poison: 25 >> amplifier ticks between damage
        // Level I: every 25 ticks (1.25s), II: 12, III: 6
        int32_t interval = 25 >> amplifier;
        return interval > 0 ? (duration % interval == 0) : true;
    }
    if (id == PotionRegistry::WITHER) {
        // Wither: 40 >> amplifier ticks between damage
        // Level I: every 40 ticks (2s), II: 20, III: 10
        int32_t interval = 40 >> amplifier;
        return interval > 0 ? (duration % interval == 0) : true;
    }
    if (id == PotionRegistry::HUNGER) {
        // Hunger: every tick
        return true;
    }
    return false;
}

// ═════════════════════════════════════════════════════════════════════════════
// PotionEffect
// ═════════════════════════════════════════════════════════════════════════════

bool PotionEffect::onUpdate() {
    // Java: PotionEffect.onUpdate
    if (duration > 0) {
        const Potion* potion = PotionRegistry::getById(potionId);
        if (potion && !potion->isInstant) {
            if (potion->isReady(duration, amplifier)) {
                // Effect triggers (actual effect application is done by entity)
            }
            --duration;
        }
    }
    return duration > 0;
}

void PotionEffect::combine(const PotionEffect& other) {
    // Java: PotionEffect.combine
    if (other.potionId != potionId) return;

    if (other.amplifier > amplifier) {
        amplifier = other.amplifier;
        duration = other.duration;
    } else if (other.amplifier == amplifier && other.duration > duration) {
        duration = other.duration;
    }

    ambient = other.ambient;
}

// ═════════════════════════════════════════════════════════════════════════════
// PotionRegistry
// ═════════════════════════════════════════════════════════════════════════════

std::vector<Potion> PotionRegistry::potions_;
bool PotionRegistry::initialized_ = false;

void PotionRegistry::init() {
    if (initialized_) return;

    potions_ = {
        // Java: Potion.moveSpeed — ID 1, good, color 8171462, +20% speed (op 2)
        {1, "potion.moveSpeed", false, 8171462, 1.0, false,
         {{"generic.movementSpeed", {"91AEAA56-376B-4498-935B-2F7F68070635",
           "potion.moveSpeed", 0.2, 2}}}},

        // Java: Potion.moveSlowdown — ID 2, bad, color 5926017, -15% speed (op 2)
        {2, "potion.moveSlowdown", true, 5926017, 0.5, false,
         {{"generic.movementSpeed", {"7107DE5E-7CE8-4030-940E-514C1F160890",
           "potion.moveSlowdown", -0.15, 2}}}},

        // Java: Potion.digSpeed — ID 3, good, color 14270531, effectiveness 1.5
        {3, "potion.digSpeed", false, 14270531, 1.5, false, {}},

        // Java: Potion.digSlowdown — ID 4, bad, color 4866583
        {4, "potion.digSlowDown", true, 4866583, 0.5, false, {}},

        // Java: Potion.damageBoost — ID 5, good, color 9643043, +3 attack (op 2)
        {5, "potion.damageBoost", false, 9643043, 1.0, false,
         {{"generic.attackDamage", {"648D7064-6A60-4F59-8ABE-C2C23A6DD7A9",
           "potion.damageBoost", 3.0, 2}}}},

        // Java: Potion.heal — ID 6, good, color 16262179, INSTANT
        {6, "potion.heal", false, 16262179, 1.0, true, {}},

        // Java: Potion.harm — ID 7, bad, color 4393481, INSTANT
        {7, "potion.harm", true, 4393481, 0.5, true, {}},

        // Java: Potion.jump — ID 8, good, color 7889559
        {8, "potion.jump", false, 7889559, 1.0, false, {}},

        // Java: Potion.confusion — ID 9, bad, color 5578058, effectiveness 0.25
        {9, "potion.confusion", true, 5578058, 0.25, false, {}},

        // Java: Potion.regeneration — ID 10, good, color 13458603, effectiveness 0.25
        {10, "potion.regeneration", false, 13458603, 0.25, false, {}},

        // Java: Potion.resistance — ID 11, good, color 10044730
        {11, "potion.resistance", false, 10044730, 1.0, false, {}},

        // Java: Potion.fireResistance — ID 12, good, color 14981690
        {12, "potion.fireResistance", false, 14981690, 1.0, false, {}},

        // Java: Potion.waterBreathing — ID 13, good, color 3035801
        {13, "potion.waterBreathing", false, 3035801, 1.0, false, {}},

        // Java: Potion.invisibility — ID 14, good, color 8356754
        {14, "potion.invisibility", false, 8356754, 1.0, false, {}},

        // Java: Potion.blindness — ID 15, bad, color 2039587, effectiveness 0.25
        {15, "potion.blindness", true, 2039587, 0.25, false, {}},

        // Java: Potion.nightVision — ID 16, good, color 2039713
        {16, "potion.nightVision", false, 0x1F1FA1, 1.0, false, {}},

        // Java: Potion.hunger — ID 17, bad, color 5797459
        {17, "potion.hunger", true, 5797459, 0.5, false, {}},

        // Java: Potion.weakness — ID 18, bad, color 4738376, +2 attack mod (op 0)
        // Note: despite positive amount, weakness reduces damage (Java quirk)
        {18, "potion.weakness", true, 0x484D48, 0.5, false,
         {{"generic.attackDamage", {"22653B89-116E-49DC-9B6B-9971489B5BE5",
           "potion.weakness", 2.0, 0}}}},

        // Java: Potion.poison — ID 19, bad, color 5149489, effectiveness 0.25
        {19, "potion.poison", true, 5149489, 0.25, false, {}},

        // Java: Potion.wither — ID 20, bad, color 3484199, effectiveness 0.25
        {20, "potion.wither", true, 3484199, 0.25, false, {}},

        // Java: Potion.healthBoost — ID 21, good, color 16284963, +4 maxHP (op 0)
        {21, "potion.healthBoost", false, 16284963, 1.0, false,
         {{"generic.maxHealth", {"5D6F0BA2-1186-46AC-B896-C61C5CEE99CC",
           "potion.healthBoost", 4.0, 0}}}},

        // Java: Potion.absorption — ID 22, good, color 2445477
        {22, "potion.absorption", false, 0x2552A5, 1.0, false, {}},

        // Java: Potion.saturation — ID 23, good, color 16262179, INSTANT
        {23, "potion.saturation", false, 16262179, 1.0, true, {}},
    };

    initialized_ = true;
    std::cout << "[Potion] Registered " << potions_.size() << " potion effects\n";
}

const Potion* PotionRegistry::getById(int32_t id) {
    for (const auto& pot : potions_) {
        if (pot.id == id) return &pot;
    }
    return nullptr;
}

const std::vector<Potion>& PotionRegistry::getAll() {
    return potions_;
}

int32_t PotionRegistry::getCount() {
    return static_cast<int32_t>(potions_.size());
}

} // namespace mccpp
