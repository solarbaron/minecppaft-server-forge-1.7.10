/**
 * StatList.h — Complete statistics and achievement registry.
 *
 * Java references:
 *   - net.minecraft.stats.StatList (171 lines)
 *   - net.minecraft.stats.StatBase (133 lines)
 *   - net.minecraft.stats.StatBasic (15 lines)
 *   - net.minecraft.stats.StatCrafting (22 lines)
 *   - net.minecraft.stats.AchievementList (95 lines)
 *   - net.minecraft.stats.Achievement (95 lines)
 *   - net.minecraft.stats.StatisticsFile (178 lines)
 *
 * Statistics architecture:
 *   - 23 general stats (leaveGame through treasureFished)
 *   - mineBlockStatArray[4096]: one per block ID
 *   - objectCraftStats[32000]: craftable items from recipe/smelting registry
 *   - objectUseStats[32000]: all items
 *   - objectBreakStats[32000]: only damageable items
 *   - Per-entity: killEntity.X and entityKilledBy.X
 *   - Block alias merging: 14 pairs (water/flowing_water, lit/unlit, etc.)
 *   - 33 achievements in dependency tree
 *
 * Per-player statistics: StatisticsFile stores int32_t per stat key string
 *
 * Thread safety: StatList is static registry (read-only after init).
 *   StatisticsFile is per-player, accessed on player's thread.
 * JNI readiness: String keys match Java stat IDs exactly.
 */
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// StatBase — Single statistic with string ID.
// Java: net.minecraft.stats.StatBase (133 lines)
//
//   statId: unique key (e.g., "stat.leaveGame", "stat.mineBlock.4")
//   isIndependent: true for general stats (not tied to items)
//   isAchievement: true for achievement stats
// ═══════════════════════════════════════════════════════════════════════════

struct StatBase {
    std::string statId;
    bool isIndependent = false;
    bool isAchievement = false;

    StatBase() = default;
    explicit StatBase(const std::string& id, bool independent = false)
        : statId(id), isIndependent(independent) {}
};

// ═══════════════════════════════════════════════════════════════════════════
// Achievement — Achievement with tree dependency and display position.
// Java: net.minecraft.stats.Achievement (95 lines)
//
//   parent: prerequisite achievement (null for root)
//   displayCol, displayRow: position in achievement GUI
//   isSpecial: highlighted in gold (challenge achievements)
//   itemId/meta: icon item
// ═══════════════════════════════════════════════════════════════════════════

struct Achievement {
    std::string statId;
    std::string shortName;
    int32_t displayCol = 0;
    int32_t displayRow = 0;
    int32_t iconItemId = 0;
    int32_t iconItemMeta = 0;
    std::string parentId;  // empty = root achievement
    bool isSpecial = false;

    Achievement() = default;
    Achievement(const std::string& id, const std::string& name,
                int32_t col, int32_t row, int32_t itemId,
                const std::string& parent = "", bool special = false)
        : statId(id), shortName(name), displayCol(col), displayRow(row),
          iconItemId(itemId), parentId(parent), isSpecial(special) {}
};

// ═══════════════════════════════════════════════════════════════════════════
// StatList — Complete statistics registry (all vanilla 1.7.10 stats).
// Java: net.minecraft.stats.StatList (171 lines)
// ═══════════════════════════════════════════════════════════════════════════

class StatList {
public:
    // ─── General statistics (23 total) ───
    // Java: StatList lines 34-56

