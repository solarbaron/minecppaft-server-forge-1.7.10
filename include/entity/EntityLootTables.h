/**
 * EntityLootTables.h — Comprehensive entity death drop tables and XP values.
 *
 * Java references (grep across 20+ entity classes for getDropItem/dropFewItems/dropRareDrop):
 *   - EntityLiving.onDeath / dropFewItems / dropRareDrop / getDropItem
 *   - All hostile, passive, and utility mob subclasses
 *
 * Drop system architecture (Java: EntityLiving.onDeath, lines 150-230):
 *   1. Check recentlyHit > 0 (killed by player within 100 ticks)
 *   2. Call dropFewItems(recentlyHit > 0) for base drops
 *   3. If recentlyHit > 0: roll rare drop via dropRareDrop(lootingLevel)
 *      Rare drop chance = 5% base (1 in 200 reduced by looting*1)
 *   4. Drop equipment with per-slot chances
 *   5. Drop XP orbs: getExperiencePoints(killer) * split
 *
 * Base drop formula (Java: EntityLiving.dropFewItems):
 *   Item item = getDropItem()
 *   if (item != null): dropItem(item, rand.nextInt(3))  [0-2 items]
 *   Looting: +rand.nextInt(lootingLevel+1) items
 *
 * Thread safety: Called on entity death, single entity thread.
 * JNI readiness: Simple data tables, POD structs.
 */
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Item IDs (matching vanilla 1.7.10 registry)
// ═══════════════════════════════════════════════════════════════════════════

namespace ItemID {
    // Common drops
    static constexpr int32_t ROTTEN_FLESH = 367;
    static constexpr int32_t BONE = 352;
    static constexpr int32_t ARROW = 262;
    static constexpr int32_t GUNPOWDER = 289;
    static constexpr int32_t STRING = 287;
    static constexpr int32_t SPIDER_EYE = 375;
    static constexpr int32_t ENDER_PEARL = 368;
    static constexpr int32_t BLAZE_ROD = 369;
    static constexpr int32_t GHAST_TEAR = 370;
    static constexpr int32_t SLIME_BALL = 341;
    static constexpr int32_t MAGMA_CREAM = 378;
    static constexpr int32_t LEATHER = 334;
    static constexpr int32_t RAW_BEEF = 363;
    static constexpr int32_t COOKED_BEEF = 364;
    static constexpr int32_t RAW_PORKCHOP = 319;
    static constexpr int32_t COOKED_PORKCHOP = 320;
    static constexpr int32_t RAW_CHICKEN = 365;
    static constexpr int32_t COOKED_CHICKEN = 366;
    static constexpr int32_t FEATHER = 288;
    static constexpr int32_t WOOL = 35;  // Block ID
    static constexpr int32_t INK_SAC = 351;
    static constexpr int32_t SNOWBALL = 332;
    static constexpr int32_t IRON_INGOT = 265;
    static constexpr int32_t CARROT = 391;
    static constexpr int32_t POTATO = 392;
    static constexpr int32_t NETHER_STAR = 399;
    static constexpr int32_t WITHER_SKELETON_SKULL = 397; // meta 1
    static constexpr int32_t COAL = 263;
    static constexpr int32_t MUSIC_DISC_13 = 2256;
    static constexpr int32_t MUSIC_DISC_CAT = 2257;
    static constexpr int32_t EGG = 344;
}

// ═══════════════════════════════════════════════════════════════════════════
// LootDrop — Single drop entry with quantity and conditions.
// ═══════════════════════════════════════════════════════════════════════════

struct LootDrop {
    int32_t itemId = 0;
    int32_t itemMeta = 0;        // Damage/metadata value
    int32_t minCount = 0;        // Minimum drop count
    int32_t maxCount = 2;        // Maximum base drop count (before looting)
    bool requiresPlayerKill = false;
    bool isRareDrop = false;     // 2.5% base chance (5% in Java = 1/200 per drop)
    bool scalesWithLooting = true;
    bool onFireDrop = false;     // Use cooked variant if entity is on fire
    int32_t onFireItemId = 0;    // Cooked item ID when on fire
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityLootTable — Complete loot table for an entity type.
// ═══════════════════════════════════════════════════════════════════════════

struct EntityLootTable {
    std::string entityName;
    int32_t baseXP = 0;
    int32_t maxXP = 0;           // randomized between baseXP and maxXP
    bool childXPMultiplier = false; // If true, child mobs give 2.5x XP
    std::vector<LootDrop> baseDrops;
    std::vector<LootDrop> rareDrops;

