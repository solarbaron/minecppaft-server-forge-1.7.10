/**
 * NetHandlerPlayServer.h — Server-side play packet processing.
 *
 * Java reference: net.minecraft.network.NetHandlerPlayServer (910 lines)
 *
 * This is the core packet handler class that processes all client-to-server
 * play packets. It implements movement validation, anti-cheat, and bridges
 * packet data to game logic via EntityPlayerMP.
 *
 * Key anti-cheat constants and algorithms:
 *   - Movement speed: sqrt(delta²) > 100 → "moved too quickly"
 *   - Wrong move check: delta² > 0.0625 after moveEntity
 *   - Illegal stance: (stance - y) must be 0.1 - 1.65
 *   - Illegal position: |x| or |z| > 3.2E7
 *   - Floating: 80 ticks without ground contact → kick
 *   - Flying detection: motionY >= -0.03125 while in air
 *   - Digging reach: distance² < 36.0 (6 blocks)
 *   - Placement reach: distance² < 64.0 (8 blocks)
 *   - Entity interact reach: seen=36.0, unseen=9.0
 *   - Chat spam: +20 per message, threshold 200 → kick
 *   - Item drop throttle: +20 per drop, cap 200
 *
 * Thread safety: One handler per connection, ticked on server thread.
 * JNI readiness: Simple state with clear lifecycle.
 */
#pragma once

#include <cstdint>
#include <string>
#include <chrono>
#include <unordered_map>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// NetHandlerPlayServer constants
// Java: NetHandlerPlayServer fields and magic numbers
// ═══════════════════════════════════════════════════════════════════════════

namespace NetPlayConstants {
    // ─── Keep-Alive ───
    // Java: if ((long)networkTickCount - lastSentPingPacket > 40L)
    static constexpr int64_t KEEPALIVE_INTERVAL_TICKS = 40;

    // ─── Ping calculation ───
    // Java: ping = (ping * 3 + n) / 4  (exponential moving average)
    static constexpr int32_t PING_WEIGHT_OLD = 3;
    static constexpr int32_t PING_WEIGHT_TOTAL = 4;

    // ─── Movement validation ───
    // Java: if (d4 > 1.65 || d4 < 0.1)  [stance - y]
    static constexpr double STANCE_MAX = 1.65;
    static constexpr double STANCE_MIN = 0.1;

    // Java: if (Math.abs(x) > 3.2E7 || Math.abs(z) > 3.2E7)
    static constexpr double POSITION_LIMIT = 3.2E7;

    // Java: if (d14 > 100.0)  [squared move distance]
    static constexpr double SPEED_CHECK_THRESHOLD_SQ = 100.0;

    // Java: if (d14 > 0.0625)  [wrong move detection]
    static constexpr double WRONG_MOVE_THRESHOLD_SQ = 0.0625;

    // Java: float f4 = 0.0625f  [collision contract margin]
    static constexpr float COLLISION_MARGIN = 0.0625f;

    // Java: if (d11 > -0.5 || d11 < 0.5) d11 = 0  [vertical snap]
    static constexpr double VERTICAL_SNAP = 0.5;

    // ─── Flying detection ───
    // Java: if (d15 >= -0.03125) ++floatingTickCount
    static constexpr double FLOAT_VELOCITY_THRESHOLD = -0.03125;

    // Java: if (floatingTickCount > 80) kick
    static constexpr int32_t MAX_FLOATING_TICKS = 80;

    // Java: expand(f4, f4, f4).addCoord(0.0, -0.55, 0.0)
    static constexpr double GROUND_CHECK_Y = -0.55;

    // ─── Digging reach ───
    // Java: if (d4 > 36.0)  [distance² from block center]
    static constexpr double DIGGING_REACH_SQ = 36.0;

    // Java: double d2 = posY - (y + 0.5) + 1.5
    static constexpr double DIGGING_Y_OFFSET = 1.5;

    // ─── Block placement ───
    // Java: getDistanceSq(x + 0.5, y + 0.5, z + 0.5) < 64.0
    static constexpr double PLACEMENT_REACH_SQ = 64.0;

    // Java: direction == 255 → tryUseItem (right-click air)
    static constexpr int32_t PLACEMENT_USE_ITEM = 255;

