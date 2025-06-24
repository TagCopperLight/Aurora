#pragma once

#include "aurora_window.hpp"
#include "aurora_pipeline.hpp"

namespace aurora {
    class AuroraApp {
        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            void run();

        private:
            AuroraWindow auroraWindow{WIDTH, HEIGHT, "Aurora Vulkan App"};
            AuroraPipeline auroraPipeline{"../shaders/shader.vert.spv", "../shaders/shader.frag.spv"}; 
    };
}