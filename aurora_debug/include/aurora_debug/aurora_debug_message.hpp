#pragma once

#include <string>
#include <cstdint>

namespace aurora::debug {

    enum class MessageType : uint8_t {
        Log       = 0,  // plain log line
        Watch     = 1,  // variable name + value
        Profiling = 2,  // frame timing data
        Custom    = 3,  // application-defined widget data
    };

    struct DebugMessage {
        MessageType type;
        std::string payload; // newline-delimited JSON string
    };

}