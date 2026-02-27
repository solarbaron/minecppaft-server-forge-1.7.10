#pragma once
// AchievementManager — achievement tracking and statistics.
// Ported from vanilla 1.7.10 StatBase (pu.java), Achievement (pw.java),
// AchievementList (px.java), StatisticsFile (qf.java).
//
// Protocol 5 packets:
//   S→C 0x37: Statistics (count, entries of statName + value)
//   S→C 0x3A: Map Data (used also for achievement popups in 1.7)
//
// 1.7.10 achievements are tracked per-player as stat counters.

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "networking/PacketBuffer.h"

namespace mc {

// ============================================================
// S→C 0x37 Statistics — iz.java
// ============================================================
struct StatisticsPacket {
    struct Entry {
        std::string statName;
        int32_t     value;
    };
    std::vector<Entry> entries;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x37);
        buf.writeVarInt(static_cast<int32_t>(entries.size()));
        for (auto& e : entries) {
            buf.writeString(e.statName);
            buf.writeVarInt(e.value);
        }
        return buf;
    }
};

// ============================================================
// Achievement definitions — px.java (AchievementList)
// ============================================================
struct AchievementDef {
    std::string id;          // e.g. "achievement.openInventory"
    std::string name;        // Display name
    std::string description;
    std::string parentId;    // Empty if root achievement
    int16_t     iconItemId;
    int16_t     iconDamage;
};

// All vanilla 1.7.10 achievements
inline const std::vector<AchievementDef>& getAchievements() {
    static const std::vector<AchievementDef> achievements = {
        {"achievement.openInventory",    "Taking Inventory",     "Open your inventory",                   "",                              340, 0},
        {"achievement.mineWood",         "Getting Wood",         "Attack a tree until wood pops out",     "achievement.openInventory",      17, 0},
        {"achievement.buildWorkBench",   "Benchmarking",         "Craft a workbench with 4 planks",       "achievement.mineWood",           58, 0},
        {"achievement.buildPickaxe",     "Time to Mine!",        "Use planks and sticks to make a pickaxe","achievement.buildWorkBench",    270, 0},
        {"achievement.buildFurnace",     "Hot Topic",            "Construct a furnace from 8 cobblestone","achievement.buildPickaxe",       61, 0},
        {"achievement.acquireIron",      "Acquire Hardware",     "Smelt an iron ingot",                   "achievement.buildFurnace",      265, 0},
        {"achievement.buildHoe",         "Time to Farm!",        "Use planks and sticks to make a hoe",   "achievement.buildWorkBench",    290, 0},
        {"achievement.makeBread",        "Bake Bread",           "Turn wheat into bread",                 "achievement.buildHoe",          297, 0},
        {"achievement.bakeCake",         "The Lie",              "Wheat, sugar, milk and eggs!",          "achievement.buildHoe",          354, 0},
        {"achievement.buildBetterPickaxe","Getting an Upgrade",  "Construct a better pickaxe",            "achievement.buildPickaxe",      274, 0},
        {"achievement.cookFish",         "Delicious Fish",       "Catch and cook a fish",                 "achievement.buildFurnace",      350, 0},
        {"achievement.onARail",          "On A Rail",            "Travel by minecart 1km from start",     "achievement.acquireIron",       328, 0},
        {"achievement.buildSword",       "Time to Strike!",      "Use planks and sticks to make a sword", "achievement.buildWorkBench",    268, 0},
        {"achievement.killEnemy",        "Monster Hunter",       "Attack and destroy a monster",          "achievement.buildSword",        367, 0},
        {"achievement.killCow",          "Cow Tipper",           "Harvest some leather",                  "achievement.buildSword",        334, 0},
        {"achievement.flyPig",           "When Pigs Fly",        "Fly a pig off a cliff",                 "achievement.killCow",           329, 0},
        {"achievement.snipeSkeleton",    "Sniper Duel",          "Kill a skeleton from 50+ meters",       "achievement.killEnemy",         261, 0},
        {"achievement.diamonds",         "DIAMONDS!",            "Acquire diamonds with iron tools",      "achievement.acquireIron",       264, 0},
        {"achievement.portal",           "We Need to Go Deeper", "Build a Nether portal",                "achievement.diamonds",           49, 0},
        {"achievement.ghast",            "Return to Sender",     "Destroy a Ghast with a fireball",       "achievement.portal",           370, 0},
        {"achievement.blazeRod",         "Into Fire",            "Relieve a Blaze of its rod",            "achievement.portal",           369, 0},
        {"achievement.potion",           "Local Brewery",        "Brew a potion",                         "achievement.blazeRod",         373, 0},
        {"achievement.theEnd",           "The End?",             "Locate the End",                        "achievement.blazeRod",         381, 0},
        {"achievement.theEnd2",          "The End.",             "Defeat the Ender Dragon",               "achievement.theEnd",           122, 0},
        {"achievement.enchantments",     "Enchanter",            "Enchant an item at an Ench. Table",     "achievement.diamonds",         116, 0},
        {"achievement.overkill",         "Overkill",             "Deal 9 hearts in a single hit",         "achievement.enchantments",     276, 0},
        {"achievement.bookcase",         "Librarian",            "Build some bookshelves",                "achievement.enchantments",      47, 0},
    };
    return achievements;
}

