#pragma once

#include "aurora_device.hpp"

#include <string>
#include <vector>

namespace aurora {
    struct PipelineConfigInfo {};

    class AuroraPipeline {
        public:
            AuroraPipeline(AuroraDevice& device, const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo);

            ~AuroraPipeline() {};

            AuroraPipeline(const AuroraPipeline&) = delete;
            void operator=(const AuroraPipeline&) = delete;

            static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

        private:
            static std::vector<char> readFile(const std::string& filePath);
            void createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo);

            void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

            AuroraDevice& auroraDevice;
            VkPipeline graphicsPipeline;
            VkShaderModule vertShaderModule;
            VkShaderModule fragShaderModule;
    };
}