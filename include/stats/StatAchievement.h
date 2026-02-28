/**
 * StatAchievement.h — Statistics and achievement system.
 *
 * Java references:
 *   - net.minecraft.stats.StatBase — Base stat definition
 *   - net.minecraft.stats.StatList — All tracked statistics
 *   - net.minecraft.stats.Achievement — Achievement with parent chain
 *   - net.minecraft.stats.AchievementList — All 33 vanilla achievements
 *
 * All 33 vanilla 1.7.10 achievements with exact parent chains, display
 * coordinates, and special flags from the decompiled source.
 *
 * Thread safety:
 *   - Registry is static/const after initialization.
 *   - Per-player stat tracking uses separate StatFileWriter (not here).
 *
 * JNI readiness: String-based stat IDs for easy JVM mapping.
 */
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// StatBase — Base statistic definition.
// Java reference: net.minecraft.stats.StatBase
// ═══════════════════════════════════════════════════════════════════════════

struct StatBase {
    std::string statId;      // e.g. "stat.leaveGame"
    std::string displayName; // Human-readable name
    bool isIndependent;      // Not dependent on other stats
    bool isAchievement;      // Is an Achievement subclass
};

// ═══════════════════════════════════════════════════════════════════════════
// Achievement — Achievement with parent chain and display info.
// Java reference: net.minecraft.stats.Achievement
//
// Each achievement has:
//   - statId: unique string ID
//   - displayName: short name for icon
//   - parentId: parent achievement (empty = root)
//   - displayCol/Row: position on achievement GUI
//   - iconItemId: item to show as icon
//   - isSpecial: challenge achievement (different frame)
// ═══════════════════════════════════════════════════════════════════════════

struct Achievement {
    std::string statId;
    std::string displayName;
    std::string parentId;   // Empty = root achievement
    int32_t displayCol;     // X position on achievement screen
    int32_t displayRow;     // Y position on achievement screen
    int32_t iconItemId;     // Item/block ID for icon
    int32_t iconMeta;       // Item metadata for icon
    bool isSpecial;         // Challenge achievement (gold frame)
    bool isIndependent;     // No parent required
};

// ═══════════════════════════════════════════════════════════════════════════
// General statistics — Tracked per-player counters.
// Java reference: net.minecraft.stats.StatList
// ═══════════════════════════════════════════════════════════════════════════

namespace Stats {
    // General stats
    inline const std::string LEAVE_GAME        = "stat.leaveGame";
    inline const std::string PLAY_ONE_MINUTE   = "stat.playOneMinute";
    inline const std::string WALK_ONE_CM       = "stat.walkOneCm";
    inline const std::string SWIM_ONE_CM       = "stat.swimOneCm";
    inline const std::string FALL_ONE_CM       = "stat.fallOneCm";
    inline const std::string CLIMB_ONE_CM      = "stat.climbOneCm";
    inline const std::string FLY_ONE_CM        = "stat.flyOneCm";
    inline const std::string DIVE_ONE_CM       = "stat.diveOneCm";
    inline const std::string MINECART_ONE_CM   = "stat.minecartOneCm";
    inline const std::string BOAT_ONE_CM       = "stat.boatOneCm";
    inline const std::string PIG_ONE_CM        = "stat.pigOneCm";
    inline const std::string HORSE_ONE_CM      = "stat.horseOneCm";
    inline const std::string JUMP              = "stat.jump";
    inline const std::string DROP              = "stat.drop";
    inline const std::string DAMAGE_DEALT      = "stat.damageDealt";
    inline const std::string DAMAGE_TAKEN      = "stat.damageTaken";
    inline const std::string DEATHS            = "stat.deaths";
    inline const std::string MOB_KILLS         = "stat.mobKills";
    inline const std::string ANIMALS_BRED      = "stat.animalsBred";
    inline const std::string PLAYER_KILLS      = "stat.playerKills";
    inline const std::string FISH_CAUGHT       = "stat.fishCaught";
    inline const std::string JUNK_FISHED       = "stat.junkFished";
    inline const std::string TREASURE_FISHED   = "stat.treasureFished";
}

