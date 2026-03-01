/**
 * PotionSystem.h — Complete potion registry, effects, and brewing mechanics.
 *
 * Java references:
 *   - net.minecraft.potion.Potion (204 lines)
 *   - net.minecraft.potion.PotionEffect (135 lines)
 *   - net.minecraft.potion.PotionHelper (402 lines)
 *
 * Thread safety: Potion registry is read-only after init. PotionEffects
 * are per-entity and tick on single entity thread.
 */
#pragma once

#include <cstdint>
#include <cmath>
#include <array>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Potion — Static potion type registry.
// Java: net.minecraft.potion.Potion (204 lines)
//
//   32 potion slots (potionTypes[32]), 24 actual effects (IDs 1-23)
//   Each has: id, isBadEffect, liquidColor, name, effectiveness,
//             statusIconIndex, attribute modifiers
//
//   performEffect: per-tick effects for regen/poison/wither/hunger/saturation
//     regen: heal(1) if health < maxHealth
//     poison: attack(magic, 1) if health > 1
//     wither: attack(wither, 1) regardless
//     hunger: addExhaustion(0.025*(amp+1))
//     saturation: addStats(amp+1, 1.0)
//   heal/harm:
//     heal(!undead) or harm(undead): heal(max(4<<amp, 0))
//     harm(!undead) or heal(undead): attack(magic, 6<<amp)
//
//   isReady (tick intervals):
//     regen: 50 >> amplifier
//     poison: 25 >> amplifier
//     wither: 40 >> amplifier
//     hunger: every tick
//
//   affectEntity (instant, distance-scaled):
//     heal: (int)(d * (4<<amp) + 0.5)
//     harm: (int)(d * (6<<amp) + 0.5)
// ═══════════════════════════════════════════════════════════════════════════

struct PotionType {
    int32_t id;
    bool isBadEffect;
    int32_t liquidColor;
    const char* name;
    double effectiveness;
    int32_t iconIndex; // col + row*8
    bool isInstant;
    // Attribute modifier info
    int32_t attrModOp; // -1 = none, 0/1/2 = operation
    double attrModAmount;
};

class PotionRegistry {
public:
    // ─── Potion IDs ───
    static constexpr int32_t SPEED = 1;
    static constexpr int32_t SLOWNESS = 2;
    static constexpr int32_t HASTE = 3;
    static constexpr int32_t MINING_FATIGUE = 4;
    static constexpr int32_t STRENGTH = 5;
    static constexpr int32_t INSTANT_HEALTH = 6;
    static constexpr int32_t INSTANT_DAMAGE = 7;
    static constexpr int32_t JUMP_BOOST = 8;
    static constexpr int32_t NAUSEA = 9;
    static constexpr int32_t REGENERATION = 10;
    static constexpr int32_t RESISTANCE = 11;
    static constexpr int32_t FIRE_RESISTANCE = 12;
    static constexpr int32_t WATER_BREATHING = 13;
    static constexpr int32_t INVISIBILITY = 14;
    static constexpr int32_t BLINDNESS = 15;
    static constexpr int32_t NIGHT_VISION = 16;
    static constexpr int32_t HUNGER = 17;
    static constexpr int32_t WEAKNESS = 18;
    static constexpr int32_t POISON = 19;
    static constexpr int32_t WITHER = 20;
    static constexpr int32_t HEALTH_BOOST = 21;
    static constexpr int32_t ABSORPTION = 22;
    static constexpr int32_t SATURATION = 23;

    static constexpr int32_t NUM_POTIONS = 32;
    static constexpr int32_t NUM_ACTUAL = 24; // 1..23

