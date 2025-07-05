#include "aurora_app/graphics/aurora_msdf_atlas.hpp"

#include <stdexcept>
#include <spdlog/spdlog.h>
#include <cstring>

namespace aurora {
    AuroraMSDFAtlas::AuroraMSDFAtlas(AuroraDevice& device, const std::string& fontPath)
    : AuroraMSDFAtlas(device, fontPath, Config{}) {
    }

    AuroraMSDFAtlas::AuroraMSDFAtlas(AuroraDevice& device, const std::string& fontPath, const Config& config)
    : auroraDevice{device}, config{config}, fontHandle{nullptr}, fontGeometry{&glyphGeometry} {
        // Initialize FreeType handle
        freetypeHandle = msdfgen::initializeFreetype();

        if (!loadFont(fontPath)) {
            throw std::runtime_error("Failed to load font: " + fontPath);
        }
        
        if (!generateAtlas()) {
            throw std::runtime_error("Failed to generate MSDF atlas");
        }

        uploadAtlasToTexture();
    }

    AuroraMSDFAtlas::~AuroraMSDFAtlas() {
        freeFont();
        msdfgen::deinitializeFreetype(freetypeHandle);
    }

    bool AuroraMSDFAtlas::loadFont(const std::string& fontPath) {
        freeFont();

        fontHandle = msdfgen::loadFont(freetypeHandle, fontPath.c_str());

        if (!fontHandle) {
            spdlog::error("Failed to load font from path: {}", fontPath);
            return false;
        }

        spdlog::info("Font loaded successfully from path: {}", fontPath);
        return true;
    }

    bool AuroraMSDFAtlas::generateAtlas() {
        if (!fontHandle) {
            spdlog::error("Font handle is null, cannot generate atlas");
            return false;
        }

        std::vector<msdf_atlas::GlyphGeometry> glyphs;
        msdf_atlas::FontGeometry fontGeometry(&glyphs);

        fontGeometry.loadCharset(fontHandle, 1.0, msdf_atlas::Charset::ASCII);

        const double maxCornerAngle = 3.0;
        for (msdf_atlas::GlyphGeometry &glyph : glyphs) {
            glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);
        }

        msdf_atlas::TightAtlasPacker packer;
        packer.setDimensionsConstraint(msdf_atlas::DimensionsConstraint::SQUARE);
        
        packer.setMinimumScale(24); // Set minimum scale for glyphs
        packer.setPixelRange(2.0); // Set pixel range for distance field
        packer.setMiterLimit(1.0); // Set miter limit for glyphs

        packer.pack(glyphs.data(), static_cast<int>(glyphs.size()));

        int width = 0, height = 0;
        packer.getDimensions(width, height);

        config.width = width;
        config.height = height;

        msdf_atlas::ImmediateAtlasGenerator<
            float,
            3,
            msdf_atlas::msdfGenerator,
            msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3>
        > generator(config.width, config.height);

        msdf_atlas::GeneratorAttributes attributes;
        
        generator.setAttributes(attributes);
        generator.setThreadCount(4);

        generator.generate(glyphs.data(), static_cast<int>(glyphs.size()));

        // Store the generated atlas bitmap data
        atlasStorage = std::make_unique<msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3>>(
            generator.atlasStorage()
        );

        // Store glyph geometry for later access
        glyphGeometry = std::move(glyphs);
        this->fontGeometry = msdf_atlas::FontGeometry(&glyphGeometry);

        createAtlasTexture();

