#include "aurora_ui/components/aurora_segment.hpp"
#include "aurora_ui/graphics/aurora_render_system_manager.hpp"

#include <cmath>

namespace aurora {
    AuroraSegment::AuroraSegment(AuroraComponentInfo &componentInfo, glm::vec4 color) : AuroraComponentInterface{componentInfo} {
        this->color = color;
        initialize();
    }

    void AuroraSegment::setEndpoints(glm::vec2 from, glm::vec2 to, float thickness) {
        glm::vec2 delta = to - from;
        setPosition(from);
        setRotation(std::atan2(delta.y, delta.x));
        setScale(glm::length(delta), thickness);
    }

    void AuroraSegment::initialize() {
        model = componentInfo.renderSystemManager.getSharedModel("segment", [] {
            return std::vector<AuroraModel::Vertex>{
                {glm::vec2{0.f, -0.5f}, glm::vec4(1.f)},
                {glm::vec2{1.f, -0.5f}, glm::vec4(1.f)},
                {glm::vec2{1.f, 0.5f}, glm::vec4(1.f)},
                {glm::vec2{0.f, 0.5f}, glm::vec4(1.f)},
            };
        });
    }
}
