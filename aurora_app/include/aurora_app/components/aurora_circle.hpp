#pragma once

#include "aurora_component_interface.hpp"

namespace aurora {
    class AuroraCircle : public AuroraComponentInterface {
        public:
            AuroraCircle(AuroraComponentInfo &componentInfo, float radius, glm::vec4 color);

            const std::string& getVertexShaderPath() const override {
                static const std::string vertexPath = "shaders/shader.vert.spv";
                return vertexPath;
            }
            
            const std::string& getFragmentShaderPath() const override {
                static const std::string fragmentPath = "shaders/shader.frag.spv";
                return fragmentPath;
            }
            
            VkPrimitiveTopology getTopology() const override {
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            }

        private:
            void initialize() override;
            std::vector<AuroraModel::Vertex> createCircleVertices(int numSegments);

            float radius;
    };
}