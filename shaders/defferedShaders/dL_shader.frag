#version 450

// --- G-Buffer attachments as input attachments ---
layout(set = 1, binding = 0, input_attachment_index = 0) uniform subpassInput gPosition;
layout(set = 1, binding = 1, input_attachment_index = 1) uniform subpassInput gNormal;
layout(set = 1, binding = 2, input_attachment_index = 2) uniform subpassInput gAlbedo;

// ---- OUT ATTRIBUTES -----
layout(location = 0) out vec4 outColor;

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


// Local variables
const float ambient = 0.05;
const float specularStrength  = 0.5;
const float specularPow  = 64;


void main() {

    vec3 fragPos = subpassLoad(gPosition).xyz;
    vec3 normal = subpassLoad(gNormal).xyz;
    vec3 fragColor = subpassLoad(gAlbedo).xyz;
    
    float fragLightDistance  = distance(ubo.lightPos , fragPos);
    vec3 lightDir = normalize(ubo.lightPos - fragPos);

    float diffuse = (1/pow(fragLightDistance, 2)) * max(dot(lightDir, normal), 0);

    vec3 viewDir = normalize(ubo.cameraPos - fragPos);

    float spec = pow(max(dot(-viewDir, reflect(lightDir, normal)) , 0), specularPow);

    vec3 specular = specularStrength * spec * ubo.lightColor;
    outColor = (diffuse + ambient + vec4(specular, 1)) * vec4(fragColor, 1);
    // outColor = subpassLoad(gAlbedo);
}


