#pragma once

#include "aurora_component_interface.hpp"

#include <string>

namespace aurora {
    class AuroraTerminal : public AuroraComponentInterface {
        public:
            AuroraTerminal(AuroraComponentInfo &componentInfo, glm::vec2 size);

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
    };
}