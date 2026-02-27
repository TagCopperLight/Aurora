#include "aurora_ui/components/aurora_panel.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"

namespace aurora {

    AuroraEntryHandle::AuroraEntryHandle(std::weak_ptr<AuroraText> ref, bool enclosed, const glm::vec4& color, float max_width, float y)
        : ref{ref}, enclosed{enclosed}, color{color}, max_width{max_width}, y{y} {}

    void AuroraEntryHandle::setValue(const std::string& value) {
        setValue(value, color);
    }

    void AuroraEntryHandle::setValue(const std::string& value, const glm::vec4& newColor) {
        color = newColor;
        if (auto text = ref.lock()) {
            if (enclosed) {
                text->setSegments({
                    {"[", AuroraThemeSettings::get().TEXT_PRIMARY},
                    {value, color},
                    {"]", AuroraThemeSettings::get().TEXT_PRIMARY},
                });
            } else {
                text->setSegments({{value, color}});
            }
            text->setPosition(max_width - text->getTextBounds().x, y);
        }
    }

    AuroraPanelSection::AuroraPanelSection(AuroraComponentInfo& info, float max_width, float x, float& cursor_y, AddChildFn addChild)
        : info{info}, max_width{max_width}, x{x}, cursor_y{cursor_y}, add_child{std::move(addChild)} {}

    AuroraEntryHandle AuroraPanelSection::addEntry(const std::string& name, const std::string& value) {
        return addEntry(name, value, false, AuroraThemeSettings::get().TEXT_PRIMARY);
    }

    AuroraEntryHandle AuroraPanelSection::addEntry(const std::string& name, const std::string& value, bool enclosed, const glm::vec4& color) {
        auto name_component = std::make_shared<AuroraText>(info, name, 16.f);
        name_component->setPosition(x, cursor_y);
        add_child(name_component);

        std::shared_ptr<AuroraText> value_component;
        if (enclosed) {
            value_component = std::make_shared<AuroraText>(info, std::vector<TextSegment>{
                {"[", AuroraThemeSettings::get().TEXT_PRIMARY},
                {value, color},
                {"]", AuroraThemeSettings::get().TEXT_PRIMARY},
            }, 16.f);
        } else {
            value_component = std::make_shared<AuroraText>(info, value, 16.f, color);
        }
        value_component->setPosition(max_width - value_component->getTextBounds().x, cursor_y);
        add_child(value_component);

        cursor_y += 30.f;
        return AuroraEntryHandle{value_component, enclosed, color, max_width, cursor_y - 30.f};
    }

    AuroraPanel::AuroraPanel(AuroraComponentInfo& info, float width)
        : AuroraComponentInterface{info}, width{width} {}

    AuroraPanelSection& AuroraPanel::addSection(const std::string& title) {
        if (!sections.empty()) {
            cursor_y += 20.f;
        }

        auto title_component = std::make_shared<AuroraText>(componentInfo, "[" + title + "]", 16.f, AuroraThemeSettings::get().TEXT_SECONDARY);
        title_component->setPosition(50.f, cursor_y);
        addChild(title_component);
        cursor_y += 40.f;

        sections.push_back(std::unique_ptr<AuroraPanelSection>(new AuroraPanelSection(
            componentInfo, width, 50.f, cursor_y,
            [this](std::shared_ptr<AuroraComponentInterface> child) { addChild(child); }
        )));
        return *sections.back();
    }
}
