/**
 * Crafting.h — Crafting and smelting recipe system.
 *
 * Java references:
 *   - net.minecraft.item.crafting.IRecipe
 *   - net.minecraft.item.crafting.ShapedRecipes
 *   - net.minecraft.item.crafting.ShapelessRecipes
 *   - net.minecraft.item.crafting.CraftingManager
 *   - net.minecraft.item.crafting.FurnaceRecipes
 *
 * Thread safety:
 *   - CraftingManager and FurnaceRecipes are initialized once at startup.
 *   - Recipe matching is read-only and inherently thread-safe.
 *
 * JNI readiness: Integer item IDs, simple data structures. Recipe lists can
 * be enumerated via index for JVM binding.
 */
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include <unordered_map>

#include "inventory/Inventory.h"

namespace mccpp {

// ─── Crafting Grid ─────────────────────────────────────────────────────────
// Simplified InventoryCrafting — a flat 3×3 or 2×2 grid of ItemStacks.
// Java reference: net.minecraft.inventory.InventoryCrafting

class CraftingGrid {
public:
    static constexpr int32_t GRID_SIZE = 3;  // 3×3 crafting table (2×2 for player)

    CraftingGrid(int32_t width = GRID_SIZE, int32_t height = GRID_SIZE)
        : width_(width), height_(height), items_(width * height, std::nullopt) {}

    // Java: InventoryCrafting.getStackInRowAndColumn(int col, int row)
    std::optional<ItemStack> getStackInRowAndColumn(int32_t col, int32_t row) const {
        if (col >= 0 && col < width_ && row >= 0 && row < height_) {
            return items_[col + row * width_];
        }
        return std::nullopt;
    }

    void setStack(int32_t col, int32_t row, const std::optional<ItemStack>& stack) {
        if (col >= 0 && col < width_ && row >= 0 && row < height_) {
            items_[col + row * width_] = stack;
        }
    }

    int32_t getWidth() const { return width_; }
    int32_t getHeight() const { return height_; }
    int32_t getSizeInventory() const { return static_cast<int32_t>(items_.size()); }

    std::optional<ItemStack> getStackInSlot(int32_t index) const {
        if (index >= 0 && index < static_cast<int32_t>(items_.size())) return items_[index];
        return std::nullopt;
    }

private:
    int32_t width_, height_;
    std::vector<std::optional<ItemStack>> items_;
};

// ─── IRecipe ───────────────────────────────────────────────────────────────
// Java reference: net.minecraft.item.crafting.IRecipe

class IRecipe {
public:
    virtual ~IRecipe() = default;

    // Java: IRecipe.matches(InventoryCrafting, World)
    virtual bool matches(const CraftingGrid& grid) const = 0;

    // Java: IRecipe.getCraftingResult(InventoryCrafting)
    virtual ItemStack getCraftingResult(const CraftingGrid& grid) const = 0;

    // Java: IRecipe.getRecipeOutput()
    virtual ItemStack getRecipeOutput() const = 0;

    // Java: IRecipe.getRecipeSize()
    virtual int32_t getRecipeSize() const = 0;
};

// ─── ShapedRecipes ─────────────────────────────────────────────────────────
// Java reference: net.minecraft.item.crafting.ShapedRecipes
//
// Grid-based recipe with width×height pattern. Supports mirroring.
// Pattern items use damage=32767 (Short.MAX_VALUE) for wildcard matching.

class ShapedRecipes : public IRecipe {
public:
    // Java: ShapedRecipes(int width, int height, ItemStack[] pattern, ItemStack output)
    ShapedRecipes(int32_t width, int32_t height,
                  std::vector<std::optional<ItemStack>> pattern,
                  ItemStack output)
        : recipeWidth_(width), recipeHeight_(height)
        , recipeItems_(std::move(pattern))
        , recipeOutput_(std::move(output)) {}

    bool matches(const CraftingGrid& grid) const override;
    ItemStack getCraftingResult(const CraftingGrid& grid) const override;
    ItemStack getRecipeOutput() const override { return recipeOutput_; }
    int32_t getRecipeSize() const override { return recipeWidth_ * recipeHeight_; }

private:
    // Java: ShapedRecipes.checkMatch(InventoryCrafting, int, int, boolean)
    bool checkMatch(const CraftingGrid& grid, int32_t startX, int32_t startY, bool mirror) const;

