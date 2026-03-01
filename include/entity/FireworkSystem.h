/**
 * FireworkSystem.h — Firework rocket entity, firework item, firework charge.
 *
 * Java references:
 *   - net.minecraft.entity.item.EntityFireworkRocket (122 lines)
 *   - net.minecraft.item.ItemFirework (28 lines)
 *   - net.minecraft.item.ItemFireworkCharge (12 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ENTITY FIREWORK ROCKET (EntityFireworkRocket)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Size: 0.25 × 0.25
 * Cannot be attacked (canAttackWithItem = false)
 *
 * Spawning:
 *   Position: exact click position (x + hitX, y + hitY, z + hitZ)
 *   yOffset: 0.0
 *
 * Initial motion:
 *   motionX: gaussian * 0.001
 *   motionZ: gaussian * 0.001
 *   motionY: 0.05
 *
 * Per-tick physics (onUpdate):
 *   motionX *= 1.15  (horizontal acceleration outward)
 *   motionZ *= 1.15
 *   motionY += 0.04  (upward thrust, counteracts gravity)
 *   moveEntity(motionX, motionY, motionZ)
 *
 * Rotation:
 *   horizontalSpeed = sqrt(motionX² + motionZ²)
 *   rotationYaw = atan2(motionX, motionZ) * 180/π
 *   rotationPitch = atan2(motionY, horizontalSpeed) * 180/π
 *   Wrap to ±180, then lerp 0.2 toward target
 *
 * Lifetime:
 *   Flight = NBT tag.Fireworks.Flight (byte, default 0)
 *   lifetime = 10 * (1 + Flight) + rand(6) + rand(7)
 *   Flight=0 → 10-22 ticks (0.5-1.1 sec)
 *   Flight=1 → 20-32 ticks (1.0-1.6 sec)
 *   Flight=2 → 30-42 ticks (1.5-2.1 sec)
 *   Flight=3 → 40-52 ticks (2.0-2.6 sec)
 *
 * Explosion:
 *   When fireworkAge > lifetime (server-side only):
 *   setEntityState(17) → client triggers explosion particles
 *   setDead()
 *
 * Sound: "fireworks.launch" at volume 3.0, pitch 1.0 on first tick
 *
 * Trail particles (client only):
 *   "fireworksSpark" at (posX, posY-0.3, posZ) every tick
 *   Motion: (gaussian*0.05, -motionY*0.5, gaussian*0.05)
 *
 * DataWatcher:
 *   Index 8, type 5 (ItemStack) — firework rocket item with NBT
 *
 * NBT:
 *   Life: int (current age)
 *   LifeTime: int (max age)
 *   FireworksItem: compound (ItemStack NBT)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ITEM FIREWORK (ItemFirework)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * onItemUse: spawns EntityFireworkRocket at click position
 *   Server-side: creates entity with item stack NBT
 *   Survival: decrements stack size
 *   Creative: no consumption
 *
 * Item ID: 401 (fireworks)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ITEM FIREWORK CHARGE (ItemFireworkCharge)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Empty class — just extends Item
 * All behavior is through NBT tags attached during crafting
 *
 * NBT structure (Explosion tag):
 *   Flicker: byte (twinkle effect)
 *   Trail: byte (trail particles)
 *   Type: byte (0=small, 1=large, 2=star, 3=creeper, 4=burst)
 *   Colors: int[] (base colors)
 *   FadeColors: int[] (fade colors)
 *
 * Item ID: 402 (firework_charge)
 *
 * Thread safety: Entity on server thread.
 * JNI readiness: DataWatcher for ItemStack access.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Firework Rocket Entity Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FireworkRocketConstants {
    // ─── Entity size ───
    static constexpr float WIDTH = 0.25f;
    static constexpr float HEIGHT = 0.25f;

    // ─── Initial motion ───
    static constexpr double INITIAL_MOTION_Y = 0.05;
    static constexpr double INITIAL_GAUSSIAN_SCALE = 0.001;

    // ─── Per-tick physics ───
    static constexpr double HORIZONTAL_MULTIPLIER = 1.15;  // motionX/Z *= 1.15
    static constexpr double UPWARD_THRUST = 0.04;           // motionY += 0.04

    // ─── Lifetime formula ───
    // lifetime = 10 * (1 + Flight) + rand(6) + rand(7)
    static constexpr int32_t LIFETIME_BASE_MULTIPLIER = 10;
    static constexpr int32_t LIFETIME_FLIGHT_OFFSET = 1;
    static constexpr int32_t LIFETIME_RANDOM_A = 6;
    static constexpr int32_t LIFETIME_RANDOM_B = 7;

    inline int32_t calculateLifetime(int32_t flight, int32_t randA, int32_t randB) {
        return LIFETIME_BASE_MULTIPLIER * (LIFETIME_FLIGHT_OFFSET + flight) + randA + randB;
    }

    // ─── Rotation lerp ───
    static constexpr float ROTATION_LERP = 0.2f;
    static constexpr double PI = 3.1415927410125732;  // Java float π cast to double

    // ─── Explosion ───
    static constexpr int8_t ENTITY_STATE_EXPLODE = 17;

    // ─── Sound ───
    static constexpr const char* LAUNCH_SOUND = "fireworks.launch";
    static constexpr float LAUNCH_VOLUME = 3.0f;
    static constexpr float LAUNCH_PITCH = 1.0f;

    // ─── Trail particles ───
    static constexpr const char* TRAIL_PARTICLE = "fireworksSpark";
    static constexpr double TRAIL_Y_OFFSET = -0.3;
    static constexpr double TRAIL_GAUSSIAN = 0.05;

    // ─── DataWatcher ───
    static constexpr int32_t DW_FIREWORK_ITEM = 8;
    static constexpr int32_t DW_TYPE_ITEMSTACK = 5;

    // ─── NBT keys ───
    static constexpr const char* NBT_LIFE = "Life";
    static constexpr const char* NBT_LIFETIME = "LifeTime";
    static constexpr const char* NBT_FIREWORKS_ITEM = "FireworksItem";
    static constexpr const char* NBT_FIREWORKS = "Fireworks";
    static constexpr const char* NBT_FLIGHT = "Flight";
}

// ═══════════════════════════════════════════════════════════════════════════
// Firework Item Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FireworkItemConstants {
    static constexpr int32_t ITEM_ID = 401;
}

// ═══════════════════════════════════════════════════════════════════════════
// Firework Charge Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace FireworkChargeConstants {
    static constexpr int32_t ITEM_ID = 402;

    // ─── Explosion shape types ───
    static constexpr int32_t SHAPE_SMALL_BALL = 0;
    static constexpr int32_t SHAPE_LARGE_BALL = 1;
    static constexpr int32_t SHAPE_STAR = 2;
    static constexpr int32_t SHAPE_CREEPER = 3;
    static constexpr int32_t SHAPE_BURST = 4;
    static constexpr int32_t NUM_SHAPES = 5;

    // ─── NBT keys ───
    static constexpr const char* NBT_EXPLOSION = "Explosion";
    static constexpr const char* NBT_FLICKER = "Flicker";
    static constexpr const char* NBT_TRAIL = "Trail";
    static constexpr const char* NBT_TYPE = "Type";
    static constexpr const char* NBT_COLORS = "Colors";
    static constexpr const char* NBT_FADE_COLORS = "FadeColors";
    static constexpr const char* NBT_EXPLOSIONS = "Explosions";
}

} // namespace mccpp
