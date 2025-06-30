#version 450

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    mat4 transform;
    vec3 color;
} push;

void main() {
    // Create a smoother, more eye-pleasing gradient
    float gradient = 1.0 - fragColor.r;
    
    // Apply a smooth curve to make the transition more natural
    float alpha = smoothstep(0.0, 1.0, gradient) * 0.2;
    
    outColor = vec4(0.0, 0.0, 0.0, alpha);
}