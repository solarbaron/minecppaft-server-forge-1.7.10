/**
 * ForgeEventTypes.h — Comprehensive Forge event type catalog.
 *
 * Extends JNIBridge.h ForgeEvent base with all major Forge 1.7.10 events
 * that mods commonly subscribe to. Each event maps to a specific Java class
 * in the Forge/FML event system.
 *
 * Event hierarchy (net.minecraftforge.event.*):
 *   Event (base)
 *     ├── world.BlockEvent
 *     │   ├── BreakEvent (in JNIBridge.h)
 *     │   ├── PlaceEvent
 *     │   └── HarvestDropsEvent
 *     ├── world.WorldEvent
 *     │   ├── Load
 *     │   ├── Unload
 *     │   └── Save
 *     ├── world.ChunkEvent
 *     │   ├── Load
 *     │   └── Unload
 *     ├── world.ExplosionEvent
 *     │   ├── Start
 *     │   └── Detonate
 *     ├── entity.EntityEvent
 *     │   ├── EntityJoinWorldEvent (in JNIBridge.h)
 *     │   └── EntityConstructing
 *     ├── entity.living.LivingEvent
 *     │   ├── LivingUpdateEvent
 *     │   ├── LivingHurtEvent
 *     │   ├── LivingDeathEvent
 *     │   ├── LivingDropsEvent
 *     │   ├── LivingAttackEvent
 *     │   └── LivingFallEvent
 *     ├── entity.player.PlayerEvent
 *     │   ├── PlayerInteractEvent
 *     │   ├── PlayerLoggedInEvent
 *     │   ├── PlayerLoggedOutEvent
 *     │   ├── PlayerRespawnEvent
 *     │   ├── PlayerChangedDimensionEvent
 *     │   └── ItemCraftedEvent
 *     ├── entity.item.ItemTossEvent
 *     ├── entity.item.ItemExpireEvent
 *     ├── ServerChatEvent (in JNIBridge.h)
 *     └── CommandEvent
 *
 * FML events (cpw.mods.fml.common.*):
 *     ├── gameevent.TickEvent
 *     │   ├── ServerTickEvent (in JNIBridge.h)
 *     │   ├── WorldTickEvent
 *     │   └── PlayerTickEvent
 *     └── event.FMLServerStartingEvent, etc.
 *
 * Thread safety: Events are created on the server thread and dispatched
 * synchronously. The EventBus in JNIBridge.h handles synchronization.
 * JNI readiness: All events use simple POD-like fields for easy JNI mapping.
 */
#pragma once

#include "JNIBridge.h"
#include <cstdint>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Block Events
// Java: net.minecraftforge.event.world.BlockEvent
// ═══════════════════════════════════════════════════════════════════════════

// Block place event — fired when a block is placed by a player
// Maps to: net.minecraftforge.event.world.BlockEvent.PlaceEvent
class BlockPlaceEvent : public ForgeEvent {
public:
    BlockPlaceEvent(int32_t x, int32_t y, int32_t z,
                    int32_t blockId, int32_t meta,
                    int32_t placedAgainst, int32_t playerId)
        : x_(x), y_(y), z_(z), blockId_(blockId), meta_(meta),
          placedAgainst_(placedAgainst), playerId_(playerId) {}

    std::string getEventClass() const override {
        return "net/minecraftforge/event/world/BlockEvent$PlaceEvent";
    }
    bool isCancelable() const override { return true; }

    int32_t getX() const { return x_; }
    int32_t getY() const { return y_; }
    int32_t getZ() const { return z_; }
    int32_t getBlockId() const { return blockId_; }
    int32_t getMeta() const { return meta_; }
    int32_t getPlacedAgainst() const { return placedAgainst_; }
    int32_t getPlayerId() const { return playerId_; }

private:
    int32_t x_, y_, z_, blockId_, meta_, placedAgainst_, playerId_;
};

