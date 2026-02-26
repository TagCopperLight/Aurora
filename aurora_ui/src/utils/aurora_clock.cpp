#include "aurora_ui/utils/aurora_clock.hpp"
#include "aurora_engine/profiling/aurora_profiler.hpp"
#include "aurora_engine/utils/log.hpp"
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
        log::ui()->info("AuroraClock initialized with target frame rate: {} FPS", targetFrameRate);
    }

    AuroraClock::~AuroraClock() {
        if (csvLoggingEnabled_ && csvFile_.is_open()) {
            csvFile_.close();
            log::ui()->info("Frame time data saved to CSV file");
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
        log::ui()->info("Frame rate limit {}", enable ? "enabled" : "disabled");
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
            csvFile_ << "timestamp_ms,frame_time_ms,fps,render_components_ms,poll_events_ms,begin_frame_ms,end_frame_ms,profiler_ui_ms\n";
            csvFile_ << std::fixed << std::setprecision(3);
            csvLoggingEnabled_ = true;
            log::ui()->info("CSV logging enabled, writing to: {}", filename);
        } else {
            log::ui()->error("Failed to open CSV file for writing: {}", filename);
            csvLoggingEnabled_ = false;
        }
    }

    void AuroraClock::disableCSVLogging() {
        if (csvLoggingEnabled_ && csvFile_.is_open()) {
            csvFile_.close();
            log::ui()->info("CSV logging disabled");
        }
        csvLoggingEnabled_ = false;
    }

    void AuroraClock::logFrameTimeToCSV() {
        if (csvLoggingEnabled_ && csvFile_.is_open()) {
            auto& profiler = AuroraProfiler::instance();
            double renderTime = profiler.getStats("Render Components").current;
            double pollTime = profiler.getStats("Poll Events").current;
            double beginTime = profiler.getStats("Begin Frame").current;
            double endTime = profiler.getStats("End Frame").current;
            double uiTime = profiler.getStats("Profiler UI Update").current;
            
            csvFile_ << timestampMs_ << "," << frameTimeMs_ << "," << fps_ << "," << renderTime << "," << pollTime << "," << beginTime << "," << endTime << "," << uiTime << "\n";
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
