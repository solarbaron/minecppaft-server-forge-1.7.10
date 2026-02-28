/**
 * Connection.h — Per-client connection state machine.
 *
 * Reference: net.minecraft.network.NetworkManager
 * Manages a single client through Handshake → Status → Login → Play states.
 *
 * C++ adaptation: two threads per connection (read + write), with a
 * thread-safe packet queue for outbound packets. Uses VarInt length-prefixed
 * framing per the protocol specification.
 */
#pragma once

#include <atomic>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace mccpp {

class PacketHandler; // forward decl

/**
 * Connection protocol state — mirrors the vanilla state machine.
 * Java reference: net.minecraft.network.EnumConnectionState
 */
enum class ConnectionState : int {
    Handshake = -1,
    Status    = 1,
    Login     = 2,
    Play      = 3,
};

/**
 * Connection — manages a single client socket.
 *
 * Java reference: net.minecraft.network.NetworkManager
 *
 * Thread model (multi-threaded adaptation):
 *   - Read thread:  reads VarInt-framed packets, dispatches to PacketHandler
 *   - Write thread: drains outbound queue and writes to socket
 *
 * Lifecycle:
 *   1. Constructed by TcpListener accept callback
 *   2. Call start() to spawn read/write threads
 *   3. Call disconnect() to cleanly close (sends optional kick reason)
 *   4. Destructor joins threads and closes socket
 */
class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(int socketFd, const std::string& remoteAddress, uint16_t remotePort);
    ~Connection();

    // Non-copyable
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    /**
     * Start read/write threads with the given initial handler.
     */
    void start(std::shared_ptr<PacketHandler> handler);

    /**
     * Swap the active packet handler (state transition).
     * Java reference: NetworkManager.setNetHandler()
     * Thread-safe — can be called from the read thread or externally.
     */
    void setHandler(std::shared_ptr<PacketHandler> handler);

    /**
     * Queue a raw packet (VarInt packetId + payload) for sending.
     * Java reference: NetworkManager.scheduleOutboundPacket()
     * Thread-safe.
     */
    void sendPacket(std::vector<uint8_t> data);

    /**
     * Close the connection, optionally sending a disconnect/kick reason first.
     * Java reference: NetworkManager.closeChannel()
     */
    void disconnect(const std::string& reason = "");

    /**
     * Set the protocol state (transitions Handshake→Status/Login→Play).
     * Java reference: NetworkManager.setConnectionState()
     */
    void setState(ConnectionState state);

    ConnectionState getState() const { return state_.load(std::memory_order_acquire); }
    bool isConnected() const { return connected_.load(std::memory_order_relaxed); }

    const std::string& getRemoteAddress() const { return remoteAddress_; }
    uint16_t getRemotePort() const { return remotePort_; }

    /**
     * Check if there are pending outbound packets.
     */
    bool hasOutboundData() const;

private:
    void readLoop();
    void writeLoop();
    void closeSocket();

    // Socket
    int            socketFd_;
    std::string    remoteAddress_;
    uint16_t       remotePort_;

    // State
    std::atomic<ConnectionState> state_{ConnectionState::Handshake};
    std::atomic<bool> connected_{true};

    // Threads
    std::thread readThread_;
    std::thread writeThread_;

    // Handler — mutex-protected for thread-safe swaps
    mutable std::mutex handlerMutex_;
    std::shared_ptr<PacketHandler> handler_;

    // Outbound queue (mutex-protected)
    mutable std::mutex          outMutex_;
    std::deque<std::vector<uint8_t>> outQueue_;

    // Read buffer
    static constexpr size_t READ_BUF_SIZE = 8192;
};

} // namespace mccpp
