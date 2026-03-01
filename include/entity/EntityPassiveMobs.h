/**
 * EntityPassiveMobs.h — Core passive farm animal implementations.
 *
 * Java references:
 *   - net.minecraft.entity.passive.EntityCow (124 lines)
 *   - net.minecraft.entity.passive.EntityPig (198 lines)
 *   - net.minecraft.entity.passive.EntitySheep (241 lines)
 *   - net.minecraft.entity.passive.EntityChicken (195 lines)
 *
 * Thread safety: Entity tick on single thread per entity.
 */
#pragma once

#include <cmath>
#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityCow — Passive farm animal, milkable.
// Java: net.minecraft.entity.passive.EntityCow (124 lines)
//
//   Size: 0.9×1.3
//   Attributes: maxHealth 10, moveSpeed 0.2
//   Milking: player right-click with bucket → milk_bucket
//   Drops: leather (rand(3) + looting), beef/cooked_beef (rand(3)+1 + looting)
//   Sound volume: 0.4
//   Tempt: wheat
//   AI: swim, panic(2.0), mate(1.0), tempt(1.25,wheat), followParent(1.25),
//       wander(1.0), watchClosest(player,6), lookIdle
// ═══════════════════════════════════════════════════════════════════════════

class EntityCow {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    bool isBurning = false;
    int32_t entityId = 0;

    static constexpr float WIDTH = 0.9f;
    static constexpr float HEIGHT = 1.3f;
    static constexpr double MAX_HEALTH = 10.0;
    static constexpr double MOVE_SPEED = 0.2;
    static constexpr float SOUND_VOLUME = 0.4f;

    // Items
    static constexpr int32_t DROP_LEATHER = 334;
    static constexpr int32_t DROP_BEEF = 363;
    static constexpr int32_t DROP_COOKED_BEEF = 364;
    static constexpr int32_t ITEM_BUCKET = 325;
    static constexpr int32_t ITEM_MILK_BUCKET = 335;
    static constexpr int32_t BREED_ITEM_WHEAT = 296;

    struct DropResult {
        int32_t leatherCount;
        int32_t meatItemId;
        int32_t meatCount;
    };

    DropResult calculateDrops(int32_t rand3a, int32_t lootRandA,
                                int32_t rand3b, int32_t lootRandB) const {
        DropResult result{};
        result.leatherCount = rand3a + lootRandA;
        result.meatItemId = isBurning ? DROP_COOKED_BEEF : DROP_BEEF;
        result.meatCount = rand3b + 1 + lootRandB;
        return result;
    }

    // Milking: bucket → milk_bucket
    static bool canMilk(int32_t heldItemId) { return heldItemId == ITEM_BUCKET; }

    static constexpr const char* LIVING_SOUND = "mob.cow.say";
    static constexpr const char* HURT_SOUND = "mob.cow.hurt";
    static constexpr const char* DEATH_SOUND = "mob.cow.hurt";
    static constexpr const char* STEP_SOUND = "mob.cow.step";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityPig — Rideable passive animal.
// Java: net.minecraft.entity.passive.EntityPig (198 lines)
//
//   Size: 0.9×0.9
//   Attributes: maxHealth 10, moveSpeed 0.25
//   Saddle: DataWatcher 16 bit 0, saved as NBT "Saddle" boolean
//   Riding: right-click saddled pig, steered with carrot_on_a_stick
//   Controlled speed: 0.3 base
//   Lightning: transform to PigZombie with golden_sword
//   Fall > 5: flyPig achievement for rider
//   Breeding item: carrot
//   Drops: porkchop/cooked_porkchop (rand(3)+1 + looting), saddle if saddled
//   AI: swim, panic(1.25), controlledByPlayer(0.3), mate(1.0),
//       tempt(1.2,carrot_on_stick), tempt(1.2,carrot), followParent(1.1),
//       wander(1.0), watchClosest(player,6), lookIdle
// ═══════════════════════════════════════════════════════════════════════════

class EntityPig {
public:
    double posX = 0, posY = 0, posZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    bool isDead = false;
    bool isBurning = false;
    bool saddled = false;
    int32_t entityId = 0;

    static constexpr float WIDTH = 0.9f;
    static constexpr float HEIGHT = 0.9f;
    static constexpr double MAX_HEALTH = 10.0;
    static constexpr double MOVE_SPEED = 0.25;
    static constexpr float CONTROLLED_SPEED = 0.3f;
    static constexpr float FLY_PIG_FALL_THRESHOLD = 5.0f;

