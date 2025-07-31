#pragma once

#include "aurora_component_interface.hpp"
#include "aurora_app/components/aurora_text.hpp"
#include <string>

namespace aurora {
    class AuroraUI : public AuroraComponentInterface {
        public:
            AuroraUI(AuroraComponentInfo &componentInfo, float width);

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

            void addTitle(std::string title);
            void addStringVariable(std::string title, std::string* variable);
            void refreshVariables();

        private:
            void initialize() override;

            float width;
            glm::vec2 cursorPosition = {50.f, 30.f};

            std::vector<std::shared_ptr<AuroraText>> variableComponents;
            std::vector<std::string*> variables;
    };
}