#pragma once

#include "aurora_component_interface.hpp"

namespace aurora {
    class AuroraRoundedBorders : public AuroraComponentInterface {
        public:
            AuroraRoundedBorders(AuroraDevice &device, glm::vec2 size, float radius, float borderWidth);

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
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            }

        private:
            void initialize() override;
            std::vector<AuroraModel::Vertex> createRoundedRectangleVertices(int numSegments, float deltaLength);

            glm::vec2 size;
            float radius;
            float borderWidth;
    };
}