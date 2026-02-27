#pragma once
// PacketHandler — dispatches incoming packets by connection state.
// Implements Handshake, Status, Login, and Play states for protocol 5 (1.7.10).
// Now with Player entity tracking, World management, chat broadcast,
// and entity spawn/despawn for multiplayer.

#include <cstdint>
#include <string>
#include <iostream>
#include <chrono>
#include <atomic>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "networking/Connection.h"
#include "networking/PacketBuffer.h"
#include "networking/ConnectionState.h"
#include "networking/PlayPackets.h"
#include "entity/Player.h"
#include "world/World.h"
#include "command/CommandHandler.h"
#include "persistence/PlayerDataIO.h"
#include <functional>

namespace mc {

class PacketHandler {
public:
    // Server description shown in the server list
    std::string motd = "A MineCPPaft Server";
    int maxPlayers = 20;

    // World
    World world;

    void handle(Connection& conn, PacketBuffer& buf) {
        int32_t packetId = buf.readVarInt();

        switch (conn.state()) {
            case ConnectionState::Handshaking:
                handleHandshake(conn, packetId, buf);
                break;
            case ConnectionState::Status:
                handleStatus(conn, packetId, buf);
                break;
            case ConnectionState::Login:
                handleLogin(conn, packetId, buf);
                break;
            case ConnectionState::Play:
                handlePlay(conn, packetId, buf);
                break;
        }
    }

    // Called each tick for play-state connections
    void tick(Connection& conn) {
        if (conn.state() != ConnectionState::Play) return;

        auto now = std::chrono::steady_clock::now();
        auto it = players_.find(conn.fd());
        if (it == players_.end()) return;
        auto& player = it->second;

        // Send Keep Alive every 20 seconds
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - player.lastKeepAlive);
        if (elapsed.count() >= 20) {
            KeepAlivePacket ka;
            ka.keepAliveId = static_cast<int32_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()).count() & 0x7FFFFFFF
            );
            conn.sendPacket(ka.serialize());
            player.lastKeepAlive = now;
        }
    }

    // Called each server tick for world updates
    void worldTick(std::unordered_map<int, Connection>& connections) {
        world.tick();

        // Send time update every 20 ticks (1 second)
        if (world.worldTime % 20 == 0) {
            TimeUpdatePacket time;
            time.worldAge = world.worldTime;
            time.timeOfDay = world.dayTime;

            for (auto& [fd, conn] : connections) {
                if (conn.state() == ConnectionState::Play) {
                    conn.sendPacket(time.serialize());
                }
            }
        }
    }

    void onDisconnect(int fd, std::unordered_map<int, Connection>& connections) {
        auto it = players_.find(fd);
        if (it == players_.end()) return;

        auto& player = it->second;
        std::cout << "[PLAY] " << player.name << " left the game\n";

        // Save player data before removing
        playerDataIO_.savePlayer(player);

        // Notify other players to destroy this entity + remove from tab
        DestroyEntitiesPacket destroy;
        destroy.entityIds.push_back(player.entityId);

        auto msg = ChatMessagePacket::makeText(
            "\u00a7e" + player.name + " left the game");

        PlayerListItemPacket removeTab;
        removeTab.playerName = player.name;
        removeTab.online = false;
        removeTab.ping = 0;

        for (auto& [otherFd, otherConn] : connections) {
            if (otherFd != fd && otherConn.state() == ConnectionState::Play) {
                otherConn.sendPacket(destroy.serialize());
                otherConn.sendPacket(msg.serialize());
                otherConn.sendPacket(removeTab.serialize());
            }
        }

        players_.erase(it);
    }

    int onlineCount() const { return static_cast<int>(players_.size()); }

    const std::unordered_map<int, Player>& players() const { return players_; }

    // Set connections reference for movement/animation broadcast
    void setConnections(std::unordered_map<int, Connection>* conns) { connections_ = conns; }

