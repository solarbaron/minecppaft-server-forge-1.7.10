/**
 * DispenserDropper.h — Dispenser and dropper tile entity and block mechanics.
 *
 * Java references:
 *   - net.minecraft.tileentity.TileEntityDispenser (162 lines)
 *   - net.minecraft.block.BlockDispenser (202 lines)
 *   - net.minecraft.block.BlockDropper (36 lines)
 *   - net.minecraft.dispenser.BehaviorDefaultDispenseItem (75 lines)
 *   - net.minecraft.dispenser.BehaviorProjectileDispense (65 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TILE ENTITY DISPENSER (TileEntityDispenser)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Inventory: 9 slots (3×3 grid), stack limit 64
 *
 * Random slot selection (func_146017_i — reservoir sampling):
 *   - For each non-null slot i: with probability 1/(count_so_far + 1)
 *     select this slot
 *   - Returns -1 if empty (all null)
 *   - Ensures uniform distribution among non-empty slots
 *
 * Add item to first empty slot (func_146019_a):
 *   - Linear scan for null/null-item slot, place item
 *   - Return slot index, or -1 if full
 *
 * NBT: Items (TagList, slot byte, unsigned: & 0xFF), CustomName (string)
 * Useable range: 64.0 distance²
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BLOCK DISPENSER (BlockDispenser)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Tick rate: 4 ticks (0.2 seconds)
 *
 * Redstone activation (onNeighborBlockChange):
 *   - Check: isBlockIndirectlyGettingPowered at (x,y,z) OR (x,y+1,z)
 *   - Metadata bit 3 = "triggered" flag
 *   - Rising edge (powered && !triggered):
 *     scheduleBlockUpdate with tickRate(4)
 *     set triggered flag
 *   - Falling edge (!powered && triggered):
 *     clear triggered flag
 *
 * updateTick (scheduled, server-side):
 *   - Call func_149941_e to dispense
 *
 * Dispense logic (func_149941_e):
 *   1. Get random non-empty slot from tile entity
 *   2. If empty (slot -1): play click sound (effect 1001)
 *   3. Get item's dispense behavior from registry
 *   4. Call behavior.dispense(blockSource, itemStack)
 *   5. Put result back (null if stackSize 0)
 *
 * Dispense behavior registry:
 *   - RegistryDefaulted: default = BehaviorDefaultDispenseItem
 *   - Registered per Item class (see DispenserBehaviors below)
 *
 * Dispense position (getIPositionFromBlockSource):
 *   - X = blockX + 0.7 * facingOffsetX
 *   - Y = blockY + 0.7 * facingOffsetY
 *   - Z = blockZ + 0.7 * facingOffsetZ
 *
 * Auto-facing on placement:
 *   - Uses BlockPistonBase.determineOrientation (player look direction)
 *   - Dispenser can face all 6 directions
 *
 * Initial face calculation (func_149938_m — non-player placement):
 *   - Check south/north/east/west neighbors for solid blocks
 *   - Face away from solid blocks
 *
 * Block break: scatter all items with gaussian motion + 0.2 upward
 *   - Stack splits: 10-31 items per EntityItem
 *   - Offset: position + rand * 0.8 + 0.1 (centered in block)
 *   - Motion: gaussian * 0.05, Y += 0.2
 *
 * Comparator: Container.calcRedstoneFromInventory
 *
 * ═══════════════════════════════════════════════════════════════════════
 * BLOCK DROPPER (BlockDropper — extends BlockDispenser)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Overrides dispense to NOT use behavior registry:
 *   - Always drops item as EntityItem (like BehaviorDefaultDispenseItem)
 *   - OR inserts into adjacent IInventory (hopper-like insertion)
 *
 * Container name: "container.dropper"
 *
 * Thread safety: Block updates on server thread.
 * JNI readiness: Simple registry pattern for dispense behaviors.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Dispenser Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DispenserConstants {
    // ─── Block IDs ───
    static constexpr int32_t DISPENSER_ID = 23;
    static constexpr int32_t DROPPER_ID = 158;

    // ─── Inventory ───
    static constexpr int32_t INVENTORY_SIZE = 9;  // 3×3
    static constexpr int32_t STACK_LIMIT = 64;

    // ─── Tick rate ───
    // Java: tickRate = 4 (0.2 seconds)
    static constexpr int32_t TICK_RATE = 4;

    // ─── Block metadata ───
    // Java: direction = meta & 7, triggered = (meta & 8) != 0
    static constexpr int32_t DIRECTION_MASK = 7;
    static constexpr int32_t TRIGGERED_FLAG = 8;

    // Direction values (same as EnumFacing ordinals):
    // 0 = DOWN, 1 = UP, 2 = NORTH, 3 = SOUTH, 4 = WEST, 5 = EAST
    static constexpr int32_t DIR_DOWN = 0;
    static constexpr int32_t DIR_UP = 1;
    static constexpr int32_t DIR_NORTH = 2;
    static constexpr int32_t DIR_SOUTH = 3;
    static constexpr int32_t DIR_WEST = 4;
    static constexpr int32_t DIR_EAST = 5;

    inline int32_t getDirection(int32_t metadata) {
        return metadata & DIRECTION_MASK;
    }

    inline bool isTriggered(int32_t metadata) {
        return (metadata & TRIGGERED_FLAG) != 0;
    }

    // ─── Dispense position offset ───
    // Java: blockPos + 0.7 * facingOffset
    static constexpr double DISPENSE_OFFSET = 0.7;

    // ─── Empty dispenser sound ───
    // Java: playAuxSFX(1001, ...) — click sound
    static constexpr int32_t SFX_CLICK = 1001;
    // Java: playAuxSFX(1000, ...) — dispense sound
    static constexpr int32_t SFX_DISPENSE = 1000;
    // Java: playAuxSFX(1002, ...) — shoot arrow/fireball
    static constexpr int32_t SFX_LAUNCH = 1002;

    // ─── Useable distance ───
    static constexpr double USE_DISTANCE_SQ = 64.0;

    // ─── Container names ───
    static constexpr const char* DISPENSER_NAME = "container.dispenser";
    static constexpr const char* DROPPER_NAME = "container.dropper";

    // ─── NBT tags ───
    static constexpr const char* TAG_ITEMS = "Items";
    static constexpr const char* TAG_CUSTOM_NAME = "CustomName";
    static constexpr const char* TAG_SLOT = "Slot";
}

// ═══════════════════════════════════════════════════════════════════════════
// EnumFacing offsets for dispenser
// Java: EnumFacing.getFront(direction).getFrontOffset[XYZ]
// ═══════════════════════════════════════════════════════════════════════════

namespace EnumFacingOffsets {
    // Index by direction: 0=DOWN, 1=UP, 2=NORTH, 3=SOUTH, 4=WEST, 5=EAST
    static constexpr int32_t frontOffsetX[] = { 0,  0,  0,  0, -1,  1};
    static constexpr int32_t frontOffsetY[] = {-1,  1,  0,  0,  0,  0};
    static constexpr int32_t frontOffsetZ[] = { 0,  0, -1,  1,  0,  0};
}

// ═══════════════════════════════════════════════════════════════════════════
// Item scatter on block break
// Java: BlockDispenser.breakBlock — scatter items with motion
// ═══════════════════════════════════════════════════════════════════════════

namespace ItemScatter {
    // ─── Position offset ───
    // Java: (float)n + rand * 0.8f + 0.1f
    static constexpr float OFFSET_SCALE = 0.8f;
    static constexpr float OFFSET_BASE = 0.1f;

    // ─── Stack split range ───
    // Java: nextInt(21) + 10 = 10..30
    static constexpr int32_t SPLIT_MIN = 10;
    static constexpr int32_t SPLIT_RANGE = 21;

    // ─── Motion ───
    // Java: gaussian * 0.05f, Y += 0.2f
    static constexpr float MOTION_SCALE = 0.05f;
    static constexpr float MOTION_Y_BONUS = 0.2f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Default Dispense Behavior
// Java: BehaviorDefaultDispenseItem — shoot item as EntityItem
// ═══════════════════════════════════════════════════════════════════════════

namespace DefaultDispenseBehavior {
    // ─── Entity spawn position ───
    // Java: dispenseStack position at block + 0.7 * facing
    // Then entity at position with motion

    // ─── Item velocity ───
    // Java: motionX/Z = 6.0 * facingOffset, motionY = 0.2
    // With gaussian random * 0.0075 * accuracy(6)
    static constexpr double BASE_SPEED = 6.0;
    static constexpr double Y_SPEED = 0.2;
    static constexpr double UNCERTAINTY = 0.0075;
    static constexpr int32_t ACCURACY = 6;

    // Java: item spawn delay = 10 ticks after dispense
    static constexpr int32_t PICKUP_DELAY = 10;

    // ─── dispenseSound ───
    // Java: world.playAuxSFX(1000, x, y, z, 0)
    static constexpr int32_t DISPENSE_SOUND_ID = 1000;

    // ─── dispenseParticles ───
    // Java: world.playAuxSFX(2000, x, y, z, direction)
    static constexpr int32_t PARTICLE_EFFECT_ID = 2000;
}

// ═══════════════════════════════════════════════════════════════════════════
// Registered Dispense Behaviors (from BlockDispenser static init)
// Java: Bootstrap.func_151584_a() registers special behaviors
// ═══════════════════════════════════════════════════════════════════════════

namespace DispenseBehaviors {
    // ─── Projectile items ───
    // Items that use BehaviorProjectileDispense:
    // - Arrow (262): EntityArrow
    // - Egg (344): EntityEgg
    // - Snowball (332): EntitySnowball
    // - XP Bottle (384): EntityExpBottle
    // - Splash Potion (373): EntityPotion (splash only)
    // - Firework (401): EntityFireworkRocket
    // - Fire Charge (385): EntitySmallFireball

    // ─── Projectile velocity ───
    // Java: BehaviorProjectileDispense
    // getProjectileInaccuracy() varies by type
    static constexpr float ARROW_INACCURACY = 6.0f;
    static constexpr float EGG_INACCURACY = 6.0f;
    static constexpr float SNOWBALL_INACCURACY = 6.0f;
    static constexpr float XP_BOTTLE_INACCURACY = 6.0f;
    static constexpr float FIRE_CHARGE_INACCURACY = 6.0f;

    // Java: projectile velocity
    // Arrow: 1.1, Egg/Snowball: 1.1/3, XP Bottle: 1.1/3
    static constexpr float ARROW_VELOCITY = 1.1f;
    // Generic (egg, snowball, xp bottle)
    static constexpr float GENERIC_VELOCITY = 1.1f;

    // ─── Special behaviors ───
    // Water Bucket (326): place water source at position
    // Lava Bucket (327): place lava source at position
    // Bucket (325): pick up fluid at position
    // Flint and Steel (259): ignite block
    // Bone Meal (351 meta 15): apply bone meal
    // TNT (46): prime TNT entity
    // Spawn Egg (383): spawn entity
    // Minecart (328): place minecart on rails
    // Boat (333): place boat on water
    // Firework (401): launch firework rocket
    // Pumpkin/Skull: equip armor stand / wither assembly
    // Command Block Minecart (422): place special minecart

    // ─── Liquid dispense ───
    static constexpr int32_t WATER_BUCKET_ID = 326;
    static constexpr int32_t LAVA_BUCKET_ID = 327;
    static constexpr int32_t BUCKET_ID = 325;
    static constexpr int32_t FLINT_AND_STEEL_ID = 259;
    static constexpr int32_t TNT_BLOCK_ID = 46;
    static constexpr int32_t SPAWN_EGG_ID = 383;
    static constexpr int32_t BOAT_ID = 333;
    static constexpr int32_t MINECART_ID = 328;
    static constexpr int32_t FIREWORK_ID = 401;
}

// ═══════════════════════════════════════════════════════════════════════════
// Dropper-specific Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace DropperConstants {
    // ─── Dropper behavior ───
    // Java: BlockDropper overrides func_149941_e
    // If destination has IInventory: insert item via hopper insertion
    // Else: drop as EntityItem (default behavior)

    // ─── Insert side ───
    // Java: uses Facing.oppositeSide for insertion direction
    // Same logic as hopper push

    // ─── Drop effect IDs ───
    // Java: playAuxSFX(1000, ...) — success
    //       playAuxSFX(1001, ...) — empty click
    static constexpr int32_t SFX_DISPENSE = 1000;
    static constexpr int32_t SFX_CLICK = 1001;

    // ─── Drop behavior ───
    // When dropping (no adjacent inventory):
    // Same as BehaviorDefaultDispenseItem
    // When inserting (adjacent inventory):
    // Transfer 1 item, respecting ISidedInventory
}

// ═══════════════════════════════════════════════════════════════════════════
// Reservoir Sampling (random non-empty slot selection)
// Java: func_146017_i — uniform random from non-empty slots
// ═══════════════════════════════════════════════════════════════════════════

// Algorithm:
// int selected = -1; int count = 1;
// for (i = 0; i < 9; i++) {
//   if (slots[i] == null) continue;
//   if (rand.nextInt(count++) == 0) selected = i;
// }
// return selected;

// This produces uniform distribution: each non-empty slot has
// equal probability of being chosen. The beauty is it needs
// only a single pass and O(1) memory.

} // namespace mccpp
