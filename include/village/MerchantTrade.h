/**
 * MerchantTrade.h — Villager trading system: recipes and recipe list.
 *
 * Java references:
 *   - net.minecraft.village.MerchantRecipe — Single trade recipe
 *   - net.minecraft.village.MerchantRecipeList — List of trade recipes
 *   - net.minecraft.village.IMerchant — Merchant interface
 *
 * Each recipe has: buy item, optional second buy item, sell item,
 * use count, max uses (default 7). Recipes become disabled when
 * uses >= maxUses.
 *
 * The recipe list supports:
 *   - canRecipeBeUsed: Match input items to a recipe (with index hint)
 *   - addToListWithCheck: Dedup by item IDs, replace if lower stack sizes
 *   - Packet serialization: count byte, items, hasSecondBuy, isDisabled
 *   - NBT persistence: "Recipes" tag list
 *
 * Not thread-safe: trades are handled on entity tick thread.
 * JNI readiness: Simple data, standard containers.
 */
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// TradeItem — Simplified item representation for trade recipes.
// ═══════════════════════════════════════════════════════════════════════════

struct TradeItem {
    int32_t itemId = 0;
    int32_t stackSize = 1;
    int32_t metadata = 0;

    bool matches(const TradeItem& other) const {
        return itemId == other.itemId; // Java: getItem() == comparison
    }

