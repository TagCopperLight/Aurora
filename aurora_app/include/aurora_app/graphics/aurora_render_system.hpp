#pragma once

#include "aurora_pipeline.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_app/components/aurora_component_interface.hpp"

#include <memory>
#include <vector>

namespace aurora {
    class AuroraRenderSystem {
        public:
            AuroraRenderSystem(AuroraDevice& device, VkRenderPass renderPass, const std::string& vertFilePath, const std::string& fragFilePath, VkPrimitiveTopology topology);
            ~AuroraRenderSystem();

            AuroraRenderSystem(const AuroraRenderSystem&) = delete;
            AuroraRenderSystem &operator=(const AuroraRenderSystem&) = delete;

            void renderComponents(VkCommandBuffer commandBuffer);

            void addComponent(std::unique_ptr<AuroraComponentInterface> component) {
                components.push_back(std::move(component));
            }
        
            size_t getComponentCount() const {
                return components.size();
            }
            
        private:
            void createPipelineLayout();
            void createPipeline(VkRenderPass renderPass, const std::string& vertFilePath, const std::string& fragFilePath, VkPrimitiveTopology topology);

            AuroraDevice& auroraDevice;

            std::unique_ptr<AuroraPipeline> auroraPipeline;
            VkPipelineLayout pipelineLayout;

            std::vector<std::unique_ptr<AuroraComponentInterface>> components;
    };
}