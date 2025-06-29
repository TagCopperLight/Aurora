#pragma once

#include "aurora_component_interface.hpp"

namespace aurora {
    class AuroraRoundedRectangle : public AuroraComponentInterface {
        public:
            AuroraRoundedRectangle(AuroraDevice &device, glm::vec2 size, float radius);

            // Component type-specific shader and topology information
            const std::string& getVertexShaderPath() const override {
                static const std::string vertexPath = "aurora_app/shaders/shader.vert.spv";
                return vertexPath;
            }
            
            const std::string& getFragmentShaderPath() const override {
                static const std::string fragmentPath = "aurora_app/shaders/shader.frag.spv";
                return fragmentPath;
            }
            
            VkPrimitiveTopology getTopology() const override {
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            }

        private:
            void initialize() override;
            std::vector<AuroraModel::Vertex> createRoundedRectangleVertices(int numSegments);

            glm::vec2 size;
            float radius;
    };
}