    bool matchesWithSize(const TradeItem& other) const {
        return itemId == other.itemId && stackSize >= other.stackSize;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// MerchantRecipe — Single villager trade recipe.
// Java reference: net.minecraft.village.MerchantRecipe
// ═══════════════════════════════════════════════════════════════════════════

class MerchantRecipe {
public:
    MerchantRecipe() = default;

    // Java: MerchantRecipe(ItemStack, ItemStack, ItemStack)
    MerchantRecipe(const TradeItem& buy, const TradeItem& sell)
        : itemToBuy_(buy), itemToSell_(sell), maxTradeUses_(7) {}

    MerchantRecipe(const TradeItem& buy, const TradeItem& secondBuy, const TradeItem& sell)
        : itemToBuy_(buy), secondItemToBuy_(secondBuy),
          hasSecondItem_(true), itemToSell_(sell), maxTradeUses_(7) {}

    // ─── Getters ───

    const TradeItem& getItemToBuy() const { return itemToBuy_; }
    const TradeItem& getItemToSell() const { return itemToSell_; }

    bool hasSecondItemToBuy() const { return hasSecondItem_; }
    const TradeItem& getSecondItemToBuy() const { return secondItemToBuy_; }

    int32_t getToolUses() const { return toolUses_; }
    int32_t getMaxTradeUses() const { return maxTradeUses_; }

    // Java: isRecipeDisabled
    bool isDisabled() const { return toolUses_ >= maxTradeUses_; }

    // Java: incrementToolUses
    void incrementUses() { ++toolUses_; }

    // Java: func_82783_a — increase max trades
    void increaseMaxUses(int32_t amount) { maxTradeUses_ += amount; }

    // ─── Comparison ───

    // Java: hasSameIDsAs — same item types
    bool hasSameIds(const MerchantRecipe& other) const {
        if (!itemToBuy_.matches(other.itemToBuy_)) return false;
        if (!itemToSell_.matches(other.itemToSell_)) return false;
        // Both must have or not have second item
        if (!hasSecondItem_ && !other.hasSecondItem_) return true;
        if (hasSecondItem_ && other.hasSecondItem_) {
            return secondItemToBuy_.matches(other.secondItemToBuy_);
        }
        return false;
    }

    // Java: hasSameItemsAs — same IDs but lower stack sizes (need replacement)
    bool hasSameItemsButLower(const MerchantRecipe& other) const {
        if (!hasSameIds(other)) return false;
        return itemToBuy_.stackSize < other.itemToBuy_.stackSize ||
               (hasSecondItem_ && secondItemToBuy_.stackSize < other.secondItemToBuy_.stackSize);
    }

    // ─── NBT data (for serialization) ───

    struct NBTData {
        TradeItem buy;
        TradeItem sell;
        bool hasSecondBuy = false;
        TradeItem secondBuy;
        int32_t uses = 0;
        int32_t maxUses = 7;
    };

    NBTData toNBT() const {
        NBTData data;
        data.buy = itemToBuy_;
        data.sell = itemToSell_;
        data.hasSecondBuy = hasSecondItem_;
        data.secondBuy = secondItemToBuy_;
        data.uses = toolUses_;
        data.maxUses = maxTradeUses_;
        return data;
    }

    void fromNBT(const NBTData& data) {
        itemToBuy_ = data.buy;
        itemToSell_ = data.sell;
        hasSecondItem_ = data.hasSecondBuy;
        secondItemToBuy_ = data.secondBuy;
        toolUses_ = data.uses;
        maxTradeUses_ = data.maxUses;
    }

private:
    TradeItem itemToBuy_;
    TradeItem secondItemToBuy_;
    bool hasSecondItem_ = false;
    TradeItem itemToSell_;
    int32_t toolUses_ = 0;
    int32_t maxTradeUses_ = 7;
};

// ═══════════════════════════════════════════════════════════════════════════
// MerchantRecipeList — List of trade recipes with matching and serialization.
// Java reference: net.minecraft.village.MerchantRecipeList
// ═══════════════════════════════════════════════════════════════════════════

class MerchantRecipeList {
public:
    MerchantRecipeList() = default;

    // ─── Recipe management ───

    void addRecipe(const MerchantRecipe& recipe) {
        recipes_.push_back(recipe);
    }

    // Java: addToListWithCheck — dedup by item IDs, replace if lower stack sizes
    void addWithCheck(const MerchantRecipe& recipe) {
        for (size_t i = 0; i < recipes_.size(); ++i) {
            if (recipe.hasSameIds(recipes_[i])) {
                if (recipe.hasSameItemsButLower(recipes_[i])) {
                    recipes_[i] = recipe;
                }
                return; // Already exists (or replaced)
            }
        }
        recipes_.push_back(recipe);
    }

    // Java: canRecipeBeUsed — match input items to a recipe
    // If indexHint > 0 and < size, check that index first.
    // Otherwise, linear scan.
    MerchantRecipe* findMatchingRecipe(const TradeItem& buy1,
                                         const TradeItem* buy2,
                                         int32_t indexHint) {
        // Try specific index first
        if (indexHint > 0 && indexHint < static_cast<int32_t>(recipes_.size())) {
            auto& recipe = recipes_[static_cast<size_t>(indexHint)];
            if (matchesRecipe(recipe, buy1, buy2)) {
                return &recipe;
            }
            return nullptr; // Java returns null if hint doesn't match
        }

        // Linear scan
        for (auto& recipe : recipes_) {
            if (matchesRecipe(recipe, buy1, buy2)) {
                return &recipe;
            }
        }
        return nullptr;
    }

    // ─── Access ───

    size_t size() const { return recipes_.size(); }
    bool empty() const { return recipes_.empty(); }

    MerchantRecipe& operator[](size_t idx) { return recipes_[idx]; }
    const MerchantRecipe& operator[](size_t idx) const { return recipes_[idx]; }

    const std::vector<MerchantRecipe>& getRecipes() const { return recipes_; }
    std::vector<MerchantRecipe>& getRecipes() { return recipes_; }

    // ─── Packet serialization format ───
    // Java: func_151391_a(PacketBuffer)
    // Format: count:byte, then for each recipe:
    //   buy:ItemStack, sell:ItemStack, hasSecondBuy:bool, [secondBuy:ItemStack], isDisabled:bool

    struct PacketRecipe {
        TradeItem buy;
        TradeItem sell;
        bool hasSecondBuy;
        TradeItem secondBuy;
        bool isDisabled;
    };

    std::vector<PacketRecipe> toPacketFormat() const {
        std::vector<PacketRecipe> result;
        result.reserve(recipes_.size());
        for (const auto& recipe : recipes_) {
            PacketRecipe pr;
            pr.buy = recipe.getItemToBuy();
            pr.sell = recipe.getItemToSell();
            pr.hasSecondBuy = recipe.hasSecondItemToBuy();
            pr.secondBuy = recipe.getSecondItemToBuy();
            pr.isDisabled = recipe.isDisabled();
            result.push_back(pr);
        }
        return result;
    }

    // ─── NBT persistence ───

    struct NBTListData {
        std::vector<MerchantRecipe::NBTData> recipes;
    };

    NBTListData toNBT() const {
        NBTListData data;
        data.recipes.reserve(recipes_.size());
        for (const auto& recipe : recipes_) {
            data.recipes.push_back(recipe.toNBT());
        }
        return data;
    }

    void fromNBT(const NBTListData& data) {
        recipes_.clear();
        recipes_.reserve(data.recipes.size());
        for (const auto& nbt : data.recipes) {
            MerchantRecipe recipe;
            recipe.fromNBT(nbt);
            recipes_.push_back(recipe);
        }
    }

private:
    // Java: canRecipeBeUsed matching logic
    static bool matchesRecipe(const MerchantRecipe& recipe,
                                const TradeItem& buy1,
                                const TradeItem* buy2) {
        // First buy item must match
        if (!buy1.matchesWithSize(recipe.getItemToBuy())) return false;

        // Second buy item check
        if (recipe.hasSecondItemToBuy()) {
            if (buy2 == nullptr) return false;
            if (!buy2->matchesWithSize(recipe.getSecondItemToBuy())) return false;
        } else {
            if (buy2 != nullptr) return false; // Java: secondBuy == null && slot != null → skip
        }

        return true;
    }

    std::vector<MerchantRecipe> recipes_;
};

} // namespace mccpp
