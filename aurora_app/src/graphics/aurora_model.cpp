#include "aurora_app/graphics/aurora_model.hpp"

#include <cassert>
#include <spdlog/spdlog.h>
#include <cassert>
#include <spdlog/spdlog.h>
#include <cstring>

namespace aurora {
    AuroraModel::AuroraModel(AuroraDevice& device, const AuroraModel::Builder &builder) : auroraDevice{device} {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }

    AuroraModel::~AuroraModel() {
        if (vertexAllocation.isValid()) {
            auroraDevice.getVertexBufferPool().free(vertexAllocation);
        }
        if (hasIndexBuffer && indexAllocation.isValid()) {
            auroraDevice.getIndexBufferPool().free(indexAllocation);
        }
    }

    void AuroraModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

        vertexAllocation = auroraDevice.getVertexBufferPool().allocate(bufferSize);
        if (!vertexAllocation.isValid()) {
             spdlog::error("Failed to allocate vertex buffer from pool!");
             return;
        }

        auto stagingAllocation = auroraDevice.getStagingBufferPool().allocate(bufferSize);
        if (!stagingAllocation.isValid()) {
             spdlog::error("Failed to allocate staging buffer from pool!");
             return; 
        }

        if (stagingAllocation.mappedMemory) {
             memcpy(stagingAllocation.mappedMemory, vertices.data(), (size_t)bufferSize);
        }

        auroraDevice.copyBuffer(
            stagingAllocation.buffer, 
            vertexAllocation.buffer, 
            bufferSize, 
            stagingAllocation.offset, 
            vertexAllocation.offset
        );

        auroraDevice.getStagingBufferPool().free(stagingAllocation);
    }

    void AuroraModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if (!hasIndexBuffer) {
            return;
        }
        
        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

        indexAllocation = auroraDevice.getIndexBufferPool().allocate(bufferSize);
        if (!indexAllocation.isValid()) {
             spdlog::error("Failed to allocate index buffer from pool!");
             return;
        }

        auto stagingAllocation = auroraDevice.getStagingBufferPool().allocate(bufferSize);
        if (!stagingAllocation.isValid()) {
             spdlog::error("Failed to allocate staging buffer from pool!");
             return; 
        }

        if (stagingAllocation.mappedMemory) {
             memcpy(stagingAllocation.mappedMemory, indices.data(), (size_t)bufferSize);
        }

        auroraDevice.copyBuffer(
            stagingAllocation.buffer, 
            indexAllocation.buffer, 
            bufferSize, 
            stagingAllocation.offset, 
            indexAllocation.offset
        );

        auroraDevice.getStagingBufferPool().free(stagingAllocation);
    }

    void AuroraModel::draw(VkCommandBuffer commandBuffer, uint32_t instanceCount) {
        if (hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
        } else {
            vkCmdDraw(commandBuffer, vertexCount, instanceCount, 0, 0);
        }
    }

    void AuroraModel::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {vertexAllocation.buffer};
        VkDeviceSize offsets[] = {vertexAllocation.offset};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexAllocation.buffer, indexAllocation.offset, VK_INDEX_TYPE_UINT32);
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