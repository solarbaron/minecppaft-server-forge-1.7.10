#pragma once
// FurnaceManager — smelting recipes and fuel burn times.
// Ported from vanilla furnace tile entity (aks.java / TileEntityFurnace).
//
// Vanilla burn mechanics:
//   - Fuel burns for a set number of ticks (e.g. coal = 1600 ticks = 80 sec)
//   - Each smelt takes 200 ticks (10 sec) — constant for all recipes
//   - When fuel runs out mid-smelt, progress resets backward
//   - Double output for ore -> ingot in vanilla (we simplify to 1:1)

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <optional>
#include <iostream>

#include "inventory/Inventory.h"

namespace mc {

// Smelting recipe: input item -> output item
struct SmeltingRecipe {
    int16_t inputId;
    int16_t inputMeta;   // -1 = any
    ItemStack output;
    float experience;    // XP granted per smelt
};

// Fuel burn times in ticks (from aks.java getItemBurnTime)
namespace FuelBurnTime {
    constexpr int COAL         = 1600;  // 80 seconds
    constexpr int CHARCOAL     = 1600;
    constexpr int LOG          = 300;   // 15 seconds
    constexpr int PLANK        = 300;   // 15 seconds
    constexpr int STICK        = 100;   // 5 seconds
    constexpr int WOODEN_SLAB  = 150;   // 7.5 seconds
    constexpr int LAVA_BUCKET  = 20000; // 1000 seconds
    constexpr int BLAZE_ROD    = 2400;  // 120 seconds
    constexpr int SAPLING      = 100;
    constexpr int BOOKSHELF    = 300;
    constexpr int WOODEN_TOOL  = 200;   // 10 seconds (swords, picks, etc.)
    constexpr int COAL_BLOCK   = 16000; // 800 seconds (10x coal)
}

// Smelt time is always 200 ticks in vanilla
constexpr int SMELT_TIME = 200;

class FurnaceManager {
public:
    FurnaceManager() { registerVanillaRecipes(); registerFuels(); }

    // Find smelting result for an input item
    std::optional<SmeltingRecipe> findRecipe(int16_t itemId, int16_t meta = 0) const {
        for (auto& r : recipes_) {
            if (r.inputId == itemId && (r.inputMeta == -1 || r.inputMeta == meta)) {
                return r;
            }
        }
        return std::nullopt;
    }

    // Get fuel burn time for an item (0 = not a fuel)
    int getFuelBurnTime(int16_t itemId, int16_t meta = 0) const {
        auto it = fuels_.find(itemId);
        if (it != fuels_.end()) return it->second;

        // Wood items (any damage) — check material type
        if (itemId >= 268 && itemId <= 271) return FuelBurnTime::WOODEN_TOOL; // Wood tools
        if (itemId == 290) return FuelBurnTime::WOODEN_TOOL; // Wooden hoe

        return 0;
    }

    // Singleton
    static FurnaceManager& instance() {
        static FurnaceManager mgr;
        return mgr;
    }

private:
    std::vector<SmeltingRecipe> recipes_;
    std::unordered_map<int16_t, int> fuels_;

    void addRecipe(int16_t input, ItemStack output, float xp, int16_t meta = -1) {
        recipes_.push_back({input, meta, std::move(output), xp});
    }

