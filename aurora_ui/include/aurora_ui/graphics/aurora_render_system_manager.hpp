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

            // Stops drawing the component and its children. They are kept alive until the frames
            // in flight that may still use their GPU resources are done.
            void removeComponent(std::shared_ptr<AuroraComponentInterface> component);

            // Re-sorts components by layer before the next frame
            void markDrawOrderDirty() { drawOrderDirty = true; }

            // Returns a model shared by every component asking for the same key, so identical
            // geometry is drawn with a single instanced draw call. The vertices must therefore not
            // depend on per-component state other than what is encoded in the key.
            std::shared_ptr<AuroraModel> getSharedModel(const std::string& key, const std::function<std::vector<AuroraModel::Vertex>()>& createVertices);

        private:
            AuroraRenderSystem* findCompatibleRenderSystem(const AuroraComponentInterface& component);

            std::unique_ptr<AuroraRenderSystem> createRenderSystem(const AuroraComponentInterface& component);
            
            void addComponentToRenderSystems(std::shared_ptr<AuroraComponentInterface> component);

            void recalculateAllDepths(std::vector<std::shared_ptr<AuroraComponentInterface>>& components, size_t& index, float depthIncrement);

            static size_t countComponentTree(const std::vector<std::shared_ptr<AuroraComponentInterface>>& components);

            AuroraDevice& auroraDevice;
            AuroraRenderer& auroraRenderer;
            std::vector<std::unique_ptr<AuroraRenderSystem>> renderSystems;
            
            std::unique_ptr<AuroraDescriptorPool> globalDescriptorPool;
            std::unique_ptr<AuroraMSDFAtlas> msdfAtlas;

            std::vector<std::shared_ptr<AuroraComponentInterface>> components;
            std::vector<std::shared_ptr<AuroraComponentInterface>> componentQueue;
            bool drawOrderDirty = false;

            struct RetiredComponent {
                uint64_t frame;
                std::shared_ptr<AuroraComponentInterface> component;
            };
            std::vector<RetiredComponent> retiredComponents;
            uint64_t frameCount = 0;

            std::unordered_map<std::string, std::weak_ptr<AuroraModel>> sharedModels;
            size_t sharedModelsPruneSize = 64;

            static constexpr float MAX_DEPTH = 0.9999f;
            // Preferred spacing between consecutive components; shrunk when there are more than MAX_DEPTH / DEPTH_INCREMENT of them.
            // Ordering stays exact as long as the step is coarser than the depth format's resolution (1/65535 for D16).
            static constexpr float DEPTH_INCREMENT = 0.0001f;
    };
}
