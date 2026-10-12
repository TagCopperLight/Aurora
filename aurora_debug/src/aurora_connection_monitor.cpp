#include "aurora_debug/aurora_connection_monitor.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"

#include <string>

namespace aurora::debug {

    AuroraConnectionMonitor::AuroraConnectionMonitor(const AuroraDebugServer& server, AuroraPanelSection& section)
        : server{server} {
        const auto& theme = AuroraThemeSettings::get();
        section.addEntry("PORT", std::to_string(server.getPort()));
        status = section.addEntry("STATUS", "STOPPED", true, theme.ERROR);
        clients = section.addEntry("CLIENTS", "0");
    }

    void AuroraConnectionMonitor::update() {
        bool running = server.isRunning();
        size_t count = server.getClientCount();
        if (shown && running == shownRunning && count == shownClients) return;

        const auto& theme = AuroraThemeSettings::get();
        if (!running) {
            status.setValue("STOPPED", theme.ERROR);
        } else if (count == 0) {
            status.setValue("WAITING", theme.ORANGE);
        } else {
            status.setValue("CONNECTED", theme.SUCCESS);
        }
        clients.setValue(std::to_string(count));

        shown = true;
        shownRunning = running;
        shownClients = count;
    }

}
