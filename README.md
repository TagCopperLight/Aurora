# Aurora

A C++17 toolkit for visualizing simulations and debugging live data, rendered
with Vulkan. Combine 2D shapes, text, trails, and panels in a small application,
or receive watch values from another process over TCP.

## Build and run

The current build targets Linux. You'll need CMake, a C++17 compiler, Vulkan
drivers and development files, `glslc`, GLFW 3.3+, GLM, spdlog, Fontconfig,
FreeType, libpng, zlib, and the `msdfgen` and `msdf-atlas-gen` libraries and headers.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DAURORA_ENABLE_GPROF=OFF
cmake --build build -j
cd build
./debug_example/debug_example
```

The example opens a fullscreen window and listens on TCP port 9000. From another
terminal, run `python3 debug_example/watch_example.py` at the repository root to
send live values. Run executables from the build directory so they can find
`assets/` and `shaders/`.

## Create a visualization

Subclass `aurora::AuroraUI`, create components in `onSetup()`, and update them in
`onUpdate(dt)`. Aurora handles the window and render loop.

```cpp
#include <aurora_ui/aurora_ui.hpp>
#include <aurora_ui/components/aurora_circle.hpp>
#include <aurora_ui/components/aurora_panel.hpp>

class App : public aurora::AuroraUI {
    std::shared_ptr<aurora::AuroraCircle> dot;
    float x = 100.f;

    void onSetup() override {
        dot = create<aurora::AuroraCircle>(10.f, glm::vec4(1.f));
        auto panel = create<aurora::AuroraPanel>(400.f);
        panel->addSection("SIMULATION").setEntry("Status", "Running");
    }

    void onUpdate(float dt) override {
        x += 50.f * dt;
        dot->setPosition(x, 300.f);
    }
};

int main() {
    App app;
    app.run();
}
```

Add Aurora with CMake's `add_subdirectory()` and link your executable to
`aurora_ui`, plus `Freetype::Freetype`, `PNG::PNG`, and `ZLIB::ZLIB` for the current
static-library build. Link `aurora_debug` when using the TCP server. See
[the debug example](debug_example/src/main.cpp) for panels, connection monitoring,
and performance counters.

## Layout

- `aurora_engine`: Vulkan device, swapchain, buffers, textures, and profiling.
- `aurora_ui`: components, text rendering, panels, and the application loop.
- `aurora_debug`: TCP sessions, watch messages, and frame streaming.
- `debug_example`: a live watch viewer and Python clients.
- `aurora_bench` and [`bench/`](bench/README.md): rendering and performance checks.