    // ─── Full registry ───
    static constexpr std::array<PotionType, NUM_ACTUAL> TYPES = {{
        {0,  false, 0,        "",                    1.0,  -1, false, -1, 0.0},       // placeholder 0
        {1,  false, 8171462,  "potion.moveSpeed",    1.0,   0, false,  2, 0.2},       // speed
        {2,  true,  5926017,  "potion.moveSlowdown", 0.5,   1, false,  2, -0.15},     // slowness
        {3,  false, 14270531, "potion.digSpeed",     1.5,   2, false, -1, 0.0},       // haste
        {4,  true,  4866583,  "potion.digSlowDown",  0.5,   3, false, -1, 0.0},       // mining fatigue
        {5,  false, 9643043,  "potion.damageBoost",  1.0,   4, false,  2, 3.0},       // strength
        {6,  false, 16262179, "potion.heal",         1.0,  -1, true,  -1, 0.0},       // instant health
        {7,  true,  4393481,  "potion.harm",         0.5,  -1, true,  -1, 0.0},       // instant damage
        {8,  false, 7889559,  "potion.jump",         1.0,  10, false, -1, 0.0},       // jump boost
        {9,  true,  5578058,  "potion.confusion",    0.25, 11, false, -1, 0.0},       // nausea
        {10, false, 13458603, "potion.regeneration",  0.25,  7, false, -1, 0.0},      // regen
        {11, false, 10044730, "potion.resistance",    1.0,  14, false, -1, 0.0},      // resistance
        {12, false, 14981690, "potion.fireResistance", 1.0, 15, false, -1, 0.0},     // fire resist
        {13, false, 3035801,  "potion.waterBreathing", 1.0, 16, false, -1, 0.0},     // water breathing
        {14, false, 8356754,  "potion.invisibility",   1.0,  8, false, -1, 0.0},     // invisibility
        {15, true,  2039587,  "potion.blindness",      0.25, 13, false, -1, 0.0},    // blindness
        {16, false, 0x1F1FA1, "potion.nightVision",    1.0, 12, false, -1, 0.0},     // night vision
        {17, true,  5797459,  "potion.hunger",         0.5,   9, false, -1, 0.0},    // hunger
        {18, true,  0x484D48, "potion.weakness",       0.5,   5, false,  0, 2.0},    // weakness
        {19, true,  5149489,  "potion.poison",         0.25,  6, false, -1, 0.0},    // poison
        {20, true,  3484199,  "potion.wither",         0.25,  9, false, -1, 0.0},    // wither
        {21, false, 16284963, "potion.healthBoost",    1.0,  18, false,  0, 4.0},    // health boost
        {22, false, 0x2552A5, "potion.absorption",     1.0,  18, false, -1, 0.0},    // absorption
        {23, false, 16262179, "potion.saturation",     1.0,  -1, true,  -1, 0.0},    // saturation
    }};

    static const PotionType& get(int32_t id) {
        if (id >= 1 && id < NUM_ACTUAL) return TYPES[id];
        return TYPES[0];
    }

    // ─── isReady — tick interval for periodic effects ───
    // Java: regen: 50>>amp, poison: 25>>amp, wither: 40>>amp, hunger: every tick
    static bool isReady(int32_t potionId, int32_t duration, int32_t amplifier) {
        int32_t interval;
        if (potionId == REGENERATION) {
            interval = 50 >> amplifier;
        } else if (potionId == POISON) {
            interval = 25 >> amplifier;
        } else if (potionId == WITHER) {
            interval = 40 >> amplifier;
        } else if (potionId == HUNGER) {
            return true;
        } else {
            return false;
        }
        return interval > 0 ? (duration % interval == 0) : true;
    }

    // ─── performEffect damage/heal amounts ───
    static constexpr float REGEN_HEAL = 1.0f;
    static constexpr float POISON_DAMAGE = 1.0f;
    static constexpr float WITHER_DAMAGE = 1.0f;
    static constexpr float HUNGER_EXHAUSTION_BASE = 0.025f;
    static constexpr int32_t HEAL_BASE = 4;   // 4 << amplifier
    static constexpr int32_t HARM_BASE = 6;   // 6 << amplifier

    static int32_t getHealAmount(int32_t amplifier) { return HEAL_BASE << amplifier; }
    static int32_t getHarmDamage(int32_t amplifier) { return HARM_BASE << amplifier; }

