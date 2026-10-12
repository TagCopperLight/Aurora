#include "aurora_ui/aurora_ui.hpp"
#include "aurora_ui/utils/aurora_clock.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"
#include "aurora_engine/profiling/aurora_profiler.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "aurora_engine/utils/log.hpp"
#include <fontconfig/fontconfig.h>

namespace aurora {
    AuroraUI::AuroraUI(const std::string& title, VkSampleCountFlagBits maxMsaaSamples)
        : auroraWindow{WIDTH, HEIGHT, title},
          auroraDevice{auroraWindow, maxMsaaSamples},
          auroraRenderer{auroraWindow, auroraDevice, AuroraThemeSettings::get().BACKGROUND} {
        FcInit();
        log::ui()->info("Initializing Aurora UI");
        renderSystemManager = std::make_unique<AuroraRenderSystemManager>(auroraDevice, auroraRenderer);
        componentInfo = std::make_unique<AuroraComponentInfo>(AuroraComponentInfo{auroraDevice, *renderSystemManager});
        log::ui()->info("Aurora UI ready");
    }

    AuroraUI::~AuroraUI() {}

    void AuroraUI::destroy(const std::shared_ptr<AuroraComponentInterface>& component) {
        renderSystemManager->removeComponent(component);
    }

    void AuroraUI::run() {
        AuroraCamera camera;
        AuroraClock clock(60, frameRateLimit);

        auto& profiler = AuroraProfiler::instance();
        profiler.setEnabled(true);

        onSetup();

        while (!auroraWindow.shouldClose()) {
            clock.beginFrame();

            {
                AURORA_PROFILE("Poll Events");
                glfwPollEvents();
            }

            uint32_t width = auroraRenderer.getWidth();
            uint32_t height = auroraRenderer.getHeight();

            camera.setOrthographicProjection(0, width, height, 0, 0, 1);

            {
                AURORA_PROFILE("Update");
                onUpdate(static_cast<float>(clock.getFrameTimeMs()) / 1000.f);
            }

            VkCommandBuffer commandBuffer;
            {
                AURORA_PROFILE("Begin Frame");
                commandBuffer = auroraRenderer.beginFrame();
            }

            if (commandBuffer) {
                {
                    AURORA_PROFILE("Render Components");
                    auroraRenderer.beginSwapChainRenderPass(commandBuffer);
                    renderSystemManager->renderAllComponents(auroraRenderer.getCurrentCommandBuffer(), camera);
                    auroraRenderer.endSwapChainRenderPass(commandBuffer);
                }
                AURORA_PROFILE("End Frame");
                auroraRenderer.endFrame();
            } else {
                log::ui()->warn("Failed to begin frame, skipping rendering");
            }

            profiler.setFrameTime(clock.getFrameTimeMs());
            clock.endFrame();
            onFrameEnd();
            profiler.newFrame();
        }

        vkDeviceWaitIdle(auroraDevice.device());
    }
}