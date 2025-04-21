#version 450

// G-buffer
layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput inAlbedo;
layout(input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput inPosition;
layout(input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput inNormal;
layout(input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput inMetallicRoughness;
layout(input_attachment_index = 4, set = 1, binding = 4) uniform subpassInput inAO;
layout(input_attachment_index = 5, set = 1, binding = 5) uniform subpassInput inEmissive;
// // 光照参数
// layout(set = 0, binding = 0) uniform LightUBO {
//     vec4 lightPosition;   
//     vec4 lightColor;
// } ubo;

layout(location = 0) out vec4 outColor;

// vec3 calculateLighting(vec3 albedo, vec3 pos, vec3 normal, float metallic, float roughness) {
//     // 这里实现光照模型（例如 Cook-Torrance BRDF）
//     // 示例：简单漫反射
//     vec3 lightDir = normalize(ubo.lightPosition.xyz - pos);
//     float diff = max(dot(normal, lightDir), 0.0);
//     return albedo * ubo.lightColor.rgb * ubo.lightColor.w * diff;
// }

void main() {
    vec4 albedo = subpassLoad(inAlbedo);
    // vec4 position = subpassLoad(inPosition);
    // vec4 normal = subpassLoad(inNormal);
    // vec4 metallicRoughness = subpassLoad(inMetallicRoughness);
    outColor = albedo;
}