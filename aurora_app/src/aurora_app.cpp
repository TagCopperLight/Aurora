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
        createRenderSystems();
        spdlog::info("Aurora Application ready");
    }

    AuroraApp::~AuroraApp() {}

    void AuroraApp::run() {
        while (!auroraWindow.shouldClose()) {
            glfwPollEvents();

            if (auto commandBuffer = auroraRenderer.beginFrame()) {
                auroraRenderer.beginSwapChainRenderPass(commandBuffer);

                for (const auto& renderSystem : renderSystems) {
                    renderSystem->renderComponents(auroraRenderer.getCurrentCommandBuffer());
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
        // Create triangle render system
        auto triangleRenderSystem = std::make_unique<AuroraRenderSystem>(
            auroraDevice,
            auroraRenderer.getSwapChainRenderPass(),
            "aurora_app/shaders/shader.vert.spv",
            "aurora_app/shaders/shader.frag.spv",
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
        );

        std::vector<glm::vec3> colors{
            {1.f, .7f, .73f},
            {1.f, .87f, .73f},
            {1.f, 1.f, .73f},
            {.73f, 1.f, .8f},
            {.73, .88f, 1.f}
        };

        for (auto& color : colors) {
            color = glm::pow(color, glm::vec3{2.2f});
        }

        // Add triangle components to the render system
        for (int i = 0; i < 40; i++) {
            auto triangle = std::make_unique<AuroraTriangleComponent>(auroraDevice);
            triangle->transform.scale = glm::vec2(.5f) + i * 0.025f;
            triangle->transform.rotation = i * glm::pi<float>() * .025f;
            triangle->color = colors[i % colors.size()];
            triangleRenderSystem->addComponent(std::move(triangle));
        }
        
        spdlog::info("Created {} components", triangleRenderSystem->getComponentCount());
        renderSystems.push_back(std::move(triangleRenderSystem));
    }
}