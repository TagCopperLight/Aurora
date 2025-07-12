#include "aurora_app/utils/aurora_clock.hpp"
#include <spdlog/spdlog.h>
#include <thread>
#include <iomanip>

namespace aurora {
    AuroraClock::AuroraClock(int targetFrameRate) 
        : targetFrameRate_(targetFrameRate),
          targetFrameTime_(std::chrono::microseconds(1000000 / targetFrameRate)),
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
        updateTimingData();
        
        if (csvLoggingEnabled_) {
            logFrameTimeToCSV();
        }
    }

    void AuroraClock::waitForFrameRate() {
        auto renderTime = frameEndTime_ - frameStartTime_;
        auto renderTimeMicros = std::chrono::duration_cast<std::chrono::microseconds>(renderTime);
        
        if (renderTimeMicros < targetFrameTime_) {
            auto sleepTime = targetFrameTime_ - renderTimeMicros;
            std::this_thread::sleep_for(sleepTime);
        }
        
        frameEndTime_ = std::chrono::high_resolution_clock::now();
        updateTimingData();
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
            csvFile_ << "timestamp_ms,frame_time_ms,fps\n";
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
            csvFile_ << timestampMs_ << "," << frameTimeMs_ << "," << fps_ << "\n";
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
