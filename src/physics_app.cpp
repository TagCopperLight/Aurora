#include "physics_app.hpp"

#include "aurora_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>


namespace aurora {
    class GravityPhysicsSystem {
        public:
            GravityPhysicsSystem(float strength) : strengthGravity{strength} {}

            const float strengthGravity;

            void update(std::vector<AuroraGameObject>& objs, float dt, unsigned int substeps = 1) {
                const float stepDelta = dt / substeps;
                for (int i = 0; i < substeps; i++) {
                    stepSimulation(objs, stepDelta);
                }
            }

            glm::vec2 computeForce(AuroraGameObject& fromObj, AuroraGameObject& toObj) const {
                auto offset = fromObj.transform2d.translation - toObj.transform2d.translation;
                float distanceSquared = glm::dot(offset, offset);

                // clown town - just going to return 0 if objects are too close together...
                if (glm::abs(distanceSquared) < 1e-10f) {
                    return {.0f, .0f};
                }

                float force = strengthGravity * toObj.rigidBody2d.mass * fromObj.rigidBody2d.mass / distanceSquared;
                return force * offset / glm::sqrt(distanceSquared);
            }

    private:
        void stepSimulation(std::vector<AuroraGameObject>& physicsObjs, float dt) {
            for (auto iterA = physicsObjs.begin(); iterA != physicsObjs.end(); ++iterA) {
                auto& objA = *iterA;
                for (auto iterB = iterA; iterB != physicsObjs.end(); ++iterB) {
                    if (iterA == iterB) continue;
                    auto& objB = *iterB;

                    auto force = computeForce(objA, objB);
                    objA.rigidBody2d.velocity += dt * -force / objA.rigidBody2d.mass;
                    objB.rigidBody2d.velocity += dt * force / objB.rigidBody2d.mass;
                }
            }

            for (auto& obj : physicsObjs) {
                obj.transform2d.translation += dt * obj.rigidBody2d.velocity;
            }
        }
    };

    class Vec2FieldSystem {
        public:
            void update(const GravityPhysicsSystem& physicsSystem, std::vector<AuroraGameObject>& physicsObjs, std::vector<AuroraGameObject>& vectorField) {
                for (auto& vf : vectorField) {
                    glm::vec2 direction{};
                    for (auto& obj : physicsObjs) {
                        direction += physicsSystem.computeForce(obj, vf);
                    }

                    vf.transform2d.scale.x = 0.005f + 0.045f * glm::clamp(glm::log(glm::length(direction) + 1) / 3.f, 0.f, 1.f);
                    vf.transform2d.rotation = atan2(direction.y, direction.x);
                }
            }
    };

    std::unique_ptr<AuroraModel> createSquareModel(AuroraDevice& device, glm::vec2 offset) {
        std::vector<AuroraModel::Vertex> vertices = {
            {{-0.5f, -0.5f}},
            {{0.5f, 0.5f}},
            {{-0.5f, 0.5f}},
            {{-0.5f, -0.5f}},
            {{0.5f, -0.5f}},
            {{0.5f, 0.5f}},
        };
        for (auto& v : vertices) {
            v.position += offset;
        }
        return std::make_unique<AuroraModel>(device, vertices);
    }

    std::unique_ptr<AuroraModel> createCircleModel(AuroraDevice& device, unsigned int numSides) {
        std::vector<AuroraModel::Vertex> uniqueVertices{};
        for (int i = 0; i < numSides; i++) {
            float angle = i * glm::two_pi<float>() / numSides;
            uniqueVertices.push_back({{glm::cos(angle), glm::sin(angle)}});
        }
        uniqueVertices.push_back({});

        std::vector<AuroraModel::Vertex> vertices{};
        for (int i = 0; i < numSides; i++) {
            vertices.push_back(uniqueVertices[i]);
            vertices.push_back(uniqueVertices[(i + 1) % numSides]);
            vertices.push_back(uniqueVertices[numSides]);
        }
        return std::make_unique<AuroraModel>(device, vertices);
    }

    PhysicsApp::PhysicsApp() { loadGameObjects(); }

    PhysicsApp::~PhysicsApp() {}

    void PhysicsApp::run() {
        // create some models
        std::shared_ptr<AuroraModel> squareModel = createSquareModel(auroraDevice, {.5f, .0f});
        std::shared_ptr<AuroraModel> circleModel = createCircleModel(auroraDevice, 64);

        // create physics objects
        std::vector<AuroraGameObject> physicsObjects{};
        auto red = AuroraGameObject::createGameObject();
        red.transform2d.scale = glm::vec2{.05f};
        red.transform2d.translation = {.5f, .5f};
        red.color = {1.f, 0.f, 0.f};
        red.rigidBody2d.velocity = {-.5f, .0f};
        red.model = circleModel;
        physicsObjects.push_back(std::move(red));
        auto blue = AuroraGameObject::createGameObject();

        blue.transform2d.scale = glm::vec2{.05f};
        blue.transform2d.translation = {-.45f, -.25f};
        blue.color = {0.f, 0.f, 1.f};
        blue.rigidBody2d.velocity = {.5f, .0f};
        blue.model = circleModel;
        // blue.rigidBody2d.mass = 10.f;
        physicsObjects.push_back(std::move(blue));

        auto green = AuroraGameObject::createGameObject();
        green.transform2d.scale = glm::vec2{.05f};
        green.transform2d.translation = {.5f, -.5f};
        green.color = {0.f, 1.f, 0.f};
        green.rigidBody2d.velocity = {-1.0f, .0f};
        green.model = circleModel;
        // physicsObjects.push_back(std::move(green));

        // create vector field
        std::vector<AuroraGameObject> vectorField{};
        int gridCount = 40;
        for (int i = 0; i < gridCount; i++) {
            for (int j = 0; j < gridCount; j++) {
                auto vf = AuroraGameObject::createGameObject();
                vf.transform2d.scale = glm::vec2(0.005f);
                vf.transform2d.translation = {
                    -1.0f + (i + 0.5f) * 2.0f / gridCount,
                    -1.0f + (j + 0.5f) * 2.0f / gridCount
                };
                vf.color = glm::vec3(1.0f);
                vf.model = squareModel;
                vectorField.push_back(std::move(vf));
            }
        }

        GravityPhysicsSystem gravitySystem{0.81f};
        Vec2FieldSystem vecFieldSystem{};

        AuroraRenderSystem simpleRenderSystem{auroraDevice, auroraRenderer.getSwapChainRenderPass()};

        while (!auroraWindow.shouldClose()) {
            glfwPollEvents();

            if (auto commandBuffer = auroraRenderer.beginFrame()) {
                // update systems
                gravitySystem.update(physicsObjects, 1.f / 60 / 4, 5);
                vecFieldSystem.update(gravitySystem, physicsObjects, vectorField);

                // render system
                auroraRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, physicsObjects);
                simpleRenderSystem.renderGameObjects(commandBuffer, vectorField);
                auroraRenderer.endSwapChainRenderPass(commandBuffer);
                auroraRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(auroraDevice.device());
    }

    void PhysicsApp::loadGameObjects() {
        std::vector<AuroraModel::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
        auto auroraModel = std::make_shared<AuroraModel>(auroraDevice, vertices);

        auto triangle = AuroraGameObject::createGameObject();
        triangle.model = auroraModel;
        triangle.color = {.1f, .8f, .1f};
        triangle.transform2d.translation.x = .2f;
        triangle.transform2d.scale = {2.f, .5f};
        triangle.transform2d.rotation = .25f * glm::two_pi<float>();

        gameObjects.push_back(std::move(triangle));
    }
}