    void registerVanillaRecipes() {
        // === Ores -> Ingots ===
        addRecipe(14, ItemStack(266, 1, 0), 1.0f);  // Gold ore -> gold ingot
        addRecipe(15, ItemStack(265, 1, 0), 0.7f);  // Iron ore -> iron ingot
        addRecipe(56, ItemStack(264, 1, 0), 1.0f);  // Diamond ore -> diamond
        addRecipe(21, ItemStack(351, 1, 4), 0.2f);  // Lapis ore -> lapis
        addRecipe(73, ItemStack(331, 1, 0), 0.7f);  // Redstone ore -> redstone
        addRecipe(129, ItemStack(388, 1, 0), 1.0f); // Emerald ore -> emerald
        addRecipe(153, ItemStack(406, 1, 0), 0.2f); // Nether quartz ore -> quartz

        // === Coal ===
        addRecipe(16, ItemStack(263, 1, 0), 0.1f);  // Coal ore -> coal

        // === Food ===
        addRecipe(319, ItemStack(320, 1, 0), 0.35f); // Raw porkchop -> cooked
        addRecipe(363, ItemStack(364, 1, 0), 0.35f); // Raw beef -> steak
        addRecipe(365, ItemStack(366, 1, 0), 0.35f); // Raw chicken -> cooked
        addRecipe(349, ItemStack(350, 1, 0), 0.35f); // Raw fish -> cooked fish
        addRecipe(392, ItemStack(393, 1, 0), 0.35f); // Potato -> baked potato

        // === Sand -> Glass ===
        addRecipe(12, ItemStack(20, 1, 0), 0.1f);   // Sand -> glass

        // === Cobblestone -> Stone ===
        addRecipe(4, ItemStack(1, 1, 0), 0.1f);     // Cobblestone -> stone

        // === Clay -> Brick ===
        addRecipe(337, ItemStack(336, 1, 0), 0.3f);  // Clay ball -> brick
        addRecipe(82, ItemStack(172, 1, 0), 0.35f);  // Clay block -> hardened clay

        // === Wood -> Charcoal ===
        addRecipe(17, ItemStack(263, 1, 1), 0.15f);  // Log -> charcoal

        // === Netherrack -> Nether brick item ===
        addRecipe(87, ItemStack(405, 1, 0), 0.1f);   // Netherrack -> nether brick item

        // === Cactus -> Cactus green ===
        addRecipe(81, ItemStack(351, 1, 2), 0.2f);   // Cactus -> green dye

        // === Sponge (wet -> dry) ===
        addRecipe(19, ItemStack(19, 1, 0), 0.15f);   // Wet sponge -> sponge

        // === Stone brick -> cracked stone brick ===
        addRecipe(98, ItemStack(98, 1, 2), 0.1f);    // Stone brick -> cracked
    }

    void registerFuels() {
        // Solids
        fuels_[263] = FuelBurnTime::COAL;       // Coal (meta 0) / Charcoal (meta 1)
        fuels_[173] = FuelBurnTime::COAL_BLOCK; // Block of coal

        // Wood
        fuels_[17]  = FuelBurnTime::LOG;        // Log
        fuels_[5]   = FuelBurnTime::PLANK;      // Planks
        fuels_[280] = FuelBurnTime::STICK;       // Stick
        fuels_[6]   = FuelBurnTime::SAPLING;     // Sapling
        fuels_[47]  = FuelBurnTime::BOOKSHELF;   // Bookshelf

        // Special
        fuels_[327] = FuelBurnTime::LAVA_BUCKET; // Lava bucket
        fuels_[369] = FuelBurnTime::BLAZE_ROD;   // Blaze rod

        // Wooden items
        fuels_[58]  = FuelBurnTime::PLANK;       // Crafting table
        fuels_[54]  = FuelBurnTime::PLANK;       // Chest
        fuels_[72]  = FuelBurnTime::PLANK;       // Wooden pressure plate
        fuels_[85]  = FuelBurnTime::PLANK;       // Fence
        fuels_[107] = FuelBurnTime::PLANK;       // Fence gate
        fuels_[65]  = FuelBurnTime::PLANK;       // Ladder
        fuels_[53]  = FuelBurnTime::PLANK;       // Oak stairs
        fuels_[134] = FuelBurnTime::PLANK;       // Spruce stairs
        fuels_[135] = FuelBurnTime::PLANK;       // Birch stairs
        fuels_[136] = FuelBurnTime::PLANK;       // Jungle stairs
        fuels_[143] = FuelBurnTime::PLANK;       // Button
        fuels_[64]  = FuelBurnTime::PLANK;       // Wooden door
        fuels_[96]  = FuelBurnTime::PLANK;       // Trapdoor
        fuels_[355] = FuelBurnTime::PLANK;       // Bed (takes 300 ticks but close enough)
        fuels_[25]  = FuelBurnTime::PLANK;       // Note block
        fuels_[84]  = FuelBurnTime::PLANK;       // Jukebox
    }
};

// ============================================================
// FurnaceTileEntity — per-furnace state with tick logic
// ============================================================
// Matches aks.java TileEntityFurnace: 3 slots (input, fuel, output)
struct FurnaceTileEntity {
    int32_t x = 0, y = 0, z = 0; // World position

