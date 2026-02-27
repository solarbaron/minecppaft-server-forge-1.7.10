#pragma once
// CraftingManager — recipe matching for 2x2 (player inventory) and 3x3 (workbench).
// Ported from the obfuscated crafting classes in reference-java.
// The CraftingManager singleton holds all shaped and shapeless recipes.
// Shaped recipes match exact grid patterns; shapeless match any arrangement.

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <optional>
#include <algorithm>

#include "inventory/Inventory.h"

namespace mc {

// A crafting recipe ingredient
struct CraftingIngredient {
    int16_t itemId = 0;
    int16_t metadata = -1; // -1 = any metadata

    bool matches(int16_t id, int16_t meta) const {
        if (itemId == 0) return id == 0; // empty slot matches empty
        if (id != itemId) return false;
        return metadata == -1 || metadata == meta;
    }
};

// Shaped recipe — grid pattern must match exactly (can be offset)
struct ShapedRecipe {
    int width = 0, height = 0;
    std::vector<CraftingIngredient> ingredients; // width * height
    ItemStack result;

    // Check if this recipe matches a grid (gridW x gridH)
    bool matches(const std::vector<ItemStack>& grid, int gridW, int gridH) const {
        // Try all offsets
        for (int ox = 0; ox <= gridW - width; ++ox) {
            for (int oy = 0; oy <= gridH - height; ++oy) {
                if (matchesAt(grid, gridW, gridH, ox, oy, false)) return true;
                if (matchesAt(grid, gridW, gridH, ox, oy, true)) return true; // mirrored
            }
        }
        return false;
    }

private:
    bool matchesAt(const std::vector<ItemStack>& grid, int gridW, int gridH,
                   int offX, int offY, bool mirror) const {
        for (int gx = 0; gx < gridW; ++gx) {
            for (int gy = 0; gy < gridH; ++gy) {
                int rx = gx - offX;
                int ry = gy - offY;

                CraftingIngredient expected;
                if (rx >= 0 && rx < width && ry >= 0 && ry < height) {
                    int idx = mirror ? ((width - 1 - rx) + ry * width)
                                     : (rx + ry * width);
                    expected = ingredients[idx];
                }

                auto& slot = grid[gx + gy * gridW];
                if (!expected.matches(slot.itemId, slot.damage)) {
                    return false;
                }
            }
        }
        return true;
    }
};

// Shapeless recipe — any arrangement of ingredients
struct ShapelessRecipe {
    std::vector<CraftingIngredient> ingredients;
    ItemStack result;

