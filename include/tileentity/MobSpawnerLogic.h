/**
 * MobSpawnerLogic.h — Mob spawner block activation, spawn cycle, and entity creation.
 *
 * Java reference:
 *   - net.minecraft.tileentity.MobSpawnerBaseLogic (242 lines)
 *   - net.minecraft.tileentity.MobSpawnerBaseLogic$WeightedRandomMinecart
 *
 * ═══════════════════════════════════════════════════════════════════════
 * MOB SPAWNER BASE LOGIC (MobSpawnerBaseLogic)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Activation check:
 *   - isActivated: getClosestPlayer within activatingRangeFromPlayer (16 blocks)
 *   - Distance measured from spawner center (pos + 0.5)
 *   - If no player nearby, entire updateSpawner skipped
 *
 * Server-side spawn cycle (updateSpawner):
 *   1. If spawnDelay == -1: call resetTimer (first tick initialization)
 *   2. If spawnDelay > 0: decrement and return (no spawn this tick)
 *   3. When spawnDelay reaches 0: attempt spawn loop
 *
 * Spawn loop (up to spawnCount = 4 attempts):
 *   1. Create entity by name (EntityList.createEntityByName)
 *   2. If entity null: abort entirely
 *   3. Count same-type entities in AABB:
 *      - Center on spawner, expand spawnRange*2 XZ, 4 Y
 *      - If count >= maxNearbyEntities (6): resetTimer and return
 *   4. Calculate spawn position:
 *      - X = spawnerX + (rand() - rand()) * spawnRange
 *      - Y = spawnerY + rand(3) - 1
 *      - Z = spawnerZ + (rand() - rand()) * spawnRange
 *      - Yaw = rand() * 360, Pitch = 0
 *   5. If EntityLiving: check getCanSpawnHere(), skip if false
 *   6. Call spawnEntity() to place in world
 *   7. Play effect 2004 (smoke + flame particles)
 *   8. If EntityLiving: spawnExplosionParticle
 *
 * After loop: if at least one entity spawned, resetTimer
 *
 * resetTimer:
 *   - spawnDelay = minSpawnDelay + rand(maxSpawnDelay - minSpawnDelay)
 *   - If max <= min: use min as fixed delay
 *   - If SpawnPotentials list exists: pick weighted random entity
 *
 * spawnEntity:
 *   - If randomEntity has extra NBT (SpawnData): merge into entity NBT
 *     - Write entity to NBT, overlay SpawnData keys, read back
 *     - Supports "Riding" chain for mounted entities:
 *       Walk Riding NBT, create rider, position at same coords, mountEntity
 *   - If no SpawnData: call onSpawnWithEgg(null) + spawnEntityInWorld
 *
 * Client-side: smoke + flame particles, spin animation
 *   field_98287_c: render rotation (0-360 continuous)
 *   field_98284_d: previous rotation (for interpolation)
 *   Rotation speed: 1000 / (spawnDelay + 200) degrees per tick
 *
 * Legacy fix: "Minecart" → "MinecartRideable" (old world compat)
 *
 * NBT format:
 *   EntityId (string) — mob name
 *   Delay (short) — current spawn delay
 *   MinSpawnDelay (short) — 200 default
 *   MaxSpawnDelay (short) — 800 default
 *   SpawnCount (short) — 4 default
 *   MaxNearbyEntities (short) — 6 default
 *   RequiredPlayerRange (short) — 16 default
 *   SpawnRange (short) — 4 default
 *   SpawnData (compound) — extra NBT for spawned entity
 *   SpawnPotentials (list of compound) — weighted entity pool
 *     Each: Type (string), Weight (int), Properties (compound)
 *
 * Thread safety: Tile entity update on server thread.
 * JNI readiness: Simple POD constants and string entity names.
 */
#pragma once

#include <cstdint>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Mob Spawner Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace MobSpawnerConstants {
    // ─── Default values (from Java field initializers) ───
    // Java: spawnDelay = 20
    static constexpr int32_t INITIAL_SPAWN_DELAY = 20;
    // Java: minSpawnDelay = 200
    static constexpr int32_t DEFAULT_MIN_DELAY = 200;
    // Java: maxSpawnDelay = 800
    static constexpr int32_t DEFAULT_MAX_DELAY = 800;
    // Java: spawnCount = 4
    static constexpr int32_t DEFAULT_SPAWN_COUNT = 4;
    // Java: maxNearbyEntities = 6
    static constexpr int32_t DEFAULT_MAX_NEARBY = 6;
    // Java: activatingRangeFromPlayer = 16
    static constexpr int32_t DEFAULT_PLAYER_RANGE = 16;
    // Java: spawnRange = 4
    static constexpr int32_t DEFAULT_SPAWN_RANGE = 4;

    // ─── Default entity ───
    // Java: mobID = "Pig"
    static constexpr const char* DEFAULT_MOB_ID = "Pig";

    // ─── Spawn position offsets ───
    // Java: X/Z = spawner + (rand() - rand()) * spawnRange
    //       Y = spawnerY + rand(3) - 1  (so Y-1, Y, or Y+1)
    static constexpr int32_t Y_RANDOM_RANGE = 3;
    static constexpr int32_t Y_OFFSET = 1;

    // ─── Entity cap AABB ───
    // Java: AABB expand(spawnRange * 2, 4.0, spawnRange * 2)
    static constexpr double CAP_Y_EXPAND = 4.0;

    // ─── Uninitialized delay sentinel ───
    // Java: if (spawnDelay == -1) resetTimer()
    static constexpr int32_t DELAY_UNINITIALIZED = -1;

    // ─── Client render rotation ───
    // Java: rotation = (rotation + 1000 / (spawnDelay + 200)) % 360
    static constexpr double RENDER_ROTATION_NUMERATOR = 1000.0;
    static constexpr double RENDER_ROTATION_OFFSET = 200.0;
    static constexpr double RENDER_ROTATION_MOD = 360.0;

    // ─── World effect ID ───
    // Java: playAuxSFX(2004, ...) — smoke + flame particles
    static constexpr int32_t SPAWN_PARTICLE_EFFECT = 2004;

    // ─── Block ID ───
    static constexpr int32_t MOB_SPAWNER_ID = 52;

    // ─── Legacy entity name fix ───
    // Java: if ("Minecart") → "MinecartRideable"
    static constexpr const char* LEGACY_MINECART = "Minecart";
    static constexpr const char* FIXED_MINECART = "MinecartRideable";
}

