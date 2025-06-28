#include "aurora_app/aurora_app.hpp"
#include "aurora_app/graphics/aurora_render_system.hpp"

#include "aurora_app/components/aurora_triangle_component.hpp"

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
        createComponents();
        spdlog::info("Aurora Application ready");
    }

    AuroraApp::~AuroraApp() {}

    void AuroraApp::run() {
        AuroraRenderSystem auroraRenderSystem{auroraDevice, auroraRenderer.getSwapChainRenderPass()};

        while (!auroraWindow.shouldClose()) {
            glfwPollEvents();

            for (auto& component : components) {
                component->update(0.0016f); // Assuming a fixed delta time for simplicity
            }
            
            if (auto commandBuffer = auroraRenderer.beginFrame()) {
                auroraRenderer.beginSwapChainRenderPass(commandBuffer);

                auroraRenderSystem.renderComponents(commandBuffer, components);

                auroraRenderer.endSwapChainRenderPass(commandBuffer);
                auroraRenderer.endFrame();
            } else {
                spdlog::warn("Failed to begin frame, skipping rendering");
            }
        }

        vkDeviceWaitIdle(auroraDevice.device());
    }

    void AuroraApp::createComponents() {
        components.push_back(std::make_unique<AuroraTriangleComponent>(auroraDevice));
        spdlog::info("Created {} components", components.size());
    }
}