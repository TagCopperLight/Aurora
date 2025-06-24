#include "aurora_app.hpp"

namespace aurora {
    void AuroraApp::run() {
        while (!auroraWindow.shouldClose()) {
            glfwPollEvents();
        }
    }
}