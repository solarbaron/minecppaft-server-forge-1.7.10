/**
 * TcpListener.h — Multi-threaded TCP accept loop.
 *
 * Reference: nc.java (obfuscated ServerConnection) — uses Netty ServerBootstrap.
 * C++ adaptation: POSIX sockets + dedicated accept thread, dispatching new
 * connections to the server's connection manager.
 */
#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>

namespace mccpp {

class MinecraftServer; // forward decl

/**
 * Callback invoked when a new client socket is accepted.
 * Parameters: socket file descriptor, remote address string, remote port.
 */
using AcceptCallback = std::function<void(int fd, const std::string& address, uint16_t port)>;

/**
 * TcpListener — binds to a port and runs an async accept loop on a
 * dedicated thread.
 *
 * Lifecycle:
 *   1. Construct with bind address + port
 *   2. Call start() to begin accepting
 *   3. Call stop() to shut down (blocks until accept thread exits)
 */
class TcpListener {
public:
    TcpListener(const std::string& bindAddress, uint16_t port);
    ~TcpListener();

    // Non-copyable, non-movable
    TcpListener(const TcpListener&) = delete;
    TcpListener& operator=(const TcpListener&) = delete;

    /**
     * Set the callback for accepted connections. Must be set before start().
     */
    void setAcceptCallback(AcceptCallback callback);

    /**
     * Bind and start the accept loop on a background thread.
     * Returns true on success, false if bind/listen fails.
     */
    bool start();

    /**
     * Signal the accept loop to stop and join the thread.
     */
    void stop();

    bool isRunning() const { return running_.load(std::memory_order_relaxed); }

    uint16_t getPort() const { return port_; }

private:
    void acceptLoop();

    std::string    bindAddress_;
    uint16_t       port_;
    int            serverFd_ = -1;
    std::atomic<bool> running_{false};
    std::thread    acceptThread_;
    AcceptCallback onAccept_;
};

} // namespace mccpp
