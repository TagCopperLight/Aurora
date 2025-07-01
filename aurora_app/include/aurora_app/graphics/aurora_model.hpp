#pragma once

#include "aurora_engine/core/aurora_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

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

            AuroraModel(AuroraDevice &device, const std::vector<Vertex> &vertices);
            ~AuroraModel();

            AuroraModel(const AuroraModel &) = delete;
            AuroraModel &operator=(const AuroraModel &) = delete;

            void bind(VkCommandBuffer commandBuffer);
            void draw(VkCommandBuffer commandBuffer);

        private:
            void createVertexBuffers(const std::vector<Vertex> &vertices);

            AuroraDevice &auroraDevice;
            VkBuffer vertexBuffer;
            VkDeviceMemory vertexBufferMemory;
            uint32_t vertexCount;
    };
}