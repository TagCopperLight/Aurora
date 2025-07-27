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
          topology{createInfo.topology} {
        createPipelineLayout();
        createPipeline(createInfo.renderPass, createInfo.vertFilePath, createInfo.fragFilePath, createInfo.topology);
    }

    AuroraRenderSystem::~AuroraRenderSystem() {
        vkDestroyPipelineLayout(auroraDevice.device(), pipelineLayout, nullptr);
    }

    void AuroraRenderSystem::createPipelineLayout() {
        auto descriptorSetLayout = AuroraDescriptorSetLayout::Builder(auroraDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        std::vector<VkDescriptorSetLayout> setLayouts{descriptorSetLayout->getDescriptorSetLayout()};
        descriptorSetLayouts.push_back(std::move(descriptorSetLayout));

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        pipelineLayoutInfo.pSetLayouts = setLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

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

    void AuroraRenderSystem::createComponentUniformBuffers(size_t componentIndex, AuroraMSDFAtlas* msdfAtlas) {
        if (componentUniformBuffers.size() <= componentIndex) {
            componentUniformBuffers.resize(componentIndex + 1);
            componentDescriptorSets.resize(componentIndex + 1);
        }
        
        componentUniformBuffers[componentIndex].resize(AuroraSwapChain::MAX_FRAMES_IN_FLIGHT);
        componentDescriptorSets[componentIndex].resize(AuroraSwapChain::MAX_FRAMES_IN_FLIGHT);
        
        for (int frameIndex = 0; frameIndex < AuroraSwapChain::MAX_FRAMES_IN_FLIGHT; frameIndex++) {
            auto uniformBuffer = std::make_unique<AuroraBuffer>(
                auroraDevice,
                sizeof(ComponentUniform),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                auroraDevice.properties.limits.minUniformBufferOffsetAlignment
            );
            
            uniformBuffer->map();
            ComponentUniform uniformData{};
            uniformData.color = components[componentIndex]->color;
            uniformBuffer->writeToBuffer(&uniformData);
            
            componentUniformBuffers[componentIndex][frameIndex] = std::move(uniformBuffer);
            
            auto bufferInfo = componentUniformBuffers[componentIndex][frameIndex]->descriptorInfo();
            auto imageInfo = msdfAtlas->getDescriptorInfo();
            
            globalDescriptorPool->allocateDescriptor(descriptorSetLayouts[0]->getDescriptorSetLayout(), componentDescriptorSets[componentIndex][frameIndex]);
            
            AuroraDescriptorWriter(*descriptorSetLayouts[0], *globalDescriptorPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .overwrite(componentDescriptorSets[componentIndex][frameIndex]);
        }
    }

    void AuroraRenderSystem::addComponent(std::shared_ptr<AuroraComponentInterface> component) {
        size_t componentIndex = components.size();
        components.push_back(component);
        createComponentUniformBuffers(componentIndex, msdfAtlas);
    }

    void AuroraRenderSystem::updateComponentUniform(size_t componentIndex, const ComponentUniform& uniformData, int frameIndex) {
        ComponentUniform localUniformData = uniformData;
        componentUniformBuffers[componentIndex][frameIndex]->writeToBuffer(&localUniformData);
    }

    void AuroraRenderSystem::renderComponents(VkCommandBuffer commandBuffer, const AuroraCamera& camera, int frameIndex) {
        auroraPipeline->bind(commandBuffer);

        for (size_t i = 0; i < components.size(); i++) {
            const auto& component = components[i];
            
            if (component->isHidden() || !component->model) {
                continue;
            }

            ComponentUniform componentUniform{};
            componentUniform.transform = camera.getProjection() * component->getWorldTransform();
            componentUniform.color = component->color;

            updateComponentUniform(i, componentUniform, frameIndex);
            
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &componentDescriptorSets[i][frameIndex], 0, nullptr);
            
            component->model->bind(commandBuffer);
            component->model->draw(commandBuffer);
        }
    }

    bool AuroraRenderSystem::isCompatibleWith(const AuroraComponentInterface& component) const {
        return vertexShaderPath == component.getVertexShaderPath() &&
               fragmentShaderPath == component.getFragmentShaderPath() &&
               topology == component.getTopology();
    }
}