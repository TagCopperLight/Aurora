#include "aurora_ui/aurora_ui.hpp"
#include "aurora_ui/utils/aurora_clock.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"
#include "aurora_engine/profiling/aurora_profiler.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <spdlog/spdlog.h>

namespace aurora {
    AuroraUI::AuroraUI(const std::string& title)
        : auroraWindow{WIDTH, HEIGHT, title},
          auroraDevice{auroraWindow},
          auroraRenderer{auroraWindow, auroraDevice, AuroraThemeSettings::get().BACKGROUND} {
        spdlog::info("Initializing Aurora UI");
        renderSystemManager = std::make_unique<AuroraRenderSystemManager>(auroraDevice, auroraRenderer);
        spdlog::info("Aurora UI ready");
    }

    AuroraUI::~AuroraUI() {}

    void AuroraUI::run(std::function<void(AuroraComponentInfo&)> setup) {
        AuroraCamera camera;
        AuroraClock clock(60, true);

        AuroraComponentInfo componentInfo{auroraDevice, *renderSystemManager};

        auto& profiler = AuroraProfiler::instance();
        profiler.setEnabled(true);

        if (setup) {
            setup(componentInfo);
        }

        while (!auroraWindow.shouldClose()) {
            clock.beginFrame();

            glfwPollEvents();

            uint32_t width = auroraRenderer.getWidth();
            uint32_t height = auroraRenderer.getHeight();

            camera.setOrthographicProjection(0, width, height, 0, 0, 1);

            VkCommandBuffer commandBuffer = auroraRenderer.beginFrame();

            if (commandBuffer) {
                auroraRenderer.beginSwapChainRenderPass(commandBuffer);
                renderSystemManager->renderAllComponents(auroraRenderer.getCurrentCommandBuffer(), camera);
                auroraRenderer.endSwapChainRenderPass(commandBuffer);
                auroraRenderer.endFrame();
            } else {
                spdlog::warn("Failed to begin frame, skipping rendering");
            }

            profiler.setFrameTime(clock.getFrameTimeMs());
            clock.endFrame();
            profiler.newFrame();
        }

        vkDeviceWaitIdle(auroraDevice.device());
    }
}