/**
 * GameRules.h — World game rules system.
 *
 * Java reference: net.minecraft.world.GameRules
 *
 * All 9 vanilla 1.7.10 game rules with string-based storage,
 * boolean/int getters, and NBT serialization.
 *
 * Thread safety:
 *   - GameRules is per-world, accessed from server thread.
 *   - Uses std::shared_mutex for concurrent read support.
 *
 * JNI readiness: String-based key/value pairs for easy JVM mapping.
 */
#pragma once

#include <cstdint>
#include <map>
#include <shared_mutex>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// GameRuleValue — Internal storage for a single game rule.
// Java reference: net.minecraft.world.GameRules$Value
// ═══════════════════════════════════════════════════════════════════════════

struct GameRuleValue {
    std::string stringValue;

    GameRuleValue() = default;
    explicit GameRuleValue(const std::string& val) : stringValue(val) {}

    // Java: getGameRuleBooleanValue
    bool getBooleanValue() const {
        return stringValue == "true";
    }

    // Java: getGameRuleIntValue (used in some mods, not base game)
    int32_t getIntValue() const {
        try { return std::stoi(stringValue); }
        catch (...) { return 0; }
    }

    void setValue(const std::string& val) {
        stringValue = val;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GameRules — Per-world game rule storage.
// Java reference: net.minecraft.world.GameRules
//
// Default rules (1.7.10):
//   doFireTick:          true  — Fire spreads and naturally extinguishes
//   mobGriefing:         true  — Mobs can modify blocks (creeper, enderman, etc.)
//   keepInventory:       false — Players keep items on death
//   doMobSpawning:       true  — Mobs spawn naturally
//   doMobLoot:           true  — Mobs drop items
//   doTileDrops:         true  — Blocks drop items when broken
//   commandBlockOutput:  true  — Command blocks output to chat
//   naturalRegeneration: true  — Players regenerate health naturally
//   doDaylightCycle:     true  — Time progresses
// ═══════════════════════════════════════════════════════════════════════════

class GameRules {
public:
    GameRules();

    // Java: addGameRule
    void addGameRule(const std::string& name, const std::string& value);

    // Java: setOrCreateGameRule
    void setOrCreateGameRule(const std::string& name, const std::string& value);

    // Java: getGameRuleStringValue
    std::string getGameRuleStringValue(const std::string& name) const;

    // Java: getGameRuleBooleanValue
    bool getGameRuleBooleanValue(const std::string& name) const;

    // Java: hasRule
    bool hasRule(const std::string& name) const;

    // Java: getRules — returns all rule names
    std::vector<std::string> getRules() const;

    // Java: writeGameRulesToNBT — returns map of name→value for NBT writing
    std::map<std::string, std::string> writeToMap() const;

    // Java: readGameRulesFromNBT — reads from map of name→value
    void readFromMap(const std::map<std::string, std::string>& data);

    // Rule name constants
    static constexpr const char* DO_FIRE_TICK          = "doFireTick";
    static constexpr const char* MOB_GRIEFING          = "mobGriefing";
    static constexpr const char* KEEP_INVENTORY        = "keepInventory";
    static constexpr const char* DO_MOB_SPAWNING       = "doMobSpawning";
    static constexpr const char* DO_MOB_LOOT           = "doMobLoot";
    static constexpr const char* DO_TILE_DROPS         = "doTileDrops";
    static constexpr const char* COMMAND_BLOCK_OUTPUT  = "commandBlockOutput";
    static constexpr const char* NATURAL_REGENERATION  = "naturalRegeneration";
    static constexpr const char* DO_DAYLIGHT_CYCLE     = "doDaylightCycle";

private:
    // Java uses TreeMap (sorted) — we use std::map for same sorted order
    std::map<std::string, GameRuleValue> rules_;
    mutable std::shared_mutex mutex_;  // Thread safety: L21 game rules mutex
};

} // namespace mccpp
