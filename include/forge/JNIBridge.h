/**
 * JNIBridge.h — Forge/JNI bridge infrastructure.
 *
 * Provides the foundation for running Minecraft 1.7.10 Forge mods
 * within the C++ server via JNI. This includes:
 *   - JVM lifecycle management (create/destroy)
 *   - Thread-local JNIEnv management for multi-threaded access
 *   - Class and method caching for performance
 *   - Event bus for C++ → Java event dispatch
 *   - Registry bridge for exposing C++ Block/Item/Entity to JVM
 *   - Forge mod loader integration hooks
 *
 * Architecture:
 *   The C++ server owns the JVM. Native methods registered via JNI
 *   allow Java Forge mods to call back into C++ game objects.
 *   The event bus dispatches game events (block break, entity spawn, etc.)
 *   to registered Java event handlers via reflection.
 *
 * Thread safety:
 *   - JVM is created once at startup, destroyed at shutdown.
 *   - JNIEnv is thread-local (one per thread, obtained via AttachCurrentThread).
 *   - Class/method caches use global refs (valid across threads).
 *   - Event dispatch is synchronized via mutex per event type.
 *
 * Lock hierarchy (extends main server hierarchy):
 *   L10: JNIBridge::mutex_ (JVM lifecycle)
 *   L11: EventBus::mutex_ (event handler registration)
 *   L12: RegistryBridge::mutex_ (registry synchronization)
 */
#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// Forward declarations — JNI types are opaque when JNI is not available
// When building with JNI support, these resolve to <jni.h> types
#ifdef MCCPP_JNI_ENABLED
#include <jni.h>
#else
// Stub types for compilation without JNI headers
using JavaVM = void;
using JNIEnv = void;
using jclass = void*;
using jmethodID = void*;
using jfieldID = void*;
using jobject = void*;
using jstring = void*;
using jint = int32_t;
using jlong = int64_t;
using jfloat = float;
using jdouble = double;
using jboolean = uint8_t;
#endif

// ─── JNI Configuration ────────────────────────────────────────────────────

struct JNIConfig {
    // Path to JVM shared library (libjvm.so / jvm.dll)
    std::string jvmLibPath;

    // JVM classpath (Forge JARs, mod JARs, libraries)
    std::vector<std::string> classpath;

    // Additional JVM options (-Xmx, -Xms, etc.)
    std::vector<std::string> jvmOptions;

    // Forge version string
    std::string forgeVersion = "10.13.4.1614";

    // Minecraft version
    std::string mcVersion = "1.7.10";

    // Max JVM heap size
    std::string maxHeap = "1024m";

    // Enable JNI debug output
    bool debug = false;
};

// ─── CachedClass ──────────────────────────────────────────────────────────
// Pre-resolved Java class reference with cached method/field IDs.

struct CachedMethod {
    std::string name;
    std::string signature;
    jmethodID id = nullptr;
    bool isStatic = false;
};

struct CachedField {
    std::string name;
    std::string signature;
    jfieldID id = nullptr;
    bool isStatic = false;
};

struct CachedClass {
    std::string className;          // e.g. "net/minecraftforge/fml/common/FMLCommonHandler"
    jclass classRef = nullptr;      // Global reference (valid across threads)
    std::unordered_map<std::string, CachedMethod> methods;
    std::unordered_map<std::string, CachedField> fields;
};

// ═══════════════════════════════════════════════════════════════════════════
// JNIBridge — JVM lifecycle and thread-local env management.
//
// Usage:
//   JNIBridge::init(config);
//   JNIEnv* env = JNIBridge::getEnv();
//   // ... use env for JNI calls ...
//   JNIBridge::shutdown();
// ═══════════════════════════════════════════════════════════════════════════

class JNIBridge {
public:
    // Initialize JVM with given configuration
    // Must be called from the main thread before any JNI operations
    static bool init(const JNIConfig& config);

    // Shutdown JVM and release all resources
    static void shutdown();

    // Get thread-local JNIEnv (attaches current thread if needed)
    // Returns nullptr if JVM is not initialized
    static JNIEnv* getEnv();

    // Detach current thread from JVM (call before thread exit)
    static void detachCurrentThread();

    // Check if JVM is initialized and running
    static bool isInitialized() { return initialized_.load(std::memory_order_acquire); }

