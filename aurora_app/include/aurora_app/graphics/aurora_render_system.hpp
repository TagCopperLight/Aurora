#pragma once

#include "aurora_pipeline.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_camera.hpp"
#include "aurora_engine/core/aurora_descriptors.hpp"
#include "aurora_engine/core/aurora_buffer.hpp"
#include "aurora_engine/core/aurora_swap_chain.hpp"
#include "aurora_app/graphics/aurora_msdf_atlas.hpp"

#include <memory>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace aurora {
    class AuroraComponentInterface;
    
    struct ComponentUniform {
        glm::mat4 transform;
        alignas(16) glm::vec4 color;
    };
}

namespace aurora {
    struct RenderSystemCreateInfo {
        VkRenderPass renderPass;
        const std::string& vertFilePath;
        const std::string& fragFilePath;
        VkPrimitiveTopology topology;
        AuroraDescriptorPool* descriptorPool;
        AuroraMSDFAtlas* msdfAtlas;
    };

    class AuroraRenderSystem {
        public:
            AuroraRenderSystem(AuroraDevice& device, const RenderSystemCreateInfo& createInfo);
            ~AuroraRenderSystem();

            AuroraRenderSystem(const AuroraRenderSystem&) = delete;
            AuroraRenderSystem &operator=(const AuroraRenderSystem&) = delete;

            void renderComponents(VkCommandBuffer commandBuffer, const AuroraCamera& camera, int frameIndex);

            void addComponent(std::shared_ptr<AuroraComponentInterface> component);
            
            void updateComponentUniform(size_t componentIndex, const ComponentUniform& uniformData, int frameIndex);
            
            size_t getComponentCount() const {
                return components.size();
            }
            
            const std::vector<std::shared_ptr<AuroraComponentInterface>>& getComponents() const {
                return components;
            }
            
            bool isCompatibleWith(const AuroraComponentInterface& component) const;
            
            
            const std::string& getVertexShaderPath() const { return vertexShaderPath; }
            const std::string& getFragmentShaderPath() const { return fragmentShaderPath; }
            VkPrimitiveTopology getTopology() const { return topology; }
            
        private:
            void createPipelineLayout();
            void createPipeline(VkRenderPass renderPass, const std::string& vertFilePath, const std::string& fragFilePath, VkPrimitiveTopology topology);
            void createComponentUniformBuffers(size_t componentIndex, AuroraMSDFAtlas* msdfAtlas);

            AuroraDevice& auroraDevice;

            std::unique_ptr<AuroraPipeline> auroraPipeline;
            VkPipelineLayout pipelineLayout;

            // Per-component, per-frame uniform buffers [componentIndex][frameIndex]
            std::vector<std::vector<std::unique_ptr<AuroraBuffer>>> componentUniformBuffers;
            
            // Per-component, per-frame descriptor sets [componentIndex][frameIndex]
            std::vector<std::vector<VkDescriptorSet>> componentDescriptorSets;

            AuroraDescriptorPool* globalDescriptorPool;
            std::vector<std::unique_ptr<AuroraDescriptorSetLayout>> descriptorSetLayouts{};
            
            // Store reference to MSDF atlas for descriptor set creation
            AuroraMSDFAtlas* msdfAtlas;

            std::vector<std::shared_ptr<AuroraComponentInterface>> components;
            
            
            std::string vertexShaderPath;
            std::string fragmentShaderPath;
            VkPrimitiveTopology topology;
    };
}