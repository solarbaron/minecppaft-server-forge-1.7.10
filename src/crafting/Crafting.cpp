/**
 * Crafting.cpp — Crafting system implementation.
 *
 * Java references:
 *   net.minecraft.item.crafting.ShapedRecipes — grid pattern match with mirror
 *   net.minecraft.item.crafting.ShapelessRecipes — orderless ingredient match
 *   net.minecraft.item.crafting.CraftingManager — recipe registry singleton
 *   net.minecraft.item.crafting.FurnaceRecipes — smelting map
 *
 * Key behaviors preserved from Java:
 *   - ShapedRecipes.checkMatch: slides pattern across 3×3 grid, tries mirrored
 *   - Damage value 32767 (Short.MAX_VALUE) = wildcard, matches any damage
 *   - CraftingManager sorts recipes by size (larger first) for priority
 *   - FurnaceRecipes uses item ID + damage pair, wildcard = any damage
 */

#include "crafting/Crafting.h"

#include <algorithm>
#include <iostream>

namespace mccpp {

// Wildcard damage value — matches any damage
// Java: Short.MAX_VALUE = 32767
static constexpr int32_t WILDCARD_DAMAGE = 32767;

// ═══════════════════════════════════════════════════════════════════════════
// ShapedRecipes
// ═══════════════════════════════════════════════════════════════════════════

bool ShapedRecipes::matches(const CraftingGrid& grid) const {
    // Java: ShapedRecipes.matches(InventoryCrafting, World)
    // Try all possible offsets on the 3×3 grid, both normal and mirrored
    for (int32_t x = 0; x <= 3 - recipeWidth_; ++x) {
        for (int32_t y = 0; y <= 3 - recipeHeight_; ++y) {
            if (checkMatch(grid, x, y, true)) return true;
            if (checkMatch(grid, x, y, false)) return true;
        }
    }
    return false;
}

bool ShapedRecipes::checkMatch(const CraftingGrid& grid, int32_t startX, int32_t startY, bool mirror) const {
    // Java: ShapedRecipes.checkMatch(InventoryCrafting, int, int, boolean)
    for (int32_t x = 0; x < 3; ++x) {
        for (int32_t y = 0; y < 3; ++y) {
            int32_t rx = x - startX;
            int32_t ry = y - startY;

            std::optional<ItemStack> required = std::nullopt;
            if (rx >= 0 && ry >= 0 && rx < recipeWidth_ && ry < recipeHeight_) {
                int32_t idx = mirror
                    ? (recipeWidth_ - rx - 1 + ry * recipeWidth_)
                    : (rx + ry * recipeWidth_);
                required = recipeItems_[idx];
            }

            auto gridItem = grid.getStackInRowAndColumn(x, y);

            // Both null → ok
            if (!gridItem.has_value() && !required.has_value()) continue;

            // One null, other not → fail
            if (!gridItem.has_value() || !required.has_value()) return false;

            // Item type must match
            if (required->getItemId() != gridItem->getItemId()) return false;

            // Damage must match (unless wildcard)
            if (required->getDamage() != WILDCARD_DAMAGE &&
                required->getDamage() != gridItem->getDamage()) {
                return false;
            }
        }
    }
    return true;
}

ItemStack ShapedRecipes::getCraftingResult(const CraftingGrid& /*grid*/) const {
    // Java: ShapedRecipes.getCraftingResult(InventoryCrafting)
    return recipeOutput_.copy();
}

// ═══════════════════════════════════════════════════════════════════════════
// ShapelessRecipes
// ═══════════════════════════════════════════════════════════════════════════

bool ShapelessRecipes::matches(const CraftingGrid& grid) const {
    // Java: ShapelessRecipes.matches(InventoryCrafting, World)
    // Clone ingredient list, remove matched items one by one
    std::vector<ItemStack> remaining(recipeItems_);

    for (int32_t row = 0; row < 3; ++row) {
        for (int32_t col = 0; col < 3; ++col) {
            auto gridItem = grid.getStackInRowAndColumn(col, row);
            if (!gridItem.has_value()) continue;

            bool found = false;
            for (auto it = remaining.begin(); it != remaining.end(); ++it) {
                if (it->getItemId() == gridItem->getItemId() &&
                    (it->getDamage() == WILDCARD_DAMAGE ||
                     it->getDamage() == gridItem->getDamage())) {
                    found = true;
                    remaining.erase(it);
                    break;
                }
            }

            if (!found) return false;
        }
    }

    return remaining.empty();
}

ItemStack ShapelessRecipes::getCraftingResult(const CraftingGrid& /*grid*/) const {
    // Java: ShapelessRecipes.getCraftingResult(InventoryCrafting)
    return recipeOutput_.copy();
}

// ═══════════════════════════════════════════════════════════════════════════
// CraftingManager
// ═══════════════════════════════════════════════════════════════════════════

CraftingManager& CraftingManager::getInstance() {
    static CraftingManager instance;
    return instance;
}

CraftingManager::CraftingManager() {
    // Java: CraftingManager constructor registers all vanilla recipes.
    // Recipes are registered by RecipesTools, RecipesWeapons, RecipesArmor, etc.
    // For now, register a representative subset; full recipe registration
    // will be data-driven when all items are in the registry.

    // Example: planks from logs (shapeless, 4 planks from 1 log of any type)
    // Java: addShapelessRecipe(new ItemStack(Blocks.planks, 4), Blocks.log)
    // Block IDs: log=17, planks=5
    addShapelessRecipe(ItemStack(5, 4, 0), {ItemStack(17, 1, WILDCARD_DAMAGE)});

    // Log2 (acacia/dark oak) → planks
    // Block ID: log2=162
    addShapelessRecipe(ItemStack(5, 4, 0), {ItemStack(162, 1, WILDCARD_DAMAGE)});

    // Sticks from planks (shaped 2×1)
    // Item IDs: stick=280, planks=5
    {
        std::vector<std::optional<ItemStack>> pattern = {
            ItemStack(5, 1, WILDCARD_DAMAGE),
            ItemStack(5, 1, WILDCARD_DAMAGE),
        };
        addRecipe(std::make_unique<ShapedRecipes>(1, 2, std::move(pattern), ItemStack(280, 4, 0)));
    }

    // Crafting table (shaped 2×2)
    // Block ID: crafting_table=58
    {
        std::vector<std::optional<ItemStack>> pattern = {
            ItemStack(5, 1, WILDCARD_DAMAGE), ItemStack(5, 1, WILDCARD_DAMAGE),
            ItemStack(5, 1, WILDCARD_DAMAGE), ItemStack(5, 1, WILDCARD_DAMAGE),
        };
        addRecipe(std::make_unique<ShapedRecipes>(2, 2, std::move(pattern), ItemStack(58, 1, 0)));
    }

    // Chest (shaped 3×3 ring of planks)
    // Block ID: chest=54
    {
        std::vector<std::optional<ItemStack>> pattern = {
            ItemStack(5,1,WILDCARD_DAMAGE), ItemStack(5,1,WILDCARD_DAMAGE), ItemStack(5,1,WILDCARD_DAMAGE),
            ItemStack(5,1,WILDCARD_DAMAGE), std::nullopt,                    ItemStack(5,1,WILDCARD_DAMAGE),
            ItemStack(5,1,WILDCARD_DAMAGE), ItemStack(5,1,WILDCARD_DAMAGE), ItemStack(5,1,WILDCARD_DAMAGE),
        };
        addRecipe(std::make_unique<ShapedRecipes>(3, 3, std::move(pattern), ItemStack(54, 1, 0)));
    }

    // Furnace (shaped 3×3 ring of cobblestone)
    // Block ID: furnace=61, cobblestone=4
    {
        std::vector<std::optional<ItemStack>> pattern = {
            ItemStack(4,1,0), ItemStack(4,1,0), ItemStack(4,1,0),
            ItemStack(4,1,0), std::nullopt,     ItemStack(4,1,0),
            ItemStack(4,1,0), ItemStack(4,1,0), ItemStack(4,1,0),
        };
        addRecipe(std::make_unique<ShapedRecipes>(3, 3, std::move(pattern), ItemStack(61, 1, 0)));
    }

    // Wooden pickaxe (shaped 3×2)
    // Item ID: wooden_pickaxe=270, stick=280, planks=5
    {
        std::vector<std::optional<ItemStack>> pattern = {
            ItemStack(5,1,WILDCARD_DAMAGE), ItemStack(5,1,WILDCARD_DAMAGE), ItemStack(5,1,WILDCARD_DAMAGE),
            std::nullopt,                    ItemStack(280,1,0),              std::nullopt,
            std::nullopt,                    ItemStack(280,1,0),              std::nullopt,
        };
        addRecipe(std::make_unique<ShapedRecipes>(3, 3, std::move(pattern), ItemStack(270, 1, 0)));
    }

    // Stone pickaxe
    // Item ID: stone_pickaxe=274, cobblestone=4
    {
        std::vector<std::optional<ItemStack>> pattern = {
            ItemStack(4,1,0), ItemStack(4,1,0), ItemStack(4,1,0),
            std::nullopt,     ItemStack(280,1,0), std::nullopt,
            std::nullopt,     ItemStack(280,1,0), std::nullopt,
        };
        addRecipe(std::make_unique<ShapedRecipes>(3, 3, std::move(pattern), ItemStack(274, 1, 0)));
    }

    // Iron pickaxe
    // Item ID: iron_pickaxe=257, iron_ingot=265
    {
        std::vector<std::optional<ItemStack>> pattern = {
            ItemStack(265,1,0), ItemStack(265,1,0), ItemStack(265,1,0),
            std::nullopt,       ItemStack(280,1,0), std::nullopt,
            std::nullopt,       ItemStack(280,1,0), std::nullopt,
        };
        addRecipe(std::make_unique<ShapedRecipes>(3, 3, std::move(pattern), ItemStack(257, 1, 0)));
    }

    // Diamond pickaxe
    // Item ID: diamond_pickaxe=278, diamond=264
    {
        std::vector<std::optional<ItemStack>> pattern = {
            ItemStack(264,1,0), ItemStack(264,1,0), ItemStack(264,1,0),
            std::nullopt,       ItemStack(280,1,0), std::nullopt,
            std::nullopt,       ItemStack(280,1,0), std::nullopt,
        };
        addRecipe(std::make_unique<ShapedRecipes>(3, 3, std::move(pattern), ItemStack(278, 1, 0)));
    }

    // Torch (shaped 1×2)
    // Block ID: torch=50, Items: coal=263 (damage 0 or 1), stick=280
    {
        std::vector<std::optional<ItemStack>> pattern = {
            ItemStack(263, 1, WILDCARD_DAMAGE),
            ItemStack(280, 1, 0),
        };
        addRecipe(std::make_unique<ShapedRecipes>(1, 2, std::move(pattern), ItemStack(50, 4, 0)));
    }

    // Sort recipes by size (largest first) — matches Java behavior
    // Java: Collections.sort(recipes, ...)
    std::sort(recipes_.begin(), recipes_.end(),
        [](const std::unique_ptr<IRecipe>& a, const std::unique_ptr<IRecipe>& b) {
            return a->getRecipeSize() > b->getRecipeSize();
        }
    );

    std::cout << "[Crafting] Registered " << recipes_.size() << " recipes\n";
}

void CraftingManager::addRecipe(std::unique_ptr<IRecipe> recipe) {
    recipes_.push_back(std::move(recipe));
}

void CraftingManager::addShapelessRecipe(ItemStack output, std::vector<ItemStack> ingredients) {
    recipes_.push_back(std::make_unique<ShapelessRecipes>(std::move(output), std::move(ingredients)));
}

std::optional<ItemStack> CraftingManager::findMatchingRecipe(const CraftingGrid& grid) const {
    // Java: CraftingManager.findMatchingRecipe(InventoryCrafting, World)
    for (const auto& recipe : recipes_) {
        if (recipe->matches(grid)) {
            return recipe->getCraftingResult(grid);
        }
    }
    return std::nullopt;
}

// ═══════════════════════════════════════════════════════════════════════════
// FurnaceRecipes
// ═══════════════════════════════════════════════════════════════════════════

FurnaceRecipes& FurnaceRecipes::instance() {
    static FurnaceRecipes inst;
    return inst;
}

FurnaceRecipes::FurnaceRecipes() {
    // Java: FurnaceRecipes constructor — registers all vanilla smelting recipes
    // Using vanilla block/item IDs from Minecraft 1.7.10

    // Ores → Ingots/Gems
    addSmelting(15, ItemStack(265, 1, 0), 0.7f);   // iron_ore(15) → iron_ingot(265)
    addSmelting(14, ItemStack(266, 1, 0), 1.0f);   // gold_ore(14) → gold_ingot(266)
    addSmelting(56, ItemStack(264, 1, 0), 1.0f);   // diamond_ore(56) → diamond(264)
    addSmelting(129, ItemStack(388, 1, 0), 1.0f);  // emerald_ore(129) → emerald(388)
    addSmelting(16, ItemStack(263, 1, 0), 0.1f);   // coal_ore(16) → coal(263)
    addSmelting(73, ItemStack(331, 1, 0), 0.7f);   // redstone_ore(73) → redstone(331)
    addSmelting(21, ItemStack(351, 1, 4), 0.2f);   // lapis_ore(21) → lapis(351:4)
    addSmelting(153, ItemStack(406, 1, 0), 0.2f);  // quartz_ore(153) → quartz(406)

    // Blocks
    addSmelting(12, ItemStack(20, 1, 0), 0.1f);    // sand(12) → glass(20)
    addSmelting(4, ItemStack(1, 1, 0), 0.1f);      // cobblestone(4) → stone(1)
    addSmelting(82, ItemStack(172, 1, 0), 0.35f);  // clay(82) → hardened_clay(172)
    addSmelting(81, ItemStack(351, 1, 2), 0.2f);   // cactus(81) → green dye(351:2)
    addSmelting(87, ItemStack(405, 1, 0), 0.1f);   // netherrack(87) → netherbrick(405)

    // Logs → charcoal
    addSmelting(17, ItemStack(263, 1, 1), 0.15f);  // log(17) → charcoal(263:1)
    addSmelting(162, ItemStack(263, 1, 1), 0.15f); // log2(162) → charcoal(263:1)

    // Food
    addSmelting(319, ItemStack(320, 1, 0), 0.35f); // porkchop(319) → cooked_porkchop(320)
    addSmelting(363, ItemStack(364, 1, 0), 0.35f); // beef(363) → cooked_beef(364)
    addSmelting(365, ItemStack(366, 1, 0), 0.35f); // chicken(365) → cooked_chicken(366)
    addSmelting(392, ItemStack(393, 1, 0), 0.35f); // potato(392) → baked_potato(393)

    // Fish (raw → cooked, damage 0 = normal fish)
    addSmeltingRecipe(349, 0, ItemStack(350, 1, 0), 0.35f); // fish:0 → cooked_fish:0
    addSmeltingRecipe(349, 1, ItemStack(350, 1, 1), 0.35f); // salmon:1 → cooked_salmon:1

    // Clay
    addSmelting(337, ItemStack(336, 1, 0), 0.3f);  // clay_ball(337) → brick(336)

    std::cout << "[Smelting] Registered " << recipes_.size() << " smelting recipes\n";
}

void FurnaceRecipes::addSmeltingRecipe(int32_t inputId, int32_t inputDamage,
                                        ItemStack output, float experience) {
    recipes_.push_back({inputId, inputDamage, std::move(output), experience});
}

std::optional<ItemStack> FurnaceRecipes::getSmeltingResult(int32_t itemId, int32_t damage) const {
    // Java: FurnaceRecipes.getSmeltingResult(ItemStack)
    for (const auto& recipe : recipes_) {
        if (recipe.inputId == itemId &&
            (recipe.inputDamage == WILDCARD_DAMAGE || recipe.inputDamage == damage)) {
            return recipe.output.copy();
        }
    }
    return std::nullopt;
}

float FurnaceRecipes::getSmeltingExperience(int32_t itemId, int32_t damage) const {
    // Java: FurnaceRecipes.getSmeltingExperience(ItemStack)
    for (const auto& recipe : recipes_) {
        if (recipe.inputId == itemId &&
            (recipe.inputDamage == WILDCARD_DAMAGE || recipe.inputDamage == damage)) {
            return recipe.experience;
        }
    }
    return 0.0f;
}

} // namespace mccpp