    // Find and cache a Java class (creates global ref)
    static CachedClass* cacheClass(const std::string& className);

    // Get a previously cached class
    static CachedClass* getCachedClass(const std::string& className);

    // Cache a method ID for a cached class
    static bool cacheMethod(CachedClass& cls, const std::string& name,
                            const std::string& sig, bool isStatic = false);

    // Cache a field ID for a cached class
    static bool cacheField(CachedClass& cls, const std::string& name,
                           const std::string& sig, bool isStatic = false);

    // Register native methods for a Java class
    static bool registerNatives(const std::string& className,
                                 const void* methods, int32_t methodCount);

private:
    static std::mutex mutex_;
    static JavaVM* jvm_;
    static std::atomic<bool> initialized_;
    static std::unordered_map<std::string, CachedClass> classCache_;
    static thread_local JNIEnv* threadEnv_;
};

// ═══════════════════════════════════════════════════════════════════════════
// ForgeEvent — Base class for events dispatched through the event bus.
//
// Maps to net.minecraftforge.fml.common.eventhandler.Event
// ═══════════════════════════════════════════════════════════════════════════

class ForgeEvent {
public:
    virtual ~ForgeEvent() = default;

    // Event type identifier (matches Java class name)
    virtual std::string getEventClass() const = 0;

    // Whether this event can be canceled
    virtual bool isCancelable() const { return false; }

    // Cancel state
    bool isCanceled() const { return canceled_; }
    void setCanceled(bool canceled) { canceled_ = canceled; }

    // Result (for HasResult events)
    enum class Result { DENY, DEFAULT, ALLOW };
    Result getResult() const { return result_; }
    void setResult(Result result) { result_ = result; }

private:
    bool canceled_ = false;
    Result result_ = Result::DEFAULT;
};

// ─── Concrete events ──────────────────────────────────────────────────────

// Block break event — fired when a block is about to be broken
// Maps to: net.minecraftforge.event.world.BlockEvent.BreakEvent
class BlockBreakEvent : public ForgeEvent {
public:
    BlockBreakEvent(int32_t x, int32_t y, int32_t z, int32_t blockId, int32_t meta)
        : x_(x), y_(y), z_(z), blockId_(blockId), meta_(meta) {}

    std::string getEventClass() const override {
        return "net/minecraftforge/event/world/BlockEvent$BreakEvent";
    }
    bool isCancelable() const override { return true; }

    int32_t getX() const { return x_; }
    int32_t getY() const { return y_; }
    int32_t getZ() const { return z_; }
    int32_t getBlockId() const { return blockId_; }
    int32_t getMeta() const { return meta_; }

private:
    int32_t x_, y_, z_, blockId_, meta_;
};

// Entity spawn event
// Maps to: net.minecraftforge.event.entity.EntityJoinWorldEvent
class EntitySpawnEvent : public ForgeEvent {
public:
    EntitySpawnEvent(int32_t entityId, double x, double y, double z)
        : entityId_(entityId), x_(x), y_(y), z_(z) {}

    std::string getEventClass() const override {
        return "net/minecraftforge/event/entity/EntityJoinWorldEvent";
    }
    bool isCancelable() const override { return true; }

    int32_t getEntityId() const { return entityId_; }
    double getX() const { return x_; }
    double getY() const { return y_; }
    double getZ() const { return z_; }

private:
    int32_t entityId_;
    double x_, y_, z_;
};

// Player chat event
// Maps to: net.minecraftforge.event.ServerChatEvent
class ServerChatEvent : public ForgeEvent {
public:
    ServerChatEvent(const std::string& username, const std::string& message)
        : username_(username), message_(message) {}

    std::string getEventClass() const override {
        return "net/minecraftforge/event/ServerChatEvent";
    }
    bool isCancelable() const override { return true; }

    const std::string& getUsername() const { return username_; }
    const std::string& getMessage() const { return message_; }
    void setMessage(const std::string& msg) { message_ = msg; }

private:
    std::string username_;
    std::string message_;
};

// Server tick event
// Maps to: cpw.mods.fml.common.gameevent.TickEvent.ServerTickEvent
class ServerTickEvent : public ForgeEvent {
public:
    enum class Phase { START, END };

