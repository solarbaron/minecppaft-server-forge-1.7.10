#pragma once
// Connection — represents a single client TCP connection.
// Handles raw byte recv/send plus VarInt-framed packet extraction.
// The Java equivalent is the Netty pipeline + NetworkManager (obfuscated).

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include <deque>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "networking/ConnectionState.h"
#include "networking/PacketBuffer.h"
#include "networking/VarInt.h"

namespace mc {

class Connection {
public:
    explicit Connection(int fd, const std::string& address)
        : fd_(fd), address_(address), state_(ConnectionState::Handshaking) {}

    ~Connection() {
        close();
    }

    // Non-copyable
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    // Move constructible + assignable
    Connection(Connection&& other) noexcept
        : fd_(other.fd_), address_(std::move(other.address_)),
          state_(other.state_), recvBuf_(std::move(other.recvBuf_)),
          sendQueue_(std::move(other.sendQueue_)), closed_(other.closed_) {
        other.fd_ = -1;
        other.closed_ = true;
    }

    Connection& operator=(Connection&& other) noexcept {
        if (this != &other) {
            close();
            fd_ = other.fd_;
            address_ = std::move(other.address_);
            state_ = other.state_;
            recvBuf_ = std::move(other.recvBuf_);
            sendQueue_ = std::move(other.sendQueue_);
            closed_ = other.closed_;
            other.fd_ = -1;
            other.closed_ = true;
        }
        return *this;
    }

    // Receive data from socket into internal buffer. Returns false if connection closed/error.
    bool recv() {
        uint8_t tmp[4096];
        ssize_t n = ::recv(fd_, tmp, sizeof(tmp), 0);
        if (n <= 0) {
            return false;
        }
        recvBuf_.insert(recvBuf_.end(), tmp, tmp + n);
        return true;
    }

    // Try to extract one VarInt-framed packet from the receive buffer.
    // Returns true if a complete packet was extracted into `out`.
    // Protocol 47 framing: [VarInt packetLength] [packetId + data]
    bool tryReadPacket(PacketBuffer& out) {
        if (recvBuf_.empty()) return false;

        size_t offset = 0;
        int32_t packetLength;
        try {
            packetLength = VarInt::read(recvBuf_.data(), recvBuf_.size(), offset);
        } catch (...) {
            // Not enough data for VarInt yet
            return false;
        }

        if (packetLength < 0 || packetLength > 2097152) { // 2MB max
            closed_ = true;
            return false;
        }

        size_t totalNeeded = offset + static_cast<size_t>(packetLength);
        if (recvBuf_.size() < totalNeeded) {
            return false; // Incomplete packet
        }

        // Extract the packet payload (packetId + data)
        std::vector<uint8_t> payload(recvBuf_.begin() + offset,
                                      recvBuf_.begin() + totalNeeded);
        recvBuf_.erase(recvBuf_.begin(), recvBuf_.begin() + totalNeeded);

        out = PacketBuffer(std::move(payload));
        return true;
    }

    // Queue a packet for sending: [VarInt length] [data]
    void sendPacket(const PacketBuffer& packet) {
        std::vector<uint8_t> frame;
        VarInt::write(frame, static_cast<int32_t>(packet.data().size()));
        frame.insert(frame.end(), packet.data().begin(), packet.data().end());

        std::lock_guard<std::mutex> lock(sendMtx_);
        sendQueue_.push_back(std::move(frame));
    }

    // Flush queued data to socket. Returns false on error.
    bool flush() {
        std::lock_guard<std::mutex> lock(sendMtx_);
        while (!sendQueue_.empty()) {
            auto& front = sendQueue_.front();
            ssize_t sent = ::send(fd_, front.data(), front.size(), MSG_NOSIGNAL);
            if (sent < 0) {
                return false;
            }
            if (static_cast<size_t>(sent) < front.size()) {
                front.erase(front.begin(), front.begin() + sent);
                return true; // Partial send, try again later
            }
            sendQueue_.pop_front();
        }
        return true;
    }

    void close() {
        if (!closed_ && fd_ >= 0) {
            ::close(fd_);
            closed_ = true;
        }
    }

    void setNonBlocking() {
        int flags = fcntl(fd_, F_GETFL, 0);
        fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
    }

    // Getters/Setters
    ConnectionState state() const { return state_; }
    void setState(ConnectionState s) { state_ = s; }

    const std::string& address() const { return address_; }
    int fd() const { return fd_; }
    bool isClosed() const { return closed_; }

private:
    int fd_;
    std::string address_;
    ConnectionState state_;
    std::vector<uint8_t> recvBuf_;
    std::deque<std::vector<uint8_t>> sendQueue_;
    std::mutex sendMtx_;
    bool closed_ = false;
};

} // namespace mc