    // Items
    static constexpr int32_t DROP_PORKCHOP = 319;
    static constexpr int32_t DROP_COOKED_PORKCHOP = 320;
    static constexpr int32_t ITEM_SADDLE = 329;
    static constexpr int32_t ITEM_CARROT_ON_STICK = 398;
    static constexpr int32_t BREED_ITEM_CARROT = 391;

    struct DropResult {
        int32_t meatItemId;
        int32_t meatCount;
        bool dropSaddle;
    };

    DropResult calculateDrops(int32_t rand3, int32_t lootRand) const {
        DropResult result{};
        result.meatItemId = isBurning ? DROP_COOKED_PORKCHOP : DROP_PORKCHOP;
        result.meatCount = rand3 + 1 + lootRand;
        result.dropSaddle = saddled;
        return result;
    }

    // canBeSteered: rider holds carrot_on_a_stick
    static bool canSteer(int32_t riderHeldItemId) {
        return riderHeldItemId == ITEM_CARROT_ON_STICK;
    }

    // Lightning → PigZombie with golden sword
    bool shouldTransformOnLightning() const { return true; }

    static constexpr const char* LIVING_SOUND = "mob.pig.say";
    static constexpr const char* HURT_SOUND = "mob.pig.say";
    static constexpr const char* DEATH_SOUND = "mob.pig.death";
    static constexpr const char* STEP_SOUND = "mob.pig.step";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntitySheep — Wool-producing animal with 16 colors.
// Java: net.minecraft.entity.passive.EntitySheep (241 lines)
//
//   Size: 0.9×1.3
//   Attributes: maxHealth 8, moveSpeed 0.23
//   DataWatcher 16: low 4 bits = fleece color (0-15), bit 4 = sheared
//   Shearing: right-click with shears, not sheared, not child
//     → drop 1+rand(3) wool blocks with color, random motion
//     → setSheared(true), play "mob.sheep.shear"
//   Eat grass: regrow wool (setSheared false), baby +60 growth
//   Drops: if not sheared → 1 wool with color
//   Color mixing: parents' dye colors → crafting recipe match
//   Spawn colors (getRandomFleeceColor):
//     rand(100): <5 → 15(black), <10 → 7(gray), <15 → 8(lightGray),
//     <18 → 12(brown), else 0(white)
//     Additional: rand(500)==0 → 6(pink)
//   Fleece color table: 16 RGB entries for rendering
//   AI: swim, panic(1.25), mate(1.0), tempt(1.1,wheat), followParent(1.1),
//       eatGrass, wander(1.0), watchClosest(player,6), lookIdle
// ═══════════════════════════════════════════════════════════════════════════

class EntitySheep {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    int32_t fleeceColor = 0;
    bool sheared = false;
    int32_t sheepTimer = 0;

    static constexpr float WIDTH = 0.9f;
    static constexpr float HEIGHT = 1.3f;
    static constexpr double MAX_HEALTH = 8.0;
    static constexpr double MOVE_SPEED = 0.23;

    // Items
    static constexpr int32_t BLOCK_WOOL = 35;
    static constexpr int32_t ITEM_SHEARS = 359;
    static constexpr int32_t BREED_ITEM_WHEAT = 296;

    // Shearing: 1-3 wool
    static constexpr int32_t MIN_SHEAR_WOOL = 1;
    static constexpr int32_t MAX_SHEAR_WOOL_RAND = 3;

    // ─── Fleece color table (16 RGB entries) ───
    // Java: EntitySheep.fleeceColorTable[16][3]
    static constexpr float FLEECE_COLOR_TABLE[16][3] = {
        {1.0f, 1.0f, 1.0f},      // 0  white
        {0.85f, 0.5f, 0.2f},     // 1  orange
        {0.7f, 0.3f, 0.85f},     // 2  magenta
        {0.4f, 0.6f, 0.85f},     // 3  light_blue
        {0.9f, 0.9f, 0.2f},      // 4  yellow
        {0.5f, 0.8f, 0.1f},      // 5  lime
        {0.95f, 0.5f, 0.65f},    // 6  pink
        {0.3f, 0.3f, 0.3f},      // 7  gray
        {0.6f, 0.6f, 0.6f},      // 8  light_gray
        {0.3f, 0.5f, 0.6f},      // 9  cyan
        {0.5f, 0.25f, 0.7f},     // 10 purple
        {0.2f, 0.3f, 0.7f},      // 11 blue
        {0.4f, 0.3f, 0.2f},      // 12 brown
        {0.4f, 0.5f, 0.2f},      // 13 green
        {0.6f, 0.2f, 0.2f},      // 14 red
        {0.1f, 0.1f, 0.1f},      // 15 black
    };

