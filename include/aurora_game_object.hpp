#pragma once

#include "aurora_model.hpp"

#include <memory>
#include <spdlog/spdlog.h>

namespace aurora {
    struct Transform2dComponent {
        glm::vec2 translation{};
        glm::vec2 scale{1.0f, 1.0f};
        float rotation;

        glm::mat2 mat2() {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);
            glm::mat2 rotMat = glm::mat2{{c, s}, {-s, c}};
            glm::mat2 scaleMat = glm::mat2{scale.x, 0.0f, 0.0f, scale.y};
            return rotMat * scaleMat;
        }
    };

    struct RigidBody2dComponent {
        glm::vec2 velocity;
        float mass{1.0f};
    };

    class AuroraGameObject {
        public:
            using id_t = unsigned int;

            static AuroraGameObject createGameObject() {
                static id_t currentId = 0;
                return AuroraGameObject{currentId++};
            }
            
            AuroraGameObject(const AuroraGameObject &) = delete;
            AuroraGameObject &operator=(const AuroraGameObject &) = delete;
            AuroraGameObject(AuroraGameObject &&) = default;
            AuroraGameObject &operator=(AuroraGameObject &&) = default;
            
            id_t getId() { return id; }

            std::shared_ptr<AuroraModel> model{};
            glm::vec3 color{};
            Transform2dComponent transform2d{};
            RigidBody2dComponent rigidBody2d{};

        private:
            AuroraGameObject(id_t objId) : id(objId) {}

            id_t id;
    };
}