    static constexpr const char* LEAVE_GAME = "stat.leaveGame";
    static constexpr const char* PLAY_ONE_MINUTE = "stat.playOneMinute";
    static constexpr const char* WALK_ONE_CM = "stat.walkOneCm";
    static constexpr const char* SWIM_ONE_CM = "stat.swimOneCm";
    static constexpr const char* FALL_ONE_CM = "stat.fallOneCm";
    static constexpr const char* CLIMB_ONE_CM = "stat.climbOneCm";
    static constexpr const char* FLY_ONE_CM = "stat.flyOneCm";
    static constexpr const char* DIVE_ONE_CM = "stat.diveOneCm";
    static constexpr const char* MINECART_ONE_CM = "stat.minecartOneCm";
    static constexpr const char* BOAT_ONE_CM = "stat.boatOneCm";
    static constexpr const char* PIG_ONE_CM = "stat.pigOneCm";
    static constexpr const char* HORSE_ONE_CM = "stat.horseOneCm";
    static constexpr const char* JUMP = "stat.jump";
    static constexpr const char* DROP = "stat.drop";
    static constexpr const char* DAMAGE_DEALT = "stat.damageDealt";
    static constexpr const char* DAMAGE_TAKEN = "stat.damageTaken";
    static constexpr const char* DEATHS = "stat.deaths";
    static constexpr const char* MOB_KILLS = "stat.mobKills";
    static constexpr const char* ANIMALS_BRED = "stat.animalsBred";
    static constexpr const char* PLAYER_KILLS = "stat.playerKills";
    static constexpr const char* FISH_CAUGHT = "stat.fishCaught";
    static constexpr const char* JUNK_FISHED = "stat.junkFished";
    static constexpr const char* TREASURE_FISHED = "stat.treasureFished";

    // All general stat IDs for iteration
    static const std::vector<const char*>& getGeneralStats() {
        static const std::vector<const char*> stats = {
            LEAVE_GAME, PLAY_ONE_MINUTE, WALK_ONE_CM, SWIM_ONE_CM,
            FALL_ONE_CM, CLIMB_ONE_CM, FLY_ONE_CM, DIVE_ONE_CM,
            MINECART_ONE_CM, BOAT_ONE_CM, PIG_ONE_CM, HORSE_ONE_CM,
            JUMP, DROP, DAMAGE_DEALT, DAMAGE_TAKEN, DEATHS,
            MOB_KILLS, ANIMALS_BRED, PLAYER_KILLS, FISH_CAUGHT,
            JUNK_FISHED, TREASURE_FISHED
        };
        return stats;
    }

    // ─── Per-item stat key format ───
    // Java: "stat.mineBlock." + blockId, "stat.craftItem." + itemId, etc.
    static std::string mineBlockStat(int32_t blockId) {
        return "stat.mineBlock." + std::to_string(blockId);
    }
    static std::string craftItemStat(int32_t itemId) {
        return "stat.craftItem." + std::to_string(itemId);
    }
    static std::string useItemStat(int32_t itemId) {
        return "stat.useItem." + std::to_string(itemId);
    }
    static std::string breakItemStat(int32_t itemId) {
        return "stat.breakItem." + std::to_string(itemId);
    }

    // ─── Per-entity stat key format ───
    // Java: "stat.killEntity." + entityName, "stat.entityKilledBy." + entityName
    static std::string killEntityStat(const std::string& entityName) {
        return "stat.killEntity." + entityName;
    }
    static std::string entityKilledByStat(const std::string& entityName) {
        return "stat.entityKilledBy." + entityName;
    }

    // ─── Block alias pairs for stat merging ───
    // Java: StatList.replaceAllSimilarBlocks (lines 120-135)
    // When blocks have lit/unlit variants, stats merge to one entry
    struct BlockAlias {
        int32_t fromBlockId;
        int32_t toBlockId;
    };

