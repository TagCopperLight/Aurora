#pragma once

#include "aurora_engine/core/aurora_window.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_renderer.hpp"
#include "aurora_ui/graphics/aurora_render_system_manager.hpp"
#include "aurora_ui/components/aurora_component_info.hpp"
#include "aurora_ui/components/aurora_component_interface.hpp"
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
            virtual void onSetup() {}

            virtual void onUpdate(float) {}

            // Called at the end of every frame, before per-frame profiler data is reset
            virtual void onFrameEnd() {}

            // Called with the GLFW key code (GLFW_KEY_*) and modifier bits when a key is pressed or repeats
            virtual void onKey(int /*key*/, int /*mods*/) {}

            void setFrameRateLimit(bool enabled) { frameRateLimit = enabled; }
            void requestClose() { auroraWindow.requestClose(); }
            AuroraRenderer& getRenderer() { return auroraRenderer; }

            // Creates a component and starts drawing it, e.g. create<AuroraCircle>(radius, color)
            template <typename T, typename... Args>
            std::shared_ptr<T> create(Args&&... args) {
                auto component = std::make_shared<T>(*componentInfo, std::forward<Args>(args)...);
                component->addToRenderSystem();
                return component;
            }

            // Stops drawing a component created with create(), along with its children
            void destroy(const std::shared_ptr<AuroraComponentInterface>& component);

            // Needed to build children before handing them to addChild()
            AuroraComponentInfo& getComponentInfo() { return *componentInfo; }

        private:
            AuroraWindow auroraWindow;
            AuroraDevice auroraDevice;
            AuroraRenderer auroraRenderer;

            std::unique_ptr<AuroraRenderSystemManager> renderSystemManager;
            std::unique_ptr<AuroraComponentInfo> componentInfo;

            bool frameRateLimit = false;
    };
}