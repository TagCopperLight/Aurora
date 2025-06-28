#pragma once

#include "aurora_component_interface.hpp"

namespace aurora {
    class AuroraTriangleComponent : public AuroraComponentInterface {
        public:
            AuroraTriangleComponent(AuroraDevice &device);

            void initialize() override;

            void update(float deltaTime) override;
    };
}