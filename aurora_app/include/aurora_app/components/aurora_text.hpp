#pragma once

#include "aurora_component_interface.hpp"
#include "aurora_app/graphics/aurora_msdf_atlas.hpp"

#include <string>
#include <memory>

namespace aurora {
    class AuroraText : public AuroraComponentInterface {
        public:
            AuroraText(AuroraDevice &device, const std::string& text, const AuroraMSDFAtlas& atlas, float fontSize = 24.0f);

            // Component type-specific shader and topology information
            const std::string& getVertexShaderPath() const override {
                static const std::string vertexPath = "aurora_app/shaders/text.vert.spv";
                return vertexPath;
            }
            
            const std::string& getFragmentShaderPath() const override {
                static const std::string fragmentPath = "aurora_app/shaders/text.frag.spv";
                return fragmentPath;
            }
            
            VkPrimitiveTopology getTopology() const override {
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            }
            
            // Text management
            void setText(const std::string& newText);
            const std::string& getText() const { return text; }
            
            void setFontSize(float newFontSize);
            float getFontSize() const { return fontSize; }
            
            glm::vec2 getTextBounds() const;
            
            void rebuildGeometry();
        private:
            void initialize() override;
            
            std::vector<AuroraModel::Vertex> createTextVertices();
            void createQuadForCharacter(char character, glm::vec2 position, 
                                      std::vector<AuroraModel::Vertex>& vertices,
                                      std::vector<uint32_t>& indices, 
                                      uint32_t& currentIndex);

            std::string text;
            const AuroraMSDFAtlas& msdfAtlas;
            float fontSize;
            glm::vec2 textBounds;
    };
}
