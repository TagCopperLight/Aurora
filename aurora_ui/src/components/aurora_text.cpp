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
        if (segments.size() == 1) {
            if (segments[0].text == newText) return;
            segments[0].text = newText; // reuses the string's capacity
        } else {
            glm::vec4 currentColor = segments.empty() ? AuroraThemeSettings::get().TEXT_PRIMARY : segments[0].color;
            segments = {{newText, currentColor}};
        }
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

        size_t charCount = 0;
        for (const auto& seg : segments) charCount += seg.text.size();

        if (charCount == 0) return;
        cachedVertices.reserve(charCount * 4);

        const bool useKerning = msdfAtlas.hasKerning();
        const AuroraMSDFAtlas::GlyphInfo* spaceInfo = msdfAtlas.findGlyph(' ');

        glm::vec2 cursor = {0.0f, 0.0f};
        float scale = fontSize / 0.80741f;

        float minX = 0.0f, maxX = 0.0f;
        float minY = 0.0f, maxY = 0.0f;
        bool first = true;

        // Kerning works across segment boundaries, so the next character may come from a later segment
        auto nextCharacter = [&](size_t segmentIndex, size_t charIndex, char& next) {
            if (charIndex + 1 < segments[segmentIndex].text.size()) {
                next = segments[segmentIndex].text[charIndex + 1];
                return true;
            }
            for (size_t s = segmentIndex + 1; s < segments.size(); ++s) {
                if (!segments[s].text.empty()) {
                    next = segments[s].text[0];
                    return true;
                }
            }
            return false;
        };

        for (size_t segmentIndex = 0; segmentIndex < segments.size(); ++segmentIndex) {
            const std::string& text = segments[segmentIndex].text;
            const glm::vec4& charColor = segments[segmentIndex].color;

            for (size_t charIndex = 0; charIndex < text.size(); ++charIndex) {
                const char character = text[charIndex];

                if (character == ' ') {
                    if (spaceInfo) {
                        cursor.x += static_cast<float>(spaceInfo->advance) * scale;
                    } else {
                        cursor.x += fontSize * 0.25f;
                    }
                    continue;
                }

                const AuroraMSDFAtlas::GlyphInfo* glyphInfoPtr = msdfAtlas.findGlyph(character);
                if (!glyphInfoPtr) {
                    static std::set<char> missingChars;
                    if (missingChars.find(character) == missingChars.end()) {
                        log::ui()->warn("Glyph not found for character: '{}'", character);
                        missingChars.insert(character);
                    }
                    continue;
                }
                const AuroraMSDFAtlas::GlyphInfo& glyphInfo = *glyphInfoPtr;

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

                AuroraModel::Vertex& v1 = cachedVertices.emplace_back(glm::vec3(glyphPos.x, glyphPos.y, 0.0f), charColor);
                v1.texCoord = glm::vec2(glyphInfo.atlasBounds.x, glyphInfo.atlasBounds.y + glyphInfo.atlasBounds.w);

                AuroraModel::Vertex& v2 = cachedVertices.emplace_back(glm::vec3(glyphPos.x + glyphSize.x, glyphPos.y, 0.0f), charColor);
                v2.texCoord = glm::vec2(glyphInfo.atlasBounds.x + glyphInfo.atlasBounds.z, glyphInfo.atlasBounds.y + glyphInfo.atlasBounds.w);

                AuroraModel::Vertex& v3 = cachedVertices.emplace_back(glm::vec3(glyphPos.x + glyphSize.x, glyphPos.y + glyphSize.y, 0.0f), charColor);
                v3.texCoord = glm::vec2(glyphInfo.atlasBounds.x + glyphInfo.atlasBounds.z, glyphInfo.atlasBounds.y);

                AuroraModel::Vertex& v4 = cachedVertices.emplace_back(glm::vec3(glyphPos.x, glyphPos.y + glyphSize.y, 0.0f), charColor);
                v4.texCoord = glm::vec2(glyphInfo.atlasBounds.x, glyphInfo.atlasBounds.y);

                cursor.x += static_cast<float>(glyphInfo.advance) * scale;

                char next;
                if (useKerning && nextCharacter(segmentIndex, charIndex, next)) {
                    double kerning = msdfAtlas.getKerning(character, next);
                    cursor.x += static_cast<float>(kerning) * scale;
                }
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
