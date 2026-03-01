/**
 * EntityMiscItems.h — Miscellaneous item entities.
 *
 * Java references:
 *   - net.minecraft.entity.item.EntityTNTPrimed (96 lines)
 *   - net.minecraft.entity.item.EntityFallingBlock (210 lines)
 *   - net.minecraft.entity.item.EntityPainting (83 lines)
 *   - net.minecraft.entity.item.EntityItemFrame (158 lines)
 *   - net.minecraft.entity.item.EntityEnderCrystal (84 lines)
 *
 * Thread safety: Entity tick on single thread per entity.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <array>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityTNTPrimed — Lit TNT entity.
// Java: net.minecraft.entity.item.EntityTNTPrimed (96 lines)
//
//   Size: 0.98×0.98, yOffset = height/2
//   Prevents entity spawning
//   Initial motion: random angle, motionXZ = -sin/cos * 0.02, motionY = 0.2
//   Fuse: 80 ticks (default)
//   Gravity: 0.04, drag: 0.98 all axes
//   On ground: XZ friction 0.7, Y bounce -0.5
//   Explode: power 4.0, causes fire
//   Smoke particle while fuse > 0
//   Can be collided with while alive
//   Can't trigger walking
//   NBT: "Fuse" byte
// ═══════════════════════════════════════════════════════════════════════════

class EntityTNTPrimed {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    bool isDead = false;
    bool onGround = false;
    int32_t entityId = 0;
    int32_t fuse = 80;
    int32_t placerEntityId = -1;

    static constexpr float WIDTH = 0.98f;
    static constexpr float HEIGHT = 0.98f;
    static constexpr int32_t DEFAULT_FUSE = 80;
    static constexpr double GRAVITY = 0.04;
    static constexpr double DRAG = 0.98;
    static constexpr double GROUND_FRICTION = 0.7;
    static constexpr double GROUND_BOUNCE_Y = -0.5;
    static constexpr float EXPLOSION_POWER = 4.0f;
    static constexpr bool CAUSES_FIRE = true;
    static constexpr double LAUNCH_SPEED = 0.02;
    static constexpr double LAUNCH_Y = 0.2;

    // ─── Initial launch ───
    // Java: random angle = Math.random() * PI * 2
    struct LaunchResult {
        double motionX, motionY, motionZ;
    };

    static LaunchResult calculateLaunch(double randomAngle) {
        return {
            -std::sin(randomAngle) * LAUNCH_SPEED,
            LAUNCH_Y,
            -std::cos(randomAngle) * LAUNCH_SPEED
        };
    }

    // ─── Physics tick ───
    void tickMotion() {
        motionY -= GRAVITY;
        posX += motionX;
        posY += motionY;
        posZ += motionZ;
        motionX *= DRAG;
        motionY *= DRAG;
        motionZ *= DRAG;
        if (onGround) {
            motionX *= GROUND_FRICTION;
            motionZ *= GROUND_FRICTION;
            motionY *= GROUND_BOUNCE_Y;
        }
    }

    bool shouldExplode() {
        return fuse-- <= 0;
    }

    static constexpr const char* PARTICLE = "smoke";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityFallingBlock — Sand/gravel/anvil falling entity.
// Java: net.minecraft.entity.item.EntityFallingBlock (210 lines)
//
//   Size: 0.98×0.98, yOffset = height/2
//   Gravity: 0.04, drag: 0.98 all axes
//   On ground: XZ friction 0.7, Y bounce -0.5
//   First tick: if source block matches, remove it, else die
//   Ground landing:
//     - Not piston_extension: place block if canPlace + !canFallBelow
//     - Copy tile entity data (skip x/y/z keys)
//     - Or drop item (shouldDropItem && !destroyed)
//   Timeout: fallTime>100 at y<1||y>256 or fallTime>600: drop and die
//   Air block material: die immediately
//
//   ─── Anvil damage ───
//   hurtEntities default false (true if anvil block)
//   fallHurtAmount: 2.0, fallHurtMax: 40
//   Damage: floor((fall-1) * fallHurtAmount), capped at fallHurtMax
//   Anvil degradation: 5%+5%*n chance, metadata damage++, >2 breaks
//   DamageSource: anvil (if anvil block) or fallingBlock
//
//   NBT: Tile(byte)/TileID(int), Data(byte), Time(byte),
//         DropItem(bool), HurtEntities(bool), FallHurtAmount(float),
//         FallHurtMax(int), TileEntityData(compound)
// ═══════════════════════════════════════════════════════════════════════════

class EntityFallingBlock {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    bool isDead = false;
    bool onGround = false;
    int32_t entityId = 0;

    int32_t blockId = 0;
    int32_t metadata = 0;
    int32_t fallTime = 0;
    bool shouldDropItem = true;
    bool destroyed = false;      // field_145808_f
    bool hurtEntities = false;
    int32_t fallHurtMax = 40;
    float fallHurtAmount = 2.0f;
    bool hasTileEntityData = false;

    static constexpr float WIDTH = 0.98f;
    static constexpr float HEIGHT = 0.98f;
    static constexpr double GRAVITY = 0.04;
    static constexpr double DRAG = 0.98;
    static constexpr double GROUND_FRICTION = 0.7;
    static constexpr double GROUND_BOUNCE_Y = -0.5;
    static constexpr int32_t BLOCK_ANVIL = 145;
    static constexpr int32_t BLOCK_PISTON_EXT = 36;
    static constexpr int32_t BLOCK_SAND = 12;
    static constexpr int32_t TIMEOUT_HEIGHT = 100;
    static constexpr int32_t TIMEOUT_TICKS = 600;
    static constexpr int32_t Y_MIN = 1;
    static constexpr int32_t Y_MAX = 256;

    // ─── Physics tick ───
    void tickMotion() {
        ++fallTime;
        motionY -= GRAVITY;
        posX += motionX;
        posY += motionY;
        posZ += motionZ;
        motionX *= DRAG;
        motionY *= DRAG;
        motionZ *= DRAG;
    }

    void applyGroundFriction() {
        motionX *= GROUND_FRICTION;
        motionZ *= GROUND_FRICTION;
        motionY *= GROUND_BOUNCE_Y;
    }

    // ─── Fall damage ───
    // Java: ceil(fall-1) * fallHurtAmount, capped at fallHurtMax
    int32_t calculateFallDamage(float fallDistance) const {
        int32_t raw = static_cast<int32_t>(std::ceil(fallDistance - 1.0f));
        if (raw <= 0) return 0;
        int32_t dmg = static_cast<int32_t>(std::floor(raw * fallHurtAmount));
        return dmg > fallHurtMax ? fallHurtMax : dmg;
    }

    // ─── Anvil degradation ───
    // Java: 5%+5%*n chance, metadata damage++, >2 breaks
    struct AnvilResult {
        bool degraded;
        int32_t newMetadata;
        bool broken;
    };

    AnvilResult checkAnvilDamage(float fallDist, float randFloat) const {
        AnvilResult r{};
        int32_t n = static_cast<int32_t>(std::ceil(fallDist - 1.0f));
        if (n <= 0) return r;
        float chance = 0.05f + n * 0.05f;
        if (randFloat < chance) {
            int32_t stage = metadata >> 2;
            int32_t orient = metadata & 3;
            if (++stage > 2) {
                r.broken = true;
            } else {
                r.newMetadata = orient | (stage << 2);
                r.degraded = true;
            }
        }
        return r;
    }

    bool shouldTimeout(int32_t y) const {
        return (fallTime > TIMEOUT_HEIGHT && (y < Y_MIN || y > Y_MAX)) || fallTime > TIMEOUT_TICKS;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityPainting — Wall painting with 26 art types.
// Java: net.minecraft.entity.item.EntityPainting (83 lines)
//   + EntityPainting$EnumArt
//
//   Extends EntityHanging
//   Random selection from art types that fit on the wall
//   Drop: painting item (not in creative)
//   NBT: "Motive" string (art title)
//   Default fallback: Kebab
// ═══════════════════════════════════════════════════════════════════════════

class EntityPainting {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;
    int32_t hangingDirection = 0;
    int32_t hangingX = 0, hangingY = 0, hangingZ = 0;

    // ─── EnumArt ───
    // Java: EntityPainting$EnumArt — 26 vanilla paintings
    struct ArtType {
        const char* title;
        int32_t sizeX; // pixels (16 = 1 block)
        int32_t sizeY;
    };

    static constexpr int32_t NUM_ART = 26;
    static constexpr std::array<ArtType, NUM_ART> ART_TYPES = {{
        {"Kebab",           16, 16},
        {"Aztec",           16, 16},
        {"Alban",           16, 16},
        {"Aztec2",          16, 16},
        {"Bomb",            16, 16},
        {"Plant",           16, 16},
        {"Wasteland",       16, 16},
        {"Pool",            32, 16},
        {"Courbet",         32, 16},
        {"Sea",             32, 16},
        {"Sunset",          32, 16},
        {"Creebet",         32, 16},
        {"Wanderer",        16, 32},
        {"Graham",          16, 32},
        {"Match",           32, 32},
        {"Bust",            32, 32},
        {"Stage",           32, 32},
        {"Void",            32, 32},
        {"SkullAndRoses",   32, 32},
        {"Wither",          32, 32},
        {"Fighters",        64, 32},
        {"Pointer",         64, 64},
        {"Pigscene",        64, 64},
        {"BurningSkull",    64, 64},
        {"Skeleton",        64, 48},
        {"DonkeyKong",      64, 48},
    }};

    int32_t artIndex = 0; // index into ART_TYPES

    int32_t getWidthPixels() const { return ART_TYPES[artIndex].sizeX; }
    int32_t getHeightPixels() const { return ART_TYPES[artIndex].sizeY; }
    const char* getTitle() const { return ART_TYPES[artIndex].title; }

    // Find art by title, returns 0 (Kebab) if not found
    static int32_t findArtByTitle(const char* title) {
        for (int32_t i = 0; i < NUM_ART; ++i) {
            // Simple strcmp equivalent
            const char* a = ART_TYPES[i].title;
            const char* b = title;
            bool match = true;
            while (*a && *b) {
                if (*a++ != *b++) { match = false; break; }
            }
            if (match && *a == *b) return i;
        }
        return 0; // Kebab default
    }

    static constexpr int32_t DROP_ITEM = 321; // painting
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityItemFrame — Wall-mounted item display.
// Java: net.minecraft.entity.item.EntityItemFrame (158 lines)
//
//   Extends EntityHanging, size 9×9 pixels
//   DataWatcher:
//     2 (ItemStack, type 5)
//     3 (rotation byte, mod 4)
//   Item drop chance: 1.0 (default)
//   Attack: first hit drops item, second hit drops frame
//   Interact: place held item (stackSize=1) or rotate existing
//   Map tracking: removes "frame-{entityId}" from map data
//   Creative: no item/frame drop
//   NBT: "Item" compound, "ItemRotation" byte, "ItemDropChance" float
// ═══════════════════════════════════════════════════════════════════════════

class EntityItemFrame {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;
    int32_t hangingDirection = 0;
    int32_t hangingX = 0, hangingY = 0, hangingZ = 0;

    // DW state
    bool hasItem = false;
    int32_t displayedItemId = 0;
    int32_t displayedItemDamage = 0;
    int32_t displayedItemCount = 0;
    int32_t rotation = 0; // 0-3
    float itemDropChance = 1.0f;

    static constexpr int32_t WIDTH_PIXELS = 9;
    static constexpr int32_t HEIGHT_PIXELS = 9;
    static constexpr int32_t MAX_ROTATION = 4;
    static constexpr int32_t DROP_FRAME_ITEM = 389; // item_frame

    void setRotation(int32_t r) { rotation = r % MAX_ROTATION; }
    void rotate() { setRotation(rotation + 1); }

    bool shouldDropItem(float randFloat) const {
        return hasItem && randFloat < itemDropChance;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityEnderCrystal — End crystal on obsidian pillars.
// Java: net.minecraft.entity.item.EntityEnderCrystal (84 lines)
//
//   Size: 2×2, yOffset = height/2
//   HP: 5, DW 8 = health int
//   innerRotation: initialized to rand(100000)
//   End dimension: places fire block at position each tick
//   On damage: HP = 0, explosion power 6.0, causes fire
//   Can be collided with
//   Can't trigger walking
//   Prevents entity spawning
//   No NBT save/load
// ═══════════════════════════════════════════════════════════════════════════

class EntityEnderCrystal {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    int32_t innerRotation = 0;
    int32_t health = 5;

    static constexpr float WIDTH = 2.0f;
    static constexpr float HEIGHT = 2.0f;
    static constexpr int32_t INITIAL_HEALTH = 5;
    static constexpr float EXPLOSION_POWER = 6.0f;
    static constexpr bool CAUSES_FIRE = true;
    static constexpr int32_t ROTATION_RANGE = 100000;
    static constexpr int32_t BLOCK_FIRE = 51;

    void tick() { ++innerRotation; }

    bool onDamage() {
        if (isDead) return false;
        health = 0;
        isDead = true;
        return true; // should explode
    }
};

} // namespace mccpp
