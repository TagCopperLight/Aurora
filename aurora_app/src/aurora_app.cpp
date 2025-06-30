#include "aurora_app/aurora_app.hpp"
#include "aurora_app/graphics/aurora_render_system_manager.hpp"

#include "aurora_app/components/aurora_card.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <spdlog/spdlog.h>
#include <cassert>
// #include <chrono>
// #include <thread>

namespace aurora {
    AuroraApp::AuroraApp() {
        spdlog::info("Initializing Aurora Application");
        renderSystemManager = std::make_unique<AuroraRenderSystemManager>(auroraDevice, auroraRenderer.getSwapChainRenderPass());
        createRenderSystems();
        spdlog::info("Aurora Application ready");
    }

    AuroraApp::~AuroraApp() {}

    void AuroraApp::run() {
        AuroraCamera camera;
        
        const auto targetFrameTime = std::chrono::microseconds(1000000/60);
        auto lastFrameTime = std::chrono::high_resolution_clock::now();
        
        while (!auroraWindow.shouldClose()) {
            auto frameStart = std::chrono::high_resolution_clock::now();
            
            glfwPollEvents();

            float aspect = auroraRenderer.getAspectRatio();
            camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);

            if (auto commandBuffer = auroraRenderer.beginFrame()) {
                auroraRenderer.beginSwapChainRenderPass(commandBuffer);

                renderSystemManager->renderAllComponents(auroraRenderer.getCurrentCommandBuffer(), camera);

                auroraRenderer.endSwapChainRenderPass(commandBuffer);
                auroraRenderer.endFrame();
            } else {
                spdlog::warn("Failed to begin frame, skipping rendering");
            }
            
            auto frameEnd = std::chrono::high_resolution_clock::now();
            auto frameTime = frameEnd - frameStart;
            
            if (frameTime < targetFrameTime) {
                std::this_thread::sleep_for(targetFrameTime - frameTime);
            }
            
            lastFrameTime = frameStart;
        }

        vkDeviceWaitIdle(auroraDevice.device());
    }

    void AuroraApp::createRenderSystems() {
        auto cardComponent = std::make_unique<AuroraCard>(auroraDevice, glm::vec2(0.8f, 0.8f), glm::vec4(0.784f, 0.38f, 0.286f, 1.0f));
        renderSystemManager->addComponent(std::move(cardComponent));

        spdlog::info("Created {} render systems with {} total components",
                     renderSystemManager->getRenderSystemCount(),
                     renderSystemManager->getTotalComponentCount());
    }
}