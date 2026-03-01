/**
 * DaylightTripwire.h — Daylight sensor and tripwire/hook redstone input blocks.
 *
 * Java references:
 *   - net.minecraft.block.BlockDaylightDetector (92 lines)
 *   - net.minecraft.tileentity.TileEntityDaylightDetector (empty tile entity)
 *   - net.minecraft.block.BlockTripWire (174 lines)
 *   - net.minecraft.block.BlockTripWireHook (309 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DAYLIGHT SENSOR (BlockDaylightDetector)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: wood, height 0.375 (6/16 blocks — slab-like)
 *
 * Signal calculation (func_149957_e — called by TileEntity updateEntity):
 *   1. If dimension has no sky → return (no signal)
 *   2. Get saved sky light value at position minus skylightSubtracted
 *   3. Get celestialAngleRadians(1.0)
 *   4. Angle adjustment:
 *      - If angle < PI: angle += (0.0 - angle) * 0.2
 *      - If angle >= PI: angle += (2*PI - angle) * 0.2
 *   5. signal = round(skyLight * cos(adjustedAngle))
 *   6. Clamp to 0-15
 *   7. If metadata != signal: setBlockMetadataWithNotify(signal, flags=3)
 *
 * Power output:
 *   - isProvidingWeakPower → metadata value (0-15)
 *   - canProvidePower → true
 *   - No strong power
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TRIPWIRE STRING (BlockTripWire)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, tickRate: 10
 * No collision box (entities pass through)
 *
 * Metadata bits:
 *   bit 0: activated (entity standing on it)
 *   bit 1: suspended (no solid surface below)
 *   bit 2: attached (connected to hooks on both ends)
 *   bit 3: disarmed (broken with shears — no hook notification)
 *
 * Block bounds based on state:
 *   - Not suspended: 0.09375 height (1.5/16)
 *   - Suspended + not attached: 0.5 height
 *   - Suspended + attached: 0.0625-0.15625 height (above ground)
 *
 * Entity detection (func_150140_e):
 *   1. Get all entities in wireblock AABB
 *   2. Filter: !doesEntityNotTriggerPressurePlate
 *   3. If entity found && not activated: set bit 0
 *   4. If no entity && activated: clear bit 0
 *   5. If state changed: update metadata, notify hooks
 *   6. If entity present: schedule update in tickRate(10) ticks
 *
 * Hook notification (func_150138_a — scan for connected hooks):
 *   - Scan in both horizontal directions (Direction 0 and 1)
 *   - Up to 42 blocks in each direction
 *   - Stop at tripwire_hook facing this direction
 *   - Stop at non-tripwire block
 *   - Notify found hook via func_150136_a
 *
 * Break by shears: set disarmed flag (bit 3) before drop
 * Break on break: notify hooks with activated flag set
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TRIPWIRE HOOK (BlockTripWireHook)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: circuits, tickRate: 10
 * Wall-mounted on normal cube blocks (sides 2-5 only)
 * No collision box
 *
 * Metadata bits:
 *   bits 0-1: direction (0=south, 1=west, 2=north, 3=east)
 *   bit 2: attached (connected to opposite hook with valid chain)
 *   bit 3: powered (entity on wire)
 *
 * Chain scanning (func_150136_a — 42-block line scan):
 *   For each block 1..41 in the facing direction:
 *   - If tripwire_hook facing opposite → found endpoint, set chainLength
 *   - If tripwire string:
 *     - Check suspended alignment (must match hook suspension)
 *     - Track if any wire segment is activated (bit 0 set + not disarmed)
 *   - If other block → chain broken, not attached
 *   - If distance == insertedIndex → use provided metadata
 *
 *   After scan:
 *   attached = chainLength > 1 && all wires aligned
 *   powered = attached && any wire activated
 *
 *   Update opposite hook with same attached/powered state
 *   Update all wire segments between hooks: set/clear attached bit
 *
 * Sound effects:
 *   - Connect (attached, !wasAttached): click 0.4f vol, 0.7f pitch
 *   - Disconnect (!attached, wasAttached): bowhit 0.4f vol, 1.2/(rand*0.2+0.9)
 *   - Trip (powered, !wasPowered): click 0.4f vol, 0.6f pitch
 *   - Untrip (!powered, wasPowered): click 0.4f vol, 0.5f pitch
 *
 * Power:
 *   - Weak power: 15 if powered (bit 3), else 0
 *   - Strong power: 15 only to the attached wall block (direction-specific)
 *     direction 2 → side 2, direction 0 → side 3
 *     direction 1 → side 4, direction 3 → side 5
 *
 * Thread safety: Block updates on server thread.
 * JNI readiness: Simple constants and metadata parsing.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Daylight Sensor Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DaylightSensorConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 151;

    // ─── Block dimensions ───
    static constexpr float HEIGHT = 0.375f;  // 6/16 blocks

    // ─── Celestial angle adjustment ───
    // Java: angle < PI → angle += (0 - angle) * 0.2
    //       angle >= PI → angle += (2*PI - angle) * 0.2
    static constexpr float ANGLE_SMOOTHING = 0.2f;

    // ─── Signal range ───
    static constexpr int32_t MIN_SIGNAL = 0;
    static constexpr int32_t MAX_SIGNAL = 15;

    // ─── Signal calculation ───
    // Java: round(skyLight * cos(adjustedAngle)), clamped 0-15
    inline int32_t calculateSignal(int32_t skyLight, int32_t skylightSubtracted, float celestialAngle) {
        int32_t light = skyLight - skylightSubtracted;
        float adjusted = celestialAngle;

        // Smooth toward 0 or 2*PI for linear-ish behavior
        if (adjusted < static_cast<float>(M_PI)) {
            adjusted += (0.0f - adjusted) * ANGLE_SMOOTHING;
        } else {
            adjusted += (static_cast<float>(M_PI * 2.0) - adjusted) * ANGLE_SMOOTHING;
        }

        int32_t signal = static_cast<int32_t>(std::round(
            static_cast<float>(light) * std::cos(adjusted)));

        if (signal < MIN_SIGNAL) signal = MIN_SIGNAL;
        if (signal > MAX_SIGNAL) signal = MAX_SIGNAL;
        return signal;
    }

    // ─── Sky light enum ───
    static constexpr int32_t SKY_LIGHT_TYPE = 1;  // EnumSkyBlock.Sky
}

// ═══════════════════════════════════════════════════════════════════════════
// Tripwire String Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TripwireConstants {
    // ─── Block IDs ───
    static constexpr int32_t TRIPWIRE_ID = 132;
    static constexpr int32_t TRIPWIRE_HOOK_ID = 131;

    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 10;

    // ─── Metadata bits ───
    static constexpr int32_t BIT_ACTIVATED = 1;   // bit 0: entity on wire
    static constexpr int32_t BIT_SUSPENDED = 2;   // bit 1: no solid below
    static constexpr int32_t BIT_ATTACHED  = 4;   // bit 2: connected to hooks
    static constexpr int32_t BIT_DISARMED  = 8;   // bit 3: broken with shears

    inline bool isActivated(int32_t meta) { return (meta & BIT_ACTIVATED) != 0; }
    inline bool isSuspended(int32_t meta) { return (meta & BIT_SUSPENDED) != 0; }
    inline bool isAttached(int32_t meta)  { return (meta & BIT_ATTACHED)  != 0; }
    inline bool isDisarmed(int32_t meta)  { return (meta & BIT_DISARMED)  != 0; }

    // ─── Chain scan max distance ───
    // Java: for (int j = 1; j < 42; ++j)
    static constexpr int32_t MAX_CHAIN_LENGTH = 42;

    // ─── Block bounds ───
    // Ground (not suspended): 0.09375 (1.5/16)
    static constexpr float HEIGHT_GROUND = 0.09375f;
    // Suspended not attached: 0.5
    static constexpr float HEIGHT_SUSPENDED_FREE = 0.5f;
    // Suspended attached: 0.0625 to 0.15625 (1/16 to 2.5/16)
    static constexpr float HEIGHT_ATTACHED_MIN = 0.0625f;
    static constexpr float HEIGHT_ATTACHED_MAX = 0.15625f;

    // ─── Item drop ───
    static constexpr int32_t STRING_ITEM_ID = 287;
}

// ═══════════════════════════════════════════════════════════════════════════
// Tripwire Hook Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace TripwireHookConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 131;

    // ─── Metadata ───
    static constexpr int32_t DIRECTION_MASK = 3;
    static constexpr int32_t ATTACHED_FLAG = 4;
    static constexpr int32_t POWERED_FLAG = 8;

    inline int32_t getDirection(int32_t meta) { return meta & DIRECTION_MASK; }
    inline bool isAttached(int32_t meta) { return (meta & ATTACHED_FLAG) != 0; }
    inline bool isPowered(int32_t meta) { return (meta & POWERED_FLAG) != 0; }

    // ─── Direction values ───
    // 0 = south (+Z), 1 = west (-X), 2 = north (-Z), 3 = east (+X)
    static constexpr int32_t DIR_SOUTH = 0;
    static constexpr int32_t DIR_WEST = 1;
    static constexpr int32_t DIR_NORTH = 2;
    static constexpr int32_t DIR_EAST = 3;

    // ─── Placement side mapping ───
    // Java: side 2 → direction 2, side 3 → direction 0
    //       side 4 → direction 1, side 5 → direction 3
    // Requires normal cube behind
    static constexpr int32_t SIDE_TO_DIRECTION[] = {-1, -1, 2, 0, 1, 3};

    // ─── Max chain length ───
    static constexpr int32_t MAX_CHAIN_LENGTH = 42;

    // ─── Power output ───
    static constexpr int32_t SIGNAL_STRENGTH = 15;

    // Strong power direction mapping:
    // Java: direction 2 → side 2, direction 0 → side 3
    //       direction 1 → side 4, direction 3 → side 5
    inline int32_t getStrongPowerSide(int32_t direction) {
        switch (direction) {
            case 2: return 2;   // north → north face
            case 0: return 3;   // south → south face
            case 1: return 4;   // west → west face
            case 3: return 5;   // east → east face
            default: return -1;
        }
    }

    // ─── Hook bounding box ───
    // Java: f = 0.1875f (3/16)
    static constexpr float HOOK_WIDTH = 0.1875f;
    static constexpr float HOOK_Y_MIN = 0.2f;
    static constexpr float HOOK_Y_MAX = 0.8f;

    // ─── Sounds ───
    static constexpr const char* SOUND_CLICK = "random.click";
    static constexpr const char* SOUND_BOWHIT = "random.bowhit";
    static constexpr float SOUND_VOLUME = 0.4f;

    // Click pitches for different events:
    static constexpr float PITCH_CONNECT = 0.7f;     // attached, !wasAttached
    static constexpr float PITCH_TRIP = 0.6f;         // powered, !wasPowered
    static constexpr float PITCH_UNTRIP = 0.5f;       // !powered, wasPowered
    // Disconnect: bowhit with 1.2/(rand*0.2+0.9) pitch
    static constexpr float DISCONNECT_PITCH_NUM = 1.2f;
    static constexpr float DISCONNECT_PITCH_RAND = 0.2f;
    static constexpr float DISCONNECT_PITCH_BASE = 0.9f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Direction offsets for tripwire scanning
// Java: net.minecraft.util.Direction
// ═══════════════════════════════════════════════════════════════════════════

namespace TripwireDirection {
    // Java: Direction.offsetX = {0, -1, 0, 1}
    static constexpr int32_t offsetX[] = {0, -1, 0, 1};
    // Java: Direction.offsetZ = {1, 0, -1, 0}
    static constexpr int32_t offsetZ[] = {1, 0, -1, 0};

    // Java: Direction.rotateOpposite = {2, 3, 0, 1}
    static constexpr int32_t rotateOpposite[] = {2, 3, 0, 1};
}

} // namespace mccpp
