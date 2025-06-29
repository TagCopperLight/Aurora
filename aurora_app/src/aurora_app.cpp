#include "aurora_app/aurora_app.hpp"
#include "aurora_app/graphics/aurora_render_system_manager.hpp"

#include "aurora_app/components/aurora_circle.hpp"
#include "aurora_app/components/aurora_rounded_rect.hpp"
#include "aurora_app/components/aurora_triangle.hpp"
#include "aurora_app/components/aurora_rounded_borders.hpp"
#include "aurora_app/components/aurora_rounded_shadows.hpp"

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
        renderSystemManager = std::make_unique<AuroraRenderSystemManager>(auroraDevice, auroraRenderer.getSwapChainRenderPass());
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

                renderSystemManager->renderAllComponents(auroraRenderer.getCurrentCommandBuffer(), camera);

                auroraRenderer.endSwapChainRenderPass(commandBuffer);
                auroraRenderer.endFrame();
            } else {
                spdlog::warn("Failed to begin frame, skipping rendering");
            }
        }

        vkDeviceWaitIdle(auroraDevice.device());
    }

    void AuroraApp::createRenderSystems() {
        auto bordersComponent = std::make_unique<AuroraRoundedBorders>(auroraDevice, glm::vec2(0.8f, 0.8f), 0.1f, 0.0075f);
        bordersComponent->color = {0.784f, 0.38f, 0.286f};
        float borderZ = bordersComponent->transform.translation.z;
        renderSystemManager->addComponent(std::move(bordersComponent));

        auto roundedRectComponent = std::make_unique<AuroraRoundedRectangle>(auroraDevice, glm::vec2(0.8f, 0.8f), 0.1f);
        roundedRectComponent->color = {0.196f, 0.196f, 0.196f};
        roundedRectComponent->transform.translation.z = borderZ + 0.01f;
        renderSystemManager->addComponent(std::move(roundedRectComponent));

        auto shadowBordersComponent = std::make_unique<AuroraRoundedShadows>(auroraDevice, glm::vec2(0.8f, 0.8f), 0.1f, 0.025f);
        renderSystemManager->addComponent(std::move(shadowBordersComponent));

        spdlog::info("Created {} render systems with {} total components",
                     renderSystemManager->getRenderSystemCount(),
                     renderSystemManager->getTotalComponentCount());
    }
}