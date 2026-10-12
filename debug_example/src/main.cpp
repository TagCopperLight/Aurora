#include "aurora_ui/aurora_ui.hpp"
#include "aurora_ui/components/aurora_panel.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"
#include "aurora_debug/aurora_debug_server.hpp"

#include "aurora_engine/utils/log.hpp"
#include "aurora_engine/profiling/aurora_profiler.hpp"
#include <fontconfig/fontconfig.h>
#include <cstdio>
#include <unordered_map>
#include <string>

static std::string jsonExtract(const std::string& json, const std::string& key) {
    auto pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return {};
    pos = json.find(':', pos);
    if (pos == std::string::npos) return {};
    ++pos;
    while (pos < json.size() && json[pos] == ' ') ++pos;
    if (pos >= json.size()) return {};
    if (json[pos] == '"') {
        ++pos;
        auto end = json.find('"', pos);
        return (end == std::string::npos) ? json.substr(pos) : json.substr(pos, end - pos);
    }
    auto end = json.find_first_of(",}", pos);
    return json.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
}

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

            auto& perf_section = panel->addSection("PERFORMANCE");
            perf_fps = perf_section.addEntry("FPS", "-");
            perf_frame = perf_section.addEntry("FRAME TIME", "-");
            perf_cpu = perf_section.addEntry("CPU WORK", "-");
            perf_gpu = perf_section.addEntry("GPU FRAME", "-");
            perf_draw_calls = perf_section.addEntry("DRAW CALLS", "-");

            watches_section = &panel->addSection("WATCHES");

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
                if (msg.type == aurora::debug::MessageType::Watch) {
                    handleWatch(msg.payload);
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
            auto& profiler = aurora::AuroraProfiler::instance();

            double frame_time = profiler.getFrameTime();

            // CPU-side work: every phase except time spent blocked on the GPU
            double cpu_work = profiler.getStats("Poll Events").current + profiler.getStats("Update").current
                            + profiler.getStats("Begin Frame").current - profiler.getStats("GPU Wait").current
                            + profiler.getStats("Render Components").current + profiler.getStats("End Frame").current;

            perf_fps.setValue(formatValue("%.0f", frame_time > 0.0 ? 1000.0 / frame_time : 0.0));
            perf_frame.setValue(formatValue("%.2f ms", frame_time));
            perf_cpu.setValue(formatValue("%.3f ms", cpu_work));
            perf_gpu.setValue(formatValue("%.3f ms", profiler.getStats("GPU Frame").current));
            perf_draw_calls.setValue(std::to_string(profiler.getCounter("Draw Calls")));
        }

    private:
        static std::string formatValue(const char* fmt, double value) {
            char buffer[32];
            std::snprintf(buffer, sizeof(buffer), fmt, value);
            return buffer;
        }

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

        void handleWatch(const std::string& payload) {
            auto name  = jsonExtract(payload, "name");
            auto value = jsonExtract(payload, "value");
            if (name.empty()) return;

            auto it = watch_entries.find(name);
            if (it == watch_entries.end()) {
                watch_entries[name] = watches_section->addEntry(name, value);
            } else {
                it->second.setValue(value);
            }
        }

        std::shared_ptr<aurora::AuroraPanel> panel;
        aurora::AuroraEntryHandle network_status;
        aurora::AuroraEntryHandle client_count;

        aurora::AuroraEntryHandle perf_fps;
        aurora::AuroraEntryHandle perf_frame;
        aurora::AuroraEntryHandle perf_cpu;
        aurora::AuroraEntryHandle perf_gpu;
        aurora::AuroraEntryHandle perf_draw_calls;

        aurora::AuroraPanelSection* watches_section{nullptr};
        std::unordered_map<std::string, aurora::AuroraEntryHandle> watch_entries;

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
