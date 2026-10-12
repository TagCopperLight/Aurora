#include "aurora_ui/components/aurora_rounded_rect.hpp"
#include "aurora_ui/utils/aurora_utils.hpp"
#include "aurora_ui/graphics/aurora_render_system_manager.hpp"

#include <memory>

namespace aurora {
    AuroraRoundedRectangle::AuroraRoundedRectangle(AuroraComponentInfo &componentInfo, glm::vec2 size, float radius)
        : AuroraComponentInterface{componentInfo}, size{size}, radius{radius} {
        initialize();
    }

    void AuroraRoundedRectangle::initialize() {
        glm::vec4 vertexColor = color;
        model = componentInfo.renderSystemManager.getSharedModel(
            AuroraUtils::modelKey("rounded_rect", {size.x, size.y, radius, vertexColor.r, vertexColor.g, vertexColor.b, vertexColor.a}),
            [&] { return AuroraUtils::createRoundedRectangleVertices(size, radius, 16, 0.0f, vertexColor); });
    }

}