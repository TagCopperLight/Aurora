#include "aurora_app/components/aurora_circle_component.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/constants.hpp>

#include <memory>

namespace aurora {
    AuroraCircleComponent::AuroraCircleComponent(AuroraDevice &device, float radius, glm::vec2 position)
        : AuroraComponentInterface{device}, radius{radius}, position{position} {
        initialize();
    }

    void AuroraCircleComponent::initialize() {
        std::vector<AuroraModel::Vertex> vertices = createCircleVertices(64);
        model = std::make_shared<AuroraModel>(auroraDevice, vertices);
        color = {1.0f, 1.0f, 1.0f};
    }

    std::vector<AuroraModel::Vertex> AuroraCircleComponent::createCircleVertices(int numSegments) {
        std::vector<AuroraModel::Vertex> vertices;
        float angleIncrement = 2.0f * glm::pi<float>() / numSegments;

        for (int i = 0; i < numSegments; ++i) {
            float angle = i * angleIncrement;
            glm::vec2 position = {radius * glm::cos(angle), radius * glm::sin(angle)};
            vertices.push_back({position, color});
        }

        return vertices;
    }
}