#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform Light {
    vec3 u_LightDir; // Should be normalized
};

layout(set = 0, binding = 2) uniform sampler2D u_Texture;

void main() {
    vec3 normal = normalize(fragNormal);
    float diffuse = max(dot(normal, normalize(-u_LightDir)), 0.1); // soft minimum
    vec3 texColor = texture(u_Texture, fragUV).rgb;
    outColor = vec4(texColor * diffuse, 1.0);
}
