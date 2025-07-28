#include "aurora_app/components/aurora_rounded_shadows.hpp"
#include "aurora_app/utils/aurora_theme_settings.hpp"
#include "aurora_app/utils/aurora_utils.hpp"

#include <memory>
#include <iostream>

namespace aurora {
    AuroraRoundedShadows::AuroraRoundedShadows(AuroraComponentInfo &componentInfo, glm::vec2 size, float radius, float borderWidth)
        : AuroraComponentInterface{componentInfo}, size{size}, radius{radius}, borderWidth{borderWidth} {
        initialize();
    }

    void AuroraRoundedShadows::initialize() {
        std::vector<AuroraModel::Vertex> outerVertices = AuroraUtils::createRoundedRectangleVertices(size, radius, 8, -borderWidth, AuroraThemeSettings::SHADOW_TRANSPARENT);
        std::vector<AuroraModel::Vertex> innerVertices = AuroraUtils::createRoundedRectangleVertices(size, radius, 8, 0.0f, AuroraThemeSettings::SHADOW_MEDIUM);
        std::vector<AuroraModel::Vertex> vertices;

        for (size_t i = 0; i < outerVertices.size(); ++i) {
            vertices.push_back(outerVertices[i]);
            vertices.push_back(innerVertices[i]);
        }

        vertices.push_back(outerVertices[0]);
        vertices.push_back(innerVertices[0]);

        AuroraModel::Builder builder{};
        builder.vertices = vertices;
        model = std::make_shared<AuroraModel>(componentInfo.auroraDevice, builder);
    }

}