#version 450

layout(location = 0) in vec2 positions;
layout(location = 1) in vec3 color;

// Push constant struct
layout(push_constant) uniform Push {
    mat2 transform;
    vec2 offset;
    vec3 color;
} push;

void main() {
    gl_Position = vec4(push.transform * positions + push.offset, 0.0, 1);
}