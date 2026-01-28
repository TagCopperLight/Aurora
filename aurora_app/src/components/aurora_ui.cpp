#include "aurora_app/components/aurora_ui.hpp"
#include "aurora_app/components/aurora_text.hpp"

#include <string>

namespace aurora {
    AuroraUI::AuroraUI(AuroraComponentInfo &componentInfo, float width)
        : AuroraComponentInterface{componentInfo}, width{width} {
        initialize();
    }

    void AuroraUI::initialize() {
        auto title1 = std::make_shared<AuroraText>(componentInfo, "[NETWORK STATUS]", 16.f, AuroraThemeSettings::TEXT_SECONDARY);
        title1->setPosition(50.f, 30.f);
        // addChild(title1);

        auto active_routes = std::make_shared<AuroraText>(componentInfo, "ACTIVE ROUTES", 16.f);
        active_routes->setPosition(50.f, 70.f);
        // addChild(active_routes);

        auto active_routes_data = std::make_shared<AuroraText>(componentInfo, "12/106", 16.f);
        active_routes_data->setPosition(400.f - active_routes_data->getTextBounds().x, 70.f);
        // addChild(active_routes_data);

        auto data_age = std::make_shared<AuroraText>(componentInfo, "DATA AGE", 16.f);
        data_age->setPosition(50.f, 100.f);
        addChild(data_age);

        auto data_age_data = std::make_shared<AuroraText>(componentInfo, "3s", 16.f, AuroraThemeSettings::SUCCESS);
        data_age_data->setPosition(400.f - data_age_data->getTextBounds().x, 100.f);
        addChild(data_age_data);

        auto update_rate = std::make_shared<AuroraText>(componentInfo, "UPDATE RATE", 16.f);
        update_rate->setPosition(50.f, 130.f);
        addChild(update_rate);

        auto update_rate_data = std::make_shared<AuroraText>(componentInfo, "5m", 16.f);
        update_rate_data->setPosition(400.f - update_rate_data->getTextBounds().x, 130.f);
        addChild(update_rate_data);

        auto title2 = std::make_shared<AuroraText>(componentInfo, "[API STATUS]", 16.f, AuroraThemeSettings::TEXT_SECONDARY);
        title2->setPosition(50.f, 190.f);
        addChild(title2);

        auto fiveorg = std::make_shared<AuroraText>(componentInfo, "511.ORG", 16.f);
        fiveorg->setPosition(50.f, 230.f);
        addChild(fiveorg);

        auto fiveorg_data = std::make_shared<AuroraText>(componentInfo, "[ OK ]", 16.f, AuroraThemeSettings::SUCCESS);
        fiveorg_data->setPosition(400.f - fiveorg_data->getTextBounds().x, 230.f);
        addChild(fiveorg_data);

        auto bartapi = std::make_shared<AuroraText>(componentInfo, "BART API", 16.f);
        bartapi->setPosition(50.f, 260.f);
        addChild(bartapi);

        auto bartapi_data = std::make_shared<AuroraText>(componentInfo, "[FAIL]", 16.f, AuroraThemeSettings::ERROR);
        bartapi_data->setPosition(400.f - bartapi_data->getTextBounds().x, 260.f);
        addChild(bartapi_data);
    }

    void AuroraUI::addTitle(std::string title){
        auto titleComponent = std::make_shared<AuroraText>(componentInfo, "[" + title + "]", 16.f, AuroraThemeSettings::TEXT_SECONDARY);
        titleComponent->setPosition(cursorPosition);
        addChild(titleComponent);
        cursorPosition.y += 40.f;
    }

    void AuroraUI::addStringVariable(std::string title, std::string* variable){
        auto titleComponent = std::make_shared<AuroraText>(componentInfo, title, 16.f);
        titleComponent->setPosition(cursorPosition);
        addChild(titleComponent);
        
        auto variableComponent = std::make_shared<AuroraText>(componentInfo, *variable, 16.f);
        variableComponent->setPosition(width - variableComponent->getTextBounds().x, cursorPosition.y);
        addChild(variableComponent);
        cursorPosition.y += 30.f;

        variableComponents.push_back(variableComponent);
        variables.push_back(variable);
    }

    void AuroraUI::refreshVariables(){
        for (size_t i = 0; i < variableComponents.size(); i++){
            variableComponents[i]->setText(*variables[i]);
        }
    }
}