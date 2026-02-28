/**
 * InventoryPlayer.cpp — Player inventory implementation.
 *
 * Java reference: net.minecraft.entity.player.InventoryPlayer
 *
 * Armor value calculation, damage distribution, clear and copy operations.
 */

#include "entity/InventoryPlayer.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace mccpp {

// Java armor item IDs and their damageReduceAmount values
// boots(0), leggings(1), chestplate(2), helmet(3)
// Leather: 298-301 → 1,2,3,1  Chain: 302-305 → 1,4,5,2
// Iron: 306-309 → 2,5,6,2     Gold: 314-317 → 1,3,5,2
// Diamond: 310-313 → 3,6,8,3

static int32_t getArmorReduction(int32_t itemId) {
    switch (itemId) {
        // Leather: helmet=298, chest=299, legs=300, boots=301
        case 298: return 1;  case 299: return 3;  case 300: return 2;  case 301: return 1;
        // Chain: helmet=302, chest=303, legs=304, boots=305
        case 302: return 2;  case 303: return 5;  case 304: return 4;  case 305: return 1;
        // Iron: helmet=306, chest=307, legs=308, boots=309
        case 306: return 2;  case 307: return 6;  case 308: return 5;  case 309: return 2;
        // Diamond: helmet=310, chest=311, legs=312, boots=313
        case 310: return 3;  case 311: return 8;  case 312: return 6;  case 313: return 3;
        // Gold: helmet=314, chest=315, legs=316, boots=317
        case 314: return 2;  case 315: return 5;  case 316: return 3;  case 317: return 1;
        default: return 0;
    }
}

static bool isArmorItem(int32_t itemId) {
    return itemId >= 298 && itemId <= 317;
}

int32_t InventoryPlayer::getTotalArmorValue() const {
    int32_t total = 0;
    for (int32_t i = 0; i < ARMOR_SIZE; ++i) {
        if (!armorInventory[i].isEmpty() && isArmorItem(armorInventory[i].itemId)) {
            total += getArmorReduction(armorInventory[i].itemId);
        }
    }
    return total;
}

void InventoryPlayer::damageArmor(float damage) {
    // Java: damage /= 4.0f; if (damage < 1.0f) damage = 1.0f;
    damage /= 4.0f;
    if (damage < 1.0f) damage = 1.0f;

    for (int32_t i = 0; i < ARMOR_SIZE; ++i) {
        if (!armorInventory[i].isEmpty() && isArmorItem(armorInventory[i].itemId)) {
            // Java: damageItem — reduce durability
            armorInventory[i].metadata += static_cast<int32_t>(damage);
            // Check if broken (simplified: max durability check would use item registry)
            if (armorInventory[i].stackSize == 0) {
                armorInventory[i].clear();
            }
        }
    }
    inventoryChanged = true;
}

int32_t InventoryPlayer::clearInventory(int32_t itemId, int32_t metadata) {
    int32_t count = 0;

    // Java: clear main inventory
    for (int32_t i = 0; i < MAIN_SIZE; ++i) {
        if (mainInventory[i].isEmpty()) continue;
        if (itemId >= 0 && mainInventory[i].itemId != itemId) continue;
        if (metadata >= 0 && mainInventory[i].metadata != metadata) continue;
        count += mainInventory[i].stackSize;
        mainInventory[i].clear();
    }

    // Java: clear armor inventory
    for (int32_t i = 0; i < ARMOR_SIZE; ++i) {
        if (armorInventory[i].isEmpty()) continue;
        if (itemId >= 0 && armorInventory[i].itemId != itemId) continue;
        if (metadata >= 0 && armorInventory[i].metadata != metadata) continue;
        count += armorInventory[i].stackSize;
        armorInventory[i].clear();
    }

    // Java: clear cursor item
    if (!cursorItem.isEmpty()) {
        if (itemId < 0 || cursorItem.itemId == itemId) {
            if (metadata < 0 || cursorItem.metadata == metadata) {
                count += cursorItem.stackSize;
                cursorItem.clear();
            }
        }
    }

    inventoryChanged = true;
    return count;
}

void InventoryPlayer::dropAllItems() {
    for (int32_t i = 0; i < MAIN_SIZE; ++i) {
        mainInventory[i].clear();
    }
    for (int32_t i = 0; i < ARMOR_SIZE; ++i) {
        armorInventory[i].clear();
    }
    inventoryChanged = true;
}

void InventoryPlayer::copyInventory(const InventoryPlayer& source) {
    for (int32_t i = 0; i < MAIN_SIZE; ++i) {
        mainInventory[i] = source.mainInventory[i];
    }
    for (int32_t i = 0; i < ARMOR_SIZE; ++i) {
        armorInventory[i] = source.armorInventory[i];
    }
    currentItem = source.currentItem;
}

} // namespace mccpp
