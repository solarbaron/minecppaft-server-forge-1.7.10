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
    // References: RecipesTools, RecipesWeapons, RecipesArmor, RecipesIngots,
    //             RecipesCrafting, RecipesFood, RecipesDyes
    // Block/item IDs from vanilla 1.7.10 (Blocks.java, Items.java)

    // Shorthand aliases
    constexpr int32_t W = WILDCARD_DAMAGE;
    auto I = [](int32_t id, int32_t count = 1, int32_t dmg = 0) { return ItemStack(id, count, dmg); };
    auto N = []() -> std::optional<ItemStack> { return std::nullopt; };

    // Helper: register a 3×3 shaped recipe
    auto shaped3 = [&](std::vector<std::optional<ItemStack>> p, ItemStack out) {
        addRecipe(std::make_unique<ShapedRecipes>(3, 3, std::move(p), std::move(out)));
    };
    // Helper: register a 3×2 shaped recipe (3 wide, 2 tall)
    auto shaped32 = [&](std::vector<std::optional<ItemStack>> p, ItemStack out) {
        addRecipe(std::make_unique<ShapedRecipes>(3, 2, std::move(p), std::move(out)));
    };
    // Helper: register a 2×2 shaped recipe
    auto shaped22 = [&](std::vector<std::optional<ItemStack>> p, ItemStack out) {
        addRecipe(std::make_unique<ShapedRecipes>(2, 2, std::move(p), std::move(out)));
    };
    // Helper: register a 1×2 shaped recipe
    auto shaped12 = [&](std::vector<std::optional<ItemStack>> p, ItemStack out) {
        addRecipe(std::make_unique<ShapedRecipes>(1, 2, std::move(p), std::move(out)));
    };
    // Helper: register a 2×3 shaped recipe
    auto shaped23 = [&](std::vector<std::optional<ItemStack>> p, ItemStack out) {
        addRecipe(std::make_unique<ShapedRecipes>(2, 3, std::move(p), std::move(out)));
    };
    // Helper: register a 1×3 shaped recipe
    auto shaped13 = [&](std::vector<std::optional<ItemStack>> p, ItemStack out) {
        addRecipe(std::make_unique<ShapedRecipes>(1, 3, std::move(p), std::move(out)));
    };
    auto shaped31 = [&](std::vector<std::optional<ItemStack>> p, ItemStack out) {
        addRecipe(std::make_unique<ShapedRecipes>(3, 1, std::move(p), std::move(out)));
    };
    auto shaped21 = [&](std::vector<std::optional<ItemStack>> p, ItemStack out) {
        addRecipe(std::make_unique<ShapedRecipes>(2, 1, std::move(p), std::move(out)));
    };

    // ─── Material constants ───────────────────────────────────────────
    // IDs: planks=5, cobble=4, iron_ingot=265, gold_ingot=266, diamond=264, stick=280

    // ═══════════════════════════════════════════════════════════════════
    // Basic materials
    // ═══════════════════════════════════════════════════════════════════

    // Planks from logs
    addShapelessRecipe(I(5,4,0), {I(17,1,W)});   // log → oak planks
    addShapelessRecipe(I(5,4,0), {I(162,1,W)});   // log2 → planks

    // Sticks
    shaped12({I(5,1,W), I(5,1,W)}, I(280,4,0));

    // Crafting table
    shaped22({I(5,1,W), I(5,1,W), I(5,1,W), I(5,1,W)}, I(58,1,0));

    // Torch (coal/charcoal + stick)
    shaped12({I(263,1,W), I(280,1,0)}, I(50,4,0));

    // ═══════════════════════════════════════════════════════════════════
    // Tools — all 5 materials (RecipesTools.java)
    // Pattern: pickaxe=MMM/_S_/_S_, axe=MM_/MS_/_S_, shovel=M/_S/_S, hoe=MM/_S/_S
    // ═══════════════════════════════════════════════════════════════════

    auto addTools = [&](int32_t mat, int32_t pick, int32_t axe, int32_t shovel, int32_t hoe, int32_t dmg = 0) {
        auto M = I(mat,1,dmg);
        auto S = I(280,1,0);
        // Pickaxe (3×3)
        shaped3({M, M, M, N(), S, N(), N(), S, N()}, I(pick,1,0));
        // Axe (3×3 — MM_/MS_/_S_)
        shaped3({M, M, N(), M, S, N(), N(), S, N()}, I(axe,1,0));
        // Shovel (1×3)
        shaped13({M, S, S}, I(shovel,1,0));
        // Hoe (3×3 — MM_/_S_/_S_)
        shaped3({M, M, N(), N(), S, N(), N(), S, N()}, I(hoe,1,0));
    };
    addTools(5,   270, 271, 269, 290, W);  // Wood (planks, wildcard damage)
    addTools(4,   274, 275, 273, 291);      // Stone (cobblestone)
    addTools(265, 257, 258, 256, 292);      // Iron
    addTools(264, 278, 279, 277, 293);      // Diamond
    addTools(266, 285, 286, 284, 294);      // Gold

    // ═══════════════════════════════════════════════════════════════════
    // Weapons — swords (RecipesWeapons.java)
    // Pattern: M/M/S (1×3)
    // ═══════════════════════════════════════════════════════════════════

    auto addSword = [&](int32_t mat, int32_t sword, int32_t dmg = 0) {
        shaped13({I(mat,1,dmg), I(mat,1,dmg), I(280,1,0)}, I(sword,1,0));
    };
    addSword(5,   268, W);  // Wooden sword
    addSword(4,   272);      // Stone sword
    addSword(265, 267);      // Iron sword
    addSword(264, 276);      // Diamond sword
    addSword(266, 283);      // Gold sword

    // Bow (sticks + string)
    // Item ID: bow=261, string=287
    shaped3({N(), I(280,1,0), I(287,1,0),
             I(280,1,0), N(), I(287,1,0),
             N(), I(280,1,0), I(287,1,0)}, I(261,1,0));

    // Arrow (flint + stick + feather)
    // Item IDs: arrow=262, flint=318, feather=288
    shaped13({I(318,1,0), I(280,1,0), I(288,1,0)}, I(262,4,0));

    // ═══════════════════════════════════════════════════════════════════
    // Armor — all 5 materials (RecipesArmor.java)
    // Helmet=MMM/M_M, Chest=M_M/MMM/MMM, Legs=MMM/M_M/M_M, Boots=M_M/M_M
    // ═══════════════════════════════════════════════════════════════════

    auto addArmor = [&](int32_t mat, int32_t helm, int32_t chest, int32_t legs, int32_t boots) {
        auto M = I(mat,1,0);
        // Helmet (3×2)
        shaped32({M, M, M, M, N(), M}, I(helm,1,0));
        // Chestplate (3×3)
        shaped3({M, N(), M, M, M, M, M, M, M}, I(chest,1,0));
        // Leggings (3×3)
        shaped3({M, M, M, M, N(), M, M, N(), M}, I(legs,1,0));
        // Boots (3×2)
        shaped32({M, N(), M, M, N(), M}, I(boots,1,0));
    };
    addArmor(334, 298, 299, 300, 301);  // Leather (leather=334)
    addArmor(265, 306, 307, 308, 309);  // Iron
    addArmor(264, 310, 311, 312, 313);  // Diamond
    addArmor(266, 314, 315, 316, 317);  // Gold

    // ═══════════════════════════════════════════════════════════════════
    // Building blocks
    // ═══════════════════════════════════════════════════════════════════

    // Chest (8 planks ring)
    shaped3({I(5,1,W), I(5,1,W), I(5,1,W),
             I(5,1,W), N(),       I(5,1,W),
             I(5,1,W), I(5,1,W), I(5,1,W)}, I(54,1,0));

    // Furnace (8 cobblestone ring)
    shaped3({I(4,1,0), I(4,1,0), I(4,1,0),
             I(4,1,0), N(),       I(4,1,0),
             I(4,1,0), I(4,1,0), I(4,1,0)}, I(61,1,0));

    // Ladder (sticks in ladder pattern, 3 ladders)
    shaped3({I(280,1,0), N(),       I(280,1,0),
             I(280,1,0), I(280,1,0), I(280,1,0),
             I(280,1,0), N(),       I(280,1,0)}, I(65,3,0));

    // Fence (planks + sticks — 2 fence)
    // Block ID: fence=85
    shaped31({I(280,1,0), I(5,1,W), I(280,1,0)}, I(85,2,0));

    // Fence gate (sticks + planks)
    // Block ID: fence_gate=107
    shaped31({I(5,1,W), I(280,1,0), I(5,1,W)}, I(107,1,0));

    // Wooden door
    // Item IDs: wooden_door_item=324, planks=5
    shaped23({I(5,1,W), I(5,1,W), I(5,1,W), I(5,1,W), I(5,1,W), I(5,1,W)}, I(324,1,0));

    // Iron door
    // Item IDs: iron_door_item=330, iron_ingot=265
    shaped23({I(265,1,0), I(265,1,0), I(265,1,0), I(265,1,0), I(265,1,0), I(265,1,0)}, I(330,1,0));

    // Trapdoor (6 planks — 2 trapdoors)
    // Block ID: trapdoor=96
    shaped32({I(5,1,W), I(5,1,W), I(5,1,W), I(5,1,W), I(5,1,W), I(5,1,W)}, I(96,2,0));

    // Wooden slabs (3 planks → 6 slabs)
    // Block ID: wooden_slab=126
    shaped31({I(5,1,0), I(5,1,0), I(5,1,0)}, I(126,6,0));

    // Stone slabs (3 stone → 6 slabs)
    // Block ID: stone_slab=44
    shaped31({I(1,1,0), I(1,1,0), I(1,1,0)}, I(44,6,0));

    // Cobblestone slabs
    shaped31({I(4,1,0), I(4,1,0), I(4,1,0)}, I(44,6,3));

    // Brick slabs (brick block=45)
    shaped31({I(45,1,0), I(45,1,0), I(45,1,0)}, I(44,6,4));

    // Stone brick slabs (stone_brick=98)
    shaped31({I(98,1,0), I(98,1,0), I(98,1,0)}, I(44,6,5));

    // Sandstone slabs (sandstone=24)
    shaped31({I(24,1,0), I(24,1,0), I(24,1,0)}, I(44,6,1));

    // Oak stairs (Block ID=53)
    shaped3({I(5,1,0), N(),      N(),
             I(5,1,0), I(5,1,0), N(),
             I(5,1,0), I(5,1,0), I(5,1,0)}, I(53,4,0));

    // Cobblestone stairs (Block ID=67)
    shaped3({I(4,1,0), N(),      N(),
             I(4,1,0), I(4,1,0), N(),
             I(4,1,0), I(4,1,0), I(4,1,0)}, I(67,4,0));

    // Stone brick stairs (Block ID=109)
    shaped3({I(98,1,0), N(),       N(),
             I(98,1,0), I(98,1,0), N(),
             I(98,1,0), I(98,1,0), I(98,1,0)}, I(109,4,0));

    // Brick stairs (Block ID=108)
    shaped3({I(45,1,0), N(),       N(),
             I(45,1,0), I(45,1,0), N(),
             I(45,1,0), I(45,1,0), I(45,1,0)}, I(108,4,0));

    // Stone bricks (4 stone → 4 stone bricks, Block ID=98)
    shaped22({I(1,1,0), I(1,1,0), I(1,1,0), I(1,1,0)}, I(98,4,0));

    // Sandstone (4 sand → 1 sandstone, Block ID=24, sand=12)
    shaped22({I(12,1,0), I(12,1,0), I(12,1,0), I(12,1,0)}, I(24,1,0));

    // Brick block (4 bricks → 1 brick block, brick_item=336, brick_block=45)
    shaped22({I(336,1,0), I(336,1,0), I(336,1,0), I(336,1,0)}, I(45,1,0));

    // Snow block (4 snowballs, Block ID=80, snowball=332)
    shaped22({I(332,1,0), I(332,1,0), I(332,1,0), I(332,1,0)}, I(80,1,0));

    // Glowstone block (4 dust, Block ID=89, dust=348)
    shaped22({I(348,1,0), I(348,1,0), I(348,1,0), I(348,1,0)}, I(89,1,0));

    // Wool (4 string → 1 wool, Block ID=35, string=287)
    shaped22({I(287,1,0), I(287,1,0), I(287,1,0), I(287,1,0)}, I(35,1,0));

    // TNT (5 gunpowder + 4 sand)
    // Block ID=46, gunpowder=289, sand=12
    shaped3({I(289,1,0), I(12,1,0), I(289,1,0),
             I(12,1,0),  I(289,1,0), I(12,1,0),
             I(289,1,0), I(12,1,0), I(289,1,0)}, I(46,1,0));

    // Bookshelf (6 planks + 3 books)
    // Block ID=47, book=340
    shaped3({I(5,1,W), I(5,1,W), I(5,1,W),
             I(340,1,0), I(340,1,0), I(340,1,0),
             I(5,1,W), I(5,1,W), I(5,1,W)}, I(47,1,0));

    // Jack-o-lantern (pumpkin + torch)
    // Block ID=91, pumpkin=86
    shaped21({I(86,1,0), I(50,1,0)}, I(91,1,0));

    // ═══════════════════════════════════════════════════════════════════
    // Utility items
    // ═══════════════════════════════════════════════════════════════════

    // Bed (3 wool + 3 planks)
    // Item ID=355, wool=35
    shaped32({I(35,1,W), I(35,1,W), I(35,1,W),
              I(5,1,W),  I(5,1,W),  I(5,1,W)}, I(355,1,0));

    // Sign (6 planks + stick)
    // Item ID=323
    shaped3({I(5,1,W), I(5,1,W), I(5,1,W),
             I(5,1,W), I(5,1,W), I(5,1,W),
             N(),       I(280,1,0), N()}, I(323,3,0));

    // Boat (5 planks)
    // Item ID=333
    shaped32({I(5,1,W), N(),       I(5,1,W),
              I(5,1,W), I(5,1,W), I(5,1,W)}, I(333,1,0));

    // Bowl (3 planks)
    // Item ID=281
    shaped32({I(5,1,W), N(),       I(5,1,W),
              N(),       I(5,1,W), N()}, I(281,4,0));

    // Bucket (3 iron ingots)
    // Item ID=325
    shaped32({I(265,1,0), N(),        I(265,1,0),
              N(),         I(265,1,0), N()}, I(325,1,0));

    // Compass (4 iron + 1 redstone)
    // Item ID=345, redstone=331
    shaped3({N(),        I(265,1,0), N(),
             I(265,1,0), I(331,1,0), I(265,1,0),
             N(),        I(265,1,0), N()}, I(345,1,0));

    // Clock (4 gold + 1 redstone)
    // Item ID=347
    shaped3({N(),        I(266,1,0), N(),
             I(266,1,0), I(331,1,0), I(266,1,0),
             N(),        I(266,1,0), N()}, I(347,1,0));

    // Shears (2 iron ingots)
    // Item ID=359
    shaped22({N(), I(265,1,0), I(265,1,0), N()}, I(359,1,0));

    // Flint and steel (iron + flint)
    // Item ID=259, flint=318
    addShapelessRecipe(I(259,1,0), {I(265,1,0), I(318,1,0)});

    // Fishing rod (3 sticks + 2 string)
    // Item ID=346
    shaped3({N(),        N(),        I(280,1,0),
             N(),        I(280,1,0), I(287,1,0),
             I(280,1,0), N(),        I(287,1,0)}, I(346,1,0));

    // Book (3 paper + 1 leather)
    // Item ID=340, paper=339, leather=334
    addShapelessRecipe(I(340,1,0), {I(339,1,0), I(339,1,0), I(339,1,0), I(334,1,0)});

    // Paper (3 sugar cane → 3 paper)
    // Item IDs: paper=339, reeds=338
    shaped31({I(338,1,0), I(338,1,0), I(338,1,0)}, I(339,3,0));

    // Painting (8 sticks + 1 wool)
    // Item ID=321
    shaped3({I(280,1,0), I(280,1,0), I(280,1,0),
             I(280,1,0), I(35,1,W),  I(280,1,0),
             I(280,1,0), I(280,1,0), I(280,1,0)}, I(321,1,0));

    // ═══════════════════════════════════════════════════════════════════
    // Redstone components
    // ═══════════════════════════════════════════════════════════════════

    // Piston (planks + cobble + iron + redstone)
    // Block ID=33
    shaped3({I(5,1,W),  I(5,1,W),  I(5,1,W),
             I(4,1,0),  I(265,1,0), I(4,1,0),
             I(4,1,0),  I(331,1,0), I(4,1,0)}, I(33,1,0));

    // Sticky piston (slimeball + piston)
    // Block ID=29, slimeball=341
    shaped12({I(341,1,0), I(33,1,0)}, I(29,1,0));

    // Dispenser (bow + cobble + redstone)
    // Block ID=23
    shaped3({I(4,1,0), I(4,1,0),  I(4,1,0),
             I(4,1,0), I(261,1,0), I(4,1,0),
             I(4,1,0), I(331,1,0), I(4,1,0)}, I(23,1,0));

    // Dropper (cobble ring + redstone)
    // Block ID=158
    shaped3({I(4,1,0), I(4,1,0),  I(4,1,0),
             I(4,1,0), N(),        I(4,1,0),
             I(4,1,0), I(331,1,0), I(4,1,0)}, I(158,1,0));

    // Lever (stick + cobble)
    // Block ID=69
    shaped12({I(280,1,0), I(4,1,0)}, I(69,1,0));

    // Stone button (1 stone)
    // Block ID=77
    addShapelessRecipe(I(77,1,0), {I(1,1,0)});

    // Wooden button (1 planks)
    // Block ID=143
    addShapelessRecipe(I(143,1,0), {I(5,1,W)});

    // Stone pressure plate (2 stone)
    // Block ID=70
    shaped21({I(1,1,0), I(1,1,0)}, I(70,1,0));

    // Wooden pressure plate (2 planks)
    // Block ID=72
    shaped21({I(5,1,W), I(5,1,W)}, I(72,1,0));

    // Repeater (3 stone + 2 torches + 1 redstone)
    // Block ID=93 (item), redstone_torch=76
    shaped31({I(331,1,0), I(76,1,0), I(76,1,0)}, I(356,1,0));

    // Redstone torch (redstone + stick)
    // Block ID=76
    shaped12({I(331,1,0), I(280,1,0)}, I(76,1,0));

    // ═══════════════════════════════════════════════════════════════════
    // Storage blocks (RecipesIngots.java)
    // ═══════════════════════════════════════════════════════════════════

    // Iron block (9 ingots → 1 block)
    shaped3({I(265,1,0),I(265,1,0),I(265,1,0),
             I(265,1,0),I(265,1,0),I(265,1,0),
             I(265,1,0),I(265,1,0),I(265,1,0)}, I(42,1,0));

    // Gold block
    shaped3({I(266,1,0),I(266,1,0),I(266,1,0),
             I(266,1,0),I(266,1,0),I(266,1,0),
             I(266,1,0),I(266,1,0),I(266,1,0)}, I(41,1,0));

    // Diamond block
    shaped3({I(264,1,0),I(264,1,0),I(264,1,0),
             I(264,1,0),I(264,1,0),I(264,1,0),
             I(264,1,0),I(264,1,0),I(264,1,0)}, I(57,1,0));

    // Emerald block
    shaped3({I(388,1,0),I(388,1,0),I(388,1,0),
             I(388,1,0),I(388,1,0),I(388,1,0),
             I(388,1,0),I(388,1,0),I(388,1,0)}, I(133,1,0));

    // Lapis block
    shaped3({I(351,1,4),I(351,1,4),I(351,1,4),
             I(351,1,4),I(351,1,4),I(351,1,4),
             I(351,1,4),I(351,1,4),I(351,1,4)}, I(22,1,0));

    // Redstone block
    shaped3({I(331,1,0),I(331,1,0),I(331,1,0),
             I(331,1,0),I(331,1,0),I(331,1,0),
             I(331,1,0),I(331,1,0),I(331,1,0)}, I(152,1,0));

    // Coal block
    shaped3({I(263,1,0),I(263,1,0),I(263,1,0),
             I(263,1,0),I(263,1,0),I(263,1,0),
             I(263,1,0),I(263,1,0),I(263,1,0)}, I(173,1,0));

    // Block → items (reverse storage)
    addShapelessRecipe(I(265,9,0), {I(42,1,0)});   // Iron block → 9 ingots
    addShapelessRecipe(I(266,9,0), {I(41,1,0)});   // Gold block → 9 ingots
    addShapelessRecipe(I(264,9,0), {I(57,1,0)});   // Diamond block → 9 diamonds
    addShapelessRecipe(I(388,9,0), {I(133,1,0)});  // Emerald block → 9 emeralds
    addShapelessRecipe(I(351,9,4), {I(22,1,0)});   // Lapis block → 9 lapis
    addShapelessRecipe(I(331,9,0), {I(152,1,0)});  // Redstone block → 9 redstone
    addShapelessRecipe(I(263,9,0), {I(173,1,0)});  // Coal block → 9 coal

    // Gold/Iron nuggets
    addShapelessRecipe(I(371,9,0), {I(266,1,0)});  // Gold ingot → 9 nuggets
    shaped3({I(371,1,0),I(371,1,0),I(371,1,0),
             I(371,1,0),I(371,1,0),I(371,1,0),
             I(371,1,0),I(371,1,0),I(371,1,0)}, I(266,1,0));  // 9 nuggets → ingot

    // ═══════════════════════════════════════════════════════════════════
    // Food (RecipesFood.java)
    // ═══════════════════════════════════════════════════════════════════

    // Bread (3 wheat)
    // Item ID=297, wheat=296
    shaped31({I(296,1,0), I(296,1,0), I(296,1,0)}, I(297,1,0));

    // Cake (3 milk + 2 sugar + 1 egg + 3 wheat)
    // Item ID=354, sugar=353, egg=344, milk_bucket=335
    shaped3({I(335,1,0), I(335,1,0), I(335,1,0),
             I(353,1,0), I(344,1,0), I(353,1,0),
             I(296,1,0), I(296,1,0), I(296,1,0)}, I(354,1,0));

    // Cookies (2 wheat + 1 cocoa)
    // Item ID=357, cocoa_beans=dye:3=351:3
    shaped31({I(296,1,0), I(351,1,3), I(296,1,0)}, I(357,8,0));

    // Mushroom stew (red + brown mushroom + bowl)
    // Item ID=282, red_mushroom=40, brown_mushroom=39, bowl=281
    addShapelessRecipe(I(282,1,0), {I(39,1,0), I(40,1,0), I(281,1,0)});

    // Golden apple (8 gold ingots + apple)
    // Item ID=322, apple=260
    shaped3({I(266,1,0), I(266,1,0), I(266,1,0),
             I(266,1,0), I(260,1,0), I(266,1,0),
             I(266,1,0), I(266,1,0), I(266,1,0)}, I(322,1,0));

    // Pumpkin pie (pumpkin + sugar + egg)
    // Item ID=400, pumpkin=86
    addShapelessRecipe(I(400,1,0), {I(86,1,0), I(353,1,0), I(344,1,0)});

    // Sugar from sugar cane
    addShapelessRecipe(I(353,1,0), {I(338,1,0)});

    // Melon seeds (melon slice → seeds)
    // Item ID=362, melon=360
    addShapelessRecipe(I(362,1,0), {I(360,1,0)});

    // Pumpkin seeds
    // Item ID=361, pumpkin=86
    addShapelessRecipe(I(361,4,0), {I(86,1,0)});

    // ═══════════════════════════════════════════════════════════════════
    // Miscellaneous
    // ═══════════════════════════════════════════════════════════════════

    // Chest minecart
    addShapelessRecipe(I(342,1,0), {I(328,1,0), I(54,1,0)});

    // Furnace minecart
    addShapelessRecipe(I(343,1,0), {I(328,1,0), I(61,1,0)});

    // Rails (6 iron + 1 stick → 16 rails)
    // Block ID=66
    shaped3({I(265,1,0), N(),        I(265,1,0),
             I(265,1,0), I(280,1,0), I(265,1,0),
             I(265,1,0), N(),        I(265,1,0)}, I(66,16,0));

    // Powered rail (6 gold + 1 stick + 1 redstone → 6 powered)
    // Block ID=27
    shaped3({I(266,1,0), N(),        I(266,1,0),
             I(266,1,0), I(280,1,0), I(266,1,0),
             I(266,1,0), I(331,1,0), I(266,1,0)}, I(27,6,0));

    // Minecart (5 iron)
    // Item ID=328
    shaped32({I(265,1,0), N(),        I(265,1,0),
              I(265,1,0), I(265,1,0), I(265,1,0)}, I(328,1,0));

    // Glass pane (6 glass → 16 panes)
    // Block ID=102, glass=20
    shaped32({I(20,1,0), I(20,1,0), I(20,1,0),
              I(20,1,0), I(20,1,0), I(20,1,0)}, I(102,16,0));

    // Iron bars (6 iron → 16 bars)
    // Block ID=101
    shaped32({I(265,1,0), I(265,1,0), I(265,1,0),
              I(265,1,0), I(265,1,0), I(265,1,0)}, I(101,16,0));

    // Cobblestone wall (6 cobble → 6 walls)
    // Block ID=139
    shaped32({I(4,1,0), I(4,1,0), I(4,1,0),
              I(4,1,0), I(4,1,0), I(4,1,0)}, I(139,6,0));

    // ─── Sort by recipe size (larger first) ─────────────────────────
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

    // Additional vanilla recipes
    addSmelting(19, ItemStack(19, 1, 0), 0.15f);   // wet_sponge(19:1) → sponge(19:0) (simplified: 19→19)
    addSmelting(98, ItemStack(98, 1, 2), 0.1f);    // stone_brick(98:0) → cracked_stone_brick(98:2)
    addSmelting(1, ItemStack(1, 1, 0), 0.1f);      // stone(1) → stone(1) (smooth stone, re-smelt)

    // Gold/Iron tool/armor → nuggets (Java: FurnaceRecipes — smelting gear)
    // Simplified: any gold tool or armor ID → 1 gold nugget (371)
    addSmelting(283, ItemStack(371, 1, 0), 0.1f);  // gold_sword → gold_nugget
    addSmelting(285, ItemStack(371, 1, 0), 0.1f);  // gold_pickaxe → gold_nugget
    addSmelting(286, ItemStack(371, 1, 0), 0.1f);  // gold_axe → gold_nugget
    addSmelting(284, ItemStack(371, 1, 0), 0.1f);  // gold_shovel → gold_nugget
    addSmelting(294, ItemStack(371, 1, 0), 0.1f);  // gold_hoe → gold_nugget
    addSmelting(314, ItemStack(371, 1, 0), 0.1f);  // gold_helmet → gold_nugget
    addSmelting(315, ItemStack(371, 1, 0), 0.1f);  // gold_chestplate → gold_nugget
    addSmelting(316, ItemStack(371, 1, 0), 0.1f);  // gold_leggings → gold_nugget
    addSmelting(317, ItemStack(371, 1, 0), 0.1f);  // gold_boots → gold_nugget
    // Iron tool/armor → iron_ingot (265)
    addSmelting(267, ItemStack(265, 1, 0), 0.1f);  // iron_sword → iron_ingot
    addSmelting(257, ItemStack(265, 1, 0), 0.1f);  // iron_pickaxe → iron_ingot
    addSmelting(258, ItemStack(265, 1, 0), 0.1f);  // iron_axe → iron_ingot
    addSmelting(256, ItemStack(265, 1, 0), 0.1f);  // iron_shovel → iron_ingot
    addSmelting(292, ItemStack(265, 1, 0), 0.1f);  // iron_hoe → iron_ingot
    addSmelting(306, ItemStack(265, 1, 0), 0.1f);  // iron_helmet → iron_ingot
    addSmelting(307, ItemStack(265, 1, 0), 0.1f);  // iron_chestplate → iron_ingot
    addSmelting(308, ItemStack(265, 1, 0), 0.1f);  // iron_leggings → iron_ingot
    addSmelting(309, ItemStack(265, 1, 0), 0.1f);  // iron_boots → iron_ingot

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
