#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>

#include "networking/TcpServer.h"
#include "networking/PacketHandler.h"

// Matches MinecraftServer.run() tick timing: 50ms per tick
static constexpr int TICK_MS = 50;
static std::atomic<bool> g_running{true};

void signalHandler(int) {
    g_running = false;
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::string bindAddr = "0.0.0.0";
    uint16_t port = 25565;

    // Parse --port flag (matches MinecraftServer.main() arg parsing)
    for (int i = 0; i < argc; ++i) {
        if (std::string(argv[i]) == "--port" && i + 1 < argc) {
            port = static_cast<uint16_t>(std::stoi(argv[i + 1]));
        }
    }

    std::cout << "========================================\n";
    std::cout << "  MineCPPaft Server v0.1.0\n";
    std::cout << "  Minecraft 1.7.10 Protocol (47)\n";
    std::cout << "========================================\n";

    mc::TcpServer server(bindAddr, port);
    if (!server.start()) {
        std::cerr << "Failed to start server!\n";
        return 1;
    }

    mc::PacketHandler handler;

    std::cout << "Server running. Press Ctrl+C to stop.\n";

    // Main tick loop — mirrors MinecraftServer.run() at 50ms/tick
    while (g_running) {
        auto tickStart = std::chrono::steady_clock::now();

        server.tick([&handler](mc::Connection& conn, mc::PacketBuffer& buf) {
            handler.handle(conn, buf);
        });

        auto tickEnd = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tickEnd - tickStart);
        auto sleepTime = std::chrono::milliseconds(TICK_MS) - elapsed;
        if (sleepTime.count() > 0) {
            std::this_thread::sleep_for(sleepTime);
        }
    }

    std::cout << "\nStopping server...\n";
    server.stop();
    std::cout << "Server stopped.\n";
    return 0;
}
