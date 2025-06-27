#pragma once

#include "graphics/aurora_model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <spdlog/spdlog.h>

namespace aurora {
    struct TransformComponent {
        glm::vec2 translation{};
        glm::vec2 scale{1.f, 1.f};
        float rotation;

        glm::mat4 mat4() {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);
            
            glm::mat4 rotationMat = glm::mat4(
                c, s, 0.f, 0.f,
                -s, c, 0.f, 0.f,
                0.f, 0.f, 1.f, 0.f,
                0.f, 0.f, 0.f, 1.f
            );
            
            glm::mat4 scaleMat = glm::mat4(
                scale.x, 0.f, 0.f, 0.f,
                0.f, scale.y, 0.f, 0.f,
                0.f, 0.f, 1.f, 0.f,
                0.f, 0.f, 0.f, 1.f
            );
            
            glm::mat4 translationMat = glm::mat4(
                1.f, 0.f, 0.f, 0.f,
                0.f, 1.f, 0.f, 0.f,
                0.f, 0.f, 1.f, 0.f,
                translation.x, translation.y, 0.f, 1.f
            );
            
            return translationMat * rotationMat * scaleMat;
        };
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
            TransformComponent transform{};

        private:
            AuroraGameObject(id_t objId) : id(objId) {}

            id_t id;
    };
}