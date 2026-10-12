#include "aurora_engine/core/aurora_renderer.hpp"

#include <stdexcept>
#include <array>
#include "aurora_engine/utils/log.hpp"
#include "aurora_engine/profiling/aurora_profiler.hpp"
#include <cassert>
#include <cstdio>

namespace aurora {
    AuroraRenderer::AuroraRenderer(AuroraWindow& window, AuroraDevice& device, const glm::vec4& backgroundColor) 
        : auroraWindow(window), auroraDevice(device), backgroundColor(backgroundColor) {
        currentFrameIndex = 0;
        isFrameStarted = false;
        recreateSwapChain();
        createCommandBuffers();
        createTimestampQueries();
    }

    AuroraRenderer::~AuroraRenderer() {
        if (timestampQueryPool != VK_NULL_HANDLE) {
            vkDestroyQueryPool(auroraDevice.device(), timestampQueryPool, nullptr);
        }
        freeCommandBuffers();
    }

    void AuroraRenderer::createTimestampQueries() {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(auroraDevice.getPhysicalDevice(), &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(auroraDevice.getPhysicalDevice(), &queueFamilyCount, queueFamilies.data());
        uint32_t validBits = queueFamilies[auroraDevice.findPhysicalQueueFamilies().graphicsFamily].timestampValidBits;

        if (validBits == 0 || auroraDevice.properties.limits.timestampPeriod <= 0.0f) {
            log::engine()->warn("GPU timestamps not supported, GPU frame timing disabled");
            return;
        }
        timestampMask = validBits >= 64 ? ~0ull : ((1ull << validBits) - 1);

        VkQueryPoolCreateInfo queryPoolInfo{};
        queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        queryPoolInfo.queryCount = 2 * AuroraSwapChain::MAX_FRAMES_IN_FLIGHT;

        if (vkCreateQueryPool(auroraDevice.device(), &queryPoolInfo, nullptr, &timestampQueryPool) != VK_SUCCESS) {
            log::engine()->warn("Failed to create timestamp query pool, GPU frame timing disabled");
            timestampQueryPool = VK_NULL_HANDLE;
            return;
        }
        timestampPending.assign(AuroraSwapChain::MAX_FRAMES_IN_FLIGHT, false);
    }

    // Called once this frame slot's fence has been waited on, so results are available
    void AuroraRenderer::readTimestampQueries() {
        if (timestampQueryPool == VK_NULL_HANDLE || !timestampPending[currentFrameIndex]) {
            return;
        }

        uint64_t timestamps[2];
        VkResult result = vkGetQueryPoolResults(
            auroraDevice.device(), timestampQueryPool,
            2 * currentFrameIndex, 2, sizeof(timestamps), timestamps, sizeof(uint64_t),
            VK_QUERY_RESULT_64_BIT);

        if (result == VK_SUCCESS) {
            double periodNs = auroraDevice.properties.limits.timestampPeriod;
            uint64_t ticks = (timestamps[1] - timestamps[0]) & timestampMask;
            lastGpuFrameTimeMs = static_cast<double>(ticks) * periodNs / 1e6;
            AuroraProfiler::instance().addSample("GPU Frame", lastGpuFrameTimeMs);
        }
        timestampPending[currentFrameIndex] = false;
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
        log::engine()->debug("Creating command buffers");
        commandBuffers.resize(AuroraSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = auroraDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(auroraDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers");
        }
        log::engine()->debug("Created {} command buffers", commandBuffers.size());
    }

    void AuroraRenderer::freeCommandBuffers() {
        log::engine()->debug("Freeing command buffers");
        vkFreeCommandBuffers(auroraDevice.device(), auroraDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
        log::engine()->debug("Command buffers freed successfully");
    }
    VkCommandBuffer AuroraRenderer::beginFrame(){
        assert(!isFrameStarted && "Frame already in progress");
        
        auto result = auroraSwapChain->acquireNextImage(&currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            log::engine()->debug("Swap chain out of date, recreating");
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image");
        }

        isFrameStarted = true;

        readTimestampQueries();

        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer");
        }

        if (timestampQueryPool != VK_NULL_HANDLE) {
            vkCmdResetQueryPool(commandBuffer, timestampQueryPool, 2 * currentFrameIndex, 2);
            vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, timestampQueryPool, 2 * currentFrameIndex);
        }

        return commandBuffer;
    }

    void AuroraRenderer::endFrame(){
        assert(isFrameStarted && "Cannot end frame because no frame is in progress");
        auto commandBuffer = getCurrentCommandBuffer();

        if (timestampQueryPool != VK_NULL_HANDLE) {
            vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, timestampQueryPool, 2 * currentFrameIndex + 1);
            timestampPending[currentFrameIndex] = true;
        }

        if (!capturePath.empty()) {
            recordCapture(commandBuffer);
        }

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer");
        }

