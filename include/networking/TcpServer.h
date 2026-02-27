#pragma once
// TcpServer — listens on a TCP port, accepts connections, and drives the
// network tick loop. Equivalent to the Netty ServerBootstrap in the Java server.

#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <iostream>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <unordered_map>

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "networking/Connection.h"

namespace mc {

// Callback types
using PacketCallback = std::function<void(Connection&, PacketBuffer&)>;
using DisconnectCallback = std::function<void(int fd)>;
using TickCallback = std::function<void(Connection&)>;

class TcpServer {
public:
    TcpServer(const std::string& bindAddress, uint16_t port)
        : bindAddress_(bindAddress), port_(port) {}

    ~TcpServer() {
        stop();
    }

    bool start() {
        listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (listenFd_ < 0) {
            std::cerr << "[NET] Failed to create socket: " << strerror(errno) << "\n";
            return false;
        }

        int opt = 1;
        setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        if (bindAddress_ == "0.0.0.0" || bindAddress_.empty()) {
            addr.sin_addr.s_addr = INADDR_ANY;
        } else {
            inet_pton(AF_INET, bindAddress_.c_str(), &addr.sin_addr);
        }

        if (bind(listenFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            std::cerr << "[NET] Failed to bind to " << bindAddress_ << ":" << port_
                      << " - " << strerror(errno) << "\n";
            ::close(listenFd_);
            listenFd_ = -1;
            return false;
        }

        if (listen(listenFd_, 128) < 0) {
            std::cerr << "[NET] Failed to listen: " << strerror(errno) << "\n";
            ::close(listenFd_);
            listenFd_ = -1;
            return false;
        }

        // Non-blocking listen socket
        int flags = fcntl(listenFd_, F_GETFL, 0);
        fcntl(listenFd_, F_SETFL, flags | O_NONBLOCK);

        running_ = true;
        std::cout << "[NET] Listening on " << bindAddress_ << ":" << port_ << "\n";
        return true;
    }

    void stop() {
        running_ = false;
        if (listenFd_ >= 0) {
            ::close(listenFd_);
            listenFd_ = -1;
        }
        connections_.clear();
    }

    // Called once per server tick (~50ms). Accepts new connections,
    // reads data, extracts packets, calls per-connection tick, and flushes.
    void tick(PacketCallback packetHandler,
              TickCallback connectionTick = nullptr,
              DisconnectCallback disconnectHandler = nullptr) {
        if (!running_) return;

        acceptNewConnections();

        // Collect fds to process (iterate copy to allow safe removal)
        std::vector<int> fds;
        fds.reserve(connections_.size());
        for (auto& [fd, _] : connections_) {
            fds.push_back(fd);
        }

        for (int fd : fds) {
            auto it = connections_.find(fd);
            if (it == connections_.end()) continue;
            auto& conn = it->second;

            // Receive
            if (!conn.recv()) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    conn.close();
                    if (disconnectHandler) disconnectHandler(fd);
                    connections_.erase(it);
                    continue;
                }
            }

            // Extract and handle packets
            PacketBuffer packet;
            while (conn.tryReadPacket(packet)) {
                try {
                    packetHandler(conn, packet);
                } catch (const std::exception& e) {
                    std::cerr << "[NET] Error handling packet from "
                              << conn.address() << ": " << e.what() << "\n";
                    conn.close();
                    break;
                }
            }

            // Per-connection tick (keep alive, etc.)
            if (!conn.isClosed() && connectionTick) {
                connectionTick(conn);
            }

            // Flush send queue
            if (!conn.isClosed()) {
                if (!conn.flush()) {
                    conn.close();
                }
            }

            // Remove closed connections
            if (conn.isClosed()) {
                if (disconnectHandler) disconnectHandler(fd);
                connections_.erase(fd);
            }
        }
    }

    size_t connectionCount() const { return connections_.size(); }
    bool isRunning() const { return running_; }

    // Access connections map (for broadcasting)
    std::unordered_map<int, Connection>& connections() { return connections_; }
    const std::unordered_map<int, Connection>& connections() const { return connections_; }

private:
    void acceptNewConnections() {
        while (true) {
            sockaddr_in clientAddr{};
            socklen_t clientLen = sizeof(clientAddr);
            int clientFd = accept(listenFd_,
                                   reinterpret_cast<sockaddr*>(&clientAddr),
                                   &clientLen);
            if (clientFd < 0) {
                break; // No more pending connections (non-blocking)
            }

            // Set TCP_NODELAY (like Java Netty does)
            int opt = 1;
            setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));

            // Non-blocking client socket
            int flags = fcntl(clientFd, F_GETFL, 0);
            fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

            // Build address string
            char addrStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, addrStr, sizeof(addrStr));
            std::string address = std::string(addrStr) + ":" +
                                  std::to_string(ntohs(clientAddr.sin_port));

            std::cout << "[NET] New connection from " << address << "\n";
            connections_.emplace(clientFd, Connection(clientFd, address));
        }
    }

    std::string bindAddress_;
    uint16_t port_;
    int listenFd_ = -1;
    bool running_ = false;
    std::unordered_map<int, Connection> connections_;
};

} // namespace mc
