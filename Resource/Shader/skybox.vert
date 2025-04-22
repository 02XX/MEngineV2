#version 450

layout(std140,set = 0, binding = 0) uniform CameraParam
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraPosition; // Camera position in world space
}
cameraParam;

// 硬编码 8 个顶点
const vec3 vertices[8] = vec3[](
    vec3(-1.0, -1.0, -1.0), // 0
    vec3( 1.0, -1.0, -1.0), // 1
    vec3(-1.0,  1.0, -1.0), // 2
    vec3( 1.0,  1.0, -1.0), // 3
    vec3(-1.0, -1.0,  1.0), // 4
    vec3( 1.0, -1.0,  1.0), // 5
    vec3(-1.0,  1.0,  1.0), // 6
    vec3( 1.0,  1.0,  1.0)  // 7
);
layout(location = 2) out vec3 outTexCoord;
// 硬编码索引（36 个索引，6 个面 × 2 个三角形 × 3 个顶点）
const uint indices[36] = uint[](
    // 前面
    0, 1, 2,  2, 1, 3,
    // 后面
    5, 4, 7,  7, 4, 6,
    // 左面
    4, 0, 6,  6, 0, 2,
    // 右面
    1, 5, 3,  3, 5, 7,
    // 上面
    2, 3, 6,  6, 3, 7,
    // 下面
    4, 5, 0,  0, 5, 1
);

void main() {
    // 根据 gl_VertexIndex 获取索引
    uint index = indices[gl_VertexIndex];
    vec3 position = vertices[index];

    // 移除视图矩阵的平移分量 
    // 移除视图矩阵的平移分量（仅保留旋转）
    // mat4 viewNoTranslation = mat4(mat3(cameraParam.viewMatrix));
    vec4 pos = cameraParam.projectionMatrix * cameraParam.viewMatrix * vec4(position, 1.0);

    // 确保深度值为 1.0（在裁剪空间中 z/w = 1.0）
    gl_Position = pos.xyww;
    
    // 确保天空盒深度值为 1.0

    // 传递纹理坐标
    outTexCoord = position;
}