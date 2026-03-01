/**
 * EnderDragonBoss.h — Ender Dragon boss entity and end portal creation.
 *
 * Java reference:
 *   - net.minecraft.entity.boss.EntityDragon (510 lines)
 *   - net.minecraft.entity.boss.EntityDragonPart (inner class)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ENTITY PROPERTIES
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Health: 200 HP (maxHealth)
 * Size: 16.0 × 8.0 blocks
 * Immunity: fire (isImmuneToFire = true)
 * Movement: noClip = true (passes through blocks, destroys them)
 * Sound volume: 5.0f (very loud)
 * Cannot despawn, cannot be directly attacked (attackEntityFrom → false)
 * Only takes damage through multi-part hitboxes (attackEntityFromPart)
 * Damage sources: only EntityPlayer or explosion damage
 *
 * ═══════════════════════════════════════════════════════════════════════
 * MULTI-PART HITBOXES (7 parts)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Part         Init Size   Runtime Size
 * ─────────── ─────────── ───────────
 * head         6.0 × 6.0   3.0 × 3.0
 * body         8.0 × 8.0   5.0 × 3.0
 * tail1        4.0 × 4.0   2.0 × 2.0
 * tail2        4.0 × 4.0   2.0 × 2.0
 * tail3        4.0 × 4.0   2.0 × 2.0
 * wing1        4.0 × 4.0   4.0 × 2.0
 * wing2        4.0 × 4.0   4.0 × 3.0
 *
 * Damage routing:
 *   - Head: full damage
 *   - Other parts: damage = damage / 4.0 + 1.0
 *
 * Head hitbox: attacks entities within 1-block expansion → 10 damage
 * Wing hitboxes: push entities outward (4-block expansion, knockback)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * RING BUFFER (flight path smoothing)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * 64-element circular buffer: double[64][3]
 *   [0] = rotation yaw
 *   [1] = Y position
 *   [2] = (unused in vanilla)
 *
 * getMovementOffsets(offset, partialTick):
 *   Interpolates between buffer[(index - offset) & 63] and [(index-offset-1) & 63]
 *   Used for head, tail, and animation positioning
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FLIGHT AI
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Target selection (setNewTarget):
 *   - 50% chance: target random player
 *   - 50% chance: random point near origin
 *     X: ±60 from (0,0), Y: 70 + rand*50 (70-120)
 *     Must be >100 distance² from current position
 *
 * Movement:
 *   - Calculate direction to target
 *   - Clamp vertical delta to ±0.6
 *   - motionY += delta * 0.1
 *   - Yaw turn rate clamped to ±50°
 *   - randomYawVelocity *= 0.8 (damping)
 *   - Speed = dot(lookVec, targetVec) * 0.5 + 0.5, clamped [0, 1]
 *   - Thrust: moveFlying(0, -1, 0.06 * (speed * nearFactor + (1-nearFactor)))
 *   - After movement: friction X/Z *= (0.8 + 0.15 * alignFactor), Y *= 0.91
 *   - If slowed (from block collision): all motion *= 0.8
 *
 * Re-targeting conditions:
 *   - forceNewTarget flag
 *   - distance² < 100 (too close)
 *   - distance² > 22500 (too far, 150 blocks)
 *   - collided horizontally or vertically
 *
 * ═══════════════════════════════════════════════════════════════════════
 * ENDER CRYSTAL HEALING
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Crystal scanning (10% per tick):
 *   - Search 32-block radius for EntityEnderCrystal
 *   - Track nearest crystal as healingEnderCrystal
 *
 * Healing:
 *   - Every 10 ticks: +1 HP (if health < maxHealth)
 *   - Crystal destroyed: 10 damage to dragon head
 *   - Client renders healing beam to crystal
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BLOCK DESTRUCTION
 * ═══════════════════════════════════════════════════════════════════════
 *
 * destroyBlocksInAABB (head and body hitboxes):
 *   - Cannot destroy: obsidian, end_stone, bedrock
 *   - Cannot destroy if mobGriefing gamerule is false
 *   - All other blocks → setBlockToAir
 *   - Returns true if any indestructible blocks were hit (slows dragon)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * DEATH SEQUENCE
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Duration: 200 ticks (10 seconds)
 * deathTicks 1: broadcast sound 1018
 * deathTicks 150-200 (every 5 ticks): drop 1000 XP (split)
 * deathTicks 180-200: huge explosion particles
 * deathTicks 200: drop 2000 final XP, create portal, setDead
 * Total XP: 12,000 (10 × 1000 + 2000)
 * Movement during death: Y += 0.1, yaw += 20° per tick
 *
 * ═══════════════════════════════════════════════════════════════════════
 * END PORTAL GENERATION (createEnderPortal)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Created at (dragonX, 64, dragonZ) on death:
 *
 * Structure (Y=63-96, radius 4):
 *   Layer Y < 64: bedrock disk (radius 3.5)
 *   Layer Y = 64: bedrock ring (3.5 to 4), end_portal blocks (radius 2.5)
 *   Layer Y > 64: air (clear above)
 *
 * Center column:
 *   Y=64: bedrock
 *   Y=65: bedrock
 *   Y=66: bedrock + 4 torches (NSEW)
 *   Y=67: bedrock
 *   Y=68: dragon_egg
 *
 * Thread safety: Entity updates on server thread.
 * JNI readiness: Simple constants and part layout.
 */
