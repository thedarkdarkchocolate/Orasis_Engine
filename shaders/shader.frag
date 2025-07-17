#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 fragPos;

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    mat4 transform; // projection * view * world(model)
    mat4 modelMatrix;
} push;


// Local variables
vec3 lightPos = {1.f, -3.5, -1.f};

void main() {

    vec3 lightDir = normalize(lightPos - fragPos);

    float diffuse = max(dot(lightDir, normal), 0);
    
    outColor = diffuse * vec4(fragColor, 1);

}