#include "aurora_app/components/aurora_rounded_shadows.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/constants.hpp>

#include <memory>
#include <iostream>

namespace aurora {
    AuroraRoundedShadows::AuroraRoundedShadows(AuroraDevice &device, glm::vec2 size, float radius, float borderWidth)
        : AuroraComponentInterface{device}, size{size}, radius{radius}, borderWidth{borderWidth} {
        initialize();
    }

    void AuroraRoundedShadows::initialize() {
        std::vector<AuroraModel::Vertex> outerVertices = createRoundedRectangleVertices(8, borderWidth);
        std::vector<AuroraModel::Vertex> innerVertices = createRoundedRectangleVertices(8, 0.f);
        std::vector<AuroraModel::Vertex> vertices;

        for (size_t i = 0; i < outerVertices.size(); ++i) {
            vertices.push_back(outerVertices[i]);
            vertices.back().color = {0.0f, 0.0f, 0.0f, 0.0f};
            
            vertices.push_back(innerVertices[i]);
            vertices.back().color = {0.0f, 0.0f, 0.0f, 0.3f};
        }

        vertices.push_back(outerVertices[0]);
        vertices.back().color = {0.0f, 0.0f, 0.0f, 0.0f};

        vertices.push_back(innerVertices[0]);
        vertices.back().color = {0.0f, 0.0f, 0.0f, 0.3f};

        AuroraModel::Builder builder{};
        builder.vertices = vertices;
        model = std::make_shared<AuroraModel>(auroraDevice, builder);
        color = {1.0f, 1.0f, 1.0f, 1.0f};
    }

    std::vector<AuroraModel::Vertex> AuroraRoundedShadows::createRoundedRectangleVertices(int numSegments, float deltaLength) {
        std::vector<AuroraModel::Vertex> vertices;
        float nRadius = radius + deltaLength;
        deltaLength = - deltaLength;
        
        std::vector<glm::vec2> corners = {
            {nRadius + deltaLength, -nRadius - deltaLength},                      
            {nRadius + deltaLength, -size.y + nRadius + deltaLength},            
            {size.x - nRadius - deltaLength, -size.y + nRadius + deltaLength},         
            {size.x - nRadius - deltaLength, -nRadius - deltaLength},                  
        };

        for (int i = 0; i < 4; ++i) {
            glm::vec2 corner = corners[i];
            float angleStart = glm::pi<float>() * 0.5f * i + glm::pi<float>() * 0.5f; 
            float angleEnd = angleStart + glm::pi<float>() * 0.5f;

            for (int j = 0; j <= numSegments; ++j) {
                float angle = angleStart + (angleEnd - angleStart) * j / numSegments;
                glm::vec2 position = corner + nRadius * glm::vec2(glm::cos(angle), glm::sin(angle));
                vertices.push_back({position, color});
            }
        }

        return vertices;
    }
}