#pragma once

#include <cstdint>
#include <cmath>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Dragon Entity Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DragonConstants {
    // ─── Base stats ───
    static constexpr double MAX_HEALTH = 200.0;
    static constexpr float WIDTH = 16.0f;
    static constexpr float HEIGHT = 8.0f;
    static constexpr float SOUND_VOLUME = 5.0f;
    static constexpr bool IMMUNE_TO_FIRE = true;
    static constexpr bool NO_CLIP = true;

    // ─── Sounds ───
    static constexpr const char* SOUND_GROWL = "mob.enderdragon.growl";
    static constexpr const char* SOUND_HIT = "mob.enderdragon.hit";
    static constexpr const char* SOUND_WINGS = "mob.enderdragon.wings";
    static constexpr float WING_SOUND_VOLUME = 5.0f;
    static constexpr float WING_SOUND_PITCH_BASE = 0.8f;
    static constexpr float WING_SOUND_PITCH_RAND = 0.3f;
    // Wing flap threshold: cos crosses -0.3
    static constexpr float WING_FLAP_THRESHOLD = -0.3f;

    // ─── Death broadcast ───
    static constexpr int32_t DEATH_SOUND_ID = 1018;
}

// ═══════════════════════════════════════════════════════════════════════════
// Multi-Part Hitbox Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DragonParts {
    static constexpr int32_t NUM_PARTS = 7;

    // Part indices
    static constexpr int32_t HEAD = 0;
    static constexpr int32_t BODY = 1;
    static constexpr int32_t TAIL1 = 2;
    static constexpr int32_t TAIL2 = 3;
    static constexpr int32_t TAIL3 = 4;
    static constexpr int32_t WING1 = 5;
    static constexpr int32_t WING2 = 6;

    // Runtime sizes (width, height)
    struct PartSize {
        float width;
        float height;
    };

    static constexpr PartSize RUNTIME_SIZES[] = {
        {3.0f, 3.0f},  // head
        {5.0f, 3.0f},  // body
        {2.0f, 2.0f},  // tail1
        {2.0f, 2.0f},  // tail2
        {2.0f, 2.0f},  // tail3
        {4.0f, 2.0f},  // wing1
        {4.0f, 3.0f},  // wing2
    };

    // ─── Damage routing ───
    // Head: full damage
    // Other parts: damage = damage / 4.0 + 1.0
    static constexpr float BODY_DAMAGE_DIVISOR = 4.0f;
    static constexpr float BODY_DAMAGE_BASE = 1.0f;

    // ─── Head attack ───
    static constexpr float HEAD_ATTACK_DAMAGE = 10.0f;
    static constexpr double HEAD_ATTACK_EXPAND = 1.0;

    // ─── Wing knockback ───
    static constexpr double WING_EXPAND = 4.0;
    static constexpr double WING_Y_EXPAND = 2.0;
    static constexpr double WING_Y_OFFSET = -2.0;
    static constexpr double KNOCKBACK_Y = 0.2;

    // ─── Part positions (offsets from dragon center) ───
    // Body: (sin(yaw)*0.5, 0, -cos(yaw)*0.5)
    static constexpr float BODY_OFFSET = 0.5f;

    // Wings: ±cos(yaw)*4.5 on X, +2 Y, ±sin(yaw)*4.5 on Z
    static constexpr float WING_OFFSET = 4.5f;
    static constexpr float WING_Y = 2.0f;

    // Head: sin(yaw-yawVel*0.01)*5.5*cos(pitch), Y + height delta, -cos()*5.5*cos(pitch)
    static constexpr float HEAD_DISTANCE = 5.5f;

    // Tail: segments at 1.5 forward + (i+1)*2.0 behind
    static constexpr float TAIL_BASE_OFFSET = 1.5f;
    static constexpr float TAIL_SEGMENT_LENGTH = 2.0f;
    static constexpr float TAIL_Y_OFFSET = 1.5f;  // +1.5 Y
}