    int32_t recipeWidth_;
    int32_t recipeHeight_;
    std::vector<std::optional<ItemStack>> recipeItems_;
    ItemStack recipeOutput_;
};

// ─── ShapelessRecipes ──────────────────────────────────────────────────────
// Java reference: net.minecraft.item.crafting.ShapelessRecipes
//
// Order-independent ingredient matching.

class ShapelessRecipes : public IRecipe {
public:
    // Java: ShapelessRecipes(ItemStack output, List<ItemStack> ingredients)
    ShapelessRecipes(ItemStack output, std::vector<ItemStack> ingredients)
        : recipeOutput_(std::move(output))
        , recipeItems_(std::move(ingredients)) {}

    bool matches(const CraftingGrid& grid) const override;
    ItemStack getCraftingResult(const CraftingGrid& grid) const override;
    ItemStack getRecipeOutput() const override { return recipeOutput_; }
    int32_t getRecipeSize() const override { return static_cast<int32_t>(recipeItems_.size()); }

private:
    ItemStack recipeOutput_;
    std::vector<ItemStack> recipeItems_;
};

// ─── CraftingManager ───────────────────────────────────────────────────────
// Java reference: net.minecraft.item.crafting.CraftingManager
//
// Singleton registry of all crafting recipes. findMatchingRecipe() iterates
// the recipe list to find the first match (order-dependent, larger recipes first).

class CraftingManager {
public:
    // Java: CraftingManager.getInstance()
    static CraftingManager& getInstance();

    // Java: CraftingManager.addRecipe(ItemStack, Object...) — simplified
    // Takes pre-built recipe objects instead of varargs
    void addRecipe(std::unique_ptr<IRecipe> recipe);

    // Java: CraftingManager.addShapelessRecipe(ItemStack, Object...)
    void addShapelessRecipe(ItemStack output, std::vector<ItemStack> ingredients);

    // Java: CraftingManager.findMatchingRecipe(InventoryCrafting, World)
    std::optional<ItemStack> findMatchingRecipe(const CraftingGrid& grid) const;

    // Java: CraftingManager.getRecipeList()
    const std::vector<std::unique_ptr<IRecipe>>& getRecipeList() const { return recipes_; }

    int32_t getRecipeCount() const { return static_cast<int32_t>(recipes_.size()); }

private:
    CraftingManager();
    std::vector<std::unique_ptr<IRecipe>> recipes_;
};

// ─── FurnaceRecipes ────────────────────────────────────────────────────────
// Java reference: net.minecraft.item.crafting.FurnaceRecipes
//
// Simple input→output+XP smelting map. Uses item ID + damage as key.
// Damage value of 32767 matches any damage (wildcard).

struct SmeltingRecipe {
    int32_t inputId;
    int32_t inputDamage;   // 32767 = any damage
    ItemStack output;
    float experience;
};

class FurnaceRecipes {
public:
    // Java: FurnaceRecipes.instance()
    static FurnaceRecipes& instance();

    // Java: FurnaceRecipes.addSmeltingRecipe(ItemStack, ItemStack, float)
    void addSmeltingRecipe(int32_t inputId, int32_t inputDamage,
                           ItemStack output, float experience);

    // Java: FurnaceRecipes.addSmelting(Item, ItemStack, float) — wildcard damage
    void addSmelting(int32_t inputId, ItemStack output, float experience) {
        addSmeltingRecipe(inputId, 32767, std::move(output), experience);
    }

    // Java: FurnaceRecipes.getSmeltingResult(ItemStack)
    std::optional<ItemStack> getSmeltingResult(int32_t itemId, int32_t damage) const;

    // Java: FurnaceRecipes.getSmeltingExperience(ItemStack)
    float getSmeltingExperience(int32_t itemId, int32_t damage) const;

    int32_t getRecipeCount() const { return static_cast<int32_t>(recipes_.size()); }

private:
    FurnaceRecipes();
    std::vector<SmeltingRecipe> recipes_;
};

} // namespace mccpp
