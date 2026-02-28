/**
 * Connection.cpp — Per-client connection implementation.
 *
 * Reference: net.minecraft.network.NetworkManager
 * Implements VarInt-framed packet reading/writing with dedicated threads.
 */

#include "networking/Connection.h"
#include "networking/PacketHandler.h"
#include "types/VarInt.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

namespace mccpp {

Connection::Connection(int socketFd, const std::string& remoteAddress, uint16_t remotePort)
    : socketFd_(socketFd), remoteAddress_(remoteAddress), remotePort_(remotePort) {}

Connection::~Connection() {
    disconnect();
    if (readThread_.joinable()) readThread_.join();
    if (writeThread_.joinable()) writeThread_.join();
    closeSocket();
}

void Connection::start(std::shared_ptr<PacketHandler> handler) {
    {
        std::lock_guard<std::mutex> lock(handlerMutex_);
        handler_ = std::move(handler);
    }
    readThread_ = std::thread(&Connection::readLoop, this);
    writeThread_ = std::thread(&Connection::writeLoop, this);
}

void Connection::setHandler(std::shared_ptr<PacketHandler> handler) {
    // Java reference: NetworkManager.setNetHandler()
    std::lock_guard<std::mutex> lock(handlerMutex_);
    handler_ = std::move(handler);
}

void Connection::sendPacket(std::vector<uint8_t> data) {
    // Java reference: NetworkManager.scheduleOutboundPacket()
    if (!connected_.load(std::memory_order_relaxed)) return;

    // Frame the packet: VarInt(total length) + data
    // data already contains VarInt(packetId) + payload
    std::vector<uint8_t> framed;
    writeVarInt(framed, static_cast<int32_t>(data.size()));
    framed.insert(framed.end(), data.begin(), data.end());

    std::lock_guard<std::mutex> lock(outMutex_);
    outQueue_.push_back(std::move(framed));
}

void Connection::disconnect(const std::string& reason) {
    // Java reference: NetworkManager.closeChannel()
    if (!connected_.exchange(false)) return;

    if (!reason.empty()) {
        std::cout << "[Connection] Disconnecting " << remoteAddress_ << ":" << remotePort_
                  << " — " << reason << "\n";
    }

    // Notify handler
    std::shared_ptr<PacketHandler> h;
    {
        std::lock_guard<std::mutex> lock(handlerMutex_);
        h = handler_;
    }
    if (h) {
        h->onDisconnect(reason);
    }
}

void Connection::setState(ConnectionState state) {
    // Java reference: NetworkManager.setConnectionState()
    state_.store(state, std::memory_order_release);
}

bool Connection::hasOutboundData() const {
    std::lock_guard<std::mutex> lock(outMutex_);
    return !outQueue_.empty();
}

void Connection::readLoop() {
    std::vector<uint8_t> buffer;
    uint8_t readBuf[READ_BUF_SIZE];

    while (connected_.load(std::memory_order_relaxed)) {
        ssize_t bytesRead = ::recv(socketFd_, readBuf, READ_BUF_SIZE, 0);
        if (bytesRead <= 0) {
            if (bytesRead == 0) {
                // Java reference: NetworkManager.channelInactive() → "disconnect.endOfStream"
                disconnect("End of stream");
            } else if (errno != EINTR) {
                disconnect(std::string("Read error: ") + std::strerror(errno));
            }
            break;
        }

        buffer.insert(buffer.end(), readBuf, readBuf + bytesRead);

        // Process complete packets from the buffer
        // Java reference: NetworkManager.processReceivedPackets() processes up to 1000 packets/tick
        while (buffer.size() > 0 && connected_.load(std::memory_order_relaxed)) {
            // Try to read packet length (VarInt)
            size_t available = buffer.size();
            int offset = 0;
            int32_t packetLength = 0;
            bool lengthComplete = false;

            for (size_t i = 0; i < available && i < 5; ++i) {
                uint8_t byte = buffer[i];
                packetLength |= static_cast<int32_t>(byte & 0x7F) << (7 * i);
                ++offset;
                if ((byte & 0x80) == 0) {
                    lengthComplete = true;
                    break;
                }
            }

            if (!lengthComplete) {
                break; // Need more data for the length prefix
            }

            if (packetLength < 0 || packetLength > 2097152) { // 2 MB max
                disconnect("Packet too large");
                break;
            }

            size_t totalNeeded = static_cast<size_t>(offset) + static_cast<size_t>(packetLength);
            if (buffer.size() < totalNeeded) {
                break; // Need more data for the packet body
            }

            // Extract the packet body
            const uint8_t* packetData = buffer.data() + offset;
            size_t packetDataLen = static_cast<size_t>(packetLength);

            if (packetDataLen == 0) {
                disconnect("Empty packet");
                break;
            }

            // Read packet ID from the body
            auto idResult = readVarInt(packetData, packetDataLen);
            int32_t packetId = idResult.value;
            const uint8_t* payload = packetData + idResult.bytesRead;
            size_t payloadLen = packetDataLen - static_cast<size_t>(idResult.bytesRead);

            // Dispatch to handler (thread-safe access)
            {
                std::shared_ptr<PacketHandler> h;
                {
                    std::lock_guard<std::mutex> lock(handlerMutex_);
                    h = handler_;
                }
                if (h) {
                    h->handlePacket(packetId, payload, payloadLen, *this);
                }
            }

            // Remove processed bytes from buffer
            buffer.erase(buffer.begin(), buffer.begin() + static_cast<long>(totalNeeded));
        }
    }
}

void Connection::writeLoop() {
    while (connected_.load(std::memory_order_relaxed)) {
        std::vector<uint8_t> packet;

        {
            std::lock_guard<std::mutex> lock(outMutex_);
            if (outQueue_.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            packet = std::move(outQueue_.front());
            outQueue_.pop_front();
        }

        // Write the full packet to socket
        size_t totalSent = 0;
        while (totalSent < packet.size()) {
            ssize_t sent = ::send(socketFd_, packet.data() + totalSent,
                                  packet.size() - totalSent, MSG_NOSIGNAL);
            if (sent <= 0) {
                if (errno == EINTR) continue;
                disconnect(std::string("Write error: ") + std::strerror(errno));
                return;
            }
            totalSent += static_cast<size_t>(sent);
        }
    }
}

void Connection::closeSocket() {
    if (socketFd_ >= 0) {
        ::shutdown(socketFd_, SHUT_RDWR);
        ::close(socketFd_);
        socketFd_ = -1;
    }
}

} // namespace mccpp
