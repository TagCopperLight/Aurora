#include "aurora_engine/core/aurora_buffer_pool.hpp"
#include "aurora_engine/utils/log.hpp"

namespace aurora {

    AuroraBufferPool::AuroraBufferPool(
        AuroraDevice& device,
        VkDeviceSize pageSize,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags memoryProperties)
        : device{device}, pageSize{pageSize}, usage{usage}, memoryProperties{memoryProperties} {
    }

    AuroraBufferPool::~AuroraBufferPool() {
        pages.clear();
    }

    BufferAllocation AuroraBufferPool::allocate(VkDeviceSize size, VkDeviceSize alignment) {
        std::lock_guard<std::mutex> lock(poolMutex);

        for (size_t i = 0; i < pages.size(); ++i) {
            VkDeviceSize offset;
            if (pages[i]->allocate(size, alignment, offset)) {
                BufferAllocation alloc{};
                alloc.buffer = pages[i]->buffer->getBuffer();
                alloc.offset = offset;
                alloc.size = size;
                alloc.pageId = static_cast<uint32_t>(i);
                alloc.pageOffset = offset;
                
                if (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                    alloc.mappedMemory = static_cast<char*>(pages[i]->buffer->getMappedMemory()) + offset;
                } else {
                    alloc.mappedMemory = nullptr;
                }

                return alloc;
            }
        }

        createNewPage();
        
        VkDeviceSize offset;
        auto& newPage = pages.back();
        if (newPage->allocate(size, alignment, offset)) {
            BufferAllocation alloc{};
            alloc.buffer = newPage->buffer->getBuffer();
            alloc.offset = offset;
            alloc.size = size;
            alloc.pageId = static_cast<uint32_t>(pages.size() - 1);
            alloc.pageOffset = offset;

            if (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                 alloc.mappedMemory = static_cast<char*>(newPage->buffer->getMappedMemory()) + offset;
            } else {
                 alloc.mappedMemory = nullptr;
            }
            
            return alloc;
        }

        log::engine()->error("Failed to allocate buffer of size {} even after creating new page. Page size: {}", size, pageSize);
        return {}; 
    }

    void AuroraBufferPool::free(const BufferAllocation& allocation) {
         std::lock_guard<std::mutex> lock(poolMutex);
         
         if (allocation.pageId < pages.size()) {
             pages[allocation.pageId]->free(allocation.pageOffset, allocation.size);
         }
    }

    void AuroraBufferPool::createNewPage() {
        auto page = std::make_unique<Page>();
        
        page->buffer = std::make_unique<AuroraBuffer>(
            device,
            pageSize,
            1,
            usage,
            memoryProperties
        );
        
        if (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            page->buffer->map();
        }

        page->size = pageSize;
        page->allocatedSize = 0;
        
        page->freeMap.insert({0, pageSize});
        
        pages.push_back(std::move(page));
        log::engine()->debug("Allocated new Buffer Pool Page (ID: {})", pages.size() - 1);
    }

    bool AuroraBufferPool::Page::allocate(VkDeviceSize size, VkDeviceSize alignment, VkDeviceSize& outOffset) {
        for (auto it = freeMap.begin(); it != freeMap.end(); ++it) {
            VkDeviceSize currentOffset = it->first;
            VkDeviceSize currentSize = it->second;

            VkDeviceSize alignedOffset = (currentOffset + alignment - 1) & ~(alignment - 1);
            VkDeviceSize padding = alignedOffset - currentOffset;

            if (currentSize >= size + padding) {
                outOffset = alignedOffset;
                VkDeviceSize remainingSize = currentSize - (size + padding);

                freeMap.erase(it);

                if (padding > 0) {
                    freeMap[currentOffset] = padding;
                }

                if (remainingSize > 0) {
                    freeMap[outOffset + size] = remainingSize;
                }
                
                allocatedSize += size;
                return true;
            }
        }
        return false;
    }

    void AuroraBufferPool::Page::free(VkDeviceSize offset, VkDeviceSize size) {
        auto next = freeMap.lower_bound(offset);
        auto prev = (next == freeMap.begin()) ? freeMap.end() : std::prev(next);

        bool mergedWithPrev = false;
        if (prev != freeMap.end() && (prev->first + prev->second) == offset) {
            prev->second += size;
            mergedWithPrev = true;
        }
        if (next != freeMap.end() && (offset + size) == next->first) {
            if (mergedWithPrev) {
                prev->second += next->second;
                freeMap.erase(next);
            } else {
                VkDeviceSize nextSize = next->second;
                freeMap.erase(next);
                freeMap[offset] = size + nextSize;
            }
        } else {
            if (!mergedWithPrev) {
                freeMap[offset] = size;
            }
        }
        
        allocatedSize -= size;
    }

}