// ============================================================
// Statistics definitions — common stat names
// ============================================================
namespace StatId {
    inline const std::string LEAVE_GAME       = "stat.leaveGame";
    inline const std::string PLAY_ONE_MINUTE  = "stat.playOneMinute";
    inline const std::string WALK_ONE_CM      = "stat.walkOneCm";
    inline const std::string SWIM_ONE_CM      = "stat.swimOneCm";
    inline const std::string FALL_ONE_CM      = "stat.fallOneCm";
    inline const std::string CLIMB_ONE_CM     = "stat.climbOneCm";
    inline const std::string FLY_ONE_CM       = "stat.flyOneCm";
    inline const std::string DIVE_ONE_CM      = "stat.diveOneCm";
    inline const std::string MINECART_ONE_CM  = "stat.minecartOneCm";
    inline const std::string BOAT_ONE_CM      = "stat.boatOneCm";
    inline const std::string PIG_ONE_CM       = "stat.pigOneCm";
    inline const std::string HORSE_ONE_CM     = "stat.horseOneCm";
    inline const std::string JUMP              = "stat.jump";
    inline const std::string DROP              = "stat.drop";
    inline const std::string DAMAGE_DEALT     = "stat.damageDealt";
    inline const std::string DAMAGE_TAKEN     = "stat.damageTaken";
    inline const std::string DEATHS           = "stat.deaths";
    inline const std::string MOB_KILLS        = "stat.mobKills";
    inline const std::string PLAYER_KILLS     = "stat.playerKills";
    inline const std::string FISH_CAUGHT      = "stat.fishCaught";
    inline const std::string ANIMALS_BRED     = "stat.animalsBred";
    inline const std::string TREASURE_FISHED  = "stat.treasureFished";
    inline const std::string JUNK_FISHED      = "stat.junkFished";

    // Per-block/item stats: stat.mineBlock.X, stat.useItem.X, stat.breakItem.X, stat.craftItem.X
    inline std::string mineBlock(int blockId) { return "stat.mineBlock." + std::to_string(blockId); }
    inline std::string useItem(int itemId)    { return "stat.useItem." + std::to_string(itemId); }
    inline std::string breakItem(int itemId)  { return "stat.breakItem." + std::to_string(itemId); }
    inline std::string craftItem(int itemId)  { return "stat.craftItem." + std::to_string(itemId); }
    inline std::string killEntity(const std::string& mob) { return "stat.killEntity." + mob; }
    inline std::string killedBy(const std::string& mob)   { return "stat.entityKilledBy." + mob; }
}

// ============================================================
// Per-player achievement and statistics tracker
// ============================================================
class PlayerStats {
public:
    // Increment a statistic
    void addStat(const std::string& statId, int32_t amount = 1) {
        stats_[statId] += amount;
        dirty_.insert(statId);
    }

    // Set a statistic
    void setStat(const std::string& statId, int32_t value) {
        stats_[statId] = value;
        dirty_.insert(statId);
    }

    // Get stat value
    int32_t getStat(const std::string& statId) const {
        auto it = stats_.find(statId);
        return it != stats_.end() ? it->second : 0;
    }

    // Grant an achievement
    bool grantAchievement(const std::string& achievementId) {
        if (achievements_.count(achievementId)) return false;

        // Check parent requirement
        for (auto& ach : getAchievements()) {
            if (ach.id == achievementId) {
                if (!ach.parentId.empty() && !achievements_.count(ach.parentId)) {
                    return false; // Parent not unlocked
                }
                break;
            }
        }

        achievements_.insert(achievementId);
        addStat(achievementId, 1);
        return true;
    }

    // Check if player has an achievement
    bool hasAchievement(const std::string& achievementId) const {
        return achievements_.count(achievementId) > 0;
    }

