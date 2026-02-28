/**
 * Scoreboard.h — Server scoreboard system.
 *
 * Java references:
 *   - net.minecraft.scoreboard.Scoreboard — Main scoreboard container
 *   - net.minecraft.scoreboard.ScoreObjective — Named objective with criteria
 *   - net.minecraft.scoreboard.Score — Player-objective value pair
 *   - net.minecraft.scoreboard.ScorePlayerTeam — Team with prefix/suffix
 *   - net.minecraft.scoreboard.IScoreObjectiveCriteria — Criteria interface
 *   - net.minecraft.scoreboard.Team — Base team class
 *
 * Thread safety:
 *   - Scoreboard uses std::shared_mutex for concurrent read, exclusive write.
 *   - Lock hierarchy: L20 (scoreboard mutex)
 *   - Score modifications are atomic within the scoreboard lock.
 *
 * JNI readiness: String-based lookups, integer scores, simple struct layout.
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// Forward declarations
class Scoreboard;

// ═══════════════════════════════════════════════════════════════════════════
// IScoreObjectiveCriteria — Criteria type for objectives.
// Java reference: net.minecraft.scoreboard.IScoreObjectiveCriteria
// ═══════════════════════════════════════════════════════════════════════════

struct ScoreCriteria {
    std::string name;
    bool readOnly;  // Java: isReadOnly()

    // Java: IScoreObjectiveCriteria.func_96635_a — aggregate score from list
    // For "dummy" criteria, returns 0; for health, returns current health, etc.
    int32_t aggregateScore(const std::vector<int32_t>& /*values*/) const {
        return 0; // Dummy/trigger criteria
    }
};

// Built-in criteria types
// Java: IScoreObjectiveCriteria static fields
namespace Criteria {
    inline const ScoreCriteria DUMMY     = {"dummy", false};
    inline const ScoreCriteria TRIGGER   = {"trigger", false};
    inline const ScoreCriteria DEATH_COUNT = {"deathCount", true};
    inline const ScoreCriteria PLAYER_KILL_COUNT = {"playerKillCount", true};
    inline const ScoreCriteria TOTAL_KILL_COUNT = {"totalKillCount", true};
    inline const ScoreCriteria HEALTH    = {"health", true};
}

// ═══════════════════════════════════════════════════════════════════════════
// ScoreObjective — Named objective with a criteria type.
// Java reference: net.minecraft.scoreboard.ScoreObjective
// ═══════════════════════════════════════════════════════════════════════════

struct ScoreObjective {
    std::string name;
    std::string displayName;
    const ScoreCriteria* criteria;

    ScoreObjective() : criteria(nullptr) {}
    ScoreObjective(const std::string& n, const ScoreCriteria* c)
        : name(n), displayName(n), criteria(c) {}
};

// ═══════════════════════════════════════════════════════════════════════════
// Score — Player-objective value pair.
// Java reference: net.minecraft.scoreboard.Score
//
// Comparator sorts descending by score points (higher first).
// ═══════════════════════════════════════════════════════════════════════════

struct Score {
    std::string playerName;
    std::string objectiveName;
    int32_t scorePoints = 0;

    // Java: Score.increseScore [sic] — add to score
    void increaseScore(int32_t amount, bool readOnly) {
        if (readOnly) return; // Java throws, we silently skip
        scorePoints += amount;
    }

    // Java: Score.decreaseScore
    void decreaseScore(int32_t amount, bool readOnly) {
        if (readOnly) return;
        scorePoints -= amount;
    }

