#include "aurora_ui/graphics/aurora_render_system.hpp"
#include "aurora_ui/components/aurora_component_interface.hpp"
#include "aurora_engine/profiling/aurora_profiler.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <algorithm>
#include "aurora_engine/utils/log.hpp"
#include <cassert>

namespace aurora {
    AuroraRenderSystem::AuroraRenderSystem(AuroraDevice& device, const RenderSystemCreateInfo& createInfo) 
        : auroraDevice{device}, 
          globalDescriptorPool{createInfo.descriptorPool}, 
          msdfAtlas{createInfo.msdfAtlas},
          vertexShaderPath{createInfo.vertFilePath}, 
          fragmentShaderPath{createInfo.fragFilePath}, 
          topology{createInfo.topology},
          needsTextureBinding{createInfo.needsTextureBinding},
          transparent{createInfo.isTransparent} {
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
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
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
        AuroraPipeline::defaultPipelineConfigInfo(pipelineConfig, topology, auroraDevice.msaaSamples);
        
        if (transparent) {
             pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        }

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
        batchesDirty = true;
        createComponentDescriptorSets(componentIndex, msdfAtlas);
    }

    void AuroraRenderSystem::removeComponent(std::shared_ptr<AuroraComponentInterface> component) {
        auto it = std::find(components.begin(), components.end(), component);
        if (it != components.end()) {
            components.erase(it);
            batchesDirty = true;
        }
    }

    bool AuroraRenderSystem::batchesOutdated() const {
        if (batchesDirty || batchedModels.size() != components.size()) {
            return true;
        }
        for (size_t i = 0; i < components.size(); ++i) {
            if (components[i]->model.get() != batchedModels[i]) {
                return true;
            }
        }
        return false;
    }

    void AuroraRenderSystem::rebuildBatches() {
        batches.clear();
        batchedModels.clear();
        batchedModels.reserve(components.size());

        for (const auto& component : components) {
            AuroraModel* model = component->model.get();
            batchedModels.push_back(model);
            if (!model) {
                continue;
            }
            if (batches.empty() || batches.back().model != model) {
                batches.push_back({model, {}});
            }
            batches.back().members.push_back(component.get());
        }
        batchesDirty = false;
    }

    void AuroraRenderSystem::renderComponents(VkCommandBuffer commandBuffer, const AuroraCamera& camera, int frameIndex) {
        auroraPipeline->bind(commandBuffer);

        if (needsTextureBinding && sharedDescriptorSet != VK_NULL_HANDLE) {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &sharedDescriptorSet, 0, nullptr);
        }

        PushConstantData pushData{};
        pushData.projectionViewMatrix = camera.getProjection();
        
        vkCmdPushConstants(
            commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(PushConstantData),
            &pushData
        );

        if (batchesOutdated()) {
            rebuildBatches();
        }
        if (batches.empty()) {
            return;
        }

        // Sized for every component; hidden ones are skipped so fewer instances may be written
        AuroraBuffer& instanceBuffer = getInstanceBuffer(frameIndex, sizeof(AuroraModel::InstanceData) * components.size());
        auto* instanceMemory = static_cast<AuroraModel::InstanceData*>(instanceBuffer.getMappedMemory());

        VkBuffer instanceBuffers[] = {instanceBuffer.getBuffer()};
        VkDeviceSize instanceOffsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 1, 1, instanceBuffers, instanceOffsets);

        // Models mostly live in a few shared pool buffers. Their offsets are vertex-aligned, so
        // each buffer is bound once and models are selected through firstVertex / vertexOffset.
        VkBuffer boundVertexBuffer = VK_NULL_HANDLE;
        VkBuffer boundIndexBuffer = VK_NULL_HANDLE;
        VkDeviceSize boundIndexOffset = 0;

        uint32_t firstInstance = 0;
        uint64_t drawCalls = 0;
        for (const auto& batch : batches) {
             AuroraModel* model = batch.model;
             uint32_t instanceCount = 0;
             for (const AuroraComponentInterface* component : batch.members) {
                 if (component->isHidden()) {
                     continue;
                 }
                 AuroraModel::InstanceData& data = instanceMemory[firstInstance + instanceCount];
                 data.modelMatrix = component->getWorldTransform();
                 data.color = component->color;
                 instanceCount++;
             }
             if (instanceCount == 0) continue;
             if (model->getVertexBuffer() == VK_NULL_HANDLE) {
                 // Vertex allocation failed when the model was created; nothing to draw
                 firstInstance += instanceCount;
                 continue;
             }

             VkDeviceSize vertexOffset = model->getVertexBufferOffset();
             if (vertexOffset % sizeof(AuroraModel::Vertex) != 0) {
                 model->bind(commandBuffer);
                 boundVertexBuffer = VK_NULL_HANDLE;
                 boundIndexBuffer = VK_NULL_HANDLE;
                 model->draw(commandBuffer, instanceCount, firstInstance);
             } else {
                 if (model->getVertexBuffer() != boundVertexBuffer) {
                     boundVertexBuffer = model->getVertexBuffer();
                     VkDeviceSize zeroOffset = 0;
                     vkCmdBindVertexBuffers(commandBuffer, 0, 1, &boundVertexBuffer, &zeroOffset);
                 }

                 uint32_t firstVertex = static_cast<uint32_t>(vertexOffset / sizeof(AuroraModel::Vertex));
                 if (model->hasIndices()) {
                     if (model->getIndexBuffer() != boundIndexBuffer || model->getIndexBufferOffset() != boundIndexOffset) {
                         boundIndexBuffer = model->getIndexBuffer();
                         boundIndexOffset = model->getIndexBufferOffset();
                         vkCmdBindIndexBuffer(commandBuffer, boundIndexBuffer, boundIndexOffset, VK_INDEX_TYPE_UINT32);
                     }
                     vkCmdDrawIndexed(commandBuffer, model->getIndexCount(), instanceCount, 0, static_cast<int32_t>(firstVertex), firstInstance);
                 } else {
                     vkCmdDraw(commandBuffer, model->getVertexCount(), instanceCount, firstVertex, firstInstance);
                 }
             }

             firstInstance += instanceCount;
             drawCalls++;
        }

        AuroraProfiler::instance().incrementCounter("Draw Calls", drawCalls);
    }

    AuroraBuffer& AuroraRenderSystem::getInstanceBuffer(int frameIndex, VkDeviceSize requiredSize) {
        auto& buffer = frameInstanceBuffers[frameIndex];
        if (!buffer || buffer->getBufferSize() < requiredSize) {
            // Grow geometrically so a slowly growing scene doesn't reallocate every frame
            VkDeviceSize capacity = buffer ? buffer->getBufferSize() : 0;
            capacity = std::max(requiredSize, capacity * 2);
            buffer = std::make_unique<AuroraBuffer>(
                auroraDevice,
                capacity,
                1,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
            buffer->map();
        }
        return *buffer;
    }

    bool AuroraRenderSystem::isCompatibleWith(const AuroraComponentInterface& component) const {
        return vertexShaderPath == component.getVertexShaderPath() &&
               fragmentShaderPath == component.getFragmentShaderPath() &&
               topology == component.getTopology() &&
               needsTextureBinding == component.needsTextureBinding() &&
               transparent == component.isTransparent();
    }
}