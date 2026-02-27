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

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "networking/Connection.h"

namespace mc {

// Callback type for handling a newly extracted packet on a connection
using PacketCallback = std::function<void(Connection&, PacketBuffer&)>;

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
    // reads data, extracts packets, and flushes send queues.
    void tick(PacketCallback handler) {
        if (!running_) return;

        acceptNewConnections();

        for (auto it = connections_.begin(); it != connections_.end();) {
            auto& conn = *it;

            // Receive
            if (!conn->recv()) {
                // recv returned 0 or error on a non-blocking socket
                // For EAGAIN/EWOULDBLOCK this is fine, check errno
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    conn->close();
                    it = connections_.erase(it);
                    continue;
                }
            }

            // Extract and handle packets
            PacketBuffer packet;
            while (conn->tryReadPacket(packet)) {
                try {
                    handler(*conn, packet);
                } catch (const std::exception& e) {
                    std::cerr << "[NET] Error handling packet from "
                              << conn->address() << ": " << e.what() << "\n";
                    conn->close();
                    break;
                }
            }

            // Flush send queue
            if (!conn->isClosed()) {
                if (!conn->flush()) {
                    conn->close();
                }
            }

            // Remove closed connections
            if (conn->isClosed()) {
                it = connections_.erase(it);
            } else {
                ++it;
            }
        }
    }

    size_t connectionCount() const { return connections_.size(); }
    bool isRunning() const { return running_; }

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

            // Build address string
            char addrStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, addrStr, sizeof(addrStr));
            std::string address = std::string(addrStr) + ":" +
                                  std::to_string(ntohs(clientAddr.sin_port));

            auto conn = std::make_unique<Connection>(clientFd, address);
            conn->setNonBlocking();

            std::cout << "[NET] New connection from " << address << "\n";
            connections_.push_back(std::move(conn));
        }
    }

    std::string bindAddress_;
    uint16_t port_;
    int listenFd_ = -1;
    bool running_ = false;
    std::vector<std::unique_ptr<Connection>> connections_;
};

} // namespace mc
