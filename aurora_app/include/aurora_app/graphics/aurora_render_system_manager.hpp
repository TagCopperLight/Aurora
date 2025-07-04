#pragma once

#include "aurora_render_system.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_camera.hpp"
#include "aurora_engine/core/aurora_descriptors.hpp"
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

            void addComponent(std::unique_ptr<AuroraComponentInterface> component);

            void renderAllComponents(VkCommandBuffer commandBuffer, const AuroraCamera& camera);

            size_t getRenderSystemCount() const {
                return renderSystems.size();
            }

            size_t getTotalComponentCount() const;

        private:
            AuroraRenderSystem* findCompatibleRenderSystem(const AuroraComponentInterface& component);

            std::unique_ptr<AuroraRenderSystem> createRenderSystem(const AuroraComponentInterface& component);

            AuroraDevice& auroraDevice;
            VkRenderPass renderPass;
            std::vector<std::unique_ptr<AuroraRenderSystem>> renderSystems;
            
            std::unique_ptr<AuroraDescriptorPool> globalDescriptorPool;
    };
}
