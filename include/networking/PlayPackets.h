/**
 * PlayPackets.h — Packet ID definitions and structures for Protocol v5 (1.7.10).
 *
 * Reference: https://web.archive.org/web/20241129034727/https://wiki.vg/index.php?title=Protocol&oldid=7368
 *
 * Packet IDs are organized by direction (Clientbound / Serverbound) and state.
 * Only Play-state packets are defined here; Handshake/Status/Login packets are
 * handled inline in their respective handlers.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ─── Clientbound (Server → Client) Play Packets ────────────────────────────

namespace ClientboundPacket {
    constexpr int32_t KeepAlive           = 0x00;
    constexpr int32_t JoinGame            = 0x01;
    constexpr int32_t ChatMessage         = 0x02;
    constexpr int32_t TimeUpdate          = 0x03;
    constexpr int32_t EntityEquipment     = 0x04;
    constexpr int32_t SpawnPosition       = 0x05;
    constexpr int32_t UpdateHealth        = 0x06;
    constexpr int32_t Respawn             = 0x07;
    constexpr int32_t PlayerPosAndLook    = 0x08;
    constexpr int32_t HeldItemChange      = 0x09;
    constexpr int32_t UseBed              = 0x0A;
    constexpr int32_t Animation           = 0x0B;
    constexpr int32_t SpawnPlayer         = 0x0C;
    constexpr int32_t CollectItem         = 0x0D;
    constexpr int32_t SpawnObject         = 0x0E;
    constexpr int32_t SpawnMob            = 0x0F;
    constexpr int32_t SpawnPainting       = 0x10;
    constexpr int32_t SpawnExpOrb         = 0x11;
    constexpr int32_t EntityVelocity      = 0x12;
    constexpr int32_t DestroyEntities     = 0x13;
    constexpr int32_t Entity              = 0x14;
    constexpr int32_t EntityRelMove       = 0x15;
    constexpr int32_t EntityLook          = 0x16;
    constexpr int32_t EntityLookAndRelMove= 0x17;
    constexpr int32_t EntityTeleport      = 0x18;
    constexpr int32_t EntityHeadLook      = 0x19;
    constexpr int32_t EntityStatus        = 0x1A;
    constexpr int32_t AttachEntity        = 0x1B;
    constexpr int32_t EntityMetadata      = 0x1C;
    constexpr int32_t EntityEffect        = 0x1D;
    constexpr int32_t RemoveEntityEffect  = 0x1E;
    constexpr int32_t SetExperience       = 0x1F;
    constexpr int32_t EntityProperties    = 0x20;
    constexpr int32_t ChunkData           = 0x21;
    constexpr int32_t MultiBlockChange    = 0x22;
    constexpr int32_t BlockChange         = 0x23;
    constexpr int32_t BlockAction         = 0x24;
    constexpr int32_t BlockBreakAnim      = 0x25;
    constexpr int32_t MapChunkBulk        = 0x26;
    constexpr int32_t Explosion           = 0x27;
    constexpr int32_t Effect              = 0x28;
    constexpr int32_t SoundEffect         = 0x29;
    constexpr int32_t Particle            = 0x2A;
    constexpr int32_t ChangeGameState     = 0x2B;
    constexpr int32_t SpawnGlobalEntity   = 0x2C;
    constexpr int32_t OpenWindow          = 0x2D;
    constexpr int32_t CloseWindow         = 0x2E;
    constexpr int32_t SetSlot             = 0x2F;
    constexpr int32_t WindowItems         = 0x30;
    constexpr int32_t WindowProperty      = 0x31;
    constexpr int32_t ConfirmTransaction  = 0x32;
    constexpr int32_t UpdateSign          = 0x33;
    constexpr int32_t Maps                = 0x34;
    constexpr int32_t UpdateBlockEntity   = 0x35;
    constexpr int32_t SignEditorOpen       = 0x36;
    constexpr int32_t Statistics          = 0x37;
    constexpr int32_t PlayerListItem      = 0x38;
    constexpr int32_t PlayerAbilities     = 0x39;
    constexpr int32_t TabComplete         = 0x3A;
    constexpr int32_t ScoreboardObjective = 0x3B;
    constexpr int32_t UpdateScore         = 0x3C;
    constexpr int32_t DisplayScoreboard   = 0x3D;
    constexpr int32_t Teams               = 0x3E;
    constexpr int32_t PluginMessage       = 0x3F;
    constexpr int32_t Disconnect          = 0x40;
} // namespace ClientboundPacket

// ─── Serverbound (Client → Server) Play Packets ────────────────────────────

namespace ServerboundPacket {
    constexpr int32_t KeepAlive           = 0x00;
    constexpr int32_t ChatMessage         = 0x01;
    constexpr int32_t UseEntity           = 0x02;
    constexpr int32_t Player              = 0x03;
    constexpr int32_t PlayerPosition      = 0x04;
    constexpr int32_t PlayerLook          = 0x05;
    constexpr int32_t PlayerPosAndLook    = 0x06;
    constexpr int32_t PlayerDigging       = 0x07;
    constexpr int32_t PlayerBlockPlace    = 0x08;
    constexpr int32_t HeldItemChange      = 0x09;
    constexpr int32_t Animation           = 0x0A;
    constexpr int32_t EntityAction        = 0x0B;
    constexpr int32_t SteerVehicle        = 0x0C;
    constexpr int32_t CloseWindow         = 0x0D;
    constexpr int32_t ClickWindow         = 0x0E;
    constexpr int32_t ConfirmTransaction  = 0x0F;
    constexpr int32_t CreativeInventory   = 0x10;
    constexpr int32_t EnchantItem         = 0x11;
    constexpr int32_t UpdateSign          = 0x12;
    constexpr int32_t PlayerAbilities     = 0x13;
    constexpr int32_t TabComplete         = 0x14;
    constexpr int32_t ClientSettings      = 0x15;
    constexpr int32_t ClientStatus        = 0x16;
    constexpr int32_t PluginMessage       = 0x17;
} // namespace ServerboundPacket

// ─── Handshake / Status / Login packet IDs (for completeness) ───────────────

namespace HandshakePacket {
    constexpr int32_t Handshake = 0x00;
}

namespace StatusPacket {
    constexpr int32_t Request   = 0x00;  // C→S
    constexpr int32_t Response  = 0x00;  // S→C
    constexpr int32_t Ping      = 0x01;  // C→S
    constexpr int32_t Pong      = 0x01;  // S→C
}

namespace LoginPacket {
    constexpr int32_t Disconnect         = 0x00;  // S→C
    constexpr int32_t EncryptionRequest  = 0x01;  // S→C
    constexpr int32_t LoginSuccess       = 0x02;  // S→C
    constexpr int32_t LoginStart         = 0x00;  // C→S
    constexpr int32_t EncryptionResponse = 0x01;  // C→S
}

} // namespace mccpp
