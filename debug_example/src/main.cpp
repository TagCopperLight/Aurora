#include "aurora_ui/aurora_ui.hpp"
#include "aurora_ui/components/aurora_panel.hpp"
#include "aurora_ui/profiling/aurora_performance_monitor.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"
#include "aurora_debug/aurora_debug_json.hpp"
#include "aurora_debug/aurora_debug_server.hpp"

#include "aurora_engine/utils/log.hpp"
#include <memory>
#include <string>

class DebugApp : public aurora::AuroraUI {
    public:
        DebugApp() : AuroraUI{"Aurora Debug"} {
            setFrameRateLimit(false);
        }

    protected:
        void onSetup(aurora::AuroraComponentInfo& info) override {
            panel = std::make_shared<aurora::AuroraPanel>(info, 900.f);

            auto& network_section = panel->addSection("NETWORK STATUS");
            network_section.addEntry("PORT", "9000");
            network_status = network_section.addEntry("STATUS", "DISCONNECTED", true, aurora::AuroraThemeSettings::get().ERROR);
            client_count = network_section.addEntry("CLIENTS", "0");

            performance = std::make_unique<aurora::AuroraPerformanceMonitor>(panel->addSection("PERFORMANCE"));

            // Watches are listed below, in sections created as they first arrive

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
                aurora::debug::WatchMessage watch;
                if (msg.type == aurora::debug::MessageType::Watch) {
                    if (aurora::debug::WatchMessage::parse(msg.payload, watch)) {
                        panel->getSection(watch.section).setEntry(watch.name, watch.value);
                    }
                } else {
                    static const char* typeNames[] = {"LOG", "WATCH", "PROFILING", "CUSTOM"};
                    auto idx = static_cast<uint8_t>(msg.type);
                    aurora::log::debug()->info("[{}] {}", (idx < 4) ? typeNames[idx] : "UNKNOWN", msg.payload);
                }
            });

            server.start();
        }

        void onUpdate(float /*dt*/) override {
            server.poll();
        }

        void onFrameEnd() override {
            performance->update();
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
        std::unique_ptr<aurora::AuroraPerformanceMonitor> performance;

        aurora::debug::AuroraDebugServer server{9000};
        int connectedClients{0};
};

int main() {
    aurora::log::init(spdlog::level::debug);

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
