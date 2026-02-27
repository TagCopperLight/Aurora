#pragma once

#include "aurora_component_interface.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"
#include <string>
#include <vector>

namespace aurora {

    enum class AuroraConnectionStatus : uint8_t {
        ERROR = 0,
        DISCONNECTED = 1,
        CONNECTED = 2,
    };

    class AuroraPanel : public AuroraComponentInterface {
        public:
            AuroraPanel(AuroraComponentInfo &componentInfo, float max_width, uint16_t port);

            const std::string& getVertexShaderPath() const override {
                static const std::string vertexPath = "shaders/shader.vert.spv";
                return vertexPath;
            }
            
            const std::string& getFragmentShaderPath() const override {
                static const std::string fragmentPath = "shaders/shader.frag.spv";
                return fragmentPath;
            }
            
            VkPrimitiveTopology getTopology() const override {
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            }

        private:
            void initialize() override;

            float max_width;
            uint16_t port;
    };

    class AuroraPanelEntry {
        public:
            AuroraPanelEntry(const std::string& name, const std::string& value, const bool enclosed, const glm::vec4 color) : name(name), value(value), enclosed(enclosed), color(color) {}
            AuroraPanelEntry(const std::string& name, const std::string& value, const bool enclosed) : name(name), value(value), enclosed(enclosed), color(AuroraThemeSettings::get().TEXT_PRIMARY) {}

            std::string getName() const { return name; }
            std::string getValue() const { return value; }
            bool isEnclosed() const { return enclosed; }
            glm::vec4 getColor() const { return color; }

        private:
            std::string name;
            std::string value;
            bool enclosed;
            glm::vec4 color;
    };

    class AuroraPanelSection {
        public:
            explicit AuroraPanelSection(AuroraComponentInfo &componentInfo, const std::string& title, float max_width, float x, float y)
                : componentInfo(componentInfo), title(title), max_width(max_width), x(x), y(y) {}

            void addEntry(const AuroraPanelEntry& entry);

            std::vector<std::shared_ptr<AuroraComponentInterface>> getComponents();

        private:
            AuroraComponentInfo &componentInfo;
            std::string title;
            float max_width;
            float x;
            float y;
            
            float y_offset = 0.f;
            std::vector<AuroraPanelEntry> entries;
    };
}