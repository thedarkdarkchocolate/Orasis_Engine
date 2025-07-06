#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;


// Push constant struct
layout(push_constant) uniform Push {
    mat4 transform;
    vec3 color;
} push;

void main() {
    gl_Position = push.transform * vec4(aPos, 1.f);

    fragColor = color;
}