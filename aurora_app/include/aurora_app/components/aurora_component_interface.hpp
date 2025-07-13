#pragma once

#include "aurora_app/graphics/aurora_model.hpp"
#include "aurora_app/components/aurora_component_info.hpp"

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
            
            return translationMat * rotationMat * scaleMat;
        };
    };
    
    class AuroraComponentInterface {
        public:
            explicit AuroraComponentInterface(AuroraComponentInfo &componentInfo) : componentInfo{componentInfo} {}
            virtual ~AuroraComponentInterface() = default;

            virtual void update(float) {};
            
            bool isHidden() const { return hidden; }
            void setHidden(bool value) { hidden = value; }
            
            virtual const std::string& getVertexShaderPath() const = 0;
            virtual const std::string& getFragmentShaderPath() const = 0;
            virtual VkPrimitiveTopology getTopology() const = 0;
            
            std::shared_ptr<AuroraModel> model{};
            glm::vec4 color{};
            TransformComponent transform{};

            virtual void addChild(std::shared_ptr<AuroraComponentInterface> child) {
                child->parent = this;
                children.push_back(child);
            }

            virtual void updateHierarchy(float deltaTime) {
                update(deltaTime);

                for (auto &child : children) {
                    child->update(deltaTime);
                    child->updateHierarchy(deltaTime);
                }
            }
            
            std::vector<std::shared_ptr<AuroraComponentInterface>>& getChildren() {
                return children;
            }
            
            void setDepth(float depth) {
                transform.translation.z = depth;
            }
            
            void setPosition(float x, float y) {
                transform.translation.x = x;
                transform.translation.y = y;
            }
            
            void setPosition(const glm::vec2& position) {
                setPosition(position.x, position.y);
            }
            
            glm::mat4 getWorldTransform() const {
                glm::mat4 worldTransform = transform.mat4();
                
                if (parent != nullptr) {
                    glm::mat4 parentTransform = parent->getWorldTransform();
                    
                    float ourZDepth = worldTransform[3][2];
                    worldTransform = parentTransform * worldTransform;
                    worldTransform[3][2] = ourZDepth;
                }
                
                return worldTransform;
            }
            
            void clearParent() {
                parent = nullptr;
            }
            
            AuroraComponentInterface* getParent() const {
                return parent;
            }
            
        protected:
            AuroraComponentInfo &componentInfo;
            std::vector<std::shared_ptr<AuroraComponentInterface>> children;
            AuroraComponentInterface* parent = nullptr;
            
        private:
            virtual void initialize() {};

            bool hidden = false;

    };
}