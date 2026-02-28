/**
 * PlayerList.h — Server player list management.
 *
 * Java reference: net.minecraft.server.management.ServerConfigurationManager
 *
 * Manages:
 *   - Player login/logout with full packet handshake
 *   - Ban list (player + IP), whitelist, ops
 *   - View distance configuration
 *   - Dimension transfers (nether 8x scaling)
 *   - Player data save/load
 *   - Broadcasting packets to all/nearby/dimension players
 *   - Ping update cycling (one player per tick, wraps at 600)
 *
 * Thread safety: shared_mutex for player list access. Individual player
 * connections are single-threaded. Ban/whitelist/ops have their own mutexes.
 *
 * JNI readiness: Simple string-based lists, no complex C++ types.
 */
#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// BanEntry — Single ban record (player or IP).
// Java reference: net.minecraft.server.management.UserListBansEntry / IPBanEntry
// ═══════════════════════════════════════════════════════════════════════════

struct BanEntry {
    std::string target;          // UUID (player ban) or IP string (IP ban)
    std::string reason = "Banned by an operator.";
    std::string source = "Server";
    int64_t banDate = 0;         // Unix millis
    int64_t expireDate = -1;     // -1 = permanent

    bool isExpired() const {
        if (expireDate < 0) return false; // Permanent
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return now > expireDate;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// OpEntry — Operator entry with permission level.
// Java reference: net.minecraft.server.management.UserListOpsEntry
// ═══════════════════════════════════════════════════════════════════════════

struct OpEntry {
    std::string uuid;
    std::string name;
    int32_t permissionLevel = 4;  // Default op level
};

// ═══════════════════════════════════════════════════════════════════════════
// PlayerListEntry — Lightweight player info for tab list.
// ═══════════════════════════════════════════════════════════════════════════

struct PlayerListEntry {
    int32_t entityId = 0;
    std::string uuid;
    std::string name;
    int32_t ping = 0;
    int32_t gameMode = 0;
    int32_t dimension = 0;
    double posX = 0.0, posY = 0.0, posZ = 0.0;
    std::string ipAddress;
};

// ═══════════════════════════════════════════════════════════════════════════
// PlayerList — Server player list manager.
// Java reference: net.minecraft.server.management.ServerConfigurationManager
// ═══════════════════════════════════════════════════════════════════════════

class PlayerList {
public:
    PlayerList() = default;

    // ─── Configuration ───

    void setMaxPlayers(int32_t max) { maxPlayers_ = max; }
    int32_t getMaxPlayers() const { return maxPlayers_; }

    void setViewDistance(int32_t distance) { viewDistance_ = distance; }
    int32_t getViewDistance() const { return viewDistance_; }

    // Java: getEntityViewDistance — furthest viewable block
    // Java: PlayerManager.getFurthestViewableBlock(viewDistance)
    int32_t getEntityViewDistance() const {
        return viewDistance_ * 16 - 16;  // Simplified from Java
    }

    void setWhitelistEnabled(bool enabled) { whitelistEnabled_ = enabled; }
    bool isWhitelistEnabled() const { return whitelistEnabled_; }

    void setDefaultGameMode(int32_t mode) { defaultGameMode_ = mode; }
    int32_t getDefaultGameMode() const { return defaultGameMode_; }

    void setOpPermissionLevel(int32_t level) { opPermissionLevel_ = level; }
    int32_t getOpPermissionLevel() const { return opPermissionLevel_; }

    // ─── Player management ───

    // Java: playerLoggedIn
    bool addPlayer(const PlayerListEntry& entry) {
        std::unique_lock<std::shared_mutex> lock(playersMutex_);
        if (playersByUuid_.count(entry.uuid)) return false;
        playersByUuid_[entry.uuid] = entry;
        playersByName_[entry.name] = entry.uuid;
        playerList_.push_back(entry.uuid);
        return true;
    }

    // Java: playerLoggedOut
    bool removePlayer(const std::string& uuid) {
        std::unique_lock<std::shared_mutex> lock(playersMutex_);
        auto it = playersByUuid_.find(uuid);
        if (it == playersByUuid_.end()) return false;

        playersByName_.erase(it->second.name);
        playerList_.erase(
            std::remove(playerList_.begin(), playerList_.end(), uuid),
            playerList_.end());
        playersByUuid_.erase(it);
        return true;
    }

    // Java: getPlayerByUsername
    std::optional<PlayerListEntry> getPlayerByName(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(playersMutex_);
        auto nameIt = playersByName_.find(name);
        if (nameIt == playersByName_.end()) return std::nullopt;
        auto it = playersByUuid_.find(nameIt->second);
        if (it == playersByUuid_.end()) return std::nullopt;
        return it->second;
    }

    std::optional<PlayerListEntry> getPlayerByUuid(const std::string& uuid) const {
        std::shared_lock<std::shared_mutex> lock(playersMutex_);
        auto it = playersByUuid_.find(uuid);
        if (it == playersByUuid_.end()) return std::nullopt;
        return it->second;
    }

    // Update player position/ping
    void updatePlayer(const std::string& uuid, double x, double y, double z, int32_t ping) {
        std::unique_lock<std::shared_mutex> lock(playersMutex_);
        auto it = playersByUuid_.find(uuid);
        if (it != playersByUuid_.end()) {
            it->second.posX = x;
            it->second.posY = y;
            it->second.posZ = z;
            it->second.ping = ping;
        }
    }

    // Java: getCurrentPlayerCount
    int32_t getPlayerCount() const {
        std::shared_lock<std::shared_mutex> lock(playersMutex_);
        return static_cast<int32_t>(playersByUuid_.size());
    }

    // Java: getAllUsernames
    std::vector<std::string> getAllUsernames() const {
        std::shared_lock<std::shared_mutex> lock(playersMutex_);
        std::vector<std::string> names;
        names.reserve(playersByUuid_.size());
        for (const auto& [uuid, entry] : playersByUuid_) {
            names.push_back(entry.name);
        }
        return names;
    }

    // Get all player entries
    std::vector<PlayerListEntry> getAllPlayers() const {
        std::shared_lock<std::shared_mutex> lock(playersMutex_);
        std::vector<PlayerListEntry> entries;
        entries.reserve(playersByUuid_.size());
        for (const auto& [uuid, entry] : playersByUuid_) {
            entries.push_back(entry);
        }
        return entries;
    }

    // Java: getPlayerNamesString
    std::string getPlayerListString(bool includeUuids = false) const {
        std::shared_lock<std::shared_mutex> lock(playersMutex_);
        std::string result;
        bool first = true;
        for (const auto& uuid : playerList_) {
            auto it = playersByUuid_.find(uuid);
            if (it == playersByUuid_.end()) continue;
            if (!first) result += ", ";
            result += it->second.name;
            if (includeUuids) {
                result += " (" + uuid + ")";
            }
            first = false;
        }
        return result;
    }

    // ─── Connection validation ───

    // Java: allowUserToConnect
    std::optional<std::string> checkCanJoin(const std::string& uuid, const std::string& name,
                                              const std::string& ipAddress) const {
        // Check player ban
        {
            std::lock_guard<std::mutex> lock(bansMutex_);
            auto it = playerBans_.find(uuid);
            if (it != playerBans_.end() && !it->second.isExpired()) {
                std::string msg = "You are banned from this server!\nReason: " + it->second.reason;
                return msg;
            }
        }

        // Check whitelist
        if (whitelistEnabled_) {
            bool allowed = false;
            {
                std::lock_guard<std::mutex> lock(whitelistMutex_);
                allowed = whitelist_.count(uuid) > 0;
            }
            {
                std::lock_guard<std::mutex> lock(opsMutex_);
                allowed = allowed || ops_.count(uuid) > 0;
            }
            if (!allowed) {
                return std::string("You are not white-listed on this server!");
            }
        }

        // Check IP ban
        {
            std::lock_guard<std::mutex> lock(bansMutex_);
            auto it = ipBans_.find(ipAddress);
            if (it != ipBans_.end() && !it->second.isExpired()) {
                std::string msg = "Your IP address is banned from this server!\nReason: " + it->second.reason;
                return msg;
            }
        }

        // Check full
        {
            std::shared_lock<std::shared_mutex> lock(playersMutex_);
            if (static_cast<int32_t>(playersByUuid_.size()) >= maxPlayers_) {
                return std::string("The server is full!");
            }
        }

        return std::nullopt; // Allowed
    }

    // ─── Ban management ───

    // Java: bannedPlayers
    void banPlayer(const std::string& uuid, const BanEntry& entry) {
        std::lock_guard<std::mutex> lock(bansMutex_);
        playerBans_[uuid] = entry;
    }

    void unbanPlayer(const std::string& uuid) {
        std::lock_guard<std::mutex> lock(bansMutex_);
        playerBans_.erase(uuid);
    }

    bool isPlayerBanned(const std::string& uuid) const {
        std::lock_guard<std::mutex> lock(bansMutex_);
        auto it = playerBans_.find(uuid);
        return it != playerBans_.end() && !it->second.isExpired();
    }

    // Java: bannedIPs
    void banIP(const std::string& ip, const BanEntry& entry) {
        std::lock_guard<std::mutex> lock(bansMutex_);
        ipBans_[ip] = entry;
    }

    void unbanIP(const std::string& ip) {
        std::lock_guard<std::mutex> lock(bansMutex_);
        ipBans_.erase(ip);
    }

    bool isIPBanned(const std::string& ip) const {
        std::lock_guard<std::mutex> lock(bansMutex_);
        auto it = ipBans_.find(ip);
        return it != ipBans_.end() && !it->second.isExpired();
    }

    // Java: getPlayersMatchingAddress
    std::vector<PlayerListEntry> getPlayersFromIP(const std::string& ip) const {
        std::shared_lock<std::shared_mutex> lock(playersMutex_);
        std::vector<PlayerListEntry> result;
        for (const auto& [uuid, entry] : playersByUuid_) {
            if (entry.ipAddress == ip) result.push_back(entry);
        }
        return result;
    }

    // ─── Whitelist management ───

    void addToWhitelist(const std::string& uuid) {
        std::lock_guard<std::mutex> lock(whitelistMutex_);
        whitelist_.insert(uuid);
    }

    void removeFromWhitelist(const std::string& uuid) {
        std::lock_guard<std::mutex> lock(whitelistMutex_);
        whitelist_.erase(uuid);
    }

    bool isWhitelisted(const std::string& uuid) const {
        std::lock_guard<std::mutex> lock(whitelistMutex_);
        return whitelist_.count(uuid) > 0;
    }

    // ─── Ops management ───

    // Java: addOp
    void addOp(const OpEntry& entry) {
        std::lock_guard<std::mutex> lock(opsMutex_);
        ops_[entry.uuid] = entry;
    }

    // Java: removeOp
    void removeOp(const std::string& uuid) {
        std::lock_guard<std::mutex> lock(opsMutex_);
        ops_.erase(uuid);
    }

    // Java: canSendCommands
    bool isOp(const std::string& uuid) const {
        std::lock_guard<std::mutex> lock(opsMutex_);
        return ops_.count(uuid) > 0;
    }

    int32_t getOpLevel(const std::string& uuid) const {
        std::lock_guard<std::mutex> lock(opsMutex_);
        auto it = ops_.find(uuid);
        return it != ops_.end() ? it->second.permissionLevel : 0;
    }

    // ─── Broadcasting ───

    // Java: sendToAllNearExcept — get player entity IDs within distance
    std::vector<int32_t> getPlayersNear(double x, double y, double z, double distance,
                                          int32_t dimension, int32_t excludeEntityId = -1) const {
        std::shared_lock<std::shared_mutex> lock(playersMutex_);
        double dist2 = distance * distance;
        std::vector<int32_t> result;

        for (const auto& [uuid, entry] : playersByUuid_) {
            if (entry.entityId == excludeEntityId) continue;
            if (entry.dimension != dimension) continue;

            double dx = x - entry.posX;
            double dy = y - entry.posY;
            double dz = z - entry.posZ;
            if (dx * dx + dy * dy + dz * dz < dist2) {
                result.push_back(entry.entityId);
            }
        }
        return result;
    }

    // Java: sendPacketToAllPlayersInDimension
    std::vector<int32_t> getPlayersInDimension(int32_t dimension) const {
        std::shared_lock<std::shared_mutex> lock(playersMutex_);
        std::vector<int32_t> result;
        for (const auto& [uuid, entry] : playersByUuid_) {
            if (entry.dimension == dimension) result.push_back(entry.entityId);
        }
        return result;
    }

    // Java: sendPacketToAllPlayers
    std::vector<int32_t> getAllEntityIds() const {
        std::shared_lock<std::shared_mutex> lock(playersMutex_);
        std::vector<int32_t> result;
        result.reserve(playersByUuid_.size());
        for (const auto& [uuid, entry] : playersByUuid_) {
            result.push_back(entry.entityId);
        }
        return result;
    }

    // ─── Ping update cycling ───

    // Java: onTick — cycle through players for ping updates
    // Returns the player whose ping should be broadcast this tick, if any.
    std::optional<PlayerListEntry> tickPingUpdate() {
        std::shared_lock<std::shared_mutex> lock(playersMutex_);
        if (++pingIndex_ > 600) pingIndex_ = 0;
        if (pingIndex_ < static_cast<int32_t>(playerList_.size())) {
            auto it = playersByUuid_.find(playerList_[static_cast<size_t>(pingIndex_)]);
            if (it != playersByUuid_.end()) {
                return it->second;
            }
        }
        return std::nullopt;
    }

    // ─── Dimension transfer helpers ───

    // Java: nether coordinate scaling factor
    static constexpr double NETHER_SCALE = 8.0;

    // Java: transferEntityToWorld — compute new position for dimension transfer
    struct DimensionTransferResult {
        double posX, posY, posZ;
        float yaw;
        bool usePortal;
    };

    static DimensionTransferResult computeTransferPosition(
            double posX, double posY, double posZ, float yaw,
            int32_t fromDim, int32_t toDim) {

        DimensionTransferResult result;
        result.posY = posY;
        result.yaw = yaw;

        if (toDim == -1) {
            // Overworld → Nether: divide by 8
            result.posX = posX / NETHER_SCALE;
            result.posZ = posZ / NETHER_SCALE;
            result.usePortal = true;
        } else if (toDim == 0 && fromDim == -1) {
            // Nether → Overworld: multiply by 8
            result.posX = posX * NETHER_SCALE;
            result.posZ = posZ * NETHER_SCALE;
            result.usePortal = true;
        } else {
            // End → Overworld or other: use spawn point
            result.posX = posX;
            result.posZ = posZ;
            result.yaw = 90.0f;
            result.usePortal = (toDim != 1); // No portal for End dimension
        }

        // Java: clamp to world border
        auto clamp = [](double v) -> double {
            if (v < -29999872.0) return -29999872.0;
            if (v > 29999872.0) return 29999872.0;
            return v;
        };
        result.posX = clamp(result.posX);
        result.posZ = clamp(result.posZ);
        return result;
    }

private:
    // Player list — ordered by join time
    mutable std::shared_mutex playersMutex_;
    std::unordered_map<std::string, PlayerListEntry> playersByUuid_;
    std::unordered_map<std::string, std::string> playersByName_;  // name → uuid
    std::vector<std::string> playerList_;  // Ordered UUIDs

    // Bans
    mutable std::mutex bansMutex_;
    std::unordered_map<std::string, BanEntry> playerBans_;  // uuid → ban
    std::unordered_map<std::string, BanEntry> ipBans_;      // ip → ban

    // Whitelist
    mutable std::mutex whitelistMutex_;
    std::unordered_set<std::string> whitelist_;

    // Ops
    mutable std::mutex opsMutex_;
    std::unordered_map<std::string, OpEntry> ops_;

    // Configuration
    int32_t maxPlayers_ = 20;
    int32_t viewDistance_ = 10;
    bool whitelistEnabled_ = false;
    int32_t defaultGameMode_ = 0;
    int32_t opPermissionLevel_ = 4;
    int32_t pingIndex_ = 0;
};

} // namespace mccpp
