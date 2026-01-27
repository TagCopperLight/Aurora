#pragma once

#include "aurora_engine/core/aurora_device.hpp"
#include "aurora_engine/core/aurora_buffer.hpp"

#include <vector>
#include <memory>
#include <mutex>
#include <list>

namespace aurora {

    struct BufferAllocation {
        VkBuffer buffer;
        VkDeviceSize offset;
        VkDeviceSize size;
        void* mappedMemory;
        
        uint32_t pageId;
        VkDeviceSize pageOffset;

        bool isValid() const { return buffer != VK_NULL_HANDLE; }
    };

    class AuroraBufferPool {
    public:
        AuroraBufferPool(
            AuroraDevice& device,
            VkDeviceSize pageSize = 64 * 1024 * 1024,
            VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        
        ~AuroraBufferPool();

        BufferAllocation allocate(VkDeviceSize size, VkDeviceSize alignment = 256);
        void free(const BufferAllocation& allocation);

    private:
        struct Page {
            std::unique_ptr<AuroraBuffer> buffer;
            VkDeviceSize size;
            VkDeviceSize allocatedSize;
            
            struct FreeBlock {
                VkDeviceSize offset;
                VkDeviceSize size;
            };
            std::list<FreeBlock> freeList;

            bool allocate(VkDeviceSize size, VkDeviceSize alignment, VkDeviceSize& outOffset);
            void free(VkDeviceSize offset, VkDeviceSize size);
        };

        AuroraDevice& device;
        VkDeviceSize pageSize;
        VkBufferUsageFlags usage;
        VkMemoryPropertyFlags memoryProperties;
        
        std::vector<std::unique_ptr<Page>> pages;
        std::mutex poolMutex;

        void createNewPage();
    };

}
