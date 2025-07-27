#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform componentUniform {
    mat4 transform;
    vec3 color;
} componentUbo;

void main() {
    gl_Position = componentUbo.transform * vec4(position, 1.0);
    fragColor = color * vec4(componentUbo.color, 1.0);
}