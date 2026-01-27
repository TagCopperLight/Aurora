#include "aurora_engine/core/aurora_buffer_pool.hpp"
#include <spdlog/spdlog.h>

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

        spdlog::error("Failed to allocate buffer of size {} even after creating new page. Page size: {}", size, pageSize);
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
        
        page->freeList.push_back({0, pageSize});
        
        pages.push_back(std::move(page));
        spdlog::info("Allocated new Buffer Pool Page (ID: {})", pages.size() - 1);
    }

    bool AuroraBufferPool::Page::allocate(VkDeviceSize size, VkDeviceSize alignment, VkDeviceSize& outOffset) {
        for (auto it = freeList.begin(); it != freeList.end(); ++it) {
            VkDeviceSize alignedOffset = (it->offset + alignment - 1) & ~(alignment - 1);
            VkDeviceSize padding = alignedOffset - it->offset;
            
            if (it->size >= size + padding) {
                outOffset = alignedOffset;
                
                VkDeviceSize remainingSize = it->size - (size + padding);
                
                if (padding > 0) {
                     it->size = padding;
                     
                     if (remainingSize > 0) {
                         freeList.insert(std::next(it), {outOffset + size, remainingSize});
                     }
                } else {
                    if (remainingSize > 0) {
                        it->offset += size;
                        it->size = remainingSize;
                    } else {
                        freeList.erase(it);
                    }
                }
                
                allocatedSize += size;
                return true;
            }
        }
        return false;
    }

    void AuroraBufferPool::Page::free(VkDeviceSize offset, VkDeviceSize size) {
        auto it = freeList.begin();
        while (it != freeList.end() && it->offset < offset) {
            ++it;
        }
        
        auto inserted = freeList.insert(it, {offset, size});
        
        auto next = std::next(inserted);
        if (next != freeList.end() && (inserted->offset + inserted->size) == next->offset) {
            inserted->size += next->size;
            freeList.erase(next);
        }
        
        if (inserted != freeList.begin()) {
            auto prev = std::prev(inserted);
            if ((prev->offset + prev->size) == inserted->offset) {
                prev->size += inserted->size;
                freeList.erase(inserted);
            }
        }
        
        allocatedSize -= size;
    }

}
