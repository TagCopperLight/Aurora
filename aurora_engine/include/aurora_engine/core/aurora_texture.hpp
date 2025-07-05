#pragma once

#include "aurora_device.hpp"

#include <memory>

namespace aurora {
    class AuroraTexture {
        public:
            AuroraTexture(AuroraDevice &device, VkFormat format, VkExtent3D extent, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount);
            ~AuroraTexture();

            AuroraTexture(const AuroraTexture &) = delete;
            AuroraTexture& operator=(const AuroraTexture &) = delete;
            AuroraTexture(AuroraTexture &&) = delete;
            AuroraTexture &operator=(AuroraTexture &&) = delete;

            VkImageView imageView() const { return textureImageView; }
            VkSampler sampler() const { return textureSampler; }
            VkImage getImage() const { return textureImage; }
            VkImageLayout getImageLayout() const { return textureLayout; }
            VkExtent3D getExtent() const { return extent; }
            VkFormat getFormat() const { return format; }
            
            const VkDescriptorImageInfo& getDescriptorInfo() const { return descriptor; }

            void updateDescriptor();
            void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);

        private:
            void createTextureImageView(VkImageViewType viewType);
            void createTextureSampler();

            VkDescriptorImageInfo descriptor{};
            AuroraDevice &auroraDevice;
            VkImage textureImage = VK_NULL_HANDLE;
            VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
            VkImageView textureImageView = VK_NULL_HANDLE;
            VkSampler textureSampler = VK_NULL_HANDLE;
            VkFormat format;
            VkImageLayout textureLayout;
            VkExtent3D extent{};
            uint32_t mipLevels{1};
            uint32_t layerCount{1};
    };
}
