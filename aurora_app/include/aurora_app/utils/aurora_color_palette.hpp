#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace aurora {
    class AuroraColorPalette {
    public:
        // Primary UI Colors
        static constexpr glm::vec4 PRIMARY = {0.784f, 0.38f, 0.286f, 1.0f};

        // Background Colors
        static constexpr glm::vec4 BACKGROUND = {0.314f, 0.314f, 0.314f, 1.0f};
        
        // Surface Colors
        static constexpr glm::vec4 SURFACE = {0.196f, 0.196f, 0.196f, 1.0f};
        
        // Text Colors
        static constexpr glm::vec4 TEXT_PRIMARY = {1.0f, 1.0f, 1.0f, 1.0f};           // White text
        static constexpr glm::vec4 TEXT_SECONDARY = {0.8f, 0.8f, 0.8f, 1.0f};         // Light gray text
        static constexpr glm::vec4 TEXT_DISABLED = {0.5f, 0.5f, 0.5f, 1.0f};          // Disabled text

        // Shadow Colors
        static constexpr glm::vec4 SHADOW_LIGHT = {0.0f, 0.0f, 0.0f, 0.1f};           // Light shadow
        static constexpr glm::vec4 SHADOW_MEDIUM = {0.0f, 0.0f, 0.0f, 0.2f};          // Medium shadow
        static constexpr glm::vec4 SHADOW_HEAVY = {0.0f, 0.0f, 0.0f, 0.4f};           // Heavy shadow
        static constexpr glm::vec4 SHADOW_TRANSPARENT = {0.0f, 0.0f, 0.0f, 0.0f};     // No shadow

        // Status Colors
        static constexpr glm::vec4 SUCCESS = {0.298f, 0.686f, 0.314f, 1.0f};          // Green
        static constexpr glm::vec4 WARNING = {1.0f, 0.596f, 0.0f, 1.0f};              // Orange
        static constexpr glm::vec4 ERROR = {0.957f, 0.263f, 0.212f, 1.0f};            // Red
        static constexpr glm::vec4 INFO = {0.129f, 0.588f, 0.953f, 1.0f};             // Blue

        // Utility Colors
        static constexpr glm::vec4 TRANSPARENT = {0.0f, 0.0f, 0.0f, 0.0f};
        static constexpr glm::vec4 WHITE = {1.0f, 1.0f, 1.0f, 1.0f};
        static constexpr glm::vec4 BLACK = {0.0f, 0.0f, 0.0f, 1.0f};
        
        // Utility Functions
        static glm::vec4 hexToVec4(unsigned int hex, float alpha = 1.0f) {
            float r = ((hex >> 16) & 0xFF) / 255.0f;
            float g = ((hex >> 8) & 0xFF) / 255.0f;
            float b = (hex & 0xFF) / 255.0f;

            return glm::vec4(r, g, b, alpha);
        }
    };
}