    explicit ServerTickEvent(Phase phase) : phase_(phase) {}

    std::string getEventClass() const override {
        return "cpw/mods/fml/common/gameevent/TickEvent$ServerTickEvent";
    }

    Phase getPhase() const { return phase_; }

private:
    Phase phase_;
};

// ═══════════════════════════════════════════════════════════════════════════
// EventBus — C++ side of the Forge event bus.
//
// Maps to: net.minecraftforge.common.MinecraftForge.EVENT_BUS
//
// Dispatches ForgeEvent objects to:
//   1. C++ event handlers (for native mods/plugins)
//   2. Java event handlers via JNI (for Forge mods)
//
// Thread safety: shared_mutex for handler registration (rare writes),
// lock-free dispatch via snapshot of handler list.
// ═══════════════════════════════════════════════════════════════════════════

class EventBus {
public:
    using EventHandler = std::function<void(ForgeEvent&)>;

    static EventBus& instance();

    // Register a C++ event handler for a specific event class
    void registerHandler(const std::string& eventClass, EventHandler handler);

    // Post an event to all registered handlers
    // Returns true if the event was NOT canceled
    bool post(ForgeEvent& event);

    // Get the number of registered handlers
    int32_t getHandlerCount() const;

private:
    EventBus() = default;
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::vector<EventHandler>> handlers_;
};

// ═══════════════════════════════════════════════════════════════════════════
// RegistryBridge — Exposes C++ registries to the JVM.
//
// When Forge mods access Block.getBlockById(), Item.getItemById(), etc.,
// those calls route through JNI native methods back to C++ registries.
//
// This class manages the mapping between C++ registry entries and their
// Java-side proxy objects.
// ═══════════════════════════════════════════════════════════════════════════

class RegistryBridge {
public:
    static RegistryBridge& instance();

    // Synchronize C++ block registry to Java
    void syncBlocks();

    // Synchronize C++ item registry to Java
    void syncItems();

    // Register a native method mapping
    void registerNativeMethod(const std::string& className,
                               const std::string& methodName,
                               const std::string& signature,
                               void* fnPtr);

    // Get count of registered native bindings
    int32_t getNativeBindingCount() const;

private:
    RegistryBridge() = default;
    mutable std::mutex mutex_;

    struct NativeBinding {
        std::string className;
        std::string methodName;
        std::string signature;
        void* fnPtr;
    };
    std::vector<NativeBinding> bindings_;
};

// ═══════════════════════════════════════════════════════════════════════════
// ForgeModLoader — Discovery and lifecycle management for Forge mods.
//
// Maps to: cpw.mods.fml.common.Loader
//
// Lifecycle phases:
//   1. DISCOVERY — Scan mods directory for JAR files
//   2. CONSTRUCTION — Instantiate mod classes
//   3. PRE_INIT — FMLPreInitializationEvent
//   4. INIT — FMLInitializationEvent
//   5. POST_INIT — FMLPostInitializationEvent
//   6. AVAILABLE — Server is ready, mods are fully loaded
//   7. UNLOADING — Server shutdown
// ═══════════════════════════════════════════════════════════════════════════

class ForgeModLoader {
public:
    enum class LoaderState {
        NOINIT,
        DISCOVERY,
        CONSTRUCTION,
        PRE_INIT,
        INIT,
        POST_INIT,
        AVAILABLE,
        UNLOADING,
        ERRORED
    };

    struct ModInfo {
        std::string modId;
        std::string name;
        std::string version;
        std::string jarPath;
        std::string mainClass;
        bool loaded = false;
    };

    static ForgeModLoader& instance();

    // Scan mods directory for Forge mod JARs
    int32_t discoverMods(const std::string& modsDir);

    // Run the Forge mod loading lifecycle
    bool loadMods();

    // Get current loader state
    LoaderState getState() const { return state_.load(std::memory_order_acquire); }

    // Get list of discovered/loaded mods
    const std::vector<ModInfo>& getMods() const { return mods_; }

    // Get mod count
    int32_t getModCount() const { return static_cast<int32_t>(mods_.size()); }

private:
    ForgeModLoader() = default;
    std::atomic<LoaderState> state_{LoaderState::NOINIT};
    std::vector<ModInfo> mods_;
    std::mutex mutex_;
};

} // namespace mccpp