    // ─── Equipment drop chances ───
    // Java: EntityLiving.equipmentDropChances[5]
    //   Default = 0.085f for each slot
    //   0.0f means never drop, >1.0f means always drop undamaged
    std::array<float, 5> equipmentDropChances = {
        0.085f, 0.085f, 0.085f, 0.085f, 0.085f
    };
};

// ═══════════════════════════════════════════════════════════════════════════
// LootTableRegistry — All vanilla 1.7.10 entity drop tables.
//
// Drop mechanics constants:
//   RARE_DROP_CHANCE = 1/200 per looting attempt (5% total)
//     Reduced by lootingLevel: 1/(200 - lootingLevel*40)
//     Minimum: 1/120 with Looting III
//   RECENTLY_HIT_TICKS = 100 (5 seconds)
//   EQUIPMENT_BASE_CHANCE = 0.085f (8.5%)
//     +1% per looting level
// ═══════════════════════════════════════════════════════════════════════════

class LootTableRegistry {
public:
    // ─── Drop system constants ───
    static constexpr int32_t RECENTLY_HIT_TICKS = 100;
    static constexpr float EQUIPMENT_BASE_CHANCE = 0.085f;
    static constexpr int32_t RARE_DROP_BASE = 200;
    static constexpr int32_t RARE_DROP_LOOTING_REDUCTION = 40;
    static constexpr int32_t RARE_DROP_MINIMUM = 120;

    // ═══════════════════════════════════════════════════════════════
    // Hostile mob drop tables
    // ═══════════════════════════════════════════════════════════════

    // Zombie: rotten_flesh 0-2, rare: iron_ingot/carrot/potato
    // Java: EntityZombie (XP: 5, child: 12)
    static EntityLootTable zombie() {
        EntityLootTable t;
        t.entityName = "Zombie";
        t.baseXP = 5; t.maxXP = 5;
        t.childXPMultiplier = true;
        t.baseDrops = {{ItemID::ROTTEN_FLESH, 0, 0, 2}};
        t.rareDrops = {
            {ItemID::IRON_INGOT, 0, 1, 1, true, true},
            {ItemID::CARROT, 0, 1, 1, true, true},
            {ItemID::POTATO, 0, 1, 1, true, true}
        };
        return t;
    }

    // Skeleton: bone 0-2, arrow 0-2, rare: bow (wither: coal, rare: skull)
    // Java: EntitySkeleton (XP: 5)
    static EntityLootTable skeleton() {
        EntityLootTable t;
        t.entityName = "Skeleton";
        t.baseXP = 5; t.maxXP = 5;
        t.baseDrops = {
            {ItemID::ARROW, 0, 0, 2},
            {ItemID::BONE, 0, 0, 2}
        };
        return t;
    }

    // Wither Skeleton: coal 0-1, rare: wither skull (2.5% + looting)
    // Java: EntitySkeleton with skeletonType == 1 (XP: 5)
    static EntityLootTable witherSkeleton() {
        EntityLootTable t;
        t.entityName = "WitherSkeleton";
        t.baseXP = 5; t.maxXP = 5;
        t.baseDrops = {{ItemID::COAL, 0, 0, 1}};
        t.rareDrops = {{ItemID::WITHER_SKELETON_SKULL, 1, 1, 1, true, true}};
        return t;
    }

    // Creeper: gunpowder 0-2, rare: music disc (if killed by skeleton)
    // Java: EntityCreeper (XP: 5)
    static EntityLootTable creeper() {
        EntityLootTable t;
        t.entityName = "Creeper";
        t.baseXP = 5; t.maxXP = 5;
        t.baseDrops = {{ItemID::GUNPOWDER, 0, 0, 2}};
        // Music discs only when killed by skeleton arrow — special case
        return t;
    }

    // Spider: string 0-2, rare: spider_eye
    // Java: EntitySpider (XP: 5)
    static EntityLootTable spider() {
        EntityLootTable t;
        t.entityName = "Spider";
        t.baseXP = 5; t.maxXP = 5;
        t.baseDrops = {{ItemID::STRING, 0, 0, 2}};
        t.rareDrops = {{ItemID::SPIDER_EYE, 0, 1, 1, true, true}};
        return t;
    }

    // Enderman: ender_pearl 0-1
    // Java: EntityEnderman (XP: 5)
    static EntityLootTable enderman() {
        EntityLootTable t;
        t.entityName = "Enderman";
        t.baseXP = 5; t.maxXP = 5;
        t.baseDrops = {{ItemID::ENDER_PEARL, 0, 0, 1}};
        return t;
    }

