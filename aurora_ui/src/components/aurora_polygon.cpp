#include "aurora_ui/components/aurora_polygon.hpp"
#include "aurora_ui/graphics/aurora_render_system_manager.hpp"
#include "aurora_ui/utils/aurora_utils.hpp"

namespace aurora {
    AuroraPolygon::AuroraPolygon(AuroraComponentInfo &componentInfo, std::vector<glm::vec2> points, glm::vec4 color)
        : AuroraComponentInterface{componentInfo}, points{std::move(points)} {
        this->color = color;
        initialize();
    }

    void AuroraPolygon::initialize() {
        std::string key = "polygon";
        for (const auto& point : points) key += AuroraUtils::format(":%g,%g", point.x, point.y);

        model = componentInfo.renderSystemManager.getSharedModel(key, [&] {
            std::vector<AuroraModel::Vertex> vertices;
            vertices.reserve(points.size());
            for (const auto& point : points) vertices.push_back({point, glm::vec4(1.f)});
            return vertices;
        });
    }
}