// ═══════════════════════════════════════════════════════════════════════════
// AchievementRegistry — Static registry of all vanilla achievements.
// Java reference: net.minecraft.stats.AchievementList static initializer
// ═══════════════════════════════════════════════════════════════════════════

class AchievementRegistry {
public:
    static void init();
    static const Achievement* getById(const std::string& id);
    static const std::vector<Achievement>& getAll();
    static int32_t getCount();

    // Achievement IDs — matching Java's AchievementList static fields
    static constexpr const char* OPEN_INVENTORY     = "achievement.openInventory";
    static constexpr const char* MINE_WOOD          = "achievement.mineWood";
    static constexpr const char* BUILD_WORKBENCH    = "achievement.buildWorkBench";
    static constexpr const char* BUILD_PICKAXE      = "achievement.buildPickaxe";
    static constexpr const char* BUILD_FURNACE      = "achievement.buildFurnace";
    static constexpr const char* ACQUIRE_IRON       = "achievement.acquireIron";
    static constexpr const char* BUILD_HOE          = "achievement.buildHoe";
    static constexpr const char* MAKE_BREAD         = "achievement.makeBread";
    static constexpr const char* BAKE_CAKE          = "achievement.bakeCake";
    static constexpr const char* BUILD_BETTER_PICK  = "achievement.buildBetterPickaxe";
    static constexpr const char* COOK_FISH          = "achievement.cookFish";
    static constexpr const char* ON_A_RAIL          = "achievement.onARail";
    static constexpr const char* BUILD_SWORD        = "achievement.buildSword";
    static constexpr const char* KILL_ENEMY         = "achievement.killEnemy";
    static constexpr const char* KILL_COW           = "achievement.killCow";
    static constexpr const char* FLY_PIG            = "achievement.flyPig";
    static constexpr const char* SNIPE_SKELETON     = "achievement.snipeSkeleton";
    static constexpr const char* DIAMONDS           = "achievement.diamonds";
    static constexpr const char* DIAMONDS_TO_YOU    = "achievement.diamondsToYou";
    static constexpr const char* PORTAL             = "achievement.portal";
    static constexpr const char* GHAST              = "achievement.ghast";
    static constexpr const char* BLAZE_ROD          = "achievement.blazeRod";
    static constexpr const char* POTION             = "achievement.potion";
    static constexpr const char* THE_END            = "achievement.theEnd";
    static constexpr const char* THE_END2           = "achievement.theEnd2";
    static constexpr const char* ENCHANTMENTS       = "achievement.enchantments";
    static constexpr const char* OVERKILL           = "achievement.overkill";
    static constexpr const char* BOOKCASE           = "achievement.bookcase";
    static constexpr const char* BREED_COW          = "achievement.breedCow";
    static constexpr const char* SPAWN_WITHER       = "achievement.spawnWither";
    static constexpr const char* KILL_WITHER        = "achievement.killWither";
    static constexpr const char* FULL_BEACON        = "achievement.fullBeacon";
    static constexpr const char* EXPLORE_ALL_BIOMES = "achievement.exploreAllBiomes";

private:
    static std::vector<Achievement> achievements_;
    static bool initialized_;
};

// ═══════════════════════════════════════════════════════════════════════════
// StatTracker — Per-player stat tracking.
// Java reference: net.minecraft.stats.StatFileWriter (simplified)
//
// Thread safety: per-player instance, no cross-thread sharing needed.
// ═══════════════════════════════════════════════════════════════════════════

class StatTracker {
public:
    // Increment a stat counter
    void addStat(const std::string& statId, int32_t amount = 1);

    // Get stat value
    int32_t getStat(const std::string& statId) const;

    // Check if achievement is unlocked
    bool hasAchievement(const std::string& achievementId) const;

    // Unlock an achievement (checks parent chain)
    bool unlockAchievement(const std::string& achievementId);

    // Get all stats
    const std::unordered_map<std::string, int32_t>& getAllStats() const {
        return stats_;
    }

    // Get all unlocked achievements
    const std::vector<std::string>& getUnlockedAchievements() const {
        return unlockedAchievements_;
    }

private:
    std::unordered_map<std::string, int32_t> stats_;
    std::vector<std::string> unlockedAchievements_;
};

} // namespace mccpp
