#pragma once

#include <chrono>
#include <unordered_map>
#include <string>
#include <atomic>
#include <mutex>
#include <limits>

namespace aurora {
    class AuroraProfiler {
    public:
        struct StatisticalData {
            double current = 0.0;
            double average = 0.0;
            double minimum = std::numeric_limits<double>::max();
            double maximum = 0.0;
            double framePercentage = 0.0;
            uint32_t sampleCount = 0;
            
            void reset() {
                current = 0.0;
                average = 0.0;
                minimum = std::numeric_limits<double>::max();
                maximum = 0.0;
                framePercentage = 0.0;
                sampleCount = 0;
            }
        };

        struct ProfileBlock {
            const char* name;
            std::chrono::high_resolution_clock::time_point startTime;
            
            ProfileBlock(const char* blockName) : name(blockName) {
                if (AuroraProfiler::instance().isEnabled()) {
                    startTime = std::chrono::high_resolution_clock::now();
                }
            }
            
            ~ProfileBlock() {
                if (AuroraProfiler::instance().isEnabled()) {
                    auto endTime = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                    AuroraProfiler::instance().addSample(name, duration);
                }
            }
        };

        static AuroraProfiler& instance() {
            static AuroraProfiler instance;
            return instance;
        }

        void addSample(const char* name, double timeMs);
        const StatisticalData& getStats(const char* name) const;
        void newFrame();
        void setEnabled(bool enabled) { enabled_ = enabled; }
        bool isEnabled() const { return enabled_; }
        void setFrameTime(double frameTimeMs) { currentFrameTime_ = frameTimeMs; }
        double getCurrentFPS() const { 
            return currentFrameTime_ > 0.0 ? 1000.0 / currentFrameTime_ : 0.0; 
        }
        double getFrameTime() const { return currentFrameTime_; }

        void setCounter(const char* name, uint64_t value);
        void incrementCounter(const char* name, uint64_t value = 1);
        uint64_t getCounter(const char* name) const;
        const std::unordered_map<std::string, uint64_t>& getCounters() const;
        const std::unordered_map<std::string, StatisticalData>& getAllStats() const { return stats_; }

    private:
        AuroraProfiler() = default;
        ~AuroraProfiler() = default;
        AuroraProfiler(const AuroraProfiler&) = delete;
        AuroraProfiler& operator=(const AuroraProfiler&) = delete;

        std::unordered_map<std::string, StatisticalData> stats_;
        std::unordered_map<std::string, uint64_t> counters_;
        std::atomic<bool> enabled_{true};
        std::mutex dataMutex_;
        double currentFrameTime_ = 0.0;

        static constexpr uint32_t AVERAGE_WINDOW = 600;
    };

    #ifdef AURORA_PROFILING_ENABLED
        #define AURORA_PROFILE(name) AuroraProfiler::ProfileBlock _prof(name)
    #else
        #define AURORA_PROFILE(name)
    #endif
}
