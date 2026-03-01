/**
 * ClientPlayPackets.h — All 26 client-to-server play state packet structures.
 *
 * Java references (net.minecraft.network.play.client):
 *   - C00PacketKeepAlive (28 lines)
 *   - C01PacketChatMessage (32 lines)
 *   - C02PacketUseEntity (44 lines)
 *   - C03PacketPlayer (77 lines) + C04/C05/C06 subclasses
 *   - C07PacketPlayerDigging (61 lines)
 *   - C08PacketPlayerBlockPlacement (83 lines)
 *   - C09PacketHeldItemChange (30 lines)
 *   - C0APacketAnimation (26 lines)
 *   - C0BPacketEntityAction (40 lines)
 *   - C0CPacketInput (36 lines)
 *   - C0DPacketCloseWindow (26 lines)
 *   - C0EPacketClickWindow (60 lines)
 *   - C0FPacketConfirmTransaction (32 lines)
 *   - C10PacketCreativeInventoryAction (36 lines)
 *   - C11PacketEnchantItem (30 lines)
 *   - C12PacketUpdateSign (44 lines)
 *   - C13PacketPlayerAbilities (40 lines)
 *   - C14PacketTabComplete (28 lines)
 *   - C15PacketClientSettings (40 lines)
 *   - C16PacketClientStatus (26 lines)
 *   - C17PacketCustomPayload (32 lines)
 *
 * Wire format notes:
 *   - C03: base reads 1 byte (onGround)
 *   - C04 (position): reads x/stance/y/z as doubles + onGround byte
 *     NOTE: stance comes BEFORE y in wire (y=feet, stance=eyes)
 *   - C05 (look): reads yaw/pitch as floats + onGround byte
 *   - C06 (posLook): reads x/stance/y/z doubles + yaw/pitch floats + onGround
 *   - C08 facing: UByte/16.0f gives 0.0-1.0 sub-block position
 *
 * Thread safety: Packet structs are value types, parsed on connection thread.
 * JNI readiness: Simple POD structs.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// C00 — Keep Alive
// Packet ID: 0x00
// Fields: Int keepAliveId
// ═══════════════════════════════════════════════════════════════════════════

struct C00KeepAlive {
    static constexpr uint8_t PACKET_ID = 0x00;
    int32_t keepAliveId = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// C01 — Chat Message
// Packet ID: 0x01
// Fields: String message (max 100 chars)
// ═══════════════════════════════════════════════════════════════════════════

struct C01ChatMessage {
    static constexpr uint8_t PACKET_ID = 0x01;
    std::string message;  // Max 100 characters
    static constexpr int32_t MAX_LENGTH = 100;
};

// ═══════════════════════════════════════════════════════════════════════════
// C02 — Use Entity
// Packet ID: 0x02
// Fields: Int targetEntityId, Byte action
//   action: 0=interact, 1=attack
// ═══════════════════════════════════════════════════════════════════════════

struct C02UseEntity {
    static constexpr uint8_t PACKET_ID = 0x02;
    int32_t targetEntityId = 0;
    int8_t action = 0;

    static constexpr int8_t ACTION_INTERACT = 0;
    static constexpr int8_t ACTION_ATTACK = 1;
};

// ═══════════════════════════════════════════════════════════════════════════
// C03 — Player (on ground only)
// Packet ID: 0x03
// Fields: UByte onGround
//
// C04 — Player Position
// Packet ID: 0x04
// Fields: Double x, Double stance(eye height), Double y(feet), Double z,
//         UByte onGround
//   Wire order: x, stance, y, z (NOT x, y, z, stance!)
//
// C05 — Player Look
// Packet ID: 0x05
// Fields: Float yaw, Float pitch, UByte onGround
//
// C06 — Player Position and Look
// Packet ID: 0x06
// Fields: Double x, Double stance, Double y, Double z,
//         Float yaw, Float pitch, UByte onGround
// ═══════════════════════════════════════════════════════════════════════════

struct C03Player {
    static constexpr uint8_t PACKET_ID = 0x03;
    bool onGround = false;
    bool hasPosition = false;  // true for C04/C06
    bool hasRotation = false;  // true for C05/C06
    double x = 0, y = 0, z = 0;
    double stance = 0;         // Eye height position (y + 1.62 standing)
    float yaw = 0, pitch = 0;
};

struct C04PlayerPosition {
    static constexpr uint8_t PACKET_ID = 0x04;
    double x = 0;
    double stance = 0;  // Read BEFORE y on wire
    double y = 0;       // Feet position
    double z = 0;
    bool onGround = false;
};

struct C05PlayerLook {
    static constexpr uint8_t PACKET_ID = 0x05;
    float yaw = 0, pitch = 0;
    bool onGround = false;
};

struct C06PlayerPosLook {
    static constexpr uint8_t PACKET_ID = 0x06;
    double x = 0;
    double stance = 0;
    double y = 0;
    double z = 0;
    float yaw = 0, pitch = 0;
    bool onGround = false;
};

// ═══════════════════════════════════════════════════════════════════════════
// C07 — Player Digging
// Packet ID: 0x07
// Fields: UByte status, Int x, UByte y, Int z, UByte face
//
// Status values:
//   0 = Started digging
//   1 = Cancelled digging
//   2 = Finished digging
//   3 = Drop item stack
//   4 = Drop item
//   5 = Shoot arrow / finish eating
//
// Face values: 0=Down, 1=Up, 2=North, 3=South, 4=West, 5=East
// ═══════════════════════════════════════════════════════════════════════════

struct C07PlayerDigging {
    static constexpr uint8_t PACKET_ID = 0x07;
    uint8_t status = 0;
    int32_t x = 0;
    uint8_t y = 0;
    int32_t z = 0;
    uint8_t face = 0;

    static constexpr uint8_t STATUS_START_DIGGING = 0;
    static constexpr uint8_t STATUS_CANCEL_DIGGING = 1;
    static constexpr uint8_t STATUS_FINISH_DIGGING = 2;
    static constexpr uint8_t STATUS_DROP_ITEM_STACK = 3;
    static constexpr uint8_t STATUS_DROP_ITEM = 4;
    static constexpr uint8_t STATUS_SHOOT_ARROW = 5;
};

// ═══════════════════════════════════════════════════════════════════════════
// C08 — Player Block Placement
// Packet ID: 0x08
// Fields: Int x, UByte y, Int z, UByte direction,
//         Slot heldItem, UByte facingX, UByte facingY, UByte facingZ
//
// Facing values: UByte / 16.0f = 0.0-1.0 sub-block cursor position
// direction: 0-5 face (same as digging face), 255=special (eat/shoot)
// Special case: x=-1,y=255,z=-1 = right-click air (use item, no block)
// ═══════════════════════════════════════════════════════════════════════════

struct C08PlayerBlockPlacement {
    static constexpr uint8_t PACKET_ID = 0x08;
    int32_t x = 0;
    uint8_t y = 0;
    int32_t z = 0;
    uint8_t direction = 0;
    // Held item slot data (parsed separately)
    int16_t heldItemId = -1;
    int8_t heldItemCount = 0;
    int16_t heldItemDamage = 0;
    // Cursor position within block face (0.0-1.0)
    float facingX = 0, facingY = 0, facingZ = 0;

    static constexpr float FACING_SCALE = 1.0f / 16.0f;
    static constexpr uint8_t DIRECTION_SPECIAL = 255;

    bool isUseItem() const {
        return x == -1 && y == 255 && z == -1;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// C09 — Held Item Change
// Packet ID: 0x09
// Fields: Short slotId (0-8)
// ═══════════════════════════════════════════════════════════════════════════

struct C09HeldItemChange {
    static constexpr uint8_t PACKET_ID = 0x09;
    int16_t slotId = 0;  // 0-8 hotbar slot
};

// ═══════════════════════════════════════════════════════════════════════════
// C0A — Animation (swing arm)
// Packet ID: 0x0A
// Fields: Int entityId, Byte animation
//   animation: 1=Swing arm (only valid client→server value)
// ═══════════════════════════════════════════════════════════════════════════

struct C0AAnimation {
    static constexpr uint8_t PACKET_ID = 0x0A;
    int32_t entityId = 0;
    int8_t animation = 1;
};

// ═══════════════════════════════════════════════════════════════════════════
// C0B — Entity Action
// Packet ID: 0x0B
// Fields: Int entityId, Byte actionId, Int jumpBoost
//
// Actions:
//   1=Crouch, 2=Uncrouch, 3=Leave bed,
//   4=Start sprinting, 5=Stop sprinting,
//   6=Horse jump (jumpBoost = jump power 0-100)
// ═══════════════════════════════════════════════════════════════════════════

struct C0BEntityAction {
    static constexpr uint8_t PACKET_ID = 0x0B;
    int32_t entityId = 0;
    int8_t actionId = 0;
    int32_t jumpBoost = 0;

    static constexpr int8_t ACTION_CROUCH = 1;
    static constexpr int8_t ACTION_UNCROUCH = 2;
    static constexpr int8_t ACTION_LEAVE_BED = 3;
    static constexpr int8_t ACTION_START_SPRINTING = 4;
    static constexpr int8_t ACTION_STOP_SPRINTING = 5;
    static constexpr int8_t ACTION_HORSE_JUMP = 6;
};

// ═══════════════════════════════════════════════════════════════════════════
// C0C — Steer Vehicle (horse/pig/boat input)
// Packet ID: 0x0C
// Fields: Float sideways, Float forward, Bool jump, Bool unmount
// ═══════════════════════════════════════════════════════════════════════════

struct C0CSteerVehicle {
    static constexpr uint8_t PACKET_ID = 0x0C;
    float sideways = 0;
    float forward = 0;
    bool jump = false;
    bool unmount = false;
};

// ═══════════════════════════════════════════════════════════════════════════
// C0D — Close Window
// Packet ID: 0x0D
// Fields: Byte windowId
// ═══════════════════════════════════════════════════════════════════════════

struct C0DCloseWindow {
    static constexpr uint8_t PACKET_ID = 0x0D;
    int8_t windowId = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// C0E — Click Window (inventory click)
// Packet ID: 0x0E
// Fields: Byte windowId, Short slot, Byte button, Short actionNumber,
//         Byte mode, Slot clickedItem
//
// Mode+Button combinations:
//   mode 0: button 0=left click, 1=right click
//   mode 1: button 0=shift+left, 1=shift+right
//   mode 2: button 0-8=number key (hotbar slot)
//   mode 3: button 2=creative middle click
//   mode 4: button 0=drop, 1=ctrl+drop (stack)
//   mode 5: button 0=left drag start, 4=right drag start,
//           1=left drag add, 5=right drag add,
//           2=left drag end, 6=right drag end
//   mode 6: button 0=double click to fill stack
// ═══════════════════════════════════════════════════════════════════════════

struct C0EClickWindow {
    static constexpr uint8_t PACKET_ID = 0x0E;
    int8_t windowId = 0;
    int16_t slot = 0;
    int8_t button = 0;
    int16_t actionNumber = 0;
    int8_t mode = 0;
    // Clicked item slot data parsed separately
    int16_t itemId = -1;
    int8_t itemCount = 0;
    int16_t itemDamage = 0;

    static constexpr int8_t MODE_NORMAL = 0;
    static constexpr int8_t MODE_SHIFT = 1;
    static constexpr int8_t MODE_NUMBER_KEY = 2;
    static constexpr int8_t MODE_CREATIVE_MIDDLE = 3;
    static constexpr int8_t MODE_DROP = 4;
    static constexpr int8_t MODE_DRAG = 5;
    static constexpr int8_t MODE_DOUBLE_CLICK = 6;
};

// ═══════════════════════════════════════════════════════════════════════════
// C0F — Confirm Transaction
// Packet ID: 0x0F
// Fields: Byte windowId, Short actionNumber, Bool accepted
// ═══════════════════════════════════════════════════════════════════════════

struct C0FConfirmTransaction {
    static constexpr uint8_t PACKET_ID = 0x0F;
    int8_t windowId = 0;
    int16_t actionNumber = 0;
    bool accepted = false;
};

// ═══════════════════════════════════════════════════════════════════════════
// C10 — Creative Inventory Action
// Packet ID: 0x10
// Fields: Short slot, Slot clickedItem
// ═══════════════════════════════════════════════════════════════════════════

struct C10CreativeInventoryAction {
    static constexpr uint8_t PACKET_ID = 0x10;
    int16_t slot = 0;
    // Item data parsed separately
    int16_t itemId = -1;
    int8_t itemCount = 0;
    int16_t itemDamage = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// C11 — Enchant Item
// Packet ID: 0x11
// Fields: Byte windowId, Byte enchantment (0, 1, or 2 = slot position)
// ═══════════════════════════════════════════════════════════════════════════

struct C11EnchantItem {
    static constexpr uint8_t PACKET_ID = 0x11;
    int8_t windowId = 0;
    int8_t enchantment = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// C12 — Update Sign
// Packet ID: 0x12
// Fields: Int x, Short y, Int z, String line1-4
// ═══════════════════════════════════════════════════════════════════════════

struct C12UpdateSign {
    static constexpr uint8_t PACKET_ID = 0x12;
    int32_t x = 0;
    int16_t y = 0;
    int32_t z = 0;
    std::string line1, line2, line3, line4;
    static constexpr int32_t MAX_LINE_LENGTH = 15;
};

// ═══════════════════════════════════════════════════════════════════════════
// C13 — Player Abilities
// Packet ID: 0x13
// Fields: Byte flags, Float flySpeed, Float walkSpeed
//   Client only sends: flying flag (0x02)
// ═══════════════════════════════════════════════════════════════════════════

struct C13PlayerAbilities {
    static constexpr uint8_t PACKET_ID = 0x13;
    uint8_t flags = 0;
    float flySpeed = 0.05f;
    float walkSpeed = 0.1f;
};

// ═══════════════════════════════════════════════════════════════════════════
// C14 — Tab Complete
// Packet ID: 0x14
// Fields: String text (partial command/chat)
// ═══════════════════════════════════════════════════════════════════════════

struct C14TabComplete {
    static constexpr uint8_t PACKET_ID = 0x14;
    std::string text;
};

// ═══════════════════════════════════════════════════════════════════════════
// C15 — Client Settings
// Packet ID: 0x15
// Fields: String locale, Byte viewDistance, Byte chatFlags,
//         Bool chatColors, Byte difficulty, Bool showCape
//
// chatFlags: 0=enabled, 1=commands only, 2=hidden
// ═══════════════════════════════════════════════════════════════════════════

struct C15ClientSettings {
    static constexpr uint8_t PACKET_ID = 0x15;
    std::string locale;
    int8_t viewDistance = 0;  // Client render distance (chunks)
    int8_t chatFlags = 0;
    bool chatColors = true;
    int8_t difficulty = 0;  // Unused by server in 1.7.10
    bool showCape = true;
};

// ═══════════════════════════════════════════════════════════════════════════
// C16 — Client Status
// Packet ID: 0x16
// Fields: Byte actionId
//   0=Perform respawn, 1=Request stats, 2=Open inventory achievement
// ═══════════════════════════════════════════════════════════════════════════

struct C16ClientStatus {
    static constexpr uint8_t PACKET_ID = 0x16;
    int8_t actionId = 0;

    static constexpr int8_t ACTION_PERFORM_RESPAWN = 0;
    static constexpr int8_t ACTION_REQUEST_STATS = 1;
    static constexpr int8_t ACTION_OPEN_INVENTORY = 2;
};

// ═══════════════════════════════════════════════════════════════════════════
// C17 — Plugin Message
// Packet ID: 0x17
// Fields: String channel, Short length, [Byte]* data
// ═══════════════════════════════════════════════════════════════════════════

struct C17PluginMessage {
    static constexpr uint8_t PACKET_ID = 0x17;
    std::string channel;
    std::vector<uint8_t> data;
};

} // namespace mccpp