    // Blaze: blaze_rod 0-1 (only player kill)
    // Java: EntityBlaze (XP: 10)
    static EntityLootTable blaze() {
        EntityLootTable t;
        t.entityName = "Blaze";
        t.baseXP = 10; t.maxXP = 10;
        t.baseDrops = {{ItemID::BLAZE_ROD, 0, 0, 1, true}};
        return t;
    }

    // Ghast: ghast_tear 0-1, gunpowder 0-2
    // Java: EntityGhast (XP: 5)
    static EntityLootTable ghast() {
        EntityLootTable t;
        t.entityName = "Ghast";
        t.baseXP = 5; t.maxXP = 5;
        t.baseDrops = {
            {ItemID::GHAST_TEAR, 0, 0, 1},
            {ItemID::GUNPOWDER, 0, 0, 2}
        };
        return t;
    }

    // Slime: slime_ball 0-2 (only smallest size)
    // Java: EntitySlime (XP: size == 1 ? 1 : size*2 for large)
    static EntityLootTable slime() {
        EntityLootTable t;
        t.entityName = "Slime";
        t.baseXP = 1; t.maxXP = 4;
        t.baseDrops = {{ItemID::SLIME_BALL, 0, 0, 2}};
        return t;
    }

    // Magma Cube: magma_cream 0-1 (only smallest size)
    // Java: EntityMagmaCube (XP: size == 1 ? 1 : size*2)
    static EntityLootTable magmaCube() {
        EntityLootTable t;
        t.entityName = "MagmaCube";
        t.baseXP = 1; t.maxXP = 4;
        t.baseDrops = {{ItemID::MAGMA_CREAM, 0, 0, 1}};
        return t;
    }

    // Silverfish: nothing
    // Java: EntitySilverfish (XP: 5)
    static EntityLootTable silverfish() {
        EntityLootTable t;
        t.entityName = "Silverfish";
        t.baseXP = 5; t.maxXP = 5;
        return t;
    }

    // ═══════════════════════════════════════════════════════════════
    // Passive mob drop tables
    // ═══════════════════════════════════════════════════════════════

    // Cow: leather 0-2, raw_beef 1-3 (cooked if on fire)
    // Java: EntityCow (XP: 1-3)
    static EntityLootTable cow() {
        EntityLootTable t;
        t.entityName = "Cow";
        t.baseXP = 1; t.maxXP = 3;
        t.baseDrops = {
            {ItemID::LEATHER, 0, 0, 2},
            {ItemID::RAW_BEEF, 0, 1, 3, false, false, true, true, ItemID::COOKED_BEEF}
        };
        return t;
    }

    // Pig: raw_porkchop 1-3 (cooked if on fire)
    // Java: EntityPig (XP: 1-3)
    static EntityLootTable pig() {
        EntityLootTable t;
        t.entityName = "Pig";
        t.baseXP = 1; t.maxXP = 3;
        t.baseDrops = {
            {ItemID::RAW_PORKCHOP, 0, 1, 3, false, false, true, true, ItemID::COOKED_PORKCHOP}
        };
        return t;
    }

    // Chicken: feather 0-2, raw_chicken 1 (cooked if on fire)
    // Also drops egg every 6000-12000 ticks while alive
    // Java: EntityChicken (XP: 1-3)
    static EntityLootTable chicken() {
        EntityLootTable t;
        t.entityName = "Chicken";
        t.baseXP = 1; t.maxXP = 3;
        t.baseDrops = {
            {ItemID::FEATHER, 0, 0, 2},
            {ItemID::RAW_CHICKEN, 0, 1, 1, false, false, true, true, ItemID::COOKED_CHICKEN}
        };
        return t;
    }

    // Sheep: wool 1 (with color meta from fleece)
    // Java: EntitySheep (XP: 1-3)
    static EntityLootTable sheep() {
        EntityLootTable t;
        t.entityName = "Sheep";
        t.baseXP = 1; t.maxXP = 3;
        // Wool drop is handled specially — color based on sheep's fleece
        // 1 wool block when unsheared, 0 when sheared
        t.baseDrops = {{ItemID::WOOL, 0, 1, 1}};
        return t;
    }

    // Squid: ink_sac 1-3
    // Java: EntitySquid (XP: 1-3)
    static EntityLootTable squid() {
        EntityLootTable t;
        t.entityName = "Squid";
        t.baseXP = 1; t.maxXP = 3;
        t.baseDrops = {{ItemID::INK_SAC, 0, 1, 3}};
        return t;
    }

    // Mooshroom: same as cow (leather + raw_beef)
    static EntityLootTable mooshroom() {
        auto t = cow();
        t.entityName = "Mooshroom";
        return t;
    }

