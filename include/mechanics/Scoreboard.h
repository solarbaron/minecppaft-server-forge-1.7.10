#pragma once
// Scoreboard — objectives, scores, teams, and display slots.
// Ported from azo.java (Scoreboard) and azs.java (ScoreObjective).
//
// Vanilla scoreboard:
//   - Objectives: named objectives with criteria and display name
//   - Scores: per-player per-objective integer scores
//   - Teams: named teams with prefix, suffix, color, friendly fire settings
//   - Display slots: 0=list, 1=sidebar, 2=belowName
//
// Uses packet definitions from PlayPackets.h (0x3B-0x3E)

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "networking/PlayPackets.h"

namespace mc {

// Scoreboard criteria types — from azl.java (IScoreObjectiveCriteria)
enum class ScoreboardCriteria {
    DUMMY,           // Manual only
    TRIGGER,         // Player-triggered
    DEATH_COUNT,     // Deaths
    PLAYER_KILL_COUNT, // Player kills
    TOTAL_KILL_COUNT,  // Total kills
    HEALTH,          // Current health
};

// Scoreboard objective
struct ScoreboardObjective {
    std::string name;         // Internal name (max 16 chars)
    std::string displayName;  // Display name (max 32 chars)
    ScoreboardCriteria criteria = ScoreboardCriteria::DUMMY;
};

// Team settings
struct ScoreboardTeam {
    std::string name;          // Internal name (max 16)
    std::string displayName;   // Display (max 32)
    std::string prefix;        // Prefix (max 16)
    std::string suffix;        // Suffix (max 16)
    bool friendlyFire = true;
    bool seeFriendlyInvisible = true;
    int8_t color = -1;         // -1=none, 0-15=chat color
    std::vector<std::string> members;
};

// S→C 0x3E Teams — not yet in PlayPackets.h
struct TeamsPacket {
    std::string teamName;
    int8_t mode; // 0=create, 1=remove, 2=update, 3=addPlayers, 4=removePlayers
    std::string displayName;
    std::string prefix;
    std::string suffix;
    int8_t friendlyFire = 0; // 0=off, 1=on, 3=seeInvisible
    std::vector<std::string> players;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x3E);
        buf.writeString(teamName);
        buf.writeByte(mode);

        if (mode == 0 || mode == 2) {
            buf.writeString(displayName);
            buf.writeString(prefix);
            buf.writeString(suffix);
            buf.writeByte(friendlyFire);
        }

        if (mode == 0 || mode == 3 || mode == 4) {
            buf.writeShort(static_cast<int16_t>(players.size()));
            for (auto& p : players) {
                buf.writeString(p);
            }
        }

        return buf;
    }
};

// Main Scoreboard manager
class Scoreboard {
public:
    // === Objectives ===
    bool addObjective(const std::string& name, const std::string& dispName,
                      ScoreboardCriteria criteria = ScoreboardCriteria::DUMMY) {
        if (objectives_.count(name)) return false;
        objectives_[name] = {name, dispName, criteria};
        return true;
    }

    bool removeObjective(const std::string& name) {
        if (!objectives_.count(name)) return false;
        objectives_.erase(name);
        scores_.erase(name);
        for (auto& [slot, objName] : displaySlots_) {
            if (objName == name) objName.clear();
        }
        return true;
    }

    const ScoreboardObjective* getObjective(const std::string& name) const {
        auto it = objectives_.find(name);
        return it != objectives_.end() ? &it->second : nullptr;
    }

    // === Scores ===
    void setScore(const std::string& objective, const std::string& player, int32_t value) {
        scores_[objective][player] = value;
    }

    int32_t getScore(const std::string& objective, const std::string& player) const {
        auto it = scores_.find(objective);
        if (it == scores_.end()) return 0;
        auto pit = it->second.find(player);
        return pit != it->second.end() ? pit->second : 0;
    }

    void addScore(const std::string& objective, const std::string& player, int32_t delta) {
        scores_[objective][player] += delta;
    }

    void resetScore(const std::string& objective, const std::string& player) {
        auto it = scores_.find(objective);
        if (it != scores_.end()) it->second.erase(player);
    }

    std::vector<std::pair<std::string, int32_t>> getScores(const std::string& objective) const {
        std::vector<std::pair<std::string, int32_t>> result;
        auto it = scores_.find(objective);
        if (it != scores_.end()) {
            for (auto& [n, v] : it->second) result.emplace_back(n, v);
            std::sort(result.begin(), result.end(),
                      [](auto& a, auto& b) { return a.second > b.second; });
        }
        return result;
    }

