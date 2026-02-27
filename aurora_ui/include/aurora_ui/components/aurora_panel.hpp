#pragma once

#include "aurora_component_interface.hpp"
#include "aurora_ui/components/aurora_text.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace aurora {
    class AuroraEntryHandle {
        public:
            AuroraEntryHandle() = default;
            void setValue(const std::string& value);
            void setValue(const std::string& value, const glm::vec4& color);

        private:
            AuroraEntryHandle(std::weak_ptr<AuroraText> ref, bool enclosed, const glm::vec4& color, float max_width, float y);

            std::weak_ptr<AuroraText> ref;
            bool enclosed = false;
            glm::vec4 color{};
            float max_width = 0.f;
            float y = 0.f;

            friend class AuroraPanelSection;
    };

    class AuroraPanelSection {
        public:
            AuroraEntryHandle addEntry(const std::string& name, const std::string& value);
            AuroraEntryHandle addEntry(const std::string& name, const std::string& value, bool enclosed, const glm::vec4& color);

        private:
            using AddChildFn = std::function<void(std::shared_ptr<AuroraComponentInterface>)>;

            AuroraPanelSection(AuroraComponentInfo& info, float max_width, float x, float& cursor_y, AddChildFn addChild);

            AuroraComponentInfo& info;
            float max_width;
            float x;
            float& cursor_y;
            AddChildFn add_child;

            friend class AuroraPanel;
    };

    class AuroraPanel : public AuroraComponentInterface {
        public:
            AuroraPanel(AuroraComponentInfo& info, float width);

            AuroraPanelSection& addSection(const std::string& title);

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
            void initialize() override {}

            float width;
            float cursor_y = 30.f;
            std::vector<std::unique_ptr<AuroraPanelSection>> sections;
    };
}
