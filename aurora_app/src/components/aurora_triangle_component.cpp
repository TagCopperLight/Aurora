#include "aurora_app/components/aurora_triangle_component.hpp"

#include <memory>

namespace aurora {
    AuroraTriangleComponent::AuroraTriangleComponent(AuroraDevice &device) : AuroraComponentInterface{device} {
        initialize();
    }

    void AuroraTriangleComponent::initialize() {
        std::vector<AuroraModel::Vertex> vertices = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };

        model = std::make_shared<AuroraModel>(auroraDevice, vertices);
        color = {1.0f, 1.0f, 1.0f};
    }

    void AuroraTriangleComponent::update(float deltaTime) {
        transform.rotation += deltaTime;
    }
}