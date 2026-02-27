#pragma once
// VillagerTrades — merchant recipe system for villager trading.
// Ported from bdt.java (EntityVillager) and bdx.java (MerchantRecipeList).
//
// Vanilla villager trading:
//   - 5 professions: Farmer(0), Librarian(1), Priest(2), Blacksmith(3), Butcher(4)
//   - Each profession has multiple career tiers with trade recipes
//   - MerchantRecipe: itemToBuy + optional itemToBuy2 -> itemToSell
//   - maxTradeUses: limits how many times a trade can be used
//   - Trade window: OpenWindow (type "minecraft:villager") + TradeList (plugin channel)
//
// Network:
//   0x2D OpenWindow — open merchant trade window
//   Plugin channel "MC|TrList" — send trade list data

#include <cstdint>
#include <string>
#include <vector>
#include <random>
#include <chrono>

#include "networking/PacketBuffer.h"

namespace mc {

// Simple item for trade recipes (item ID + count + metadata)
struct TradeItem {
    int16_t itemId = 0;
    int8_t count = 1;
    int16_t meta = 0;

    bool isEmpty() const { return itemId <= 0 || count <= 0; }
};

// Single merchant recipe — from bdv.java (MerchantRecipe)
struct MerchantRecipe {
    TradeItem buyA;        // First input
    TradeItem buyB;        // Second input (optional)
    TradeItem sell;        // Output
    int32_t uses = 0;      // Times used
    int32_t maxUses = 7;   // Max uses before disabled
    bool disabled = false;

    bool hasBuyB() const { return !buyB.isEmpty(); }

    bool isDisabled() const { return disabled || uses >= maxUses; }

    void incrementUses() {
        ++uses;
        if (uses >= maxUses) disabled = true;
    }
};

// Villager profession IDs — from bdt.java
enum class VillagerProfession : uint8_t {
    FARMER      = 0,
    LIBRARIAN   = 1,
    PRIEST      = 2,
    BLACKSMITH  = 3,
    BUTCHER     = 4,
};

// Build profession-specific trade lists
class VillagerTrades {
public:
    // Generate trades for a given profession
    static std::vector<MerchantRecipe> generateTrades(VillagerProfession profession) {
        std::vector<MerchantRecipe> recipes;

        switch (profession) {
            case VillagerProfession::FARMER:
                recipes.push_back(emeraldBuy(296, 18, 22)); // Wheat -> Emerald
                recipes.push_back(emeraldBuy(319, 14, 18)); // Porkchop -> Emerald
                recipes.push_back(emeraldBuy(392, 15, 19)); // Potato -> Emerald
                recipes.push_back(emeraldSell(297, 2, 4));   // Emerald -> Bread
                recipes.push_back(emeraldSell(400, 6, 10));  // Emerald -> Pumpkin Pie
                recipes.push_back(emeraldSell(260, 5, 7));   // Emerald -> Apple
                break;

            case VillagerProfession::LIBRARIAN:
                recipes.push_back(emeraldBuy(339, 24, 36)); // Paper -> Emerald
                recipes.push_back(emeraldBuy(340, 1, 1));   // Book -> Emerald
                recipes.push_back(emeraldSell(339, 4, 6));   // Emerald -> Paper (back)
                recipes.push_back(emeraldSell(102, 3, 5));   // Emerald -> Glass Pane
                recipes.push_back(emeraldSell(47, 1, 1));    // Emerald -> Bookshelf
                break;

            case VillagerProfession::PRIEST:
                recipes.push_back(emeraldBuy(348, 36, 40)); // Glowstone Dust -> Emerald
                recipes.push_back(emeraldSell(381, 1, 1));   // Emerald -> Ender Pearl
                recipes.push_back(emeraldSell(384, 4, 7));   // Emerald -> Bottle o' Enchanting
                recipes.push_back(emeraldSell(376, 1, 1));   // Emerald -> Eye of Ender
                break;

            case VillagerProfession::BLACKSMITH:
                recipes.push_back(emeraldBuy(263, 16, 24)); // Coal -> Emerald
                recipes.push_back(emeraldBuy(265, 7, 9));   // Iron Ingot -> Emerald
                recipes.push_back(emeraldBuy(266, 3, 4));   // Gold Ingot -> Emerald
                recipes.push_back(emeraldBuy(264, 1, 1));   // Diamond -> Emerald
                recipes.push_back(emeraldSell(257, 1, 1));   // Emerald -> Iron Pickaxe
                recipes.push_back(emeraldSell(307, 1, 1));   // Emerald -> Iron Chestplate
                recipes.push_back(emeraldSell(276, 1, 1));   // Emerald -> Diamond Sword
                recipes.push_back(emeraldSell(278, 1, 1));   // Emerald -> Diamond Pickaxe
                break;

            case VillagerProfession::BUTCHER:
                recipes.push_back(emeraldBuy(319, 14, 18)); // Porkchop -> Emerald
                recipes.push_back(emeraldBuy(365, 14, 18)); // Chicken -> Emerald
                recipes.push_back(emeraldBuy(363, 14, 18)); // Beef -> Emerald
                recipes.push_back(emeraldSell(320, 5, 7));   // Emerald -> Cooked Porkchop
                recipes.push_back(emeraldSell(364, 6, 8));   // Emerald -> Steak
                break;
        }

        // Randomize max uses slightly
        std::mt19937 rng(static_cast<unsigned>(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        for (auto& r : recipes) {
            r.maxUses = 2 + (rng() % 11); // 2-12 uses
        }

        return recipes;
    }

    // Serialize trade list for MC|TrList plugin channel
    static PacketBuffer serializeTradeList(int32_t windowId,
                                           const std::vector<MerchantRecipe>& recipes) {
        PacketBuffer buf;
        buf.writeInt(windowId);
        buf.writeByte(static_cast<uint8_t>(recipes.size()));

        for (auto& r : recipes) {
            // Buy item A
            buf.writeShort(r.buyA.itemId);
            buf.writeByte(r.buyA.count);
            buf.writeShort(r.buyA.meta);

            // Sell item
            buf.writeShort(r.sell.itemId);
            buf.writeByte(r.sell.count);
            buf.writeShort(r.sell.meta);

            // Has second buy item
            buf.writeBoolean(r.hasBuyB());
            if (r.hasBuyB()) {
                buf.writeShort(r.buyB.itemId);
                buf.writeByte(r.buyB.count);
                buf.writeShort(r.buyB.meta);
            }

            // Disabled flag
            buf.writeBoolean(r.isDisabled());

            // Uses / max uses
            buf.writeInt(r.uses);
            buf.writeInt(r.maxUses);
        }

        return buf;
    }

private:
    // Helper: create "N items -> 1 emerald" recipe
    static MerchantRecipe emeraldBuy(int16_t itemId, int minCount, int maxCount) {
        MerchantRecipe r;
        r.buyA.itemId = itemId;
        r.buyA.count = static_cast<int8_t>((minCount + maxCount) / 2);
        r.buyA.meta = 0;
        r.sell.itemId = 388; // Emerald
        r.sell.count = 1;
        r.sell.meta = 0;
        return r;
    }

    // Helper: create "1 emerald -> N items" recipe
    static MerchantRecipe emeraldSell(int16_t itemId, int minCount, int maxCount) {
        MerchantRecipe r;
        r.buyA.itemId = 388; // Emerald
        r.buyA.count = 1;
        r.buyA.meta = 0;
        r.sell.itemId = itemId;
        r.sell.count = static_cast<int8_t>((minCount + maxCount) / 2);
        r.sell.meta = 0;
        return r;
    }
};

} // namespace mc
