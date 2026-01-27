#include "aurora_app/profiling/aurora_profiler_ui.hpp"
#include "aurora_app/components/aurora_text.hpp"
#include "aurora_app/utils/aurora_theme_settings.hpp"
#include <cstdio>
#include <spdlog/spdlog.h>

namespace aurora {
    AuroraProfilerUI::AuroraProfilerUI(AuroraComponentInfo& componentInfo, float width)
        : AuroraComponentInterface(componentInfo),
          profiler_(AuroraProfiler::instance()),
          width_(width) {
        
        auto title = std::make_shared<AuroraText>(componentInfo, "[PROFILER]", 16.0f);
        title->setPosition(50.0f, 30.0f);
        title->color = AuroraThemeSettings::TEXT_SECONDARY;
        title->addToRenderSystem();
        displayElements_.push_back(title);
        
        currentLine_++;
        
        float yOffset = 40.0f + (currentLine_ * lineHeight_);
        fpsText_ = std::make_shared<AuroraText>(componentInfo, "FPS: 0.0 | Frame Time: 0.00ms", 16.0f);
        fpsText_->setPosition(50.0f, yOffset);
        fpsText_->color = AuroraThemeSettings::TEXT_PRIMARY;
        fpsText_->addToRenderSystem();
        displayElements_.push_back(fpsText_);
        
        currentLine_++;
    }
    
    void AuroraProfilerUI::addProfiledFunction(const char* functionName) {
        trackedFunctions_.push_back(std::string(functionName));
        
        float yOffset = 40.0f + (currentLine_ * lineHeight_);
        
        auto funcText = std::make_shared<AuroraText>(componentInfo, functionName, 16.0f);
        funcText->setPosition(50.0f, yOffset);
        funcText->color = AuroraThemeSettings::TEXT_PRIMARY;
        funcText->addToRenderSystem();
        displayElements_.push_back(funcText);
        
        currentLine_++;
        
        auto avgText = std::make_shared<AuroraText>(componentInfo, "  Avg: 0.00ms", 16.0f);
        avgText->setPosition(70.0f, yOffset + 30.0f);
        avgText->color = AuroraThemeSettings::TEXT_PRIMARY;
        avgText->addToRenderSystem();
        displayElements_.push_back(avgText);
        
        auto minMaxText = std::make_shared<AuroraText>(componentInfo, "  Min: 0.00ms Max: 0.00ms", 16.0f);
        minMaxText->setPosition(70.0f, yOffset + 60.0f);
        minMaxText->color = AuroraThemeSettings::TEXT_PRIMARY;
        minMaxText->addToRenderSystem();
        displayElements_.push_back(minMaxText);
        
        currentLine_ += 3;
    }

    void AuroraProfilerUI::addTrackedCounter(const char* counterName) {
        trackedCounters_.push_back(std::string(counterName));
        
        float yOffset = 40.0f + (currentLine_ * lineHeight_);
        
        auto counterText = std::make_shared<AuroraText>(componentInfo, counterName, 16.0f);
        counterText->setPosition(50.0f, yOffset);
        counterText->color = AuroraThemeSettings::TEXT_PRIMARY;
        counterText->addToRenderSystem();
        displayElements_.push_back(counterText);
        
        currentLine_++;
    }
    
    void AuroraProfilerUI::update(float deltaTime) {
        AURORA_PROFILE("Profiler UI Update");
        updateDisplayStrings();
    }
    
    void AuroraProfilerUI::updateDisplayStrings() {
        if (!profiler_.isEnabled()) return;
        
        double fps = profiler_.getCurrentFPS();
        double frameTime = profiler_.getFrameTime();
        
        if (fpsText_) {
            formatTime(frameTime, formatBuffer_.data(), formatBuffer_.size());
            std::string combinedStr = "FPS: " + std::to_string(static_cast<int>(fps + 0.5)) + 
                                    " | Frame Time: " + std::string(formatBuffer_.data()) + "ms";
            fpsText_->setText(combinedStr);
        }
        
        size_t elementIndex = 2;
        
        for (const auto& functionName : trackedFunctions_) {
            const auto& stats = profiler_.getStats(functionName.c_str());
            
            if (elementIndex < displayElements_.size()) {
                formatTime(stats.current, formatBuffer_.data(), formatBuffer_.size());
                std::string currentStr = functionName + ": " + std::string(formatBuffer_.data()) + "ms";
                
                formatPercentage(stats.framePercentage, formatBuffer_.data(), formatBuffer_.size());
                currentStr += " (" + std::string(formatBuffer_.data()) + "%)";
                
                displayElements_[elementIndex]->setText(currentStr);
                elementIndex++;
                
                if (elementIndex < displayElements_.size()) {
                    formatTime(stats.average, formatBuffer_.data(), formatBuffer_.size());
                    std::string avgStr = "  Avg: " + std::string(formatBuffer_.data()) + "ms";
                    displayElements_[elementIndex]->setText(avgStr);
                    elementIndex++;
                }
                
                if (elementIndex < displayElements_.size()) {
                    formatTime(stats.minimum, formatBuffer_.data(), formatBuffer_.size());
                    std::string minStr = std::string(formatBuffer_.data());
                    
                    formatTime(stats.maximum, formatBuffer_.data(), formatBuffer_.size());
                    std::string maxStr = std::string(formatBuffer_.data());
                    
                    std::string minMaxStr = "  Min: " + minStr + "ms Max: " + maxStr + "ms";
                    displayElements_[elementIndex]->setText(minMaxStr);
                    elementIndex++;
                }
            }
        }

        for (const auto& counterName : trackedCounters_) {
            if (elementIndex < displayElements_.size()) {
                uint64_t value = profiler_.getCounter(counterName.c_str());
                std::string counterStr = counterName + ": " + std::to_string(value);
                displayElements_[elementIndex]->setText(counterStr);
                elementIndex++;
            }
        }
    }
    
    void AuroraProfilerUI::formatTime(double timeMs, char* buffer, size_t bufferSize) {
        std::snprintf(buffer, bufferSize, "%.2f", timeMs);
    }
    
    void AuroraProfilerUI::formatPercentage(double percentage, char* buffer, size_t bufferSize) {
        std::snprintf(buffer, bufferSize, "%.1f", percentage);
    }
}
