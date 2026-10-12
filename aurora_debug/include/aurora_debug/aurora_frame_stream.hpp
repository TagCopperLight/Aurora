#pragma once

#include "aurora_ui/components/aurora_panel.hpp"

#include <chrono>
#include <string>

namespace aurora::debug {

    // Receives frames streamed as MessageType::Custom and keeps only the latest one, since several
    // can arrive between two renders. Shows link statistics in a panel section: FRAMES (drawn),
    // FRAMES / S (received), LATENCY and NOT DRAWN (replaced before being drawn).
    // Latency is read from the frame's optional "sent" field, in unix seconds. It compares wall
    // clocks, so it is only meaningful when both programs run on the same machine.
    class AuroraFrameStream {
        public:
            explicit AuroraFrameStream(AuroraPanelSection& section, double windowSeconds = 0.5);

            // Call from AuroraDebugServer::onMessage for each frame
            void receive(const std::string& payload);

            // Moves the latest frame into `frame`. Returns false if none arrived since the last call.
            bool takeLatest(std::string& frame);

            // Drops the frame waiting to be drawn, e.g. when the sender disconnects
            void clear() { pending.clear(); }

            // Call once per frame; statistics are averaged over `windowSeconds`
            void update();

        private:
            struct Window {
                std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
                long received = 0;
                long dropped = 0;
                double latency = 0.0;
                long latencySamples = 0;
            };

            AuroraEntryHandle drawnEntry;
            AuroraEntryHandle rateEntry;
            AuroraEntryHandle latencyEntry;
            AuroraEntryHandle droppedEntry;

            std::string pending;
            long drawn = 0;
            long dropped = 0;
            double windowSeconds;
            Window window;
    };

}
