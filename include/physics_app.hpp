#pragma once

#include "aurora_window.hpp"
#include "aurora_device.hpp"
#include "aurora_game_object.hpp"
#include "aurora_renderer.hpp"

#include <memory>
#include <vector>

namespace aurora {
    class PhysicsApp {
        public:
            static constexpr int WIDTH = 1200;
            static constexpr int HEIGHT = 900;

            PhysicsApp();
            ~PhysicsApp();

            PhysicsApp(const PhysicsApp&) = delete;
            PhysicsApp &operator=(const PhysicsApp&) = delete;

            void run();

        private:
            void sierpinski(std::vector<AuroraModel::Vertex>& vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top);
            void loadGameObjects(); 

            AuroraWindow auroraWindow{WIDTH, HEIGHT, "Physics App"};
            AuroraDevice auroraDevice{auroraWindow};
            AuroraRenderer auroraRenderer{auroraWindow, auroraDevice};

            std::vector<AuroraGameObject> gameObjects;
    };
}