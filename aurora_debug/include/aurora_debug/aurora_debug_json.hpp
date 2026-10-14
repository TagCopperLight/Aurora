#pragma once

#include "aurora_debug_message.hpp"

#include <string>
#include <vector>

namespace aurora::debug {

    // Minimal readers for the flat JSON objects sent as DebugMessage payloads.
    // They look up a top-level key and do not handle nested objects.
    namespace json {

        // Returns the value of `key` as text: strings are unescaped, numbers and literals are returned verbatim.
        // Returns an empty string if the key is missing.
        std::string getString(const std::string& payload, const std::string& key);

        // Returns the value of `key` as a number, or `fallback` if the key is missing or not a number.
        double getNumber(const std::string& payload, const std::string& key, double fallback = 0.0);

        // Reads a flat array of numbers, e.g. "key": [1.0, 2.5, -3]
        std::vector<float> getNumberArray(const std::string& payload, const std::string& key);

        // Returns `text` as a quoted JSON string, escaping quotes, backslashes and control characters
        std::string quote(const std::string& text);
    }

    // Payload of a MessageType::Watch message: {"section": "<panel section>", "name": "<label>", "value": "<text>"}
    // `section` is optional and defaults to "WATCHES".
    struct WatchMessage {
        std::string section;
        std::string name;
        std::string value;

        // Returns false if the payload has no name
        static bool parse(const std::string& payload, WatchMessage& out);
    };

    // Payload of a MessageType::Command message, sent by the app to its clients: {"command": "<name>"}
    // The command names are agreed between the app and its clients.
    struct CommandMessage {
        std::string name;

        DebugMessage toMessage() const;
    };

}
