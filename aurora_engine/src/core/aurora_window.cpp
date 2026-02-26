#include "aurora_engine/core/aurora_window.hpp"

#include <stdexcept>
#include "aurora_engine/utils/log.hpp"

namespace aurora {
    AuroraWindow::AuroraWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
        initWindow();
    }

    AuroraWindow::~AuroraWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void AuroraWindow::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        
        width = mode->width;
        height = mode->height;

        log::engine()->info("Creating window: {} with size {}x{}", windowName, width, height);

        window = glfwCreateWindow(width, height, windowName.c_str(), monitor, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }
        
    void AuroraWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void AuroraWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto auroraWindow = reinterpret_cast<AuroraWindow *>(glfwGetWindowUserPointer(window));
        auroraWindow->framebufferResized = true;
        auroraWindow->width = width;
        auroraWindow->height = height;
    }
}