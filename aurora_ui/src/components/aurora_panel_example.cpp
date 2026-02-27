#include "aurora_ui/components/aurora_panel_example.hpp"
#include "aurora_ui/components/aurora_text.hpp"

#include <spdlog/spdlog.h>
#include <string>

namespace aurora {
    AuroraPanelExample::AuroraPanelExample(AuroraComponentInfo &componentInfo, float width)
        : AuroraComponentInterface{componentInfo}, width{width} {
        initialize();
    }

    void AuroraPanelExample::initialize() {
        auto title1 = std::make_shared<AuroraText>(componentInfo, "[NETWORK STATUS]", 16.f, AuroraThemeSettings::get().TEXT_SECONDARY);
        title1->setPosition(50.f, 30.f);
        addChild(title1);

        auto active_routes = std::make_shared<AuroraText>(componentInfo, "ACTIVE ROUTES", 16.f);
        active_routes->setPosition(50.f, 70.f);
        addChild(active_routes);

        auto active_routes_data = std::make_shared<AuroraText>(componentInfo, "12/106", 16.f);
        active_routes_data->setPosition(400.f - active_routes_data->getTextBounds().x, 70.f);
        addChild(active_routes_data);

        auto data_age = std::make_shared<AuroraText>(componentInfo, "DATA AGE", 16.f);
        data_age->setPosition(50.f, 100.f);
        addChild(data_age);

        auto data_age_data = std::make_shared<AuroraText>(componentInfo, "3s", 16.f, AuroraThemeSettings::get().SUCCESS);
        data_age_data->setPosition(400.f - data_age_data->getTextBounds().x, 100.f);
        addChild(data_age_data);

        auto update_rate = std::make_shared<AuroraText>(componentInfo, "UPDATE RATE", 16.f);
        update_rate->setPosition(50.f, 130.f);
        addChild(update_rate);

        auto update_rate_data = std::make_shared<AuroraText>(componentInfo, "5m", 16.f);
        update_rate_data->setPosition(400.f - update_rate_data->getTextBounds().x, 130.f);
        addChild(update_rate_data);

        auto title2 = std::make_shared<AuroraText>(componentInfo, "[API STATUS]", 16.f, AuroraThemeSettings::get().TEXT_SECONDARY);
        title2->setPosition(50.f, 190.f);
        addChild(title2);

        auto fiveorg = std::make_shared<AuroraText>(componentInfo, "511.ORG", 16.f);
        fiveorg->setPosition(50.f, 230.f);
        addChild(fiveorg);
        
        auto fiveorg_bracket_right = std::make_shared<AuroraText>(componentInfo, "]", 16.f);
        auto fiveorg_data = std::make_shared<AuroraText>(componentInfo, "OK", 16.f, AuroraThemeSettings::get().SUCCESS);
        auto fiveorg_bracket_left = std::make_shared<AuroraText>(componentInfo, "[", 16.f);

        float space_padding = 10.0f;
        float fiveorg_x = 400.f - fiveorg_bracket_right->getTextBounds().x;
        fiveorg_bracket_right->setPosition(fiveorg_x, 260.f);
        addChild(fiveorg_bracket_right);

        fiveorg_x -= (fiveorg_data->getTextBounds().x + space_padding);
        fiveorg_data->setPosition(fiveorg_x, 260.f);
        addChild(fiveorg_data);

        fiveorg_x -= (fiveorg_bracket_left->getTextBounds().x + space_padding);
        fiveorg_bracket_left->setPosition(fiveorg_x, 260.f);
        addChild(fiveorg_bracket_left);

        auto bartapi = std::make_shared<AuroraText>(componentInfo, "BART API", 16.f);
        bartapi->setPosition(50.f, 290.f);
        addChild(bartapi);

        auto bartapi_bracket_right = std::make_shared<AuroraText>(componentInfo, "]", 16.f);
        auto bartapi_data = std::make_shared<AuroraText>(componentInfo, "FAIL", 16.f, AuroraThemeSettings::get().ERROR);
        auto bartapi_bracket_left = std::make_shared<AuroraText>(componentInfo, "[", 16.f);

        float bartapi_x = 400.f - bartapi_bracket_right->getTextBounds().x;
        bartapi_bracket_right->setPosition(bartapi_x, 290.f);
        addChild(bartapi_bracket_right);

        bartapi_x -= bartapi_data->getTextBounds().x;
        bartapi_data->setPosition(bartapi_x, 290.f);
        addChild(bartapi_data);

        bartapi_x -= bartapi_bracket_left->getTextBounds().x;
        bartapi_bracket_left->setPosition(bartapi_x, 290.f);
        addChild(bartapi_bracket_left);
    }

    void AuroraPanelExample::addTitle(std::string title){
        auto titleComponent = std::make_shared<AuroraText>(componentInfo, "[" + title + "]", 16.f, AuroraThemeSettings::get().TEXT_SECONDARY);
        titleComponent->setPosition(cursorPosition);
        addChild(titleComponent);
        cursorPosition.y += 40.f;
    }
}