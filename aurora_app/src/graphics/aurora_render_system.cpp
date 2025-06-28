#include "aurora_app/graphics/aurora_render_system.hpp"
#include "aurora_app/components/aurora_component_interface.hpp"

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
        glm::mat4 transform{1.0f};
        alignas(16) glm::vec3 color;
    };

    AuroraRenderSystem::AuroraRenderSystem(AuroraDevice& device, VkRenderPass renderPass, const std::string& vertFilePath, const std::string& fragFilePath, VkPrimitiveTopology topology) : auroraDevice{device} {
        createPipelineLayout();
        createPipeline(renderPass, vertFilePath, fragFilePath, topology);
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

    void AuroraRenderSystem::createPipeline(VkRenderPass renderPass, const std::string& vertFilePath, const std::string& fragFilePath, VkPrimitiveTopology topology) {
        assert(pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline");

        PipelineConfigInfo pipelineConfig{};
        AuroraPipeline::defaultPipelineConfigInfo(pipelineConfig, topology);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        auroraPipeline = std::make_unique<AuroraPipeline>(auroraDevice, vertFilePath, fragFilePath, pipelineConfig);
    }

    void AuroraRenderSystem::renderComponents(VkCommandBuffer commandBuffer) {
        int i = 0;
        for (auto& component : components) {
            i += 1;
            component->transform.rotation = glm::mod(component->transform.rotation + 0.0001f * i, glm::two_pi<float>());
        }

        auroraPipeline->bind(commandBuffer);

        for (const auto& component : components) {
            if (component->isHidden()) {
                continue;
            }

            PushConstantsData push{};
            push.transform = component->transform.mat4();
            push.color = component->color;

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantsData), &push);

            component->model->bind(commandBuffer);
            component->model->draw(commandBuffer);
        }
    }
}