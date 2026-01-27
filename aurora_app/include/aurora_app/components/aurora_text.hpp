#pragma once

#include "aurora_component_interface.hpp"
#include "aurora_app/utils/aurora_theme_settings.hpp"

#include <string>

namespace aurora {
    class AuroraText : public AuroraComponentInterface {
        public:
            AuroraText(AuroraComponentInfo &componentInfo, const std::string& text, float fontSize = 24.0f, glm::vec4 fontColor = AuroraThemeSettings::TEXT_PRIMARY);

            const std::string& getVertexShaderPath() const override {
                static const std::string vertexPath = "shaders/text.vert.spv";
                return vertexPath;
            }
            
            const std::string& getFragmentShaderPath() const override {
                static const std::string fragmentPath = "shaders/text.frag.spv";
                return fragmentPath;
            }
            
            VkPrimitiveTopology getTopology() const override {
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            }
            
            bool needsTextureBinding() const override {
                return true;
            }
            
            void setText(const std::string& newText);
            const std::string& getText() const { return text; }
            
            void setFontSize(float newFontSize);
            float getFontSize() const { return fontSize; }
            
            glm::vec2 getTextBounds() const;
            
            void rebuildGeometry();
        private:
            void initialize() override;
            
            std::vector<AuroraModel::Vertex> createTextVertices();
            void createQuadForCharacter(
                char character,
                glm::vec2 position, 
                std::vector<AuroraModel::Vertex>& vertices,
                std::vector<uint32_t>& indices, 
                uint32_t& currentIndex
            );

            std::string text;
            float fontSize;
            glm::vec4 fontColor;
            glm::vec2 textBounds;
            
            size_t currentVertexCapacity = 0;
            size_t currentIndexCapacity = 0;
    };
}
