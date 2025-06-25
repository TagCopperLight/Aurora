#include "aurora_window.hpp"

#include <stdexcept>
#include <spdlog/spdlog.h>

namespace aurora {
    AuroraWindow::AuroraWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
        spdlog::info("Creating Aurora Window: {}x{} - '{}'", width, height, windowName);
        initWindow();
        spdlog::info("Aurora Window created successfully");
    }

    AuroraWindow::~AuroraWindow() {
        spdlog::info("Destroying Aurora Window");
        glfwDestroyWindow(window);
        glfwTerminate();
        spdlog::info("Aurora Window destroyed");
    }

    void AuroraWindow::initWindow() {
        spdlog::info("Initializing GLFW window");
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        spdlog::info("GLFW window initialized");
    }
        
    void AuroraWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface){
        spdlog::info("Creating window surface");
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        spdlog::info("Window surface created successfully");
    }
}