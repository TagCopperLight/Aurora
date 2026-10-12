#pragma once

#include "aurora_render_system.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_renderer.hpp"
#include "aurora_engine/core/aurora_camera.hpp"
#include "aurora_engine/core/aurora_descriptors.hpp"
#include "aurora_ui/graphics/aurora_msdf_atlas.hpp"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
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
            
            AuroraMSDFAtlas& getMSDFAtlas() { return *msdfAtlas; }
            const AuroraMSDFAtlas& getMSDFAtlas() const { return *msdfAtlas; }

            void addComponentToQueue(std::shared_ptr<AuroraComponentInterface> component) {
                componentQueue.push_back(component);
                components.push_back(component);
            }

            void removeComponent(std::shared_ptr<AuroraComponentInterface> component);

            // Returns a model shared by every component asking for the same key, so identical
            // geometry is drawn with a single instanced draw call. The vertices must therefore not
            // depend on per-component state other than what is encoded in the key.
            std::shared_ptr<AuroraModel> getSharedModel(const std::string& key, const std::function<std::vector<AuroraModel::Vertex>()>& createVertices);

        private:
            AuroraRenderSystem* findCompatibleRenderSystem(const AuroraComponentInterface& component);

            std::unique_ptr<AuroraRenderSystem> createRenderSystem(const AuroraComponentInterface& component);
            
            void addComponentToRenderSystems(std::shared_ptr<AuroraComponentInterface> component);

            void recalculateAllDepths(std::vector<std::shared_ptr<AuroraComponentInterface>>& components, float& depth, float depthIncrement);

            AuroraDevice& auroraDevice;
            AuroraRenderer& auroraRenderer;
            std::vector<std::unique_ptr<AuroraRenderSystem>> renderSystems;
            
            std::unique_ptr<AuroraDescriptorPool> globalDescriptorPool;
            std::unique_ptr<AuroraMSDFAtlas> msdfAtlas;

            std::vector<std::shared_ptr<AuroraComponentInterface>> components;
            std::vector<std::shared_ptr<AuroraComponentInterface>> componentQueue;

            std::unordered_map<std::string, std::weak_ptr<AuroraModel>> sharedModels;
            size_t sharedModelsPruneSize = 64;

            static constexpr float MAX_DEPTH = 0.9999f;
            static constexpr float DEPTH_INCREMENT = 0.0001f;
    };
}
