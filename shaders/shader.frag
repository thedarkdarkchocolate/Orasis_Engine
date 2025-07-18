#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 fragPos;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    vec3 lightPos;
} ubo;


layout(push_constant) uniform Push {
    mat4 model;
} push;


void main() {

    vec3 lightDir = normalize(ubo.lightPos - fragPos);

    float diffuse = max(dot(lightDir, normal), 0);
    
    outColor = diffuse * vec4(fragColor, 1);

}