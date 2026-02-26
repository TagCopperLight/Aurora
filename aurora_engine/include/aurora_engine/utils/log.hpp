#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

namespace aurora::log {

    namespace detail {
        static constexpr const char* PATTERN = "[%H:%M:%S.%e] [%-6n] [%^%-5l%$] %v";

        inline std::shared_ptr<spdlog::logger> make(const char* name) {
            auto l = spdlog::get(name);
            if (!l) {
                l = spdlog::stdout_color_mt(name);
                l->set_pattern(PATTERN);
            }
            return l;
        }
    }

    inline std::shared_ptr<spdlog::logger>& engine() {
        static auto l = detail::make("engine");
        return l;
    }

    inline std::shared_ptr<spdlog::logger>& ui() {
        static auto l = detail::make("ui");
        return l;
    }

    inline std::shared_ptr<spdlog::logger>& debug() {
        static auto l = detail::make("debug");
        return l;
    }

    inline void init(spdlog::level::level_enum level = spdlog::level::info) {
        engine()->set_level(level);
        ui()->set_level(level);
        debug()->set_level(level);
    }
}