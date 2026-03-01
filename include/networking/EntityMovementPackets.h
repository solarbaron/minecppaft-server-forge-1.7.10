/**
 * EntityMovementPackets.h — Entity spawn, movement, and metadata packet structures.
 *
 * Java references:
 *   - S0CPacketSpawnPlayer (100 lines)
 *   - S0EPacketSpawnObject (139 lines)
 *   - S0FPacketSpawnMob (130 lines)
 *   - S10PacketSpawnPainting (70 lines)
 *   - S11PacketSpawnExperienceOrb (60 lines)
 *   - S12PacketEntityVelocity (60 lines)
 *   - S13PacketDestroyEntities (40 lines)
 *   - S14PacketEntity (52 lines)
 *   - S14$S15PacketEntityRelMove (35 lines)
 *   - S14$S16PacketEntityLook (32 lines)
 *   - S14$S17PacketEntityLookMove (38 lines)
 *   - S18PacketEntityTeleport (70 lines)
 *   - S19PacketEntityHeadLook (40 lines)
 *   - S1BPacketEntityAttach (38 lines)
 *   - S1CPacketEntityMetadata (38 lines)
 *   - S1DPacketEntityEffect (44 lines)
 *   - S1EPacketRemoveEntityEffect (36 lines)
 *   - S20PacketEntityProperties (80 lines)
 *   - S04PacketEntityEquipment (48 lines)
 *   - S0BPacketAnimation (36 lines)
 *   - S0DPacketCollectItem (36 lines)
 *   - S2CPacketSpawnGlobalEntity (48 lines)
 *
 * Fixed-point coordinates: pos * 32.0 → int32_t (5 fractional bits)
 * Angle encoding: angle * 256.0f / 360.0f → byte (256 steps per revolution)
 * Velocity encoding: vel * 8000.0 → int16_t (clamped to ±3.9 blocks/tick)
 *
 * Thread safety: Packet structs are value types, no shared state.
 * JNI readiness: Simple POD structs with integer fields.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Coordinate encoding helpers
// ═══════════════════════════════════════════════════════════════════════════

namespace PacketEncoding {
    // Fixed-point position: pos * 32 → int32
    // Java: MathHelper.floor_double(pos * 32.0)
    static constexpr double POS_SCALE = 32.0;

    // Angle: angle * 256 / 360 → byte
    // Java: MathHelper.floor_float(angle * 256.0f / 360.0f)
    static constexpr float ANGLE_SCALE = 256.0f / 360.0f;

    // Velocity: vel * 8000 → short (clamped ±3.9)
    // Java: (int)(clamp(vel, -3.9, 3.9) * 8000.0)
    static constexpr double VEL_SCALE = 8000.0;
    static constexpr double VEL_CLAMP = 3.9;

    static int32_t encodePos(double pos) {
        // floor_double
        return static_cast<int32_t>(pos >= 0 ? pos * POS_SCALE : pos * POS_SCALE - 1);
    }

    static int8_t encodeAngle(float angle) {
        return static_cast<int8_t>(static_cast<int32_t>(angle * ANGLE_SCALE) & 0xFF);
    }

    static int16_t encodeVelocity(double vel) {
        if (vel < -VEL_CLAMP) vel = -VEL_CLAMP;
        if (vel > VEL_CLAMP) vel = VEL_CLAMP;
        return static_cast<int16_t>(vel * VEL_SCALE);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// GameProfile property (for S0CPacketSpawnPlayer)
// ═══════════════════════════════════════════════════════════════════════════

struct GameProfileProperty {
    std::string name;
    std::string value;
    std::string signature;
};

// ═══════════════════════════════════════════════════════════════════════════
// S0C — Spawn Player
// Packet ID: 0x0C
// Fields: VarInt entityId, String UUID, String name, VarInt propCount,
//   [String name, String value, String sig]* props,
//   Int x, Int y, Int z, Byte yaw, Byte pitch, Short currentItem,
//   DataWatcher metadata
// ═══════════════════════════════════════════════════════════════════════════

struct S0CSpawnPlayer {
    static constexpr uint8_t PACKET_ID = 0x0C;
    int32_t entityId = 0;
    std::string uuid;          // UUID as string (36 chars)
    std::string playerName;    // Max 16 chars
    std::vector<GameProfileProperty> properties;
    int32_t x = 0, y = 0, z = 0;  // Fixed-point * 32
    int8_t yaw = 0, pitch = 0;    // Angle bytes
    int16_t currentItem = 0;       // Held item ID
    // DataWatcher metadata written separately
};

// ═══════════════════════════════════════════════════════════════════════════
// S0E — Spawn Object (projectiles, minecarts, boats, etc.)
// Packet ID: 0x0E
// Fields: VarInt entityId, Byte type, Int x/y/z, Byte pitch/yaw,
//   Int data, [Short velX/Y/Z if data > 0]
//
// Object types (Java: EntityTrackerEntry.func_151260_c):
//   1=Boat, 2=ItemStack, 10=Minecart, 50=PrimedTnt,
//   51=EnderCrystal, 60=Arrow(data=shooterId),
//   61=Snowball, 62=Egg, 63=Fireball(data=shooterId),
//   64=SmallFireball(data=shooterId), 65=EnderPearl,
//   66=WitherSkull(data=shooterId), 70=FallingBlock(data=blockId|meta<<16),
//   71=ItemFrame(data=direction), 72=EyeOfEnder,
//   73=Potion(data=potionDamage), 74=FallingDragonEgg,
//   75=ExpBottle, 76=Firework, 77=LeashKnot
// ═══════════════════════════════════════════════════════════════════════════

struct S0ESpawnObject {
    static constexpr uint8_t PACKET_ID = 0x0E;
    int32_t entityId = 0;          // VarInt
    int8_t type = 0;               // Object type byte
    int32_t x = 0, y = 0, z = 0;  // Fixed-point * 32
    int8_t pitch = 0, yaw = 0;    // Angle bytes
    int32_t data = 0;             // Type-specific data
    // Velocity only present if data > 0
    int16_t velX = 0, velY = 0, velZ = 0;
    bool hasVelocity() const { return data > 0; }

    // Object type constants
    static constexpr int8_t TYPE_BOAT = 1;
    static constexpr int8_t TYPE_ITEM = 2;
    static constexpr int8_t TYPE_MINECART = 10;
    static constexpr int8_t TYPE_PRIMED_TNT = 50;
    static constexpr int8_t TYPE_ENDER_CRYSTAL = 51;
    static constexpr int8_t TYPE_ARROW = 60;
    static constexpr int8_t TYPE_SNOWBALL = 61;
    static constexpr int8_t TYPE_EGG = 62;
    static constexpr int8_t TYPE_FIREBALL = 63;
    static constexpr int8_t TYPE_SMALL_FIREBALL = 64;
    static constexpr int8_t TYPE_ENDER_PEARL = 65;
    static constexpr int8_t TYPE_WITHER_SKULL = 66;
    static constexpr int8_t TYPE_FALLING_BLOCK = 70;
    static constexpr int8_t TYPE_ITEM_FRAME = 71;
    static constexpr int8_t TYPE_EYE_OF_ENDER = 72;
    static constexpr int8_t TYPE_POTION = 73;
    static constexpr int8_t TYPE_FALLING_DRAGON_EGG = 74;
    static constexpr int8_t TYPE_EXP_BOTTLE = 75;
    static constexpr int8_t TYPE_FIREWORK = 76;
    static constexpr int8_t TYPE_LEASH_KNOT = 77;
};

// ═══════════════════════════════════════════════════════════════════════════
// S0F — Spawn Mob
// Packet ID: 0x0F
// Fields: VarInt entityId, UByte type, Int x/y/z, Byte yaw/pitch/headYaw,
//   Short velX/Y/Z, DataWatcher metadata
// ═══════════════════════════════════════════════════════════════════════════

struct S0FSpawnMob {
    static constexpr uint8_t PACKET_ID = 0x0F;
    int32_t entityId = 0;
    uint8_t mobType = 0;            // Entity type ID (50-120)
    int32_t x = 0, y = 0, z = 0;
    int8_t yaw = 0, pitch = 0;
    int8_t headYaw = 0;
    int16_t velX = 0, velY = 0, velZ = 0;
    // DataWatcher metadata appended
};

// ═══════════════════════════════════════════════════════════════════════════
// S10 — Spawn Painting
// Packet ID: 0x10
// Fields: VarInt entityId, String title, Int x/y/z, Int direction
// ═══════════════════════════════════════════════════════════════════════════

struct S10SpawnPainting {
    static constexpr uint8_t PACKET_ID = 0x10;
    int32_t entityId = 0;
    std::string title;
    int32_t x = 0, y = 0, z = 0;
    int32_t direction = 0;  // 0=South, 1=West, 2=North, 3=East
};

// ═══════════════════════════════════════════════════════════════════════════
// S11 — Spawn Experience Orb
// Packet ID: 0x11
// Fields: VarInt entityId, Int x/y/z, Short xpValue
// ═══════════════════════════════════════════════════════════════════════════

struct S11SpawnExperienceOrb {
    static constexpr uint8_t PACKET_ID = 0x11;
    int32_t entityId = 0;
    int32_t x = 0, y = 0, z = 0;
    int16_t xpValue = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// S12 — Entity Velocity
// Packet ID: 0x12
// Fields: Int entityId, Short velX/Y/Z
// ═══════════════════════════════════════════════════════════════════════════

struct S12EntityVelocity {
    static constexpr uint8_t PACKET_ID = 0x12;
    int32_t entityId = 0;
    int16_t velX = 0, velY = 0, velZ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// S13 — Destroy Entities
// Packet ID: 0x13
// Fields: VarInt count, [VarInt entityId]*
// ═══════════════════════════════════════════════════════════════════════════

struct S13DestroyEntities {
    static constexpr uint8_t PACKET_ID = 0x13;
    std::vector<int32_t> entityIds;
};

// ═══════════════════════════════════════════════════════════════════════════
// S14 — Entity (base, no movement)
// Packet ID: 0x14
// Fields: Int entityId
//
// S15 — Entity Relative Move
// Packet ID: 0x15
// Fields: Int entityId, Byte dX, Byte dY, Byte dZ
//   Delta: (byte)(pos*32 - lastPos*32), max ±4 blocks
//
// S16 — Entity Look
// Packet ID: 0x16
// Fields: Int entityId, Byte yaw, Byte pitch
//
// S17 — Entity Look and Relative Move
// Packet ID: 0x17
// Fields: Int entityId, Byte dX/dY/dZ, Byte yaw/pitch
// ═══════════════════════════════════════════════════════════════════════════

struct S14Entity {
    static constexpr uint8_t PACKET_ID = 0x14;
    int32_t entityId = 0;
};

struct S15EntityRelMove {
    static constexpr uint8_t PACKET_ID = 0x15;
    int32_t entityId = 0;
    int8_t dX = 0, dY = 0, dZ = 0;  // Fixed-point delta: (newPos-oldPos)*32
};

struct S16EntityLook {
    static constexpr uint8_t PACKET_ID = 0x16;
    int32_t entityId = 0;
    int8_t yaw = 0, pitch = 0;
};

struct S17EntityLookMove {
    static constexpr uint8_t PACKET_ID = 0x17;
    int32_t entityId = 0;
    int8_t dX = 0, dY = 0, dZ = 0;
    int8_t yaw = 0, pitch = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// S18 — Entity Teleport (absolute position)
// Packet ID: 0x18
// Fields: Int entityId, Int x/y/z, Byte yaw, Byte pitch
// ═══════════════════════════════════════════════════════════════════════════

struct S18EntityTeleport {
    static constexpr uint8_t PACKET_ID = 0x18;
    int32_t entityId = 0;
    int32_t x = 0, y = 0, z = 0;  // Fixed-point * 32
    int8_t yaw = 0, pitch = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// S19 — Entity Head Look
// Packet ID: 0x19
// Fields: Int entityId, Byte headYaw
// ═══════════════════════════════════════════════════════════════════════════

struct S19EntityHeadLook {
    static constexpr uint8_t PACKET_ID = 0x19;
    int32_t entityId = 0;
    int8_t headYaw = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// S1B — Entity Attach (leash, riding)
// Packet ID: 0x1B
// Fields: Int entityId, Int vehicleId, UByte leash
//   leash: 0 = attach, 1 = leash
// ═══════════════════════════════════════════════════════════════════════════

struct S1BEntityAttach {
    static constexpr uint8_t PACKET_ID = 0x1B;
    int32_t entityId = 0;
    int32_t vehicleId = 0;
    uint8_t leash = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// S1C — Entity Metadata
// Packet ID: 0x1C
// Fields: Int entityId, DataWatcher metadata
// ═══════════════════════════════════════════════════════════════════════════

struct S1CEntityMetadata {
    static constexpr uint8_t PACKET_ID = 0x1C;
    int32_t entityId = 0;
    // DataWatcher entries serialized separately
    // Format: [(byte typeAndIndex, value)*] terminated by 0x7F
};

// ═══════════════════════════════════════════════════════════════════════════
// S1D — Entity Effect
// Packet ID: 0x1D
// Fields: Int entityId, Byte effectId, Byte amplifier, Short duration
// ═══════════════════════════════════════════════════════════════════════════

struct S1DEntityEffect {
    static constexpr uint8_t PACKET_ID = 0x1D;
    int32_t entityId = 0;
    int8_t effectId = 0;
    int8_t amplifier = 0;
    int16_t duration = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// S1E — Remove Entity Effect
// Packet ID: 0x1E
// Fields: Int entityId, Byte effectId
// ═══════════════════════════════════════════════════════════════════════════

struct S1ERemoveEntityEffect {
    static constexpr uint8_t PACKET_ID = 0x1E;
    int32_t entityId = 0;
    int8_t effectId = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// S04 — Entity Equipment
// Packet ID: 0x04
// Fields: Int entityId, Short slot, Slot item
//   slot: 0=held, 1-4=armor boots→helmet
// ═══════════════════════════════════════════════════════════════════════════

struct S04EntityEquipment {
    static constexpr uint8_t PACKET_ID = 0x04;
    int32_t entityId = 0;
    int16_t slot = 0;
    // Item data serialized separately (Slot format)
    int16_t itemId = -1;    // -1 = empty
    int8_t count = 0;
    int16_t damage = 0;
    // NBT data follows if itemId != -1
};

// ═══════════════════════════════════════════════════════════════════════════
// S0B — Animation
// Packet ID: 0x0B
// Fields: VarInt entityId, UByte animation
//   0=Swing arm, 1=Take damage, 2=Leave bed, 3=Eat food, 4=Critical, 5=Magic crit
// ═══════════════════════════════════════════════════════════════════════════

struct S0BAnimation {
    static constexpr uint8_t PACKET_ID = 0x0B;
    int32_t entityId = 0;
    uint8_t animation = 0;

    static constexpr uint8_t ANIM_SWING_ARM = 0;
    static constexpr uint8_t ANIM_TAKE_DAMAGE = 1;
    static constexpr uint8_t ANIM_LEAVE_BED = 2;
    static constexpr uint8_t ANIM_EAT_FOOD = 3;
    static constexpr uint8_t ANIM_CRITICAL = 4;
    static constexpr uint8_t ANIM_MAGIC_CRITICAL = 5;
};

// ═══════════════════════════════════════════════════════════════════════════
// S0D — Collect Item (pickup animation)
// Packet ID: 0x0D
// Fields: Int collectedEntityId, Int collectorEntityId
// ═══════════════════════════════════════════════════════════════════════════

struct S0DCollectItem {
    static constexpr uint8_t PACKET_ID = 0x0D;
    int32_t collectedEntityId = 0;
    int32_t collectorEntityId = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// S2C — Spawn Global Entity (lightning bolt)
// Packet ID: 0x2C
// Fields: VarInt entityId, Byte type(1=lightning), Int x/y/z
// ═══════════════════════════════════════════════════════════════════════════

struct S2CSpawnGlobalEntity {
    static constexpr uint8_t PACKET_ID = 0x2C;
    int32_t entityId = 0;
    int8_t type = 1;  // 1 = thunderbolt
    int32_t x = 0, y = 0, z = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// S20 — Entity Properties (attribute modifiers)
// Packet ID: 0x20
// Fields: Int entityId, Int count,
//   [String key, Double value, VarInt modCount,
//    [Long uuidHigh, Long uuidLow, Double amount, Byte operation]*]*
// ═══════════════════════════════════════════════════════════════════════════

struct EntityAttributeModifier {
    int64_t uuidHigh = 0, uuidLow = 0;
    double amount = 0.0;
    int8_t operation = 0;  // 0=add, 1=multiplyBase, 2=multiplyTotal
};

struct EntityAttribute {
    std::string key;  // e.g., "generic.maxHealth"
    double baseValue = 0.0;
    std::vector<EntityAttributeModifier> modifiers;
};

struct S20EntityProperties {
    static constexpr uint8_t PACKET_ID = 0x20;
    int32_t entityId = 0;
    std::vector<EntityAttribute> attributes;
};

// ═══════════════════════════════════════════════════════════════════════════
// S19 — Entity Status (second S19, different packet ID in wiki)
// Packet ID: 0x1A
// Fields: Int entityId, Byte status
//   Status codes:
//     2=hurt, 3=dead, 6=wolfTaming, 7=wolfTamed,
//     8=shaking water, 9=eating, 10=sheep grass,
//     15=witchMagic, 16=zombieConversion, 17=fireworkExplosion
// ═══════════════════════════════════════════════════════════════════════════

struct S1AEntityStatus {
    static constexpr uint8_t PACKET_ID = 0x1A;
    int32_t entityId = 0;
    int8_t status = 0;

    static constexpr int8_t STATUS_HURT = 2;
    static constexpr int8_t STATUS_DEAD = 3;
    static constexpr int8_t STATUS_WOLF_TAMING = 6;
    static constexpr int8_t STATUS_WOLF_TAMED = 7;
    static constexpr int8_t STATUS_SHAKE_WATER = 8;
    static constexpr int8_t STATUS_EATING = 9;
    static constexpr int8_t STATUS_SHEEP_GRASS = 10;
    static constexpr int8_t STATUS_WITCH_MAGIC = 15;
    static constexpr int8_t STATUS_ZOMBIE_CONVERT = 16;
    static constexpr int8_t STATUS_FIREWORK = 17;
};

} // namespace mccpp