    // Comparator: descending by score
    static bool compare(const Score& a, const Score& b) {
        return a.scorePoints > b.scorePoints;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// ScorePlayerTeam — Team with name formatting and rules.
// Java reference: net.minecraft.scoreboard.ScorePlayerTeam
//
// Features:
//   - Color prefix and suffix for name formatting
//   - Friendly fire toggle
//   - See friendly invisibles toggle
//   - Bitfield encoding: bit 0 = friendly fire, bit 1 = see invisibles
// ═══════════════════════════════════════════════════════════════════════════

struct ScorePlayerTeam {
    std::string registeredName;   // Internal name (immutable)
    std::string displayName;      // Display name
    std::string namePrefix;       // Color prefix (e.g. "§c")
    std::string nameSuffix;       // Color suffix (e.g. "§r")
    std::set<std::string> members;
    bool allowFriendlyFire = true;
    bool seeFriendlyInvisibles = true;

    ScorePlayerTeam() = default;
    ScorePlayerTeam(const std::string& name)
        : registeredName(name), displayName(name) {}

    // Java: ScorePlayerTeam.formatString — prefix + name + suffix
    std::string formatPlayerName(const std::string& playerName) const {
        return namePrefix + playerName + nameSuffix;
    }

    // Java: ScorePlayerTeam.formatPlayerName(Team, String) — static version
    static std::string formatWithTeam(const ScorePlayerTeam* team,
                                        const std::string& playerName) {
        if (!team) return playerName;
        return team->formatPlayerName(playerName);
    }

    // Java: ScorePlayerTeam.func_98299_i — bitfield encoding
    int32_t getFlagBits() const {
        int32_t flags = 0;
        if (allowFriendlyFire) flags |= 1;
        if (seeFriendlyInvisibles) flags |= 2;
        return flags;
    }

    // Set flags from bitfield
    void setFlagBits(int32_t flags) {
        allowFriendlyFire = (flags & 1) != 0;
        seeFriendlyInvisibles = (flags & 2) != 0;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Display slots — Where objectives are shown to players.
// Java reference: net.minecraft.scoreboard.Scoreboard.objectiveDisplaySlots
// ═══════════════════════════════════════════════════════════════════════════

enum class DisplaySlot : int32_t {
    LIST       = 0,  // Tab player list
    SIDEBAR    = 1,  // Right side of screen
    BELOW_NAME = 2   // Below player name tags
};

constexpr int32_t NUM_DISPLAY_SLOTS = 3;

// ═══════════════════════════════════════════════════════════════════════════
// Scoreboard — Main scoreboard system.
// Java reference: net.minecraft.scoreboard.Scoreboard
//
// Thread safety: std::shared_mutex at lock hierarchy L20.
// Read operations take shared lock, write operations take exclusive lock.
// ═══════════════════════════════════════════════════════════════════════════

class Scoreboard {
public:
    Scoreboard() = default;

    // ─── Objectives ───

    // Java: Scoreboard.addScoreObjective
    bool addObjective(const std::string& name, const ScoreCriteria* criteria);

    // Java: Scoreboard.getObjective
    const ScoreObjective* getObjective(const std::string& name) const;

    // Java: Scoreboard.func_96519_k — Remove objective
    bool removeObjective(const std::string& name);

    // Java: Scoreboard.getScoreObjectives
    std::vector<ScoreObjective> getObjectives() const;

    // ─── Scores ───

    // Java: Scoreboard.getValueFromObjective
    Score& getOrCreateScore(const std::string& playerName,
                             const std::string& objectiveName);

    // Java: Scoreboard.getSortedScores
    std::vector<Score> getSortedScores(const std::string& objectiveName) const;

    // Java: Scoreboard.func_96515_c — Remove all scores for a player
    void removePlayer(const std::string& playerName);

    // Java: Scoreboard.func_96510_d — Get all scores for a player
    std::vector<Score> getPlayerScores(const std::string& playerName) const;

    // ─── Display Slots ───

    // Java: Scoreboard.setObjectiveInDisplaySlot
    void setDisplaySlot(DisplaySlot slot, const std::string& objectiveName);

    // Java: Scoreboard.getObjectiveInDisplaySlot
    std::optional<std::string> getDisplaySlot(DisplaySlot slot) const;

    // Java: Scoreboard.getObjectiveDisplaySlot — slot index to name
    static std::string getDisplaySlotName(int32_t slot);

    // Java: Scoreboard.getObjectiveDisplaySlotNumber — name to index
    static int32_t getDisplaySlotNumber(const std::string& name);

    // ─── Teams ───

    // Java: Scoreboard.createTeam
    bool createTeam(const std::string& name);

    // Java: Scoreboard.getTeam
    const ScorePlayerTeam* getTeam(const std::string& name) const;
    ScorePlayerTeam* getTeamMutable(const std::string& name);

    // Java: Scoreboard.removeTeam
    bool removeTeam(const std::string& name);

    // Java: Scoreboard.getTeams
    std::vector<ScorePlayerTeam> getTeams() const;

    // Java: Scoreboard.getPlayersTeam
    const ScorePlayerTeam* getPlayersTeam(const std::string& playerName) const;

    // Java: Scoreboard.func_151392_a — Add player to team
    bool addPlayerToTeam(const std::string& playerName,
                          const std::string& teamName);

    // Java: Scoreboard.removePlayerFromTeams
    bool removePlayerFromTeams(const std::string& playerName);

    // Java: Scoreboard.removePlayerFromTeam
    bool removePlayerFromTeam(const std::string& playerName,
                               const std::string& teamName);

    // Java: Scoreboard.getTeamNames
    std::vector<std::string> getTeamNames() const;

private:
    mutable std::shared_mutex mutex_;  // Lock hierarchy L20

    // Java: scoreObjectives — name → objective
    std::unordered_map<std::string, ScoreObjective> objectives_;

    // Java: field_96544_c — playerName → {objectiveName → Score}
    std::unordered_map<std::string,
        std::unordered_map<std::string, Score>> playerScores_;

    // Java: objectiveDisplaySlots[3]
    std::string displaySlots_[NUM_DISPLAY_SLOTS];

    // Java: teams — name → team
    std::unordered_map<std::string, ScorePlayerTeam> teams_;

    // Java: teamMemberships — playerName → team name
    std::unordered_map<std::string, std::string> teamMemberships_;
};

} // namespace mccpp
