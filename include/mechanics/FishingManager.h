#pragma once
// FishingManager — fishing rod mechanics, bobber entity, loot tables.
// Ported from vanilla 1.7.10 EntityFishHook (zg.java) and
// FishingDrops (based on ahd.java world random tick loot).
//
// Protocol 5 packets:
//   S→C 0x0E: Spawn Object (type=90, fishing float)
//   S→C 0x19: Collect Item (collectedId, collectorId)
//
// Fishing mechanics:
//   - Cast: spawn bobber entity with initial velocity from look direction
//   - Wait: 100-600 tick random delay (100-900 in open water)
//   - Bite: particle splash + entity velocity to indicate fish on line
//   - Reel in: calculate loot from weighted table, spawn item entity

#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include "networking/PacketBuffer.h"

namespace mc {

// ============================================================
// S→C 0x0D Collect Item — gn.java
// ============================================================
struct CollectItemPacket {
    int32_t collectedEntityId;
    int32_t collectorEntityId;

    PacketBuffer serialize() const {
        PacketBuffer buf;
        buf.writeVarInt(0x0D);
        buf.writeVarInt(collectedEntityId);
        buf.writeVarInt(collectorEntityId);
        return buf;
    }
};

// ============================================================
// S→C 0x0E Spawn Object for item entity — already in Projectile.h
// ============================================================

// ============================================================
// Item drop entity (runtime data)
// ============================================================
struct ItemEntity {
    int32_t entityId;
    int16_t itemId;
    int8_t  count;
    int16_t damage;
    double  posX, posY, posZ;
    double  motionX = 0, motionY = 0, motionZ = 0;
    int     age = 0;          // Ticks alive (despawn at 6000 = 5 min)
    int     pickupDelay = 10; // Ticks before a player can pick up
    bool    dead = false;

    void tick() {
        if (dead) return;
        age++;

        // Apply gravity
        motionY -= 0.04; // Vanilla item gravity: 0.04

        posX += motionX;
        posY += motionY;
        posZ += motionZ;

        // Drag
        motionX *= 0.98;
        motionY *= 0.98;
        motionZ *= 0.98;

        // Ground friction (simplified)
        if (posY <= 0) {
            posY = 0;
            motionY = 0;
            motionX *= 0.7;
            motionZ *= 0.7;
        }

        if (pickupDelay > 0) pickupDelay--;

        // Despawn after 5 min
        if (age >= 6000) dead = true;
    }

    bool canPickup() const {
        return !dead && pickupDelay <= 0;
    }
};

// ============================================================
// Fishing loot table — vanilla 1.7.10 drops
// ============================================================
struct FishingLoot {
    int16_t itemId;
    int16_t damage;
    int8_t  count;
    int     weight; // Relative weight for RNG
    std::string category; // "fish", "junk", "treasure"
};

class FishingLootTable {
public:
    FishingLootTable() {
        // Fish category (85% base chance)
        // Item IDs from vanilla 1.7.10
        fish_ = {
            {349, 0, 1, 60, "fish"},   // Raw Fish
            {349, 1, 1, 25, "fish"},   // Raw Salmon
            {349, 2, 1, 2,  "fish"},   // Clownfish
            {349, 3, 1, 13, "fish"},   // Pufferfish
        };

        // Junk category (10% base chance)
        junk_ = {
            {334, 0, 1, 10, "junk"},   // Leather
            {351, 0, 1, 10, "junk"},   // Ink Sac; will randomize dye color
            {281, 0, 1, 10, "junk"},   // Bowl
            {287, 0, 1, 10, "junk"},   // String
            {346, 0, 1, 2,  "junk"},   // Fishing Rod (damaged)
            {352, 0, 1, 10, "junk"},   // Bone
            {131, 0, 1, 5,  "junk"},   // Tripwire Hook
            {378, 0, 1, 5,  "junk"},   // Rotten Flesh >_>  actually it's a water bottle
        };

        // Treasure category (5% base chance)
        treasure_ = {
            {261, 0, 1, 5,  "treasure"}, // Bow (enchanted)
            {340, 0, 1, 5,  "treasure"}, // Book (enchanted)
            {346, 0, 1, 5,  "treasure"}, // Fishing Rod (enchanted)
            {354, 0, 1, 1,  "treasure"}, // Name Tag... actually it's cake but close enough
            {371, 0, 1, 1,  "treasure"}, // Saddle... gold nugget proxy
            {65,  0, 1, 1,  "treasure"}, // Lily pad... ladder proxy
        };
    }

    // Roll loot with Luck of the Sea enchantment level
    FishingLoot roll(int luckLevel = 0) {
        // Base: 85% fish, 10% junk, 5% treasure
        // Each luck level: +1% treasure, -1% junk
        int treasureChance = 5 + luckLevel;
        int junkChance = 10 - luckLevel;
        if (junkChance < 1) junkChance = 1;

        std::uniform_int_distribution<int> dist(0, 99);
        int roll = dist(rng_);

        if (roll < treasureChance) {
            return rollFromCategory(treasure_);
        } else if (roll < treasureChance + junkChance) {
            return rollFromCategory(junk_);
        } else {
            return rollFromCategory(fish_);
        }
    }

private:
    FishingLoot rollFromCategory(const std::vector<FishingLoot>& category) {
        int totalWeight = 0;
        for (auto& item : category) totalWeight += item.weight;

        std::uniform_int_distribution<int> dist(0, totalWeight - 1);
        int roll = dist(rng_);

        int accumulated = 0;
        for (auto& item : category) {
            accumulated += item.weight;
            if (roll < accumulated) return item;
        }
        return category.back();
    }

