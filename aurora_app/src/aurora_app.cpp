#include "aurora_app/aurora_app.hpp"
#include "aurora_app/graphics/aurora_render_system_manager.hpp"
#include "aurora_app/utils/aurora_clock.hpp"
#include "aurora_engine/profiling/aurora_profiler.hpp"
#include "aurora_app/profiling/aurora_profiler_ui.hpp"

#include "aurora_app/components/aurora_component_info.hpp"
#include "aurora_app/components/aurora_circle.hpp"
#include "aurora_app/components/aurora_text.hpp"
#include "aurora_app/utils/aurora_theme_settings.hpp"

#include <vector>
#include <string>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <spdlog/spdlog.h>
#include <cassert>

namespace aurora {
    AuroraApp::AuroraApp() {
        spdlog::info("Initializing Aurora Application");
        renderSystemManager = std::make_unique<AuroraRenderSystemManager>(auroraDevice, auroraRenderer);
        spdlog::info("Aurora Application ready");
    }

    AuroraApp::~AuroraApp() {}

    void AuroraApp::run() {
        AuroraCamera camera;
        AuroraClock clock(60, true);

        AuroraComponentInfo componentInfo{auroraDevice, *renderSystemManager};
        
        auto& profiler = AuroraProfiler::instance();
        profiler.setEnabled(true);
        
        auto profilerUI = std::make_shared<AuroraProfilerUI>(componentInfo, 400.f);

        struct ColorInfo {
            std::string name;
            glm::vec4 color;
        };

        std::vector<ColorInfo> colors = {
            {"PRIMARY", AuroraThemeSettings::get().PRIMARY},
            {"SECONDARY", AuroraThemeSettings::get().SECONDARY},
            {"ORANGE", AuroraThemeSettings::get().ORANGE},
            {"DISABLED", AuroraThemeSettings::get().DISABLED},
            {"BACKGROUND", AuroraThemeSettings::get().BACKGROUND},
            {"DELIMITER", AuroraThemeSettings::get().DELIMITER},
            {"TEXT_PRIMARY", AuroraThemeSettings::get().TEXT_PRIMARY},
            {"TEXT_SECONDARY", AuroraThemeSettings::get().TEXT_SECONDARY},
            {"TEXT_DISABLED", AuroraThemeSettings::get().TEXT_DISABLED},
            {"SHADOW_LIGHT", AuroraThemeSettings::get().SHADOW_LIGHT},
            {"SHADOW_MEDIUM", AuroraThemeSettings::get().SHADOW_MEDIUM},
            {"SHADOW_HEAVY", AuroraThemeSettings::get().SHADOW_HEAVY},
            {"SHADOW_TRANSPARENT", AuroraThemeSettings::get().SHADOW_TRANSPARENT},
            {"SUCCESS", AuroraThemeSettings::get().SUCCESS},
            {"WARNING", AuroraThemeSettings::get().WARNING},
            {"ERROR", AuroraThemeSettings::get().ERROR},
            {"INFO", AuroraThemeSettings::get().INFO},
            {"TRANSPARENT", AuroraThemeSettings::get().TRANSPARENT},
            {"WHITE", AuroraThemeSettings::get().WHITE},
            {"BLACK", AuroraThemeSettings::get().BLACK}
        };

        float startY = 200.0f;
        float startX = 50.0f;
        float spacingY = 40.0f;

        for (size_t i = 0; i < colors.size(); ++i) {
            auto circle = std::make_shared<AuroraCircle>(componentInfo, 10.0f, colors[i].color);
            circle->setPosition({startX, startY + i * spacingY});
            renderSystemManager->addComponentToQueue(circle);

            auto text = std::make_shared<AuroraText>(componentInfo, colors[i].name, 20.0f);
            text->setPosition({startX + 30.0f, startY + i * spacingY - 10.0f});
            renderSystemManager->addComponentToQueue(text);
        }

        // clock.enableCSVLogging();
        
        while (!auroraWindow.shouldClose()) {
            clock.beginFrame();
            
            glfwPollEvents();

            uint32_t width = auroraRenderer.getWidth();
            uint32_t height = auroraRenderer.getHeight();

            camera.setOrthographicProjection(0, width, height, 0, 0, 1);

            VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
            commandBuffer = auroraRenderer.beginFrame();

            if (commandBuffer) {
                auroraRenderer.beginSwapChainRenderPass(commandBuffer);

                renderSystemManager->renderAllComponents(auroraRenderer.getCurrentCommandBuffer(), camera);

                auroraRenderer.endSwapChainRenderPass(commandBuffer);

                auroraRenderer.endFrame();
            } else {
                spdlog::warn("Failed to begin frame, skipping rendering");
            }

            profiler.setFrameTime(clock.getFrameTimeMs());
            profilerUI->update(clock.getFrameTimeMs() / 1000.0f);
            
            clock.endFrame();
            
            profiler.newFrame();
        }

        vkDeviceWaitIdle(auroraDevice.device());
    }
}