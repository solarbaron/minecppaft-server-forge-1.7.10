/**
 * WorldEventPackets.h — World change, block update, and UI packet structures.
 *
 * Java references:
 *   - S23PacketBlockChange (59 lines)
 *   - S22PacketMultiBlockChange (82 lines)
 *   - S24PacketBlockAction (52 lines)
 *   - S25PacketBlockBreakAnim (42 lines)
 *   - S27PacketExplosion (95 lines)
 *   - S28PacketEffect (45 lines)
 *   - S29PacketSoundEffect (60 lines)
 *   - S2APacketParticles (88 lines)
 *   - S2BPacketChangeGameState (40 lines)
 *   - S2DPacketOpenWindow (64 lines)
 *   - S2EPacketCloseWindow (30 lines)
 *   - S2FPacketSetSlot (45 lines)
 *   - S30PacketWindowItems (60 lines)
 *   - S31PacketWindowProperty (36 lines)
 *   - S32PacketConfirmTransaction (40 lines)
 *   - S33PacketUpdateSign (52 lines)
 *   - S34PacketMaps (60 lines)
 *   - S35PacketUpdateTileEntity (52 lines)
 *   - S36PacketSignEditorOpen (30 lines)
 *   - S38PacketPlayerListItem (48 lines)
 *   - S39PacketPlayerAbilities (52 lines)
 *   - S3APacketTabComplete (36 lines)
 *   - S3BPacketScoreboardObjective (48 lines)
 *   - S3CPacketUpdateScore (52 lines)
 *   - S3DPacketDisplayScoreboard (36 lines)
 *   - S3EPacketTeams (92 lines)
 *   - S3FPacketCustomPayload (36 lines)
 *   - S40PacketDisconnect (32 lines)
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
// S23 — Block Change
// Packet ID: 0x23
// Fields: Int x, UByte y, Int z, VarInt blockId, UByte metadata
// ═══════════════════════════════════════════════════════════════════════════

struct S23BlockChange {
    static constexpr uint8_t PACKET_ID = 0x23;
    int32_t x = 0;
    uint8_t y = 0;
    int32_t z = 0;
    int32_t blockId = 0;   // VarInt
    uint8_t metadata = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// S22 — Multi Block Change
// Packet ID: 0x22
// Fields: Int chunkX, Int chunkZ, VarInt recordCount,
//   Int dataSize, [Short coordAndMeta(xz4+y8+?4), VarInt blockId]*
//
// Record format: (x<<12 | z<<8 | y) as short, then VarInt blockId<<4|meta
// ═══════════════════════════════════════════════════════════════════════════

struct MultiBlockRecord {
    uint8_t xRel = 0;    // 0-15 within chunk
    uint8_t yAbs = 0;    // 0-255
    uint8_t zRel = 0;    // 0-15 within chunk
    int32_t blockId = 0;
    uint8_t metadata = 0;
};

struct S22MultiBlockChange {
    static constexpr uint8_t PACKET_ID = 0x22;
    int32_t chunkX = 0, chunkZ = 0;
    std::vector<MultiBlockRecord> records;
};

// ═══════════════════════════════════════════════════════════════════════════
// S24 — Block Action (note blocks, pistons, chests)
// Packet ID: 0x24
// Fields: Int x, Short y, Int z, UByte data1, UByte data2, VarInt blockId
//   NoteBlock: data1=instrument, data2=pitch
//   Piston: data1=0extend/1retract, data2=direction
//   Chest: data1=1, data2=numPlayersViewing
// ═══════════════════════════════════════════════════════════════════════════

struct S24BlockAction {
    static constexpr uint8_t PACKET_ID = 0x24;
    int32_t x = 0;
    int16_t y = 0;
    int32_t z = 0;
    uint8_t data1 = 0;
    uint8_t data2 = 0;
    int32_t blockId = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// S25 — Block Break Animation
// Packet ID: 0x25
// Fields: VarInt entityId, Int x/y/z, Byte destroyStage (0-9, or -1 to reset)
// ═══════════════════════════════════════════════════════════════════════════

struct S25BlockBreakAnim {
    static constexpr uint8_t PACKET_ID = 0x25;
    int32_t entityId = 0;
    int32_t x = 0, y = 0, z = 0;
    int8_t destroyStage = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// S27 — Explosion
// Packet ID: 0x27
// Fields: Float x/y/z, Float radius, Int recordCount,
//   [Byte dx, Byte dy, Byte dz]* records,
//   Float playerMotionX/Y/Z
// ═══════════════════════════════════════════════════════════════════════════

struct ExplosionRecord {
    int8_t dx = 0, dy = 0, dz = 0;  // Relative to explosion center
};

struct S27Explosion {
    static constexpr uint8_t PACKET_ID = 0x27;
    float x = 0, y = 0, z = 0;
    float radius = 0;
    std::vector<ExplosionRecord> records;
    float playerMotionX = 0, playerMotionY = 0, playerMotionZ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// S28 — Effect (block break particles, sounds)
// Packet ID: 0x28
// Fields: Int effectId, Int x, Byte y, Int z, Int data, Bool global
//   1000=Click2, 1001=Click1, 1002=Bow, 1003=Door, 1004=Fizz,
//   1005=RecordPlay(data=recordItemId), 1010=GhastWarn, 1011=GhastShoot,
//   1012=ZombieWood, 1014=ZombieMetal, 1015=ZombieBroken,
//   1016=Wither, 1017=WitherBorn, 2000=Smoke(data=direction),
//   2001=BlockBreak(data=blockId), 2002=SplashPotion(data=potionDamage),
//   2003=EyeOfEnder, 2004=MobSpawn, 2005=BoneMeal(data=count)
// ═══════════════════════════════════════════════════════════════════════════

struct S28Effect {
    static constexpr uint8_t PACKET_ID = 0x28;
    int32_t effectId = 0;
    int32_t x = 0;
    uint8_t y = 0;
    int32_t z = 0;
    int32_t data = 0;
    bool disableRelativeVolume = false;

    // Effect IDs
    static constexpr int32_t CLICK2 = 1000;
    static constexpr int32_t CLICK1 = 1001;
    static constexpr int32_t BOW_FIRE = 1002;
    static constexpr int32_t DOOR = 1003;
    static constexpr int32_t FIZZ = 1004;
    static constexpr int32_t RECORD_PLAY = 1005;
    static constexpr int32_t GHAST_WARN = 1007;
    static constexpr int32_t GHAST_SHOOT = 1009;
    static constexpr int32_t BLAZE_SHOOT = 1018;
    static constexpr int32_t ZOMBIE_WOOD = 1012;
    static constexpr int32_t ZOMBIE_METAL = 1014;
    static constexpr int32_t ZOMBIE_BROKEN = 1015;
    static constexpr int32_t WITHER_SHOOT = 1016;
    static constexpr int32_t WITHER_BORN = 1017;
    static constexpr int32_t SMOKE = 2000;
    static constexpr int32_t BLOCK_BREAK = 2001;
    static constexpr int32_t SPLASH_POTION = 2002;
    static constexpr int32_t EYE_OF_ENDER = 2003;
    static constexpr int32_t MOB_SPAWN = 2004;
    static constexpr int32_t BONE_MEAL = 2005;
};

// ═══════════════════════════════════════════════════════════════════════════
// S29 — Sound Effect
// Packet ID: 0x29
// Fields: String soundName, Int x*8, Int y*8, Int z*8, Float volume, Byte pitch
//   Pitch: 63 = normal, ranges 0-255
// ═══════════════════════════════════════════════════════════════════════════

struct S29SoundEffect {
    static constexpr uint8_t PACKET_ID = 0x29;
    std::string soundName;
    int32_t x = 0, y = 0, z = 0;  // Position * 8 (fixed-point)
    float volume = 1.0f;
    uint8_t pitch = 63;
};

// ═══════════════════════════════════════════════════════════════════════════
// S2A — Particles
// Packet ID: 0x2A
// Fields: String particleName, Float x/y/z, Float offsetX/Y/Z,
//   Float speed, Int count
// ═══════════════════════════════════════════════════════════════════════════

struct S2AParticles {
    static constexpr uint8_t PACKET_ID = 0x2A;
    std::string particleName;
    float x = 0, y = 0, z = 0;
    float offsetX = 0, offsetY = 0, offsetZ = 0;
    float speed = 0;
    int32_t count = 1;
};

// ═══════════════════════════════════════════════════════════════════════════
// S2B — Change Game State
// Packet ID: 0x2B
// Fields: UByte reason, Float value
//   0=Invalid bed, 1=Rain start, 2=Rain end, 3=Change gamemode,
//   4=Win game, 5=Demo message, 6=Arrow hit player,
//   7=Fade value (thundering), 8=Fade time, 10=Pufferfish sting
// ═══════════════════════════════════════════════════════════════════════════

struct S2BChangeGameState {
    static constexpr uint8_t PACKET_ID = 0x2B;
    uint8_t reason = 0;
    float value = 0;

    static constexpr uint8_t INVALID_BED = 0;
    static constexpr uint8_t RAIN_START = 1;
    static constexpr uint8_t RAIN_END = 2;
    static constexpr uint8_t CHANGE_GAMEMODE = 3;
    static constexpr uint8_t WIN_GAME = 4;
    static constexpr uint8_t DEMO_MESSAGE = 5;
    static constexpr uint8_t ARROW_HIT_PLAYER = 6;
    static constexpr uint8_t FADE_VALUE = 7;
    static constexpr uint8_t FADE_TIME = 8;
};

// ═══════════════════════════════════════════════════════════════════════════
// Window / Container Packets
// ═══════════════════════════════════════════════════════════════════════════

// S2D — Open Window
// Packet ID: 0x2D
// Fields: UByte windowId, UByte inventoryType, String title,
//   UByte slotCount, Bool useProvidedTitle, [Int entityId if type=11]
//
// Inventory types:
//   0=Chest, 1=Workbench, 2=Furnace, 3=Dispenser, 4=Enchanting,
//   5=Brewing, 6=Villager, 7=Beacon, 8=Anvil, 9=Hopper,
//   10=Dropper, 11=Horse
struct S2DOpenWindow {
    static constexpr uint8_t PACKET_ID = 0x2D;
    uint8_t windowId = 0;
    uint8_t inventoryType = 0;
    std::string title;
    uint8_t slotCount = 0;
    bool useProvidedTitle = true;
    int32_t entityId = 0;  // Only for type 11 (horse)
};

// S2E — Close Window
// Packet ID: 0x2E
// Fields: UByte windowId
struct S2ECloseWindow {
    static constexpr uint8_t PACKET_ID = 0x2E;
    uint8_t windowId = 0;
};

// S2F — Set Slot
// Packet ID: 0x2F
// Fields: Byte windowId, Short slot, Slot slotData
struct S2FSetSlot {
    static constexpr uint8_t PACKET_ID = 0x2F;
    int8_t windowId = 0;
    int16_t slot = 0;
    // Slot data written separately (itemId short, count byte, damage short, NBT)
    int16_t itemId = -1;
    int8_t count = 0;
    int16_t damage = 0;
};

// S30 — Window Items
// Packet ID: 0x30
// Fields: UByte windowId, Short count, [Slot]* items
struct S30WindowItems {
    static constexpr uint8_t PACKET_ID = 0x30;
    uint8_t windowId = 0;
    int16_t slotCount = 0;
    // Slots written separately
};

// S31 — Window Property (furnace progress, enchant levels, etc.)
// Packet ID: 0x31
// Fields: UByte windowId, Short property, Short value
struct S31WindowProperty {
    static constexpr uint8_t PACKET_ID = 0x31;
    uint8_t windowId = 0;
    int16_t property = 0;
    int16_t value = 0;
};

// S32 — Confirm Transaction
// Packet ID: 0x32
// Fields: UByte windowId, Short actionNumber, Bool accepted
struct S32ConfirmTransaction {
    static constexpr uint8_t PACKET_ID = 0x32;
    uint8_t windowId = 0;
    int16_t actionNumber = 0;
    bool accepted = false;
};

// ═══════════════════════════════════════════════════════════════════════════
// World Data Packets
// ═══════════════════════════════════════════════════════════════════════════

// S33 — Update Sign
// Packet ID: 0x33
// Fields: Int x, Short y, Int z, String line1-4
struct S33UpdateSign {
    static constexpr uint8_t PACKET_ID = 0x33;
    int32_t x = 0;
    int16_t y = 0;
    int32_t z = 0;
    std::string line1, line2, line3, line4;
};

// S35 — Update Tile Entity
// Packet ID: 0x35
// Fields: Int x, Short y, Int z, UByte action, Short dataLength, [Byte]* nbtData
//   Actions: 1=MobSpawner, 2=CommandBlock, 3=Beacon, 4=Skull, 5=FlowerPot
struct S35UpdateTileEntity {
    static constexpr uint8_t PACKET_ID = 0x35;
    int32_t x = 0;
    int16_t y = 0;
    int32_t z = 0;
    uint8_t action = 0;
    // NBT data appended separately

    static constexpr uint8_t ACTION_MOB_SPAWNER = 1;
    static constexpr uint8_t ACTION_COMMAND_BLOCK = 2;
    static constexpr uint8_t ACTION_BEACON = 3;
    static constexpr uint8_t ACTION_SKULL = 4;
    static constexpr uint8_t ACTION_FLOWER_POT = 5;
};

// S36 — Sign Editor Open
// Packet ID: 0x36
// Fields: Int x, Int y, Int z
struct S36SignEditorOpen {
    static constexpr uint8_t PACKET_ID = 0x36;
    int32_t x = 0, y = 0, z = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// Player List / Scoreboard / Tab
// ═══════════════════════════════════════════════════════════════════════════

// S38 — Player List Item
// Packet ID: 0x38
// Fields: String playerName, Bool online, Short ping
struct S38PlayerListItem {
    static constexpr uint8_t PACKET_ID = 0x38;
    std::string playerName;  // Max 16 chars
    bool online = true;
    int16_t ping = 0;        // Milliseconds
};

// S39 — Player Abilities
// Packet ID: 0x39
// Fields: Byte flags, Float flySpeed, Float walkSpeed
//   Flags: 0x01=invulnerable, 0x02=flying, 0x04=allowFlying, 0x08=creativeMode
struct S39PlayerAbilities {
    static constexpr uint8_t PACKET_ID = 0x39;
    uint8_t flags = 0;
    float flySpeed = 0.05f;    // Default: 0.05
    float walkSpeed = 0.1f;    // Default: 0.1

    static constexpr uint8_t FLAG_INVULNERABLE = 0x01;
    static constexpr uint8_t FLAG_FLYING = 0x02;
    static constexpr uint8_t FLAG_ALLOW_FLYING = 0x04;
    static constexpr uint8_t FLAG_CREATIVE_MODE = 0x08;
};

// S3A — Tab Complete
// Packet ID: 0x3A
// Fields: VarInt count, [String]* matches
struct S3ATabComplete {
    static constexpr uint8_t PACKET_ID = 0x3A;
    std::vector<std::string> matches;
};

// S3F — Plugin Message
// Packet ID: 0x3F
// Fields: String channel, Short length, [Byte]* data
struct S3FPluginMessage {
    static constexpr uint8_t PACKET_ID = 0x3F;
    std::string channel;
    std::vector<uint8_t> data;
};

// S40 — Disconnect
// Packet ID: 0x40
// Fields: String reason (JSON chat component)
struct S40Disconnect {
    static constexpr uint8_t PACKET_ID = 0x40;
    std::string reason;  // JSON chat component
};

} // namespace mccpp
