#pragma once

#include "aurora_engine/core/aurora_window.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_renderer.hpp"
#include "aurora_app/components/aurora_component_interface.hpp"

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
            void createComponents();

            AuroraWindow auroraWindow{WIDTH, HEIGHT, "Aurora Vulkan App"};
            AuroraDevice auroraDevice{auroraWindow};
            AuroraRenderer auroraRenderer{auroraWindow, auroraDevice};

            std::vector<std::unique_ptr<AuroraComponentInterface>> components;
    };
}