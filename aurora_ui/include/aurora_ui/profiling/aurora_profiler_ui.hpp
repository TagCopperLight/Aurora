#pragma once

#include "aurora_ui/components/aurora_component_interface.hpp"
#include "aurora_engine/profiling/aurora_profiler.hpp"
#include <vector>
#include <string>
#include <memory>

namespace aurora {
    class AuroraText;
    
    class AuroraProfilerUI : public AuroraComponentInterface {
    public:
        AuroraProfilerUI(AuroraComponentInfo& componentInfo, float width);
        
        void addProfiledFunction(const char* functionName);
        void addTrackedCounter(const char* counterName);
        void update(float deltaTime) override;
        
        const std::string& getVertexShaderPath() const override {
            static const std::string vertexPath = "shaders/text.vert.spv";
            return vertexPath;
        }
        
        const std::string& getFragmentShaderPath() const override {
            static const std::string fragmentPath = "shaders/text.frag.spv";
            return fragmentPath;
        }
        
        VkPrimitiveTopology getTopology() const override {
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
        
    private:
        AuroraProfiler& profiler_;
        std::vector<std::string> trackedFunctions_;
        std::vector<std::string> trackedCounters_;
        std::vector<std::shared_ptr<AuroraText>> displayElements_;
        
        std::shared_ptr<AuroraText> fpsText_;
        
        std::array<char, 64> formatBuffer_;
        
        void updateDisplayStrings();
        void formatTime(double timeMs, char* buffer, size_t bufferSize);
        void formatPercentage(double percentage, char* buffer, size_t bufferSize);
        
        float width_;
        float lineHeight_ = 30.0f;
        int currentLine_ = 0;
    };
}
