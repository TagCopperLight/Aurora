#pragma once

#include "aurora_pipeline.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_camera.hpp"
#include "aurora_engine/core/aurora_descriptors.hpp"
#include "aurora_app/components/aurora_component_interface.hpp"
#include "aurora_app/graphics/aurora_msdf_atlas.hpp"

#include <memory>
#include <vector>

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

            void renderComponents(VkCommandBuffer commandBuffer, const AuroraCamera& camera);

            void addComponent(std::unique_ptr<AuroraComponentInterface> component) {
                components.push_back(std::move(component));
            }
        
            size_t getComponentCount() const {
                return components.size();
            }
            
            // Check if this render system is compatible with a component
            bool isCompatibleWith(const AuroraComponentInterface& component) const {
                return vertexShaderPath == component.getVertexShaderPath() &&
                       fragmentShaderPath == component.getFragmentShaderPath() &&
                       topology == component.getTopology();
            }
            
            // Getters for render system properties
            const std::string& getVertexShaderPath() const { return vertexShaderPath; }
            const std::string& getFragmentShaderPath() const { return fragmentShaderPath; }
            VkPrimitiveTopology getTopology() const { return topology; }
            
        private:
            void createPipelineLayout();
            void createPipeline(VkRenderPass renderPass, const std::string& vertFilePath, const std::string& fragFilePath, VkPrimitiveTopology topology);
            void createUniformBuffers(AuroraMSDFAtlas* msdfAtlas);

            AuroraDevice& auroraDevice;

            std::unique_ptr<AuroraPipeline> auroraPipeline;
            VkPipelineLayout pipelineLayout;

            std::vector<std::unique_ptr<AuroraBuffer>> uniformBuffers;

            AuroraDescriptorPool* globalDescriptorPool;
            std::vector<std::unique_ptr<AuroraDescriptorSetLayout>> descriptorSetLayouts{};
            std::vector<VkDescriptorSet> descriptorSets{};

            std::vector<std::unique_ptr<AuroraComponentInterface>> components;
            
            // Store render system properties for compatibility checking
            std::string vertexShaderPath;
            std::string fragmentShaderPath;
            VkPrimitiveTopology topology;
    };
}