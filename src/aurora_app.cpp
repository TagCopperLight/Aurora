#include "aurora_app.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <spdlog/spdlog.h>
#include <cassert>

namespace aurora {
    struct PushConstantsData {
        glm::mat2 transform{1.0f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    AuroraApp::AuroraApp() {
        spdlog::debug("Initializing Aurora Application");
        loadGameObjects();
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

    void AuroraApp::loadGameObjects() {
        spdlog::debug("Loading 3D models");
        std::vector<AuroraModel::Vertex> vertices = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
        // std::vector<AuroraModel::Vertex> vertices;
        // sierpinski(vertices, 5, {0.0f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f});

        auto auroraModel = std::make_shared<AuroraModel>(auroraDevice, vertices);
        spdlog::debug("Loaded model with {} vertices", vertices.size());

        std::vector<glm::vec3> colors{
            {1.f, .7f, .73f},
            {1.f, .87f, .73f},
            {1.f, 1.f, .73f},
            {.73f, 1.f, .8f},
            {.73, .88f, 1.f}
        };
        for (auto& color : colors) {
            color = glm::pow(color, glm::vec3{2.2f});
        }
        for (int i = 0; i < 40; i++) {
            auto triangle = AuroraGameObject::createGameObject();
            triangle.model = auroraModel;
            triangle.transform2d.scale = glm::vec2(.5f) + i * 0.025f;
            triangle.transform2d.rotation = i * glm::pi<float>() * .025f;
            triangle.color = colors[i % colors.size()];
            gameObjects.push_back(std::move(triangle));
        }
    }

    void AuroraApp::createPipelineLayout() {
        spdlog::debug("Creating pipeline layout");

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstantsData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(auroraDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout");
        }
        spdlog::debug("Pipeline layout created successfully");
    }

    void AuroraApp::createPipeline() {
        spdlog::debug("Creating graphics pipeline");
        assert(auroraSwapChain != nullptr && "Swap chain must be created before creating the pipeline");
        assert(pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline");

        PipelineConfigInfo pipelineConfig{};
        AuroraPipeline::defaultPipelineConfigInfo(pipelineConfig);
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
        
        if (auroraSwapChain == nullptr) {
            auroraSwapChain = std::make_unique<AuroraSwapChain>(auroraDevice, extent);
        } else {
            auroraSwapChain = std::make_unique<AuroraSwapChain>(auroraDevice, extent, std::move(auroraSwapChain));
            if (auroraSwapChain->imageCount() != commandBuffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }

        // If the render pass is compatible, we can reuse the existing pipeline
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

    void AuroraApp::freeCommandBuffers() {
        spdlog::debug("Freeing command buffers");
        vkFreeCommandBuffers(auroraDevice.device(), auroraDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
        spdlog::debug("Command buffers freed successfully");
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
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(auroraSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(auroraSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, auroraSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

        renderGameObjects(commandBuffers[imageIndex]);

        vkCmdEndRenderPass(commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer");
        }
    }

    void AuroraApp::renderGameObjects(VkCommandBuffer commandBuffer) {
        int i = 0;
        for (auto& obj : gameObjects) {
            i += 1;
            obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.0001f * i, glm::two_pi<float>());
        }

        auroraPipeline->bind(commandBuffer);

        for (auto& obj : gameObjects) {
            PushConstantsData push{};
            push.transform = obj.transform2d.mat2();
            push.offset = obj.transform2d.translation;
            push.color = obj.color;
            
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantsData), &push);

            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
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