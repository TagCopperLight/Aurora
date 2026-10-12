#include "aurora_ui/components/aurora_circle.hpp"
#include "aurora_ui/utils/aurora_utils.hpp"
#include "aurora_ui/graphics/aurora_render_system_manager.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/constants.hpp>

#include <memory>

namespace aurora {
    AuroraCircle::AuroraCircle(AuroraComponentInfo &componentInfo, float radius, glm::vec4 color)
        : AuroraComponentInterface{componentInfo}, radius{radius} {
        this->color = color;
        initialize();
    }

    void AuroraCircle::initialize() {
        model = componentInfo.renderSystemManager.getSharedModel(
            AuroraUtils::modelKey("circle", {radius, 64.f}),
            [&] { return createCircleVertices(64); });
    }

    std::vector<AuroraModel::Vertex> AuroraCircle::createCircleVertices(int numSegments) {
        std::vector<AuroraModel::Vertex> vertices;
        float angleIncrement = 2.0f * glm::pi<float>() / numSegments;

        for (int i = 0; i < numSegments; ++i) {
            float angle = i * angleIncrement;
            glm::vec2 position = {radius * glm::cos(angle), radius * glm::sin(angle)};
            vertices.push_back({position, glm::vec4(1.0f)});
        }

        return vertices;
    }
}