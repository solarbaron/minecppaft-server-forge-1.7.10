/**
 * TcpListener.cpp — Multi-threaded TCP accept loop implementation.
 *
 * Reference: nc.java (obfuscated ServerConnection)
 * Uses POSIX sockets; Netty concepts mapped to raw syscalls.
 */

#include "networking/TcpListener.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

namespace mccpp {

TcpListener::TcpListener(const std::string& bindAddress, uint16_t port)
    : bindAddress_(bindAddress), port_(port) {}

TcpListener::~TcpListener() {
    stop();
}

void TcpListener::setAcceptCallback(AcceptCallback callback) {
    onAccept_ = std::move(callback);
}

bool TcpListener::start() {
    serverFd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd_ < 0) {
        std::cerr << "[TcpListener] Failed to create socket: " << std::strerror(errno) << "\n";
        return false;
    }

    // Allow address reuse (avoid "Address already in use" on restart)
    int opt = 1;
    ::setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    if (bindAddress_ == "0.0.0.0" || bindAddress_.empty()) {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (::inet_pton(AF_INET, bindAddress_.c_str(), &addr.sin_addr) <= 0) {
            std::cerr << "[TcpListener] Invalid bind address: " << bindAddress_ << "\n";
            ::close(serverFd_);
            serverFd_ = -1;
            return false;
        }
    }

    if (::bind(serverFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "[TcpListener] Bind failed on port " << port_ << ": "
                  << std::strerror(errno) << "\n";
        ::close(serverFd_);
        serverFd_ = -1;
        return false;
    }

    if (::listen(serverFd_, 128) < 0) {
        std::cerr << "[TcpListener] Listen failed: " << std::strerror(errno) << "\n";
        ::close(serverFd_);
        serverFd_ = -1;
        return false;
    }

    running_.store(true, std::memory_order_release);
    acceptThread_ = std::thread(&TcpListener::acceptLoop, this);

    std::cout << "[TcpListener] Listening on " << bindAddress_ << ":" << port_ << "\n";
    return true;
}

void TcpListener::stop() {
    if (!running_.exchange(false)) {
        return;
    }

    // Close the server socket to unblock accept()
    if (serverFd_ >= 0) {
        ::shutdown(serverFd_, SHUT_RDWR);
        ::close(serverFd_);
        serverFd_ = -1;
    }

    if (acceptThread_.joinable()) {
        acceptThread_.join();
    }
}

void TcpListener::acceptLoop() {
    while (running_.load(std::memory_order_acquire)) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);

        int clientFd = ::accept(serverFd_, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (clientFd < 0) {
            if (!running_.load(std::memory_order_relaxed)) {
                break; // We're shutting down
            }
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "[TcpListener] Accept error: " << std::strerror(errno) << "\n";
            continue;
        }

        // Disable Nagle's algorithm (Minecraft protocol is latency-sensitive)
        int flag = 1;
        ::setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

        // Get remote address string
        char addrBuf[INET_ADDRSTRLEN];
        ::inet_ntop(AF_INET, &clientAddr.sin_addr, addrBuf, sizeof(addrBuf));
        std::string remoteAddr(addrBuf);
        uint16_t remotePort = ntohs(clientAddr.sin_port);

        std::cout << "[TcpListener] Accepted connection from "
                  << remoteAddr << ":" << remotePort << "\n";

        if (onAccept_) {
            onAccept_(clientFd, remoteAddr, remotePort);
        } else {
            // No handler registered, close immediately
            ::close(clientFd);
        }
    }
}

} // namespace mccpp
