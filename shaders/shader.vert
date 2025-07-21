#version 450

// ---- IN ATTRIBUTES -----
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec2 aUV;

// ---- OUT ATTRIBUTES -----
layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 normal;

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

    gl_Position = ubo.projection * ubo.view * push.model * vec4(aPos, 1.f);

    
    fragColor = aColor;
    normal = normalize(aNormal);

    // Position of the vertex in world space and when it get passed to frag it gets interpolated 
    fragPos = vec3(push.model * vec4(aPos, 1.f));
}