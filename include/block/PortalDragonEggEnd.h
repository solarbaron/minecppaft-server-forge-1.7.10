/**
 * PortalDragonEggEnd.h — Dimension transport and trophy blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockPortal (116 lines)
 *   - net.minecraft.block.BlockDragonEgg (115 lines)
 *   - net.minecraft.block.BlockEndPortal (85 lines)
 *   - net.minecraft.block.BlockEndPortalFrame (81 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * NETHER PORTAL (BlockPortal)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: portal, extends BlockBreakable("portal")
 * No collision (null), not normal, setTickRandomly(true)
 *
 * Metadata: axis orientation
 *   0: undetermined (auto-detect from neighbors)
 *   1: X axis (portal faces X direction)
 *   2: Z axis (portal faces Z direction)
 *
 * Bounds: based on axis
 *   X-axis: 0.375-0.625 X (4/16 wide), full Y, full Z
 *   Z-axis: full X, full Y, 0.375-0.625 Z
 *
 * Entity collision: setInPortal()
 *   Requires: no ridingEntity, no riddenByEntity
 *   Teleports to/from Nether after portal cooldown
 *
 * Zombie Pigman spawning (updateTick):
 *   Only in overworld (isSurfaceWorld)
 *   gamerule doMobSpawning enabled
 *   Chance: rand(2000) < difficulty ID (0-3)
 *   Spawns entity 57 (zombie pigman) at solid ground
 *   Sets timeUntilPortal = portalCooldown
 *
 * Portal creation (tryToCreatePortal):
 *   Size inner class validates obsidian frame
 *   Tries both X and Z orientations
 *   Min 3×2, max 21×21 portal interior
 *
 * Portal validation (onNeighborBlockChange):
 *   Checks frame integrity per axis
 *   Invalid → set to air
 *
 * Drops: nothing (quantityDropped = 0)
 *
 * Block ID: portal(90)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DRAGON EGG (BlockDragonEgg)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: dragonEgg
 * Bounds: 1/16 inset XZ (0.0625 to 0.9375), full height
 * Not opaque, render type 27
 *
 * Falling: like sand/gravel, tick rate 5
 *   canFallBelow check → EntityFallingBlock spawn
 *   fallInstantly mode for world gen
 *
 * Teleportation (right-click or left-click):
 *   1000 attempts to find air block
 *   Range: ±rand(16) X, ±rand(8) Y, ±rand(16) Z
 *   Server: move block to found position
 *   Client: 128 "portal" particles along trajectory
 *
 * Block ID: dragon_egg(122)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * END PORTAL (BlockEndPortal)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: portal, extends BlockContainer
 * TileEntityEndPortal: visual renderer
 * Light level: 1.0 (15)
 * Height: 1/16 (0.0625), no collision (empty list)
 * Render type -1 (TileEntity renderer)
 *
 * Entity collision: travelToDimension(1)
 *   Requires: no ridingEntity, no riddenByEntity, server side
 *   Teleports to The End (dimension 1)
 *
 * Placement: overworld only (dimensionId == 0)
 *   field_149948_a: suppress dimension check (for structure gen)
 *
 * MapColor: obsidianColor
 * Drops: nothing
 *
 * Block ID: end_portal(119)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * END PORTAL FRAME (BlockEndPortalFrame)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: rock
 * Not opaque, render type 26
 *
 * Collision: 2 boxes
 *   1. Base: full XZ, 0 to 13/16 Y (0.8125)
 *   2. Eye (if inserted): 5/16-11/16 XZ, 13/16-1 Y
 *
 * Metadata:
 *   bits 0-1: direction (yaw → (yaw+2)%4)
 *   bit 2: eye of ender inserted
 *
 * isEnderEyeInserted: (meta & 4) != 0
 *
 * Comparator: 15 if eye inserted, 0 if empty
 * Drops: nothing
 *
 * Stronghold eye insertion:
 *   Right-clicking with eye of ender sets bit 2
 *   Checks 3×3 ring for complete portal (12 frames with eyes)
 *   If complete → creates 3×3 end_portal inside
 *
 * Block IDs: end_portal_frame(120)
 *
 * Thread safety: Portal dimension transport on server thread.
 * JNI readiness: Dimension events for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Nether Portal Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace NetherPortalConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 90;

    // ─── Metadata ───
    static constexpr int32_t AXIS_MASK = 3;
    static constexpr int32_t AXIS_X = 1;
    static constexpr int32_t AXIS_Z = 2;

    // ─── Bounds ───
    static constexpr float PORTAL_THIN = 0.375f;       // 6/16
    static constexpr float PORTAL_THICK = 0.625f;      // 10/16
    // default thin = 2/16 (0.125) from center

    // ─── Portal size ───
    static constexpr int32_t MIN_WIDTH = 2;
    static constexpr int32_t MAX_WIDTH = 21;
    static constexpr int32_t MIN_HEIGHT = 3;
    static constexpr int32_t MAX_HEIGHT = 21;

    // ─── Frame ───
    static constexpr int32_t OBSIDIAN_ID = 49;

    // ─── Mob spawning ───
    static constexpr int32_t SPAWN_CHANCE = 2000;
    static constexpr int32_t ZOMBIE_PIGMAN_ID = 57;

    // ─── Dimension ───
    static constexpr int32_t NETHER_DIM = -1;
    static constexpr int32_t OVERWORLD_DIM = 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Dragon Egg Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DragonEggConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 122;

    // ─── Properties ───
    static constexpr int32_t RENDER_TYPE = 27;
    static constexpr int32_t TICK_RATE = 5;

    // ─── Bounds ───
    static constexpr float INSET = 0.0625f;            // 1/16

    // ─── Teleportation ───
    static constexpr int32_t TELEPORT_ATTEMPTS = 1000;
    static constexpr int32_t TELEPORT_RANGE_XZ = 16;
    static constexpr int32_t TELEPORT_RANGE_Y = 8;
    static constexpr int32_t PARTICLE_COUNT = 128;

    // ─── Falling ───
    static constexpr int32_t FALL_CHECK_RADIUS = 32;
}

// ═══════════════════════════════════════════════════════════════════════════
// End Portal Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EndPortalConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 119;

    // ─── Properties ───
    static constexpr float LIGHT_LEVEL = 1.0f;         // = 15
    static constexpr float HEIGHT = 0.0625f;            // 1/16

    // ─── Dimension ───
    static constexpr int32_t END_DIM = 1;
    static constexpr int32_t OVERWORLD_DIM = 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// End Portal Frame Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EndPortalFrameConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 120;

    // ─── Properties ───
    static constexpr int32_t RENDER_TYPE = 26;

    // ─── Metadata ───
    static constexpr int32_t DIRECTION_MASK = 3;       // bits 0-1
    static constexpr int32_t EYE_FLAG = 4;             // bit 2

    inline bool isEyeInserted(int32_t meta) { return (meta & EYE_FLAG) != 0; }

    // ─── Bounds ───
    static constexpr float BASE_HEIGHT = 0.8125f;      // 13/16
    // Eye collision: 5/16 to 11/16 XZ, 13/16 to 1 Y
    static constexpr float EYE_MIN_XZ = 0.3125f;       // 5/16
    static constexpr float EYE_MAX_XZ = 0.6875f;       // 11/16

    // ─── Comparator ───
    static constexpr int32_t EYE_SIGNAL = 15;
    static constexpr int32_t EMPTY_SIGNAL = 0;

    // ─── Portal ring ───
    static constexpr int32_t RING_SIZE = 12;            // frames needed
    static constexpr int32_t PORTAL_INTERIOR = 9;       // 3×3 portal blocks
}

} // namespace mccpp
