#include "aurora_ui/components/aurora_triangle.hpp"
#include "aurora_ui/graphics/aurora_render_system_manager.hpp"

#include <memory>

namespace aurora {
    AuroraTriangle::AuroraTriangle(AuroraComponentInfo &componentInfo) : AuroraComponentInterface{componentInfo} {
        initialize();
    }

    void AuroraTriangle::initialize() {
        model = componentInfo.renderSystemManager.getSharedModel("triangle", [] {
            return std::vector<AuroraModel::Vertex>{
                {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
                {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
                {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
            };
        });
        color = {1.0f, 1.0f, 1.0f, 1.0f};
    }
}