/**
 * StatAchievement.cpp — Achievement and statistics system implementation.
 *
 * Java references:
 *   net.minecraft.stats.AchievementList — static initializer (33 achievements)
 *   net.minecraft.stats.Achievement — parent chain, special flag
 *   net.minecraft.stats.StatFileWriter — stat tracking (simplified)
 *
 * Achievement parent chains match Java exactly:
 *   openInventory → mineWood → buildWorkBench → buildPickaxe → ...
 *   Two main branches: tools/combat and nether/end
 *
 * Icon item IDs use vanilla block/item IDs for JNI compatibility.
 */

#include "stats/StatAchievement.h"
#include <algorithm>
#include <iostream>

namespace mccpp {

// ═════════════════════════════════════════════════════════════════════════════
// AchievementRegistry
// ═════════════════════════════════════════════════════════════════════════════

std::vector<Achievement> AchievementRegistry::achievements_;
bool AchievementRegistry::initialized_ = false;

void AchievementRegistry::init() {
    if (initialized_) return;

    // Java: AchievementList static initializer
    // Format: {statId, displayName, parentId, col, row, iconItemId, iconMeta, isSpecial, isIndependent}
    // Item IDs: book=340, log=17, crafting_table=58, wooden_pickaxe=270, furnace=61,
    //           iron_ingot=265, wooden_hoe=290, bread=297, cake=354, stone_pickaxe=274,
    //           cooked_fish=350, rail=66, wooden_sword=268, bone=352, leather=334,
    //           saddle=329, bow=261, diamond_ore=56, diamond=264, obsidian=49,
    //           ghast_tear=370, blaze_rod=369, potion=373, ender_eye=381,
    //           dragon_egg=122, enchanting_table=116, diamond_sword=276,
    //           bookshelf=47, wheat=296, skull=397, nether_star=399, beacon=138,
    //           diamond_boots=313

    achievements_ = {
        // Root: Open Inventory
        {OPEN_INVENTORY, "openInventory", "", 0, 0, 340, 0, false, true},

        // Branch 1: Mining/Crafting
        {MINE_WOOD, "mineWood", OPEN_INVENTORY, 2, 1, 17, 0, false, false},
        {BUILD_WORKBENCH, "buildWorkBench", MINE_WOOD, 4, -1, 58, 0, false, false},
        {BUILD_PICKAXE, "buildPickaxe", BUILD_WORKBENCH, 4, 2, 270, 0, false, false},
        {BUILD_FURNACE, "buildFurnace", BUILD_PICKAXE, 3, 4, 61, 0, false, false},
        {ACQUIRE_IRON, "acquireIron", BUILD_FURNACE, 1, 4, 265, 0, false, false},

        // Branch 2: Farming
        {BUILD_HOE, "buildHoe", BUILD_WORKBENCH, 2, -3, 290, 0, false, false},
        {MAKE_BREAD, "makeBread", BUILD_HOE, -1, -3, 297, 0, false, false},
        {BAKE_CAKE, "bakeCake", BUILD_HOE, 0, -5, 354, 0, false, false},

        // Branch 3: Tools
        {BUILD_BETTER_PICK, "buildBetterPickaxe", BUILD_PICKAXE, 6, 2, 274, 0, false, false},
        {COOK_FISH, "cookFish", BUILD_FURNACE, 2, 6, 350, 0, false, false},
        {ON_A_RAIL, "onARail", ACQUIRE_IRON, 2, 3, 66, 0, true, false}, // SPECIAL

        // Branch 4: Combat
        {BUILD_SWORD, "buildSword", BUILD_WORKBENCH, 6, -1, 268, 0, false, false},
        {KILL_ENEMY, "killEnemy", BUILD_SWORD, 8, -1, 352, 0, false, false},
        {KILL_COW, "killCow", BUILD_SWORD, 7, -3, 334, 0, false, false},
        {FLY_PIG, "flyPig", KILL_COW, 9, -3, 329, 0, true, false}, // SPECIAL
        {SNIPE_SKELETON, "snipeSkeleton", KILL_ENEMY, 7, 0, 261, 0, true, false}, // SPECIAL

        // Branch 5: Mining deep
        {DIAMONDS, "diamonds", ACQUIRE_IRON, -1, 5, 56, 0, false, false},
        {DIAMONDS_TO_YOU, "diamondsToYou", DIAMONDS, -1, 2, 264, 0, false, false},

        // Branch 6: Nether
        {PORTAL, "portal", DIAMONDS, -1, 7, 49, 0, false, false},
        {GHAST, "ghast", PORTAL, -4, 8, 370, 0, true, false}, // SPECIAL
        {BLAZE_ROD, "blazeRod", PORTAL, 0, 9, 369, 0, false, false},
        {POTION, "potion", BLAZE_ROD, 2, 8, 373, 0, false, false},

        // Branch 7: The End
        {THE_END, "theEnd", BLAZE_ROD, 3, 10, 381, 0, true, false}, // SPECIAL
        {THE_END2, "theEnd2", THE_END, 4, 13, 122, 0, true, false}, // SPECIAL

        // Branch 8: Enchanting
        {ENCHANTMENTS, "enchantments", DIAMONDS, -4, 4, 116, 0, false, false},
        {OVERKILL, "overkill", ENCHANTMENTS, -4, 1, 276, 0, true, false}, // SPECIAL
        {BOOKCASE, "bookcase", ENCHANTMENTS, -3, 6, 47, 0, false, false},

        // Branch 9: Animal Husbandry
        {BREED_COW, "breedCow", KILL_COW, 7, -5, 296, 0, false, false},

        // Branch 10: Wither/Beacon
        {SPAWN_WITHER, "spawnWither", THE_END2, 7, 12, 397, 1, false, false},
        {KILL_WITHER, "killWither", SPAWN_WITHER, 7, 10, 399, 0, false, false},
        {FULL_BEACON, "fullBeacon", KILL_WITHER, 7, 8, 138, 0, true, false}, // SPECIAL

        // Branch 11: Exploration
        {EXPLORE_ALL_BIOMES, "exploreAllBiomes", THE_END, 4, 8, 313, 0, true, false}, // SPECIAL
    };

    initialized_ = true;
    std::cout << "[Achievement] Registered " << achievements_.size() << " achievements\n";
}

const Achievement* AchievementRegistry::getById(const std::string& id) {
    for (const auto& ach : achievements_) {
        if (ach.statId == id) return &ach;
    }
    return nullptr;
}

const std::vector<Achievement>& AchievementRegistry::getAll() {
    return achievements_;
}

int32_t AchievementRegistry::getCount() {
    return static_cast<int32_t>(achievements_.size());
}

// ═════════════════════════════════════════════════════════════════════════════
// StatTracker — Per-player stat tracking
// ═════════════════════════════════════════════════════════════════════════════

void StatTracker::addStat(const std::string& statId, int32_t amount) {
    stats_[statId] += amount;
}

int32_t StatTracker::getStat(const std::string& statId) const {
    auto it = stats_.find(statId);
    return it != stats_.end() ? it->second : 0;
}

bool StatTracker::hasAchievement(const std::string& achievementId) const {
    return std::find(unlockedAchievements_.begin(),
                     unlockedAchievements_.end(),
                     achievementId) != unlockedAchievements_.end();
}

bool StatTracker::unlockAchievement(const std::string& achievementId) {
    // Already unlocked?
    if (hasAchievement(achievementId)) return false;

    const Achievement* ach = AchievementRegistry::getById(achievementId);
    if (!ach) return false;

    // Java: check parent chain (must have parent unlocked first)
    if (!ach->isIndependent && !ach->parentId.empty()) {
        if (!hasAchievement(ach->parentId)) {
            return false; // Parent not unlocked
        }
    }

    unlockedAchievements_.push_back(achievementId);
    addStat(achievementId, 1);
    return true;
}

} // namespace mccpp
