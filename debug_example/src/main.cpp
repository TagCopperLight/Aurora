#include "aurora_ui/aurora_ui.hpp"
#include "aurora_ui/components/aurora_panel.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"
#include "aurora_debug/aurora_debug_server.hpp"

#include "aurora_engine/utils/log.hpp"
#include <fontconfig/fontconfig.h>

class DebugApp : public aurora::AuroraUI {
    public:
        DebugApp() : AuroraUI{"Aurora Debug"} {}

    protected:
        void onSetup(aurora::AuroraComponentInfo& info) override {
            panel = std::make_shared<aurora::AuroraPanel>(info, 900.f);

            auto& network_section = panel->addSection("NETWORK STATUS");
            network_section.addEntry("PORT", "9000");
            network_status = network_section.addEntry("STATUS", "DISCONNECTED", true, aurora::AuroraThemeSettings::get().ERROR);
            client_count = network_section.addEntry("CLIENTS", "0");

            auto& messages_section = panel->addSection("LAST MESSAGE");
            last_msg_type = messages_section.addEntry("TYPE", "-");
            last_msg_payload = messages_section.addEntry("PAYLOAD", "-");

            panel->addToRenderSystem();

            server.onConnect([this](aurora::debug::AuroraDebugSession& session) {
                connectedClients++;
                aurora::log::debug()->info("Client connected: {}", session.getAddress());
                updateNetworkStatus();
            });

            server.onDisconnect([this](const std::string& address) {
                if (connectedClients > 0) connectedClients--;
                aurora::log::debug()->info("Client disconnected: {}", address);
                updateNetworkStatus();
            });

            server.onMessage([this](aurora::debug::AuroraDebugSession&, const aurora::debug::DebugMessage& msg) {
                static const char* typeNames[] = {"LOG", "WATCH", "PROFILING", "CUSTOM"};
                auto idx = static_cast<uint8_t>(msg.type);
                const char* typeName = (idx < 4) ? typeNames[idx] : "UNKNOWN";
                aurora::log::debug()->info("[{}] {}", typeName, msg.payload);
                last_msg_type.setValue(typeName);
                last_msg_payload.setValue(msg.payload);
            });

            server.start();
        }

        void onUpdate(float dt) override {
            (void)dt;
            server.poll();
        }

    private:
        void updateNetworkStatus() {
            if (connectedClients > 0) {
                network_status.setValue("CONNECTED", aurora::AuroraThemeSettings::get().SUCCESS);
            } else if (connectedClients == 0) {
                network_status.setValue("DISCONNECTED", aurora::AuroraThemeSettings::get().ERROR);
            } else {
                network_status.setValue("LISTENING", aurora::AuroraThemeSettings::get().ORANGE);
            }
            client_count.setValue(std::to_string(connectedClients));
        }

        std::shared_ptr<aurora::AuroraPanel> panel;
        aurora::AuroraEntryHandle network_status;
        aurora::AuroraEntryHandle client_count;
        aurora::AuroraEntryHandle last_msg_type;
        aurora::AuroraEntryHandle last_msg_payload;

        aurora::debug::AuroraDebugServer server{9000};
        int connectedClients{0};
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
