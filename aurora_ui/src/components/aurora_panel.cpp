#include "aurora_ui/components/aurora_panel.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"

namespace aurora {
    namespace {
        constexpr float PANEL_TOP = 30.f;
        constexpr float PANEL_LEFT = 50.f;
        constexpr float TITLE_HEIGHT = 40.f;
        constexpr float ENTRY_HEIGHT = 30.f;
        constexpr float SECTION_GAP = 20.f;
    }

    AuroraEntryHandle::AuroraEntryHandle(std::weak_ptr<AuroraText> ref, bool enclosed, const glm::vec4& color, float max_width)
        : ref{ref}, enclosed{enclosed}, color{color}, max_width{max_width} {}

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
            // The entry may have moved since it was created, so its current height is kept
            text->setPosition(max_width - text->getTextBounds().x, text->transform.translation.y);
        }
    }

    AuroraPanelSection::AuroraPanelSection(AuroraPanel& panel, std::string title, float y)
        : panel{panel}, title{std::move(title)}, bottom{y} {}

    AuroraEntryHandle AuroraPanelSection::addEntry(const std::string& name, const std::string& value) {
        return addEntry(name, value, false, AuroraThemeSettings::get().TEXT_PRIMARY);
    }

    AuroraEntryHandle AuroraPanelSection::addEntry(const std::string& name, const std::string& value, bool enclosed, const glm::vec4& color) {
        auto& info = panel.componentInfo;

        auto name_component = std::make_shared<AuroraText>(info, name, 16.f);
        name_component->setPosition(PANEL_LEFT, bottom);
        addComponent(name_component);

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
        value_component->setPosition(panel.width - value_component->getTextBounds().x, bottom);
        addComponent(value_component);

        bottom += ENTRY_HEIGHT;
        panel.sectionGrew(*this, ENTRY_HEIGHT);
        return AuroraEntryHandle{value_component, enclosed, color, panel.width};
    }

    void AuroraPanelSection::setEntry(const std::string& name, const std::string& value) {
        auto it = namedEntries.find(name);
        if (it == namedEntries.end()) {
            namedEntries.emplace(name, addEntry(name, value));
        } else {
            it->second.setValue(value);
        }
    }

    void AuroraPanelSection::addComponent(std::shared_ptr<AuroraComponentInterface> component) {
        components.push_back(component);
        panel.addChild(component);
    }

    void AuroraPanelSection::moveBy(float dy) {
        for (auto& component : components) {
            component->setPosition(component->transform.translation.x, component->transform.translation.y + dy);
        }
        bottom += dy;
    }

    AuroraPanel::AuroraPanel(AuroraComponentInfo& info, float width)
        : AuroraComponentInterface{info}, width{width} {}

    AuroraPanelSection& AuroraPanel::addSection(const std::string& title) {
        float y = sections.empty() ? PANEL_TOP : sections.back()->bottom + SECTION_GAP;
        sections.push_back(std::unique_ptr<AuroraPanelSection>(new AuroraPanelSection(*this, title, y + TITLE_HEIGHT)));

        auto title_component = std::make_shared<AuroraText>(componentInfo, "[" + title + "]", 16.f, AuroraThemeSettings::get().TEXT_SECONDARY);
        title_component->setPosition(PANEL_LEFT, y);
        sections.back()->addComponent(title_component);

        return *sections.back();
    }

    AuroraPanelSection& AuroraPanel::getSection(const std::string& title) {
        for (auto& section : sections) {
            if (section->title == title) return *section;
        }
        return addSection(title);
    }

    void AuroraPanel::sectionGrew(const AuroraPanelSection& section, float dy) {
        bool below = false;
        for (auto& other : sections) {
            if (below) other->moveBy(dy);
            if (other.get() == &section) below = true;
        }
    }
}
