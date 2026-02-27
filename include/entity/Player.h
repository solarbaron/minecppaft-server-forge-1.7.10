#pragma once
// Player entity — ported from mw.java (EntityPlayerMP) and yz.java (EntityPlayer)
// Fields:
//   posX/Y/Z = s/t/u, yaw/pitch = y/z, eyeHeight = 1.62f
//   health = bp, food = bG, gameMode = c.b()
//   inventory = bm (PlayerInventory)
//
// For now, a simplified version tracking position, rotation, health, food,
// game mode, and entity ID. Full inventory and abilities will come later.

#include <cstdint>
#include <string>
#include <chrono>
#include <array>
#include "nbt/NBT.h"

namespace mc {

// Game modes — ahk.java
enum class GameMode : uint8_t {
    Survival  = 0,
    Creative  = 1,
    Adventure = 2,
};

// Player entity data
struct Player {
    // Entity base (sa.java fields)
    int32_t entityId = 0;
    std::string name;
    std::string uuid = "00000000-0000-0000-0000-000000000000";

    // Position (sa.java: s, t, u)
    double posX = 0.5;
    double posY = 64.0;
    double posZ = 0.5;

    // Rotation (sa.java: y, z)
    float yaw = 0.0f;
    float pitch = 0.0f;

    // Physical state
    bool onGround = false;

    // Player-specific (yz.java, mw.java)
    static constexpr float EYE_HEIGHT = 1.62f; // mw.g() returns 1.62f

    // Health & food (sv.java)
    float health = 20.0f;     // Max 20.0
    int32_t foodLevel = 20;   // Max 20
    float saturation = 5.0f;  // Starting saturation
    float exhaustion = 0.0f;

    // Experience (yz.java)
    int32_t experienceLevel = 0;
    float experienceProgress = 0.0f;  // 0.0 to 1.0
    int32_t totalExperience = 0;

    // Game mode
    GameMode gameMode = GameMode::Survival;

    // Abilities (mw.java playerAbilities)
    bool invulnerable = false;
    bool flying = false;
    bool allowFlying = false;
    float flySpeed = 0.05f;
    float walkSpeed = 0.1f;

    // Connection state
    int connectionFd = -1;  // Socket fd for this player

    // Keep alive tracking
    std::chrono::steady_clock::time_point lastKeepAlive = std::chrono::steady_clock::now();
    int32_t lastKeepAliveId = 0;

    // Dimension
    int8_t dimension = 0;  // 0=overworld, -1=nether, 1=end

    // Inventory — 36 main + 4 armor + 1 offhand (simplified)
    // Full inventory implementation will come later
    // For now just slot count for protocol packets
    static constexpr int INVENTORY_SIZE = 45;

    // NBT serialization — matches mw.b(dh) / mw.a(dh)
    std::shared_ptr<nbt::NBTTagCompound> saveToNBT() const {
        auto tag = std::make_shared<nbt::NBTTagCompound>();

        // Position
        auto pos = std::make_shared<nbt::NBTTagList>();
        pos->add(std::make_shared<nbt::NBTTagDouble>(posX));
        pos->add(std::make_shared<nbt::NBTTagDouble>(posY));
        pos->add(std::make_shared<nbt::NBTTagDouble>(posZ));
        tag->setList("Pos", pos);

        // Rotation
        auto rot = std::make_shared<nbt::NBTTagList>();
        rot->add(std::make_shared<nbt::NBTTagFloat>(yaw));
        rot->add(std::make_shared<nbt::NBTTagFloat>(pitch));
        tag->setList("Rotation", rot);

        // Player data
        tag->setFloat("Health", health);
        tag->setInt("foodLevel", foodLevel);
        tag->setFloat("foodSaturationLevel", saturation);
        tag->setFloat("foodExhaustionLevel", exhaustion);
        tag->setInt("XpLevel", experienceLevel);
        tag->setFloat("XpP", experienceProgress);
        tag->setInt("XpTotal", totalExperience);
        tag->setInt("playerGameType", static_cast<int32_t>(gameMode));
        tag->setInt("Dimension", dimension);
        tag->setBoolean("OnGround", onGround);

        return tag;
    }

    void loadFromNBT(const nbt::NBTTagCompound& tag) {
        // Position
        if (tag.hasKey("Pos")) {
            auto pos = tag.getList("Pos", nbt::TAG_Double);
            if (pos->size() >= 3) {
                posX = std::dynamic_pointer_cast<nbt::NBTTagDouble>(pos->tags[0])->value;
                posY = std::dynamic_pointer_cast<nbt::NBTTagDouble>(pos->tags[1])->value;
                posZ = std::dynamic_pointer_cast<nbt::NBTTagDouble>(pos->tags[2])->value;
            }
        }

        // Rotation
        if (tag.hasKey("Rotation")) {
            auto rot = tag.getList("Rotation", nbt::TAG_Float);
            if (rot->size() >= 2) {
                yaw = std::dynamic_pointer_cast<nbt::NBTTagFloat>(rot->tags[0])->value;
                pitch = std::dynamic_pointer_cast<nbt::NBTTagFloat>(rot->tags[1])->value;
            }
        }

        // Player data
        if (tag.hasKey("Health")) health = tag.getFloat("Health");
        if (tag.hasKey("foodLevel")) foodLevel = tag.getInt("foodLevel");
        if (tag.hasKey("foodSaturationLevel")) saturation = tag.getFloat("foodSaturationLevel");
        if (tag.hasKey("foodExhaustionLevel")) exhaustion = tag.getFloat("foodExhaustionLevel");
        if (tag.hasKey("XpLevel")) experienceLevel = tag.getInt("XpLevel");
        if (tag.hasKey("XpP")) experienceProgress = tag.getFloat("XpP");
        if (tag.hasKey("XpTotal")) totalExperience = tag.getInt("XpTotal");
        if (tag.hasKey("playerGameType")) gameMode = static_cast<GameMode>(tag.getInt("playerGameType"));
        if (tag.hasKey("Dimension")) dimension = static_cast<int8_t>(tag.getInt("Dimension"));
        if (tag.hasKey("OnGround")) onGround = tag.getBoolean("OnGround");
    }

    // Get eye position Y (mw.g() = 1.62f)
    double eyeY() const { return posY + EYE_HEIGHT; }

    // Get chunk coordinates
    int chunkX() const { return static_cast<int>(posX) >> 4; }
    int chunkZ() const { return static_cast<int>(posZ) >> 4; }
};

} // namespace mc