    static const std::vector<BlockAlias>& getBlockAliases() {
        static const std::vector<BlockAlias> aliases = {
            {9, 8},     // flowing_water → water
            {11, 10},   // flowing_lava → lava
            {91, 86},   // lit_pumpkin → pumpkin
            {62, 61},   // lit_furnace → furnace
            {74, 73},   // lit_redstone_ore → redstone_ore
            {94, 93},   // powered_repeater → unpowered_repeater
            {150, 149}, // powered_comparator → unpowered_comparator
            {76, 75},   // redstone_torch → unlit_redstone_torch
            {124, 123}, // lit_redstone_lamp → redstone_lamp
            {40, 39},   // red_mushroom → brown_mushroom
            {43, 44},   // double_stone_slab → stone_slab
            {125, 126}, // double_wooden_slab → wooden_slab
            {2, 3},     // grass → dirt
            {60, 3},    // farmland → dirt
        };
        return aliases;
    }

    // ─── Array sizes ───
    static constexpr int32_t MINE_BLOCK_ARRAY_SIZE = 4096;
    static constexpr int32_t OBJECT_STAT_ARRAY_SIZE = 32000;
};

// ═══════════════════════════════════════════════════════════════════════════
// AchievementList — All 33 vanilla 1.7.10 achievements.
// Java: net.minecraft.stats.AchievementList (95 lines)
//
// Achievement tree:
//   openInventory (root) → mineWood → buildWorkBench →
//     ├─ buildPickaxe → buildFurnace → acquireIron →
//     │   ├─ onARail (special)
//     │   └─ diamonds → diamondsToYou
//     │       ├─ portal → ghast (special), blazeRod →
//     │       │   ├─ potion
//     │       │   └─ theEnd (special) → theEnd2 (special)
//     │       │       └─ spawnWither → killWither → fullBeacon (special)
//     │       │       └─ exploreAllBiomes (special)
//     │       └─ enchantments → overkill (special), bookcase
//     ├─ buildBetterPickaxe
//     ├─ cookFish
//     ├─ buildHoe → makeBread, bakeCake
//     └─ buildSword → killEnemy → snipeSkeleton (special)
//         └─ killCow → flyPig (special), breedCow
// ═══════════════════════════════════════════════════════════════════════════

class AchievementList {
public:
    // Achievement IDs (matching Java exactly)
    static constexpr const char* OPEN_INVENTORY = "achievement.openInventory";
    static constexpr const char* MINE_WOOD = "achievement.mineWood";
    static constexpr const char* BUILD_WORK_BENCH = "achievement.buildWorkBench";
    static constexpr const char* BUILD_PICKAXE = "achievement.buildPickaxe";
    static constexpr const char* BUILD_FURNACE = "achievement.buildFurnace";
    static constexpr const char* ACQUIRE_IRON = "achievement.acquireIron";
    static constexpr const char* BUILD_HOE = "achievement.buildHoe";
    static constexpr const char* MAKE_BREAD = "achievement.makeBread";
    static constexpr const char* BAKE_CAKE = "achievement.bakeCake";
    static constexpr const char* BUILD_BETTER_PICKAXE = "achievement.buildBetterPickaxe";
    static constexpr const char* COOK_FISH = "achievement.cookFish";
    static constexpr const char* ON_A_RAIL = "achievement.onARail";
    static constexpr const char* BUILD_SWORD = "achievement.buildSword";
    static constexpr const char* KILL_ENEMY = "achievement.killEnemy";
    static constexpr const char* KILL_COW = "achievement.killCow";
    static constexpr const char* FLY_PIG = "achievement.flyPig";
    static constexpr const char* SNIPE_SKELETON = "achievement.snipeSkeleton";
    static constexpr const char* DIAMONDS = "achievement.diamonds";
    static constexpr const char* DIAMONDS_TO_YOU = "achievement.diamondsToYou";
    static constexpr const char* PORTAL = "achievement.portal";
    static constexpr const char* GHAST = "achievement.ghast";
    static constexpr const char* BLAZE_ROD = "achievement.blazeRod";
    static constexpr const char* POTION = "achievement.potion";
    static constexpr const char* THE_END = "achievement.theEnd";
    static constexpr const char* THE_END_2 = "achievement.theEnd2";
    static constexpr const char* ENCHANTMENTS = "achievement.enchantments";
    static constexpr const char* OVERKILL = "achievement.overkill";
    static constexpr const char* BOOKCASE = "achievement.bookcase";
    static constexpr const char* BREED_COW = "achievement.breedCow";
    static constexpr const char* SPAWN_WITHER = "achievement.spawnWither";
    static constexpr const char* KILL_WITHER = "achievement.killWither";
    static constexpr const char* FULL_BEACON = "achievement.fullBeacon";
    static constexpr const char* EXPLORE_ALL_BIOMES = "achievement.exploreAllBiomes";

