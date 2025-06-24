#include "aurora_pipeline.hpp"

#include <iostream>
#include <fstream>

namespace aurora {
    AuroraPipeline::AuroraPipeline(const std::string& vertFilePath, const std::string& fragFilePath){
        createGraphicsPipeline(vertFilePath, fragFilePath);
    }

    std::vector<char> AuroraPipeline::readFile(const std::string& filePath) {
        std::ifstream file{filePath, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        
        file.close();

        return buffer;
    }

    void AuroraPipeline::createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath) {
        auto vertCode = readFile(vertFilePath);
        auto fragCode = readFile(fragFilePath);

        std::cout << "Vertex shader code size: " << vertCode.size() << " bytes\n";
        std::cout << "Fragment shader code size: " << fragCode.size() << " bytes\n";
    }
}