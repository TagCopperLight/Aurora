#include "aurora_ui/profiling/aurora_performance_monitor.hpp"
#include "aurora_ui/utils/aurora_utils.hpp"
#include "aurora_engine/profiling/aurora_profiler.hpp"

#include <algorithm>
#include <string>

namespace aurora {
    AuroraPerformanceMonitor::AuroraPerformanceMonitor(AuroraPanelSection& section, double windowSeconds)
        : windowSeconds{windowSeconds} {
        fps = section.addEntry("FPS", "-");
        frameTime = section.addEntry("FRAME TIME", "-");
        cpuWork = section.addEntry("CPU WORK", "-");
        gpuFrame = section.addEntry("GPU FRAME", "-");
        drawCalls = section.addEntry("DRAW CALLS", "-");
    }

    double AuroraPerformanceMonitor::getCpuWorkTime() {
        // Phase names are the AURORA_PROFILE blocks of AuroraUI::run() and AuroraSwapChain
        auto& profiler = AuroraProfiler::instance();
        return profiler.getStats("Poll Events").current + profiler.getStats("Update").current
             + profiler.getStats("Begin Frame").current - profiler.getStats("GPU Wait").current
             + profiler.getStats("Render Components").current + profiler.getStats("End Frame").current;
    }

    bool AuroraPerformanceMonitor::update() {
        auto& profiler = AuroraProfiler::instance();

        window.frames++;
        window.frameTime += profiler.getFrameTime();
        window.maxFrameTime = std::max(window.maxFrameTime, profiler.getFrameTime());
        window.cpuWork += getCpuWorkTime();
        window.gpuFrame += profiler.getStats("GPU Frame").current;

        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - window.start).count();
        if (elapsed < windowSeconds) return false;

        double frames = static_cast<double>(window.frames);
        fps.setValue(AuroraUtils::format("%.0f", frames / elapsed));
        frameTime.setValue(AuroraUtils::format("%.2f ms (max %.1f)", window.frameTime / frames, window.maxFrameTime));
        cpuWork.setValue(AuroraUtils::format("%.3f ms", window.cpuWork / frames));
        gpuFrame.setValue(AuroraUtils::format("%.3f ms", window.gpuFrame / frames));
        drawCalls.setValue(std::to_string(profiler.getCounter("Draw Calls")));

        window = Window{};
        window.start = now;
        return true;
    }
}