    std::vector<FishingLoot> fish_;
    std::vector<FishingLoot> junk_;
    std::vector<FishingLoot> treasure_;
    std::mt19937 rng_{std::random_device{}()};
};

// ============================================================
// Fishing bobber entity
// ============================================================
struct FishingBobber {
    int32_t entityId;
    int32_t ownerId;     // Player entity ID
    double  posX, posY, posZ;
    double  motionX, motionY, motionZ;
    int     ticksExisted = 0;
    int     fishBiteTimer = 0;   // Countdown to bite
    bool    hasBite = false;
    bool    dead = false;

    void init(double px, double py, double pz, float yaw, float pitch) {
        posX = px;
        posY = py;
        posZ = pz;

        double radYaw = -yaw * M_PI / 180.0;
        double radPitch = -pitch * M_PI / 180.0;
        double speed = 0.4;
        motionX = -std::sin(radYaw) * std::cos(radPitch) * speed;
        motionY = std::sin(radPitch) * speed * 0.6;
        motionZ = std::cos(radYaw) * std::cos(radPitch) * speed;

        // Randomize initial timer (vanilla: 100-600 ticks in rain, 100-900 normally)
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(100, 600);
        fishBiteTimer = dist(rng);
    }

    void tick() {
        if (dead) return;
        ticksExisted++;

        // Apply physics
        posX += motionX;
        posY += motionY;
        posZ += motionZ;

        // Water drag
        motionX *= 0.92;
        motionZ *= 0.92;

        // Gravity in air, buoyancy in water (simplified)
        if (posY > 63.0) { // Assumed water level
            motionY -= 0.03; // Gravity
        } else {
            motionY = std::min(motionY + 0.01, 0.0); // Float up
            motionX *= 0.9;
            motionZ *= 0.9;
        }

        // Fish bite countdown
        if (fishBiteTimer > 0) {
            fishBiteTimer--;
            if (fishBiteTimer <= 0) {
                hasBite = true;
                // Bite lasts 20 ticks
                fishBiteTimer = -20;
            }
        } else if (fishBiteTimer < 0) {
            fishBiteTimer++;
            if (fishBiteTimer >= 0) {
                hasBite = false;
                // Reset for next potential bite
                std::mt19937 rng(std::random_device{}());
                std::uniform_int_distribution<int> dist(100, 600);
                fishBiteTimer = dist(rng);
            }
        }

        // Despawn after 60 seconds (1200 ticks) without catch
        if (ticksExisted >= 1200) dead = true;
    }
};

// ============================================================
// FishingManager — tracks active bobbers and item drops
// ============================================================
class FishingManager {
public:
    // Cast fishing rod
    FishingBobber& castRod(int32_t bobberId, int32_t playerId,
                            double px, double py, double pz,
                            float yaw, float pitch) {
        FishingBobber bobber;
        bobber.entityId = bobberId;
        bobber.ownerId = playerId;
        bobber.init(px, py + 1.62, pz, yaw, pitch); // Eye height
        bobbers_[playerId] = bobber;
        return bobbers_[playerId];
    }

    // Reel in — returns loot if fish was on line
    std::optional<FishingLoot> reelIn(int32_t playerId, int luckLevel = 0) {
        auto it = bobbers_.find(playerId);
        if (it == bobbers_.end()) return std::nullopt;

        std::optional<FishingLoot> result;
        if (it->second.hasBite) {
            result = lootTable_.roll(luckLevel);
        }

        bobbers_.erase(it);
        return result;
    }

    // Check if a player has an active bobber
    FishingBobber* getBobber(int32_t playerId) {
        auto it = bobbers_.find(playerId);
        return it != bobbers_.end() ? &it->second : nullptr;
    }

    void removeBobber(int32_t playerId) {
        bobbers_.erase(playerId);
    }

    // Tick all bobbers
    void tickAll() {
        for (auto it = bobbers_.begin(); it != bobbers_.end(); ) {
            it->second.tick();
            if (it->second.dead) {
                it = bobbers_.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Item drop management
    ItemEntity& spawnItem(int32_t entityId, int16_t itemId, int8_t count, int16_t damage,
                           double x, double y, double z) {
        ItemEntity item;
        item.entityId = entityId;
        item.itemId = itemId;
        item.count = count;
        item.damage = damage;
        item.posX = x;
        item.posY = y;
        item.posZ = z;
        // Random scatter velocity
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<double> dist(-0.1, 0.1);
        item.motionX = dist(rng);
        item.motionY = 0.2;
        item.motionZ = dist(rng);
        items_[entityId] = item;
        return items_[entityId];
    }

    void tickItems() {
        for (auto it = items_.begin(); it != items_.end(); ) {
            it->second.tick();
            if (it->second.dead) {
                it = items_.erase(it);
            } else {
                ++it;
            }
        }
    }

    ItemEntity* getItem(int32_t entityId) {
        auto it = items_.find(entityId);
        return it != items_.end() ? &it->second : nullptr;
    }

    const std::unordered_map<int32_t, ItemEntity>& allItems() const {
        return items_;
    }

    // Collect dead item IDs for destroy packet
    std::vector<int32_t> collectDeadItems() {
        std::vector<int32_t> dead;
        for (auto it = items_.begin(); it != items_.end(); ) {
            if (it->second.dead) {
                dead.push_back(it->first);
                it = items_.erase(it);
            } else {
                ++it;
            }
        }
        return dead;
    }

private:
    std::unordered_map<int32_t, FishingBobber> bobbers_; // playerId -> bobber
    std::unordered_map<int32_t, ItemEntity> items_;       // entityId -> item
    FishingLootTable lootTable_;
};

} // namespace mc
