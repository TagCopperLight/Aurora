#pragma once

#include <glm/glm.hpp>

namespace aurora {
    class AuroraThemeSettings {
    public:
        static constexpr const char* PRIMARY         = "#CEC4F6";
        static constexpr const char* SECONDARY       = "#73BFF9";
        static constexpr const char* ORANGE          = "#EEBA66";
        static constexpr const char* DISABLED        = "#404352";
        static constexpr const char* BACKGROUND      = "#0D0E13";
        static constexpr const char* DELIMITER       = "#5A5A62";
        static constexpr const char* TEXT_PRIMARY    = "#D9D9D9";
        static constexpr const char* TEXT_SECONDARY  = "#CBC6E6";
        static constexpr const char* TEXT_DISABLED   = "#8B8C91";
        static constexpr const char* SHADOW_LIGHT        = "#0000001A";
        static constexpr const char* SHADOW_MEDIUM       = "#00000033";
        static constexpr const char* SHADOW_HEAVY        = "#00000066";
        static constexpr const char* SHADOW_TRANSPARENT  = "#00000000";
        static constexpr const char* SUCCESS         = "#DFF3AC";
        static constexpr const char* WARNING         = "#FF9800";
        static constexpr const char* ERROR           = "#EDB3B6";
        static constexpr const char* INFO            = "#2196F3";
        static constexpr const char* TRANSPARENT     = "#00000000";
        static constexpr const char* WHITE           = "#FFFFFFFF";
        static constexpr const char* BLACK           = "#000000FF";

        struct Colors {
            glm::vec4 PRIMARY;
            glm::vec4 SECONDARY;
            glm::vec4 ORANGE;
            glm::vec4 DISABLED;
            glm::vec4 BACKGROUND;
            glm::vec4 DELIMITER;
            glm::vec4 TEXT_PRIMARY;
            glm::vec4 TEXT_SECONDARY;
            glm::vec4 TEXT_DISABLED;
            glm::vec4 SHADOW_LIGHT;
            glm::vec4 SHADOW_MEDIUM;
            glm::vec4 SHADOW_HEAVY;
            glm::vec4 SHADOW_TRANSPARENT;
            glm::vec4 SUCCESS;
            glm::vec4 WARNING;
            glm::vec4 ERROR;
            glm::vec4 INFO;
            glm::vec4 TRANSPARENT;
            glm::vec4 WHITE;
            glm::vec4 BLACK;
        };

        static const Colors& get();

        static constexpr const char* FONT_PATH = "assets/Monaco.otf";

    private:
        static glm::vec4 fromHex(const char* hex);
        static Colors init();
    };
}