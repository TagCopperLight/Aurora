#pragma once

#include "aurora_window.hpp"
#include "aurora_pipeline.hpp"
#include "aurora_device.hpp"
#include "aurora_swap_chain.hpp"
#include "aurora_model.hpp"

#include <memory>
#include <vector>

namespace aurora {
    class AuroraApp {
        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            AuroraApp();
            ~AuroraApp();

            AuroraApp(const AuroraApp&) = delete;
            AuroraApp &operator=(const AuroraApp&) = delete;

            void run();

        private:
            void sierpinski(std::vector<AuroraModel::Vertex>& vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top);
            void loadModels(); 
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void drawFrame();

            AuroraWindow auroraWindow{WIDTH, HEIGHT, "Aurora Vulkan App"};
            AuroraDevice auroraDevice{auroraWindow};
            AuroraSwapChain auroraSwapChain{auroraDevice, auroraWindow.getExtent()};
            std::unique_ptr<AuroraPipeline> auroraPipeline;
            VkPipelineLayout pipelineLayout;
            std::vector<VkCommandBuffer> commandBuffers;
            std::unique_ptr<AuroraModel> auroraModel;
    };
}