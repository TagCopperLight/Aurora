#include "aurora_app/graphics/aurora_model.hpp"

#include <cassert>
#include <spdlog/spdlog.h>
#include <memory>

namespace aurora {
    AuroraModel::AuroraModel(AuroraDevice& device, const AuroraModel::Builder &builder) : auroraDevice{device} {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }

    AuroraModel::~AuroraModel() {}

    void AuroraModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        AuroraBuffer stagingBuffer{
            auroraDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)vertices.data());

        vertexBuffer = std::make_unique<AuroraBuffer>(
            auroraDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        auroraDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }

    void AuroraModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if (!hasIndexBuffer) {
            return;
        }
        
        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        AuroraBuffer stagingBuffer{
            auroraDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)indices.data());

        indexBuffer = std::make_unique<AuroraBuffer>(
            auroraDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        auroraDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }

    void AuroraModel::draw(VkCommandBuffer commandBuffer, uint32_t instanceCount) {
        if (hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
        } else {
            vkCmdDraw(commandBuffer, vertexCount, instanceCount, 0, 0);
        }
    }

    void AuroraModel::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    std::vector<VkVertexInputBindingDescription> AuroraModel::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(2);

        // Vertex Data
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        // Instance Data
        bindingDescriptions[1].binding = 1;
        bindingDescriptions[1].stride = sizeof(InstanceData);
        bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> AuroraModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord)});
        
        attributeDescriptions.push_back({3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(InstanceData, modelMatrix)});
        attributeDescriptions.push_back({4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(InstanceData, modelMatrix) + sizeof(glm::vec4)});
        attributeDescriptions.push_back({5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(InstanceData, modelMatrix) + 2 * sizeof(glm::vec4)});
        attributeDescriptions.push_back({6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(InstanceData, modelMatrix) + 3 * sizeof(glm::vec4)});

        attributeDescriptions.push_back({7, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(InstanceData, color)});

        return attributeDescriptions;
    }
}