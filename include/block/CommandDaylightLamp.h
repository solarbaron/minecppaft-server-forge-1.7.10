/**
 * CommandDaylightLamp.h — Command block, daylight sensor, redstone lamp.
 *
 * Java references:
 *   - net.minecraft.block.BlockCommandBlock (97 lines)
 *   - net.minecraft.block.BlockDaylightDetector (92 lines)
 *   - net.minecraft.block.BlockRedstoneLight (66 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * COMMAND BLOCK (BlockCommandBlock)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: iron, extends BlockContainer
 * TileEntityCommandBlock: contains CommandBlockLogic
 *   Stores command string, last output, successCount
 *
 * Tick rate: 1 (executes immediately on next tick)
 * quantityDropped: 0 (not obtainable in survival)
 *
 * Metadata:
 *   bit 0: triggered flag (rising edge detection)
 *
 * Redstone activation:
 *   Rising edge: powered + not triggered → set bit 0, schedule tick
 *   Falling edge: not powered + triggered → clear bit 0
 *   updateTick: execute command via CommandBlockLogic.func_145755_a
 *     Then updateNeighborsAboutBlockChange (for comparator)
 *
 * Interaction: displayGUIEditSign (reuses sign editor GUI)
 *   OP permission required (server enforces)
 *
 * Comparator override:
 *   Signal = CommandBlockLogic.getSuccessCount()
 *   (0-15 based on last command execution result)
 *
 * Custom name: transfers from ItemStack to CommandBlockLogic
 *
 * Block ID: command_block (137)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DAYLIGHT DETECTOR (BlockDaylightDetector)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood, extends BlockContainer
 * TileEntityDaylightDetector: updates light level
 * Height: 6/16 (0.375)
 * canProvidePower = true
 *
 * Metadata: stores power level 0-15 directly
 * Weak power: returns metadata value (all faces)
 *
 * Light calculation (func_149957_e):
 *   1. Skip if hasNoSky (Nether/End → always 0)
 *   2. Get saved sky light value at position
 *   3. Subtract skylightSubtracted (rain/thunder darken)
 *   4. Get celestial angle in radians
 *   5. Smooth angle: if < PI → angle += (0 - angle) * 0.2
 *                    else → angle += (2*PI - angle) * 0.2
 *   6. Multiply light by cos(smoothed angle)
 *   7. Round, clamp 0-15
 *   8. Update metadata if changed
 *
 * Called from TileEntityDaylightDetector.updateEntity() each tick
 *
 * Block ID: daylight_detector (151)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * REDSTONE LAMP (BlockRedstoneLight)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: redstoneLight
 * Two variants: redstone_lamp(123) and lit_redstone_lamp(124)
 * field_150171_a = isLit
 *
 * Lit: lightLevel 1.0 (15)
 * Unlit: lightLevel 0
 *
 * State transitions:
 *   Unlit + powered → immediately setBlock to lit (no delay)
 *   Lit + unpowered → schedule tick 4 → setBlock to unlit
 *     4-tick delay prevents rapid flashing from repeater circuits
 *
 * Drops: always redstone_lamp (unlit variant item)
 *
 * Block IDs: redstone_lamp(123), lit_redstone_lamp(124)
 *
 * Thread safety: Block state changes on server thread.
 * JNI readiness: Command system for Forge command events.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Command Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CommandBlockConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 137;

    // ─── Properties ───
    static constexpr int32_t TICK_RATE = 1;
    static constexpr int32_t QUANTITY_DROPPED = 0;

    // ─── Metadata ───
    static constexpr int32_t TRIGGERED_FLAG = 1;     // bit 0
    static constexpr int32_t TRIGGERED_CLEAR = ~1;   // 0xFFFFFFFE

    // ─── Comparator ───
    // Signal = CommandBlockLogic.getSuccessCount()
    // Range: typically 0-15 (clamped elsewhere)
}

// ═══════════════════════════════════════════════════════════════════════════
// Daylight Detector Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DaylightDetectorConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 151;

    // ─── Properties ───
    static constexpr float HEIGHT = 0.375f;           // 6/16

    // ─── Power ───
    // Metadata directly stores signal 0-15
    static constexpr int32_t MAX_POWER = 15;
    static constexpr int32_t MIN_POWER = 0;

    // ─── Light calculation ───
    // Angle smoothing factor
    static constexpr float SMOOTH_FACTOR = 0.2f;

    // Light formula:
    // skyLight = getSavedLightValue(Sky) - skylightSubtracted
    // smoothAngle = angle + (target - angle) * 0.2
    //   target = 0 if angle < PI, 2*PI if angle >= PI
    // power = round(skyLight * cos(smoothAngle))
    // clamp to [0, 15]

    inline int32_t calculatePower(int32_t skyLight, float celestialAngleRadians) {
        float angle = celestialAngleRadians;
        if (angle < static_cast<float>(M_PI)) {
            angle += (0.0f - angle) * SMOOTH_FACTOR;
        } else {
            angle += (static_cast<float>(M_PI) * 2.0f - angle) * SMOOTH_FACTOR;
        }
        int32_t power = static_cast<int32_t>(std::round(
            static_cast<float>(skyLight) * std::cos(angle)));
        if (power < MIN_POWER) power = MIN_POWER;
        if (power > MAX_POWER) power = MAX_POWER;
        return power;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Redstone Lamp Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace RedstoneLampConstants {
    // ─── Block IDs ───
    static constexpr int32_t UNLIT_ID = 123;     // redstone_lamp
    static constexpr int32_t LIT_ID = 124;        // lit_redstone_lamp

    // ─── Light ───
    static constexpr float LIT_LIGHT_LEVEL = 1.0f;   // = 15

    // ─── Turn-off delay ───
    static constexpr int32_t OFF_DELAY = 4;           // ticks
    // On: instant (no delay)
    // Off: 4 tick delay (prevents rapid flashing)

    // ─── Drops ───
    // Always drops redstone_lamp (unlit variant)
}

} // namespace mccpp