    // Get all 33 achievements with tree structure
    // Java: AchievementList static initializer (lines 57-91)
    //   (statId, shortName, col, row, iconItemId, parentId, isSpecial)
    static const std::vector<Achievement>& getAchievements() {
        static const std::vector<Achievement> achievements = {
            // Root
            {OPEN_INVENTORY, "openInventory", 0, 0, 340, ""},              // Items.book
            // Tree from mineWood
            {MINE_WOOD, "mineWood", 2, 1, 17, OPEN_INVENTORY},             // Blocks.log
            {BUILD_WORK_BENCH, "buildWorkBench", 4, -1, 58, MINE_WOOD},    // Blocks.crafting_table
            {BUILD_PICKAXE, "buildPickaxe", 4, 2, 270, BUILD_WORK_BENCH},  // Items.wooden_pickaxe
            {BUILD_FURNACE, "buildFurnace", 3, 4, 61, BUILD_PICKAXE},      // Blocks.furnace
            {ACQUIRE_IRON, "acquireIron", 1, 4, 265, BUILD_FURNACE},       // Items.iron_ingot
            // Farming branch
            {BUILD_HOE, "buildHoe", 2, -3, 290, BUILD_WORK_BENCH},         // Items.wooden_hoe
            {MAKE_BREAD, "makeBread", -1, -3, 297, BUILD_HOE},             // Items.bread
            {BAKE_CAKE, "bakeCake", 0, -5, 354, BUILD_HOE},                // Items.cake
            // Mining branch
            {BUILD_BETTER_PICKAXE, "buildBetterPickaxe", 6, 2, 274, BUILD_PICKAXE}, // Items.stone_pickaxe
            {COOK_FISH, "cookFish", 2, 6, 350, BUILD_FURNACE},             // Items.cooked_fish
            {ON_A_RAIL, "onARail", 2, 3, 66, ACQUIRE_IRON, true},          // Blocks.rail (special)
            // Combat branch
            {BUILD_SWORD, "buildSword", 6, -1, 268, BUILD_WORK_BENCH},     // Items.wooden_sword
            {KILL_ENEMY, "killEnemy", 8, -1, 352, BUILD_SWORD},            // Items.bone
            {KILL_COW, "killCow", 7, -3, 334, BUILD_SWORD},                // Items.leather
            {FLY_PIG, "flyPig", 9, -3, 329, KILL_COW, true},               // Items.saddle (special)
            {SNIPE_SKELETON, "snipeSkeleton", 7, 0, 261, KILL_ENEMY, true}, // Items.bow (special)
            // Diamond branch
            {DIAMONDS, "diamonds", -1, 5, 56, ACQUIRE_IRON},               // Blocks.diamond_ore
            {DIAMONDS_TO_YOU, "diamondsToYou", -1, 2, 264, DIAMONDS},      // Items.diamond
            // Nether branch
            {PORTAL, "portal", -1, 7, 49, DIAMONDS},                       // Blocks.obsidian
            {GHAST, "ghast", -4, 8, 370, PORTAL, true},                    // Items.ghast_tear (special)
            {BLAZE_ROD, "blazeRod", 0, 9, 369, PORTAL},                    // Items.blaze_rod
            {POTION, "potion", 2, 8, 373, BLAZE_ROD},                      // Items.potionitem
            // End branch
            {THE_END, "theEnd", 3, 10, 381, BLAZE_ROD, true},              // Items.ender_eye (special)
            {THE_END_2, "theEnd2", 4, 13, 122, THE_END, true},             // Blocks.dragon_egg (special)
            // Enchanting branch
            {ENCHANTMENTS, "enchantments", -4, 4, 116, DIAMONDS},          // Blocks.enchanting_table
            {OVERKILL, "overkill", -4, 1, 276, ENCHANTMENTS, true},        // Items.diamond_sword (special)
            {BOOKCASE, "bookcase", -3, 6, 47, ENCHANTMENTS},               // Blocks.bookshelf
            // Breeding
            {BREED_COW, "breedCow", 7, -5, 296, KILL_COW},                 // Items.wheat
            // Wither branch
            {SPAWN_WITHER, "spawnWither", 7, 12, 397, THE_END_2},          // Items.skull (meta 1)
            {KILL_WITHER, "killWither", 7, 10, 399, SPAWN_WITHER},         // Items.nether_star
            {FULL_BEACON, "fullBeacon", 7, 8, 138, KILL_WITHER, true},     // Blocks.beacon (special)
            // Exploration
            {EXPLORE_ALL_BIOMES, "exploreAllBiomes", 4, 8, 313, THE_END, true}, // Items.diamond_boots (special)
        };
        return achievements;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// StatisticsFile — Per-player statistics storage.
// Java: net.minecraft.stats.StatisticsFile (178 lines)
//
//   Stores int32_t values per stat key string.
//   Saved as JSON: { "stat.walkOneCm": 12345, ... }
//   Achievement progress stored separately.
//
//   threadsafe: per-player, only accessed on that player's thread
// ═══════════════════════════════════════════════════════════════════════════

class StatisticsFile {
public:
    // ─── Read/write stat values ───
    int32_t getStat(const std::string& statId) const {
        auto it = stats_.find(statId);
        return it != stats_.end() ? it->second : 0;
    }

    void setStat(const std::string& statId, int32_t value) {
        stats_[statId] = value;
        dirty_ = true;
    }

    void increaseStat(const std::string& statId, int32_t amount) {
        stats_[statId] += amount;
        dirty_ = true;
    }

    // ─── Achievement tracking ───
    bool hasAchievement(const std::string& achievementId) const {
        auto it = achievements_.find(achievementId);
        return it != achievements_.end() && it->second;
    }

    void unlockAchievement(const std::string& achievementId) {
        achievements_[achievementId] = true;
        dirty_ = true;
    }

    // ─── Biome exploration tracking (for exploreAllBiomes) ───
    void addExploredBiome(int32_t biomeId) {
        exploredBiomes_.insert({biomeId, true});
    }

    bool hasExploredBiome(int32_t biomeId) const {
        return exploredBiomes_.find(biomeId) != exploredBiomes_.end();
    }

    int32_t getExploredBiomeCount() const {
        return static_cast<int32_t>(exploredBiomes_.size());
    }

    // ─── Distance stat helpers ───
    // Java: distances are stored in centimeters (cm)
    // Player movement tracking calls these with cm values
    void addDistance(const char* statId, double centimeters) {
        if (centimeters > 0) {
            increaseStat(statId, static_cast<int32_t>(centimeters));
        }
    }

    // ─── Dirty flag for save scheduling ───
    bool isDirty() const { return dirty_; }
    void markClean() { dirty_ = false; }

    // ─── Bulk access for serialization ───
    const std::unordered_map<std::string, int32_t>& getAllStats() const {
        return stats_;
    }

    const std::unordered_map<std::string, bool>& getAllAchievements() const {
        return achievements_;
    }

private:
    std::unordered_map<std::string, int32_t> stats_;
    std::unordered_map<std::string, bool> achievements_;
    std::unordered_map<int32_t, bool> exploredBiomes_;
    bool dirty_ = false;
};

} // namespace mccpp