    // 3 slots: 0=input, 1=fuel, 2=output
    ItemStack slots[3];

    // Burn state
    int burnTimeRemaining = 0;  // Ticks of fuel remaining
    int currentItemBurnTime = 0; // Total burn time of current fuel item
    int smeltProgress = 0;       // Ticks of current smelting (0-200)

    bool isBurning() const { return burnTimeRemaining > 0; }

    // Tick the furnace — returns true if state changed (needs packet update)
    bool tick() {
        bool changed = false;
        bool wasBurning = isBurning();

        // Decrease burn time
        if (burnTimeRemaining > 0) {
            --burnTimeRemaining;
            changed = true;
        }

        // Check if we can smelt
        auto& fm = FurnaceManager::instance();
        auto recipe = canSmelt(fm);

        if (recipe.has_value()) {
            // If not burning, try to consume fuel
            if (!isBurning()) {
                int fuelTime = fm.getFuelBurnTime(slots[1].itemId, slots[1].damage);
                if (fuelTime > 0) {
                    burnTimeRemaining = fuelTime;
                    currentItemBurnTime = fuelTime;

                    // Consume fuel
                    --slots[1].count;
                    if (slots[1].count <= 0) {
                        // Lava bucket leaves empty bucket
                        if (slots[1].itemId == 327) {
                            slots[1] = ItemStack(325, 1, 0); // Empty bucket
                        } else {
                            slots[1] = ItemStack();
                        }
                    }
                    changed = true;
                }
            }

            // If burning, advance smelt progress
            if (isBurning()) {
                ++smeltProgress;
                if (smeltProgress >= SMELT_TIME) {
                    smeltProgress = 0;
                    doSmelt(*recipe);
                    changed = true;
                }
            } else {
                // Not burning — reset progress
                if (smeltProgress > 0) {
                    smeltProgress = 0;
                    changed = true;
                }
            }
        } else {
            // Nothing to smelt — reset progress
            if (smeltProgress > 0) {
                smeltProgress = 0;
                changed = true;
            }
        }

        // Update burning state if changed
        if (wasBurning != isBurning()) {
            changed = true;
        }

        return changed;
    }

private:
    std::optional<SmeltingRecipe> canSmelt(const FurnaceManager& fm) const {
        if (slots[0].isEmpty()) return std::nullopt;

        auto recipe = fm.findRecipe(slots[0].itemId, slots[0].damage);
        if (!recipe) return std::nullopt;

        // Check if output slot can accept result
        if (slots[2].isEmpty()) return recipe;
        if (slots[2].itemId != recipe->output.itemId ||
            slots[2].damage != recipe->output.damage) return std::nullopt;
        if (slots[2].count + recipe->output.count > 64) return std::nullopt;

        return recipe;
    }

    void doSmelt(const SmeltingRecipe& recipe) {
        // Consume input
        --slots[0].count;
        if (slots[0].count <= 0) {
            slots[0] = ItemStack();
        }

        // Add to output
        if (slots[2].isEmpty()) {
            slots[2] = recipe.output;
        } else {
            slots[2].count += recipe.output.count;
        }
    }
};

} // namespace mc
