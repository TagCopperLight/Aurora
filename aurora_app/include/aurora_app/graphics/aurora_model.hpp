#pragma once

#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>

namespace aurora {
    class AuroraModel {
        public:
            struct Vertex {
                glm::vec3 position;
                glm::vec4 color;
                glm::vec2 texCoord;

                // Default constructor
                Vertex() = default;

                // Constructor for vec3 position and vec4 color
                Vertex(const glm::vec3& pos, const glm::vec4& col) : position(pos), color(col), texCoord(0.0f, 0.0f) {}

                // Constructor for vec2 position (z=0) and vec4 color
                Vertex(const glm::vec2& pos, const glm::vec4& col) : position(pos.x, pos.y, 0.0f), color(col), texCoord(0.0f, 0.0f) {}

                static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
                static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            };

            struct Builder {
                std::vector<Vertex> vertices{};
                std::vector<uint32_t> indices{};
            };

            AuroraModel(AuroraDevice &device, const AuroraModel::Builder &builder);
            ~AuroraModel();

            AuroraModel(const AuroraModel &) = delete;
            AuroraModel &operator=(const AuroraModel &) = delete;

            void bind(VkCommandBuffer commandBuffer);
            void draw(VkCommandBuffer commandBuffer);

        private:
            void createVertexBuffers(const std::vector<Vertex> &vertices);
            void createIndexBuffers(const std::vector<uint32_t> &indices);

            AuroraDevice &auroraDevice;

            std::unique_ptr<AuroraBuffer> vertexBuffer;
            uint32_t vertexCount;

            bool hasIndexBuffer = false;
            std::unique_ptr<AuroraBuffer> indexBuffer;
            uint32_t indexCount;
    };
}