        auto result = auroraSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

        if (captureBuffer != VK_NULL_HANDLE) {
            writeCapture();
        }

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || auroraWindow.wasWindowResized()) {
            log::engine()->debug("Swap chain out of date or window resized, recreating swap chain");
            auroraWindow.resetWindowResizedFlag();
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
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

        std::array<VkClearValue, 3> clearValues{};
        clearValues[0].color = {{backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a}};
        clearValues[1].depthStencil = {1.0f, 0};
        clearValues[2].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
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

    void AuroraRenderer::recordCapture(VkCommandBuffer commandBuffer) {
        if (!auroraSwapChain->supportsCapture()) {
            log::engine()->error("Swap chain images do not support transfer, cannot capture frame");
            capturePath.clear();
            return;
        }

        VkExtent2D extent = auroraSwapChain->getSwapChainExtent();
        VkDeviceSize size = static_cast<VkDeviceSize>(extent.width) * extent.height * 4;
        auroraDevice.createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, captureBuffer, captureMemory);

        VkFormat format = auroraSwapChain->getSwapChainImageFormat();
        if (format != VK_FORMAT_B8G8R8A8_SRGB && format != VK_FORMAT_B8G8R8A8_UNORM &&
            format != VK_FORMAT_R8G8B8A8_SRGB && format != VK_FORMAT_R8G8B8A8_UNORM) {
            log::engine()->error("Unsupported swap chain format for capture: {}", static_cast<int>(format));
            vkDestroyBuffer(auroraDevice.device(), captureBuffer, nullptr);
            vkFreeMemory(auroraDevice.device(), captureMemory, nullptr);
            captureBuffer = VK_NULL_HANDLE;
            captureMemory = VK_NULL_HANDLE;
            capturePath.clear();
            return;
        }

        VkImage image = auroraSwapChain->getImage(currentImageIndex);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        // ALL_COMMANDS chains with the render pass's implicit final layout transition (dst BOTTOM_OF_PIPE)
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkBufferImageCopy region{};
        region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        region.imageExtent = {extent.width, extent.height, 1};
        vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, captureBuffer, 1, &region);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = 0;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    void AuroraRenderer::writeCapture() {
        vkDeviceWaitIdle(auroraDevice.device());

        VkExtent2D extent = auroraSwapChain->getSwapChainExtent();
        VkFormat format = auroraSwapChain->getSwapChainImageFormat();
        bool bgr = format == VK_FORMAT_B8G8R8A8_SRGB || format == VK_FORMAT_B8G8R8A8_UNORM;

        void* data = nullptr;
        vkMapMemory(auroraDevice.device(), captureMemory, 0, VK_WHOLE_SIZE, 0, &data);

        if (FILE* file = std::fopen(capturePath.c_str(), "wb")) {
            std::fprintf(file, "P6\n%u %u\n255\n", extent.width, extent.height);
            const auto* pixels = static_cast<const uint8_t*>(data);
            std::vector<uint8_t> row(extent.width * 3);
            for (uint32_t y = 0; y < extent.height; ++y) {
                for (uint32_t x = 0; x < extent.width; ++x) {
                    const uint8_t* p = pixels + (static_cast<size_t>(y) * extent.width + x) * 4;
                    row[x * 3 + 0] = bgr ? p[2] : p[0];
                    row[x * 3 + 1] = p[1];
                    row[x * 3 + 2] = bgr ? p[0] : p[2];
                }
                std::fwrite(row.data(), 1, row.size(), file);
            }
            std::fclose(file);
            log::engine()->info("Captured frame to {}", capturePath);
        } else {
            log::engine()->error("Failed to open capture file {}", capturePath);
        }

        vkUnmapMemory(auroraDevice.device(), captureMemory);
        vkDestroyBuffer(auroraDevice.device(), captureBuffer, nullptr);
        vkFreeMemory(auroraDevice.device(), captureMemory, nullptr);
        captureBuffer = VK_NULL_HANDLE;
        captureMemory = VK_NULL_HANDLE;
        capturePath.clear();
    }
}
