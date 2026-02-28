/**
 * GameRules.cpp — Game rules system implementation.
 *
 * Java reference: net.minecraft.world.GameRules
 *
 * 9 default vanilla game rules initialized in constructor.
 * Thread-safe with shared_mutex for concurrent read support.
 */

#include "world/GameRules.h"
#include <iostream>
#include <mutex>

namespace mccpp {

GameRules::GameRules() {
    // Java: GameRules constructor — 9 default rules
    rules_[DO_FIRE_TICK]          = GameRuleValue("true");
    rules_[MOB_GRIEFING]          = GameRuleValue("true");
    rules_[KEEP_INVENTORY]        = GameRuleValue("false");
    rules_[DO_MOB_SPAWNING]       = GameRuleValue("true");
    rules_[DO_MOB_LOOT]           = GameRuleValue("true");
    rules_[DO_TILE_DROPS]         = GameRuleValue("true");
    rules_[COMMAND_BLOCK_OUTPUT]  = GameRuleValue("true");
    rules_[NATURAL_REGENERATION]  = GameRuleValue("true");
    rules_[DO_DAYLIGHT_CYCLE]     = GameRuleValue("true");
}

void GameRules::addGameRule(const std::string& name, const std::string& value) {
    std::unique_lock lock(mutex_);
    rules_[name] = GameRuleValue(value);
}

void GameRules::setOrCreateGameRule(const std::string& name, const std::string& value) {
    std::unique_lock lock(mutex_);
    auto it = rules_.find(name);
    if (it != rules_.end()) {
        it->second.setValue(value);
    } else {
        rules_[name] = GameRuleValue(value);
    }
}

std::string GameRules::getGameRuleStringValue(const std::string& name) const {
    std::shared_lock lock(mutex_);
    auto it = rules_.find(name);
    if (it != rules_.end()) {
        return it->second.stringValue;
    }
    return "";
}

bool GameRules::getGameRuleBooleanValue(const std::string& name) const {
    std::shared_lock lock(mutex_);
    auto it = rules_.find(name);
    if (it != rules_.end()) {
        return it->second.getBooleanValue();
    }
    return false;
}

bool GameRules::hasRule(const std::string& name) const {
    std::shared_lock lock(mutex_);
    return rules_.count(name) > 0;
}

std::vector<std::string> GameRules::getRules() const {
    std::shared_lock lock(mutex_);
    std::vector<std::string> result;
    result.reserve(rules_.size());
    for (const auto& [key, _] : rules_) {
        result.push_back(key);
    }
    return result;
}

std::map<std::string, std::string> GameRules::writeToMap() const {
    std::shared_lock lock(mutex_);
    std::map<std::string, std::string> result;
    for (const auto& [key, val] : rules_) {
        result[key] = val.stringValue;
    }
    return result;
}

void GameRules::readFromMap(const std::map<std::string, std::string>& data) {
    std::unique_lock lock(mutex_);
    for (const auto& [key, val] : data) {
        auto it = rules_.find(key);
        if (it != rules_.end()) {
            it->second.setValue(val);
        } else {
            rules_[key] = GameRuleValue(val);
        }
    }
}

} // namespace mccpp