    // Horse: leather 0-2
    // Java: EntityHorse (XP: 1-3)
    static EntityLootTable horse() {
        EntityLootTable t;
        t.entityName = "Horse";
        t.baseXP = 1; t.maxXP = 3;
        t.baseDrops = {{ItemID::LEATHER, 0, 0, 2}};
        return t;
    }

    // Wolf/Ocelot: nothing
    // Java: EntityWolf/EntityOcelot (XP: 1-3)
    static EntityLootTable wolf() {
        EntityLootTable t;
        t.entityName = "Wolf";
        t.baseXP = 1; t.maxXP = 3;
        return t;
    }

    // ═══════════════════════════════════════════════════════════════
    // Utility mob drop tables
    // ═══════════════════════════════════════════════════════════════

    // Snow Golem: snowball 0-15
    // Java: EntitySnowman (XP: 0)
    static EntityLootTable snowGolem() {
        EntityLootTable t;
        t.entityName = "SnowGolem";
        t.baseXP = 0; t.maxXP = 0;
        t.baseDrops = {{ItemID::SNOWBALL, 0, 0, 15}};
        return t;
    }

    // Iron Golem: iron_ingot 3-5, poppy 0-2
    // Java: EntityIronGolem (XP: 0)
    static EntityLootTable ironGolem() {
        EntityLootTable t;
        t.entityName = "IronGolem";
        t.baseXP = 0; t.maxXP = 0;
        t.baseDrops = {
            {ItemID::IRON_INGOT, 0, 3, 5},
            {38, 0, 0, 2}  // Red flower (poppy) = Block ID 38
        };
        return t;
    }

    // ═══════════════════════════════════════════════════════════════
    // Boss mob drop tables
    // ═══════════════════════════════════════════════════════════════

    // Ender Dragon: 12000 XP on first kill, 500 on repeat
    // Java: EntityDragon (XP: 12000)
    static EntityLootTable enderDragon() {
        EntityLootTable t;
        t.entityName = "EnderDragon";
        t.baseXP = 12000; t.maxXP = 12000;
        return t;
    }

    // Wither: nether_star 1, 50 XP
    // Java: EntityWither (XP: 50)
    static EntityLootTable wither() {
        EntityLootTable t;
        t.entityName = "Wither";
        t.baseXP = 50; t.maxXP = 50;
        t.baseDrops = {{ItemID::NETHER_STAR, 0, 1, 1}};
        return t;
    }

    // ═══════════════════════════════════════════════════════════════
    // Drop calculation helpers
    // ═══════════════════════════════════════════════════════════════

    // Calculate rare drop chance with looting
    // Java: chance = 1/(200 - lootingLevel*40), min 1/120
    static float getRareDropChance(int32_t lootingLevel) {
        int32_t divisor = RARE_DROP_BASE - lootingLevel * RARE_DROP_LOOTING_REDUCTION;
        if (divisor < RARE_DROP_MINIMUM) divisor = RARE_DROP_MINIMUM;
        return 1.0f / static_cast<float>(divisor);
    }

    // Calculate equipment drop chance with looting
    // Java: chance += lootingLevel * 0.01f
    static float getEquipmentDropChance(float baseChance, int32_t lootingLevel) {
        return baseChance + static_cast<float>(lootingLevel) * 0.01f;
    }

    // ═══════════════════════════════════════════════════════════════
    // XP calculation
    // Java: EntityLivingBase.getExperiencePoints
    //   Base XP from entity type
    //   Armor adds XP: armorValue * 1-3 random
    //   Child zombies: XP * 2.5
    // ═══════════════════════════════════════════════════════════════

    static int32_t calculateXP(const EntityLootTable& table, bool isChild) {
        int32_t xp = table.baseXP;
        if (table.maxXP > table.baseXP) {
            // Random between baseXP and maxXP (caller provides RNG)
            xp = table.baseXP; // Placeholder; actual = baseXP + rand(maxXP-baseXP+1)
        }
        if (isChild && table.childXPMultiplier) {
            xp = static_cast<int32_t>(static_cast<float>(xp) * 2.5f);
        }
        return xp;
    }

    // ═══════════════════════════════════════════════════════════════
    // Chicken egg timer
    // Java: EntityChicken.onLivingUpdate
    //   timeUntilNextEgg: 6000 + rand(6000) ticks [5-10 minutes]
    //   Drops 1 egg and plays "mob.chicken.plop" sound
    // ═══════════════════════════════════════════════════════════════
    static constexpr int32_t CHICKEN_EGG_MIN_TICKS = 6000;
    static constexpr int32_t CHICKEN_EGG_MAX_TICKS = 12000;
};

} // namespace mccpp
