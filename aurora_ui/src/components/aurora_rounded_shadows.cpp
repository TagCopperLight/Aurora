#include "aurora_ui/components/aurora_rounded_shadows.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"
#include "aurora_ui/utils/aurora_utils.hpp"
#include "aurora_ui/graphics/aurora_render_system_manager.hpp"

#include <memory>

namespace aurora {
    AuroraRoundedShadows::AuroraRoundedShadows(AuroraComponentInfo &componentInfo, glm::vec2 size, float radius, float borderWidth)
        : AuroraComponentInterface{componentInfo}, size{size}, radius{radius}, borderWidth{borderWidth} {
        initialize();
    }

    void AuroraRoundedShadows::initialize() {
        const glm::vec4& outerColor = AuroraThemeSettings::get().SHADOW_TRANSPARENT;
        const glm::vec4& innerColor = AuroraThemeSettings::get().SHADOW_MEDIUM;
        model = componentInfo.renderSystemManager.getSharedModel(
            AuroraUtils::modelKey("rounded_shadows", {size.x, size.y, radius, borderWidth,
                outerColor.r, outerColor.g, outerColor.b, outerColor.a, innerColor.r, innerColor.g, innerColor.b, innerColor.a}),
            [&] {
                std::vector<AuroraModel::Vertex> outerVertices = AuroraUtils::createRoundedRectangleVertices(size, radius, 8, -borderWidth, AuroraThemeSettings::get().SHADOW_TRANSPARENT);
                std::vector<AuroraModel::Vertex> innerVertices = AuroraUtils::createRoundedRectangleVertices(size, radius, 8, 0.0f, AuroraThemeSettings::get().SHADOW_MEDIUM);
                std::vector<AuroraModel::Vertex> vertices;

                for (size_t i = 0; i < outerVertices.size(); ++i) {
                    vertices.push_back(outerVertices[i]);
                    vertices.push_back(innerVertices[i]);
                }

                vertices.push_back(outerVertices[0]);
                vertices.push_back(innerVertices[0]);
                return vertices;
            });
    }

}