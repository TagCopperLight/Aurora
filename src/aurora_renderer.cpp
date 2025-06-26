#include "aurora_renderer.hpp"

#include <stdexcept>
#include <array>
#include <spdlog/spdlog.h>
#include <cassert>

namespace aurora {
    AuroraRenderer::AuroraRenderer(AuroraWindow& window, AuroraDevice& device) : auroraWindow(window), auroraDevice(device) {
        recreateSwapChain();
        createCommandBuffers();
    }

    AuroraRenderer::~AuroraRenderer() {
        freeCommandBuffers();
    }
    
    void AuroraRenderer::recreateSwapChain() {
        auto extent = auroraWindow.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = auroraWindow.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(auroraDevice.device());
        
        if (auroraSwapChain == nullptr) {
            auroraSwapChain = std::make_unique<AuroraSwapChain>(auroraDevice, extent);
        } else {
            std::shared_ptr<AuroraSwapChain> oldSwapChain = std::move(auroraSwapChain);
            auroraSwapChain = std::make_unique<AuroraSwapChain>(auroraDevice, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*auroraSwapChain.get())) {
                throw std::runtime_error("Swap chain image(or depth) format has changed");
            }
        }
    }

    void AuroraRenderer::createCommandBuffers() {
        spdlog::debug("Creating command buffers");
        commandBuffers.resize(AuroraSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = auroraDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(auroraDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers");
        }
        spdlog::debug("Created {} command buffers", commandBuffers.size());
    }

    void AuroraRenderer::freeCommandBuffers() {
        spdlog::debug("Freeing command buffers");
        vkFreeCommandBuffers(auroraDevice.device(), auroraDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
        spdlog::debug("Command buffers freed successfully");
    }
    VkCommandBuffer AuroraRenderer::beginFrame(){
        assert(!isFrameStarted && "Frame already in progress");
        
        auto result = auroraSwapChain->acquireNextImage(&currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            spdlog::debug("Swap chain out of date, recreating");
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image");
        }

        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer");
        }

        return commandBuffer;
    }

    void AuroraRenderer::endFrame(){
        assert(isFrameStarted && "Cannot end frame because no frame is in progress");
        auto commandBuffer = getCurrentCommandBuffer();

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer");
        }

        auto result = auroraSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || auroraWindow.wasWindowResized()) {
            spdlog::debug("Swap chain out of date or window resized, recreating swap chain");
            auroraWindow.resetWindowResizedFlag();
            recreateSwapChain();
        }

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit command buffers");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % AuroraSwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void AuroraRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer){
        assert(isFrameStarted && "Cannot begin swap chain render pass because no frame is in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Command buffer is not the current command buffer");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = auroraSwapChain->getRenderPass();
        renderPassInfo.framebuffer = auroraSwapChain->getFrameBuffer(currentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = auroraSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(auroraSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(auroraSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, auroraSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void AuroraRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer){
        assert(isFrameStarted && "Cannot end swap chain render pass because no frame is in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Command buffer is not the current command buffer");

        vkCmdEndRenderPass(commandBuffer);
    }
}