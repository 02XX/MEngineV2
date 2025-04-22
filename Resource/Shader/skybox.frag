#version 450
layout(location = 0) out vec4 outColor;
layout(location = 2) in vec3 inTexCoord;

layout(set = 0, binding = 2) uniform sampler2D skybox;
vec2 DirectionToUV(vec3 dir) {
    dir = normalize(dir); // 归一化方向向量
    float theta = atan(dir.z, dir.x); // 范围 [-π, π]
    float phi = acos(dir.y);          // 范围 [0, π]
    float u = (theta + 3.14159265) / (2.0 * 3.14159265); // 转换到 [0,1]
    float v = 1.0 - phi / 3.14159265; // 反转垂直方向
    return vec2(u, v);
}
void main() {
    // outColor = texture(skybox, DirectionToUV(inTexCoord));
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}