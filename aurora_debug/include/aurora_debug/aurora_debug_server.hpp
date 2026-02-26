#pragma once

#include "aurora_debug_session.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace aurora::debug {

    // Listens for incoming connections from programs being debugged.
    // Call poll() once per frame to accept connections and dispatch messages
    // without blocking the render loop.
    class AuroraDebugServer {
        public:
            explicit AuroraDebugServer(uint16_t port = 9000);
            ~AuroraDebugServer();

            AuroraDebugServer(const AuroraDebugServer&) = delete;
            AuroraDebugServer& operator=(const AuroraDebugServer&) = delete;

            void start();
            void stop();

            // Call once per frame.
            void poll();

            bool isRunning() const;
            uint16_t getPort() const;

            // Callbacks — set before calling start().
            void onConnect(std::function<void(AuroraDebugSession&)> handler);
            void onDisconnect(std::function<void(const std::string& address)> handler);
            void onMessage(std::function<void(AuroraDebugSession&, const DebugMessage&)> handler);

        private:
            void acceptConnections();
            void pollSessions();

            uint16_t port;
            int serverFd{-1};
            bool running{false};

            std::vector<std::unique_ptr<AuroraDebugSession>> sessions;

            std::function<void(AuroraDebugSession&)> connectHandler;
            std::function<void(const std::string&)> disconnectHandler;
            std::function<void(AuroraDebugSession&, const DebugMessage&)> messageHandler;
    };

}