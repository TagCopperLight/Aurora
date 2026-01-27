#include "aurora_app/utils/aurora_clock.hpp"
#include "aurora_app/profiling/aurora_profiler.hpp"
#include <spdlog/spdlog.h>
#include <thread>
#include <iomanip>

namespace aurora {
    AuroraClock::AuroraClock(int targetFrameRate, bool enableFrameRateLimit) 
        : targetFrameRate_(targetFrameRate),
          targetFrameTime_(std::chrono::microseconds(1000000 / targetFrameRate) - std::chrono::microseconds(225)),
          enableFrameRateLimit_(enableFrameRateLimit),
          appStartTime_(std::chrono::high_resolution_clock::now()),
          frameTimeMs_(0.0),
          fps_(0.0),
          timestampMs_(0.0),
          csvLoggingEnabled_(false) {
        spdlog::info("AuroraClock initialized with target frame rate: {} FPS", targetFrameRate);
    }

    AuroraClock::~AuroraClock() {
        if (csvLoggingEnabled_ && csvFile_.is_open()) {
            csvFile_.close();
            spdlog::info("Frame time data saved to CSV file");
        }
    }

    void AuroraClock::beginFrame() {
        frameStartTime_ = std::chrono::high_resolution_clock::now();
    }

    void AuroraClock::endFrame() {
        frameEndTime_ = std::chrono::high_resolution_clock::now();
        
        if (enableFrameRateLimit_) {
            auto renderTime = frameEndTime_ - frameStartTime_;
            auto renderTimeMicros = std::chrono::duration_cast<std::chrono::microseconds>(renderTime);
            
            if (renderTimeMicros < targetFrameTime_) {
                auto sleepTime = targetFrameTime_ - renderTimeMicros;
                std::this_thread::sleep_for(sleepTime);
            }
            
            frameEndTime_ = std::chrono::high_resolution_clock::now();
        }
        
        updateTimingData();
        
        if (csvLoggingEnabled_) {
            logFrameTimeToCSV();
        }
    }

    void AuroraClock::setFrameRateLimit(bool enable) {
        enableFrameRateLimit_ = enable;
        spdlog::info("Frame rate limit {}", enable ? "enabled" : "disabled");
    }

    double AuroraClock::getFrameTimeMs() const {
        return frameTimeMs_;
    }

    double AuroraClock::getFPS() const {
        return fps_;
    }

    double AuroraClock::getTimestampMs() const {
        return timestampMs_;
    }

    void AuroraClock::enableCSVLogging(const std::string& filename) {
        if (csvLoggingEnabled_ && csvFile_.is_open()) {
            csvFile_.close();
        }
        
        csvFile_.open(filename);
        if (csvFile_.is_open()) {
            csvFile_ << "timestamp_ms,frame_time_ms,fps,draw_calls,render_components_ms,poll_events_ms,begin_frame_ms,end_frame_ms\n";
            csvFile_ << std::fixed << std::setprecision(3);
            csvLoggingEnabled_ = true;
            spdlog::info("CSV logging enabled, writing to: {}", filename);
        } else {
            spdlog::error("Failed to open CSV file for writing: {}", filename);
            csvLoggingEnabled_ = false;
        }
    }

    void AuroraClock::disableCSVLogging() {
        if (csvLoggingEnabled_ && csvFile_.is_open()) {
            csvFile_.close();
            spdlog::info("CSV logging disabled");
        }
        csvLoggingEnabled_ = false;
    }

    void AuroraClock::logFrameTimeToCSV() {
        if (csvLoggingEnabled_ && csvFile_.is_open()) {
            auto& profiler = AuroraProfiler::instance();
            uint64_t drawCalls = profiler.getCounter("Draw Calls");
            double renderTime = profiler.getStats("Render Components").current;
            double pollTime = profiler.getStats("Poll Events").current;
            double beginTime = profiler.getStats("Begin Frame").current;
            double endTime = profiler.getStats("End Frame").current;
            
            csvFile_ << timestampMs_ << "," << frameTimeMs_ << "," << fps_ << "," << drawCalls << "," << renderTime << "," << pollTime << "," << beginTime << "," << endTime << "\n";
        }
    }

    void AuroraClock::updateTimingData() {
        auto frameTime = frameEndTime_ - frameStartTime_;
        auto timeSinceStart = frameStartTime_ - appStartTime_;
        
        timestampMs_ = std::chrono::duration<double, std::milli>(timeSinceStart).count();
        frameTimeMs_ = std::chrono::duration<double, std::milli>(frameTime).count();
        fps_ = 1000.0 / frameTimeMs_;
    }
}
