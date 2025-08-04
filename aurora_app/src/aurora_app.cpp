#define AURORA_PROFILING_ENABLED
#include "aurora_app/aurora_app.hpp"
#include "aurora_app/graphics/aurora_render_system_manager.hpp"
#include "aurora_app/utils/aurora_clock.hpp"
#include "aurora_app/profiling/aurora_profiler.hpp"
#include "aurora_app/profiling/aurora_profiler_ui.hpp"

#include "aurora_app/components/aurora_component_info.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <spdlog/spdlog.h>
#include <cassert>

namespace aurora {
    AuroraApp::AuroraApp() {
        spdlog::info("Initializing Aurora Application");
        renderSystemManager = std::make_unique<AuroraRenderSystemManager>(auroraDevice, auroraRenderer);
        createRenderSystems();
        spdlog::info("Aurora Application ready");
    }

    AuroraApp::~AuroraApp() {}

    void AuroraApp::run() {
        AuroraCamera camera;
        AuroraClock clock(60, false);

        AuroraComponentInfo componentInfo{auroraDevice, *renderSystemManager};
        
        auto& profiler = AuroraProfiler::instance();
        profiler.setEnabled(true);

        auto profilerUI = std::make_shared<AuroraProfilerUI>(componentInfo, 400.f);
        profilerUI->addProfiledFunction("Render Components");
        profilerUI->setUpdateFrequency(60.0f);
        
        clock.enableCSVLogging();
        
        while (!auroraWindow.shouldClose()) {
            clock.beginFrame();
            
            glfwPollEvents();

            uint32_t width = auroraRenderer.getWidth();
            uint32_t height = auroraRenderer.getHeight();

            camera.setOrthographicProjection(0, width, height, 0, 0, 1);

            auto commandBuffer = auroraRenderer.beginFrame();
            if (commandBuffer) {
                auroraRenderer.beginSwapChainRenderPass(commandBuffer);

                {
                AURORA_PROFILE("Render Components");
                renderSystemManager->renderAllComponents(auroraRenderer.getCurrentCommandBuffer(), camera);
                }

                auroraRenderer.endSwapChainRenderPass(commandBuffer);

                auroraRenderer.endFrame();
            } else {
                spdlog::warn("Failed to begin frame, skipping rendering");
            }
            
            profiler.setFrameTime(clock.getFrameTimeMs());
            
            profilerUI->update(clock.getFrameTimeMs() / 1000.0f);
            
            profiler.newFrame();

            clock.endFrame();
        }

        vkDeviceWaitIdle(auroraDevice.device());
    }

    void AuroraApp::createRenderSystems() {
    }
}