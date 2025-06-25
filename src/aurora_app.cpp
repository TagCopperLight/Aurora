#include "aurora_app.hpp"

#include <stdexcept>
#include <array>
#include <spdlog/spdlog.h>

namespace aurora {
    AuroraApp::AuroraApp() {
        spdlog::info("Initializing Aurora Application");
        loadModels();
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
        spdlog::info("Aurora Application initialized successfully");
    }

    AuroraApp::~AuroraApp() {
        spdlog::info("Destroying Aurora Application");
        vkDestroyPipelineLayout(auroraDevice.device(), pipelineLayout, nullptr);
        spdlog::info("Aurora Application destroyed");
    }

    void AuroraApp::run() {
        spdlog::info("Starting Aurora Application main loop");
        while (!auroraWindow.shouldClose()) {
            glfwPollEvents();
            drawFrame();
        }

        spdlog::info("Aurora Application main loop ended, waiting for device idle");
        vkDeviceWaitIdle(auroraDevice.device());
        spdlog::info("Aurora Application run completed");
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
        spdlog::info("Loading 3D models");
        std::vector<AuroraModel::Vertex> vertices = {};
        sierpinski(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});

        auroraModel = std::make_unique<AuroraModel>(auroraDevice, vertices);
        spdlog::info("Loaded model with {} vertices", vertices.size());
    }

    void AuroraApp::createPipelineLayout() {
        spdlog::info("Creating pipeline layout");
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(auroraDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout");
        }
        spdlog::info("Pipeline layout created successfully");
    }

    void AuroraApp::createPipeline() {
        spdlog::info("Creating graphics pipeline");
        auto pipelineConfig = AuroraPipeline::defaultPipelineConfigInfo(auroraSwapChain.width(), auroraSwapChain.height());
        pipelineConfig.renderPass = auroraSwapChain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        auroraPipeline = std::make_unique<AuroraPipeline>(auroraDevice, "shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
        spdlog::info("Graphics pipeline created successfully");
    }

    void AuroraApp::createCommandBuffers() {
        spdlog::info("Creating command buffers");
        commandBuffers.resize(auroraSwapChain.imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = auroraDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(auroraDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers");
        }
        spdlog::info("Created {} command buffers", commandBuffers.size());

        for (int i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            
            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("Failed to begin recording command buffer");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = auroraSwapChain.getRenderPass();
            renderPassInfo.framebuffer = auroraSwapChain.getFrameBuffer(i);

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = auroraSwapChain.getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            auroraPipeline->bind(commandBuffers[i]);
            auroraModel->bind(commandBuffers[i]);
            auroraModel->draw(commandBuffers[i]);

            vkCmdEndRenderPass(commandBuffers[i]);
            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to record command buffer");
            }
        }
    }
    void AuroraApp::drawFrame() {
        uint32_t imageIndex;
        auto result = auroraSwapChain.acquireNextImage(&imageIndex);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image");
        }

        result = auroraSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit command buffers");
        }
    }
}