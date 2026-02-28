/**
 * EntityPlayerMP.h — Server-side player entity.
 *
 * Java reference: net.minecraft.entity.player.EntityPlayerMP
 *
 * The central class tying together:
 *   - Networking (NetHandlerPlayServer / playerNetServerHandler)
 *   - Inventory + Container (openContainer, inventoryContainer)
 *   - Position tracking (managedPosX/Z)
 *   - Chunk loading queue (loadedChunks)
 *   - Health/food/XP change detection for efficient sync
 *   - GUI window management (windowId cycling)
 *   - PVP, dimension travel, death, combat
 *
 * Thread safety: Each EntityPlayerMP is owned by one connection thread.
 * Shared access (e.g., from entity tracker) uses atomic fields where needed.
 *
 * JNI readiness: Simple fields, no opaque pointers.
 */
#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>
#include <limits>
#include <mutex>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// ChunkCoordIntPair — (chunkX, chunkZ) pair for chunk loading queue.
// Java reference: net.minecraft.world.ChunkCoordIntPair
// ═══════════════════════════════════════════════════════════════════════════

struct ChunkCoordIntPair {
    int32_t chunkX = 0;
    int32_t chunkZ = 0;

    bool operator==(const ChunkCoordIntPair& o) const {
        return chunkX == o.chunkX && chunkZ == o.chunkZ;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GameProfile — Player identity (UUID + name).
// Java reference: com.mojang.authlib.GameProfile
// ═══════════════════════════════════════════════════════════════════════════

struct GameProfile {
    std::string uuid;
    std::string name;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityPlayerMP — Server-side player entity.
// Java reference: net.minecraft.entity.player.EntityPlayerMP
// ═══════════════════════════════════════════════════════════════════════════

class EntityPlayerMP {
public:
    // ─── Identity ───
    int32_t entityId = 0;
    GameProfile gameProfile;
    std::string translator = "en_US";

    // ─── Position and rotation ───
    double posX = 0.0, posY = 0.0, posZ = 0.0;
    float rotationYaw = 0.0f, rotationPitch = 0.0f;
    bool onGround = false;

    // Java: managedPosX/Z — last known "acknowledged" position for chunk loading
    double managedPosX = 0.0;
    double managedPosZ = 0.0;

    // ─── Dimension ───
    int32_t dimension = 0;             // 0=overworld, -1=nether, 1=end

    // ─── Health, food, experience ───
    float health = 20.0f;
    float maxHealth = 20.0f;
    float absorptionAmount = 0.0f;
    int32_t foodLevel = 20;
    float saturationLevel = 5.0f;
    float experience = 0.0f;          // 0.0-1.0 bar progress
    int32_t experienceLevel = 0;
    int32_t experienceTotal = 0;

    // Java: Change tracking for efficient sync
    float lastHealth = -1.0e8f;
    int32_t lastFoodLevel = -99999999;
    bool wasHungry = true;
    int32_t lastExperience = -99999999;
    float lastHealthPlusAbsorption = std::numeric_limits<float>::min();

    // ─── Combat ───
    int32_t hurtResistantTime = 0;     // Invulnerability ticks after damage
    int32_t deathTime = 0;
    bool isDead = false;
    int32_t scoreValue = 0;

    // Java: field_147101_bU — invulnerability timer on join (60 ticks)
    int32_t joinInvulnerabilityTimer = 60;

    // ─── Player state ───
    bool sleeping = false;
    bool sneaking = false;
    bool sprinting = false;
    bool isJumping = false;
    float moveStrafing = 0.0f;
    float moveForward = 0.0f;
    int32_t ticksExisted = 0;

    // ─── Capabilities (creative, flying, etc.) ───
    bool isCreativeMode = false;
    bool isFlying = false;
    bool allowFlying = false;
    bool invulnerable = false;
    float flySpeed = 0.05f;           // Default fly speed
    float walkSpeed = 0.1f;           // Default walk speed

    // ─── Chunk loading ───
    std::deque<ChunkCoordIntPair> loadedChunks;    // Chunks queued for sending
    std::vector<int32_t> destroyedItemsNetCache;   // Entity IDs to destroy

    // ─── GUI / Container ───
    int32_t currentWindowId = 0;
    bool isChangingQuantityOnly = false;

    // ─── Chat settings ───
    int32_t chatVisibility = 0;         // 0=full, 1=commands, 2=hidden
    bool chatColours = true;

    // ─── Networking ───
    std::atomic<int32_t> ping{0};       // Latency in ms
    bool playerConqueredTheEnd = false;
    int64_t playerLastActiveTime = 0;   // Milliseconds

    // ─── Constructors ───

    EntityPlayerMP() = default;

    EntityPlayerMP(int32_t entityId, const GameProfile& profile, int32_t dim)
        : entityId(entityId), gameProfile(profile), dimension(dim) {}

    // ─── Core methods ───

    // Java: getEyeHeight — always 1.62f for players
    float getEyeHeight() const { return 1.62f; }

    // Java: getNextWindowId — cycles 1-100
    int32_t getNextWindowId() {
        currentWindowId = currentWindowId % 100 + 1;
        return currentWindowId;
    }

    // Java: isEntityInvulnerable
    bool isEntityInvulnerable() const {
        return invulnerable;
    }

    // Java: getPlayerIP — extracted from net handler
    std::string playerIP;

    // Java: getCommandSenderName
    const std::string& getCommandSenderName() const {
        return gameProfile.name;
    }

    // ─── Position ───

    void setPosition(double x, double y, double z) {
        posX = x; posY = y; posZ = z;
    }

    void setLocationAndAngles(double x, double y, double z, float yaw, float pitch) {
        posX = x; posY = y; posZ = z;
        rotationYaw = yaw; rotationPitch = pitch;
    }

    void setPositionAndUpdate(double x, double y, double z) {
        posX = x; posY = y; posZ = z;
        // In full implementation: sends S08PacketPlayerPosLook
    }

    // ─── Health / Damage ───

    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getAbsorptionAmount() const { return absorptionAmount; }
    void setAbsorptionAmount(float amount) { absorptionAmount = std::max(0.0f, amount); }

    // Java: setPlayerHealthUpdated — force health resync
    void setPlayerHealthUpdated() {
        lastHealth = -1.0e8f;
    }

    // Java: attackEntityFrom — with PVP checking
    bool attackEntityFrom(const std::string& damageType, float amount, bool pvpEnabled) {
        if (isEntityInvulnerable()) return false;

        // Java: joinInvulnerabilityTimer > 0 && not outOfWorld
        if (joinInvulnerabilityTimer > 0 && damageType != "outOfWorld") {
            return false;
        }

        // PVP check is done at higher level
        health -= amount;
        if (health <= 0.0f) {
            health = 0.0f;
            onDeath(damageType);
        }
        return true;
    }

    // Java: onDeath
    void onDeath(const std::string& /*damageType*/) {
        isDead = true;
        // In full implementation: drop inventory if !keepInventory,
        // update scoreboard, send death message
    }

    // ─── Experience ───

    // Java: addExperienceLevel
    void addExperienceLevel(int32_t levels) {
        experienceLevel += levels;
        if (experienceLevel < 0) {
            experienceLevel = 0;
            experience = 0.0f;
            experienceTotal = 0;
        }
        lastExperience = -1; // Force resync
    }

    // Java: xpBarCap — XP needed for next level
    int32_t xpBarCap() const {
        if (experienceLevel >= 30) return 112 + (experienceLevel - 30) * 9;
        if (experienceLevel >= 15) return 37 + (experienceLevel - 15) * 5;
        return 7 + experienceLevel * 2;
    }

    // ─── Tick logic ───

    // Java: onUpdate — main server-side tick
    void onUpdate() {
        --joinInvulnerabilityTimer;
        if (hurtResistantTime > 0) --hurtResistantTime;

        // Container change detection happens here
        // Chunk sending happens here
        // Entity destruction cache flushing happens here

        ++ticksExisted;
    }

    // Java: onUpdateEntity — called after super.onUpdate()
    // Returns what changed so the caller can send appropriate packets.
    struct TickSyncResult {
        bool healthChanged = false;
        bool experienceChanged = false;
        float newHealth = 0.0f;
        int32_t newFoodLevel = 0;
        float newSaturation = 0.0f;
        float newExperienceBar = 0.0f;
        int32_t newExperienceTotal = 0;
        int32_t newExperienceLevel = 0;
    };

    TickSyncResult checkForSyncUpdates() {
        TickSyncResult result;

        // Java: health/food/saturation sync
        if (health != lastHealth ||
            foodLevel != lastFoodLevel ||
            (saturationLevel == 0.0f) != wasHungry) {

            result.healthChanged = true;
            result.newHealth = health;
            result.newFoodLevel = foodLevel;
            result.newSaturation = saturationLevel;

            lastHealth = health;
            lastFoodLevel = foodLevel;
            wasHungry = (saturationLevel == 0.0f);
        }

        // Java: experience sync
        if (experienceTotal != lastExperience) {
            result.experienceChanged = true;
            result.newExperienceBar = experience;
            result.newExperienceTotal = experienceTotal;
            result.newExperienceLevel = experienceLevel;
            lastExperience = experienceTotal;
        }

        // Java: health + absorption for scoreboard
        if (health + absorptionAmount != lastHealthPlusAbsorption) {
            lastHealthPlusAbsorption = health + absorptionAmount;
            // Update scoreboard health objective
        }

        return result;
    }

    // ─── Player state ───

    void setSneaking(bool sneak) { sneaking = sneak; }
    bool isSneaking() const { return sneaking; }

    void setSprinting(bool sprint) { sprinting = sprint; }
    bool isSprinting() const { return sprinting; }

    // Java: setEntityActionState — from C0CPacketInput
    void setEntityActionState(float strafe, float forward, bool jump, bool sneak) {
        // Only applies when riding
        if (strafe >= -1.0f && strafe <= 1.0f) moveStrafing = strafe;
        if (forward >= -1.0f && forward <= 1.0f) moveForward = forward;
        isJumping = jump;
        setSneaking(sneak);
    }

    // ─── Chat settings ───

    // Java: func_147100_a — handle C15PacketClientSettings
    void handleClientSettings(const std::string& lang, int32_t viewDistance,
                               int32_t chatFlags, bool colors) {
        translator = lang;
        chatVisibility = chatFlags;
        chatColours = colors;
        // View distance clamping happens at server level
        (void)viewDistance;
    }

    // ─── Player abilities ───

    // Returns flags byte for S39PacketPlayerAbilities
    uint8_t getAbilitiesFlags() const {
        uint8_t flags = 0;
        if (invulnerable) flags |= 0x01;
        if (isFlying) flags |= 0x02;
        if (allowFlying) flags |= 0x04;
        if (isCreativeMode) flags |= 0x08;
        return flags;
    }

    // Set from C13PacketPlayerAbilities
    void setAbilitiesFromPacket(uint8_t flags, float fly, float walk) {
        // Only allow changing flying state if allowed
        if (allowFlying) {
            isFlying = (flags & 0x02) != 0;
        } else {
            isFlying = false;
        }
        // flySpeed and walkSpeed are server-authoritative
        (void)fly;
        (void)walk;
    }

    // ─── Gamemode ───

    // Java: setGameType
    void setGameType(int32_t gameType) {
        isCreativeMode = (gameType == 1);
        allowFlying = isCreativeMode;
        invulnerable = isCreativeMode;
        if (!isCreativeMode) {
            isFlying = false;
        }
    }

    int32_t getGameType() const {
        if (isCreativeMode) return 1;
        return 0; // Survival by default
    }

    // ─── Command permissions ───

    // Java: canCommandSenderUseCommand
    bool canUseCommand(int32_t permLevel, const std::string& command,
                        bool isDedicatedServer, bool isOp, int32_t opLevel) const {
        // seed is always available on integrated server
        if (command == "seed" && !isDedicatedServer) return true;
        // tell, help, me always available
        if (command == "tell" || command == "help" || command == "me") return true;
        if (isOp) return opLevel >= permLevel;
        return false;
    }

    // ─── Dimension travel ───

    // Java: travelToDimension — handles achievements
    struct DimensionTravelResult {
        bool conqueredEnd = false;
        int32_t targetDimension = 0;
        bool shouldTransfer = false;
    };

    DimensionTravelResult travelToDimension(int32_t targetDim) {
        DimensionTravelResult result;
        result.targetDimension = targetDim;

        if (dimension == 1 && targetDim == 1) {
            // Returning from End
            playerConqueredTheEnd = true;
            result.conqueredEnd = true;
            // Send ChangeGameState(4, 0) — show end credits
        } else {
            result.shouldTransfer = true;
            // Reset sync tracking
            lastExperience = -1;
            lastHealth = -1.0f;
            lastFoodLevel = -1;
        }
        return result;
    }

    // ─── Clone player (respawn) ───
    // Java: clonePlayer
    void cloneFrom(const EntityPlayerMP& other, bool keepEverything) {
        if (keepEverything) {
            // Copy all state (e.g., dimension change)
            health = other.health;
            foodLevel = other.foodLevel;
            saturationLevel = other.saturationLevel;
            experienceLevel = other.experienceLevel;
            experienceTotal = other.experienceTotal;
            experience = other.experience;
        }
        // Always reset sync tracking
        lastExperience = -1;
        lastHealth = -1.0f;
        lastFoodLevel = -1;
        // Copy destroyed items cache
        destroyedItemsNetCache.insert(destroyedItemsNetCache.end(),
            other.destroyedItemsNetCache.begin(),
            other.destroyedItemsNetCache.end());
    }

    // ─── Death / Sleep helpers ───

    // Java: mountEntityAndWakeUp
    void mountEntityAndWakeUp() {
        if (sleeping) {
            sleeping = false;
            // wakeUpPlayer(true, false, false)
        }
    }

    // ─── Packet sending callback ───
    // The actual network handler will be set by the connection system.
    // This callback pattern avoids a circular dependency.
    using SendPacketFn = std::function<void(const std::vector<uint8_t>&)>;
    SendPacketFn sendPacket;

    // ─── Entity destruction queueing ───

    // Java: destroyedItemsNetCache — entities to tell client to remove
    void queueEntityDestruction(int32_t entityId) {
        destroyedItemsNetCache.push_back(entityId);
    }

    // Java: flush up to 127 at a time
    std::vector<int32_t> flushDestroyedEntities() {
        std::vector<int32_t> batch;
        size_t count = std::min<size_t>(destroyedItemsNetCache.size(), 127);
        if (count > 0) {
            batch.assign(destroyedItemsNetCache.begin(),
                         destroyedItemsNetCache.begin() + static_cast<ptrdiff_t>(count));
            destroyedItemsNetCache.erase(destroyedItemsNetCache.begin(),
                                          destroyedItemsNetCache.begin() + static_cast<ptrdiff_t>(count));
        }
        return batch;
    }
};

} // namespace mccpp
