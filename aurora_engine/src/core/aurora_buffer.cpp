#include "aurora_engine/core/aurora_buffer.hpp"

#include <cassert>
#include <cstring>

namespace aurora {
    VkDeviceSize AuroraBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
        if (minOffsetAlignment > 0) {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
    }

    AuroraBuffer::AuroraBuffer(AuroraDevice &device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment)
    : auroraDevice{device}, instanceCount{instanceCount}, instanceSize{instanceSize}, usageFlags{usageFlags}, memoryPropertyFlags{memoryPropertyFlags} {
        alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
        bufferSize = alignmentSize * instanceCount;
        device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
    }

    AuroraBuffer::~AuroraBuffer() {
        unmap();
        vkDestroyBuffer(auroraDevice.device(), buffer, nullptr);
        vkFreeMemory(auroraDevice.device(), memory, nullptr);
    }

    VkResult AuroraBuffer::map(VkDeviceSize size, VkDeviceSize offset) {
        assert(buffer && memory && "Called map on buffer before create");
        return vkMapMemory(auroraDevice.device(), memory, offset, size, 0, &mapped);
    }

    void AuroraBuffer::unmap() {
        if (mapped) {
            vkUnmapMemory(auroraDevice.device(), memory);
            mapped = nullptr;
        }
    }

    void AuroraBuffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
        assert(mapped && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE) {
            memcpy(mapped, data, bufferSize);
        } else {
            char *memOffset = (char *)mapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    VkResult AuroraBuffer::flush(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(auroraDevice.device(), 1, &mappedRange);
    }

    VkResult AuroraBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;

        return vkInvalidateMappedMemoryRanges(auroraDevice.device(), 1, &mappedRange);
    }

    VkDescriptorBufferInfo AuroraBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        return VkDescriptorBufferInfo{buffer, offset, size};
    }

    void AuroraBuffer::writeToIndex(void *data, int index) {
        writeToBuffer(data, instanceSize, index * alignmentSize);
    }

    VkResult AuroraBuffer::flushIndex(int index) { return flush(alignmentSize, index * alignmentSize); }

    VkDescriptorBufferInfo AuroraBuffer::descriptorInfoForIndex(int index) {
        return descriptorInfo(alignmentSize, index * alignmentSize);
    }

    VkResult AuroraBuffer::invalidateIndex(int index) {
        return invalidate(alignmentSize, index * alignmentSize);
    }
}