// Harvest drops event — modify block drops
// Maps to: net.minecraftforge.event.world.BlockEvent.HarvestDropsEvent
class HarvestDropsEvent : public ForgeEvent {
public:
    HarvestDropsEvent(int32_t x, int32_t y, int32_t z,
                      int32_t blockId, int32_t meta,
                      int32_t fortuneLevel, float dropChance)
        : x_(x), y_(y), z_(z), blockId_(blockId), meta_(meta),
          fortuneLevel_(fortuneLevel), dropChance_(dropChance) {}

    std::string getEventClass() const override {
        return "net/minecraftforge/event/world/BlockEvent$HarvestDropsEvent";
    }

    int32_t getX() const { return x_; }
    int32_t getY() const { return y_; }
    int32_t getZ() const { return z_; }
    int32_t getBlockId() const { return blockId_; }
    int32_t getMeta() const { return meta_; }
    int32_t getFortuneLevel() const { return fortuneLevel_; }
    float getDropChance() const { return dropChance_; }
    void setDropChance(float chance) { dropChance_ = chance; }

private:
    int32_t x_, y_, z_, blockId_, meta_, fortuneLevel_;
    float dropChance_;
};

// ═══════════════════════════════════════════════════════════════════════════
// World Events
// Java: net.minecraftforge.event.world.WorldEvent
// ═══════════════════════════════════════════════════════════════════════════

class WorldLoadEvent : public ForgeEvent {
public:
    explicit WorldLoadEvent(int32_t dimensionId) : dimensionId_(dimensionId) {}
    std::string getEventClass() const override {
        return "net/minecraftforge/event/world/WorldEvent$Load";
    }
    int32_t getDimensionId() const { return dimensionId_; }
private:
    int32_t dimensionId_;
};

class WorldUnloadEvent : public ForgeEvent {
public:
    explicit WorldUnloadEvent(int32_t dimensionId) : dimensionId_(dimensionId) {}
    std::string getEventClass() const override {
        return "net/minecraftforge/event/world/WorldEvent$Unload";
    }
    int32_t getDimensionId() const { return dimensionId_; }
private:
    int32_t dimensionId_;
};

class WorldSaveEvent : public ForgeEvent {
public:
    explicit WorldSaveEvent(int32_t dimensionId) : dimensionId_(dimensionId) {}
    std::string getEventClass() const override {
        return "net/minecraftforge/event/world/WorldEvent$Save";
    }
    int32_t getDimensionId() const { return dimensionId_; }
private:
    int32_t dimensionId_;
};

// ═══════════════════════════════════════════════════════════════════════════
// Chunk Events
// Java: net.minecraftforge.event.world.ChunkEvent
// ═══════════════════════════════════════════════════════════════════════════

class ChunkLoadEvent : public ForgeEvent {
public:
    ChunkLoadEvent(int32_t chunkX, int32_t chunkZ, int32_t dimensionId)
        : chunkX_(chunkX), chunkZ_(chunkZ), dimensionId_(dimensionId) {}
    std::string getEventClass() const override {
        return "net/minecraftforge/event/world/ChunkEvent$Load";
    }
    int32_t getChunkX() const { return chunkX_; }
    int32_t getChunkZ() const { return chunkZ_; }
    int32_t getDimensionId() const { return dimensionId_; }
private:
    int32_t chunkX_, chunkZ_, dimensionId_;
};

class ChunkUnloadEvent : public ForgeEvent {
public:
    ChunkUnloadEvent(int32_t chunkX, int32_t chunkZ, int32_t dimensionId)
        : chunkX_(chunkX), chunkZ_(chunkZ), dimensionId_(dimensionId) {}
    std::string getEventClass() const override {
        return "net/minecraftforge/event/world/ChunkEvent$Unload";
    }
    int32_t getChunkX() const { return chunkX_; }
    int32_t getChunkZ() const { return chunkZ_; }
    int32_t getDimensionId() const { return dimensionId_; }
private:
    int32_t chunkX_, chunkZ_, dimensionId_;
};

// ═══════════════════════════════════════════════════════════════════════════
// Explosion Events
// Java: net.minecraftforge.event.world.ExplosionEvent
// ═══════════════════════════════════════════════════════════════════════════

