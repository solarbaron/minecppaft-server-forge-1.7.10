/**
 * Chat.cpp — Chat component JSON serialization.
 *
 * Reference: fj.java (obfuscated IChatComponent)
 */

#include "types/Chat.h"

#include <sstream>

namespace mccpp {

/**
 * Escape a string for JSON output.
 */
static std::string jsonEscape(const std::string& str) {
    std::string result;
    result.reserve(str.size() + 8);
    for (char c : str) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:   result += c;      break;
        }
    }
    return result;
}

std::string ChatComponent::toJson() const {
    std::ostringstream ss;
    ss << "{\"text\":\"" << jsonEscape(text) << "\"";

    if (!color.empty())  ss << ",\"color\":\"" << color << "\"";
    if (bold)            ss << ",\"bold\":true";
    if (italic)          ss << ",\"italic\":true";
    if (underline)       ss << ",\"underlined\":true";
    if (strikethrough)   ss << ",\"strikethrough\":true";
    if (obfuscated)      ss << ",\"obfuscated\":true";

    if (!extra.empty()) {
        ss << ",\"extra\":[";
        for (size_t i = 0; i < extra.size(); ++i) {
            if (i > 0) ss << ",";
            ss << extra[i].toJson();
        }
        ss << "]";
    }

    ss << "}";
    return ss.str();
}

} // namespace mccpp
