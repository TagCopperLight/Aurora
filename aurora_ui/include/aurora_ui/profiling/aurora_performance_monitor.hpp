#pragma once

#include "aurora_ui/components/aurora_panel.hpp"

#include <chrono>
#include <cstdint>

namespace aurora {
    // Fills a panel section with frame timings (FPS, frame time, CPU work, GPU frame, draw calls).
    // Per-frame values flicker too much to read, so they are averaged over a short window.
    class AuroraPerformanceMonitor {
        public:
            explicit AuroraPerformanceMonitor(AuroraPanelSection& section, double windowSeconds = 0.5);

            // Call once per frame from AuroraUI::onFrameEnd(). Returns true when the window ended and
            // the displayed values were refreshed; getLastWindowSeconds() then gives its exact duration.
            bool update();

            double getLastWindowSeconds() const { return lastWindowSeconds; }

            // Time the CPU spent on the last frame: every phase of the frame except waiting on the GPU
            static double getCpuWorkTime();

        private:
            struct Window {
                std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
                long frames = 0;
                double frameTime = 0.0;
                double maxFrameTime = 0.0;
                double cpuWork = 0.0;
                double gpuFrame = 0.0;
            };

            AuroraEntryHandle fps;
            AuroraEntryHandle frameTime;
            AuroraEntryHandle cpuWork;
            AuroraEntryHandle gpuFrame;
            AuroraEntryHandle drawCalls;

            double windowSeconds;
            double lastWindowSeconds = 0.0;
            Window window;
    };
}
