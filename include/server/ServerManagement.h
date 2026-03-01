/**
 * ServerManagement.h — Server management lists and player block interaction.
 *
 * Java references:
 *   - net.minecraft.server.management.UserList (160 lines)
 *   - net.minecraft.server.management.UserListEntry (57 lines)
 *   - net.minecraft.server.management.UserListBans (46 lines)
 *   - net.minecraft.server.management.UserListBansEntry (73 lines)
 *   - net.minecraft.server.management.BanList (46 lines — IP bans)
 *   - net.minecraft.server.management.IPBanEntry (73 lines)
 *   - net.minecraft.server.management.UserListOps (26 lines)
 *   - net.minecraft.server.management.UserListOpsEntry (41 lines)
 *   - net.minecraft.server.management.UserListWhitelist (32 lines)
 *   - net.minecraft.server.management.UserListWhitelistEntry (35 lines)
 *   - net.minecraft.server.management.ItemInWorldManager (240 lines)
 *
 * Thread safety:
 *   - UserList: std::shared_mutex for concurrent read, exclusive write
 *   - ItemInWorldManager: per-player, accessed on player's tick thread
 * JNI readiness: Simple string keys, POD-compatible entry structs.
 */
#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// UserListEntry — Base entry with optional expiry date.
// Java: net.minecraft.server.management.UserListEntry (57 lines)
//
//   value: the key object (player name, IP, etc.)
//   created: when the entry was added
//   source: who added it (e.g., "Server")
//   expires: when the ban/entry expires (empty = never)
//   reason: human-readable reason
//
//   hasBanExpired: checks if current time > expires
// ═══════════════════════════════════════════════════════════════════════════

struct UserListEntry {
    std::string value;           // key (player name or IP)
    std::string created;         // ISO date string
    std::string source;          // who added (default: "Server")
    std::string expires;         // ISO date or "forever"
    std::string reason;          // reason text

    UserListEntry() = default;
    explicit UserListEntry(const std::string& val,
                           const std::string& src = "Server",
                           const std::string& rsn = "Banned by an operator.")
        : value(val), source(src), reason(rsn), expires("forever") {}

