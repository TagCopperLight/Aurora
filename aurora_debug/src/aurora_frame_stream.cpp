#include "aurora_debug/aurora_frame_stream.hpp"
#include "aurora_debug/aurora_debug_json.hpp"
#include "aurora_ui/utils/aurora_utils.hpp"

#include <algorithm>

namespace aurora::debug {

    AuroraFrameStream::AuroraFrameStream(AuroraPanelSection& section, double windowSeconds, double spanSeconds)
        : windowSeconds{windowSeconds}, spanSeconds{spanSeconds} {
        drawnEntry = section.addEntry("FRAMES", "0");
        rateEntry = section.addEntry("FRAMES / S", "-");
        latencyEntry = section.addEntry("LATENCY", "-");
        droppedEntry = section.addEntry("NOT DRAWN", "0");
    }

    void AuroraFrameStream::receive(const std::string& payload) {
        if (!pending.empty()) dropped++;
        pending = payload;
        received.push_back(Clock::now());
    }

    bool AuroraFrameStream::takeLatest(std::string& frame) {
        if (pending.empty()) return false;
        frame.swap(pending);
        pending.clear();

        double sent = json::getNumber(frame, "sent", -1.0);
        if (sent >= 0.0) {
            double now = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
            latencies.push_back({Clock::now(), (now - sent) * 1000.0});
        }

        drawn++;
        drawnEntry.setValue(std::to_string(drawn));
        return true;
    }

    void AuroraFrameStream::update() {
        auto now = Clock::now();
        if (std::chrono::duration<double>(now - lastRefresh).count() < windowSeconds) return;
        lastRefresh = now;

        auto spanStart = now - std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(spanSeconds));
        while (!received.empty() && received.front() < spanStart) received.pop_front();
        while (!latencies.empty() && latencies.front().at < spanStart) latencies.pop_front();

        // Right after startup, less than a full span has been observed
        double span = std::min(spanSeconds, std::chrono::duration<double>(now - created).count());
        rateEntry.setValue(span > 0.0 ? AuroraUtils::format("%.1f", received.size() / span) : "-");

        if (latencies.empty()) {
            latencyEntry.setValue("-");
        } else {
            double total = 0.0;
            for (const auto& sample : latencies) total += sample.ms;
            latencyEntry.setValue(AuroraUtils::format("%.2f ms", total / latencies.size()));
        }

        droppedEntry.setValue(std::to_string(dropped));
    }

}
