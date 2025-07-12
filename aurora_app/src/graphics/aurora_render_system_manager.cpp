#include "aurora_app/graphics/aurora_render_system_manager.hpp"
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

    void AuroraRenderSystemManager::addComponent(std::unique_ptr<AuroraComponentInterface> component) {
        if (!component) {
            spdlog::warn("Attempted to add null component to RenderSystemManager");
            return;
        }

        addComponentRecursive(std::move(component));
    }

    void AuroraRenderSystemManager::addComponentRecursive(std::unique_ptr<AuroraComponentInterface> component) {
        component->setDepth(currentDepth);
        currentDepth -= DEPTH_INCREMENT;

        auto children = std::move(component->getChildren());
        for (auto& child : children) {
            glm::mat4 childWorldTransform = child->getWorldTransform();
            
            child->transform.translation.x = childWorldTransform[3][0];
            child->transform.translation.y = childWorldTransform[3][1];
            child->transform.translation.z = childWorldTransform[3][2];
            
            child->clearParent();
            
            addComponentRecursive(std::move(child));
        }

        AuroraRenderSystem* compatibleSystem = findCompatibleRenderSystem(*component);
        
        if (compatibleSystem) {
            compatibleSystem->addComponent(std::move(component));
        } else {
            auto newRenderSystem = createRenderSystem(*component);
            newRenderSystem->addComponent(std::move(component));
            renderSystems.push_back(std::move(newRenderSystem));
        }
    }

    void AuroraRenderSystemManager::renderAllComponents(VkCommandBuffer commandBuffer, const AuroraCamera& camera) {
        for (const auto& renderSystem : renderSystems) {
            if (renderSystem->getComponentCount() > 0) {
                renderSystem->renderComponents(commandBuffer, camera);
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
            auroraRenderer.getSwapChainRenderPass(), // Get current render pass
            component.getVertexShaderPath(),
            component.getFragmentShaderPath(),
            component.getTopology(),
            globalDescriptorPool.get(),
            msdfAtlas.get()
        };

        return std::make_unique<AuroraRenderSystem>(auroraDevice, createInfo);
    }
}
