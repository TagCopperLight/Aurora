#pragma once

#include "aurora_window.hpp"
#include "aurora_pipeline.hpp"
#include "aurora_device.hpp"

namespace aurora {
    class AuroraApp {
        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            void run();

        private:
            AuroraWindow auroraWindow{WIDTH, HEIGHT, "Aurora Vulkan App"};
            AuroraDevice auroraDevice{auroraWindow};
            AuroraPipeline auroraPipeline{ auroraDevice, "../shaders/shader.vert.spv", "../shaders/shader.frag.spv", AuroraPipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT) }; 
    };
}