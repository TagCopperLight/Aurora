#pragma once

#include "aurora_component_interface.hpp"
#include "aurora_ui/components/aurora_circle.hpp"

#include <memory>
#include <vector>

namespace aurora {
    // The recent path of a moving object, drawn as dots that shrink and fade into the
    // background with age. Shapes are opaque, so fading is a blend towards the background color.
    class AuroraTrail : public AuroraComponentInterface {
        public:
            AuroraTrail(AuroraComponentInfo& componentInfo, size_t length, float radius, glm::vec4 color);

            // Adds the newest point, dropping the oldest one once the trail is full
            void push(glm::vec2 position);
            void clear();
            void setColor(glm::vec4 color);

            const std::string& getVertexShaderPath() const override {
                static const std::string vertexPath = "shaders/shader.vert.spv";
                return vertexPath;
            }

            const std::string& getFragmentShaderPath() const override {
                static const std::string fragmentPath = "shaders/shader.frag.spv";
                return fragmentPath;
            }

            VkPrimitiveTopology getTopology() const override {
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            }

        private:
            void refresh();

            std::vector<std::shared_ptr<AuroraCircle>> dots;
            std::vector<glm::vec2> points;
            size_t head = 0;
            size_t size = 0;
            float radius;
            glm::vec4 trailColor;
    };
}
