/**
 * EntityBossMobs.h — Boss entities and remaining hostile mobs.
 *
 * Java references:
 *   - net.minecraft.entity.boss.EntityWither (439 lines)
 *   - net.minecraft.entity.boss.EntityDragon (510 lines)
 *   - net.minecraft.entity.monster.EntitySilverfish (193 lines)
 *   - net.minecraft.entity.monster.EntityCaveSpider (54 lines)
 *
 * Thread safety: Entity tick on single thread per entity.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <array>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityWither — Three-headed boss mob.
// Java: net.minecraft.entity.boss.EntityWither (439 lines)
//
//   Size: 0.9×4.0, HP 300, speed 0.6, follow range 40, armor 4, XP 50
//   Fire immune, undead, immune to potions, immune to drowning
//   No fall damage, no web, can't be mounted
//   Never despawns (entityAge=0 on tick)
//
//   DataWatcher:
//     17 (center head target entity ID int)
//     18 (left head target int)
//     19 (right head target int)
//     20 (invulnerability time int)
//
//   ─── Invulnerability phase ───
//   Starts at 220 ticks, counts down
//   At 0: explosion power 7 (mob griefing check), broadcast sound 1013
//   HP starts at maxHP/3, heals 10/tick during invul
//
//   ─── Armored phase ───
//   When HP ≤ maxHP/2 (150): blocks arrow damage
//
//   ─── Flight ───
//   motionY *= 0.6
//   Chase center target: approach at 0.5 speed, accel 0.6
//   Yaw from atan2 (57.295776 conversion)
//
//   ─── Head targeting ───
//   Side heads (1,2): scan 10+rand(10) ticks, search 20×8×20 AABB
//   Random skull: after 15 shots, fire blue skull (invulnerable) at random ±10,±5
//   Center head: ranged attack 40 tick cooldown, 20 range
//   Target requires line of sight, <900 dist², alive
//   0.1% chance blue skull on center head
//
//   ─── Head positions ───
//   Center: posX, posY+3, posZ
//   Side: cos/sin(yaw+180*(n-1)) * 1.3, posY+2.2
//   Head rotation: clamp pitch ±40/s, yaw ±10/s
//
//   ─── Block breaking ───
//   After 20 ticks from damage: destroy 3×3×4 grid
//   Skip: air, bedrock, end_portal, end_portal_frame, command_block
//   Sound 1012 on success
//
//   ─── Damage immunity ───
//   Immune to: drowning, same creature attribute (undead)
//   Armored: immune to arrows
//   Invul time > 0: immune to all
//   On damage: +3 to side head shot counters, reset block break to 20
//
//   ─── Drops ───
//   Nether star (always)
//   Achievement: killWither for all players within 50×100×50
//
//   NBT: "Invul" integer
// ═══════════════════════════════════════════════════════════════════════════

class EntityWither {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0;
    float renderYawOffset = 0;
    bool isDead = false;
    int32_t entityId = 0;
    int32_t ticksExisted = 0;

    // Target IDs for 3 heads (DW 17, 18, 19)
    std::array<int32_t, 3> headTargetIds = {0, 0, 0};
    int32_t invulTime = 0; // DW 20

    // Side head timers
    std::array<int32_t, 2> nextHeadUpdate = {0, 0};
    std::array<int32_t, 2> headShotCount = {0, 0};
    int32_t blockBreakTimer = 0;

    // Head rotation animation
    std::array<float, 2> headPitch = {0, 0};
    std::array<float, 2> headYaw = {0, 0};
    std::array<float, 2> prevHeadPitch = {0, 0};
    std::array<float, 2> prevHeadYaw = {0, 0};

    static constexpr float WIDTH = 0.9f;
    static constexpr float HEIGHT = 4.0f;
    static constexpr double MAX_HEALTH = 300.0;
    static constexpr double MOVE_SPEED = 0.6;
    static constexpr double FOLLOW_RANGE = 40.0;
    static constexpr int32_t ARMOR = 4;
    static constexpr int32_t XP_VALUE = 50;

    // Invul phase
    static constexpr int32_t INVUL_TIME = 220;
    static constexpr float SPAWN_EXPLOSION_POWER = 7.0f;
    static constexpr float INVUL_HEAL_RATE = 10.0f;
    static constexpr int32_t INVUL_HEAL_INTERVAL = 10;

    // Armored phase
    static constexpr float ARMOR_THRESHOLD_RATIO = 0.5f; // HP ≤ max/2

    // Flight
    static constexpr double FLIGHT_DRAG = 0.6;
    static constexpr double CHASE_SPEED = 0.5;
    static constexpr double CHASE_ACCEL = 0.6;
    static constexpr double CHASE_MIN_DIST_SQ = 9.0;
    static constexpr float RAD_TO_DEG = 57.295776f;

    // Head targeting
    static constexpr int32_t HEAD_SCAN_BASE = 10;
    static constexpr int32_t HEAD_SCAN_RAND = 10;
    static constexpr int32_t HEAD_COOLDOWN = 40;
    static constexpr int32_t HEAD_COOLDOWN_RAND = 20;
    static constexpr int32_t BLUE_SKULL_THRESHOLD = 15;
    static constexpr double HEAD_SEARCH_XZ = 20.0;
    static constexpr double HEAD_SEARCH_Y = 8.0;
    static constexpr double MAX_TARGET_DIST_SQ = 900.0;
    static constexpr float BLUE_SKULL_CHANCE = 0.001f; // center head

    // Block breaking
    static constexpr int32_t BLOCK_BREAK_DELAY = 20;

    // Normal heal
    static constexpr float NORMAL_HEAL = 1.0f;
    static constexpr int32_t NORMAL_HEAL_INTERVAL = 20;

    // Achievement range
    static constexpr double ACH_RANGE_XZ = 50.0;
    static constexpr double ACH_RANGE_Y = 100.0;

    // Drop
    static constexpr int32_t DROP_NETHER_STAR = 399;

    // ─── Head positions ───
    // Java: func_82214_u, func_82208_v, func_82213_w
    struct HeadPosition { double x, y, z; };

    HeadPosition getHeadPos(int32_t head) const {
        if (head <= 0) return {posX, posY + 3.0, posZ};
        float angle = (renderYawOffset + 180.0f * (head - 1)) / 180.0f * static_cast<float>(M_PI);
        return {
            posX + std::cos(angle) * 1.3,
            posY + 2.2,
            posZ + std::sin(angle) * 1.3
        };
    }

    // ─── Flight physics ───
    void tickFlight(double targetX, double targetY, double targetZ) {
        motionY *= FLIGHT_DRAG;
        if (posY < targetY) {
            if (motionY < 0) motionY = 0;
            motionY += (CHASE_SPEED - motionY) * CHASE_ACCEL;
        }
        double dx = targetX - posX;
        double dz = targetZ - posZ;
        double distSq = dx * dx + dz * dz;
        if (distSq > CHASE_MIN_DIST_SQ) {
            double dist = std::sqrt(distSq);
            motionX += (dx / dist * CHASE_SPEED - motionX) * CHASE_ACCEL;
            motionZ += (dz / dist * CHASE_SPEED - motionZ) * CHASE_ACCEL;
        }
        if (motionX * motionX + motionZ * motionZ > 0.05) {
            rotationYaw = static_cast<float>(std::atan2(motionZ, motionX)) * RAD_TO_DEG - 90.0f;
        }
    }

    bool isArmored(float currentHP) const {
        return currentHP <= MAX_HEALTH * ARMOR_THRESHOLD_RATIO;
    }

    void startInvul() {
        invulTime = INVUL_TIME;
    }

    // Head rotation clamping
    static float clampAngle(float current, float target, float maxDelta) {
        float diff = target - current;
        // Wrap to -180..180
        while (diff > 180.0f) diff -= 360.0f;
        while (diff < -180.0f) diff += 360.0f;
        if (diff > maxDelta) diff = maxDelta;
        if (diff < -maxDelta) diff = -maxDelta;
        return current + diff;
    }

    static constexpr const char* LIVING_SOUND = "mob.wither.idle";
    static constexpr const char* HURT_SOUND = "mob.wither.hurt";
    static constexpr const char* DEATH_SOUND = "mob.wither.death";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityDragon — Ender Dragon boss.
// Java: net.minecraft.entity.boss.EntityDragon (510 lines)
//
//   Size: 16×8, HP 200, fire immune, no clip, volume 5
//   7 multi-part hitboxes:
//     Head: 6×6 (takes full damage)
//     Body: 8×8 → set to 5×3 in tick (takes damage/4+1)
//     Tail1,2,3: 4×4 → set to 2×2
//     Wing1,2: 4×4 → set to 4×2, 4×3
//   Ring buffer: 64 entries [yaw, posY] for animation interpolation
//
//   ─── Flight physics ───
//   animTime: 0.2/(sqrt(motionXZ)*10+1), pow(2,motionY) if not slowed
//   Target selection: 50% closest player, 50% random pos around 0,0
//     Random: Y=70+rand*50, XZ=rand*120-60, must be dist²>100 from self
//     Player: targetY = boundingBox.minY + 0.4+dist/80-1 (cap 10)
//   Gaussian wander when no target: targetXZ += Gaussian*2
//   Force new target when: dist²<100 or >22500 or horizontal/vertical collision
//
//   Vertical chase: clamp ±0.6, motionY += clampedDY * 0.1
//   Yaw: 180-atan2(dx,dz), clamp turn ±50°
//   Speed: dot product with direction vec, multiply by (f+0.5)/1.5
//   Yaw velocity: dampened 0.8, turn rate 0.7/speed
//   moveFlying(0, -1, 0.06*(thrust*steerFactor+(1-steerFactor)))
//   Slowed: motion *= 0.8
//   Drag: XZ *= (dotProduct+1)/2 * 0.15 + 0.8, Y *= 0.91
//
//   ─── Combat ───
//   Head damage: full. Body/tail/wing: f/4+1
//   Only player damage or explosions are valid
//   Wing collision: push entities (dx/dist²*4, 0.2, dz/dist²*4)
//   Head collision: 10 damage to entities
//   Block destruction: head+body AABB, skip obsidian/endstone/bedrock
//
//   ─── Ender crystal healing ───
//   Check every 10 ticks for nearest crystal within 32 blocks
//   Heal 1/10 ticks when crystal exists
//   Crystal destroyed: 10 damage to head
//
//   ─── Death ───
//   200 tick animation, rotate +20°/tick, move up 0.1/tick
//   XP: 12000 total (1000 per 5 ticks during 150-200, 2000 at tick 200)
//   Generate end portal:
//     Y=64, radius 4 circle: outer bedrock ring, inner end_portal
//     Center pillar: bedrock Y64-64+3, dragon_egg at Y64+4
//     4 torches at Y64+2 (±1,0), (0,±1)
//   Sound 1018 at tick 1
//   Large explosion particles 180-200
//
//   NBT: none (stateless across save/load)
//   Never despawns
// ═══════════════════════════════════════════════════════════════════════════

class EntityDragon {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    float renderYawOffset = 0;
    float randomYawVelocity = 0;
    bool isDead = false;
    bool slowed = false;
    bool forceNewTarget = false;
    int32_t entityId = 0;
    int32_t ticksExisted = 0;
    int32_t deathTicks = 0;

    // Target
    double targetX = 0, targetY = 100.0, targetZ = 0;

    // Ring buffer for animation interpolation
    static constexpr int32_t RING_SIZE = 64;
    double ringBuffer[RING_SIZE][3] = {};
    int32_t ringBufferIndex = -1;

    // Animation
    float prevAnimTime = 0, animTime = 0;

    static constexpr float WIDTH = 16.0f;
    static constexpr float HEIGHT = 8.0f;
    static constexpr double MAX_HEALTH = 200.0;
    static constexpr float SOUND_VOLUME = 5.0f;

    // Multi-part hitbox sizes (used during tick)
    struct PartSize { float w, h; };
    static constexpr PartSize HEAD_SIZE = {3.0f, 3.0f};
    static constexpr PartSize BODY_SIZE = {5.0f, 3.0f};
    static constexpr PartSize TAIL_SIZE = {2.0f, 2.0f};
    static constexpr PartSize WING_SIZE_1 = {4.0f, 2.0f};
    static constexpr PartSize WING_SIZE_2 = {4.0f, 3.0f};

    // Flight
    static constexpr double VERTICAL_CLAMP = 0.6;
    static constexpr double VERTICAL_ACCEL = 0.1;
    static constexpr double YAW_CLAMP = 50.0;
    static constexpr double FORCE_NEW_MIN_SQ = 100.0;
    static constexpr double FORCE_NEW_MAX_SQ = 22500.0;
    static constexpr float MOVE_THRUST = 0.06f;
    static constexpr float SLOW_FACTOR = 0.8f;
    static constexpr float YAW_DECAY = 0.8f;
    static constexpr float YAW_TURN_RATE = 0.7f;
    static constexpr double Y_DRAG = 0.91;

    // Combat
    static constexpr float HEAD_DAMAGE_MULT = 1.0f;
    static constexpr float BODY_DAMAGE_DIV = 4.0f;
    static constexpr float BODY_DAMAGE_ADD = 1.0f;
    static constexpr float COLLISION_DAMAGE = 10.0f;
    static constexpr float WING_PUSH_Y = 0.2f;

    // Crystal healing
    static constexpr int32_t CRYSTAL_CHECK_INTERVAL = 10;
    static constexpr float CRYSTAL_SEARCH_RANGE = 32.0f;
    static constexpr float CRYSTAL_HEAL = 1.0f;
    static constexpr float CRYSTAL_DESTROY_DAMAGE = 10.0f;

    // Death
    static constexpr int32_t DEATH_DURATION = 200;
    static constexpr float DEATH_ROTATE_SPEED = 20.0f;
    static constexpr double DEATH_RISE_SPEED = 0.1;
    static constexpr int32_t XP_DURING_DEATH = 1000;
    static constexpr int32_t XP_AT_DEATH = 2000;
    static constexpr int32_t DEATH_XP_START = 150;

    // End portal generation
    static constexpr int32_t PORTAL_Y = 64;
    static constexpr int32_t PORTAL_RADIUS = 4;

    // Random target
    static constexpr double RANDOM_Y_BASE = 70.0;
    static constexpr double RANDOM_Y_RANGE = 50.0;
    static constexpr double RANDOM_XZ_RANGE = 120.0;
    static constexpr double RANDOM_XZ_OFFSET = 60.0;
    static constexpr double PLAYER_Y_OFFSET_DIV = 80.0;
    static constexpr double PLAYER_Y_MAX = 10.0;

    // ─── Ring buffer interpolation ───
    // Java: getMovementOffsets(int n, float f)
    struct MovementOffset { double yaw, y, unused; };

    MovementOffset getMovementOffset(int32_t index, float partialTick) const {
        if (partialTick == 0) partialTick = 0;
        float f = 1.0f - partialTick;
        int32_t idx1 = (ringBufferIndex - index) & 0x3F;
        int32_t idx2 = (ringBufferIndex - index - 1) & 0x3F;

        double yaw = ringBuffer[idx1][0];
        double yawDelta = ringBuffer[idx2][0] - yaw;
        // Wrap to -180..180
        while (yawDelta > 180.0) yawDelta -= 360.0;
        while (yawDelta < -180.0) yawDelta += 360.0;

        return {
            yaw + yawDelta * f,
            ringBuffer[idx1][1] + (ringBuffer[idx2][1] - ringBuffer[idx1][1]) * f,
            ringBuffer[idx1][2] + (ringBuffer[idx2][2] - ringBuffer[idx1][2]) * f
        };
    }

    // ─── Part damage calculation ───
    static float calculatePartDamage(bool isHead, float rawDamage) {
        if (isHead) return rawDamage;
        return rawDamage / BODY_DAMAGE_DIV + BODY_DAMAGE_ADD;
    }

    // ─── Animation speed ───
    float calculateAnimSpeed() const {
        double horzSpeed = std::sqrt(motionX * motionX + motionZ * motionZ);
        float base = 0.2f / (static_cast<float>(horzSpeed) * 10.0f + 1.0f);
        if (slowed) return base * 0.5f;
        return base * static_cast<float>(std::pow(2.0, motionY));
    }

    // ─── End portal generation spec ───
    struct PortalBlock { int32_t x, y, z, blockId; };

    // Check if position is inside portal circle
    static bool isInPortalCircle(int32_t dx, int32_t dz, int32_t radius) {
        double r = radius - 0.5;
        return dx * dx + dz * dz <= r * r;
    }

    static bool isInInnerCircle(int32_t dx, int32_t dz, int32_t radius) {
        double r = (radius - 1) - 0.5;
        return dx * dx + dz * dz <= r * r;
    }

    static constexpr const char* LIVING_SOUND = "mob.enderdragon.growl";
    static constexpr const char* HURT_SOUND = "mob.enderdragon.hit";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntitySilverfish — Block-hiding mob that summons allies.
// Java: net.minecraft.entity.monster.EntitySilverfish (193 lines)
//
//   Size: 0.3×0.7, HP 8, speed 0.6, damage 1
//   Arthropod creature attribute
//   No walking trigger, no item drops (item 0)
//   Yaw snapped to body yaw each tick
//
//   ─── Ally summoning ───
//   On damage (entity or magic source): set cooldown 20
//   At cooldown 0: spiral search ±10×±5 for monster_egg blocks
//     - Spiral pattern: 0,1,-1,2,-2,...
//     - Break block (or replace if !mobGriefing), 50% continue
//
//   ─── Block hiding ───
//   When idle (no target, no path): rand(6) face direction
//     If adjacent block can be silverfish block: convert to monster_egg, die
//     Else: wander
//
//   ─── Attack ───
//   Melee: attackTime 20, range <1.2, Y overlap check
//
//   ─── Spawn ───
//   Valid at any light level
//   No player within 5 blocks
//   Prefers stone (path weight 10)
//
//   Sound: say/hit/kill at "mob.silverfish.*"
//   Step: 0.15 volume
// ═══════════════════════════════════════════════════════════════════════════

class EntitySilverfish {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    int32_t allySummonCooldown = 0;

    static constexpr float WIDTH = 0.3f;
    static constexpr float HEIGHT = 0.7f;
    static constexpr double MAX_HEALTH = 8.0;
    static constexpr double MOVE_SPEED = 0.6;
    static constexpr double ATTACK_DAMAGE = 1.0;
    static constexpr int32_t ATTACK_COOLDOWN = 20;
    static constexpr float ATTACK_RANGE = 1.2f;

    // Ally summoning
    static constexpr int32_t SUMMON_DELAY = 20;
    static constexpr int32_t SUMMON_SEARCH_XZ = 10;
    static constexpr int32_t SUMMON_SEARCH_Y = 5;

    // Block hiding
    static constexpr int32_t BLOCK_MONSTER_EGG = 97;

    // Spawn
    static constexpr double PLAYER_MIN_DIST = 5.0;
    static constexpr float STONE_PATH_WEIGHT = 10.0f;
    static constexpr float STEP_VOLUME = 0.15f;

    // ─── Spiral search pattern ───
    // Java uses: n = n <= 0 ? 1-n : 0-n  (generates 0,1,-1,2,-2,...)
    static int32_t spiralNext(int32_t n) {
        return n <= 0 ? 1 - n : -n;
    }

    // Start summoning
    void onDamaged(bool isEntitySource) {
        if (allySummonCooldown <= 0 && isEntitySource) {
            allySummonCooldown = SUMMON_DELAY;
        }
    }

    // Tick
    void tick() {
        if (allySummonCooldown > 0) --allySummonCooldown;
    }

    static constexpr const char* LIVING_SOUND = "mob.silverfish.say";
    static constexpr const char* HURT_SOUND = "mob.silverfish.hit";
    static constexpr const char* DEATH_SOUND = "mob.silverfish.kill";
    static constexpr const char* STEP_SOUND = "mob.silverfish.step";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityCaveSpider — Smaller poisonous spider variant.
// Java: net.minecraft.entity.monster.EntityCaveSpider (54 lines)
//
//   Extends EntitySpider
//   Size: 0.7×0.5, HP 12
//   Poison on attack: Normal=7s(140 ticks), Hard=15s(300 ticks), Easy=none
//   No spawn effects (overridden to no-op)
// ═══════════════════════════════════════════════════════════════════════════

class EntityCaveSpider {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    static constexpr float WIDTH = 0.7f;
    static constexpr float HEIGHT = 0.5f;
    static constexpr double MAX_HEALTH = 12.0;

    // Poison effect
    static constexpr int32_t POISON_ID = 19; // Potion.poison
    static constexpr int32_t POISON_AMPLIFIER = 0;
    static constexpr int32_t POISON_EASY = 0;
    static constexpr int32_t POISON_NORMAL = 7 * 20; // 7 seconds = 140 ticks
    static constexpr int32_t POISON_HARD = 15 * 20;  // 15 seconds = 300 ticks

    // Returns poison duration in ticks based on difficulty
    // Java: 0=PEACEFUL, 1=EASY, 2=NORMAL, 3=HARD
    static int32_t getPoisonDuration(int32_t difficulty) {
        switch (difficulty) {
            case 2: return POISON_NORMAL;
            case 3: return POISON_HARD;
            default: return POISON_EASY;
        }
    }
};

} // namespace mccpp
