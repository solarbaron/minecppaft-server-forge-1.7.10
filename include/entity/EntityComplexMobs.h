/**
 * EntityComplexMobs.h — Villager and Horse implementations.
 *
 * Java references:
 *   - net.minecraft.entity.passive.EntityVillager (581 lines)
 *   - net.minecraft.entity.passive.EntityHorse (1195 lines)
 *
 * Thread safety: Entity tick on single thread per entity.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <array>
#include <utility>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityVillager — NPC with profession-based trading.
// Java: net.minecraft.entity.passive.EntityVillager (581 lines)
//
//   Size: 0.6×1.8, speed 0.5
//   5 professions: 0=Farmer, 1=Librarian, 2=Priest, 3=Blacksmith, 4=Butcher
//   DataWatcher 16 (profession int)
//   Village binding: 70+rand(50) ticks, radius × 0.6
//   Trading:
//     - func_146091_a: buys item for emeralds (quantity from villagersSellingList)
//     - func_146089_b: sells item for emeralds (price from blacksmithSellingList)
//     - Probability adjustment: existing + sqrt(numTrades)*0.2, cap 0.9
//     - Trade reset: 40 ticks timeout, re-enable disabled recipes (rand(6)+rand(6)+2 uses)
//     - New recipe added on reset (1 per cycle)
//     - +1 village reputation on trade reset
//     - Regen 200 ticks on trade reset
//     - Wealth tracks emeralds earned
//   Librarian: 7% enchanted book (random enchant, price 2+rand(5+level*10)+3*level)
//   Priest: 5% enchanted items
//   Farmer: gravel→flint recipe 50%
//   Reputation:
//     - Player attacks: -1 (adult) / -3 (child)
//     - Player kills: -2
//     - Mob kills: end mating season
//     - No killer within 16: end mating season
//   NPC behaviors: mating, playing, flee zombies 8 blocks
//   No despawn, no leashing
//   Sounds: trading→haggle, else→idle
//   AI: swim, flee zombie 8, trade, look@trade, moveIndoors, restrictDoor,
//       openDoor, moveToRestrict 0.6, mate, followGolem, play 0.32,
//       watch(player 3/1.0), watch(villager 5/0.02), wander 0.6, watch(mob 8)
// ═══════════════════════════════════════════════════════════════════════════

class EntityVillager {
public:
    double posX = 0, posY = 0, posZ = 0;
    bool isDead = false;
    int32_t entityId = 0;

    int32_t profession = 0;
    int32_t wealth = 0;
    bool isMating = false;
    bool isPlaying = false;
    int32_t randomTickDivider = 0;
    int32_t timeUntilReset = 0;
    bool needsInitialization = false;
    bool isLookingForHome = false;

    // ─── Constants ───
    static constexpr float WIDTH = 0.6f;
    static constexpr float HEIGHT = 1.8f;
    static constexpr double MOVE_SPEED = 0.5;
    static constexpr int32_t NUM_PROFESSIONS = 5;
    static constexpr int32_t HOME_CHECK_BASE = 70;
    static constexpr int32_t HOME_CHECK_RAND = 50;
    static constexpr float VILLAGE_RADIUS_MULT = 0.6f;
    static constexpr int32_t TRADE_RESET_TIME = 40;
    static constexpr int32_t REGEN_DURATION = 200;
    static constexpr float FLEE_ZOMBIE_RANGE = 8.0f;
    static constexpr double FLEE_SPEED = 0.6;
    static constexpr int32_t NEW_HOME_REPUTATION = 5;
    static constexpr int32_t ITEM_EMERALD = 388;

    // Reputation values
    static constexpr int32_t REP_PLAYER_ATTACKS_ADULT = -1;
    static constexpr int32_t REP_PLAYER_ATTACKS_CHILD = -3;
    static constexpr int32_t REP_PLAYER_KILLS = -2;
    static constexpr int32_t REP_TRADE_COMPLETE = 1;

    // ─── Trading price tables ───
    // Buy prices: items villager buys (player gives items, gets emerald)
    // Format: {itemId, minQty, maxQty}
    struct PriceRange {
        int32_t itemId;
        int32_t minQty;
        int32_t maxQty;
    };

    static constexpr std::array<PriceRange, 20> BUY_PRICES = {{
        {263,  16, 24},  // coal
        {265,  8, 10},   // iron_ingot
        {266,  8, 10},   // gold_ingot
        {264,  4,  6},   // diamond
        {339, 24, 36},   // paper
        {340, 11, 13},   // book
        {387,  1,  1},   // written_book
        {368,  3,  4},   // ender_pearl
        {381,  2,  3},   // ender_eye
        {319, 14, 18},   // porkchop
        {363, 14, 18},   // beef
        {365, 14, 18},   // chicken
        {350,  9, 13},   // cooked_fish
        {295, 34, 48},   // wheat_seeds
        {362, 30, 38},   // melon_seeds
        {361, 30, 38},   // pumpkin_seeds
        {296, 18, 22},   // wheat
        // wool (block 35→item) 14-22
        {371, 36, 64},   // rotten_flesh
        {0,   14, 22},   // wool placeholder
        {0, 0, 0},       // sentinel
    }};

    // Sell prices: items villager sells (player pays emeralds, gets item)
    // Negative values = player gets multiple items for 1 emerald
    static constexpr std::array<PriceRange, 40> SELL_PRICES = {{
        {259,  3,  4},   // flint_and_steel
        {359,  3,  4},   // shears
        {267,  7, 11},   // iron_sword
        {276, 12, 14},   // diamond_sword
        {258,  6,  8},   // iron_axe
        {279,  9, 12},   // diamond_axe
        {257,  7,  9},   // iron_pickaxe
        {278, 10, 12},   // diamond_pickaxe
        {256,  4,  6},   // iron_shovel
        {277,  7,  8},   // diamond_shovel
        {292,  4,  6},   // iron_hoe
        {293,  7,  8},   // diamond_hoe
        {309,  4,  6},   // iron_boots
        {313,  7,  8},   // diamond_boots
        {306,  4,  6},   // iron_helmet
        {310,  7,  8},   // diamond_helmet
        {307, 10, 14},   // iron_chestplate
        {311, 16, 19},   // diamond_chestplate
        {308,  8, 10},   // iron_leggings
        {312, 11, 14},   // diamond_leggings
        {305,  5,  7},   // chainmail_boots
        {302,  5,  7},   // chainmail_helmet
        {303, 11, 15},   // chainmail_chestplate
        {304,  9, 11},   // chainmail_leggings
        {297, -4, -2},   // bread (negative = player gets abs(qty) for 1 emerald)
        {360, -8, -4},   // melon
        {260, -8, -4},   // apple
        {357,-10, -7},   // cookie
        // glass block -5 to -3
        // bookshelf 3-4
        {298,  4,  5},   // leather_chestplate
        {301,  2,  4},   // leather_boots
        {298,  2,  4},   // leather_helmet
        {300,  2,  4},   // leather_leggings
        {329,  6,  8},   // saddle
        {384, -4, -1},   // xp_bottle
        {331, -4, -1},   // redstone
        {345, 10, 12},   // compass
        {347, 10, 12},   // clock
        {262,-12, -8},   // arrow
        {0, 0, 0},       // sentinel
    }};

    // ─── Trade probability ───
    // Java: adjustProbability adds sqrt(numTrades)*0.2, caps at 0.9
    static float adjustProbability(float base, int32_t numTrades) {
        float adjusted = base + std::sqrt(static_cast<float>(numTrades)) * 0.2f;
        if (adjusted > 0.9f) return 0.9f - (adjusted - 0.9f);
        return adjusted;
    }

    // Price lookup
    static int32_t randomPrice(int32_t min, int32_t max, int32_t randInt) {
        if (min >= max) return min;
        return min + randInt % (max - min);
    }

    // ─── Trade reset ───
    struct TradeResetResult {
        bool shouldRegen;
        bool updateReputation;
        int32_t regenDuration;
    };

    TradeResetResult tickTradeReset(bool isTrading) {
        TradeResetResult result{};
        if (!isTrading && timeUntilReset > 0) {
            if (--timeUntilReset <= 0) {
                result.shouldRegen = true;
                result.regenDuration = REGEN_DURATION;
                if (needsInitialization) {
                    needsInitialization = false;
                    result.updateReputation = true;
                }
            }
        }
        return result;
    }

    // ─── Sounds ───
    const char* getLivingSound(bool trading) const {
        return trading ? "mob.villager.haggle" : "mob.villager.idle";
    }

    bool canDespawn() const { return false; }
    bool allowLeashing() const { return false; }

    static constexpr const char* HURT_SOUND = "mob.villager.hit";
    static constexpr const char* DEATH_SOUND = "mob.villager.death";
    static constexpr const char* YES_SOUND = "mob.villager.yes";
    static constexpr const char* NO_SOUND = "mob.villager.no";
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityHorse — Rideable mob with 5 types and genetic breeding.
// Java: net.minecraft.entity.passive.EntityHorse (1195 lines)
//
//   Size: 1.4×1.6, max HP 53, speed 0.225, volume 0.8, talk interval 400
//   5 types: 0=Horse, 1=Donkey, 2=Mule, 3=Zombie, 4=Skeleton
//   7 base colors × 5 markings = 35 variants (low byte=color, high byte=marking)
//   Armor: 0=none(0), 1=iron(5), 2=gold(7), 3=diamond(11)
//   DataWatcher:
//     16 (flags int: 2=tame, 4=saddle, 8=chest, 16=bred, 32=eating, 64=rearing, 128=mouth)
//     19 (type byte)
//     20 (variant int)
//     21 (owner UUID string)
//     22 (armor index int)
//   Custom attribute: horse.jumpStrength (default 0.7, range 0-2)
//   Temper: 0-100, incremented by food, taming succeeds when temper exceeds random threshold
//   Inventory: 2 slots (saddle+armor) for horse, 17 for chested donkey/mule
//
//   ─── Rider movement physics ───
//   Strafe: rider.moveStrafing * 0.5
//   Forward: rider.moveForward (backward *= 0.25, reset gallop)
//   Step height: 1.0 when ridden (0.5 otherwise)
//   Jump movement factor: aimMoveSpeed * 0.1
//   Jump: motionY = jumpStrength * jumpPower
//     + jump potion bonus (amplifier+1) * 0.1
//     + horizontal push -0.4*sin(yaw)*jumpPower in X, 0.4*cos(yaw)*jumpPower in Z
//   Jump power: 0.4 + 0.4 * charge/90 (cap 1.0 at charge ≥ 90)
//   Rearing blocks movement when on ground and not jumping
//
//   ─── Genetic breeding ───
//   HP: (parent1.baseHP + parent2.baseHP + random15+rand8+rand9) / 3
//   Jump: (parent1.jump + parent2.jump + 0.4+rand*0.2*3) / 3
//   Speed: (parent1.speed + parent2.speed + (0.45+rand*0.3*3)*0.25) / 3
//   Horse × Horse → Horse (variant: 4/9 parent1, 4/9 parent2, 1/9 random color
//     marking: 2/5 parent1, 2/5 parent2, 1/5 random)
//   Horse × Donkey → Mule (no variant)
//   Mating: no rider, no mount, tamed, adult, not sterile, full HP
//   Sterile: undead or mule
//
//   ─── Food ───
//   Wheat: heal 2, growth 60, temper 3
//   Sugar: heal 1, growth 30, temper 3
//   Bread: heal 7, growth 180, temper 3
//   Hay: heal 20, growth 180, temper 0
//   Apple: heal 3, growth 60, temper 3
//   Golden carrot: heal 4, growth 60, temper 5, breeding
//   Golden apple: heal 10, growth 240, temper 10, breeding
//
//   ─── Sounds ───
//   Type 0: horse.idle/hit/death/angry
//   Type 1,2: donkey.idle/hit/death/angry
//   Type 3: zombie.idle/hit/death (no angry)
//   Type 4: skeleton.idle/hit/death (no angry)
//   Steps: gallop (rider, every 3, >5), breathe 10%, wood (≤5 or wood block), soft
//
//   ─── Misc ───
//   Fall damage: ceil(dist*0.5-3), rider also takes damage
//   Foal spawn: 20% (age -24000)
//   Undead: HP 15, speed 0.2, can't breed/eat/leash
//   Donkey jump: 0.5 fixed
//   Natural heal: 1 HP every 900 ticks
//   Eat grass: rand(300)==0 when idle, duration 50 ticks
//   Tail flick: every 200 ticks
//   Max 6 per chunk
//   Can't use ladders
//   Can't be pushed when ridden
//   Drops: leather (horse), rotten_flesh (zombie), bone (skeleton)
// ═══════════════════════════════════════════════════════════════════════════

class EntityHorse {
public:
    double posX = 0, posY = 0, posZ = 0;
    double motionX = 0, motionY = 0, motionZ = 0;
    float rotationYaw = 0, rotationPitch = 0;
    bool isDead = false;
    bool onGround = false;
    int32_t entityId = 0;
    int32_t ticksExisted = 0;

    // Horse state
    int32_t horseType = 0;
    int32_t horseVariant = 0;
    int32_t flags = 0; // DW 16
    int32_t armorIndex = 0;
    int32_t temper = 0;
    bool horseJumping = false;
    float jumpPower = 0;
    bool hasReproduced = false;

    // Animation
    int32_t eatingHaystackCounter = 0;
    int32_t openMouthCounter = 0;
    int32_t jumpRearingCounter = 0;
    int32_t gallopTime = 0;
    float headLean = 0, prevHeadLean = 0;
    float rearingAmount = 0, prevRearingAmount = 0;
    float mouthOpenness = 0, prevMouthOpenness = 0;
    int32_t tailFlickCounter = 0;
    int32_t eatGrassCounter = 0;

    // ─── Constants ───
    static constexpr float WIDTH = 1.4f;
    static constexpr float HEIGHT = 1.6f;
    static constexpr double BASE_MAX_HEALTH = 53.0;
    static constexpr double BASE_MOVE_SPEED = 0.225;
    static constexpr double BASE_JUMP_STRENGTH = 0.7;
    static constexpr float SOUND_VOLUME = 0.8f;
    static constexpr int32_t TALK_INTERVAL = 400;
    static constexpr int32_t MAX_TEMPER = 100;
    static constexpr int32_t MAX_PER_CHUNK = 6;
    static constexpr int32_t FOAL_AGE = -24000;
    static constexpr int32_t FOAL_CHANCE = 5; // 1/5 = 20%

    // Type constants
    static constexpr int32_t TYPE_HORSE = 0;
    static constexpr int32_t TYPE_DONKEY = 1;
    static constexpr int32_t TYPE_MULE = 2;
    static constexpr int32_t TYPE_ZOMBIE = 3;
    static constexpr int32_t TYPE_SKELETON = 4;
    static constexpr int32_t NUM_COLORS = 7;
    static constexpr int32_t NUM_MARKINGS = 5;

    // Armor values: {none=0, iron=5, gold=7, diamond=11}
    static constexpr std::array<int32_t, 4> ARMOR_VALUES = {0, 5, 7, 11};

    // Flag bits
    static constexpr int32_t FLAG_TAME = 2;
    static constexpr int32_t FLAG_SADDLE = 4;
    static constexpr int32_t FLAG_CHEST = 8;
    static constexpr int32_t FLAG_BRED = 16;
    static constexpr int32_t FLAG_EATING = 32;
    static constexpr int32_t FLAG_REARING = 64;
    static constexpr int32_t FLAG_MOUTH = 128;

    // Inventory
    static constexpr int32_t INV_NORMAL = 2;
    static constexpr int32_t INV_CHESTED = 17;
    static constexpr int32_t SLOT_SADDLE = 0;
    static constexpr int32_t SLOT_ARMOR = 1;

    // Timing
    static constexpr int32_t NATURAL_HEAL_INTERVAL = 900;
    static constexpr int32_t EAT_GRASS_CHANCE = 300;
    static constexpr int32_t EAT_GRASS_DURATION = 50;
    static constexpr int32_t TAIL_FLICK_INTERVAL = 200;
    static constexpr int32_t MOUTH_OPEN_DURATION = 30;
    static constexpr int32_t REARING_DURATION = 20;
    static constexpr int32_t EAT_GRASS_FIND_DURATION = 300;

    // ─── Food table ───
    struct FoodItem {
        int32_t itemId;
        float heal;
        int32_t growth;
        int32_t temperBonus;
        bool canBreed;
    };

    static constexpr std::array<FoodItem, 7> FOOD_TABLE = {{
        {296,  2.0f,  60, 3, false},  // wheat
        {353,  1.0f,  30, 3, false},  // sugar
        {297,  7.0f, 180, 3, false},  // bread
        // hay_block (block 170) handled separately
        {0,   20.0f, 180, 0, false},  // hay_block placeholder
        {260,  3.0f,  60, 3, false},  // apple
        {396,  4.0f,  60, 5, true},   // golden_carrot
        {322, 10.0f, 240, 10, true},  // golden_apple
    }};

    // ─── Flag helpers ───
    bool getFlag(int32_t bit) const { return (flags & bit) != 0; }
    void setFlag(int32_t bit, bool v) { flags = v ? (flags | bit) : (flags & ~bit); }

    bool isTame() const { return getFlag(FLAG_TAME); }
    bool isSaddled() const { return getFlag(FLAG_SADDLE); }
    bool isChested() const { return getFlag(FLAG_CHEST); }
    bool isEating() const { return getFlag(FLAG_EATING); }
    bool isRearing() const { return getFlag(FLAG_REARING); }
    bool isMouthOpen() const { return getFlag(FLAG_MOUTH); }
    bool isUndead() const { return horseType == TYPE_ZOMBIE || horseType == TYPE_SKELETON; }
    bool isSterile() const { return isUndead() || horseType == TYPE_MULE; }
    bool canWearArmor() const { return horseType == TYPE_HORSE; }
    bool canCarryChest() const { return horseType == TYPE_DONKEY || horseType == TYPE_MULE; }
    int32_t getArmorValue() const { return ARMOR_VALUES[armorIndex]; }
    int32_t getInventorySize() const {
        return (isChested() && canCarryChest()) ? INV_CHESTED : INV_NORMAL;
    }

    // ─── Rider movement ───
    struct RiderMovement {
        float strafe;
        float forward;
        float stepHeight;
        float jumpMoveFactor;
    };

    RiderMovement calculateRiderMovement(float riderStrafe, float riderForward,
                                           float aiMoveSpeed) const {
        RiderMovement rm{};
        rm.strafe = riderStrafe * 0.5f;
        rm.forward = riderForward;
        if (rm.forward <= 0) rm.forward *= 0.25f;
        rm.stepHeight = 1.0f;
        rm.jumpMoveFactor = aiMoveSpeed * 0.1f;
        return rm;
    }

    // ─── Jump ───
    // Java: setJumpPower(int n) — charge→jumpPower
    float calculateJumpPower(int32_t chargeLevel) const {
        if (chargeLevel < 0) return 0;
        return chargeLevel >= 90 ? 1.0f : 0.4f + 0.4f * chargeLevel / 90.0f;
    }

    // Jump motionY = jumpStrength * jumpPower + potion bonus
    struct JumpResult {
        double motionY;
        double pushX; // -0.4 * sin(yaw) * jumpPower
        double pushZ; //  0.4 * cos(yaw) * jumpPower
        bool playSound;
    };

    JumpResult calculateJump(double jumpStrength, int32_t potionAmplifier,
                               float yawRad, bool hasForward) const {
        JumpResult result{};
        result.motionY = jumpStrength * jumpPower;
        if (potionAmplifier >= 0) {
            result.motionY += (potionAmplifier + 1) * 0.1;
        }
        if (hasForward) {
            result.pushX = -0.4 * std::sin(yawRad) * jumpPower;
            result.pushZ = 0.4 * std::cos(yawRad) * jumpPower;
            result.playSound = true;
        }
        return result;
    }

    // ─── Genetic breeding ───
    struct BreedingResult {
        int32_t childType;
        int32_t childVariant;
    };

    static BreedingResult calculateBreeding(int32_t type1, int32_t type2,
                                              int32_t variant1, int32_t variant2,
                                              int32_t rand9, int32_t rand5,
                                              int32_t randColor7, int32_t randMark5) {
        BreedingResult result{};
        if (type1 == type2) {
            result.childType = type1;
        } else if ((type1 == 0 && type2 == 1) || (type1 == 1 && type2 == 0)) {
            result.childType = TYPE_MULE;
        }

        if (result.childType == TYPE_HORSE) {
            int32_t color = rand9 < 4 ? (variant1 & 0xFF) :
                            (rand9 < 8 ? (variant2 & 0xFF) : randColor7);
            int32_t marking;
            if (rand5 < 2) marking = variant1 & 0xFF00;
            else if (rand5 < 4) marking = variant2 & 0xFF00;
            else marking = (randMark5 << 8) & 0xFF00;
            result.childVariant = color | marking;
        }
        return result;
    }

    // Random initial stats (used for both spawn and breeding third roll)
    static float randomHP(int32_t rand8, int32_t rand9) {
        return 15.0f + rand8 + rand9;
    }

    static double randomJump(double rand1, double rand2, double rand3) {
        return 0.4 + rand1 * 0.2 + rand2 * 0.2 + rand3 * 0.2;
    }

    static double randomSpeed(double rand1, double rand2, double rand3) {
        return (0.45 + rand1 * 0.3 + rand2 * 0.3 + rand3 * 0.3) * 0.25;
    }

    // Three-parent average
    static double breedStat(double parent1, double parent2, double random) {
        return (parent1 + parent2 + random) / 3.0;
    }

    // Mating check
    bool canMate(bool hasRider, bool isMounted, float hp, float maxHP) const {
        return !hasRider && !isMounted && isTame() &&
               !isSterile() && hp >= maxHP;
    }

    // ─── Fall damage ───
    // Java: ceil(dist*0.5 - 3)
    static int32_t calculateFallDamage(float distance) {
        int32_t dmg = static_cast<int32_t>(std::ceil(distance * 0.5f - 3.0f));
        return dmg > 0 ? dmg : 0;
    }

    // ─── Animation blending ───
    void tickAnimations() {
        prevHeadLean = headLean;
        if (isEating()) {
            headLean += (1.0f - headLean) * 0.4f + 0.05f;
            if (headLean > 1.0f) headLean = 1.0f;
        } else {
            headLean += (0.0f - headLean) * 0.4f - 0.05f;
            if (headLean < 0.0f) headLean = 0.0f;
        }

        prevRearingAmount = rearingAmount;
        if (isRearing()) {
            headLean = 0; prevHeadLean = 0;
            rearingAmount += (1.0f - rearingAmount) * 0.4f + 0.05f;
            if (rearingAmount > 1.0f) rearingAmount = 1.0f;
        } else {
            rearingAmount += (0.8f * rearingAmount * rearingAmount * rearingAmount - rearingAmount)
                              * 0.6f - 0.05f;
            if (rearingAmount < 0.0f) rearingAmount = 0.0f;
        }

        prevMouthOpenness = mouthOpenness;
        if (isMouthOpen()) {
            mouthOpenness += (1.0f - mouthOpenness) * 0.7f + 0.05f;
            if (mouthOpenness > 1.0f) mouthOpenness = 1.0f;
        } else {
            mouthOpenness += (0.0f - mouthOpenness) * 0.7f - 0.05f;
            if (mouthOpenness < 0.0f) mouthOpenness = 0.0f;
        }
    }

    // ─── Foal scaling ───
    static float getHorseSize(int32_t growingAge) {
        if (growingAge >= 0) return 1.0f;
        return 0.5f + static_cast<float>(-24000 - growingAge) / -24000.0f * 0.5f;
    }

    // ─── Drops ───
    static int32_t getDropItem(int32_t type, bool quarterChance) {
        if (type == TYPE_SKELETON) return 352; // bone
        if (type == TYPE_ZOMBIE) return quarterChance ? 0 : 367; // rotten_flesh
        return 334; // leather
    }

    // ─── Sounds ───
    const char* getLivingSound(bool rare) const {
        if (horseType == TYPE_ZOMBIE) return "mob.horse.zombie.idle";
        if (horseType == TYPE_SKELETON) return "mob.horse.skeleton.idle";
        if (horseType == TYPE_DONKEY || horseType == TYPE_MULE) return "mob.horse.donkey.idle";
        return "mob.horse.idle";
    }

    const char* getHurtSound() const {
        if (horseType == TYPE_ZOMBIE) return "mob.horse.zombie.hit";
        if (horseType == TYPE_SKELETON) return "mob.horse.skeleton.hit";
        if (horseType == TYPE_DONKEY || horseType == TYPE_MULE) return "mob.horse.donkey.hit";
        return "mob.horse.hit";
    }

    const char* getDeathSound() const {
        if (horseType == TYPE_ZOMBIE) return "mob.horse.zombie.death";
        if (horseType == TYPE_SKELETON) return "mob.horse.skeleton.death";
        if (horseType == TYPE_DONKEY || horseType == TYPE_MULE) return "mob.horse.donkey.death";
        return "mob.horse.death";
    }

    const char* getAngrySound() const {
        if (horseType == TYPE_ZOMBIE || horseType == TYPE_SKELETON) return nullptr;
        if (horseType == TYPE_DONKEY || horseType == TYPE_MULE) return "mob.horse.donkey.angry";
        return "mob.horse.angry";
    }

    // Step sound logic
    struct StepSoundResult {
        bool playGallop;
        bool playBreathe;
        bool playWood;
        bool playSoft;
    };

    StepSoundResult getStepSound(bool hasRider, bool isWood, int32_t rand10) {
        StepSoundResult result{};
        if (hasRider && horseType != TYPE_DONKEY && horseType != TYPE_MULE) {
            ++gallopTime;
            if (gallopTime > 5 && gallopTime % 3 == 0) {
                result.playGallop = true;
                if (horseType == TYPE_HORSE && rand10 == 0) result.playBreathe = true;
            } else if (gallopTime <= 5) {
                result.playWood = true;
            }
        } else if (isWood) {
            result.playWood = true;
        } else {
            result.playSoft = true;
        }
        return result;
    }

    // Undead attributes
    static constexpr double UNDEAD_HP = 15.0;
    static constexpr double UNDEAD_SPEED = 0.2;
    static constexpr double DONKEY_JUMP = 0.5;
    static constexpr double DONKEY_SPEED = 0.175;
};

} // namespace mccpp
