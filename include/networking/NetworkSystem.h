/**
 * NetworkSystem.h — Server-side connection lifecycle manager.
 *
 * Java reference: net.minecraft.network.NetworkSystem (112 lines)
 *
 * Manages all active client connections and their lifecycle:
 *   1. addLanEndpoint() — bind listening socket
 *   2. networkTick() — called every server tick:
 *      a. Remove closed connections (call onDisconnect handler)
 *      b. Process received packets on open connections
 *      c. Handle exceptions (disconnect with "Internal server error")
 *   3. terminateEndpoints() — clean shutdown
 *
 * Key behaviors:
 *   - Synchronized list of NetworkManagers
 *   - Closed connections trigger handler.onDisconnect(exitMessage)
 *   - Packet processing exceptions on non-local channels log warning
 *     and send S40PacketDisconnect("Internal server error")
 *   - Local channel exceptions are fatal (crash report)
 *
 * Thread safety: networkManagers list is synchronized.
 * JNI readiness: Simple lifecycle manager.
 */
#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace mccpp {

// Forward declarations
class Connection;

// ═══════════════════════════════════════════════════════════════════════════
// NetworkSystem — manages all active client connections.
// ═══════════════════════════════════════════════════════════════════════════

class NetworkSystem {
public:
    std::atomic<bool> isAlive{true};

    // ─── Add a new connection ───
    // Java: NetworkSystem$1 creates NetworkManager and adds to list
    // Called by TcpListener accept callback
    void addConnection(std::shared_ptr<Connection> connection) {
        std::lock_guard<std::mutex> lock(mutex_);
        connections_.push_back(std::move(connection));
    }

    // ─── Network tick ───
    // Java: networkTick() — called every server tick on the main thread
    //
    // For each connection:
    //   1. If not connected: remove and call onDisconnect
    //   2. If connected: processReceivedPackets()
    //      On exception: disconnect with "Internal server error"
    //
    // Returns: number of active connections after tick
    int32_t networkTick() {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = connections_.begin();
        while (it != connections_.end()) {
            auto& conn = *it;

            if (!conn || !isConnectionOpen(conn)) {
                // Connection closed — notify handler and remove
                if (disconnectCallback_) {
                    disconnectCallback_(conn);
                }
                it = connections_.erase(it);
                continue;
            }

            // Process received packets
            bool success = processConnection(conn);
            if (!success) {
                // Exception during processing — disconnect
                if (errorCallback_) {
                    errorCallback_(conn, "Internal server error");
                }
                it = connections_.erase(it);
                continue;
            }

            ++it;
        }

        return static_cast<int32_t>(connections_.size());
    }

    // ─── Terminate all endpoints ───
    // Java: terminateEndpoints()
    void terminateEndpoints() {
        isAlive.store(false, std::memory_order_release);
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& conn : connections_) {
            if (disconnectCallback_) {
                disconnectCallback_(conn);
            }
        }
        connections_.clear();
    }

    // ─── Connection count ───
    int32_t getConnectionCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return static_cast<int32_t>(connections_.size());
    }

    // ─── Callbacks ───
    // Set by the server to hook into connection lifecycle events
    using ConnectionCallback = std::function<void(std::shared_ptr<Connection>&)>;
    using ErrorCallback = std::function<void(std::shared_ptr<Connection>&, const std::string&)>;

    void setDisconnectCallback(ConnectionCallback cb) { disconnectCallback_ = std::move(cb); }
    void setErrorCallback(ErrorCallback cb) { errorCallback_ = std::move(cb); }

    // ─── Connection processing ───
    // Override points for processing connections
    // Default implementations check Connection state
    using ProcessCallback = std::function<bool(std::shared_ptr<Connection>&)>;
    using OpenCheck = std::function<bool(const std::shared_ptr<Connection>&)>;

    void setProcessCallback(ProcessCallback cb) { processCallback_ = std::move(cb); }
    void setOpenCheck(OpenCheck cb) { openCheck_ = std::move(cb); }

private:
    bool isConnectionOpen(const std::shared_ptr<Connection>& conn) const {
        if (openCheck_) return openCheck_(conn);
        return true;  // Default: assume open
    }

    bool processConnection(std::shared_ptr<Connection>& conn) {
        if (processCallback_) return processCallback_(conn);
        return true;  // Default: no-op success
    }

    mutable std::mutex mutex_;
    std::vector<std::shared_ptr<Connection>> connections_;

    ConnectionCallback disconnectCallback_;
    ErrorCallback errorCallback_;
    ProcessCallback processCallback_;
    OpenCheck openCheck_;
};

} // namespace mccpp
