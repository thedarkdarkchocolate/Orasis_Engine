#version 450

// ---- IN ATTRIBUTES -----
layout(location = 0) in vec2 fragOffset;

// ---- OUT ATTRIBUTES -----
layout (location = 0) out vec4 outColor;


layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec3 lightPos;
    vec3 lightColor;
    vec3 cameraPos;
} ubo;


void main() {

    if (sqrt(dot(fragOffset, fragOffset)) >= 1)
        discard;
    outColor = vec4(ubo.lightColor, 1.f);
}