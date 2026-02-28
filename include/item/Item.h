/**
 * Item.h — Item base class and registry.
 *
 * Java reference: net.minecraft.item.Item
 *
 * Items use a RegistryNamespaced shared with blocks for IDs 0-255 (ItemBlock).
 * Standalone items start at ID 256.
 *
 * The registry is populated once at startup via Item::registerItems().
 * After that, it is read-only and thread-safe without locks.
 *
 * JNI readiness: flat property layout, int IDs for fast lookup.
 */
#pragma once

#include "block/Block.h"
#include "registry/Registry.h"

#include <cstdint>
#include <string>

namespace mccpp {

/**
 * ItemCategory — simplified item type classification.
 * Used to differentiate tools, weapons, armor, food, etc.
 * Full subclass hierarchy will be expanded as needed.
 */
enum class ItemCategory : uint8_t {
    Generic,
    Block,      // ItemBlock — wraps a block
    Tool,       // shovels, pickaxes, axes, hoes
    Sword,
    Bow,
    Armor,
    Food,
    Potion,
    Record,     // music discs
};

/**
 * ToolMaterial — matches Java Item$ToolMaterial.
 */
enum class ToolMaterial : uint8_t {
    WOOD,
    STONE,
    IRON,
    EMERALD,  // diamond tier
    GOLD,
    NONE,
};

/**
 * ArmorMaterial — matches Java ItemArmor$ArmorMaterial.
 */
enum class ArmorMaterial : uint8_t {
    CLOTH,   // leather
    CHAIN,
    IRON,
    DIAMOND,
    GOLD,
    NONE,
};

/**
 * Item — base item type with vanilla 1.7.10 properties.
 *
 * Java reference: net.minecraft.item.Item
 */
class Item {
public:
    // ─── Static registry ────────────────────────────────────────────────
    // Java reference: Item.itemRegistry
    static RegistryNamespaced<Item*> itemRegistry;

    /**
     * Initialize the item registry with all vanilla 1.7.10 items.
     * Java reference: Item.registerItems()
     * Must be called once at startup after Block::registerBlocks().
     */
    static void registerItems();

    /**
     * Get item by numeric ID.
     * Java reference: Item.getItemById(int)
     */
    static Item* getItemById(int32_t id) {
        return itemRegistry.getObjectById(id);
    }

    /**
     * Get numeric ID from item pointer.
     * Java reference: Item.getIdFromItem(Item)
     */
    static int32_t getIdFromItem(Item* item) {
        if (!item) return 0;
        return itemRegistry.getIDForObject(item);
    }

    /**
     * Get the Item form of a block.
     * Java reference: Item.getItemFromBlock(Block)
     */
    static Item* getItemFromBlock(Block* block) {
        return getItemById(Block::getIdFromBlock(block));
    }

    // ─── Constructor ────────────────────────────────────────────────────
    Item();
    virtual ~Item() = default;

    // ─── Builder methods (Java's fluent setters) ────────────────────────
    Item& setMaxStackSize(int n);
    Item& setMaxDurability(int n);
    Item& setUnlocalizedName(const std::string& name);
    Item& setTextureName(const std::string& name);
    Item& setFull3D();
    Item& setHasSubtypes(bool v);
    Item& setContainerItem(Item* item);

    // Category/material shortcuts for registration
    Item& setCategory(ItemCategory cat);
    Item& setToolMaterial(ToolMaterial mat);
    Item& setArmorMaterial(ArmorMaterial mat);
    Item& setArmorSlot(int slot);
    Item& setFoodValues(int healAmount, float saturation, bool isWolfFood);

    // ─── Property getters ───────────────────────────────────────────────
    int getMaxStackSize() const { return maxStackSize_; }
    int getMaxDurability() const { return maxDurability_; }
    bool isDamageable() const { return maxDurability_ > 0 && !hasSubtypes_; }
    bool getHasSubtypes() const { return hasSubtypes_; }
    bool isFull3D() const { return full3D_; }
    ItemCategory getCategory() const { return category_; }
    ToolMaterial getToolMaterial() const { return toolMaterial_; }
    ArmorMaterial getArmorMaterial() const { return armorMaterial_; }
    int getArmorSlot() const { return armorSlot_; }
    Item* getContainerItem() const { return containerItem_; }
    bool hasContainerItem() const { return containerItem_ != nullptr; }
    const std::string& getUnlocalizedName() const { return unlocalizedName_; }
    const std::string& getTextureName() const { return textureName_; }
    int getFoodHealAmount() const { return foodHealAmount_; }
    float getFoodSaturation() const { return foodSaturation_; }
    bool isWolfFood() const { return isWolfFood_; }

protected:
    int            maxStackSize_    = 64;
    int            maxDurability_   = 0;
    bool           full3D_          = false;
    bool           hasSubtypes_     = false;
    ItemCategory   category_        = ItemCategory::Generic;
    ToolMaterial   toolMaterial_    = ToolMaterial::NONE;
    ArmorMaterial  armorMaterial_   = ArmorMaterial::NONE;
    int            armorSlot_       = -1;
    Item*          containerItem_   = nullptr;
    std::string    unlocalizedName_;
    std::string    textureName_;

    // Food properties (simplified; in Java this is a subclass)
    int            foodHealAmount_  = 0;
    float          foodSaturation_  = 0.0f;
    bool           isWolfFood_      = false;
};

} // namespace mccpp