    // Get dirty stats for sync and clear
    StatisticsPacket getDirtyPacket() {
        StatisticsPacket pkt;
        for (auto& statId : dirty_) {
            pkt.entries.push_back({statId, stats_[statId]});
        }
        dirty_.clear();
        return pkt;
    }

    // Get full stats packet
    StatisticsPacket getFullPacket() const {
        StatisticsPacket pkt;
        for (auto& [id, val] : stats_) {
            pkt.entries.push_back({id, val});
        }
        return pkt;
    }

    bool isDirty() const { return !dirty_.empty(); }

    // Serialize to map for NBT persistence
    const std::unordered_map<std::string, int32_t>& allStats() const { return stats_; }
    const std::unordered_set<std::string>& allAchievements() const { return achievements_; }

    // Load from saved data
    void loadStats(const std::unordered_map<std::string, int32_t>& saved) {
        stats_ = saved;
        for (auto& [id, val] : saved) {
            // Detect achievements
            if (id.rfind("achievement.", 0) == 0 && val > 0) {
                achievements_.insert(id);
            }
        }
    }

private:
    std::unordered_map<std::string, int32_t> stats_;
    std::unordered_set<std::string> achievements_;
    std::unordered_set<std::string> dirty_;
};

// ============================================================
// AchievementManager — manages all players' stats
// ============================================================
class AchievementManager {
public:
    PlayerStats& getPlayerStats(int entityId) {
        return playerStats_[entityId];
    }

    void removePlayer(int entityId) {
        playerStats_.erase(entityId);
    }

    // Track common events
    void onBlockMined(int entityId, int blockId) {
        auto& stats = getPlayerStats(entityId);
        stats.addStat(StatId::mineBlock(blockId));

        // Achievement: Getting Wood
        if (blockId == 17) { // OAK_LOG
            stats.grantAchievement("achievement.mineWood");
        }
    }

    void onItemCrafted(int entityId, int itemId) {
        auto& stats = getPlayerStats(entityId);
        stats.addStat(StatId::craftItem(itemId));

        // Achievement checks
        switch (itemId) {
            case 58:  stats.grantAchievement("achievement.buildWorkBench"); break;
            case 270: stats.grantAchievement("achievement.buildPickaxe"); break;
            case 274: stats.grantAchievement("achievement.buildBetterPickaxe"); break;
            case 61:  stats.grantAchievement("achievement.buildFurnace"); break;
            case 290: stats.grantAchievement("achievement.buildHoe"); break;
            case 268: stats.grantAchievement("achievement.buildSword"); break;
            case 297: stats.grantAchievement("achievement.makeBread"); break;
            case 354: stats.grantAchievement("achievement.bakeCake"); break;
        }
    }

    void onItemSmelted(int entityId, int itemId) {
        auto& stats = getPlayerStats(entityId);
        if (itemId == 265) { // Iron Ingot
            stats.grantAchievement("achievement.acquireIron");
        }
        if (itemId == 350) { // Cooked Fish
            stats.grantAchievement("achievement.cookFish");
        }
    }

    void onMobKilled(int entityId, const std::string& mobType) {
        auto& stats = getPlayerStats(entityId);
        stats.addStat(StatId::MOB_KILLS);
        stats.addStat(StatId::killEntity(mobType));
        stats.grantAchievement("achievement.killEnemy");
    }

    void onDiamondMined(int entityId) {
        getPlayerStats(entityId).grantAchievement("achievement.diamonds");
    }

    void onEnchant(int entityId) {
        getPlayerStats(entityId).grantAchievement("achievement.enchantments");
    }

    void onOpenInventory(int entityId) {
        getPlayerStats(entityId).grantAchievement("achievement.openInventory");
    }

    void onFishCaught(int entityId) {
        getPlayerStats(entityId).addStat(StatId::FISH_CAUGHT);
    }

    void onPlayerDeath(int entityId) {
        getPlayerStats(entityId).addStat(StatId::DEATHS);
    }

    void onDamageDealt(int entityId, float damage) {
        getPlayerStats(entityId).addStat(StatId::DAMAGE_DEALT,
                                          static_cast<int32_t>(damage * 10.0f));
    }

    void onWalk(int entityId, double distanceCm) {
        getPlayerStats(entityId).addStat(StatId::WALK_ONE_CM,
                                          static_cast<int32_t>(distanceCm));
    }

    void onJump(int entityId) {
        getPlayerStats(entityId).addStat(StatId::JUMP);
    }

private:
    std::unordered_map<int, PlayerStats> playerStats_;
};

} // namespace mc
