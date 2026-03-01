/**
 * AnvilPressurePlates.h — Falling anvil, pressure plates (binary + weighted).
 *
 * Java references:
 *   - net.minecraft.block.BlockAnvil (97 lines)
 *   - net.minecraft.block.BlockBasePressurePlate (183 lines)
 *   - net.minecraft.block.BlockPressurePlate (56 lines)
 *   - net.minecraft.block.BlockPressurePlateWeighted (47 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ANVIL (BlockAnvil)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: anvil, extends BlockFalling (gravity-affected)
 * Light opacity: 0, not opaque, not normal, render type 35
 *
 * Metadata layout:
 *   bits 0-1: rotation (0-3)
 *   bits 2-3: damage state
 *     0 (meta 0-3) = intact
 *     1 (meta 4-7) = slightly damaged
 *     2 (meta 8-11) = very damaged
 *
 * Placement rotation mapping (yaw → meta):
 *   yaw_index = (floor(yaw * 4 / 360 + 0.5) & 3 + 1) % 4
 *   Mapping: 0→2, 1→3, 2→0, 3→1
 *   Preserves damage bits from existing metadata
 *
 * Bounds per rotation:
 *   Rotation 1 or 3 (N/S): (0, 0, 0.125) → (1, 1, 0.875)
 *   Rotation 0 or 2 (E/W): (0.125, 0, 0) → (0.875, 1, 1)
 *
 * Falling behavior:
 *   onStartFalling: setHurtEntities(true) — damages entities when landing
 *   playSoundWhenFallen: playAuxSFX(1022) — anvil land sound
 *
 * Drop damage: meta >> 2 (preserves damage state)
 * Right-click: opens repair GUI (displayGUIAnvil)
 *
 * Block ID: 145
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BASE PRESSURE PLATE (BlockBasePressurePlate)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Abstract base, creative tab: redstone, tick randomly
 * No collision AABB (entities walk through)
 * Passable (pathfinding ignores)
 * Mobility flag: 1 (can be pushed by pistons)
 *
 * Heights:
 *   Unpressed: 1/16 (0.0625) tall
 *   Pressed:   0.5/16 (0.03125) tall
 *   Edge inset: 1/16 each side
 *
 * Tick rate: 20 ticks (1 second)
 *
 * Entity detection:
 *   Sensitive AABB: (x+2/16, y, z+2/16) → (x+14/16, y+0.25, z+14/16)
 *   onEntityCollided: if power == 0, check entities
 *   updateTick: if power > 0, recheck entities (deactivation)
 *
 * State update (setStateIfMobInteractsWithPlate):
 *   If power changed: update metadata + notify neighbors
 *   Sound: press = "random.click" vol 0.3 pitch 0.5
 *          release = "random.click" vol 0.3 pitch 0.6
 *   If still pressed: schedule another tick
 *
 * Redstone output:
 *   Weak power: all sides = getPowerFromMeta
 *   Strong power: only face 1 (up) = getPowerFromMeta
 *   canProvidePower: true
 *
 * Support: needs solidTopSurface or fence below
 * breakBlock: notifies neighbors if was powered
 * updateNeighbors: notifies at (x,y,z) and (x,y-1,z)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PRESSURE PLATE (BlockPressurePlate) — Binary
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Sensitivity modes:
 *   everything — any entity
 *   mobs — EntityLivingBase only
 *   players — EntityPlayer only
 *
 * Power: binary — 0 or 15
 * Meta: 0 (unpressed) or 1 (pressed)
 *   getMetaFromPower: power > 0 ? 1 : 0
 *   getPowerFromMeta: meta == 1 ? 15 : 0
 *
 * getPlateState: lists entities in sensitive AABB
 *   Filters by doesEntityNotTriggerPressurePlate()
 *   Returns 15 if any valid entity found, else 0
 *
 * Variants:
 *   Stone plate (70): mobs sensitivity, rock material
 *   Wood plate (72): everything sensitivity, wood material
 *
 * ═══════════════════════════════════════════════════════════════════════
 * WEIGHTED PRESSURE PLATE (BlockPressurePlateWeighted) — Analog
 * ═══════════════════════════════════════════════════════════════════════
 *
 * field_150068_a: max entity count for full power (capacity)
 *   Light plate (147, gold): capacity = 15
 *   Heavy plate (148, iron): capacity = 150
 *
 * Tick rate: 10 ticks (0.5 seconds, faster than binary)
 *
 * Power: analog 0-15, proportional to entity count
 *   count = min(entities.size(), capacity)
 *   ratio = count / capacity
 *   power = ceil(ratio * 15)
 *
 * Meta = power directly (0-15)
 *   getPowerFromMeta: return meta
 *   getMetaFromPower: return power
 *
 * Block IDs: light_weighted (147), heavy_weighted (148)
 *
 * Thread safety: Block/redstone on server thread.
 * JNI readiness: Simple metadata, binary/analog output.
 */
