#include "aurora_app/profiling/aurora_profiler.hpp"
#include <algorithm>

namespace aurora {
    void AuroraProfiler::addSample(const char* name, double timeMs) {
        if (!enabled_) return;

        std::lock_guard<std::mutex> lock(dataMutex_);
        
        std::string nameStr(name);
        auto& stats = stats_[nameStr];
        
        stats.current += timeMs;
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

    void AuroraProfiler::setCounter(const char* name, uint64_t value) {
        if (!enabled_) return;
        std::lock_guard<std::mutex> lock(dataMutex_);
        counters_[name] = value;
    }

    void AuroraProfiler::incrementCounter(const char* name, uint64_t value) {
        if (!enabled_) return;
        std::lock_guard<std::mutex> lock(dataMutex_);
        counters_[name] += value;
    }

    uint64_t AuroraProfiler::getCounter(const char* name) const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(dataMutex_));
        auto it = counters_.find(name);
        if (it != counters_.end()) {
            return it->second;
        }
        return 0;
    }

    const std::unordered_map<std::string, uint64_t>& AuroraProfiler::getCounters() const {
        return counters_;
    }

    void AuroraProfiler::newFrame() {
        std::lock_guard<std::mutex> lock(dataMutex_);
        for (auto& [name, stats] : stats_) {
            stats.current = 0.0;
            stats.framePercentage = 0.0;
        }
        for (auto& [name, value] : counters_) {
           value = 0;
        }
    }
}
