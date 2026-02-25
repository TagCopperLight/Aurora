#pragma once

#include "aurora_ui/graphics/aurora_model.hpp"
#include "aurora_ui/components/aurora_component_info.hpp"

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace aurora {
    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec2 scale{1.f, 1.f};
        float rotation = 0.0f;

        glm::mat4 mat4() const {
            glm::mat4 transform = glm::mat4(1.0f);
            transform = glm::translate(transform, translation);
            transform = glm::rotate(transform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
            transform = glm::scale(transform, glm::vec3(scale.x, scale.y, 1.0f));
            return transform;
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
                child->parent = weak_from_this();
                children.push_back(child);

                if (rendering) {
                    child->addToRenderSystem();
                }
            }

            virtual void addToRenderSystem();

            glm::mat4 getWorldTransform() const {
                return worldTransform;
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
            std::weak_ptr<AuroraComponentInterface> parent;
            
        private:
            virtual void initialize() {};

            bool hidden = false;
            bool rendering = false;

            glm::mat4 worldTransform{1.0f};

            void updateWorldTransform() {
                glm::mat4 localTransform = transform.mat4();
                
                if (auto parentPtr = parent.lock()) {
                    float ourZDepth = localTransform[3][2];
                    worldTransform = parentPtr->getWorldTransform() * localTransform;
                    worldTransform[3][2] = ourZDepth;
                } else {
                    worldTransform = localTransform;
                }
                
                for (const auto& child : children) {
                    child->updateWorldTransform();
                }
            }

    };
}