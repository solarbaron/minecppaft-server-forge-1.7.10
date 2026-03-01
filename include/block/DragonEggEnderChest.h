/**
 * DragonEggEnderChest.h — Dragon egg teleportation and ender chest per-player storage.
 *
 * Java references:
 *   - net.minecraft.block.BlockDragonEgg (115 lines)
 *   - net.minecraft.block.BlockEnderChest (102 lines)
 *   - net.minecraft.tileentity.TileEntityEnderChest (82 lines)
 *   - net.minecraft.inventory.InventoryEnderChest (77 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DRAGON EGG (BlockDragonEgg)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: dragonEgg, tick rate: 5
 * Bounds: inset 1/16 on XZ (0.0625-0.9375), full height
 * Not opaque, not normal block, render type 27
 *
 * Gravity:
 *   - Extends fallable block behavior (like sand/gravel)
 *   - onBlockAdded / onNeighborBlockChange → schedule tick
 *   - updateTick → check if can fall (air/liquid/fire below)
 *   - If fallInstantly OR no chunks loaded: instant teleport down
 *   - Otherwise: spawn EntityFallingBlock at (x+0.5, y+0.5, z+0.5)
 *   - Chunk check radius: 32
 *
 * Teleportation (on click or activate):
 *   func_150019_m — 1000 attempts:
 *   - Random target: x ± rand(16)-rand(16), y ± rand(8)-rand(8), z ± rand(16)-rand(16)
 *   - Range: effectively -15 to +15 on X/Z, -7 to +7 on Y
 *   - If target block is air:
 *     - Server: move egg to target, set source to air
 *     - Client: 128 portal particles interpolated between old/new position
 *   - First successful attempt ends search
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ENDER CHEST BLOCK (BlockEnderChest)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: rock
 * Bounds: inset 1/16 on XZ, 14/16 height (0.0625, 0, 0.0625 → 0.9375, 0.875, 0.9375)
 * Not opaque, not normal, render type 22
 *
 * Drops: 8 obsidian (no silk touch) OR 1 ender chest (silk touch)
 *
 * Facing (onBlockPlacedBy):
 *   Java: floor(yaw * 4 / 360 + 0.5) & 3
 *   n5=0 → meta 2 (south)
 *   n5=1 → meta 5 (west)
 *   n5=2 → meta 3 (north)
 *   n5=3 → meta 4 (east)
 *
 * Opening (onBlockActivated):
 *   - Cannot open if normal cube above
 *   - Gets player's per-player InventoryEnderChest
 *   - Associates tile entity with inventory
 *   - Opens GUI via displayGUIChest
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TILE ENTITY ENDER CHEST (TileEntityEnderChest)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Lid animation:
 *   - field_145972_a (lidAngle): 0.0 (closed) to 1.0 (open)
 *   - prevLidAngle: previous tick value
 *   - Ramp rate: ±0.1f per tick
 *   - Open sound at: viewers > 0 AND lidAngle == 0 (first open)
 *   - Close sound at: lidAngle crosses down past 0.5
 *   - Sounds: "random.chestopen" / "random.chestclosed" vol=0.5
 *     pitch = rand*0.1 + 0.9
 *
 * Viewer tracking:
 *   - field_145973_j: number of players viewing
 *   - func_145969_a(): viewer count++ (on open)
 *   - func_145970_b(): viewer count-- (on close)
 *   - Block events sync count to client (event ID 1)
 *
 * Block event refresh: every 80 ticks (field_145974_k % 20*4 == 0)
 *
 * Usability check: within 64 blocks (8 block distance squared)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ENDER CHEST INVENTORY (InventoryEnderChest)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Per-player, 27 slots (3 rows of 9)
 * Name: "container.enderchest", not custom-named
 *
 * NBT format:
 *   - TAG_List of TAG_Compound entries
 *   - Each compound: "Slot" (byte), plus ItemStack NBT fields
 *   - Loading: clear all slots first, then set from NBT
 *   - Saving: skip null slots
 *
 * Inventory lifecycle:
 *   - openChest: increment tile entity viewer count
 *   - closeChest: decrement viewer count, clear tile entity reference
 *   - isUseableByPlayer: check tile entity distance (64.0 dist²)
 *
 * Thread safety: Block positions on server thread. Inventory per-player.
 * JNI readiness: Simple constants and slot layout.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Dragon Egg Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DragonEggConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 122;

    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 5;

    // ─── Block bounds ───
    // Java: (0.0625, 0, 0.0625) → (0.9375, 1.0, 0.9375)
    static constexpr float BOUNDS_INSET = 0.0625f;  // 1/16

    // ─── Gravity ───
    // Check radius for chunk existence
    static constexpr int32_t GRAVITY_CHECK_RADIUS = 32;
    // Falling block spawn offset
    static constexpr float FALL_SPAWN_OFFSET = 0.5f;

    // ─── Teleportation ───
    // Max attempts to find valid teleport target
    static constexpr int32_t TELEPORT_ATTEMPTS = 1000;

    // Range: x ± rand(16)-rand(16), z same
    static constexpr int32_t TELEPORT_XZ_RAND = 16;
    // Effective range: -15 to +15

    // Range: y ± rand(8)-rand(8)
    static constexpr int32_t TELEPORT_Y_RAND = 8;
    // Effective range: -7 to +7

    // ─── Client particles ───
    static constexpr int32_t TELEPORT_PARTICLE_COUNT = 128;
    static constexpr const char* PARTICLE_TYPE = "portal";
    static constexpr float PARTICLE_VELOCITY_RANGE = 0.2f;

    // ─── Render type ───
    static constexpr int32_t RENDER_TYPE = 27;
}

// ═══════════════════════════════════════════════════════════════════════════
// Ender Chest Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EnderChestBlockConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 130;

    // ─── Block bounds ───
    // Java: (0.0625, 0, 0.0625) → (0.9375, 0.875, 0.9375)
    static constexpr float BOUNDS_INSET_XZ = 0.0625f;
    static constexpr float BOUNDS_HEIGHT = 0.875f;  // 14/16

    // ─── Drops ───
    // Without silk touch: 8 obsidian
    static constexpr int32_t OBSIDIAN_ID = 49;
    static constexpr int32_t OBSIDIAN_DROP_COUNT = 8;
    // With silk touch: 1 ender chest (via canSilkHarvest = true)

    // ─── Facing metadata ───
    // Java: floor(yaw * 4 / 360 + 0.5) & 3 → facing
    // n5=0 → 2(south), n5=1 → 5(west), n5=2 → 3(north), n5=3 → 4(east)
    static constexpr int32_t YAW_TO_META[] = {2, 5, 3, 4};

    inline int32_t getFacingFromYaw(float yaw) {
        int32_t index = static_cast<int32_t>(yaw * 4.0f / 360.0f + 0.5f) & 3;
        return YAW_TO_META[index];
    }

    // ─── Open condition ───
    // Cannot open if normal cube above (solid opaque block)

    // ─── Render type ───
    static constexpr int32_t RENDER_TYPE = 22;
}

// ═══════════════════════════════════════════════════════════════════════════
// Ender Chest Tile Entity Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EnderChestTileConstants {
    // ─── Lid animation ───
    // Speed: ±0.1f per tick
    static constexpr float LID_SPEED = 0.1f;
    static constexpr float LID_CLOSED = 0.0f;
    static constexpr float LID_OPEN = 1.0f;

    // Close sound triggers when lid crosses down past 0.5
    static constexpr float CLOSE_SOUND_THRESHOLD = 0.5f;

    // ─── Sounds ───
    static constexpr const char* SOUND_OPEN = "random.chestopen";
    static constexpr const char* SOUND_CLOSE = "random.chestclosed";
    static constexpr float SOUND_VOLUME = 0.5f;
    static constexpr float SOUND_PITCH_BASE = 0.9f;
    static constexpr float SOUND_PITCH_RAND = 0.1f;
    // Sound position: (x + 0.5, y + 0.5, z + 0.5)

    // ─── Block event refresh ───
    // Java: field_145974_k % (20 * 4) == 0 → refresh every 80 ticks
    static constexpr int32_t REFRESH_INTERVAL = 80;  // 20 * 4
    static constexpr int32_t BLOCK_EVENT_ID = 1;

    // ─── Usability distance ───
    // Java: distanceSq(x+0.5, y+0.5, z+0.5) <= 64.0
    static constexpr double MAX_DISTANCE_SQ = 64.0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Ender Chest Inventory Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EnderChestInventoryConstants {
    // ─── Inventory ───
    static constexpr const char* NAME = "container.enderchest";
    static constexpr bool IS_CUSTOM_NAME = false;
    static constexpr int32_t SIZE = 27;  // 3 rows × 9 columns

    // ─── NBT format ───
    static constexpr const char* NBT_SLOT_KEY = "Slot";
    // Slot byte is AND'd with 0xFF (unsigned)
    // Each entry is a compound with Slot + ItemStack data
}

} // namespace mccpp
