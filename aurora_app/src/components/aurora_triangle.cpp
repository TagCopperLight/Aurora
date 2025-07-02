#include "aurora_app/components/aurora_triangle.hpp"

#include <memory>

namespace aurora {
    AuroraTriangle::AuroraTriangle(AuroraDevice &device) : AuroraComponentInterface{device} {
        initialize();
    }

    void AuroraTriangle::initialize() {
        std::vector<AuroraModel::Vertex> vertices = {
            {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
        };

        AuroraModel::Builder builder{};
        builder.vertices = vertices;
        model = std::make_shared<AuroraModel>(auroraDevice, builder);
        color = {1.0f, 1.0f, 1.0f, 1.0f};
    }
}