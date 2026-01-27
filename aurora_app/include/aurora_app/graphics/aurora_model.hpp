#pragma once

#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_buffer_pool.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace aurora {
    class AuroraModel {
        public:
            struct InstanceData {
                glm::mat4 modelMatrix;
                glm::vec4 color;
            };

            struct Vertex {
                glm::vec3 position;
                glm::vec4 color;
                glm::vec2 texCoord;

                Vertex() = default;

                Vertex(const glm::vec3& pos, const glm::vec4& col) : position(pos), color(col), texCoord(0.0f, 0.0f) {}

                Vertex(const glm::vec2& pos, const glm::vec4& col) : position(pos.x, pos.y, 0.0f), color(col), texCoord(0.0f, 0.0f) {}

                static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
                static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            };

            struct Builder {
                std::vector<Vertex> vertices{};
                std::vector<uint32_t> indices{};
                bool isDynamic = false;
            };

            AuroraModel(AuroraDevice &device, const AuroraModel::Builder &builder);
            ~AuroraModel();

            AuroraModel(const AuroraModel &) = delete;
            AuroraModel &operator=(const AuroraModel &) = delete;

            void bind(VkCommandBuffer commandBuffer);
            void draw(VkCommandBuffer commandBuffer, uint32_t instanceCount = 1);

            void updateVertexData(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
            void updateIndexData(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

            void setVertexCount(uint32_t count) { vertexCount = count; }
            uint32_t getVertexCount() const { return vertexCount; }

            void setIndexCount(uint32_t count) { indexCount = count; }
            uint32_t getIndexCount() const { return indexCount; }

            bool isDynamic() const { return isDynamicModel; }

        private:
            void createVertexBuffers(const std::vector<Vertex> &vertices);
            void createIndexBuffers(const std::vector<uint32_t> &indices);

            AuroraDevice &auroraDevice;

            BufferAllocation vertexAllocation;
            uint32_t vertexCount;

            bool hasIndexBuffer = false;
            BufferAllocation indexAllocation;
            uint32_t indexCount;
            
            bool isDynamicModel = false;
    };
}