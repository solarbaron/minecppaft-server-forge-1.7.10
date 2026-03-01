/**
 * TorchBlock.h — Torch placement, support validation, and ray trace bounds.
 *
 * Java reference:
 *   - net.minecraft.block.BlockTorch (182 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TORCH (BlockTorch)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits
 * No collision box (null)
 * Not opaque, not normal, render type 2
 * Tick randomly
 *
 * Metadata:
 *   0 = unplaced (auto-select on tick/add)
 *   1 = on west wall (-X) → support at (x-1, y, z)
 *   2 = on east wall (+X) → support at (x+1, y, z)
 *   3 = on north wall (-Z) → support at (x, y, z-1)
 *   4 = on south wall (+Z) → support at (x, y, z+1)
 *   5 = on floor → support at (x, y-1, z)
 *
 * Floor support (canPlaceTorchOn):
 *   - doesBlockHaveSolidTopSurface (any block with solid top)
 *   - OR: fence, nether_brick_fence, glass, cobblestone_wall
 *   Special blocks that torches can rest on despite no solid top surface
 *
 * Wall support: isBlockNormalCubeDefault (solid normal cube)
 *
 * Placement (onBlockPlaced):
 *   Side mapping: side 1→meta 5, side 2→meta 4, side 3→meta 3,
 *                 side 4→meta 2, side 5→meta 1
 *   Each side validated before setting
 *
 * Auto-select (onBlockAdded, meta == 0):
 *   Priority: west(-X), east(+X), north(-Z), south(+Z), floor
 *   First valid support wins
 *
 * Neighbor change (func_150108_b):
 *   - First check: can torch exist at all (dropTorchIfCantStay)
 *   - Then: check if specific support for current facing is gone
 *
 * Ray trace bounds per facing:
 *   Wall (meta 1-4): 0.15 half-width, Y range 0.2-0.8
 *     meta 1 (west wall): (0, 0.2, 0.35) → (0.3, 0.8, 0.65)
 *     meta 2 (east wall): (0.7, 0.2, 0.35) → (1, 0.8, 0.65)
 *     meta 3 (north wall): (0.35, 0.2, 0) → (0.65, 0.8, 0.3)
 *     meta 4 (south wall): (0.35, 0.2, 0.7) → (0.65, 0.8, 1)
 *   Floor (meta 5): 0.1 half-width, Y range 0-0.6
 *     (0.4, 0, 0.4) → (0.6, 0.6, 0.6)
 *
 * Also applies to: redstone torch (BlockRedstoneTorch extends BlockTorch)
 *
 * Block IDs: torch (50), redstone_torch_off (75), redstone_torch_on (76)
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Simple metadata layout.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Torch Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TorchConstants {
    // ─── Block IDs ───
    static constexpr int32_t TORCH_ID = 50;
    static constexpr int32_t REDSTONE_TORCH_OFF_ID = 75;
    static constexpr int32_t REDSTONE_TORCH_ON_ID = 76;

    // ─── Metadata ───
    static constexpr int32_t META_UNPLACED = 0;
    static constexpr int32_t META_WEST_WALL = 1;   // support at -X
    static constexpr int32_t META_EAST_WALL = 2;   // support at +X
    static constexpr int32_t META_NORTH_WALL = 3;  // support at -Z
    static constexpr int32_t META_SOUTH_WALL = 4;  // support at +Z
    static constexpr int32_t META_FLOOR = 5;        // support at Y-1

    // ─── Side → Meta mapping (onBlockPlaced) ───
    // Side 1 (top) → meta 5 (floor)
    // Side 2 (south) → meta 4
    // Side 3 (north) → meta 3
    // Side 4 (east) → meta 2
    // Side 5 (west) → meta 1
    static constexpr int32_t SIDE_TO_META[] = {0, 5, 4, 3, 2, 1};

    // ─── Support block offsets ───
    struct SupportOffset {
        int32_t dx, dy, dz;
    };

    // Indexed by meta (0 invalid, 1-5 valid)
    static constexpr SupportOffset SUPPORTS[] = {
        { 0,  0,  0},  // meta 0: unplaced (invalid)
        {-1,  0,  0},  // meta 1: west wall
        {+1,  0,  0},  // meta 2: east wall
        { 0,  0, -1},  // meta 3: north wall
        { 0,  0, +1},  // meta 4: south wall
        { 0, -1,  0},  // meta 5: floor
    };

    // ─── Floor support blocks ───
    // In addition to doesBlockHaveSolidTopSurface:
    static constexpr int32_t FENCE_ID = 85;
    static constexpr int32_t NETHER_BRICK_FENCE_ID = 113;
    static constexpr int32_t GLASS_ID = 20;
    static constexpr int32_t COBBLESTONE_WALL_ID = 139;

    // ─── Ray trace bounds ───
    struct RayBounds {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
    };

    // f = 0.15 for wall, 0.1 for floor
    // Wall metas 1-4, floor meta 5
    static constexpr RayBounds RAY_BOUNDS[] = {
        // meta 0: invalid (use floor bounds)
        {0.4f, 0.0f, 0.4f, 0.6f, 0.6f, 0.6f},
        // meta 1 (west wall): (0, 0.2, 0.35) → (0.3, 0.8, 0.65)
        {0.0f, 0.2f, 0.35f, 0.3f, 0.8f, 0.65f},
        // meta 2 (east wall): (0.7, 0.2, 0.35) → (1, 0.8, 0.65)
        {0.7f, 0.2f, 0.35f, 1.0f, 0.8f, 0.65f},
        // meta 3 (north wall): (0.35, 0.2, 0) → (0.65, 0.8, 0.3)
        {0.35f, 0.2f, 0.0f, 0.65f, 0.8f, 0.3f},
        // meta 4 (south wall): (0.35, 0.2, 0.7) → (0.65, 0.8, 1)
        {0.35f, 0.2f, 0.7f, 0.65f, 0.8f, 1.0f},
        // meta 5 (floor): (0.4, 0, 0.4) → (0.6, 0.6, 0.6)
        {0.4f, 0.0f, 0.4f, 0.6f, 0.6f, 0.6f},
    };

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 2;

    // ─── Auto-select priority (onBlockAdded, meta 0) ───
    // 1. West wall (-X)
    // 2. East wall (+X)
    // 3. North wall (-Z)
    // 4. South wall (+Z)
    // 5. Floor (Y-1)
}

} // namespace mccpp
