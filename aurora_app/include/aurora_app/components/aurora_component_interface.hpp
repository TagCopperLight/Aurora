#pragma once

#include "aurora_app/graphics/aurora_model.hpp"
#include "aurora_engine/core/aurora_device.hpp"

#include <memory>

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
            
            // Apply transformations in the correct order: T * R * S
            // This ensures scale happens first, then rotation, then translation
            return translationMat * rotationMat * scaleMat;
        };
    };
    
    class AuroraComponentInterface {
        public:
            explicit AuroraComponentInterface(AuroraDevice &device) : auroraDevice{device} {}
            virtual ~AuroraComponentInterface() = default;

            virtual void initialize() {};
            virtual void update(float) {};

            bool isHidden() const { return hidden; }
            void setHidden(bool value) { hidden = value; }

            std::shared_ptr<AuroraModel> model{};
            glm::vec3 color{};
            TransformComponent transform{};

        protected:
            AuroraDevice &auroraDevice;

        private:
            bool hidden = false;

    };
}