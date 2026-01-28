#pragma once

#include "aurora_app/graphics/aurora_model.hpp"
#include "aurora_app/components/aurora_component_info.hpp"

#include <memory>

namespace aurora {
    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec2 scale{1.f, 1.f};
        float rotation = 0.0f;

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
        }
    };
    
    class AuroraComponentInterface : public std::enable_shared_from_this<AuroraComponentInterface> {
        public:
            explicit AuroraComponentInterface(AuroraComponentInfo &componentInfo) : componentInfo{componentInfo} {
                updateWorldTransform();
            }
            virtual ~AuroraComponentInterface() = default;

            virtual void update(float) {};
            
            bool isHidden() const { return hidden; }
            void setHidden(bool value) { hidden = value; }
            
            virtual const std::string& getVertexShaderPath() const = 0;
            virtual const std::string& getFragmentShaderPath() const = 0;
            virtual VkPrimitiveTopology getTopology() const = 0;
            
            virtual bool needsTextureBinding() const {
                return false;
            }
            
            virtual bool isTransparent() const {
                return false;
            }
            
            std::shared_ptr<AuroraModel> model{};
            glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            TransformComponent transform{};

            virtual void addChild(std::shared_ptr<AuroraComponentInterface> child) {
                child->parent = this;
                children.push_back(child);

                if (rendering) {
                    child->addToRenderSystem();
                }
            }

            virtual void addToRenderSystem();

            glm::mat4 getWorldTransform() const {
                return worldTransform;
            }

            glm::mat4 getMVPMatrix(const glm::mat4& projectionMatrix) {
                if (worldTransformDirty) {
                    mvpMatrix = projectionMatrix * worldTransform;
                    worldTransformDirty = false;
                }
                return mvpMatrix;
            }

            std::vector<std::shared_ptr<AuroraComponentInterface>>& getChildren() {
                return children;
            }
            
            void setDepth(float depth) {
                if (transform.translation.z != depth) {
                    transform.translation.z = depth;
                    updateWorldTransform();
                }
            }
            
            void setPosition(float x, float y) {
                if (transform.translation.x != x || transform.translation.y != y) {
                    transform.translation.x = x;
                    transform.translation.y = y;
                    updateWorldTransform();
                }
            }
            
            void setPosition(const glm::vec2& position) {
                setPosition(position.x, position.y);
            }

            void setScale(float x, float y) {
                if (transform.scale.x != x || transform.scale.y != y) {
                    transform.scale.x = x;
                    transform.scale.y = y;
                    updateWorldTransform();
                }
            }

            void setScale(const glm::vec2& scale) {
                setScale(scale.x, scale.y);
            }

            void setRotation(float rotation) {
                if (transform.rotation != rotation) {
                    transform.rotation = rotation;
                    updateWorldTransform();
                }
            }

            void setRendering(bool value) {
                rendering = value;
            }
            
        protected:
            AuroraComponentInfo &componentInfo;
            std::vector<std::shared_ptr<AuroraComponentInterface>> children;
            AuroraComponentInterface* parent = nullptr;
            
        private:
            virtual void initialize() {};

            bool hidden = false;
            bool rendering = false;

            glm::mat4 worldTransform{1.0f};
            glm::mat4 mvpMatrix{1.0f};
            mutable bool worldTransformDirty = true;

            void updateWorldTransform() {
                glm::mat4 localTransform = transform.mat4();
                
                if (parent != nullptr) {
                    float ourZDepth = localTransform[3][2];
                    worldTransform = parent->getWorldTransform() * localTransform;
                    worldTransform[3][2] = ourZDepth;
                } else {
                    worldTransform = localTransform;
                }
                
                worldTransformDirty = true;
                
                for (const auto& child : children) {
                    child->updateWorldTransform();
                }
            }

    };
}