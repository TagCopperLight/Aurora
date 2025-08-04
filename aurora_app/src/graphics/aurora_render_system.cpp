#include "aurora_app/graphics/aurora_render_system.hpp"
#include "aurora_app/components/aurora_component_interface.hpp"
#include "aurora_engine/core/aurora_buffer.hpp"
#include "aurora_engine/core/aurora_swap_chain.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <spdlog/spdlog.h>
#include <cassert>

namespace aurora {
    AuroraRenderSystem::AuroraRenderSystem(AuroraDevice& device, const RenderSystemCreateInfo& createInfo) 
        : auroraDevice{device}, 
          globalDescriptorPool{createInfo.descriptorPool}, 
          msdfAtlas{createInfo.msdfAtlas},
          vertexShaderPath{createInfo.vertFilePath}, 
          fragmentShaderPath{createInfo.fragFilePath}, 
          topology{createInfo.topology},
          needsTextureBinding{createInfo.needsTextureBinding} {
        createPipelineLayout();
        createPipeline(createInfo.renderPass, createInfo.vertFilePath, createInfo.fragFilePath, createInfo.topology);
    }

    AuroraRenderSystem::~AuroraRenderSystem() {
        vkDestroyPipelineLayout(auroraDevice.device(), pipelineLayout, nullptr);
    }

    void AuroraRenderSystem::createPipelineLayout() {
        std::vector<VkDescriptorSetLayout> setLayouts;
        
        if (needsTextureBinding) {
            auto descriptorSetLayout = AuroraDescriptorSetLayout::Builder(auroraDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();
            setLayouts.push_back(descriptorSetLayout->getDescriptorSetLayout());
            descriptorSetLayouts.push_back(std::move(descriptorSetLayout));
        }

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        pipelineLayoutInfo.pSetLayouts = setLayouts.data();
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

    void AuroraRenderSystem::createComponentDescriptorSets(size_t /*componentIndex*/, AuroraMSDFAtlas* msdfAtlas) {
        if (!needsTextureBinding) {
            return;
        }
        
        if (sharedDescriptorSet == VK_NULL_HANDLE) {
            auto imageInfo = msdfAtlas->getDescriptorInfo();
            
            globalDescriptorPool->allocateDescriptor(descriptorSetLayouts[0]->getDescriptorSetLayout(), sharedDescriptorSet);
            
            AuroraDescriptorWriter(*descriptorSetLayouts[0], *globalDescriptorPool)
                .writeImage(0, &imageInfo)
                .overwrite(sharedDescriptorSet);
        }
    }

    void AuroraRenderSystem::addComponent(std::shared_ptr<AuroraComponentInterface> component) {
        size_t componentIndex = components.size();
        components.push_back(component);
        createComponentDescriptorSets(componentIndex, msdfAtlas);
    }

    void AuroraRenderSystem::renderComponents(VkCommandBuffer commandBuffer, const AuroraCamera& camera, int /*frameIndex*/) {
        auroraPipeline->bind(commandBuffer);

        if (needsTextureBinding && sharedDescriptorSet != VK_NULL_HANDLE) {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &sharedDescriptorSet, 0, nullptr);
        }

        glm::mat4 projectionMatrix = camera.getProjection();

        for (size_t i = 0; i < components.size(); i++) {
            const auto& component = components[i];
            
            if (component->isHidden() || !component->model) {
                continue;
            }

            PushConstantData pushData{};
            pushData.transform = component->getMVPMatrix(projectionMatrix);
            pushData.color = component->color;

            vkCmdPushConstants(
                commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PushConstantData),
                &pushData
            );
            
            component->model->bind(commandBuffer);
            component->model->draw(commandBuffer);
        }
    }

    bool AuroraRenderSystem::isCompatibleWith(const AuroraComponentInterface& component) const {
        return vertexShaderPath == component.getVertexShaderPath() &&
               fragmentShaderPath == component.getFragmentShaderPath() &&
               topology == component.getTopology() &&
               needsTextureBinding == component.needsTextureBinding();
    }
}