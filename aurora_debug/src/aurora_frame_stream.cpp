#include "aurora_debug/aurora_frame_stream.hpp"
#include "aurora_debug/aurora_debug_json.hpp"
#include "aurora_ui/utils/aurora_utils.hpp"

namespace aurora::debug {

    AuroraFrameStream::AuroraFrameStream(AuroraPanelSection& section, double windowSeconds)
        : windowSeconds{windowSeconds} {
        drawnEntry = section.addEntry("FRAMES", "0");
        rateEntry = section.addEntry("FRAMES / S", "-");
        latencyEntry = section.addEntry("LATENCY", "-");
        droppedEntry = section.addEntry("NOT DRAWN", "0");
    }

    void AuroraFrameStream::receive(const std::string& payload) {
        if (!pending.empty()) window.dropped++;
        pending = payload;
        window.received++;
    }

    bool AuroraFrameStream::takeLatest(std::string& frame) {
        if (pending.empty()) return false;
        frame.swap(pending);
        pending.clear();

        double sent = json::getNumber(frame, "sent", -1.0);
        if (sent >= 0.0) {
            double now = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
            window.latency += (now - sent) * 1000.0;
            window.latencySamples++;
        }

        drawn++;
        drawnEntry.setValue(std::to_string(drawn));
        return true;
    }

    void AuroraFrameStream::update() {
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - window.start).count();
        if (elapsed < windowSeconds) return;

        rateEntry.setValue(AuroraUtils::format("%.1f", window.received / elapsed));
        latencyEntry.setValue(window.latencySamples > 0 ? AuroraUtils::format("%.2f ms", window.latency / window.latencySamples) : "-");
        dropped += window.dropped;
        droppedEntry.setValue(std::to_string(dropped));

        window = Window{};
        window.start = now;
    }

}
