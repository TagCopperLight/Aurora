#include "aurora_app/components/aurora_rounded_rect.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/constants.hpp>

#include "spdlog/spdlog.h"

#include <memory>
#include <iostream>

namespace aurora {
    AuroraRoundedRectangle::AuroraRoundedRectangle(AuroraDevice &device, glm::vec2 size, float radius)
        : AuroraComponentInterface{device}, size{size}, radius{radius} {
        initialize();
    }

    void AuroraRoundedRectangle::initialize() {
        std::vector<AuroraModel::Vertex> vertices = createRoundedRectangleVertices(16);
        AuroraModel::Builder builder{};
        builder.vertices = vertices;
        model = std::make_shared<AuroraModel>(auroraDevice, builder);
        color = {1.0f, 1.0f, 1.0f, 1.0f};
    }

    std::vector<AuroraModel::Vertex> AuroraRoundedRectangle::createRoundedRectangleVertices(int numSegments) {
        std::vector<AuroraModel::Vertex> vertices;

        std::vector<glm::vec2> corners = {
            // Bottom left - Top left - Top right - Bottom right
            {radius, size.y - radius}, // Bottom left
            {radius, radius}, // Top left           
            {size.x - radius, radius}, // Top right
            {size.x - radius, size.y - radius}, // Bottom right
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

        return vertices;
    }
}