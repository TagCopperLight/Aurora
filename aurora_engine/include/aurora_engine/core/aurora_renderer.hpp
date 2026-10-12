#pragma once

#include "aurora_window.hpp"
#include "aurora_device.hpp"
#include "aurora_swap_chain.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <vector>
#include <cassert>

namespace aurora {
    class AuroraRenderer {
        public:
            AuroraRenderer(AuroraWindow& window, AuroraDevice& device, const glm::vec4& backgroundColor);
            ~AuroraRenderer();

            AuroraRenderer(const AuroraRenderer&) = delete;
            AuroraRenderer &operator=(const AuroraRenderer&) = delete;

            VkRenderPass getSwapChainRenderPass() const { return auroraSwapChain->getRenderPass(); }
            float getAspectRatio() const { return auroraSwapChain->extentAspectRatio(); }
            uint32_t getWidth() const { return auroraSwapChain->width(); }
            uint32_t getHeight() const { return auroraSwapChain->height(); }
            bool isFrameInProgress() const { return isFrameStarted; }

            VkCommandBuffer getCurrentCommandBuffer() const {
                assert(isFrameStarted && "Frame has not started yet");
                return commandBuffers[currentFrameIndex];
            }

            int getFrameIndex() const {
                assert(isFrameStarted && "Frame has not started yet");
                return currentFrameIndex;
            }

            VkCommandBuffer beginFrame();
            void endFrame();

            void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
            void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

            // GPU time of the most recently completed frame, in ms (0 until available)
            double getLastGpuFrameTimeMs() const { return lastGpuFrameTimeMs; }

            // Write the next presented frame to a binary PPM file (waits for the GPU, debug use only)
            void requestCapture(const std::string& path) { capturePath = path; }

        private:
            void createCommandBuffers();
            void freeCommandBuffers();
            void recreateSwapChain();
            void createTimestampQueries();
            void readTimestampQueries();
            void recordCapture(VkCommandBuffer commandBuffer);
            void writeCapture();

            AuroraWindow& auroraWindow;
            AuroraDevice& auroraDevice;
            std::unique_ptr<AuroraSwapChain> auroraSwapChain;
            std::vector<VkCommandBuffer> commandBuffers;
            glm::vec4 backgroundColor;

            uint32_t currentImageIndex;
            int currentFrameIndex;
            bool isFrameStarted;

            VkQueryPool timestampQueryPool = VK_NULL_HANDLE;
            std::vector<bool> timestampPending;
            uint64_t timestampMask = ~0ull;
            double lastGpuFrameTimeMs = 0.0;

            std::string capturePath;
            VkBuffer captureBuffer = VK_NULL_HANDLE;
            VkDeviceMemory captureMemory = VK_NULL_HANDLE;
    };
}