    bool hasBanExpired() const {
        if (expires.empty() || expires == "forever") return false;
        // In production, parse ISO date and compare to current time
        // For now, never-expiring entries are the common case
        return false;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// UserList — Generic JSON-backed list with thread-safe access.
// Java: net.minecraft.server.management.UserList (160 lines)
//
//   values: HashMap<String, UserListEntry>
//   saveFile: JSON file path
//   lanServer: if true, entries are relaxed
//
//   Thread safety: std::shared_mutex for concurrent reads
//   File I/O: save on add/remove, load on startup
// ═══════════════════════════════════════════════════════════════════════════

class UserList {
public:
    explicit UserList(const std::string& filePath)
        : saveFilePath_(filePath) {}

    // ─── Add entry (exclusive lock) ───
    void addEntry(const UserListEntry& entry) {
        std::unique_lock lock(mutex_);
        values_[getKey(entry.value)] = entry;
        // Auto-save on modification
        saveToFile_();
    }

    // ─── Get entry (shared lock, removes expired first) ───
    std::optional<UserListEntry> getEntry(const std::string& key) {
        std::unique_lock lock(mutex_);
        removeExpired_();
        auto it = values_.find(getKey(key));
        if (it != values_.end()) return it->second;
        return std::nullopt;
    }

    // ─── Remove entry (exclusive lock) ───
    void removeEntry(const std::string& key) {
        std::unique_lock lock(mutex_);
        values_.erase(getKey(key));
        saveToFile_();
    }

    // ─── Check if entry exists (shared lock) ───
    bool hasEntry(const std::string& key) {
        std::shared_lock lock(mutex_);
        return values_.find(getKey(key)) != values_.end();
    }

    // ─── Get all keys (shared lock) ───
    std::vector<std::string> getKeys() const {
        std::shared_lock lock(mutex_);
        std::vector<std::string> keys;
        keys.reserve(values_.size());
        for (const auto& [k, v] : values_) {
            keys.push_back(k);
        }
        return keys;
    }

    // ─── Check if list is empty ───
    // Java: hasEntries() returns values.size() < 1 (confusingly named)
    bool isEmpty() const {
        std::shared_lock lock(mutex_);
        return values_.size() < 1;
    }

    size_t size() const {
        std::shared_lock lock(mutex_);
        return values_.size();
    }

    bool isLanServer() const { return lanServer_; }
    void setLanServer(bool lan) { lanServer_ = lan; }

    // ─── File I/O ───
    // Simplified JSON-like persistence (in production, use proper JSON)
    void loadFromFile() {
        std::unique_lock lock(mutex_);
        // Load entries from saveFilePath_
        // In production, parse JSON array of UserListEntry objects
    }

    void saveToFile() {
        std::unique_lock lock(mutex_);
        saveToFile_();
    }

protected:
    virtual std::string getKey(const std::string& value) const {
        // Default: lowercase the key
        std::string lower = value;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower;
    }

private:
    void removeExpired_() {
        std::vector<std::string> expired;
        for (const auto& [key, entry] : values_) {
            if (entry.hasBanExpired()) {
                expired.push_back(key);
            }
        }
        for (const auto& key : expired) {
            values_.erase(key);
        }
    }

    void saveToFile_() {
        // In production, serialize to JSON and write to saveFilePath_
    }

    std::string saveFilePath_;
    std::unordered_map<std::string, UserListEntry> values_;
    bool lanServer_ = true;
    mutable std::shared_mutex mutex_;
};

// ═══════════════════════════════════════════════════════════════════════════
// UserListBans — Player name ban list.
// Java: net.minecraft.server.management.UserListBans (46 lines)
// File: banned-players.json
// ═══════════════════════════════════════════════════════════════════════════

class UserListBans : public UserList {
public:
    UserListBans() : UserList("banned-players.json") {}

    bool isBanned(const std::string& playerName) {
        return hasEntry(playerName);
    }

    std::optional<UserListEntry> getBanEntry(const std::string& playerName) {
        return getEntry(playerName);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// BanList — IP address ban list.
// Java: net.minecraft.server.management.BanList (46 lines)
// File: banned-ips.json
//
//   addressToString: extracts IP from socket address
//     Removes leading "/" and trailing ":port"
//     e.g., "/192.168.1.1:25565" → "192.168.1.1"
// ═══════════════════════════════════════════════════════════════════════════

class BanList : public UserList {
public:
    BanList() : UserList("banned-ips.json") {}

    bool isBanned(const std::string& socketAddress) {
        return hasEntry(addressToString(socketAddress));
    }

    std::optional<UserListEntry> getBanEntry(const std::string& socketAddress) {
        return getEntry(addressToString(socketAddress));
    }

    // Extract IP from socket address string
    // Java: BanList.addressToString(SocketAddress)
    //   Remove "/" prefix and ":port" suffix
    static std::string addressToString(const std::string& addr) {
        std::string result = addr;
        // Remove leading "/"
        auto slashPos = result.find('/');
        if (slashPos != std::string::npos) {
            result = result.substr(slashPos + 1);
        }
        // Remove trailing ":port"
        auto colonPos = result.find(':');
        if (colonPos != std::string::npos) {
            result = result.substr(0, colonPos);
        }
        return result;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// UserListOps — Operator list with permission levels.
// Java: net.minecraft.server.management.UserListOps (26 lines)
// File: ops.json
//
// OpsEntry adds: permissionLevel (1-4), bypassesPlayerLimit
// ═══════════════════════════════════════════════════════════════════════════

struct OpsEntry : public UserListEntry {
    int32_t permissionLevel = 4;
    bool bypassesPlayerLimit = false;

    OpsEntry() = default;
    explicit OpsEntry(const std::string& name, int32_t level = 4,
                      bool bypass = false)
        : UserListEntry(name), permissionLevel(level),
          bypassesPlayerLimit(bypass) {}
};

class UserListOps : public UserList {
public:
    UserListOps() : UserList("ops.json") {}

    int32_t getPermissionLevel(const std::string& playerName) {
        auto entry = getEntry(playerName);
        // Default to 0 if not an op
        return entry.has_value() ? 4 : 0;
        // In production: deserialize OpsEntry with permissionLevel field
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// UserListWhitelist — Whitelist for allowed players.
// Java: net.minecraft.server.management.UserListWhitelist (32 lines)
// File: whitelist.json
// ═══════════════════════════════════════════════════════════════════════════

class UserListWhitelist : public UserList {
public:
    UserListWhitelist() : UserList("whitelist.json") {}

    bool isWhitelisted(const std::string& playerName) {
        return hasEntry(playerName);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// ItemInWorldManager — Per-player block interaction state machine.
// Java: net.minecraft.server.management.ItemInWorldManager (240 lines)
//
// Block breaking state machine:
//   1. onBlockClicked: start breaking (or instant break in creative)
//   2. updateBlockRemoving: tick-based progress (called every tick)
//   3. blockRemoving: finish break attempt (dig packet with STOP status)
//   4. cancelDestroyingBlock: abort
//
// Progress tracking:
//   curblockDamage: global tick counter
//   initialDamage: tick when breaking started
//   durabilityRemainingOnBlock: cached progress (0-10 for crack stage)
//   receivedFinishDiggingPacket: delayed harvest flag
//
// Game mode interactions:
//   Creative: instant break (no sword on blocks)
//   Adventure: requires CanDestroy tag on tool
//   Survival: normal breaking with progress
//
// Block interaction:
//   activateBlockOrUseItem: right-click on block
//     Priority: block.onBlockActivated first (unless sneaking with item)
//     Then: itemStack.tryPlaceItemIntoWorld
//     Creative: preserves stack size and metadata
//   tryUseItem: right-click with item (no block target)
// ═══════════════════════════════════════════════════════════════════════════

class ItemInWorldManager {
public:
    // ─── Game mode ───
    enum class GameType : int32_t {
        NOT_SET = -1,
        SURVIVAL = 0,
        CREATIVE = 1,
        ADVENTURE = 2,
        SPECTATOR = 3  // Not in 1.7.10 but included for forward compat
    };

    GameType gameType = GameType::NOT_SET;

    // ─── Block breaking state ───
    bool isDestroyingBlock = false;
    int32_t initialDamage = 0;
    int32_t curBlockX = 0, curBlockY = 0, curBlockZ = 0;
    int32_t curblockDamage = 0;  // Global tick counter for this player

    // Delayed harvest state
    bool receivedFinishDiggingPacket = false;
    int32_t posX = 0, posY = 0, posZ = 0;
    int32_t initialBlockDamage = 0;

    int32_t durabilityRemainingOnBlock = -1;

    // ─── Accessors ───
    bool isCreative() const { return gameType == GameType::CREATIVE; }

    bool isAdventure() const { return gameType == GameType::ADVENTURE; }

    bool isSurvival() const { return gameType == GameType::SURVIVAL; }

    // ═══════════════════════════════════════════════════════════════
    // updateBlockRemoving — Tick the block breaking state machine.
    // Java: ItemInWorldManager.updateBlockRemoving() (lines 59-94)
    //
    // Called every tick on the player's thread.
    // Handles both in-progress breaking and delayed harvest.
    //
    // Progress formula:
    //   relativeHardness = block.getPlayerRelativeBlockHardness(player)
    //   totalProgress = relativeHardness * (ticksElapsed + 1)
    //   crackStage = (int)(totalProgress * 10.0f)
    //   if (totalProgress >= 1.0f) → harvest block
    // ═══════════════════════════════════════════════════════════════

    struct BreakTickResult {
        bool shouldHarvest = false;
        int32_t harvestX = 0, harvestY = 0, harvestZ = 0;
        int32_t crackStage = -1;  // 0-10, or -1 for no update
        int32_t crackX = 0, crackY = 0, crackZ = 0;
        bool stoppedBreaking = false;
    };

    BreakTickResult updateBlockRemoving(float blockHardness, bool blockIsAir,
                                         float delayedBlockHardness,
                                         bool delayedBlockIsAir)
    {
        BreakTickResult result;
        ++curblockDamage;

        if (receivedFinishDiggingPacket) {
            if (delayedBlockIsAir) {
                receivedFinishDiggingPacket = false;
            } else {
                int32_t elapsed = curblockDamage - initialBlockDamage;
                float progress = delayedBlockHardness *
                                 static_cast<float>(elapsed + 1);
                auto crack = static_cast<int32_t>(progress * 10.0f);

                if (crack != durabilityRemainingOnBlock) {
                    result.crackStage = crack;
                    result.crackX = posX;
                    result.crackY = posY;
                    result.crackZ = posZ;
                    durabilityRemainingOnBlock = crack;
                }

                if (progress >= 1.0f) {
                    receivedFinishDiggingPacket = false;
                    result.shouldHarvest = true;
                    result.harvestX = posX;
                    result.harvestY = posY;
                    result.harvestZ = posZ;
                }
            }
        } else if (isDestroyingBlock) {
            if (blockIsAir) {
                result.crackStage = -1;
                result.crackX = curBlockX;
                result.crackY = curBlockY;
                result.crackZ = curBlockZ;
                durabilityRemainingOnBlock = -1;
                isDestroyingBlock = false;
                result.stoppedBreaking = true;
            } else {
                int32_t elapsed = curblockDamage - initialDamage;
                float progress = blockHardness *
                                 static_cast<float>(elapsed + 1);
                auto crack = static_cast<int32_t>(progress * 10.0f);

                if (crack != durabilityRemainingOnBlock) {
                    result.crackStage = crack;
                    result.crackX = curBlockX;
                    result.crackY = curBlockY;
                    result.crackZ = curBlockZ;
                    durabilityRemainingOnBlock = crack;
                }
            }
        }

        return result;
    }

    // ═══════════════════════════════════════════════════════════════
    // onBlockClicked — Player starts breaking a block.
    // Java: ItemInWorldManager.onBlockClicked(x, y, z, side) (lines 96-125)
    //
    // Creative: instant break (unless sword on block)
    // Adventure: check CanDestroy tag
    // Survival: start progress tracking
    //
    // Returns: true if block should be instantly harvested
    // ═══════════════════════════════════════════════════════════════

    struct BlockClickResult {
        bool instantHarvest = false;
        bool extinguishFire = true;
        bool startedBreaking = false;
        int32_t crackStage = -1;
    };

    BlockClickResult onBlockClicked(int32_t x, int32_t y, int32_t z,
                                     float relativeHardness,
                                     bool blockIsAir,
                                     bool isAdventureExempt)
    {
        BlockClickResult result;

        if (isAdventure() && !isAdventureExempt) return result;

        if (isCreative()) {
            result.instantHarvest = true;
            return result;
        }

        // Survival mode
        initialDamage = curblockDamage;

        if (!blockIsAir && relativeHardness >= 1.0f) {
            // Instant break (e.g., tall grass with shears)
            result.instantHarvest = true;
        } else {
            isDestroyingBlock = true;
            curBlockX = x;
            curBlockY = y;
            curBlockZ = z;
            auto crack = static_cast<int32_t>(relativeHardness * 10.0f);
            durabilityRemainingOnBlock = crack;
            result.crackStage = crack;
            result.startedBreaking = true;
        }

        return result;
    }

    // ═══════════════════════════════════════════════════════════════
    // blockRemoving — Player finishes digging (STOP_DESTROY_BLOCK).
    // Java: ItemInWorldManager.blockRemoving(x, y, z) (lines 127-147)
    //
    // Must match current breaking position.
    // If progress >= 0.7f: instant harvest.
    // If < 0.7f: enter delayed harvest state (receivedFinishDiggingPacket).
    // ═══════════════════════════════════════════════════════════════

    struct BlockRemoveResult {
        bool shouldHarvest = false;
        bool delayedHarvest = false;
    };

    BlockRemoveResult blockRemoving(int32_t x, int32_t y, int32_t z,
                                     float relativeHardness, bool blockIsAir)
    {
        BlockRemoveResult result;

        if (x != curBlockX || y != curBlockY || z != curBlockZ) return result;

        if (!blockIsAir) {
            int32_t elapsed = curblockDamage - initialDamage;
            float progress = relativeHardness *
                             static_cast<float>(elapsed + 1);

            if (progress >= 0.7f) {
                isDestroyingBlock = false;
                durabilityRemainingOnBlock = -1;
                result.shouldHarvest = true;
            } else if (!receivedFinishDiggingPacket) {
                isDestroyingBlock = false;
                receivedFinishDiggingPacket = true;
                posX = x;
                posY = y;
                posZ = z;
                initialBlockDamage = initialDamage;
                result.delayedHarvest = true;
            }
        }

        return result;
    }

    // ═══════════════════════════════════════════════════════════════
    // cancelDestroyingBlock — Abort current block breaking.
    // Java: ItemInWorldManager.cancelDestroyingBlock(x, y, z) (line 149-152)
    // ═══════════════════════════════════════════════════════════════

    void cancelDestroyingBlock() {
        isDestroyingBlock = false;
        durabilityRemainingOnBlock = -1;
    }

    // ═══════════════════════════════════════════════════════════════
    // Block activation priority (right-click):
    // Java: ItemInWorldManager.activateBlockOrUseItem (lines 217-233)
    //
    //   1. If (!sneaking || no held item): try block.onBlockActivated
    //   2. If no block activation: try itemStack.tryPlaceItemIntoWorld
    //   3. Creative mode: preserve stack size and metadata
    // ═══════════════════════════════════════════════════════════════

    struct ActivationPriority {
        static constexpr int32_t BLOCK_FIRST = 0;  // !sneaking || no item
        static constexpr int32_t ITEM_ONLY = 1;     // sneaking with item
    };

    int32_t getActivationPriority(bool isSneaking, bool hasHeldItem) const {
        if (!isSneaking || !hasHeldItem) {
            return ActivationPriority::BLOCK_FIRST;
        }
        return ActivationPriority::ITEM_ONLY;
    }
};

} // namespace mccpp
