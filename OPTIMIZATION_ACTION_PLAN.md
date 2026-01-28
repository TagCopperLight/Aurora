# Aurora Engine Optimization Action Plan

## Overview
This document outlines a comprehensive optimization strategy for the Aurora graphics engine.
**Status Update**: A review of the codebase revealed that some previously proposed optimizations (Push Constants, Profiler) are already implemented. The plan has been updated to focus on high-impact missing features.

## Priority 1: Instanced Rendering System - [DONE]

### Current Problem
The `AuroraRenderSystem` iterates over every component and issues a separate draw call (`vkCmdDraw` or `vkCmdDrawIndexed`) for each one.
```cpp
// Current loop in AuroraRenderSystem::renderComponents
for (size_t i = 0; i < components.size(); i++) {
    // ... push constants ...
    component->model->bind(commandBuffer);
    component->model->draw(commandBuffer);
}
```
If you have 1000 trees using the same model, this results in 1000 draw calls, which is a significant CPU bottleneck (high driver overhead).

### Solution Strategy
Implement **Instanced Rendering** to draw multiple instances of the same model in a single draw call.

#### Phase 1A: Instance Data & Shader Support
**Target Files:**
- `aurora_app/include/aurora_app/graphics/aurora_model.hpp`
- `aurora_app/shaders/*.vert`

**Implementation:**
1.  Define `InstanceData` structure (Model Matrix, Color, Custom Data).
2.  Modify Vertex Shader to accept `InstanceData` as vertex attributes (e.g., locations 5-10) with `rate = VK_VERTEX_INPUT_RATE_INSTANCE`.
3.  Update `AuroraModel` to support binding an extra `VkBuffer` as the instance buffer.

#### Phase 1B: Render System Batching
**Target Files:**
- `aurora_app/src/graphics/aurora_render_system.cpp`

**Implementation:**
1.  Group components by `AuroraModel` pointer.
2.  For each group, collect instance data into a host-mapped buffer.
3.  Bind the model once.
4.  Bind the instance buffer.
5.  Issue `vkCmdDrawIndexed(..., instanceCount, ...)` once.

**Expected Performance Gain:** 10x-100x reduction in draw calls for repetitive scenes.

---

## Priority 2: Buffer Sub-allocation Strategy - [DONE]

### Current Problem
`AuroraModel::createVertexBuffers` creates a separate `VkBuffer` (and `VkDeviceMemory`) for every single model.
```cpp
// aurora_model.cpp
vertexBuffer = std::make_unique<AuroraBuffer>(...);
```
This impacts performance for:
- **Text Rendering**: Every text change creates a new buffer.
- **Many Small Objects**: High memory overhead and fragmentation.

### Solution Strategy
Implement a **Buffer Pool** with sub-allocation.

#### Phase 2A: Buffer Pool Manager
**New Files:**
- `aurora_engine/include/aurora_engine/core/aurora_buffer_pool.hpp`

**Implementation:**
- Create large (e.g., 64MB) GPU buffers.
- Allocate "sub-buffers" by returning offsets into the large buffer.
- Manage free lists or ring buffers for dynamic data (like text).

#### Phase 2B: Integration
- Modify `AuroraModel` to allocate from the pool.
- Modify `AuroraText` to use a dynamic ring buffer within the pool for frequently changing text.

**Expected Performance Gain:** Reduced memory fragmentation, faster text updates, lower driver overhead.

---

## Priority 6: Optimizing Text Rendering - [DONE]

### Current Problem
`AuroraText::setText` is extremely expensive because it triggers `rebuildGeometry()`, which destroys the old `AuroraModel` and creates a brand new one.
```cpp
// aurora_text.cpp
model = std::make_shared<AuroraModel>(...); 
```
`AuroraModel` constructor allocates new `VkDeviceMemory` and creating new `VkBuffer` objects every time text changes. For a UI updating every frame (like FPS counter or Profiler), this causes:
- Massive memory allocation/deallocation overhead.
- Pipeline stalls waiting for upload.
- High "Profiler UI Update" times (~4-5ms).

### Solution Strategy
Implement **Dynamic Text Buffers** to reuse memory.

#### Phase 6A: Dynamic Model Support
- Modify `AuroraModel` to support `updateVertexBuffer` on existing buffers.
- Use `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT` for text buffers so we can map and write directly without staging buffers.

#### Phase 6B: Double Buffering
- To avoid race conditions (writing to a buffer the GPU is reading), implement **Double Buffering** or **Ring Buffering** for dynamic text.
- If the new text is smaller than the capacity, just overwrite. If larger, reallocate (with growth factor).

**Expected Performance Gain:** Near-zero cost for `setText` (just `memcpy`).

