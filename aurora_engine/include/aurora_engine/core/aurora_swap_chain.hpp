#pragma once

#include "aurora_device.hpp"

#include <string>
#include <vector>
#include <memory>

namespace aurora {
    class AuroraSwapChain {
        public:
            static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

            AuroraSwapChain(AuroraDevice &deviceRef, VkExtent2D windowExtent);
            AuroraSwapChain(AuroraDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<AuroraSwapChain> previous);
            ~AuroraSwapChain();

            AuroraSwapChain(const AuroraSwapChain &) = delete;
            AuroraSwapChain& operator=(const AuroraSwapChain &) = delete;

            VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
            VkRenderPass getRenderPass() { return renderPass; }
            VkImageView getImageView(int index) { return swapChainImageViews[index]; }
            size_t imageCount() { return swapChainImages.size(); }
            VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
            VkExtent2D getSwapChainExtent() { return swapChainExtent; }
            uint32_t width() { return swapChainExtent.width; }
            uint32_t height() { return swapChainExtent.height; }

            float extentAspectRatio() {
                return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
            }
            VkFormat findDepthFormat();

            VkResult acquireNextImage(uint32_t *imageIndex);
            VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

            bool compareSwapFormats(const AuroraSwapChain &swapChain) const {
                return swapChain.swapChainImageFormat == swapChainImageFormat && swapChain.swapChainDepthFormat == swapChainDepthFormat;
            }

        private:
            void init();
            void createSwapChain();
            void createImageViews();
            void createColorResources();
            void createDepthResources();
            void createRenderPass();
            void createFramebuffers();
            void createSyncObjects();

            VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
            VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
            VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

            VkFormat swapChainImageFormat;
            VkFormat swapChainDepthFormat;
            VkExtent2D swapChainExtent;

            std::vector<VkFramebuffer> swapChainFramebuffers;
            VkRenderPass renderPass;

            std::vector<VkImage> depthImages;
            std::vector<VkDeviceMemory> depthImageMemorys;
            std::vector<VkImageView> depthImageViews;
            std::vector<VkImage> swapChainImages;
            std::vector<VkImageView> swapChainImageViews;
            
            // MSAA resources
            std::vector<VkImage> colorImages;
            std::vector<VkDeviceMemory> colorImageMemorys;
            std::vector<VkImageView> colorImageViews;

            AuroraDevice &device;
            VkExtent2D windowExtent;

            VkSwapchainKHR swapChain;
            std::shared_ptr<AuroraSwapChain> oldSwapChain;

            std::vector<VkSemaphore> imageAvailableSemaphores;
            std::vector<VkSemaphore> renderFinishedSemaphores;
            std::vector<VkFence> inFlightFences;
            std::vector<VkFence> imagesInFlight;
            size_t currentFrame = 0;
    };
}
