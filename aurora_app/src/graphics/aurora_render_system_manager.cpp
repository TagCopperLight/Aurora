#include "aurora_app/graphics/aurora_render_system_manager.hpp"
#include "aurora_app/components/aurora_component_interface.hpp"
#include <spdlog/spdlog.h>

#include <memory>

namespace aurora {
    AuroraRenderSystemManager::AuroraRenderSystemManager(AuroraDevice& device, AuroraRenderer& renderer) 
    : auroraDevice{device}, auroraRenderer{renderer} {
        globalDescriptorPool = AuroraDescriptorPool::Builder(auroraDevice)
            .setMaxSets(100)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
            .build();

        msdfAtlas = std::make_unique<AuroraMSDFAtlas>(auroraDevice, "/home/tag/Downloads/font.ttf");
        spdlog::info("RenderSystemManager initialized");
    }

    void AuroraRenderSystemManager::addComponentToRenderSystems(std::shared_ptr<AuroraComponentInterface> component) {
        AuroraRenderSystem* compatibleSystem = findCompatibleRenderSystem(*component);
        
        if (compatibleSystem) {
            compatibleSystem->addComponent(component);
        } else {
            auto newRenderSystem = createRenderSystem(*component);
            newRenderSystem->addComponent(component);
            renderSystems.push_back(std::move(newRenderSystem));
        }
    }

    void AuroraRenderSystemManager::recalculateAllDepths(std::vector<std::shared_ptr<AuroraComponentInterface>>& components, float depth, float depthIncrement) {
        for (const auto& component : components) {
            component->setDepth(depth);
            depth -= depthIncrement;

            auto& children = component->getChildren();
            recalculateAllDepths(children, depth, depthIncrement);
        }
    }

    void AuroraRenderSystemManager::renderAllComponents(VkCommandBuffer commandBuffer, const AuroraCamera& camera) {
        if (!componentQueue.empty()) {
            spdlog::info("Processing component queue with {} components", componentQueue.size());
            for (const auto& component : componentQueue) {
                addComponentToRenderSystems(component);
            }
            componentQueue.clear();

            recalculateAllDepths(components, MAX_DEPTH, DEPTH_INCREMENT);
            spdlog::info("Created {} render systems with {} total components", renderSystems.size(), getTotalComponentCount());
        }

        for (const auto& renderSystem : renderSystems) {
            if (renderSystem->getComponentCount() > 0) {
                renderSystem->renderComponents(commandBuffer, camera, auroraRenderer.getFrameIndex());
            }
        }
    }

    size_t AuroraRenderSystemManager::getTotalComponentCount() const {
        size_t totalCount = 0;
        for (const auto& renderSystem : renderSystems) {
            totalCount += renderSystem->getComponentCount();
        }
        return totalCount;
    }

    AuroraRenderSystem* AuroraRenderSystemManager::findCompatibleRenderSystem(const AuroraComponentInterface& component) {
        for (const auto& renderSystem : renderSystems) {
            if (renderSystem->isCompatibleWith(component)) {
                return renderSystem.get();
            }
        }
        return nullptr;
    }

    std::unique_ptr<AuroraRenderSystem> AuroraRenderSystemManager::createRenderSystem(const AuroraComponentInterface& component) {
        RenderSystemCreateInfo createInfo{
            auroraRenderer.getSwapChainRenderPass(),
            component.getVertexShaderPath(),
            component.getFragmentShaderPath(),
            component.getTopology(),
            globalDescriptorPool.get(),
            msdfAtlas.get()
        };

        return std::make_unique<AuroraRenderSystem>(auroraDevice, createInfo);
    }
}
