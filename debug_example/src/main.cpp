#include "aurora_ui/aurora_ui.hpp"
#include "aurora_ui/components/aurora_panel.hpp"

#include <cstdlib>
#include <memory>
#include "aurora_engine/utils/log.hpp"
#include <fontconfig/fontconfig.h>

class DebugApp : public aurora::AuroraUI {
    public:
        DebugApp() : AuroraUI{"Aurora Debug"} {}

    protected:
        void onSetup(aurora::AuroraComponentInfo& info) override {
            panel = std::make_shared<aurora::AuroraPanel>(info, 400.f);
            panel->addToRenderSystem();
        }

        void onUpdate(float dt) override {
            (void)dt;
        }

    private:
        std::shared_ptr<aurora::AuroraPanel> panel;
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