#include "aurora_debug/aurora_debug_json.hpp"

#include <cctype>
#include <cstdlib>

namespace aurora::debug {

    namespace {
        size_t skipSpaces(const std::string& text, size_t pos) {
            while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos]))) ++pos;
            return pos;
        }

        // Position of the first character of the value for `key`, or npos.
        // A quoted occurrence of the key only counts if it is followed by ':', so string values are skipped.
        size_t findValue(const std::string& payload, const std::string& key) {
            const std::string quoted = "\"" + key + "\"";
            for (size_t pos = payload.find(quoted); pos != std::string::npos; pos = payload.find(quoted, pos + 1)) {
                size_t colon = skipSpaces(payload, pos + quoted.size());
                if (colon < payload.size() && payload[colon] == ':') {
                    size_t value = skipSpaces(payload, colon + 1);
                    return value < payload.size() ? value : std::string::npos;
                }
            }
            return std::string::npos;
        }
    }

    namespace json {

        std::string getString(const std::string& payload, const std::string& key) {
            size_t pos = findValue(payload, key);
            if (pos == std::string::npos) return {};

            if (payload[pos] != '"') {
                size_t end = payload.find_first_of(",}]", pos);
                size_t last = (end == std::string::npos ? payload.size() : end);
                while (last > pos && std::isspace(static_cast<unsigned char>(payload[last - 1]))) --last;
                return payload.substr(pos, last - pos);
            }

            std::string value;
            for (++pos; pos < payload.size() && payload[pos] != '"'; ++pos) {
                char c = payload[pos];
                if (c != '\\' || pos + 1 >= payload.size()) {
                    value += c;
                    continue;
                }

                char escaped = payload[++pos];
                switch (escaped) {
                    case 'n': value += '\n'; break;
                    case 't': value += '\t'; break;
                    case 'r': value += '\r'; break;
                    case 'b': value += '\b'; break;
                    case 'f': value += '\f'; break;
                    case 'u': {
                        // Only ASCII code points are kept, anything else is shown as '?'
                        long code = pos + 4 < payload.size() ? std::strtol(payload.substr(pos + 1, 4).c_str(), nullptr, 16) : 0;
                        value += (code > 0 && code < 0x80) ? static_cast<char>(code) : '?';
                        pos += 4;
                        break;
                    }
                    default: value += escaped; break; // \" \\ \/
                }
            }
            return value;
        }

        double getNumber(const std::string& payload, const std::string& key, double fallback) {
            size_t pos = findValue(payload, key);
            if (pos == std::string::npos) return fallback;

            const char* begin = payload.c_str() + pos;
            char* end = nullptr;
            double value = std::strtod(begin, &end);
            return end == begin ? fallback : value;
        }

        std::vector<float> getNumberArray(const std::string& payload, const std::string& key) {
            std::vector<float> values;
            size_t begin = findValue(payload, key);
            if (begin == std::string::npos || payload[begin] != '[') return values;
            size_t end = payload.find(']', begin);
            if (end == std::string::npos) return values;

            const char* cursor = payload.c_str() + begin + 1;
            const char* last = payload.c_str() + end;
            while (cursor < last) {
                char* next = nullptr;
                float value = std::strtof(cursor, &next);
                if (next == cursor) {
                    ++cursor; // separator
                    continue;
                }
                values.push_back(value);
                cursor = next;
            }
            return values;
        }
    }

    bool WatchMessage::parse(const std::string& payload, WatchMessage& out) {
        out.name = json::getString(payload, "name");
        if (out.name.empty()) return false;

        out.section = json::getString(payload, "section");
        if (out.section.empty()) out.section = "WATCHES";
        out.value = json::getString(payload, "value");
        return true;
    }

}
