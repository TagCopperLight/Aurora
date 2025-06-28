#pragma once

#include "aurora_window.hpp"
#include "aurora_device.hpp"
#include "aurora_swap_chain.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace aurora {
    class AuroraRenderer {
        public:
            AuroraRenderer(AuroraWindow& window, AuroraDevice& device);
            ~AuroraRenderer();

            AuroraRenderer(const AuroraRenderer&) = delete;
            AuroraRenderer &operator=(const AuroraRenderer&) = delete;

            VkRenderPass getSwapChainRenderPass() const { return auroraSwapChain->getRenderPass(); }
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

        private:
            void createCommandBuffers();
            void freeCommandBuffers();
            void recreateSwapChain();

            AuroraWindow& auroraWindow;
            AuroraDevice& auroraDevice;
            std::unique_ptr<AuroraSwapChain> auroraSwapChain;
            std::vector<VkCommandBuffer> commandBuffers;

            uint32_t currentImageIndex;
            int currentFrameIndex;
            bool isFrameStarted;
    };
}