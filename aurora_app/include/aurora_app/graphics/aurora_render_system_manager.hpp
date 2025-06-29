#pragma once

#include "aurora_render_system.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_camera.hpp"
#include "aurora_app/components/aurora_component_interface.hpp"
#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace aurora {
    class AuroraRenderSystemManager {
        public:
            AuroraRenderSystemManager(AuroraDevice& device, VkRenderPass renderPass);
            ~AuroraRenderSystemManager() = default;

            AuroraRenderSystemManager(const AuroraRenderSystemManager&) = delete;
            AuroraRenderSystemManager &operator=(const AuroraRenderSystemManager&) = delete;

            // Add a component to the appropriate render system (creates new system if needed)
            void addComponent(std::unique_ptr<AuroraComponentInterface> component);

            // Render all components in all render systems
            void renderAllComponents(VkCommandBuffer commandBuffer, const AuroraCamera& camera);

            // Get total number of render systems
            size_t getRenderSystemCount() const {
                return renderSystems.size();
            }

            // Get total number of components across all render systems
            size_t getTotalComponentCount() const;

        private:
            // Find a compatible render system for the given component
            AuroraRenderSystem* findCompatibleRenderSystem(const AuroraComponentInterface& component);

            // Create a new render system for the given component
            std::unique_ptr<AuroraRenderSystem> createRenderSystem(const AuroraComponentInterface& component);

            AuroraDevice& auroraDevice;
            VkRenderPass renderPass;
            std::vector<std::unique_ptr<AuroraRenderSystem>> renderSystems;
    };
}
