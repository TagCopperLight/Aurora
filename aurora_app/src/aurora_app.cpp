#include "aurora_app/aurora_app.hpp"
#include "aurora_app/graphics/aurora_render_system.hpp"

#include "aurora_app/components/aurora_circle_component.hpp"
#include "aurora_app/components/aurora_rounded_rect_component.hpp"
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
        createRenderSystems();
        spdlog::info("Aurora Application ready");
    }

    AuroraApp::~AuroraApp() {}

    void AuroraApp::run() {
        AuroraCamera camera;
        
        while (!auroraWindow.shouldClose()) {
            glfwPollEvents();

            float aspect = auroraRenderer.getAspectRatio();
            camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);

            if (auto commandBuffer = auroraRenderer.beginFrame()) {
                auroraRenderer.beginSwapChainRenderPass(commandBuffer);

                for (const auto& renderSystem : renderSystems) {
                    renderSystem->renderComponents(auroraRenderer.getCurrentCommandBuffer(), camera);
                }

                auroraRenderer.endSwapChainRenderPass(commandBuffer);
                auroraRenderer.endFrame();
            } else {
                spdlog::warn("Failed to begin frame, skipping rendering");
            }
        }

        vkDeviceWaitIdle(auroraDevice.device());
    }

    void AuroraApp::createRenderSystems() {
        auto circleRenderSystem = std::make_unique<AuroraRenderSystem>(
            auroraDevice,
            auroraRenderer.getSwapChainRenderPass(),
            "aurora_app/shaders/shader.vert.spv",
            "aurora_app/shaders/shader.frag.spv",
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN
        );

        auto circleComponent = std::make_unique<AuroraCircleComponent>(auroraDevice, 0.1f);
        circleComponent->color = {1.0f, 0.0f, 0.0f}; // Red color
        circleRenderSystem->addComponent(std::move(circleComponent));
        auto roundedRectComponent = std::make_unique<AuroraRoundedRectangleComponent>(auroraDevice, glm::vec2(0.5f, 0.5f), 0.1f);
        circleRenderSystem->addComponent(std::move(roundedRectComponent));
        auto roundedRectComponent2 = std::make_unique<AuroraRoundedRectangleComponent>(auroraDevice, glm::vec2(0.5f, 0.5f), 0.0f);
        roundedRectComponent2->color = {0.0f, 1.0f, 0.0f}; // Green color
        circleRenderSystem->addComponent(std::move(roundedRectComponent2));

        renderSystems.push_back(std::move(circleRenderSystem));
    }
}