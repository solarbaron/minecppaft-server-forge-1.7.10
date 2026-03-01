/**
 * LeashKnotEnderEye.h — Leash knot on fences and eye of ender homing flight.
 *
 * Java references:
 *   - net.minecraft.entity.EntityLeashKnot (113 lines)
 *   - net.minecraft.entity.item.EntityEnderEye (137 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LEASH KNOT (EntityLeashKnot)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Extends EntityHanging, 9×9 pixels (0.5625 blocks)
 * Position: block center (x+0.5, y+0.5, z+0.5)
 * setDirection is no-op (not directional like paintings)
 *
 * Validation:
 *   onValidSurface: block at position has renderType 11 (fences)
 *
 * Interaction (interactFirst):
 *   1. If player holds lead:
 *      Search 7-block radius for EntityLiving leashed to player
 *      Transfer all matching leashes to this knot
 *   2. If no leash transferred (or no lead held):
 *      Destroy knot (setDead)
 *      Creative mode: clear all leashes attached to this knot within 7 blocks
 *
 * No NBT persistence:
 *   writeToNBTOptional returns false (never saved to world)
 *   writeEntityToNBT and readEntityFromNBT are empty
 *   Knots are recreated from leashed entity NBT on world load
 *
 * Static helpers:
 *   func_110129_a: create and force-spawn knot at position
 *   getKnotForBlock: search 2×2×2 area for existing knot at coordinates
 *
 * onBroken: no-op (doesn't drop lead — leashed entities do)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * EYE OF ENDER (EntityEnderEye)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Size: 0.25 × 0.25
 * Cannot be attacked, brightness always 1.0
 * No NBT persistence
 *
 * Spawning (moveTowards):
 *   If distance > 12 blocks:
 *     targetXZ = pos + normalized_direction * 12
 *     targetY = posY + 8 (rises 8 blocks)
 *   Else:
 *     targetXZ = stronghold XZ
 *     targetY = stronghold Y (chunk height)
 *
 *   shatterOrDrop = rand(5) > 0 → 80% chance to drop, 20% to shatter
 *
 * Per-tick flight (onUpdate, server-side):
 *   1. Calculate angle to target: atan2(dZ, dX)
 *   2. Speed interpolation: speed += (distance - speed) * 0.0025
 *      If distance < 1.0: speed *= 0.8, motionY *= 0.8
 *   3. motionX = cos(angle) * speed
 *      motionZ = sin(angle) * speed
 *   4. motionY lerp toward ±1.0:
 *      If below target: motionY += (1.0 - motionY) * 0.015
 *      If above target: motionY += (-1.0 - motionY) * 0.015
 *
 * Position update: direct add (posX += motionX, not moveEntity)
 * Rotation: atan2 with 0.2 lerp (same as firework)
 *
 * Despawn (80 ticks):
 *   If shatterOrDrop (80%): spawn EntityItem with ender_eye
 *   If shatter (20%): play SFX 2003 (ender eye break sound)
 *
 * Particles:
 *   In water: 4 bubble particles per tick
 *   In air: 1 portal particle per tick
 *
 * Entity IDs: leash_knot=77, ender_eye=15 (not entity ID, spawn type)
 *
 * Thread safety: Entity on server thread.
 * JNI readiness: Simple field layout.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Leash Knot Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LeashKnotConstants {
    // ─── Entity dimensions ───
    static constexpr int32_t WIDTH_PIXELS = 9;
    static constexpr int32_t HEIGHT_PIXELS = 9;

    // ─── Position offset ───
    static constexpr double CENTER_OFFSET = 0.5;

    // ─── Leash search radius ───
    static constexpr double LEASH_RADIUS = 7.0;

    // ─── Valid surface ───
    // Block must have renderType 11 (fence posts)
    static constexpr int32_t FENCE_RENDER_TYPE = 11;

    // ─── Knot search area (getKnotForBlock) ───
    // 2×2×2 AABB centered on block: (x-1, y-1, z-1) → (x+1, y+1, z+1)
    static constexpr double SEARCH_RADIUS = 1.0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Eye of Ender Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EnderEyeConstants {
    // ─── Entity size ───
    static constexpr float WIDTH = 0.25f;
    static constexpr float HEIGHT = 0.25f;

    // ─── Target clamping ───
    static constexpr float MAX_TARGET_DISTANCE = 12.0f;
    static constexpr double RISE_HEIGHT = 8.0;  // rises 8 blocks when far

    // ─── Drop/shatter chance ───
    // rand(5) > 0 → 80% drop, 20% shatter
    static constexpr int32_t SHATTER_CHANCE = 5;  // 1 in 5 = 20% shatter

    // ─── Flight physics ───
    static constexpr double SPEED_INTERPOLATION = 0.0025;
    static constexpr double CLOSE_SLOWDOWN = 0.8;       // speed *= 0.8 when < 1 block
    static constexpr float CLOSE_DISTANCE = 1.0f;
    static constexpr double Y_LERP_RATE = 0.015;        // motionY lerp speed
    static constexpr double Y_TARGET_UP = 1.0;
    static constexpr double Y_TARGET_DOWN = -1.0;

    // ─── Rotation ───
    static constexpr float ROTATION_LERP = 0.2f;
    static constexpr double PI = 3.1415927410125732;     // Java float π

    // ─── Despawn ───
    static constexpr int32_t DESPAWN_TICKS = 80;

    // ─── Sound effects ───
    static constexpr int32_t SFX_SHATTER = 2003;  // ender eye break

    // ─── Brightness ───
    static constexpr float BRIGHTNESS = 1.0f;  // always fully lit

    // ─── Particles ───
    static constexpr const char* BUBBLE_PARTICLE = "bubble";
    static constexpr const char* PORTAL_PARTICLE = "portal";
    static constexpr int32_t BUBBLE_COUNT = 4;
}

} // namespace mccpp