    // ─── affectEntity — instant potion with distance multiplier ───
    static int32_t instantHealAmount(int32_t amplifier, double multiplier) {
        return static_cast<int32_t>(multiplier * (HEAL_BASE << amplifier) + 0.5);
    }
    static int32_t instantHarmDamage(int32_t amplifier, double multiplier) {
        return static_cast<int32_t>(multiplier * (HARM_BASE << amplifier) + 0.5);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// PotionEffect — Active potion effect instance on an entity.
// Java: net.minecraft.potion.PotionEffect (~135 lines)
//
//   potionID, duration (ticks), amplifier, isSplashPotion, isAmbient
//   Tick-down: --duration each tick
//   Combine: takes longer duration, higher amplifier
//   NBT: "Id" byte, "Amplifier" byte, "Duration" int, "Ambient" boolean
// ═══════════════════════════════════════════════════════════════════════════

struct PotionEffect {
    int32_t potionID = 0;
    int32_t duration = 0;
    int32_t amplifier = 0;
    bool isSplashPotion = false;
    bool isAmbient = false;

    bool tick() { return --duration > 0; }
    bool isExpired() const { return duration <= 0; }

    void combine(const PotionEffect& other) {
        if (other.amplifier > amplifier) {
            amplifier = other.amplifier;
            duration = other.duration;
        } else if (other.amplifier == amplifier && other.duration > duration) {
            duration = other.duration;
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// PotionHelper — Bit-flag brewing recipe system.
// Java: net.minecraft.potion.PotionHelper (402 lines)
//
//   ─── Ingredient effects (bit manipulation strings) ───
//   sugar:        "-0+1-2-3&4-4+13"  → speed
//   ghastTear:    "+0-1-2-3&4-4+13"  → regen
//   spiderEye:    "-0-1+2-3&4-4+13"  → poison
//   fermented:    "-0+3-4+13"        → corruption
//   speckledMelon:"+0-1+2-3&4-4+13"  → heal
//   blazePowder:  "+0-1-2+3&4-4+13"  → strength
//   magmaCream:   "+0+1-2-3&4-4+13"  → fire resist
//   goldenCarrot: "-0+1+2-3+13&4-4"  → night vision
//   pufferfish:   "+0-1+2+3+13&4-4"  → water breathing
//   glowstone:    "+5-6-7"           → amplifier up
//   redstone:     "-5+6-7"           → duration up
//   gunpowder:    "+14&13-13"        → splash
//
//   ─── Duration calculation ───
//   Base: 1200 * (score*3 + (score-1)*2) ticks
//   Shifted right by amplifier
//   Multiplied by effectiveness
//   Splash: *= 0.75 + 0.5 (round)
//
//   ─── Color calculation ───
//   Weighted RGB average across all effects * (amplifier+1)
//   Default: 3694022 (water blue)
// ═══════════════════════════════════════════════════════════════════════════

class PotionHelper {
public:
    // ─── Ingredient effect strings ───
    static constexpr const char* SUGAR_EFFECT = "-0+1-2-3&4-4+13";
    static constexpr const char* GHAST_TEAR_EFFECT = "+0-1-2-3&4-4+13";
    static constexpr const char* SPIDER_EYE_EFFECT = "-0-1+2-3&4-4+13";
    static constexpr const char* FERMENTED_SPIDER_EYE_EFFECT = "-0+3-4+13";
    static constexpr const char* SPECKLED_MELON_EFFECT = "+0-1+2-3&4-4+13";
    static constexpr const char* BLAZE_POWDER_EFFECT = "+0-1-2+3&4-4+13";
    static constexpr const char* MAGMA_CREAM_EFFECT = "+0+1-2-3&4-4+13";
    static constexpr const char* GOLDEN_CARROT_EFFECT = "-0+1+2-3+13&4-4";
    static constexpr const char* PUFFERFISH_EFFECT = "+0-1+2+3+13&4-4";
    static constexpr const char* GLOWSTONE_EFFECT = "+5-6-7";
    static constexpr const char* REDSTONE_EFFECT = "-5+6-7";
    static constexpr const char* GUNPOWDER_EFFECT = "+14&13-13";

    static constexpr int32_t DEFAULT_POTION_COLOR = 3694022;
    static constexpr int32_t SPLASH_FLAG = 0x4000;
    static constexpr int32_t MASK = 0x7FFF; // Short.MAX_VALUE

    // ─── Potion requirements ───
    // Maps potion ID → requirement string
    struct PotionReq {
        int32_t potionId;
        const char* requirement;
    };
    static constexpr int32_t NUM_REQS = 14;
    static constexpr std::array<PotionReq, NUM_REQS> REQUIREMENTS = {{
        {10, "0 & !1 & !2 & !3 & 0+6"},    // regeneration
        {1,  "!0 & 1 & !2 & !3 & 1+6"},     // speed
        {12, "0 & 1 & !2 & !3 & 0+6"},      // fire resistance
        {6,  "0 & !1 & 2 & !3"},             // heal
        {19, "!0 & !1 & 2 & !3 & 2+6"},     // poison
        {18, "!0 & !1 & !2 & 3 & 3+6"},     // weakness
        {7,  "!0 & !1 & 2 & 3"},             // harm
        {2,  "!0 & 1 & !2 & 3 & 3+6"},      // slowness
        {5,  "0 & !1 & !2 & 3 & 3+6"},      // strength
        {16, "!0 & 1 & 2 & !3 & 2+6"},      // night vision
        {14, "!0 & 1 & 2 & 3 & 2+6"},       // invisibility
        {13, "0 & !1 & 2 & 3 & 2+6"},       // water breathing
        {11, "0 & 1 & !2 & !3 & 0+6"},      // resistance (same as fire resist)
        {8,  "!0 & 1 & !2 & !3 & 1+6"},     // jump boost (same as speed)
    }};

    // ─── Potion amplifiers ───
    // All amplified potions use bit 5
    struct PotionAmp {
        int32_t potionId;
        const char* amplifier;
    };
    static constexpr int32_t NUM_AMPS = 8;
    static constexpr std::array<PotionAmp, NUM_AMPS> AMPLIFIERS = {{
        {1,  "5"},  // speed
        {3,  "5"},  // haste
        {5,  "5"},  // strength
        {10, "5"},  // regen
        {7,  "5"},  // harm
        {6,  "5"},  // heal
        {11, "5"},  // resistance
        {19, "5"},  // poison
    }};

    // ─── Bit operations ───
    static bool checkFlag(int32_t value, int32_t bit) {
        return (value & (1 << bit)) != 0;
    }

    static int32_t countSetFlags(int32_t n) {
        int32_t count = 0;
        while (n > 0) {
            n &= n - 1;
            ++count;
        }
        return count;
    }

    // ─── applyIngredient — core bit manipulation ───
    static int32_t brewBitOp(int32_t n, int32_t bit, bool clear, bool toggle, bool require) {
        if (require) {
            return checkFlag(n, bit) ? n : 0;
        }
        if (clear) {
            n &= ~(1 << bit);
        } else if (toggle) {
            n ^= (1 << bit);
        } else {
            n |= (1 << bit);
        }
        return n;
    }

    // ─── Duration calculation ───
    // Java: 1200 * (score*3 + (score-1)*2) >> amplifier * effectiveness
    // Splash: *= 0.75 then round
    static int32_t calculateDuration(int32_t score, int32_t amplifier, double effectiveness, bool isSplash) {
        int32_t duration = 1200 * (score * 3 + (score - 1) * 2);
        duration >>= amplifier;
        duration = static_cast<int32_t>(std::round(duration * effectiveness));
        if (isSplash) {
            duration = static_cast<int32_t>(std::round(duration * 0.75 + 0.5));
        }
        return duration;
    }

    // ─── Potion color ───
    // Weighted RGB average. Each effect contributes (amplifier+1) samples.
    static int32_t calcColor(const PotionEffect* effects, int32_t count) {
        if (count == 0) return DEFAULT_POTION_COLOR;
        float r = 0, g = 0, b = 0, total = 0;
        for (int32_t i = 0; i < count; ++i) {
            int32_t color = PotionRegistry::get(effects[i].potionID).liquidColor;
            for (int32_t a = 0; a <= effects[i].amplifier; ++a) {
                r += ((color >> 16) & 0xFF) / 255.0f;
                g += ((color >> 8) & 0xFF) / 255.0f;
                b += (color & 0xFF) / 255.0f;
                total += 1.0f;
            }
        }
        r = r / total * 255.0f;
        g = g / total * 255.0f;
        b = b / total * 255.0f;
        return (static_cast<int32_t>(r) << 16) | (static_cast<int32_t>(g) << 8) | static_cast<int32_t>(b);
    }

    // ─── 32 potion name prefixes ───
    static constexpr int32_t NUM_PREFIXES = 32;
    static constexpr const char* PREFIXES[NUM_PREFIXES] = {
        "potion.prefix.mundane",      "potion.prefix.uninteresting",
        "potion.prefix.bland",        "potion.prefix.clear",
        "potion.prefix.milky",        "potion.prefix.diffuse",
        "potion.prefix.artless",      "potion.prefix.thin",
        "potion.prefix.awkward",      "potion.prefix.flat",
        "potion.prefix.bulky",        "potion.prefix.bungling",
        "potion.prefix.buttered",     "potion.prefix.smooth",
        "potion.prefix.suave",        "potion.prefix.debonair",
        "potion.prefix.thick",        "potion.prefix.elegant",
        "potion.prefix.fancy",        "potion.prefix.charming",
        "potion.prefix.dashing",      "potion.prefix.refined",
        "potion.prefix.cordial",      "potion.prefix.sparkling",
        "potion.prefix.potent",       "potion.prefix.foul",
        "potion.prefix.odorless",     "potion.prefix.rank",
        "potion.prefix.harsh",        "potion.prefix.acrid",
        "potion.prefix.gross",        "potion.prefix.stinky",
    };

    // ─── Prefix lookup ───
    // Java: func_77908_a extracts bits 5,4,3,2,1 into 5-bit index
    static int32_t getPrefixIndex(int32_t potionMeta) {
        return (checkFlag(potionMeta, 5) ? 16 : 0)
             | (checkFlag(potionMeta, 4) ? 8 : 0)
             | (checkFlag(potionMeta, 3) ? 4 : 0)
             | (checkFlag(potionMeta, 2) ? 2 : 0)
             | (checkFlag(potionMeta, 1) ? 1 : 0);
    }
};

} // namespace mccpp
