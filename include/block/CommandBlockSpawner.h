/**
 * CommandBlockSpawner.h — Command block and mob spawner.
 *
 * Java references:
 *   - net.minecraft.block.BlockCommandBlock (97 lines)
 *   - net.minecraft.block.BlockMobSpawner (48 lines)
 *   - net.minecraft.command.server.CommandBlockLogic (referenced)
 *   - net.minecraft.tileentity.MobSpawnerBaseLogic (referenced)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * COMMAND BLOCK (BlockCommandBlock)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: iron, extends BlockContainer (TileEntityCommandBlock)
 * Drops: NOTHING (quantityDropped = 0)
 *
 * Metadata:
 *   bit 0: powered flag (has received rising edge)
 *
 * Redstone behavior:
 *   Rising edge only — triggers on power ON, not OFF
 *   onNeighborBlockChange:
 *     - Power received AND not yet powered (bit 0=0):
 *       Set bit 0, schedule tick (tickRate = 1)
 *     - No power AND currently powered (bit 0=1):
 *       Clear bit 0
 *   updateTick:
 *     Execute command via CommandBlockLogic.func_145755_a
 *     Then notify neighbors about block change
 *
 * Activation: opens sign edit GUI (displayGUIEditSign)
 *   This is the command input interface
 *
 * Comparator output: getSuccessCount() from CommandBlockLogic
 *   Returns how many entities/successes the last command affected
 *
 * Placement: custom name from item display name → command block name
 *
 * Block ID: 137
 *
 * ═══════════════════════════════════════════════════════════════════════
 * MOB SPAWNER (BlockMobSpawner)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: rock, extends BlockContainer (TileEntityMobSpawner)
 * Not opaque (cage-like appearance)
 * Drops: NO items, but DOES drop XP
 *
 * XP on break: 15 + random(15) + random(15)
 *   Range: 15-44, average ~29.5
 *   Triangle distribution centered on ~29
 *
 * Spawning logic (from MobSpawnerBaseLogic):
 *   Entity type: string name (e.g. "Zombie", "Skeleton")
 *   Spawn delay: 200-800 ticks initially
 *     Resets to random range [minDelay, maxDelay] after spawn
 *     Default: minDelay=200, maxDelay=800
 *
 *   Spawn conditions:
 *     - Player within activating range (default 16 blocks)
 *     - Light level check for hostile mobs
 *     - Space check (no collision with blocks)
 *     - Max nearby entities check (default 6 within 4×4×4 area)
 *
 *   Spawn area: 4×1×4 blocks around spawner
 *     Random X: spawnerX + rand * 4 - 2
 *     Random Y: spawnerY
 *     Random Z: spawnerZ + rand * 4 - 2
 *
 *   Entities per spawn: 1-4 (spawnCount default = 4)
 *   maxNearbyEntities: 6
 *   activatingRange: 16 blocks (horizontal)
 *   spawnRange: 4 blocks
 *
 *   NBT fields:
 *     EntityId: type to spawn
 *     Delay: current delay
 *     MinSpawnDelay: 200
 *     MaxSpawnDelay: 800
 *     SpawnCount: 4
 *     MaxNearbyEntities: 6
 *     RequiredPlayerRange: 16
 *     SpawnRange: 4
 *     SpawnPotentials: weighted list of entity+NBT
 *
 * Block ID: 52
 *
 * Thread safety: Command block execution on server thread only.
 * JNI readiness: CommandBlockLogic accessible for Forge.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Command Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace CommandBlockConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 137;

    // ─── Metadata ───
    static constexpr int32_t POWERED_FLAG = 1;     // bit 0
    static constexpr int32_t POWERED_CLEAR = ~POWERED_FLAG;  // 0xFFFFFFFE

    // ─── Tick rate ───
    static constexpr int32_t TICK_RATE = 1;  // execute next tick

    // ─── Comparator ───
    // Output = successCount from last command execution

    // ─── Drops ───
    static constexpr int32_t DROP_QUANTITY = 0;  // no item drops

    // ─── Redstone trigger ───
    // Rising edge: powered && !wasAlreadyPowered → schedule tick
    // Falling edge: !powered && wasPowered → clear flag only
}

// ═══════════════════════════════════════════════════════════════════════════
// Mob Spawner Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace MobSpawnerConstants {
    // ─── Block ID ───
    static constexpr int32_t BLOCK_ID = 52;

    // ─── XP drops ───
    // Java: 15 + rand.nextInt(15) + rand.nextInt(15)
    static constexpr int32_t XP_BASE = 15;
    static constexpr int32_t XP_RANDOM_RANGE = 15;
    // Total range: 15 to 44
    // Average: ~29.5 (triangle distribution)

    // ─── Item drops ───
    static constexpr int32_t DROP_QUANTITY = 0;  // no item drops

    // ─── Spawning parameters (MobSpawnerBaseLogic defaults) ───
    static constexpr int32_t DEFAULT_MIN_DELAY = 200;   // ticks
    static constexpr int32_t DEFAULT_MAX_DELAY = 800;    // ticks
    static constexpr int32_t DEFAULT_SPAWN_COUNT = 4;
    static constexpr int32_t DEFAULT_MAX_NEARBY = 6;
    static constexpr int32_t DEFAULT_PLAYER_RANGE = 16;  // blocks
    static constexpr int32_t DEFAULT_SPAWN_RANGE = 4;    // blocks

    // ─── Spawn area ───
    // XZ: spawner ± spawnRange, Y: spawner
    // Random: x + rand * spawnRange*2 - spawnRange

    // ─── NBT keys ───
    static constexpr const char* NBT_ENTITY_ID = "EntityId";
    static constexpr const char* NBT_DELAY = "Delay";
    static constexpr const char* NBT_MIN_DELAY = "MinSpawnDelay";
    static constexpr const char* NBT_MAX_DELAY = "MaxSpawnDelay";
    static constexpr const char* NBT_SPAWN_COUNT = "SpawnCount";
    static constexpr const char* NBT_MAX_NEARBY = "MaxNearbyEntities";
    static constexpr const char* NBT_PLAYER_RANGE = "RequiredPlayerRange";
    static constexpr const char* NBT_SPAWN_RANGE = "SpawnRange";
    static constexpr const char* NBT_SPAWN_POTENTIALS = "SpawnPotentials";
    static constexpr const char* NBT_SPAWN_DATA = "SpawnData";
}

} // namespace mccpp