private:
    std::unordered_map<int, Player> players_;
    std::atomic<int32_t> nextEntityId_{1};
    std::unordered_map<int, Connection>* connections_ = nullptr;
    CommandHandler commandHandler_;
    PlayerDataIO playerDataIO_;

    // Generate offline UUID from player name — simplified UUID v3 approach
    // Vanilla uses UUID.nameUUIDFromBytes("OfflinePlayer:" + name)
    static std::string offlineUuid(const std::string& name) {
        std::string input = "OfflinePlayer:" + name;
        // Simple hash-based UUID generation (not cryptographic, but deterministic)
        std::hash<std::string> hasher;
        size_t h1 = hasher(input);
        size_t h2 = hasher(input + "salt");

        char buf[37];
        snprintf(buf, sizeof(buf),
                 "%08x-%04x-%04x-%04x-%012llx",
                 static_cast<uint32_t>(h1 >> 32),
                 static_cast<uint16_t>(h1 >> 16),
                 static_cast<uint16_t>((h1 & 0xFFFF) | 0x3000), // version 3
                 static_cast<uint16_t>((h2 >> 48) | 0x8000),     // variant 1
                 static_cast<unsigned long long>(h2 & 0xFFFFFFFFFFFFULL));
        return std::string(buf);
    }

    // === Handshake (state -1) ===
    void handleHandshake(Connection& conn, int32_t packetId, PacketBuffer& buf) {
        if (packetId != 0x00) {
            conn.close();
            return;
        }

        int32_t protocolVersion = buf.readVarInt();
        std::string serverAddress = buf.readString(255);
        uint16_t serverPort = buf.readUnsignedShort();
        int32_t nextState = buf.readVarInt();

        (void)protocolVersion;
        (void)serverAddress;
        (void)serverPort;

        if (nextState == 1) {
            conn.setState(ConnectionState::Status);
        } else if (nextState == 2) {
            conn.setState(ConnectionState::Login);
        } else {
            conn.close();
        }
    }

    // === Status (state 1) ===
    void handleStatus(Connection& conn, int32_t packetId, PacketBuffer& buf) {
        if (packetId == 0x00) {
            // S→C Status Response
            // kf.java: protocol version 5, name "1.7.10"
            std::string json = R"({)"
                R"("version":{"name":"1.7.10","protocol":5},)"
                R"("players":{"max":)" + std::to_string(maxPlayers) +
                R"(,"online":)" + std::to_string(onlineCount()) +
                R"(,"sample":[]},)"
                R"("description":{"text":")" + motd + R"("})"
                R"(})";

            PacketBuffer resp;
            resp.writeVarInt(0x00);
            resp.writeString(json);
            conn.sendPacket(resp);
        }
        else if (packetId == 0x01) {
            int64_t payload = buf.readLong();
            PacketBuffer resp;
            resp.writeVarInt(0x01);
            resp.writeLong(payload);
            conn.sendPacket(resp);
        }
    }

    // === Login (state 2) ===
    void handleLogin(Connection& conn, int32_t packetId, PacketBuffer& buf) {
        if (packetId == 0x00) {
            std::string playerName = buf.readString(16);
            std::cout << "[PKT] Login Start: " << playerName << "\n";

            // Offline mode: generate name-based UUID (matches vanilla OfflinePlayer)
            std::string uuid = offlineUuid(playerName);

            PacketBuffer resp;
            resp.writeVarInt(0x02);
            resp.writeString(uuid);
            resp.writeString(playerName);
            conn.sendPacket(resp);

            conn.setState(ConnectionState::Play);

            sendJoinSequence(conn, playerName, uuid);
        }
    }

    // === Play join sequence ===
    void sendJoinSequence(Connection& conn, const std::string& playerName,
                          const std::string& uuid) {
        int32_t eid = nextEntityId_++;

        Player player;
        player.entityId = eid;
        player.name = playerName;
        player.uuid = uuid;
        player.connectionFd = conn.fd();
        player.posX = 0.5;
        player.posY = 4.0;  // Spawn on top of flat world (bedrock+2dirt+grass = y=4)
        player.posZ = 0.5;
        player.lastKeepAlive = std::chrono::steady_clock::now();

        // Try to load saved data (overrides defaults if file exists)
        playerDataIO_.loadPlayer(player);

        // 1. Join Game — hd.java
        JoinGamePacket joinGame;
        joinGame.entityId = eid;
        joinGame.gameMode = static_cast<uint8_t>(player.gameMode);
        joinGame.hardcore = false;
        joinGame.dimension = player.dimension;
        joinGame.difficulty = 1; // Easy
        joinGame.maxPlayers = static_cast<uint8_t>(maxPlayers);
        joinGame.levelType = "flat";
        conn.sendPacket(joinGame.serialize());

        // 2. MC|Brand
        auto brand = PluginMessagePacket::makeBrand("MineCPPaft");
        conn.sendPacket(brand.serialize());

        // 3. Spawn Position
        SpawnPositionPacket spawnPos;
        spawnPos.x = 0;
        spawnPos.y = 4;
        spawnPos.z = 0;
        conn.sendPacket(spawnPos.serialize());

        // 4. Player Abilities
        PlayerAbilitiesPacket abilities;
        abilities.invulnerable = player.invulnerable;
        abilities.flying = player.flying;
        abilities.allowFlying = player.allowFlying;
        abilities.creativeMode = (player.gameMode == GameMode::Creative);
        abilities.flySpeed = player.flySpeed;
        abilities.walkSpeed = player.walkSpeed;
        conn.sendPacket(abilities.serialize());

        // 5. Time
        TimeUpdatePacket time;
        time.worldAge = world.worldTime;
        time.timeOfDay = world.dayTime;
        conn.sendPacket(time.serialize());

        // 6. Chunks around spawn (7x7)
        for (int cx = -3; cx <= 3; ++cx) {
            for (int cz = -3; cz <= 3; ++cz) {
                auto& chunk = world.getChunk(cx, cz);
                auto pkt = ChunkDataPacket::fromChunkColumn(chunk, true);
                conn.sendPacket(pkt.serialize());
            }
        }

        // 7. Player Position And Look
        PlayerPositionAndLookPacket posLook;
        posLook.x = player.posX;
        posLook.y = player.posY;
        posLook.z = player.posZ;
        posLook.yaw = player.yaw;
        posLook.pitch = player.pitch;
        posLook.onGround = false;
        conn.sendPacket(posLook.serialize());

        // Store player BEFORE broadcasting (so we have the entity ID)
        players_[conn.fd()] = player;

        // 8. Send player inventory contents
        {
            PacketBuffer invBuf;
            invBuf.writeVarInt(0x30); // Window Items
            invBuf.writeByte(0);      // Window ID 0 = player inventory
            invBuf.writeShort(45);    // 45 slots
            player.inventory.writeAllSlots(invBuf);
            conn.sendPacket(invBuf);
        }

        // 9. Update Health — ib.java
        UpdateHealthPacket hp;
        hp.health = player.health;
        hp.food = player.foodLevel;
        hp.saturation = player.saturation;
        conn.sendPacket(hp.serialize());

        // 10. Set Experience — ia.java
        SetExperiencePacket xp;
        xp.barProgress = player.experienceProgress;
        xp.level = player.experienceLevel;
        xp.totalExp = player.totalExperience;
        conn.sendPacket(xp.serialize());

        // 11. Player List — add self to tab list
        PlayerListItemPacket selfTab;
        selfTab.playerName = playerName;
        selfTab.online = true;
        selfTab.ping = 0;
        conn.sendPacket(selfTab.serialize());

        // 12. Add all existing players to new player's tab list
        for (auto& [otherFd, otherPlayer] : players_) {
            if (otherFd == conn.fd()) continue;
            PlayerListItemPacket otherTab;
            otherTab.playerName = otherPlayer.name;
            otherTab.online = true;
            otherTab.ping = 50;
            conn.sendPacket(otherTab.serialize());
        }

        std::cout << "[PLAY] " << playerName << " (eid=" << eid
                  << ") joined the game\n";
    }

    // Helper: broadcast entity movement to all other play-state connections
    void broadcastMovement(const Player& player) {
        if (!connections_) return;

        auto tp = EntityTeleportPacket::fromPlayer(
            player.entityId, player.posX, player.posY, player.posZ,
            player.yaw, player.pitch);

        EntityHeadLookPacket headLook;
        headLook.entityId = player.entityId;
        headLook.headYaw = EntityLookPacket::toAngle(player.yaw);

        for (auto& [fd, conn] : *connections_) {
            if (fd != player.connectionFd && conn.state() == ConnectionState::Play) {
                conn.sendPacket(tp.serialize());
                conn.sendPacket(headLook.serialize());
            }
        }
    }

    // === Play (state 0) ===
    void handlePlay(Connection& conn, int32_t packetId, PacketBuffer& buf) {
        auto it = players_.find(conn.fd());
        Player* player = (it != players_.end()) ? &it->second : nullptr;

        switch (packetId) {
            case 0x00: {
                // C→S Keep Alive
                buf.readInt();
                break;
            }
            case 0x01: {
                // C→S Chat Message
                std::string message = buf.readString(100);
                if (player) {
                    if (!message.empty() && message[0] == '/' && connections_) {
                        // Command
                        std::cout << "[CMD] " << player->name << ": " << message << "\n";
                        commandHandler_.handleCommand(message, *player, conn,
                            players_, *connections_, world);
                    } else {
                        // Normal chat
                        std::cout << "[CHAT] <" << player->name << "> " << message << "\n";
                        if (connections_) {
                            broadcastChat(player->name, message, *connections_);
                        }
                    }
                }
                break;
            }
            case 0x03: {
                // C→S Player (ground only)
                if (player) player->onGround = buf.readBoolean();
                break;
            }
            case 0x04: {
                // C→S Player Position
                if (player) {
                    player->posX = buf.readDouble();
                    player->posY = buf.readDouble();
                    buf.readDouble(); // headY
                    player->posZ = buf.readDouble();
                    player->onGround = buf.readBoolean();
                    broadcastMovement(*player);
                }
                break;
            }
            case 0x05: {
                // C→S Player Look
                if (player) {
                    player->yaw = buf.readFloat();
                    player->pitch = buf.readFloat();
                    player->onGround = buf.readBoolean();
                    broadcastMovement(*player);
                }
                break;
            }
            case 0x06: {
                // C→S Player Position And Look
                if (player) {
                    player->posX = buf.readDouble();
                    player->posY = buf.readDouble();
                    buf.readDouble(); // headY
                    player->posZ = buf.readDouble();
                    player->yaw = buf.readFloat();
                    player->pitch = buf.readFloat();
                    player->onGround = buf.readBoolean();
                    broadcastMovement(*player);
                }
                break;
            }
            case 0x07: {
                // C→S Player Digging
                int8_t status = static_cast<int8_t>(buf.readByte());
                int32_t x = buf.readInt();
                uint8_t y = buf.readByte();
                int32_t z = buf.readInt();
                uint8_t face = buf.readByte();
                (void)face;

                // Status 0 = started digging (instant break in creative)
                // Status 2 = finished digging (block breaks)
                if ((status == 2 || (status == 0 && player &&
                     player->gameMode == GameMode::Creative)) && player) {
                    uint16_t oldBlock = world.getBlock(x, y, z);
                    world.setBlock(x, y, z, BlockID::AIR);

                    // Broadcast block change to all players
                    if (connections_) {
                        BlockChangePacket bc;
                        bc.x = x; bc.y = static_cast<int32_t>(y); bc.z = z;
                        bc.blockId = 0; bc.metadata = 0;
                        for (auto& [fd, c] : *connections_) {
                            if (c.state() == ConnectionState::Play) {
                                c.sendPacket(bc.serialize());
                            }
                        }

                        // Play break sound
                        auto sound = NamedSoundEffectPacket::at(
                            "dig.stone", x + 0.5, y + 0.5, z + 0.5);
                        for (auto& [fd, c] : *connections_) {
                            if (c.state() == ConnectionState::Play) {
                                c.sendPacket(sound.serialize());
                            }
                        }
                    }
                }
                break;
            }
            case 0x08: {
                // C→S Block Placement
                int32_t x = buf.readInt();
                uint8_t y = buf.readByte();
                int32_t z = buf.readInt();
                uint8_t face = buf.readByte();
                auto heldItem = ItemStack::readFromPacket(buf);
                buf.readByte(); // cursorX
                buf.readByte(); // cursorY
                buf.readByte(); // cursorZ

                // Adjust position based on face
                if (player && !heldItem.isEmpty() && x != -1) {
                    int32_t bx = x, bz = z;
                    int32_t by = static_cast<int32_t>(y);
                    switch (face) {
                        case 0: --by; break; // Bottom
                        case 1: ++by; break; // Top
                        case 2: --bz; break; // North
                        case 3: ++bz; break; // South
                        case 4: --bx; break; // West
                        case 5: ++bx; break; // East
                        default: break;
                    }
                    // Place block if the held item is a block (ID < 256)
                    if (heldItem.itemId > 0 && heldItem.itemId < 256) {
                        world.setBlock(bx, by, bz, static_cast<uint16_t>(heldItem.itemId));

                        // Broadcast block change
                        if (connections_) {
                            BlockChangePacket bc;
                            bc.x = bx; bc.y = by; bc.z = bz;
                            bc.blockId = heldItem.itemId; bc.metadata = 0;
                            for (auto& [fd, c] : *connections_) {
                                if (c.state() == ConnectionState::Play) {
                                    c.sendPacket(bc.serialize());
                                }
                            }

                            // Play place sound
                            auto sound = NamedSoundEffectPacket::at(
                                "dig.stone", bx + 0.5, by + 0.5, bz + 0.5);
                            for (auto& [fd, c] : *connections_) {
                                if (c.state() == ConnectionState::Play) {
                                    c.sendPacket(sound.serialize());
                                }
                            }
                        }
                    }
                }
                break;
            }
            case 0x09: {
                // C→S Held Item Change
                int16_t slot = buf.readShort();
                if (player && slot >= 0 && slot < 9) {
                    player->inventory.currentSlot = static_cast<int8_t>(slot);
                }
                break;
            }
            case 0x0A: {
                // C→S Animation — broadcast to other players
                buf.readVarInt(); // entityId (unused in C→S)
                buf.readByte();   // animation
                if (player && connections_) {
                    AnimationPacket anim;
                    anim.entityId = player->entityId;
                    anim.animation = 0; // swing arm
                    for (auto& [fd, c] : *connections_) {
                        if (fd != conn.fd() && c.state() == ConnectionState::Play) {
                            c.sendPacket(anim.serialize());
                        }
                    }
                }
                break;
            }
            case 0x10: {
                // C→S Creative Inventory Action
                int16_t slotNum = buf.readShort();
                auto clickedItem = ItemStack::readFromPacket(buf);
                if (player) {
                    if (clickedItem.isEmpty()) {
                        player->inventory.setWindowSlot(slotNum, std::nullopt);
                    } else {
                        player->inventory.setWindowSlot(slotNum, clickedItem);
                    }
                }
                break;
            }
            case 0x15: {
                // C→S Client Settings
                buf.readString(16); // locale
                buf.readByte();     // view distance
                buf.readByte();     // chat mode
                buf.readBoolean();  // chat colors
                buf.readByte();     // difficulty
                buf.readBoolean();  // show cape
                break;
            }
            case 0x16: {
                // C→S Client Status
                buf.readVarInt();
                break;
            }
            case 0x17: {
                // C→S Plugin Message
                std::string channel = buf.readString(20);
                int16_t len = buf.readShort();
                if (len > 0) buf.readBytes(static_cast<size_t>(len));
                break;
            }
            case 0x14: {
                // C→S Tab Complete
                std::string text = buf.readString(256);
                if (player && !text.empty() && text[0] == '/') {
                    auto completions = commandHandler_.getCompletions(text, players_);
                    TabCompletePacket tc;
                    tc.matches = completions;
                    conn.sendPacket(tc.serialize());
                }
                break;
            }
            default:
                break;
        }
    }

