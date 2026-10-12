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

            explicit AuroraUI(const std::string& title = "Aurora", VkSampleCountFlagBits maxMsaaSamples = VK_SAMPLE_COUNT_8_BIT);
            virtual ~AuroraUI();

            AuroraUI(const AuroraUI&) = delete;
            AuroraUI& operator=(const AuroraUI&) = delete;

            void run();

        protected:
            virtual void onSetup(AuroraComponentInfo&) {}

            virtual void onUpdate(float) {}

            // Called at the end of every frame, before per-frame profiler data is reset
            virtual void onFrameEnd() {}

            void setFrameRateLimit(bool enabled) { frameRateLimit = enabled; }
            void requestClose() { auroraWindow.requestClose(); }
            AuroraRenderer& getRenderer() { return auroraRenderer; }

        private:
            AuroraWindow auroraWindow;
            AuroraDevice auroraDevice;
            AuroraRenderer auroraRenderer;

            std::unique_ptr<AuroraRenderSystemManager> renderSystemManager;

            bool frameRateLimit = true;
    };
}