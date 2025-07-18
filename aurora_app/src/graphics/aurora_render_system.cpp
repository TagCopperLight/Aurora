#include "aurora_app/graphics/aurora_render_system.hpp"
#include "aurora_app/components/aurora_component_interface.hpp"
#include "aurora_engine/core/aurora_buffer.hpp"

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

    struct ColorUbo {
        glm::vec4 color;
    };

    AuroraRenderSystem::AuroraRenderSystem(AuroraDevice& device, const RenderSystemCreateInfo& createInfo) 
        : auroraDevice{device}, 
          globalDescriptorPool{createInfo.descriptorPool}, 
          vertexShaderPath{createInfo.vertFilePath}, 
          fragmentShaderPath{createInfo.fragFilePath}, 
          topology{createInfo.topology} {
        createPipelineLayout();
        createPipeline(createInfo.renderPass, createInfo.vertFilePath, createInfo.fragFilePath, createInfo.topology);
        createUniformBuffers(createInfo.msdfAtlas);
    }

    AuroraRenderSystem::~AuroraRenderSystem() {
        vkDestroyPipelineLayout(auroraDevice.device(), pipelineLayout, nullptr);
    }

    void AuroraRenderSystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstantsData);

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

    void AuroraRenderSystem::createUniformBuffers(AuroraMSDFAtlas* msdfAtlas) {
        auto colorBuffer = std::make_unique<AuroraBuffer>(
            auroraDevice,
            sizeof(ColorUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            auroraDevice.properties.limits.minUniformBufferOffsetAlignment
        );

        auto stagingBuffer = std::make_unique<AuroraBuffer>(
            auroraDevice,
            sizeof(ColorUbo),
            1,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            auroraDevice.properties.limits.minUniformBufferOffsetAlignment
        );

        stagingBuffer->map();
        ColorUbo ubo{};
        ubo.color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
        stagingBuffer->writeToBuffer(&ubo);

        VkCommandBuffer commandBuffer = auroraDevice.beginSingleTimeCommands();
        
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = sizeof(ColorUbo);
        
        vkCmdCopyBuffer(commandBuffer, stagingBuffer->getBuffer(), colorBuffer->getBuffer(), 1, &copyRegion);
        
        auroraDevice.endSingleTimeCommands(commandBuffer);

        uniformBuffers.push_back(std::move(colorBuffer));

        descriptorSets.resize(1);
        auto bufferInfo = uniformBuffers[0]->descriptorInfo();
        auto imageInfo = msdfAtlas->getDescriptorInfo();
        AuroraDescriptorWriter(*descriptorSetLayouts[0], *globalDescriptorPool)
            .writeBuffer(0, &bufferInfo)
            .writeImage(1, &imageInfo)
            .build(descriptorSets[0]);
    }

    void AuroraRenderSystem::renderComponents(VkCommandBuffer commandBuffer, const AuroraCamera& camera) {
        auroraPipeline->bind(commandBuffer);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSets.data(), 0, nullptr);

        for (const auto& component : components) {
            if (component->isHidden() || !component->model) {
                continue;
            }

            PushConstantsData push{};
            push.transform = camera.getProjection() * component->getWorldTransform();
            push.color = component->color;
            
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantsData), &push);

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