#pragma once

#include "aurora_component_interface.hpp"

#include <string>

namespace aurora {
    class AuroraCard : public AuroraComponentInterface {
        public:
            AuroraCard(AuroraComponentInfo &componentInfo, glm::vec2 size, glm::vec4 borderColor, std::string title = "");

            const std::string& getVertexShaderPath() const override {
                static const std::string vertexPath = "shaders/shader.vert.spv";
                return vertexPath;
            }
            
            const std::string& getFragmentShaderPath() const override {
                static const std::string fragmentPath = "shaders/shader.frag.spv";
                return fragmentPath;
            }
            
            VkPrimitiveTopology getTopology() const override {
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            }

        private:
            void initialize() override;

            glm::vec2 size;
            glm::vec4 borderColor;
            std::string title;
    };
}