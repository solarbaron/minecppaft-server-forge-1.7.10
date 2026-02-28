/**
 * Chat.h — Minecraft chat component (JSON text format).
 *
 * Reference: fj.java (obfuscated) — IChatComponent and its implementations.
 * Protocol version 5 sends chat as JSON strings.
 */
#pragma once

#include <string>
#include <vector>

namespace mccpp {

/**
 * Minimal chat component — enough for server messages, kick reasons,
 * and MOTD. Full implementation (click events, hover events, selectors)
 * will be added when play packets require them.
 */
struct ChatComponent {
    std::string text;
    std::string color;       // empty = inherit
    bool bold      = false;
    bool italic    = false;
    bool underline = false;
    bool strikethrough = false;
    bool obfuscated    = false;
    std::vector<ChatComponent> extra;

    /**
     * Serialize to Minecraft JSON chat format.
     * Example: {"text":"Hello","color":"gold","bold":true}
     */
    std::string toJson() const;

    /**
     * Create a simple text-only component.
     */
    static ChatComponent of(const std::string& msg) {
        return ChatComponent{msg};
    }

    /**
     * Create a component with color.
     */
    static ChatComponent ofColored(const std::string& msg, const std::string& col) {
        ChatComponent c;
        c.text = msg;
        c.color = col;
        return c;
    }
};

} // namespace mccpp