#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Anvil Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace AnvilConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 145;

    // ─── Damage states ───
    static constexpr int32_t DAMAGE_INTACT = 0;
    static constexpr int32_t DAMAGE_SLIGHTLY = 1;
    static constexpr int32_t DAMAGE_VERY = 2;
    static constexpr int32_t NUM_DAMAGE_STATES = 3;

    static constexpr const char* DAMAGE_NAMES[] = {
        "intact", "slightlyDamaged", "veryDamaged"
    };

    // ─── Metadata ───
    static constexpr int32_t ROTATION_MASK = 3;    // bits 0-1
    static constexpr int32_t DAMAGE_SHIFT = 2;     // bits 2-3

    inline int32_t getRotation(int32_t meta) { return meta & ROTATION_MASK; }
    inline int32_t getDamage(int32_t meta) { return meta >> DAMAGE_SHIFT; }
    inline int32_t makeMeta(int32_t rotation, int32_t damage) {
        return (rotation & ROTATION_MASK) | (damage << DAMAGE_SHIFT);
    }

    // ─── Placement rotation ───
    // yaw_index = (floor(yaw * 4 / 360 + 0.5) & 3 + 1) % 4
    // Maps: 0→2, 1→3, 2→0, 3→1
    static constexpr int32_t ROTATION_MAP[4] = {2, 3, 0, 1};

    // ─── Bounds ───
    // N/S (rotation 1 or 3): (0, 0, 0.125) → (1, 1, 0.875)
    // E/W (rotation 0 or 2): (0.125, 0, 0) → (0.875, 1, 1)
    static constexpr float EDGE_INSET = 0.125f;   // 2/16

    // ─── Falling ───
    static constexpr int32_t SFX_LAND = 1022;  // anvil impact sound
    // setHurtEntities(true) when starting to fall

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 35;
}

// ═══════════════════════════════════════════════════════════════════════════
// Base Pressure Plate Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PressurePlateConstants {
    // ─── Heights ───
    static constexpr float HEIGHT_UNPRESSED = 0.0625f;     // 1/16
    static constexpr float HEIGHT_PRESSED = 0.03125f;      // 0.5/16
    static constexpr float EDGE_INSET = 0.0625f;           // 1/16

    // ─── Sensitive AABB ───
    // (x+2/16, y, z+2/16) → (x+14/16, y+0.25, z+14/16)
    static constexpr float SENSITIVE_INSET = 0.125f;       // 2/16
    static constexpr double SENSITIVE_HEIGHT = 0.25;

    // ─── Tick rates ───
    static constexpr int32_t BINARY_TICK_RATE = 20;        // 1 second
    static constexpr int32_t WEIGHTED_TICK_RATE = 10;       // 0.5 seconds

    // ─── Sound ───
    static constexpr const char* CLICK_SOUND = "random.click";
    static constexpr float CLICK_VOLUME = 0.3f;
    static constexpr float PITCH_PRESS = 0.5f;
    static constexpr float PITCH_RELEASE = 0.6f;
    static constexpr double SOUND_Y_OFFSET = 0.1;

    // ─── Redstone ───
    // Weak power: all sides
    // Strong power: face 1 (up) only
    static constexpr int32_t STRONG_POWER_FACE = 1;

    // ─── Mobility ───
    static constexpr int32_t MOBILITY_FLAG = 1;  // piston pushable

    // ─── Item render bounds ───
    static constexpr float ITEM_HALF_WIDTH = 0.5f;
    static constexpr float ITEM_HALF_HEIGHT = 0.125f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Binary Pressure Plate Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace BinaryPlateConstants {
    // ─── Block IDs ───
    static constexpr int32_t STONE_PLATE_ID = 70;
    static constexpr int32_t WOOD_PLATE_ID = 72;

    // ─── Sensitivity ───
    enum class Sensitivity : int32_t {
        EVERYTHING = 0,  // any entity (wooden)
        MOBS = 1,        // EntityLivingBase (stone)
        PLAYERS = 2      // EntityPlayer only
    };

    // ─── Power ───
    // Binary: 0 or 15
    static constexpr int32_t POWER_ON = 15;
    static constexpr int32_t POWER_OFF = 0;

    // Meta ↔ Power
    inline int32_t metaFromPower(int32_t power) { return power > 0 ? 1 : 0; }
    inline int32_t powerFromMeta(int32_t meta) { return meta == 1 ? POWER_ON : POWER_OFF; }
}

// ═══════════════════════════════════════════════════════════════════════════
// Weighted Pressure Plate Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace WeightedPlateConstants {
    // ─── Block IDs ───
    static constexpr int32_t LIGHT_PLATE_ID = 147;  // gold
    static constexpr int32_t HEAVY_PLATE_ID = 148;   // iron

    // ─── Capacity (field_150068_a) ───
    static constexpr int32_t LIGHT_CAPACITY = 15;    // gold
    static constexpr int32_t HEAVY_CAPACITY = 150;    // iron

    // ─── Power formula ───
    // count = min(entities.size(), capacity)
    // ratio = count / capacity
    // power = ceil(ratio * 15)
    inline int32_t calculatePower(int32_t entityCount, int32_t capacity) {
        int32_t clamped = std::min(entityCount, capacity);
        if (clamped <= 0) return 0;
        float ratio = static_cast<float>(clamped) / static_cast<float>(capacity);
        return static_cast<int32_t>(std::ceil(ratio * 15.0f));
    }

    // Meta = power directly (0-15)
    inline int32_t metaFromPower(int32_t power) { return power; }
    inline int32_t powerFromMeta(int32_t meta) { return meta; }
}

} // namespace mccpp
