#include "aurora_ui/aurora_ui.hpp"
#include "aurora_ui/components/aurora_panel.hpp"
#include "aurora_ui/profiling/aurora_performance_monitor.hpp"
#include "aurora_debug/aurora_connection_monitor.hpp"
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
        void onSetup() override {
            panel = create<aurora::AuroraPanel>(900.f);

            connection = std::make_unique<aurora::debug::AuroraConnectionMonitor>(server, panel->addSection("NETWORK STATUS"));
            performance = std::make_unique<aurora::AuroraPerformanceMonitor>(panel->addSection("PERFORMANCE"));

            // Watches are listed below, in sections created as they first arrive

            server.onConnect([](aurora::debug::AuroraDebugSession& session) {
                aurora::log::debug()->info("Client connected: {}", session.getAddress());
            });

            server.onDisconnect([](const std::string& address) {
                aurora::log::debug()->info("Client disconnected: {}", address);
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
            connection->update();
            performance->update();
        }

    private:
        std::shared_ptr<aurora::AuroraPanel> panel;
        std::unique_ptr<aurora::debug::AuroraConnectionMonitor> connection;
        std::unique_ptr<aurora::AuroraPerformanceMonitor> performance;

        aurora::debug::AuroraDebugServer server{9000};
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
