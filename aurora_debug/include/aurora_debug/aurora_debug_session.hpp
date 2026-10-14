#pragma once

#include "aurora_debug_message.hpp"

#include <functional>
#include <string>

namespace aurora::debug {

    // Represents one connected client (a program being debugged).
    // Programs act as clients; they connect to the AuroraDebugServer.
    class AuroraDebugSession {
        public:
            explicit AuroraDebugSession(int socketFd, const std::string& address);
            ~AuroraDebugSession();

            AuroraDebugSession(const AuroraDebugSession&) = delete;
            AuroraDebugSession& operator=(const AuroraDebugSession&) = delete;

            // Non-blocking: drains pending socket data and fires onMessage for
            // each complete message received. Returns false if the connection
            // was closed and the session should be removed.
            bool poll(const std::function<void(const DebugMessage&)>& onMessage);

            // Queues a message for the client and sends as much of it as the socket accepts.
            // Whatever is left is sent by the next poll(), so this never blocks either.
            void send(const DebugMessage& msg);

            bool isConnected() const;
            const std::string& getAddress() const;

        private:
            void flush();

            int fd;
            bool connected;
            std::string address;
            std::string readBuffer;  // accumulates partial data between polls
            std::string writeBuffer; // data queued by send() that the socket has not taken yet
    };

} // namespace aurora::debug