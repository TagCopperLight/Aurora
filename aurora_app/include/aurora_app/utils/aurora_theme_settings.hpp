#pragma once

#include <glm/glm.hpp>

namespace aurora {
    class AuroraThemeSettings {
    public:
        // Primary UI Colors
        static constexpr glm::vec4 PRIMARY = {0.808, 0.769f, 0.965f, 1.0f};
        static constexpr glm::vec4 SECONDARY = {0.451f, 0.749f, 0.976f, 1.0f};
        static constexpr glm::vec4 ORANGE = {0.933f, 0.729f, 0.4f, 1.0f};
        static constexpr glm::vec4 DISABLED = {0.251f, 0.263f, 0.322f, 1.0f};

        // Background Colors
        // static constexpr glm::vec4 BACKGROUND = {0.314f, 0.314f, 0.314f, 1.0f};
        static constexpr glm::vec4 BACKGROUND = {0.051f, 0.055f, 0.075f, 1.0f};
        
        // Surface Colors
        static constexpr glm::vec4 DELIMITER = {0.353f, 0.353f, 0.384f, 1.0f};
        
        // Text Colors
        // static constexpr glm::vec4 TEXT_PRIMARY = {1.0f, 1.0f, 1.0f, 1.0f};           // White text
        static constexpr glm::vec4 TEXT_PRIMARY = {0.85, 0.85, 0.85, 1.0f};
        static constexpr glm::vec4 TEXT_SECONDARY = {0.796f, 0.776f, 0.902f, 1.0f};         // Light gray text
        static constexpr glm::vec4 TEXT_DISABLED = {0.545f, 0.549f, 0.569f, 1.0f};          // Disabled text

        // Shadow Colors
        static constexpr glm::vec4 SHADOW_LIGHT = {0.0f, 0.0f, 0.0f, 0.1f};           // Light shadow
        static constexpr glm::vec4 SHADOW_MEDIUM = {0.0f, 0.0f, 0.0f, 0.2f};          // Medium shadow
        static constexpr glm::vec4 SHADOW_HEAVY = {0.0f, 0.0f, 0.0f, 0.4f};           // Heavy shadow
        static constexpr glm::vec4 SHADOW_TRANSPARENT = {0.0f, 0.0f, 0.0f, 0.0f};     // No shadow

        // Status Colors
        static constexpr glm::vec4 SUCCESS = {0.875f, 0.954f, 0.675f, 1.0f};          // Green
        static constexpr glm::vec4 WARNING = {1.0f, 0.596f, 0.0f, 1.0f};              // Orange
        static constexpr glm::vec4 ERROR = {0.931f, 0.702f, 0.714f, 1.0f};            // Red
        static constexpr glm::vec4 INFO = {0.129f, 0.588f, 0.953f, 1.0f};             // Blue

        // Utility Colors
        static constexpr glm::vec4 TRANSPARENT = {0.0f, 0.0f, 0.0f, 0.0f};
        static constexpr glm::vec4 WHITE = {1.0f, 1.0f, 1.0f, 1.0f};
        static constexpr glm::vec4 BLACK = {0.0f, 0.0f, 0.0f, 1.0f};
        
        // Font Path
        static constexpr const char* FONT_PATH = "assets/Monaco.otf";
    };
}
