#pragma once

#include "aurora_render_system.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_renderer.hpp"
#include "aurora_engine/core/aurora_camera.hpp"
#include "aurora_engine/core/aurora_descriptors.hpp"
#include "aurora_app/components/aurora_component_interface.hpp"
#include "aurora_app/graphics/aurora_msdf_atlas.hpp"

#include <memory>
#include <vector>

namespace aurora {
    class AuroraRenderSystemManager {
        public:
            AuroraRenderSystemManager(AuroraDevice& device, AuroraRenderer& renderer);
            ~AuroraRenderSystemManager() = default;

            AuroraRenderSystemManager(const AuroraRenderSystemManager&) = delete;
            AuroraRenderSystemManager &operator=(const AuroraRenderSystemManager&) = delete;

            void addComponent(std::shared_ptr<AuroraComponentInterface> component);
            
            void renderAllComponents(VkCommandBuffer commandBuffer, const AuroraCamera& camera);

            size_t getRenderSystemCount() const {
                return renderSystems.size();
            }

            size_t getTotalComponentCount() const;
            
            const AuroraMSDFAtlas& getMSDFAtlas() const { return *msdfAtlas; }

        private:
            AuroraRenderSystem* findCompatibleRenderSystem(const AuroraComponentInterface& component);

            std::unique_ptr<AuroraRenderSystem> createRenderSystem(const AuroraComponentInterface& component);
            
            void addComponentRecursive(std::shared_ptr<AuroraComponentInterface> component);

            AuroraDevice& auroraDevice;
            AuroraRenderer& auroraRenderer;
            std::vector<std::unique_ptr<AuroraRenderSystem>> renderSystems;
            
            std::unique_ptr<AuroraDescriptorPool> globalDescriptorPool;
            std::unique_ptr<AuroraMSDFAtlas> msdfAtlas;

            float currentDepth = 0.9999f;
            static constexpr float DEPTH_INCREMENT = 0.0001f;
    };
}
