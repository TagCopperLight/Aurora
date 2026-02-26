#include "aurora_debug/aurora_debug_server.hpp"

#include "aurora_engine/utils/log.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

namespace aurora::debug {

    AuroraDebugServer::AuroraDebugServer(uint16_t port) : port{port} {}

    AuroraDebugServer::~AuroraDebugServer() {
        stop();
    }

    void AuroraDebugServer::start() {
        serverFd = socket(AF_INET, SOCK_STREAM, 0);
        if (serverFd < 0) {
            aurora::log::debug()->error("Failed to create server socket");
            return;
        }

        // Allow immediate reuse of the port after restart.
        int opt = 1;
        setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        // Non-blocking accept loop.
        int flags = fcntl(serverFd, F_GETFL, 0);
        fcntl(serverFd, F_SETFL, flags | O_NONBLOCK);

        sockaddr_in addr{};
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port        = htons(port);

        if (bind(serverFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            aurora::log::debug()->error("Failed to bind on port {}", port);
            close(serverFd);
            serverFd = -1;
            return;
        }

        listen(serverFd, 8);
        running = true;
        aurora::log::debug()->info("Server listening on port {}", port);
    }

    void AuroraDebugServer::stop() {
        if (!running) return;
        running = false;
        sessions.clear();
        if (serverFd >= 0) {
            close(serverFd);
            serverFd = -1;
        }
        aurora::log::debug()->info("Server stopped");
    }

    void AuroraDebugServer::poll() {
        if (!running) return;
        acceptConnections();
        pollSessions();
    }

    void AuroraDebugServer::acceptConnections() {
        sockaddr_in clientAddr{};
        socklen_t len = sizeof(clientAddr);
        int clientFd;

        while ((clientFd = accept(serverFd, reinterpret_cast<sockaddr*>(&clientAddr), &len)) >= 0) {
            std::string address = std::string(inet_ntoa(clientAddr.sin_addr))
                                + ":" + std::to_string(ntohs(clientAddr.sin_port));

            auto session = std::make_unique<AuroraDebugSession>(clientFd, address);
            if (connectHandler) connectHandler(*session);
            sessions.push_back(std::move(session));
        }
    }

    void AuroraDebugServer::pollSessions() {
        auto it = sessions.begin();
        while (it != sessions.end()) {
            auto& session = **it;
            bool alive = session.poll([&](const DebugMessage& msg) {
                if (messageHandler) messageHandler(session, msg);
            });

            if (!alive) {
                if (disconnectHandler) disconnectHandler(session.getAddress());
                it = sessions.erase(it);
            } else {
                ++it;
            }
        }
    }

    bool AuroraDebugServer::isRunning() const { return running; }
    uint16_t AuroraDebugServer::getPort() const { return port; }

    void AuroraDebugServer::onConnect(std::function<void(AuroraDebugSession&)> handler) {
        connectHandler = std::move(handler);
    }

    void AuroraDebugServer::onDisconnect(std::function<void(const std::string&)> handler) {
        disconnectHandler = std::move(handler);
    }

    void AuroraDebugServer::onMessage(std::function<void(AuroraDebugSession&, const DebugMessage&)> handler) {
        messageHandler = std::move(handler);
    }

} // namespace aurora::debug