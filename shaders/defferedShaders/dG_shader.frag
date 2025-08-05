#version 450

// ---- IN ATTRIBUTES -----
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 fragColor;

// ---- OUT ATTRIBUTES -----
layout(location = 0) out vec4 outPos;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec3 lightPos;
    vec3 lightColor;
    vec3 cameraPos;
} ubo;


// Push constant struct
layout(push_constant) uniform Push {
    mat4 model;         // transformation matrix from local to world space for model
} push;

// Local variables

void main() {

    outPos = vec4(aPos, 1.0);
    outNormal = vec4(normalize(aNormal), 1.f);
    outColor = vec4(clamp(fragColor, vec3(0), vec3(1)), 1.0);
}