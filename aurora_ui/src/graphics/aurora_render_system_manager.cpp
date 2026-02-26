#include "aurora_ui/graphics/aurora_render_system_manager.hpp"
#include "aurora_ui/components/aurora_component_interface.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"
#include "aurora_engine/utils/log.hpp"

#include <memory>

namespace aurora {
    AuroraRenderSystemManager::AuroraRenderSystemManager(AuroraDevice& device, AuroraRenderer& renderer) 
    : auroraDevice{device}, auroraRenderer{renderer} {
        globalDescriptorPool = AuroraDescriptorPool::Builder(auroraDevice)
            .setMaxSets(100)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
            .build();

        msdfAtlas = std::make_unique<AuroraMSDFAtlas>(auroraDevice, AuroraThemeSettings::FONT_PATH);
        log::ui()->info("RenderSystemManager initialized");
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

    void AuroraRenderSystemManager::removeComponent(std::shared_ptr<AuroraComponentInterface> component) {
        auto it = std::find(components.begin(), components.end(), component);
        if (it != components.end()) {
            components.erase(it);
        }
        auto itQueue = std::find(componentQueue.begin(), componentQueue.end(), component);
        if (itQueue != componentQueue.end()) {
            componentQueue.erase(itQueue);
        }
        for (auto& sys : renderSystems) {
            sys->removeComponent(component);
        }
    }

    void AuroraRenderSystemManager::recalculateAllDepths(std::vector<std::shared_ptr<AuroraComponentInterface>>& components, float& depth, float depthIncrement) {
        for (const auto& component : components) {
            component->setDepth(depth);
            depth -= depthIncrement;

            auto& children = component->getChildren();
            if (!children.empty()) {
                recalculateAllDepths(children, depth, depthIncrement);
            }
        }
    }

    void AuroraRenderSystemManager::renderAllComponents(VkCommandBuffer commandBuffer, const AuroraCamera& camera) {
        if (!componentQueue.empty()) {
            log::ui()->debug("Processing component queue with {} components", componentQueue.size());
            for (const auto& component : componentQueue) {
                addComponentToRenderSystems(component);
            }
            componentQueue.clear();

            float depth = MAX_DEPTH;
            recalculateAllDepths(components, depth, DEPTH_INCREMENT);
            log::ui()->debug("Created {} render systems with {} total components", renderSystems.size(), getTotalComponentCount());
        }

        std::vector<AuroraRenderSystem*> opaqueSystems;
        std::vector<AuroraRenderSystem*> transparentSystems;

        for (const auto& renderSystem : renderSystems) {
            if (renderSystem->getComponentCount() > 0) {
                if (renderSystem->isTransparent()) {
                    transparentSystems.push_back(renderSystem.get());
                } else {
                    opaqueSystems.push_back(renderSystem.get());
                }
            }
        }

        for (auto renderSystem : opaqueSystems) {
            renderSystem->renderComponents(commandBuffer, camera, auroraRenderer.getFrameIndex());
        }

        for (auto renderSystem : transparentSystems) {
            renderSystem->renderComponents(commandBuffer, camera, auroraRenderer.getFrameIndex());
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
            msdfAtlas.get(),
            component.needsTextureBinding(),
            component.isTransparent()
        };

        return std::make_unique<AuroraRenderSystem>(auroraDevice, createInfo);
    }
}
