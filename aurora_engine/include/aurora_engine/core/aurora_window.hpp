#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace aurora {
    class AuroraWindow {
        public:
            AuroraWindow(int width, int height, std::string name);
            ~AuroraWindow();

            AuroraWindow(const AuroraWindow&) = delete;
            AuroraWindow& operator=(const AuroraWindow&) = delete;
            
            bool shouldClose() { return glfwWindowShouldClose(window); }
            VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
            bool wasWindowResized() { return framebufferResized; }
            void resetWindowResizedFlag() { framebufferResized = false; }

            void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

        private:
            static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
            void initWindow();

            int width;
            int height;
            bool framebufferResized = false;

            std::string windowName;
            GLFWwindow* window;
    };
}