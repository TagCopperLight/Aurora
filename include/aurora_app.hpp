#pragma once

#include "aurora_window.hpp"
#include "aurora_pipeline.hpp"
#include "aurora_device.hpp"
#include "aurora_swap_chain.hpp"
#include "aurora_game_object.hpp"

#include <memory>
#include <vector>

namespace aurora {
    class AuroraApp {
        public:
            static constexpr int WIDTH = 1200;
            static constexpr int HEIGHT = 900;

            AuroraApp();
            ~AuroraApp();

            AuroraApp(const AuroraApp&) = delete;
            AuroraApp &operator=(const AuroraApp&) = delete;

            void run();

        private:
            void sierpinski(std::vector<AuroraModel::Vertex>& vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top);
            void loadGameObjects(); 
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void freeCommandBuffers();
            void drawFrame();
            void recreateSwapChain();
            void recordCommandBuffer(int imageIndex);
            void renderGameObjects(VkCommandBuffer commandBuffer);

            AuroraWindow auroraWindow{WIDTH, HEIGHT, "Aurora Vulkan App"};
            AuroraDevice auroraDevice{auroraWindow};
            std::unique_ptr<AuroraSwapChain> auroraSwapChain;
            std::unique_ptr<AuroraPipeline> auroraPipeline;
            VkPipelineLayout pipelineLayout;
            std::vector<VkCommandBuffer> commandBuffers;
            std::vector<AuroraGameObject> gameObjects;
    };
}