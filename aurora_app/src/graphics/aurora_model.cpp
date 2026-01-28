#include "aurora_app/graphics/aurora_model.hpp"

#include <cassert>
#include <spdlog/spdlog.h>
#include <cassert>
#include <spdlog/spdlog.h>
#include <cstring>

namespace aurora {
    AuroraModel::AuroraModel(AuroraDevice& device, const AuroraModel::Builder &builder) 
        : auroraDevice{device}, isDynamicModel{builder.isDynamic} {
        createVertexBuffers(builder.vertices);
        
        if (builder.sharedIndexAllocation) {
            indexAllocation = *builder.sharedIndexAllocation;
            indexCount = static_cast<uint32_t>(builder.indices.size());
            hasIndexBuffer = true;
            ownsIndexBuffer = false;
        } else {
            createIndexBuffers(builder.indices);
        }
    }

    AuroraModel::~AuroraModel() {
        if (vertexAllocation.isValid()) {
            if (isDynamicModel) {
                auroraDevice.getDynamicVertexBufferPool().free(vertexAllocation);
            } else {
                auroraDevice.getVertexBufferPool().free(vertexAllocation);
            }
        }
        if (hasIndexBuffer && indexAllocation.isValid() && ownsIndexBuffer) {
            if (isDynamicModel) {
                 auroraDevice.getDynamicIndexBufferPool().free(indexAllocation);
            } else {
                 auroraDevice.getIndexBufferPool().free(indexAllocation);
            }
        }
    }

    void AuroraModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

        if (isDynamicModel) {
            vertexAllocation = auroraDevice.getDynamicVertexBufferPool().allocate(bufferSize);
            if (!vertexAllocation.isValid()) {
                spdlog::error("Failed to allocate dynamic vertex buffer from pool!");
                return;
            }
            if (vertexAllocation.mappedMemory) {
                memcpy(vertexAllocation.mappedMemory, vertices.data(), (size_t)bufferSize);
            } else {
                 spdlog::error("Dynamic buffer allocation has no mapped memory!");
            }
        } else {
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
    }

    void AuroraModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if (!hasIndexBuffer) {
            return;
        }
        
        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

        if (isDynamicModel) {
            indexAllocation = auroraDevice.getDynamicIndexBufferPool().allocate(bufferSize);
            if (!indexAllocation.isValid()) {
                spdlog::error("Failed to allocate dynamic index buffer from pool!");
                return;
            }
             if (indexAllocation.mappedMemory) {
                memcpy(indexAllocation.mappedMemory, indices.data(), (size_t)bufferSize);
            } else {
                 spdlog::error("Dynamic index allocation has no mapped memory!");
            }
        } else {
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
    }

    void AuroraModel::draw(VkCommandBuffer commandBuffer, uint32_t instanceCount) {
        if (hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
        } else {
            vkCmdDraw(commandBuffer, vertexCount, instanceCount, 0, 0);
        }
    }

    void AuroraModel::updateVertexData(const void* data, VkDeviceSize size, VkDeviceSize offset) {
        if (!isDynamicModel) {
            spdlog::error("Cannot update static vertex buffer directly!");
            return;
        }
        
        if (offset + size > vertexAllocation.size) {
            spdlog::error("Update size out of bounds for vertex buffer!");
            return;
        }

        if (vertexAllocation.mappedMemory) {
            memcpy(static_cast<char*>(vertexAllocation.mappedMemory) + offset, data, (size_t)size);
        }
    }

    void AuroraModel::updateIndexData(const void* data, VkDeviceSize size, VkDeviceSize offset) {
        if (!isDynamicModel) {
            spdlog::error("Cannot update static index buffer directly!");
            return;
        }

        if (offset + size > indexAllocation.size) {
             spdlog::error("Update size out of bounds for index buffer!");
             return;
        }

        if (indexAllocation.mappedMemory) {
            memcpy(static_cast<char*>(indexAllocation.mappedMemory) + offset, data, (size_t)size);
        }
    }

    void AuroraModel::resizeVertexBuffer(VkDeviceSize newSize) {
        if (!isDynamicModel) {
            spdlog::error("Cannot resize static vertex buffer!");
            return;
        }

        if (newSize <= vertexAllocation.size) {
            return;
        }

        auto newAlloc = auroraDevice.getDynamicVertexBufferPool().allocate(newSize);
        if (!newAlloc.isValid()) {
             spdlog::error("Failed to resize dynamic vertex buffer!");
             return;
        }

        if (vertexAllocation.mappedMemory && newAlloc.mappedMemory) {
            memcpy(newAlloc.mappedMemory, vertexAllocation.mappedMemory, (size_t)vertexAllocation.size);
        }

        auroraDevice.getDynamicVertexBufferPool().free(vertexAllocation);
        vertexAllocation = newAlloc;
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