// ═══════════════════════════════════════════════════════════════════════════
// Ring Buffer Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DragonRingBuffer {
    static constexpr int32_t SIZE = 64;
    static constexpr int32_t MASK = 63;  // SIZE - 1, for circular indexing
    static constexpr int32_t COMPONENTS = 3;  // yaw, Y, (unused)
    static constexpr int32_t YAW_INDEX = 0;
    static constexpr int32_t Y_INDEX = 1;
}

// ═══════════════════════════════════════════════════════════════════════════
// Flight AI Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DragonFlightAI {
    // ─── Target selection ───
    static constexpr int32_t PLAYER_TARGET_CHANCE = 2;  // 50% = rand(2)==0
    static constexpr float TARGET_Y_BASE = 70.0f;
    static constexpr float TARGET_Y_RANGE = 50.0f;
    static constexpr float TARGET_XZ_RANGE = 120.0f;
    static constexpr float TARGET_XZ_OFFSET = 60.0f;
    static constexpr double MIN_TARGET_DISTANCE_SQ = 100.0;

    // ─── Re-targeting thresholds ───
    static constexpr double TOO_CLOSE_SQ = 100.0;    // 10 blocks
    static constexpr double TOO_FAR_SQ = 22500.0;    // 150 blocks

    // ─── Movement ───
    static constexpr float VERTICAL_CLAMP = 0.6f;
    static constexpr float VERTICAL_ACCEL = 0.1f;
    static constexpr double YAW_CLAMP = 50.0;   // ±50 degrees
    static constexpr float YAW_DAMPING = 0.8f;
    static constexpr float BASE_YAW_FACTOR = 0.7f;
    static constexpr float YAW_APPLY_SCALE = 0.1f;

    // ─── Thrust ───
    static constexpr float THRUST = 0.06f;
    static constexpr float STRAFE = 0.0f;
    static constexpr float FORWARD = -1.0f;

    // ─── Friction ───
    static constexpr float FRICTION_BASE = 0.8f;
    static constexpr float FRICTION_ALIGN = 0.15f;
    static constexpr double FRICTION_Y = 0.91;

    // ─── Slow mode ───
    static constexpr double SLOW_FACTOR = 0.8;

    // ─── Player tracking ───
    // When tracking player: targetY = player.minY + (0.4 + dist/80 - 1), capped at +10
    static constexpr double PLAYER_Y_BASE = 0.4;
    static constexpr double PLAYER_Y_DIST_SCALE = 80.0;
    static constexpr double PLAYER_Y_OFFSET = -1.0;
    static constexpr double PLAYER_Y_MAX = 10.0;

    // ─── Animation ───
    static constexpr float ANIM_SLOW_SCALE = 0.5f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Crystal Healing Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DragonCrystalHealing {
    // Scan for crystals: 10% per tick
    static constexpr int32_t SCAN_CHANCE = 10;  // rand(10) == 0
    static constexpr float SCAN_RADIUS = 32.0f;

    // Healing rate: +1 HP every 10 ticks
    static constexpr int32_t HEAL_INTERVAL = 10;  // ticksExisted % 10 == 0
    static constexpr float HEAL_AMOUNT = 1.0f;

    // Crystal destroyed: 10 damage to dragon
    static constexpr float CRYSTAL_DEATH_DAMAGE = 10.0f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Block Destruction Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DragonDestruction {
    // Indestructible blocks
    static constexpr int32_t OBSIDIAN_ID = 49;
    static constexpr int32_t END_STONE_ID = 121;
    static constexpr int32_t BEDROCK_ID = 7;

    // Gamerule
    static constexpr const char* MOB_GRIEFING_RULE = "mobGriefing";
}

// ═══════════════════════════════════════════════════════════════════════════
// Death Sequence Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DragonDeath {
    // Duration
    static constexpr int32_t TOTAL_DEATH_TICKS = 200;

    // XP drops during death (ticks 150-200, every 5 ticks = 10 times)
    static constexpr int32_t XP_DROP_START = 150;
    static constexpr int32_t XP_DROP_INTERVAL = 5;
    static constexpr int32_t XP_PER_DROP = 1000;

    // Final XP on tick 200
    static constexpr int32_t FINAL_XP = 2000;

    // Total XP: 10 × 1000 + 2000 = 12000
    static constexpr int32_t TOTAL_XP = 12000;

    // Explosion particles (ticks 180-200)
    static constexpr int32_t PARTICLE_START = 180;
    static constexpr float PARTICLE_XZ_RANGE = 8.0f;
    static constexpr float PARTICLE_Y_RANGE = 4.0f;
    static constexpr float PARTICLE_Y_OFFSET = 2.0f;

    // Death movement
    static constexpr double DEATH_Y_SPEED = 0.1;
    static constexpr float DEATH_YAW_SPEED = 20.0f;

    // Damage on hit while dying: redirects to retreat
    static constexpr float RETREAT_DISTANCE = 5.0f;
    static constexpr float RETREAT_Y_RANGE = 3.0f;
    static constexpr float RETREAT_XZ_RANGE = 2.0f;
}