    // === Display Slots ===
    void setDisplaySlot(int8_t slot, const std::string& objective) {
        displaySlots_[slot] = objective;
    }

    std::string getDisplaySlot(int8_t slot) const {
        auto it = displaySlots_.find(slot);
        return it != displaySlots_.end() ? it->second : "";
    }

    // === Teams ===
    bool addTeam(const std::string& name, const std::string& dispName = "") {
        if (teams_.count(name)) return false;
        ScoreboardTeam team;
        team.name = name;
        team.displayName = dispName.empty() ? name : dispName;
        teams_[name] = std::move(team);
        return true;
    }

    bool removeTeam(const std::string& name) {
        auto it = teams_.find(name);
        if (it == teams_.end()) return false;
        for (auto& m : it->second.members) playerTeams_.erase(m);
        teams_.erase(it);
        return true;
    }

    ScoreboardTeam* getTeam(const std::string& name) {
        auto it = teams_.find(name);
        return it != teams_.end() ? &it->second : nullptr;
    }

    bool addPlayerToTeam(const std::string& teamName, const std::string& player) {
        auto it = teams_.find(teamName);
        if (it == teams_.end()) return false;
        removePlayerFromTeam(player);
        it->second.members.push_back(player);
        playerTeams_[player] = teamName;
        return true;
    }

    bool removePlayerFromTeam(const std::string& player) {
        auto pit = playerTeams_.find(player);
        if (pit == playerTeams_.end()) return false;
        auto tit = teams_.find(pit->second);
        if (tit != teams_.end()) {
            auto& m = tit->second.members;
            m.erase(std::remove(m.begin(), m.end(), player), m.end());
        }
        playerTeams_.erase(pit);
        return true;
    }

    std::string getPlayerTeam(const std::string& player) const {
        auto it = playerTeams_.find(player);
        return it != playerTeams_.end() ? it->second : "";
    }

    // === Packet Helpers (uses PlayPackets.h structs) ===
    ScoreboardObjectivePacket makeObjectivePacket(const std::string& objName, uint8_t mode) const {
        ScoreboardObjectivePacket pkt;
        pkt.name = objName;
        pkt.mode = mode;
        auto obj = getObjective(objName);
        pkt.displayName = obj ? obj->displayName : "";
        return pkt;
    }

    UpdateScorePacket makeScorePacket(const std::string& objective,
                                      const std::string& player, uint8_t action) const {
        UpdateScorePacket pkt;
        pkt.itemName = player;
        pkt.action = action;
        pkt.objectiveName = objective;
        pkt.value = (action == 0) ? getScore(objective, player) : 0;
        return pkt;
    }

    DisplayScoreboardPacket makeDisplayPacket(int8_t slot) const {
        DisplayScoreboardPacket pkt;
        pkt.position = static_cast<uint8_t>(slot);
        pkt.scoreName = getDisplaySlot(slot);
        return pkt;
    }

    TeamsPacket makeTeamPacket(const std::string& name, int8_t mode) const {
        TeamsPacket pkt;
        pkt.teamName = name;
        pkt.mode = mode;
        auto it = teams_.find(name);
        if (it != teams_.end()) {
            pkt.displayName = it->second.displayName;
            pkt.prefix = it->second.prefix;
            pkt.suffix = it->second.suffix;
            pkt.friendlyFire = it->second.friendlyFire ? 1 : 0;
            if (it->second.seeFriendlyInvisible) pkt.friendlyFire |= 2;
            pkt.players = it->second.members;
        }
        return pkt;
    }

    std::vector<std::string> objectiveNames() const {
        std::vector<std::string> names;
        for (auto& [n, _] : objectives_) names.push_back(n);
        return names;
    }

    std::vector<std::string> teamNames() const {
        std::vector<std::string> names;
        for (auto& [n, _] : teams_) names.push_back(n);
        return names;
    }

private:
    std::unordered_map<std::string, ScoreboardObjective> objectives_;
    std::unordered_map<std::string, std::unordered_map<std::string, int32_t>> scores_;
    std::unordered_map<int8_t, std::string> displaySlots_;
    std::unordered_map<std::string, ScoreboardTeam> teams_;
    std::unordered_map<std::string, std::string> playerTeams_;
};

} // namespace mc
