#pragma once

#include "aurora_app/graphics/aurora_model.hpp"
#include "aurora_engine/core/aurora_device.hpp"

#include <memory>

namespace aurora {
    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec2 scale{1.f, 1.f};
        float rotation;

        glm::mat4 mat4() const {
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
                translation.x, translation.y, translation.z, 1.f
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

            virtual void update(float) {};
            
            bool isHidden() const { return hidden; }
            void setHidden(bool value) { hidden = value; }
            
            // Get shader file paths and topology for this component type
            virtual const std::string& getVertexShaderPath() const = 0;
            virtual const std::string& getFragmentShaderPath() const = 0;
            virtual VkPrimitiveTopology getTopology() const = 0;
            
            std::shared_ptr<AuroraModel> model{};
            glm::vec3 color{};
            TransformComponent transform{};

            virtual void addChild(std::unique_ptr<AuroraComponentInterface> child) {
                child->parent = this;
                children.push_back(std::move(child));
            }

            virtual void updateHierarchy(float deltaTime) {
                update(deltaTime);

                for (auto &child : children) {
                    child->update(deltaTime);
                    child->updateHierarchy(deltaTime);
                }
            }
            
            std::vector<std::unique_ptr<AuroraComponentInterface>>& getChildren() {
                return children;
            }
            
        protected:
            AuroraDevice &auroraDevice;
            std::vector<std::unique_ptr<AuroraComponentInterface>> children;
            AuroraComponentInterface* parent = nullptr;
            
        private:
            virtual void initialize() {};

            bool hidden = false;

    };
}