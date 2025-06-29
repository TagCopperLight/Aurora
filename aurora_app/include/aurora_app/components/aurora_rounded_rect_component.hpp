#pragma once

#include "aurora_component_interface.hpp"

namespace aurora {
    class AuroraRoundedRectangleComponent : public AuroraComponentInterface {
        public:
            AuroraRoundedRectangleComponent(AuroraDevice &device, glm::vec2 size, float radius);

        private:
            void initialize() override;
            std::vector<AuroraModel::Vertex> createRoundedRectangleVertices(int numSegments);

            glm::vec2 size;
            float radius;
    };
}