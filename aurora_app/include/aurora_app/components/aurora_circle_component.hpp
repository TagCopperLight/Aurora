#pragma once

#include "aurora_component_interface.hpp"

namespace aurora {
    class AuroraCircleComponent : public AuroraComponentInterface {
        public:
            AuroraCircleComponent(AuroraDevice &device, float radius);

        private:
            void initialize() override;
            std::vector<AuroraModel::Vertex> createCircleVertices(int numSegments);

            float radius;
    };
}