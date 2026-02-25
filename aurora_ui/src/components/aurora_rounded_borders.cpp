#include "aurora_ui/components/aurora_rounded_borders.hpp"
#include "aurora_ui/utils/aurora_utils.hpp"

#include <memory>

namespace aurora {
    AuroraRoundedBorders::AuroraRoundedBorders(AuroraComponentInfo &componentInfo, glm::vec2 size, float radius, float borderWidth)
        : AuroraComponentInterface{componentInfo}, size{size}, radius{radius}, borderWidth{borderWidth} {
        initialize();
    }

    void AuroraRoundedBorders::initialize() {
        std::vector<AuroraModel::Vertex> outerVertices = AuroraUtils::createRoundedRectangleVertices(size, radius, 8, 0.0f, color);
        std::vector<AuroraModel::Vertex> innerVertices = AuroraUtils::createRoundedRectangleVertices(size, radius, 8, borderWidth, color);
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