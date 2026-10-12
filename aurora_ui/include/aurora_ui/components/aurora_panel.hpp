#pragma once

#include "aurora_component_interface.hpp"
#include "aurora_ui/components/aurora_text.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace aurora {
    class AuroraEntryHandle {
        public:
            AuroraEntryHandle() = default;
            void setValue(const std::string& value);
            void setValue(const std::string& value, const glm::vec4& color);

        private:
            AuroraEntryHandle(std::weak_ptr<AuroraText> ref, bool enclosed, const glm::vec4& color, float max_width);

            std::weak_ptr<AuroraText> ref;
            bool enclosed = false;
            glm::vec4 color{};
            float max_width = 0.f;

            friend class AuroraPanelSection;
    };

    class AuroraPanel;

    class AuroraPanelSection {
        public:
            AuroraEntryHandle addEntry(const std::string& name, const std::string& value);
            AuroraEntryHandle addEntry(const std::string& name, const std::string& value, bool enclosed, const glm::vec4& color);

            // Sets the value of the entry called `name`, adding it at the end of the section the first time
            void setEntry(const std::string& name, const std::string& value);

            const std::string& getTitle() const { return title; }

        private:
            AuroraPanelSection(AuroraPanel& panel, std::string title, float y);

            void addComponent(std::shared_ptr<AuroraComponentInterface> component);
            void moveBy(float dy);

            AuroraPanel& panel;
            std::string title;
            float bottom;
            std::vector<std::shared_ptr<AuroraComponentInterface>> components;
            std::unordered_map<std::string, AuroraEntryHandle> namedEntries;

            friend class AuroraPanel;
    };

    // A column of titled sections, each listing name/value entries. Sections can keep
    // growing after later ones were added: the sections below move down to make room.
    class AuroraPanel : public AuroraComponentInterface {
        public:
            AuroraPanel(AuroraComponentInfo& info, float width);

            AuroraPanelSection& addSection(const std::string& title);

            // Returns the first section with this title, adding it at the bottom of the panel if there is none
            AuroraPanelSection& getSection(const std::string& title);

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

            // Moves every section after `section` down by `dy`
            void sectionGrew(const AuroraPanelSection& section, float dy);

            float width;
            std::vector<std::unique_ptr<AuroraPanelSection>> sections;

            friend class AuroraPanelSection;
    };
}
