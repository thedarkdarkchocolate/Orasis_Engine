#version 450

// ---- IN ATTRIBUTES -----

// ---- OUT ATTRIBUTES -----

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
const vec3 quad[6] = vec3[](
  vec3(-1.0, -1.0, 0.f),
  vec3(-1.0, 1.0, 0.f),
  vec3(1.0, -1.0, 0.f),
  vec3(1.0, -1.0, 0.f),
  vec3(-1.0, 1.0, 0.f),
  vec3(1.0, 1.0, 0.f)
);



void main() {

    gl_Position = push.model * vec4(quad[gl_VertexIndex], 1.f);


}