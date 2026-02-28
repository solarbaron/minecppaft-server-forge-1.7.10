/**
 * CombatTracker.h — Combat logging and death message generation.
 *
 * Java references:
 *   - net.minecraft.util.CombatTracker — Combat log manager
 *   - net.minecraft.util.CombatEntry — Individual damage event
 *
 * Tracks damage events to generate accurate death messages.
 * Determines strongest attacker with player priority (1/3 threshold).
 * Combat timeout: 100 ticks (no combat), 300 ticks (in combat).
 *
 * Thread safety: Per-entity, accessed from server thread only.
 *
 * JNI readiness: Simple struct layout.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// CombatEntry — One damage event in combat log.
// Java reference: net.minecraft.util.CombatEntry
// ═══════════════════════════════════════════════════════════════════════════

struct CombatEntry {
    std::string damageType;    // DamageSource type string
    int32_t tick = 0;          // Entity ticksExisted when hit
    float healthBeforeDmg = 0.0f;   // Health before damage
    float damageAmount = 0.0f;      // Actual damage dealt
    std::string locationContext;     // "ladder", "vines", "water", or ""
    float fallDistance = 0.0f;       // Fall distance at time of hit

    // Java: getDamageSrc().getEntity() — attacker entity ID
    int32_t attackerEntityId = -1;
    std::string attackerName;  // Attacker display name
    bool attackerIsPlayer = false;

    // Java: func_94559_f — did this entry involve a living attacker?
    bool hasLivingAttacker() const { return attackerEntityId >= 0; }

    // Java: func_94563_c — damage amount for comparison
    float getDamageAmount() const { return damageAmount; }

    // Java: func_94561_i — fall distance
    float getFallDistance() const { return fallDistance; }

    // Java: func_94562_g — location context
    const std::string& getLocationContext() const { return locationContext; }
};

// ═══════════════════════════════════════════════════════════════════════════
// CombatTracker — Manages combat log for one entity.
// Java reference: net.minecraft.util.CombatTracker
// ═══════════════════════════════════════════════════════════════════════════

class CombatTracker {
public:
    // Java: combatEntries
    std::vector<CombatEntry> entries;

    // Java: field_94555_c — tick of last damage
    int32_t lastDamageTick = 0;

    // Java: field_152775_d — combat start tick
    int32_t combatStartTick = 0;

    // Java: field_152776_e — combat end tick
    int32_t combatEndTick = 0;

    // Java: field_94552_d — in PvP combat
    bool inCombat = false;

    // Java: field_94553_e — taking damage recently
    bool takingDamage = false;

    // Java: field_94551_f — current location context
    std::string locationContext;

    // Java: func_94545_a — update location context based on entity state
    // Called with entity state: onLadder (block="ladder"/"vine"), inWater
    void updateLocationContext(bool onLadder, bool isOnVine, bool inWater) {
        locationContext.clear();
        if (onLadder) {
            locationContext = isOnVine ? "vines" : "ladder";
        } else if (inWater) {
            locationContext = "water";
        }
    }

    // Java: func_94547_a — record a combat hit
    void recordDamage(const std::string& damageType, int32_t entityTick,
                      float healthBefore, float damage, float fallDist,
                      int32_t attackerId = -1, const std::string& attackerName = "",
                      bool attackerIsPlayer = false) {
        // Reset timeout
        checkCombatEnd(entityTick, true);

        CombatEntry entry;
        entry.damageType = damageType;
        entry.tick = entityTick;
        entry.healthBeforeDmg = healthBefore;
        entry.damageAmount = damage;
        entry.locationContext = locationContext;
        entry.fallDistance = fallDist;
        entry.attackerEntityId = attackerId;
        entry.attackerName = attackerName;
        entry.attackerIsPlayer = attackerIsPlayer;

        entries.push_back(entry);
        lastDamageTick = entityTick;
        takingDamage = true;

        if (entry.hasLivingAttacker() && !inCombat) {
            inCombat = true;
            combatStartTick = entityTick;
            combatEndTick = entityTick;
        }
    }

    // Java: func_94549_h — check if combat has timed out
    void checkCombatEnd(int32_t entityTick, bool entityAlive) {
        int32_t timeout = inCombat ? 300 : 100;
        if (takingDamage && (!entityAlive || entityTick - lastDamageTick > timeout)) {
            bool wasInCombat = inCombat;
            takingDamage = false;
            inCombat = false;
            combatEndTick = entityTick;
            if (wasInCombat) {
                // Would trigger combat end event
            }
            entries.clear();
        }
    }

    // Java: func_94550_c — get strongest attacker (prioritize players)
    struct AttackerInfo {
        int32_t entityId = -1;
        std::string name;
        bool isPlayer = false;
        bool found = false;
    };

    AttackerInfo getStrongestAttacker() const {
        AttackerInfo bestLiving, bestPlayer;
        float bestLivingDmg = 0.0f, bestPlayerDmg = 0.0f;

        for (const auto& e : entries) {
            if (e.attackerEntityId < 0) continue;

            if (e.attackerIsPlayer && e.damageAmount > bestPlayerDmg) {
                bestPlayerDmg = e.damageAmount;
                bestPlayer.entityId = e.attackerEntityId;
                bestPlayer.name = e.attackerName;
                bestPlayer.isPlayer = true;
                bestPlayer.found = true;
            }

            if (e.damageAmount > bestLivingDmg) {
                bestLivingDmg = e.damageAmount;
                bestLiving.entityId = e.attackerEntityId;
                bestLiving.name = e.attackerName;
                bestLiving.isPlayer = e.attackerIsPlayer;
                bestLiving.found = true;
            }
        }

        // Java: player wins if their damage >= livingDamage / 3
        if (bestPlayer.found && bestPlayerDmg >= bestLivingDmg / 3.0f) {
            return bestPlayer;
        }
        return bestLiving;
    }

    // Java: func_151521_b — get death message translation key
    // Returns: {messageKey, args...} for ChatComponentTranslation
    struct DeathMessage {
        std::string key;               // e.g., "death.attack.player"
        std::string victimName;
        std::string attackerName;
        std::string weaponName;
        bool hasFallContext = false;
    };

    DeathMessage getDeathMessage(const std::string& victimName) const {
        DeathMessage msg;
        msg.victimName = victimName;

        if (entries.empty()) {
            msg.key = "death.attack.generic";
            return msg;
        }

        const auto& lastEntry = entries.back();

        // Check if fall death with a cause
        const CombatEntry* fallCause = findBestFallCause();

        if (fallCause != nullptr && lastEntry.damageType == "fall") {
            const std::string& ctx = fallCause->getLocationContext();
            std::string suffix = ctx.empty() ? "generic" : ctx;

            if (fallCause->damageType == "fall" || fallCause->damageType == "outOfWorld") {
                msg.key = "death.fell.accident." + suffix;
            } else if (!fallCause->attackerName.empty() &&
                       fallCause->attackerName != lastEntry.attackerName) {
                msg.key = "death.fell.assist";
                msg.attackerName = fallCause->attackerName;
            } else if (!lastEntry.attackerName.empty()) {
                msg.key = "death.fell.finish";
                msg.attackerName = lastEntry.attackerName;
            } else {
                msg.key = "death.fell.killer";
            }
            msg.hasFallContext = true;
        } else {
            // Standard death message from damage source
            msg.key = "death.attack." + lastEntry.damageType;
            msg.attackerName = lastEntry.attackerName;
        }

        return msg;
    }

    // Reset tracker
    void reset() {
        entries.clear();
        lastDamageTick = 0;
        combatStartTick = 0;
        combatEndTick = 0;
        inCombat = false;
        takingDamage = false;
        locationContext.clear();
    }

private:
    // Java: func_94544_f — find best fall cause entry
    const CombatEntry* findBestFallCause() const {
        const CombatEntry* bestFall = nullptr;
        float bestFallDist = 0.0f;

        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& e = entries[i];
            if ((e.damageType == "fall" || e.damageType == "outOfWorld") &&
                e.fallDistance > 0.0f && e.fallDistance > bestFallDist) {
                bestFall = (i > 0) ? &entries[i - 1] : &e;
                bestFallDist = e.fallDistance;
            }
        }

        if (bestFallDist > 5.0f && bestFall != nullptr) {
            return bestFall;
        }
        return nullptr;
    }
};

} // namespace mccpp
