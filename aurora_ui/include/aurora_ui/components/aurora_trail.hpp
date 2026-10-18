#pragma once

#include "aurora_component_interface.hpp"
#include "aurora_ui/components/aurora_segment.hpp"

#include <memory>
#include <vector>

namespace aurora {
    // The recent path of a moving object, drawn as a line that thins and fades into the
    // background with age. Shapes are opaque, so fading is a blend towards the background color.
    class AuroraTrail : public AuroraComponentInterface {
        public:
            AuroraTrail(AuroraComponentInfo& componentInfo, size_t length, float thickness, glm::vec4 color);

            // Adds the newest point, dropping the oldest one once the trail is full. Without `connected`
            // no segment joins it to the previous point, e.g. when the object wraps around the view.
            void push(glm::vec2 point, bool connected = true);
            void clear();
            void setColor(glm::vec4 color);

            // Points are drawn at origin + point * scale, so they can be pushed in any unit and the
            // trail follows its view when it moves. Thickness stays in pixels. Defaults to pixels.
            void setMapping(glm::vec2 origin, glm::vec2 scale);

            // The newest point follows the pushes until it is `step` away from the one before it, so the
            // trail covers the same distance whatever the push rate. In point units, 0 keeps every push.
            void setMinStep(float step) { minStep = step; }

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

            std::vector<std::shared_ptr<AuroraSegment>> segments;
            std::vector<glm::vec2> points;
            std::vector<bool> connections;  // whether each point is joined to the one before it
            size_t head = 0;
            size_t size = 0;
            float thickness;
            glm::vec4 trailColor;
            glm::vec2 origin{0.f, 0.f};
            glm::vec2 scale{1.f, 1.f};
            float minStep = 0.f;
    };
}
