# Aurora Engine Optimization Action Plan

## Overview
This document outlines a comprehensive optimization strategy for the Aurora graphics engine.
**Status Update**: A review of the codebase revealed that some previously proposed optimizations (Push Constants, Profiler) are already implemented. The plan has been updated to focus on high-impact missing features.

## Priority 1: Instanced Rendering System

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

## Priority 2: Buffer Sub-allocation Strategy

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

## Priority 3: Multithreading & Job System

### Current Problem
`AuroraRenderSystemManager::renderAllComponents` executes rendering logic on the main thread.
```cpp
// aurora_render_system_manager.cpp
for (const auto& renderSystem : renderSystems) {
    renderSystem->renderComponents(...);
}
```
This underutilizes multi-core CPUs.

### Solution Strategy
Implement a **Job System** to record Secondary Command Buffers in parallel.

#### Phase 3A: Job System
**New Files:**
- `aurora_engine/core/aurora_job_system.hpp`

**Implementation:**
- A simple thread pool with a task queue.

#### Phase 3B: Parallel Command Recording
- Change `renderComponents` to record into a `VkCommandBuffer` (Secondary).
- Launch a job for each `AuroraRenderSystem` or batch of components.
- Main thread executes `vkCmdExecuteCommands` to replay them.

**Expected Performance Gain:** Linear scaling of CPU recording time with core count.

---

## Priority 4: Frustum Culling

### Current Problem
`AuroraRenderSystem::renderComponents` iterates *all* components and draws them, even if they are behind the camera or off-screen.
```cpp
if (component->isHidden() || !component->model) continue; // No culling check
```

### Solution Strategy
Add a simple bounding volume check.

#### Phase 4A: Bounding Volumes
- Add `AABB` or `Sphere` to `AuroraModel`.
- Transform AABB by component's Model Matrix.

#### Phase 4B: Culling Check
- In `renderComponents`, check Transformed AABB vs Camera Frustum.
- Skip specific draw calls if outside.

**Expected Performance Gain:** Linear reduction in GPU load based on how many objects are off-screen.

---

## Priority 5: Data-Oriented Memory Layout (SoA)

### Current Problem
Components are stored as `std::vector<std::shared_ptr<AuroraComponentInterface>>`.
- **Pointer Chasing**: Accessing a component requires a pointer dereference.
- **Cache Misses**: Components are scattered in heap memory.
- **Virtual Calls**: `renderComponents` calls virtual methods like `getMVPMatrix` (which involves recursive tree traversal).

### Solution Strategy
Transition from Object-Oriented Hierarchy to Data-Oriented Arrays for hot paths.

#### Phase 5A: Flat Transform Array
- Maintain a flat `std::vector<glm::mat4>` of world transforms.
- Update this flat array in a linear pass (dirty flag system).

#### Phase 5B: Component Render List
- Instead of iterating the scene graph for rendering, maintain a linear `RenderList` struct:
    ```cpp
    struct RenderList {
        vector<mat4> transforms;
        vector<Model*> models;
        vector<vec4> colors;
    };
    ```
- Fill this list during `update()`, iterate it linearly during `render()`.

**Expected Performance Gain:** Significant CPU cache hit rate improvement.

---

## Notes on Removed Items from Previous Plan
- **Push Constants**: Removed because `AuroraRenderSystem` already uses `vkCmdPushConstants`.
- **Profiler**: Removed because `AuroraProfiler` and `AuroraProfilerUI` are already implemented and integrated in `AuroraApp`.
