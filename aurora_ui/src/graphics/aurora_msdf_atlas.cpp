#include "aurora_ui/graphics/aurora_msdf_atlas.hpp"

#include <stdexcept>
#include "aurora_engine/utils/log.hpp"
#include <cstring>

namespace aurora {
    AuroraMSDFAtlas::AuroraMSDFAtlas(AuroraDevice& device, const std::string& fontPath)
    : AuroraMSDFAtlas(device, fontPath, Config{}) {
    }

    AuroraMSDFAtlas::AuroraMSDFAtlas(AuroraDevice& device, const std::string& fontPath, const Config& config)
    : auroraDevice{device}, config{config}, fontHandle{nullptr}, fontGeometry{&glyphGeometry} {
        
        freetypeHandle = msdfgen::initializeFreetype();

        if (!loadFont(fontPath)) {
            throw std::runtime_error("Failed to load font: " + fontPath);
        }
        
        if (!generateAtlas()) {
            throw std::runtime_error("Failed to generate MSDF atlas");
        }

        uploadAtlasToTexture();

        log::ui()->info("MSDF Atlas generated with dimensions: {}x{}", config.width, config.height);
    }

    AuroraMSDFAtlas::~AuroraMSDFAtlas() {
        freeFont();
        msdfgen::deinitializeFreetype(freetypeHandle);
    }

    bool AuroraMSDFAtlas::loadFont(const std::string& fontPath) {
        freeFont();

        fontHandle = msdfgen::loadFont(freetypeHandle, fontPath.c_str());

        if (!fontHandle) {
            log::ui()->error("Failed to load font from path: {}", fontPath);
            return false;
        }

        return true;
    }

    bool AuroraMSDFAtlas::generateAtlas() {
        if (!fontHandle) {
            log::ui()->error("Font handle is null, cannot generate atlas");
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
        
        packer.setMinimumScale(96); 
        packer.setPixelRange(4.0); 
        packer.setMiterLimit(1.0);
        packer.setSpacing(2.0);

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

        
        atlasStorage = std::make_unique<msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3>>(
            generator.atlasStorage()
        );

        
        glyphGeometry = std::move(glyphs);
        fontGeometry = msdf_atlas::FontGeometry(&glyphGeometry);

        buildGlyphCache();
        buildKerningCache();

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
        
        glyphCache.clear();
        kerningCache.clear();
    }

    void AuroraMSDFAtlas::buildGlyphCache() {
        glyphCache.clear();
        glyphCache.reserve(glyphGeometry.size());
        
        for (const auto& glyph : glyphGeometry) {
            char character = static_cast<char>(glyph.getCodepoint());
            glyphCache[character] = &glyph;
        }
        
        log::ui()->debug("Built glyph cache with {} entries", glyphCache.size());
    }

    void AuroraMSDFAtlas::buildKerningCache() {
        kerningCache.clear();
        
        auto kerningMap = fontGeometry.getKerning();
        kerningCache.reserve(kerningMap.size());
        
        for (const auto& kerningPair : kerningMap) {
            uint64_t key = (static_cast<uint64_t>(kerningPair.first.first) << 32) | 
                          static_cast<uint64_t>(kerningPair.first.second);
            kerningCache[key] = kerningPair.second;
        }
        
        log::ui()->debug("Built kerning cache with {} entries", kerningCache.size());
    }

    void AuroraMSDFAtlas::uploadAtlasToTexture() {
        if (!atlasTexture || !atlasStorage) {
            log::ui()->error("Atlas texture or storage is null, cannot upload data");
            return;
        }

        msdfgen::BitmapConstRef<msdf_atlas::byte, 3> bitmapRef = *atlasStorage;
        const msdf_atlas::byte* bitmapData = bitmapRef.pixels;

        std::vector<uint8_t> rgbaData(config.width * config.height * 4);
        for (size_t i = 0; i < config.width * config.height; ++i) {
            rgbaData[i * 4 + 0] = bitmapData[i * 3 + 0]; 
            rgbaData[i * 4 + 1] = bitmapData[i * 3 + 1]; 
            rgbaData[i * 4 + 2] = bitmapData[i * 3 + 2]; 
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
    }
    
    void AuroraMSDFAtlas::saveAtlasAsPNG(const std::string& outputPath) const {
        if (!atlasTexture) {
            log::ui()->error("Atlas texture is not initialized, cannot save as PNG");
            return;
        }

        msdfgen::BitmapConstRef<msdf_atlas::byte, 3> bitmapRef = *atlasStorage;

        bool success = msdf_atlas::saveImage(
            bitmapRef,
            msdf_atlas::ImageFormat::PNG,
            outputPath.c_str()
        );

        if (success) {
            log::ui()->info("Successfully saved MSDF atlas as PNG to {}", outputPath);
        } else {
            log::ui()->error("Failed to save MSDF atlas as PNG");
        }
    }
    
    bool AuroraMSDFAtlas::getGlyphInfo(char character, GlyphInfo& glyphInfo) const {
        auto it = glyphCache.find(character);
        if (it == glyphCache.end()) {
            return false;
        }
        
        const auto& glyph = *it->second;
        
        double left, bottom, right, top;
        glyph.getQuadAtlasBounds(left, bottom, right, top);
        
        glyphInfo.atlasBounds = glm::vec4(
            static_cast<float>(left / config.width),
            static_cast<float>(bottom / config.height),
            static_cast<float>((right - left) / config.width),
            static_cast<float>((top - bottom) / config.height)
        );
        
        glyph.getQuadPlaneBounds(left, bottom, right, top);
        glyphInfo.planeBounds = glm::vec4(
            static_cast<float>(left),
            static_cast<float>(bottom),
            static_cast<float>(right - left),
            static_cast<float>(top - bottom)
        );
        
        glyphInfo.advance = glyph.getAdvance();
        
        return true;
    }
    
    double AuroraMSDFAtlas::getKerning(char left, char right) const {
        uint64_t key = (static_cast<uint64_t>(static_cast<unsigned char>(left)) << 32) | 
                       static_cast<uint64_t>(static_cast<unsigned char>(right));
        
        auto it = kerningCache.find(key);
        if (it != kerningCache.end()) {
            return it->second;
        }
        
        return 0.0;
    }

    const BufferAllocation& AuroraMSDFAtlas::getSharedIndexAllocation() {
        if (!sharedIndexAllocation.isValid()) {
            ensureSharedIndexBuffer();
        }
        return sharedIndexAllocation;
    }

    void AuroraMSDFAtlas::ensureSharedIndexBuffer() {
        if (sharedIndexAllocation.isValid()) return;

        std::vector<uint32_t> indices;
        indices.reserve(MAX_TEXT_CHARS * 6);
        for (size_t i=0; i<MAX_TEXT_CHARS; ++i) {
            uint32_t base = static_cast<uint32_t>(i * 4);
            indices.push_back(base);
            indices.push_back(base+1);
            indices.push_back(base+2);
            indices.push_back(base+2);
            indices.push_back(base+3);
            indices.push_back(base);
        }

        VkDeviceSize size = indices.size() * sizeof(uint32_t);
        sharedIndexAllocation = auroraDevice.getIndexBufferPool().allocate(size);

        auto staging = auroraDevice.getStagingBufferPool().allocate(size);
        if (staging.mappedMemory) {
            memcpy(staging.mappedMemory, indices.data(), (size_t)size);
        }
        auroraDevice.copyBuffer(staging.buffer, sharedIndexAllocation.buffer, size, staging.offset, sharedIndexAllocation.offset);
        auroraDevice.getStagingBufferPool().free(staging);
    }
}