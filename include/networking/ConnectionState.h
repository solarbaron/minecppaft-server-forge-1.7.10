#pragma once
// Connection states for Minecraft protocol 47 (1.7.10).
// Maps to the obfuscated enum used in the Java server's network pipeline.

namespace mc {

enum class ConnectionState {
    Handshaking = -1,
    Play        = 0,
    Status      = 1,
    Login       = 2
};

} // namespace mc
