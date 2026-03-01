/**
 * PortalSystemBlocks.h — Nether portal, end portal, end portal frame.
 *
 * Java references:
 *   - net.minecraft.block.BlockPortal (116 lines)
 *   - net.minecraft.block.BlockEndPortal (85 lines)
 *   - net.minecraft.block.BlockEndPortalFrame (81 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * NETHER PORTAL (BlockPortal)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockBreakable("portal", Material.portal, false)
 * Tick randomly: true
 * No collision AABB, drops 0
 *
 * Axis meta: func_149999_b(meta) = meta & 3
 *   0 = unknown (auto-detect from neighbors)
 *   1 = X-axis (portal spans X/Y)
 *   2 = Z-axis (portal spans Z/Y)
 *
 * Axis offsets: field_150001_a = {empty, {3,1}, {2,0}}
 *
 * Bounds: axis-dependent
 *   Axis 1: X 0-1, Z 0.375-0.625 (4/16 thick)
 *   Axis 2: X 0.375-0.625, Z 0-1
 *   Auto-detect: check ±X for same block → axis 1, else axis 2
 *
 * Mob spawning (updateTick, overworld only):
 *   Condition: doMobSpawning + rand(2000) < difficulty
 *   Find solid below, spawn zombie pigman (ID 57) at center+1.1Y
 *   Set entity.timeUntilPortal = getPortalCooldown()
 *
 * tryToCreatePortal:
 *   Checks both axes (1 and 2) with BlockPortal$Size validator
 *   If valid frame + 0 existing portal blocks → fill with portal
 *
 * Neighbor change: validates portal structure, air if invalid
 * Entity collision: setInPortal() (no rider/ridden)
 *
 * Block ID: portal(90)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * END PORTAL (BlockEndPortal)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockContainer, material: portal
 * TileEntity: TileEntityEndPortal (TESR renderer)
 * Render type: -1
 * Light level: 1.0 (max)
 * Not opaque, not normal
 * Drops: 0
 *
 * Bounds: 1/16 thin slab (Y 0-0.0625)
 * No collision boxes (addCollisionBoxesToList is empty)
 *
 * Entity collision: travelToDimension(1) — The End
 *   Requires: no rider, no ridden, server-side only
 *
 * onBlockAdded:
 *   field_149948_a flag bypasses dimension check
 *   dim != 0 → setBlockToAir (end portal only in overworld)
 *
 * Map color: obsidian
 *
 * Block ID: end_portal(119)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * END PORTAL FRAME (BlockEndPortalFrame)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: rock, render type 26
 * Not opaque (for end-eye rendering)
 * Drops: null (indestructible in survival)
 *
 * Compound collision:
 *   Base: full XZ, Y 0-0.8125 (13/16)
 *   Eye (when inserted): XZ 0.3125-0.6875 (6/16), Y 0.8125-1.0
 *
 * Metadata:
 *   bits 0-1: direction — (floor(yaw*4/360+0.5) & 3 + 2) % 4
 *   bit 2: eye inserted (1) / empty (0)
 *
 * Comparator: 15 when eye inserted, 0 when empty
 *
 * Block ID: end_portal_frame(120)
 *
 * Thread safety: Portal validation on server thread.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Nether Portal Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace NetherPortalConst {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 90;

    // ─── Axis meta ───
    static constexpr int32_t AXIS_MASK = 3;
    static constexpr int32_t AXIS_X = 1;
    static constexpr int32_t AXIS_Z = 2;

    // ─── Bounds ───
    static constexpr float THICKNESS = 0.125f;           // → 0.375-0.625
    static constexpr float HALF = 0.5f;

    // ─── Mob spawning ───
    static constexpr int32_t SPAWN_CHANCE = 2000;
    static constexpr int32_t ZOMBIE_PIGMAN_ID = 57;
    static constexpr double SPAWN_Y_OFFSET = 1.1;

    // ─── Axis offsets table ───
    // field_150001_a: {empty, {3,1}, {2,0}}
    // axis 1 → check sides 3,1
    // axis 2 → check sides 2,0
}

// ═══════════════════════════════════════════════════════════════════════════
// End Portal Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EndPortalConst {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 119;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = -1;

    // ─── Light ───
    static constexpr float LIGHT_LEVEL = 1.0f;

    // ─── Height ───
    static constexpr float HEIGHT = 0.0625f;             // 1/16

    // ─── Destination ───
    static constexpr int32_t TARGET_DIM = 1;             // The End
}

// ═══════════════════════════════════════════════════════════════════════════
// End Portal Frame Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EndFrameConst {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 120;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 26;

    // ─── Meta ───
    static constexpr int32_t DIR_MASK = 3;               // bits 0-1
    static constexpr int32_t EYE_FLAG = 4;               // bit 2

    // ─── Base bounds ───
    static constexpr float BASE_HEIGHT = 0.8125f;        // 13/16

    // ─── Eye pedestal bounds ───
    static constexpr float EYE_MIN_XZ = 0.3125f;         // 5/16
    static constexpr float EYE_MAX_XZ = 0.6875f;         // 11/16
    // Y: 0.8125 → 1.0

    // ─── Comparator ───
    static constexpr int32_t EYE_POWER = 15;
}

} // namespace mccpp
