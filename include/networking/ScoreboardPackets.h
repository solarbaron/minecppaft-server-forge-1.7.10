/**
 * ScoreboardPackets.h — Scoreboard and team packet structures.
 *
 * Java references:
 *   - S3BPacketScoreboardObjective (45 lines)
 *   - S3CPacketUpdateScore (52 lines)
 *   - S3DPacketDisplayScoreboard (36 lines)
 *   - S3EPacketTeams (93 lines)
 *
 * These complete the play-state server-to-client packet set.
 *
 * Thread safety: Packet structs are value types, no shared state.
 * JNI readiness: Simple POD structs.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// S3B — Scoreboard Objective
// Packet ID: 0x3B
// Fields: String objectiveName(max 16), String objectiveDisplay(max 32),
//         Byte mode
//
// Mode:
//   0 = Create objective
//   1 = Remove objective
//   2 = Update display name
// ═══════════════════════════════════════════════════════════════════════════

struct S3BScoreboardObjective {
    static constexpr uint8_t PACKET_ID = 0x3B;
    std::string objectiveName;    // Max 16 chars
    std::string displayName;      // Max 32 chars
    int8_t mode = 0;

    static constexpr int8_t MODE_CREATE = 0;
    static constexpr int8_t MODE_REMOVE = 1;
    static constexpr int8_t MODE_UPDATE = 2;
};

// ═══════════════════════════════════════════════════════════════════════════
// S3C — Update Score
// Packet ID: 0x3C
// Fields: String itemName(max 16), Byte action, String objectiveName(max 16),
//         [Int value if action != 1]
//
// Action:
//   0 = Create/update score
//   1 = Remove score
// ═══════════════════════════════════════════════════════════════════════════

struct S3CUpdateScore {
    static constexpr uint8_t PACKET_ID = 0x3C;
    std::string itemName;         // Player name, max 16 chars
    int8_t action = 0;
    std::string objectiveName;    // Max 16 chars
    int32_t value = 0;            // Only sent if action != 1

    static constexpr int8_t ACTION_UPDATE = 0;
    static constexpr int8_t ACTION_REMOVE = 1;

    bool hasValue() const { return action != ACTION_REMOVE; }
};

// ═══════════════════════════════════════════════════════════════════════════
// S3D — Display Scoreboard
// Packet ID: 0x3D
// Fields: Byte position, String objectiveName(max 16)
//
// Position:
//   0 = player list (tab)
//   1 = sidebar
//   2 = below name
// ═══════════════════════════════════════════════════════════════════════════

struct S3DDisplayScoreboard {
    static constexpr uint8_t PACKET_ID = 0x3D;
    int8_t position = 0;
    std::string objectiveName;

    static constexpr int8_t POS_PLAYER_LIST = 0;
    static constexpr int8_t POS_SIDEBAR = 1;
    static constexpr int8_t POS_BELOW_NAME = 2;
};

// ═══════════════════════════════════════════════════════════════════════════
// S3E — Teams
// Packet ID: 0x3E
// Fields: String teamName(max 16), Byte mode,
//   [if mode 0 or 2: String displayName(max 32), String prefix(max 16),
//                     String suffix(max 16), Byte friendlyFire]
//   [if mode 0, 3, or 4: Short playerCount, String[] players(max 40 each)]
//
// Mode:
//   0 = Create team (all fields + players)
//   1 = Remove team (no extra fields)
//   2 = Update team info (display/prefix/suffix/flags)
//   3 = Add players to team
//   4 = Remove players from team
//
// friendlyFire flags:
//   0x01 = allowFriendlyFire
//   0x02 = seeFriendlyInvisibles
// ═══════════════════════════════════════════════════════════════════════════

struct S3ETeams {
    static constexpr uint8_t PACKET_ID = 0x3E;
    std::string teamName;          // Max 16 chars
    int8_t mode = 0;

    // Only for mode 0, 2:
    std::string displayName;       // Max 32 chars
    std::string prefix;            // Max 16 chars (color prefix)
    std::string suffix;            // Max 16 chars (color suffix)
    int8_t friendlyFireFlags = 0;

    // Only for mode 0, 3, 4:
    std::vector<std::string> players;  // Max 40 chars each

    static constexpr int8_t MODE_CREATE = 0;
    static constexpr int8_t MODE_REMOVE = 1;
    static constexpr int8_t MODE_UPDATE = 2;
    static constexpr int8_t MODE_ADD_PLAYERS = 3;
    static constexpr int8_t MODE_REMOVE_PLAYERS = 4;

    static constexpr int8_t FLAG_FRIENDLY_FIRE = 0x01;
    static constexpr int8_t FLAG_SEE_INVISIBLE = 0x02;

    bool hasTeamInfo() const { return mode == MODE_CREATE || mode == MODE_UPDATE; }
    bool hasPlayers() const {
        return mode == MODE_CREATE || mode == MODE_ADD_PLAYERS || mode == MODE_REMOVE_PLAYERS;
    }
};

} // namespace mccpp
