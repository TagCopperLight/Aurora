#pragma once

#include <chrono>
#include <fstream>

namespace aurora {
    class AuroraClock {
    public:
        AuroraClock(int targetFrameRate = 60, bool enableFrameRateLimit = true);
        ~AuroraClock();

        void beginFrame();
        void endFrame();
        
        void setFrameRateLimit(bool enable);
        
        double getFrameTimeMs() const;
        double getFPS() const;
        double getTimestampMs() const;
        
        void enableCSVLogging(const std::string& filename = "frame_times.csv");
        void disableCSVLogging();
        void logFrameTimeToCSV();

    private:
        int targetFrameRate_;
        std::chrono::microseconds targetFrameTime_;
        bool enableFrameRateLimit_;
        
        std::chrono::high_resolution_clock::time_point appStartTime_;
        std::chrono::high_resolution_clock::time_point frameStartTime_;
        std::chrono::high_resolution_clock::time_point frameEndTime_;
        
        double frameTimeMs_;
        double fps_;
        double timestampMs_;
        
        bool csvLoggingEnabled_;
        std::ofstream csvFile_;
        
        void updateTimingData();
    };
}
