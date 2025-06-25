#include "aurora_app.hpp"

#include <stdexcept>
#include <array>

namespace aurora {
    AuroraApp::AuroraApp() {
        loadModels();
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }

    AuroraApp::~AuroraApp() {
        vkDestroyPipelineLayout(auroraDevice.device(), pipelineLayout, nullptr);
    }

    void AuroraApp::run() {
        while (!auroraWindow.shouldClose()) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(auroraDevice.device());
    }

    void AuroraApp::loadModels() {
        std::vector<AuroraModel::Vertex> vertices = {
            {{0.0f, -0.5f}},
            {{0.5f, 0.5f}},
            {{-0.5f, 0.5f}}
        };

        auroraModel = std::make_unique<AuroraModel>(auroraDevice, vertices);
    }

    void AuroraApp::createPipelineLayout() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(auroraDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout");
        }
    }

    void AuroraApp::createPipeline() {
        auto pipelineConfig = AuroraPipeline::defaultPipelineConfigInfo(auroraSwapChain.width(), auroraSwapChain.height());
        pipelineConfig.renderPass = auroraSwapChain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        auroraPipeline = std::make_unique<AuroraPipeline>(auroraDevice, "shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
    }

    void AuroraApp::createCommandBuffers() {
        commandBuffers.resize(auroraSwapChain.imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = auroraDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(auroraDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers");
        }

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