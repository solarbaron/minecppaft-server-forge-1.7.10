#pragma once
// PotionEffect — all vanilla 1.7.10 potion effects and active effect tracking.
// Ported from rw.java (Potion) and rx.java (PotionEffect).
//
// Vanilla has 23 potion effect IDs (1-23). Each has:
//   - ID, name, instant vs duration, color, beneficial/harmful
//   - Tick behavior (e.g., regeneration heals every N ticks)
//
// Active effects on entities: ID + amplifier (0-based) + duration (ticks)
// Network: 0x1D EntityEffect, 0x1E RemoveEntityEffect

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace mc {

// Potion effect IDs — from rw.java (Potion)
namespace PotionID {
    constexpr int8_t SPEED           = 1;
    constexpr int8_t SLOWNESS        = 2;
    constexpr int8_t HASTE           = 3;
    constexpr int8_t MINING_FATIGUE  = 4;
    constexpr int8_t STRENGTH        = 5;
    constexpr int8_t INSTANT_HEALTH  = 6;
    constexpr int8_t INSTANT_DAMAGE  = 7;
    constexpr int8_t JUMP_BOOST      = 8;
    constexpr int8_t NAUSEA          = 9;
    constexpr int8_t REGENERATION    = 10;
    constexpr int8_t RESISTANCE      = 11;
    constexpr int8_t FIRE_RESISTANCE = 12;
    constexpr int8_t WATER_BREATHING = 13;
    constexpr int8_t INVISIBILITY    = 14;
    constexpr int8_t BLINDNESS       = 15;
    constexpr int8_t NIGHT_VISION    = 16;
    constexpr int8_t HUNGER          = 17;
    constexpr int8_t WEAKNESS        = 18;
    constexpr int8_t POISON          = 19;
    constexpr int8_t WITHER          = 20;
    constexpr int8_t HEALTH_BOOST    = 21;
    constexpr int8_t ABSORPTION      = 22;
    constexpr int8_t SATURATION      = 23;
}

// Potion type metadata
struct PotionType {
    int8_t id;
    std::string name;
    bool instant;       // Instant effect (health/damage) vs duration
    bool beneficial;    // Beneficial (blue particles) vs harmful (green)
    int color;          // RGB particle color

    // Tick interval for duration effects (0 = no periodic tick)
    // e.g., regeneration ticks every 50 ticks at amplifier 0
    int tickInterval(int amplifier) const {
        switch (id) {
            case PotionID::REGENERATION:
                return 50 >> amplifier; // 50, 25, 12, 6 ticks
            case PotionID::POISON:
                return 25 >> amplifier; // 25, 12, 6, 3 ticks
            case PotionID::WITHER:
                return 40 >> amplifier; // 40, 20, 10, 5 ticks
            case PotionID::HUNGER:
                return 40 >> amplifier;
            case PotionID::SATURATION:
                return 10 >> amplifier;
            default:
                return 0; // No periodic tick
        }
    }
};

// Active potion effect on an entity
struct ActivePotionEffect {
    int8_t effectId;
    int8_t amplifier;    // 0 = level I, 1 = level II, etc.
    int32_t duration;    // Remaining ticks (-1 = infinite)
    bool ambient;        // From beacon (reduced particles)
    bool showParticles;

    bool isExpired() const { return duration == 0; }

    // Tick this effect, returns true if it performed an action this tick
    bool tick() {
        if (duration > 0) --duration;
        return false;
    }
};

// Manages active effects on a single entity
class PotionEffectManager {
public:
    // Add or replace an effect
    void addEffect(const ActivePotionEffect& effect) {
        // If already has this effect, replace if new is stronger/longer
        auto it = effects_.find(effect.effectId);
        if (it != effects_.end()) {
            if (effect.amplifier > it->second.amplifier ||
                (effect.amplifier == it->second.amplifier &&
                 effect.duration > it->second.duration)) {
                it->second = effect;
            }
        } else {
            effects_[effect.effectId] = effect;
        }
    }

    // Remove an effect by ID
    void removeEffect(int8_t effectId) {
        effects_.erase(effectId);
    }

    // Check if entity has an effect
    bool hasEffect(int8_t effectId) const {
        return effects_.count(effectId) > 0;
    }

    // Get effect amplifier (returns -1 if not present)
    int getAmplifier(int8_t effectId) const {
        auto it = effects_.find(effectId);
        return it != effects_.end() ? it->second.amplifier : -1;
    }

    // Tick all effects, returns list of expired effect IDs
    std::vector<int8_t> tickAll() {
        std::vector<int8_t> expired;

        for (auto it = effects_.begin(); it != effects_.end(); ) {
            it->second.tick();
            if (it->second.isExpired()) {
                expired.push_back(it->first);
                it = effects_.erase(it);
            } else {
                ++it;
            }
        }

        return expired;
    }

    // Get all active effects for packet sending
    const std::unordered_map<int8_t, ActivePotionEffect>& effects() const {
        return effects_;
    }

    // Check if regeneration should tick this game tick
    bool shouldRegenTick() const {
        auto it = effects_.find(PotionID::REGENERATION);
        if (it == effects_.end()) return false;
        int interval = PotionType{PotionID::REGENERATION, "", false, true, 0}
            .tickInterval(it->second.amplifier);
        if (interval <= 0) return false;
        return (it->second.duration % interval) == 0;
    }

    // Check if poison should tick this game tick
    bool shouldPoisonTick() const {
        auto it = effects_.find(PotionID::POISON);
        if (it == effects_.end()) return false;
        int interval = PotionType{PotionID::POISON, "", false, false, 0}
            .tickInterval(it->second.amplifier);
        if (interval <= 0) return false;
        return (it->second.duration % interval) == 0;
    }

    // Check if wither should tick this game tick
    bool shouldWitherTick() const {
        auto it = effects_.find(PotionID::WITHER);
        if (it == effects_.end()) return false;
        int interval = PotionType{PotionID::WITHER, "", false, false, 0}
            .tickInterval(it->second.amplifier);
        if (interval <= 0) return false;
        return (it->second.duration % interval) == 0;
    }

    // Get speed modifier (0.0 = no effect)
    float getSpeedModifier() const {
        auto it = effects_.find(PotionID::SPEED);
        if (it != effects_.end())
            return 0.2f * (it->second.amplifier + 1); // +20% per level
        it = effects_.find(PotionID::SLOWNESS);
        if (it != effects_.end())
            return -0.15f * (it->second.amplifier + 1); // -15% per level
        return 0.0f;
    }

    // Get damage modifier from strength/weakness
    float getDamageModifier() const {
        float mod = 0.0f;
        auto it = effects_.find(PotionID::STRENGTH);
        if (it != effects_.end())
            mod += 1.3f * (it->second.amplifier + 1); // +130% per level
        it = effects_.find(PotionID::WEAKNESS);
        if (it != effects_.end())
            mod -= 0.5f * (it->second.amplifier + 1); // -50% per level
        return mod;
    }

    void clear() { effects_.clear(); }

private:
    std::unordered_map<int8_t, ActivePotionEffect> effects_;
};

// Registry of all potion types
class PotionRegistry {
public:
    PotionRegistry() { registerAll(); }

    const PotionType* getById(int8_t id) const {
        auto it = byId_.find(id);
        return it != byId_.end() ? &it->second : nullptr;
    }

    static PotionRegistry& instance() {
        static PotionRegistry reg;
        return reg;
    }

private:
    std::unordered_map<int8_t, PotionType> byId_;

    void add(int8_t id, const std::string& name, bool instant, bool beneficial, int color) {
        byId_[id] = PotionType{id, name, instant, beneficial, color};
    }

    void registerAll() {
        add(1,  "Speed",            false, true,  0x7CAFC6);
        add(2,  "Slowness",         false, false, 0x5A6C81);
        add(3,  "Haste",            false, true,  0xD9C043);
        add(4,  "Mining Fatigue",   false, false, 0x4A4217);
        add(5,  "Strength",         false, true,  0x932423);
        add(6,  "Instant Health",   true,  true,  0xF82423);
        add(7,  "Instant Damage",   true,  false, 0x430A09);
        add(8,  "Jump Boost",       false, true,  0x786297);
        add(9,  "Nausea",           false, false, 0x551D4A);
        add(10, "Regeneration",     false, true,  0xCD5CAB);
        add(11, "Resistance",       false, true,  0x99453A);
        add(12, "Fire Resistance",  false, true,  0xE49A3A);
        add(13, "Water Breathing",  false, true,  0x2E5299);
        add(14, "Invisibility",     false, true,  0x7F8392);
        add(15, "Blindness",        false, false, 0x1F1F23);
        add(16, "Night Vision",     false, true,  0x1F1FA1);
        add(17, "Hunger",           false, false, 0x587653);
        add(18, "Weakness",         false, false, 0x484D48);
        add(19, "Poison",           false, false, 0x4E9331);
        add(20, "Wither",           false, false, 0x352A27);
        add(21, "Health Boost",     false, true,  0xF87D23);
        add(22, "Absorption",       false, true,  0x2552A5);
        add(23, "Saturation",       true,  true,  0xF82423);
    }
};

} // namespace mc
