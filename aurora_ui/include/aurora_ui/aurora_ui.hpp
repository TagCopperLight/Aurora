#pragma once

#include "aurora_engine/core/aurora_window.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_renderer.hpp"
#include "aurora_ui/graphics/aurora_render_system_manager.hpp"
#include "aurora_ui/components/aurora_component_info.hpp"
#include <memory>
#include <string>

namespace aurora {
    class AuroraUI {
        public:
            static constexpr int WIDTH = 1920;
            static constexpr int HEIGHT = 1080;

            explicit AuroraUI(const std::string& title = "Aurora");
            virtual ~AuroraUI();

            AuroraUI(const AuroraUI&) = delete;
            AuroraUI& operator=(const AuroraUI&) = delete;

            void run();

        protected:
            virtual void onSetup(AuroraComponentInfo&) {}

            virtual void onUpdate(float) {}

        private:
            AuroraWindow auroraWindow;
            AuroraDevice auroraDevice;
            AuroraRenderer auroraRenderer;

            std::unique_ptr<AuroraRenderSystemManager> renderSystemManager;
    };
}