class ExplosionStartEvent : public ForgeEvent {
public:
    ExplosionStartEvent(double x, double y, double z, float strength)
        : x_(x), y_(y), z_(z), strength_(strength) {}
    std::string getEventClass() const override {
        return "net/minecraftforge/event/world/ExplosionEvent$Start";
    }
    bool isCancelable() const override { return true; }
    double getX() const { return x_; }
    double getY() const { return y_; }
    double getZ() const { return z_; }
    float getStrength() const { return strength_; }
private:
    double x_, y_, z_;
    float strength_;
};

class ExplosionDetonateEvent : public ForgeEvent {
public:
    ExplosionDetonateEvent(double x, double y, double z, float strength)
        : x_(x), y_(y), z_(z), strength_(strength) {}
    std::string getEventClass() const override {
        return "net/minecraftforge/event/world/ExplosionEvent$Detonate";
    }
    double getX() const { return x_; }
    double getY() const { return y_; }
    double getZ() const { return z_; }
    float getStrength() const { return strength_; }
private:
    double x_, y_, z_;
    float strength_;
};

// ═══════════════════════════════════════════════════════════════════════════
// Living Entity Events
// Java: net.minecraftforge.event.entity.living.*
// ═══════════════════════════════════════════════════════════════════════════

class LivingHurtEvent : public ForgeEvent {
public:
    LivingHurtEvent(int32_t entityId, const std::string& damageType, float amount)
        : entityId_(entityId), damageType_(damageType), amount_(amount) {}
    std::string getEventClass() const override {
        return "net/minecraftforge/event/entity/living/LivingHurtEvent";
    }
    bool isCancelable() const override { return true; }
    int32_t getEntityId() const { return entityId_; }
    const std::string& getDamageType() const { return damageType_; }
    float getAmount() const { return amount_; }
    void setAmount(float amount) { amount_ = amount; }
private:
    int32_t entityId_;
    std::string damageType_;
    float amount_;
};

class LivingDeathEvent : public ForgeEvent {
public:
    LivingDeathEvent(int32_t entityId, const std::string& damageType, int32_t sourceId)
        : entityId_(entityId), damageType_(damageType), sourceId_(sourceId) {}
    std::string getEventClass() const override {
        return "net/minecraftforge/event/entity/living/LivingDeathEvent";
    }
    bool isCancelable() const override { return true; }
    int32_t getEntityId() const { return entityId_; }
    const std::string& getDamageType() const { return damageType_; }
    int32_t getSourceId() const { return sourceId_; }
private:
    int32_t entityId_;
    std::string damageType_;
    int32_t sourceId_;
};

class LivingAttackEvent : public ForgeEvent {
public:
    LivingAttackEvent(int32_t entityId, const std::string& damageType, float amount)
        : entityId_(entityId), damageType_(damageType), amount_(amount) {}
    std::string getEventClass() const override {
        return "net/minecraftforge/event/entity/living/LivingAttackEvent";
    }
    bool isCancelable() const override { return true; }
    int32_t getEntityId() const { return entityId_; }
    const std::string& getDamageType() const { return damageType_; }
    float getAmount() const { return amount_; }
private:
    int32_t entityId_;
    std::string damageType_;
    float amount_;
};

class LivingFallEvent : public ForgeEvent {
public:
    LivingFallEvent(int32_t entityId, float distance)
        : entityId_(entityId), distance_(distance) {}
    std::string getEventClass() const override {
        return "net/minecraftforge/event/entity/living/LivingFallEvent";
    }
    bool isCancelable() const override { return true; }
    int32_t getEntityId() const { return entityId_; }
    float getDistance() const { return distance_; }
    void setDistance(float distance) { distance_ = distance; }
private:
    int32_t entityId_;
    float distance_;
};

