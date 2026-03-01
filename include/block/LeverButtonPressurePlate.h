/**
 * LeverButtonPressurePlate.h — Redstone input blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockLever (315 lines)
 *   - net.minecraft.block.BlockButton (309 lines)
 *   - net.minecraft.block.BlockBasePressurePlate (183 lines)
 *   - net.minecraft.block.BlockPressurePlate (56 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * LEVER (BlockLever)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, no collision, render type 12
 * Not opaque, not normal, canProvidePower = true
 *
 * Metadata (8 orientations + power bit):
 *   bits 0-2: orientation (0-7)
 *     0: ceiling, z-axis aligned
 *     1: wall, x- face
 *     2: wall, x+ face
 *     3: wall, z- face
 *     4: wall, z+ face
 *     5: floor, z-axis aligned
 *     6: floor, x-axis aligned
 *     7: ceiling, x-axis aligned
 *   bit 3: powered (on) state
 *
 * invertMetadata mapping:
 *   0→0, 1→5, 2→4, 3→3, 4→2, 5→1
 *
 * Placement:
 *   Face 0 (bottom of block above) → orientation 0 (ceiling)
 *   Face 1 (top of block below) → orientation 5 (floor)
 *   Faces 2-5: wall orientations 4,3,2,1
 *   Floor/ceiling: yaw-based rotation sets 5/6 or 0/7
 *     yaw*4/360+0.5 & 1: even→z-axis, odd→x-axis
 *
 * Bounds (3/16 = 0.1875):
 *   Wall: 3/16 thick, 6/10 height (0.2-0.8), 6/16 wide
 *   Floor (5/6): 8/16 × 0.6 high × 8/16, centered
 *   Ceiling (0/7): 8/16 × 0.6 high × 8/16, top-aligned (0.4-1.0)
 *
 * Toggle (onBlockActivated):
 *   XOR bit 3: n7 = 8 - (meta & 8)
 *   Sound: "random.click", vol 0.3, pitch 0.6 (on) or 0.5 (off)
 *   Notify self + attached surface block
 *
 * Power:
 *   Weak: 15 when on, 0 when off (all directions)
 *   Strong: 15 only toward attached surface
 *     0/7→face 0, 5/6→face 1, 4→face 2, 3→face 3, 2→face 4, 1→face 5
 *
 * Block ID: lever (69)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BUTTON (BlockButton)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, abstract (wooden vs stone subclasses)
 * No collision, not opaque, render type (not set in base)
 * canProvidePower = true
 *
 * Variants:
 *   Wooden: tick 30, can be activated by arrows
 *   Stone: tick 20, player-only activation
 *
 * Metadata:
 *   bits 0-2: orientation 1-4 (wall-only, no floor/ceiling)
 *     1: x- wall, 2: x+ wall, 3: z- wall, 4: z+ wall
 *   bit 3: pressed state
 *
 * Bounds:
 *   Depressed (bit 3): depth 1/16 (0.0625)
 *   Released: depth 2/16 (0.125)
 *   Centered on face: 6/16 height (0.375-0.625), 6/16 wide
 *
 * Activation (onBlockActivated):
 *   If already pressed → return true (no-op)
 *   Set bit 3, schedule tick, sound "random.click" 0.3/0.6
 *
 * Arrow activation (wooden only):
 *   onEntityCollidedWithBlock → activateButton
 *   Scans EntityArrow within button bounds
 *   Arrow present + not pressed → press
 *   No arrow + pressed → release
 *   Schedule next tick while arrow present
 *
 * Deactivation (updateTick):
 *   Wooden: runs activateButton (checks for arrows)
 *   Stone: immediately releases, sound 0.3/0.5
 *
 * Power:
 *   Weak: 15 when pressed, 0 off
 *   Strong: 15 only toward attached face
 *     5→face 1, 4→face 2, 3→face 3, 2→face 4, 1→face 5
 *
 * Block IDs: stone_button(77), wooden_button(143)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * PRESSURE PLATE (BlockBasePressurePlate + BlockPressurePlate)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: stone or wood, tick 20, mobility 1
 * No collision, passable, not opaque
 * canProvidePower = true
 *
 * Bounds:
 *   Unpressed: 1/16 inset XZ, 1/16 high (0.0625)
 *   Pressed: 1/16 inset XZ, 1/32 high (0.03125)
 *
 * Placement: solid top surface below OR on fence (85/113)
 *
 * Entity detection AABB (getSensitiveAABB):
 *   2/16 inset XZ, 0 to 0.25 high
 *
 * Sensitivity modes (BlockPressurePlate):
 *   everything: any entity (excludes doesEntityNotTriggerPressurePlate)
 *   mobs: EntityLivingBase only
 *   players: EntityPlayer only
 *
 * Power: binary — meta 0→0, meta 1→15
 *   Weak: 15 when pressed (all faces)
 *   Strong: 15 only face 1 (below)
 *
 * State machine:
 *   onEntityCollidedWithBlock (power=0) → check entities → setMeta
 *   updateTick (power>0) → recheck entities → update or schedule
 *   Sound: activate "random.click" 0.3/0.6, deactivate 0.3/0.5
 *   Notify self + block below
 *
 * Block IDs: stone_pressure_plate(70), wooden_pressure_plate(72)
 *
 * Thread safety: All redstone on server thread.
 * JNI readiness: Power provider interface for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Lever Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace LeverConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 69;

    // ─── Metadata ───
    static constexpr int32_t ORIENTATION_MASK = 7;   // bits 0-2
    static constexpr int32_t POWERED_FLAG = 8;        // bit 3

    // 8 orientations:
    static constexpr int32_t CEILING_Z = 0;   // ceiling, z-aligned
    static constexpr int32_t WALL_X_NEG = 1;  // x- face
    static constexpr int32_t WALL_X_POS = 2;  // x+ face
    static constexpr int32_t WALL_Z_NEG = 3;  // z- face
    static constexpr int32_t WALL_Z_POS = 4;  // z+ face
    static constexpr int32_t FLOOR_Z = 5;     // floor, z-aligned
    static constexpr int32_t FLOOR_X = 6;     // floor, x-aligned
    static constexpr int32_t CEILING_X = 7;   // ceiling, x-aligned

    // invertMetadata: placement face → orientation
    inline int32_t invertMetadata(int32_t n) {
        switch(n) {
            case 0: return 0;
            case 1: return 5;
            case 2: return 4;
            case 3: return 3;
            case 4: return 2;
            case 5: return 1;
            default: return -1;
        }
    }

    // Strong power: orientation → face direction
    // 0/7→0, 5/6→1, 4→2, 3→3, 2→4, 1→5
    inline int32_t getStrongFace(int32_t orient) {
        switch(orient) {
            case 0: case 7: return 0;
            case 5: case 6: return 1;
            case 4: return 2;
            case 3: return 3;
            case 2: return 4;
            case 1: return 5;
            default: return -1;
        }
    }

    // ─── Bounds ───
    static constexpr float WALL_DEPTH = 0.1875f;       // 3/16
    static constexpr float WALL_Y_MIN = 0.2f;
    static constexpr float WALL_Y_MAX = 0.8f;
    static constexpr float FLOOR_HALF = 0.25f;         // 4/16
    static constexpr float FLOOR_HEIGHT = 0.6f;

    // ─── Power ───
    static constexpr int32_t POWER_ON = 15;
    static constexpr int32_t POWER_OFF = 0;

    // ─── Sound ───
    static constexpr float CLICK_VOLUME = 0.3f;
    static constexpr float CLICK_PITCH_ON = 0.6f;
    static constexpr float CLICK_PITCH_OFF = 0.5f;

    // ─── Render ───
    static constexpr int32_t RENDER_TYPE = 12;
}

// ═══════════════════════════════════════════════════════════════════════════
// Button Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace ButtonConstants {
    // ─── Block IDs ───
    static constexpr int32_t STONE_BUTTON_ID = 77;
    static constexpr int32_t WOODEN_BUTTON_ID = 143;

    // ─── Tick rate ───
    static constexpr int32_t WOODEN_TICK = 30;
    static constexpr int32_t STONE_TICK = 20;

    // ─── Metadata ───
    static constexpr int32_t ORIENTATION_MASK = 7;   // bits 0-2
    static constexpr int32_t PRESSED_FLAG = 8;        // bit 3

    // Wall orientations: 1=x-, 2=x+, 3=z-, 4=z+

    // ─── Bounds ───
    static constexpr float BUTTON_Y_MIN = 0.375f;     // 6/16
    static constexpr float BUTTON_Y_MAX = 0.625f;     // 10/16
    static constexpr float BUTTON_HALF = 0.1875f;      // 3/16
    static constexpr float DEPTH_RELEASED = 0.125f;    // 2/16
    static constexpr float DEPTH_PRESSED = 0.0625f;    // 1/16

    // Strong power: orientation → face
    // 5→1, 4→2, 3→3, 2→4, 1→5
    inline int32_t getStrongFace(int32_t orient) {
        switch(orient) {
            case 5: return 1;
            case 4: return 2;
            case 3: return 3;
            case 2: return 4;
            case 1: return 5;
            default: return -1;
        }
    }

    // ─── Power ───
    static constexpr int32_t POWER_ON = 15;
    static constexpr int32_t POWER_OFF = 0;

    // ─── Sound ───
    static constexpr float CLICK_VOLUME = 0.3f;
    static constexpr float CLICK_PITCH_PRESS = 0.6f;
    static constexpr float CLICK_PITCH_RELEASE = 0.5f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Pressure Plate Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace PressurePlateConstants {
    // ─── Block IDs ───
    static constexpr int32_t STONE_PLATE_ID = 70;
    static constexpr int32_t WOODEN_PLATE_ID = 72;

    // ─── Sensitivity ───
    enum class Sensitivity : int32_t {
        EVERYTHING = 0,   // any entity (excluding notTrigger)
        MOBS = 1,         // EntityLivingBase only
        PLAYERS = 2       // EntityPlayer only
    };
    // Stone: mobs, Wood: everything

    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 20;

    // ─── Bounds ───
    static constexpr float INSET = 0.0625f;            // 1/16
    static constexpr float HEIGHT_RELEASED = 0.0625f;   // 1/16
    static constexpr float HEIGHT_PRESSED = 0.03125f;   // 1/32

    // ─── Detection AABB ───
    static constexpr float SENSITIVE_INSET = 0.125f;    // 2/16
    static constexpr double SENSITIVE_HEIGHT = 0.25;

    // ─── Power ───
    // Binary: meta 0→power 0, meta 1→power 15
    static constexpr int32_t POWER_ON = 15;
    static constexpr int32_t POWER_OFF = 0;
    static constexpr int32_t META_ON = 1;
    static constexpr int32_t META_OFF = 0;

    // ─── Strong power ───
    // Only face 1 (below)
    static constexpr int32_t STRONG_FACE = 1;

    // ─── Placement ───
    // Requires solid top surface below OR fence (85/113)
    static constexpr int32_t FENCE_ID = 85;
    static constexpr int32_t NETHER_FENCE_ID = 113;

    // ─── Mobility ───
    static constexpr int32_t MOBILITY = 1;

    // ─── Sound ───
    static constexpr float CLICK_VOLUME = 0.3f;
    static constexpr float CLICK_PITCH_PRESS = 0.6f;
    static constexpr float CLICK_PITCH_RELEASE = 0.5f;
    static constexpr double SOUND_Y_OFFSET = 0.1;
}

} // namespace mccpp