// ═══════════════════════════════════════════════════════════════════════════
// End Portal Generation Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace EndPortalGen {
    // Portal Y level
    static constexpr int32_t PORTAL_Y = 64;

    // Structure radius
    static constexpr int32_t RADIUS = 4;

    // Disk thresholds (radius - 0.5 squared)
    // Outer: (4 - 0.5)² = 12.25
    static constexpr double OUTER_RADIUS_SQ = 12.25;   // (3.5)²
    // Inner: (3 - 0.5)² = 6.25
    static constexpr double INNER_RADIUS_SQ = 6.25;    // (2.5)²

    // Vertical range: Y=63 to Y=96
    static constexpr int32_t CLEAR_HEIGHT = 32;  // Y = portal_y - 1 to portal_y + 32

    // Center column blocks (from PORTAL_Y up):
    // +0: bedrock, +1: bedrock, +2: bedrock, +3: bedrock, +4: dragon_egg
    // Torches at +2: ±1X, ±1Z
    static constexpr int32_t COLUMN_HEIGHT = 4;
    static constexpr int32_t EGG_OFFSET = 4;    // dragon_egg at Y+4
    static constexpr int32_t TORCH_OFFSET = 2;  // torches at Y+2

    // Block IDs
    static constexpr int32_t BEDROCK = 7;
    static constexpr int32_t END_PORTAL = 119;
    static constexpr int32_t AIR = 0;
    static constexpr int32_t TORCH = 50;
    static constexpr int32_t DRAGON_EGG = 122;
}

} // namespace mccpp
