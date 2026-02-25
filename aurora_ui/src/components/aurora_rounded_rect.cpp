#include "aurora_ui/components/aurora_rounded_rect.hpp"
#include "aurora_ui/utils/aurora_utils.hpp"

#include <memory>

namespace aurora {
    AuroraRoundedRectangle::AuroraRoundedRectangle(AuroraComponentInfo &componentInfo, glm::vec2 size, float radius)
        : AuroraComponentInterface{componentInfo}, size{size}, radius{radius} {
        initialize();
    }

    void AuroraRoundedRectangle::initialize() {
        std::vector<AuroraModel::Vertex> vertices = AuroraUtils::createRoundedRectangleVertices(size, radius, 16, 0.0f, color);
        AuroraModel::Builder builder{};
        builder.vertices = vertices;
        model = std::make_shared<AuroraModel>(componentInfo.auroraDevice, builder);
    }

}