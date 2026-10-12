#pragma once

#include "aurora_ui/graphics/aurora_model.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"
#include <glm/glm.hpp>
#include <initializer_list>
#include <string>
#include <vector>

namespace aurora {
    namespace AuroraUtils {
        std::vector<AuroraModel::Vertex> createRoundedRectangleVertices(
            glm::vec2 size, 
            float radius, 
            int numSegments, 
            float borderOffset = 0.0f,
            glm::vec4 color = AuroraThemeSettings::get().WHITE
        );

        // Builds a cache key for AuroraRenderSystemManager::getSharedModel from a shape name and its parameters
        std::string modelKey(const char* shape, std::initializer_list<float> params);
    }
}
