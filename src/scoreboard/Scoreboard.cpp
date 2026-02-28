/**
 * Scoreboard.cpp — Scoreboard system implementation.
 *
 * Java references:
 *   net.minecraft.scoreboard.Scoreboard — All CRUD operations
 *   net.minecraft.scoreboard.ScorePlayerTeam — Team management
 *   net.minecraft.scoreboard.Score — Score comparator
 *
 * Thread safety: All public methods acquire shared_mutex (L20).
 * Read operations: shared_lock, write operations: unique_lock.
 */

#include "scoreboard/Scoreboard.h"
#include <iostream>

namespace mccpp {

// ═════════════════════════════════════════════════════════════════════════════
// Objectives
// ═════════════════════════════════════════════════════════════════════════════

bool Scoreboard::addObjective(const std::string& name, const ScoreCriteria* criteria) {
    std::unique_lock lock(mutex_);
    if (objectives_.count(name)) return false; // Java throws
    objectives_.emplace(name, ScoreObjective(name, criteria));
    return true;
}

const ScoreObjective* Scoreboard::getObjective(const std::string& name) const {
    std::shared_lock lock(mutex_);
    auto it = objectives_.find(name);
    return it != objectives_.end() ? &it->second : nullptr;
}

bool Scoreboard::removeObjective(const std::string& name) {
    std::unique_lock lock(mutex_);
    auto it = objectives_.find(name);
    if (it == objectives_.end()) return false;

    // Java: clear display slots referencing this objective
    for (int i = 0; i < NUM_DISPLAY_SLOTS; ++i) {
        if (displaySlots_[i] == name) {
            displaySlots_[i].clear();
        }
    }

    // Java: remove all scores for this objective
    for (auto& [playerName, objScores] : playerScores_) {
        objScores.erase(name);
    }

    objectives_.erase(it);
    return true;
}

std::vector<ScoreObjective> Scoreboard::getObjectives() const {
    std::shared_lock lock(mutex_);
    std::vector<ScoreObjective> result;
    result.reserve(objectives_.size());
    for (const auto& [name, obj] : objectives_) {
        result.push_back(obj);
    }
    return result;
}

// ═════════════════════════════════════════════════════════════════════════════
// Scores
// ═════════════════════════════════════════════════════════════════════════════

Score& Scoreboard::getOrCreateScore(const std::string& playerName,
                                      const std::string& objectiveName) {
    std::unique_lock lock(mutex_);
    auto& objScores = playerScores_[playerName];
    auto it = objScores.find(objectiveName);
    if (it == objScores.end()) {
        Score score;
        score.playerName = playerName;
        score.objectiveName = objectiveName;
        score.scorePoints = 0;
        auto [inserted, _] = objScores.emplace(objectiveName, std::move(score));
        return inserted->second;
    }
    return it->second;
}

std::vector<Score> Scoreboard::getSortedScores(const std::string& objectiveName) const {
    std::shared_lock lock(mutex_);
    std::vector<Score> result;
    for (const auto& [playerName, objScores] : playerScores_) {
        auto it = objScores.find(objectiveName);
        if (it != objScores.end()) {
            result.push_back(it->second);
        }
    }
    // Java: Collections.sort(scores, Score.scoreComparator) — descending
    std::sort(result.begin(), result.end(), Score::compare);
    return result;
}

void Scoreboard::removePlayer(const std::string& playerName) {
    std::unique_lock lock(mutex_);
    playerScores_.erase(playerName);
}

std::vector<Score> Scoreboard::getPlayerScores(const std::string& playerName) const {
    std::shared_lock lock(mutex_);
    std::vector<Score> result;
    auto it = playerScores_.find(playerName);
    if (it != playerScores_.end()) {
        for (const auto& [objName, score] : it->second) {
            result.push_back(score);
        }
    }
    return result;
}

// ═════════════════════════════════════════════════════════════════════════════
// Display Slots
// ═════════════════════════════════════════════════════════════════════════════

void Scoreboard::setDisplaySlot(DisplaySlot slot, const std::string& objectiveName) {
    std::unique_lock lock(mutex_);
    int32_t idx = static_cast<int32_t>(slot);
    if (idx >= 0 && idx < NUM_DISPLAY_SLOTS) {
        displaySlots_[idx] = objectiveName;
    }
}

std::optional<std::string> Scoreboard::getDisplaySlot(DisplaySlot slot) const {
    std::shared_lock lock(mutex_);
    int32_t idx = static_cast<int32_t>(slot);
    if (idx >= 0 && idx < NUM_DISPLAY_SLOTS && !displaySlots_[idx].empty()) {
        return displaySlots_[idx];
    }
    return std::nullopt;
}

// Java: Scoreboard.getObjectiveDisplaySlot
std::string Scoreboard::getDisplaySlotName(int32_t slot) {
    switch (slot) {
        case 0: return "list";
        case 1: return "sidebar";
        case 2: return "belowName";
        default: return "";
    }
}

// Java: Scoreboard.getObjectiveDisplaySlotNumber
int32_t Scoreboard::getDisplaySlotNumber(const std::string& name) {
    if (name == "list") return 0;
    if (name == "sidebar") return 1;
    if (name == "belowName") return 2;
    return -1;
}

// ═════════════════════════════════════════════════════════════════════════════
// Teams
// ═════════════════════════════════════════════════════════════════════════════

bool Scoreboard::createTeam(const std::string& name) {
    std::unique_lock lock(mutex_);
    if (teams_.count(name)) return false; // Java throws
    teams_.emplace(name, ScorePlayerTeam(name));
    return true;
}

const ScorePlayerTeam* Scoreboard::getTeam(const std::string& name) const {
    std::shared_lock lock(mutex_);
    auto it = teams_.find(name);
    return it != teams_.end() ? &it->second : nullptr;
}

ScorePlayerTeam* Scoreboard::getTeamMutable(const std::string& name) {
    std::unique_lock lock(mutex_);
    auto it = teams_.find(name);
    return it != teams_.end() ? &it->second : nullptr;
}

bool Scoreboard::removeTeam(const std::string& name) {
    std::unique_lock lock(mutex_);
    auto it = teams_.find(name);
    if (it == teams_.end()) return false;

    // Java: remove all team membership entries
    for (const auto& member : it->second.members) {
        teamMemberships_.erase(member);
    }

    teams_.erase(it);
    return true;
}

std::vector<ScorePlayerTeam> Scoreboard::getTeams() const {
    std::shared_lock lock(mutex_);
    std::vector<ScorePlayerTeam> result;
    result.reserve(teams_.size());
    for (const auto& [name, team] : teams_) {
        result.push_back(team);
    }
    return result;
}

const ScorePlayerTeam* Scoreboard::getPlayersTeam(const std::string& playerName) const {
    std::shared_lock lock(mutex_);
    auto it = teamMemberships_.find(playerName);
    if (it == teamMemberships_.end()) return nullptr;
    auto teamIt = teams_.find(it->second);
    return teamIt != teams_.end() ? &teamIt->second : nullptr;
}

bool Scoreboard::addPlayerToTeam(const std::string& playerName,
                                   const std::string& teamName) {
    std::unique_lock lock(mutex_);
    auto teamIt = teams_.find(teamName);
    if (teamIt == teams_.end()) return false;

    // Java: remove from current team first
    auto memberIt = teamMemberships_.find(playerName);
    if (memberIt != teamMemberships_.end()) {
        auto oldTeamIt = teams_.find(memberIt->second);
        if (oldTeamIt != teams_.end()) {
            oldTeamIt->second.members.erase(playerName);
        }
    }

    teamMemberships_[playerName] = teamName;
    teamIt->second.members.insert(playerName);
    return true;
}

bool Scoreboard::removePlayerFromTeams(const std::string& playerName) {
    std::unique_lock lock(mutex_);
    auto it = teamMemberships_.find(playerName);
    if (it == teamMemberships_.end()) return false;

    auto teamIt = teams_.find(it->second);
    if (teamIt != teams_.end()) {
        teamIt->second.members.erase(playerName);
    }
    teamMemberships_.erase(it);
    return true;
}

bool Scoreboard::removePlayerFromTeam(const std::string& playerName,
                                        const std::string& teamName) {
    std::unique_lock lock(mutex_);
    auto memberIt = teamMemberships_.find(playerName);
    if (memberIt == teamMemberships_.end() || memberIt->second != teamName) {
        return false; // Java throws IllegalStateException
    }

    auto teamIt = teams_.find(teamName);
    if (teamIt != teams_.end()) {
        teamIt->second.members.erase(playerName);
    }
    teamMemberships_.erase(memberIt);
    return true;
}

std::vector<std::string> Scoreboard::getTeamNames() const {
    std::shared_lock lock(mutex_);
    std::vector<std::string> result;
    result.reserve(teams_.size());
    for (const auto& [name, team] : teams_) {
        result.push_back(name);
    }
    return result;
}

} // namespace mccpp
