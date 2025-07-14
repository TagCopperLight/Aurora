#include "aurora_app/components/aurora_terminal.hpp"
#include "aurora_app/components/aurora_card.hpp"
#include "aurora_app/components/aurora_text.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/constants.hpp>

#include <spdlog/spdlog.h>

#include <memory>
#include <sstream>

namespace aurora {
    AuroraTerminal::AuroraTerminal(AuroraComponentInfo &componentInfo, glm::vec2 size, float fontSize, float padding)
        : AuroraComponentInterface{componentInfo}, size{size}, fontSize{fontSize}, padding{padding} {
        calculateDimensions();
        initialize();
    }

    void AuroraTerminal::calculateDimensions() {
        float availableWidth = size.x - (2 * padding);
        float availableHeight = size.y - (2 * padding);
        
        lineHeight = fontSize * 1.2f;
        
        maxLines = static_cast<int>(availableHeight / lineHeight);
        
        float charWidth = fontSize * 0.6f;
        maxCharsPerLine = static_cast<int>(availableWidth / charWidth);
        
        maxLines = std::max(1, maxLines);
        maxCharsPerLine = std::max(10, maxCharsPerLine);
        
        spdlog::info("Terminal dimensions: {}x{}, maxLines: {}, maxCharsPerLine: {}", size.x, size.y, maxLines, maxCharsPerLine);
    }

    void AuroraTerminal::initialize() {
        auto cardComponent = std::make_shared<AuroraCard>(componentInfo, size, glm::vec4(0.784f, 0.38f, 0.286f, 1.0f));
        addChild(cardComponent);

        // auto textComponent = std::make_shared<AuroraText>(componentInfo, "> Hello World !", 15.0f);
        // textComponent->setPosition(50 * 0.8f, 50 * 0.5f);

        
        addText("> Hello World !");
    }

    std::vector<std::string> AuroraTerminal::wrapText(const std::string& text, size_t maxWidth) {
        std::vector<std::string> wrappedLines;
        
        if (text.empty()) {
            wrappedLines.push_back("");
            return wrappedLines;
        }
        
        std::istringstream words(text);
        std::string word;
        std::string currentLine;
        
        while (words >> word) {
            if (!currentLine.empty() && (currentLine.length() + 1 + word.length()) > maxWidth) {
                wrappedLines.push_back(currentLine);
                currentLine = word;
            } else {
                if (!currentLine.empty()) {
                    currentLine += " ";
                }
                currentLine += word;
            }
        }
        
        if (!currentLine.empty()) {
            wrappedLines.push_back(currentLine);
        }
        
        return wrappedLines;
    }

    void AuroraTerminal::addText(const std::string& text) {
        // spdlog::info("addText called: '{}', current lines: {}", text, lines.size());
        
        std::vector<std::string> newLines = wrapText(text, maxCharsPerLine);
        // spdlog::info("Text wrapped into {} lines", newLines.size());

        for (const auto& line : newLines) {
            lines.push_back(line);
            // spdlog::info("Added line: '{}'", line);
        }
        
        while (static_cast<int>(lines.size()) > maxLines) {
            lines.erase(lines.begin());
        }
        
        // spdlog::info("Final line count: {}", lines.size());
        
        refreshDisplay();
    }

    void AuroraTerminal::refreshDisplay() {
        // spdlog::info("refreshDisplay called, lines to display: {}", lines.size());
        
        auto& children = getChildren();
        
        // spdlog::info("Current children count: {}", children.size());
        
        if (children.size() > 1) {
            children.erase(children.begin() + 1, children.end());
            // spdlog::info("Removed old text components, children count now: {}", children.size());
        }
        
        for (size_t i = 0; i < lines.size(); ++i) {
            auto textComponent = std::make_shared<AuroraText>(componentInfo, lines[i], fontSize);
            
            float x = padding;
            float y = padding + (i * lineHeight);
            
            // spdlog::info("Creating text component {} at position ({}, {}) with text: '{}'", i, x, y, lines[i]);
            
            textComponent->setPosition(x, y);
            spdlog::info("Adding a terminal text component");
            addChild(textComponent);
        }
        
        // spdlog::info("refreshDisplay complete, final children count: {}", getChildren().size());
    }
}