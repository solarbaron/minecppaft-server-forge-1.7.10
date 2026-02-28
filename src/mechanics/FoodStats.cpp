/**
 * FoodStats.cpp — Food item values implementation.
 *
 * Java reference: net.minecraft.item.ItemFood constructor calls
 *   and net.minecraft.init.Items static initializer
 *
 * All vanilla food items with exact healAmount and saturationModifier
 * values from the Java source code.
 */

#include "mechanics/FoodStats.h"
#include <iostream>

namespace mccpp {

// All vanilla food items (1.7.10)
// Java: new ItemFood(healAmount, saturationModifier, isWolfFood)
// Saturation gained = healAmount * saturationModifier * 2.0
static FoodValue allFoodValues[] = {
    // id,  heal, satMod
    {260,   4, 0.3f},    // Apple
    {282,   6, 0.6f},    // Mushroom Stew
    {297,   5, 0.6f},    // Bread
    {319,   3, 0.3f},    // Raw Porkchop
    {320,   8, 0.8f},    // Cooked Porkchop
    {349,   2, 0.1f},    // Raw Fish
    {350,   5, 0.6f},    // Cooked Fish (also clownfish=1/0.1, pufferfish=1/0.1)
    {354,   2, 0.1f},    // Cake (per slice, 7 slices total = 14)
    {357,   2, 0.1f},    // Cookie
    {360,   2, 1.2f},    // Melon Slice
    {363,   3, 0.3f},    // Raw Beef
    {364,   8, 0.8f},    // Steak
    {365,   2, 0.3f},    // Raw Chicken
    {366,   6, 0.6f},    // Cooked Chicken
    {367,   4, 0.1f},    // Rotten Flesh
    {375,   2, 0.8f},    // Spider Eye
    {391,   3, 0.6f},    // Carrot
    {392,   1, 0.3f},    // Potato
    {393,   6, 0.6f},    // Baked Potato
    {394,   2, 0.3f},    // Poisonous Potato
    {396,   6, 1.2f},    // Golden Carrot
    {400,   8, 0.3f},    // Pumpkin Pie
    {322,   4, 1.2f},    // Golden Apple (normal)
};

FoodValue FoodValues::values_[64];
int32_t FoodValues::count_ = 0;
bool FoodValues::initialized_ = false;

void FoodValues::init() {
    if (initialized_) return;

    count_ = static_cast<int32_t>(sizeof(allFoodValues) / sizeof(allFoodValues[0]));
    for (int32_t i = 0; i < count_ && i < 64; ++i) {
        values_[i] = allFoodValues[i];
    }

    initialized_ = true;
    std::cout << "[FoodStats] Registered " << count_ << " food values\n";
}

const FoodValue* FoodValues::getByItemId(int32_t itemId) {
    for (int32_t i = 0; i < count_; ++i) {
        if (values_[i].itemId == itemId) return &values_[i];
    }
    return nullptr;
}

int32_t FoodValues::getCount() {
    return count_;
}

} // namespace mccpp
