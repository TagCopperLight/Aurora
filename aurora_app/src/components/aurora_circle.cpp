#include "aurora_app/components/aurora_circle.hpp"
#include "aurora_app/utils/aurora_theme_settings.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/constants.hpp>

#include <memory>

namespace aurora {
    AuroraCircle::AuroraCircle(AuroraComponentInfo &componentInfo, float radius)
        : AuroraComponentInterface{componentInfo}, radius{radius} {
        initialize();
    }

    void AuroraCircle::initialize() {
        std::vector<AuroraModel::Vertex> vertices = createCircleVertices(64);
        AuroraModel::Builder builder{};
        builder.vertices = vertices;
        model = std::make_shared<AuroraModel>(componentInfo.auroraDevice, builder);
        color = AuroraThemeSettings::WHITE;
    }

    std::vector<AuroraModel::Vertex> AuroraCircle::createCircleVertices(int numSegments) {
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