        return true;
    }

    void AuroraMSDFAtlas::createAtlasTexture() {
        VkExtent3D extent = {config.width, config.height, 1};

        atlasTexture = std::make_unique<AuroraTexture>(
            auroraDevice,
            VK_FORMAT_R8G8B8A8_UNORM,
            extent,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_SAMPLE_COUNT_1_BIT
        );

        atlasTexture->updateDescriptor();
    }

    void AuroraMSDFAtlas::freeFont() {
        if (fontHandle) {
            msdfgen::destroyFont(fontHandle);
            fontHandle = nullptr;
        }
    }

    void AuroraMSDFAtlas::uploadAtlasToTexture() {
        if (!atlasTexture || !atlasStorage) {
            spdlog::error("Atlas texture or storage is null, cannot upload data");
            return;
        }

        msdfgen::BitmapConstRef<msdf_atlas::byte, 3> bitmapRef = *atlasStorage;
        const msdf_atlas::byte* bitmapData = bitmapRef.pixels;

        std::vector<uint8_t> rgbaData(config.width * config.height * 4);
        for (size_t i = 0; i < config.width * config.height; ++i) {
            rgbaData[i * 4 + 0] = bitmapData[i * 3 + 0]; // R
            rgbaData[i * 4 + 1] = bitmapData[i * 3 + 1]; // G
            rgbaData[i * 4 + 2] = bitmapData[i * 3 + 2]; // B
            rgbaData[i * 4 + 3] = 255;
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        VkDeviceSize bufferSize = rgbaData.size();

        auroraDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory
        );

        void* data;
        vkMapMemory(auroraDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, rgbaData.data(), bufferSize);
        vkUnmapMemory(auroraDevice.device(), stagingBufferMemory);

        VkCommandBuffer commandBuffer = auroraDevice.beginSingleTimeCommands();
        
        atlasTexture->transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {config.width, config.height, 1};

        vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, atlasTexture->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        
        atlasTexture->transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        
        auroraDevice.endSingleTimeCommands(commandBuffer);

        vkDestroyBuffer(auroraDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(auroraDevice.device(), stagingBufferMemory, nullptr);
        
        spdlog::info("Successfully uploaded MSDF atlas data to texture");
    }
    
    void AuroraMSDFAtlas::saveAtlasAsPNG(const std::string& outputPath) const {
        if (!atlasTexture) {
            spdlog::error("Atlas texture is not initialized, cannot save as PNG");
            return;
        }

        msdfgen::BitmapConstRef<msdf_atlas::byte, 3> bitmapRef = *atlasStorage;

        bool success = msdf_atlas::saveImage(
            bitmapRef,
            msdf_atlas::ImageFormat::PNG,
            outputPath.c_str()
        );

        if (success) {
            spdlog::info("Successfully saved MSDF atlas as PNG to {}", outputPath);
        } else {
            spdlog::error("Failed to save MSDF atlas as PNG");
        }
    }
    
    bool AuroraMSDFAtlas::getGlyphInfo(char character, GlyphInfo& glyphInfo) const {
        // Find the glyph in the geometry
        for (const auto& glyph : glyphGeometry) {
            if (glyph.getCodepoint() == static_cast<msdf_atlas::unicode_t>(character)) {
                // Get atlas bounds (texture coordinates)
                double left, bottom, right, top;
                glyph.getQuadAtlasBounds(left, bottom, right, top);
                
                // Convert to normalized atlas coordinates (0-1)
                glyphInfo.atlasBounds = glm::vec4(
                    static_cast<float>(left / config.width),
                    static_cast<float>(bottom / config.height),
                    static_cast<float>((right - left) / config.width),
                    static_cast<float>((top - bottom) / config.height)
                );
                
                // Get plane bounds (font units)
                glyph.getQuadPlaneBounds(left, bottom, right, top);
                glyphInfo.planeBounds = glm::vec4(
                    static_cast<float>(left),
                    static_cast<float>(bottom),
                    static_cast<float>(right - left),
                    static_cast<float>(top - bottom)
                );
                
                // Get advance
                glyphInfo.advance = glyph.getAdvance();
                
                return true;
            }
        }
        return false;
    }
    
    double AuroraMSDFAtlas::getKerning(char left, char right) const {
        // Find both glyphs
        const msdf_atlas::GlyphGeometry* leftGlyph = nullptr;
        const msdf_atlas::GlyphGeometry* rightGlyph = nullptr;
        
        for (const auto& glyph : glyphGeometry) {
            if (glyph.getCodepoint() == static_cast<msdf_atlas::unicode_t>(left)) {
                leftGlyph = &glyph;
            }
            if (glyph.getCodepoint() == static_cast<msdf_atlas::unicode_t>(right)) {
                rightGlyph = &glyph;
            }
        }
        
        if (leftGlyph && rightGlyph) {
            auto kerningMap = fontGeometry.getKerning();
            auto it = kerningMap.find({leftGlyph->getCodepoint(), rightGlyph->getCodepoint()});
            if (it != kerningMap.end()) {
                return it->second;
            }
        }
        
        return 0.0;
    }
}