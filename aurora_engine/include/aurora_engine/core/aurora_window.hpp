#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <functional>
#include <string>

namespace aurora {
    class AuroraWindow {
        public:
            AuroraWindow(int width, int height, std::string name);
            ~AuroraWindow();

            AuroraWindow(const AuroraWindow&) = delete;
            AuroraWindow& operator=(const AuroraWindow&) = delete;
            
            bool shouldClose() { return glfwWindowShouldClose(window); }
            void requestClose() { glfwSetWindowShouldClose(window, GLFW_TRUE); }
            VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
            bool wasWindowResized() { return framebufferResized; }
            void resetWindowResizedFlag() { framebufferResized = false; }

            void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

            // Called with the GLFW key code and modifier bits whenever a key is pressed or repeats
            void setKeyHandler(std::function<void(int key, int mods)> handler) { keyHandler = std::move(handler); }

            // Called with the GLFW button code and modifier bits whenever a mouse button is pressed
            void setMouseButtonHandler(std::function<void(int button, int mods)> handler) { mouseButtonHandler = std::move(handler); }

            // In framebuffer pixels, the units components are laid out in, which differ from screen
            // coordinates on scaled displays
            glm::vec2 getCursorPosition() const;

        private:
            static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
            static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
            void initWindow();

            int width;
            int height;
            bool framebufferResized = false;

            std::string windowName;
            GLFWwindow* window;
            std::function<void(int, int)> keyHandler;
            std::function<void(int, int)> mouseButtonHandler;
    };
}