    // ─── Entity interaction reach ───
    // Java: seen=36.0, unseen=9.0  [distance²]
    static constexpr double ENTITY_INTERACT_SEEN_SQ = 36.0;
    static constexpr double ENTITY_INTERACT_UNSEEN_SQ = 9.0;

    // ─── Chat spam ───
    // Java: chatSpamThresholdCount += 20; if (> 200) kick
    static constexpr int32_t CHAT_COST = 20;
    static constexpr int32_t CHAT_SPAM_LIMIT = 200;

    // ─── Item drop throttle ───
    // Java: itemDropThreshold += 20; cap 200
    static constexpr int32_t DROP_COST = 20;
    static constexpr int32_t DROP_LIMIT = 200;

    // ─── Idle timeout ───
    // Java: getMaxPlayerIdleMinutes() * 1000 * 60
    static constexpr int64_t IDLE_TIMEOUT_MULTIPLIER = 1000 * 60;

    // ─── Player movement packets ───
    // Java: S08PacketPlayerPosLook adds +1.62f to y (eye height)
    static constexpr float EYE_HEIGHT_OFFSET = 1.62f;

    // Java: -999.0 sentinel for "no position update"
    static constexpr double NO_POS_SENTINEL = -999.0;

    // ─── Hotbar validation ───
    // Java: slot < 0 || slot >= InventoryPlayer.getHotbarSize()
    static constexpr int32_t HOTBAR_SIZE = 9;

    // ─── Creative drop validation ───
    // Java: stackSize <= 64 && stackSize > 0
    static constexpr int32_t MAX_STACK_SIZE = 64;
}

// ═══════════════════════════════════════════════════════════════════════════
// NetHandlerPlayServer — Per-connection state and packet handler.
// ═══════════════════════════════════════════════════════════════════════════

class NetHandlerPlayServer {
public:
    // ─── State ───
    int32_t networkTickCount = 0;
    int32_t floatingTickCount = 0;
    bool receivedMovePacket = false;  // field_147366_g
    int32_t lastKeepAliveId = 0;     // field_147378_h
    int64_t lastPingTime = 0;
    int64_t lastSentPingPacket = 0;
    int32_t chatSpamThresholdCount = 0;
    int32_t itemDropThreshold = 0;
    double lastPosX = 0, lastPosY = 0, lastPosZ = 0;
    bool hasMoved = true;

    // ─── Tick update ───
    // Java: onNetworkTick() — called every server tick
    //   1. Send keep-alive every 40 ticks
    //   2. Decrement chat spam counter
    //   3. Decrement item drop counter
    //   4. Check idle timeout
    void onNetworkTick() {
        receivedMovePacket = false;
        ++networkTickCount;

        // Keep-alive
        if (static_cast<int64_t>(networkTickCount) - lastSentPingPacket
                > NetPlayConstants::KEEPALIVE_INTERVAL_TICKS) {
            lastSentPingPacket = networkTickCount;
            lastPingTime = currentTimeMillis();
            lastKeepAliveId = static_cast<int32_t>(lastPingTime);
            // Send S00PacketKeepAlive(lastKeepAliveId)
        }

        // Spam decay
        if (chatSpamThresholdCount > 0) --chatSpamThresholdCount;
        if (itemDropThreshold > 0) --itemDropThreshold;
    }

    // ─── Ping update ───
    // Java: processKeepAlive — exponential moving average
    void processKeepAlive(int32_t receivedId, int32_t& playerPing) {
        if (receivedId == lastKeepAliveId) {
            int32_t delta = static_cast<int32_t>(currentTimeMillis() - lastPingTime);
            playerPing = (playerPing * NetPlayConstants::PING_WEIGHT_OLD + delta)
                         / NetPlayConstants::PING_WEIGHT_TOTAL;
        }
    }

