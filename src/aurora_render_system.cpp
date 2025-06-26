#include "aurora_render_system.hpp"

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

    AuroraRenderSystem::AuroraRenderSystem(AuroraDevice& device, VkRenderPass renderPass) : auroraDevice{device} {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    AuroraRenderSystem::~AuroraRenderSystem() {
        vkDestroyPipelineLayout(auroraDevice.device(), pipelineLayout, nullptr);
    }

    void AuroraRenderSystem::createPipelineLayout() {
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
    }

    void AuroraRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline");

        PipelineConfigInfo pipelineConfig{};
        AuroraPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        auroraPipeline = std::make_unique<AuroraPipeline>(auroraDevice, "shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
    }

    void AuroraRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<AuroraGameObject>& gameObjects) {
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
}