class LivingDropsEvent : public ForgeEvent {
public:
    LivingDropsEvent(int32_t entityId, const std::string& damageType,
                     int32_t lootingLevel, bool recentlyHit)
        : entityId_(entityId), damageType_(damageType),
          lootingLevel_(lootingLevel), recentlyHit_(recentlyHit) {}
    std::string getEventClass() const override {
        return "net/minecraftforge/event/entity/living/LivingDropsEvent";
    }
    bool isCancelable() const override { return true; }
    int32_t getEntityId() const { return entityId_; }
    const std::string& getDamageType() const { return damageType_; }
    int32_t getLootingLevel() const { return lootingLevel_; }
    bool wasRecentlyHit() const { return recentlyHit_; }
private:
    int32_t entityId_;
    std::string damageType_;
    int32_t lootingLevel_;
    bool recentlyHit_;
};

// ═══════════════════════════════════════════════════════════════════════════
// Player Events
// Java: net.minecraftforge.event.entity.player.*
// ═══════════════════════════════════════════════════════════════════════════

class PlayerInteractEvent : public ForgeEvent {
public:
    enum class Action { RIGHT_CLICK_AIR, RIGHT_CLICK_BLOCK, LEFT_CLICK_BLOCK };

    PlayerInteractEvent(int32_t playerId, Action action,
                        int32_t x, int32_t y, int32_t z, int32_t face)
        : playerId_(playerId), action_(action),
          x_(x), y_(y), z_(z), face_(face) {}
    std::string getEventClass() const override {
        return "net/minecraftforge/event/entity/player/PlayerInteractEvent";
    }
    bool isCancelable() const override { return true; }
    int32_t getPlayerId() const { return playerId_; }
    Action getAction() const { return action_; }
    int32_t getX() const { return x_; }
    int32_t getY() const { return y_; }
    int32_t getZ() const { return z_; }
    int32_t getFace() const { return face_; }
private:
    int32_t playerId_;
    Action action_;
    int32_t x_, y_, z_, face_;
};

class PlayerLoggedInEvent : public ForgeEvent {
public:
    explicit PlayerLoggedInEvent(int32_t playerId, const std::string& name)
        : playerId_(playerId), name_(name) {}
    std::string getEventClass() const override {
        return "cpw/mods/fml/common/gameevent/PlayerEvent$PlayerLoggedInEvent";
    }
    int32_t getPlayerId() const { return playerId_; }
    const std::string& getName() const { return name_; }
private:
    int32_t playerId_;
    std::string name_;
};

class PlayerLoggedOutEvent : public ForgeEvent {
public:
    explicit PlayerLoggedOutEvent(int32_t playerId, const std::string& name)
        : playerId_(playerId), name_(name) {}
    std::string getEventClass() const override {
        return "cpw/mods/fml/common/gameevent/PlayerEvent$PlayerLoggedOutEvent";
    }
    int32_t getPlayerId() const { return playerId_; }
    const std::string& getName() const { return name_; }
private:
    int32_t playerId_;
    std::string name_;
};

class PlayerRespawnEvent : public ForgeEvent {
public:
    PlayerRespawnEvent(int32_t playerId, int32_t dimensionId, bool endConquered)
        : playerId_(playerId), dimensionId_(dimensionId), endConquered_(endConquered) {}
    std::string getEventClass() const override {
        return "cpw/mods/fml/common/gameevent/PlayerEvent$PlayerRespawnEvent";
    }
    int32_t getPlayerId() const { return playerId_; }
    int32_t getDimensionId() const { return dimensionId_; }
    bool isEndConquered() const { return endConquered_; }
private:
    int32_t playerId_;
    int32_t dimensionId_;
    bool endConquered_;
};

class PlayerChangedDimensionEvent : public ForgeEvent {
public:
    PlayerChangedDimensionEvent(int32_t playerId, int32_t fromDim, int32_t toDim)
        : playerId_(playerId), fromDim_(fromDim), toDim_(toDim) {}
    std::string getEventClass() const override {
        return "cpw/mods/fml/common/gameevent/PlayerEvent$PlayerChangedDimensionEvent";
    }
    int32_t getPlayerId() const { return playerId_; }
    int32_t getFromDimension() const { return fromDim_; }
    int32_t getToDimension() const { return toDim_; }
private:
    int32_t playerId_;
    int32_t fromDim_, toDim_;
};

