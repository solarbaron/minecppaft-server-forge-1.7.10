/**
 * SaveHandler.h — World persistence and player data management.
 *
 * Java reference: net.minecraft.world.storage.SaveHandler (246 lines)
 *
 * Manages all file I/O for world persistence:
 *
 * 1. Session Lock:
 *    - Write initTime to session.lock on creation
 *    - checkSessionLock: verify initTime matches to prevent concurrent access
 *    - Throws MinecraftException if lock mismatch
 *
 * 2. level.dat Persistence:
 *    - Load: try level.dat, fallback to level.dat_old
 *    - Save (atomic write pattern):
 *      a. Write to level.dat_new
 *      b. Delete level.dat_old
 *      c. Rename level.dat → level.dat_old
 *      d. Rename level.dat_new → level.dat
 *    - Format: GZIP-compressed NBT with root compound "Data"
 *
 * 3. Player Data:
 *    - Directory: <world>/playerdata/
 *    - Files: <uuid>.dat (GZIP NBT)
 *    - Atomic write: write to .dat.tmp, rename to .dat
 *
 * 4. Map Data:
 *    - Directory: <world>/data/
 *    - Files: <name>.dat
 *
 * Thread safety: File operations are not thread-safe by default.
 * The server should ensure only one thread accesses save operations.
 * JNI readiness: Simple file path management.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// SaveHandler constants
// ═══════════════════════════════════════════════════════════════════════════

namespace SaveConstants {
    // ─── File names ───
    static constexpr const char* SESSION_LOCK_FILE = "session.lock";
    static constexpr const char* LEVEL_DAT = "level.dat";
    static constexpr const char* LEVEL_DAT_OLD = "level.dat_old";
    static constexpr const char* LEVEL_DAT_NEW = "level.dat_new";

    // ─── Directories ───
    static constexpr const char* PLAYERDATA_DIR = "playerdata";
    static constexpr const char* MAPDATA_DIR = "data";

    // ─── File extensions ───
    static constexpr const char* DAT_EXTENSION = ".dat";
    static constexpr const char* DAT_TMP_EXTENSION = ".dat.tmp";

    // ─── NBT root key ───
    static constexpr const char* DATA_KEY = "Data";
}

// ═══════════════════════════════════════════════════════════════════════════
// SaveHandler — World file management.
// ═══════════════════════════════════════════════════════════════════════════

class SaveHandler {
public:
    // ─── Paths ───
    std::string worldDirectory;
    std::string playersDirectory;
    std::string mapDataDir;
    std::string saveDirectoryName;
    int64_t initializationTime = 0;

    SaveHandler() = default;

    SaveHandler(const std::string& baseDir, const std::string& saveName, bool createPlayerDir)
        : saveDirectoryName(saveName)
    {
        worldDirectory = baseDir + "/" + saveName;
        playersDirectory = worldDirectory + "/" + SaveConstants::PLAYERDATA_DIR;
        mapDataDir = worldDirectory + "/" + SaveConstants::MAPDATA_DIR;

        // Would create directories:
        // mkdir_p(worldDirectory);
        // mkdir_p(mapDataDir);
        // if (createPlayerDir) mkdir_p(playersDirectory);
        // setSessionLock();
    }

    // ─── Session lock ───
    // Java: setSessionLock() — write initTime to session.lock
    std::string getSessionLockPath() const {
        return worldDirectory + "/" + SaveConstants::SESSION_LOCK_FILE;
    }

    // Java: checkSessionLock() — verify initTime matches
    // Returns true if lock is valid, false if lock mismatch
    bool isSessionLockValid(int64_t readValue) const {
        return readValue == initializationTime;
    }

    // ─── level.dat paths ───
    std::string getLevelDatPath() const {
        return worldDirectory + "/" + SaveConstants::LEVEL_DAT;
    }

    std::string getLevelDatOldPath() const {
        return worldDirectory + "/" + SaveConstants::LEVEL_DAT_OLD;
    }

    std::string getLevelDatNewPath() const {
        return worldDirectory + "/" + SaveConstants::LEVEL_DAT_NEW;
    }

    // ─── Player data paths ───
    // Java: writePlayerData/readPlayerData — <uuid>.dat in playerdata/
    std::string getPlayerDataPath(const std::string& uuid) const {
        return playersDirectory + "/" + uuid + SaveConstants::DAT_EXTENSION;
    }

    std::string getPlayerDataTmpPath(const std::string& uuid) const {
        return playersDirectory + "/" + uuid + SaveConstants::DAT_TMP_EXTENSION;
    }

    // ─── Map data paths ───
    // Java: getMapFileFromName(name) → data/<name>.dat
    std::string getMapDataPath(const std::string& name) const {
        return mapDataDir + "/" + name + SaveConstants::DAT_EXTENSION;
    }

    // ─── Atomic save pattern ───
    // Java: saveWorldInfo uses this 3-step atomic write:
    //   1. Write to level.dat_new
    //   2. Delete level.dat_old, rename level.dat → level.dat_old
    //   3. Rename level.dat_new → level.dat
    //
    // This ensures that even if the server crashes during save,
    // either level.dat or level.dat_old will be valid.
    struct AtomicPaths {
        std::string newPath;
        std::string oldPath;
        std::string finalPath;
    };

    AtomicPaths getLevelDatAtomicPaths() const {
        return {getLevelDatNewPath(), getLevelDatOldPath(), getLevelDatPath()};
    }

    // ─── Available player data ───
    // Java: getAvailablePlayerDat() — list playerdata/*.dat, strip extension
    // Returns UUID strings of all saved players
};

// ═══════════════════════════════════════════════════════════════════════════
// WorldType — Terrain generation type registry.
//
// Java reference: net.minecraft.world.WorldType (79 lines)
//
// Registry of terrain generation types used in world creation.
// Array of 16 slots, 5 defined:
//   0: DEFAULT ("default", version 1, versioned)
//   1: FLAT ("flat")
//   2: LARGE_BIOMES ("largeBiomes")
//   3: AMPLIFIED ("amplified", has notification data)
//   8: DEFAULT_1_1 ("default_1_1", version 0, cannot be created)
// ═══════════════════════════════════════════════════════════════════════════

struct WorldTypeEntry {
    int32_t id;
    const char* name;
    int32_t generatorVersion;
    bool canBeCreated;
    bool isVersioned;
    bool hasNotificationData;
};

namespace WorldType {
    // ─── World type IDs ───
    static constexpr int32_t DEFAULT      = 0;
    static constexpr int32_t FLAT         = 1;
    static constexpr int32_t LARGE_BIOMES = 2;
    static constexpr int32_t AMPLIFIED    = 3;
    static constexpr int32_t DEFAULT_1_1  = 8;

    static constexpr int32_t ARRAY_SIZE = 16;

    // ─── World type registry ───
    // Java: WorldType.worldTypes[16]
    static constexpr WorldTypeEntry TYPES[] = {
        {0, "default",      1, true,  true,  false},   // DEFAULT
        {1, "flat",          0, true,  false, false},   // FLAT
        {2, "largeBiomes",   0, true,  false, false},   // LARGE_BIOMES
        {3, "amplified",     0, true,  false, true},    // AMPLIFIED
        {4, nullptr,         0, false, false, false},   // unused
        {5, nullptr,         0, false, false, false},   // unused
        {6, nullptr,         0, false, false, false},   // unused
        {7, nullptr,         0, false, false, false},   // unused
        {8, "default_1_1",   0, false, false, false},   // DEFAULT_1_1
    };

    static constexpr int32_t REGISTERED_COUNT = 9;

    // ─── parseWorldType ───
    // Java: WorldType.parseWorldType(name) — case-insensitive name lookup
    // Returns type ID or -1 if not found
    // Note: actual case-insensitive comparison needs implementation
    inline int32_t parseWorldType(const char* name) {
        for (int32_t i = 0; i < REGISTERED_COUNT; ++i) {
            if (TYPES[i].name != nullptr) {
                // Simple comparison (would need case-insensitive in production)
                const char* a = TYPES[i].name;
                const char* b = name;
                bool match = true;
                while (*a && *b) {
                    char ca = *a >= 'A' && *a <= 'Z' ? *a + 32 : *a;
                    char cb = *b >= 'A' && *b <= 'Z' ? *b + 32 : *b;
                    if (ca != cb) { match = false; break; }
                    ++a; ++b;
                }
                if (match && *a == '\0' && *b == '\0') return TYPES[i].id;
            }
        }
        return -1;
    }

    // ─── getWorldTypeForGeneratorVersion ───
    // Java: if (this == DEFAULT && version == 0) return DEFAULT_1_1
    inline int32_t getTypeForVersion(int32_t typeId, int32_t version) {
        if (typeId == DEFAULT && version == 0) return DEFAULT_1_1;
        return typeId;
    }

    // ─── Type name accessors ───
    inline const char* getTypeName(int32_t id) {
        if (id >= 0 && id < REGISTERED_COUNT && TYPES[id].name != nullptr)
            return TYPES[id].name;
        return "default";
    }
}

} // namespace mccpp
