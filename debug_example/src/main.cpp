
#include "aurora_ui/aurora_ui.hpp"
#include "aurora_ui/components/aurora_panel.hpp"

#include <cstdlib>
#include <spdlog/spdlog.h>
#include <fontconfig/fontconfig.h>

int main(){
    spdlog::set_level(spdlog::level::debug);

    FcInit();

    spdlog::info("Starting Aurora Debug Example");
    aurora::AuroraUI ui{"Aurora Debug"};

    try {
        ui.run([](aurora::AuroraComponentInfo& info) {
            auto panel = std::make_shared<aurora::AuroraPanel>(info, 400.f);
            panel->addToRenderSystem();
        });
    } catch (const std::exception &e) {
        spdlog::error("{}", e.what());
        return EXIT_FAILURE;
    }

    spdlog::info("Aurora Debug Example exiting successfully");
    return EXIT_SUCCESS;
}