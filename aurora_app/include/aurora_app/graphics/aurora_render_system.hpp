#pragma once

#include "aurora_pipeline.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "../aurora_game_object.hpp"

#include <memory>
#include <vector>

namespace aurora {
    class AuroraRenderSystem {
        public:
            AuroraRenderSystem(AuroraDevice& device, VkRenderPass renderPass);
            ~AuroraRenderSystem();

            AuroraRenderSystem(const AuroraRenderSystem&) = delete;
            AuroraRenderSystem &operator=(const AuroraRenderSystem&) = delete;

            void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<AuroraGameObject>& gameObjects);
        private:
            void createPipelineLayout();
            void createPipeline(VkRenderPass renderPass);

            AuroraDevice& auroraDevice;

            std::unique_ptr<AuroraPipeline> auroraPipeline;
            VkPipelineLayout pipelineLayout;
    };
}