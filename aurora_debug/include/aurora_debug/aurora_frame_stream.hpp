#pragma once

#include "aurora_ui/components/aurora_panel.hpp"

#include <chrono>
#include <deque>
#include <string>

namespace aurora::debug {

    // Receives frames streamed as MessageType::Custom and keeps only the latest one, since several
    // can arrive between two renders. Shows link statistics in a panel section: FRAMES (drawn),
    // FRAMES / S (received), LATENCY and NOT DRAWN (replaced before being drawn).
    // Latency is read from the frame's optional "sent" field, in unix seconds. It compares wall
    // clocks, so it is only meaningful when both programs run on the same machine.
    class AuroraFrameStream {
        public:
            explicit AuroraFrameStream(AuroraPanelSection& section, double windowSeconds = 0.0, double spanSeconds = 0.5);

            // Call from AuroraDebugServer::onMessage for each frame
            void receive(const std::string& payload);

            // Moves the latest frame into `frame`. Returns false if none arrived since the last call.
            bool takeLatest(std::string& frame);

            // Drops the frame waiting to be drawn, e.g. when the sender disconnects
            void clear() { pending.clear(); }

            // Call once per frame; statistics are computed over the last `spanSeconds`
            void update();

            // Time between two refreshes of the statistics; 0 refreshes them every frame
            void setWindowSeconds(double seconds) { windowSeconds = seconds; }

            // Sliding span the statistics are computed over. Frames arrive at the sender's rate, so it
            // must hold several of them for FRAMES / S and LATENCY to be meaningful.
            void setSpanSeconds(double seconds) { spanSeconds = seconds; }

        private:
            using Clock = std::chrono::steady_clock;

            struct LatencySample {
                Clock::time_point at;
                double ms;
            };

            AuroraEntryHandle drawnEntry;
            AuroraEntryHandle rateEntry;
            AuroraEntryHandle latencyEntry;
            AuroraEntryHandle droppedEntry;

            std::string pending;
            long drawn = 0;
            long dropped = 0;
            double windowSeconds;
            double spanSeconds;
            Clock::time_point created = Clock::now();
            Clock::time_point lastRefresh = created;
            std::deque<Clock::time_point> received;
            std::deque<LatencySample> latencies;
    };

}
