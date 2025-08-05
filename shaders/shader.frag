#version 450

// ---- IN ATTRIBUTES -----
layout (location = 0) in vec3 fragPos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 fragColor;

// ---- OUT ATTRIBUTES -----
layout (location = 0) out vec4 outColor;



layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec3 lightPos;
    vec3 lightColor;
    vec3 cameraPos;
} ubo;


layout(push_constant) uniform Push {
    mat4 model;
} push;


// Local variables
const float ambient = 0.05;
const float specularStrength  = 0.5;
const float specularPow  = 64;


void main() {


    float fragLightDistance  = distance(ubo.lightPos , fragPos);
    vec3 lightDir = normalize(ubo.lightPos - fragPos);

    float diffuse = (1/pow(fragLightDistance, 2)) * max(dot(lightDir, normal), 0);

    vec3 viewDir = normalize(ubo.cameraPos - fragPos);

    float spec = pow(max(dot(-viewDir, reflect(lightDir, normal)) , 0), specularPow);

    vec3 specular = specularStrength * spec * ubo.lightColor;
    // vec3 specular = vec3(0);

    outColor = (diffuse + ambient + vec4(specular, 1)) * vec4(fragColor, 1);

}