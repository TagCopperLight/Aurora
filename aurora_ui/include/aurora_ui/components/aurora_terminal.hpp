#pragma once

#include "aurora_component_interface.hpp"

#include <string>
#include <vector>

namespace aurora {
    class AuroraTerminal : public AuroraComponentInterface {
        public:
            AuroraTerminal(AuroraComponentInfo &componentInfo, glm::vec2 size, float fontSize = 15.0f, float padding = 40.0f);

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

            void addText(const std::string& text);

        private:
            void initialize() override;
            void calculateDimensions();
            std::vector<std::string> wrapText(const std::string& text, size_t maxWidth);
            void refreshDisplay();

            glm::vec2 size;
            float fontSize;
            float padding;
            float lineHeight;
            int maxLines;
            int maxCharsPerLine;
            
            std::vector<std::string> lines;
    };
}