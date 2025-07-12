#pragma once

#include "aurora_app/graphics/aurora_model.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace aurora {
    namespace AuroraUtils {
        std::vector<AuroraModel::Vertex> createRoundedRectangleVertices(
            glm::vec2 size, 
            float radius, 
            int numSegments, 
            float borderOffset = 0.0f,
            glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f}
        );
    }
}
