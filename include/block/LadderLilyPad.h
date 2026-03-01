/**
 * LadderLilyPad.h — Ladder climbable panel and lily pad water decoration.
 *
 * Java references:
 *   - net.minecraft.block.BlockLadder (125 lines)
 *   - net.minecraft.block.BlockLilyPad (57 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LADDER (BlockLadder)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits
 * Not opaque, not normal, render type 8
 * Climbable: entities can climb when inside ladder bounds
 * Thickness: 2/16 (0.125) — flat panel against wall
 *
 * Metadata = facing direction (which wall the ladder is on):
 *   2 = on south wall (+Z) → bounds: (0, 0, 0.875) → (1, 1, 1)
 *   3 = on north wall (-Z) → bounds: (0, 0, 0) → (1, 1, 0.125)
 *   4 = on east wall (+X)  → bounds: (0.875, 0, 0) → (1, 1, 1)
 *   5 = on west wall (-X)  → bounds: (0, 0, 0) → (0.125, 1, 1)
 *
 * Support block mapping (backing wall):
 *   meta 2 → check (x, y, z+1) isNormalCube
 *   meta 3 → check (x, y, z-1) isNormalCube
 *   meta 4 → check (x+1, y, z) isNormalCube
 *   meta 5 → check (x-1, y, z) isNormalCube
 *
 * Placement (onBlockPlaced):
 *   Priority: prefer matching side click, fall back to first valid wall
 *   side 2 → meta 2, side 3 → meta 3, side 4 → meta 4, side 5 → meta 5
 *
 * canPlaceBlockAt: at least one adjacent normal cube
 * onNeighborBlockChange: break if support wall removed
 *
 * Block ID: 65
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LILY PAD (BlockLilyPad)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends BlockBush (plant-like behavior)
 * Height: 1/64 (0.015625) — nearly flat on water surface
 * Full XZ coverage: (0, 0, 0) → (1, 0.015625, 1)
 * Render type: 23
 *
 * Placement:
 *   - Can place on: still water only (water block with meta 0)
 *   - canBlockStay: Y in [0, 256), block below is water material
 *     with metadata 0 (still, not flowing)
 *   - canPlaceBlockOn: only Blocks.water (not flowing_water)
 *
 * Collision:
 *   - Normal entities: collides normally (walks on top)
 *   - Boats: NO collision (boats pass through)
 *     Java: if entity instanceof EntityBoat → skip
 *
 * Block ID: 111
 *
 * Thread safety: Block state on server thread.
 * JNI readiness: Simple metadata layout.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Ladder Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LadderConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 65;

    // ─── Thickness ───
    static constexpr float THICKNESS = 0.125f;  // 2/16

    // ─── Metadata facing values ───
    // These match vanilla block face IDs
    static constexpr int32_t FACING_SOUTH = 2;  // ladder on +Z wall
    static constexpr int32_t FACING_NORTH = 3;  // ladder on -Z wall
    static constexpr int32_t FACING_EAST = 4;   // ladder on +X wall
    static constexpr int32_t FACING_WEST = 5;   // ladder on -X wall

    // ─── Bounds per facing ───
    struct FaceBounds {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
    };

    // Indexed by (meta - 2) for metas 2-5
    static constexpr FaceBounds BOUNDS[] = {
        // meta 2 (south wall): (0, 0, 1-t) → (1, 1, 1)
        {0.0f, 0.0f, 1.0f - THICKNESS, 1.0f, 1.0f, 1.0f},
        // meta 3 (north wall): (0, 0, 0) → (1, 1, t)
        {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, THICKNESS},
        // meta 4 (east wall): (1-t, 0, 0) → (1, 1, 1)
        {1.0f - THICKNESS, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f},
        // meta 5 (west wall): (0, 0, 0) → (t, 1, 1)
        {0.0f, 0.0f, 0.0f, THICKNESS, 1.0f, 1.0f},
    };

    // ─── Support block offsets ───
    // Which adjacent block supports the ladder
    struct SupportOffset {
        int32_t dx, dy, dz;
    };

    // Indexed by (meta - 2) for metas 2-5
    static constexpr SupportOffset SUPPORTS[] = {
        { 0, 0, +1},  // meta 2: south wall → block behind at +Z
        { 0, 0, -1},  // meta 3: north wall → block behind at -Z
        {+1, 0,  0},  // meta 4: east wall → block behind at +X
        {-1, 0,  0},  // meta 5: west wall → block behind at -X
    };

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 8;

    // ─── Climb speed ───
    // Entity climb speed is handled in Entity.java:
    // if isOnLadder: motionY = max(motionY, -0.15)
    // and can move up at 0.2/tick when holding forward
}

// ═══════════════════════════════════════════════════════════════════════════
// Lily Pad Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LilyPadConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 111;

    // ─── Dimensions ───
    // Nearly flat: 1/64 height
    static constexpr float HEIGHT = 0.015625f;  // 1/64
    // Full XZ: 0 to 1 (float f = 0.5, so 0.5-0.5=0 to 0.5+0.5=1)

    // ─── Placement ───
    // Can place on: Blocks.water (still water, ID 9)
    static constexpr int32_t WATER_BLOCK_ID = 9;   // still water
    // Must be still water (metadata == 0)
    static constexpr int32_t STILL_WATER_META = 0;

    // Y range: [0, 256)
    static constexpr int32_t MIN_Y = 0;
    static constexpr int32_t MAX_Y = 256;

    // ─── Collision ───
    // Boats pass through (no collision with EntityBoat)
    // All other entities collide normally

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 23;

    // ─── Block below check ───
    // Java: world.getBlock(x, y-1, z).getMaterial() == Material.water
    //       && world.getBlockMetadata(x, y-1, z) == 0
}

} // namespace mccpp
