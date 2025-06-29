#include "aurora_app/components/aurora_rounded_rect.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/constants.hpp>

#include <memory>
#include <iostream>

namespace aurora {
    AuroraRoundedRectangle::AuroraRoundedRectangle(AuroraDevice &device, glm::vec2 size, float radius)
        : AuroraComponentInterface{device}, size{size}, radius{radius} {
        initialize();
    }

    void AuroraRoundedRectangle::initialize() {
        std::vector<AuroraModel::Vertex> vertices = createRoundedRectangleVertices(16);
        model = std::make_shared<AuroraModel>(auroraDevice, vertices);
        color = {1.0f, 1.0f, 1.0f};
    }

    std::vector<AuroraModel::Vertex> AuroraRoundedRectangle::createRoundedRectangleVertices(int numSegments) {
        std::vector<AuroraModel::Vertex> vertices;

        std::vector<glm::vec2> corners = {
            {radius, -radius},                      // Top Left 
            {radius, -size.y + radius},            // Bottom Left 
            {size.x - radius, -size.y + radius},         // Bottom Right
            {size.x - radius, -radius},                  // Top Right
        };

        for (int i = 0; i < 4; ++i) {
            glm::vec2 corner = corners[i];
            float angleStart = glm::pi<float>() * 0.5f * i + glm::pi<float>() * 0.5f; 
            float angleEnd = angleStart + glm::pi<float>() * 0.5f;

            for (int j = 0; j <= numSegments; ++j) {
                float angle = angleStart + (angleEnd - angleStart) * j / numSegments;
                glm::vec2 position = corner + radius * glm::vec2(glm::cos(angle), glm::sin(angle));
                vertices.push_back({position, color});
            }
        }

        // for (int i = 0; i < 4; ++i) {
        //     std::cout << "Adding corner " << i << ": " << corners[i].x << ", " << corners[i].y << std::endl;
        //     vertices.push_back({corners[i], color});
        // }

        return vertices;
    }
}