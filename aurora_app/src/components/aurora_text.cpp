#include "aurora_app/components/aurora_text.hpp"
#include "aurora_app/graphics/aurora_render_system_manager.hpp"

#include <memory>
#include <spdlog/spdlog.h>
#include <set>

namespace aurora {
    AuroraText::AuroraText(AuroraComponentInfo &componentInfo, const std::string& text, float fontSize, glm::vec4 fontColor)
        : AuroraComponentInterface{componentInfo}, text{text}, fontSize{fontSize}, fontColor{fontColor} {
        initialize();
    }

    void AuroraText::initialize() {
        color = fontColor;
        rebuildGeometry();
    }

    void AuroraText::setText(const std::string& newText) {
        if (text != newText) {
            text = newText;
            rebuildGeometry();
        }
    }

    void AuroraText::setFontSize(float newFontSize) {
        if (fontSize != newFontSize) {
            fontSize = newFontSize;
            rebuildGeometry();
        }
    }

    glm::vec2 AuroraText::getTextBounds() const {
        return textBounds;
    }



    void AuroraText::rebuildGeometry() {
        updateTextVertices();
        
        AuroraMSDFAtlas& msdfAtlas = componentInfo.renderSystemManager.getMSDFAtlas();

        if (model && model->isDynamic()) {
            VkDeviceSize requiredSize = cachedVertices.size() * sizeof(AuroraModel::Vertex);
            model->resizeVertexBuffer(requiredSize);
            model->updateVertexData(cachedVertices.data(), requiredSize);
            model->setVertexCount(static_cast<uint32_t>(cachedVertices.size()));
            model->setIndexCount(static_cast<uint32_t>(cachedVertices.size() / 4 * 6));
            return;
        }

        size_t vertexCapacity = cachedVertices.size() > 0 ? cachedVertices.size() * 2 : 128;
        
        if (cachedVertices.capacity() < vertexCapacity) cachedVertices.reserve(vertexCapacity);

        AuroraModel::Builder builder{};
        builder.vertices = cachedVertices;
        builder.vertices.resize(vertexCapacity);
        
        builder.sharedIndexAllocation = &msdfAtlas.getSharedIndexAllocation();
        builder.isDynamic = true;
        
        model = std::make_shared<AuroraModel>(componentInfo.auroraDevice, builder);
        
        model->setVertexCount(static_cast<uint32_t>(cachedVertices.size()));
        model->setIndexCount(static_cast<uint32_t>(cachedVertices.size() / 4 * 6));
        
        currentVertexCapacity = vertexCapacity;
    }

    void AuroraText::updateTextVertices() {
        cachedVertices.clear();
        const AuroraMSDFAtlas& msdfAtlas = componentInfo.renderSystemManager.getMSDFAtlas();

        if (text.empty()) {
            return;
        }

        glm::vec2 cursor = {0.0f, 0.0f};
        float scale = fontSize / 0.80741f;
        
        float minX = 0.0f, maxX = 0.0f;
        float minY = 0.0f, maxY = 0.0f;
        bool first = true;

        for (size_t i = 0; i < text.length(); ++i) {
            char character = text[i];
            
            
            if (character == ' ') {
                AuroraMSDFAtlas::GlyphInfo spaceInfo;
                if (msdfAtlas.getGlyphInfo(' ', spaceInfo)) {
                    cursor.x += static_cast<float>(spaceInfo.advance) * scale;
                } else {
                    cursor.x += fontSize * 0.25f; 
                }
                continue;
            }

            AuroraMSDFAtlas::GlyphInfo glyphInfo;
            if (!msdfAtlas.getGlyphInfo(character, glyphInfo)) {
                static std::set<char> missingChars;
                if (missingChars.find(character) == missingChars.end()) {
                    spdlog::warn("Glyph not found for character: '{}'", character);
                    missingChars.insert(character);
                }
                continue;
            }

            
            glm::vec2 glyphPos = cursor;
            glyphPos.x += glyphInfo.planeBounds.x * scale;
            glyphPos.y -= (glyphInfo.planeBounds.y + glyphInfo.planeBounds.w) * scale;
            
            glm::vec2 glyphSize = {
                glyphInfo.planeBounds.z * scale,
                glyphInfo.planeBounds.w * scale
            };
            
            if (first) {
                minX = glyphPos.x;
                maxX = glyphPos.x + glyphSize.x;
                minY = glyphPos.y;
                maxY = glyphPos.y + glyphSize.y;
                first = false;
            } else {
                minX = std::min(minX, glyphPos.x);
                maxX = std::max(maxX, glyphPos.x + glyphSize.x);
                minY = std::min(minY, glyphPos.y);
                maxY = std::max(maxY, glyphPos.y + glyphSize.y);
            }

            AuroraModel::Vertex v1(glm::vec3(glyphPos.x, glyphPos.y, 0.0f), color);
            v1.texCoord = glm::vec2(glyphInfo.atlasBounds.x, glyphInfo.atlasBounds.y + glyphInfo.atlasBounds.w);
            cachedVertices.push_back(v1);
            
            AuroraModel::Vertex v2(glm::vec3(glyphPos.x + glyphSize.x, glyphPos.y, 0.0f), color);
            v2.texCoord = glm::vec2(glyphInfo.atlasBounds.x + glyphInfo.atlasBounds.z, glyphInfo.atlasBounds.y + glyphInfo.atlasBounds.w);
            cachedVertices.push_back(v2);
            
            AuroraModel::Vertex v3(glm::vec3(glyphPos.x + glyphSize.x, glyphPos.y + glyphSize.y, 0.0f), color);
            v3.texCoord = glm::vec2(glyphInfo.atlasBounds.x + glyphInfo.atlasBounds.z, glyphInfo.atlasBounds.y);
            cachedVertices.push_back(v3);
            
            AuroraModel::Vertex v4(glm::vec3(glyphPos.x, glyphPos.y + glyphSize.y, 0.0f), color);
            v4.texCoord = glm::vec2(glyphInfo.atlasBounds.x, glyphInfo.atlasBounds.y);
            cachedVertices.push_back(v4);
            
            cursor.x += static_cast<float>(glyphInfo.advance) * scale;
            
            if (i < text.length() - 1) {
                double kerning = msdfAtlas.getKerning(character, text[i + 1]);
                cursor.x += static_cast<float>(kerning) * scale;
            }
        }

        float offsetX = -minX;
        float offsetY = -minY;
        for (auto& vertex : cachedVertices) {
            vertex.position.x += offsetX;
            vertex.position.y += offsetY;
        }

        textBounds = {maxX - minX, maxY - minY};
    }
}
