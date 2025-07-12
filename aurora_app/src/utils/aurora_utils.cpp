#include "aurora_app/utils/aurora_utils.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/constants.hpp>

namespace aurora {
    namespace AuroraUtils {
        std::vector<AuroraModel::Vertex> createRoundedRectangleVertices(
            glm::vec2 size, 
            float radius, 
            int numSegments, 
            float borderOffset,
            glm::vec4 color
        ) {
            std::vector<AuroraModel::Vertex> vertices;
            
            std::vector<glm::vec2> corners = {
                {radius, size.y - radius},
                {radius, radius},
                {size.x - radius, radius},
                {size.x - radius, size.y - radius},
            };

            for (int i = 0; i < 4; ++i) {
                glm::vec2 corner = corners[i];
                float angleStart = glm::pi<float>() * 0.5f * i + glm::pi<float>() * 0.5f; 
                float angleEnd = angleStart + glm::pi<float>() * 0.5f;

                for (int j = 0; j <= numSegments; ++j) {
                    float angle = angleStart + (angleEnd - angleStart) * j / numSegments;
                    glm::vec2 position = corner + (radius - borderOffset) * glm::vec2(glm::cos(angle), glm::sin(angle));
                    vertices.push_back({position, color});
                }
            }

            return vertices;
        }
    }
}
