#include "aurora_app/aurora_app.hpp"
#include "aurora_app/graphics/aurora_render_system_manager.hpp"
#include "aurora_app/utils/aurora_clock.hpp"

#include "aurora_app/components/aurora_component_info.hpp"
#include "aurora_app/components/aurora_terminal.hpp"
#include "aurora_app/components/aurora_card.hpp"

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
        AuroraClock clock(60);
        
        while (!auroraWindow.shouldClose()) {
            clock.beginFrame();
            
            glfwPollEvents();

            uint32_t width = auroraRenderer.getWidth();
            uint32_t height = auroraRenderer.getHeight();

            camera.setOrthographicProjection(0, width, height, 0, 0, 1);

            if (auto commandBuffer = auroraRenderer.beginFrame()) {
                auroraRenderer.beginSwapChainRenderPass(commandBuffer);

                renderSystemManager->renderAllComponents(auroraRenderer.getCurrentCommandBuffer(), camera);

                auroraRenderer.endSwapChainRenderPass(commandBuffer);
                auroraRenderer.endFrame();
            } else {
                spdlog::warn("Failed to begin frame, skipping rendering");
            }
            
            clock.endFrame();
            clock.waitForFrameRate();
        }

        vkDeviceWaitIdle(auroraDevice.device());
    }

    void AuroraApp::createRenderSystems() {
        AuroraComponentInfo componentInfo{auroraDevice, *renderSystemManager};

        auto terminalComponent = std::make_shared<AuroraTerminal>(componentInfo, glm::vec2(500.0f, 500.0f));
        terminalComponent->setPosition(20.0f, 20.0f);
        spdlog::info("Adding terminal component to render system");
        terminalComponent->addToRenderSystem();

        // terminalComponent->addText("Vestibulum");
        terminalComponent->addText("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed non risus. Suspendisse lectus tortor, dignissim sit amet, adipiscing nec, ultricies sed, dolor. Cras elementum ultrices diam. Maecenas ligula massa, varius a, semper congue, euismod non, mi. Proin porttitor, orci nec nonummy molestie, enim est eleifend mi, non fermentum diam nisl sit amet erat. Duis semper. Duis arcu massa, scelerisque vitae, consequat in, pretium a, enim. Pellentesque congue. Ut in risus volutpat libero pharetra tempor. Cras vestibulum bibendum augue. Praesent egestas leo in pede. Praesent blandit odio eu enim. Pellentesque sed dui ut augue blandit sodales. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Aliquam nibh. Mauris ac mauris sed pede pellentesque fermentum. Maecenas adipiscing ante non diam sodales hendrerit. ");

        // terminalComponent->addText("This is a short line");
        // terminalComponent->addText("This is a much longer line that will automatically wrap to the next line when it exceeds the terminal width");
        // terminalComponent->addText("Another line");
    }
}