    // ─── Movement validation ───
    // Returns true if movement is valid, false if player should be teleported back
    bool validateMovement(double newX, double newY, double newZ,
                          double stance, double motionX, double motionY,
                          double motionZ, bool onGround, bool isFirstMove) const
    {
        // Stance check
        double stanceHeight = stance - newY;
        if (stanceHeight > NetPlayConstants::STANCE_MAX ||
            stanceHeight < NetPlayConstants::STANCE_MIN) {
            return false;  // "Illegal stance"
        }

        // Position limit
        if (std::abs(newX) > NetPlayConstants::POSITION_LIMIT ||
            std::abs(newZ) > NetPlayConstants::POSITION_LIMIT) {
            return false;  // "Illegal position"
        }

        // Speed check
        double dx = newX - lastPosX;
        double dy = newY - lastPosY;
        double dz = newZ - lastPosZ;
        double clampedDx = std::min(std::abs(dx), std::abs(motionX));
        double clampedDy = std::min(std::abs(dy), std::abs(motionY));
        double clampedDz = std::min(std::abs(dz), std::abs(motionZ));
        double speedSq = clampedDx * clampedDx + clampedDy * clampedDy + clampedDz * clampedDz;

        if (speedSq > NetPlayConstants::SPEED_CHECK_THRESHOLD_SQ) {
            return false;  // "moved too quickly"
        }

        return true;
    }

    // ─── Digging reach check ───
    bool isWithinDiggingReach(double playerX, double playerY, double playerZ,
                              int32_t blockX, int32_t blockY, int32_t blockZ) const
    {
        double dx = playerX - (static_cast<double>(blockX) + 0.5);
        double dy = playerY - (static_cast<double>(blockY) + 0.5)
                    + NetPlayConstants::DIGGING_Y_OFFSET;
        double dz = playerZ - (static_cast<double>(blockZ) + 0.5);
        return (dx * dx + dy * dy + dz * dz) <= NetPlayConstants::DIGGING_REACH_SQ;
    }

    // ─── Placement reach check ───
    bool isWithinPlacementReach(double playerX, double playerY, double playerZ,
                                int32_t blockX, int32_t blockY, int32_t blockZ) const
    {
        double dx = playerX - (static_cast<double>(blockX) + 0.5);
        double dy = playerY - (static_cast<double>(blockY) + 0.5);
        double dz = playerZ - (static_cast<double>(blockZ) + 0.5);
        return (dx * dx + dy * dy + dz * dz) < NetPlayConstants::PLACEMENT_REACH_SQ;
    }

    // ─── Entity interaction reach check ───
    bool isWithinEntityReach(double distanceSq, bool canSee) const {
        double reach = canSee ? NetPlayConstants::ENTITY_INTERACT_SEEN_SQ
                              : NetPlayConstants::ENTITY_INTERACT_UNSEEN_SQ;
        return distanceSq < reach;
    }

    // ─── Chat spam check ───
    bool processChatSpam() {
        chatSpamThresholdCount += NetPlayConstants::CHAT_COST;
        return chatSpamThresholdCount <= NetPlayConstants::CHAT_SPAM_LIMIT;
    }

    // ─── Item drop throttle ───
    bool processItemDrop() {
        if (itemDropThreshold >= NetPlayConstants::DROP_LIMIT) return false;
        itemDropThreshold += NetPlayConstants::DROP_COST;
        return true;
    }

    // ─── Hotbar validation ───
    static bool isValidHotbarSlot(int16_t slot) {
        return slot >= 0 && slot < NetPlayConstants::HOTBAR_SIZE;
    }

    // ─── Position sync ───
    // Java: setPlayerLocation — teleport player, reset hasMoved
    void setPlayerLocation(double x, double y, double z) {
        hasMoved = false;
        lastPosX = x;
        lastPosY = y;
        lastPosZ = z;
        // Send S08PacketPlayerPosLook(x, y + EYE_HEIGHT_OFFSET, z, yaw, pitch, false)
    }

    // ─── Block placement direction offsets ───
    // Java: processPlayerBlockPlacement lines 407-424
    // After placement, send block change for both the target block and
    // the adjacent block in the placement direction
    struct DirectionOffset {
        int32_t dx, dy, dz;
    };

    static DirectionOffset getDirectionOffset(int32_t direction) {
        switch (direction) {
            case 0: return {0, -1, 0};   // Down
            case 1: return {0, +1, 0};   // Up
            case 2: return {0, 0, -1};   // North
            case 3: return {0, 0, +1};   // South
            case 4: return {-1, 0, 0};   // West
            case 5: return {+1, 0, 0};   // East
            default: return {0, 0, 0};
        }
    }

private:
    int64_t currentTimeMillis() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()
        ).count();
    }
};

} // namespace mccpp
