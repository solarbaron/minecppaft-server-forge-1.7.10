/**
 * WorldInfo.h — World metadata, difficulty, and world type.
 *
 * Java references:
 *   - net.minecraft.world.EnumDifficulty — 4 difficulty levels
 *   - net.minecraft.world.WorldType — 5 world generation types
 *   - net.minecraft.world.storage.WorldInfo — World metadata
 *   - net.minecraft.world.WorldSettings — World creation settings
 *   - net.minecraft.world.WorldSettings$GameType — Game modes
 *
 * Thread safety:
 *   - WorldInfo is per-world, modified from server thread.
 *   - Uses std::shared_mutex for concurrent read access.
 *
 * JNI readiness: Integer IDs for enums, simple struct layout.
 */
#pragma once

#include <cstdint>
#include <shared_mutex>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EnumDifficulty — Server difficulty levels.
// Java reference: net.minecraft.world.EnumDifficulty
// ═══════════════════════════════════════════════════════════════════════════

enum class Difficulty : int32_t {
    PEACEFUL = 0,
    EASY     = 1,
    NORMAL   = 2,
    HARD     = 3
};

inline Difficulty getDifficulty(int32_t id) {
    return static_cast<Difficulty>(id % 4);
}

inline const char* getDifficultyName(Difficulty d) {
    switch (d) {
        case Difficulty::PEACEFUL: return "Peaceful";
        case Difficulty::EASY:     return "Easy";
        case Difficulty::NORMAL:   return "Normal";
        case Difficulty::HARD:     return "Hard";
    }
    return "Unknown";
}

// ═══════════════════════════════════════════════════════════════════════════
// GameType — Game mode types.
// Java reference: net.minecraft.world.WorldSettings$GameType
// ═══════════════════════════════════════════════════════════════════════════

enum class GameType : int32_t {
    NOT_SET   = -1,  // Java: NOT_SET
    SURVIVAL  = 0,
    CREATIVE  = 1,
    ADVENTURE = 2,
    SPECTATOR = 3    // Not in 1.7.10 but needed for protocol
};

inline GameType getGameType(int32_t id) {
    switch (id) {
        case 0: return GameType::SURVIVAL;
        case 1: return GameType::CREATIVE;
        case 2: return GameType::ADVENTURE;
        case 3: return GameType::SPECTATOR;
        default: return GameType::NOT_SET;
    }
}

inline const char* getGameTypeName(GameType g) {
    switch (g) {
        case GameType::SURVIVAL:  return "Survival";
        case GameType::CREATIVE:  return "Creative";
        case GameType::ADVENTURE: return "Adventure";
        case GameType::SPECTATOR: return "Spectator";
        default:                  return "Unknown";
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// WorldType — World generation types.
// Java reference: net.minecraft.world.WorldType
// IDs: 0=default, 1=flat, 2=largeBiomes, 3=amplified, 8=default_1_1
// ═══════════════════════════════════════════════════════════════════════════

struct WorldTypeInfo {
    int32_t id;
    std::string name;
    int32_t generatorVersion;
    bool canBeCreated;
    bool isVersioned;
    bool hasNotificationData;
};

class WorldTypes {
public:
    static void init();
    static const WorldTypeInfo* getById(int32_t id);
    static const WorldTypeInfo* getByName(const std::string& name);

    static constexpr int32_t DEFAULT_ID      = 0;
    static constexpr int32_t FLAT_ID         = 1;
    static constexpr int32_t LARGE_BIOMES_ID = 2;
    static constexpr int32_t AMPLIFIED_ID    = 3;
    static constexpr int32_t DEFAULT_1_1_ID  = 8;

private:
    static WorldTypeInfo types_[16];
    static bool initialized_;
};

// ═══════════════════════════════════════════════════════════════════════════
// WorldInfo — Per-world metadata.
// Java reference: net.minecraft.world.storage.WorldInfo
//
// Stores: seed, spawn position, time, weather, difficulty, game type,
// world type, structures, hardcore, world border, etc.
// ═══════════════════════════════════════════════════════════════════════════

class WorldInfo {
public:
    WorldInfo();

    // ─── Seed ───
    int64_t getSeed() const;
    void setSeed(int64_t seed);

    // ─── Spawn position ───
    int32_t getSpawnX() const;
    int32_t getSpawnY() const;
    int32_t getSpawnZ() const;
    void setSpawnPosition(int32_t x, int32_t y, int32_t z);

    // ─── World time ───
    int64_t getWorldTotalTime() const;
    void setWorldTotalTime(int64_t time);
    int64_t getWorldTime() const;
    void setWorldTime(int64_t time);

    // ─── Weather ───
    bool isRaining() const;
    void setRaining(bool raining);
    int32_t getRainTime() const;
    void setRainTime(int32_t time);
    bool isThundering() const;
    void setThundering(bool thundering);
    int32_t getThunderTime() const;
    void setThunderTime(int32_t time);

    // ─── Game settings ───
    GameType getGameType() const;
    void setGameType(GameType type);
    Difficulty getDifficulty() const;
    void setDifficulty(Difficulty diff);
    bool isHardcore() const;
    void setHardcore(bool hardcore);
    bool areCommandsAllowed() const;
    void setCommandsAllowed(bool allowed);
    bool isMapFeaturesEnabled() const;
    void setMapFeaturesEnabled(bool enabled);

    // ─── World type ───
    int32_t getWorldTypeId() const;
    void setWorldTypeId(int32_t id);
    std::string getGeneratorOptions() const;
    void setGeneratorOptions(const std::string& opts);

    // ─── World name ───
    std::string getWorldName() const;
    void setWorldName(const std::string& name);

    // ─── World border (1.7.10 additions) ───
    double getBorderCenterX() const;
    double getBorderCenterZ() const;
    void setBorderCenter(double x, double z);
    double getBorderSize() const;
    void setBorderSize(double size);

    // ─── Size on disk ───
    int64_t getSizeOnDisk() const;
    void setSizeOnDisk(int64_t size);

    // ─── Last played ───
    int64_t getLastTimePlayed() const;
    void setLastTimePlayed(int64_t time);

private:
    mutable std::shared_mutex mutex_;  // Thread safety: L22 world info mutex

    int64_t seed_ = 0;
    int32_t spawnX_ = 0, spawnY_ = 64, spawnZ_ = 0;
    int64_t totalTime_ = 0;
    int64_t worldTime_ = 0;
    bool raining_ = false;
    int32_t rainTime_ = 0;
    bool thundering_ = false;
    int32_t thunderTime_ = 0;
    GameType gameType_ = GameType::SURVIVAL;
    Difficulty difficulty_ = Difficulty::EASY;
    bool hardcore_ = false;
    bool commandsAllowed_ = false;
    bool mapFeatures_ = true;
    int32_t worldTypeId_ = 0;
    std::string generatorOptions_;
    std::string worldName_ = "world";
    double borderCenterX_ = 0.0;
    double borderCenterZ_ = 0.0;
    double borderSize_ = 60000000.0;  // Java default: 6e7
    int64_t sizeOnDisk_ = 0;
    int64_t lastPlayed_ = 0;
};

} // namespace mccpp
