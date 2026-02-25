#pragma once

#include "aurora_pipeline.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_camera.hpp"
#include "aurora_engine/core/aurora_descriptors.hpp"
#include "aurora_engine/core/aurora_buffer_pool.hpp"
#include "aurora_ui/graphics/aurora_msdf_atlas.hpp"
#include "aurora_ui/graphics/aurora_model.hpp"

#include <memory>
#include <vector>
#include <unordered_map>
#include <map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace aurora {
    class AuroraComponentInterface;
    

    struct PushConstantData {
        glm::mat4 projectionViewMatrix;
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
        bool needsTextureBinding;
        bool isTransparent;
    };

    class AuroraRenderSystem {
        public:
            AuroraRenderSystem(AuroraDevice& device, const RenderSystemCreateInfo& createInfo);
            ~AuroraRenderSystem();

            AuroraRenderSystem(const AuroraRenderSystem&) = delete;
            AuroraRenderSystem &operator=(const AuroraRenderSystem&) = delete;

            void renderComponents(VkCommandBuffer commandBuffer, const AuroraCamera& camera, int frameIndex);

            void addComponent(std::shared_ptr<AuroraComponentInterface> component);
            void removeComponent(std::shared_ptr<AuroraComponentInterface> component);
            
            size_t getComponentCount() const {
                return components.size();
            }
            
            const std::vector<std::shared_ptr<AuroraComponentInterface>>& getComponents() const {
                return components;
            }
            
            bool isCompatibleWith(const AuroraComponentInterface& component) const;
            bool isTransparent() const { return transparent; }
            
            
            const std::string& getVertexShaderPath() const { return vertexShaderPath; }
            const std::string& getFragmentShaderPath() const { return fragmentShaderPath; }
            VkPrimitiveTopology getTopology() const { return topology; }
            
        private:
            void createPipelineLayout();
            void createPipeline(VkRenderPass renderPass, const std::string& vertFilePath, const std::string& fragFilePath, VkPrimitiveTopology topology);
            void createComponentDescriptorSets(size_t componentIndex, AuroraMSDFAtlas* msdfAtlas);

            AuroraDevice& auroraDevice;

            std::unique_ptr<AuroraPipeline> auroraPipeline;
            VkPipelineLayout pipelineLayout;

            VkDescriptorSet sharedDescriptorSet = VK_NULL_HANDLE;

            AuroraDescriptorPool* globalDescriptorPool;
            std::vector<std::unique_ptr<AuroraDescriptorSetLayout>> descriptorSetLayouts{};
            
            AuroraMSDFAtlas* msdfAtlas;

            std::vector<std::shared_ptr<AuroraComponentInterface>> components;
            
            std::map<int, std::vector<BufferAllocation>> frameInstanceAllocations;
            std::unordered_map<AuroraModel*, std::vector<AuroraModel::InstanceData>> batches;
            
            std::string vertexShaderPath;
            std::string fragmentShaderPath;
            VkPrimitiveTopology topology;
            bool needsTextureBinding;
            bool transparent;
    };
}