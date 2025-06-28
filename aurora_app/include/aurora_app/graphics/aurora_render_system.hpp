#pragma once

#include "aurora_pipeline.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_app/components/aurora_component_interface.hpp"

#include <memory>
#include <vector>

namespace aurora {
    class AuroraRenderSystem {
        public:
            AuroraRenderSystem(AuroraDevice& device, VkRenderPass renderPass);
            ~AuroraRenderSystem();

            AuroraRenderSystem(const AuroraRenderSystem&) = delete;
            AuroraRenderSystem &operator=(const AuroraRenderSystem&) = delete;

            void renderComponents(VkCommandBuffer commandBuffer, const std::vector<std::unique_ptr<AuroraComponentInterface>>& components);
        private:
            void createPipelineLayout();
            void createPipeline(VkRenderPass renderPass);

            AuroraDevice& auroraDevice;

            std::unique_ptr<AuroraPipeline> auroraPipeline;
            VkPipelineLayout pipelineLayout;
    };
}