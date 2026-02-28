/**
 * Block.h — Block base class and registry.
 *
 * Java reference: net.minecraft.block.Block
 *
 * Each block has an integer ID (0-175), a namespaced string name, and
 * properties (hardness, resistance, light value/opacity, etc.).
 *
 * The registry is populated once at startup via Block::registerBlocks().
 * After that, it is read-only and thread-safe without locks.
 *
 * JNI readiness: flat property layout, int IDs for fast array lookup.
 */
#pragma once

#include "registry/Registry.h"

#include <cstdint>
#include <string>

namespace mccpp {

/**
 * Material — simplified block material enum.
 * Java reference: net.minecraft.block.material.Material
 * Full Material class will be expanded later; for now we capture the
 * essential material categories needed for block properties.
 */
enum class Material : uint8_t {
    Air,
    Grass,
    Ground,     // dirt
    Wood,
    Rock,
    Iron,       // metal
    Water,
    Lava,
    Sand,
    Cloth,      // wool
    Fire,
    Plants,
    Vine,
    Sponge,
    Ice,
    PackedIce,
    Snow,
    Clay,
    Glass,
    Portal,
    Web,
    Redstone,
    Piston,
    TNT,
    DragonEgg,
    Coral,      // unused in vanilla 1.7.10, placeholder
    Cake,
    Circuits,   // redstone wire, repeater, etc.
    Carpet,
};

/**
 * Block — base block type with vanilla 1.7.10 properties.
 *
 * Java reference: net.minecraft.block.Block
 */
class Block {
public:
    // ─── Static registry ────────────────────────────────────────────────
    // Java reference: Block.blockRegistry
    static RegistryNamespaced<Block*> blockRegistry;

    /**
     * Initialize the block registry with all vanilla 1.7.10 blocks.
     * Java reference: Block.registerBlocks()
     * Must be called once at startup before any other systems access blocks.
     */
    static void registerBlocks();

    /**
     * Get block by numeric ID.
     * Java reference: Block.getBlockById(int)
     */
    static Block* getBlockById(int32_t id) {
        return blockRegistry.getObjectById(id);
    }

    /**
     * Get numeric ID from block pointer.
     * Java reference: Block.getIdFromBlock(Block)
     */
    static int32_t getIdFromBlock(Block* block) {
        return blockRegistry.getIDForObject(block);
    }

    /**
     * Get block by string name (supports both "stone" and "minecraft:stone").
     * Java reference: Block.getBlockFromName(String)
     */
    static Block* getBlockFromName(const std::string& name);

    // ─── Constructor ────────────────────────────────────────────────────
    Block() : material_(Material::Air) {} // default ctor for storage arrays
    explicit Block(Material material);
    virtual ~Block() = default;

    // ─── Builder methods (Java's fluent setters) ────────────────────────
    Block& setHardness(float h);
    Block& setResistance(float r);
    Block& setLightLevel(float f);
    Block& setLightOpacity(int opacity);
    Block& setUnlocalizedName(const std::string& name);
    Block& setTextureName(const std::string& name);
    Block& setTickRandomly(bool v);
    Block& setBlockUnbreakable();
    Block& disableStats();

    // ─── Property getters ───────────────────────────────────────────────

    Material getMaterial() const { return material_; }
    float getHardness() const { return hardness_; }
    float getResistance() const { return resistance_; }
    int getLightValue() const { return lightValue_; }
    int getLightOpacity() const { return lightOpacity_; }
    bool isFullBlock() const { return fullBlock_; }
    bool getTickRandomly() const { return needsRandomTick_; }
    bool isOpaqueCube() const { return opaqueCube_; }
    const std::string& getUnlocalizedName() const { return unlocalizedName_; }
    const std::string& getTextureName() const { return textureName_; }

    /**
     * Get block bounds (AABB within the block space).
     * Java reference: Block.minX/Y/Z, Block.maxX/Y/Z
     */
    double getMinX() const { return minX_; }
    double getMinY() const { return minY_; }
    double getMinZ() const { return minZ_; }
    double getMaxX() const { return maxX_; }
    double getMaxY() const { return maxY_; }
    double getMaxZ() const { return maxZ_; }

    /**
     * Explosion resistance.
     * Java reference: Block.getExplosionResistance() — resistance / 5.0f
     */
    float getExplosionResistance() const { return resistance_ / 5.0f; }

    // ─── Block bounds ───────────────────────────────────────────────────
    void setBlockBounds(float x1, float y1, float z1, float x2, float y2, float z2);

protected:
    Material material_;

    // Properties — exact Java field names and defaults
    float  hardness_        = 0.0f;
    float  resistance_      = 0.0f;
    int    lightValue_      = 0;     // 0-15
    int    lightOpacity_    = 255;   // 0-255
    bool   fullBlock_       = true;
    bool   opaqueCube_      = true;
    bool   translucent_     = false;
    bool   needsRandomTick_ = false;
    bool   enableStats_     = true;
    bool   useNeighborBrightness_ = false;
    float  slipperiness_    = 0.6f;

    // Block bounds (AABB)
    double minX_ = 0.0, minY_ = 0.0, minZ_ = 0.0;
    double maxX_ = 1.0, maxY_ = 1.0, maxZ_ = 1.0;

    std::string unlocalizedName_;
    std::string textureName_;
};

} // namespace mccpp
