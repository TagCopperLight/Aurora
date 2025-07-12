#pragma once

#include "aurora_component_interface.hpp"

namespace aurora {
    class AuroraRoundedShadows : public AuroraComponentInterface {
        public:
            AuroraRoundedShadows(AuroraComponentInfo &componentInfo, glm::vec2 size, float radius, float borderWidth);

            const std::string& getVertexShaderPath() const override {
                static const std::string vertexPath = "shaders/shadow.vert.spv";
                return vertexPath;
            }
            
            const std::string& getFragmentShaderPath() const override {
                static const std::string fragmentPath = "shaders/shadow.frag.spv";
                return fragmentPath;
            }
            
            VkPrimitiveTopology getTopology() const override {
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            }

        private:
            void initialize() override;

            glm::vec2 size;
            float radius;
            float borderWidth;
    };
}