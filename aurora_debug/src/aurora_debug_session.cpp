#include "aurora_debug/aurora_debug_session.hpp"

#include "aurora_engine/utils/log.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

// Protocol: newline-delimited JSON messages.
// Each message is one line: <1-byte type><json payload>\n
// Example: \x00{"level":"warn","msg":"velocity is NaN"}\n

namespace aurora::debug {

    AuroraDebugSession::AuroraDebugSession(int socketFd, const std::string& address)
        : fd{socketFd}, connected{true}, address{address} {
        // Set socket to non-blocking so poll() never stalls the render loop.
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        aurora::log::debug()->info("Client connected: {}", address);
    }

    AuroraDebugSession::~AuroraDebugSession() {
        if (fd >= 0) {
            close(fd);
        }
    }

    bool AuroraDebugSession::isConnected() const {
        return connected;
    }

    const std::string& AuroraDebugSession::getAddress() const {
        return address;
    }

    bool AuroraDebugSession::poll(const std::function<void(const DebugMessage&)>& onMessage) {
        if (!connected) return false;

        char buf[4096];
        ssize_t n;

        while ((n = recv(fd, buf, sizeof(buf), 0)) > 0) {
            readBuffer.append(buf, static_cast<size_t>(n));
        }

        if (n == 0 || (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
            connected = false;
            return false;
        }

        // Parse complete lines from the buffer.
        std::size_t pos;
        while ((pos = readBuffer.find('\n')) != std::string::npos) {
            std::string line = readBuffer.substr(0, pos);
            readBuffer.erase(0, pos + 1);

            if (line.empty()) continue;

            // TODO: first byte is the MessageType, remainder is the JSON payload.
            DebugMessage msg;
            msg.type    = static_cast<MessageType>(static_cast<uint8_t>(line[0]));
            msg.payload = line.substr(1);

            if (onMessage) onMessage(msg);
        }

        return true;
    }

} // namespace aurora::debug