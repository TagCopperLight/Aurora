#pragma once

#include "aurora_component_interface.hpp"

namespace aurora {
    class AuroraRoundedShadows : public AuroraComponentInterface {
        public:
            AuroraRoundedShadows(AuroraDevice &device, glm::vec2 size, float radius, float borderWidth);

            const std::string& getVertexShaderPath() const override {
                static const std::string vertexPath = "aurora_app/shaders/shadow.vert.spv";
                return vertexPath;
            }
            
            const std::string& getFragmentShaderPath() const override {
                static const std::string fragmentPath = "aurora_app/shaders/shadow.frag.spv";
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