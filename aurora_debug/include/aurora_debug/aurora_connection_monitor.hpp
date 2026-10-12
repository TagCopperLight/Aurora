#pragma once

#include "aurora_debug_server.hpp"
#include "aurora_ui/components/aurora_panel.hpp"

namespace aurora::debug {

    // Shows the state of an AuroraDebugServer in a panel section: PORT, STATUS and CLIENTS.
    class AuroraConnectionMonitor {
        public:
            AuroraConnectionMonitor(const AuroraDebugServer& server, AuroraPanelSection& section);

            // Call once per frame
            void update();

        private:
            const AuroraDebugServer& server;
            AuroraEntryHandle status;
            AuroraEntryHandle clients;

            bool shownRunning = false;
            size_t shownClients = 0;
            bool shown = false;
    };

}
