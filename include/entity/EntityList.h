/**
 * EntityList.h — Entity type registry.
 *
 * Java reference: net.minecraft.entity.EntityList
 *
 * 57 registered entity types with string↔ID mapping.
 * 24 spawn eggs with primary+secondary colors.
 *
 * Thread safety: Static/const after init.
 *
 * JNI readiness: Integer IDs, string-based type names.
 */
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityEggInfo — Spawn egg colors.
// Java reference: net.minecraft.entity.EntityList$EntityEggInfo
// ═══════════════════════════════════════════════════════════════════════════

struct EntityEggInfo {
    int32_t entityId;
    int32_t primaryColor;
    int32_t secondaryColor;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityListEntry — One registered entity type.
// ═══════════════════════════════════════════════════════════════════════════

struct EntityListEntry {
    std::string name;   // e.g., "Creeper"
    int32_t id;         // e.g., 50
    bool hasEgg;
    int32_t eggPrimary;
    int32_t eggSecondary;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityList — Entity type registry.
// Java reference: net.minecraft.entity.EntityList
// ═══════════════════════════════════════════════════════════════════════════

class EntityList {
public:
    static void init();

    // Java: string → ID
    static int32_t getIdByName(const std::string& name);

    // Java: ID → string
    static const std::string& getNameById(int32_t id);

    // Java: entityEggs lookup
    static const EntityEggInfo* getEggInfo(int32_t id);

    // Get all registered entries
    static const std::vector<EntityListEntry>& getAllEntries();

    // Get all spawn eggs
    static const std::vector<EntityEggInfo>& getAllEggs();

    static int32_t getEntityCount();
    static int32_t getEggCount();

private:
    static std::unordered_map<std::string, int32_t> nameToId_;
    static std::unordered_map<int32_t, std::string> idToName_;
    static std::vector<EntityListEntry> entries_;
    static std::vector<EntityEggInfo> eggs_;
    static std::unordered_map<int32_t, size_t> eggIndex_;
    static bool initialized_;
};

} // namespace mccpp
