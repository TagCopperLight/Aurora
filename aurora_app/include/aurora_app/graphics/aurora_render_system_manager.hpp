#pragma once

#include "aurora_render_system.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_renderer.hpp"
#include "aurora_engine/core/aurora_camera.hpp"
#include "aurora_engine/core/aurora_descriptors.hpp"
// #include "aurora_app/components/aurora_component_interface.hpp"
#include "aurora_app/graphics/aurora_msdf_atlas.hpp"

#include <memory>
#include <vector>

namespace aurora {
    class AuroraComponentInterface;
    class AuroraRenderSystemManager {
        public:
            AuroraRenderSystemManager(AuroraDevice& device, AuroraRenderer& renderer);
            ~AuroraRenderSystemManager() = default;

            AuroraRenderSystemManager(const AuroraRenderSystemManager&) = delete;
            AuroraRenderSystemManager &operator=(const AuroraRenderSystemManager&) = delete;

            void renderAllComponents(VkCommandBuffer commandBuffer, const AuroraCamera& camera);

            size_t getRenderSystemCount() const {
                return renderSystems.size();
            }

            size_t getTotalComponentCount() const;
            
            const AuroraMSDFAtlas& getMSDFAtlas() const { return *msdfAtlas; }

            void addComponentToQueue(std::shared_ptr<AuroraComponentInterface> component) {
                componentQueue.push_back(component);
                components.push_back(component);
            }

        private:
            AuroraRenderSystem* findCompatibleRenderSystem(const AuroraComponentInterface& component);

            std::unique_ptr<AuroraRenderSystem> createRenderSystem(const AuroraComponentInterface& component);
            
            void addComponentToRenderSystems(std::shared_ptr<AuroraComponentInterface> component);

            void recalculateAllDepths(std::vector<std::shared_ptr<AuroraComponentInterface>>& components, float depth, float depthIncrement);

            AuroraDevice& auroraDevice;
            AuroraRenderer& auroraRenderer;
            std::vector<std::unique_ptr<AuroraRenderSystem>> renderSystems;
            
            std::unique_ptr<AuroraDescriptorPool> globalDescriptorPool;
            std::unique_ptr<AuroraMSDFAtlas> msdfAtlas;

            std::vector<std::shared_ptr<AuroraComponentInterface>> components;
            std::vector<std::shared_ptr<AuroraComponentInterface>> componentQueue;

            static constexpr float MAX_DEPTH = 0.9999f;
            static constexpr float DEPTH_INCREMENT = 0.0001f;
    };
}
