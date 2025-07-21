#version 450

// ---- OUT ATTRIBUTES -----
layout(location = 0) out vec2 fragOffset;

struct PointLight {
    vec4 position;  // ignore w
    vec4 color;     // w -> intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec3 lightPos;
    vec3 lightColor;
    vec3 cameraPos;
} ubo;

// Local variables
const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);


const float LIGHT_RADIUS = 0.3;

void main() {

    fragOffset = OFFSETS[gl_VertexIndex];

    vec3 cameraRight_w = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
    vec3 cameraUp_w    = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

    vec3 posWorld = ubo.lightPos 
        + LIGHT_RADIUS * fragOffset.x * cameraRight_w
        + LIGHT_RADIUS * fragOffset.y * cameraUp_w;
    
    gl_Position = ubo.projection * ubo.view * vec4(posWorld, 1.f);

}