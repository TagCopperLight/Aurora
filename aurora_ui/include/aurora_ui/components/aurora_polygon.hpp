#pragma once

#include "aurora_component_interface.hpp"

#include <vector>

namespace aurora {
    // A filled convex polygon, e.g. a triangle for an icon. Points are relative to the
    // component position, in order around the outline. Polygons with the same points share a model.
    class AuroraPolygon : public AuroraComponentInterface {
        public:
            AuroraPolygon(AuroraComponentInfo &componentInfo, std::vector<glm::vec2> points, glm::vec4 color);

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
            void initialize() override;

            std::vector<glm::vec2> points;
    };
}