    // ─── Spawn color distribution ───
    // Java: EntitySheep.getRandomFleeceColor
    //   rand(100): <5 → 15(black), <10 → 7(gray), <15 → 8(lightGray),
    //   <18 → 12(brown), else 0(white)
    //   Additional: rand(500)==0 → 6(pink)
    static int32_t getRandomFleeceColor(int32_t rand100, int32_t rand500) {
        if (rand100 < 5) return 15;   // black
        if (rand100 < 10) return 7;   // gray
        if (rand100 < 15) return 8;   // light_gray
        if (rand100 < 18) return 12;  // brown
        if (rand500 == 0) return 6;   // pink (0.2%)
        return 0;                     // white (82%)
    }

    // ─── DataWatcher encoding ───
    void setFleeceColor(int32_t color) {
        fleeceColor = color & 0xF;
    }

    void setSheared(bool s) { sheared = s; }
    bool getSheared() const { return sheared; }
    int32_t getFleeceColor() const { return fleeceColor; }

    // DW byte = (sheared ? 0x10 : 0) | (color & 0xF)
    uint8_t getDataWatcherByte() const {
        return static_cast<uint8_t>((sheared ? 0x10 : 0) | (fleeceColor & 0xF));
    }

    void setFromDataWatcher(uint8_t dw) {
        fleeceColor = dw & 0xF;
        sheared = (dw & 0x10) != 0;
    }

    // ─── Shearing ───
    struct ShearResult {
        int32_t woolCount;
        int32_t woolColor;
    };

    ShearResult shear(int32_t rand3) {
        ShearResult result{};
        result.woolCount = MIN_SHEAR_WOOL + rand3;
        result.woolColor = fleeceColor;
        sheared = true;
        return result;
    }

    bool canShear(bool isChild) const { return !sheared && !isChild; }

    // ─── Eat grass → regrow wool ───
    void eatGrassBonus(bool isChild, int32_t& growthOut) {
        sheared = false;
        if (isChild) growthOut = 60;
    }

    // ─── Child color mixing ───
    // Java: getDyeBasedOnParents
    //   - Convert both parents' fleece to dye (15 - fleeceColor)
    //   - Try crafting recipe for 2 dyes
    //   - If valid → baby gets crafted dye color (15 - result)
    //   - If invalid → random parent's dye color
    static int32_t mixColors(int32_t parentColor1, int32_t parentColor2,
                               int32_t craftedDyeMeta, bool craftMatch, bool randBool) {
        int32_t dye1 = 15 - parentColor1;
        int32_t dye2 = 15 - parentColor2;
        if (craftMatch) return 15 - craftedDyeMeta;
        return 15 - (randBool ? dye1 : dye2);
    }

    static constexpr const char* LIVING_SOUND = "mob.sheep.say";
    static constexpr const char* HURT_SOUND = "mob.sheep.say";
    static constexpr const char* DEATH_SOUND = "mob.sheep.say";
    static constexpr const char* STEP_SOUND = "mob.sheep.step";
    static constexpr const char* SHEAR_SOUND = "mob.sheep.shear";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityChicken — Egg-laying passive animal with slow fall.
// Java: net.minecraft.entity.passive.EntityChicken (195 lines)
//
//   Size: 0.3×0.7
//   Attributes: maxHealth 4, moveSpeed 0.25
//   Slow fall: motionY *= 0.6 when falling (!onGround && motionY < 0)
//   No fall damage: fall() is empty
//   Wing flapping: destPos += (onGround ? -1 : 4) * 0.3, clamped [0,1]
//     wingFlap *= 0.9, if airborne && wingFlap < 1 → wingFlap = 1
//   Egg laying: every 6000-12000 ticks, play "mob.chicken.plop", drop egg
//     Not in creative, not child, not chicken jockey
//   Chicken jockey: field_152118_bv (IsChickenJockey NBT)
//     canDespawn: true when jockey + no rider
//     XP: 10 when jockey
//   Breeding item: any ItemSeeds
//   Drops: feather (rand(3) + looting), chicken/cooked_chicken (1)
//   Rider position: posX + sin(yaw)*0.1, posY + height*0.5 + riderOffset, posZ - cos(yaw)*0.1
//   AI: swim, panic(1.4), mate(1.0), tempt(1.0,wheat_seeds), followParent(1.1),
//       wander(1.0), watchClosest(player,6), lookIdle
// ═══════════════════════════════════════════════════════════════════════════

class EntityChicken {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionY = 0;
    bool isDead = false;
    bool isBurning = false;
    bool onGround = false;
    int32_t entityId = 0;

