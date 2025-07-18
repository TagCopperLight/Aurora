#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform Push {
    mat4 transform;
    vec3 color;
} push;

void main() {
    // gl_Position = vec4(position, 0.0, 1.0);
    gl_Position = push.transform * vec4(position, 1.0);
    fragColor = color;
}