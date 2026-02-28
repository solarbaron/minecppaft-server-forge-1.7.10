/**
 * Scoreboard.h — Complete scoreboard system.
 *
 * Java references:
 *   net.minecraft.scoreboard.Scoreboard
 *   net.minecraft.scoreboard.ScoreObjective
 *   net.minecraft.scoreboard.ScorePlayerTeam
 *   net.minecraft.scoreboard.Score
 *   net.minecraft.scoreboard.IScoreObjectiveCriteria
 *
 * Implements objectives with criteria, teams with prefix/suffix,
 * player scores, 3 display slots, and sorted score retrieval.
 *
 * Thread safety: All mutating operations lock internal mutex.
 *
 * JNI readiness: String-keyed maps, simple types.
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// ScoreCriteria — Objective criteria type.
// Java reference: net.minecraft.scoreboard.IScoreObjectiveCriteria
// ═══════════════════════════════════════════════════════════════════════════

struct ScoreCriteria {
    std::string name;
    bool readOnly = false;

    // Java: INSTANCES static map — known criteria
    static const std::string DUMMY;
    static const std::string TRIGGER;
    static const std::string DEATH_COUNT;
    static const std::string PLAYER_KILL_COUNT;
    static const std::string TOTAL_KILL_COUNT;
    static const std::string HEALTH;
};

// Static criteria names
inline const std::string ScoreCriteria::DUMMY = "dummy";
inline const std::string ScoreCriteria::TRIGGER = "trigger";
inline const std::string ScoreCriteria::DEATH_COUNT = "deathCount";
inline const std::string ScoreCriteria::PLAYER_KILL_COUNT = "playerKillCount";
inline const std::string ScoreCriteria::TOTAL_KILL_COUNT = "totalKillCount";
inline const std::string ScoreCriteria::HEALTH = "health";

// ═══════════════════════════════════════════════════════════════════════════
// ScoreObjective — A named scoreboard objective.
// Java reference: net.minecraft.scoreboard.ScoreObjective
// ═══════════════════════════════════════════════════════════════════════════

struct ScoreObjective {
    std::string name;
    std::string displayName;
    ScoreCriteria criteria;
    int32_t renderType = 0; // 0=integer, 1=hearts

    ScoreObjective() = default;
    ScoreObjective(const std::string& n, const std::string& dn, const ScoreCriteria& c)
        : name(n), displayName(dn), criteria(c) {}
};

// ═══════════════════════════════════════════════════════════════════════════
// ScorePlayerTeam — A named team with members.
// Java reference: net.minecraft.scoreboard.ScorePlayerTeam
// ═══════════════════════════════════════════════════════════════════════════

struct ScorePlayerTeam {
    std::string registeredName;
    std::string displayName;
    std::string prefix;
    std::string suffix;
    bool allowFriendlyFire = true;
    bool canSeeFriendlyInvisibles = true;
    std::set<std::string> members;

    ScorePlayerTeam() = default;
    ScorePlayerTeam(const std::string& name)
        : registeredName(name), displayName(name) {}

    // Java: formatString
    std::string formatString(const std::string& playerName) const {
        return prefix + playerName + suffix;
    }

    // Java: func_98299_i — pack flags as bitmask
    int32_t getFriendlyFlags() const {
        int32_t flags = 0;
        if (allowFriendlyFire) flags |= 1;
        if (canSeeFriendlyInvisibles) flags |= 2;
        return flags;
    }

    // Java: setFriendlyFlags
    void setFriendlyFlags(int32_t flags) {
        allowFriendlyFire = (flags & 1) != 0;
        canSeeFriendlyInvisibles = (flags & 2) != 0;
    }

    // Java: static formatPlayerName
    static std::string formatPlayerName(const ScorePlayerTeam* team, const std::string& name) {
        return team ? team->formatString(name) : name;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// ScoreEntry — A player's score for an objective.
// Java reference: net.minecraft.scoreboard.Score
// ═══════════════════════════════════════════════════════════════════════════

struct ScoreEntry {
    std::string playerName;
    std::string objectiveName;
    int32_t points = 0;

    ScoreEntry() = default;
    ScoreEntry(const std::string& player, const std::string& obj)
        : playerName(player), objectiveName(obj) {}

    void increaseScore(int32_t n) { points += n; }
    void decreaseScore(int32_t n) { points -= n; }

    // Java: scoreComparator — sort by score descending
    static bool compareDescending(const ScoreEntry& a, const ScoreEntry& b) {
        return a.points > b.points;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Scoreboard — Full scoreboard state.
// Java reference: net.minecraft.scoreboard.Scoreboard
// ═══════════════════════════════════════════════════════════════════════════

class Scoreboard {
public:
    Scoreboard() = default;

    // ─── Objectives ───

    // Java: getObjective
    const ScoreObjective* getObjective(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = objectives_.find(name);
        return it != objectives_.end() ? &it->second : nullptr;
    }

    // Java: addScoreObjective
    ScoreObjective* addObjective(const std::string& name, const std::string& displayName,
                                  const ScoreCriteria& criteria) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (objectives_.count(name)) return nullptr; // Already exists
        objectives_[name] = ScoreObjective(name, displayName, criteria);
        criteriaObjectives_[criteria.name].push_back(name);
        return &objectives_[name];
    }

    // Java: func_96519_k — remove objective
    void removeObjective(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = objectives_.find(name);
        if (it == objectives_.end()) return;

        // Clear display slots
        for (int i = 0; i < 3; ++i) {
            if (displaySlots_[i] == name) displaySlots_[i].clear();
        }

        // Remove from criteria index
        auto& critList = criteriaObjectives_[it->second.criteria.name];
        critList.erase(std::remove(critList.begin(), critList.end(), name), critList.end());

        // Remove all scores for this objective
        for (auto& [player, scores] : playerScores_) {
            scores.erase(name);
        }

        objectives_.erase(it);
    }

    // Java: getScoreObjectives
    std::vector<std::string> getObjectiveNames() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> names;
        names.reserve(objectives_.size());
        for (const auto& [k, v] : objectives_) names.push_back(k);
        return names;
    }

    // ─── Display Slots ───
    // Java: 0=list, 1=sidebar, 2=belowName

    void setDisplaySlot(int32_t slot, const std::string& objectiveName) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (slot >= 0 && slot < 3) displaySlots_[slot] = objectiveName;
    }

    std::string getDisplaySlot(int32_t slot) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return (slot >= 0 && slot < 3) ? displaySlots_[slot] : "";
    }

    // Java: getObjectiveDisplaySlot — slot number to name
    static std::string getDisplaySlotName(int32_t slot) {
        switch (slot) {
            case 0: return "list";
            case 1: return "sidebar";
            case 2: return "belowName";
            default: return "";
        }
    }

    // Java: getObjectiveDisplaySlotNumber — name to slot number
    static int32_t getDisplaySlotNumber(const std::string& name) {
        if (name == "list") return 0;
        if (name == "sidebar") return 1;
        if (name == "belowName") return 2;
        return -1;
    }

    // ─── Scores ───

    // Java: getValueFromObjective
    ScoreEntry& getOrCreateScore(const std::string& playerName, const std::string& objectiveName) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& playerMap = playerScores_[playerName];
        auto it = playerMap.find(objectiveName);
        if (it == playerMap.end()) {
            playerMap[objectiveName] = ScoreEntry(playerName, objectiveName);
        }
        return playerMap[objectiveName];
    }

    // Java: getSortedScores — all scores for an objective, sorted descending
    std::vector<ScoreEntry> getSortedScores(const std::string& objectiveName) const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<ScoreEntry> result;
        for (const auto& [player, scores] : playerScores_) {
            auto it = scores.find(objectiveName);
            if (it != scores.end()) {
                result.push_back(it->second);
            }
        }
        std::sort(result.begin(), result.end(), ScoreEntry::compareDescending);
        return result;
    }

    // Java: func_96515_c — remove all scores for a player
    void removePlayerScores(const std::string& playerName) {
        std::lock_guard<std::mutex> lock(mutex_);
        playerScores_.erase(playerName);
    }

    // ─── Teams ───

    // Java: getTeam
    const ScorePlayerTeam* getTeam(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = teams_.find(name);
        return it != teams_.end() ? &it->second : nullptr;
    }

    // Java: createTeam
    ScorePlayerTeam* createTeam(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (teams_.count(name)) return nullptr; // Already exists
        teams_[name] = ScorePlayerTeam(name);
        return &teams_[name];
    }

    // Java: removeTeam
    void removeTeam(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = teams_.find(name);
        if (it == teams_.end()) return;

        // Remove all member->team mappings
        for (const auto& member : it->second.members) {
            teamMemberships_.erase(member);
        }
        teams_.erase(it);
    }

    // Java: func_151392_a — add player to team
    bool addPlayerToTeam(const std::string& playerName, const std::string& teamName) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = teams_.find(teamName);
        if (it == teams_.end()) return false;

        // Remove from current team first
        auto curTeam = teamMemberships_.find(playerName);
        if (curTeam != teamMemberships_.end()) {
            auto oldIt = teams_.find(curTeam->second);
            if (oldIt != teams_.end()) {
                oldIt->second.members.erase(playerName);
            }
        }

        teamMemberships_[playerName] = teamName;
        it->second.members.insert(playerName);
        return true;
    }

    // Java: removePlayerFromTeams
    bool removePlayerFromTeams(const std::string& playerName) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = teamMemberships_.find(playerName);
        if (it == teamMemberships_.end()) return false;

        auto teamIt = teams_.find(it->second);
        if (teamIt != teams_.end()) {
            teamIt->second.members.erase(playerName);
        }
        teamMemberships_.erase(it);
        return true;
    }

    // Java: getPlayersTeam
    std::string getPlayersTeam(const std::string& playerName) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = teamMemberships_.find(playerName);
        return it != teamMemberships_.end() ? it->second : "";
    }

    // Java: getTeamNames
    std::vector<std::string> getTeamNames() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> names;
        names.reserve(teams_.size());
        for (const auto& [k, v] : teams_) names.push_back(k);
        return names;
    }

private:
    mutable std::mutex mutex_;

    // Java: scoreObjectives — name -> objective
    std::unordered_map<std::string, ScoreObjective> objectives_;

    // Java: scoreObjectiveCriterias — criteria name -> list of objective names
    std::unordered_map<std::string, std::vector<std::string>> criteriaObjectives_;

    // Java: field_96544_c — player name -> (objective name -> score)
    std::unordered_map<std::string,
        std::unordered_map<std::string, ScoreEntry>> playerScores_;

    // Java: objectiveDisplaySlots[3] — 0=list, 1=sidebar, 2=belowName
    std::string displaySlots_[3];

    // Java: teams — team name -> team
    std::unordered_map<std::string, ScorePlayerTeam> teams_;

    // Java: teamMemberships — player name -> team name
    std::unordered_map<std::string, std::string> teamMemberships_;
};

} // namespace mccpp