    // Chicken-specific
    int32_t timeUntilNextEgg = 6000;
    bool isChickenJockey = false;

    // Wing flapping (render)
    float wingRotation = 0;
    float destPos = 0;
    float prevDestPos = 0;
    float wingFlap = 1.0f;
    float prevWingRotation = 0;

    static constexpr float WIDTH = 0.3f;
    static constexpr float HEIGHT = 0.7f;
    static constexpr double MAX_HEALTH = 4.0;
    static constexpr double MOVE_SPEED = 0.25;

    // Egg timing
    static constexpr int32_t EGG_MIN_TIME = 6000;
    static constexpr int32_t EGG_RAND_TIME = 6000;

    // Slow fall
    static constexpr double FALL_DAMPEN = 0.6;

    // Items
    static constexpr int32_t DROP_FEATHER = 288;
    static constexpr int32_t DROP_CHICKEN = 365;
    static constexpr int32_t DROP_COOKED_CHICKEN = 366;
    static constexpr int32_t ITEM_EGG = 344;
    static constexpr int32_t JOCKEY_XP = 10;

    // Rider offset for jockey
    static constexpr float RIDER_OFFSET_X = 0.1f;

    // ═══════════════════════════════════════════════════════════════════════
    // onLivingUpdate — Wing flapping + slow fall + egg laying.
    // ═══════════════════════════════════════════════════════════════════════

    struct LivingTickResult {
        bool layEgg;
        bool playPlop;
    };

    LivingTickResult onLivingUpdate(bool isChild) {
        LivingTickResult result{};

        // Wing animation
        prevWingRotation = wingRotation;
        prevDestPos = destPos;
        destPos += (onGround ? -1.0f : 4.0f) * 0.3f;
        if (destPos < 0) destPos = 0;
        if (destPos > 1.0f) destPos = 1.0f;

        if (!onGround && wingFlap < 1.0f) wingFlap = 1.0f;
        wingFlap *= 0.9f;

        // Slow fall
        if (!onGround && motionY < 0) {
            motionY *= FALL_DAMPEN;
        }

        wingRotation += wingFlap * 2.0f;

        // Egg laying
        if (!isChild && !isChickenJockey) {
            --timeUntilNextEgg;
            if (timeUntilNextEgg <= 0) {
                result.layEgg = true;
                result.playPlop = true;
                // timeUntilNextEgg reset by caller: rand(6000) + 6000
            }
        }

        return result;
    }

    void resetEggTimer(int32_t rand6000) {
        timeUntilNextEgg = rand6000 + EGG_MIN_TIME;
    }

    // No fall damage
    void fall(float /*distance*/) { /* no-op */ }

    // Breeding: any seeds
    static bool isBreedingItem(bool isSeeds) { return isSeeds; }

    // Can despawn only if chicken jockey with no rider
    bool canDespawn(bool hasRider) const {
        return isChickenJockey && !hasRider;
    }

    // XP
    int32_t getExperiencePoints(int32_t baseXP) const {
        return isChickenJockey ? JOCKEY_XP : baseXP;
    }

    struct DropResult {
        int32_t featherCount;
        int32_t meatItemId;
    };

    DropResult calculateDrops(int32_t rand3, int32_t lootRand) const {
        DropResult result{};
        result.featherCount = rand3 + lootRand;
        result.meatItemId = isBurning ? DROP_COOKED_CHICKEN : DROP_CHICKEN;
        return result;
    }

    static constexpr const char* LIVING_SOUND = "mob.chicken.say";
    static constexpr const char* HURT_SOUND = "mob.chicken.hurt";
    static constexpr const char* DEATH_SOUND = "mob.chicken.hurt";
    static constexpr const char* STEP_SOUND = "mob.chicken.step";
    static constexpr const char* EGG_SOUND = "mob.chicken.plop";
};

} // namespace mccpp
