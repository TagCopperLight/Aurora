#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in mat4 instanceModelMatrix;
layout(location = 7) in vec4 instanceColor;

layout(location = 0) out vec4 fragColor;


layout(push_constant) uniform PushConstants {
    mat4 projectionViewMatrix;
} pc;

void main() {
    vec4 worldPosition = instanceModelMatrix * vec4(position, 1.0);
    gl_Position = pc.projectionViewMatrix * worldPosition;
    fragColor = color * instanceColor;
}