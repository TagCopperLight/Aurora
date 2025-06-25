#include "aurora_window.hpp"

#include <stdexcept>
#include <spdlog/spdlog.h>

namespace aurora {
    AuroraWindow::AuroraWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
        spdlog::debug("Creating Aurora Window: {}x{} - '{}'", width, height, windowName);
        initWindow();
        spdlog::debug("Aurora Window created successfully");
    }

    AuroraWindow::~AuroraWindow() {
        spdlog::debug("Destroying Aurora Window");
        glfwDestroyWindow(window);
        glfwTerminate();
        spdlog::debug("Aurora Window destroyed");
    }

    void AuroraWindow::initWindow() {
        spdlog::debug("Initializing GLFW window");
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        spdlog::debug("GLFW window initialized");
    }
        
    void AuroraWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface){
        spdlog::debug("Creating window surface");
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        spdlog::debug("Window surface created successfully");
    }
}