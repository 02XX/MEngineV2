#version 450

layout(location = 0) in vec3 inPosition;  // Location 0
layout(location = 1) in vec3 inNormal;    // Location 1
layout(location = 2) in vec2 inTexCoords; // Location 2

layout(location = 0) out vec3 outWorldPos;  // Location 0
layout(location = 1) out vec3 outNormal;   // Location 1
layout(location = 2) out vec2 outTexCoords; // Location 2

layout(set = 0, binding = 0) uniform UniformBuffer
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
}
transform;

layout(push_constant) uniform PushConstant
{
    mat4 modelMatrix;
}
pushConstant;
void main()
{
    gl_Position = vec4(inPosition, 1.0);
    vec4 worldPos = pushConstant.modelMatrix * vec4(inPosition, 1.0);
    outWorldPos = worldPos.xyz;
    outNormal = mat3(transpose(inverse(pushConstant.modelMatrix))) * inNormal;
    outTexCoords = inTexCoords;
}