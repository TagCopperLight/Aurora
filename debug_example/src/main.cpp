#include "aurora_ui/aurora_ui.hpp"
#include "aurora_ui/components/aurora_panel.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"

#include "aurora_engine/utils/log.hpp"
#include <fontconfig/fontconfig.h>

class DebugApp : public aurora::AuroraUI {
    public:
        DebugApp() : AuroraUI{"Aurora Debug"} {}

    protected:
        void onSetup(aurora::AuroraComponentInfo& info) override {
            panel = std::make_shared<aurora::AuroraPanel>(info, 400.f);

            auto& network_section = panel->addSection("NETWORK STATUS");
            network_section.addEntry("PORT", "9000");
            network_status = network_section.addEntry("STATUS", "DISCONNECTED", true, aurora::AuroraThemeSettings::get().ERROR);

            panel->addToRenderSystem();
        }

        void onUpdate(float dt) override {
            (void)dt;
            network_status.setValue("CONNECTED", aurora::AuroraThemeSettings::get().SUCCESS);
        }

    private:
        std::shared_ptr<aurora::AuroraPanel> panel;

        aurora::AuroraEntryHandle network_status;
};

int main() {
    aurora::log::init(spdlog::level::debug);

    FcInit();

    aurora::log::engine()->info("Starting Aurora Debug Example");
    DebugApp app{};

    try {
        app.run();
    } catch (const std::exception& e) {
        aurora::log::engine()->error("{}", e.what());
        return EXIT_FAILURE;
    }

    aurora::log::engine()->info("Aurora Debug Example exiting successfully");
    return EXIT_SUCCESS;
}
