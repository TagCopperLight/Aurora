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
#include <chrono>
#include <thread>
#include <fstream>
#include <iomanip>

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

        const int targetFrameRate = 60;
        const auto targetFrameTime = std::chrono::microseconds(1000000 / targetFrameRate);
        auto lastFrameTime = std::chrono::high_resolution_clock::now();
        auto appStartTime = std::chrono::high_resolution_clock::now();
        
        // Frame time logging
        std::ofstream frameTimeFile("frame_times.csv");
        frameTimeFile << "timestamp_ms,frame_time_ms,fps\n";
        frameTimeFile << std::fixed << std::setprecision(3);
        
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
            auto renderTime = frameEnd - frameStart;

            // auto renderTimeMicros = std::chrono::duration_cast<std::chrono::microseconds>(renderTime);
            // if (renderTimeMicros < targetFrameTime) {
            //     auto sleepTime = targetFrameTime - renderTimeMicros;
            //     std::this_thread::sleep_for(sleepTime);
            // }
            
            auto actualFrameEnd = std::chrono::high_resolution_clock::now();
            auto frameTime = actualFrameEnd - frameStart;
            
            auto timeSinceStart = frameStart - appStartTime;
            double timestampMs = std::chrono::duration<double, std::milli>(timeSinceStart).count();
            double frameTimeMs = std::chrono::duration<double, std::milli>(frameTime).count();
            double fps = 1000.0 / frameTimeMs;
            
            frameTimeFile << timestampMs << "," << frameTimeMs << "," << fps << "\n";
            
            lastFrameTime = frameStart;
        }

        frameTimeFile.close();
        spdlog::info("Frame time data saved to frame_times.csv");
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