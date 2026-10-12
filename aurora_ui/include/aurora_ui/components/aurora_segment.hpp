#pragma once

#include "aurora_component_interface.hpp"

namespace aurora {
    // A straight line between two points. Every segment shares the same unit quad
    // (x in [0, 1], y in [-0.5, 0.5]), positioned at the start point, rotated along
    // the segment and scaled to (length, thickness).
    class AuroraSegment : public AuroraComponentInterface {
        public:
            AuroraSegment(AuroraComponentInfo &componentInfo, glm::vec4 color);

            void setEndpoints(glm::vec2 from, glm::vec2 to, float thickness);

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
    };
}
