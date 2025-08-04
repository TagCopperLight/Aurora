#include "aurora_app/profiling/aurora_profiler.hpp"
#include <algorithm>

namespace aurora {
    void AuroraProfiler::addSample(const char* name, double timeMs) {
        if (!enabled_) return;

        std::lock_guard<std::mutex> lock(dataMutex_);
        
        std::string nameStr(name);
        auto& stats = stats_[nameStr];
        
        stats.current = timeMs;
        stats.sampleCount++;
        
        if (stats.sampleCount == 1) {
            stats.average = timeMs;
        } else {
            constexpr double alpha = 2.0 / (AVERAGE_WINDOW + 1.0);
            stats.average = alpha * timeMs + (1.0 - alpha) * stats.average;
        }
        
        stats.minimum = std::min(stats.minimum, timeMs);
        stats.maximum = std::max(stats.maximum, timeMs);
        
        if (currentFrameTime_ > 0.0) {
            stats.framePercentage = (stats.current / currentFrameTime_) * 100.0;
        }
    }

    const AuroraProfiler::StatisticalData& AuroraProfiler::getStats(const char* name) const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(dataMutex_));
        
        std::string nameStr(name);
        auto it = stats_.find(nameStr);
        if (it != stats_.end()) {
            return it->second;
        }
        
        static StatisticalData defaultStats;
        return defaultStats;
    }

    void AuroraProfiler::newFrame() {
        std::lock_guard<std::mutex> lock(dataMutex_);
        for (auto& [name, stats] : stats_) {
            stats.current = 0.0;
            stats.framePercentage = 0.0;
        }
    }
}
