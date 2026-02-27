#include "aurora_ui/components/aurora_text.hpp"
#include "aurora_ui/graphics/aurora_render_system_manager.hpp"

#include <memory>
#include "aurora_engine/utils/log.hpp"
#include <set>

namespace aurora {
    AuroraText::AuroraText(AuroraComponentInfo &componentInfo, const std::string& text, float fontSize, glm::vec4 fontColor)
        : AuroraComponentInterface{componentInfo}, segments{{{text, fontColor}}}, fontSize{fontSize} {
        initialize();
    }

    AuroraText::AuroraText(AuroraComponentInfo &componentInfo, std::vector<TextSegment> segments, float fontSize)
        : AuroraComponentInterface{componentInfo}, segments{std::move(segments)}, fontSize{fontSize} {
        initialize();
    }

    void AuroraText::initialize() {
        color = glm::vec4(1.0f);
        rebuildGeometry();
    }

    void AuroraText::setText(const std::string& newText) {
        glm::vec4 currentColor = segments.empty() ? AuroraThemeSettings::get().TEXT_PRIMARY : segments[0].color;
        if (segments.size() == 1 && segments[0].text == newText) return;
        segments = {{newText, currentColor}};
        rebuildGeometry();
    }

    void AuroraText::setSegments(std::vector<TextSegment> newSegments) {
        segments = std::move(newSegments);
        rebuildGeometry();
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
        cachedFullText.clear();
        for (const auto& seg : segments) cachedFullText += seg.text;

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

        // Build a flat character+color list so kerning works across segment boundaries
        struct CharEntry { char c; glm::vec4 color; };
        std::vector<CharEntry> chars;
        for (const auto& seg : segments) {
            for (char c : seg.text) chars.push_back({c, seg.color});
        }

        if (chars.empty()) return;

        glm::vec2 cursor = {0.0f, 0.0f};
        float scale = fontSize / 0.80741f;

        float minX = 0.0f, maxX = 0.0f;
        float minY = 0.0f, maxY = 0.0f;
        bool first = true;

        for (size_t i = 0; i < chars.size(); ++i) {
            const char character = chars[i].c;
            const glm::vec4& charColor = chars[i].color;

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
                    log::ui()->warn("Glyph not found for character: '{}'", character);
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

            AuroraModel::Vertex v1(glm::vec3(glyphPos.x, glyphPos.y, 0.0f), charColor);
            v1.texCoord = glm::vec2(glyphInfo.atlasBounds.x, glyphInfo.atlasBounds.y + glyphInfo.atlasBounds.w);
            cachedVertices.push_back(v1);

            AuroraModel::Vertex v2(glm::vec3(glyphPos.x + glyphSize.x, glyphPos.y, 0.0f), charColor);
            v2.texCoord = glm::vec2(glyphInfo.atlasBounds.x + glyphInfo.atlasBounds.z, glyphInfo.atlasBounds.y + glyphInfo.atlasBounds.w);
            cachedVertices.push_back(v2);

            AuroraModel::Vertex v3(glm::vec3(glyphPos.x + glyphSize.x, glyphPos.y + glyphSize.y, 0.0f), charColor);
            v3.texCoord = glm::vec2(glyphInfo.atlasBounds.x + glyphInfo.atlasBounds.z, glyphInfo.atlasBounds.y);
            cachedVertices.push_back(v3);

            AuroraModel::Vertex v4(glm::vec3(glyphPos.x, glyphPos.y + glyphSize.y, 0.0f), charColor);
            v4.texCoord = glm::vec2(glyphInfo.atlasBounds.x, glyphInfo.atlasBounds.y);
            cachedVertices.push_back(v4);

            cursor.x += static_cast<float>(glyphInfo.advance) * scale;

            if (i < chars.size() - 1) {
                double kerning = msdfAtlas.getKerning(character, chars[i + 1].c);
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