    bool matches(const std::vector<ItemStack>& grid, int gridW, int gridH) const {
        std::vector<bool> used(ingredients.size(), false);
        int filledSlots = 0;

        for (int i = 0; i < gridW * gridH; ++i) {
            auto& slot = grid[i];
            if (slot.isEmpty()) continue;
            ++filledSlots;

            bool found = false;
            for (size_t j = 0; j < ingredients.size(); ++j) {
                if (!used[j] && ingredients[j].matches(slot.itemId, slot.damage)) {
                    used[j] = true;
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }

        return filledSlots == static_cast<int>(ingredients.size());
    }
};

// The CraftingManager — holds all recipes and finds matches
class CraftingManager {
public:
    CraftingManager() { registerVanillaRecipes(); }

    // Find matching recipe for a crafting grid
    std::optional<ItemStack> findMatch(const std::vector<ItemStack>& grid,
                                        int gridW, int gridH) const {
        for (auto& r : shapedRecipes_) {
            if (r.matches(grid, gridW, gridH)) return r.result;
        }
        for (auto& r : shapelessRecipes_) {
            if (r.matches(grid, gridW, gridH)) return r.result;
        }
        return std::nullopt;
    }

    // Singleton instance
    static CraftingManager& instance() {
        static CraftingManager mgr;
        return mgr;
    }

private:
    std::vector<ShapedRecipe> shapedRecipes_;
    std::vector<ShapelessRecipe> shapelessRecipes_;

    // Helper to add shaped recipe
    void addShaped(int w, int h, std::vector<CraftingIngredient> ing, ItemStack result) {
        shapedRecipes_.push_back({w, h, std::move(ing), std::move(result)});
    }

    // Helper to add shapeless recipe
    void addShapeless(std::vector<CraftingIngredient> ing, ItemStack result) {
        shapelessRecipes_.push_back({std::move(ing), std::move(result)});
    }

    // Shorthand
    static CraftingIngredient I(int16_t id, int16_t meta = -1) { return {id, meta}; }
    static CraftingIngredient E() { return {0, -1}; } // empty

    void registerVanillaRecipes() {
        // === Wood & Basic ===
        // Planks from log (shapeless, any log type)
        addShapeless({I(17)}, ItemStack(5, 4, 0));    // Oak log -> 4 planks
        addShapeless({I(17, 1)}, ItemStack(5, 4, 1)); // Spruce
        addShapeless({I(17, 2)}, ItemStack(5, 4, 2)); // Birch
        addShapeless({I(17, 3)}, ItemStack(5, 4, 3)); // Jungle

        // Sticks from planks
        addShaped(1, 2, {I(5), I(5)}, ItemStack(280, 4, 0)); // 2 planks -> 4 sticks

        // Crafting table
        addShaped(2, 2, {I(5),I(5),I(5),I(5)}, ItemStack(58, 1, 0));

        // Furnace
        addShaped(3, 3, {
            I(4),I(4),I(4),
            I(4),E(), I(4),
            I(4),I(4),I(4)
        }, ItemStack(61, 1, 0));

        // Chest
        addShaped(3, 3, {
            I(5),I(5),I(5),
            I(5),E(), I(5),
            I(5),I(5),I(5)
        }, ItemStack(54, 1, 0));

        // === Tools (Wooden) ===
        // Wooden pickaxe
        addShaped(3, 3, {
            I(5),  I(5),  I(5),
            E(),   I(280),E(),
            E(),   I(280),E()
        }, ItemStack(270, 1, 0));

        // Wooden axe
        addShaped(2, 3, {
            I(5), I(5),
            I(5), I(280),
            E(),  I(280)
        }, ItemStack(271, 1, 0));

        // Wooden shovel
        addShaped(1, 3, {I(5), I(280), I(280)}, ItemStack(269, 1, 0));

        // Wooden sword
        addShaped(1, 3, {I(5), I(5), I(280)}, ItemStack(268, 1, 0));

        // Wooden hoe
        addShaped(2, 3, {
            I(5), I(5),
            E(),  I(280),
            E(),  I(280)
        }, ItemStack(290, 1, 0));

        // === Tools (Stone) ===
        addShaped(3, 3, {
            I(4),  I(4),  I(4),
            E(),   I(280),E(),
            E(),   I(280),E()
        }, ItemStack(274, 1, 0)); // Stone pickaxe

        addShaped(1, 3, {I(4), I(280), I(280)}, ItemStack(273, 1, 0)); // Stone shovel

        addShaped(1, 3, {I(4), I(4), I(280)}, ItemStack(272, 1, 0)); // Stone sword

        // === Tools (Iron) ===
        addShaped(3, 3, {
            I(265),I(265),I(265),
            E(),   I(280),E(),
            E(),   I(280),E()
        }, ItemStack(257, 1, 0)); // Iron pickaxe

        addShaped(1, 3, {I(265), I(280), I(280)}, ItemStack(256, 1, 0)); // Iron shovel

        addShaped(1, 3, {I(265), I(265), I(280)}, ItemStack(267, 1, 0)); // Iron sword

        // === Tools (Diamond) ===
        addShaped(3, 3, {
            I(264),I(264),I(264),
            E(),   I(280),E(),
            E(),   I(280),E()
        }, ItemStack(278, 1, 0)); // Diamond pickaxe

        addShaped(1, 3, {I(264), I(280), I(280)}, ItemStack(277, 1, 0)); // Diamond shovel

        addShaped(1, 3, {I(264), I(264), I(280)}, ItemStack(276, 1, 0)); // Diamond sword

        // === Tools (Gold) ===
        addShaped(3, 3, {
            I(266),I(266),I(266),
            E(),   I(280),E(),
            E(),   I(280),E()
        }, ItemStack(285, 1, 0)); // Gold pickaxe

        addShaped(1, 3, {I(266), I(280), I(280)}, ItemStack(284, 1, 0)); // Gold shovel

        addShaped(1, 3, {I(266), I(266), I(280)}, ItemStack(283, 1, 0)); // Gold sword

        // === Armor (Iron) ===
        addShaped(3, 3, {
            I(265),I(265),I(265),
            I(265),E(),   I(265),
            E(),   E(),   E()
        }, ItemStack(306, 1, 0)); // Iron helmet

        addShaped(3, 3, {
            I(265),E(),   I(265),
            I(265),I(265),I(265),
            I(265),I(265),I(265)
        }, ItemStack(307, 1, 0)); // Iron chestplate

        addShaped(3, 3, {
            I(265),I(265),I(265),
            I(265),E(),   I(265),
            I(265),E(),   I(265)
        }, ItemStack(308, 1, 0)); // Iron leggings

        addShaped(3, 3, {
            E(),   E(),   E(),
            I(265),E(),   I(265),
            I(265),E(),   I(265)
        }, ItemStack(309, 1, 0)); // Iron boots

        // === Armor (Diamond) ===
        addShaped(3, 3, {
            I(264),I(264),I(264),
            I(264),E(),   I(264),
            E(),   E(),   E()
        }, ItemStack(310, 1, 0)); // Diamond helmet

        addShaped(3, 3, {
            I(264),E(),   I(264),
            I(264),I(264),I(264),
            I(264),I(264),I(264)
        }, ItemStack(311, 1, 0)); // Diamond chestplate

        addShaped(3, 3, {
            I(264),I(264),I(264),
            I(264),E(),   I(264),
            I(264),E(),   I(264)
        }, ItemStack(312, 1, 0)); // Diamond leggings

        addShaped(3, 3, {
            E(),   E(),   E(),
            I(264),E(),   I(264),
            I(264),E(),   I(264)
        }, ItemStack(313, 1, 0)); // Diamond boots

        // === Blocks ===
        // Iron block (9 ingots)
        addShaped(3, 3, {
            I(265),I(265),I(265),
            I(265),I(265),I(265),
            I(265),I(265),I(265)
        }, ItemStack(42, 1, 0));

        // Gold block
        addShaped(3, 3, {
            I(266),I(266),I(266),
            I(266),I(266),I(266),
            I(266),I(266),I(266)
        }, ItemStack(41, 1, 0));

        // Diamond block
        addShaped(3, 3, {
            I(264),I(264),I(264),
            I(264),I(264),I(264),
            I(264),I(264),I(264)
        }, ItemStack(57, 1, 0));

        // Ingots from blocks (shapeless)
        addShapeless({I(42)}, ItemStack(265, 9, 0));  // Iron block -> 9 ingots
        addShapeless({I(41)}, ItemStack(266, 9, 0));  // Gold block -> 9 ingots
        addShapeless({I(57)}, ItemStack(264, 9, 0));  // Diamond block -> 9 diamonds

        // === Miscellaneous ===
        // Torch
        addShaped(1, 2, {I(263), I(280)}, ItemStack(50, 4, 0)); // Coal + stick

        // Ladder
        addShaped(3, 3, {
            I(280),E(),   I(280),
            I(280),I(280),I(280),
            I(280),E(),   I(280)
        }, ItemStack(65, 3, 0));

        // Bow
        addShaped(3, 3, {
            E(),   I(280),I(287),
            I(280),E(),   I(287),
            E(),   I(280),I(287)
        }, ItemStack(261, 1, 0));

        // Arrow (flint + stick + feather)
        addShaped(1, 3, {I(318), I(280), I(288)}, ItemStack(262, 4, 0));

        // Stone button (shapeless)
        addShapeless({I(1)}, ItemStack(77, 1, 0));

        // Sandstone (4 sand)
        addShaped(2, 2, {I(12),I(12),I(12),I(12)}, ItemStack(24, 1, 0));

        // Snow block
        addShaped(2, 2, {I(332),I(332),I(332),I(332)}, ItemStack(80, 1, 0));

        // Bookshelf
        addShaped(3, 3, {
            I(5),  I(5),  I(5),
            I(340),I(340),I(340),
            I(5),  I(5),  I(5)
        }, ItemStack(47, 1, 0));

        // Book (paper + leather)
        addShapeless({I(339), I(339), I(339), I(334)}, ItemStack(340, 1, 0));

        // Paper (3 sugar cane)
        addShaped(3, 1, {I(338), I(338), I(338)}, ItemStack(339, 3, 0));

        // Bucket (3 iron ingots)
        addShaped(3, 2, {
            I(265),E(),   I(265),
            E(),   I(265),E()
        }, ItemStack(325, 1, 0));

        // Bread (3 wheat)
        addShaped(3, 1, {I(296), I(296), I(296)}, ItemStack(297, 1, 0));

        // Bowl (3 planks)
        addShaped(3, 2, {
            I(5), E(),  I(5),
            E(),  I(5), E()
        }, ItemStack(281, 4, 0));

        // Mushroom stew (shapeless)
        addShapeless({I(281), I(39), I(40)}, ItemStack(282, 1, 0));

        // Sugar (sugar cane)
        addShapeless({I(338)}, ItemStack(353, 1, 0));

        // Cake
        addShaped(3, 3, {
            I(335),I(335),I(335),
            I(353),I(344),I(353),
            I(296),I(296),I(296)
        }, ItemStack(354, 1, 0));

        // Bed
        addShaped(3, 2, {
            I(35), I(35), I(35),
            I(5),  I(5),  I(5)
        }, ItemStack(355, 1, 0));
    }
};

} // namespace mc
