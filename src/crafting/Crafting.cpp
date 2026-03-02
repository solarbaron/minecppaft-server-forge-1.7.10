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

    // ═══════════════════════════════════════════════════════════════════
    // Functional blocks (RecipesCrafting.java, misc)
    // ═══════════════════════════════════════════════════════════════════

    // Enchanting table (book + diamond × 2 + obsidian × 4)
    // Block ID=116
    shaped3({N(),        I(340,1,0), N(),
             I(264,1,0), I(49,1,0),  I(264,1,0),
             I(49,1,0),  I(49,1,0),  I(49,1,0)}, I(116,1,0));

    // Brewing stand (blaze rod + cobble × 3)
    // Block ID=117 (item=379)
    shaped3({N(),       I(369,1,0), N(),
             I(4,1,0),  I(4,1,0),   I(4,1,0),
             N(),       N(),         N()}, I(379,1,0));

    // Anvil (iron block × 3 + iron ingot × 4)
    // Block ID=145
    shaped3({I(42,1,0),   I(42,1,0),  I(42,1,0),
             N(),          I(265,1,0), N(),
             I(265,1,0),   I(265,1,0), I(265,1,0)}, I(145,1,0));

    // Hopper (iron × 5 + chest)
    // Block ID=154
    shaped3({I(265,1,0), N(),       I(265,1,0),
             I(265,1,0), I(54,1,0), I(265,1,0),
             N(),        I(265,1,0), N()}, I(154,1,0));

    // Beacon (glass × 5 + nether star + obsidian × 3)
    // Block ID=138, nether_star=399
    shaped3({I(20,1,0),  I(20,1,0),  I(20,1,0),
             I(20,1,0),  I(399,1,0), I(20,1,0),
             I(49,1,0),  I(49,1,0),  I(49,1,0)}, I(138,1,0));

    // Ender chest (obsidian × 8 + ender eye)
    // Block ID=130, ender_eye=381
    shaped3({I(49,1,0),  I(49,1,0),  I(49,1,0),
             I(49,1,0),  I(381,1,0), I(49,1,0),
             I(49,1,0),  I(49,1,0),  I(49,1,0)}, I(130,1,0));

    // Comparator (3 torch + 1 quartz + 3 stone)
    // Item ID=404, redstone_torch=76, quartz=406
    shaped3({N(),       I(76,1,0),  N(),
             I(76,1,0), I(406,1,0), I(76,1,0),
             I(1,1,0),  I(1,1,0),   I(1,1,0)}, I(404,1,0));

    // Daylight sensor (glass × 3 + quartz × 3 + wood slab × 3)
    // Block ID=151, quartz=406, wooden_slab=126
    shaped3({I(20,1,0),  I(20,1,0),  I(20,1,0),
             I(406,1,0), I(406,1,0), I(406,1,0),
             I(126,1,W), I(126,1,W), I(126,1,W)}, I(151,1,0));

    // Tripwire hook (iron + stick + planks)
    // Block ID=131
    shaped13({I(265,1,0), I(280,1,0), I(5,1,W)}, I(131,2,0));

    // Trapped chest (chest + tripwire hook)
    // Block ID=146
    addShapelessRecipe(I(146,1,0), {I(54,1,0), I(131,1,0)});

    // Jukebox (planks × 8 + diamond)
    // Block ID=84
    shaped3({I(5,1,W),  I(5,1,W),  I(5,1,W),
             I(5,1,W),  I(264,1,0), I(5,1,W),
             I(5,1,W),  I(5,1,W),  I(5,1,W)}, I(84,1,0));

    // Note block (planks × 8 + redstone)
    // Block ID=25
    shaped3({I(5,1,W),  I(5,1,W),   I(5,1,W),
             I(5,1,W),  I(331,1,0),  I(5,1,W),
             I(5,1,W),  I(5,1,W),   I(5,1,W)}, I(25,1,0));

    // ─── More stairs ─────────────────────────────────────────────────

    // Spruce stairs (Block ID=134, spruce_planks=5:1)
    shaped3({I(5,1,1), N(),      N(),
             I(5,1,1), I(5,1,1), N(),
             I(5,1,1), I(5,1,1), I(5,1,1)}, I(134,4,0));

    // Birch stairs (Block ID=135, birch_planks=5:2)
    shaped3({I(5,1,2), N(),      N(),
             I(5,1,2), I(5,1,2), N(),
             I(5,1,2), I(5,1,2), I(5,1,2)}, I(135,4,0));

    // Jungle stairs (Block ID=136, jungle_planks=5:3)
    shaped3({I(5,1,3), N(),      N(),
             I(5,1,3), I(5,1,3), N(),
             I(5,1,3), I(5,1,3), I(5,1,3)}, I(136,4,0));

    // Sandstone stairs (Block ID=128)
    shaped3({I(24,1,0), N(),       N(),
             I(24,1,0), I(24,1,0), N(),
             I(24,1,0), I(24,1,0), I(24,1,0)}, I(128,4,0));

    // Nether brick stairs (Block ID=114, nether_brick=112)
    shaped3({I(112,1,0), N(),        N(),
             I(112,1,0), I(112,1,0), N(),
             I(112,1,0), I(112,1,0), I(112,1,0)}, I(114,4,0));

    // Quartz stairs (Block ID=156, quartz_block=155)
    shaped3({I(155,1,0), N(),        N(),
             I(155,1,0), I(155,1,0), N(),
             I(155,1,0), I(155,1,0), I(155,1,0)}, I(156,4,0));

    // Nether brick fence (Block ID=113, nether_brick=112)
    shaped32({I(112,1,0), I(112,1,0), I(112,1,0),
              I(112,1,0), I(112,1,0), I(112,1,0)}, I(113,6,0));

    // ─── Rails ──────────────────────────────────────────────────────

    // Detector rail (iron × 6 + stone_pressure_plate + redstone → 6 rails)
    // Block ID=28
    shaped3({I(265,1,0), N(),        I(265,1,0),
             I(265,1,0), I(70,1,0),  I(265,1,0),
             I(265,1,0), I(331,1,0), I(265,1,0)}, I(28,6,0));

    // Activator rail (iron × 6 + stick × 2 + redstone_torch → 6 rails)
    // Block ID=157
    shaped3({I(265,1,0), I(280,1,0), I(265,1,0),
             I(265,1,0), I(76,1,0),  I(265,1,0),
             I(265,1,0), I(280,1,0), I(265,1,0)}, I(157,6,0));

    // TNT minecart
    addShapelessRecipe(I(407,1,0), {I(328,1,0), I(46,1,0)});

    // Hopper minecart
    addShapelessRecipe(I(408,1,0), {I(328,1,0), I(154,1,0)});

    // ─── Pressure plates ─────────────────────────────────────────────

    // Light weighted pressure plate (gold × 2)
    // Block ID=147
    shaped21({I(266,1,0), I(266,1,0)}, I(147,1,0));

    // Heavy weighted pressure plate (iron × 2)
    // Block ID=148
    shaped21({I(265,1,0), I(265,1,0)}, I(148,1,0));

    // ═══════════════════════════════════════════════════════════════════
    // Dye recipes (RecipesDyes.java)
    // BlockColored.func_150031_c(n) = 15 - n (dye damage → wool/clay meta)
    // Items.dye = 351, Blocks.wool = 35, Blocks.stained_hardened_clay = 159
    // Blocks.stained_glass = 95, Blocks.stained_glass_pane = 160, Blocks.carpet = 171
    // ═══════════════════════════════════════════════════════════════════

    for (int32_t dyeColor = 0; dyeColor < 16; ++dyeColor) {
        int32_t woolMeta = 15 - dyeColor; // func_150031_c

        // Dye + white wool → colored wool
        addShapelessRecipe(I(35, 1, woolMeta), {I(351, 1, dyeColor), I(35, 1, 0)});

        // 8 hardened clay + dye → 8 stained hardened clay
        shaped3({I(172,1,0), I(172,1,0), I(172,1,0),
                 I(172,1,0), I(351,1,dyeColor), I(172,1,0),
                 I(172,1,0), I(172,1,0), I(172,1,0)}, I(159, 8, woolMeta));

        // 8 glass + dye → 8 stained glass
        shaped3({I(20,1,0), I(20,1,0), I(20,1,0),
                 I(20,1,0), I(351,1,dyeColor), I(20,1,0),
                 I(20,1,0), I(20,1,0), I(20,1,0)}, I(95, 8, woolMeta));

        // 6 stained glass → 16 stained glass panes (note: pane meta = dye color, not 15-n)
        shaped32({I(95,1,dyeColor), I(95,1,dyeColor), I(95,1,dyeColor),
                  I(95,1,dyeColor), I(95,1,dyeColor), I(95,1,dyeColor)}, I(160, 16, dyeColor));

        // 2 wool → 3 carpet
        shaped21({I(35,1,dyeColor), I(35,1,dyeColor)}, I(171, 3, dyeColor));
    }

    // Flower → dye conversions
    addShapelessRecipe(I(351,1,11), {I(37,1,0)});    // dandelion → yellow dye
    addShapelessRecipe(I(351,1,1), {I(38,1,0)});     // poppy → rose red
    addShapelessRecipe(I(351,3,15), {I(352,1,0)});   // bone(352) → 3 bone meal

    // Dye mixing
    addShapelessRecipe(I(351,2,9), {I(351,1,1), I(351,1,15)});     // red+white → pink
    addShapelessRecipe(I(351,2,14), {I(351,1,1), I(351,1,11)});    // red+yellow → orange
    addShapelessRecipe(I(351,2,10), {I(351,1,2), I(351,1,15)});    // green+white → lime
    addShapelessRecipe(I(351,2,8), {I(351,1,0), I(351,1,15)});     // black+white → gray
    addShapelessRecipe(I(351,2,7), {I(351,1,8), I(351,1,15)});     // gray+white → light gray
    addShapelessRecipe(I(351,3,7), {I(351,1,0), I(351,1,15), I(351,1,15)});  // black+2white → light gray
    addShapelessRecipe(I(351,2,12), {I(351,1,4), I(351,1,15)});    // lapis+white → light blue
    addShapelessRecipe(I(351,2,6), {I(351,1,4), I(351,1,2)});      // lapis+green → cyan
    addShapelessRecipe(I(351,2,5), {I(351,1,4), I(351,1,1)});      // lapis+red → purple
    addShapelessRecipe(I(351,2,13), {I(351,1,5), I(351,1,9)});     // purple+pink → magenta
    addShapelessRecipe(I(351,3,13), {I(351,1,4), I(351,1,1), I(351,1,9)});   // lapis+red+pink → magenta
    addShapelessRecipe(I(351,4,13), {I(351,1,4), I(351,1,1), I(351,1,1), I(351,1,15)});  // lapis+2red+white → magenta

    // Flower variants → dye (red_flower subtypes)
    addShapelessRecipe(I(351,1,12), {I(38,1,1)});    // blue orchid → light blue
    addShapelessRecipe(I(351,1,13), {I(38,1,2)});    // allium → magenta
    addShapelessRecipe(I(351,1,7), {I(38,1,3)});     // azure bluet → light gray
    addShapelessRecipe(I(351,1,1), {I(38,1,4)});     // red tulip → rose red
    addShapelessRecipe(I(351,1,14), {I(38,1,5)});    // orange tulip → orange
    addShapelessRecipe(I(351,1,7), {I(38,1,6)});     // white tulip → light gray
    addShapelessRecipe(I(351,1,9), {I(38,1,7)});     // pink tulip → pink
    addShapelessRecipe(I(351,1,7), {I(38,1,8)});     // oxeye daisy → light gray

    // Double plant → dye
    addShapelessRecipe(I(351,2,11), {I(175,1,0)});   // sunflower → 2 yellow
    addShapelessRecipe(I(351,2,13), {I(175,1,1)});   // lilac → 2 magenta
    addShapelessRecipe(I(351,2,1), {I(175,1,4)});    // rose bush → 2 red
    addShapelessRecipe(I(351,2,9), {I(175,1,5)});    // peony → 2 pink

    // ═══════════════════════════════════════════════════════════════════
    // RecipesFood.java — Food crafting recipes
    // ═══════════════════════════════════════════════════════════════════

    // Mushroom stew (bowl+brown_mushroom+red_mushroom → mushroom_stew)
    addShapelessRecipe(I(282,1,0), {I(39,1,0), I(40,1,0), I(281,1,0)});

    // Cookies (wheat+cocoa+wheat → 8 cookies)
    // Item 357=cookie, 351:3=cocoa beans, 296=wheat
    shaped13({I(296,1,0), I(351,1,3), I(296,1,0)}, I(357,8,0));

    // Melon block (9 melon slices → melon block)
    // Block 103=melon_block, Item 360=melon_slice
    shaped3({I(360,1,0), I(360,1,0), I(360,1,0),
             I(360,1,0), I(360,1,0), I(360,1,0),
             I(360,1,0), I(360,1,0), I(360,1,0)}, I(103,1,0));

    // Melon seeds (melon slice → melon seeds)
    addShapelessRecipe(I(362,1,0), {I(360,1,0)});

    // Pumpkin seeds (pumpkin → 4 pumpkin seeds)
    addShapelessRecipe(I(361,4,0), {I(86,1,0)});

    // Pumpkin pie (pumpkin+sugar+egg → pumpkin pie)
    addShapelessRecipe(I(400,1,0), {I(86,1,0), I(353,1,0), I(344,1,0)});

    // Fermented spider eye (spider_eye+brown_mushroom+sugar)
    addShapelessRecipe(I(376,1,0), {I(375,1,0), I(39,1,0), I(353,1,0)});

    // Blaze powder (blaze rod → 2)
    addShapelessRecipe(I(377,2,0), {I(369,1,0)});

    // Magma cream (blaze powder + slime ball)
    addShapelessRecipe(I(378,1,0), {I(377,1,0), I(341,1,0)});

    // Bread (3 wheat → bread)
    shaped13({I(296,1,0), I(296,1,0), I(296,1,0)}, I(297,1,0));

    // Sugar (sugar cane → sugar)
    addShapelessRecipe(I(353,1,0), {I(338,1,0)});

    // Cake (milk+sugar+egg+wheat → cake)
    // 335=bucket_of_milk, 353=sugar, 344=egg, 296=wheat
    shaped3({I(335,1,0), I(335,1,0), I(335,1,0),
             I(353,1,0), I(344,1,0), I(353,1,0),
             I(296,1,0), I(296,1,0), I(296,1,0)}, I(354,1,0));

    // Golden apple (8 gold ingot + apple → golden apple)
    shaped3({I(266,1,0), I(266,1,0), I(266,1,0),
             I(266,1,0), I(260,1,0), I(266,1,0),
             I(266,1,0), I(266,1,0), I(266,1,0)}, I(322,1,0));

    // Golden carrot (8 gold nugget + carrot)
    shaped3({I(371,1,0), I(371,1,0), I(371,1,0),
             I(371,1,0), I(391,1,0), I(371,1,0),
             I(371,1,0), I(371,1,0), I(371,1,0)}, I(396,1,0));

    // Glistering melon (gold nugget + melon slice)
    addShapelessRecipe(I(382,1,0), {I(371,1,0), I(360,1,0)});

    // ═══════════════════════════════════════════════════════════════════
    // RecipesCrafting.java — Utility blocks
    // ═══════════════════════════════════════════════════════════════════

    // Iron bars (6 iron → 16 bars)  Block 101
    shaped32({I(265,1,0), I(265,1,0), I(265,1,0),
              I(265,1,0), I(265,1,0), I(265,1,0)}, I(101,16,0));

    // Glass pane (6 glass → 16 panes)  Block 102
    shaped32({I(20,1,0), I(20,1,0), I(20,1,0),
              I(20,1,0), I(20,1,0), I(20,1,0)}, I(102,16,0));

    // Redstone lamp (4 redstone + glowstone)  Block 123
    shaped3({N(),       I(331,1,0), N(),
             I(331,1,0), I(89,1,0),  I(331,1,0),
             N(),       I(331,1,0), N()}, I(123,1,0));

    // Stone brick (4 stone → 4 stone brick)  Block 98
    shaped22({I(1,1,0), I(1,1,0),
              I(1,1,0), I(1,1,0)}, I(98,4,0));

    // Nether brick block (4 nether brick items → 1 block)  Block 112, item 405
    shaped22({I(405,1,0), I(405,1,0),
              I(405,1,0), I(405,1,0)}, I(112,1,0));

    // Sandstone smooth (4 sandstone → 4 smooth)
    shaped22({I(24,1,0), I(24,1,0),
              I(24,1,0), I(24,1,0)}, I(24,4,2));

    // Quartz pillar (2 quartz → 2 pillar)
    shaped12({I(155,1,0), I(155,1,0)}, I(155,2,2));

    // ═══════════════════════════════════════════════════════════════════
    // RecipesIngots.java — Block↔Ingot conversions (missing ones)
    // ═══════════════════════════════════════════════════════════════════

    // Hay block (9 wheat → hay block 170)
    shaped3({I(296,1,0), I(296,1,0), I(296,1,0),
             I(296,1,0), I(296,1,0), I(296,1,0),
             I(296,1,0), I(296,1,0), I(296,1,0)}, I(170,1,0));

    // Hay block → 9 wheat
    addShapelessRecipe(I(296,9,0), {I(170,1,0)});

    // Coal block (9 coal → coal block 173)
    shaped3({I(263,1,0), I(263,1,0), I(263,1,0),
             I(263,1,0), I(263,1,0), I(263,1,0),
             I(263,1,0), I(263,1,0), I(263,1,0)}, I(173,1,0));

    // Coal block → 9 coal
    addShapelessRecipe(I(263,9,0), {I(173,1,0)});

    // ═══════════════════════════════════════════════════════════════════
    // Misc item recipes from CraftingManager.java
    // ═══════════════════════════════════════════════════════════════════

    // Paper (3 sugar cane → 3 paper)  Item 339, sugar_cane 338
    shaped13({I(338,1,0), I(338,1,0), I(338,1,0)}, I(339,3,0));

    // Book (3 paper + leather → book)  Item 340, paper 339, leather 334
    addShapelessRecipe(I(340,1,0), {I(339,1,0), I(339,1,0), I(339,1,0), I(334,1,0)});

    // Map (8 paper + compass → empty map)  Item 395, compass 345
    shaped3({I(339,1,0), I(339,1,0), I(339,1,0),
             I(339,1,0), I(345,1,0), I(339,1,0),
             I(339,1,0), I(339,1,0), I(339,1,0)}, I(395,1,0));

    // Compass (4 iron + redstone → compass)  Item 345
    shaped3({N(),       I(265,1,0), N(),
             I(265,1,0), I(331,1,0), I(265,1,0),
             N(),       I(265,1,0), N()}, I(345,1,0));

    // Clock (4 gold + redstone → clock)  Item 347
    shaped3({N(),       I(266,1,0), N(),
             I(266,1,0), I(331,1,0), I(266,1,0),
             N(),       I(266,1,0), N()}, I(347,1,0));

    // Piston (planks + cobble + iron + redstone)  Block 33
    shaped3({I(5,1,W),  I(5,1,W),   I(5,1,W),
             I(4,1,0),  I(265,1,0), I(4,1,0),
             I(4,1,0),  I(331,1,0), I(4,1,0)}, I(33,1,0));

    // Sticky piston (slime ball + piston)  Block 29
    shaped12({I(341,1,0), I(33,1,0)}, I(29,1,0));

    // TNT (5 gunpowder + 4 sand)  Block 46, gunpowder 289
    shaped3({I(289,1,0), I(12,1,0), I(289,1,0),
             I(12,1,0), I(289,1,0), I(12,1,0),
             I(289,1,0), I(12,1,0), I(289,1,0)}, I(46,1,0));

    // Ladder (7 sticks → 3 ladders)  Block 65
    shaped3({I(280,1,0), N(),       I(280,1,0),
             I(280,1,0), I(280,1,0), I(280,1,0),
             I(280,1,0), N(),       I(280,1,0)}, I(65,3,0));

    // Rail (6 iron + stick → 16 rails)  Block 66
    shaped3({I(265,1,0), N(),       I(265,1,0),
             I(265,1,0), I(280,1,0), I(265,1,0),
             I(265,1,0), N(),       I(265,1,0)}, I(66,16,0));

    // Powered rail (6 gold + stick + redstone → 6 rails)  Block 27
    shaped3({I(266,1,0), N(),       I(266,1,0),
             I(266,1,0), I(280,1,0), I(266,1,0),
             I(266,1,0), I(331,1,0), I(266,1,0)}, I(27,6,0));

    // Minecart (5 iron)  Item 328
    shaped3({N(),       N(),       N(),
             I(265,1,0), N(),       I(265,1,0),
             I(265,1,0), I(265,1,0), I(265,1,0)}, I(328,1,0));

    // Chest minecart
    addShapelessRecipe(I(342,1,0), {I(328,1,0), I(54,1,0)});

    // Furnace minecart
    addShapelessRecipe(I(343,1,0), {I(328,1,0), I(61,1,0)});

    // Boat (5 planks)  Item 333
    shaped3({N(),       N(),       N(),
             I(5,1,W),  N(),       I(5,1,W),
             I(5,1,W),  I(5,1,W),  I(5,1,W)}, I(333,1,0));

    // Fishing rod (3 sticks + 2 string)  Item 346
    shaped3({N(),       N(),       I(280,1,0),
             N(),       I(280,1,0), I(287,1,0),
             I(280,1,0), N(),       I(287,1,0)}, I(346,1,0));

    // Shears (2 iron)  Item 359
    shaped22({N(),       I(265,1,0),
              I(265,1,0), N()}, I(359,1,0));

    // Flint and steel (iron + flint)  Item 259
    addShapelessRecipe(I(259,1,0), {I(265,1,0), I(318,1,0)});

    // Bowl (3 planks → 4 bowls)  Item 281
    shaped3({N(),       N(),       N(),
             I(5,1,W),  N(),       I(5,1,W),
             N(),       I(5,1,W),  N()}, I(281,4,0));

    // Eye of Ender (blaze powder + ender pearl)  Item 381
    addShapelessRecipe(I(381,1,0), {I(377,1,0), I(368,1,0)});

    // Fire charge (blaze powder + coal + gunpowder → 3)  Item 385
    addShapelessRecipe(I(385,3,0), {I(377,1,0), I(263,1,0), I(289,1,0)});

    // Flower pot (3 bricks)  Item 390
    shaped3({N(),       N(),       N(),
             I(336,1,0), N(),       I(336,1,0),
             N(),       I(336,1,0), N()}, I(390,1,0));

    // Item frame (8 sticks + leather)  Item 389
    shaped3({I(280,1,0), I(280,1,0), I(280,1,0),
             I(280,1,0), I(334,1,0), I(280,1,0),
             I(280,1,0), I(280,1,0), I(280,1,0)}, I(389,1,0));

    // Painting (8 sticks + wool)  Item 321
    shaped3({I(280,1,0), I(280,1,0), I(280,1,0),
             I(280,1,0), I(35,1,W),  I(280,1,0),
             I(280,1,0), I(280,1,0), I(280,1,0)}, I(321,1,0));

    // Lead (4 string + slime ball → 2 leads)  Item 420
    shaped3({I(287,1,0), I(287,1,0), N(),
             I(287,1,0), I(341,1,0), N(),
             N(),       N(),       I(287,1,0)}, I(420,2,0));

    // Acacia stairs (Block ID=163, acacia_planks=5:4)
    shaped3({I(5,1,4), N(),      N(),
             I(5,1,4), I(5,1,4), N(),
             I(5,1,4), I(5,1,4), I(5,1,4)}, I(163,4,0));

    // Dark oak stairs (Block ID=164, dark_oak_planks=5:5)
    shaped3({I(5,1,5), N(),      N(),
             I(5,1,5), I(5,1,5), N(),
             I(5,1,5), I(5,1,5), I(5,1,5)}, I(164,4,0));

    // Torch (charcoal/coal + stick → 4 torches)  Block 50
    shaped12({I(263,1,0), I(280,1,0)}, I(50,4,0));

    // Snow block (4 snowballs → snow block)  Block 80, snowball 332
    shaped22({I(332,1,0), I(332,1,0),
              I(332,1,0), I(332,1,0)}, I(80,1,0));

    // Clay block (4 clay balls → clay block)  Block 82, clay_ball 337
    shaped22({I(337,1,0), I(337,1,0),
              I(337,1,0), I(337,1,0)}, I(82,1,0));

    // Glowstone (4 glowstone dust → glowstone)  Block 89, dust 348
    shaped22({I(348,1,0), I(348,1,0),
              I(348,1,0), I(348,1,0)}, I(89,1,0));

    // Bookshelf (6 planks + 3 books → bookshelf)  Block 47
    shaped3({I(5,1,W),  I(5,1,W),  I(5,1,W),
             I(340,1,0), I(340,1,0), I(340,1,0),
             I(5,1,W),  I(5,1,W),  I(5,1,W)}, I(47,1,0));

    // Jack o'Lantern (pumpkin + torch → jack o'lantern)  Block 91
    addShapelessRecipe(I(91,1,0), {I(86,1,0), I(50,1,0)});

    // Ender eye recipe (enderpearl + blaze_powder → ender eye)
    // Already added above. Skip duplicate.

    // Bed (3 wool + 3 planks)  Item 355
    shaped32({I(35,1,W), I(35,1,W), I(35,1,W),
              I(5,1,W),  I(5,1,W),  I(5,1,W)}, I(355,1,0));

    // Armor stand — not in 1.7.10, skip

    // ═══════════════════════════════════════════════════════════════════
    // Slabs — 3 planks/stone → 6 slabs (Java: Block.registerBlocks slabs)
    // Wood slabs: ID 126 meta 0-5, Stone slabs: ID 44 meta 0-7
    // ═══════════════════════════════════════════════════════════════════

    // Wood slabs (3 planks → 6 slabs for each wood type)
    for (int32_t wt = 0; wt < 6; ++wt)
        shaped13({I(5,1,wt), I(5,1,wt), I(5,1,wt)}, I(126,6,wt));

    // Stone slab (3 stone → 6 stone slabs)  meta 0
    shaped13({I(1,1,0), I(1,1,0), I(1,1,0)}, I(44,6,0));

    // Sandstone slab  meta 1
    shaped13({I(24,1,0), I(24,1,0), I(24,1,0)}, I(44,6,1));

    // Cobblestone slab  meta 3
    shaped13({I(4,1,0), I(4,1,0), I(4,1,0)}, I(44,6,3));

    // Brick slab  meta 4
    shaped13({I(45,1,0), I(45,1,0), I(45,1,0)}, I(44,6,4));

    // Stone brick slab  meta 5
    shaped13({I(98,1,0), I(98,1,0), I(98,1,0)}, I(44,6,5));

    // Nether brick slab  meta 6
    shaped13({I(112,1,0), I(112,1,0), I(112,1,0)}, I(44,6,6));

    // Quartz slab  meta 7
    shaped13({I(155,1,0), I(155,1,0), I(155,1,0)}, I(44,6,7));

    // ═══════════════════════════════════════════════════════════════════
    // Fences, Walls, Doors, and more misc items
    // ═══════════════════════════════════════════════════════════════════

    // Oak fence (4 planks + 2 sticks → 3 fences)  Block 85
    shaped32({I(5,1,0),  I(280,1,0), I(5,1,0),
              I(5,1,0),  I(280,1,0), I(5,1,0)}, I(85,3,0));

    // Fence gate (2 planks + 4 sticks → fence gate)  Block 107
    shaped32({I(280,1,0), I(5,1,0),  I(280,1,0),
              I(280,1,0), I(5,1,0),  I(280,1,0)}, I(107,1,0));

    // Cobblestone wall (6 cobblestone → 6 walls)  Block 139
    shaped32({I(4,1,0), I(4,1,0), I(4,1,0),
              I(4,1,0), I(4,1,0), I(4,1,0)}, I(139,6,0));

    // Mossy cobblestone wall (6 mossy → 6 walls)  Block 139:1
    shaped32({I(48,1,0), I(48,1,0), I(48,1,0),
              I(48,1,0), I(48,1,0), I(48,1,0)}, I(139,6,1));

    // Wooden door (6 planks → door)  Item 324
    shaped32({I(5,1,W),  I(5,1,W),
              I(5,1,W),  I(5,1,W),
              I(5,1,W),  I(5,1,W)}, I(324,1,0));

    // Iron door (6 iron → iron door)  Item 330
    shaped32({I(265,1,0), I(265,1,0),
              I(265,1,0), I(265,1,0),
              I(265,1,0), I(265,1,0)}, I(330,1,0));

    // Trapdoor (6 planks → 2 trapdoors)  Block 96
    shaped32({I(5,1,W),  I(5,1,W),  I(5,1,W),
              I(5,1,W),  I(5,1,W),  I(5,1,W)}, I(96,2,0));

    // Sign (6 planks + stick → 3 signs)  Item 323
    shaped3({I(5,1,W),  I(5,1,W),  I(5,1,W),
             I(5,1,W),  I(5,1,W),  I(5,1,W),
             N(),       I(280,1,0), N()}, I(323,3,0));

    // Bucket (3 iron)  Item 325
    shaped3({N(),       N(),       N(),
             I(265,1,0), N(),       I(265,1,0),
             N(),       I(265,1,0), N()}, I(325,1,0));

    // Arrow (flint + stick + feather → 4 arrows)  Item 262
    shaped3({N(),       I(318,1,0), N(),
             N(),       I(280,1,0), N(),
             N(),       I(288,1,0), N()}, I(262,4,0));

    // Tripwire hook (iron + stick + plank → 2)  Block 131
    shaped3({N(),       I(265,1,0), N(),
             N(),       I(280,1,0), N(),
             N(),       I(5,1,W),   N()}, I(131,2,0));

    // String to wool (4 string → wool)  Block 35
    shaped22({I(287,1,0), I(287,1,0),
              I(287,1,0), I(287,1,0)}, I(35,1,0));

    // Cauldron (7 iron)  Item 380
    shaped3({I(265,1,0), N(),       I(265,1,0),
             I(265,1,0), N(),       I(265,1,0),
             I(265,1,0), I(265,1,0), I(265,1,0)}, I(380,1,0));

    // Brick block (4 bricks → brick block)  Block 45
    shaped22({I(336,1,0), I(336,1,0),
              I(336,1,0), I(336,1,0)}, I(45,1,0));

    // Quartz block (4 quartz → quartz block)  Block 155
    shaped22({I(406,1,0), I(406,1,0),
              I(406,1,0), I(406,1,0)}, I(155,1,0));

    // Sticks (2 planks → 4 sticks)
    shaped12({I(5,1,W), I(5,1,W)}, I(280,4,0));

    // Planks (1 log → 4 planks) for each log type
    for (int32_t lt = 0; lt < 4; ++lt)
        addShapelessRecipe(I(5,4,lt), {I(17,1,lt)});
    // Log2 types (acacia=0, dark_oak=1)
    addShapelessRecipe(I(5,4,4), {I(162,1,0)}); // acacia log → acacia planks
    addShapelessRecipe(I(5,4,5), {I(162,1,1)}); // dark oak log → dark oak planks

    // ═══════════════════════════════════════════════════════════════════
    // CraftingManager parity audit — missing recipes from Java ref
    // ═══════════════════════════════════════════════════════════════════

    // Glass bottle (3 glass → 3 bottles) Item 374
    shaped3({N(),       N(),       N(),
             I(20,1,0), N(),       I(20,1,0),
             N(),       I(20,1,0), N()}, I(374,3,0));

    // Powered rail (6 gold + stick + redstone → 6) Block 27
    shaped3({I(266,1,0), N(),       I(266,1,0),
             I(266,1,0), I(280,1,0), I(266,1,0),
             I(266,1,0), I(331,1,0), I(266,1,0)}, I(27,6,0));

    // Activator rail (6 iron + torch + 2 stick → 6) Block 157
    shaped3({I(265,1,0), I(280,1,0), I(265,1,0),
             I(265,1,0), I(76,1,0),  I(265,1,0),
             I(265,1,0), I(280,1,0), I(265,1,0)}, I(157,6,0));

    // Detector rail (6 iron + stone pressure plate + redstone → 6) Block 28
    shaped3({I(265,1,0), N(),       I(265,1,0),
             I(265,1,0), I(70,1,0), I(265,1,0),
             I(265,1,0), I(331,1,0), I(265,1,0)}, I(28,6,0));

    // Chest minecart (chest + minecart → 1) Item 342
    shaped12({I(54,1,0), I(328,1,0)}, I(342,1,0));

    // Furnace minecart (furnace + minecart → 1) Item 343
    shaped12({I(61,1,0), I(328,1,0)}, I(343,1,0));

    // TNT minecart (TNT + minecart → 1) Item 407
    shaped12({I(46,1,0), I(328,1,0)}, I(407,1,0));

    // Hopper minecart (hopper + minecart → 1) Item 408
    shaped12({I(154,1,0), I(328,1,0)}, I(408,1,0));

    // Brewing stand (blaze rod + 3 cobblestone → 1) Item 379
    shaped3({N(),       I(369,1,0), N(),
             I(4,1,0), I(4,1,0),  I(4,1,0),
             N(),       N(),       N()}, I(379,1,0));

    // Enchanting table (4 obsidian + 2 diamond + book → 1) Block 116
    shaped3({N(),       I(340,1,0), N(),
             I(264,1,0), I(49,1,0),  I(264,1,0),
             I(49,1,0), I(49,1,0),  I(49,1,0)}, I(116,1,0));

    // Anvil (3 iron blocks + 4 iron ingots → 1) Block 145
    shaped3({I(42,1,0), I(42,1,0), I(42,1,0),
             N(),       I(265,1,0), N(),
             I(265,1,0), I(265,1,0), I(265,1,0)}, I(145,1,0));

    // Hopper (5 iron + chest → 1) Block 154
    shaped3({I(265,1,0), N(),       I(265,1,0),
             I(265,1,0), I(54,1,0), I(265,1,0),
             N(),       I(265,1,0), N()}, I(154,1,0));

    // Dispenser (7 cobblestone + bow + redstone → 1) Block 23
    shaped3({I(4,1,0),  I(4,1,0),  I(4,1,0),
             I(4,1,0),  I(261,1,0), I(4,1,0),
             I(4,1,0),  I(331,1,0), I(4,1,0)}, I(23,1,0));

    // Dropper (7 cobblestone + redstone → 1) Block 158
    shaped3({I(4,1,0),  I(4,1,0),  I(4,1,0),
             I(4,1,0),  N(),       I(4,1,0),
             I(4,1,0),  I(331,1,0), I(4,1,0)}, I(158,1,0));

    // Nether brick fence (6 nether brick → 6) Block 113
    shaped32({I(112,1,0), I(112,1,0), I(112,1,0),
              I(112,1,0), I(112,1,0), I(112,1,0)}, I(113,6,0));

    // Snow layer (3 snow blocks → 6 snow layers) Block 78
    shaped13({I(80,1,0), I(80,1,0), I(80,1,0)}, I(78,6,0));

    // Redstone torch (redstone + stick → 1) Block 76
    shaped12({I(331,1,0), I(280,1,0)}, I(76,1,0));

    // Repeater (2 redstone torch + redstone + 3 stone → 1) Item 356
    shaped3({N(),       N(),       N(),
             I(76,1,0), I(331,1,0), I(76,1,0),
             I(1,1,0),  I(1,1,0),  I(1,1,0)}, I(356,1,0));

    // Comparator (3 redstone torch + quartz + 3 stone → 1) Item 404
    shaped3({N(),       I(76,1,0), N(),
             I(76,1,0), I(406,1,0), I(76,1,0),
             I(1,1,0),  I(1,1,0),  I(1,1,0)}, I(404,1,0));

    // Lever (stick + cobblestone → 1) Block 69
    shaped12({I(280,1,0), I(4,1,0)}, I(69,1,0));

    // Stone button (1 stone → 1) Block 77
    addShapelessRecipe(I(77,1,0), {I(1,1,0)});

    // Wooden button (1 plank → 1) Block 143
    addShapelessRecipe(I(143,1,0), {I(5,1,W)});

    // Stone pressure plate (2 stone → 1) Block 70
    shaped12({I(1,1,0), I(1,1,0)}, I(70,1,0));

    // Wooden pressure plate (2 planks → 1) Block 72
    shaped12({I(5,1,W), I(5,1,W)}, I(72,1,0));

    // Heavy weighted pressure plate (2 iron → 1) Block 148
    shaped12({I(265,1,0), I(265,1,0)}, I(148,1,0));

    // Light weighted pressure plate (2 gold → 1) Block 147
    shaped12({I(266,1,0), I(266,1,0)}, I(147,1,0));

    // Jukebox (8 planks + diamond → 1) Block 84
    shaped3({I(5,1,W),  I(5,1,W),  I(5,1,W),
             I(5,1,W),  I(264,1,0), I(5,1,W),
             I(5,1,W),  I(5,1,W),  I(5,1,W)}, I(84,1,0));

    // Note block (8 planks + redstone → 1) Block 25
    shaped3({I(5,1,W),  I(5,1,W),  I(5,1,W),
             I(5,1,W),  I(331,1,0), I(5,1,W),
             I(5,1,W),  I(5,1,W),  I(5,1,W)}, I(25,1,0));

    // Daylight detector (3 glass + 3 quartz + 3 wood slab → 1) Block 151
    shaped3({I(20,1,0), I(20,1,0),  I(20,1,0),
             I(406,1,0), I(406,1,0), I(406,1,0),
             I(126,1,0), I(126,1,0), I(126,1,0)}, I(151,1,0));

    // Carrot on a stick (fishing rod + carrot → 1) Item 398
    addShapelessRecipe(I(398,1,0), {I(346,1,0), I(391,1,0)});

    // Writable book (book + ink sac + feather → 1) Item 386
    addShapelessRecipe(I(386,1,0), {I(340,1,0), I(351,1,0), I(288,1,0)});

    // Chest (8 planks → 1) Block 54
    shaped3({I(5,1,W),  I(5,1,W),  I(5,1,W),
             I(5,1,W),  N(),       I(5,1,W),
             I(5,1,W),  I(5,1,W),  I(5,1,W)}, I(54,1,0));

    // Furnace (8 cobblestone → 1) Block 61
    shaped3({I(4,1,0),  I(4,1,0),  I(4,1,0),
             I(4,1,0),  N(),       I(4,1,0),
             I(4,1,0),  I(4,1,0),  I(4,1,0)}, I(61,1,0));

    // Crafting table (4 planks → 1) Block 58
    shaped22({I(5,1,W), I(5,1,W),
              I(5,1,W), I(5,1,W)}, I(58,1,0));

    // Stone stairs (cobblestone → 4) Block 67
    shaped3({I(4,1,0),  N(),       N(),
             I(4,1,0),  I(4,1,0),  N(),
             I(4,1,0),  I(4,1,0),  I(4,1,0)}, I(67,4,0));

    // Brick stairs (brick block → 4) Block 108
    shaped3({I(45,1,0), N(),       N(),
             I(45,1,0), I(45,1,0), N(),
             I(45,1,0), I(45,1,0), I(45,1,0)}, I(108,4,0));

    // Stone brick stairs (stone brick → 4) Block 109
    shaped3({I(98,1,0), N(),       N(),
             I(98,1,0), I(98,1,0), N(),
             I(98,1,0), I(98,1,0), I(98,1,0)}, I(109,4,0));

    // Nether brick stairs (nether brick → 4) Block 114
    shaped3({I(112,1,0), N(),       N(),
             I(112,1,0), I(112,1,0), N(),
             I(112,1,0), I(112,1,0), I(112,1,0)}, I(114,4,0));

    // Sandstone stairs (sandstone → 4) Block 128
    shaped3({I(24,1,0), N(),       N(),
             I(24,1,0), I(24,1,0), N(),
             I(24,1,0), I(24,1,0), I(24,1,0)}, I(128,4,0));

    // Quartz stairs (quartz block → 4) Block 156
    shaped3({I(155,1,0), N(),       N(),
             I(155,1,0), I(155,1,0), N(),
             I(155,1,0), I(155,1,0), I(155,1,0)}, I(156,4,0));

    // Bow (3 string + 3 sticks → 1) Item 261
    shaped3({N(),       I(280,1,0), I(287,1,0),
             I(280,1,0), N(),       I(287,1,0),
             N(),       I(280,1,0), I(287,1,0)}, I(261,1,0));

    // Fence (6 sticks → 2) Block 85 — Java: stick variant
    // Note: Java fence uses sticks only (our earlier one used planks+sticks)

    // Golden apple enchanted (8 gold blocks + apple → 1) Item 322:1
    shaped3({I(41,1,0), I(41,1,0), I(41,1,0),
             I(41,1,0), I(260,1,0), I(41,1,0),
             I(41,1,0), I(41,1,0), I(41,1,0)}, I(322,1,1));

    // ═══════════════════════════════════════════════════════════════════
    // RecipesDyes — Java ref: net.minecraft.item.crafting.RecipesDyes
    // func_150031_c(n) = 15 - n (dye damage → wool/clay/glass meta)
    // ═══════════════════════════════════════════════════════════════════

    // 16 dye colors: wool dyeing, stained clay, stained glass, glass panes, carpets
    for (int32_t d = 0; d < 16; ++d) {
        int32_t colorMeta = 15 - d; // func_150031_c

        // Wool dyeing: dye + white wool → colored wool
        // Block 35 (wool), Item 351 (dye)
        addShapelessRecipe(I(35, 1, colorMeta), {I(351, 1, d), I(35, 1, 0)});

        // Stained hardened clay: 8 hardened clay + dye → 8 stained clay
        // Block 159 (stained_hardened_clay), Block 172 (hardened_clay)
        shaped3({I(172,1,0), I(172,1,0), I(172,1,0),
                 I(172,1,0), I(351,1,d), I(172,1,0),
                 I(172,1,0), I(172,1,0), I(172,1,0)}, I(159, 8, colorMeta));

        // Stained glass: 8 glass + dye → 8 stained glass
        // Block 95 (stained_glass), Block 20 (glass)
        shaped3({I(20,1,0), I(20,1,0), I(20,1,0),
                 I(20,1,0), I(351,1,d), I(20,1,0),
                 I(20,1,0), I(20,1,0), I(20,1,0)}, I(95, 8, colorMeta));

        // Stained glass pane: 6 stained glass → 16 stained glass panes
        // Block 160 (stained_glass_pane)
        shaped32({I(95,1,d), I(95,1,d), I(95,1,d),
                  I(95,1,d), I(95,1,d), I(95,1,d)}, I(160, 16, d));

        // Carpet: 2 wool → 3 carpet
        // Block 171 (carpet)
        shaped12({I(35,1,d), I(35,1,d)}, I(171, 3, d));
    }

    // Dye from flowers:
    addShapelessRecipe(I(351,1,11), {I(37,1,0)});  // yellow flower → dandelion yellow
    addShapelessRecipe(I(351,1,1),  {I(38,1,0)});  // red flower (poppy) → rose red
    addShapelessRecipe(I(351,3,15), {I(352,1,0)});  // bone → 3 bonemeal  (item 352=bone)

    // Dye mixing:
    addShapelessRecipe(I(351,2,9),  {I(351,1,1), I(351,1,15)});  // red + white → pink
    addShapelessRecipe(I(351,2,14), {I(351,1,1), I(351,1,11)});  // red + yellow → orange
    addShapelessRecipe(I(351,2,10), {I(351,1,2), I(351,1,15)});  // green + white → lime
    addShapelessRecipe(I(351,2,8),  {I(351,1,0), I(351,1,15)});  // black + white → gray
    addShapelessRecipe(I(351,2,7),  {I(351,1,8), I(351,1,15)});  // gray + white → light gray
    addShapelessRecipe(I(351,2,12), {I(351,1,4), I(351,1,15)});  // lapis + white → light blue
    addShapelessRecipe(I(351,2,6),  {I(351,1,4), I(351,1,2)});   // lapis + green → cyan
    addShapelessRecipe(I(351,2,5),  {I(351,1,4), I(351,1,1)});   // lapis + red → purple
    addShapelessRecipe(I(351,2,13), {I(351,1,5), I(351,1,9)});   // purple + pink → magenta

    // Light gray from 3 inputs:
    addShapelessRecipe(I(351,3,7), {I(351,1,0), I(351,1,15), I(351,1,15)});

    // Magenta alternatives:
    addShapelessRecipe(I(351,3,13), {I(351,1,4), I(351,1,1), I(351,1,9)});
    addShapelessRecipe(I(351,4,13), {I(351,1,4), I(351,1,1), I(351,1,1), I(351,1,15)});

    // Flowers → dye (red_flower subtypes):
    addShapelessRecipe(I(351,1,12), {I(38,1,1)});  // blue orchid → light blue
    addShapelessRecipe(I(351,1,13), {I(38,1,2)});  // allium → magenta
    addShapelessRecipe(I(351,1,7),  {I(38,1,3)});  // azure bluet → light gray
    addShapelessRecipe(I(351,1,1),  {I(38,1,4)});  // red tulip → rose red
    addShapelessRecipe(I(351,1,14), {I(38,1,5)});  // orange tulip → orange
    addShapelessRecipe(I(351,1,7),  {I(38,1,6)});  // white tulip → light gray
    addShapelessRecipe(I(351,1,9),  {I(38,1,7)});  // pink tulip → pink
    addShapelessRecipe(I(351,1,7),  {I(38,1,8)});  // oxeye daisy → light gray

    // Double plants → 2 dye:
    addShapelessRecipe(I(351,2,11), {I(175,1,0)});  // sunflower → 2 yellow
    addShapelessRecipe(I(351,2,13), {I(175,1,1)});  // lilac → 2 magenta
    addShapelessRecipe(I(351,2,1),  {I(175,1,4)});  // rose bush → 2 red
    addShapelessRecipe(I(351,2,9),  {I(175,1,5)});  // peony → 2 pink

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
