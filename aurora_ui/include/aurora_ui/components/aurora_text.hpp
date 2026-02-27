#pragma once

#include "aurora_component_interface.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"

#include <string>
#include <vector>

namespace aurora {
    struct TextSegment {
        std::string text;
        glm::vec4 color;
    };

    class AuroraText : public AuroraComponentInterface {
        public:
            AuroraText(AuroraComponentInfo &componentInfo, const std::string& text, float fontSize = 24.0f, glm::vec4 fontColor = AuroraThemeSettings::get().TEXT_PRIMARY);
            AuroraText(AuroraComponentInfo &componentInfo, std::vector<TextSegment> segments, float fontSize = 24.0f);

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

            bool isTransparent() const override {
                return true;
            }

            void setText(const std::string& newText);
            const std::string& getText() const { return cachedFullText; }

            void setSegments(std::vector<TextSegment> newSegments);
            const std::vector<TextSegment>& getSegments() const { return segments; }

            void setFontSize(float newFontSize);
            float getFontSize() const { return fontSize; }

            glm::vec2 getTextBounds() const;

            void rebuildGeometry();
        private:
            void initialize() override;
            void updateTextVertices();

            std::vector<TextSegment> segments;
            float fontSize;
            glm::vec2 textBounds;

            std::string cachedFullText;

            size_t currentVertexCapacity = 0;
            std::vector<AuroraModel::Vertex> cachedVertices;
    };
}
