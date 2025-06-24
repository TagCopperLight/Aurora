#include "aurora_window.hpp"

#include <stdexcept>

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
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    }
        
    void AuroraWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface){
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }
}