class ItemCraftedEvent : public ForgeEvent {
public:
    ItemCraftedEvent(int32_t playerId, int32_t itemId, int32_t count, int32_t meta)
        : playerId_(playerId), itemId_(itemId), count_(count), meta_(meta) {}
    std::string getEventClass() const override {
        return "cpw/mods/fml/common/gameevent/PlayerEvent$ItemCraftedEvent";
    }
    int32_t getPlayerId() const { return playerId_; }
    int32_t getItemId() const { return itemId_; }
    int32_t getCount() const { return count_; }
    int32_t getMeta() const { return meta_; }
private:
    int32_t playerId_;
    int32_t itemId_, count_, meta_;
};

// ═══════════════════════════════════════════════════════════════════════════
// Tick Events
// Java: cpw.mods.fml.common.gameevent.TickEvent
// ═══════════════════════════════════════════════════════════════════════════

class WorldTickEvent : public ForgeEvent {
public:
    enum class Phase { START, END };
    WorldTickEvent(Phase phase, int32_t dimensionId)
        : phase_(phase), dimensionId_(dimensionId) {}
    std::string getEventClass() const override {
        return "cpw/mods/fml/common/gameevent/TickEvent$WorldTickEvent";
    }
    Phase getPhase() const { return phase_; }
    int32_t getDimensionId() const { return dimensionId_; }
private:
    Phase phase_;
    int32_t dimensionId_;
};

class PlayerTickEvent : public ForgeEvent {
public:
    enum class Phase { START, END };
    PlayerTickEvent(Phase phase, int32_t playerId)
        : phase_(phase), playerId_(playerId) {}
    std::string getEventClass() const override {
        return "cpw/mods/fml/common/gameevent/TickEvent$PlayerTickEvent";
    }
    Phase getPhase() const { return phase_; }
    int32_t getPlayerId() const { return playerId_; }
private:
    Phase phase_;
    int32_t playerId_;
};

// ═══════════════════════════════════════════════════════════════════════════
// Command Event
// Java: net.minecraftforge.event.CommandEvent
// ═══════════════════════════════════════════════════════════════════════════

class CommandEvent : public ForgeEvent {
public:
    CommandEvent(const std::string& commandName, const std::string& senderName,
                 const std::vector<std::string>& args)
        : commandName_(commandName), senderName_(senderName), args_(args) {}
    std::string getEventClass() const override {
        return "net/minecraftforge/event/CommandEvent";
    }
    bool isCancelable() const override { return true; }
    const std::string& getCommandName() const { return commandName_; }
    const std::string& getSenderName() const { return senderName_; }
    const std::vector<std::string>& getArgs() const { return args_; }
    void setArgs(const std::vector<std::string>& args) { args_ = args; }
private:
    std::string commandName_;
    std::string senderName_;
    std::vector<std::string> args_;
};

// ═══════════════════════════════════════════════════════════════════════════
// FML Lifecycle Events
// Java: cpw.mods.fml.common.event.*
// ═══════════════════════════════════════════════════════════════════════════

class FMLServerStartingEvent : public ForgeEvent {
public:
    std::string getEventClass() const override {
        return "cpw/mods/fml/common/event/FMLServerStartingEvent";
    }
};

class FMLServerStartedEvent : public ForgeEvent {
public:
    std::string getEventClass() const override {
        return "cpw/mods/fml/common/event/FMLServerStartedEvent";
    }
};

class FMLServerStoppingEvent : public ForgeEvent {
public:
    std::string getEventClass() const override {
        return "cpw/mods/fml/common/event/FMLServerStoppingEvent";
    }
};

class FMLServerStoppedEvent : public ForgeEvent {
public:
    std::string getEventClass() const override {
        return "cpw/mods/fml/common/event/FMLServerStoppedEvent";
    }
};

} // namespace mccpp
