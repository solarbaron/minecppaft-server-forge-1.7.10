/**
 * Registry.h — Generic named+ID registry, matching Java's RegistryNamespaced.
 *
 * Java reference: net.minecraft.util.RegistryNamespaced
 *
 * Provides bidirectional lookup: name↔object and id↔object.
 * Names are auto-namespaced with "minecraft:" prefix if no colon present.
 *
 * Thread safety: read-only after initialization (registerBlocks/registerItems
 * complete before any other threads access). No mutex needed.
 *
 * JNI readiness: int IDs for fast lookup, string names for Java interop.
 */
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

/**
 * RegistryNamespaced — bidirectional name+ID registry.
 *
 * Java reference: net.minecraft.util.RegistryNamespaced
 * T must be a pointer type or shared_ptr.
 */
template<typename T>
class RegistryNamespaced {
public:
    /**
     * Register an object with an integer ID and string name.
     * Java reference: RegistryNamespaced.addObject(int, String, Object)
     */
    void addObject(int32_t id, const std::string& name, T object) {
        std::string nsName = ensureNamespaced(name);

        // ID → object
        if (id >= static_cast<int32_t>(idToObject_.size())) {
            idToObject_.resize(static_cast<size_t>(id) + 1, T{});
        }
        idToObject_[id] = object;

        // name → object (bidirectional)
        nameToObject_[nsName] = object;
        objectToName_[object] = nsName;
        objectToId_[object] = id;
    }

    /**
     * Get object by string name.
     */
    T getObject(const std::string& name) const {
        auto it = nameToObject_.find(ensureNamespaced(name));
        return (it != nameToObject_.end()) ? it->second : T{};
    }

    /**
     * Get object by integer ID.
     * Java reference: RegistryNamespaced.getObjectById(int)
     */
    T getObjectById(int32_t id) const {
        if (id < 0 || id >= static_cast<int32_t>(idToObject_.size())) return T{};
        return idToObject_[id];
    }

    /**
     * Get the integer ID for an object.
     * Java reference: RegistryNamespaced.getIDForObject(Object)
     */
    int32_t getIDForObject(T object) const {
        auto it = objectToId_.find(object);
        return (it != objectToId_.end()) ? it->second : -1;
    }

    /**
     * Get the name for an object.
     */
    std::string getNameForObject(T object) const {
        auto it = objectToName_.find(object);
        return (it != objectToName_.end()) ? it->second : "";
    }

    /**
     * Check if a name is registered.
     */
    bool containsKey(const std::string& name) const {
        return nameToObject_.find(ensureNamespaced(name)) != nameToObject_.end();
    }

    /**
     * Check if an ID is registered.
     */
    bool containsId(int32_t id) const {
        return id >= 0 && id < static_cast<int32_t>(idToObject_.size()) && idToObject_[id] != T{};
    }

    /**
     * Number of registered objects.
     */
    size_t size() const { return nameToObject_.size(); }

    /**
     * Max registered ID + 1 (for array sizing).
     */
    int32_t maxId() const { return static_cast<int32_t>(idToObject_.size()); }

private:
    /**
     * Java reference: RegistryNamespaced.ensureNamespaced()
     * Prepends "minecraft:" if no colon in the name.
     */
    static std::string ensureNamespaced(const std::string& name) {
        if (name.find(':') == std::string::npos) {
            return "minecraft:" + name;
        }
        return name;
    }

    std::vector<T>                        idToObject_;
    std::unordered_map<std::string, T>    nameToObject_;
    std::unordered_map<T, std::string>    objectToName_;
    std::unordered_map<T, int32_t>        objectToId_;
};

} // namespace mccpp
