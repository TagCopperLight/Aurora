#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D msdfTexture;

layout(push_constant) uniform Push {
    mat4 transform;
    vec3 color;
} push;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float screenPxDistance() {
    vec2 unitRange = vec2(4.0) / vec2(textureSize(msdfTexture, 0));
    vec2 screenTexSize = vec2(1.0) / fwidth(fragTexCoord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

void main() {
    vec3 msd = texture(msdfTexture, fragTexCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxDistance();
    float opacity = clamp((sd - 0.5) * screenPxDistance + 0.5, 0.0, 1.0);
    
    
    vec3 finalColor = fragColor.rgb * push.color;
    outColor = vec4(finalColor, fragColor.a * opacity);
}
