#pragma once

#include <msdf-atlas-gen/msdf-atlas-gen.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "aurora_engine/core/aurora_texture.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_buffer_pool.hpp"

#include <string>
#include <memory>
#include <unordered_map>

namespace aurora {
    class AuroraMSDFAtlas {
        public:
            struct Config {
                uint32_t width = 1024;
                uint32_t height = 1024;
            };
            
            AuroraMSDFAtlas(AuroraDevice& device, const std::string& fontPath);
            AuroraMSDFAtlas(AuroraDevice& device, const std::string& fontPath, const Config& config);
            ~AuroraMSDFAtlas();

            AuroraMSDFAtlas(const AuroraMSDFAtlas &) = delete;
            AuroraMSDFAtlas& operator=(const AuroraMSDFAtlas &) = delete;
            AuroraMSDFAtlas(AuroraMSDFAtlas &&) = delete;
            AuroraMSDFAtlas &operator=(AuroraMSDFAtlas &&) = delete;

            bool loadFont(const std::string& fontPath);
            bool generateAtlas();
            void saveAtlasAsPNG(const std::string& outputPath) const;
            void uploadAtlasToTexture();

            uint32_t getAtlasWidth() const { return config.width; }
            uint32_t getAtlasHeight() const { return config.height; }

            const AuroraTexture& getAtlasTexture() const { return *atlasTexture; }
            const VkDescriptorImageInfo& getDescriptorInfo() const { return atlasTexture->getDescriptorInfo(); }
            
            struct GlyphInfo {
                glm::vec4 atlasBounds;
                glm::vec4 planeBounds;
                double advance;
            };
            
            bool getGlyphInfo(char character, GlyphInfo& glyphInfo) const;
            double getKerning(char left, char right) const;

            const BufferAllocation& getSharedIndexAllocation();

        private:
            void createAtlasTexture();
            void buildGlyphCache();
            void buildKerningCache();

            AuroraDevice& auroraDevice;
            Config config;

            msdfgen::FreetypeHandle* freetypeHandle = nullptr;
            msdfgen::FontHandle* fontHandle;

            std::unique_ptr<AuroraTexture> atlasTexture;
            
            std::unique_ptr<msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3>> atlasStorage;
            
            
            std::vector<msdf_atlas::GlyphGeometry> glyphGeometry;
            msdf_atlas::FontGeometry fontGeometry;

            mutable std::unordered_map<char, const msdf_atlas::GlyphGeometry*> glyphCache;
            mutable std::unordered_map<uint64_t, double> kerningCache;

            BufferAllocation sharedIndexAllocation{};
            static constexpr size_t MAX_TEXT_CHARS = 16384;
            void ensureSharedIndexBuffer();

            void freeFont();
    };
}