// ═══════════════════════════════════════════════════════════════════════════
// NBT Tags for Mob Spawner
// ═══════════════════════════════════════════════════════════════════════════

namespace MobSpawnerNBT {
    static constexpr const char* TAG_ENTITY_ID = "EntityId";
    static constexpr const char* TAG_DELAY = "Delay";
    static constexpr const char* TAG_MIN_DELAY = "MinSpawnDelay";
    static constexpr const char* TAG_MAX_DELAY = "MaxSpawnDelay";
    static constexpr const char* TAG_SPAWN_COUNT = "SpawnCount";
    static constexpr const char* TAG_MAX_NEARBY = "MaxNearbyEntities";
    static constexpr const char* TAG_PLAYER_RANGE = "RequiredPlayerRange";
    static constexpr const char* TAG_SPAWN_RANGE = "SpawnRange";
    static constexpr const char* TAG_SPAWN_DATA = "SpawnData";
    static constexpr const char* TAG_SPAWN_POTENTIALS = "SpawnPotentials";

    // ─── SpawnPotentials entry ───
    static constexpr const char* TAG_TYPE = "Type";
    static constexpr const char* TAG_WEIGHT = "Weight";
    static constexpr const char* TAG_PROPERTIES = "Properties";

    // ─── Riding chain ───
    static constexpr const char* TAG_RIDING = "Riding";
    static constexpr const char* TAG_ID = "id";
}

// ═══════════════════════════════════════════════════════════════════════════
// Weighted Random Entity Entry (SpawnPotentials)
// Java: MobSpawnerBaseLogic$WeightedRandomMinecart
// ═══════════════════════════════════════════════════════════════════════════

struct SpawnPotentialEntry {
    std::string entityType;
    int32_t weight = 1;
    // Extra NBT tag compound data (field_98222_b)
    // In actual implementation, this would be an NBT compound
    // For now, we track the structure
    bool hasExtraData = false;
};

// ═══════════════════════════════════════════════════════════════════════════
// Dungeon Spawner Mob IDs
// Java: WorldGenDungeons — possible spawner mobs in dungeons
// ═══════════════════════════════════════════════════════════════════════════

namespace DungeonSpawnerMobs {
    // Java: WorldGenDungeons picks from {"Skeleton", "Zombie", "Zombie", "Spider"}
    // Weights: Zombie 50%, Skeleton 25%, Spider 25%
    static constexpr const char* SKELETON = "Skeleton";
    static constexpr const char* ZOMBIE = "Zombie";
    static constexpr const char* SPIDER = "Spider";
    static constexpr const char* CAVE_SPIDER = "CaveSpider";  // mineshaft spawners
    static constexpr const char* BLAZE = "Blaze";              // nether fortress
    static constexpr const char* SILVERFISH = "Silverfish";    // stronghold

    // Java: WorldGenDungeons.pickMobSpawner(Random)
    // Selection array: {"Skeleton", "Zombie", "Zombie", "Spider"}
    static constexpr const char* DUNGEON_MOBS[] = {
        SKELETON, ZOMBIE, ZOMBIE, SPIDER
    };
    static constexpr int32_t NUM_DUNGEON_MOBS = 4;
}

// ═══════════════════════════════════════════════════════════════════════════
// Spawner delay calculation helper
// Java: spawnDelay = max <= min ? min : min + rand(max - min)
// ═══════════════════════════════════════════════════════════════════════════

inline int32_t calculateSpawnDelay(int32_t minDelay, int32_t maxDelay,
                                     int32_t randomValue) {
    if (maxDelay <= minDelay) return minDelay;
    return minDelay + (randomValue % (maxDelay - minDelay));
}

// ═══════════════════════════════════════════════════════════════════════════
// Spawn position calculation helper
// Java: spawner + (rand() - rand()) * spawnRange
// ═══════════════════════════════════════════════════════════════════════════

inline double calculateSpawnOffset(double spawnerPos, double rand1,
                                    double rand2, int32_t spawnRange) {
    return spawnerPos + (rand1 - rand2) * static_cast<double>(spawnRange);
}

// ═══════════════════════════════════════════════════════════════════════════
// Client render rotation calculation
// Java: (rotation + 1000 / (spawnDelay + 200)) % 360
// ═══════════════════════════════════════════════════════════════════════════

inline double calculateRenderRotation(double currentRotation, int32_t spawnDelay) {
    double speed = MobSpawnerConstants::RENDER_ROTATION_NUMERATOR /
                   (static_cast<double>(spawnDelay) + MobSpawnerConstants::RENDER_ROTATION_OFFSET);
    double newRotation = currentRotation + speed;
    // Java: % 360.0
    while (newRotation >= MobSpawnerConstants::RENDER_ROTATION_MOD) {
        newRotation -= MobSpawnerConstants::RENDER_ROTATION_MOD;
    }
    return newRotation;
}

} // namespace mccpp