public:
    // Broadcast a chat message to all play-state connections
    void broadcastChat(const std::string& playerName, const std::string& message,
                       std::unordered_map<int, Connection>& connections) {
        auto pkt = ChatMessagePacket::makeChat(playerName, message);
        for (auto& [fd, conn] : connections) {
            if (conn.state() == ConnectionState::Play) {
                conn.sendPacket(pkt.serialize());
            }
        }
    }

    // Broadcast a spawn packet for a new player to all other players
    void broadcastSpawn(const Player& player,
                        std::unordered_map<int, Connection>& connections) {
        SpawnPlayerPacket spawn;
        spawn.entityId = player.entityId;
        spawn.uuid = player.uuid;
        spawn.name = player.name;
        spawn.x = player.posX;
        spawn.y = player.posY;
        spawn.z = player.posZ;
        spawn.yaw = player.yaw;
        spawn.pitch = player.pitch;
        spawn.currentItem = 0;

        auto joinMsg = ChatMessagePacket::makeText(
            "\u00a7e" + player.name + " joined the game");

        // Add new player to other players' tab lists
        PlayerListItemPacket newTab;
        newTab.playerName = player.name;
        newTab.online = true;
        newTab.ping = 0;

        for (auto& [fd, conn] : connections) {
            if (fd != player.connectionFd && conn.state() == ConnectionState::Play) {
                conn.sendPacket(spawn.serialize());
                conn.sendPacket(joinMsg.serialize());
                conn.sendPacket(newTab.serialize());
            }
        }

        // Send existing players to the new player
        auto it = connections.find(player.connectionFd);
        if (it != connections.end()) {
            for (auto& [otherFd, otherPlayer] : players_) {
                if (otherFd == player.connectionFd) continue;
                SpawnPlayerPacket otherSpawn;
                otherSpawn.entityId = otherPlayer.entityId;
                otherSpawn.uuid = otherPlayer.uuid;
                otherSpawn.name = otherPlayer.name;
                otherSpawn.x = otherPlayer.posX;
                otherSpawn.y = otherPlayer.posY;
                otherSpawn.z = otherPlayer.posZ;
                otherSpawn.yaw = otherPlayer.yaw;
                otherSpawn.pitch = otherPlayer.pitch;
                otherSpawn.currentItem = 0;
                it->second.sendPacket(otherSpawn.serialize());
            }
        }
    }
};

} // namespace mc
