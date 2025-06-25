#include "aurora_app.hpp"

#include <stdexcept>
#include <array>
#include <spdlog/spdlog.h>

namespace aurora {
    AuroraApp::AuroraApp() {
        spdlog::debug("Initializing Aurora Application");
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
        spdlog::debug("Aurora Application initialized successfully");
    }

    AuroraApp::~AuroraApp() {
        spdlog::debug("Destroying Aurora Application");
        vkDestroyPipelineLayout(auroraDevice.device(), pipelineLayout, nullptr);
        spdlog::debug("Aurora Application destroyed");
    }

    void AuroraApp::run() {
        spdlog::debug("Starting Aurora Application main loop");
        while (!auroraWindow.shouldClose()) {
            glfwPollEvents();
            drawFrame();
        }

        spdlog::debug("Aurora Application main loop ended, waiting for device idle");
        vkDeviceWaitIdle(auroraDevice.device());
        spdlog::debug("Aurora Application run completed");
    }

    void AuroraApp::sierpinski(std::vector<AuroraModel::Vertex>& vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top) {
        if (depth == 0) {
            vertices.push_back({top});
            vertices.push_back({right});
            vertices.push_back({left});
        } else {
            auto leftTop = 0.5f * (left + top);
            auto rightTop = 0.5f * (right + top);
            auto leftRight = 0.5f * (left + right);
            sierpinski(vertices, depth - 1, left, leftRight, leftTop);
            sierpinski(vertices, depth - 1, leftRight, right, rightTop);
            sierpinski(vertices, depth - 1, leftTop, rightTop, top);
        }
    }

    void AuroraApp::loadModels() {
        spdlog::debug("Loading 3D models");
        std::vector<AuroraModel::Vertex> vertices = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
        // std::vector<AuroraModel::Vertex> vertices;
        // sierpinski(vertices, 5, {0.0f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f});

        auroraModel = std::make_unique<AuroraModel>(auroraDevice, vertices);
        spdlog::debug("Loaded model with {} vertices", vertices.size());
    }

    void AuroraApp::createPipelineLayout() {
        spdlog::debug("Creating pipeline layout");
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(auroraDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout");
        }
        spdlog::debug("Pipeline layout created successfully");
    }

    void AuroraApp::createPipeline() {
        spdlog::debug("Creating graphics pipeline");
        auto pipelineConfig = AuroraPipeline::defaultPipelineConfigInfo(auroraSwapChain->width(), auroraSwapChain->height());
        pipelineConfig.renderPass = auroraSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        auroraPipeline = std::make_unique<AuroraPipeline>(auroraDevice, "shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
        spdlog::debug("Graphics pipeline created successfully");
    }

    void AuroraApp::recreateSwapChain() {
        spdlog::debug("Recreating swap chain");
        auto extent = auroraWindow.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = auroraWindow.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(auroraDevice.device());
        auroraSwapChain = nullptr;
        auroraSwapChain = std::make_unique<AuroraSwapChain>(auroraDevice, extent);
        createPipeline();
        spdlog::debug("Swap chain recreated successfully");
    }

    void AuroraApp::createCommandBuffers() {
        spdlog::debug("Creating command buffers");
        commandBuffers.resize(auroraSwapChain->imageCount());

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

    void AuroraApp::recordCommandBuffer(int imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = auroraSwapChain->getRenderPass();
        renderPassInfo.framebuffer = auroraSwapChain->getFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = auroraSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        auroraPipeline->bind(commandBuffers[imageIndex]);
        auroraModel->bind(commandBuffers[imageIndex]);
        auroraModel->draw(commandBuffers[imageIndex]);

        vkCmdEndRenderPass(commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer");
        }
    }

    void AuroraApp::drawFrame() {
        uint32_t imageIndex;
        auto result = auroraSwapChain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            spdlog::debug("Swap chain out of date, recreating");
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image");
        }

        recordCommandBuffer(imageIndex);
        result = auroraSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || auroraWindow.wasWindowResized()) {
            spdlog::debug("Swap chain out of date or window resized, recreating swap chain");
            auroraWindow.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit command buffers");
        }
    }
}