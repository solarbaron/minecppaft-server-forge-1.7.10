/**
 * MerchantRecipe.h — Villager trade recipe struct.
 *
 * Java reference: net.minecraft.village.MerchantRecipe
 * Extracted to its own header to avoid circular dependency between
 * MinecraftServer.h and PacketHandler.h.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ─── Villager trading — Java: MerchantRecipe ─────────────────────
struct MerchantRecipe {
    int16_t buyItemId1 = 0;
    int16_t buyDamage1 = 0;
    int8_t  buyCount1 = 1;
    int16_t buyItemId2 = -1;  // -1 = no second buy item
    int16_t buyDamage2 = 0;
    int8_t  buyCount2 = 0;
    int16_t sellItemId = 0;
    int16_t sellDamage = 0;
    int8_t  sellCount = 1;
    int32_t toolUses = 0;
    int32_t maxTradeUses = 7;
    bool isDisabled() const { return toolUses >= maxTradeUses; }
    bool hasSecondItem() const { return buyItemId2 > 0; }
};

} // namespace mccpp
