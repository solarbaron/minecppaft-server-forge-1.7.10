/**
 * main.cpp — MineCPPaft server entry point.
 *
 * Reference: MinecraftServer.main() in MinecraftServer.java
 *
 * Initializes the server, installs signal handlers for clean shutdown,
 * and starts the main tick loop.
 */

#include "server/MinecraftServer.h"

#include <csignal>
#include <cstdlib>
#include <iostream>

static mccpp::MinecraftServer* g_server = nullptr;

/**
 * Signal handler for clean shutdown (Ctrl+C / SIGTERM).
 * Mirrors Java's Runtime.addShutdownHook() in MinecraftServer.main().
 */
static void signalHandler(int sig) {
    std::cout << "\n[Main] Received signal " << sig << ", stopping server...\n";
    if (g_server) {
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    std::cout << "╔══════════════════════════════════════╗\n"
              << "║          MineCPPaft Server           ║\n"
              << "║     Minecraft 1.7.10 (Protocol 5)    ║\n"
              << "║        C++ Server Rewrite            ║\n"
              << "╚══════════════════════════════════════╝\n\n";

    mccpp::MinecraftServer server;
    g_server = &server;

    // Parse command-line arguments (mirrors Java main() argument parsing)
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        std::string next = (i + 1 < argc) ? argv[i + 1] : "";

        if (arg == "--port" && !next.empty()) {
            server.setPort(static_cast<uint16_t>(std::atoi(next.c_str())));
            ++i;
        } else if (arg == "--bind" && !next.empty()) {
            server.setBindAddress(next);
            ++i;
        } else if (arg == "--motd" && !next.empty()) {
            server.setMotd(next);
            ++i;
        } else if (arg == "--max-players" && !next.empty()) {
            server.setMaxPlayers(std::atoi(next.c_str()));
            ++i;
        } else if (arg == "--help") {
            std::cout << "Usage: minecppaft-server [options]\n"
                      << "  --port <port>         Server port (default: 25565)\n"
                      << "  --bind <address>      Bind address (default: 0.0.0.0)\n"
                      << "  --motd <message>      Server MOTD\n"
                      << "  --max-players <count> Max player count (default: 20)\n"
                      << "  --help                Show this help\n";
            return 0;
        }
    }

    // Install signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGPIPE, SIG_IGN); // Ignore broken pipe (handled in Connection)

    // Initialize
    if (!server.init()) {
        std::cerr << "[Main] Server initialization failed!\n";
        return 1;
    }

    // Run the main tick loop (blocking)
    server.run();

    g_server = nullptr;
    return 0;
}
