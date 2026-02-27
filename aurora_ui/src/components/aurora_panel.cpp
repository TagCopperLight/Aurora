#include "aurora_ui/components/aurora_panel.hpp"
#include "aurora_ui/components/aurora_text.hpp"
#include <vector>

namespace aurora {
    AuroraPanel::AuroraPanel(AuroraComponentInfo &componentInfo, float max_width, uint16_t port)
        : AuroraComponentInterface{componentInfo}, max_width{max_width}, port{port} {
        initialize();
    }

    void AuroraPanel::initialize() {
        auto network_section = std::make_shared<AuroraPanelSection>(componentInfo, "NETWORK STATUS", max_width, 50.f, 30.f);
        network_section->addEntry(AuroraPanelEntry("PORT", std::to_string(port), false));
        network_section->addEntry(AuroraPanelEntry("STATUS", "CONNECTED", true, AuroraThemeSettings::get().SUCCESS));
        
        for (auto& component : network_section->getComponents()) {
            addChild(component);
        }
    }

    void AuroraPanelSection::addEntry(const AuroraPanelEntry& entry) {
        entries.emplace_back(entry);
    }

    std::vector<std::shared_ptr<AuroraComponentInterface>> AuroraPanelSection::getComponents() {
        std::vector<std::shared_ptr<AuroraComponentInterface>> components;

        auto title_component = std::make_shared<AuroraText>(componentInfo, "[" + title + "]", 16.f, AuroraThemeSettings::get().TEXT_SECONDARY);
        title_component->setPosition(x, y + y_offset);
        components.emplace_back(title_component);

        y_offset += 40.f;

        for (const auto& entry : entries) {
            auto entry_component = std::make_shared<AuroraText>(componentInfo, entry.getName(), 16.f);
            entry_component->setPosition(x, y + y_offset);
            components.emplace_back(entry_component);

            std::shared_ptr<AuroraText> entry_component_data;
            if (entry.isEnclosed()) {
                entry_component_data = std::make_shared<AuroraText>(componentInfo, std::vector<TextSegment>{
                    {"[", AuroraThemeSettings::get().TEXT_PRIMARY},
                    {entry.getValue(), entry.getColor()},
                    {"]", AuroraThemeSettings::get().TEXT_PRIMARY},
                }, 16.f);
            } else {
                entry_component_data = std::make_shared<AuroraText>(componentInfo, entry.getValue(), 16.f, entry.getColor());
            }
            entry_component_data->setPosition(max_width - entry_component_data->getTextBounds().x, y + y_offset);
            components.emplace_back(entry_component_data);

            y_offset += 30.f;
        }
        return components;
    }
}