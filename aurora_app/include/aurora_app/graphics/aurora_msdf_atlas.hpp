#pragma once

#include <msdf-atlas-gen/msdf-atlas-gen.h>

#include "aurora_engine/core/aurora_texture.hpp"
#include "aurora_engine/core/aurora_device.hpp"

#include <string>
#include <memory>

namespace aurora {
    class AuroraMSDFAtlas {
        public:
            struct Config {
                uint32_t width = 512;
                uint32_t height = 512;
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

        private:
            void createAtlasTexture();

            AuroraDevice& auroraDevice;
            Config config;

            msdfgen::FreetypeHandle* freetypeHandle = nullptr;
            msdfgen::FontHandle* fontHandle;

            std::unique_ptr<AuroraTexture> atlasTexture;
            
            std::unique_ptr<msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3>> atlasStorage;

            void freeFont();
    };
}